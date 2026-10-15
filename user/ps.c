
#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/riscv.h"
#include "kernel/spinlock.h"
#include "kernel/proc.h"
#include "kernel/uproc.h"
#include "user/user.h"

static char *
state_name(int state)
{
  switch (state) {
  case UNUSED:
    return "unused";
  case USED:
    return "used";
  case SLEEPING:
    return "sleeping";
  case RUNNABLE:
    return "runnable";
  case RUNNING:
    return "running";
  case ZOMBIE:
    return "zombie";
  default:
    return "unknown";
  }
}

int
main(int argc, char *argv[])
{
  struct uproc processes[NPROC];
  int count;

  if (argc != 1) {
    fprintf(2, "usage: ps\n");
    exit(1);
  }

  count = getprocs(processes, NPROC);

  if (count < 0) {
    fprintf(2, "ps: getprocs failed\n");
    exit(1);
  }

  printf("PID\tPPID\tSTATE\tSIZE\tNAME\n");

  for (int i = 0; i < count; i++) {
    printf("%d\t%d\t%s\t%lu\t%s\n",
           processes[i].pid,
           processes[i].ppid,
           state_name(processes[i].state),
           processes[i].sz,
           processes[i].name);
  }

  exit(0);
}
