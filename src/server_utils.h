

#define CREDENTIALS_FILE "data/credentials.txt"
#define CHAT_DIR "data/groupchats"
#define GUESTS_FILE "data/guests.txt"

#define PARSING_ERROR "Parsing Error : command is not valid\n"
#define INVALID_USERNAME "Username is not valid\n"
#define INVALID_PASSWORD "Password is not valid\n"
#define ADD_USER_ERROR "An error occurred while registering\n"
#define ADD_USER_SUCCESS "Account succesfully created\n"
#define LOGIN_ERROR "An error occurred while logging in\n"
#define LOGIN_SUCCESS "Succesfully logged in\n"
#define USER_EXISTS "Username already exists\n"
#define USER_DOESNT_EXIST "Username doesn't exist\n"
#define LOGOUT_SUCCESS "Succesfully logged out\n"
#define LOGIN_REQUIRED "Login required for this action\n"
#define LOGOUT_REQUIRED "Must logout first\n"
#define CHAT_CREATION_ERROR "An error occurred while creating this chat\n"
#define CHAT_CREATION_SUCCESS "Chat succesfully created\n"
#define CHAT_EXISTS "Chat already exists\n"
#define CHAT_DOESNT_EXIST "Chat doesn't exist\n"
#define SEND_ERROR "An error occurred while sending message\n"
#define SEND_SUCCESS "Message succesfully sent\n"
#define BAD_PERMISSION "You don't have the right permissions for this action\n"
#define ADD_CHAT_MEMBER_ERROR "An error occurred while adding user in chat\n"
#define ADD_CHAT_MEMBER_SUCCESS "User succesfully added to chat group\n"
#define READ_CHAT_ERROR "An error occured while reading chat\n"
#define TOO_LONG "Cannot read over 5 messages\n"
#define LOGIN_GUEST_ERROR "An error occurred while signing you in as a guest\n"
#define LOGOUT_ERROR "An error occurred while signing you out\n"
#define CHATS_ERROR "An error occurred while determining your chat memberships\n"

typedef struct {
    char username[32];
} user;


char* parse_exec(user* u, char* msg);

int is_valid_username(char* un);

int is_valid_password(char* password);

int add_user(char* un, char* pwd);

int correct_credentials(char* username, char* password);

int user_exists(char* username);

int chat_exists(char* chat_name);

int create_chat(char* chat_name, user* u);

int send_chat_msg(char* chat_name, char* msg, user* u);

int add_chat_member(char* chat_name, char* new_member, user* u);

int is_chat_member(char* chat_name, char* member_name);

char* read_chat(char* chat_name, int num_msgs);

void generate_guest_username(user* u);

int login_guest(char* guest_name);

int is_guest(char* name);

int del_user(char* username, char* filepath);

int del_guest(char* guest_name);

int clear_guests();

char* chats_of(char* username);

