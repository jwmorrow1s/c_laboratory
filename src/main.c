#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#if defined(PROJECT_TEST_BUILD)
#include <stdlib.h>
#include "../include/arena.h"
#include "../include/bump.h"
#include "../include/mem_err.h"
#endif

#define M_expect(cond, test_text)                            \
  do {                                                       \
    printf("%s#L%d %s: " test_text "\n", __FILE__, __LINE__, \
           (cond) ? "PASS" : (test_failures++, "FAIL"));     \
  } while (0)
#define M_declare_test_start(test)              \
  do {                                          \
    printf("BEGIN\t\t" test " test\n");         \
    printf("------------------------------\n"); \
  } while (0)
#define M_declare_test_end(test)                \
  do {                                          \
    printf("------------------------------\n"); \
    printf("END\t\t" test " test\n\n");         \
  } while (0)

static uint8_t test_failures = 0;

enum {
  TEST_ARENA = 1,
  TEST_BUMP = 2,
  TEST_ALL,
};

static uint8_t is_test(uint8_t check, uint8_t test_type)
{
  return check & test_type;
}

static uint8_t eql(const char *a, char *b)
{
  uint8_t iter = 0;
  while (a[iter] == b[iter] && a[iter] != '\0')
    iter++;
  return a[iter] == b[iter];
}

int main(int argc, char **argv)
{
#if defined(PROJECT_TEST_BUILD)

  uint8_t tests = 0;

  if (argc == 1)
    tests = TEST_ALL;
  for (int i = 1; i < argc; i++) {
    char *value = argv[i];
    if (eql("arena", value)) {
      tests |= TEST_ARENA;
    }
    if (eql("bump", value)) {
      tests |= TEST_BUMP;
    }
  }

  if (tests == TEST_ALL || is_test(tests, TEST_ARENA)) {
    M_declare_test_start("arena");

    Arena *arena = NULL;
    AllocationResult op_result = MemoryErrors.NoError;
    void *data_ptr = NULL;
    op_result = ModuleArena.arena_init(3, &arena);
    M_expect(op_result == MemoryErrors.NoError &&
                     ModuleArena.get_free_count() == 0 &&
                     ModuleArena.get_alloc_count() == 2,
             "expect 0 frees and 2 allocs on initialization");
    M_expect(arena->_capacity == 3 && arena->_cursor == 0 &&
                     arena->_data != NULL && arena->_next == NULL,
             "expect initialized arena values");

    op_result = ModuleArena.alloc(arena, 2, &data_ptr);
    M_expect(op_result == MemoryErrors.NoError && data_ptr != NULL &&
                     arena->_capacity == 3 && arena->_cursor == 2 &&
                     arena->_next == NULL,
             "expected values for arena-based allocation of size 2");

    op_result = ModuleArena.alloc(arena, 1, &data_ptr);

    M_expect(op_result == MemoryErrors.NoError && data_ptr != NULL &&
                     arena->_cursor == 3,
             "additional small allocation from arena has cursor == capacity");

    op_result = ModuleArena.alloc(arena, 2, &data_ptr);
    M_expect(
            op_result == MemoryErrors.NoError && data_ptr != NULL &&
                    arena->_cursor == 3 && arena->_capacity == 3,
            "expect previous arena's state to be unchanged after another allocation exceeds original's capacity");

    M_expect(arena->_next->_cursor == 2 && arena->_next->_capacity == 3 &&
                     arena->_next->_next == NULL,
             "expect next arena's state after another allocation of size 2");
    M_expect(ModuleArena.get_alloc_count() == 4,
             "expect 4 calls to alloc after another arena initialized");

    op_result = ModuleArena.alloc(arena, 2, &data_ptr);
    M_expect(
            op_result == MemoryErrors.NoError && data_ptr != NULL &&
                    arena->_cursor == 3 && arena->_capacity == 3,
            "expect original arena's cursor to still be unchanged after yet another arena added");

    M_expect(
            arena->_next->_cursor == 2 && arena->_next->_capacity == 3,
            "expect previous arena's state to be unchanged after allocation of another arena");
    M_expect(arena->_next->_next->_next == NULL &&
                     arena->_next->_next->_cursor == 2 &&
                     arena->_next->_next->_capacity == 3,
             "expect final arena's state");
    M_expect(ModuleArena.get_alloc_count() == 6,
             "expect 6 calls to alloc after final arena initialized");

    op_result = ModuleArena.reset(arena);
    M_expect(op_result == MemoryErrors.NoError && arena->_cursor == 0 &&
                     arena->_next->_cursor == 0 && arena->_next->_next->_cursor == 0,
             "arena reset resets all cursors to 0, making them writable again");

    op_result = ModuleArena.alloc(arena, 3, &data_ptr);
    op_result = ModuleArena.alloc(arena, 1, &data_ptr);
    M_expect(
            op_result == MemoryErrors.NoError && data_ptr != NULL &&
                    arena->_cursor == 3 && arena->_next->_cursor == 1 &&
                    arena->_next->_next->_cursor == 0 &&
                    arena->_next->_next->_next == NULL,
            "after reset no new arenas are created and memory is reused in the expected way");

    op_result = ModuleArena.arena_deinit(arena);

    M_expect(op_result == MemoryErrors.NoError &&
                     ModuleArena.get_alloc_count() == ModuleArena.get_free_count(),
             "expect call to free for every call to alloc");

    M_declare_test_end("arena");
  }
  if (tests == TEST_ALL || is_test(tests, TEST_BUMP)) {
    M_declare_test_start("arena");

    Bump bump;
    size_t size = 3;
    char data[3] = { 0 };
    void *return_addr = NULL;
    AllocationResult op_result = MemoryErrors.NoError;

    op_result = ModuleBump.bump_init(&bump, data, size);
    M_expect(op_result == MemoryErrors.NoError && bump._data == data &&
                     bump._cursor == 0 && bump._capacity == 3,
             "bump should be initialized in expected way");
    op_result = ModuleBump.alloc(&bump, 1, &return_addr);
    M_expect(op_result == MemoryErrors.NoError && bump._cursor == 1,
             "bump should increment cursor");
    op_result = ModuleBump.alloc(&bump, 2, &return_addr);
    M_expect(op_result == MemoryErrors.NoError && bump._cursor == 3,
             "bump should increment cursor again");
    op_result = ModuleBump.alloc(&bump, 1, &return_addr);
    M_expect(op_result == MemoryErrors.MaximumCapacityExceeded,
             "another bump should return NULL, because capacity was exceeded");
    op_result = ModuleBump.reset(&bump);
    M_expect(op_result == MemoryErrors.NoError && bump._cursor == 0,
             "bump should have 0 cursor after reset");

    M_declare_test_end("arena");
  }

  if (test_failures) {
    printf("[ATTN] There are test failures!\n");
    return 1;
  }

  return 0;
#else
  printf("[INFO] Not a test build\n");
  return 0;
#endif // PROJECT_TEST_BUILD
}
