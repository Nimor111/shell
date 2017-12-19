#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <wait.h>
#include <string.h>
#include <ctype.h>

#define BUF_SIZE 80
#define ARG_COUNT 10

char *string_trim_inplace(char *s) {
    while(isspace((unsigned char) *s)) s++;
    if (*s) {
        char *p = s;
        while(*p) p++;
        while(isspace((unsigned char) *(--p)));
        p[1] = '\0';
    }

    return s;
}


int main(int argc, char const ** argv) {
    char const * prompt = "msh> ";
    while(1) {
        write(STDOUT_FILENO, prompt, strlen(prompt));

        char command[BUF_SIZE];

        size_t n;
        if ((n = read(STDIN_FILENO, command, sizeof(command))) == -1) {
            perror("Error: ");
            exit(1);
        }
        command[n - 1] = '\0';
        char* cleared_command = string_trim_inplace(command);

        char *args[ARG_COUNT];
        int args_count = 0;

        int i;
        char buf[BUF_SIZE];
        int buf_index = 0;
        for ( i = 0; i <= strlen(cleared_command); i++ ) {
            if (cleared_command[i] == ' ' || i == strlen(cleared_command)) {
                buf[buf_index] = '\0';
                args[args_count] = (char*)malloc(sizeof(buf));
                strncpy(args[args_count], buf, sizeof(buf));
                args_count++;
                buf_index = 0;
            } else {
                buf[buf_index++] = cleared_command[i];
            }
        }

        args[args_count] = NULL;

        pid_t pid;
        if ((pid = fork()) == -1 ) {
            perror("Error: ");
            exit(1);
        }

        if (!pid) { // child
            if ( execvp(*args, (char* const*)&args) == -1 ) {
                perror("Execvp error: ");
                exit(1);
            }
        } else { // parent
            int status;
            wait(&status);
            printf("Child finished with status %d\n", status);
            continue;
        }

        for ( i = 0; i < args_count; ++i)
            free(args[i]);
    }
}
