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

		// read user input
		char input_buf[BUFSIZ];
		read(STDIN_FILENO, input_buf, sizeof(input_buf));

		char* command = strtok(input_buf, " \n");
		char* filename = strtok(NULL, " \n");

		if (!strcmp(command, "put")) {
			// send command
			// wait for response?
			// open file
			// send file size
			// send file contents
		} else if (!strcmp(command, "get")) {
			// send command
			// wait for response?
			// read file size
			// open file (according to size?)
			// read file contents into file
		} else {
			write(srv, input_buf, strlen(input_buf));
		}
	}

	// Close connection
	shutdown(srv, SHUT_RDWR);
	close(srv);
	printf("Client Exit\n");

	return 0;
}
