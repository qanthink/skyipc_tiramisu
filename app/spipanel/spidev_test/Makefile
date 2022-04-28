#CROSS_COMPILE="arm-linux-gnueabihf-"
CROSS_COMPILE=arm-linux-

CFLAG = --std=c++11

C++ = $(CROSS_COMPILE)g++ #$(CFLAG)

all: 
	$(C++) *.cpp -o spidev_test

clean:
	    $(RM) spidev_test

