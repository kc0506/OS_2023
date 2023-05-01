// 1
#include "../kernel/types.h"
// 2
#include "../user/setjmp.h"
// 3
#include "../user/threads.h"
// 4
#include "../user/user.h"
#define NULL 0

static struct thread *current_thread = NULL;
static int id = 1;
static jmp_buf env_st;
static jmp_buf env_tmp;

struct thread *thread_create(void (*f)(void *), void *arg) {
    struct thread *t = (struct thread *)malloc(sizeof(struct thread));
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long)malloc(sizeof(unsigned long) * 0x100);
    new_stack_p = new_stack + 0x100 * 8 - 0x2 * 8;
    t->fp = f;
    t->arg = arg;
    t->ID = id;
    t->buf_set = 0;
    t->stack = (void *)new_stack;
    t->stack_p = (void *)new_stack_p;
    id++;

    // part 2
    t->sig_handler[0] = NULL_FUNC;
    t->sig_handler[1] = NULL_FUNC;
    t->signo = -1;
    t->handler_buf_set = 0;
    return t;
}
void thread_add_runqueue(struct thread *t) {
    if (current_thread == NULL) {
        // TODO
        current_thread = t;
        t->next = t->previous = current_thread;
    } else {
        // TODO
        struct thread *prev = current_thread->previous;
        prev->next = t;
        current_thread->previous = t;

        t->previous = prev;
        t->next = current_thread;

        t->sig_handler[0] = current_thread->sig_handler[0];
        t->sig_handler[1] = current_thread->sig_handler[1];
    }
}
void thread_yield(void) {
    // TODO
    struct thread *cur = current_thread;

    if (cur->signo == -1) {
        int jmp_val = setjmp(cur->env);
        if (!jmp_val) {
            cur->buf_set = 1;
            schedule();
            dispatch();
        }
    } else {
        int jmp_val = setjmp(cur->handler_env);
        setjmp(env_tmp);
        if (!jmp_val) {
            cur->handler_buf_set = 1;
            schedule();
            dispatch();
        }
    }
}

void dispatch(void) {
    // TODO

    struct thread *cur = current_thread;
    // struct jmp_buf_data *tmp;
    int signo = cur->signo;

    if (signo != -1) {
        void (*handler)(int) = cur->sig_handler[signo];
        if (handler == NULL_FUNC) {
            thread_exit();
        }

        // handle
        if (cur->handler_buf_set) {
            longjmp(cur->handler_env, 0);
        } else {
            if (!setjmp(env_tmp)) {  // get current ra
                if (cur->buf_set) {
                    env_tmp->sp = cur->env->sp;  // get previous sp
                } else {
                    env_tmp->sp = (unsigned long)cur->stack_p;  // init sp
                }
                longjmp(env_tmp, 0);  // jump back with correct sp
            }

            handler(signo);
            cur = current_thread;
            cur->signo = -1;
        }
    }

    if (cur->buf_set) {
        // load from context
        longjmp(cur->env, 0);
    } else {
        // initialize

        if (!cur->handler_buf_set) {
            if (!setjmp(env_tmp)) {  // get current ra
                env_tmp->sp = (unsigned long)cur->stack_p;  // modify sp
                longjmp(env_tmp, 0);                        // jump back with correct sp
            }
        }
        cur->fp(cur->arg);
        thread_exit();
    }
}
void schedule(void) {
    // TODO
    current_thread = current_thread->next;
}
void thread_exit(void) {
    if (current_thread->next != current_thread) {
        // TODO
        struct thread *nxt = current_thread->next;
        nxt->previous = current_thread->previous;
        current_thread->previous->next = nxt;
        free(current_thread);
        current_thread = nxt;
        dispatch();
    } else {
        // TODO
        // Hint: No more thread to execute
        free(current_thread);
        current_thread = NULL;
        longjmp(env_st, 0);
    }
}
void thread_start_threading(void) {
    // TODO
    // dispatch();

    // save initial state
    // printf("start......\n\n");

    while (current_thread != NULL) {
        int jmp_val = setjmp(env_st);
        if (!jmp_val) {
            dispatch();
        }
    }
    // printf("end!\n\n");
    return;
}
// part 2
void thread_register_handler(int signo, void (*handler)(int)) {
    // TODO
    current_thread->sig_handler[signo] = handler;
}
void thread_kill(struct thread *t, int signo) {
    // TODO
    t->signo = signo;
}