
#include "kernel/types.h"
#include "user/user.h"

static void
usage(void)
{
  fprintf(2, "usage: trace <mask> <cmd> [args...]\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  int mask = 0;

  if (argc < 3 || argv[1][0] == '\0')
    usage();

  // Read the tracing mask.
  for (int i = 0; argv[1][i] != '\0'; i++) {
    char c = argv[1][i];

    if (c < '0' || c > '9')
      usage();

    int digit = c - '0';

    if (mask > (2147483647 - digit) / 10)
      usage();

    mask = mask * 10 + digit;
  }

  // Enable tracing for this process.
  if (trace(mask) < 0) {
    fprintf(2, "trace: syscall failed\n");
    exit(1);
  }

  // Replace this process with the requested command.
  exec(argv[2], &argv[2]);

  // Only reached if exec fails.
  fprintf(2, "trace: exec failed\n");
  exit(1);
}
