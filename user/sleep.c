
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if (argc != 2 || argv[1][0] == '\0') {
        fprintf(2, "usage: sleep <ticks>\n");
        exit(1);
    }

    int ticks = 0;

    for (int i = 0; argv[1][i] != '\0'; i++) {
        char c = argv[1][i];

        if (c < '0' || c > '9') {
            fprintf(2, "usage: sleep <ticks>\n");
            exit(1);
        }

        int digit = c - '0';

        if (ticks > (2147483647 - digit) / 10) {
            fprintf(2, "usage: sleep <ticks>\n");
            exit(1);
        }

        ticks = ticks * 10 + digit;
    }

    pause(ticks);
    exit(0);
}
