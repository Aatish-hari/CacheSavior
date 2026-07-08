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
#define MAX_BYTES 1024

handle_request(int client_socket, struct ParsedRequest *request, char* tempreq);
connectRemoteServer(char* host_addrs, int server_port);

#endif