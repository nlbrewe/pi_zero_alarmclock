//command line parameters

int clflag24H = 12;  //command line format 12h/24h
int clvolume = 0;  //command line volume
char cltune[80] = "";
int clihiddenalarm=0600;  //time for hidden fixed alarm
int clExitflg = 0;
int clqflg = 0;  //simple query flag
int clprogflg = 0;
char clsndtest[10] = "stop";  //test duration(seconds)
int vfnd = 0;  //volume flag/value found
int hfnd = 0;  //hidden alarm flag/value found
int ffnd = 0;  //hour format flag/value found
int tfnd = 0;  //sound test "play/stop"
int opt;  //command line parsing

