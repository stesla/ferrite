#ifndef GC_H
#define GC_H

#include "types.h"

ref_t make_cons();
ref_t make_fixnum(int i);
ref_t make_string(char *s);

ref_t get_cdr();
void set_car(ref_t c, ref_t x);
void set_cdr(ref_t c, ref_t x);

#endif
