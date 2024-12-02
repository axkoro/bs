#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "protocol.h"

#define PORT 9000

#define PATH_MAX 4096
#define ARGS_MAX 8

int initializeServerSocket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int sockopt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&sockopt,
               sizeof(sockopt));  // enable immediate reuse of this port after closing the socket
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&sockopt,
               sizeof(sockopt));  // disable delayed tcp segments

    struct sockaddr_in srv_addr = {
        .sin_family = AF_INET, .sin_port = htons(PORT), .sin_addr.s_addr = INADDR_ANY};

    if (bind(sockfd, (struct sockaddr*)&srv_addr, sizeof(struct sockaddr_in)) < 0) {
        printf("Couldn't bind to address");
        if (errno == EADDRINUSE) printf(": Adress already in use");
        fflush(stdout);

        exit(-1);
    }

    listen(sockfd, 0);

    return sockfd;
}

void writePrompt(int fd) {
    char* cwd = getcwd(NULL, 0);

    char prompt[strlen(cwd) + 4];  // + msg_type + '>' + ' ' + string terminator
    snprintf(prompt, sizeof(prompt), "%c%s> ", MSG_PROMPT,
             cwd);  // prefix prompt with message type indicator

    write(fd, prompt, strlen(prompt) + 1);  // +1 to full null-terminated string

    free(cwd);
}

void writeOutput(int fd, char* msg) {
    char prefixed_msg[strlen(msg) + 2];  // + msg_type + string terminator
    snprintf(prefixed_msg, sizeof(prefixed_msg), "%c%s", MSG_OUTPUT,
             msg);  // prefix msg with message type indicator

    write(fd, prefixed_msg, sizeof(prefixed_msg) + 1);
}

/**
 * Reads a command from the given file descriptor and parses its arguments.
 *
 * @param fd   The file descriptor to read the command from.
 * @param buf  A buffer to store the command read from the file descriptor.
 * @param args An array to store the parsed arguments. args[0] holds the command itself and the last
 * value is NULL (see argv[] for exec() in POSIX).
 * @return     The number of arguments parsed.
 */
int readCommand(int fd, char* buf, size_t buf_len, char* args[], size_t args_len) {
    read(fd, buf, buf_len);

    args[0] = strtok(buf, " \n");

    char* arg;
    int i = 1;
    while ((arg = strtok(NULL, " \n")) != NULL) {
        args[i++] = arg;
    }
    args[i] = NULL;

    int argc = i - 1;
    return argc;
}
void handleClient(void *arg){
	int cli = *(int*)arg;
	free(arg);
	char msg[] = "Connection established!\n";
	writeOutput(cli, msg);

    // Shell
    while (1) {
        writePrompt(cli);

        char command_buf[BUFSIZ];
        char* args[ARGS_MAX];
        readCommand(cli, command_buf, sizeof(command_buf), args, sizeof(args));
        char* command = args[0];

        if (!strcmp(command, "exit")) {
            shutdown(cli, SHUT_RDWR);
            close(cli);
            exit(0);
        } else if (!strcmp(command, "cd")) {
            char path[PATH_MAX];

            if (args[1][0] == '/') {  // absolute path
                snprintf(path, sizeof(path), "%s", args[1]);
            } else {  // relative path
                char* cwd = getcwd(NULL, 0);
                snprintf(path, sizeof(path), "%s/%s", cwd, args[1]);
                free(cwd);
            }

            if (chdir(path) != 0) perror("chdir failed");
        } else if (!strcmp(command, "get")) {  // TODO:

        } else if (!strcmp(command, "put")) {  // TODO:
                                               // open file (if nonexistent)
                                               // confirm ready to receive
                                               // read from socket
        } else {                               // execute file
            int monitored_process = fork();
            if (monitored_process == 0) {
                execvp(command, args);
                exit(-1);
            } else {
                int status;
                waitpid(monitored_process, &status, 0);
            }
        }
    }

    // Close connection
    shutdown(cli, SHUT_RDWR);
    close(cli);

	pthread_exit(NULL);	
	
}

int main() {
	int sockfd = initializeServerSocket();
while(1){
	struct sockaddr_in cli_addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int cli = accept(sockfd, (struct sockaddr*)&cli_addr, &addr_len);
	int* pcli = malloc(sizeof(int));
	*pcli = cli;
	pthread_t tid;
	pthread_create(&tid, NULL, handleClient, pcli);
	pthread_detach(tid);

}
close(sockfd);
	return 0;
}
