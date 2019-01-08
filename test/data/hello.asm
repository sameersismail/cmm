input:
li     $v0, 5
syscall
move   $a0, $v0
jr     $ra

output:
li     $v0, 1
syscall
jr     $ra

.globl main
main:
move   $fp, $sp
sw     $ra, 0($sp)
addiu  $sp, $sp, -4

addiu  $sp, $sp, -4
li     $a0, 0
sw     $a0, 0($sp)
addiu  $sp, $sp, -4
sw     $a0, -4($fp)
addiu  $sp, $sp, 4
jal    output
j      main_exit

main_exit:
li     $v0, 10
syscall
