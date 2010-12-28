#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "error.h"
#include "gc.h"
#include "read.h"
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
static void vm_push_e(ref_t ref) { vm_push(&vm.e, ref); }
void vm_push_s(ref_t ref) { vm_push(&vm.s, ref); }

static void vm_save() {
  vm_push_d(vm.c);
  vm_push_d(vm.e);
  vm_push_d(vm.s);
}

static char vm_read_byte(int fd) {
  char ch;
  ssize_t len = read(fd, &ch, 1);
  if (len < 0) {
    switch (errno) {
    case EAGAIN: /* EWOULDBLOCK */
    case EINTR:
      return vm_read_byte(fd);
    default:
      error("GET: %s", strerror(errno));
    }
  } else if (len == 0)
    ch = EOF;
  return ch;
}

static void vm_get() {
  ref_t f = vm_pop_s();
  int fd = fixnum_to_int(f);
  char ch = vm_read_byte(fd);
  vm_push_s(make_char(ch));
}

static void vm_write_bytes(int fd, const char *bytes, size_t len) {
  ssize_t written = write(fd, bytes, len);
  if (written < 0) {
    switch (errno) {
    case EAGAIN: /* EWOULDBLOCK */
    case EINTR:
      vm_write_bytes(fd, bytes, len);
      break;
    default:
      error("PUT: %s", strerror(errno));
    }
  } else if (written < len) {
    vm_write_bytes(fd, bytes + written, len - written);
  }
}

static void vm_put() {
  int fd = fixnum_to_int(vm_pop_s());
  ref_t value = vm_pop_s();
  if (charp(value)) {
    char ch = CHAR(value);
    vm_write_bytes(fd, &ch, 1);
  } else if (stringp(value))
    vm_write_bytes(fd, STRING(value)->bytes, strlen(STRING(value)->bytes));
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
  {"DUM", OP_DUM},
  {"RAP", OP_RAP},
  {"POP", OP_POP},
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
  error("invalid op code: %x", value);
  return NULL;
}

static void print(FILE *out, ref_t value) {
  if (nilp(value))
    fprintf(out, "NIL");
  else if(truep(value))
    fprintf(out, "#t");
  else if(falsep(value))
    fprintf(out, "#f");
  else if(charp(value)) {
    char ch = CHAR(value);
    if (0x20 < ch && ch < 0x7F)
      fprintf(out, "#\\%c", ch);
    else
      fprintf(out, "#\\x%.2X", (int) ch);
  }
  else if(fixnump(value))
    fprintf(out, "%d", FIXNUM(value));
  else if(stringp(value))
    fprintf(out, "\"%s\"", STRING(value)->bytes);
  else if(opcodep(value))
    fprintf(out, "%s", vm_opcode_name(value));
  else if(consp(value)) {
    fputc('(', out);
    while (consp(value)) {
      print(out, car(value));
      value = cdr(value);
      if (!nilp(value))
        fputc(' ', out);
    }
    if (!nilp(value)) {
      fprintf(out, ". ");
      print(out, value);
    }
    fputc(')', out);
  }
}

static void vm_print() {
  int fd = fixnum_to_int(vm_pop_s());
  ref_t value = vm_pop_s();
  FILE *out = fdopen(fd, "w");
  print(out, value);
}

static void vm_read() {
  int fd = fixnum_to_int(vm_pop_s());
  fe_read(fd);
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
  vm_push_d(vm.c);
  vm_push_d(vm.e);
  vm_push_d(cdr(cdr(vm.s)));
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

static void vm_sel() {
  ref_t cond;
  cond = vm_pop_s();
  if (!boolp(cond))
    error("invalid condition for SEL");
  vm_push_d(cdr(cdr(vm.c)));
  if (cond == TRUE)
    vm.c = vm_pop_c();
  else {
    vm_pop_c();
    vm.c = vm_pop_c();
  }
}

static void vm_join () {
  vm.c = vm_pop_d();
}

static void vm_eq () {
  ref_t x = vm_pop_s(), y = vm_pop_s();
  if (x == y)
    vm_push_s(TRUE);
  else
    vm_push_s(FALSE);
}

static void vm_dum () {
  vm_push_e(PENDING);
}

static void vm_rap () {
  if (car(vm.e) != PENDING)
    error("RAP called without first calling DUM");
  vm_push_d(vm.c);
  vm_push_d(cdr(vm.e));
  vm_push_d(cdr(cdr(vm.s)));
  CONS(vm.e)->car = car(cdr(vm.s));
  vm.e = make_cons();
  CONS(vm.e)->car = car(cdr(vm.s));
  CONS(vm.e)->cdr = cdr(car(vm.s));
  vm.c = car(car(vm.s));
  vm.s = NIL;
}

static void vm_car() {
  ref_t value = car(vm.s);
  if (nilp(value))
    return;
  else if (consp(value))
    CONS(vm.s)->car = car(car(vm.s));
  else
    error("CAR must be called with a list");
}

static void vm_cdr() {
  ref_t value = car(vm.s);
  if (nilp(value))
    return;
  else if (consp(value))
    CONS(vm.s)->car = cdr(car(vm.s));
  else
    error("CDR must be called with a list");
}

static void vm_atomp() {
  CONS(vm.s)->car = consp(car(vm.s)) ? FALSE : TRUE;
}

ref_t vm_op(const char *name) {
  size_t i = 0;
  while (ops[i].token != NULL) {
    if (strcmp(name, ops[i].token) == 0)
      return ops[i].op;
    i++;
  }
  error("invalid op code: %s", name);
  return NIL;
}

void vm_do(ref_t opcode) {
  switch (opcode) {
  case NIL: vm_nil(); break;
  case OP_ADD: vm_add(); break;
  case OP_AP: vm_ap(); break;
  case OP_ATOMP: vm_atomp(); break;
  case OP_CAR: vm_car(); break;
  case OP_CDR: vm_cdr(); break;
  case OP_CONS: vm_cons(); break;
  case OP_DIV: vm_div(); break;
  case OP_DUM: vm_dum(); break;
  case OP_EQ: vm_eq(); break;
  case OP_GET: vm_get(); break;
  case OP_JOIN: vm_join(); break;
  case OP_LD: vm_ld(); break;
  case OP_LDC: vm_ldc(); break;
  case OP_LDF: vm_ldf(); break;
  case OP_MUL: vm_mul(); break;
  case OP_POP: vm_pop_s(); break;
  case OP_PRINT: vm_print(); break;
  case OP_PUT: vm_put(); break;
  case OP_RAP: vm_rap(); break;
  case OP_READ: vm_read(); break;
  case OP_RCONS: vm_rcons(); break;
  case OP_RTN: vm_rtn(); break;
  case OP_SAVE: vm_save(); break;
  case OP_SEL: vm_sel(); break;
  case OP_SUB: vm_sub(); break;
  default:
    error("unsupported opcode: 0x%.4lX", opcode);
  }
}
