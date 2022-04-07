COMPILER = gcc

SRC_DIR = src
INCLUDE_DIR = include

SRC_FILES = $(SRC_DIR)/ffs_oper.c $(SRC_DIR)/v_disk.c $(SRC_DIR)/ffs.c

build: $(SRC_FILES)
	$(COMPILER) $(SRC_FILES) -o ffs `pkg-config fuse --cflags --libs`
	echo 'Compile Finish. Use ./ffs -f <mountpoint> to mount the fs.'	

clean:
	rm disk_image

test:
	./ffs -f /tmp/ffs/
