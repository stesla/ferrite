#include "eval.h"
#include "gc.h"
#include "object.h"
#include "read.h"
#include "types.h"
#include "vm.h"

int main(int argc, char **argv) {
  fe_read();
  vm.e = vm_pop_s();
  fe_read();
  fe_eval();
  return 0;
}
