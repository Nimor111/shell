#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <wait.h>

void pipe_it() {
    int fd[2];

    pipe(fd);

    pid_t pid;

    if ((pid = fork()) == -1) {
        perror("Can't fork.");
        exit(1);
    }

    if (!pid) { // child
        close(0);
        dup(fd[0]);
        close(fd[0]);
        close(fd[1]);
        execlp("wc", "wc", "-l", 0);
   } else { // parent
        close(1);
        dup(fd[1]);
        close(fd[1]);
        close(fd[0]);
        execlp("ls", "ls", "-l", 0);
    }
}

int main(int argc, char *argv[]) {
    pipe_it();

    return 0;
}
