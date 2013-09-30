#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main (int argc, char **argv) {
	int i = 0;
	int cllen = 0;
	int dalen = 0;
	int inlen = 0;
	int breakout = 1;	
	int status = 0;	
	int lport = 34789;
	int clientmax = 0;

	char *cldata;
	
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
		perror("Bind Failed");
	}
	if ((status = listen(s, 10)) == 0) {
		printf("Liste‌ning\n");
	}
	else {
		perror("Listen Failed");
	}

	FD_SET(s, &lisocks);
	clientmax = s;
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
					}
				}
				else {
					if (dalen = recv(i, cldata, dalen, 0)) {
						printf("%s", cldata);
					}
					else {
					}	
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
