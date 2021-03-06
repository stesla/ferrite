#include <stdio.h>
#include "error.h"
#include "eval.h"
#include "vm.h"

void fe_eval() {
  vm_do(OP_SAVE);
  vm.c = vm_pop_s();
  vm.s = NIL;
  while (vm.c != NIL)
    vm_do(vm_pop_c());
  vm_do(OP_RTN);
}
