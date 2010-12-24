#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>
#include <sys/types.h>

typedef enum {
  NO = 0,
  YES = 1
} bool;

typedef unsigned long ref_t;

/* Parseable Values */
#define NIL 0x02
#define TRUE 0x0A
#define FALSE 0x0E

/* Non Parseable Values */
#define READ_BYTE 0x0032 /* used in read.c */
#define PENDING 0x0132   /* used in vm.c */

#endif
