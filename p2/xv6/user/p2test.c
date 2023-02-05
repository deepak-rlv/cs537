#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char **argv) {
  char state[10] = "test";
  printf(1,"Process ID: %d\n",getnextpid()-1);
  getprocstate(getnextpid()-1,state,sizeof(state));
  printf(1,"State of next process: %s\n",state);
  exit();
}