#define _POSIX_C_SOURCE 200809L

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
#define ARGS_MAX 2

pid_t monitored_process = 0;

void sig_handler(int signo) {
	if (monitored_process > 0) {
		kill(monitored_process, SIGINT);
	} // else ignore
}

int main(void) {
	signal(SIGINT, sig_handler);

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

		char* args[10];
		args[0] = strtok(input_buf, " \n");

		char* arg;
		int i = 1;
		while ((arg = strtok(NULL, " \n")) != NULL) {
			args[i++] = arg;
		}
		args[i] = NULL;
		int argc = i - 1;

		// Process input
		if (strcmp(args[0], "cd") == 0) {
			char path[PATH_MAX];

			if (args[0][0] == '/') { // absolute path
				snprintf(path, sizeof(path), "%s", args[0]);
			} else { // relative path
				snprintf(path, sizeof(path), "%s/%s", cwd, args[0]);
			}

			if (chdir(path) != 0) {
				perror("chdir failed");
			}

		} else if (strcmp(args[0], "wait") == 0) {
			pid_t monitored_pids[argc];
			int handled[argc];
			for (size_t i = 1; i <= argc; i++) {
				monitored_pids[i] = atoi(args[i]);
				handled[i] = 0;
			}

			int remaining = argc;
			while (remaining > 0) { // FIXME: Infinite loop after some calls to wait
				int status = 0;
				pid_t finished_process = wait(&status);

				for (size_t j = 1; j <= argc; j++) {
					if (!handled[j] && monitored_pids[j] == finished_process) {
						printf("[%d] TERMINATED\n[%d] EXIT CODE: %d\n", finished_process, finished_process, WEXITSTATUS(status));
						handled[j] = 1;
						remaining--;
						break;
					}
				}
			}

			// TODO: Ctrl+C
		} else if (strcmp(args[0], "exit") == 0) {
			exit(0);
		} else { // Execute file at path arg[0] with args
			int is_async = (strcmp(args[argc], "&") == 0);

			monitored_process = fork();
			if (monitored_process == 0) {
				execv(args[0], args);
				perror("execv failed");
				exit(-1);
			} else if (is_async) {
				// FIXME: Output appearing after next shell prompt
				printf("[%d]\n", monitored_process);
			} else {
				int status;
				waitpid(monitored_process, &status, 0);
			}

			// TODO: Pipes
		}

		monitored_process = 0;
	}
	return 0;
}
