#include "../include/log.h"

void error(char *msg, int exit_no) {
    fprintf(stderr, "error message : %s - %s\n", msg, strerror(errno));
    exit(exit_no);
}