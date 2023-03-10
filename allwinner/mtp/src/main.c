//#define DEBUG
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <linux/netlink.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "MtpServer.h"
#include "Disk/Disk.h"

#include <pthread.h>


#define KB(size)	(size*1024ULL)
#define MB(size)	(KB(size)*1024ULL)

#define AID_MEDIA_RW	1023

#ifdef MTP_SERVER_DAEMON

static struct MtpStorage *storage;

typedef struct {
	struct MtpStorage *storage;
	uint32_t id;
	char path[256];
	char description[32];
	uint64_t reserve_space;
	uint64_t max_file_size;
}mtp_storage_t;


enum {
	USB_STATE_UNKNOWN = 0,
	USB_STATE_CONNECTED = 1,
	USB_STATE_DISCONNECTED,
};

typedef struct {
	struct MtpServer *server;
	mtp_storage_t *storage_array;
	uint32_t storage_array_size;
	int controlFd;
	pthread_t tid;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int usb_state;
}mtp_handle_t;

static mtp_storage_t gStorageArray[] = {
	{NULL, 65537, "/mnt/UDISK", "Tina存储设备", MB(2), MB(4096)},
	//{NULL, 65539, "/boot", "boot", 0, 0},
	//{NULL, 65540, "/boot-res", "boot-res", 0, 0},
};

static mtp_handle_t gMtpHandle = {
	.server = NULL,
	.storage_array = gStorageArray,
	.storage_array_size = sizeof(gStorageArray)/sizeof(mtp_storage_t),
	.controlFd = -1,
};

static int mtp_server_init();
static void mtp_server_exit();
static int mtp_desc_write(void)
{
	int fd;

	if (gMtpHandle.controlFd > 0) {
		close(gMtpHandle.controlFd);
		gMtpHandle.controlFd = -1;
	}
	fd = open("/dev/usb-ffs/mtp/ep0", O_RDWR);
	if (fd < 0)
		return -1;
	/* write descriptors */
	writeDescroptors(fd);
	gMtpHandle.controlFd = fd;
	return 0;

}

static int mtp_server_init()
{
	uint32_t i;

	if (gMtpHandle.server != NULL) {
		printf("mtp already init\n");
		return -1;
	}

	printf("MtpServer init!\n");
	/* MtpServer init */
	gMtpHandle.server = MtpServerInit(AID_MEDIA_RW, 0666, 0775, gMtpHandle.controlFd);
	if (!gMtpHandle.server)
		return -1;
	/* MtpStorage init */
	for (i = 0; i < gMtpHandle.storage_array_size; i++) {
		mtp_storage_t *storage = &gMtpHandle.storage_array[i];
		storage->storage =
			MtpStorageInit(storage->id, storage->path,
					storage->description,
					storage->reserve_space,
					storage->max_file_size);
		if (!storage->storage)
			return -1;
		/* Add storage into MtpServer */
		MtpServerAddStorage(storage->storage, gMtpHandle.server);
		/* Add storage into MtpDatabase */
		MtpDataBaseAddStorage(storage->storage, gMtpHandle.server->mDataBase);
	}

	gMtpHandle.server->run(gMtpHandle.server);

	return 0;
}

static void set_timespace(struct timespec *ts, unsigned int ms)
{
	struct timeval tv;
	unsigned long us;

	gettimeofday(&tv, NULL);
	us = tv.tv_usec + ms * 1000UL;
	ts->tv_sec = tv.tv_sec + us / 1000000UL;
	ts->tv_nsec = (us % 1000000UL) * 1000UL;
}

static int mtp_cond_wait(pthread_mutex_t *mutex, pthread_cond_t *cond, int ms)
{
	int ret = 0;
	struct timespec ts;
	pthread_mutex_lock(mutex);
	if (ms != 0) {
		set_timespace(&ts, ms);
		ret = pthread_cond_timedwait(cond, mutex, &ts);
	} else {
		ret = pthread_cond_wait(cond, mutex);
	}
	pthread_mutex_unlock(mutex);
	return ret;
}

static void mtp_set_usb_state(int state)
{
	pthread_mutex_t *mutex = &gMtpHandle.mutex;
	pthread_cond_t *cond = &gMtpHandle.cond;

	pthread_mutex_lock(mutex);
	gMtpHandle.usb_state = state;
	pthread_cond_signal(cond);
	pthread_mutex_unlock(mutex);
}

