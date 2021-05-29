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

#include "server.h"


/* This function communicates with the client by: 
        - listening to the client's messages
        - parsing them and checking if the command is recognized
        - and then answering to the client 
 The client exits the process by using the command <exit> */


void* communication_loop(void* connfdaddr){
	int connfd = *((int*) connfdaddr);
    char buff[MAX_MESSAGE_LENGTH];
	user u;
	for (;;) {
		bzero(buff, MAX_MESSAGE_LENGTH);
		
		read(connfd, buff, sizeof(buff));
		
        char* response = parse_exec(&u, buff);

        if (!response){
            response = "An error occured while executing this command\n";
			write(connfd, response, strlen(response));
        }
        else {
            if (strcmp(response, "EXIT\n") == 0){
                printf("Client handled by server_thread_%u disconnected\n", (unsigned) pthread_self());
                break;
            }
			write(connfd, response, strlen(response));
			free(response);
        }
		
		
    }
	close(connfd);
}

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

    return 0;
}

int is_valid_username(char* username) {
	int length = 0; char c;
	while ( (c = *username) != '\0' ) {
		if ( c == ' ' || c == '\n' || c == '\t' || c >= 128 )
			return 0;
		username++;
		length++;
	}
	return length < 30 && length > 1;
}

int is_valid_password(char* password) {
	int length = 0; char c;
	while ( (c = *password) != '\0' ) {
		if ( c == ' ' || c == '\n' || c == '\t' || c >= 128 )
			return 0;
		password++;
		length++;
	}
	return 1;
}



int user_exists(char* username) {
	FILE* f = fopen(CREDENTIALS_FILE, "r");
	if ( f == NULL )
		return -1;
	
	char line[128]; char* t;
	while ( (t = fgets(line, 128, f)) != NULL ) {
		char* ptr = strtok(line, " ");
		if ( strcmp(ptr, username) == 0 ) {
			fclose(f);
			return 0;
		}
	}
	
	fclose(f);
	return 1;
}

int correct_credentials(char* username, char* password) {
	FILE* f = fopen(CREDENTIALS_FILE, "r");
	if ( f == NULL )
		return -1;
	
	char line[128]; char* t;
	while ( (t = fgets(line, 128, f)) != NULL ) {
		char* ptr = strtok(line, "\n");
		char* user = strtok(ptr, " ");
		char* pass = strtok(NULL, " ");
		fprintf(stdout, "%s vs %s / %s vs %s.\n", username, user, password, pass);
		if ( strcmp(user, username) == 0 ) {
			fclose(f);
			if ( strcmp(pass, password) == 0 )
				return 0; //valid combination
				
				return -2; //user exists
		}
	}
	
	fclose(f);
	return -3; //user doesn't exist
}

int add_user(char* username, char* password) {
	if ( !is_valid_password(password) ) return -2;
	if (!is_valid_username(username)) return -3;
	if ( user_exists(username) != 1 ) return -4;
	FILE* f = fopen(CREDENTIALS_FILE, "a");
	if ( f == NULL )
		return -1;
	fprintf(f, "%s %s\n", username, password);
	fclose(f);
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
	FILE* fm = fopen(members_path, "w");
	if ( fm == NULL ) return -1;
	fprintf(fm, "%s\n", u->username);
	fclose(fm);

	char chat_path[128];
	strcpy(chat_path, CHAT_DIR);
	strcat(chat_path, "/chat_");
	strcat(chat_path, chat_name);
	strcat(chat_path, ".txt");
	FILE* fc = fopen(chat_path, "w");
	if ( fc == NULL ) return -1;
	fprintf(fc, "%s : I created this chat!\n", u->username);
	fprintf(fc, "%s : I and only I can add new members!\n", u->username);
	fclose(fc);
	
	return 0;
}

int send_chat_msg(char* chat_name, char* msg, user* u){
	if (is_chat_member(chat_name, u->username) != 0) return -2;
	char path[128]; 
	strcpy(path, CHAT_DIR);
	strcat(path, "/chat_");
	strcat(path, chat_name);
	strcat(path, ".txt");
	FILE* f = fopen(path, "a");
	if ( f == NULL ) return -1;
	fprintf(f, "%s : %s", u->username, msg);
	fclose(f);
	return 0;
}

int is_chat_member(char* chat_name, char* member_name){
	char members_path[128]; 
	strcpy(members_path, CHAT_DIR);
	strcat(members_path, "/membership_");
	strcat(members_path, chat_name);
	strcat(members_path, ".txt");
	FILE* f = fopen(members_path, "r");
	if ( f == NULL ) return -1;

	char line[128];
	while (fgets(line, 128, f) != NULL ) {
		if ( strcmp(member_name, strtok(line, "\n")) == 0 ) {
			fclose(f);
			return 0;
		}
	}
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
	FILE* f = fopen(members_path, "r");
	if ( f == NULL ) return -1;
	char founder[50];
	if (fgets(founder, 50, f) == NULL) return -1;
	if (strcmp(strtok(founder, "\n"), u->username) != 0) return -4;
	fclose(f);

	//adding the new member in the membership file of the group chat
	f = fopen(members_path, "a");
	if ( f == NULL ) return -1;
	fprintf(f, "%s\n", new_member);
	fclose(f);
	return 0;
}


int make_sock(){
	int sockfd;
	struct sockaddr_in servaddr;

	// socket creation
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("server.c: socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// binding socket with server address
	if ((bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr))) != 0) {
		printf("server.c: Failed to bind socket with server address\n");
		exit(0);
	}
	else
		printf("Bound socket with server on PORT=%d\n", PORT);
	return sockfd;
}

void listen_sock(int sockfd){
	if ((listen(sockfd, 2)) != 0) {
			printf("server.c: listen failed\n");
			exit(0);
		}
	else
		printf("server listening..\n");
}

void handle_conn(int sockfd){
	for(;;){
		struct sockaddr_in new_cl_sock;
		int sz = sizeof(new_cl_sock);
		int connfd = accept(sockfd, (struct sockaddr*) &new_cl_sock, &sz);
		if (connfd < 0) {
			printf("server.c: server couldn't accept the new client\n");
			continue;
		}
		else
			printf("server accepted a new client\n");
		pthread_t client_handler;
		if (pthread_create(&client_handler, NULL, communication_loop, (void*) &connfd) != 0){
		    printf("server.c: thread creation failed\n");
        }
		pthread_detach(client_handler);
	}
}

void init_server(){
	int sockfd = make_sock();
	listen_sock(sockfd);
	handle_conn(sockfd);
	close(sockfd);
}


void main(){
	init_server();
}

