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
#include "server_utils.c"


/* 
This function communicates with the client by: 
        - receiving the client's messages
        - parsing them and checking if the command is recognized
		- executing the necessary procedures
        - and then answering to the client 
It runs in its own thread and handles one and only one client.
*/


void* communication_loop(void* connfdaddr){
	int connfd = *((int*) connfdaddr);
    char buff[MAX_MESSAGE_LENGTH];
	user u;
	bzero(u.username, 32);
	for (;;) {
		bzero(buff, MAX_MESSAGE_LENGTH);
		read(connfd, buff, sizeof(buff));
		if (strlen(buff) == 0) break;

        char* response = parse_exec(&u, buff);
		if (strcmp(response, "EXIT\n") == 0){
			printf("Client handled by server_thread_%u disconnected\n", (unsigned) pthread_self());
			break;
		}
		write(connfd, response, strlen(response));
		free(response);
    }
	close(connfd);
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

/*
This function runs in the main thread
and listens for new clients.
whenever it accepts a new client, it creates a new
communication_loop() thread that will handle that client
*/
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

/* 
This function initializes the server. It calls:
	- clear_guests() to clear the file GUESTS_FILE (Guests cannot outlive a server shutdown)
	- and function's to set up the server's TCP connection and listen for clients 
*/
void init_server(){
	if (clear_guests() < 0) printf("An Error occurred while clearing guest list");
	int sockfd = make_sock();
	listen_sock(sockfd);
	handle_conn(sockfd);
	close(sockfd);
}


void main(){
	init_server();
}

