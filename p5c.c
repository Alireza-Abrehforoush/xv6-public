#include "types.h"
#include "stat.h"
#include "user.h"
#include <stdbool.h>

int main(int argc, char *argv[])
{
  int child[5];
  bool is_child = 0;
  for (int i = 0; i < 5; i++)
	{
		child[i] = fork();
		if(child[i] == 0)
		{
      is_child = 1;
      break;
		}
	}
	while (is_child==1)
	{
    int a = 0;
    for(int i = 0; i < 200; i++)
    {
      a++;
    }
	}
  while(is_child == 0)
  {
    wait();
  }
  exit();
}
