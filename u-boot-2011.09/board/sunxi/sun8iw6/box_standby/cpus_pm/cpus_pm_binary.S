
.globl _standby_new_binary_start
.globl _standby_new_binary_end
_standby_new_binary_start:
	.incbin "cpus_pm_binary.code"
_standby_new_binary_end:
