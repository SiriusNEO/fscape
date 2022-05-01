#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

char exit_path[2333];
char* answer_path = "secret_word";

int main() {
    time_t start_time = time(0);

    printf("[fscape] === GAME START! ===\n");
    printf("Find the informer, get the secret word from him and write it into the exit as fast as you can!\n");

    scanf("%s", exit_path);

    // printf("Path: %s\n", exit_path);

    int exit_fd = open(exit_path, O_RDWR);

    if (exit_fd == -1) {
        printf("open exit file failed.\n");
        return -1;
    }

    // mmap
    struct stat sb;

    if (stat(exit_path, &sb) == -1) {
		printf("stat exit file failed.\n");
		close(exit_fd);
		return -1;
	}

    char* exit_mmaped = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, exit_fd, 0);
    int exit_size = sb.st_size;

    if (exit_mmaped == (char *)-1) {
        printf("mmap exit file failed.\n");
        return -1;
    }

    // printf("%s\n", exit_mmaped);

    int answer_fd = open(answer_path, O_RDWR);

    if (answer_fd == -1) {
        printf("open secret word file failed.\n");
        return -1;
    }

    if (stat(answer_path, &sb) == -1) {
		printf("stat secret word file failed.\n");
		close(answer_fd);
		return -1;
	}

    char* answer_mmaped = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, answer_fd, 0);
    int ans_size = sb.st_size;

    if (answer_mmaped == (char *)-1) {
        printf("mmap secret word file failed.\n");
        return -1;
    }

    while (1) {    
        // check the condition

        /*
            int check = strcmp(exit_mmaped, answer_mmaped);

            if (check != 0) {
                printf("you: %s\n", exit_mmaped);
                printf("answer: %s\n", answer_mmaped);
            }
        */

        if (strcmp(exit_mmaped, answer_mmaped) == 0) {
            printf("\n");
            printf("Congratulations! Total time used: %ld(s)\n", time(0) - start_time - 1);

	        munmap(exit_mmaped, exit_size);
	        munmap(answer_mmaped, ans_size);

            return 0;
        }

        // show time
        printf("                   \r");
        printf("Time used: %ld(s)\r", time(0) - start_time);
        fflush(stdout);
        sleep(1);
    }

    return 0;
}