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

#include <asm/assembler.h>


/*
************************************************************************************************************
*
*                                             arm_neon_init
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
	.balign   4
	.global   arm_neon_init     @r0存放起始地址，r1存放数据字节数

arm_neon_init:

	stmfd sp!, {lr}

	ldr r0, =(0xf<<20)
	mcr p15, 0, r0, c1, c0, 2   @(enable CP10/CP11, and disable ASEDIS/D32DIS)

	mov r0, #0
	mcr p15, 0, r0, c7, c5, 4   @(CP15ISB)

	mov r0, #0x40000000
	fmxr fpexc, r0              @(enable NEON)

	isb                         @(wait all code are executed including pipeline)
	dsb                         @(wait all register access are finished)

	mov r0, #0

	ldmfd sp!, {pc}


/*
************************************************************************************************************
*
*                                             add_sum_neon
*
*    name          :
*
*    parmeters     :   r0, 起始地址   r1, 数据字节数
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
	.balign   4
	.global   add_sum_neon     @r0存放起始地址，r1存放数据字节数
	.arm

add_sum_neon:
    stmfd sp!, {r2-r6, lr}

	mov r2, r0
	mov r3, r1
	mov r5, #0                          @初始化r5=0，保存返回值
	vbic.I32 q0, #0x000000ff
	vbic.I32 q0, #0x0000ff00
	vbic.I32 q0, #0x00ff0000
	vbic.I32 q0, #0xff000000

    cmp r3, #31
    bls __data_deal_32byte_unalign_0

__data_read_loop:
	vld1.I32 {d2, d3, d4, d5}, [r2]!    @把r2开始的数据，读取8(64/8)*4个字节到d0-d3当中，即Q0到Q1，完成后，r2自动变化到下一个读取地址
	vadd.I32 q3, q1, q2
	vadd.I32 q0, q0, q3                 @结果保存到Q0中
	sub r3, r3, #32                     @r3(字节数)减去32字节
	cmp r3, #31
	bhs __data_read_loop

	vadd.I32 d0, d0, d1
	vmov r5, r6, d0
	adds r5, r5, r6                     @r5保存返回值

__data_deal_32byte_unalign_0:
	and r4, r3, #3                      @r4保存非4字节对齐的字节数
	lsr r3, r3, #2                      @r3保存4字节对齐的个数
__data_deal_32byte_unalign_1:
	cmp r3, #0                          @查看是否还有未处理的4字节对齐
	beq __data_deal_4byte_unalign
	ldr r6, [r2], #4                    @读出数据
	adds r5, r5, r6                     @计算累加
	sub r3, r3, #1                      @对齐数据减1
	b __data_deal_32byte_unalign_1

__data_deal_4byte_unalign:              @处理未对齐的数据
	cmp r4, #0
	beq __data_deal_return
	rsb r4, r4, #4
	lsl r4, r4, #3
	mvn r6, #0
	lsr r6, r6, r4
	ldr r3, [r2], #4                   @读出数据
	and r6, r3, r6
	adds r5, r5, r6

__data_deal_return:
	mov r0, r5

	ldmfd sp!, {r2-r6, pc}
/*
************************************************************************************************************
*
*                                             memcpy_neon
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
	.align 4
	.global memcpy_neon
	.arm

memcpy_neon:
	cmp  r2, #0
	moveq pc, lr

	cmp r0, r1
	moveq pc, lr

	stmfd sp!, {r3-r9, lr}

	mov  r3, r0         			@r3保存目的地址
	mov  r4, r1         			@r4保存源地址
__memcpy_loop:
    cmp r2, #31 			        @判断传输字节是否小于32字节
    bls __memcpy_neon_32byte_unalign_0
	vld1.I32 {d2,d3,d4,d5}, [r4]!   @从r4开始，读取32个字节对Q1，Q2
	vst1.I32 {d2,d3,d4,d5}, [r3]!   @把Q1，Q2的数据存放到r3开始的地址
	sub r2, r2, #32
	b __memcpy_loop

__memcpy_neon_32byte_unalign_0:
	PLD(pld [r3, #0])
	PLD(pld [r4, #0])
	and r5, r2, #3                  @r5保存非4字节对齐的字节数
	lsr r6, r2, #2                  @r6保存4字节对齐的个数
__memcpy_neon_32byte_unalign_1:
	cmp r6, #0
	beq __memcpy_neon_4byte_unalign
	ldr r7, [r4], #4
	str r7, [r3], #4
	sub r6,r6,#1
	b __memcpy_neon_32byte_unalign_1
__memcpy_neon_4byte_unalign:
	cmp r5, #0
	beq __memcpy_neon_exit
	ldr r7, [r4], #4          @读出源地址最后一个word到r7，肯定会多读取
	ldr r8, [r4]              @读出目的地址最后一个word到r8
	mvn r9, #0
	rsb r5, r5, #4
	lsl r5, r5, #3            @r5=r5*8
	lsl r9, r9, r5
	and r8, r8, r9            @目的地址数据r8清零需要写入的位
	mvn r6, #0
	eor r9, r9, r6
	and r7, r7, r9            @源地址数据r7清零不能写入的位
	orr r7, r7, r8            @合并r7，r8，得到正确数据
	str r7, [r3], #4

__memcpy_neon_exit:
    ldmfd sp!, {r3-r9, lr}

/*
************************************************************************************************************
*
*                                             bmpdecode_neon
*
*    name          :
*
*    parmeters     :  r0: bmp原始数据地址
*
*                     r1: 解析后数据存放地址
*
*                     r2: 结构体的地址
*
*						{
*							int x;     x方向像素点个数
*							int y;     y方向像素点个数
*							int bmp_bpix;  每个像素的字节数
*						}
*
*    return        :  无
*
*    note          :  采取的策略是，每次读取32个字节(当一行字节数(x像素点个数*每像素点字节数)超过时)
*                                   当超过16字节不到32字节，一次读取16字节
*                                   当超过8字节且不到16时，一次读取8字节
*                                   当不到8字节时且不为0时，一次读取8字节
*
*                                   如果是ARGB格式，如果每行为奇数个像素点，一定多读4字节，然后源地址和目的地址需要修正，减掉4字节
*                                                   如果每行为偶数个像素点，则刚刚合适，不用修正
*
*                                   如果是RGB格式，每行有效字节数为3x个，补齐字节数为x&3个(4字节对齐)，
*                                   由于最少读取8字节，因此需要知道差多少字节可以补齐8字节对齐，然后最后减掉多读取的
*                                   算法：3x=有效字节数
*                                         3x&7=没有8字节对齐的部分
*                                         假设x=8m+n，则n取值为从1到7，3x=3(8m+n)=24m+3n,考虑n的取值(0值直接不考虑)
*                                         可能值为3,6,9,12,15,18,21
*										  再和7相与，可能值为3,6,1,4,7,2,5
*										  这也是没有8字节对齐的部分
*                                         用8来减掉这些值，得到的是需要补充的字节数，来保证读取能达到一次8字节
*                                         多读的值可能是5,2,7,4,1,6,3
*                                         考虑对齐因素，需要源地址修正的值应该是(结果&4)
*
*                     寄存器使用   r0  源地址
*
*                                  r1  目的地址
*
*                                  r2  结构体地址(读取后就无效)
*                                      后面保存x方向像素点个数
*
*                                  r3  y方向像素点个数
*
*                                  r4  每个像素的字节数
*
*                                  r5  目的地址多写入的字节数
*
*                                  r6  源地址多读取的字节数
*
*                                  r7  一行的有效字节数
*
*
*
************************************************************************************************************
*/
	.align 4
	.global bmpdecode_neon
	.arm

