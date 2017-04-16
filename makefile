alarmclk: alarmclk.cpp
	g++ -o alarmclk alarmclk.cpp 7seg.a SparkFunSX1509.a  -std=gnu++11 -lwiringPi
testshm: testshm.cpp
	g++ -o testshm testshm.cpp -std=gnu++11

