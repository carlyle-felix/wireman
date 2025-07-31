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
    strcpy(p, wireman);
    strcat(p, dir);

    return p;
}

int is_dir(char *dir)
{
    Path *p;
    DIR *d;
    
    p = config_path(dir);
    if (!p) {
        printf("%s directory pointer.\n", dir);
        return 1;
    }

    d = opendir(p);
    free(p);
    if (d) {
        closedir(d);
        return 1;       // true.
    }

    return 0;
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

    // cd into ~/.conf/wireman/.
    p = config_path(wireman);
    if (!p) {
        printf("%s directory pointer.\n", wireman);
        return 1;
    }
    res = chdir(p);
    printf("no prob\n");
    if (!res) {
        printf("error: failed to open dir %s\n", wireman);
        return res;
    }
    free(p);
    
    p = config_path(key_name);
    if (!p) {
        printf("%s.\n", key_name);
        free(p);
        return 1;
    } else if (is_dir(p)) {
        mkdir(p, 755);
    }

    res = chdir(p);
    if (!res) {
        printf("error: failed to open dir %s\n", key_name);
        return res;
    }
    free(p);


    p = malloc(strlen(wireman) + strlen(key_name) + 1 + strlen(key_type) + 1);
    if (!p) {
        printf("error: failed to allocate memory for %s.%s", key_name, key_type);
        return 1;
    }
    strcpy(p, wireman);
    strcat(p, key_name);
    strcat(p, ".");
    strcat(p, key_type);
    f = fopen(p, "w");
    if (!f) {
        printf("error: failed to create %s%s", key_name, key_type);
        return 1;
    }
    fprintf(f, key, NULL);
    fclose(f);
    free(p);

    chdir(home);        // TODO: change to pwd.

    return 0;
}