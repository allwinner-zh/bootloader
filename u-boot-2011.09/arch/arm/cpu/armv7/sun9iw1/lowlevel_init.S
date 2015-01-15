/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Configuration settings for the Allwinner A10-evb board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include<config.h>

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE	/* sdram load addr from config.mk */


.globl lowlevel_init
lowlevel_init:
	ldr	sp, SRAM_STACK
	str	ip, [sp]	/* stash old link register */
	mov	ip, lr		/* save link reg across call */
	bl	s_init		/* go setup pll, mux, memory */
	ldr	ip, [sp]	/* restore save ip */
	mov	lr, ip		/* restore link reg */

	/* back to arch calling code */
	mov	pc, lr

	/* the literal pools origin */
	.ltorg

SRAM_STACK:
	.word LOW_LEVEL_SRAM_STACK


.globl get_cluster_id
get_cluster_id:

mrc		p15, 0, r0, c0, c0, 5   // Read MPIDR
mov  	r0, r0, LSR #8
and		r0, r0, #0x0F           //Mask off, leaving the Cluster ID field

bx		lr

.globl get_cpu_id
get_cpu_id:

	mrc		p15, 0, r0, c0, c0, 5   // Read CPU ID register
	and		r0, r0, #0x03           //Mask off, leaving the CPU ID field

	bx		lr

/*save a7 run_time context*/
.globl save_runtime_context
save_runtime_context:
	save_runtime_context:
	/*save r0-r14 register*/
	stmia   r0, {r0 - r14}
	mov	pc, lr

.globl jump_to_resume
jump_to_resume:
	/*make some time delay*/
	ldr r0 , =0x0
	ldr r1 , =0x10000
loop:
	add r0,r0,#1
	cmp r0,r1
	bne loop
	//enable icache
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-cache
	mcr	p15, 0, r0, c1, c0, 0

    /*L2 data ram latency*/
    mrc p15,1,r0,c9,c0,2
    bic r0,r0,#(0x7<<0)
    orr r0,r0,#(0x3<<0)
    mcr p15,1,r0,c9,c0,2

/* map sram-a1 address to 0x0 */
	ldr     r0, =0x008000e0
	ldr     r1, =0x16aa0001
	str     r1, [r0]

	ldr		r1, =0x08100000
	ldmia   r1, {r0 - r14}
//	ldr 	lr, =board_init_f
    mov     pc, lr
