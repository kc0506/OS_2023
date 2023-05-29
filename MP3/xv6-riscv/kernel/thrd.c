//
#include "types.h"
//
#include "riscv.h"
//
#include "defs.h"
//
#include "date.h"
//
#include "param.h"
//
#include "memlayout.h"
//
#include "spinlock.h"
//
#include "proc.h"
//

// for mp3
uint64
sys_thrdstop(void) {
    int delay;
    uint64 context_id_ptr;
    uint64 handler, handler_arg;
    if (argint(0, &delay) < 0)
        return -1;
    if (argaddr(1, &context_id_ptr) < 0)
        return -1;
    if (argaddr(2, &handler) < 0)
        return -1;
    if (argaddr(3, &handler_arg) < 0)
        return -1;

    // TODO: mp3

    struct proc* p;
    if ((p = myproc()) == 0)
        return -1;

    int context_id;
    copyin(p->pagetable, (char*)&context_id, context_id_ptr, sizeof(context_id));

    if (context_id < -1 || context_id >= MAX_THRD_NUM)
        return -1;

    struct thrd* t = 0;
    if (context_id == -1) {
        for (int i = 0; i < MAX_THRD_NUM; i++)
            // find a free thrd
            if (p->thrds[i].state == FREE) {
                t = &p->thrds[i];
                break;
            }
        if (t == 0)
            return -1;
    } else {
        t = &p->thrds[context_id];
        // this id should be valid
        // but not waiting, since there will be at most 1 thrd at a time
        if (t->state == FREE || t->state == WAITING)
            return -1;
    }

    t->state = WAITING;
    t->delay0 = delay;
    t->delay = delay;
    t->handler = (void*)(void*)handler;
    t->arg = (void*)handler_arg;

    copyout(p->pagetable, context_id_ptr, (char*)&t->id, sizeof(int));

    p->current_thrd = t->id;

    return 0;
}

// for mp3
uint64
sys_cancelthrdstop(void) {
    int context_id, is_exit;
    if (argint(0, &context_id) < 0)
        return -1;
    if (argint(1, &is_exit) < 0)
        return -1;

    // TODO: mp3

    struct proc* p = myproc();
    struct thrd* t;

    // feature 1: cancel current handler
    int res;
    if (p->current_thrd == -1)
        return 0;
    t = &p->thrds[p->current_thrd];
    t->state = STOPPED;
    res = t->delay0 - t->delay;

    // feature 2: store context or recycle id
    if (context_id < 0 || context_id >= MAX_THRD_NUM) {
        return -1;
    }
    t = &p->thrds[context_id];
    if (is_exit == 0) {
        // store
        if (t->state == FREE)
            return -1;
        memmove(t->context, p->trapframe, sizeof(struct trapframe));
        t->state = STOPPED;
    } else {
        // recycle
        t->state = FREE;
    }

    return res;
}

// for mp3
uint64
sys_thrdresume(void) {
    // printf("call resume.\n");

    int context_id;
    if (argint(0, &context_id) < 0)
        return -1;

    struct proc* p = myproc();
    if (p == 0)
        return -1;

    // TODO: mp3

    if (context_id < 0 || context_id >= MAX_THRD_NUM)
        return -1;

    struct thrd* t = &p->thrds[context_id];
    if (t->state == FREE)  // invalid id
        return -1;

    if (t->context == 0)
        return -1;

    memmove(p->trapframe, t->context, sizeof(struct trapframe));
    return 0;
}
