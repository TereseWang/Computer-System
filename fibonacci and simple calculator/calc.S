.global main 
.text 
/*
int main(int argc, char* argv[])
{
	if(argc != 4) {
		printf("Usage: \n %s N op N \n",argv[0]);
		return -99;
	}
	else {
		char* op = argv[2];
		long x = atol(argv[1]);
		long y = atol(argv[3]);
		long result;
		if (strcmp(op,"+") == 0) {
			result = x + y;
			printf("%ld + %ld = %ld\n",x, y,result);
			return 0;
		}
		else if (strcmp(op, "-") == 0) {
			result = x - y;
			printf("%ld - %ld = %ld\n", x, y, result);
			return 0;
		}
		else if (strcmp(op, "*") == 0) {
			result = x * y;
			printf("%ld * %ld = %ld\n", x, y, result);
			return 0;
		}
		else if (strcmp(op, "/") == 0) {
			result = x / y;
			printf("%ld / %ld = %ld\n", x, y, result);
			return 0;
		}
		else {
			printf("Usage: \n %s N op N\n", argv[0]);
			return -99;
		}
	}
variable mapping:
	argc is %rdi 
	argv[0] is (%rsi), save to %r12
	argv[1] is 8(%rsi)
	argv[2] is 16(%rsi)
	argv[3] is 24(%rsi)
	x = atol(argv[1], save to %r13
	y = atol(argv[3], save to %r15
	op is %r14
}
*/

main:
	push %r12
	push %r13
	push %r14
	push %r15

	enter $8, $0
	mov (%rsi), %r12  //argv[0]
	mov 8(%rsi), %r13 //argv[1]
	mov 16(%rsi), %r14 //op = argv[2]	
	mov 24(%rsi), %r15 //argv[3]

	cmp $4, %rdi //check if argc == 4
	jne invalid_input 
	
	mov %r13, %rdi 
	call atol   //x = atol(argv[1])
	mov %rax, %r13
	mov %r15, %rdi 
	call atol  //y = atol(argv[3])
	mov %rax, %r15
	
	mov %r14, %rdi 
	mov $add, %rsi 
	call strcmp //strcmp(op, "+")
	cmp $0, %rax
	je add_operator 

	mov %r14, %rdi 
	mov $sub, %rsi 
	call strcmp  //strcmp(op, "-")
	cmp $0, %rax
	je sub_operator 
	
	mov %r14, %rdi 
	mov $mult, %rsi  //strcmp(op, "*")
	call strcmp 
	cmp $0, %rax
	je mult_operator 
	
	mov %r14, %rdi 
	mov $div, %rsi 
	call strcmp  //strcmp (op, "/")
	cmp $0, %rax
	je div_operator 

	jmp invalid_input //invalid operator 

add_operator: 
	mov %r13, %rax
	add %r15, %rax  //result = x + y
	mov $addformat, %rdi
	mov %r13, %rsi 
	mov %r15, %rdx
	mov %rax, %rcx
	mov $0, %al 
	call printf  //printf("%ld + %ld = %ld", x, y, result)
	mov $0, %rax	
	jmp main_done 

sub_operator:
	mov %r13, %rax
	sub %r15, %rax  //result = x - y
	mov $subformat, %rdi
	mov %r13, %rsi 
	mov %r15, %rdx
	mov %rax, %rcx
	mov $0, %al 
	call printf //printf("%ld - %ld = %ld", x, y, result)
	mov $0, %rax	
	jmp main_done 

	
mult_operator:
	mov %r13, %rax
	imul %r15   //result = x * y 
	mov $multformat, %rdi
	mov %r13, %rsi 
	mov %r15, %rdx
	mov %rax, %rcx
	mov $0, %al 
	call printf  //printf(%ld * %ld = %ld", x, y, result)
	mov $0, %rax	
	jmp main_done 

	
div_operator:
	mov %r13, %rax
	cqo
	idiv %r15 //result = x / y
	mov $divformat, %rdi 
	mov %r13, %rsi 
	mov %r15, %rdx
	mov %rax, %rcx
	mov $0, %al 
	call printf //printf("%ld / %ld = %ld",x, y, result)
	mov $0, %rax	
	jmp main_done
 
invalid_input:
	mov $invalidInput, %rdi 
	mov %r12, %rsi //printf("Usage: \n %s N op N\n", argv[0])
	mov $0, %al 
	call printf
	mov $-99, %rax //return -99;
	
main_done:
	leave 
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	ret 
.data 
invalidInput: .string "Usage: \n %s N op N\n"
add: .string "+"
sub: .string "-"
mult: .string "*"
div: .string "/"
addformat: .string "%ld + %ld = %ld\n"
subformat: .string "%ld - %ld = %ld\n"
multformat: .string "%ld * %ld = %ld\n"
divformat: .string "%ld / %ld = %ld\n"
