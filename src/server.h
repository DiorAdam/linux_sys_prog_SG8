#define MAX_MESSAGE_LENGTH 128
#define PORT 8080

void init_server();

int make_sock();

void listen_sock(int sockfd);

void handle_sock(int sockfd);

void* communication_loop(void* connfdaddr);

char** parse(char* msg);


