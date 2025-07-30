#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define CURL_IP "https://api.ipify.org/"

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data);

struct memory {
  char *response;
  size_t size;
};

char *curl_ip(void)
{
    CURL *curl = curl_easy_init();
    struct memory *mem = {0};
    char *str;

    if (curl) {
        CURLcode res;
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&mem);
        curl_easy_setopt(curl, CURLOPT_URL, CURL_IP);
        
        res = curl_easy_perform(curl);
        if (res) {
            return NULL;
        }

        str = malloc(16);
        str = strcpy(mem->response, str);
        free(mem->response);
        curl_easy_cleanup(curl);
    }

    return str;
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *)data;
 
    ptr = realloc(mem->response, mem->size + realsize + 1);
    if (!ptr) {
        return 0;
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;
 
    return realsize;
}