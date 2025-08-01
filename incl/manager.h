#ifndef MANAGER_H
#define MANAGER_H

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
// enum device {SERVER_HOST, SERVER_PEER};

Config new_config(void);
void clear_config(Config p);
int write_config(Config conf, Client client, char *host, char *peer);
int write_peer(Config conf, char *interface);
int delete_config(Field dev, char *interface);
int add_key(Config conf, Field key, char *s);

#endif