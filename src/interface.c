// TODO: merge with wireman.c.
#include <stdio.h>
#include <stdlib.h>

#include "../incl/hostip.h"
#include "../incl/interface.h"
#include "../incl/manager.h"

#define UDP_LEN 5
#define DEFAULT_UDP "51820"

/*
collect hostname, key, pub and port.
collect public ip and store it somewhere for later use during peer setup.
*/
int add_host(char *host) 
{
    Config conf = new_config();
    char c, port[UDP_LEN + 1] = DEFAULT_UDP;
    int i, res;

    printf("info: creating new server \"%s\"\n", host);

    // generate and store keys
    res = keygen(conf, host);
    if (res) {
        return res;
    }
    
    // get tunnel address
    res = tunnel_address(conf);
    if (res) {
        return res;
    }

    // prompt user for port (default to 51820).
    printf("Input UDP (default %s): ", DEFAULT_UDP);    // TODO: fix default, use getchar.
    for (i = 0; (c = getchar()) != '\n'; i++) {
        port[i] = c;
    }
    res = add_key(conf, PORT, port);
    if (res) {
        printf("error: failed to add port.\n");
        clear_config(conf);
        return res;
    }

    printf("listening port: %s\n", port);

    write_config(conf, HOST, host, NULL);
    clear_config(conf);
    return 0;
}

/*
*/
int add_peer(char *host, char *peer) 
{
    Config conf = new_config();
    char c, *endpoint, allow[MAX_BUFFER] = "0.0.0.0/0, ::/0";   // TODO: read host <interface>.conf for number of peers and set accordingly.
    int i, res;

    // generate and store keys
    res = keygen(conf, peer);
    if (res) {
        return res;
    }

    // get tunnel address
    res = tunnel_address(conf);
    if (res) {
        return res;
    }

    endpoint = host_ip();
    res = add_key(conf, ENDPOINT, endpoint);
    if (res) {
        printf("error: failed to add endpoint.\n");
        clear_config(conf);
        return res;
    }
    free(endpoint);

    // set allowedIPs for peer.
    printf("Input allowed IPs for peer (default: 0.0.0.0/0, ::/0): ");
    for (i = 0; (c = getchar()) != '\n'; i++) {
        allow[i] = c;
    }

    res = add_key(conf, ALLOW, allow);
    if (res) {
        printf("error: failed to add AllowedIP (peer).\n");
        return res;
    }

    write_config(conf, PEER, host, peer);
    clear_config(conf);
    return 0;
}
