#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

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
		if (strcmp(op, "+") == 0) {
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

}
