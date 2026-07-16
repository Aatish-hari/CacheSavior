#include"functions.h"


int port = 8080;
int server_socket_id;

pthread_t clients[MAX_CLIENTS];
pthread_mutex_t lock;
sem_t semaphore;

int cache_size = 0;

struct element_inside_cache *head = NULL;


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
    bcopy((char*)host->h_addr_list[0], (char*)&server_addrs.sin_addr.s_addr, host->h_length);
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
    printf("Host : %s\n", request->host);
    printf("Port : %d\n", server_port);
    printf("Path : %s\n", request->path);
    int remoteServersocket = connectRemoteServer(request->host, server_port);

    int bytes_send = send(remoteServersocket, buff, strlen(buff), 0);       //we send HTTP request we made inside buffer
    bzero(buff, MAX_BYTES);

    int bytes_recv = recv(remoteServersocket, buff, MAX_BYTES-1, 0);
    char* temp_buff = (char*)malloc(sizeof(char)*MAX_BYTES);
    int temp_buff_size = MAX_BYTES;
    int temp_buff_index = 0;

    while(bytes_recv>0){
        bytes_send = send(client_socket, buff, bytes_recv, 0);
        for(int i = 0 ; i<bytes_send/sizeof(char) ; i++){
            temp_buff[temp_buff_index] = buff[i];
            temp_buff_index++;
        }
        temp_buff_size +=MAX_BYTES;
        temp_buff = (char*)realloc(temp_buff, temp_buff_size);
        if(bytes_recv<0){
            perror("Error: error sending data to client\n");
            break;
        }
        bzero(buff, MAX_BYTES);
        bytes_recv = recv(remoteServersocket, buff, MAX_BYTES-1, 0);
    }
    free(buff);
    temp_buff[temp_buff_index] = '\0';
    adding_element_in_cache(temp_buff, strlen(temp_buff), tempreq);
    free(temp_buff);
    close(remoteServersocket);
    return 0;
}

int checkHTTPversion(char* version){
    int ans = -1;
    if(strncmp(version, "HTTP/1.1", 8) == 0 || strncmp(version, "HTTP/1.0", 8) == 0){
        ans = 1;            //this project only for version http/1s
    }                       //HTTP/2 — binary, multiplexed, a completely different wire format
    else{
        ans = -1;
    }
    return ans;
}



int Error_message(int socket, int status_code)
{
	char str[1024];
	char currentTime[50];
	time_t now = time(0);

	struct tm data = *gmtime(&now);
	strftime(currentTime,sizeof(currentTime),"%a, %d %b %Y %H:%M:%S %Z", &data);

	switch(status_code)
	{
		case 400: snprintf(str, sizeof(str), "HTTP/1.1 400 Bad Request\r\nContent-Length: 95\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>400 Bad Request</TITLE></HEAD>\n<BODY><H1>400 Bad Rqeuest</H1>\n</BODY></HTML>", currentTime);
				  printf("400 Bad Request\n");
				  send(socket, str, strlen(str), 0);
				  break;

		case 403: snprintf(str, sizeof(str), "HTTP/1.1 403 Forbidden\r\nContent-Length: 112\r\nContent-Type: text/html\r\nConnection: keep-alive\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>403 Forbidden</TITLE></HEAD>\n<BODY><H1>403 Forbidden</H1><br>Permission Denied\n</BODY></HTML>", currentTime);
				  printf("403 Forbidden\n");
				  send(socket, str, strlen(str), 0);
				  break;

		case 404: snprintf(str, sizeof(str), "HTTP/1.1 404 Not Found\r\nContent-Length: 91\r\nContent-Type: text/html\r\nConnection: keep-alive\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>\n<BODY><H1>404 Not Found</H1>\n</BODY></HTML>", currentTime);
				  printf("404 Not Found\n");
				  send(socket, str, strlen(str), 0);
				  break;

		case 500: snprintf(str, sizeof(str), "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 115\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>500 Internal Server Error</TITLE></HEAD>\n<BODY><H1>500 Internal Server Error</H1>\n</BODY></HTML>", currentTime);
				  //printf("500 Internal Server Error\n");
				  send(socket, str, strlen(str), 0);
				  break;

		case 501: snprintf(str, sizeof(str), "HTTP/1.1 501 Not Implemented\r\nContent-Length: 103\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>404 Not Implemented</TITLE></HEAD>\n<BODY><H1>501 Not Implemented</H1>\n</BODY></HTML>", currentTime);
				  printf("501 Not Implemented\n");
				  send(socket, str, strlen(str), 0);
				  break;

		case 505: snprintf(str, sizeof(str), "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Length: 125\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>505 HTTP Version Not Supported</TITLE></HEAD>\n<BODY><H1>505 HTTP Version Not Supported</H1>\n</BODY></HTML>", currentTime);
				  printf("505 HTTP Version Not Supported\n");
				  send(socket, str, strlen(str), 0);
				  break;

		default:  return -1;

	}
	return 1;
}


