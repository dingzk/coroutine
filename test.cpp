#include "uthread.h"
#include <stdio.h>

struct args {
    int n;
};

static void
foo(Uthread * S, void *ud) {
    struct args * arg = (struct args *)ud;
    int start = arg->n;
    int i;
    for (i=0;i<5;i++) {
        printf("coroutine id: %d status: %d : %d\n", S->get_sched_running(), S->get_status(), start + i);
        S->yield();
    }
}


void schedule_test(Uthread * S)
{

    struct args arg1 = { 0 };
    struct args arg2 = { 100 };

    int id1 = S->create(foo, &arg1);
    int id2 = S->create(foo, &arg2);

    printf ("finished? %d %d %d \n", S->finished(), id1, id2);

    while(!S->finished()) {
        S->resume(id2);
        S->resume(id1);
    }
    puts("main over");

}

int main()
{
    Uthread thr;

    schedule_test(&thr);

    return 0;
}
