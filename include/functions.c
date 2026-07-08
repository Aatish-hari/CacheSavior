#include"functions.h"

int connectRemoteServer(char* host_addrs, int server_port){

    int remoteSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(remoteSocket<0){
        printf("Error: socket creation failed");
        return -1;
    }

    struct hostent* host = gethostbyname(host_addrs);
    if(host == NULL){
        fprintf(stderr, "Error: no such host");
        return -1;
    }

    struct sockaddr_in server_addrs;
    bzero((char*)&server_addrs, sizeof(server_addrs));
    server_addrs.sin_family = AF_INET;
    server_addrs.sin_port = htons(server_port);
    bcopy((char*)&host->h_addr_list[0], (char*)&server_addrs.sin_addr.s_addr, host->h_length);

    if(connect(remoteSocket, (struct sockaddr*)& server_addrs, (size_t)sizeof(server_addrs))<0){
        fprintf(stderr, "Error: connecting");
        return -1;
    }
    return remoteSocket;
}


int handle_request(int client_socket, struct ParsedRequest *request, char* tempreq){
    char* buff = (char*)malloc(MAX_BYTES*sizeof(char));
    strcpy(buff, "GET");
    strcat(buff, request->path);
    strcat(buff, " ");
    strcat(buff, request->version);
    strcat(buff, "\r\n");

    int buff_len = strlen(buff);

    if(ParsedHeader_set(request, "Connection", "close")<0){
            printf("Error: Set header failed");
        }
        
            // GET http://www.example.com/index.html HTTP/1.0
            // User-Agent: TestClient

            // GET /index.html HTTP/1.1
            // Host: www.example.com
            // User-Agent: Mozilla/5.0
    if(ParsedHeader_get(request, "Host") == NULL){
        if(ParsedHeader_set(request, "Host", request->host)<0){
            printf("Set host header key failed");
        }
    }

        if(ParsedRequest_unparse_headers(request, buff+buff_len, (size_t)-buff_len)<0){
            printf("Error: unparse failed");
        }


    int server_port = 80;   //default port
    if(request->port != NULL){
        server_port = atoi(request->port);
    }
    int remoteServersocket = connectRemoteServer(request->host, server_port);
    
    
}