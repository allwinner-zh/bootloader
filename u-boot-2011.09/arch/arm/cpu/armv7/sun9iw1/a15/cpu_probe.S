/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
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


	.arm
	.global get_cpu_id	@ uint32_t get_cpu_id(void)
                        @ Returns the CPU ID (0 to 3) of the CPU executed on
get_cpu_id:
     push   {r8-r10}
     mrc	p15, 0, r0, c0, c0, 5		@ MPIDR
     ubfx	r9, r0, #0, #8			@ r9 = cpu
     ubfx	r10, r0, #8, #8			@ r10 = cluster
     mov	r8, #4
     mla	r0, r8, r10, r9			@ r4 = canonical CPU index
     pop {r8-r10}
     bx      lr



	.arm
	.globl save_runtime_context     @ save a7 run_time context

save_runtime_context:
	stmia   r0, {r0 - r14}
	bx	 lr

	.arm
	.globl jump_to_resume

jump_to_resume:
	ldr r0 , =0x0
	ldr r1 , =0x10000
loop:
	add r0,r0,#1
	cmp r0,r1
	bne loop

    @ L2 data ram latency
    mrc p15,1,r0,c9,c0,2
    bic r0,r0,#(0x7<<0)
    orr r0,r0,#(0x3<<0)
    mcr p15,1,r0,c9,c0,2

	ldr		r1, =0x08100000
	ldmia   r1, {r0 - r14}
    mov     pc, lr
