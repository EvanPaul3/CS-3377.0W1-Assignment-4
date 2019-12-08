//-----------------------------------------------------------------------------------------------------------
// Created by Evan Paul using sampleConcurrentServer.c as base code
// 12/7/2019
// For CS 3377.0W1 Assignment 4
//-----------------------------------------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sysexits.h>
#define MAXLINE 4096 /*max text line length*/
#define LISTENQ 8 /*maximum number of client connections*/

int main (int argc, char **argv)
{
	int listenfd, connfd, n, stdoutBackup;
	pid_t childpid, pidForPipe;
	socklen_t clilen;
	char buf[MAXLINE];
	char pipeBuffer[MAXLINE];
	struct sockaddr_in cliaddr, servaddr;

	alarm(300); //server terminates after 5 minutes

	//basic check of the arguments
	if (argc !=4) {
		printf("Usage: a4server1 <Server Port> <UserID> <IP-Address>"); 
		exit(1);
	}


	//Create a socket for the socket
	//If sockfd<0 there was an error in the creation of the socket
	if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		printf("Problem in creating the socket");
		exit(1);
	}

	//preparation of the socket address
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(argv[3]);
    servaddr.sin_port =  htons((int) strtol(argv[1], (char **)NULL, 10)); 

	//bind the socket
	if(bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		printf("bind error");
		exit(1);
	}

	//listen to the socket by creating a connection queue, then wait for clients
	listen (listenfd, LISTENQ);

	printf("%s\n","Server running...waiting for connections.");

	for ( ; ; ) { //wait for next connection forever
		int status;
		clilen = sizeof(cliaddr);
		
		//accept a connection
		connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
		
		
		printf("%s\n","Received request...");

		if ( (childpid = fork()) == -1 ) {	
			printf("Failed to fork");
			exit(1);
		} else if (childpid == 0) { //if it's 0, it’s a child process
			printf ("%s\n","Child created for dealing with client requests");
			
			//close listening socket
			close (listenfd);

			//receive command from client
			while ( (n = recv(connfd, buf, MAXLINE,0)) > 0)  {
        
			char *command = malloc(MAXLINE + 1);
			const size_t commandSize = MAXLINE + 1;
      
			printf("\n** Server Worker PID:");
			printf("%ld", (long)getpid());
			printf("\n");
			system("date");
        
  			//remove \r from end of string
			strncpy(command, buf, commandSize);
			
			command = strtok(command, "\t"); //delimit tab
			command = strtok(NULL, "\t");    //select string after tab
        
			//Create a second string to handle start and end of transactions
			char *commandTest = malloc(MAXLINE + 1);
				
			//create a copy of the command to check of start and end of transaction	
			strncpy(commandTest, command, commandSize);
			commandTest = strtok(commandTest, " ");
        		
			//detect start-of-transaction
			if(strncmp(commandTest, "start-of-transaction", 20) == 0) {
				//choose to read string right of space delimiter
				commandTest = strtok(NULL, " ");
				  
				//verify that User ID from text file matches User ID from server
				if(strncmp(commandTest, argv[2], 9) == 0) {
					printf("\nUser ID matches, accepted");
					send(connfd, "User ID matches, executing commands...\n", 40, 0);            
					continue;
				} else {
					//close connection if User ID does not match
					printf("%s\n", "User ID does not match server ID, rejected");
					send(connfd, "User ID rejected", 16, 0);
					exit(0);
				}
           
				
			}
			
			//detect end-of-transaction
			if(strncmp(commandTest, "end-of-transaction", 18) == 0) {
			  printf("\nEnd-of-transaction reached");
			  send(connfd, "End-of-transaction reached",26, 0);
			  continue;
			} 
			
			//remove \r from end of string
			command = strtok(command, "\r");
  
			//create a backup of stdout, used to restore stdout later
			stdoutBackup = dup(fileno(stdout));
        
			int p[2];
        
			//open a pipe to process command
			pipe2(p, 0);;
        
			//redirect stdout via pipe
			dup2(p[1], fileno(stdout));
			
			//execute command with a system call
			printf("Executing Command: ");
			printf("%s", command);
			system(command);
			close(p[1]);
      
			//restore stdout
			dup2(stdoutBackup, fileno(stdout));
			
			//read from pipe into pipeBuffer
			read(p[0], pipeBuffer, MAXLINE);
			
			//printf("\nPipe Buffer: %s", pipeBuffer); //for testing

			//send command output string from pipeBuffer to client
			printf("\nSending the output of the command to client...\n");
			send(connfd, pipeBuffer, sizeof(pipeBuffer), 0);

			//reset pipeBuffer for next command
			memset(pipeBuffer, 0, sizeof(pipeBuffer));                 
		} 
          
		if(n<0)
			printf("Read error");
          
		//end child process
		exit(0);
                    
		} else { //parent process
			if ((childpid=waitpid(childpid, &status, 0)) < 0) {
				fprintf(stderr, "Waitpid error for client connection: %s\n",
				strerror(errno));
			}
			printf("\nEnding connection to the client\n");
			printf("\nWaiting for connections\n");
		}
    
		//close socket of the server
		close(connfd);
	}	
}
