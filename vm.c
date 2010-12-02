#include "gc.h"
#include "vm.h"

struct vm vm = {NIL, NIL, NIL, NIL};

ref_t vm_pop() {
  return NIL;
}

void vm_push(ref_t ref) {
}

void vm_cons() {
  ref_t s, c;

  s = make_cons();
  set_cdr(s, vm.s);
  vm.s = s;

  c = make_cons();

  s = vm.s;
  vm.s = get_cdr(s);

  set_car(c, vm_pop());
  set_cdr(c, vm_pop());

  set_car(s, c);
  set_cdr(s, vm.s);
  vm.s = s;
}
