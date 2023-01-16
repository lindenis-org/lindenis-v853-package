TARGET := fsck_msdos
SRC := main.c boot.c check.c dir.c fat.c

$(TARGET):
	$(CC) $(CFLAGS) -g -o $@ $(SRC)

.PHONY: all clean

all: $(TARGET)

clean: rm -f *.o ${TARGET}
