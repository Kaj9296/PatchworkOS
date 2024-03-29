#ifndef _CTYPE_H
#define _CTYPE_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include "auxiliary/config.h"

#define isalnum(ch) (isdigit((ch)) || isalpha((ch)))
#define isalpha(ch) (((ch) >= 'A' && (ch) <= 'Z') || ((ch) >= 'a' && (ch) <= 'z'))
#define isdigit(ch) (((ch) >= '0' && (ch) <= '9'))

#if defined(__cplusplus)
}
#endif

#endif