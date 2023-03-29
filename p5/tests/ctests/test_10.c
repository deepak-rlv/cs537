
#include "types.h"
#include "stat.h"
#include "user.h"

// Test reference counting for CoW- new allocations from child don't affect parent

int main(void) {
    int pg_sz = 4096;
    sbrk(pg_sz);
    sbrk(10 * pg_sz);

    int init_free = getFreePagesCount();
    // printf(1, "Initial: Free=%d\n", init_free);

	int pid = fork();

    if(pid == 0){
        // int free_after_fork = getFreePagesCount();
        // printf(1, "Free after fork: %d\n", free_after_fork);
        
        sbrk(pg_sz); // allocate 1 page and never free

        // int free_pgs_after_alloc_1 = getFreePagesCount();

        // printf(1, "Free after allocation in child: %d\n", free_pgs_after_alloc_1);
        exit();
    } else {
        wait();
    }

    int final_free = getFreePagesCount();

    // printf(1, "Final free: %d\n", final_free);

    if(final_free == init_free){
        printf(1, "XV6_COW\t SUCCESS\n");
    } else {
        printf(1, "XV6_COW\t FAILED\n");
    }


 	exit();
}