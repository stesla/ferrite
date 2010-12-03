#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "error.h"
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

void vm_rcons() {
  ref_t cell = make_cons();
  CONS(cell)->cdr = vm_pop_s();
  CONS(cell)->car = vm_pop_s();
  vm_push_s(cell);
}

void vm_ldc() {
  vm_push_s(car(vm.c));
  vm_pop_c();
}


static struct {
  const char *token;
  ref_t op;
} ops [] = {
  {"LD", OP_LD},
  {"LDC", OP_LDC},
  {"LDF", OP_LDF},
  {"EQ", OP_EQ},
  {"ATOMP", OP_ATOMP},
  {"CONS", OP_CONS},
  {"RCONS", OP_RCONS},
  {"CAR", OP_CAR},
  {"CDR", OP_CDR},
  {"ADD", OP_ADD},
  {"SUB", OP_SUB},
  {"MUL", OP_MUL},
  {"DIV", OP_DIV},
  {"GET", OP_GET},
  {"PUT", OP_PUT},
  {"PRINT", OP_PRINT},
  {"READ", OP_READ},
  {"SEL", OP_SEL},
  {"JOIN", OP_JOIN},
  {"AP", OP_AP},
  {"RTN", OP_RTN},
  {"NIL", NIL},
  {NULL, 0}
};

ref_t vm_op_with_name(const char *name) {
  size_t i = 0;
  while (ops[i].token != NULL) {
    if (strcmp(name, ops[i].token) == 0)
      return ops[i].op;
    i++;
  }
  error("invalid op code");
  return NIL;
}

static const char *vm_name_for_op(ref_t value) {
  size_t i = 0;
  while (ops[i].token != NULL) {
    if (ops[i].op == value)
      return ops[i].token;
    i++;
  }
  error("invalid op code");
  return NULL;
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
  else if(opcodep(value))
    printf("%s", vm_name_for_op(value));
  else if(consp(value)) {
    fputc('(', stdout);
    while (consp(value)) {
      print(car(value));
      value = cdr(value);
      if (!nilp(value))
        fputc(' ', stdout);
    }
    if (!nilp(value)) {
      printf(". ");
      print(value);
    }
    fputc(')', stdout);
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

void vm_nil() {
  vm_push_s(NIL);
}

static ref_t locate(int list, int item) {
  int i, j;
  /* find the jth item in the ith list in vm.e */
  ref_t e = vm.e;
  for (i = list; i > 1; i--)
    e = cdr(e);
  if (nilp(e))
    error("invalid lookup list index %d", list);
  e = car(e);
  for (j = item; j > 1; j--)
    e = cdr(e);
  if (nilp(e))
    error("invalid lookup item index %d", item);
  return car(e);
}

void vm_ld() {
  ref_t loc = vm_pop_c();
  if (!(consp(loc) && fixnump(car(loc)) && fixnump(cdr(loc))))
    error("invalid environment location");
  vm_push_s(locate(FIXNUM(car(loc)), FIXNUM(cdr(loc))));
}
