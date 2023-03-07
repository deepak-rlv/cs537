#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(int argc, char *argv[]){
	int ticketsA = 3;
	int pid = getpid();
	int i=0;
	struct pstat st;
	while(i<5){
	if(settickets(ticketsA) == 0 && getpinfo(&st) == 0) {
		printf(1,"Process A stats\nTickets: %d\nStrides: %d\nPass: %d\nPID: %d\nTicks: %d\n",st.tickets[pid - 1],st.strides[pid - 1],st.pass[pid - 1],st.pid[pid - 1],st.ticks[pid - 1]);
		i++;
		sleep(1);
		sleep(1);
		sleep(1);
	}
	else {
	 printf(1, "XV6_SCHEDULER\t FAILED\n");
	 exit();
	}
	}
  	// while(wait() > 0);
	exit();
}
