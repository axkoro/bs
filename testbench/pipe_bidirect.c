#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

pid_t child_pid;

void signal_handler(int signo)
{
    if (child_pid == 0)
    {
        exit(0);
    }
    else
    {
        char msg[] = "Programmabbruch durch Nutzer\n";
        write(STDOUT_FILENO, msg, sizeof(msg) - 1);
        exit(0);
    }
}

int main(void)
{
    signal(SIGINT, signal_handler);

    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);

    int fd_log = open("log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600);
    dup2(fd_log, STDOUT_FILENO);
    close(fd_log);

    int fds[2][2];
    pipe(fds[0]); // Pipe 0: Foo -> Bar
    pipe(fds[1]); // Pipe 1: Bar -> Foo

    child_pid = fork();
    if (child_pid == 0)
    { // Foo
        int file_des[] = { fds[0][0],
                            fds[1][1] }; // Write to Pipe 1, Read from Pipe 0

        pid_t pid = getpid();
        char pid_prefix[16];
        sprintf(pid_prefix, "%d: ", pid);

        char ping[] = "Ping!\n";

        while (1)
        {
            write(file_des[1], ping, sizeof(ping));

            char buf[8];
            read(file_des[0], buf, sizeof(buf));

            write(STDOUT_FILENO, pid_prefix, strlen(pid_prefix));
            write(STDOUT_FILENO, buf, strlen(buf));

            sleep(1);
        }
    }
    else
    { // Bar
        int file_des[] = { fds[1][0],
                            fds[0][1] }; // Write to Pipe 0, Read from Pipe 1

        pid_t pid = getpid();
        char pid_prefix[16];
        sprintf(pid_prefix, "%d: ", pid);

        char pong[] = "Pong!\n";
        while (1)
        {
            char buf[8];
            read(file_des[0], buf, sizeof(buf));

            write(STDOUT_FILENO, pid_prefix, strlen(pid_prefix));
            write(STDOUT_FILENO, buf, strlen(buf));

            sleep(1);

            write(file_des[1], pong, sizeof(pong)-1);
        }

        exit(0);
    }
}