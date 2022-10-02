#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#define SIZE 256

#define randnum(min, max) \
        ((rand() % (int)(((max) + 1) - (min))) + (min))
int PORT = 0;

int clientConnect(int portNum)
{
	struct sockaddr_in serverAddr;
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd == -1) perror("socket");
	
	//bzero((void *)&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portNum);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	
	if(connect(sd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		perror("Connect error");
		close(sd);
		return -1;
	}
	return sd;
}

int validateArgs(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("<Missing game type>\n");
		printf("try ./cli numbers petarServer 9080\n");
		exit(1);
	}
	if(argc < 3)
	{
		printf("<Missing game server>\n");
		printf("try ./cli numbers petarServer 9080\n");
		exit(1);
	}
	
	if(strcmp(argv[1], "numbers") != 0)
	{
		printf("<Err:game name>\n");
		printf("try ./cli numbers petarServer 9080\n");
		exit(1);
	}
	if(strcmp(argv[2], "petarServer") != 0)
	{
		printf("<Err:server name>\n");
		printf("try ./cli numbers petarServer 9080\n");
		exit(1);
	}
	if(strcmp(argv[3], "9080") != 0)
	{
		printf("<Err:port number>\n");
		printf("try ./cli numbers petarServer 9080\n");
		exit(1);
	}
	else
	{
		return atoi(argv[3]);
	}
}

int main(int argc, char* argv[])
{
	PORT = validateArgs(argc, argv);
	int sd = clientConnect(PORT);
	char input[SIZE] = {};
	ssize_t rec = 0;
	int r = 0;
	srand(time(NULL));
	printf("Connecting to server...\n");
	printf("Joining lobby...\n");
	printf("You enter 1 to 9 or quit to exit the game.\n");
	while(1)
	{
		rec = recv(sd, input, sizeof(input), 0);
		if(rec > 0)
		{
			input[rec] = '\0';
			printf("SERVER:[%s]\n", input);
			
			if(strcmp(input, "Welcome to the server") == 0)
			{
				send(sd, "Ready to play", SIZE, 0);
			}
			if(strcmp(input, "END") == 0)
			{
				rec = 0;
			}
			if(strcmp(input, "GO") == 0)
			{
				bzero(input, sizeof(input));
				
				// auto fill
				r = randnum(1, 13);
				printf("> %d\n", r);
				if(r > 9)
				{
					snprintf(input, SIZE, "%d", r);
					usleep(5000);
					send(sd, input, SIZE, 0);
				}
				else
				{
					snprintf(input, SIZE, "MOVE:%d", r);
					usleep(5000);
					send(sd, input, SIZE, 0);
				}
				
				
				// user input
//				printf("> ");
//				scanf("%s", input);
//				if(isdigit(input[0]) != 0)
//				{
//					if(isdigit(input[1]) != 0)
//					{
//						//printf("ERROR");
//						send(sd, input, SIZE, 0);
//					}
//					else
//					{
//						int n = input[0] - '0';
//						snprintf(input, SIZE, "MOVE:%d", n);
//						send(sd, input, SIZE, 0);
//					}
//				}
//				else if(strcmp(input, "quit") == 0)
//				{
//					send(sd, "QUIT", SIZE, 0);
//					//rec = 0;
//				}
//				else
//				{
//					printf("error\n");
//					send(sd, input, SIZE, 0);
//				}
				
			}
		}
		if(rec == 0)
		{
			printf("Client quitting...\n");
			close(sd);
			exit(1);
		}
		if(rec == -1)
		{
			printf("Error on %d:[%s]\n", sd, strerror(errno));
			break;
		}
	}
	
	close(sd);
}
