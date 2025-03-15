#ifndef MEM_ERR_H_
#define MEM_ERR_H_
#include <stdint.h>

#define M_mem_err_module(module_label) \
  MemoryErrorModule module_label;      \
  mem_err_module_init(&module_label);

typedef struct MemoryErrorModule MemoryErrorModule;

typedef enum {
  MemoryErrorModuleNoError = 0,
  MemoryErrorModuleAllocationFailed,
  MemoryErrorModuleMaxmumCapacityExceeded,
} AllocationResult;

struct MemoryErrorModule {
  uint8_t NoError;
  uint8_t AllocationFailed;
  uint8_t MaximumCapacityExceeded;
};

void mem_err_module_init(MemoryErrorModule *);

#endif // !MEM_ERR_H_

#ifdef MEM_ERR_IMPLEMENTATION

void mem_err_module_init(MemoryErrorModule *self)
{
  self->NoError = MemoryErrorModuleNoError;
  self->AllocationFailed = MemoryErrorModuleAllocationFailed;
  self->MaximumCapacityExceeded = MemoryErrorModuleMaxmumCapacityExceeded;
}

#endif // !MEM_ERR_IMPLEMENTATION
