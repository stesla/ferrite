#include <stdio.h>
#include <unistd.h>
#include "gc.h"
#include "object.h"
#include "read.h"
#include "vm.h"

void fe_read() {
  vm_save_sec();

  do {
    vm_push_s(make_fixnum(STDIN_FILENO));
    vm_get();
  } while (CHAR(car(vm.s)) != EOF);

  vm_pop_s();

  while (!nilp(cdr(vm.s)))
    vm_cons();

  /* We want to turn:

       (LDC "hello world" LDC 42 CONS PRINT)

     Into this code:
  */
  /* vm_push_s(OP_PRINT); */
  /* vm_cons(); */
  /* vm_push_s(OP_CONS); */
  /* vm_cons(); */
  /* vm_push_s(make_fixnum(42)); */
  /* vm_cons(); */
  /* vm_push_s(OP_LDC); */
  /* vm_cons(); */
  /* vm_push_s(make_string("hello world")); */
  /* vm_cons(); */
  /* vm_push_s(OP_LDC); */
  /* vm_cons(); */

  vm_rtn();
}
