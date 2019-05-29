#ifndef __TESTER_HEADER_INCLUDED__
#define __TESTER_HEADER_INCLUDED__
#include <stdio.h>

#define TEST_CASE(test_name, test_body)                                        \
  static void test_name(void) {                                                \
    test_body;                                                                 \
    printf("[Test - OK] " #test_name "\n");                                    \
  }

#endif