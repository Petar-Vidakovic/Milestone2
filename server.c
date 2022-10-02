#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <time.h>

#define SIZE 256

int checkMessage(int maxSum,
	int* connections,
	char* buf,
	char* score,
	int i,
	ssize_t* rec,
	int* sum,
	int* winner, int* attempts);
int createServerSocket(int portNum);
void validateArgs(int argc);

int main(int argc, char* argv[])
{
	validateArgs(argc);
	int PORT = atoi(argv[1]);
	int maxClients = atoi(argv[3]);
	
	fd_set fds;
	socklen_t addrLen;
	struct sockaddr_in nAddr;
	ssize_t rec;
	
	int minClients = 1;
	maxClients = maxClients + 1;
	
	int status;
	int currentClients = 0;
	int sum = 0, maxSum = 30;
	
	int winner = 0;
	int check = 0;
	int attempts = 1;
	
	char buf[SIZE] = {};
	char score[SIZE] = {};
	
	int connections[maxClients];
	
	snprintf(score, SIZE, "The sum is: %d, Enter a number.", sum);
	int hostSocket = createServerSocket(PORT);
	memset(connections, -1, sizeof(connections));
	connections[0] = hostSocket;
	
	printf("Server initiated... \n");
	printf("waiting for %d connections to join the server...\n", maxClients - 1);
	
	while(1)
	{
		FD_ZERO(&fds);
		
		// add connections to the fd set
		for(int i = 0; i < maxClients; i++)
		{
			if(connections[i] >= 0)
			{
				FD_SET(connections[i], &fds);
			}
		}
		
		if(currentClients == maxClients - 1)
		{
			goto clientIO;
		}
		
		if((status = select(FD_SETSIZE, &fds, NULL, NULL, NULL)) >= 0)
		{
			// if something is happening on the server socket
			if(FD_ISSET(hostSocket, &fds))
			{
				int newSock = accept(hostSocket, (struct sockaddr*)&nAddr, &addrLen);
				if(newSock >= 0)
				{
					send(newSock, "Welcome to the server", 128, 0);
					printf("New connection on socket %d, Player %d joined the lobby\n", newSock, status);
					for(int i = 1; i < maxClients; i++)
					{
						if(connections[i] < 0)
						{
							connections[i] = newSock;
							break;
						}
					}
					currentClients++;
				}
				if(currentClients == maxClients - 1)
				{
					printf("All players have joined the lobby the game will start...\n\nNUMBERS\n");
					for(int i = 1; i < maxClients; i++)
					{
						rec = recv(connections[i], buf, SIZE, 0);
						if(rec > 0)
						{
							//recv welcomes
							buf[rec] = '\0';
							printf("Player %d:[%s]\n", i, buf);
							bzero(buf, SIZE);
						}
					}
				}
			}
			else
			{
				// the clients are interacting with the server here
				// we know the status of select is currentClients
				// so jump down into the IO
			clientIO:
				if(currentClients == maxClients - 1)
				{
					for(int i = 1; i < maxClients; i++)
					{
						int errorGo = 1;
						if((connections[i] > 0) && (FD_ISSET(connections[i], &fds)) && (sum < maxSum))
						{
							if((send(connections[i], score, SIZE, 0) && (send(connections[i], "GO", SIZE, 0) == -1)))
							{
								perror("send");
							}
							else
							{
							tryAgain:
								rec = recv(connections[i], buf, SIZE, 0);
								if(rec > 0)
								{
									check = checkMessage(maxSum,
										connections,
										buf,
										score,
										i,
										&rec,
										&sum,
										&winner,
										&errorGo);
									
									if(check == -1)
									{
										errorGo++;
										goto tryAgain;
									}
									if(check == 0)
									{
										currentClients--;
										connections[i] = -1;
									}
								}
								
								// handle connection lost
								if(rec == 0)
								{
									send(connections[i], "END", SIZE, 0);
									printf("closing connection %d\n", i);
									close(connections[i]);
									connections[i] = -1;
									currentClients--;
									FD_CLR(connections[i], &fds);
								}
								//handle recv error
								if(rec == -1)
								{
									//printf("Error on %d:[%s]\n", connections[i], strerror(errno));
									break;
								}
								// handle losers for normal win
								if(rec == -2)
								{
									for(int j = 1; j <= currentClients; j++)
									{
										if(j != winner)
										{
											send(connections[j], "YOU LOSE", SIZE, 0);
											send(connections[j], "END", SIZE, 0);
										}
									}
								}
							}
						}
						if(currentClients == minClients)
						{
							// handle winner if the other client quit out or error out
							for(int j = 1; j < maxClients; j++)
							{
								if(connections[j] >= 0)
								{
									printf("Player %d Won!\n", j);
									send(connections[j], "You win!", SIZE, 0) && send(connections[j], "END", SIZE, 0);
								}
							}
							sum = maxSum;
							goto endGame;
						}
					}
					
				} //<END OF CLIENT IO>
				
				// games over close out all connections
			endGame:
				if(sum >= maxSum)
				{
					printf("game over!!\n\n");
					for(int i = 1; i < maxClients; i++)
					{
						if(connections[i] > 0)
						{
							send(connections[i], "END", SIZE, 0);
						}
					}
					exit(1);
				}
			}
			//printf("<END OF SELECT>\n");
		}
		//printf("<END OF WHILE>\n\n");
	}
}

