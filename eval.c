#include <stdio.h>
#include "error.h"
#include "eval.h"
#include "vm.h"

void fe_eval_instruction() {
  ref_t inst = vm_pop_c();
  switch (inst) {
  case NIL:
    vm_nil();
    break;

  case OP_LD:
    vm_ld();
    break;

  case OP_LDC:
    vm_ldc();
    break;

  case OP_CONS:
    vm_cons();
    break;

  case OP_PRINT:
    vm_print();
    break;

  default:
    error("unknown instruction: 0x%lx", inst);
  }
}

void fe_eval() {
  while (vm.c != NIL)
    fe_eval_instruction();
}
