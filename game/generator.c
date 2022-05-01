#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const int DEP_LIMIT = 10, MDN_LIMIT = 10;

int max_depth, max_door_num, informer_pos, exit_pos;

int son[233][233], son_num[233], tree_incr;

int secret_word_pos, secret_word_len;
char secret_word[233][2333];

int people_word_len;
char people_word[233][2333];

FILE *bash_fp, *sw_fp, *path_fp;

int rand_range(int l, int r) {
    return rand() % (r - l + 1) + l;
}

void build_tree(int root, int depth) {
    if (son_num[root] != -1) {
        printf("build error: already built.\n");
        exit(-1);
    }

    // printf("build %d\n", root);

    son_num[root] = rand_range(1, max_door_num);

    int i;
    for (i = 1; i <= son_num[root]; ++i)
        son[root][i] = ++tree_incr;

    for (i = 1; i <= son_num[root]; ++i) {
        if (depth+1 < max_depth) 
            build_tree(son[root][i], depth+1);
    }   
}

void build_people_word() {
    people_word_len = 5;

    strcpy(people_word[0], "Oh. Hello. I'm just a passenger.");
    strcpy(people_word[1], "Where am I???? I just want to go home!!!");
    strcpy(people_word[2], "Our world is built with C Programming language. But I prefer Java.");
    strcpy(people_word[3], "ZZZZZZZZZZ. (sleeping)");
    strcpy(people_word[4], "Rooms, more rooms, and many many rooms!");
}

void build_secret_word() {
    secret_word_len = 12;

    strcpy(secret_word[0], "Moon");
    strcpy(secret_word[1], "Love");
    strcpy(secret_word[2], "Drug");
    strcpy(secret_word[3], "Bomb");
    strcpy(secret_word[4], "Butterfly");
    strcpy(secret_word[5], "Flower");
    strcpy(secret_word[6], "King");
    strcpy(secret_word[7], "Hate");
    strcpy(secret_word[8], "42");
    strcpy(secret_word[9], "Virus");
    strcpy(secret_word[10], "City");
    strcpy(secret_word[11], "Pain");

    secret_word_pos = rand_range(0, secret_word_len-1);

    // printf("%s\n", secret_word[secret_word_pos]);

    fprintf(sw_fp, secret_word[secret_word_pos]);
    fprintf(sw_fp, "\n");
}

int stk[2333], top;

void make_bash(int root) {
    fprintf(bash_fp, "mkdir room%d\n", root);
    fprintf(bash_fp, "cd room%d\n", root);
    stk[++top] = root;

    if (informer_pos == root) {
        fprintf(bash_fp, "echo \"Listen, the secret word is: %s\" > informer\n", secret_word[secret_word_pos]);
    }

    if (exit_pos == root) {
        // to insure the file is not empty
        fprintf(bash_fp, "echo \"e\" > exit\n");
        int i;
        for (i = 1; i <= top; ++i) {
            fprintf(path_fp, "room%d/", stk[i]);
        }
        fprintf(path_fp, "exit");
    }

    int gen_people = rand_range(1, 4);
    if (gen_people == 1) {
        int people_word_pos = rand_range(0, people_word_len-1);
        fprintf(bash_fp, "echo \"%s\" > NPC\n", people_word[people_word_pos]);
    }

    int i;
    for (i = 1; i <= son_num[root]; ++i) {
        make_bash(son[root][i]);
    }

    fprintf(bash_fp, "cd ..\n");
    --top;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: ./generator [max_depth] [max_door_num]\n");
        return 0;
    }

    max_depth = atoi(argv[1]);
    max_door_num = atoi(argv[2]);

    if (max_depth > DEP_LIMIT || max_door_num > MDN_LIMIT) {
        printf("ARG exceeds the limit.\n");
        return 0;
    }

    bash_fp =  fopen("gen.sh", "w");
    sw_fp   =  fopen("secret_word", "w");
    path_fp   =  fopen("path_cache", "a");
    fseek(path_fp, -1, SEEK_END);

    srand(time(NULL));
    
    memset(son_num, -1, sizeof(son_num));

    ++tree_incr;
    build_tree(1, 1);
    
    /*
    int i, j;
    for (i = 1; i <= tree_incr; ++i) {
        printf("%d: ", i);
        for (j = 1; j <= son_num[i]; ++j) 
            printf("%d ", son[i][j]);
        printf("\n");
    }

    return 0;
    */

    build_secret_word();
    build_people_word();

    // put the informer

    informer_pos = rand_range(1, tree_incr);

    // put the exit
    // the exit should not in the same pos with the informer

    exit_pos = rand_range(1, tree_incr);
    while (exit_pos == informer_pos) {
        exit_pos = rand_range(1, tree_incr);
    }

    // make the bash
    make_bash(1);

    fclose(bash_fp);
    fclose(sw_fp);
    fclose(path_fp);

    return 0;
}