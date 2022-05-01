COMPILER = gcc

SRC_DIR = src
INCLUDE_DIR = include
GAME_DIR = game

SRC_FILES = $(SRC_DIR)/ffs_oper.c $(SRC_DIR)/v_disk.c $(SRC_DIR)/ffs.c
GEN = $(GAME_DIR)/generator.c
SERVER = $(GAME_DIR)/server.c

build: $(SRC_FILES)
	$(COMPILER) $(SRC_FILES) -o ffs `pkg-config fuse --cflags --libs`
	$(COMPILER) $(GEN) -o generator
	$(COMPILER) $(SERVER) -o server
	echo 'Compile Finish. Use ./ffs -f <mountpoint> to mount the fs.'	

clean:
	umount /tmp/ffs -f
	rm disk_image

test:
	./ffs -f /tmp/ffs/
