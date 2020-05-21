#include "uthread.h"
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


void schedule_test()
{
    schedule_t s;

    struct args arg1 = { 0 };
    struct args arg2 = { 100 };

    int id1 = uthread_create(s,foo,&arg1);
    int id2 = uthread_create(s,foo,&args);

    while(!schedule_finished(s)){
        uthread_resume(s,id2);
        uthread_resume(s,id1);
    }
    puts("main over");

}

int main()
{

    schedule_test();

    return 0;
}
