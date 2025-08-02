// TODO: merge with wireman.c.
#include <stdio.h>
#include <stdlib.h>

#include "../incl/interface.h"
#include "../incl/hostip.h"
#include "../incl/manager.h"
#include "../incl/wireguard.h"

#define UDP_LEN 5
#define DEFAULT_UDP "51820"

int keygen(Config conf, char *interface);
int tunnel_address(Config conf);

/*
collect hostname, key, pub and port.
collect public ip and store it somewhere for later use during peer setup.
*/
int add_host(char *host) 
{
    Config conf = new_config();
    char c, port[UDP_LEN + 1] = DEFAULT_UDP;
    int i, res;

    printf("info: creating new server \"%s\"\n", host);

    // generate and store keys
    res = keygen(conf, host);
    if (res) {
        return res;
    }
    
    // get tunnel address
    res = tunnel_address(conf);
    if (res) {
        return res;
    }

    // prompt user for port (default to 51820).
    printf("Input UDP (default %s): ", DEFAULT_UDP);    // TODO: fix default, use getchar.
    for (i = 0; (c = getchar()) != '\n'; i++) {
        port[i] = c;
    }
    res = add_key(conf, PORT, port);
    if (res) {
        printf("error: failed to add port.\n");
        clear_config(conf);
        return res;
    }

    printf("listening port: %s\n", port);

    write_config(conf, HOST, host, NULL);
    clear_config(conf);
    return 0;
}

/*
*/
int add_peer(char *host, char *peer) 
{
    Config conf = new_config();
    char c, *endpoint, allow[MAX_BUFFER] = "0.0.0.0/0, ::/0";   // TODO: read host <interface>.conf for number of peers and set accordingly.
    int i, res;

    // generate and store keys
    res = keygen(conf, peer);
    if (res) {
        return res;
    }

    // get tunnel address
    res = tunnel_address(conf);
    if (res) {
        return res;
    }

    endpoint = host_ip();
    res = add_key(conf, ENDPOINT, endpoint);
    if (res) {
        printf("error: failed to add endpoint.\n");
        clear_config(conf);
        return res;
    }
    free(endpoint);

    // set allowedIPs for peer.
    printf("Input allowed IPs for peer (default: 0.0.0.0/0, ::/0): ");
    for (i = 0; (c = getchar()) != '\n'; i++) {
        allow[i] = c;
    }

    res = add_key(conf, ALLOW, allow);
    if (res) {
        printf("error: failed to add AllowedIP (peer).\n");
        return res;
    }

    write_config(conf, PEER, host, peer);
    clear_config(conf);
    return 0;
}

int tunnel_address(Config conf)
{
    int i, res;
    char c, ip[IP_LEN + 1];     // TODO: find defaults.

    printf("Input tunnel address (default 10.0.0.X/24): ");
    for (i = 0; (c = getchar()) != '\n'; i++) {
        ip[i] = c;
    }

    // add host ip to conf
    res = add_key(conf, ADDRESS, ip);
    if (res) {
        printf("error: failed to add tunnel address.\n");
        clear_config(conf);
        return res;
    }
    printf("tunnel address: %s\n", ip);

    return res;
}

int keygen(Config conf, char *interface)
{
    wg_key key, pub, psk;
    wg_key_b64_string key_base64, pub_base64, psk_base64;
    int res;

    // generate keys
    wg_generate_private_key(key);
    wg_generate_public_key(pub, key);
    wg_generate_preshared_key(psk);
    
    // private key
    wg_key_to_base64(key_base64, key);
    res = add_key(conf, KEY, key_base64);
    if (res) {
        printf("error: failed to add key.\n");
        clear_config(conf);
        return res;
    }

    // public key
    wg_key_to_base64(pub_base64, pub);
    res = add_key(conf, PUB, pub_base64);
    if (res) {
        printf("error: failed to add pub.\n");
        clear_config(conf);
        return res;
    }

    // preshared key
    wg_key_to_base64(psk_base64, psk);
    res = add_key(conf, PSK, psk_base64);
    if (res) {
        printf("error: failed to add psk.\n");
        clear_config(conf);
        return res;
    }

    store_key(interface, "key", key_base64);        // TODO: only store this on demand.
    store_key(interface, "pub", pub_base64);
    store_key(interface, "psk", psk_base64);

    printf("\nkey: %s\npub: %s\npsk: %s\n\n", key_base64, pub_base64, psk_base64);         // delete this line.

    return res;
}