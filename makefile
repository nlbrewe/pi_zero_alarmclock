OBJ = alarmclk.o
DEPS = alarmclk.h sharedmem.h SparkFunSX1509.h 7seg.h SX1509_registers.h
LIBS = -lwiringPi
CFLAGS = (-std=gnu++11)
CC = g++
EXTENSION = .cpp
%.o: %$(EXTENSION) $(DEPS)
#	$(CC) -c -o $@ $< $(CFLAGS)
	$(CC) -c -o $@ $< $(CFLAGS)
alarmclk: $(OBJ)
	g++ -o alarmclk alarmclk.cpp 7seg.a SparkFunSX1509.a  -std=gnu++11 -lwiringPi
.PHONY: clean
clean:  
	rm -f *.o *~ core *~
