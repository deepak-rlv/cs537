
#include "types.h"
#include "stat.h"
#include "user.h"

// Test that sbrk() reduces the number of free pages

int main(void)
{
	int iters = 100;
	int numFreePagesInitial = getFreePagesCount();
	
	// printf(1, "Initial: Free=%d\n", numFreePagesInitial);
	
	for(int i = 0; i < iters; i++){
		sbrk(4096);
	}

	int numFreePagesFinal = getFreePagesCount();
	// printf(1, "Final: Free=%d\n", numFreePagesFinal);

	if(numFreePagesFinal == numFreePagesInitial - iters) {
		printf(1, "XV6_COW\t SUCCESS\n");
	} else {
		printf(1, "XV6_COW\t FAILED\n");
	}

 	exit();
}