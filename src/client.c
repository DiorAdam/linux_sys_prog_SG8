#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "server.h"

void comm_loop_client(int sockfd){
	char buff_out[MAX_MESSAGE_LENGTH];
	char buff_in[5*MAX_MESSAGE_LENGTH];
	int pos;
	char help[400] = ""; 
	strcat(help, "supported commands are the following:\n");
	strcat(help,"\t- SIGNUP <username> <password>\n");
	strcat(help,"\t- LOGIN <username> <password>\n");
	strcat(help,"\t- GUEST\n");
	strcat(help,"\t- CREATE <chat group>\n");
	strcat(help,"\t- ADD <chat group> <username>\n");
	strcat(help,"\t- SEND <chat group> <message>\n");
	strcat(help,"\t- READ <chat group> <number of messages>\n");
	strcat(help,"\t- CHATS\n");
	strcat(help,"\t- LOGOUT\n");
	strcat(help,"\t- EXIT\n");            
	printf("%s\n",help);
	for (;;) {
		bzero(buff_in, sizeof(buff_in));
		bzero(buff_out, sizeof(buff_out));
		printf(" >>> ");
		pos = 0;
		while ((buff_out[pos] = getchar()) != '\n') pos++;

		write(sockfd, buff_out, strlen(buff_out));
		if ((strcmp(buff_out, "EXIT\n")) == 0) {
			printf("Disconnecting\n");
			break;
		}

		
		read(sockfd, buff_in, sizeof(buff_in));
		printf("%s", buff_in);
		
	}
}

int main(){
    int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("client_main.c: socket creation failed\n");
		exit(0);
	}
	else
		printf("Socket successfully created\n");

	bzero(&servaddr, sizeof(servaddr));
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);


	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
		printf("client_main.c: connection with the server failed\n");
		exit(0);
	}
	else
		printf("Connected to the server\n");

	comm_loop_client(sockfd);

	close(sockfd);
}


