#include "syscall.h"

#include <string.h>
#include <errno.h>

#include "defs/defs.h"
#include "tty/tty.h"
#include "heap/heap.h"
#include "vmm/vmm.h"
#include "time/time.h"
#include "utils/utils.h"
#include "smp/smp.h"
#include "debug/debug.h"
#include "traps/traps.h"
#include "sched/sched.h"
#include "loader/loader.h"

//TODO: Improve verify funcs, improve multithreading string safety

static inline bool verify_pointer(const void* pointer, uint64_t size)
{
    if ((uint64_t)pointer > VMM_LOWER_HALF_MAX || (uint64_t)pointer + size > VMM_LOWER_HALF_MAX)
    {
        return false;
    }

    return true;
}

static inline bool verify_string(const char* string)
{
    return verify_pointer(string, strlen(string));
}

///////////////////////////////////////////////////////

void syscall_process_exit(uint64_t status)
{
    sched_process_exit(status);
}

uint64_t syscall_spawn(const char* path)
{
    if (!verify_string(path))
    {
        return ERROR(EFAULT);
    }

    return sched_spawn(path);
}

uint64_t syscall_sleep(uint64_t nanoseconds)
{
    sched_sleep(nanoseconds);
    return 0;
}

void* syscall_allocate(void* address, uint64_t length)
{
    space_allocate(sched_process()->space, address, SIZE_IN_PAGES(length));
    return address;
}

errno_t syscall_kernel_errno(void)
{
    return sched_thread()->error;
}

fd_t syscall_open(const char* path, uint8_t flags)
{
    if (!verify_string(path))
    {
        return ERROR(EFAULT);
    }

    return vfs_open(path, flags);
}

uint64_t syscall_close(fd_t fd)
{
    return vfs_close(fd);
}

uint64_t syscall_read(fd_t fd, void* buffer, uint64_t count)
{
    if (!verify_pointer(buffer, count))
    {
        return ERROR(EFAULT);
    }

    return vfs_read(fd, buffer, count);
}

uint64_t syscall_write(fd_t fd, const void* buffer, uint64_t count)
{
    if (!verify_pointer(buffer, count))
    {
        return ERROR(EFAULT);
    }

    return vfs_write(fd, buffer, count);
}

uint64_t syscall_seek(fd_t fd, int64_t offset, uint8_t origin)
{
    return vfs_seek(fd, offset, origin);
}

uint64_t syscall_test(const char* string)
{
    tty_acquire();

    uint8_t cpuId = smp_self_unsafe()->id;

    uint8_t oldRow = tty_get_row();
    uint8_t oldColumn = tty_get_column();

    tty_set_column(0);
    tty_set_row(cpuId);

    tty_print("CPU: ");
    tty_printi(cpuId);
    tty_print(" THREAD AMOUNT: ");
    tty_printi(sched_local_thread_amount());
    tty_print(" PID: ");
    tty_printi(sched_process()->id);
    tty_print(" TID: ");
    tty_printi(sched_thread()->id);
    if (string != 0)
    {
        tty_print(" STRING: ");
        tty_print(string);
    }
    tty_print(" TIME: ");
    tty_printi(time_milliseconds());
    tty_print(" MS                  ");

    tty_set_row(oldRow);
    tty_set_column(oldColumn);

    tty_release();

    return 0;
}

///////////////////////////////////////////////////////

void syscall_handler_end(void)
{
    if (sched_process()->killed)
    {
        sched_thread_exit();
    }

    sched_yield();
}

void* syscallTable[] =
{
    syscall_process_exit,
    syscall_spawn,
    syscall_sleep,
    syscall_allocate,
    syscall_kernel_errno,
    syscall_open,
    syscall_close,
    syscall_read,
    syscall_write,
    syscall_seek,
    syscall_test
};