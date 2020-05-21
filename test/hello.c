#include <stdlib.h>
#include <ucontext.h>
#include <stdio.h>
#include <string.h>

#define CO_DEFAULT_STACK_SIZE 2 * 1024 * 1024

char *co_stack1, *co_stack2, *save1, *save2;
ucontext_t ctx1, ctx2;
ucontext_t ctx_main;

void task1()
{
    printf("hello world 1111 \n");

    printf("task1 yield \n");

//    memcpy(save1, ctx1.uc_stack.ss_sp, CO_DEFAULT_STACK_SIZE);
    swapcontext(&ctx1, &ctx_main);
//    memcpy(ctx1.uc_stack.ss_sp, save1, CO_DEFAULT_STACK_SIZE);

    printf("task1 consume \n");
}

void task2()
{
    printf("hello world 2222\n");

    printf("task2 yield \n");

//    memcpy(save1, ctx1.uc_stack.ss_sp, CO_DEFAULT_STACK_SIZE);
    swapcontext(&ctx2, &ctx_main);
//    memcpy(ctx1.uc_stack.ss_sp, save1, CO_DEFAULT_STACK_SIZE);

    printf("task2 consume \n");
}

int main()
{
    static volatile int i = 0;

    co_stack1 = (char *)malloc(CO_DEFAULT_STACK_SIZE);
    co_stack2 = (char *)malloc(CO_DEFAULT_STACK_SIZE);
    save1 = (char *)malloc(CO_DEFAULT_STACK_SIZE);
    save2 = (char *)malloc(CO_DEFAULT_STACK_SIZE);

    memset(co_stack1, 0, CO_DEFAULT_STACK_SIZE);
    memset(co_stack2, 0, CO_DEFAULT_STACK_SIZE);
    memset(save1, 0, CO_DEFAULT_STACK_SIZE);
    memset(save2, 0, CO_DEFAULT_STACK_SIZE);

    if (co_stack1 == NULL) {
        return -1;
    }

    getcontext(&ctx1);
    ctx1.uc_stack.ss_sp = co_stack1;
    ctx1.uc_stack.ss_size = CO_DEFAULT_STACK_SIZE;
    ctx1.uc_link = &ctx_main;
    makecontext(&ctx1, &task1, 0);

    getcontext(&ctx2);
    ctx2.uc_stack.ss_sp = co_stack2;
    ctx2.uc_stack.ss_size = CO_DEFAULT_STACK_SIZE;
    ctx2.uc_link = &ctx_main;
    makecontext(&ctx2, &task2, 0);

//    memcpy(save1, ctx1.uc_stack.ss_sp, CO_DEFAULT_STACK_SIZE);

//    for (i = 0; i < 10; i++) {
//////        memcpy(save1, ctx1.uc_stack.ss_sp, CO_DEFAULT_STACK_SIZE);
////        memcpy(ctx1.uc_stack.ss_sp, save1, CO_DEFAULT_STACK_SIZE);
//////        makecontext(&ctx1, &task1, 0);
////        swapcontext(&ctx_main, &ctx1);
//////        memcpy(ctx1.uc_stack.ss_sp, save1, CO_DEFAULT_STACK_SIZE);
////
//////        memcpy(save2, ctx2.uc_stack.ss_sp, CO_DEFAULT_STACK_SIZE);
//////        swapcontext(&ctx_main, &ctx2);
//////        memcpy(ctx2.uc_stack.ss_sp, save2, CO_DEFAULT_STACK_SIZE);
////    }

    while (i++ < 3) {
        swapcontext(&ctx_main, &ctx2);
        swapcontext(&ctx_main, &ctx1);
    }

    free(co_stack1);
    free(co_stack2);
    free(save1);
    free(save2);
    return 0;
}