#define MTP_CONNECT_DEBOUNCE_TIME	(500)
static void *MtpServerControlThread(void *arg)
{
	int ret;
	int state = USB_STATE_UNKNOWN;
	pthread_mutex_t *mutex = &gMtpHandle.mutex;
	pthread_cond_t *cond = &gMtpHandle.cond;
	int ms = 0;

	while (1) {
		ret = mtp_cond_wait(mutex, cond, ms);
		if (ret == ETIMEDOUT) {
			printf("wait cond timeout %d ms\n", ms);
			if (gMtpHandle.usb_state == USB_STATE_CONNECTED) {
				mtp_server_exit();
				mtp_server_init();
			}
			ms = 0;
			continue;
		} else if (ret != 0) {
			fprintf(stderr, "cond wait return %d, exit", ret);
			break;
		}
		DLOG("usb state change: %d -> %d, ret=%d\n", state, gMtpHandle.usb_state, ret);
		state = gMtpHandle.usb_state;
		switch (state) {
		case USB_STATE_UNKNOWN:
			break;
		case USB_STATE_CONNECTED:
			ms = MTP_CONNECT_DEBOUNCE_TIME;
			continue;
		case USB_STATE_DISCONNECTED:
			mtp_server_exit();
			break;
		}
		ms = 0;
	}
}

static void mtp_control_thread_create(void)
{
	pthread_mutex_init(&gMtpHandle.mutex, NULL);
	pthread_cond_init(&gMtpHandle.cond, NULL);
	pthread_create(&gMtpHandle.tid, NULL, MtpServerControlThread, NULL);
}

static void mtp_server_exit()
{
	uint32_t i;

	if (!gMtpHandle.server)
		return;
	printf("MtpServer exit!\n");

	MtpServerRelease(gMtpHandle.server);
	gMtpHandle.server = NULL;

	for (i = 0; i < gMtpHandle.storage_array_size; i++) {
		mtp_storage_t *storage = &gMtpHandle.storage_array[i];
		if (storage->storage != NULL) {
			MtpStorageRelease(storage->storage);
			storage->storage = NULL;
		}
	}
}
static int tina_usb_uevent_socket(void)
{
	struct sockaddr_nl nls;
	int nlbufsize = 64*1024;
	int s;

	memset(&nls, 0, sizeof(struct sockaddr_nl));
	nls.nl_family = AF_NETLINK;
	nls.nl_pid = getpid();
	nls.nl_groups = -1;

	if ((s = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_KOBJECT_UEVENT)) == -1) {
		printf("Failed to open socket: %s\n", strerror(errno));
		return -1;
	}
	if (setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &nlbufsize, sizeof(nlbufsize))) {
		printf("Failed to resize receive buffer: %s\n", strerror(errno));
		return -1;
	}
	if (bind(s, (void *)&nls, sizeof(struct sockaddr_nl))) {
		printf("Failed to bind socket: %s\n", strerror(errno));
		return -1;
	}
	fcntl(s, F_SETFL, O_NONBLOCK);

	return s;
}


static int tina_usb_uevent_detect(int socket)
{
#define UEVENT_MSG_LEN 1024
	char buf[UEVENT_MSG_LEN];
	int len, i = 0;

	memset(buf, 0, sizeof(buf));
	len = recv(socket, buf, sizeof(buf), MSG_DONTWAIT);
	if (len < 1)
		return -1;
	if (len >= UEVENT_MSG_LEN)
		return -1;
	buf[len] = '\0';
#if 0
        printf("=========================\n");
{
        int l=0;
        printf("recv uevent:\n");
        while(l < len) {
                printf("%s\n", &buf[l]);
                l += strlen(&buf[l]) + 1;
        }
        printf("=========================\n");
}
#endif

	while (i < len) {
		int l = strlen(buf + i)+1;
#if 0
		if (strstr(&buf[i], "USB_STATE=CONNECTED"))
			return USB_STATE_CONNECTED;
#else
		if (strstr(&buf[i], "USB_STATE=CONFIGURED"))
			return USB_STATE_CONNECTED;
#endif
		if (strstr(&buf[i], "USB_STATE=DISCONNECTED"))
			return USB_STATE_DISCONNECTED;
		i += l;
	}

	return -1;
}

