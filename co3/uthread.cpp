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

    memset(&mctx, 0, sizeof(ucontext_t));

    main_stack = (char *) malloc(sizeof(char) *STACK_SIZE);
    memset(main_stack, 0, sizeof(char) *STACK_SIZE);

    co = (coroutine_t **) malloc(sizeof(coroutine_t *) * co_cap);
    memset(co, 0, sizeof(coroutine_t *) * co_cap);
}

Uthread::~Uthread()
{
    if (co != nullptr) {
        for (auto i = 0; i < co_cap; i++) {
            if (co[i] != nullptr) {
                if (co[i]->sp != nullptr) {
                    free(co[i]->sp);
                }
                free(co[i]);
                co[i] = nullptr;
            }
        }
        free(co);
        co = nullptr;
    }

    if (main_stack != nullptr) {
        free(main_stack);
        main_stack = nullptr;
    }
}

void Uthread::main_sched(Uthread *_this)
{
    coroutine_t *C = (_this->co)[_this->sched_running];

    (C->func)(_this, C->args);

    // 正常结束进行销毁
    free(C->sp);
    C->sp = nullptr;
    C->sp_len = 0;
    C->status = DEAD;

    _this->co_num--;
    _this->sched_running = -1;
}

int Uthread::create(CO_FUNC func, void *args)
{
    if (co_num >= co_cap) {
        co = (coroutine_t **)realloc(co, sizeof(coroutine_t*) * co_cap * 2);
        memset(co + co_cap, 0, sizeof(coroutine_t*) * co_cap);
        co_cap *= 2;
    }

    int slot = -1;
    for (int i = 0; i < co_cap; ++i) {
        if (co[i] != nullptr && co[i]->status < READY) {
            slot = i;
            break;
        } else if (co[i] == nullptr) {
            slot = i;
            co[i] = (coroutine_t *) malloc(sizeof(coroutine_t));
            break;
        }
    }
    assert(slot >= 0);

    coroutine_t *C = co[slot];
    memset(C, 0, sizeof(coroutine_t));

    co_num ++;
    C->args = args;
    C->func = func;
    C->status = READY;

    return slot;
}

void Uthread::resume(int idx)
{
    assert(idx < co_cap && idx >= 0);

    coroutine_t *C = co[idx];
    switch (C->status) {
        case READY:
            getcontext(&C->ctx);
//            memset(main_stack, 0, STACK_SIZE);
            C->ctx.uc_stack.ss_sp = main_stack;
            C->ctx.uc_stack.ss_size = STACK_SIZE;
            C->ctx.uc_link = &mctx;
            C->status = ACTIVE;
            sched_running = idx;
            makecontext(&C->ctx, (void (*)(void)) main_sched, 1, this);
            swapcontext(&mctx, &C->ctx);
            break;
        case SUSPEND:
            sched_running = idx;
            C->status = ACTIVE;
            swap_in(C);
            swapcontext(&mctx, &C->ctx);
            break;
        case DEAD:
            break;
        default:
            assert(0);
    }
}

// swap in
void Uthread::swap_in(coroutine_t *C)
{
//    memset(main_stack, 0, sizeof(char) *STACK_SIZE);
    memcpy(main_stack + STACK_SIZE - C->sp_len, C->sp, C->sp_len);
}

//swap out
void Uthread::swap_out(coroutine_t *C)
{
    char dummy = 0;
    int len = main_stack + STACK_SIZE - &dummy;
    assert(len > 0 && len <= STACK_SIZE);
    if (C->sp_len < len) {
        C->sp = (char *)realloc(C->sp, len * sizeof(char));
    }
    C->sp_len = len;
    memcpy(C->sp, &dummy, C->sp_len);

    std::cout << "stack size : " << C->sp_len << std::endl;
}

void Uthread::yield()
{
    assert(sched_running < co_cap && sched_running >= 0);

    coroutine_t *C = co[sched_running];
    C->status = SUSPEND;
    sched_running = -1;

    swap_out(C);

    swapcontext(&C->ctx, &mctx);
}

bool Uthread::is_running(int idx)
{
    assert(idx < co_cap && idx >= 0);

    return co[idx]->status >= READY;
}

int Uthread::get_status(int idx)
{
    assert(idx < co_cap);

    return idx < 0 ? co[sched_running]->status : co[idx]->status;
}

int Uthread::get_sched_running()
{
    return sched_running;
}

bool Uthread::finished()
{
    if (sched_running == -1) {
        return true;
    }
    for (int i = 0; i < co_cap; ++i) {
        if (co[i]->status > READY) {
            return false;
        }
    }

    return true;
}