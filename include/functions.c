#include"functions.h"

int connectRemoteServer(char* host_addrs, int server_port){

    int remoteSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(remoteSocket<0){
        printf("Error: socket creation failed");
        return -1;
    }

    struct hostent* host = gethostbyname(host_addrs);//gets IP
    if(host == NULL){
        fprintf(stderr, "Error: no such host");
        return -1;
    }

    struct sockaddr_in server_addrs;
    bzero((char*)&server_addrs, sizeof(server_addrs));
    server_addrs.sin_family = AF_INET;
    server_addrs.sin_port = htons(server_port);
    bcopy((char*)&host->h_addr_list[0], (char*)&server_addrs.sin_addr.s_addr, host->h_length);
    // server_addrs:-
    // IP:   93.184.216.34
    // Port: 80
    // Family: IPv4

    if(connect(remoteSocket, (struct sockaddr*)& server_addrs, (size_t)sizeof(server_addrs))<0){
        fprintf(stderr, "Error: connecting");
        return -1;
    }
    return remoteSocket;
}


int handle_request(int client_socket, struct ParsedRequest *request, char* tempreq){

    //creating http request, which is going to hit web server by proxy server
    char* buff = (char*)malloc(MAX_BYTES*sizeof(char));
    strcpy(buff, "GET ");
    strcat(buff, request->path);
    strcat(buff, " ");
    strcat(buff, request->version);
    strcat(buff, "\r\n");

    int buff_len = strlen(buff);

    if(ParsedHeader_set(request, "Connection", "close")<0){
            printf("Error: Set header failed");
        }
           
    if(ParsedHeader_get(request, "Host") == NULL){
        if(ParsedHeader_set(request, "Host", request->host)<0){     //if browser doesnt send a Host, we just add Host -> "request->host", 
            printf("Set host header key failed");                  // inside the array of headers in request with other Host: www.example.com
            }                                                                                                        // User-Agent: Chrome
        }                                                                                                        //     Connection: close   
        
    

        if(ParsedRequest_unparse_headers(request, buff+buff_len, (size_t)MAX_BYTES-buff_len)<0){    
// Before:
// buff:-
// GET /index.html HTTP/1.1\r\n

// After:

// buff:-
// GET /index.html HTTP/1.1\r\n
// Host: www.example.com\r\n
// User-Agent: Chrome\r\n
// Connection: close\r\n
// \r\n
            printf("Error: unparse failed");
        }


    int server_port = 80;   //default port
    if(request->port != NULL){
        server_port = atoi(request->port);
    }
    int remoteServersocket = connectRemoteServer(request->host, server_port);

    int bytes_send = send(remoteServersocket, buff, strlen(buff), 0);       //we send HTTP request we made inside buffer
    bzero(buff, MAX_BYTES);

    int bytes_recv = recv(remoteServersocket, buff, MAX_BYTES-1, 0);
    char* temp_buff = (char*)malloc(sizeof(char)*MAX_BYTES);
    int temp_buff_size = MAX_BYTES;
    int temp_buff_index = 0;

    while(bytes_recv>0){
        bytes_recv = send(client_socket, buff, bytes_recv, 0);
        for(int i = 0 ; i<bytes_recv/sizeof(char) ; i++){
            temp_buff[temp_buff_index] = buff[i];
            temp_buff_index++;
        }
        temp_buff_size +=MAX_BYTES;
        temp_buff = (char*)realloc(temp_buff, temp_buff_size);
    }
    
    
}