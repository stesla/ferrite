#ifndef OBJECT_H
#define OBJECT_H

#include "types.h"

bool charp(ref_t value);
bool consp(ref_t value);
bool fixnump(ref_t value);
bool nilp(ref_t value);
bool stringp(ref_t value);

ref_t car(ref_t c);
ref_t cdr(ref_t c);

int fixnum_to_int(ref_t value);

#endif
