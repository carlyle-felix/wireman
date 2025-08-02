#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


#include "../incl/manager.h"
#include "../incl/root.h"
#include "../incl/wireguard.h"
#include "../incl/util.h"

#define ETC_WIREGUARD "/etc/wireguard/"
#define ETC_WIREGUARD_CONF "/etc/wireguard/%s.conf"
#define IP_LEN 15

extern Path *wireman;

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
    Path *p;
    char *str, *key;

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
            Path *temp = config_path(peer);
            if (!temp) {
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
            
            // retrieve host public key.
            key = read_key(host, BASE64PUB);
            fprintf(f, "PublicKey = %s\n", key);
            free(key);

            fprintf(f, "PresharedKey = %s\n", conf->psk);
            fprintf(f, "Endpoint = %s\n", conf->endpoint);
            fprintf(f, "AllowedIPs = %s\n", conf->allow);
            fclose(f);

            // modify /etc/wireguard/<interface>.conf.
            euid_helper(GAIN);
            f = file_copy(host);
        
            fprintf(f, "\n[Peer]\n");
            fprintf(f, "PublicKey = %s\n", conf->pub);
            fprintf(f, "PresharedKey = %s\n", conf->psk);
            
            str = conf->address;
            // change subnet on AllowedIP in host
            while (*str++ != '/');
            *str++ = '3';
            *str++ = '2';
            *str = '\0';     // make sure.
            fprintf(f, "AllowedIPs = %s\n", conf->address);

            fclose(f);
            euid_helper(DROP);
            
            break;
            
        default:
            break;
    }
    free(p);
    
    return 0;
}


/*
copy contents of old <host_interface>.conf into new
NOTE: caller must close returned file.
*/
FILE *file_copy(char *interface)
{
    FILE *old_file, *new_file;
    Path *old_conf, *new_conf, *temp_conf;
    char buffer[MAX_BUFFER];

    old_conf = malloc(strlen(ETC_WIREGUARD_CONF) + strlen(interface) + 1);
    if (!old_conf) {
        printf("error: failed to allocate memory for old file pointer\n");
        return NULL;
    }
    sprintf(old_conf, ETC_WIREGUARD_CONF, interface);

    temp_conf = malloc(strlen(ETC_WIREGUARD_CONF) + strlen(interface) + 4);
    if (!temp_conf) {
        printf("error: failed to allocate memory for temp file pointer\n");
        return NULL;
    }
    sprintf(temp_conf, ETC_WIREGUARD_CONF".old", interface);

    rename(old_conf, temp_conf);
    remove(old_conf); 
    free(old_conf);

    old_file = fopen(temp_conf, "r");
    if (!old_file) {
        printf("error: unable to open old %s.conf\n", interface);
        return NULL;
    }

    new_conf = malloc(strlen(ETC_WIREGUARD_CONF) + strlen(interface) + 1);
    if (!new_conf) {
        printf("error: failed to allocate memory for new file pointer\n");
        return NULL;
    }
    sprintf(new_conf, ETC_WIREGUARD_CONF, interface);

    new_file = fopen(new_conf, "w");
    if (!new_file) {
        printf("error: unable to open old %s.conf", interface);
        return NULL;
    }
    free(new_conf);

    while (fgets(buffer, MAX_BUFFER, old_file)) {
        fprintf(new_file, buffer);
    }
    fclose(old_file);

    return new_file;
}

/*
read keys from ~/.config/wireman/<interface>/<interface>.<type>
Keys:   BASE64KEY
        BASE64PUB
        BASE64BASE
NOTE: caller must free return value.
*/
char *read_key(char *interface, Key type)
{
    FILE *f;
    Path *temp, *p;
    char c, *value, buffer[MAX_BUFFER];
    int i;

    temp = config_path(interface);
    if (!temp) {
        return NULL;
    }
    p = malloc(strlen(temp) + strlen(interface) + 6);
    if (!p) {
        printf("error: failed to allocate memory for path in read_key().\n");
        free(temp);
        return NULL;
    }

    // initialize path pointer for key type
    switch (type) {

        case BASE64KEY:     sprintf(p, "%s/%s.key", temp, interface);
                            break;
        case BASE64PUB:     sprintf(p, "%s/%s.pub", temp, interface);
                            break;
        case BASE64PSK:     sprintf(p, "%s/%s.psk", temp, interface);
                            break;
        default:            break;
    }
    free(temp);

    f = fopen(p, "r");
    if (!f) {
        printf("error: failed to open %s", p);
        free(p);
        return NULL;
    }
    free(p);

    for (i = 0; (c = fgetc(f)) != EOF; i++) {
        buffer[i] = c;
    }   
    buffer[i] = '\0';

    value = malloc(strlen(buffer) + 1);
    if (!value) {
        printf("error: unable to allocate memory for return value in read_key().\n");
        return NULL;
    }
    strcpy(value, buffer);

    return value;
}

