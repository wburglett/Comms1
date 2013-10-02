#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>

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
	int timestamps[USERS][2];
	
	memset(cldata, '\0', sizeof(cldata[0]) * 100);
	memset(auth, '\0', sizeof(auth[0][0]) * 10 * 2);
	memset(timestamps, 0, sizeof(timestamps[0][0]) * 10 * 2);

	auth[0][0] = "Columbia";
	auth[0][1] = "116bway";
	auth[1][0] = "SEAS";
	auth[1][1] = "summerisover";
	auth[2][0] = "csee4119";
	auth[2][1] = "lotsofexams";
	auth[3][0] = "foobar";
	auth[3][1] = "passpass";
	auth[4][0] = "windows";
	auth[4][1] = "withglass";
	auth[5][0] = "Google";
	auth[5][1] = "hasglasses";
	auth[6][0] = "facebook";
	auth[6][1] = "wastingtime";
	auth[7][0] = "wikipedia";
	auth[7][1] = "donation";
	auth[8][0] = "network";
	auth[8][1] = "seemsez";

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
							for (j=0; j<=USERS; j++) {		
								if (timestamps[j][0] == 1) {
									printf("%s, ", auth[j][0]);
									present = 1;
								}
								if (present) {
									printf("are here\n\n");
									present = 0;
								}
								else {
									printf("No users are here\n\n");
								} 
							}
						}
						else if (!strcmp(cldata, "wholasthr")) {
							printf("wholasthring\n\n");
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
						
					}
					else {
						if (cldata[0] == '0') {
							user = strtok(cldata, search);
							pass = strtok(NULL, search);

							printf("%s--%s\n", user+2, pass);

							for (j=0; j<=USERS; j++) {		
								if (auth[j][0] == "hi") {
									
								}
							}

							if (cldata[1] >= 2) {
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
