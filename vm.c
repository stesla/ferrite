#include <stdio.h>
#include "gc.h"
#include "vm.h"

struct vm vm = {NIL, NIL, NIL, NIL};

static ref_t vm_pop(ref_t *r) {
  ref_t result;
  if (*r == NIL)
    return NIL;
  result = CONS(*r)->car;
  *r = CONS(*r)->cdr;
  return result;
}

static void vm_push(ref_t *r, ref_t ref) {
  ref_t cell = make_cons();
  CONS(cell)->car = gc_lookup(ref);
  CONS(cell)->cdr = *r;
  *r = cell;
}

ref_t vm_pop_c() { return vm_pop(&vm.c); }
ref_t vm_pop_s() { return vm_pop(&vm.s); }
void vm_push_c(ref_t ref) { vm_push(&vm.c, ref); }
void vm_push_s(ref_t ref) { vm_push(&vm.s, ref); }

void vm_cons() {
  ref_t cell = make_cons();
  CONS(cell)->car = vm_pop_s();
  CONS(cell)->cdr = vm_pop_s();
  vm_push_s(cell);
}

void vm_ldc() {
  vm_push_s(CONS(vm.c)->car);
  vm_pop_c();
}

typedef enum {
  NO = 0,
  YES = 1
} bool;

static inline bool nilp(ref_t value) { return value == NIL; }

static inline bool fixnump(ref_t value) {
  return (value & LOWTAG_MASK) == LOWTAG_FIXNUM;
}

static inline bool stringp(ref_t value) {
  return (value & LOWTAG_MASK) == LOWTAG_STRING;
}

static inline bool consp(ref_t value) {
  return (value & LOWTAG_MASK) == LOWTAG_CONS;
}

static void print(ref_t value) {
  if (nilp(value))
    printf("NIL");
  else if(fixnump(value))
    printf("%d", FIXNUM(value));
  else if(stringp(value))
    printf("\"%s\"", STRING(value)->bytes);
  else if(consp(value)) {
    printf("(");
    print(CONS(value)->car);
    printf(" . ");
    print(CONS(value)->cdr);
    printf(")");
  }
}

void vm_print() {
  ref_t value = vm_pop_s();
  print(value);
}
