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
void communication_loop(int connfd){
    char buff[MAX_MESSAGE_LENGTH];
	
	for (;;) {
		bzero(buff, MAX_MESSAGE_LENGTH);
		
		read(connfd, buff, sizeof(buff));
		
        char** command = parse(buff);

        if (!command){
            char response[] = "I don't understand this command\n";
		    write(connfd, response, sizeof(response));
        }
        else {
            if (strncmp(command[0], "exit", 4) == 0){
                printf("Client handled by server_thread_%u disconnected\n", (unsigned) pthread_self());
                break;
            }
        }
        free(command);
    }
}

char** parse(char* msg){
    char** ans = (char**) malloc(sizeof(char*));
    if (strncmp(msg, "exit", 4) == 0) {
        ans[0] = msg;
        return ans;
    }
    return 0;
}

void* client_listener(void* sockfdaddr){
    int connfd, len;
	struct sockaddr_in cli;
	int sockfd = *((int*) sockfdaddr);

	if ((listen(sockfd, 5)) != 0) {
		printf("server.c: listen failed\n");
		exit(0);
	}
	else
		printf("server_thread_%u listening..\n", (unsigned) pthread_self());
	len = sizeof(cli);

	connfd = accept(sockfd, (struct sockaddr*) &cli, &len);
	if (connfd < 0) {
		printf("server.c: server_thread_%u couldn't accept client\n", (unsigned) pthread_self());
		exit(0);
	}
	else
		printf("server_thread_%u accepted client\n", (unsigned) pthread_self());

    communication_loop(connfd);

}

void init_server(){
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



    // using several threads to handle multiple clients
    pthread_t client_threads[MAX_CLIENTS];
    printf("yolo");
    for (int i=0; i< MAX_CLIENTS; i++){
        if (pthread_create(&client_threads[i], NULL, client_listener, (void*) &sockfd) != 0){
		    printf("server.c: thread_%d creation failed\n", i);
        }
    }
    
    for (int i=0; i < MAX_CLIENTS; i++){
        pthread_join(client_threads[i], NULL);
    }
	
	close(sockfd);
}

