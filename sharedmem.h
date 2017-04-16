//----- SHARED MEMORY -----
struct shared_memory1_struct {
	char digits[10];  //time converted to digits
	char thumbdigits[10];  //thumbwheel converted to digits
	char hiddenalarm[10];  //hidden alarm time
	int thumb;  //integer thumbwheel switch value
	int iMode;
	int volume;  //sound volume millibells -6000 to 0
	unsigned char b24Hformat;
	unsigned char bExit;
	unsigned char bAlarmStatus;  //state of alarm switch
	char tune[80]; 
	};
