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
    /** @todo replace errors with named Memory Errors */
    M_declare_test_start("arena");

    M_arena_module(Arenas);
    M_mem_err_module(MemoryErrors);

    Arena *arena = NULL;
    AllocationResult op_result = MemoryErrors.NoError;
    void *data_ptr = NULL;
    op_result = Arenas.arena_init(3, &arena);
    M_expect(op_result == MemoryErrors.NoError &&
                     Arenas.get_free_count() == 0 &&
                     Arenas.get_alloc_count() == 2,
             "expect 0 frees and 2 allocs on initialization");
    M_expect(arena->capacity == 3 && arena->cursor == 0 &&
                     arena->data != NULL && arena->next == NULL,
             "expect initialized arena values");

    op_result = Arenas.arena_alloc(arena, 2, &data_ptr);
    M_expect(op_result == MemoryErrors.NoError && data_ptr != NULL &&
             arena->capacity == 3 && arena->cursor == 2 &&
             arena->next == NULL,
                     "expected values for arena-based allocation of size 2");

    op_result = Arenas.arena_alloc(arena, 1, &data_ptr);

    M_expect(op_result == MemoryErrors.NoError && data_ptr != NULL &&
                     arena->cursor == 3,
             "additional small allocation from arena has cursor == capacity");

    op_result = Arenas.arena_alloc(arena, 2, &data_ptr);
    M_expect(
            op_result == MemoryErrors.NoError && data_ptr != NULL &&
                    arena->cursor == 3 && arena->capacity == 3,
            "expect previous arena's state to be unchanged after another allocation exceeds original's capacity");

    M_expect(arena->next->cursor == 2 && arena->next->capacity == 3 &&
                     arena->next->next == NULL,
             "expect next arena's state after another allocation of size 2");
    M_expect(Arenas.get_alloc_count() == 4,
             "expect 4 calls to alloc after another arena initialized");

    op_result = Arenas.arena_alloc(arena, 2, &data_ptr);
    M_expect(
            op_result == MemoryErrors.NoError && data_ptr != NULL &&
                    arena->cursor == 3 && arena->capacity == 3,
            "expect original arena's cursor to still be unchanged after yet another arena added");

    M_expect(
            arena->next->cursor == 2 && arena->next->capacity == 3,
            "expect previous arena's state to be unchanged after allocation of another arena");
    M_expect(arena->next->next->next == NULL &&
                     arena->next->next->cursor == 2 &&
                     arena->next->next->capacity == 3,
             "expect final arena's state");
    M_expect(Arenas.get_alloc_count() == 6,
             "expect 6 calls to alloc after final arena initialized");

    op_result = Arenas.arena_reset(arena);
    M_expect(op_result == MemoryErrors.NoError && arena->cursor == 0 &&
                     arena->next->cursor == 0 && arena->next->next->cursor == 0,
             "arena reset resets all cursors to 0, making them writable again");

    op_result = Arenas.arena_alloc(arena, 3, &data_ptr);
    op_result = Arenas.arena_alloc(arena, 1, &data_ptr);
    M_expect(
            op_result == MemoryErrors.NoError && data_ptr != NULL &&
                    arena->cursor == 3 && arena->next->cursor == 1 &&
                    arena->next->next->cursor == 0 &&
                    arena->next->next->next == NULL,
            "after reset no new arenas are created and memory is reused in the expected way");

    op_result = Arenas.arena_deinit(arena);

    M_expect(op_result == MemoryErrors.NoError &&
                     Arenas.get_alloc_count() == Arenas.get_free_count(),
             "expect call to free for every call to alloc");

    M_declare_test_end("arena");
  }
  if (tests == TEST_ALL || is_test(tests, TEST_BUMP)) {
    M_declare_test_start("arena");
    M_bump_module(Bumps);
    M_mem_err_module(MemoryErrors);

    Bump bump;
    size_t size = 3;
    char data[3] = { 0 };
    void *return_addr = NULL;
    AllocationResult op_result = MemoryErrors.NoError;

    op_result = Bumps.bump_init(&bump, data, size);
    M_expect(op_result == MemoryErrors.NoError && bump.data == data &&
                     bump.cursor == 0 && bump.capacity == 3,
             "bump should be initialized in expected way");
    op_result = Bumps.bump_alloc(&bump, 1, &return_addr);
    M_expect(op_result == MemoryErrors.NoError && bump.cursor == 1,
             "bump should increment cursor");
    op_result = Bumps.bump_alloc(&bump, 2, &return_addr);
    M_expect(op_result == MemoryErrors.NoError && bump.cursor == 3,
             "bump should increment cursor again");
    op_result = Bumps.bump_alloc(&bump, 1, &return_addr);
    M_expect(op_result == MemoryErrors.MaximumCapacityExceeded,
             "another bump should return NULL, because capacity was exceeded");
    op_result = Bumps.bump_reset(&bump);
    M_expect(op_result == MemoryErrors.NoError && bump.cursor == 0,
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
