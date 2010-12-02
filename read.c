#include "gc.h"
#include "read.h"
#include "vm.h"

void fe_read(FILE *in) {
  while (!feof(in))
    fgetc(in);
  vm_push_s(PRINT);
  vm_cons();
  vm_push_s(CONS);
  vm_cons();
  vm_push_s(make_fixnum(42));
  vm_cons();
  vm_push_s(LDC);
  vm_cons();
  vm_push_s(make_string("hello world"));
  vm_cons();
  vm_push_s(LDC);
  vm_cons();
}
