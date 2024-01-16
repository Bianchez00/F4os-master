// Created by nick on 17/11/2023.
#include <stdlib.h>

struct message {
    long mtype;
    pid_t pid;
    int row;
    int col;
    char token;
    char content[100];
};
