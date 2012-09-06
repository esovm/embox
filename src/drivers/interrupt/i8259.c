/**
 * @file
 * @brief Interrupt controller driver for i8952 chip 9x86 platform.
 *
 * @details
 *   This driver believes that there are two i8952 chip in the system and
 *   slave is connected to master's second line.
 *   We also suppose that we use only x86 platform.
 *
 * @date 22.12.10
 * @author Nikolay Korotky
 */

#include <types.h>

#include <asm/regs.h>
#include <asm/traps.h>
#include <asm/io.h>
#include <drivers/i8259.h>
#include <hal/arch.h>
#include <hal/reg.h>
#include <drivers/irqctrl.h>

#include <embox/unit.h>

EMBOX_UNIT_INIT(unit_init);

/**
 * Initialize the PIC
 */
static int unit_init(void) {
	static int inited = 0;
	if (1 == inited) {
		return 0;
	}
	inited = 1;

	/* Initialize the master */
	out8(PIC1_ICW1, PIC1_COMMAND);
	out8(PIC1_BASE, PIC1_DATA);
	out8(PIC1_ICW3, PIC1_DATA);
	out8(PIC1_ICW4, PIC1_DATA);

	/* Initialize the slave */
	out8(PIC2_ICW1, PIC2_COMMAND);
	out8(PIC2_BASE, PIC2_DATA);
	out8(PIC2_ICW3, PIC2_DATA);
	out8(PIC2_ICW4, PIC2_DATA);

	out8(NON_SPEC_EOI, PIC1_COMMAND);
	out8(NON_SPEC_EOI, PIC2_COMMAND);

	out8(PICM_MASK, PIC1_DATA);
	out8(PICS_MASK, PIC2_DATA);

	irqctrl_enable(2); /* enable slave irq controller irq 8-16 */

	return 0;
}

void apic_init(void) {
	unit_init();
}

void irqctrl_enable(unsigned int int_nr) {
	if (int_nr > 8) {
		out8(in8(PIC2_DATA) & ~(1 << (int_nr - 8)), PIC2_DATA);
	} else {
		out8(in8(PIC1_DATA) & ~(1 << int_nr), PIC1_DATA);
	}
}

void irqctrl_disable(unsigned int int_nr) {
	if (int_nr > 8) {
		out8(in8(PIC2_DATA) | (1 << (int_nr - 8)), PIC2_DATA);
	} else {
		out8(in8(PIC1_DATA) | (1 << int_nr), PIC1_DATA);
	}
}

void irqctrl_force(unsigned int irq_num) {
	// TODO Emm?.. -- Eldar
}

int i8259_irq_pending(unsigned int irq) {
	int ret;
	unsigned int mask = 1 << irq;

	if (irq < 8) {
		ret = in8(PIC1_COMMAND) & mask;
	} else {
		ret = in8(PIC2_COMMAND) & (mask >> 8);
	}

	return ret;
}
