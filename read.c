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
    vm_do(OP_GET);
  }
  return CHAR(car(vm.s));
}

static void read_sexp();

#define LIST_START 0x2822 /* ref encoded '(' */
#define LIST_END   0x2922 /* ref encoded ')' */
#define LIST_DOT   0x2E22 /* ref encoded '.' */

static void read_list() {
  while (car(vm.s) != LIST_END)
    read_sexp();
  vm_pop_s();
  if (car(vm.s) == LIST_DOT)
    error("invalid use of list dot notation");
  if (car(cdr(vm.s)) != LIST_DOT)
    vm_do(NIL);
  else
    /* get rid of the LIST_DOT */
    CONS(vm.s)->cdr = cdr(cdr(vm.s));
  while(car(cdr(vm.s)) != LIST_START) {
    if (car(cdr(vm.s)) == LIST_DOT)
      error("invalid use of list dot notation");
    vm_do(OP_RCONS);
  }
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

static void read_string() {
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

static void read_number() {
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

static void read_identifier() {
  char ch;
  size_t size = 1; /* the top of the stack has our first char */
  ch = read_byte();
  while (isalpha(ch)) {
    ch = read_byte();
    size++;
  }
  build_string(size);
  CONS(vm.s)->car = vm_op(STRING(car(vm.s))->bytes);
  vm_push_s(make_char(ch));
  vm_push_s(READ_BYTE);
}

static void read_atom() {
  char ch = CHAR(car(vm.s));
  if (ch == '"')
    read_string();
  else if (isdigit(ch))
    read_number();
  else if (isalpha(ch))
    read_identifier();
}

static void read_comment() {
  char ch = CHAR(car(vm.s));
  while(ch != '\n')
    ch = read_byte();
  vm_pop_s();
  read_sexp();
}

static void check_eof_in_list() {
  ref_t s = vm.s;
  while (car(s) != LIST_START && !nilp(cdr(s)))
    s = cdr(s);
  if (car(s) == LIST_START)
    error("end of file reached before end of list");
}

static void read_sexp() {
  char ch = read_byte();
  while (isspace(ch)) {
    vm_pop_s();
    ch = read_byte();
  }

  if (ch == ';')
    read_comment();
  else if (ch == '(')
    read_list();
  else if (ch == EOF)
    check_eof_in_list();
  else if (ch != ')')
    read_atom();
}

void fe_read() {
  vm_do(OP_SAVE);
  vm.s = NIL;
  read_sexp();
  vm_do(OP_RTN);
}
