//
// Created by zhenkai on 2020/5/20.
//

#ifndef COROUTINE_UTHREAD_H
#define COROUTINE_UTHREAD_H

#if __APPLE__ && __MACH__
#include <sys/ucontext.h>
#else
#include <ucontext.h>
#endif

#define STACK_SIZE 1024*128
#define DEFAULT_STACK_NUM 16

class Uthread
{
private:
    typedef void(*CO_FUNC)(Uthread *, void *args);
    enum STAT {DEAD = -1, READY, ACTIVE, SUSPEND};

    struct coroutine_t {
        ucontext_t ctx;
        enum STAT status;
        char *sp;
        int sp_len;
        CO_FUNC func;
        void *args;
    };

    int sched_running;  // 当前被调度的协程索引
    int co_num;         // 当前有多少协程
    int co_cap;         // 一共可以存多少协程
    ucontext_t mctx;    // 主调度进程
    char *main_stack;

    coroutine_t **co;

protected:
    static void main_sched(Uthread *ptr);
    void swap_out(coroutine_t *C);
    void swap_in(coroutine_t *C);

public:
    Uthread();
    ~Uthread();
    int create(CO_FUNC func, void *args);
    void resume(int idx);
    void yield();
    bool is_running(int idx);
    int get_status(int idx = -1);
    bool finished();
    int get_sched_running();

};


#endif //COROUTINE_UTHREAD_H
