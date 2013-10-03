/*
Aaron Burger
October 2, 2013
Computer Networks
Columbia University

Client.c

This source describes the web client we have been
requested to create via class specifications. It
can perform the 'credentials' command to grab credential
payload from the server.
*/

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>

#define STDIN 0

int main (int argc, char **argv) {
	setvbuf(stdout, NULL, _IONBF, 0); // printf flushing control
	
	// Constant and variable initilization

	int i=0; // Iterators
	int status = 0;
	int control = -1;
	int breakout = 1;
	int lport = 34789; // Port defaults
	int tries = 0;

	long long laddr = inet_addr("127.0.0.1"); // Default IP address

	char welcome[1] = "0"; // Reference constant initilization
	char incorrect[1] = "1";
	char lock[1] = "2";
	char input[1] = "3";
	char broad[1] = "4";
	char disc[2] = "5\0";
	char who[2] = "6\0";
	char hr[2] = "7\0";
	char bc[2] = "8\0";

	char *search = " "; // String pointer initiaization
	char *end = "\0";
	
	char auth[100]; // Large array initilization
	char indata[1000];
	char clin[100];
	char bmsg[100];

	fd_set servers; // Socked list initilization
	fd_set backup;
	FD_ZERO(&servers);
	FD_ZERO(&backup);

	memset(auth, '\0', sizeof(auth[0]) * 100); // Memory setup for large arrays
	memset(indata, '\0', sizeof(indata[0]) * 100);
	memset(clin, '\0', sizeof(clin[0]) * 100);
	memset(bmsg, '\0', sizeof(clin[0]) * 100);

	if (argc == 3) {
		laddr = inet_addr(argv[1]);
		lport = atoi(argv[2]);
	}

	struct sockaddr_in saddr; // Socket structure initialization
	struct sockaddr *uaddr = (struct sockaddr *) &saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = lport;
	saddr.sin_addr.s_addr = INADDR_ANY;
	int addrsize = sizeof(saddr);

	int s = socket(PF_INET, SOCK_STREAM, 0);
	
	FD_SET(STDIN, &servers); // Add default socket and STDIN to list
	FD_SET(s, &servers);
	FD_SET(STDIN, &backup);
	FD_SET(s, &backup);

	if ((status = connect(s, uaddr, addrsize)) == 0) {
		printf("Connection established, please log in\n");
	}
	else {
		perror("Connection Failed");
	}

	while (breakout == 1) { // Main control loop
		if (status = recv(s, &indata, sizeof(indata), 0) <= 0) { // If a receive error occurs, report it but don't quit
			perror ("On data receive");
		}

		if (indata[0] == welcome[0]) { // If the server is ready for authentication
			memset(auth, '\0', sizeof(auth[0]) * 100);
			auth[0] = auth[1] = '0';

			printf("Username: "); // Present username
			read(0, clin, sizeof(clin));
			clin[strcspn(clin, "\n")] = '\0';
			strcat(auth, clin);
			strcat(auth, " ");

			printf("Password: "); // Present password
			read(0, clin, sizeof(clin));
			clin[strcspn(clin, "\n")] = '\0';
			strcat(auth, clin); // Merge the two into an auth token string

			if (status = send(s, &auth, sizeof(auth), 0) <= 0) {
				perror ("On auth send");
			}
			tries++;
		}
		else if (indata[0] == incorrect[0]) { // If the server says the credentials are invalid
			memset(auth, '\0', sizeof(auth[0]) * 100);
			printf("\nBad credentials. Please try again\n");
			auth[0] = '0';
			auth[1] = 48 + tries;

			printf("Username: "); // Get new credentials and try again
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
		else if (indata[0] == lock[0]) { // Do the same if the server is locked, but tell users to wait one minute
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
		else if (indata[0] == input[0]) { // If the server is ready for command input
			if (tries > 0) {
				printf("\nLogin Successful! Welcome to the Burger Server\n"); // Present welcome message
				tries = 0;
			}
			printf("Input command: ");

			while (breakout == 1) { // Enter command input stage
				servers = backup;
				if (select(s+1, &servers, NULL, NULL, NULL) == -1) {
					perror("Select Failed");
					breakout = 0;
				}
				for (i=0; i<=s; i++) { // For the two relavent sockets (server connection and STDIN)
					if (FD_ISSET(i, &servers)) {
						if (i == STDIN) { // If there is user input
							if (read(i, clin, sizeof(clin)) > 0) {
								clin[strcspn(clin, "\n")] = '\0';
								
								if (!strcmp(clin, "credentials")) { // Credentials asks for the credentials payload
									if (status = send(s, &input, sizeof(input), 0) <= 0) {
										perror ("On cred request send");
									}
									memset(clin, '\0', sizeof(clin[0]) * 100);
								}

								else if (!strcmp(clin, "whoelse")) { // ***
									if (status = send(s, &who, sizeof(who), 0) <= 0) {
										perror ("On who request send");
									}
									memset(clin, '\0', sizeof(clin[0]) * 100);
								}
								else if (!strcmp(clin, "wholasthr")) { // ***
									if (status = send(s, &hr, sizeof(hr), 0) <= 0) {
										perror ("On hr request send");
									}
									memset(clin, '\0', sizeof(clin[0]) * 100);
								}

								else if (!strcmp(clin, "quit")) { // Quit disconnects from the server
									breakout = 0;
									strcat(disc, auth);

									if (status = send(s, &disc, 100*sizeof(char), 0) <= 0) {
										perror ("On disc send");
									}
								}

								else if (strcmp(clin, "") != 0 && strcmp(clin, "broadcast") != 0 && !strcmp(strtok(clin, search), "broadcast")) {
									bmsg[0] = bc[0];
									strcat(bmsg, strtok(NULL, end)); // The broadcast string is formed to be sent to all clients
									if (status = send(s, &bmsg, sizeof(bmsg), 0) <= 0) {
										perror ("On bc request send");
									}
									printf("Broadcast Complete\n\n");
									memset(clin, '\0', sizeof(clin[0]) * 100);
									memset(bmsg, '\0', sizeof(bmsg[0]) * 100);
								}
								else if (strcmp(clin, "broadcast") == 0) { // Broadcast error control
									printf("Must broadcast some message\n\n");
								}
								else { // All other commands are invalid
									printf("%s is not a valid command\n\nInput command: ", clin);
								}
							}
						}
						else { // If the server is sending a message
							if (status = recv(i, indata, sizeof(indata), 0) <= 0) { // It could be offline, so disconnect
								perror ("Forced disconnect");
								breakout = 0;
							}
							else if (indata[0] == broad[0]) { // It could be presenting a server broadcast, print it
								printf("-- SERVER BROADCAST: %s --\n", indata+1);
								printf("Input command: ");
							}
							else if (indata[0] == bc[0]) { // It could be presenting a client broadcast, print it
								printf("-- CLIENT BROADCAST: %s --\n", indata+1);
								printf("Input command: ");
							}
							else if (indata[0] == input[0]) { // It could be presenting authentications, print them
								printf("\nUsername - Password Combinations\n%s\n", indata+1);
								printf("Input command: ");
							}
							else if (indata[0] == who[0] || indata[0] == hr[0]) {
								printf("%s\n", indata+1);
								printf("Input command: ");
							}
						}
					}
				}
			}
		}
		else { // If the server is force quit at some points in operation, this may happen. Forces disconnect
			printf("The server has garbled a command\n");
			breakout = 0;
		}
	}

	if ((status = close(s)) == 0) { // Close socket
		printf("Connection Closed\n");
	}
	else {
		perror("Close Failed");
	}

	return 0;
}
