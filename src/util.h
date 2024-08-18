#ifndef UTIL_H
#define UTIL_H

#include <assert.h>


// TODO: disable assertions and debug-only macros in shipping builds

#define UTIL_ASSERT(COND) assert(COND)

#define UTIL_UNREACHABLE() do{ assert("This code should never be reached!" == 0); } while (0)

#define UTIL_ARRNUM(ARR) (sizeof(ARR)/sizeof(ARR[0]))

#define UTIL_DEBUGBLOCK if (true)

// UTIL_STATIC_ASSERT
#define _UTIL_STATIC_ASSERT_3(COND,MSG) typedef char STATIC_ASSERTION_##MSG[(COND) ? 1 : -1]
#define _UTIL_STATIC_ASSERT_2(COND,L) _UTIL_STATIC_ASSERT_3(COND,AT_LINE_##L)
#define _UTIL_STATIC_ASSERT_1(COND,L) _UTIL_STATIC_ASSERT_2(COND,L)
#define UTIL_STATIC_ASSERT(COND) _UTIL_STATIC_ASSERT_1(COND,__LINE__)

#define UTIL_MIN(A, B) (((A) < (B)) ? (A) : (B))

#define UTIL_MAX(A, B) (((A) > (B)) ? (A) : (B))


#endif //UTIL_H

