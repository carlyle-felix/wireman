#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "../incl/hostip.h"

#define IP_LEN 15
#define CURL_IP "https://api.ipify.org/"

typedef struct {
  char *response;
  size_t size;
} Memory;

char *curl(Memory *buffer);
size_t write_callback(char *data, size_t size, size_t nmemb, Memory *p);

char *host_ip(void)
{
    Memory *buffer;
    char *ip = NULL;

    buffer = malloc(sizeof(Memory));
    if (!buffer) {
        printf("error: failed to allocate memory for public ip buffer.\n");
        return NULL;
    }
    
    buffer->response = malloc(sizeof(char));
    if (!buffer->response) {
        printf("error: failed to allocate memory for response buffer.\n");
        return NULL;
    }
    buffer->size = 0;
    
    curl(buffer);
    
    ip = malloc(strlen(buffer->response) + 1);
    if (!ip) {
        printf("error: failed to allocate memory for public IP return variable.\n");
        return NULL;
    }
    strcpy(ip, buffer->response);

    free(buffer->response);
    free(buffer);

    printf("\ninfo: retrieved public IP: %s\n\n", ip);
    
    return ip;
}
/*
returns host public ip retrieved from api.ipify.org.
NOTE: caller must free returned pointer.
*/
char *curl(Memory *buffer)
{
    CURL *curl;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        CURLcode res;
        
        curl_easy_setopt(curl, CURLOPT_URL, CURL_IP);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
        
        res = curl_easy_perform(curl);
        if (res) {
            printf("error: failed to retrieve public IP.\n");
            return NULL;
        }

        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }
    return buffer->response;
}

size_t write_callback(char *data, size_t size, size_t nmemb, Memory *buffer)
{
    size_t realsize = size * nmemb;
    Memory *mem = buffer;
 
    mem->response = realloc(mem->response, mem->size + realsize + 1);
    if (!mem->response) {
        return 0;
    }

    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = '\0';
 
    return realsize;
}