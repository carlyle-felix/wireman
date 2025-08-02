#ifndef UTIL_H
#define UTIL_H

typedef char Path;

int config_home(void);
char *config_path(char *dir);
int is_dir(char *dir);
int file_exists(char *file);
int recursive_remove(Path *dir);
char *get_buffer(Path *p);

#endif