
#include <stdio.h>
#include <stdlib.h>

long 
fib(long x)
{
	long result;
	if (x <= 1) {
		result = x;
	}
	else {
		result = fib(x-2) + fib(x - 1);
	}
	return result;
}

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
