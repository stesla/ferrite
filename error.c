#include <stdlib.h>
#include <stdio.h>
#include "error.h"

void error(const char *format) {
  fprintf(stderr, "%s\n", format);
  exit(1);
}
