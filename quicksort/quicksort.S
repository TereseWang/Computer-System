.global main
.text

/*
void
swap(long* xs, long ii, long jj)
{
    if (ii != jj) {
        long tmp = xs[ii];
        xs[ii] = xs[jj];
        xs[jj] = tmp;
    }
}
variable mapping 
xs is rdi 
ii is rsi 
jj is rdx
temp is r10 
xs[jj] is r11
xs[ii] = (%rdi, %rsi, 8)
xs[jj] = (%rdi, %rdx, 8)
*/

swap:
	enter $0, $0
	cmp %rsi, %rdx
	je swap_done
	
swap_cond:
	mov (%rdi, %rsi, 8), %r10 //temp = xs[ii]
	mov (%rdi, %rdx, 8), %r11 //%r11 = xs[jj]
	mov %r11, (%rdi, %rsi, 8) //xs[ii] = xs[jj]
	mov %r10, (%rdi, %rdx, 8) //xs[jj] = xs[ii]
swap_done:
	leave 
	ret 
	
/*
long
partition(long* xs, long lo, long hi)
{
    long pivot = xs[hi - 1];

    long ii = lo;
    long jj = lo;
    for (; jj < (hi - 1); ++jj) {
        if (xs[jj] < pivot) {
            swap(xs, ii, jj);
            ++ii;
        }
    }
    swap(xs, ii, hi - 1);
    return ii;
}
variable mapping:
xs is %rdi 
lo is %rsi 
hi is %rdx
hi -1 is %r12
pivot is %r13
ii is %r14
jj is %r15
xs is %rbx
*/	
partition:
	push %r12
	push %r13
	push %r14
	push %r15
	push %rbx

	enter $8, $0
	//long pivot = xs[hi - 1];
	//long ii = lo;
    	//long jj = lo;
	dec %rdx
	mov %rdx, %r12  //hi - 1
	mov (%rdi, %r12, 8), %r13 //pivot 
	mov %rsi, %r14  //ii
	mov %rsi, %r15  //jj
	mov %rdi, %rbx //xs
		
partition_loop_cond:
	//for (; jj < (hi - 1); ++jj) 
	cmp %r12, %r15
	jge partition_loop_done
	
	//if (xs[jj] < pivot)
	mov (%rbx, %r15, 8), %r10
	cmp %r13, %r10
	jge partition_do_nothing 

	mov %rbx, %rdi
	mov %r14, %rsi //swap(xs, ii,jj)
	mov %r15, %rdx
	call swap
	inc %r14
partition_do_nothing:
	inc %r15
	jmp partition_loop_cond
partition_loop_done:
	//swap(xs, ii, hi - 1);
	//return ii;
	mov %rbx, %rdi
	mov %r14, %rsi
	mov %r12, %rdx
	call swap 
	mov %r14, %rax
partition_done:
	leave 
	pop %rbx
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	ret
	
/*
void
quicksort(long* xs, long lo, long hi)
{
    if (hi - lo < 1) {
        return;
    }

    long pp = partition(xs, lo, hi);
    quicksort(xs, lo, pp);
    quicksort(xs, pp + 1, hi);
}
variable mapping:
xs is %rdi 
lo is $rsi 
hi is %rdx

xs is %r12
lo is %r13
hi is %r14
pp is %r15
*/
quicksort:
	push %r12
	push %r13
	push %r14
	push %r15

	enter $0, $0

	mov %rdi, %r12
	mov %rsi, %r13
	mov %rdx, %r14

	mov %rdx, %rax // if (hi - lo) < 1) {return;}
	sub %rsi, %rax
	cmp $1, %rax
	jl quicksort_end
	
	mov %r13, %rsi 
	call partition  //pp = partition(xs, lo, hi)
	mov %rax, %r15
	
	mov %r12, %rdi  //quicksort(xs, lo, pp)
	mov %r13, %rsi 
	mov %r15, %rdx 
	call quicksort 

	mov %r12, %rdi 
	inc %r15
	mov %r15, %rsi 
	mov %r14, %rdx //quicksort(xs, pp + 1, hi)
	call quicksort

quicksort_end:
	leave
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	ret
	
/*
void
read_int(long* ptr)
{
    if (scanf("%ld", ptr) != 1) {
        puts("bad input");
        exit(1);
    }
}
variable mapping:
ptr is %rdi
*/
read_int:
	enter $0, $0
	mov %rdi, %rsi 
	mov $scanfmt, %rdi 
	mov $0, %al 
	call scanf 
	cmp $1, %rax
	je read_int_end
bad_input_args:
	mov $bad_input, %rdi 
	call puts 
	call free
	mov $1, %rdi 
	call exit 
read_int_end:
	leave 
	ret

/*
int
main(int _argc, char* _argv[])
{
    long nn;
    read_int(&nn);

    long* xs = malloc(nn * sizeof(long));

    for (long ii = 0; ii < nn; ++ii) {
        read_int(&(xs[ii]));
    }

    quicksort(xs, 0, nn);

    printf("%ld\n", nn);
    for (long ii = 0; ii < nn; ++ii) {
        printf("%ld ", xs[ii]);
    }
    printf("\n");

    free(xs);
    return 0;
}
variable mapping:
nn is %r12
xs is %r13
ii is %r14
*/
main:
	push %r12
	push %r13
	push %r14

  	enter $8, $0
	lea (%rsp), %rdi 
	call read_int  //read_int(&nn)
	
	mov (%rsp), %rax
	mov %rax, %r12  //xs == malloc8=(nn * sizeof(long))
	mov $8, %r10 
	imul %r10
	mov %rax, %rdi 
	call malloc 
	mov %rax, %r13

	mov $0, %r14
/*
for (long ii = 0; ii < nn; ++ii) {
        read_int(&(xs[ii]));
    }

    quicksort(xs, 0, nn);

    printf("%ld\n", nn);
*/
read_int_loop_cond:
	cmp %r12, %r14
	jge read_int_loop_done 

	lea (%r13, %r14, 8), %rdi 
	call read_int 

	inc %r14
	jmp read_int_loop_cond 
read_int_loop_done:
	mov %r13, %rdi 
	mov $0, %rsi 
	mov %r12, %rdx
	call quicksort 
	
	mov $lengthfmt, %rdi 
	mov %r12, %rsi 
	mov $0, %al 
	call printf 
/*
 for (long ii = 0; ii < nn; ++ii) {
        printf("%ld ", xs[ii]);
    }
*/
	mov $0, %r14
output_loop_cond:
	cmp %r12, %r14
	jge output_loop_done 
	
	mov $stringfmt, %rdi 
	mov (%r13, %r14, 8), %rsi 
	mov $0, %al 
	call printf

	inc %r14
	jmp output_loop_cond 

output_loop_done:
	mov $newline, %rdi 
	mov $0, %al 	
	call printf   // printf("\n");

	mov %r13, %rdi 
	call free    //free(xs)
	mov $0, %rax //return 0;

main_done:
  	leave
	pop %r14
	pop %r13
	pop %r12
  	ret
.data
bad_input: .string "bad input"
scanfmt: .string "%ld"
stringfmt: .string "%ld "
lengthfmt: .string "%ld\n"
newline : .string "\n"

