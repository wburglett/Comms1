#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>

#define STDIN 0

int main (int argc, char **argv) {
	setvbuf(stdout, NULL, _IONBF, 0);
	
	int i=0;
	int status = 0;
	int control = -1;
	int breakout = 1;
	int lport = 34789;
	int tries = 0;

	long long laddr = inet_addr("127.0.0.1");

	char welcome[1] = "0";
	char incorrect[1] = "1";
	char lock[1] = "2";
	char input[1] = "3";
	char broad[1] = "4";
	char disc[1] = "5";
	
	char auth[100];
	char indata[1000];
	char clin[100];

	fd_set servers;
	fd_set backup;
	FD_ZERO(&servers);
	FD_ZERO(&backup);

	memset(auth, '\0', sizeof(auth[0]) * 100);
	memset(indata, '\0', sizeof(indata[0]) * 100);
	memset(clin, '\0', sizeof(clin[0]) * 100);

	if (argc == 3) {
		laddr = inet_addr(argv[1]);
		lport = atoi(argv[2]);
	}

	struct sockaddr_in saddr;
	struct sockaddr *uaddr = (struct sockaddr *) &saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = lport;
	saddr.sin_addr.s_addr = INADDR_ANY;
	int addrsize = sizeof(saddr);

	int s = socket(PF_INET, SOCK_STREAM, 0);
	
	FD_SET(STDIN, &servers);
	FD_SET(s, &servers);
	FD_SET(STDIN, &backup);
	FD_SET(s, &backup);

	if ((status = connect(s, uaddr, addrsize)) == 0) {
		printf("Connection established, please log in\n");
	}
	else {
		perror("Connection Failed");
	}

	while (breakout == 1) {
		if (status = recv(s, &indata, sizeof(indata), 0) <= 0) {
			perror ("On data receive");
		}

		if (indata[0] == welcome[0]) {
			memset(auth, '\0', sizeof(auth[0]) * 100);
			auth[0] = auth[1] = '0';

			printf("Username: ");
			read(0, clin, sizeof(clin));
			clin[strcspn(clin, "\n")] = '\0';
			strcat(auth, clin);
			strcat(auth, " ");

			printf("Password: ");
			read(0, clin, sizeof(clin));
			clin[strcspn(clin, "\n")] = '\0';
			strcat(auth, clin);

			if (status = send(s, &auth, sizeof(auth), 0) <= 0) {
				perror ("On auth send");
			}
			tries++;
		}
		else if (indata[0] == incorrect[0]) {
			memset(auth, '\0', sizeof(auth[0]) * 100);
			printf("\nBad credentials. Please try again\n");
			auth[0] = '0';
			auth[1] = 48 + tries;

			printf("Username: ");
			read(0, clin, sizeof(clin));
			clin[strcspn(clin, "\n")] = '\0';
			strcat(auth, clin);
			strcat(auth, " ");

			printf("Password: ");
			read(0, clin, sizeof(clin));
			clin[strcspn(clin, "\n")] = '\0';
			strcat(auth, clin);

			if (status = send(s, &auth, sizeof(auth), 0) <= 0) {
				perror ("On auth send");
			}
			tries++;
		}
		else if (indata[0] == lock[0]) {
			memset(auth, '\0', sizeof(auth[0]) * 100);
			printf("\nLogin locked. Please wait one minute before trying again\n");
			tries = 0;

			auth[0] = '0';
			auth[1] = 48 + tries;

			printf("Username: ");
			read(0, clin, sizeof(clin));
			clin[strcspn(clin, "\n")] = '\0';
			strcat(auth, clin);
			strcat(auth, " ");

			printf("Password: ");
			read(0, clin, sizeof(clin));
			clin[strcspn(clin, "\n")] = '\0';
			strcat(auth, clin);

			if (status = send(s, &auth, sizeof(auth), 0) <= 0) {
				perror ("On auth send");
			}
			tries++;
		}
		else if (indata[0] == input[0]) {
			if (tries > 0) {
				printf("\nLogin Successful! Welcome to the Burger Server\n");
				tries = 0;
			}
			printf("Input command: ");

			while (breakout == 1) {
				servers = backup;
				if (select(s+1, &servers, NULL, NULL, NULL) == -1) {
					perror("Select Failed");
					breakout = 0;
				}
				for (i=0; i<=s; i++) {
					if (FD_ISSET(i, &servers)) {
						if (i == STDIN) {
							if (read(i, clin, sizeof(clin)) > 0) {
								clin[strcspn(clin, "\n")] = '\0';
								
								if (!strcmp(clin, "credentials")) {
									if (status = send(s, &input, sizeof(input), 0) <= 0) {
										perror ("On cred request send");
									}
									memset(clin, '\0', sizeof(clin[0]) * 100);
								}
								else if (!strcmp(clin, "quit")) {
									breakout = 0;
									strcat(disc, auth);

									if (status = send(s, &disc, 100*sizeof(char), 0) <= 0) {
										perror ("On disc send");
									}
								}
								else {
									printf("%s is not a valid command\n\nInput command: ", clin);
								}
							}
						}
						else {
							if (status = recv(i, indata, sizeof(indata), 0) <= 0) {
								perror ("Forced disconnect");
								breakout = 0;
							}
							else if (indata[0] == broad[0]) {
								printf("-- SERVER BROADCAST: %s --\n", indata+1);
								printf("Input command: ");
							}
							else if (indata[0] == input[0]) {
								printf("\nUsername - Password Combinations\n%s\n", indata+1);
								printf("Input command: ");
							}
						}
					}
				}
			}
		}
		else {
			printf("The server has garbled a command\n");
			breakout = 0;
		}
	}

	if ((status = close(s)) == 0) {
		printf("Connection Closed\n");
	}
	else {
		perror("Close Failed");
	}

	return 0;
}
