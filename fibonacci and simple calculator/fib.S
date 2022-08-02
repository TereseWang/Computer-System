.global main 
.text
/*
long 
fib(long x)
{
	long result;
	if (x <= 1) {
		result = x;
	}
	else {
		result = fib(x - 1) + fib(x - 2);
	}
	return result;
}
variable mapping:
	x is rdi
	result is %r12
*/
fib:
	enter $8, $0
	// if x <= 1
	cmp $0, %rdi
	je fib_zero 

	cmp $1, %rdi 
	je fib_one 
	
	//else 
fib_recursive: 
	dec %rdi  //x - 1
	push %rdi //save x - 1
	call fib  //call fib(x-1)
	pop %rdi //pop x - 1
	push %r12 // push r12, save fib(x - 1) to %r12
	mov %rax, %r12

	// x - 1 - 1 = x - 2
	dec %rdi 
	// call fib (x - 2)
	call fib 
	// add fib (x - 1) to %rax (fib (x - 2))
	add %r12, %rax
	//pop %r12
	pop %r12

	jmp fib_done	
fib_zero:
	mov $0, %rax
	jmp fib_done 
fib_one:
	mov $1, %rax
	jmp fib_done
fib_done:
	leave 
	ret  
	
/*
int
main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Usage:\n  %s N, where N > 0\n", argv[0]);
        return -99;
    }
    else {
	    long x = atol(argv[1]);
	    if (x < 0) {
		    printf("Usage:\n %s N, where N > 0\n", argv[0]);
		    return -99;
	    }
	    else{
		    printf("fib(%ld) = %ld \n", x, fib(x));
		    return 0;
	    }
    }
}
variable mapping:
	argc is rdi
	argv[0] is (%rsi), save to %r12, in order to print twice 
	argv[1] is 8(%rsi)
	x is r13, = atol(argv[1]), save to safe register 
	because need to call it twice for fib function and also for printf  
*/

main:
	push %r12
	push %r13
	enter $16, $0
	//save argv[0], "./afib" to %r12
	mov (%rsi), %r12 

	// if argc != 2
	cmp $2, %rdi
	jne invalid_input
	
	// long x = atol(argv[1]);
	mov 8(%rsi), %rdi 
	call atol 
	mov %rax, %r13
	
	// if x < 0
	cmp $0, %r13
	jl invalid_input

valid_input:
	mov %r13, %rdi 
	call fib
	//printf("fib(%ld)= %ld \n", x , fib(x));
	mov $resultstring, %rdi 
	mov %r13, %rsi
	mov %rax, %rdx 
	mov $0, %al 
	call printf
	//return 0 
	mov $0, %rax
	jmp main_done

invalid_input:
	//printf("Usage:\n %s N, where N > 0\n", argv[0]);
	mov $errormessage, %rdi
	mov %r12, %rsi
	mov $0, %al
	call printf
	//return -99
	mov $-99, %rax
	
main_done:
	leave 
	pop %r13
	pop %r12
	ret

.data
errormessage: .string "Usage:\n %s N, where N > 0\n"
resultstring: .string "fib(%ld) = %ld\n"

