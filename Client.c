#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define STDIN 0

int main (int argc, char **argv) {
	int status = 0;
	int breakout = 1;
	int lport = 34789;
	
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

	char d[100];

	while (breakout == 1) {
		breakout = read(0, d, sizeof(d));
		if (status = send(s, d, sizeof(d), 0) > 0) {
			perror ("Sent");
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
