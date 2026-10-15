
#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define BUFSIZE 4096

static char buffer[BUFSIZE];
static char *args[MAXARG + 1];

void
usage(void)
{
  fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
  exit(1);
}

int
run(char *args[])
{
  int pid = fork();

  if (pid < 0) {
    fprintf(2, "xargs: fork failed\n");
    return 1;
  }

  if (pid == 0) {
    exec(args[0], args);
    fprintf(2, "xargs: exec failed\n");
    exit(1);
  }

  int status = 1;

  if (wait(&status) < 0)
    return 1;

  return status != 0;
}

int
main(int argc, char *argv[])
{
  int start = 1;
  int limit = 0;
  int failed = 0;

  if (argc > 1 && strcmp(argv[1], "-n") == 0) {
    if (argc < 4 || argv[2][0] == '\0')
      usage();

    for (int i = 0; argv[2][i] != '\0'; i++) {
      char c = argv[2][i];

      if (c < '0' || c > '9')
        usage();

      int digit = c - '0';

      if (limit > (2147483647 - digit) / 10)
        usage();

      limit = limit * 10 + digit;
    }

    if (limit < 1)
      usage();

    start = 3;
  }

  if (start >= argc)
    usage();

  int base = argc - start;

  if (base >= MAXARG)
    usage();

  if (limit > MAXARG - base)
    limit = MAXARG - base;

  for (int i = 0; i < base; i++)
    args[i] = argv[start + i];

  int count = 0;
  int pos = 0;
  int inword = 0;
  char ch;
  int nread;

  while ((nread = read(0, &ch, 1)) == 1) {
    int separator =
      ch == ' ' || ch == '\t' || ch == '\n';

    if (!separator) {
      if (!inword) {
        if (count >= MAXARG - base) {
          fprintf(2, "xargs: too many arguments\n");
          exit(1);
        }

        args[base + count] = &buffer[pos];
        count++;
        inword = 1;
      }

      if (pos >= BUFSIZE - 1) {
        fprintf(2, "xargs: input too long\n");
        exit(1);
      }

      buffer[pos++] = ch;
    } else {
      if (inword) {
        buffer[pos++] = '\0';
        inword = 0;
      }

      int execute = 0;

      if (limit > 0) {
        if (count >= limit)
          execute = 1;
      } else if (ch == '\n' && count > 0) {
        execute = 1;
      }

      if (execute) {
        args[base + count] = 0;

        if (run(args))
          failed = 1;

        count = 0;
        pos = 0;
      }
    }
  }

  if (nread < 0) {
    fprintf(2, "xargs: read failed\n");
    exit(1);
  }

  if (inword) {
    buffer[pos++] = '\0';
  }

  if (count > 0) {
    args[base + count] = 0;

    if (run(args))
      failed = 1;
  }

  exit(failed ? 1 : 0);
}
