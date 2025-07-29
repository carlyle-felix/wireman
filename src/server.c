#include <stdio.h>

#include "../incl/server.h"

int add_server(char *interface) {
    printf("info: creating new server \"%s\"\n", interface);
    return 0;
}

int del_server(char *interface) {
    printf("info: removing server \"%s\"\n", interface);
    return 0;
}