//
// Created by nlb on 3/14/17
//
#include <unistd.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>		//Used for shared memory
#include <iostream>
#include <cstring>
#include <time.h>
#include <getopt.h>
#include "SparkFunSX1509.h"  //i2c i/o extender
#include "7seg_bp_ada.h"     //adafruit backback 7seg 4 digit display
#include "wiringPi.h"
#include "sharedmem.h"       //shared memory map
#include "alarmclk.h"
#include "clparms.h"

int ReadThumbwheel(SX1509 io, char thumbdigits[]);
const unsigned char SX1509_ADDRESS = 0x3E;  // SX1509 I2C address for thumbwheel switch
int rc;
time_t rawtime;
struct tm *info;  
char command[80];  //command to play audio
char sounddir[80] = "/home/pi/alarmsounds/";
int iBrightness = 15;  //brightness of LED 
int ChildPID;  //PID of sound playing app
int status;  //used in waitPID call
bool bToggle = false;
long AmbientLight;
pid_t result;
char tmp[5];  //compose message for LED
char tunefullpath[160];  //holding place for full path to tune

int main(int argc, char** argv)
{
	if (wiringPiSetup () < 0) {  //set up libraries
		fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
		return 1;
	}
	auto io = SX1509();
	if(!io.begin(SX1509_ADDRESS))
	{
		std::cout << "Init for SX1509 failed" << std::endl;
		return 1;
	}
	
	for(int i=0;i<15;i++){  //set up IO expander 15 inputs with pullups, one active low output
	io.pinMode(i, INPUT_PULLUP);
	}  
 	io.pinMode(15, OUTPUT);  //alarm armed LED
 	
	//parse command line args
    while ((opt = getopt(argc, argv, "f:v:h:e")) != -1) {
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
       case 'e':  //for consistency with testshm
			clExitflg = 1;
			break;
        default: /* '?' */
            fprintf(stderr, "Usage: [-h nnnn] [-f12/24] [-v -nnn] [-e] [tune]\n");
            exit(EXIT_FAILURE);
		}
	}
	if (optind >= argc) {
		fprintf(stderr, "playing default tune\n");
		strncpy(cltune,"moon.wav",sizeof(cltune));
	} else{
		strncpy(cltune,argv[optind],sizeof(cltune));
		printf("tune argument = %s\n", argv[optind]);
	}
 	
	//Key parameters stored in shared memory so other programs can access
	printf("Creating shared memory...\n");
	shared_memory1_id = shmget((key_t)2417, sizeof(struct shared_memory1_struct), 0666 | IPC_CREAT);		//<<<<< SET THE SHARED MEMORY KEY    (Shared memory key , Size in bytes, Permission flags)
	//	Shared memory key
	//		Unique non zero integer (usually 32 bit).  Needs to avoid clashing with another other processes shared memory (you just have to pick a random value and hope - ftok() can help with this but it still doesn't guarantee to avoid colision)
	//	Permission flags
	//		Operation permissions 	Octal value
	//		Read by user 			00400
	//		Write by user 			00200
	//		Read by group 			00040
	//		Write by group 			00020
	//		Read by others 			00004
	//		Write by others			00002
	//		Examples:
	//			0666 Everyone can read and write

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

	//Assign the shared_memory segment
	shared_memory1 = (struct shared_memory1_struct *)shared_memory1_pointer;

	//Init data in Shared Memory

 	if(hfnd && clihiddenalarm > 0 && clihiddenalarm < 2400){  //hiddenalarm found, set value
		snprintf(shared_memory1->hiddenalarm,sizeof(shared_memory1->hiddenalarm),"%04d",clihiddenalarm);
	} else {
		strncpy(shared_memory1->hiddenalarm,"-1",sizeof(shared_memory1->hiddenalarm) );
	}
 	if (vfnd && clvolume > -6000 && clvolume <= 0) {  //set volume to reasonable value  (-6000 - 0)
		shared_memory1->volume=clvolume;
	}else{
		shared_memory1->volume=-2000;
	}	
	if (ffnd) {  //set hour format
		if(clflag24H == 24){
			shared_memory1->b24Hformat=true;
		} else {
			shared_memory1->b24Hformat=false;
		}
	} else {
		shared_memory1->b24Hformat=false;
	}	
 	if (strlen(cltune) == 0) {
		strncpy(cltune,"moon.wav",sizeof(cltune));
	}
	strncpy(shared_memory1->tune,cltune,sizeof(shared_memory1->tune));

	shared_memory1->bExit = false;
	shared_memory1->iMode = 1;
	//this is the main loop.  Program loops based on iMode value, which specifies state
	//1 = alarm disarmed. wait for alarm to be turned on
	//2 = alarm armed, wait for either alarm time to be reached
	//3 = in alarm for thumbwheel switch alarm, continue till alarm disarmed by switch
	//4 = wait till alarm time has passed
	//5 = illegal alarm time set - show on display
	//6 = same as 3 for hidden alarm
	//when in alarm, sound is played over and over.
	
	while(!shared_memory1->bExit ){ //forever, till someone stops us
		AmbientLight = (ReadLightIntensity()+ReadLightIntensity()+ReadLightIntensity())/3;
		//printf("%ld\n",AmbientLight);
		if( AmbientLight> 30000){
			iBrightness = 1;
		} else if(AmbientLight> 15000){
			iBrightness = 4;
			} else if(AmbientLight> 10000){
			iBrightness = 6;
		} else if(AmbientLight> 3000){
			iBrightness = 10;
		} else iBrightness = 15;
		
		shared_memory1->bAlarmStatus = !io.digitalRead(14);
		switch(shared_memory1->iMode){  //state machine to implement states 1-6
			
			case 1:  //display time; alarm not armed
				io.digitalWrite(15,true);  //armed light off
				shared_memory1->thumb = ReadThumbwheel(io,shared_memory1->thumbdigits);
				if(shared_memory1->thumb >= 2400)shared_memory1->iMode = 5;
				ShowTime(shared_memory1->digits,iBrightness,0);
				if(!io.digitalRead(14)) shared_memory1->iMode = 2;
			break;
			
			case 2:   //display time; alarm armed
				shared_memory1->thumb = ReadThumbwheel(io,shared_memory1->thumbdigits);
				if(shared_memory1->thumb >= 2400)shared_memory1->iMode = 5;
				if(io.digitalRead(14)) shared_memory1->iMode = 1;
				ShowTime(shared_memory1->digits,iBrightness,0);
				io.digitalWrite(15,false);  //active low	
				if(strncmp(shared_memory1->digits,shared_memory1->thumbdigits,4)==0) shared_memory1->iMode = 3;
				if(strlen(shared_memory1->hiddenalarm)> 0 && strncmp(shared_memory1->digits,shared_memory1->hiddenalarm,4)==0){
					 shared_memory1->iMode = 6;  //constant hidden alarm hit
				}
			break;
			
			case 3:	 //sound alarm 
				ShowTime(shared_memory1->digits,iBrightness,1);
				if(ChildPID==0){
					ChildPID=fork();
					if(ChildPID==0)
					{
						//I am the child - play and exit
						snprintf(command,sizeof(command),"%d",shared_memory1->volume);
						strncpy(tunefullpath,sounddir,sizeof(tunefullpath));
						strncat(tunefullpath,shared_memory1->tune,sizeof(tunefullpath));
						execlp("/usr/bin/omxplayer", " ","--vol",command,tunefullpath, NULL);		//Execute file: file, arguments (1 or more strings followed by NULL)
						_exit(0);
					}
					else
					{
						//I am the parent, keep looping
					} 
				}	
				if(ChildPID != 0){
				result = waitpid(ChildPID, &status, WNOHANG);
				if (result == 0) {
					//child still alive
				} else if (result == -1) {
					// error
					printf("Child process running omxplayer error from waitpid\n");
					ChildPID = 0;
				} else {
					//child exited
					ChildPID = 0;
					printf("Child process running omxplayer exited\n");
				}
				if(io.digitalRead(14)) shared_memory1->iMode = 4;  //alarm acknowledged
			}	
			break;
			
			case 4:  //ack alarm, wait for time to pass
				if(ChildPID != 0){
					system("killall omxplayer.bin");
					ChildPID = 0;
				}
				ShowTime(shared_memory1->digits,iBrightness,0);
				if(!io.digitalRead(14)){
					bToggle = !bToggle;
					io.digitalWrite(15,bToggle);  //turn off armed LED
				} else { 
					io.digitalWrite(15,true);
					bToggle = false;
				}
				if(strncmp(shared_memory1->digits,shared_memory1->thumbdigits,4)!=0 && strncmp(shared_memory1->digits,shared_memory1->hiddenalarm,4)!=0 ) shared_memory1->iMode = 1;
			break;
					
			case 5:  //bad alarm value set
				shared_memory1->thumb = ReadThumbwheel(io,shared_memory1->thumbdigits);
				if (shared_memory1->thumb >= 2400){
					io.digitalWrite(15,true);  //armed light off
					strncpy(tmp,"bad ",sizeof(tmp));
					ShowMsg(tmp);
				} else {
					shared_memory1->iMode = 1;
				}
			break;	
			
			case 6:  //hidden alarm time reached.
				ShowTime(shared_memory1->digits,iBrightness,1);
				if(ChildPID==0){
					ChildPID=fork();
					if(ChildPID==0)
					{
						//I am the child - play and exit
						snprintf(command,sizeof(command),"%d",shared_memory1->volume);
						strncpy(tunefullpath,sounddir,sizeof(tunefullpath));
						strncat(tunefullpath,shared_memory1->tune,sizeof(tunefullpath));
						execlp("/usr/bin/omxplayer", " ","--vol",command,tunefullpath, NULL);		//Execute file: file, arguments (1 or more strings followed by NULL)
						_exit(0);
					}
					else
					{
						//I am the parent, keep looping
					} 
				}	
				if(ChildPID != 0){
				result = waitpid(ChildPID, &status, WNOHANG);
				if (result == 0) {
					//child still alive
				} else if (result == -1) {
					// error
					printf("Child process running omxplayer error from waitpid\n");
					ChildPID = 0;
				} else {
					//child exited
					ChildPID = 0;
					printf("Child process running omxplayer exited\n");
				}
				if(io.digitalRead(14)) shared_memory1->iMode = 4;  //alarm acknowledged
			}	
			break; 
			
			default:
				printf("default iMode\n");
				shared_memory1->iMode = 1;
			break;	
		}  //end of switch
		usleep(100000l);
	}  //end of while !bExit 
	printf("Exiting AlarmClock program\n");
	strncpy(tmp,"Off ",sizeof(tmp));
	ShowMsg(tmp);
	usleep(100000l);
	//--------------------------------
	//----- DETACH SHARED MEMORY -----
	//--------------------------------
	//Detach and delete
	if (shmdt(shared_memory1_pointer) == -1)
	{
		fprintf(stderr, "shmdt failed\n");
		//exit(EXIT_FAILURE);
	}
	if (shmctl(shared_memory1_id, IPC_RMID, 0) == -1)
	{
		fprintf(stderr, "shmctl(IPC_RMID) failed\n");
		//exit(EXIT_FAILURE);
	}
}//end of main

