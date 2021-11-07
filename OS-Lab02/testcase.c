#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(void)
{
  printf(1, "Hello, world!\n");

  hello();  //print Hello

  char* name = "name_string";
  helloYou(name);

  int t_np = getNumProc();
  printf(1, "total number of active processes: %d \n", t_np);

  int max_pid = getMaxPid();
  printf(1, "maximum PID: %d \n", max_pid);

  exit();
}
