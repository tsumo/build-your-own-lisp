#include <stdio.h>

/* Buffer for user input */
static char input[2048];

int main(int argc, char** argv) {
    puts("Lispy version 0.0.0.0.1");
    puts("Press Ctrl+c to exit\n");

    while (1) {
        fputs("lispy> ", stdout);
        fgets(input, 2048, stdin);
        printf("No you're a %s", input);
    }

    return 0;
}