int ShowTime(char s[],int iBrightness, int iBlinkSpd)
{
	//get current time from linix and convert to text
	time( &rawtime );
	char u[10];
	char t[10];
    info = localtime( &rawtime );
	strftime(s,9,"%H%M",info);  //24h format
	strftime(t,9,"%I%M",info);  //12h format
	strftime(u,9,"%S",info);  //seconds

	//display time on LED display
	HT16K33 led_backpack1 = HT16K33_INIT(1, HT16K33_ADDR_01);
	// initialize the backpack
	rc = HT16K33_OPEN(&led_backpack1);
	if(rc != 0) {
		fprintf(stderr, "Error initializing HT16K33 led backpack (%s). Check your i2c bus (es. i2cdetect)\n", strerror(led_backpack1.lasterr));
	// you don't need to HT16K33_CLOSE() if HT16K33_OPEN() failed, but it's safe doing it.
		HT16K33_CLOSE(&led_backpack1);
	return 1;
	}
	// power on the ht16k33
	rc = HT16K33_ON(&led_backpack1);
	if(rc != 0) {
		fprintf(stderr, "Error putting the HT16K33 led backpack ON (%s). Check your i2c bus (es. i2cdetect)\n", strerror(led_backpack1.lasterr));
		// you don't need to HT16K33_OFF() if HT16K33_ON() failed, but it's safe doing it.
		HT16K33_OFF(&led_backpack1);
		HT16K33_CLOSE(&led_backpack1);
		return 1;
	}
	// set iBrightness
	HT16K33_BRIGHTNESS(&led_backpack1, iBrightness);
	if (iBlinkSpd == 0){
	// make it not blinking
		HT16K33_BLINK(&led_backpack1, HT16K33_BLINK_OFF);
	} else if(iBlinkSpd == 1) {
		HT16K33_BLINK(&led_backpack1, HT16K33_BLINK_SLOW);
	} else {
		HT16K33_BLINK(&led_backpack1, HT16K33_BLINK_FAST);
	}
	// power on the display
	HT16K33_DISPLAY(&led_backpack1, HT16K33_DISPLAY_ON);
	//show the digits
	if(shared_memory1->b24Hformat){
		HT16K33_UPDATE_DIGIT(&led_backpack1, 0, s[0] , 0);
		HT16K33_UPDATE_DIGIT(&led_backpack1, 1, s[1] , 0);
	} else {
		if (t[0] == '0' ) {  //suppress leading 0
			HT16K33_UPDATE_DIGIT(&led_backpack1, 0, ' ' , 0);
		} else {
			HT16K33_UPDATE_DIGIT(&led_backpack1, 0, t[0] , 0);
		}
		HT16K33_UPDATE_DIGIT(&led_backpack1, 1, t[1] , 0);
	}
	if(u[1] & 0x01 != 0) {  //make colon flash
		HT16K33_UPDATE_DIGIT(&led_backpack1, 2, HT16K33_COLON_OFF, 0);  //HT16K33_COLON_OFF
	} else {
		HT16K33_UPDATE_DIGIT(&led_backpack1, 2, 0x22, 0);
	}
	HT16K33_UPDATE_DIGIT(&led_backpack1, 3, s[2], 0);
	HT16K33_UPDATE_DIGIT(&led_backpack1, 4, s[3], 0);
	HT16K33_COMMIT(&led_backpack1);
	// close things (the display remains in the conditions left)
	HT16K33_CLOSE(&led_backpack1);
	return 0;
}

