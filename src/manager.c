#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../incl/manager.h"

struct config {
    char *host;
    char *peer;
    char *key;
    char *pub;
    char *psk;
    char *port;
    char *allow;
};

Config new_config(void) 
{
    Config p = malloc(sizeof(struct config));
    if (!p) {
        printf("error: failed to allocate memory for new config structure.\n");
    }
    p->host = NULL;
    p->peer = NULL;
    p->key = NULL;
    p->pub = NULL;
    p->psk = NULL;
    p->port = NULL;
    p->allow = NULL;

    return p;
}

void clear_config(Config p)
{
    free(p->host);
    free(p->peer);
    free(p->key);
    free(p->pub);
    free(p->psk);
    free(p->port);
    free(p->allow);
    free(p);
}

int add_key(Config conf, Field key, char *s)
{
    char *p = malloc(strlen(s) + 1);
    if (!p) {
        printf("error: failed to allocate memory in config for %s\n", s);
        return 1;
    }
    strcpy(p, s);

    switch (key) {
        case HOST:              conf->host = p;
                                break;
        case PEER:              conf->peer = p;
                                break;
        case KEY:               conf->key = p;
                                break;
        case PUB:               conf->pub = p;
                                break;
        case PSK:               conf->psk = p;
                                break;
        case PORT:              conf->port = p;
                                break;
        case ALLOW:             conf->allow = p;
                                break;
        default:                break;     
    }

    return 0;
}

/*
write data to configs for specified interface
Field:  HOST
        SERVER
*/
int write_config(Config conf, Field dev, char *interface)
{
    // write data to /etc/wireguard/<interface>.conf for host
    // write data to ~/.conf/wireman/<interface>.conf for peer and create entry in host

    return 0;
}

/*
delete specified interface config, and remove entries peer entries from host if dev is PEER.
Field:  HOST
        SERVER
*/
int delete_config(Field dev, char *interface)
{
    // delete specified conf, if its a peer then delete peer entry from host interface.
}