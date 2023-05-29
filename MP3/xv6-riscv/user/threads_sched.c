//
#include "kernel/types.h"
//
#include "user/user.h"
//
#include "user/list.h"
//
#include "user/threads.h"
//
#include "user/threads_sched.h"

#define NULL 0

/* default scheduling algorithm */
struct threads_sched_result schedule_default(struct threads_sched_args args) {
    struct thread *thread_with_smallest_id = NULL;
    struct thread *th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (thread_with_smallest_id == NULL || th->ID < thread_with_smallest_id->ID) {
            thread_with_smallest_id = th;
        }
    }

    struct threads_sched_result r;
    if (thread_with_smallest_id != NULL) {
        r.scheduled_thread_list_member = &thread_with_smallest_id->thread_list;
        r.allocated_time = thread_with_smallest_id->remaining_time;
    } else {
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = 1;
    }

    return r;
}

struct thread *get_missed_thread(struct threads_sched_args args) {
    struct thread *th = NULL;
    struct thread *res = NULL;

    // first check for missed threads
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (th->current_deadline <= args.current_time) {
            if (res == NULL || res->ID > th->ID)
                res = th;
        }
    }

    return res;
}

int check_run_queue_empty(struct threads_sched_args args, struct threads_sched_result *r) {
    if (!list_empty(args.run_queue))
        return 0;

    struct release_queue_entry *cur;
    int earliest_ddl = -1;
    list_for_each_entry(cur, args.release_queue, thread_list) {
        if (earliest_ddl == -1)
            earliest_ddl = cur->release_time;
        earliest_ddl = (earliest_ddl > cur->release_time) ? cur->release_time : earliest_ddl;
    }

    r->scheduled_thread_list_member = args.run_queue;
    r->allocated_time = earliest_ddl - args.current_time;
    return 1;
}

#define get_thread_by_least(th, run_queue, prop)               \
    do {                                                       \
        list_for_each_entry(th, run_queue, thread_list) {      \
            if (res == NULL || th->prop <= res->prop) {        \
                if (th->prop == res->prop && th->ID > res->ID) \
                    continue;                                  \
                res = th;                                      \
            }                                                  \
        }                                                      \
    } while (0);

/* Earliest-Deadline-First scheduling */
struct threads_sched_result schedule_edf(struct threads_sched_args args) {
    struct thread *res = NULL;
    struct thread *th = NULL;
    struct threads_sched_result r;

    res = get_missed_thread(args);
    if (res)
        return (struct threads_sched_result){
            .scheduled_thread_list_member = &res->thread_list,
            .allocated_time = 0};

    // if not, find edf
    if (check_run_queue_empty(args, &r)) {
        return r;
    }

    // get edf
    // list_for_each_entry(th, args.run_queue, thread_list) {
    //     if (res == NULL || th->current_deadline <= res->current_deadline) {
    //         if (th->current_deadline == res->current_deadline && th->ID > res->ID)
    //             continue;
    //         res = th;
    //     }
    // }

    get_thread_by_least(th, args.run_queue, current_deadline);

    int allo_time, end_time, max_end_time = res->remaining_time + args.current_time;
    end_time = max_end_time;
    if (end_time > res->current_deadline)
        end_time = res->current_deadline;

    struct release_queue_entry *cur;
    list_for_each_entry(cur, args.release_queue, thread_list) {
        if (cur->release_time < end_time && cur->release_time + cur->thrd->period < res->current_deadline)
            end_time = cur->release_time;
    }
    allo_time = end_time - args.current_time;

    r.scheduled_thread_list_member = &res->thread_list;
    r.allocated_time = allo_time;

    return r;
}

/* Rate-Monotonic Scheduling */
struct threads_sched_result schedule_rm(struct threads_sched_args args) {
    struct thread *res = NULL;
    struct thread *th = NULL;
    struct threads_sched_result r;

    // first try to find already missed thread
    res = get_missed_thread(args);
    if (res)
        return (struct threads_sched_result){
            .scheduled_thread_list_member = &res->thread_list,
            .allocated_time = 0};

    // check if run_queue is empty
    // if so, return and sleep
    if (check_run_queue_empty(args, &r)) {
        return r;
    }

    get_thread_by_least(th, args.run_queue, period);

    int allo_time, end_time, max_end_time = res->remaining_time + args.current_time;
    end_time = max_end_time;
    if (end_time > res->current_deadline)
        end_time = res->current_deadline;

    struct release_queue_entry *cur;
    list_for_each_entry(cur, args.release_queue, thread_list) {
        if (cur->release_time < end_time) {
            if (cur->thrd->period < res->period ||
                (cur->thrd->period == res->period && cur->thrd->ID < res->ID))
                end_time = cur->release_time;
        }
    }

    allo_time = end_time - args.current_time;

    r.scheduled_thread_list_member = &res->thread_list;
    r.allocated_time = allo_time;

    return r;
}
