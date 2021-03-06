/*  This file is part of simple kernel.
    Project NanoKernel (for study purposes only)
    Copyright (C) 2013  Sirius (Vdov Nikita)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <kernel.h>

/* The flag for protected mode.  */
#define CPU_CR0_PE_ON		0x1
#define CPU_CR4_PAE_ON		0x00000020
#define CPU_CR4_PSE_ON		0x00000010
#define CPU_CR0_PAGING_ON	0x80000000
#define CPU_AMD64_MSR		0xc0000080
#define CPU_AMD64_MSR_ON	0x00000100

	.file	"realmode.S"

.globl k_gdt;
.globl k_switch_to_protected_mode;
.globl k_switch_to_real_mode;

	.data
/*
 * This is the Global Descriptor Table
 *
 *  An entry, a "Segment Descriptor", looks like this:
 *
 * 31          24         19   16                 7           0
 * ------------------------------------------------------------
 * |             | |B| |A|       | |   |1|0|E|W|A|            |
 * | BASE 31..24 |G|/|L|V| LIMIT |P|DPL|  TYPE   | BASE 23:16 |  4
 * |             | |D| |L| 19..16| |   |1|1|C|R|A|            |
 * ------------------------------------------------------------
 * |                             |                            |
 * |        BASE 15..0           |       LIMIT 15..0          |  0
 * |                             |                            |
 * ------------------------------------------------------------
 *
 *  Note the ordering of the data items is reversed from the above
 *  description.
 */

/* force 8-byte alignment */
	.p2align	3

k_gdt_dummy:
	.word	0, 0
	.byte	0, 0, 0, 0

	/* -- code segment --
	 * base = 0x00000000, limit = 0xFFFFF (4 KiB Granularity), present
	 * type = 32bit code execute/read, DPL = 0
	 */
	.word	0xFFFF, 0
	.byte	0, 0x9A, 0xCF, 0

	/* -- data segment --
	 * base = 0x00000000, limit 0xFFFFF (4 KiB Granularity), present
	 * type = 32 bit data read/write, DPL = 0
	 */
	.word	0xFFFF, 0
	.byte	0, 0x92, 0xCF, 0

	/* -- stack segment --
	 * base = 0x00000000, limit KERNEL_REALMODE_STACK_ADDR (1 B Granularity), present
	 * type = 32 bit data read/write, DPL = 0
	 */
	.word	KERNEL_PROTECTEDMODE_STACK_ADDR, 0
	.byte	0, 0x92, 0x40, 0

	/* -- 16 bit real mode CS --
	 * base = 0x00000000, limit 0x0FFFF (1 B Granularity), present
	 * type = 16 bit code execute/read only/conforming, DPL = 0
	 */
	.word	0xFFFF, 0
	.byte	0, 0x9E, 0, 0

	/* -- 16 bit real mode DS --
	 * base = 0x00000000, limit 0x0FFFF (1 B Granularity), present
	 * type = 16 bit data read/write, DPL = 0
	 */
	.word	0xFFFF, 0
	.byte	0, 0x92, 0, 0

/* force 4-byte alignment */
	.p2align	2
/* this is the GDT descriptor */
gdtdesc_dummy:
	.word	0x2F			/* limit */
	.long	k_gdt_dummy		/* addr */

	.text

k_switch_to_protected_mode:
	.code16

	/* turn off interrupts */
	cli	

	/* load the GDT register */
	movl	$KERNEL_REALMODE_DATA_SEG, %eax
	movw	%ax, %ds
	lgdt	gdtdesc_dummy

	/* turn on protected mode */
	movl	%cr0, %eax
	orl	$CPU_CR0_PE_ON, %eax
	movl	%eax, %cr0

	/* jump to relocation, flush prefetch queue, and reload %cs */
	ljmp	$KERNEL_PROTECTEDMODE_CODE_SEG, $protected_mode_exec_begin

protected_mode_exec_begin:
	.code32

	/* reload other segment registers */
	movw	$KERNEL_PROTECTEDMODE_DATA_SEG, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	$KERNEL_PROTECTEDMODE_STACK_SEG, %ax
	movw	%ax, %ss

	sti
	/* fix return address */
	movzwl	(%esp), %eax
	subl	$0x2, %esp
	movl	%eax, (%esp)

	ret

k_switch_to_real_mode:
	.code32

	/* turn off interrupts */
	cli

	pushl	%ebp
	movl	%esp, %ebp
	pushl	%eax
	/* just in case, set GDT */
	movl	$KERNEL_REALMODE_DATA_SEG, %eax
	movw	%ax, %ds
	lgdt	gdtdesc_dummy

	/* set up segment limits */
	movw	$KERNEL_PSEUDO_REALMODE_DATA_SEG, %ax
	movw	%ax, %ds	
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	/* this might be an extra step */
	/* jump to a 16 bit segment */
	ljmp	$KERNEL_PSEUDO_REALMODE_CODE_SEG, $switch_code16_segment

switch_code16_segment:
	.code16

	/* clear the PE bit of CR0 */
	movl	%cr0, %eax
	andl 	$(~CPU_CR0_PE_ON), %eax
	movl	%eax, %cr0

	/* flush prefetch queue, reload %cs */
	ljmp	$KERNEL_REALMODE_CODE_SEG, $realmode_code

realmode_code:
	/* we are in real mode now
	 * set up the real mode segment registers : DS, SS, ES
	 */
	/* zero %eax */
	movw	$KERNEL_REALMODE_DATA_SEG, %ax
	movw	%ax, %ds
	xorw	%ax, %ax
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	$KERNEL_REALMODE_STACK_SEG, %ax
	movw	%ax, %ss
	/* restore interrupts */
	sti
	
	/* fix stack leak */
	movw	0x4(%bp), %ax
	movw	%ax, 0x6(%bp)
	popl	%eax
	popl	%ebp
	addw	$0x2, %sp

	ret

