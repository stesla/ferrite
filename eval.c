#include <stdio.h>
#include "error.h"
#include "eval.h"
#include "vm.h"

void fe_eval_instruction() {
  ref_t inst = vm_pop_c();
  switch (inst) {
  case OP_LDC: {
    vm_ldc();
    break;
  }

  case OP_CONS:
    vm_cons();
    break;

  case OP_PRINT: {
    vm_print();
    break;
  }

  default:
    error("unknown instruction");
  }
}

void fe_eval() {
  while (vm.c != NIL)
    fe_eval_instruction();
}
