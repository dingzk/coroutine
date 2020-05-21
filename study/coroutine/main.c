#include "coroutine.h"
#include <stdio.h>

struct args {
	int n;
};

static void
foo(struct schedule * S, void *ud) {
	struct args * arg = ud;
	int start = arg->n;
	int i;
	for (i=0;i<5;i++) {
		printf("coroutine %d : %d\n",coroutine_running(S) , start + i);
		coroutine_yield(S);
	}
}

void func2(struct schedule * S, void *ud)
{
    puts("21");
    puts("21");
    coroutine_yield(S);
    puts("22");
    puts("22");
}

void func3(struct schedule * S, void *ud)
{
    puts("31");
    puts("31");
    coroutine_yield(S);
    puts("32");
    puts("32");

}


static void
test(struct schedule *S) {
	struct args arg1 = { 0 };
	struct args arg2 = { 100 };

	int co1 = coroutine_new(S, func2, &arg1);
	int co2 = coroutine_new(S, func3, &arg2);
	printf("main start\n");
	while (coroutine_status(S,co1) && coroutine_status(S,co2)) {
		coroutine_resume(S,co1);
		coroutine_resume(S,co2);
	} 
	printf("main end\n");
}

int 
main() {
	struct schedule * S = coroutine_open();
	test(S);
	coroutine_close(S);
	
	return 0;
}