int checkMessage(int maxSum,
	int* connections,
	char* buf,
	char* score,
	int i,
	ssize_t* rec,
	int* sum,
	int* winner, int* attempts)
{
	buf[(*rec)] = '\0';
	printf("Player %d:  %s  -  ", connections[i] - 3, buf);
	
	if(strcmp(buf, "QUIT") == 0)
	{
		printf("player %d quit\n", i);
		connections[i] = -1;
		send(connections[i], "END", SIZE, 0);
		return 0;
	}
	else if(isdigit(buf[5]) != 0)
	{
		(*sum) += buf[5] - '0';
		printf("sum:%d\n", (*sum));
		snprintf(score, SIZE, "The sum is: %d, Enter a number.", (*sum));
		if((*sum) >= maxSum)
		{
			//send you wins to the ith winner
			if(send(connections[i], "YOU WIN", SIZE, 0) == -1)
			{
				perror("send");
			}
			else
			{
				(*winner) = i;
				printf("Player %d won!!\n", i);
			}
			(*rec) = -2;
		}
		return 1;
	}
	else
	{
		if(*attempts == 5)
		{
			printf("ERROR (%d/5)\n", *attempts);
			send(connections[i], "END", SIZE, 0);
			connections[i] = -1;
			printf("player %d disconnects\n", i);
			return 0;
		}
		printf("ERROR (%d/5)\n", *attempts);
		char errorIn[SIZE] = {};
		snprintf(errorIn, SIZE, "ERROR:%s", buf);
		send(connections[i], errorIn, SIZE, 0) && send(connections[i], "GO", SIZE, 0);
		return -1;
	}
}

int createServerSocket(int portNum)
{
	struct sockaddr_in host;
	int opt = 1;
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	//fcntl(serverSocket, F_SETFL, O_NONBLOCK);
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt,
		sizeof(int));
	if(serverSocket == -1)
	{
		perror("socket");
		return -1;
	}
	
	memset(&host, 0, sizeof(host));
	host.sin_family = AF_INET;
	host.sin_addr.s_addr = htonl(INADDR_ANY);
	host.sin_port = htons(portNum);
	
	if(bind(serverSocket, (struct sockaddr*)&host, sizeof(struct sockaddr_in)) == -1)
	{
		perror("bind");
		return -1;
	}
	if(listen(serverSocket, 10) == -1)
	{
		perror("listen");
		return -1;
	}
	
	return serverSocket;
}

void validateArgs(int argc)
{
	if(argc < 2)
	{
		printf("<Missing game type>\n");
		printf("try ./serv 9080 numbers 3\n");
		exit(1);
	}
	if(argc < 3)
	{
		printf("<Missing game server>\n");
		printf("try ./serv 9080 numbers 3\n");
		exit(1);
	}
	if(argc < 4)
	{
		printf("<missing player size>\n");
		printf("try ./serv 9080 numbers 3\n");
		exit(1);
	}
}