#ifndef GC_H
#define GC_H

#include "types.h"

#define LOWTAG_MASK 0x07
enum {
  LOWTAG_FIXNUM = 0x00,
  LOWTAG_CONS = 0x01,
  LOWTAG_FN = 0x03,
  LOWTAG_STRING = 0x05
};

enum {
  OBJ_CONS = 0x01,
  OBJ_STRING = 0x05
};

struct lispobj {
  uint8_t tag;
};

struct cons {
  uint8_t tag;
  ref_t car, cdr;
};

struct string {
  uint8_t tag;
  char bytes[1];
};

#define CONS(obj) ((struct cons *) ((obj) - LOWTAG_CONS))
#define FIXNUM(obj) (((int32_t) (obj)) >> 2)
#define STRING(obj) ((struct string *) ((obj) - LOWTAG_STRING))

ref_t gc_lookup(ref_t old);

ref_t make_cons();
ref_t make_fixnum(int i);
ref_t make_string(char *s);

ref_t get_car(ref_t c);
ref_t get_cdr(ref_t c);
void set_car(ref_t c, ref_t x);
void set_cdr(ref_t c, ref_t x);

#endif
