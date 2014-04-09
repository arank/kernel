#ifndef _H_COREMAP_H_
#define _H_COREMAP_H_

#define KERNEL_CMI 1

void cm_bootstrap(void);

int core_set_busy(int index, bool wait);

int core_set_free(int index);

paddr_t get_free_cme(vaddr_t vpn, bool kern);

int stat_coremap(int nargs, char **args);

struct cme {
    uint32_t vpn:       20,
             pid:       9,
             busybit:   1,
             use:       1,
             kern:      1;
    uint32_t swap:      15, /* swap index */
             slen:      10, /* sequence length */
             seq:       1,  /* this bit is 1 if it's a sequence entry except for the first one */
             dirty:     1,
             ref:       1,
             junk:      4;
};

struct coremap {
    struct spinlock lock;
    unsigned free;
    unsigned modified;

    unsigned kernel;
    //unsigned user;
    unsigned busy;
    unsigned used;
    //unsigned swap;
    //unsigned ref;

    unsigned size;
    struct cme *cm;
    int last_allocated;
} coremap;

#endif

