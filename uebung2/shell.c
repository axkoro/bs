#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "vec.h"

#define PATH_MAX 4096

int cd_sh() {
	return 0;
}

int wait_sh() {
	return 0;
}

int exit_sh() {
	// TODO: Graceful exit: Alle Ressourcen freigeben
	exit(0);
}

int main(void) {
	signal(SIGINT, SIG_IGN);

	setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
	setvbuf(stdin, NULL, _IOLBF, BUFSIZ);

	while (1) {
		// Prompt
		char prompt[PATH_MAX + 3];
		char cwd[PATH_MAX];
		getcwd(cwd, sizeof(cwd));
		snprintf(prompt, sizeof(prompt), "%s> ", cwd);
		write(STDOUT_FILENO, prompt, strlen(prompt));

		// Read input
		char input_buf[BUFSIZ];
		read(STDIN_FILENO, input_buf, BUFSIZ); // wait until line break

		// Process input (max 1 argument)
		char* command = strtok(input_buf, " \n");
		char* arg = strtok(NULL, " \n");
		// printf("command: %s\narg: %s\n", command, arg);
		// TODO: clear after exec

		if (strcmp(command, "cd") == 0) {
			char path[PATH_MAX];
			snprintf(path, sizeof(path), "%s/%s", cwd, arg);

			if (chdir(path) != 0) {
				perror("chdir failed");
			}

			// TODO: handle . and ..
		} else if (strcmp(command, "wait") == 0) { // TODO: sobald Programmausführung und unäres & abschlossen (ermöglicht Test)
			pid_t pid = atoi(arg);
			int status;
			if (waitpid(pid, &status, 0)) {
				perror("wait");
			}
		} else if (strcmp(command, "exit") == 0) {
			exit_sh();
		} else { // Dateien ausführen
			// Entweder synchron oder asynchron (tracing '&') -> waitpid(fork())
			// fork();
			// execv();
		}
	}
	return 0;
}
