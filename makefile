alarmclk: alarmclk.cpp alarmclk.h SparkFunSX1509.h 7seg_bp_ada.h sharedmem.h clparms.h
	g++ -o alarmclk alarmclk.cpp 7seg.a SparkFunSX1509.a  -std=gnu++11 -lwiringPi
testshm: testshm.cpp sharedmem.h clparms.h
	g++ -o testshm testshm.cpp -std=gnu++11
SparkFunSX1509.o: SparkFunSX1509.cpp sx1509_registers.h SparkFunSX1509.h
	g++ -c SparkFunSX1509.cpp -std=gnu++11 -lwiringPi
SparkFunSX1509.a: SparkFunSX1509.o	
	ar -rvs SparkFunSX1509.o
7seg.o: 7seg_bp_ada.c 7seg_bp_ada.h
	g++ -c 7seg_bp_ada.c -std=gnu++11
7seg.a: 7seg_bp_ada.o
	ar -rvs 7seg_bp_ada.o
