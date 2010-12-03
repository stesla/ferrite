#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gc.h"
#include "vm.h"
#include "object.h"

struct vm vm = {NIL, NIL, NIL, NIL};

static ref_t vm_pop(ref_t *r) {
  ref_t result;
  if (*r == NIL)
    return NIL;
  result = car(*r);
  *r = cdr(*r);
  return result;
}

static void vm_push(ref_t *r, ref_t ref) {
  ref_t cell = make_cons();
  CONS(cell)->car = gc_lookup(ref);
  CONS(cell)->cdr = *r;
  *r = cell;
}

ref_t vm_pop_c() { return vm_pop(&vm.c); }
ref_t vm_pop_d() { return vm_pop(&vm.d); }
ref_t vm_pop_s() { return vm_pop(&vm.s); }
void vm_push_c(ref_t ref) { vm_push(&vm.c, ref); }
void vm_push_d(ref_t ref) { vm_push(&vm.d, ref); }
void vm_push_s(ref_t ref) { vm_push(&vm.s, ref); }

void vm_save_sec() {
  vm_push_d(vm.c);
  vm_push_d(vm.e);
  vm_push_d(vm.s);
}

void vm_get() {
  ref_t f = vm_pop_s();
  int fd = fixnum_to_int(f);
  char ch;
  switch (read(fd, &ch, 1)) {
  case 1:
    vm_push_s(make_char(ch));
    break;
  case 0:
    vm_push_s(make_char(EOF));
    break;
  default:
    perror("vm_get");
    exit(1);
  }
}

void vm_cons() {
  ref_t cell = make_cons();
  CONS(cell)->car = vm_pop_s();
  CONS(cell)->cdr = vm_pop_s();
  vm_push_s(cell);
}

void vm_ldc() {
  vm_push_s(car(vm.c));
  vm_pop_c();
}

static void print(ref_t value) {
  if (nilp(value))
    printf("NIL");
  else if(charp(value))
    printf("$%c", CHAR(value));
  else if(fixnump(value))
    printf("%d", FIXNUM(value));
  else if(stringp(value))
    printf("\"%s\"", STRING(value)->bytes);
  else if(consp(value)) {
    printf("(");
    print(car(value));
    printf(" . ");
    print(cdr(value));
    printf(")");
  }
}

void vm_print() {
  ref_t value = vm_pop_s();
  print(value);
}

void vm_rtn() {
  if (nilp(vm.s))
    vm.s = vm_pop_d();
  else
    CONS(vm.s)->cdr = vm_pop_d();
  vm.e = vm_pop_d();
  vm.c = vm_pop_d();
}
