#ifndef FUNCTIONS
#define FUNCTIONS
#include"../lib/proxy_parse.h"
#include <stdio.h>
#include<string.h>
#include<strings.h>
#include<pthread.h>
#include<semaphore.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX_BYTES 4000
#define MAX_CLIENTS 10
# define MAX_SIZE 10
#define MAX_ELEMENT_SIZE 10*(1<<8)
#define MAX_CACHE_SIZE (100 * 1024 * 1024)
struct element_inside_cache{    //websites
    char* data;
    int len;
    char* url;
    int number_of_usage;   //number of time that cache element is being used, more number = more priority
    struct element_inside_cache *next;
};


int port = 8080;
int server_socket_id;

pthread_t clients[MAX_CLIENTS];
pthread_mutex_t lock;
sem_t semaphore;

int cache_size;

handle_request(int client_socket, struct ParsedRequest *request, char* tempreq);
connectRemoteServer(char* host_addrs, int server_port);
int checkHTTPversion(char* version);
int ErrorMessage(int socket, int status_code);
int adding_element_in_cache(char* data, int len, char* url);
struct element_inside_cache* find(char* url);
void remove_cache_element();

#endif