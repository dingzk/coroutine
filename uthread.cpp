//
// Created by zhenkai on 2020/5/20.
//

#include "uthread.h"
#include <iostream>
#include <assert.h>
#include <cstdlib>
#include <string.h>

Uthread::Uthread() : sched_running(-1), co_num(0), co_cap(DEFAULT_STACK_NUM)
{
    co = (coroutine_t *) malloc(sizeof(coroutine_t) * DEFAULT_STACK_NUM);
    memset(co, 0, sizeof(coroutine_t) * DEFAULT_STACK_NUM);
}

Uthread::~Uthread()
{
    if (co != nullptr) {
        free(co);
        co = nullptr;
    }
}

void Uthread::main_sched(Uthread *_this)
{
    coroutine_t &C = (_this->co)[_this->sched_running];

    (C.func)(_this, C.args);

    // 正常结束进行销毁
    C.status = DEAD;
    _this->co_num--;
    _this->sched_running = -1;
}

int Uthread::create(CO_FUNC func, void *args)
{
    if (co_num > co_cap) {
        co = (coroutine_t *)realloc(co, co_cap * 2 * sizeof(coroutine_t));
        memset(co + co_cap, 0, co_cap * sizeof(coroutine_t));
        co_cap *= 2;
        assert(co != nullptr);
    }
    int slot;
    for (int i = 0; i < co_cap; ++i) {
        if (co[i].status <= READY) {
            slot = i;
            break;
        }
    }
    assert(slot >= 0);

    coroutine_t &C = co[slot];
    memset(&C, 0, sizeof(coroutine_t));

    co_num ++;
    C.args = args;
    C.func = func;
    sched_running = slot;

    getcontext(&C.ctx);
    C.ctx.uc_stack.ss_sp = C.sp;
    C.ctx.uc_stack.ss_size = STACK_SIZE;
    C.ctx.uc_link = &mctx;
    C.status = ACTIVE;

    makecontext(&C.ctx, (void (*)(void)) main_sched, 1, this);
    swapcontext(&mctx, &C.ctx);

    return slot;
}

void Uthread::resume(int idx)
{
    assert(idx < co_cap && idx >= 0);

    enum STAT status = co[idx].status;
    switch (status) {
        case SUSPEND:
            sched_running = idx;
            swapcontext(&mctx, &co[idx].ctx);
            break;
        default:
            assert(0);
    }
}

void Uthread::yield()
{
    assert(sched_running < co_cap && sched_running >= 0);

    coroutine_t &C = co[sched_running];
    C.status = SUSPEND;
    sched_running = -1;
    swapcontext(&C.ctx, &mctx);
}

int Uthread::get_status()
{
    assert(sched_running < co_cap && sched_running >= 0);

    return co[sched_running].status;
}

int Uthread::get_sched_running()
{
    return sched_running;
}

bool Uthread::finished()
{
    for (int i = 0; i < co_cap; ++i) {
        if (co[i].status > READY) {
            return false;
        }
    }

    return true;
}