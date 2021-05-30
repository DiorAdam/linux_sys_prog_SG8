#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>

#include "server_utils.h"
#include "server.h"

pthread_mutex_t credentials_lock;
pthread_mutex_t chats_lock;
pthread_mutex_t memberships_lock;
pthread_mutex_t guests_lock;


/* 
This function 
    - parses the command from the client
    - exexcutes the command
    - and finally returns a string containing the output
*/
char* parse_exec(user* u, char* msg_ptr){
	char msg[MAX_MESSAGE_LENGTH];
	strcpy(msg, msg_ptr); 
	char* curr_tkn = strtok(msg, " \n");

	char* ans = (char*) malloc(MAX_MESSAGE_LENGTH);
	if (curr_tkn == NULL){
		strcpy(ans, PARSING_ERROR);
		return ans;
	}
    if (strcmp(curr_tkn, "EXIT") == 0) {
		curr_tkn = strtok(NULL, curr_tkn);
		if ( curr_tkn == NULL)
			strcpy(ans, "EXIT\n");
		else 
			strcpy(ans, PARSING_ERROR);
		return ans;
    }

	else if (strcmp(curr_tkn, "SIGNUP") == 0){
		char* un = strtok(NULL, " \n"); 
		if (un == NULL) {
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
		char* pwd = strtok(NULL, " \n");
		if (pwd == NULL) {
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
		curr_tkn = strtok(NULL, curr_tkn);
		if ( curr_tkn != NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}

        if (strlen(u->username)>0){
            strcpy(ans,LOGOUT_REQUIRED);
            return ans;
        }

		int res = add_user(un, pwd);
		if (res == 0){
			strcpy(ans, ADD_USER_SUCCESS);
		}
		else if (res == -1){
			strcpy(ans, ADD_USER_ERROR);
		}
		else if (res == -2){
			strcpy(ans, INVALID_PASSWORD);
		}
		else if (res == -3){
			strcpy(ans, INVALID_USERNAME);
		}
		else{
			strcpy(ans, USER_EXISTS);
		}
		
		return ans;
	}
	
	else if (strcmp(curr_tkn, "LOGIN") == 0){
		char* un = strtok(NULL, " \n"); 
		if (un == NULL) {
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
		char* pwd = strtok(NULL, " \n");
		if (pwd == NULL) {
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
		curr_tkn = strtok(NULL, curr_tkn);
		if ( curr_tkn != NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
        if (strlen(u->username)>0){
            strcpy(ans,LOGOUT_REQUIRED);
            return ans;
        }        

		int res = correct_credentials(un, pwd);
		if (res == 0){
			strcpy(u->username, un);
			strcpy(ans, LOGIN_SUCCESS);
		}
		else if (res == -1) strcpy(ans, LOGIN_ERROR);
		else if (res == -2) strcpy(ans, INVALID_PASSWORD);
		else strcpy(ans, INVALID_USERNAME);
		return ans;
	}

    else if (strcmp(curr_tkn, "GUEST") == 0){
        curr_tkn = strtok(NULL, " \n");
		if (curr_tkn != NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}

        if (strlen(u->username) > 0){
            strcpy(ans,LOGOUT_REQUIRED);
            return ans;
        }

        generate_guest_username(u);
        if (login_guest(u->username) == 0){
            sprintf(ans, "You have been registered as %s\n", u->username);
        }
        else{
            strcpy(ans, LOGIN_GUEST_ERROR);
        }
        return ans;
    }

	else if (strcmp(curr_tkn, "LOGOUT") == 0){
		curr_tkn = strtok(NULL, " \n");
		if (curr_tkn != NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}

		if (strlen(u->username) == 0){
			strcpy(ans, LOGIN_REQUIRED);
			return ans;
		}
        if (is_guest(u->username)){
            printf("yolo_guest\n");
            if (del_guest(u->username) < 0){
                strcpy(ans, LOGOUT_ERROR);
                return ans;
            }
        }
		memset(u->username, '\0', sizeof(u->username));
		strcpy(ans, LOGOUT_SUCCESS);
		return ans;
	}

	else if (strcmp(curr_tkn, "CREATE") == 0){
		char* chat_name = strtok(NULL, " \n");
		if (chat_name == NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
		curr_tkn = strtok(NULL, " \n");
		if (curr_tkn != NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
		if (strlen(u->username) == 0){
			strcpy(ans, LOGIN_REQUIRED);
			return ans;
		}
        if (is_guest(u->username)){
            strcpy(ans, BAD_PERMISSION);
            return ans;
        }
		
		int res = create_chat(chat_name, u);
		if (res == 0) strcpy(ans, CHAT_CREATION_SUCCESS);
		else if (res == -1) strcpy(ans, CHAT_CREATION_ERROR);
		else strcpy(ans, CHAT_EXISTS);
		return ans;
	}

    else if (strcmp(curr_tkn, "CHATS") == 0){
        curr_tkn = strtok(NULL, " \n");
		if (curr_tkn != NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
        if (strlen(u->username) == 0){
			strcpy(ans, LOGIN_REQUIRED);
			return ans;
		}
        char* my_chats = chats_of(u->username);
        if (my_chats == 0){
            strcpy(ans, CHATS_ERROR);
            return ans;
        }
        free(ans);
        return my_chats;
    }

	else if (strcmp(curr_tkn, "SEND") == 0){
		char* chat_name = strtok(NULL, " \n");
		if (chat_name == NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
		char* chat_msg = strtok(NULL, "");
		if (chat_msg == NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
		if (strlen(u->username) == 0){
			strcpy(ans, LOGIN_REQUIRED);
			return ans;
		}

		int res = send_chat_msg(chat_name, chat_msg, u);
		if (res == 0) strcpy(ans, SEND_SUCCESS);
		else if (res == -1) strcpy(ans, SEND_ERROR);
		else strcpy(ans, BAD_PERMISSION);
		return ans;
	}

	else if (strcmp(curr_tkn, "ADD") == 0){
		char* chat_name = strtok(NULL, " \n");
		if (chat_name == NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
		char* new_member = strtok(NULL, " \n");
		if (new_member == NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
		curr_tkn = strtok(NULL, " \n");
		if (curr_tkn != NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
		if (strlen(u->username) == 0){
			strcpy(ans, LOGIN_REQUIRED);
			return ans;
		}

		int res = add_chat_member(chat_name, new_member, u);
		if (res == 0) strcpy(ans, ADD_CHAT_MEMBER_SUCCESS);
		else if (res == -1) strcpy(ans, ADD_CHAT_MEMBER_ERROR);
		else if (res == -2) strcpy(ans, CHAT_DOESNT_EXIST);
		else if (res == -3) strcpy(ans, USER_DOESNT_EXIST);
		else strcpy(ans, BAD_PERMISSION);
		return ans;
	}
	else if (strcmp(curr_tkn, "READ") == 0){
		char* chat_name = strtok(NULL, " \n");
		if (chat_name == NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
		char* str_num_msgs = strtok(NULL, " \n");
		int num_msgs;
		if ( (str_num_msgs == NULL) || (num_msgs = atoi(str_num_msgs)) == 0){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
		if (num_msgs > 5){
			strcpy(ans, TOO_LONG);
			return ans;
		}
		curr_tkn = strtok(NULL, " \n");
		if (curr_tkn != NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}
		if (strlen(u->username) == 0){
			strcpy(ans, LOGIN_REQUIRED);
			return ans;
		}

		if (is_chat_member(chat_name, u->username) != 0){
			strcpy(ans, BAD_PERMISSION);
			return ans;
		}
		char* chat_msgs = read_chat(chat_name, num_msgs);
		if (chat_msgs == 0){
			strcpy(ans, READ_CHAT_ERROR);
			return ans;
		}
		free(ans);
		return chat_msgs;
	}
    strcpy(ans, PARSING_ERROR);
    return ans;
}


/**********************************************************************************************************************/

        /* These functions are used by parse_exec() to execute the commands from the client / 


/**********************************************************************************************************************/

/*
this takes a char* username as an input and returns
- 0 if the username is not valid
- 1 if it is valid
*/
int is_valid_username(char* username){
	int length = 0; 
	char c;
	while ( (c = *username) != '\0' ) {
		if ( c == ' ' || c == '\n' || c == '\t' || c >= 128 )
			return 0;
		username++;
		length++;
	}
    if (strncmp(username, "guest_", 6) == 0) return 0;
	return length < 30 && length > 1;
}

int is_valid_password(char* password) {
	int length = 0; 
	char c;
	while ( (c = *password) != '\0' ) {
		if ( c == ' ' || c == '\n' || c == '\t' || c >= 128 )
			return 0;
		password++;
		length++;
	}
	return 1;
}

/*
this function takes a char* username as an input and returns :
	- 0 if the user exists
	- -1 if encountered a system error
	- -2 if the user doesn't exist
*/
int user_exists(char* username) {
    if (is_guest(username)) return 0;

	pthread_mutex_lock(&credentials_lock);
	FILE* f = fopen(CREDENTIALS_FILE, "r");
	if ( f == NULL ){
		pthread_mutex_unlock(&credentials_lock);
		return -1;
	}
	
	char line[128]; char* t;
	while ( (t = fgets(line, 128, f)) != NULL ) {
		char* ptr = strtok(line, " ");
		if ( strcmp(ptr, username) == 0 ) {
			fclose(f);
			pthread_mutex_unlock(&credentials_lock);
			return 0;
		}
	}
	
	fclose(f);
	pthread_mutex_unlock(&credentials_lock);
	return -2;
}


/*
This function reads the file credentials.txt and looks for 
a matching couple (username, password)
It returns 
- 0 if it found a matching couple
- -1 in case of a system error 
- -2 if the password is wrong
- -3 if the user doesn't exist
*/
int correct_credentials(char* username, char* password) {
	pthread_mutex_lock(&credentials_lock);
	FILE* f = fopen(CREDENTIALS_FILE, "r");
	if ( f == NULL ){
		pthread_mutex_unlock(&credentials_lock);
		return -1;
	}
	
	char line[128]; char* t;
	while ( (t = fgets(line, 128, f)) != NULL ) {
		char* ptr = strtok(line, "\n");
		char* user = strtok(ptr, " ");
		char* pass = strtok(NULL, " ");
		if ( strcmp(user, username) == 0 ) {
			fclose(f);
			pthread_mutex_unlock(&credentials_lock);
			if ( strcmp(pass, password) == 0 )
				return 0; //valid credentials
				
				return -2; //user exists but password is wrong
		}
	}
	
	fclose(f);
	pthread_mutex_unlock(&credentials_lock);
	return -3; //user doesn't exist
}

/*
This function adds a user in the file credentials.txt
it returns 
- 0 in case of success
- -1 in case of system error
- -2 if the password is not valid
- -3 if the username is not valid
- -4 if the user already exists
*/
int add_user(char* username, char* password) {
	if ( !is_valid_password(password) ) return -2;
	if (!is_valid_username(username)) return -3;
	if ( user_exists(username) == 0 ) return -4;
	pthread_mutex_lock(&credentials_lock);
	FILE* f = fopen(CREDENTIALS_FILE, "a");
	if ( f == NULL ){
		pthread_mutex_unlock(&credentials_lock);
		return -1;
	}
	fprintf(f, "%s %s\n", username, password);
	fclose(f);
	pthread_mutex_unlock(&credentials_lock);
	return 0;
}

/*
this function generates a random guest username 
for the user given in argument
*/
void generate_guest_username(user* u) {
    strcpy(u->username, "guest_");
	const char charset[] = "abcdefghijklmnopqrstuvwxyz";
    srand(time(0));
    for (int i = 0; i < 10; i++) {
        int key = rand() % ((int) (strlen(charset)));
        u->username[6+i] = charset[key];
    }
    u->username[6+10] = '\0';
}


/*
This function logs in a guest by adding them in the guest file
*/
int login_guest(char* guest_name){
    pthread_mutex_lock(&guests_lock);
    FILE* f = fopen(GUESTS_FILE, "a");
	if ( f == NULL ){
		pthread_mutex_unlock(&guests_lock);
		return -1;
	}
	fprintf(f, "%s\n", guest_name);
	fclose(f);
    pthread_mutex_unlock(&guests_lock);
    return 0;
}

/*
This function determines if a user is a guest
it returns 
- 1 if it is
- 0 if it isn't
*/
int is_guest(char* name){
    return (strncmp(name, "guest_", 6) == 0);
}

/* The caller must hold the lock of 
   filepath before calling del_user */

/*
This function deletes a user from the file at filepath
it returns 
- 0 in case of success (or if the user doesn't exist)
- -1 in case of error
*/
int del_user(char* username, char* filepath){
    FILE* f = fopen(filepath, "r");
    if (f == NULL) return -1;
    char str_file[640] = ""; 
    char line[64] = "";
    while (fgets(line, 64,f)){
        if (strncmp(username, line, strlen(username)) != 0){
            strcat(str_file, line);
        }
    }
    fclose(f);
    f = fopen(filepath, "w");
    if (f == NULL) return -1;
    fprintf(f, "%s", str_file);
    fclose(f);
    return 0;
}

/*
This function deletes a guest from the guest file 
and his membership from all group_chats he was a member of
It is called everytime a guest logs out since guests don't have an account
It returns
- 0 in case of success
- -1 otherwise
*/
int del_guest(char* guest_name){
    pthread_mutex_lock(&guests_lock);
    if (del_user(guest_name, GUESTS_FILE)<0){
        pthread_mutex_unlock(&guests_lock);
        return -1;
    }
    pthread_mutex_unlock(&guests_lock);

    DIR *d;
	struct dirent *dir;
	d = opendir(CHAT_DIR);
	if ( d == NULL ) return -1;
    pthread_mutex_lock(&memberships_lock);
	while ((dir = readdir(d)) != NULL) {
        char dir_name[64];
        strcpy(dir_name, dir->d_name);
        char* dir_type = strtok(dir_name, "_");
		if (dir_type == NULL) continue;
		if ( strcmp(dir_type, "membership") == 0 ){
            char members_path[128]; 
	        strcpy(members_path, CHAT_DIR);
            strcat(members_path, "/");
            strcat(members_path, dir->d_name);

            if (del_user(guest_name, members_path) < 0){
                closedir(d);
                pthread_mutex_unlock(&memberships_lock);
                return -1;
            }
        }
    }
    closedir(d);
    pthread_mutex_unlock(&memberships_lock);
    return 0;
}


/*
This function clears the file guests.txt
It is called everytime the server restarts because
we don't want guests who didn't log out to outlive 
a server shutdown
it returns 
- 0 in case of success
- -1 otherwise
*/
int clear_guests(){
    pthread_mutex_lock(&guests_lock);
    FILE* f = fopen(GUESTS_FILE, "w");
    if (f == NULL) return -1;
    fclose(f);
    pthread_mutex_unlock(&guests_lock);
	return 0;
}

/*
This function checks if a given chat exists
by iterating through the directory data/groupchats 
to find a file named .*_<chat_name>...
it returns 
	- 0 if it found the chat
	- -1 in case of system error
	- -2 if the chat doesn't exist
*/
int chat_exists(char* chat_name) {
	DIR *d;
	struct dirent *dir;
	d = opendir(CHAT_DIR);
	if ( d == NULL ) return -1;
	while ((dir = readdir(d)) != NULL) {
		char* dir_name = strtok(dir->d_name, "_");
		if (dir_name == NULL) continue;
		dir_name = strtok(NULL, ".");
		if (dir_name == NULL) continue; 
		if ( strcmp(dir_name, chat_name) == 0 ) return 0;
	}

	closedir(d);
	return -2;
}

/*
This function creates a chat <chat_name>
and sets its founder to be user <u>
it returns 
	- 0 in case of success
	- -1 otherwise
*/
int create_chat(char* chat_name, user* u){
	if ( chat_exists(chat_name) == 0 ) return -2;
	char members_path[128]; 
	strcpy(members_path, CHAT_DIR);
	strcat(members_path, "/membership_");
	strcat(members_path, chat_name);
	strcat(members_path, ".txt");
	pthread_mutex_lock(&memberships_lock);
	FILE* fm = fopen(members_path, "w");
	if ( fm == NULL ){
		pthread_mutex_unlock(&memberships_lock);
		return -1;
	}
	fprintf(fm, "%s\n", u->username);
	fclose(fm);
	pthread_mutex_unlock(&memberships_lock);

	char chat_path[128];
	strcpy(chat_path, CHAT_DIR);
	strcat(chat_path, "/chat_");
	strcat(chat_path, chat_name);
	strcat(chat_path, ".txt");
	pthread_mutex_lock(&chats_lock);
	FILE* fc = fopen(chat_path, "w");
	if ( fc == NULL ){
		pthread_mutex_unlock(&chats_lock);
		return -1;
	}
	fprintf(fc, "%s : I created this chat!\n", u->username);
	fprintf(fc, "%s : I and only I can add new members!\n", u->username);
	fclose(fc);
	pthread_mutex_unlock(&chats_lock);
	return 0;
}


/*
This function sends a message <msg> from user <u>
in chat <chat_name>. It must be called after 
verifying the permissions of the user <u>
it returns
	- 0 in case of success
	- -1 otherwise
*/
int send_chat_msg(char* chat_name, char* msg, user* u){
	if (is_chat_member(chat_name, u->username) != 0) return -2;
	char path[128]; 
	strcpy(path, CHAT_DIR);
	strcat(path, "/chat_");
	strcat(path, chat_name);
	strcat(path, ".txt");
	pthread_mutex_lock(&chats_lock);
	FILE* f = fopen(path, "a");
	if ( f == NULL ) return -1;
	fprintf(f, "%s : %s", u->username, msg);
	fclose(f);
	pthread_mutex_unlock(&chats_lock);
	return 0;
}


/*
This function determines if a user is
a member of a chat group
it returns 
	- 0 if he is
	- -1 in case of system error
	- -2 if he isn't

*/
int is_chat_member(char* chat_name, char* member_name){
	char members_path[128]; 
	strcpy(members_path, CHAT_DIR);
	strcat(members_path, "/membership_");
	strcat(members_path, chat_name);
	strcat(members_path, ".txt");
	pthread_mutex_lock(&memberships_lock);
	FILE* f = fopen(members_path, "r");
	if ( f == NULL ){
		pthread_mutex_unlock(&memberships_lock);
		return -1;
	}

	char line[128];
	while (fgets(line, 128, f) != NULL ) {
		if ( strcmp(member_name, strtok(line, "\n")) == 0 ) {
			fclose(f);
			pthread_mutex_unlock(&memberships_lock);
			return 0;
		}
	}
	fclose(f);
	pthread_mutex_unlock(&memberships_lock);
	return -2;
}

int add_chat_member(char* chat_name, char* new_member, user* u){
	if (chat_exists(chat_name) != 0) return -2;
	if (is_chat_member(chat_name, new_member) == 0) return 0;
	if (user_exists(new_member) != 0) return -3;

	//checking if the user adding a new member is the founder of the group chat
	char members_path[128]; 
	strcpy(members_path, CHAT_DIR);
	strcat(members_path, "/membership_");
	strcat(members_path, chat_name);
	strcat(members_path, ".txt");
	pthread_mutex_lock(&memberships_lock);
	FILE* f = fopen(members_path, "r");
	if ( f == NULL ){
		pthread_mutex_unlock(&memberships_lock);
		return -1;
	}
	char founder[50];
	if (fgets(founder, 50, f) == NULL){
		fclose(f);
		pthread_mutex_unlock(&memberships_lock);
		return -1;
	}
	if (strcmp(strtok(founder, "\n"), u->username) != 0){
		fclose(f);
		pthread_mutex_unlock(&memberships_lock);
		return -4;
	}
	fclose(f);
	
	//adding the new member in the membership file of the group chat
	f = fopen(members_path, "a");
	if ( f == NULL ) {
		pthread_mutex_unlock(&memberships_lock);
		return -1;
	}
	fprintf(f, "%s\n", new_member);
	fclose(f);
	pthread_mutex_unlock(&memberships_lock);
	return 0;
}

char* read_chat(char* chat_name, int max_msgs){
	char chat_path[128];
	strcpy(chat_path, CHAT_DIR);
	strcat(chat_path, "/chat_");
	strcat(chat_path, chat_name);
	strcat(chat_path, ".txt");
	pthread_mutex_lock(&chats_lock);
	FILE* f = fopen(chat_path, "r");
	if ( f == NULL ) {
		pthread_mutex_unlock(&chats_lock);
		return 0;
	}

	int msg_counter = 0;
	char* ans = (char*) calloc(max_msgs, 128);
	char* tkn;
	char line[128];
	while (fgets(line, 128, f) != NULL ){
		if (msg_counter >= max_msgs){
			tkn = strtok(ans, "\n");
			tkn = strtok(NULL, "");
			if (tkn == NULL) tkn = "";
			strcpy(ans, tkn);
		}
		strcat(ans, line);
		msg_counter++;
	}
	fclose(f);
	pthread_mutex_unlock(&chats_lock);
	return ans;
}


char* chats_of(char* username){
    DIR *d;
	struct dirent *dir;
	d = opendir(CHAT_DIR);
	if ( d == NULL ) return 0;
    char* ans = (char*) calloc(5, 32);
	ans[0] = '\n';
	ans[1] = '\0';
	while ((dir = readdir(d)) != NULL) {
        char dir_name[64];
        strcpy(dir_name, dir->d_name);
        char* dir_type = strtok(dir_name, "_");
		if (dir_type == NULL) continue;
		if ( strcmp(dir_type, "membership") == 0 ){
            dir_type = strtok(NULL, ".");
            if (dir_type == NULL) continue;
            if (is_chat_member(dir_type, username) == 0){
                strcat(ans, dir_type);
                strcat(ans, "\n");
            }
        }
    }
    closedir(d);
    return ans;
}
