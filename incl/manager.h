#ifndef MANAGER_H
#define MANAGER_H

/*
Conf keys:  host (ipv4)
            peer (ipv4)
            key
            pub
            psk
            port
            allow (ip)
*/
typedef struct config *Config;
typedef enum f {HOST, PEER, KEY, PUB, PSK, PORT, ALLOW} Field;
// enum device {SERVER_HOST, SERVER_PEER};

Config new_config(void);
void clear_config(Config p);
int write_config(Config conf, Field dev, char *interface);
int delete_config(Field dev, char *interface);
int add_key(Config conf, Field key, char *s);

#endif