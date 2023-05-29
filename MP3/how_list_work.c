






// e.g. (thread, thread_list)
#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)


#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) ); })


// e.g. (head->next, thread, thread_list)  
//      typeof(thread.thread_list) = *list_head
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)


// e.g. list_for_each_entry(th, args.run_queue, thread_list) 
#define list_for_each_entry(pos, head, member)                 \
    for (pos = list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head);                               \
         pos = list_entry(pos->member.next, typeof(*pos), member))

