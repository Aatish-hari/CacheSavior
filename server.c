#include "lib/proxy_parse.h"
#include "include/functions.h"
#include <stdio.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>


#define MAX_CLIENTS 10
#define MAX_BYTES 1024


//its going to be a linked list of elements in cache
struct element_inside_cache{
    char* data;
    int len;
    char* url;
    int number_of_usage;   //number of time that cache element is being used, more number = more priority
    struct element_inside_cache *next;
};

struct element_inside_cache* head;  //pointer to starting of cache


int adding_element_in_cache(char* data, int len, char* url);
struct element_inside_cache* find(char* url);
void remove_cache_element();


int port = 8080;
int server_socket_id;

pthread_t clients[MAX_CLIENTS];
pthread_mutex_t lock;
sem_t semaphore;

int cache_size;

// int handle_request(int client_socket, ParsedRequest *request, char* tempreq){
//     char* buff = (char*)malloc(MAX_BYTES*sizeof(char));
//     strcpy(buff, "GET");
//     strcat(buff, request->path);
//     strcat(buff, " ");
//     strcat(buff, request->version);
//     strcat(buff, "\r\n");

//     int buff_len = strlen(buff);
//     if(ParsedHeader_get(request, "Host") == NULL){
//         if(ParsedHeader_set(request, "Host", request->host)<0){
//             printf("Set host header key failed");
//         }
//     }
// }


void* thread_func(void* new_socket){
    sem_wait(&semaphore);   //-1 semaphore value and if semaphore negative, it will wait else move on

    int socket = *(int *)new_socket;
    int bytes_send_by_client, len;

    char* buffer = (char*)calloc(MAX_BYTES, sizeof(char));
    bytes_send_by_client = recv(socket, buffer, sizeof(buffer), 0);

    while(bytes_send_by_client>0){
        len = strlen(buffer);           //len of received data
        if(strstr(buffer, "\r\n\r\n") == NULL){
            bytes_send_by_client = recv(socket, buffer+len , MAX_BYTES - len, 0);           //recv more unreceived 
        }
        else{
            break;
        }
    }

    char* tempreq = (char*) malloc(strlen(buffer)*sizeof(char));
    for(int i = 0 ; i<strlen(buffer) ; i++){
        tempreq[i] = buffer[i];
    }
    
    struct element_inside_cache* temp = find(tempreq);          //finding in cache and getting in temp

    if(temp != NULL){
        int size = temp->len/sizeof(char);
        int pos = 0;
        char response[MAX_BYTES];
        while(pos<size){
            memset(response,0, sizeof(response));
            for(int i = 0 ; i<MAX_BYTES ; i++ ){
                response[i] = temp->data[i];
                pos++;
            }
            send(socket, response, MAX_BYTES, 0);
        }
    }
    else if(bytes_send_by_client>0){        //if temp==NULL, not found in cache

        len = strlen(buffer);
        struct ParsedRequest *request = ParsedRequest_create();

        if(ParsedRequest_parse(request, buffer, len)<0){
            printf("parsing failed");
        }
        else{
            bzero(buffer, MAX_BYTES);
            if(strcmp(request->method, "GET")){
                if(request->host && request->path && checkHTTPversion(request->version) == 1){
                    bytes_send_by_client = handle_request(socket, request, tempreq);
                    if(bytes_send_by_client == -1){
                        Error_message(socket, 500);
                    }
                }
                else{
                    Error_message(socket, 500);
                }
            }
            else{
                printf("Error: Only GET method allowed");
            }
        }
        ParsedRequest_destroy(request);
    }
    else if(bytes_send_by_client==0){
        printf("client disconnected");
    }
    shutdown(socket, SHUT_RDWR);
    free(buffer);
    free(tempreq);
    sem_post(&semaphore);
    
}




int main(int argc, char* argv[]){
    int client_sock;
    int client_len;

    struct sockaddr_in server_address, client_address;
    sem_init(&semaphore, 0, MAX_CLIENTS);
    pthread_mutex_init(&lock, NULL);


    printf("starting proxy server at %d\n", port);

    server_socket_id = socket(AF_INET, SOCK_STREAM, 0);         //usual sock programming
    if(server_socket_id<0){
        perror("Error: Failed to create sockert\n");
        exit(1);
    }
    
    int reuse = 1;
    if(setsockopt(server_socket_id, SOL_SOCKET, SO_REUSEADDR, (const char)&reuse, sizeof(reuse))<0){
        perror("Error: Failed setsockopt\n");
    }

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_socket_id, (struct sockaddr*)& server_address, sizeof(server_address))<0){    //struct sockaddr* is generic pointer
        perror("Error: Failed bind\n");
        exit(1);
    }

    printf("Successful Binding on port %d\n", port);

    if(listen(server_socket_id, MAX_CLIENTS)<0){
        perror("Error: Failed listening");
        exit(1);
    }

    int i = 0; 
    int connected_clients[MAX_CLIENTS];

    while(1){
        memset(&client_address, 0, sizeof(client_address));
        int client_len = sizeof(client_address);
        int client_socket = accept(server_socket_id, (struct sockaddr*)& client_address, (socklen_t*)& client_len);

        if(client_socket<0){
            perror("Error: Faild client connection");
            exit(1);
        }else{
            connected_clients[i] = client_socket;       //storing the client socket descriptor
        }

        struct sockaddr_in *client_ptr = (struct sockaddr_in*)& client_address;     //
        struct in_addr ip_address = client_ptr->sin_addr;   //client address
        char* ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip_address, ip_str, INET_ADDRSTRLEN);   //client ip address conversion to readable formate
        printf("Server connected to client with port %d, and IP adress %s", ntohs(client_ptr->sin_port), ip_str);

        pthread_create(&clients[i], NULL, thread_func,(void*)& connected_clients[i]); //passing client fd
    }
    close(server_socket_id);
    return 0;
}