bmpdecode_neon:
	cmp r2, #0						@检查结构体地址是否合法
	moveq pc, lr

	cmp r0, r1                      @检查原始地址是否等于目的地址
	moveq pc, lr

	stmfd sp!, {r0-r8, lr}

	ldr r3, [r2, #4]                @r3保存y方向像素点个数
	ldr r4, [r2, #8]                @r4保存每个像素的字节数
	ldr r2, [r2]                    @r2保存x方向像素点个数

	cmp r4, #0
	beq __bmpdecode_filling_exit

	cmp r4, #4                      @判断一个像素点的字节数
	beq __bmpdecode_x_4pixles

									@处理一个像素点3个字节的情况
	lsl r7, r4, #2
	sub r7, r7, r4					@r7保存了一行有效的字节数  r7=3*r4
	and r5, r7, #7
	rsb r5, r5, #8					@r5保存了没有8字节对齐的字节部分, 多写入的
	and r6, r5, #4					@r6保存了需要补齐的字节部分，即会多读取的

	b __bmpdecode_y_loop

__bmpdecode_x_4pixles:              @处理一个像素点4个字节的情况
	and r5, r2, #1
	cmp r5, #1                      @判断是否是奇数个像素点
	lsleq r5, r5, #2                @如果是奇数个，则最终，源地址和目的地址都需要减去4，即多读多写的

	mov   r6, r5
	lsl r7, r4, #2

__bmpdecode_y_loop:
	mov r8, r7
__bmpdecode_x_loop:
    cmp r8, #31                     @一次处理32个字节数据
	bls __bmpdecode_filling_32bytes_less

    vld1.I32 {d2,d3,d4,d5}, [r0]!    @一次读取32个字节
    vst1.I32 {d2,d3,d4,d5}, [r1]!    @一次存储32个字节
	sub r8, r8, #32

    b __bmpdecode_x_loop

__bmpdecode_filling_32bytes_less:
	cmp r8, #15
	bls __bmpdecode_filling_16bytes_less

	vld1.I32 {d2, d3}, [r0]!    			@一次读取16个字节
    vst1.I32 {d2, d3}, [r1]!    			@一次存储16个字节

	sub r8, r8, #16
__bmpdecode_filling_16bytes_less:
	cmp r8, #7
	beq __bmpdecode_filling_8bytes_less

	vld1.I32 {d2}, [r0]!    			@一次读取8个字节
    vst1.I32 {d2}, [r1]!    			@一次存储8个字节

    sub r8, r8, #8
__bmpdecode_filling_8bytes_less:
	cmp r8, #0
	beq __bmpdecode_filling_x_finish

	vld1.I32 {d2}, [r0]!    			@一次读取8个字节
    vst1.I32 {d2}, [r1]!    			@一次存储8个字节

	sub r1, r1, r5						@修正目的地址
	sub r0, r0, r6						@修正源地址

__bmpdecode_filling_x_finish:

	sub r4, r4, #1                  @行数减1
	cmp r4, #0

	bne __bmpdecode_y_loop

__bmpdecode_filling_exit:

	stmfd sp!, {r0-r8, pc}

