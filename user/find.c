
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// Match a filename against a pattern containing *.
int
match(char *name, char *pattern)
{
  if (*pattern == '\0')
    return *name == '\0';

  if (*pattern == '*') {
    // * can match zero or more characters.
    if (match(name, pattern + 1))
      return 1;

    if (*name != '\0')
      return match(name + 1, pattern);

    return 0;
  }

  if (*name == '\0' || *name != *pattern)
    return 0;

  return match(name + 1, pattern + 1);
}

// Search a directory and its subdirectories.
void
find(char *path, char *pattern, int type)
{
  char buf[512];
  char name[DIRSIZ + 1];
  struct dirent de;
  struct stat st;
  int fd;

  fd = open(path, O_RDONLY);

  if (fd < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;

    memmove(name, de.name, DIRSIZ);
    name[DIRSIZ] = '\0';

    if (strcmp(name, ".") == 0 ||
        strcmp(name, "..") == 0)
      continue;

    if (strlen(path) + 1 + strlen(name) + 1 > sizeof(buf))
      continue;

    strcpy(buf, path);
    int len = strlen(buf);

    buf[len] = '/';
    strcpy(buf + len + 1, name);

    if (stat(buf, &st) < 0)
      continue;

    if (match(name, pattern)) {
      if (type == 0 ||
          (type == 1 &&
           (st.type == T_FILE || st.type == T_DEVICE)) ||
          (type == 2 && st.type == T_DIR)) {
        printf("%s\n", buf);
      }
    }

    if (st.type == T_DIR)
      find(buf, pattern, type);
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  int type = 0;
  int fd;
  struct stat st;

  if (argc != 3 && argc != 5) {
    fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
    exit(1);
  }

  if (argc == 5) {
    if (strcmp(argv[3], "-type") != 0) {
      fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
      exit(1);
    }

    if (strcmp(argv[4], "f") == 0)
      type = 1;
    else if (strcmp(argv[4], "d") == 0)
      type = 2;
    else {
      fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
      exit(1);
    }
  }

  fd = open(argv[1], O_RDONLY);

  if (fd < 0) {
    fprintf(2, "find: cannot open %s\n", argv[1]);
    exit(1);
  }

  if (fstat(fd, &st) < 0 || st.type != T_DIR) {
    close(fd);
    fprintf(2, "find: cannot open %s\n", argv[1]);
    exit(1);
  }

  close(fd);

  find(argv[1], argv[2], type);

  exit(0);
}
