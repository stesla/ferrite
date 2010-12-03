#include <assert.h>
#include "gc.h"
#include "object.h"

bool charp(ref_t value) {
  return (value & TAG_MASK) == TAG_CHAR;
}

bool nilp(ref_t value) {
  return value == NIL;
}

bool fixnump(ref_t value) {
  return !(value & 3);
}

bool stringp(ref_t value) {
  return (value & LOWTAG_MASK) == LOWTAG_STRING;
}

bool consp(ref_t value) {
  return (value & LOWTAG_MASK) == LOWTAG_CONS;
}

bool opcodep(ref_t value) {
  return (value & TAG_MASK) == TAG_OPCODE;
}

ref_t car(ref_t c) {
  if (nilp(c))
    return NIL;
  assert(consp(c));
  return CONS(c)->car;
}

ref_t cdr(ref_t c) {
  if (nilp(c))
    return NIL;
  assert(consp(c));
  return CONS(c)->cdr;
}

int fixnum_to_int(ref_t value) {
  assert(fixnump(value));
  return FIXNUM(value);
}
