#include <stdio.h>
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
	char *user;
	char *pass;

	char welcome[1] = "0";
	char incorrect[1] = "1";
	char lock[1] = "2";
	char input[1] = "3";
	char broad[1] = "5";

	char cldata[100];
	char *auth[USERS][2];
	long long timestamps[USERS][2];
	
	memset(cldata, '\0', sizeof(cldata[0]) * 100);
	memset(auth, '\0', sizeof(auth[0][0]) * 10 * 2);
	memset(timestamps, 0, sizeof(timestamps[0][0]) * 10 * 2);

	char temp1[100];
	char temp2[100];

	FILE* myfile=NULL;
	myfile=fopen("Credentials.txt","r");
	for(i=0; i<USERS; i++) {
		fscanf(myfile, "%[^ \n] %[^ \n]\n", temp1, temp2);
		auth[i][0] = temp1;
		auth[i][1] = temp2;
	}
	fclose(myfile);

	struct sockaddr_in saddr;
	struct sockaddr *uaddr = (struct sockaddr *) &saddr;
	struct sockaddr_storage claddr;
	struct sockaddr *ucladdr = (struct sockaddr *) &claddr;

	fd_set lisocks;
	fd_set clients;
	FD_ZERO(&lisocks);
	FD_ZERO(&clients);
	
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
				if (i == s) { // Listening socket is selected
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
					if ((dalen = read(i, cldata, sizeof(cldata))) > 0) {
						cldata[strlen(cldata)-1] = '\0';
						if (!strcmp(cldata, "whoelse")) {
							for (j=0; j<USERS; j++) {		
								if (timestamps[j][0] == 1) {
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
								if (timestamps[j][1] > time(NULL) - 3600 || timestamps[j][0] == 1) {
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
						else if (!strcmp(cldata, "broadcast")) {
							printf("broadcasting\n\n");
						}
						else if (!strcmp(cldata, "quit")) {
							breakout = 0;
						}
						else {
							printf("%s is not a valid command\n\n", cldata);
						}
						printf("Please enter a command: ");
						memset(cldata, '\0', 100);
					}
				}
				else {
					if ((dalen = recv(i, cldata, sizeof(cldata), 0)) <= 0) {
						timestamps[j][0] = 0;
					}
					else {
						if (cldata[0] == '0') {
							user = strtok(cldata, search)+2;
							pass = strtok(NULL, search);

							printf("%s--%s\n", user, pass);

							for (j=0; j<USERS; j++) {
								printf("%s %s %s %s %d\n", auth[j][0], user, auth[j][1], pass, !strcmp(auth[j][0], user));
								if (!strcmp(auth[j][0], user) && !strcmp(auth[j][1], pass)) {
									timestamps[j][0] = 1;
									timestamps[j][1] = time(NULL);
									printf("Logged and timing %lld--%lld\n", timestamps[j][0], timestamps[j][1]);
									present = 1;
									break;
								}
							}

							if (present == 0) {
								if (cldata[1] >= '2') {
									if ((status = send(clsock, &lock, sizeof(lock), 0)) <= 0) {
										perror("Send");
									}
								}
								else {
									if ((status = send(clsock, &incorrect, sizeof(incorrect), 0)) <= 0) {
										perror("Send");
									}
								}
							}
							else {
								present = 0;
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
