#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gc.h"
#include "error.h"

#define ALIGNED_SIZE(size) (((size) + LOWTAG_MASK) & ~LOWTAG_MASK)

static void *gc_alloc(size_t bytes) {
  void *result;
  if ((result = malloc(ALIGNED_SIZE(bytes))) == NULL) {
    fprintf(stderr, "out of memory");
    abort();
  }
  return result;
}

static inline ref_t make_ref(void *obj, uint8_t lowtag) {
  return ((ref_t) obj) + lowtag;
}

ref_t gc_lookup(ref_t old) {
  return old;
}

ref_t make_char(char c) {
  return (((ref_t) c) << 8) + TAG_CHAR;
}

ref_t make_cons() {
  struct cons *obj = gc_alloc(sizeof(struct cons));
  obj->tag = OBJ_CONS;
  obj->car = obj->cdr = NIL;
  return make_ref(obj, LOWTAG_CONS);
}

#define FIXNUM_MAX  536870911
#define FIXNUM_MIN -536870912

ref_t make_fixnum(int i) {
  if (FIXNUM_MIN <= i && i <= FIXNUM_MAX)
    return i << 2;
  error("out of range");
  return NIL;
}

ref_t make_string(char *s, size_t size) {
  if (s != NULL)
    size = strlen(s);
  struct string *obj = gc_alloc(sizeof(struct string) + size);
  obj->tag = OBJ_STRING;
  if (s != NULL)
    strcpy(obj->bytes, s);
  else
    memset(obj->bytes, 0, size);
  return make_ref(obj, LOWTAG_STRING);
}
