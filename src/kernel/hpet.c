#include "hpet.h"

#include "log.h"
#include "utils.h"
#include "vmm.h"

static hpet_t* hpet;
static uintptr_t address;
static uint64_t period;

void hpet_init(void)
{
    hpet = (hpet_t*)acpi_lookup("HPET");
    LOG_ASSERT(hpet != NULL, "Unable to find hpet, hardware is not compatible");

    address = (uintptr_t)vmm_kernel_map(NULL, (void*)hpet->address, PAGE_SIZE);
    period = hpet_read(HPET_GENERAL_CAPABILITIES) >> HPET_COUNTER_CLOCK_OFFSET;

    hpet_write(HPET_GENERAL_CONFIG, HPET_CFG_DISABLE);
    hpet_write(HPET_MAIN_COUNTER_VALUE, 0);
    hpet_write(HPET_GENERAL_CONFIG, HPET_CFG_ENABLE);
}

uint64_t hpet_read_counter(void)
{
    return hpet_read(HPET_MAIN_COUNTER_VALUE);
}

void hpet_reset_counter(void)
{
    hpet_write(HPET_GENERAL_CONFIG, HPET_CFG_DISABLE);
    hpet_write(HPET_MAIN_COUNTER_VALUE, 0);
    hpet_write(HPET_GENERAL_CONFIG, HPET_CFG_ENABLE);
}

uint64_t hpet_nanoseconds_per_tick(void)
{
    return period / 1000000;
}

void hpet_write(uint64_t reg, uint64_t value)
{
    WRITE_64(address + reg, value);
}

uint64_t hpet_read(uint64_t reg)
{
    return READ_64(address + reg);
}

void hpet_sleep(uint64_t milliseconds)
{
    uint64_t target = hpet_read(HPET_MAIN_COUNTER_VALUE) + (milliseconds * 1000000000000) / period;
    while (!(hpet_read(HPET_MAIN_COUNTER_VALUE) >= target))
    {
        asm volatile("pause");
    }
}

void hpet_nanosleep(uint64_t nanoseconds)
{
    uint64_t target = hpet_read(HPET_MAIN_COUNTER_VALUE) + (nanoseconds * 1000000) / period;
    while (!(hpet_read(HPET_MAIN_COUNTER_VALUE) >= target))
    {
        asm volatile("pause");
    }
}
