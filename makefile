EXEC=main
CC=g++ -std=c++11
CFLAGS=-c -g
LFLAGS=-g -o
#EXEC=$(notdir $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST)))))
CLASSES=$(foreach f,$(basename $(notdir $(wildcard *.cpp))),$(if $(wildcard $(f).h),$(f).o))

$(EXEC): $(EXEC).o $(CLASSES)
	$(CC) $(LFLAGS) $@ $^

$(EXEC).o: $(EXEC).cpp
	$(CC) $(CFLAGS) $^

$(CLASSES): %.o: %.cpp %.h
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o $(EXEC)
