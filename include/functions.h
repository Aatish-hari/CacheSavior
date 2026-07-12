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
#include <unistd.h>
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


extern int port;
extern int server_socket_id;

extern pthread_t clients[MAX_CLIENTS];
extern pthread_mutex_t lock;
extern sem_t semaphore;

extern int cache_size;
extern struct element_inside_cache *head;

int handle_request(int client_socket, struct ParsedRequest *request, char* tempreq);
int connectRemoteServer(char* host_addrs, int server_port);
int checkHTTPversion(char* version);
int Error_message(int socket, int status_code);
int adding_element_in_cache(char* data, int len, char* url);
struct element_inside_cache* find(char* url);
void remove_cache_element();

#endif