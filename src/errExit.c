#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "errExit.h"

void errExit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}