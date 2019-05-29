#include "tester.h"
#include <stdio.h>
#include <string.h>

typedef void (*TESTER_FUNC)();

typedef struct {
  char *tester_name;
  TESTER_FUNC tester_func;
} TESTER;

#define TESTER_ENTRY(TESTER_NAME)                                              \
  { .tester_name = #TESTER_NAME, .tester_func = TESTER_NAME##_test }

TESTER testers[] = {};

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

int main(int argc, const char *argv[]) {
  if (argc == 1) {
    printf("[tester] <RUN ALL TESTS>\n");
    for (size_t i = 0; i < ARRAY_LEN(testers); i++) {
      TESTER tester = testers[i];
      tester.tester_func();
    }
  } else {
    for (int i = 1; i < argc; i++) {
      for (size_t j = 0; j < ARRAY_LEN(testers); j++) {
        TESTER tester = testers[j];
        if (strcmp(argv[i], tester.tester_name) == 0) {
          printf("[tester] <RUN TEST FOR - %s>\n", tester.tester_name);
          tester.tester_func();
        }
      }
    }
  }

  return 0;
}
