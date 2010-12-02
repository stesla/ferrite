#include "types.h"

void *gc_alloc(size_t bytes);
ref_t make_cons();
ref_t get_cdr();
void set_car(ref_t c, ref_t x);
void set_cdr(ref_t c, ref_t x);
