#ifndef MANAGER_H
#define MANAGER_H

#define MAX_BUFFER 512

/*
Conf keys:  address (ipv4)
            key
            pub
            psk
            port
            allow (ip)
*/
typedef struct config *Config;
typedef char Path;

typedef enum f {ADDRESS, KEY, PUB, PSK, PORT, ENDPOINT, ALLOW} Field;
typedef enum {HOST, PEER} Client;

int config_home(void);
char *config_path(char *dir);
Config new_config(void);
void clear_config(Config p);
int write_config(Config conf, Client client, char *host, char *peer);
int add_key(Config conf, Field key, char *s);
char *read_key(char *interface, Client client, Field type);
int delete_interface(Client client, char *host, char *peer);
int keygen(Config conf, char *interface);
int tunnel_address(Config conf);


#endif