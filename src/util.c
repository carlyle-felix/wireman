#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "../incl/util.h"

Path *home, *wireman, *wireguard;

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
    wireman = strcat(p, CONFIG_WIREMAN);       // initialize global wireman path.

    // create ~/.config/wireman if it doesn't exist.
    if (!is_dir(wireman)) {
        mkdir(wireman, 0777);
    } 

    wireguard = ETC_WIREGUARD;

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

/*
copy contents of old <host_interface>.conf into new
NOTE: caller must close returned file.
*/
FILE *file_copy(char *interface)
{
    FILE *old_file, *new_file;
    Path *old_conf, *new_conf, *temp_conf;
    char buffer[MAX_BUFFER];

    old_conf = malloc(strlen(ETC_WIREGUARD_CONF) + strlen(interface) + 1);
    if (!old_conf) {
        printf("error: failed to allocate memory for old file pointer\n");
        return NULL;
    }
    sprintf(old_conf, ETC_WIREGUARD_CONF, interface);

    temp_conf = malloc(strlen(ETC_WIREGUARD_CONF) + strlen(interface) + 4);
    if (!temp_conf) {
        printf("error: failed to allocate memory for temp file pointer\n");
        return NULL;
    }
    sprintf(temp_conf, ETC_WIREGUARD_CONF".old", interface);

    rename(old_conf, temp_conf);
    remove(old_conf); 
    free(old_conf);

    old_file = fopen(temp_conf, "r");
    if (!old_file) {
        printf("error: unable to open old %s.conf\n", interface);
        return NULL;
    }

    new_conf = malloc(strlen(ETC_WIREGUARD_CONF) + strlen(interface) + 1);
    if (!new_conf) {
        printf("error: failed to allocate memory for new file pointer\n");
        return NULL;
    }
    sprintf(new_conf, ETC_WIREGUARD_CONF, interface);

    new_file = fopen(new_conf, "w");
    if (!new_file) {
        printf("error: unable to open old %s.conf", interface);
        return NULL;
    }
    free(new_conf);

    while (fgets(buffer, MAX_BUFFER, old_file)) {
        fprintf(new_file, buffer);
    }
    fclose(old_file);

    return new_file;
}