%define GET_LOADED_ADDRESS(address) address - smp_trampoline_start + SMP_TRAMPOLINE_LOAD_START 

SMP_TRAMPOLINE_LOAD_START equ 0x1000
SMP_TRAMPOLINE_DATA_PAGE_DIRECTORY equ 0x500
SMP_TRAMPOLINE_DATA_GDT equ 0x510
SMP_TRAMPOLINE_DATA_IDT equ 0x520

extern readyCpuAmount
extern gdt_load

global smp_trampoline_start
global smp_trampoline_end

[bits 16]
smp_trampoline_start:
    cli
    mov ax, 0x0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    o32 lgdt [GET_LOADED_ADDRESS(protected_mode_gdtr)]

    mov eax, cr0
    or al, 0x1
    mov cr0, eax

    jmp 0x8:(GET_LOADED_ADDRESS(smp_trampoline_protected_mode_start))

[bits 32]
smp_trampoline_protected_mode_start:
    mov bx, 0x10
    mov ds, bx
    mov es, bx
    mov ss, bx

    mov eax, cr4
    or  eax, (1 << 5)
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or  eax, (1 << 8)
    wrmsr

    mov eax, dword [SMP_TRAMPOLINE_DATA_PAGE_DIRECTORY]
    mov cr3, eax

    mov edx, cr0
    or eax, (1 << 31) | (1 << 0)
    mov cr0, eax

    lgdt [GET_LOADED_ADDRESS(long_mode_gdtr)]

    jmp 0x8:(GET_LOADED_ADDRESS(smp_trampoline_long_mode_start))

[bits 64]
smp_trampoline_long_mode_start:    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov ax, 0x0
    mov fs, ax
    mov gs, ax

    inc byte [0x100]
    hlt
    
align 16
protected_mode_gdtr:
    dw protected_mode_gdt_end - protected_mode_gdt_start - 1
    dd GET_LOADED_ADDRESS(protected_mode_gdt_start)
align 16
protected_mode_gdt_start:
    dq 0
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
protected_mode_gdt_end:

align 16
long_mode_gdtr:
    dw long_mode_gdt_end - long_mode_gdt_start - 1
    dq GET_LOADED_ADDRESS(long_mode_gdt_start)
align 16
long_mode_gdt_start:
    dq 0
    dq 0x00AF98000000FFFF
    dq 0x00CF92000000FFFF
long_mode_gdt_end:

smp_trampoline_end: