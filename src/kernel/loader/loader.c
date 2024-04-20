#include "loader.h"

#include <string.h>

#include <internal/syscalls/syscalls.h>

#include <common/elf/elf.h>

#include "heap/heap.h"
#include "defs/defs.h"
#include "pmm/pmm.h"
#include "vmm/vmm.h"
#include "sched/sched.h"
#include "utils/utils.h"
#include "debug/debug.h"
#include "vfs/utils/utils.h"

static void* loader_allocate_stack()
{    
    Thread* thread = sched_thread();
    void* address = (void*)(VMM_LOWER_HALF_MAX - (CONFIG_USER_STACK * (thread->id + 1) + PAGE_SIZE * (thread->id)));
    if (vmm_allocate(address, CONFIG_USER_STACK) == NULL)
    {
        debug_panic("Failed to allocate user stack!");
    }
    else
    {
        return address + CONFIG_USER_STACK;
    }
}

static void* loader_load_program()
{   
    const char* executable = sched_process()->executable;
    File* file = vfs_open(executable);
    if (file == NULL)
    {
        sched_process_exit(EEXEC);
    }

    char parentDir[CONFIG_MAX_PATH];
    vfs_parent_dir(parentDir, executable);
    if (vfs_chdir(parentDir) == ERR)
    {
        sched_process_exit(EEXEC);
    }

    ElfHeader header;
    if (FILE_CALL_METHOD(file, read, &header, sizeof(ElfHeader)) != sizeof(ElfHeader))
    {
        sched_process_exit(EEXEC);
    }
    if (header.ident[0] != 0x7F || header.ident[1] != 'E' || header.ident[2] != 'L' || header.ident[3] != 'F')
    {
        sched_process_exit(EEXEC);
    }

    for (uint64_t i = 0; i < header.programHeaderAmount; i++)
	{
        uint64_t offset = sizeof(ElfHeader) + header.programHeaderSize * i;
        if (FILE_CALL_METHOD(file, seek, offset, SEEK_SET) != offset)
        {
            sched_process_exit(EEXEC);
        }

        ElfProgramHeader programHeader;
        if (FILE_CALL_METHOD(file, read, &programHeader, sizeof(ElfProgramHeader)) != sizeof(ElfProgramHeader))
        {
            sched_process_exit(EEXEC);
        }

        switch (programHeader.type)
        {
	    case PT_LOAD:
        {
            if (vmm_allocate((void*)programHeader.virtualAddress, programHeader.memorySize + PAGE_SIZE) == NULL)
            {
                sched_process_exit(EEXEC);
            }

            if (FILE_CALL_METHOD(file, seek, programHeader.offset, SEEK_SET) != programHeader.offset)
            {
                sched_process_exit(EEXEC);
            }

            if (FILE_CALL_METHOD(file, read, (void*)programHeader.virtualAddress, programHeader.fileSize) != programHeader.fileSize)
            {
                sched_process_exit(EEXEC);
            }
		}
	    break;
		}
	}

    file_deref(file);
    return (void*)header.entry;
}

void loader_entry()
{
    void* rsp = loader_allocate_stack();
    void* rip = loader_load_program();

    loader_jump_to_user_space(rsp, rip);
}