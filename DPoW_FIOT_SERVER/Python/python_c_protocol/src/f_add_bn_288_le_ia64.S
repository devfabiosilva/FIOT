#
#	AUTHOR: Fábio Pereira da Silva
#	YEAR: 2019
#	LICENSE: MIT
#	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
#

## Implementation of big number 288 bit used in Nano Cryptocurrency C Library for IA64 architechture

#if F_ARM_A|F_ARM_M|F_THUMB

 #ifdef F_ARM_A
  .arch armv7-a
 #else
  .arch armv7-m
 #endif

 #ifdef F_THUMB
  .thumb
  .thumb_func
 #else
  .arm
 #endif

.text
.global f_add_bn_288_le
.align 1
.type f_add_bn_288_le, %function

f_add_bn_288_le:

 #ifdef F_THUMB

   push {r0, r1, r2, r4, r5, r6, r7, lr}

   ldr r4, [sp, #32]

   mov r7, #0

f_loop:

   ldr r5, [r0, #0]
   ldr r6, [r1, #0]
   add r5, r4
   adc r5, r6
   str r5, [r2, #0]

   ldr r5, [r0, #4]
   ldr r6, [r1, #4]
   adc r5, r6
   str r5, [r2, #4]

   ldr r5, [r0, #8]
   ldr r6, [r1, #8]
   adc r5, r6
   str r5, [r2, #8]

   ldr r5, [r0, #12]
   ldr r6, [r1, #12]
   adc r5, r5, r6
   str r5, [r2, #12]

   and r7, r7
   bne p1

   mov r4, #1
   bcs p1_1
   mov r4, #0

p1_1:

   add r0, r0, #16
   add r1, r1, #16
   add r2, r2, #16

   mov r7, #1

   b f_loop

p1:

   ldr r5, [r0, #16]
   ldr r6, [r1, #16]
   adc r5, r5, r6
   str r5, [r2, #16]

   and r3, r3
   beq p3
   bcs p2
   mov r7, #0

p2:

   str r7, [r3]

p3:

   pop {r0, r1, r2, r4, r5, r6, r7, pc}

 #else

   push {r0, r1, r2, r3, lr}

   ldr r2, [r0]

   orr r1, r2, lsl #11
   str r1, [r0]

   ldr r3, [r0, #4]
   mov r1, r3, lsl #11
   orr r1, r2, lsr #21
   str r1, [r0, #4]

   ldr r2, [r0, #8]
   mov r1, r2, lsl #11
   orr r1, r3, lsr #21
   str r1, [r0, #8]

   ldr r3, [r0, #12]
   mov r1, r3, lsl #11
   orr r1, r2, lsr #21
   str r1, [r0, #12]

   ldr r2, [r0, #16]
   mov r1, r2, lsl #11
   orr r1, r3, lsr #21
   str r1, [r0, #16]

   ldr r3, [r0, #20]
   mov r1, r3, lsl #11
   orr r1, r2, lsr #21
   str r1, [r0, #20]

   ldr r2, [r0, #24]
   mov r1, r2, lsl #11
   orr r1, r3, lsr #21
   str r1, [r0, #24]

   ldr r3, [r0, #28]
   mov r1, r3, lsl #11
   orr r1, r2, lsr #21
   str r1, [r0, #28]

   ldr r2, [r0, #32]
   mov r1, r2, lsl #11
   orr r1, r3, lsr #21
   str r1, [r0, #32]

   pop {r0, r1, r2, r3, pc}

 #endif

#else

 #ifdef F_IA64

.text
.globl	f_add_bn_288_le
.type	f_add_bn_288_le, @function
.align 8

f_add_bn_288_le:

	clc
	movl %r8d, %r8d
	andl %r8d, %r8d
	jz a1
	stc

a1:

	movq (%rdi), %rax
	adcq (%rsi), %rax
	movq %rax, (%rdx)

	movq 8(%rdi), %rax
	adcq 8(%rsi), %rax
	movq %rax, 8(%rdx)

	movq 16(%rdi), %rax
	adcq 16(%rsi), %rax
	movq %rax, 16(%rdx)

	movq 24(%rdi), %rax
	adcq 24(%rsi), %rax
	movq %rax, 24(%rdx)

	movl 32(%rdi), %eax
	adcl 32(%rsi), %eax
	movl %eax, 32(%rdx)

	movl $0, %eax

	jnc p1

	movl $1, %eax
p1:

	test %rcx, %rcx

	jz p2

	movl %eax, (%rcx)

p2:

	ret
 #else
  #error "Could not determine hardware architecture"
 #endif
#endif

.size f_add_bn_288_le, .-f_add_bn_288_le

