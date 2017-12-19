#include <stdio.h>
#include <unistd.h>

/* Does not work because cd is not an executable in the system, it uses another system call chdir() */
int main(int argc, char *argv[])
{
    char * const args[3] = {"cd", "~/home", NULL};

    execvp(*args, (char* const*)&args);
    printf("THIS FAILED");

    return 0;
}
