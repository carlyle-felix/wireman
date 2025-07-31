#ifndef UTIL_H
#define UTIL_H

#define HOST_CONFIG "/etc/wireguard/%s.conf"
#define MAX_BUFFER 256

typedef char Path;

int config_home(void);
char *config_path(char *dir);
int is_dir(char *dir);
int store_key(char *key_name, char *key_type, char *key);

#endif