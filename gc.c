#include <stdlib.h>
#include "gc.h"
#include "error.h"

/* static void *gc_alloc(size_t bytes) { */
/*   return NULL; */
/* } */

ref_t make_cons() {
  return NIL;
}

ref_t get_cdr() {
  return NIL;
}

void set_car(ref_t c, ref_t x) {
}

void set_cdr(ref_t c, ref_t x) {
}

#define FIXNUM_MAX  536870911
#define FIXNUM_MIN -536870912

ref_t make_fixnum(int i) {
  if (FIXNUM_MIN <= i && i <= FIXNUM_MAX)
    return i << 2;
  error("out of range");
  return NIL;
}

ref_t make_string(char *s) {
  return NIL;
}
