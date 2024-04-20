#include <common/boot_info/boot_info.h>

#include <string.h>

#include "tty/tty.h"
#include "smp/smp.h"
#include "time/time.h"
#include "debug/debug.h"
#include "kernel/kernel.h"
#include "sched/sched.h"
#include "defs/defs.h"
#include "utils/utils.h"
#include "vfs/utils/utils.h"

void vfs_benchmark()
{
    for (uint64_t i = 0; i < 1000000; i++)
    {
        File* file = vfs_open("B:/test1/test2/test3/test.txt");
        if (file == NULL)
        {
            tty_print(strerror(sched_thread()->error));
        }

        char buffer[32];
        memset(buffer, 0, 32);
        if (FILE_CALL_METHOD(file, read, buffer, 31) == ERR)
        {
            tty_print(strerror(sched_thread()->error));
        }

        file_deref(file);
    }
}

void main(BootInfo* bootInfo)
{
    kernel_init(bootInfo);

    /*tty_print("OPEN: ");
    File* file = vfs_open("A:/framebuffer/0");
    if (file == NULL)
    {
        tty_print(strerror(sched_thread()->error));
    }
    else
    {
        tty_print("SUCCESS");
    }
    tty_print("\n");

    char buffer[32];
    memset(buffer, 0, 32);
    tty_print("READ: ");
    if (FILE_CALL_METHOD(file, read, buffer, 31) == ERR)
    {
        tty_print(strerror(sched_thread()->error));
    }
    else
    {
        tty_print(buffer);
    }
    tty_print("\n");

    tty_print("CLOSE: ");
    file_deref(file);
    tty_print("SUCCESS\n");*/

    //BENCHMARK(vfs_benchmark);

    tty_acquire();
    tty_clear();
    tty_set_row(smp_cpu_amount() + 2);
    tty_release();

    for (uint64_t i = 0; i < 2; i++)
    {
        sched_spawn("B:/programs/parent.elf");
    }

    //Exit init thread
    sched_thread_exit();
}