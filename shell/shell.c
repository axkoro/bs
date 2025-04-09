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
    }
}

void print_prompt(void) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return;
    }
    char prompt[PATH_MAX + 3];
    snprintf(prompt, sizeof(prompt), "%s> ", cwd);
    write(STDOUT_FILENO, prompt, strlen(prompt));
}

ssize_t read_input(char *buffer, size_t size) {
    ssize_t bytes_read = read(STDIN_FILENO, buffer, size - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
    }
    return bytes_read;
}

int parse_input(char *input, char **args, int max_args) {
    int arg_count = 0;
    char *token = strtok(input, " \n");
    while (token != NULL && arg_count < max_args - 1) {
        args[arg_count++] = token;
        token = strtok(NULL, " \n");
    }
    args[arg_count] = NULL;
    return arg_count;
}

void handle_cd(char **args, int arg_count) {
    if (arg_count < 2) {
        fprintf(stderr, "cd: missing argument\n");
        return;
    }
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return;
    }
    char path[PATH_MAX];
    // Use args[1] to check for absolute vs. relative path.
    if (args[1][0] == '/') {
        snprintf(path, sizeof(path), "%s", args[1]);
    } else {
        snprintf(path, sizeof(path), "%s/%s", cwd, args[1]);
    }
    if (chdir(path) != 0) {
        perror("chdir failed");
    }
}

void handle_wait(char **args, int arg_count) {
    if (arg_count < 2) {
        fprintf(stderr, "wait: missing pid(s)\n");
        return;
    }
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
                printf("[%d] TERMINATED\n[%d] EXIT CODE: %d\n", finished_process, finished_process,
                       WEXITSTATUS(status));
                handled[j] = 1;
                remaining--;
                break;
            }
        }
    }
}

void execute_command(char **args, int arg_count) {
    int is_async = 0;
    if (arg_count > 0 && strcmp(args[arg_count - 1], "&") == 0) {
        is_async = 1;
        args[arg_count - 1] = NULL;
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

int main(void) {
    signal(SIGINT, forward_sigint);
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    setvbuf(stdin, NULL, _IOLBF, BUFSIZ);

    while (1) {
        print_prompt();

        char input_buf[BUFSIZ];
        if (read_input(input_buf, BUFSIZ) <= 0) {
            continue;
        }

        char *args[20];
        int arg_count = parse_input(input_buf, args, 20);
        if (arg_count == 0) {
            continue;
        }

        if (strcmp(args[0], "exit") == 0) {
            exit(0);
        }

        if (strcmp(args[0], "cd") == 0) {
            handle_cd(args, arg_count);
            continue;
        }

        if (strcmp(args[0], "wait") == 0) {
            handle_wait(args, arg_count);
            continue;
        }

        execute_command(args, arg_count);
    }
    return 0;
}