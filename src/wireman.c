#include <stdio.h>
#include <string.h>

#include "../incl/root.h"
#include "../incl/server.h"

int main(int argc, char *argv[]) 
{
    int user_id;

    user_id = euid_helper(CHECK);
    if (!user_id) {
        euid_helper(DROP);      // drop euid root before proceeding.
    }
    
    // read args: new server, new peer, remove server, remove peer
    if (argc == 1) {
        printf("use --help for list of operations.\n");
    } else if (!strcmp(argv[1], "--add-server")) {
        /*
        if (user_id != 0) {
            printf("error: operation requires elevated privilage.\n");
            return user_id;
        } 
        */ 
        add_server(argv[2]);
    } else if (!strcmp(argv[1], "--del-server")) {
        /*
        if (user_id != 0) {
            printf("error: operation requires elevated privilage.\n");
            return user_id;
        }
        */
        del_server(argv[2]);
    }

    return 0;
}