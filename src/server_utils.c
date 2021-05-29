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
		printf("yolo1\n");
		int res = create_chat(chat_name, u);
		printf("yolo3\n");
		if (res == 0) strcpy(ans, CHAT_CREATION_SUCCESS);
		else if (res == -1) strcpy(ans, CHAT_CREATION_ERROR);
		else strcpy(ans, CHAT_EXISTS);
		return ans;
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
		printf("yolo1\n");
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
		printf("yolo2\n");
		char* chat_msgs = read_chat(chat_name, num_msgs);
		if (chat_msgs == 0){
			strcpy(ans, READ_CHAT_ERROR);
			return ans;
		}
		free(ans);
		return chat_msgs;
	}
    return 0;
}

int is_valid_username(char* username){
	int length = 0; 
	char c;
	while ( (c = *username) != '\0' ) {
		if ( c == ' ' || c == '\n' || c == '\t' || c >= 128 )
			return 0;
		username++;
		length++;
	}
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



int user_exists(char* username) {
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
	return 1;
}

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
		fprintf(stdout, "%s vs %s / %s vs %s.\n", username, user, password, pass);
		if ( strcmp(user, username) == 0 ) {
			fclose(f);
			pthread_mutex_unlock(&credentials_lock);
			if ( strcmp(pass, password) == 0 )
				return 0; //valid combination
				
				return -2; //user exists
		}
	}
	
	fclose(f);
	pthread_mutex_unlock(&credentials_lock);
	return -3; //user doesn't exist
}

int add_user(char* username, char* password) {
	if ( !is_valid_password(password) ) return -2;
	if (!is_valid_username(username)) return -3;
	if ( user_exists(username) != 1 ) return -4;
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
 
char* random_string(char *str, size_t size) {
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK";
	if (size) {
		srand(time(0));
		--size;
		for (size_t n = 0; n < size; n++) {
			int key = rand() % (int) (sizeof(charset) - 1);
			str[n] = charset[key];
		}
		str[size] = '\0';
	}
	return str;
}

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
	return -1;
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

