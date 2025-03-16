#ifndef MEM_ERR_H_
#define MEM_ERR_H_
#include <stdint.h>

typedef enum {
  MemoryErrorModuleNoError = 0,
  MemoryErrorModuleAllocationFailed,
  MemoryErrorModuleMaxmumCapacityExceeded,
} AllocationResult;

typedef struct MemoryErrorModule MemoryErrorModule;

struct MemoryErrorModule {
  uint8_t NoError;
  uint8_t AllocationFailed;
  uint8_t MaximumCapacityExceeded;
};

extern MemoryErrorModule MemoryErrors;

#endif // !MEM_ERR_H_

#ifdef MEM_ERR_IMPLEMENTATION

MemoryErrorModule MemoryErrors = {
  .NoError = MemoryErrorModuleNoError,
  .AllocationFailed = MemoryErrorModuleAllocationFailed,
  .MaximumCapacityExceeded = MemoryErrorModuleMaxmumCapacityExceeded,
};

#endif // !MEM_ERR_IMPLEMENTATION
