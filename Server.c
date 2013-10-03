#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <time.h>

#define STDIN 0
#define USERS 9

int main (int argc, char **argv) {
	signal(SIGPIPE, SIG_IGN);
	setvbuf(stdout, NULL, _IONBF, 0);

	int i = 0;
	int j = 0;
	int cllen = 0;
	int dalen = 0;
	int inlen = 0;
	int breakout = 1;
	int present = 0;	
	int status = 0;	
	int lport = 34789;
	int clientmax = 0;

	char *search = " ";
	char *end = "\0";
	char *user;
	char *pass;

	char welcome[1] = "0";
	char incorrect[1] = "1";
	char lock[1] = "2";
	char input[1] = "3";
	char broad[1] = "4";
	char disc[1] = "5";

	char cldata[100];
	char bmsg[100];
	char *allauths = malloc(1000*sizeof(char));
	char *auth[USERS][2];
	long long timestamps[USERS][3];
	
	struct sockaddr_in saddr;
	struct sockaddr *uaddr = (struct sockaddr *) &saddr;
	struct sockaddr_storage claddr;
	struct sockaddr *ucladdr = (struct sockaddr *) &claddr;

	fd_set lisocks;
	fd_set clients;
	FD_ZERO(&lisocks);
	FD_ZERO(&clients);

	memset(bmsg, '\0', sizeof(cldata[0]) * 100);
	memset(cldata, '\0', sizeof(cldata[0]) * 100);
	memset(timestamps, 0, sizeof(timestamps[0][0]) * USERS * 3);

	for (i=0; i<USERS; i++)
		for (j=0; j<2; j++)
			auth[i][j] = malloc(100*sizeof(char));

	strcpy(allauths, "3");
	bmsg[0] = broad[0];

	FILE* myfile=NULL;
	myfile=fopen("Credentials.txt","r");
	for(i=0; i<USERS; i++) {
		fscanf(myfile, "%[^ \n] %[^ \n]\n", auth[i][0], auth[i][1]);
		strcat(allauths, auth[i][0]);
		strcat(allauths, "-");
		strcat(allauths, auth[i][1]);
		strcat(allauths, "\n");
	}
	fclose(myfile);

	if (argc == 2) {
		lport = atoi(argv[1]);
	}

	saddr.sin_family = AF_INET;
	saddr.sin_port = lport;
	saddr.sin_addr.s_addr = INADDR_ANY;
	int addrsize = sizeof(saddr);

	int s = socket(PF_INET, SOCK_STREAM, 0);
	int clsock;
	
	if ((status = bind(s, uaddr, addrsize)) == 0) {
		printf("Socket Bound\n");
	}
	else {
		perror("Bind Failed\n");
	}
	if ((status = listen(s, 10)) == 0) {
		printf("Liste‌ning\n\n");
	}
	else {
		perror("Listen Failed\n");
	}

	FD_SET(STDIN, &lisocks);
	FD_SET(s, &lisocks);

	clientmax = s;

	printf("Please enter a command: ");

	while (breakout == 1) {
		clients = lisocks;
		if (select(clientmax+1, &clients, NULL, NULL, NULL) == -1) {
			perror("Select Failed");
		}

		for (i=0; i<=clientmax; i++) {			
			if (FD_ISSET(i, &clients)) {
				if (i == s) {
					cllen = sizeof(claddr);
					clsock = accept(s, ucladdr, &cllen);
					if (clsock == -1) {
						perror("Couldn't Accept");
					}
					else {
						FD_SET(clsock, &lisocks);
						if (clsock > clientmax) clientmax = clsock;
						if ((status = send(clsock, &welcome, sizeof(welcome), 0)) <= 0) {
							perror("Send");
						}
					}
				}
				else if (i == STDIN) {
					if ((dalen = read(i, cldata, 100*sizeof(char))) > 0) {
						cldata[strlen(cldata)-1] = '\0';
						if (!strcmp(cldata, "whoelse")) {
							for (j=0; j<USERS; j++) {		
								if (timestamps[j][0] != 0) {
									printf("%s, ", auth[j][0]);
									present = 1;
								}
							}
							if (present) {
								printf("are here\n\n");
								present = 0;
							}
							else {
								printf("No users are here\n\n");
							} 
						}
						else if (!strcmp(cldata, "wholasthr")) {
							for (j=0; j<USERS; j++) {		
								if (timestamps[j][1] > time(NULL) - 3600 || timestamps[j][0] != 0) {
									printf("%s, ", auth[j][0]);
									present = 1;
								}
							}
							if (present) {
								printf("were here in the last hour\n\n");
								present = 0;
							}
							else {
								printf("No users are here\n\n");
							}
						}
						else if (!strcmp(cldata, "quit")) {
							breakout = 0;
						}
						else if (strcmp(cldata, "broadcast") != 0 && !strcmp(strtok(cldata, search), "broadcast")) {
							strcat(bmsg, strtok(NULL, end));

							for (j=0; j<=clientmax; j++) {			
								if (FD_ISSET(j, &lisocks)) {
									if (j != s && j != STDIN) {
										if ((status = send(j, &bmsg, sizeof(bmsg), 0)) <= 0) {
											perror("Send");
										}
									}
								}
							}
							printf("Broadcast Complete\n\n");
							memset(bmsg, '\0', sizeof(bmsg[0]) * 100);
							bmsg[0] = broad[0];
						}
						else if (!strcmp(cldata, "broadcast") != 0) {
							printf("Must broadcast some message\n\n");
						}
						else {
							printf("%s is not a valid command\n\n", cldata);
						}
						if (breakout) printf("Please enter a command: ");
						memset(cldata, '\0', 100);
					}
				}
				else {
					if ((status = recv(i, cldata, sizeof(cldata), 0)) <= 0) {
						close(i);
                        			FD_CLR(i, &lisocks);
					}
					else {
						if (cldata[0] == welcome[0]) {
							user = strtok(cldata, search)+2;
							pass = strtok(NULL, search);

							for (j=0; j<USERS; j++) {
								if (!strcmp(auth[j][0], user) && !strcmp(auth[j][1], pass)) {
									timestamps[j][0] += 1;
									timestamps[j][1] = time(NULL);
									timestamps[j][2] = time(NULL);
									present = 1;

									if ((status = send(i, &input, sizeof(input), 0)) <= 0) {
										perror("Send");
									}
									break;
								}
							}

							if (present == 0) {
								if (cldata[1] >= '2') {
									if ((status = send(i, &lock, sizeof(lock), 0)) <= 0) {
										perror("Send");
									}
								}
								else {
									if ((status = send(i, &incorrect, sizeof(incorrect), 0)) <= 0) {
										perror("Send");
									}
								}
							}
							else {
								present = 0;
							}
						}
						else if (cldata[0] == input[0]) {
							if ((status = send(i, allauths, 1000*sizeof(char), 0)) <= 0) {
								perror("Send");
							}
						}
						else if (cldata[0] == disc[0]) {
							user = strtok(cldata, search)+3;
							pass = strtok(NULL, search);
							for (j=0; j<USERS; j++) {
								if (!strcmp(auth[j][0], user) && !strcmp(auth[j][1], pass)) {
									timestamps[j][0]--;
									break;
								}
							}
						}
					}
					memset(cldata, '\0', 100);
				}
			}
		}
	}

	if ((status = close(s)) == 0) {
		printf("Socket Closed\n");
	}
	else {
		perror("Close Failed");
	}

	return 0;
}
