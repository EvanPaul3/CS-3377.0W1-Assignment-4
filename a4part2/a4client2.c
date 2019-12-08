//-----------------------------------------------------------------------------------------------------------
// Created by Evan Paul
// 12/7/2019
// For CS 3377.0W1 Assignment 4
//-----------------------------------------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>


#define MAXLINE 4096 /*max text line length*/

int
main(int argc, char **argv) 
{
	int sockfd;
	struct sockaddr_in servaddr;
	char acommand[MAXLINE], recvline[MAXLINE];
	
	alarm(300); //Client terminates after 5 minutes
	
	//basic check of the arguments
	if (argc !=6) {
		perror("Usage: a4client1 <Server IP> <Server Port> <UserID> <Text-File-Containing-Commands> <Output textfile>"); 
		exit(1);
	}
		
	//Create a socket for the client
	//If sockfd<0 there was an error in the creation of the socket
	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		perror("Problem in creating the socket");
		exit(2);
	}
		
	//Creation of the socket
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr= inet_addr(argv[1]);
	servaddr.sin_port =  htons((int) strtol(argv[2], (char **)NULL, 10));
	

		
	//Connection of the client to the socket 
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
		perror("Problem in connecting to the server");
		exit(3);
	}
	
  printf("\nClient ID: %s", argv[3]);
  printf("\n");
	system("date");
 
  //open file to read commands from
	FILE *inputFile;
  inputFile = fopen(argv[4], "r");
  
  //open file to write command results to
  FILE *outputFile;
  outputFile = fopen(argv[5], "w");
  
	while (fgets(acommand, MAXLINE, inputFile) != NULL)  {
		fputs("\nNew command read", outputFile);
   
		printf("\nProcessing command #");
		printf("%s", acommand);
    fputs("\nProcessing command: ", outputFile);
    fputs(acommand, outputFile);
   
    printf("Sending command to server...");
		fputs("Sending command to server...", outputFile);
   
		send(sockfd, acommand, strlen(acommand), 0);
			
      
      
		if (recv(sockfd, recvline, MAXLINE,0) == 0){
			//error: server terminated prematurely
			perror("The server terminated prematurely"); 
			exit(4);
		}
		printf("\nString received from the server: \n");
    fputs("\nReceived string from server", outputFile);
    fputs("\nResult of command:\n", outputFile);
    fputs(recvline, outputFile);
   
    //reset recvline for next command
		memset(recvline, 0, sizeof(recvline));  
	} 
	fclose(inputFile);
  fclose(outputFile);
	printf("\nThe commands of the transaction file have been exhausted, ending client...\n");
	exit(0);
}