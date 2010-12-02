#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "gc.h"
#include "read.h"
#include "eval.h"
#include "vm.h"

int main(int argc, char **argv) {
  fe_read(stdin);
  vm.c = CONS(vm.s)->car, vm.s = NIL;
  fe_eval();
  return 0;
}
