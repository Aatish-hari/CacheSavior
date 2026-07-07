#ifndef FUNCTIONS
#define FUNCTIONS
#include"../lib/proxy_parse.h"
#include <stdio.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>
#define MAX_BYTES 1024

handle_request(int client_socket, struct ParsedRequest *request, char* tempreq);


#endif