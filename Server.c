/*
Aaron Burger
October 2, 2013
Computer Networks
Columbia University

Server.c

This source describes the web server we have been
requested to create via class specifications. It
can perform the whoelse and wholasthr commands to
search for relevent clients, and the broadcast
command to send a message to clients that are currently connected

The order of the loops and help with list control was based in part
off an online tutorial that can be found here:
http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
All content and operation of the server are original.
*/


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
	signal(SIGPIPE, SIG_IGN); // supression prevention control
	setvbuf(stdout, NULL, _IONBF, 0); // printf flushing control

	// Constant and variable initilization

	int i = 0; // Iterators
	int j = 0;
	int inx = 0;
	int cllen = 0; // Socket data lengths
	int dalen = 0;
	int inlen = 0;
	int breakout = 1; // Control flow
	int present = 0;
	int prevent = 0;	
	int status = 0;	
	int lport = 34789; // Port information
	int clientmax = 0;

	char *search = " "; // String pointer initiaization
	char *end = "\0";
	char *user;
	char *pass;

	char welcome[1] = "0"; // Reference constant initilization
	char incorrect[1] = "1";
	char lock[1] = "2";
	char input[1] = "3";
	char broad[1] = "4";
	char disc[1] = "5";
	char who[2] = "6\0";
	char hr[2] = "7\0";
	char bc[2] = "8\0";

	char cldata[100]; // Large array initilization
	char bmsg[100];
	char *allauths = malloc(1000*sizeof(char));
	char *allwho = malloc(1000*sizeof(char));
	char *allhr = malloc(1000*sizeof(char));
	char *auth[USERS][2];
	long long timestamps[USERS][3];
	long long iplocks[USERS][2];
	
	struct sockaddr_in saddr; // Socket structure initialization
	struct sockaddr *uaddr = (struct sockaddr *) &saddr;
	struct sockaddr_in claddrin;
	struct sockaddr_storage claddr;
	struct sockaddr *ucladdr = (struct sockaddr *) &claddr;
	struct in_addr addr;

	fd_set lisocks; // Socket list initilization
	fd_set clients;
	FD_ZERO(&lisocks);
	FD_ZERO(&clients);

	memset(bmsg, '\0', sizeof(cldata[0]) * 100); // Memory setup for large arrays
	memset(cldata, '\0', sizeof(cldata[0]) * 100);
	memset(timestamps, 0, sizeof(timestamps[0][0]) * USERS * 3);
	memset(iplocks, 0, sizeof(iplocks[0][0]) * USERS * 2);

	for (i=0; i<USERS; i++)
		for (j=0; j<2; j++)
			auth[i][j] = malloc(100*sizeof(char));

	strcpy(allauths, "3"); // Hard coded initilizations
	bmsg[0] = broad[0];

	FILE* myfile=NULL; // Read login credentials from file
	myfile=fopen("Credentials.txt","r");
	for(i=0; i<USERS; i++) {
		fscanf(myfile, "%[^ \n] %[^ \n]\n", auth[i][0], auth[i][1]);
		strcat(allauths, auth[i][0]);
		strcat(allauths, "-");
		strcat(allauths, auth[i][1]);
		strcat(allauths, "\n");
	}
	fclose(myfile);

	if (argc == 2) { // Read port as an argument from command line, if desired
		lport = atoi(argv[1]);
	}

	saddr.sin_family = AF_INET; // Socket initialization
	saddr.sin_port = lport;
	saddr.sin_addr.s_addr = INADDR_ANY;
	int addrsize = sizeof(saddr);

	int s = socket(PF_INET, SOCK_STREAM, 0);
	int clsock;
	
	if ((status = bind(s, uaddr, addrsize)) == 0) { // Binding step
		printf("Socket Bound\n");
	}
	else {
		perror("Bind Failed -- Try Again");
	}
	if ((status = listen(s, 10)) == 0) { // Listening step
		printf("Liste‌ning\n\n");
	}
	else {
		perror("Listen Failed\n");
	}

	FD_SET(STDIN, &lisocks); // Add default socket and STDIN to list
	FD_SET(s, &lisocks);

	clientmax = s;

	printf("Please enter a command: ");

	while (breakout == 1) { // Main control loop
		clients = lisocks; // Load in all sockets to the selector
		if (select(clientmax+1, &clients, NULL, NULL, NULL) == -1) { // Select step
			perror("Select Failed");
		}

		for (i=0; i<=clientmax; i++) { // For all possible sockets	
			if (FD_ISSET(i, &clients)) { // If the socket is writing
				if (i == s) { // If the default socket is being written to, there is a new client
					cllen = sizeof(claddr);
					clsock = accept(s, ucladdr, &cllen);
					if (clsock == -1) {
						perror("Couldn't Accept");
					}
					else { // Add new client to relevent lists
						FD_SET(clsock, &lisocks);
						if (clsock > clientmax) clientmax = clsock;
						if ((status = send(clsock, &welcome, sizeof(welcome), 0)) <= 0) {
							perror("Send");
						}
					}
				}
				else if (i == STDIN) { // If the socket is STDIN, take input from user
					if ((dalen = read(i, cldata, 100*sizeof(char))) > 0) {
						cldata[strlen(cldata)-1] = '\0';
						if (!strcmp(cldata, "whoelse")) { // Whoelse command shows currently connected users
							for (j=0; j<USERS; j++) {
       		 						addr.s_addr = timestamps[j][2];
								if (timestamps[j][0] != 0) {
									printf("%s - %lld - %s\n", auth[j][0], timestamps[j][1], inet_ntoa(addr));
									present = 1;
								}
							}
							printf("\n");
							if (present) {
								present = 0;
							}
							else {
								printf("No users are here\n\n");
							}
						}
						else if (!strcmp(cldata, "wholasthr")) { // Wholasthr does that for the last hour
							for (j=0; j<USERS; j++) {
       		 						addr.s_addr = timestamps[j][2];
	
								if (timestamps[j][1] > time(NULL) - 3600 || timestamps[j][0] != 0) {
									printf("%s - %lld - %s\n", auth[j][0], timestamps[j][1], inet_ntoa(addr));
									present = 1;
								}
							}
							printf("\n");
							if (present) {
								present = 0;
							}
							else {
								printf("No users were here\n\n");
							}
						}
						else if (!strcmp(cldata, "quit")) { // Quit breaks out of the control loop
							breakout = 0;
						}
						else if (strcmp(cldata, "") != 0 && strcmp(cldata, "broadcast") != 0 && !strcmp(strtok(cldata, search), "broadcast")) {
							strcat(bmsg, strtok(NULL, end)); // The broadcast string is formed to be sent to all clients

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
						else if (strcmp(cldata, "broadcast") == 0) { // Broadcast error control
							printf("Must broadcast some message\n\n");
						}
						else { // Step for invalid commands
							printf("%s is not a valid command\n\n", cldata);
						}
						if (breakout) printf("Please enter a command: "); // Don't prompt if no more commands
						memset(cldata, '\0', 100);
					}
				}
				else { // If a client is writing to the server on its socket
					if ((status = recv(i, cldata, sizeof(cldata), 0)) <= 0) { // If there is no data, discontinue the client
						close(i);
                        			FD_CLR(i, &lisocks);
					}
					else { // If there is data, interpret it
						prevent = 0;
						getsockname(i, (struct sockaddr *)&claddrin, &cllen);
						for (j=0; j<inx; j++) {
							if (inet_addr(inet_ntoa(claddrin.sin_addr)) == iplocks[j][0] && time(NULL) < iplocks[j][1]) {
								prevent = 1;
								break;
							}
						}

						if (cldata[0] == welcome[0] && prevent == 0) { // If it is login data, verify credentials. Can't be a blocked IP
							user = strtok(cldata, search)+2;
							pass = strtok(NULL, search);

							for (j=0; j<USERS; j++) {
								if (!strcmp(auth[j][0], user) && !strcmp(auth[j][1], pass)) {
									timestamps[j][0] += 1;
									timestamps[j][1] = time(NULL);
									timestamps[j][2] = inet_addr(inet_ntoa(claddrin.sin_addr));
									present = 1;

									if ((status = send(i, &input, sizeof(input), 0)) <= 0) {
										perror("Send");
									}
									break;
								}
							}

							if (present == 0) { // If login unsuccessful
								if (cldata[1] >= '2') { // Three attempts means IP Block
									if ((status = send(i, &lock, sizeof(lock), 0)) <= 0) {
										perror("Send");
									}
									iplocks[inx][0] = inet_addr(inet_ntoa(claddrin.sin_addr));
									iplocks[inx][1] = time(NULL) + 60;
									inx++;
								}
								else { // Ask for credentials again otherwise
									if ((status = send(i, &incorrect, sizeof(incorrect), 0)) <= 0) {
										perror("Send");
									}
								}
							}
							else {
								present = 0;
							}
						}
						else if (prevent == 1) { // Send a lock message if locked, don't lock again
							if ((status = send(i, &lock, sizeof(lock), 0)) <= 0) {
								perror("Send");
							}
						}
						else if (cldata[0] == input[0]) { // If the authentication payload is requested, send it
							if ((status = send(i, allauths, 1000*sizeof(char), 0)) <= 0) {
								perror("Send");
							}
						}
						
						// *********
						else if (cldata[0] == who[0]) { // Returning the whoelse command
							sprintf(allwho, "%s", who);
							for (j=0; j<USERS; j++) {
		 						addr.s_addr = timestamps[j][2];
								if (timestamps[j][0] != 0) {
									sprintf(allwho + strlen(allwho), "%s - %lld - %s\n", auth[j][0], timestamps[j][1], inet_ntoa(addr));
									present = 1;
								}
							}
							if (present) {
								present = 0;
							}
							else {
								sprintf(allwho + strlen(allwho), "No users are here\n\n");
							}

							if ((status = send(i, allwho, 1000*sizeof(char), 0)) <= 0) {
								perror("Send");
							}
						}
						else if (cldata[0] == hr[0]) { // Returning the wholasthr command
							sprintf(allhr, "%s", hr);
							for (j=0; j<USERS; j++) {
       		 						addr.s_addr = timestamps[j][2];
	
								if (timestamps[j][1] > time(NULL) - 3600 || timestamps[j][0] != 0) {
									sprintf(allhr + strlen(allhr), "%s - %lld - %s\n", auth[j][0], timestamps[j][1], inet_ntoa(addr));
									present = 1;
								}
							}
							if (present) {
								present = 0;
							}
							else {
								sprintf(allhr + strlen(allhr), "No users were here\n\n");
							}

							if ((status = send(i, allhr, 1000*sizeof(char), 0)) <= 0) {
								perror("Send");
							}
						}
						else if (cldata[0] == bc[0]) {
							for (j=0; j<=clientmax; j++) {			
								if (FD_ISSET(j, &lisocks)) {
									if (j != s && j != STDIN) {
										if ((status = send(j, &cldata, sizeof(cldata), 0)) <= 0) {
											perror("Send");
										}
									}
								}
							}
						}

						else if (cldata[0] == disc[0]) { // If a disconnect is sent, remove the client
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

	if ((status = close(s)) == 0) { // Close the socket
		printf("Socket Closed\n");
	}
	else {
		perror("Close Failed");
	}

	return 0;
}
