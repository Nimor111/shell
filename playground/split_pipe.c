#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

char* split_until_pipe(const char* s)
{
    char* result = (char*)malloc(BUF_SIZE * sizeof(char));
    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        if (s[i] == '|') {
            result[i-1] = '\0';
            return result;
        } else {
            result[i] = s[i];
        }
    }

    return result;
}

int main(int argc, char* argv[])
{
    char* res = split_until_pipe("ls -l | wc -l");
    printf("%li\n", strlen(res));
    printf("%s\n", res);

    return 0;
}
