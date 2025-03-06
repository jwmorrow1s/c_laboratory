#ifndef MODULE_H_
#define MODULE_H_
#include <stdio.h>

/** START HEADER SECTION */

typedef struct Module Module;
void module_init(Module *self);

struct PublicFunctions {
  void (*print)(void);
};

struct Module {
  struct PublicFunctions *f;
};

/** END HEADER SECTION */

/*---------------------------------------------------------------------------*/

/** START IMPL SECTION */

#ifdef MODULE_IMPLEMENTATION

static void print(void)
{
  printf("private fn invoked!\n");
}

static struct PublicFunctions public_functions = (struct PublicFunctions){
  .print = &print,
};

void module_init(Module *self)
{
  self->f = &public_functions;
}

#endif /** MODULE_IMPLEMENTATION */

/** END IMPL SECTION */

/*---------------------------------------------------------------------------*/

#endif /** !MODULE_H_ */
