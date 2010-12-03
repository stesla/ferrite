#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "error.h"
#include "gc.h"
#include "object.h"
#include "read.h"
#include "vm.h"

#define READ_BYTE 0x32

static char read_byte() {
  if (car(vm.s) == READ_BYTE)
    vm_pop_s();
  else {
    vm_push_s(make_fixnum(STDIN_FILENO));
    vm_get();
  }
  return CHAR(car(vm.s));
}

static void read_sexp();

#define LIST_START 0x2822 /* ref encoded '(' */
#define LIST_END   0x2922 /* ref encoded ')' */

void read_list() {
  while (car(vm.s) != LIST_END)
    read_sexp();
  vm_pop_s();
  vm_nil(); /* end of list */
  while(car(cdr(vm.s)) != LIST_START)
    vm_rcons();
  /* get rid of the LIST_START */
  CONS(vm.s)->cdr = cdr(cdr(vm.s));
}

static void build_string(size_t size) {
  CONS(vm.s)->car = make_string(NULL, size);
  for (; size > 0; size--) {
    STRING(car(vm.s))->bytes[size - 1] = CHAR(car(cdr(vm.s)));
    CONS(vm.s)->cdr = cdr(cdr(vm.s));
  }
}

void read_string() {
  char ch;
  size_t size = 0;
  vm_pop_s(); /* get rid of open quote */
  ch = read_byte();
  while (ch != '"') {
    ch = read_byte();
    size++;
  }
  build_string(size);
}

void read_number() {
  char ch;
  size_t size = 1; /* the top of the stack has our first digit */
  ch = read_byte();
  while (isdigit(ch)) {
    ch = read_byte();
    size++;
  }
  build_string(size);
  CONS(vm.s)->car = make_fixnum(atoi(STRING(car(vm.s))->bytes));
  vm_push_s(make_char(ch));
  vm_push_s(READ_BYTE);
}

void read_identifier() {
  char ch;
  size_t size = 1; /* the top of the stack has our first char */
  ch = read_byte();
  while (isalpha(ch)) {
    ch = read_byte();
    size++;
  }
  build_string(size);
  CONS(vm.s)->car = vm_op_with_name(STRING(car(vm.s))->bytes);
  vm_push_s(make_char(ch));
  vm_push_s(READ_BYTE);
}

void read_atom() {
  char ch = CHAR(car(vm.s));
  if (ch == '"')
    read_string();
  else if (isdigit(ch))
    read_number();
  else if (isalpha(ch))
    read_identifier();
}

static void read_sexp() {
  char ch = read_byte();
  while (isspace(ch)) {
    vm_pop_s();
    ch = read_byte();
  }

  if (ch == '(')
    read_list();
  else if (ch == EOF) {
    ref_t s = vm.s;
    while (car(s) != LIST_START && !nilp(cdr(s)))
      s = cdr(s);
    if (car(s) == LIST_START)
      error("end of file reached before end of list");
  }
  else if (ch != ')')
    read_atom();
}

void fe_read() {
  vm_save_sec();
  vm.s = NIL;
  read_sexp();
  vm_rtn();
}
