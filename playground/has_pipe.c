#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    char const * has_pipe = "ls -l | wc -l";
    char const * no_pipe = "ls -l";

    printf("%s\n", strstr(has_pipe, "|"));
    if ( strstr(no_pipe, "|") == NULL ) {
        write(STDOUT_FILENO, "NOPE\n", 5);
    }

    return 0;
}
