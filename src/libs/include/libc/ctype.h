#ifndef _CTYPE_H
#define _CTYPE_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include "internal.h"

int isalnum(int ch);

int isalpha(int ch);

int isdigit(int ch);

#if defined(__cplusplus)
} /* extern "C" */
#endif
 
#endif