
#include "types.h"
#include "stat.h"
#include "user.h"

// Test that pages are not copied when process forks

int main(void)
{
	int iters = 100;
	for(int i = 0; i < iters; i++){
		sbrk(4096);
	}
	
	int numFreePagesInitial = getFreePagesCount();
	// printf(1, "Initial: Free=%d\n", numFreePagesInitial);

	int pid = fork();

	if(pid == 0) {
		int num_free_after_fork = getFreePagesCount();
		// printf(1, "Final: Free=%d\n", num_free_after_fork);

		if(numFreePagesInitial - num_free_after_fork >= iters) {
			printf(1, "XV6_COW\t FAILED\n");
			exit();
		}
		for(int i = 0; i < iters; i++){
			sbrk(-4096);
		}

		int numFreePagesFinal2 = getFreePagesCount();
		// printf(1, "Final2: Free=%d\n", numFreePagesFinal2);

		if(numFreePagesFinal2 != num_free_after_fork) {
			printf(1, "XV6_COW\t FAILED\n");
			exit();
		} else {
			printf(1, "XV6_COW\t SUCCESS\n");
		}
		exit();
	} else {
		wait();
	}


 	exit();
}