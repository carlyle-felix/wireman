// TODO: merge with wireman.c.
#include <stdio.h>
#include <stdlib.h>

#include "../incl/server.h"
#include "../incl/hostip.h"
#include "../incl/manager.h"

#define UDP_SIZE 5
#define UDP 51820

/*
collect hostname, key, pub and port.
collect public ip and store it somewhere for later use during peer setup.
*/
int add_server(char *interface) 
{
    Config conf = new_config();
    char *key, *port;
    int res = 1, i = UDP;

    printf("info: creating new server \"%s\"\n", interface);

    // add interface name to conf
    res = add_key(conf, HOST, interface);
    if (!res) {
        clear_config(conf);
        return 1;
    }
    // create keys here
    //res = add_key(conf, KEY, key);
    //if (!res) {
    //    clear_config(conf);
    //    return 1;
    //}

    // prompt user for port (default to 51820)
    printf("Input UDP (default %d): ", UDP);
    scanf("%d", &i); 
    port = malloc(UDP_SIZE + 1);
    sprintf(port, "%d", i);
    res = add_key(conf, PORT, port);
    if (!res) {
        clear_config(conf);
        return 1;
    }

    write_config(conf, HOST, interface);
    clear_config(conf);
    return 0;
}

int del_server(char *interface) 
{
    printf("info: removing server \"%s\"\n", interface);

    // delete host
    return 0;
}

int add_peer(char *peer) 
{
    printf("info: adding peer \"%s\"\n", peer);

    // collect and add data to conf
    return 0;
}

int del_peer(char *peer) 
{
    printf("info: removing peer \"%s\"\n", peer);

    // delete peer
    return 0;
}