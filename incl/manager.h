#ifndef MANAGER_H
#define MANAGER_H

#define MAX_BUFFER 512
#define CONFIG_WIREMAN "/.config/wireman/"

/*
Conf keys:  address (ipv4)
            key
            pub
            psk
            port
            allow (ip)
*/
typedef struct config *Config;
typedef enum f {ADDRESS, KEY, PUB, PSK, PORT, ENDPOINT, ALLOW} Field;
typedef enum {HOST, PEER} Client;
typedef enum {BASE64KEY, BASE64PUB, BASE64PSK} Key;
typedef char Path;

Config new_config(void);
void clear_config(Config p);
int write_config(Config conf, Client client, char *host, char *peer);
int add_key(Config conf, Field key, char *s);
int tunnel_address(Config conf);
int store_key(char *key_name, char *key_type, char *key);
FILE *file_copy(char *interface);
char *read_key(char *interface, Key type);
int keygen(Config conf, char *interface);

#endif