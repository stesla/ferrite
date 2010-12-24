#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "error.h"

void error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
  exit(1);
}
