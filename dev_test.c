#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/* Cmd for ioctl */
#define MEM_CLEAR 1

/*
 * print_usage - Print usage of testdev
 */
void print_usage(void) {
  printf("Usage & Format: \n");
  printf("\t./dev_test read <startpos> <readnum>\n");
  printf("\t./dev_test write <startpos> <string>\n");
  printf("\t./dev_test ioctl clear\n");
}

int main(int argc, char **argv) {
  int fd, start, num;
  char buf[1024];
  fd = open("/dev/my_driver", O_RDWR);
  if (fd < 0) {
    printf("Open error!\n");
    return 0;
  }
  if (argc == 4 && strncmp(argv[1], "read", 4) == 0) {
    start = atoi(argv[2]);
    num = atoi(argv[3]);
    lseek(fd, start, SEEK_SET);
    read(fd, buf, num);
    printf("Read: %s\n", buf);
  }
  else if (argc == 4 && strncmp(argv[1], "write", 5) == 0) {
    start = atoi(argv[2]);
    lseek(fd, start, SEEK_CUR);
    write(fd, argv[3], strlen(argv[3]));
    printf("Write succeessfully!\n");
  }
  else if (argc == 3 && strncmp(argv[1], "ioctl", 5) == 0) {
    if (strncmp(argv[2], "clear", 5) == 0) {
      ioctl(fd, MEM_CLEAR, NULL);
      printf("Clear succeessfully!\n");
    }
    else
      print_usage();
  }
  else
    print_usage();

  close(fd);
  return 0;
}
