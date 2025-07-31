// TODO: merge with wireman.c.
#include <stdio.h>
#include <stdlib.h>

#include "../incl/server.h"
#include "../incl/hostip.h"
#include "../incl/manager.h"
#include "../incl/wireguard.h"
#include "../incl/util.h"

#define UDP_LEN 5
#define DEFAULT_UDP "51820"
#define DEFAULT_ADDRESS "10.0.0.1/24"

/*
collect hostname, key, pub and port.
collect public ip and store it somewhere for later use during peer setup.
*/
int add_host(char *interface) 
{
    Config conf = new_config();
    char c, port[UDP_LEN + 1] = DEFAULT_UDP, ip[IP_LEN + 1] = DEFAULT_ADDRESS;
    int i, res;
    wg_key key, pub;
    wg_key_b64_string key_base64, pub_base64;

    printf("info: creating new server \"%s\"\n", interface);

    // generate keys
    wg_generate_private_key(key);
    wg_generate_public_key(pub, key);
    
    // host privatekey
    wg_key_to_base64(key_base64, key);
    res = add_key(conf, KEY, key_base64);
    if (res) {
        clear_config(conf);
        return res;
    }

    // host publickey
    wg_key_to_base64(pub_base64, pub);
    res = add_key(conf, PUB, pub_base64);
    if (res) {
        clear_config(conf);
        return res;
    }

    // add host ip to conf
    printf("Input host address (default %s): ", DEFAULT_ADDRESS);
    for (i = 0; (c = getchar()) != '\n'; i++) {
        ip[i] = c;
    }
    res = add_key(conf, ADDRESS, ip);
    if (res) {
        clear_config(conf);
        return res;
    }

    // prompt user for port (default to 51820).
    printf("Input UDP (default %s): ", DEFAULT_UDP);    // TODO: fix default, use getchar.
    for (i = 0; (c = getchar()) != '\n'; i++) {
        port[i] = c;
    }
    res = add_key(conf, PORT, port);
    if (res) {
        clear_config(conf);
        return res;
    }

    printf("\nprivate key: %s\npub: %s\n", key_base64, pub_base64);         // delete this line.
    printf("address: %s\n", ip);
    printf("listening port: %s\n", port);

    store_key(interface, "key", key_base64);
    store_key(interface, "pub", pub_base64);

    write_host(conf, interface);
    clear_config(conf);
    return 0;
}

int del_host(char *interface) 
{
    printf("info: removing server \"%s\"\n", interface);

    // delete host
    return 0;
}

int add_peer(char *peer) 
{
    printf("info: adding peer \"%s\"\n", peer);

    // collect and add data to conf
    return 0;
}

int del_peer(char *peer) 
{
    printf("info: removing peer \"%s\"\n", peer);

    // delete peer
    return 0;
}