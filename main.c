#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "eval.h"
#include "gc.h"
#include "object.h"
#include "read.h"
#include "types.h"
#include "vm.h"

int main(int argc, char **argv) {
  int input_fd;

  if (argc != 2) {
    fprintf(stderr, "USAGE: %s file\n", argv[0]);
    exit(1);
  }

  if (strcmp("-", argv[1]) == 0)
    input_fd = STDIN_FILENO;
  else {
    input_fd = open(argv[1], O_RDONLY);
    if (input_fd < 0) {
      perror("could not open file");
      exit(1);
    }
  }

  fe_read(input_fd);
  vm.e = vm_pop_s();
  fe_read(input_fd);
  fe_eval();
  return 0;
}
