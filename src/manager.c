#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../incl/manager.h"
#include "../incl/root.h"
#include "../incl/util.h"

struct config {
    char *address;
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
    p->address = NULL;
    p->key = NULL;
    p->pub = NULL;
    p->psk = NULL;
    p->port = NULL;
    p->allow = NULL;

    return p;
}

void clear_config(Config p)
{
    free(p->address);
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
        case ADDRESS:   conf->address = p;
                        break;
        case KEY:       conf->key = p;
                        break;
        case PUB:       conf->pub = p;
                        break;
        case PSK:       conf->psk = p;
                        break;
        case PORT:      conf->port = p;
                        break;
        case ALLOW:     conf->allow = p;
                        break;
        default:        break;     
    }

    return 0;
}

/*
write data to configs for specified interface
Field:  HOST
        SERVER
*/
int write_host(Config host, char *interface)
{
    FILE *f;
    char *path = malloc(strlen(interface) + 21);        // additional chars in path = 20
    
    if (!path) {
        printf("error: failed to allocate memory for path\n");
        return 1;
    }
    sprintf(path, HOST_CONFIG, interface);

    euid_helper(GAIN);      // gain root
    f = fopen(path, "w");
    euid_helper(DROP);      // drop root
    if (!f) {
        printf("error: failed to open %s\n", path);
        free(path);
        return 1;
    }

    fprintf(f, "[Interface]\n");
    fprintf(f, "PrivateKey = %s\n", host->key);
    fprintf(f, "Address = %s\n", host->address);
    fprintf(f, "ListenPort = %s\n", host->port);

    free(path);
    fclose(f);
    
    return 0;
}

int write_peer(Config peer, char *interface) 
{
    // write configs to ~/.config/wireman/<interface>/<interface.conf>
    // add [Peer] to host
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