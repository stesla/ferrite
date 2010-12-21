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

static void vm_save() {
  vm_push_d(vm.c);
  vm_push_d(vm.e);
  vm_push_d(vm.s);
}

static void vm_get() {
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

static void vm_put() {
  ref_t f = vm_pop_s();
  int fd = fixnum_to_int(f);
  ref_t value = vm_pop_s();
  if (charp(value)) {
    char ch = CHAR(value);
    /* TODO: decide on error conditions */
    if (write(fd, &ch, 1) != 1)
      error("error in PUT");
  }
  else if (stringp(value)) {
    const char *str = STRING(value)->bytes;
    size_t len = strlen(str);
    /* TODO: decide on error conditions */
    if (write(fd, str, len) != len)
      error("error in PUT");
  }
  else
    error("invalid argument to PUT");
}

static void vm_cons() {
  ref_t cell = make_cons();
  CONS(cell)->car = vm_pop_s();
  CONS(cell)->cdr = vm_pop_s();
  vm_push_s(cell);
}

static void vm_rcons() {
  ref_t cell = make_cons();
  CONS(cell)->cdr = vm_pop_s();
  CONS(cell)->car = vm_pop_s();
  vm_push_s(cell);
}

static void vm_ldc() {
  vm_push_s(car(vm.c));
  vm_pop_c();
}

static void vm_nil() {
  vm_push_s(NIL);
}

static void vm_ldf() {
  vm_nil(); /* allocate our storage */
  ref_t cell = CONS(vm.s)->car = make_cons();
  CONS(cell)->car = vm_pop_c();
  CONS(cell)->cdr = vm.e;
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

static const char *vm_opcode_name(ref_t value) {
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
  else if(charp(value)) {
    char ch = CHAR(value);
    if (0x20 < ch && ch < 0x7F)
      printf("#\\%c", ch);
    else
      printf("#\\x%.2X", (int) ch);
  }
  else if(fixnump(value))
    printf("%d", FIXNUM(value));
  else if(stringp(value))
    printf("\"%s\"", STRING(value)->bytes);
  else if(opcodep(value))
    printf("%s", vm_opcode_name(value));
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

static void vm_print() {
  ref_t value = vm_pop_s();
  print(value);
}

static void vm_rtn() {
  if (nilp(vm.s))
    vm.s = vm_pop_d();
  else
    CONS(vm.s)->cdr = vm_pop_d();
  vm.e = vm_pop_d();
  vm.c = vm_pop_d();
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

static void vm_ld() {
  ref_t loc = vm_pop_c();
  if (!(consp(loc) && fixnump(car(loc)) && fixnump(cdr(loc))))
    error("invalid environment location");
  vm_push_s(locate(FIXNUM(car(loc)), FIXNUM(cdr(loc))));
}

static void vm_ap() {
  vm_save();
  vm.e = make_cons();
  CONS(vm.e)->car = car(cdr(vm.s));
  CONS(vm.e)->cdr = cdr(car(vm.s));
  vm.c = car(car(vm.s));
  vm.s = NIL;
}

static void check_fixnum(ref_t x, ref_t y) {
  if (!(fixnump(x) && fixnump(y)))
    error("arguments must be fixnums");
}

static void vm_add() {
  ref_t x = vm_pop_s(), y = vm_pop_s();
  check_fixnum(x, y);
  vm_push_s(x + y);
}

static void vm_sub() {
  ref_t x = vm_pop_s(), y = vm_pop_s();
  check_fixnum(x, y);
  vm_push_s(x - y);
}

static void vm_mul() {
  ref_t x = vm_pop_s(), y = vm_pop_s();
  check_fixnum(x, y);
  vm_push_s(make_fixnum(FIXNUM(x) * FIXNUM(y)));
}

static void vm_div() {
  ref_t x = vm_pop_s(), y = vm_pop_s();
  check_fixnum(x, y);
  vm_push_s(make_fixnum(FIXNUM(x) / FIXNUM(y)));
}

ref_t vm_op(const char *name) {
  size_t i = 0;
  while (ops[i].token != NULL) {
    if (strcmp(name, ops[i].token) == 0)
      return ops[i].op;
    i++;
  }
  error("invalid op code");
  return NIL;
}

void vm_do(ref_t opcode) {
  switch (opcode) {
  case NIL: vm_nil(); break;
  case OP_ADD: vm_add(); break;
  case OP_AP: vm_ap(); break;
  case OP_CONS: vm_cons(); break;
  case OP_DIV: vm_div(); break;
  case OP_GET: vm_get(); break;
  case OP_LD: vm_ld(); break;
  case OP_LDC: vm_ldc(); break;
  case OP_LDF: vm_ldf(); break;
  case OP_MUL: vm_mul(); break;
  case OP_PRINT: vm_print(); break;
  case OP_PUT: vm_put(); break;
  case OP_RCONS: vm_rcons(); break;
  case OP_RTN: vm_rtn(); break;
  case OP_SAVE: vm_save(); break;
  case OP_SUB: vm_sub(); break;

  case OP_ATOMP:
  case OP_CAR:
  case OP_CDR:
  case OP_EQ:
  case OP_JOIN:
  case OP_READ:
  case OP_SEL:
  default:
    error("unsupported opcode: 0x%.4lX", opcode);
  }
}
