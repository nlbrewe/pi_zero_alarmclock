//
// Created by nlb on 3/29
//
#include <unistd.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/shm.h>		//Used for shared memory
#include <iostream>
#include <cstring>
#include <getopt.h>
#include "sharedmem.h"

void *shared_memory1_pointer = (void *)0;
//VARIABLES:
struct shared_memory1_struct *shared_memory1;
int shared_memory1_id;

int main(int argc, char** argv)
{
	int flag24H, opt, exflg;
    int volume, tfnd;

	
	//parse command line args
    volume = 0;
    tfnd = 0;
    flag24H = 0;
    exflg = 0;
    
    while ((opt = getopt(argc, argv, "fv:e")) != -1) {
        switch (opt) {
        case 'f':  //24H format
            flag24H = 1;
            break;
        case 'v': //volume 
            volume = atoi(optarg);
            tfnd = 1;
            break;
        case 'e':
            exflg = 1;  //exit flag
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-f] [-v nnn] [e]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
		}
	}
	printf("24Hformat=%d; volume = %d; exflg=%d; optind=%d\n", flag24H, volume, exflg, optind);

	//--------------------------------
	//----- CREATE SHARED MEMORY -----
	//--------------------------------
	printf("Creating shared memory...\n");
	shared_memory1_id = shmget((key_t)2417, sizeof(struct shared_memory1_struct), 0666 | IPC_CREAT);		//<<<<< SET THE SHARED MEMORY KEY    (Shared memory key , Size in bytes, Permission flags)

	if (shared_memory1_id == -1)
	{
		fprintf(stderr, "Shared memory shmget() failed\n");
		exit(EXIT_FAILURE);
	}

	//Make the shared memory accessible to the program
	shared_memory1_pointer = shmat(shared_memory1_id, (void *)0, 0);
	if (shared_memory1_pointer == (void *)-1)
	{
		fprintf(stderr, "Shared memory shmat() failed\n");
		exit(EXIT_FAILURE);
	}
	printf("Shared memory attached at %X\n", (int)shared_memory1_pointer);
	shared_memory1 = (struct shared_memory1_struct *)shared_memory1_pointer;
	
	shared_memory1->b24Hformat = (flag24H == 1);
	shared_memory1->volume = volume;
	shared_memory1->bExit = (exflg == 1);	//Assign the shared_memory segment
	
	printf("current time: %s\n",shared_memory1->digits);  
	printf("alarm on thumbwheel set to: %s\n",shared_memory1->thumbdigits);  
	printf("iMode: %d\n",shared_memory1->iMode);  	
	printf("volume: %d\n",shared_memory1->volume);  	
	printf("24hr format: %d\n",shared_memory1->b24Hformat); 
	printf("Exit flag: %d\n",shared_memory1->bExit); 
	printf("tune: %s\n",shared_memory1->tune); 
	printf("hidden alarm: %s\n",shared_memory1->hiddenalarm); 
	
	//shared_memory1->bExit = true;
	
	//--------------------------------
	//----- DETACH SHARED MEMORY -----
	//--------------------------------
	//Detach
	if (shmdt(shared_memory1_pointer) == -1)
	{
		fprintf(stderr, "shmdt failed\n");
		//exit(EXIT_FAILURE);
	}
}//end of main
