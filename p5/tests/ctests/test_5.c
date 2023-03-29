
#include "types.h"
#include "stat.h"
#include "user.h"

// Test that fork() does not copy the entire page table

int main(void) {
    int sz = 500;
    int* arr = (int*)malloc(sz * sizeof(int));

    sbrk(100 * 4096); // Allocate 100 pages

    for(int i = 0; i < sz; i++){
        arr[i] = i;
    }

    int x = 0; // x allocated on stack

    int init_free = getFreePagesCount();
    // printf(1, "Initial: Free=%d\n", init_free);

	int pid = fork();

    if(pid == 0){
        int free_after_fork = getFreePagesCount();
        // printf(1, "Free after fork: %d\n", free_after_fork);

        if(init_free - free_after_fork > 100){
            printf(1, "XV6_COW\t FAILED\n");
            exit();
        }
        
        // Do some reads to array
        for(int i = 0; i < sz; i++){
            x += arr[i];
        }

        int free_after_reads = getFreePagesCount();
        // printf(1, "Free after reads: %d\n", free_after_reads);
        
        if(free_after_fork != free_after_reads){
            printf(1, "XV6_COW\t FAILED\n");
        }else {
            printf(1, "XV6_COW\t SUCCESS\n");
        }
    } else {
        wait();
    }

 	exit();
}