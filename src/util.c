#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "../incl/util.h"
#include "../incl/manager.h"

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
    wireman = strcat(p, CONFIG_WIREMAN);       // initialize global wireman path.

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
    char *p;
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