/*
Make peer NULL if none.
*/
int delete_interface(Client client, char *host, char *peer)
{

    Path *p, *wg;
    int res;

    wg = malloc(strlen(ETC_WIREGUARD_CONF) + strlen(host) + 1);
    if (!wg)  {
        printf("error: failed to allocate memory for %s in delete_interface().\n", host);
        return 1;
    }
    sprintf(wg, ETC_WIREGUARD_CONF, host);

    switch (client) {

        case HOST: 
            remove(wg);
            free(wg);

            break;
        
        case PEER:
            FILE *f;
            char *pub, *buffer, *temp_buffer, *entry;
            char *key = "[Peer]";
            register int i;
            int buffer_len, pub_len, key_len, start, del;

            pub = read_key(peer, BASE64PUB);
            if (!pub) {
                return 1;
            }

            buffer = get_buffer(wg);
            if (!buffer) {
                free(pub);
                free(wg);
                return 1;
            }

            // delete peer entry in /etc/wireguard/<host>.conf.
            buffer_len = strlen(buffer);
            key_len = strlen(key);
            pub_len = strlen(pub);
            temp_buffer = buffer;
            while (*buffer) {

                // locate "[Peer]".
                for (i = 0; *buffer == key[i]; i++) {
                    buffer++;
                }

                if (i == key_len) {
                    while (*buffer != pub[0]) {
                        buffer--;
                    }
                    entry = buffer++;         // start of a peer entry.
                } else {
                    while (i-- > 0) {
                        buffer--;
                    }
                }

                // locate public key.
                for (i = 0; *buffer == pub[i]; i++) {
                    buffer++;
                }

                if (i == pub_len) {
                    for (i = 0; *buffer++ != key[0] || *buffer != EOF; i++) {
                        buffer++;
                    }

                    if (i == key_len) {
                        while (i >= 0) {
                            buffer--;
                        }
                        break;
                    } else {
                        while (i-- > 0) {
                            buffer--;
                        }
                }
                } else {
                    while (i-- > 0) {
                        buffer--;
                    }
                }

                buffer++;
            }
            free(pub);

            if (*buffer == EOF) {
                printf("info: no peer entry found in %s.conf", host);
                free(temp_buffer);
                return 0;
            }

            // move left
            start = (int) (entry - temp_buffer);
            del = (int) (buffer - entry);       // number of characters to be deleted.
            for (i = start; i < buffer_len - del; i++) {
                temp_buffer[i] = temp_buffer[i + del];
            }
            temp_buffer[buffer_len - del] = '\0';

            euid_helper(GAIN);
            f = fopen(wg, "w");
            free(wg);
            if (!f) {
                printf("error: failed to open %s.conf in delete_interface().\n", host);
                return 1;
            }
            fwrite(temp_buffer, sizeof(char), strlen(temp_buffer) + 1, f);
            free(temp_buffer);
            fclose(f);
            euid_helper(DROP);

            // recursively remove ~/.config/wireman/<interface> directory.
            p = config_path(peer);
            if (!p) {
                return 1;
            }
            res = recursive_remove(p);
            free(p);
            if (res) {
                free(pub);
                return 1;
            }

            break;

        default:
            break;

    }
   
    return 0;
}

/*
create a file ~/.conf/wireman/<key_name>.<key_type> 
containing only key.
*/
int store_key(char *key_name, char *key_type, char *key)
{
    Path *p;
    FILE *f;
    char *filename;

    // check for ~/.config/wireman/<key_name>, create if doesn't exist.
    p = config_path(key_name);
    if (!p) {
        return 1;
    } else if (!is_dir(p)) {
        mkdir(p, 0777);
    }
    free(p);

    // get absolute file path
    p = malloc(strlen(wireman) + (strlen(key_name) * 2) + 2 + strlen(key_type) + 1);
    if (!p) {
        printf("error: failed to allocate memory for %s.%s\n", key_name, key_type);
        return 1;
    }
    sprintf(p, "%s%s/%s.%s", wireman, key_name, key_name, key_type);

    // write base64
    f = fopen(p, "w");
    free(p);
    if (!f) {
        printf("error: failed to create %s.%s\n", key_name, key_type);
        return 1;
    }
    fprintf(f, key);
    fclose(f);

    return 0;
}

int tunnel_address(Config conf)
{
    int i, res;
    char c, ip[IP_LEN + 1];     // TODO: find defaults.

    printf("Input tunnel address (default 10.0.0.X/24): ");
    for (i = 0; (c = getchar()) != '\n'; i++) {
        ip[i] = c;
    }

    // add host ip to conf
    res = add_key(conf, ADDRESS, ip);
    if (res) {
        printf("error: failed to add tunnel address.\n");
        clear_config(conf);
        return res;
    }
    printf("tunnel address: %s\n", ip);

    return res;
}

int keygen(Config conf, char *interface)
{
    wg_key key, pub, psk;
    wg_key_b64_string key_base64, pub_base64, psk_base64;
    int res;

    // generate keys
    wg_generate_private_key(key);
    wg_generate_public_key(pub, key);
    wg_generate_preshared_key(psk);
    
    // private key
    wg_key_to_base64(key_base64, key);
    res = add_key(conf, KEY, key_base64);
    if (res) {
        printf("error: failed to add key.\n");
        clear_config(conf);
        return res;
    }

    // public key
    wg_key_to_base64(pub_base64, pub);
    res = add_key(conf, PUB, pub_base64);
    if (res) {
        printf("error: failed to add pub.\n");
        clear_config(conf);
        return res;
    }

    // preshared key
    wg_key_to_base64(psk_base64, psk);
    res = add_key(conf, PSK, psk_base64);
    if (res) {
        printf("error: failed to add psk.\n");
        clear_config(conf);
        return res;
    }

    store_key(interface, "key", key_base64);        // TODO: only store this on demand.
    store_key(interface, "pub", pub_base64);
    store_key(interface, "psk", psk_base64);

    printf("\nkey: %s\npub: %s\npsk: %s\n\n", key_base64, pub_base64, psk_base64);         // delete this line.

    return res;
}