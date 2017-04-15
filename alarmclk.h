#include <unistd.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>		//Used for shared memory

#include <time.h>		//Used for shared memory

#include <time.h>

void *shared_memory1_pointer = (void *)0;
//VARIABLES:
struct shared_memory1_struct *shared_memory1;
int shared_memory1_id;
int ShowTime(char s[],int iBrightness, int iBlinkSpd);
int ShowMsg(char[]);
long ReadLightIntensity(void);
