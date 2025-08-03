#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../incl/util.h"
#include "../incl/manager.h"

int is_dir(char *dir)
{
    char *p;
    DIR *d;
    
    p = config_wireman(dir);
    if (!p) {
        return 1;
    }

    d = opendir(p);
    free(p);
    if (!d) {
        closedir(d);
        return 0;
    }

    return 1;
}

int file_exists(char *file) 
{
    FILE *f;

    f = fopen(file, "r");
    if (!f) {
        return 0;
    } 
    
    return 1;
}

int recursive_remove(Path *dir)
{
    DIR *d;
    struct dirent *p;
    char path[MAX_BUFFER];
    int res;

    d = opendir(dir);
    if (!d) {
        if (file_exists(dir)) {
            remove(dir);
            return 0;
        } else {
            return 1;
        }
    }
    
    while ((p = readdir(d))) {
        if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
            continue;
        }

        sprintf(path, "%s/%s", dir, p->d_name);
        printf("\n");
        if (is_dir(path)) {
            recursive_remove(path);
        } else {
            remove(path);
        }
    }
    closedir(d);
    
    res = rmdir(dir);
    if (res) {
        printf("error: failed to remove dir %s in recursive_remove().\n", dir);
        return 1;
    }

    return 0;
}

/*
NOTE: caller must free returned buffer.
*/
char *get_buffer(Path *p) 
{

    FILE *f;
    char *buffer;
    int read, max = MAX_BUFFER;

    buffer = mem_alloc(MAX_BUFFER);
    for (;;) {

        buffer = realloc(buffer, max);
        if (!buffer) {
            printf("error: failed to realloc buffer in get_buffer().\n");
            return NULL;
        }

        f = fopen(p, "r");
        if (!f) {
            printf("error: failed to pipe %s in get_buffer().\n", p);
            free(buffer);
            return NULL;
        }

        read = fread(buffer, sizeof(char), max, f);
        if (read == max) {
            max *= 2;
        } else {
            fclose(f);
            break;
        }
    }
    
    buffer[read] = '\0';
    if (!buffer[0]) {
        printf("error: NULL buffer in get_buffer().\n");
        free(buffer);
        return NULL;
    }

    return buffer;
}

void *mem_alloc(int bytes)
{
    void *temp = malloc(bytes);
    if (!temp) {
        printf("error: unable to allocate memory in mem_alloc()\n");
        exit(EXIT_FAILURE);
    }

    return temp;
}

int key_count(Path *p, char *key)
{
    char *buffer, *temp_buffer;
    int i, len, count = 0;

    buffer = get_buffer(p);
    if (!buffer) {
        return 1;
    }

    len = strlen(key);
    temp_buffer = buffer;
    while (*buffer++) {
        
        for (i = 0; *buffer == key[i]; i++) {
            buffer++;
        }
        if (i == len) {
            count++;
        }
    }
    free(temp_buffer);

    return count;
}