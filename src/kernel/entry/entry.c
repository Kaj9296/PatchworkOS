#include <common/boot_info/boot_info.h>

#include "tty/tty.h"
#include "smp/smp.h"
#include "time/time.h"
#include "debug/debug.h"
#include "kernel/kernel.h"
#include "scheduler/scheduler.h"
#include "types/types.h"

#include "vfs/utils/utils.h"

void main(BootInfo* bootInfo)
{
    kernel_init(bootInfo);

    tty_acquire();
    for (uint64_t i = 0; i < 16; i++)
    {
        scheduler_spawn("ram:/programs/parent.elf");
    }
    tty_clear();
    tty_set_row(smp_cpu_amount() * 2 + 2);
    tty_release();

    /*uint64_t fd = vfs_open("ram:/test1/test2/test3/test.txt", O_READ);
    tty_print("OPEN: ");
    if (fd == ERROR)
    {
        tty_printi(scheduler_thread()->errno);
    }
    else
    {
        tty_print("SUCCESS");
    }

    tty_print("\nCLOSE: ");
    if (vfs_close(fd))
    {
        tty_printi(scheduler_thread()->errno);
    }
    else
    {
        tty_print("SUCCESS");
    }*/

    //Exit init thread
    scheduler_thread_exit();
}
