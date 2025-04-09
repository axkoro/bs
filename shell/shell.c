#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t monitored_process = 0;

void forward_sigint(int signo) {
    if (monitored_process > 0) {
        kill(monitored_process, SIGINT);
    }  // else ignore
}

int main(void) {
    signal(SIGINT, forward_sigint);

    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    setvbuf(stdin, NULL, _IOLBF, BUFSIZ);

    while (1) {
        // Prompt
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd failed");
            continue;
        }
        char prompt[PATH_MAX + 3];
        snprintf(prompt, sizeof(prompt), "%s> ", cwd);
        write(STDOUT_FILENO, prompt, strlen(prompt));

        // Read input
        char input_buf[BUFSIZ];
        ssize_t bytes_read = read(STDIN_FILENO, input_buf, BUFSIZ - 1);  // wait until line break
        if (bytes_read <= 0) continue;
        input_buf[bytes_read] = '\0';

        // Tokenize input
        char *args[20];
        int arg_count = 0;
        char *token = strtok(input_buf, " \n");
        while (token != NULL && arg_count < 19) {
            args[arg_count++] = token;
            token = strtok(NULL, " \n");
        }
        args[arg_count] = NULL;
        if (arg_count == 0) {
            continue;
        }

        // Built-in commands
        if (strcmp(args[0], "exit") == 0) {
            exit(0);
        }

        if (strcmp(args[0], "cd") == 0) {
            char path[PATH_MAX];
            if (arg_count < 2) {
                fprintf(stderr, "cd: missing argument\n");
            } else {
                if (args[0][0] == '/') {  // absolute path
                    snprintf(path, sizeof(path), "%s", args[1]);
                } else {  // relative path
                    snprintf(path, sizeof(path), "%s/%s", cwd, args[1]);
                }

                if (chdir(path) != 0) {
                    perror("chdir failed");
                }
            }
            continue;
        }

        if (strcmp(args[0], "wait") == 0) {
            if (arg_count < 2) {
                fprintf(stderr, "wait: missing pid(s)\n");
            } else {
                int num_pids = arg_count - 1;
                pid_t monitored_pids[num_pids];
                int handled[num_pids];
                for (int j = 0; j < num_pids; j++) {
                    monitored_pids[j] = (pid_t)atoi(args[j + 1]);
                    handled[j] = 0;
                }
                int remaining = num_pids;
                while (remaining > 0) {
                    int status = 0;
                    pid_t finished_process = wait(&status);
                    for (int j = 0; j < num_pids; j++) {
                        if (!handled[j] && monitored_pids[j] == finished_process) {
                            printf("[%d] TERMINATED\n[%d] EXIT CODE: %d\n", finished_process,
                                   finished_process, WEXITSTATUS(status));
                            handled[j] = 1;
                            remaining--;
                            break;
                        }
                    }
                }
            }
            continue;
        }

        // Execute specified program
        int is_async = 0;
        if (arg_count > 0 && strcmp(args[arg_count - 1], "&") == 0) {
            is_async = 1;
            args[arg_count - 1] = NULL;  // Remove "&"
            arg_count--;
        }

        monitored_process = fork();
        if (monitored_process == 0) {
            execvp(args[0], args);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else if (monitored_process < 0) {
            perror("fork failed");
        } else {
            if (is_async) {
                printf("[%d]\n", monitored_process);
            } else {
                int status;
                waitpid(monitored_process, &status, 0);
            }
        }
        monitored_process = 0;
    }
    return 0;
}
