#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "gc.h"
#include "object.h"
#include "read.h"
#include "vm.h"

#define READ_BYTE 0x32

static char read_byte(int fd) {
  if (car(vm.s) == READ_BYTE)
    vm_pop_s();
  else {
    vm_push_s(make_fixnum(fd));
    vm_do(OP_GET);
  }
  return CHAR(car(vm.s));
}

static void read_sexp(int fd);

#define LIST_START 0x2822 /* '(' */
#define LIST_END   0x2922 /* ')' */
#define LIST_DOT   0x2E22 /* '.' */
#define BEGIN_HEX  0x7822 /* 'x' */

static void read_list(int fd) {
  while (car(vm.s) != LIST_END)
    read_sexp(fd);
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

static void read_string_escape(int fd) {
  char ch;
  vm_pop_s();
  ch = read_byte(fd);
  vm_pop_s();
  switch (ch) {
  case 'f': ch = '\f'; break;
  case 'n': ch = '\n'; break;
  case 'r': ch = '\r'; break;
  case 't': ch = '\t'; break;
  case 'v': ch = '\v'; break;
  case '"': break;
  default:
    error("invalid string escape");
  }
  vm_push_s(make_char(ch));
}

static void read_string(int fd) {
  char ch;
  size_t size = 0;
  vm_pop_s(); /* get rid of open quote */
  ch = read_byte(fd);
  while (ch != '"') {
    ch = read_byte(fd);
    if (ch == '\\')
      read_string_escape(fd);
    size++;
  }
  build_string(size);
}

static void read_number(int fd) {
  char ch;
  size_t size = 1; /* the top of the stack has our first digit */
  ch = read_byte(fd);
  while (isdigit(ch)) {
    ch = read_byte(fd);
    size++;
  }
  build_string(size);
  CONS(vm.s)->car = make_fixnum(atoi(STRING(car(vm.s))->bytes));
  vm_push_s(make_char(ch));
  vm_push_s(READ_BYTE);
}

static void read_identifier(int fd) {
  char ch;
  size_t size = 1; /* the top of the stack has our first char */
  ch = read_byte(fd);
  while (isalpha(ch)) {
    ch = read_byte(fd);
    size++;
  }
  build_string(size);
  CONS(vm.s)->car = vm_op(STRING(car(vm.s))->bytes);
  vm_push_s(make_char(ch));
  vm_push_s(READ_BYTE);
}

static void read_atom(int fd) {
  char ch = CHAR(car(vm.s));
  if (ch == '"')
    read_string(fd);
  else if (isdigit(ch))
    read_number(fd);
  else if (isalpha(ch))
    read_identifier(fd);
}

static void read_comment(int fd) {
  char ch = CHAR(car(vm.s));
  while(ch != '\n') {
    vm_pop_s();
    ch = read_byte(fd);
  }
  vm_pop_s();
  read_sexp(fd);
}

static void check_eof_in_list() {
  ref_t s = vm.s;
  while (car(s) != LIST_START && !nilp(cdr(s)))
    s = cdr(s);
  if (car(s) == LIST_START)
    error("end of file reached before end of list");
}

static void read_character(int fd) {
  char ch;
  vm_pop_s();
  ch = read_byte(fd);
  if (ch == 'x') {
    char byte[3];
    vm_pop_s();
    byte[0] = read_byte(fd);
    vm_pop_s();
    byte[1] = read_byte(fd);
    vm_pop_s();
    byte[2] = 0;
    errno = 0;
    ch = (char) strtol(byte, NULL, 16);
    if (errno != 0)
      error("invalid escape \"\\x%s\"", byte);
    vm_push_s(make_char(ch));
  }
  else
    /* Do nothing. The one character we read is what we want to leave
       on the stack */
    ;
}

static void read_special(int fd) {
  char ch;
  vm_pop_s();
  ch = read_byte(fd);
  if (ch == '\\')
    read_character(fd);
  else if (ch == '!')
    read_comment(fd);
  else if (ch == 'f') {
    vm_pop_s();
    vm_push_s(FALSE);
  }
  else if (ch == 't') {
    vm_pop_s();
    vm_push_s(TRUE);
  }
  else
    error("invalid syntax at byte 0x%.2X", ch);
}

static void read_sexp(int fd) {
  char ch = read_byte(fd);
  while (isspace(ch)) {
    vm_pop_s();
    ch = read_byte(fd);
  }

  if (ch == ';')
    read_comment(fd);
  else if (ch == '#')
    read_special(fd);
  else if (ch == '(')
    read_list(fd);
  else if (ch == EOF)
    check_eof_in_list();
  else if (ch != ')')
    read_atom(fd);
}

void fe_read(int fd) {
  vm_do(OP_SAVE);
  vm.s = NIL;
  read_sexp(fd);
  vm_do(OP_RTN);
}
