#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "../incl/hostip.h"

#define CURL_IP "https://api.ipify.org/"

struct memory {
  char *response;
  size_t size;
};

size_t write_callback(char *p, size_t size, size_t nmemb, void *data);

char *curl_ip(void)
{
    CURL *curl = curl_easy_init();
    struct memory *mem = {0};
    char *s;

    if (curl) {
        CURLcode res;
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&mem);
        curl_easy_setopt(curl, CURLOPT_URL, CURL_IP);
        
        res = curl_easy_perform(curl);
        if (res) {
            return NULL;
        }

        s = malloc(IP_LEN + 1);
        if (!s) {
            printf("error: failed to allocate memory in curl_ip().\n");
        } else {
            strcpy(s, mem->response);
            free(mem->response);
        }

        curl_easy_cleanup(curl);
    }

    return s;
}

size_t write_callback(char *p, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *)data;
 
    p = realloc(mem->response, mem->size + realsize + 1);
    if (!p) {
        return 0;
    }

    mem->response = p;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;
 
    return realsize;
}