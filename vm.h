#ifndef VM_H
#define VM_H

#include "types.h"

struct vm {
  ref_t s, e, c, d;
};

struct vm vm;

ref_t vm_pop_c();
ref_t vm_pop_s();
void vm_push_c(ref_t ref);
void vm_push_s(ref_t ref);
void vm_cons();
void vm_ldc();
void vm_print();

#endif
