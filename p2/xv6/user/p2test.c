#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char **argv) {
  printf(1,"%d\n",getnextpid());
  char state[10] = "test";
  getprocstate(getnextpid()-1,state,7);
  printf(1,"State: %s\n",state);
  exit();
}