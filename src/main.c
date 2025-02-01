#include <stdio.h> 
#include "../include/allocator.h"

int main(void) {
  Allocator alloc = { .context = NULL };
  printf("Hello World!");
  return 0;
}
