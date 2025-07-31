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
typedef enum f {ADDRESS, KEY, PUB, PSK, PORT, ALLOW} Field;
// enum device {SERVER_HOST, SERVER_PEER};

Config new_config(void);
void clear_config(Config p);
int write_host(Config conf, char *interface);
int write_peer(Config conf, char *interface);
int delete_config(Field dev, char *interface);
int add_key(Config conf, Field key, char *s);

#endif