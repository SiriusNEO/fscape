#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h> 

char exit_path[2333];

int main(int argc, char *argv[]) {

	int fd = -1;
	struct stat sb;
	char *mmaped = NULL;

    extern int errno;

	scanf("%s", exit_path);

	fd = open(exit_path, O_RDWR);
	if (fd == -1) {
		fprintf(stderr, "fail on open %s\n", argv[1]);
		return -1;
	}

	if (stat(argv[1], &sb) == -1) {
		fprintf(stderr, "fail on stat %s\n", argv[1]);
		close(fd);
		return -1;
	}

    printf("%d\n", sb.st_size);

	mmaped = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    int fd1 = open("secret_word", O_RDWR);
	char* mmaped1 = (char *)mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
	if (mmaped == (char *)-1) {
		fprintf(stderr, "fail on mmap %s, error = %d\n", argv[1], errno);
		close(fd);
		return -1;
	}

	printf("%d\n", strcmp(mmaped, mmaped1));

	munmap(mmaped, sb.st_size);

	return 0;
}