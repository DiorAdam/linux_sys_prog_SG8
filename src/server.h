#define MAX_MESSAGE_LENGTH 128
#define PORT 8080

#define CREDENTIALS_FILE "../data/credentials.txt"

#define PARSING_ERROR "Parsing Error : command is not valid\n"
#define INVALID_USERNAME "Username is not valid\n"
#define INVALID_PASSWORD "Password is not valid\n"
#define ADD_USER_ERROR "An error occurred while registering\n"
#define ADD_USER_SUCCESS "Account succesfully created\n"
#define LOGIN_ERROR "An error occurred while logging in\n"
#define LOGIN_SUCCESS "Succesfully logged in\n"
#define USER_EXISTS "Username already exists\n"
#define LOGOUT_ERROR "Not logged in\n"
#define LOGOUT_SUCCESS "Succesfully logged out\n"




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

int correctCredentials(char* username, char* password);

int userExists(char* username);

