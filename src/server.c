#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

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

		int res = addUser(un, pwd);
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
		int res = correctCredentials(un, pwd);
		if (res == 0){
			strcpy(u->username, un);
			strcpy(ans, LOGIN_SUCCESS);
		}
		else if (res == -1){
			strcpy(ans, LOGIN_ERROR);
		}
		else if (res == -2){
			strcpy(ans, INVALID_PASSWORD);
		}
		else{
			strcpy(ans, INVALID_USERNAME);
		}
		return ans;
	}

	else if (strcmp(curr_tkn, "LOGOUT") == 0){
		curr_tkn = strtok(NULL, " \n");
		if (curr_tkn != NULL){
			strcpy(ans, PARSING_ERROR);
			return ans;
		}

		if (!strlen(u->username)){
			strcpy(ans, LOGOUT_ERROR);
			return ans;
		}
		memset(u->username, '\0', sizeof(u->username));
		strcpy(ans, LOGOUT_SUCCESS);
		return ans;
	}

    return 0;
}

int isValidUsername(char* username) {
	int length = 0; char c;
	while ( (c = *username) != '\0' ) {
		if ( c == ' ' || c == '\n' || c == '\t' || c >= 128 )
			return 0;
		username++;
		length++;
	}
	return length < 30 && length > 1;
}

int isValidPassword(char* password) {
	int length = 0; char c;
	while ( (c = *password) != '\0' ) {
		if ( c == ' ' || c == '\n' || c == '\t' || c >= 128 )
			return 0;
		password++;
		length++;
	}
	return 1;
}



int userExists(char* username) {
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

int correctCredentials(char* username, char* password) {
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

int addUser(char* username, char* password) {
	if ( !isValidPassword(password) ) return -2;
	if (!isValidUsername(username)) return -3;
	if ( userExists(username) != 1 ) return -4;
	FILE* f = fopen(CREDENTIALS_FILE, "a");
	if ( f == NULL )
		return -1;
	fprintf(f, "%s %s\n", username, password);
	fclose(f);
	return 0;
}
 
char* randomString(char *str, size_t size) {
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
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

