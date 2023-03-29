
#include "types.h"
#include "stat.h"
#include "user.h"

// Test reference counting for CoW

int main(void) {
    int pg_sz = 4096;
    sbrk(pg_sz);
    sbrk(10 * pg_sz);

    // int init_free = getFreePagesCount();
    // printf(1, "Initial: Free=%d\n", init_free);

	int pid = fork();

    if(pid == 0){
        int free_after_fork = getFreePagesCount();
        // printf(1, "Free after fork: %d\n", free_after_fork);
        
        sbrk(-pg_sz); // free 1 page

        int free_pgs_after_free_1 = getFreePagesCount();

        sbrk(-10 * pg_sz); // free 10 pages

        int free_pgs_after_free_2 = getFreePagesCount();

        // printf(1, "Free after free 1: %d, free after free 2: %d\n", free_pgs_after_free_1, free_pgs_after_free_2);

        // Free pages should be the same as before fork- as only reference count was decremented to 1
        if(free_after_fork == free_pgs_after_free_1 && free_pgs_after_free_1 == free_pgs_after_free_2){
            printf(1, "XV6_COW\t SUCCESS\n");
        } else {
            printf(1, "XV6_COW\t FAILED\n");
        }
    } else {
        wait();
    }

 	exit();
}