#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "../incl/util.h"

Path *home, *wireman;

int config_home(void)
{
    Path *p;

    home = getenv("HOME");      // initialize global home path.
    
    p = malloc(strlen(home) + 18);
    if (!p) {
        printf("error: malloc failed in config_home().");
        return 1;
    }
    strcpy(p, home);
    wireman = strcat(p, "/.config/wireman/");       // initialize global wireman path.

    // create ~/.config/wireman if it doesn't exist.
    if (!is_dir(wireman)) {
        mkdir(wireman, 0777);
    } 

    return 0;
}

/*
return the absolute path of the ~/.config/wireman/<dir> directory
NOTE: free the pointer.
*/
char *config_path(char *dir) 
{
    Path *p;

    p = malloc(strlen(wireman) + strlen(dir) + 1);
    if (!p) {
        printf("error: failed to allocate memory for ");
        return NULL;
    }
    sprintf(p, "%s%s", wireman, dir);

    return p;
}

int is_dir(char *dir)
{
    Path *p;
    DIR *d;
    
    p = config_path(dir);
    if (!p) {
        printf("%s in is_dir().\n", dir);
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

/*
create a file ~/.conf/wireman/<key_name>.<key_type> 
containing only key.
*/
int store_key(char *key_name, char *key_type, char *key)
{
    Path *p;
    FILE *f;
    char *filename;
    int res;

    // check for ~/.config/wireman/<key_name>, create if doesn't exist.
    p = config_path(key_name);
    if (!p) {
        printf("%s.\n", key_name);
        return 1;
    } else if (!is_dir(p)) {
        mkdir(p, 0777);
    }
    free(p);

    // get absolute file path
    p = malloc(strlen(wireman) + (strlen(key_name) * 2) + 2 + strlen(key_type) + 1);
    if (!p) {
        printf("error: failed to allocate memory for %s.%s\n", key_name, key_type);
        return 1;
    }
    sprintf(p, "%s%s/%s.%s", wireman, key_name, key_name, key_type);

    // write base64
    f = fopen(p, "w");
    free(p);
    if (!f) {
        printf("error: failed to create %s.%s\n", key_name, key_type);
        return 1;
    }
    fprintf(f, key);
    fclose(f);

    return 0;
}