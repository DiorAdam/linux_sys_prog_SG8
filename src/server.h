#define MAX_MESSAGE_LENGTH 128
#define PORT 8080
#define MAX_CLIENTS 6

void init_server();

void* client_listener(void* sockfd);

void communication_loop(int connfd);

char** parse(char* msg);

