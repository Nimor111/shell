#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#define BUF_SIZE 80
#define ARG_COUNT 10

/* Courtesy of stack overflow */
char* string_trim_inplace(char* s)
{
    while (isspace((unsigned char)*s))
        s++;
    if (*s) {
        char* p = s;
        while (*p)
            p++;
        while (isspace((unsigned char)*(--p)))
            ;
        p[1] = '\0';
    }

    return s;
}

bool is_background_command(char const* s)
{
    for (int i = 0; i < strlen(s); i++)
        if (s[i] == '&')
            return true;

    return false;
}

void split_until_pipe(char* s, char* dest)
{
    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        if (s[i] == '|') {
            dest[i - 1] = '\0';
            break;
        } else {
            dest[i] = s[i];
        }
    }

    dest[i - 1] = '\0';
}

void exec_pipe(char** cmd1, char** cmd2)
{
    for (int i = 0; i < sizeof(cmd1) / sizeof(cmd1[0]); i++)
        printf("CMD1 ARG[%d] = %s\n", i, cmd1[i]);

    for (int i = 0; i < sizeof(cmd2) / sizeof(cmd2[0]); i++)
        printf("CMD2 ARG[%d] = %s\n", i, cmd2[i]);

    int fd[2];

    pipe(fd);

    pid_t pid;

    if ((pid = fork()) == -1) {
        perror("Error: ");
        exit(1);
    }

    if (!pid) { // child
        pid_t pid2;
        if ((pid2 = fork()) == -1) {
            perror("Error: ");
            exit(1);
        }

        if (!pid2) { // grandchild
            close(0);
            dup(fd[0]);
            close(fd[0]);
            close(fd[1]);
            if (execvp(*cmd2, (char* const*)&cmd2) == -1) {
                perror("Read from pipe exec error: ");
                exit(1);
            }
        } else { // child
            close(1);
            dup(fd[1]);
            close(fd[1]);
            close(fd[0]);
            if (execvp(*cmd1, (char* const*)&cmd1) == -1) {
                perror("Write to pipe exec error: ");
                exit(1);
            }
        }
    } else { // parent
        int _status;
        wait(&_status);
        return;
    }
}

char const* has_pipe(char* command)
{
    char* result = NULL;
    if ((result = strstr(command, "| "))) {
        return result;
    }

    return result;
}

void split_command(char const* command, char** args)
{
    int args_count = 0;

    int i;
    char buf[BUF_SIZE];
    int buf_index = 0;
    for (i = 0; i <= strlen(command); i++) {
        if (command[i] == ' ' || i == strlen(command)) {
            buf[buf_index] = '\0';
            args[args_count] = (char*)malloc(sizeof(buf));
            strncpy(args[args_count], buf, sizeof(buf));
            args_count++;
            buf_index = 0;
        } else {
            buf[buf_index++] = command[i];
        }
    }

    args[args_count] = NULL;
}

void split_background_command(char const* command, char** args)
{
    split_command(command, args);
    size_t size = sizeof(args) / sizeof(args[0]);
    args[size - 2] = NULL;
}

int main(int argc, char const** argv)
{
    char* prompt = (char*)malloc(1024 * sizeof(char));
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    sprintf(prompt, "(%s) %s", cwd, "msh> ");
    while (1) {
        /* int status; */
        /* if (waitpid(-1, &status, WNOHANG)) { */
        /*     printf("Child status: %d\n", status); */
        /* } */
        write(STDOUT_FILENO, prompt, strlen(prompt));

        char command[BUF_SIZE];

        size_t n;
        if ((n = read(STDIN_FILENO, command, sizeof(command))) == -1) {
            perror("Error: ");
            exit(1);
        }
        command[n - 1] = '\0';
        char* cleared_command = string_trim_inplace(command);

        if (!strcmp(cleared_command, "exit") || !strcmp(cleared_command, "logout")) {
            printf("Goodbye!\n");
            return 0;
        }

        char* args[ARG_COUNT];
        const char* string_after_pipe = (char*)malloc(BUF_SIZE * sizeof(char));

        // PARTIALLY WORKS
        if ((string_after_pipe = has_pipe(cleared_command)) != NULL) {
            char* args1[ARG_COUNT];
            char* cmd_before_pipe = (char*)malloc(BUF_SIZE * sizeof(char));
            split_until_pipe(cleared_command, cmd_before_pipe);
            split_command(cmd_before_pipe, args1);

            char* args2[ARG_COUNT];
            split_command(&string_after_pipe[2], args2);

            /* exec_pipe(args1, args2); */

            /**** DEBUG COMMENTS ****/
            /* for (int i = 0; args1[i] != NULL; i++) */
            /*     printf("CMD1 ARG[%d] = %s\n", i, args1[i]); */

            /* for (int i = 0; args2[i] != NULL; i++) */
            /*     printf("CMD2 ARG[%d] = %s\n", i, args2[i]); */
            /**** DEBUG COMMENTS ****/

            int fd[2];

            pipe(fd);

            pid_t pid;

            if ((pid = fork()) == -1) {
                perror("Error: ");
                exit(1);
            }

            if (!pid) { // child
                pid_t pid2;
                if ((pid2 = fork()) == -1) {
                    perror("Error: ");
                    exit(1);
                }

                if (!pid2) { // grandchild
                    close(0);
                    dup(fd[0]);
                    close(fd[0]);
                    close(fd[1]);
                    if (execvp(*args2, (char* const*)&args2) == -1) {
                        perror("Read from pipe exec error: ");
                        exit(1);
                    }
                } else { // child
                    close(1);
                    dup(fd[1]);
                    close(fd[1]);
                    close(fd[0]);
                    if (execvp(*args1, (char* const*)&args1) == -1) {
                        perror("Write to pipe exec error: ");
                        exit(1);
                    }
                }
            } else { // parent
                close(fd[0]);
                close(fd[1]);
                int _status;
                wait(&_status);
                continue;
            }
        }

        if (is_background_command(cleared_command) == true) {
            cleared_command[strlen(cleared_command) - 1] = '\0';
            split_background_command(cleared_command, args);
            pid_t pid;
            if ((pid = fork()) == -1) {
                perror("Error: ");
                exit(1);
            }

            if (!pid) { // child
                setpgid(0, 0);
                if (execvp(*args, (char* const*)&args) == -1) {
                    perror("Background Execvp error: ");
                    exit(1);
                }
            } else { // parent
                /* printf("Child finished with status %d\n", status); */
                printf("Process started with pid %d\n", pid);
                continue;
            }
        }

        split_command(cleared_command, args);

        pid_t pid;
        if ((pid = fork()) == -1) {
            perror("Error: ");
            exit(1);
        }

        if (!pid) { // child
            if (execvp(*args, (char* const*)&args) == -1) {
                perror("Execvp error: ");
                exit(1);
            }
        } else { // parent
            int status;
            wait(&status);
            /* printf("Child finished with status %d\n", status); */
            continue;
        }

        size_t size = sizeof(args) / sizeof(args[0]);

        for (int i = 0; i < size; i++)
            free(args[i]);

        fflush(stdout);
    }

    free(prompt);

    return 0;
}