struct element_inside_cache* find(char* url){
    struct element_inside_cache* temp_website = NULL;
    int temp_lock = pthread_mutex_lock(&lock);
    printf("Cahce lock acquired %d\n", temp_lock);
    if(head!=NULL){
        temp_website = head;
        while(temp_website!=NULL){
            
            if(strcmp(temp_website->url, url) == 0){
                printf("URL found in cache\n");
                printf("LRU number of usage before %d\n", temp_website->number_of_usage);
                temp_website->number_of_usage++;
                printf("LRU number of usage after %d\n", temp_website->number_of_usage);
                break;
            }
            temp_website = temp_website->next;
        }
    }
    else{
        printf("NO URL in Cache");
    }
    temp_lock = pthread_mutex_unlock(&lock);
    printf("done finding for url");
    return temp_website;
}

int adding_element_in_cache(char* data, int len, char* url){
    int temp_lock = pthread_mutex_lock(&lock);
    printf("Lock acquired by for adding element %d\n", temp_lock);
    int element_size = len + strlen(url) + sizeof(struct element_inside_cache);    
    if(element_size>MAX_ELEMENT_SIZE){
        temp_lock = pthread_mutex_unlock(&lock);
        printf("Error: Response is bigger than MAX_ELEMENT_SIZE,");
        return 0;
    }
    else{
        while(cache_size+element_size>MAX_CACHE_SIZE){
            remove_cache_element();
        }
        struct element_inside_cache *element = (struct element_inside_cache*)malloc(sizeof(struct element_inside_cache));
        element->data = (char*)malloc(len+1);
        strcpy(element->data, data);
        element->url = (char*)malloc(strlen(url)*sizeof(char)+1);
        strcpy(element->url, url);
        element->number_of_usage = 0;
        printf("number of usage of %s is %d", url, element->number_of_usage);
        element->next = head;
        head = element;
        element->len = len;
        cache_size = cache_size + element_size;
        temp_lock = pthread_mutex_unlock(&lock);
        printf("Added element into the cache\n");
        return 1;
    }
    return 0;
}

void remove_cache_element(){
   
    struct element_inside_cache *least = head;
    struct element_inside_cache *least_prev = NULL;

    struct element_inside_cache *curr = head;
    struct element_inside_cache *prev = NULL;

    // Find least frequently used node
    while (curr != NULL) {
        if (curr->number_of_usage < least->number_of_usage) {
            least = curr;                           //if 2 or more have same number of usage it will pick anyone
            least_prev = prev;                      
        }

        prev = curr;
        curr = curr->next;
    }

    // Remove node
    if (least_prev == NULL) {
        // Head is the least-used node
        head = head->next;
    } else {
        least_prev->next = least->next; //connecting with element after least used element to remove least used
    }

    // Free resources, removed least used element
    free(least->data);
    free(least->url);
    free(least);

    pthread_mutex_unlock(&lock);
    printf("removed leasted used element\n");
    return;
}
    
