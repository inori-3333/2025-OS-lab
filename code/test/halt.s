	.file	1 "halt.c"
gcc2_compiled.:
__gnu_compiled_c:
	.rdata
	.align	2
$LC0:
	.ascii	"I will shut down!\n\000"
	.text
	.align	2
	.globl	main
	.ent	main
main:
	.frame	$fp,32,$31		# vars= 8, regs= 2/0, args= 16, extra= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,32
	sw	$31,28($sp)
	sw	$fp,24($sp)
	move	$fp,$sp
	jal	__main
	li	$2,45			# 0x0000002d
	sb	$2,16($fp)
	li	$2,45			# 0x0000002d
	sb	$2,17($fp)
	addu	$4,$fp,16
	li	$5,1			# 0x00000001
	la	$6,$LC0
	jal	Write
	jal	Halt
$L1:
	move	$sp,$fp
	lw	$31,28($sp)
	lw	$fp,24($sp)
	addu	$sp,$sp,32
	j	$31
	.end	main
