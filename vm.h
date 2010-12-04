#ifndef VM_H
#define VM_H

#include "types.h"

struct vm {
  ref_t s, e, c, d;
};

struct vm vm;

typedef enum {
  /* values */
  OP_LD = 0x0112,
  OP_LDC = 0x0212,
  OP_LDF = 0x0312,

  /* predicates */
  OP_EQ = 0x0412,
  OP_ATOMP = 0x0512,

  /* list structure */
  OP_CONS = 0x0612,
  OP_RCONS = 0x0712,
  OP_CAR = 0x0812,
  OP_CDR = 0x0912,

  /* math */
  OP_ADD = 0x0A12,
  OP_SUB = 0x0B12,
  OP_MUL = 0x0C12,
  OP_DIV = 0x0D12,

  /* input / output */
  OP_GET = 0x0E12,
  OP_PUT = 0x0F12,
  OP_PRINT = 0x1012,
  OP_READ = 0x1112,

  /* conditional */
  OP_SEL = 0x1212,
  OP_JOIN = 0x1312,

  /* function application */
  OP_AP = 0x1412,
  OP_RTN = 0x1512
} op_code;

ref_t vm_pop_c();
ref_t vm_pop_d();
ref_t vm_pop_s();

void vm_push_c(ref_t ref);
void vm_push_d(ref_t ref);
void vm_push_s(ref_t ref);

void vm_save_sec();

ref_t vm_op(const char *name);
void vm_do(ref_t opcode);

#endif
