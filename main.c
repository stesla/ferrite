#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "gc.h"
#include "read.h"
#include "eval.h"

struct vm vm = {NIL, NIL, NIL, NIL};

void error(char *msg) {
  fprintf(stderr, "%s", msg);
  exit(1);
}

#define FIXNUM_MAX  536870911
#define FIXNUM_MIN -536870912


ref_t make_fixnum(int i) {
  if (FIXNUM_MIN <= i && i <= FIXNUM_MAX)
    return i << 2;
  error("out of range");
  return NIL;
}

ref_t make_string(char *s) {
  return NIL;
}

ref_t pop() {
  return NIL;
}

void push(ref_t ref) {
}

void cons() {
  ref_t s, c;

  s = make_cons();
  set_cdr(s, vm.s);
  vm.s = s;

  c = make_cons();

  s = vm.s;
  vm.s = get_cdr(s);

  set_car(c, pop());
  set_cdr(c, pop());

  set_car(s, c);
  set_cdr(s, vm.s);
  vm.s = s;
}

void fe_read(FILE *in) {
  push(PRINT); cons();
  push(PRINT); cons();
  push(CONS); cons();
  push(make_fixnum(42)); cons();
  push(LDC); cons();
  push(make_string("hello world")); cons();
  push(LDC); cons();
}

void fe_eval() {
  printf("you lose");
}

int main(int argc, char **argv) {
  fe_read(stdin);
  vm.c = vm.s, vm.s = NIL;
  fe_eval();
  return 0;
}
