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
    char *endpoint;
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
    p->endpoint = NULL;
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
    free(p->endpoint);
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
        case ENDPOINT:  conf->endpoint = p;
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
int write_config(Config conf, Client client, char *host, char *peer)
{
    FILE *f;
    Path *p, *temp;

    switch (client) {

        case HOST: 
            p = malloc(strlen(host) + 21);        // additional chars in path = 20
            if (!p) {
                printf("error: failed to allocate memory for path\n");
                return 1;
            }

            sprintf(p, ETC_WIREGUARD_CONF, host);
            euid_helper(GAIN);      // gain root
            f = fopen(p, "w");
            euid_helper(DROP);      // drop root
            if (!f) {
                printf("error: failed to open %s\n", p);
                return 1;
            }
            fprintf(f, "[Interface]\n");
            fprintf(f, "PrivateKey = %s\n", conf->key);
            fprintf(f, "Address = %s\n", conf->address);
            fprintf(f, "ListenPort = %s\n", conf->port);
            fclose(f);
            break;
        
        case PEER:
            temp = config_path(peer);
            if (!temp) {
                printf("%s in write_host\n", peer);
                return 1;
            }

            p = malloc(strlen(temp) + strlen(peer) + 7);
            if (!p) {
                printf("error: failed to allocate memory for path.\n");
                return 1;
            }
            sprintf(p, "%s/%s.conf", temp, peer);
            free(temp);

            f = fopen(p, "w");
            if (!f) {
                printf("error: failed to open %s\n", p);
                return 1;
            }
            fprintf(f, "[Interface]\n");
            fprintf(f, "PrivateKey = %s\n", conf->key);
            fprintf(f, "Address = %s\n", conf->address);
            fprintf(f, "\n[Peer]\n");
            //PublicKey from host
            fprintf(f, "PresharedKey = %s\n", conf->psk);
            fprintf(f, "Endpoint = %s\n", conf->endpoint);
            fprintf(f, "AllowedIPs = %s\n", conf->allow);
            fclose(f);

            // edit host config
            euid_helper(GAIN);
            f = file_copy(host);
            if (!f) {
                printf("error: file_copy returned garbage.\n");
                return 1;
            }
            fprintf(f, "\n[Peer]\n");
            fprintf(f, "PublicKey = %s\n", conf->pub);
            fprintf(f, "PresharedKey = %s\n", conf->psk);
            //fprintf(f, "AllowedIPs = %s\n", conf->allow); (/32)
            fclose(f);
            euid_helper(DROP);
            break;
            
        default:
            break;
    }
    free(p);
    
    return 0;
}