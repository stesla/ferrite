#include <stdlib.h>
#include <stdio.h>
#include "error.h"

void error(const char *format) {
  fprintf(stderr, "%s", format);
  exit(1);
}
