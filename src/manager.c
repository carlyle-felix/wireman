#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../incl/manager.h"
#include "../incl/root.h"
#include "../incl/util.h"
#include "../incl/wireguard.h"

#define CONFIG_WIREMAN "/.config/wireman/"
#define ETC_WIREGUARD_CONF "/etc/wireguard/%s.conf"
#define IP_LEN 15

FILE *file_copy(char *interface);
int store_key(char *key_name, char *key_type, char *key);

struct config {
    char *address;
    char *key;
    char *pub;
    char *psk;
    char *port;
    char *endpoint;
    char *allow;
};

Path *wireman;

int config_home(void)
{
    Path *home, *p;

    home = getenv("HOME");      // initialize global home path.
    
    p = mem_alloc(strlen(home) + 18);
    strcpy(p, home);
    wireman = strcat(p, CONFIG_WIREMAN);       // initialize global wireman path.

    // create ~/.config/wireman if it doesn't exist.
    if (!is_dir(wireman)) {
        mkdir(wireman, 0777);
    } 

    return 0;
}

/*
return the absolute path of the ~/.config/wireman/<dir> directory
NOTE: free the pointer.
*/
char *config_wireman(char *dir) 
{
    Path *p = mem_alloc(strlen(wireman) + strlen(dir) + 1);
    sprintf(p, "%s%s", wireman, dir);

    return p;
}

char *config_wireguard(char *interface)
{
    Path *p = mem_alloc((strlen(ETC_WIREGUARD_CONF) -2) +  strlen(interface) + 1);      // -2 for format specifier
    sprintf(p, ETC_WIREGUARD_CONF, interface);

    return p;
}

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
    char *p = mem_alloc(strlen(s) + 1);
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

/*
create a file ~/.conf/wireman/<key_name>.<key_type> 
containing only key.
*/
int store_key(char *key_name, char *key_type, char *key)
{
    Path *p, *dir;
    FILE *f;

    // check for ~/.config/wireman/<key_name>, create if doesn't exist.
    dir = config_wireman(key_name);
    if (!is_dir(dir)) {
        mkdir(dir, 0777);
    }

    // get absolute file path
    p = mem_alloc(strlen(dir) + strlen(key_name) + strlen(key_type) + 3);
    sprintf(p, "%s/%s.%s", dir, key_name, key_type);
    free(dir);

    // write base64
    f = fopen(p, "w");
    free(p);
    if (!f) {
        printf("error: failed to create %s.%s\n", key_name, key_type);
        return 1;
    }
    fprintf(f, "%s", key);
    fclose(f);

    return 0;
}

