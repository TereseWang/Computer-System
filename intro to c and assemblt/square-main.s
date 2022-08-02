.global main
.text
   main:
     enter $0, $0
     mov %rsi, %r8 # argv
     mov 8(%r8), %rdi
     call atol
     mov %rax, %rdi
     imul %rdi
     imul %rdi
     mov %rax, %rsi
     mov $format, %rdi
     mov $0, %al
     call printf
     leave
     ret


  .data
   format:
       .asciz "result = %ld\n"
