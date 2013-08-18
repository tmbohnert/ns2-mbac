#include <stdio.h>


int main(){
	int i;
	for(i=10; i>(-20); i--)
		printf("i=%i : mod=%i\n", i, i%5);
}

