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
#include "clparms.h"  //shared with alarmclk.cpp

void *shared_memory1_pointer = (void *)0;

struct shared_memory1_struct *shared_memory1;
int shared_memory1_id;

int main(int argc, char** argv)
{

 	//parse command line args
    while ((opt = getopt(argc, argv, "f:v:h:eqt:p")) != -1) {
        switch (opt) {
		case 'f':  //24H format if flag found
			clflag24H = atoi(optarg);
			if (clflag24H != 12)clflag24H = 24;
			ffnd = 1;
			break;
		case 'v': //volume -6000 to 0
			clvolume = atoi(optarg);
			vfnd = 1;
			break;
		case 'h': //hidden alarm time 0600 for example
			clihiddenalarm = atoi(optarg);
			hfnd = 1;
			break;   
		case 'e':  //exit
			clExitflg = 1;
			break;        
		case 'q':  //simple query
			clqflg = 1;
			break; 
		case 't': //testduration in seconds
            strncpy(clsndtest,optarg,sizeof(clsndtest));
			tfnd = 1;
            break;            	
		case 'p':  //progressive
			clprogflg = 1;
			break;
		default: /* '?' */
			fprintf(stderr, "Usage: [-h nnnn] [-f12/24] [-v -nnn] [-e] [tune]\n");
			exit(EXIT_FAILURE);
		}
	}
	//printf("optind,argc %d,%d\n",optind,argc);
	if (optind < argc) {
		strncpy(cltune,argv[optind],sizeof(cltune));
	}
/*	
	printf("clflag24H=%d\n",clflag24H);
	printf("clvolume=%d\n",clvolume);
	printf("cltune=%s\n",cltune);
	printf("clihiddenalarm=%d\n",clihiddenalarm);
	printf("clExitflg=%d\n",clExitflg);
	printf("clqflg=%d\n",clqflg);
	printf("clprogflg=%d\n",clprogflg);
	printf("clsndtest=%s\n",clsndtest);
	printf("vfnd=%d\n",vfnd);
	printf("hfnd=%d\n",hfnd);
	printf("ffnd=%d\n",ffnd);
	printf("tfnd=%d\n",tfnd);
*/	

	//--------------------------------
	//----- CREATE SHARED MEMORY -----
	//--------------------------------
//	printf("Creating shared memory...\n");
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
	//printf("Shared memory attached at %X\n", (int)shared_memory1_pointer);
	shared_memory1 = (struct shared_memory1_struct *)shared_memory1_pointer;

	if (clqflg){  //query and exit response
		printf("%s,",shared_memory1->digits);  
		printf("%s,",shared_memory1->thumbdigits);  
		printf("%s,",shared_memory1->hiddenalarm);  	
		printf("%d,",shared_memory1->volume);  
		printf("%d,",shared_memory1->b24Hformat); 
		printf("%s,",shared_memory1->tune);
		printf("%d",shared_memory1->bAlarmStatus);  
		exit(0);
	}
	
 	if(hfnd && clihiddenalarm > 0 && clihiddenalarm < 2400){  //hiddenalarm found, set value
		snprintf(shared_memory1->hiddenalarm,sizeof(shared_memory1->hiddenalarm),"%04d",clihiddenalarm);
	} 
 	if (vfnd && clvolume > -6000 && clvolume <= 0) {  //set volume
		shared_memory1->volume=clvolume;
	}
	if (ffnd) {  //set hour forat
		if(clflag24H == 24){
			shared_memory1->b24Hformat=true;
		} else {
			shared_memory1->b24Hformat=false;
		}
	} 
	if(tfnd){
		strncpy(shared_memory1->sndtest,clsndtest,sizeof(shared_memory1->sndtest));
	} 

	if(clprogflg == 1){
		shared_memory1->progressive = 1;
	} 

 	if (strlen(cltune) > 0) {
		strncpy(shared_memory1->tune,cltune,sizeof(shared_memory1->tune));
	}
	
	shared_memory1->bExit=clExitflg;
	
	printf("current time: %s\n",shared_memory1->digits);  
	printf("alarm on thumbwheel set to: %s\n",shared_memory1->thumbdigits);  
	printf("hiddenalarm: %s\n",shared_memory1->hiddenalarm);  	
	printf("iMode: %d\n",shared_memory1->iMode);  	
	printf("volume: %d\n",shared_memory1->volume);  
	printf("24hr format: %d\n",shared_memory1->b24Hformat); 
	printf("Exit flag: %d\n",shared_memory1->bExit); 
	printf("Progressive flag: %d\n",shared_memory1->progressive); 
	printf("Soundtest flag: %s\n",shared_memory1->sndtest); 
	printf("Alarm sound file: %s\n",shared_memory1->tune); 
	
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
