#pragma once

#include "types/types.h"

#define ICW1_ICW4 0x01		    /* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08		    /* Level triggered (edge) mode */
#define ICW1_INIT 0x10		    /* Initialization - required! */
 
#define ICW4_8086 0x01		    /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02		    /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C    /* Buffered mode/master */
#define ICW4_SFNM 0x10		    /* Special fully nested (not) */

void io_outb(uint16_t port, uint8_t val);

uint8_t io_inb(uint16_t port);

void io_wait();