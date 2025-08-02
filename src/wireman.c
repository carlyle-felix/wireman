#include <stdio.h>
#include <string.h>

#include "../incl/root.h"
#include "../incl/interface.h"
#include "../incl/manager.h"
#include "../incl/util.h"

int main(int argc, char *argv[]) 
{
    int user_id;

    user_id = euid_helper(CHECK);
    if (!user_id) {
        euid_helper(DROP);      // drop euid root before proceeding.
    }

    config_home();
    
    // read args: new server, new peer, remove server, remove peer
    if (argc == 1) {
        printf("use --help for list of operations.\n");
    } else if (!strcmp(argv[1], "--add-host")) {
        
        if (user_id != 0) {
            printf("error: operation requires elevated privilage.\n");
            return user_id;
        } 
         
        add_host(argv[2]);
    } else if (!strcmp(argv[1], "--del-host")) {

        if (user_id != 0) {
            printf("error: operation requires elevated privilage.\n");
            return user_id;
        } 

        delete_interface(HOST, argv[2], NULL);
    } else if (!strcmp(argv[1], "--host")) {
        
        if (user_id != 0) {
            printf("error: operation requires elevated privilage.\n");
            return user_id;
        } 
        
        if (!strcmp(argv[3], "--add-peer")) {
            add_peer(argv[2], argv[4]);
        } else if (!strcmp(argv[3], "--del-peer")) {
            delete_interface(PEER, argv[2], argv[4]);
        }
    }

    return 0;
}