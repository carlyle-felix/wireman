#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "../incl/root.h"

int euid_helper(Euid_ops option) 
{    
    int res;
	struct passwd *pw;
	uid_t uid;

    switch (option) {
        
        case CHECK:
            return getuid();

        case GAIN:
            res = seteuid(0);
    	if (res) {
		        printf("error: gain_root() failed\n");
                break;
            }
            break;

        case DROP:
            pw = getpwnam(getlogin());
	        uid = pw->pw_uid;
	        res = seteuid(uid);
            if (res) {
		        printf("error: drop_root() failed\n");
                break;
	        }
            pw = getpwuid(uid);
	        setenv("HOME", pw->pw_dir, 1);
            break;

        default:
            break;
    }
    
    return res;
}