static void usb_uevent_handle(int arg)
{
	int socket = arg;
	int ret = -1;
	ret = tina_usb_uevent_detect(socket);
	switch (ret) {
		case USB_STATE_CONNECTED:
#if 0
			printf("sleep...");
			usleep(500000);
			if (mtp_server_init() != 0) {
				printf("mtp server init failed\n");
				return ;
			}

#else
			printf("recv connected state\n");
			mtp_set_usb_state(USB_STATE_CONNECTED);
#endif
			break;
		case USB_STATE_DISCONNECTED:
#if 0
			mtp_server_exit();
#else
			printf("recv disconnected state\n");
			mtp_set_usb_state(USB_STATE_DISCONNECTED);
#endif
			break;
	}
	return;
}

typedef struct {
	uint32_t action;	/* 0:add, 1:remove, 2:update */
	uint32_t type;		/* 0:file, 2:dir */
	uint32_t srcPathLen;	/* src path buffer length */
	uint32_t destPathLen;	/* dest path buffer length */
	char *path;		/* object path */
}mtp_command_t;

#define MTP_FIFO_NAME	"/tmp/.mtp_fifo"

static void mtp_local_fs_update_handle(int fd)
{
	int ret;
	uint32_t id, i;
	char buf[512];
	char *srcPath = NULL, *destPath = NULL;
	mtp_command_t *command;
	struct MtpServer *server = NULL;
	struct MtpStorage *storage = NULL;

	mtp_tools_function_t func;
	enum {
		MTP_TOOLS_TYPE_FILE = 0,
		MTP_TOOLS_TYPE_DIR,
	} type;


	memset(buf, 0, sizeof(buf));
	ret = read(fd, buf, sizeof(buf));
	command = (mtp_command_t *)buf;
	printf("ret = %d, command: action:%u, type:%u, srcPathLen:%u, destPathLen:%u\n",
		ret, command->action, command->type, command->srcPathLen, command->destPathLen);

	if (ret < sizeof(mtp_command_t)) {
		printf("invalid mtp command size!\n");
		return ;
	}
	if (command->action == MTP_TOOLS_FUNCTION_CONNECT) {
		mtp_desc_write();
		return;
	}

	command->path = (char *)&command[1];
	printf("sLen:%u, dLen:%u, path: %s\n", command->srcPathLen, command->destPathLen, command->path);

	if (command->srcPathLen > 1) {
		srcPath = command->path;
	} else {
		printf("invalid src path length!\n");
		return ;
	}
	if (command->destPathLen > 1)
		destPath = command->path + command->srcPathLen;

	func = command->action;

	if (command->type == MTP_TOOLS_TYPE_FILE)
		type = MTP_FORMAT_UNDEFINED;
	else if (command->type == MTP_TOOLS_TYPE_DIR)
		type = MTP_FORMAT_ASSOCIATION;

	for (i = 0; i < gMtpHandle.storage_array_size; i++) {
		char *p = gMtpHandle.storage_array[i].path;
		if (!strncmp(p, srcPath, strlen(p))) {
			id = gMtpHandle.storage_array[i].id;
			storage = gMtpHandle.storage_array[i].storage;
		}
	}

	server = gMtpHandle.server;
	if (!server) {
		DLOG("MtpServer not connect.");
		return;
	}

	/*
	 * 新建文件，目录: ADD
	 * 删除文件，目录：REMOVE
	 * 更新文件，目录: UPDATE
	 *
	 * */

	switch (func) {
	case MTP_TOOLS_FUNCTION_ADD:
	case MTP_TOOLS_FUNCTION_REMOVE:
	case MTP_TOOLS_FUNCTION_UPDATE:
		ret = MtpToolsCommandControl(func, srcPath, type,
				id, storage, server);
		break;
	case MTP_TOOLS_FUNCTION_CUT:
		ret = MtpToolsCommandControl(MTP_TOOLS_FUNCTION_ADD, srcPath, type,
				id, storage, server);
		if (ret != 0)
			break;
		ret = MtpToolsCommandControl(MTP_TOOLS_FUNCTION_REMOVE, srcPath, type,
				id, storage, server);
		break;
	case MTP_TOOLS_FUNCTION_COPY:
		ret = MtpToolsCommandControl(MTP_TOOLS_FUNCTION_ADD, srcPath, type,
				id, storage, server);
		break;
	default:
		ret = -1;
		printf("unknow mtp tools function:%d\n", func);
		break;
	}
	if (ret < 0)
		printf("handle mtp command failed\n");
	return ;
}

