#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>
#include <sys/types.h>

typedef long ref_t;

struct vm {
  ref_t s, e, c, d;
};

struct vm vm;

#define NIL 0x03
#define TRUE 0x0A
#define FALSE 0x0E

typedef enum {
  /* values */
  LD = 0x012,
  LDC = 0x032,
  LDF = 0x052,

  /* predicates */
  EQ = 0x072,
  ATOMP = 0x092,

  /* list structure */
  CONS = 0x0B2,
  CAR = 0x0D2,
  CDR = 0x0F2,

  /* math */
  ADD = 0x112,
  SUB = 0x132,
  MUL = 0x152,
  DIV = 0x192,

  /* input / output */
  GET = 0x1B2,
  PUT = 0x1D2,
  PRINT = 0x1F2,
  READ = 0x212,

  /* conditional */
  SEL = 0x232,
  JOIN = 0x252,

  /* function application */
  AP = 0x272,
  RTN = 0x292
} op_code;

#endif TYPES_H
