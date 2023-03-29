
#include "types.h"
#include "stat.h"
#include "user.h"

// Test that pages copied are equal to the pages written to - Modify non-contiguous pages

int main(void) {
    int sz = 1000, sz2 = 10001;
    int* arr = (int*)malloc(sz * sizeof(int)); // occupies 1 page
	int* arr2 = (int*)malloc(sz2 * sizeof(int)); // occupies 10 pages

    for(int i = 0; i < sz; i++){
        arr[i] = i;
    }
	for(int i = 0; i < sz2; i++){
		arr2[i] = i;
	}

    // int init_free = getFreePagesCount();
    // printf(1, "Initial: Free=%d\n", init_free);

	int pid = fork();

    if(pid == 0){
        int free_after_fork = getFreePagesCount();
        // printf(1, "Free after fork: %d\n", free_after_fork);
        
        // Do some writes to array 1
        for(int i = 0; i < sz; i++){
            arr[i] += arr2[i];
        }
        int free_after_writes_1 = getFreePagesCount();
		// printf(1, "Free after first write: %d\n", free_after_writes_1);

		// Do some writes to array 2 - Only these pages should be copied
		arr2[sz2/10] = 1; // 1st page
        arr2[4 * sz2/10] = 4; // 4th page
        arr2[8 * sz2/10] = 8; // 8th page

		int free_after_writes_2 = getFreePagesCount();

        // printf(1, "Free after second write: %d\n", free_after_writes_2);
        
        if((free_after_fork != free_after_writes_1 + 1) || (free_after_writes_1 != free_after_writes_2 + 3)){
            printf(1, "XV6_COW\t FAILED\n");
        }else {
			printf(1, "XV6_COW\t SUCCESS\n");
        }
    } else {
        wait();
    }

 	exit();
}