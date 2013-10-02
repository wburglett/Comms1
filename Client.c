#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>

#define STDIN 0

int main (int argc, char **argv) {
	setvbuf(stdout, NULL, _IONBF, 0);
	
	int status = 0;
	int control = -1;
	int breakout = 1;
	int lport = 34789;
	int tries = 0;

	char welcome[1] = "0";
	char incorrect[1] = "1";
	char lock[1] = "2";
	char input[1] = "3";
	char broad[1] = "5";
	
	char auth[100];
	char indata[100];
	char clin[100];

	memset(auth, '\0', sizeof(auth[0]) * 100);
	memset(indata, '\0', sizeof(indata[0]) * 100);
	memset(clin, '\0', sizeof(clin[0]) * 100);

	if (argc == 2) {
		lport = atoi(argv[1]);
	}

	struct sockaddr_in saddr;
	struct sockaddr *uaddr = (struct sockaddr *) &saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = lport;
	saddr.sin_addr.s_addr = INADDR_ANY;
	int addrsize = sizeof(saddr);

	int s = socket(PF_INET, SOCK_STREAM, 0);
	
	if ((status = connect(s, uaddr, addrsize)) == 0) {
		printf("Connection Made\n");
	}
	else {
		perror("Connection Failed");
	}

	while (breakout == 1) {
		if (status = recv(s, &indata, sizeof(indata), 0) <= 0) {
			perror ("On data receive");
		}

		if (indata[0] == welcome[0]) {
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
			memset(auth, '\0', sizeof(auth[0]) * 100);
		}
		else if (indata[0] == incorrect[0]) {
			printf("Bad Login\n");
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
			memset(auth, '\0', sizeof(auth[0]) * 100);
		}
		else if (indata[0] == lock[0]) {
			printf("Login Locked\n");
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
			memset(auth, '\0', sizeof(auth[0]) * 100);
		}
		else if (indata[0] == input[0]) {
			printf("Data input\n");
		}
		else if (indata[0] == broad[0]) {
			printf("Receiving data\n");
		}
		else {
			printf("The server has garbled a command\n");
			breakout = 0;
		}

		perror ("Recv");
		printf("%d %d\n", status, indata[0]);
	}

	if ((status = close(s)) == 0) {
		printf("Socket Closed\n");
	}
	else {
		perror("Close Failed");
	}

	return 0;
}