#define s2ms(n) (n*1000)
#define MAX_EPOLL_EVENTS 40

int epollfd;
int event_cnt;

void register_usb_state_event(int socket)
{
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = socket;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, socket, &ev) == -1) {
		printf("epoll_ctl socket failed\n");
		close(epollfd);
		exit(1);
	}
	event_cnt++;
}



void register_local_fs_update_event(int fd)
{
	struct epoll_event ev;

	ev.events = EPOLLET | EPOLLIN;
	ev.data.fd = fd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		printf("epoll_ctl pipe_r failed\n");
		close(epollfd);
		exit(1);
	}
	event_cnt++;
}

int main()
{
	int socket;
	int timeout = s2ms(20), event_num = 0;
	struct epoll_event events[MAX_EPOLL_EVENTS];
	int fd;

	epollfd = epoll_create(MAX_EPOLL_EVENTS);
	if (epollfd == -1) {
		printf("epoll_create failed\n");
		return -1;
	}

	/* open netlink socket */
	socket = tina_usb_uevent_socket();
	if (socket < 0)
		return -1;

	/* create fifo */
	if (access(MTP_FIFO_NAME, F_OK) == -1) {
		if (mkfifo(MTP_FIFO_NAME, 0600) != 0) {
			printf("mkfifo %s failed\n", MTP_FIFO_NAME);
			return -1;
		}
	}
	/* open fifo */
	fd = open(MTP_FIFO_NAME, O_RDONLY | O_NONBLOCK, 0);
	if (fd < 0) {
		printf("open %s failed\n", MTP_FIFO_NAME);
		return -1;
	}

	/* listen USB uevent */
	register_usb_state_event(socket);
	/* listen local fs update command from pipe */
	register_local_fs_update_event(fd);
	/* Mtp Server thread */
	mtp_control_thread_create();

	mtp_desc_write();

	while(1) {
		int i;

		event_num = epoll_wait(epollfd, events, event_cnt, timeout);

		if (event_num == -1)
			break;
		for (i = 0; i < event_num; i++) {
#if 0
			if (events[i].data.ptr)
				(*(void (*)(int))events[i].data.ptr) (socket);
#endif
			if (events[i].data.fd == socket)
				usb_uevent_handle(socket);
			else if (events[i].data.fd == fd)
				mtp_local_fs_update_handle(fd);
		}
	}

	unlink(MTP_FIFO_NAME);
	close(socket);
	return 0;
}


#else

int main()
{
#if 1
	struct MtpServer *mserver;
	struct MtpStorage *mStorage;

	mserver = MtpServerInit(AID_MEDIA_RW, 0664, 0775);

	if (!mserver)
		goto err;

	//mStorage = MtpStorageInit(65537, "/mnt/UDISK", "Tina存储设备", 0, MB(1));
	mStorage = MtpStorageInit(65537, "/mnt/UDISK", "Tina存储设备", 0, 0);
	MtpServerAddStorage(mStorage, mserver);
	MtpDataBaseAddStorage(mStorage, mserver->mDataBase);

	mserver->run(mserver);

err:
	sleep(60);
	printf("MtpServer exit!\n");
	MtpServerRelease(mserver);
	mserver = NULL;
#else
	while (1) {
		struct MtpServer *mserver = NULL;
		struct MtpStorage *mStorage = NULL;
		int timeout = 20;

		printf("MtpServer init!\n");
		mserver = MtpServerInit(AID_MEDIA_RW, 0664, 0775);

		if (!mserver)
			goto err;

		//mStorage = MtpStorageInit(65537, "/mnt/UDISK", "Tina存储设备", 0, MB(1));
		DLOG("");
		mStorage = MtpStorageInit(65537, "/mnt/UDISK", "Tina存储设备", 0, 0);
		DLOG("");
		MtpServerAddStorage(mStorage, mserver);
		DLOG("");
		MtpDataBaseAddStorage(mStorage, mserver->mDataBase);
		DLOG("");

		mserver->run(mserver);

		printf("sleep %d second\n", timeout);
		sleep(timeout);
		//getchar();
	err:
		printf("MtpServer exit!\n");
		MtpStorageRelease(mStorage);
		MtpServerRelease(mserver);
		break;
	}
#endif
}
#endif
