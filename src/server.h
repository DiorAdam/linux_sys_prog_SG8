#define MAX_MESSAGE_LENGTH 128
#define PORT 8080

#define CREDENTIALS_FILE "../data/credentials.txt"

#define PARSING_ERROR "Parsing Error\n"



typedef struct {
    char username[32];
} user;

void init_server();

int make_sock();

void listen_sock(int sockfd);

void handle_sock(int sockfd);

void* communication_loop(void* connfdaddr);

char* parse_exec(user*, char* msg);

int isValidUsername(char* un);

int isValidPassword(char* password);

int addUser(char* un, char* pwd);


