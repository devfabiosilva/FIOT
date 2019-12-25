#
#	AUTHOR: Fábio Pereira da Silva
#	YEAR: 2019
#	LICENSE: MIT
#	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
#

## Implementation of big number 288 bit used in Nano Cryptocurrency C Library for IA64 architechture

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

.size f_add_bn_288_le, .-f_add_bn_288_le

