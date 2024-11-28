#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 9000
#define HOST "127.0.0.1"

int intializeClientSocket() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in srv_addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = inet_addr(HOST),
		.sin_port = htons(PORT)
	};

	connect(sockfd, (struct sockaddr*)&srv_addr, sizeof(struct sockaddr_in));

	return sockfd;
}

int main() {
	setvbuf(stdin, NULL, _IOLBF, BUFSIZ);
	setvbuf(stdout, NULL, _IOLBF, BUFSIZ);

	int srv = intializeClientSocket();

	char recvBuf[BUFSIZ];

	// "Connection established"
	read(srv, recvBuf, sizeof(recvBuf));
	printf("%s", recvBuf);

	while (1) {
		if (read(srv, recvBuf, sizeof(recvBuf)) == 0) {
			printf("Server has closed the connection.\n");
			break;
		}
		printf("%s", recvBuf);
		fflush(stdout);

		// send stdin to server
		char inputBuf[BUFSIZ];
		read(STDIN_FILENO, inputBuf, sizeof(inputBuf));
		write(srv, inputBuf, strlen(inputBuf));
	}

	// Close connection
	shutdown(srv, SHUT_RDWR);
	close(srv);
	printf("Client Exit\n");

	return 0;
}