int tunnel_address(Config conf)
{
    int i, res;
    char c, ip[IP_LEN + 1];     // TODO: find defaults.

    /*
        if host interface doesn't exisit in /etc/wireguard/<interface>.conf, make default IP 10.0.0.1/24

        else,

        copy the host Address into a temp variable and find the number of peers in the config
        make X = count + 2 and check that this address doesn't exist in the config then make it default.
    */

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

/*
copy contents of old <host_interface>.conf into new
NOTE: caller must close returned file and delete <interface>.conf.old.
*/
FILE *file_copy(char *interface)
{
    FILE *old_file, *new_file;
    Path *old_conf, *new_conf, *temp_conf;
    char buffer[MAX_BUFFER];

    old_conf = config_wireguard(interface);

    // rename to <interface>.conf.olf
    temp_conf = mem_alloc(strlen(ETC_WIREGUARD_CONF) + strlen(interface) + 4);
    sprintf(temp_conf, ETC_WIREGUARD_CONF".old", interface);

    // remove <interface>.conf
    rename(old_conf, temp_conf);
    remove(old_conf); 
    free(old_conf);

    // new, empty <interface>.conf
    new_conf = config_wireguard(interface);

    old_file = fopen(temp_conf, "r");
    if (!old_file) {
        printf("error: unable to open old %s.conf\n", interface);
        return NULL;
    }

    new_file = fopen(new_conf, "w");
    free(new_conf);
    if (!new_file) {
        printf("error: unable to open old %s.conf", interface);
        return NULL;
    }

    while (fgets(buffer, MAX_BUFFER, old_file)) {
        fprintf(new_file, "%s", buffer);
    }

    fclose(old_file);

    return new_file;
}

/*
write data to configs for specified interface
Field:  HOST
        SERVER
TODO: delete /etc/wireguard/<interface>.conf.old created by file_copy().
*/
int write_config(Config conf, Client client, char *host, char *peer)
{
    FILE *f;
    Path *p, *temp;
    char *ip, *key;

    switch (client) {

        case HOST: 
            p = config_wireguard(host);
            
            euid_helper(GAIN);      // gain root
            f = fopen(p, "w");
            euid_helper(DROP);      // drop root
            if (!f) {
                printf("error: failed to open %s\n", p);
                return 1;
            }
            
            fprintf(f, "[Interface]\n");
            fprintf(f, "PrivateKey = %s\n", conf->key);
            fprintf(f, "PublicKey = %s\n", conf->pub);
            fprintf(f, "Address = %s\n", conf->address);
            fprintf(f, "ListenPort = %s\n", conf->port);
            fclose(f);
            
            break;
        
        case PEER:
            temp = config_wireman(peer);

            p = mem_alloc(strlen(temp) + strlen(peer) + 7);
            sprintf(p, "%s/%s.conf", temp, peer);
            free(temp);

            f = fopen(p, "w");
            if (!f) {
                printf("error: failed to open %s\n", p);
                return 1;
            }
            fprintf(f, "[Interface]\n");
            fprintf(f, "PrivateKey = %s\n", conf->key);
            fprintf(f, "PublicKey = %s\n", conf->pub);
            fprintf(f, "Address = %s\n", conf->address);
            fprintf(f, "\n[Peer]\n");
            
            // retrieve host public key.
            key = read_key(host, HOST, PUB);
            if (!key) {
                printf("warning: host PublicKey not found.\n");
            }
            fprintf(f, "PublicKey = %s\n", key);
            free(key);

            fprintf(f, "PresharedKey = %s\n", conf->psk);
            fprintf(f, "Endpoint = %s\n", conf->endpoint);
            fprintf(f, "AllowedIPs = %s\n", conf->allow);
            fclose(f);

            ip = conf->address;
            // change subnet mask on AllowedIP in host
            while (*ip && *ip++ != '/');
            if (!*ip) {
                printf("error: incorrect address format.\n");
                return 1;
            }
            *ip++ = '3';
            *ip++ = '2';
            *ip = '\0';     // make sure.

            // modify /etc/wireguard/<interface>.conf.
            euid_helper(GAIN);
            f = file_copy(host);
        
            fprintf(f, "\n[Peer]\n");
            fprintf(f, "PublicKey = %s\n", conf->pub);
            fprintf(f, "PresharedKey = %s\n", conf->psk);
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
Make peer NULL if none.
*/
int delete_interface(Client client, char *host, char *peer)
{
    FILE *f;
    Path *p, *wg;
    int buffer_len, pub_len, key_len, start, del, res;
    char *pub, *buffer, *temp_buffer, *entry, *key = "[Peer]";;
    register int i;

    wg = config_wireguard(host);

    switch (client) {

        case HOST: 
            if (!file_exists(wg)) {
                printf("error: interface %s not found.\n", host);
                free(wg);
                return 1;
            }

            euid_helper(GAIN);
            remove(wg);
            euid_helper(DROP);
            free(wg);

            // recursively remove ~/.config/wireman/<interface> directory.
            p = config_wireman(host);
            if (!p) {
                return 1;
            }
            
            res = recursive_remove(p);
            free(p);

            break;
        
        case PEER:
            pub = read_key(peer, PEER, PUB);
            if (!pub) {
                return 1;
            }

            if (file_exists(wg)) {
                // delete peer entry in /etc/wireguard/<host>.conf.
                buffer = get_buffer(wg);
                if (!buffer) {
                    free(pub);
                    free(wg);
                    return 1;
                }

                buffer_len = strlen(buffer);
                key_len = strlen(key);
                pub_len = strlen(pub);
                temp_buffer = buffer;
                while (*buffer++) {

                    // locate "[Peer]".
                    for (i = 0; *buffer == key[i]; i++) {
                        buffer++;
                    }

                    if (i == key_len) {
                        while (*buffer != key[0]) {
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

                        while (*buffer++) {
                            // locate next "[Peer]".
                            for (i = 0; *buffer == key[i]; i++) {
                                buffer++;
                            }

                            if (i == key_len) {
                                while (*buffer != key[0]) {
                                    buffer--;
                                }
                                // buffer is now at the beginning of next entry.
                                
                                break;
                            } else {
                                while (i-- > 0) {
                                    buffer--;
                                }
                                continue;
                            }
                        } 
                        break;

                    } else {
                        while (i-- > 0) {
                            buffer--;
                        }
                    }
                }
                free(pub);

                // update counts
                start = (int) (entry - temp_buffer);    // start of entry to be deleted.
                del = (int) (buffer - entry);       // number of characters to be deleted.
                // move left
                for (i = start; i < buffer_len - del; i++) {
                    temp_buffer[i] = temp_buffer[i + del];
                }
                temp_buffer[buffer_len - del] = '\0';

                // write the modified buffer into config file
                euid_helper(GAIN);
                f = fopen(wg, "w");
                free(wg);
                if (!f) {
                    printf("error: failed to open %s.conf in delete_interface().\n", host);
                    return 1;
                }
                fwrite(temp_buffer, sizeof(char), strlen(temp_buffer), f);
                free(temp_buffer);
                fclose(f);
                euid_helper(DROP);
            }
            
            // recursively remove ~/.config/wireman/<interface> directory.
            p = config_wireman(peer);
            if (!p) {
                return 1;
            }
            
            res = recursive_remove(p);
            free(p);
            if (res) {
                return 1;
            }
            
            break;

        default:
            break;

    }
   
    return 0;
}


/*
Read any key in the Field enum as long as it's in the config file.
Client is the target config.
reads from: HOST: /etc/wireguard/<interface>.conf.
            PEER: ~/.config/wireman/<interface>/<interface>.conf.
NOTE: caller must free returned pointer.
*/
char *read_key(char *interface, Client client, Field type) 
{
    Path *p;
    char *buffer, *temp, *key = NULL, value[MAX_BUFFER];
    int len;
    register int i;

    // initialize path
    switch (client) {

        case HOST:
            p = config_wireguard(interface);
            break;

        case PEER:
            temp = config_wireman(interface);
            if (is_dir(temp)) {
                printf("error: %s not found.\n", temp);
                free(temp);
                return NULL;
            }

            p = mem_alloc(strlen(temp) + strlen(interface) + 7);
            sprintf(p, "%s/%s.conf", temp, interface);
            free(temp);

            break;

        default:
            break;
    }


    switch (type) {

        case ADDRESS:   key = mem_alloc(strlen("address") + 1);
                        strcpy(key, "Address");
                        break;
        case KEY:       key = mem_alloc(strlen("privatekey") + 1);
                        strcpy(key, "PrivateKey");
                        break;
        case PUB:       key = mem_alloc(strlen("publickey") + 1);
                        strcpy(key, "PublicKey");
                        break;
        case PSK:       key = mem_alloc(strlen("presharedkey") + 1);
                        strcpy(key, "PresharedKey");
                        break;
        case PORT:      key = mem_alloc(strlen("listeningport") + 1);
                        strcpy(key, "ListeningPort");
                        break;
        case ALLOW:     key = mem_alloc(strlen("allowedips") + 1);
                        strcpy(key, "AllowedIPs");
                        break;
        default:        printf("error: unkown key!\n");
                        return NULL;
    }

    buffer = get_buffer(p);
    free(p);
    if (!buffer) {
        return NULL;
    }

    temp = buffer;
    len = strlen(key);
    while (buffer++) {

        for (i = 0; *buffer == key[i]; i++) {
            if (*buffer != key[i]) {
                break;
            }
            buffer++;
        }

        if (i == len) {
            buffer += 3;        // increment " = ".

            for (i = 0; *buffer != '\n'; i++) {
            value[i] = *buffer++;
            }
            value[i] = '\0';

            if (value) {
                break;
            } else {
                buffer -= (len - 1);        // go back to char after intitial trigger.
            }
        }
    }
    free(temp);

    temp = mem_alloc(strlen(value) + 1);
    strcpy(temp, value);

    return temp;
}