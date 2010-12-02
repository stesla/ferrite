#ifndef VM_H
#define VM_H

#include "types.h"

struct vm {
  ref_t s, e, c, d;
};

struct vm vm;

ref_t vm_pop();
void vm_push(ref_t ref);
void vm_cons();

#endif
