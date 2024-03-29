#include <sys/io.h>

#include "internal/syscalls/syscalls.h"

uint64_t read(fd_t fd, void* buffer, uint64_t count)
{
    uint64_t result = SYSCALL(SYS_READ, 3, fd, buffer, count);
    if (result == ERR)
    {
        errno = SYSCALL(SYS_KERNEL_ERRNO, 0);
    }
    return result;
}