#pragma once

#define SYSCALL_VECTOR 0x80

extern void* syscallTable[];

extern void syscall_handler(void);