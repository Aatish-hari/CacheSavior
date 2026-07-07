#include"functions.h"


int handle_request(int client_socket, struct ParsedRequest *request, char* tempreq){
    char* buff = (char*)malloc(MAX_BYTES*sizeof(char));
    strcpy(buff, "GET");
    strcat(buff, request->path);
    strcat(buff, " ");
    strcat(buff, request->version);
    strcat(buff, "\r\n");

    int buff_len = strlen(buff);
    if(ParsedHeader_get(request, "Host") == NULL){
        if(ParsedHeader_set(request, "Host", request->host)<0){
            printf("Set host header key failed");
        }
    }
}