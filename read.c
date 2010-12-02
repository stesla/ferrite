#include "gc.h"
#include "read.h"
#include "vm.h"

void fe_read(FILE *in) {
  vm_push(PRINT);
  vm_cons();
  vm_push(PRINT);
  vm_cons();
  vm_push(CONS);
  vm_cons();
  vm_push(make_fixnum(42));
  vm_cons();
  vm_push(LDC);
  vm_cons();
  vm_push(make_string("hello world"));
  vm_cons();
  vm_push(LDC);
  vm_cons();
}
