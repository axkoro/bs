#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "protocol.h"

#define PORT 9007
#define HOST "127.0.0.1"

int intializeClientSocket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in srv_addr = {
        .sin_family = AF_INET, .sin_addr.s_addr = inet_addr(HOST), .sin_port = htons(PORT)};

    connect(sockfd, (struct sockaddr*)&srv_addr, sizeof(struct sockaddr_in));

    return sockfd;
}

int main() {
    setvbuf(stdin, NULL, _IOLBF, BUFSIZ);

    int srv = intializeClientSocket();

    while (1) {
        char msg_type;

        if (read(srv, &msg_type, sizeof(msg_type)) == 0) {
            printf("Server has closed the connection.\n");
            break;
        }

        printf("msg-type: %d\n", (short)msg_type);
        fflush(stdout);

        char recv_buf[BUFSIZ];
        if (msg_type == MSG_OUTPUT) {
            read(srv, recv_buf, sizeof(recv_buf));
            printf("%s", recv_buf);
            fflush(stdout);
        } else if (msg_type == MSG_PROMPT) {
            read(srv, recv_buf, sizeof(recv_buf));
            printf("%s", recv_buf);  // print prompt
            fflush(stdout);

            // read user input
            char input_buf[BUFSIZ];
            read(STDIN_FILENO, input_buf, sizeof(input_buf));

            char* command = strtok(input_buf, " \n");

            if (!strcmp(command, "put")) {
                input_buf[strlen(command)] = ' ';  // remove NULL-character set by strok
                write(srv, input_buf, strlen(input_buf));

                read(srv, &msg_type, sizeof(msg_type));

                if (msg_type == PUT_COMMENCE) {
                    char* file_path = strtok(NULL, " \n");
                    int file = open(file_path, O_RDONLY);
                    if (file < 0) printf("put: Couldn't open file");

                    struct stat file_stats;
                    fstat(file, &file_stats);
                    size_t size = (size_t)file_stats.st_size;
                    write(srv, &size, sizeof(off_t));

                    char file_buf[BUFSIZ];
                    int n_bytes = 0;
                    do {
                        int n_bytes = read(file, file_buf, sizeof(file_buf));
                        write(srv, file_buf, n_bytes);
                    } while (n_bytes > 0);

                    close(file);
                } else {
                    printf("put: Server didn't accept");
                }

            } else if (!strcmp(command, "get")) {
                input_buf[strlen(command)] = ' ';  // remove NULL-character set by strok
                write(srv, input_buf, strlen(input_buf));

                // wait for response?
                // read file size
                // open file (according to size?)
                // read file contents into file
            } else {
                input_buf[strlen(command)] = ' ';  // remove NULL-character set by strok
                write(srv, input_buf, strlen(input_buf));
            }
        }
    }

    // Close connection
    shutdown(srv, SHUT_RDWR);
    close(srv);
    printf("Client Exit\n");

    return 0;
}
