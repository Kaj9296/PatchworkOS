#ifndef _STDLIB_H
#define _STDLIB_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include "auxiliary/config.h"

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

_EXPORT _NORETURN void exit(int status);

_EXPORT char* lltoa(long long number, char* str, int base);
#define ltoa(number, str, base) lltoa(number, str, base)
#define itoa(number, str, base) lltoa(number, str, base)

_EXPORT char* ulltoa(unsigned long long number, char* str, int base);
#define ultoa(number, str, base) lltoa(number, str, base)
#define uitoa(number, str, base) lltoa(number, str, base)

#if defined(__cplusplus)
}
#endif

#endif