#ifndef UTIL_H
#define UTIL_H

#define ETC_WIREGUARD "/etc/wireguard/"
#define ETC_WIREGUARD_CONF "/etc/wireguard/%s.conf"
#define CONFIG_WIREMAN "/.config/wireman/"
#define MAX_BUFFER 512

typedef char Path;

int config_home(void);
char *config_path(char *dir);
int is_dir(char *dir);
int store_key(char *key_name, char *key_type, char *key);
FILE *file_copy(char *interface);

#endif