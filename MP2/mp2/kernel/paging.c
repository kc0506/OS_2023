//
#include "param.h"
//
#include "types.h"
//
#include "memlayout.h"
//
#include "riscv.h"
//
#include "spinlock.h"
//
#include "defs.h"
//
#include "proc.h"

/* NTU OS 2022 */
/* Page fault handler */

// Activate va
pte_t* lazypg(pagetable_t pagetable, uint64 va) {
    char* mem;
    pte_t* pte;
    uint64 blockno;

    pte = walk(pagetable, va, 0);
    if (pte && *pte & PTE_V)
        return pte;

    // The actual pa for va has not been mapped yet.
    // printf("handling fault\n");
    if (*pte & PTE_S) {
        blockno = PTE2BLOCKNO(*pte);
        mem = kalloc();

        begin_op();
        read_page_from_disk(ROOTDEV, mem, blockno);
        bfree_page(ROOTDEV, blockno);
        end_op();

        *pte = *pte & ~PTE_S;
        if (mappages(pagetable, PGROUNDDOWN(va), PGSIZE, (uint64)mem, PTE_FLAGS(*pte))) {
            kfree(mem);
            return 0;
        }

    } else {
        mem = kalloc();
        if (memset(mem, 0, PGSIZE) == 0) {
            return 0;
        }

        if (mappages(pagetable, PGROUNDDOWN(va), PGSIZE, (uint64)mem, PTE_W | PTE_X | PTE_R | PTE_U)) {
            kfree(mem);
            return 0;
        }
        pte = walk(pagetable, va, 0);
    }

    return pte;
}

int handle_pgfault() {
    /* Find the address that caused the fault */

    uint64 va;
    struct proc* p;

    p = myproc();
    va = r_stval();

    // printf("Handling page fault %p\n", va);

    if (va >= p->sz || va < p->trapframe->sp) {
        p->killed = 1;
        return -1;
    }

    // printf("%p\n", walk(p->pagetable, va, 0));

    if (lazypg(p->pagetable, PGROUNDDOWN(va)) == 0) {
        return -1;
    }

    return 0;
}
