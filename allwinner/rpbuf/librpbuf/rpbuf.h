#ifndef __UAPI_RPBUF_H__
#define __UAPI_RPBUF_H__

#include <linux/ioctl.h>
#include <linux/types.h>

#define RPBUF_NAME_SIZE	32	/* include trailing '\0' */

struct rpbuf_buffer_info {
	char name[RPBUF_NAME_SIZE];
	__u32 len;
};

struct rpbuf_buffer_xfer {
	__u32 offset;
	__u32 data_len;
	__s32 timeout_ms;
};

#define RPBUF_CTRL_DEV_IOCTL_MAGIC	0xb8
#define RPBUF_CTRL_DEV_IOCTL_CREATE_BUF \
	_IOW(RPBUF_CTRL_DEV_IOCTL_MAGIC, 0x1, struct rpbuf_buffer_info)
#define RPBUF_CTRL_DEV_IOCTL_DESTROY_BUF \
	_IOW(RPBUF_CTRL_DEV_IOCTL_MAGIC, 0x2, struct rpbuf_buffer_info)

#define RPBUF_BUF_DEV_IOCTL_MAGIC	0xb9
#define RPBUF_BUF_DEV_IOCTL_CHECK_AVAIL \
	_IOR(RPBUF_BUF_DEV_IOCTL_MAGIC, 0x1, __u8)
#define RPBUF_BUF_DEV_IOCTL_TRANSMIT_BUF \
	_IOW(RPBUF_BUF_DEV_IOCTL_MAGIC, 0x2, struct rpbuf_buffer_xfer)
#define RPBUF_BUF_DEV_IOCTL_RECEIVE_BUF \
	_IOWR(RPBUF_BUF_DEV_IOCTL_MAGIC, 0x3, struct rpbuf_buffer_xfer)

#endif