int ReadThumbwheel(SX1509 io, char thumbdigits[])
{
	int thumb;
	//read thumbwheel switch as integer
	thumb = io.digitalRead(0) << 3 | io.digitalRead(1) << 2 | io.digitalRead(2) << 1 | io.digitalRead(3);  
	thumb += (io.digitalRead(4) << 3 | io.digitalRead(5) << 2 | io.digitalRead(6) << 1 | io.digitalRead(7))*10;  
	thumb += (io.digitalRead(8) << 3 | io.digitalRead(9) << 2 | io.digitalRead(10) << 1 | io.digitalRead(11))*100;  
	thumb += (io.digitalRead(12) << 1 | io.digitalRead(13))*1000;
	//convert to individual digits for output
	sprintf(thumbdigits,"%04d",thumb);  
	return thumb;
}

int ShowMsg(char Msg[])
{
	//display 4 char Msg on LED display
	HT16K33 led_backpack1 = HT16K33_INIT(1, HT16K33_ADDR_01);
	// initialize the backpack
	rc = HT16K33_OPEN(&led_backpack1);
	if(rc != 0) {
		fprintf(stderr, "Error initializing HT16K33 led backpack (%s). Check your i2c bus (es. i2cdetect)\n", strerror(led_backpack1.lasterr));
	// you don't need to HT16K33_CLOSE() if HT16K33_OPEN() failed, but it's safe doing it.
		HT16K33_CLOSE(&led_backpack1);
	return 1;
	}
	// power on the ht16k33
	rc = HT16K33_ON(&led_backpack1);
	if(rc != 0) {
		fprintf(stderr, "Error putting the HT16K33 led backpack ON (%s). Check your i2c bus (es. i2cdetect)\n", strerror(led_backpack1.lasterr));
		// you don't need to HT16K33_OFF() if HT16K33_ON() failed, but it's safe doing it.
		HT16K33_OFF(&led_backpack1);
		HT16K33_CLOSE(&led_backpack1);
		return 1;
	}
	// set intensity
	HT16K33_BRIGHTNESS(&led_backpack1, 15);
	
	HT16K33_BLINK(&led_backpack1, HT16K33_BLINK_FAST);
	
	// power on the display
	HT16K33_DISPLAY(&led_backpack1, HT16K33_DISPLAY_ON);
	// show the digits
	HT16K33_UPDATE_DIGIT(&led_backpack1, 0, Msg[0] , 0);
	HT16K33_UPDATE_DIGIT(&led_backpack1, 1, Msg[1] , 0);
	HT16K33_UPDATE_DIGIT(&led_backpack1, 2, HT16K33_COLON_OFF, 0);
	HT16K33_UPDATE_DIGIT(&led_backpack1, 3, Msg[2] , 0);
	HT16K33_UPDATE_DIGIT(&led_backpack1, 4, Msg[3] , 0);
	HT16K33_COMMIT(&led_backpack1);
	// close things (the display remains in the conditions left)
	HT16K33_CLOSE(&led_backpack1);
	return 0;
}

long ReadLightIntensity()
{
	long LightCount = 0;
	pinMode(0,OUTPUT);
	digitalWrite(0,0);
	usleep(10000);
	pinMode(0,INPUT);
	while(!digitalRead(0)){
		LightCount++;
		if(LightCount > 100000)break;
	}
	pinMode(0,OUTPUT);
	digitalWrite(0,0);
	return(LightCount);
}
