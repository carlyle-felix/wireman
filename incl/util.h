#ifndef UTIL_H
#define UTIL_H

typedef char Path;


int is_dir(char *dir);
int file_exists(char *file);
int recursive_remove(Path *dir);
char *get_buffer(Path *p);
void *mem_alloc(int bytes);

#endif