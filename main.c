#include <unistd.h>
#include "eval.h"
#include "gc.h"
#include "object.h"
#include "read.h"
#include "types.h"
#include "vm.h"

int main(int argc, char **argv) {
  fe_read(STDIN_FILENO);
  vm.e = vm_pop_s();
  fe_read(STDIN_FILENO);
  fe_eval();
  return 0;
}
