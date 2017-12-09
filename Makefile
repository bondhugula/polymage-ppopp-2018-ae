VERSION=3.6

INCLUDES=-I/usr/include/python$(VERSION)m/ 
CPPFLAGS=-shared -lpython3 -fPIC -std=c++11 -Wall
LDFLAGS=-lboost_system 

all: sandbox/dpfusion.so

sandbox/dpfusion.so: sandbox/dpfusion/dpfusion.cpp
	g++ -D DEBUG=0 $(INCLUDES) $(CPPFLAGS) -O3 -o $@ $< $(LDFLAGS)

debug1:	sandbox/dpfusion/dpfusion.cpp
	g++ -D DEBUG=1 $(INCLUDES) $(CPPFLAGS) -O3 -o sandbox/dpfusion.so $< $(LDFLAGS)

debug2:	sandbox/dpfusion/dpfusion.cpp
	g++ -D DEBUG=2 $(INCLUDES) $(CPPFLAGS) -O3 -o sandbox/dpfusion.so $< $(LDFLAGS)
	
debug3:	sandbox/dpfusion/dpfusion.cpp
	g++ -D DEBUG=3 $(INCLUDES) $(CPPFLAGS) -O3 -o sandbox/dpfusion.so $< $(LDFLAGS)

gdbdebug: sandbox/dpfusion/dpfusion.cpp
	g++ -g -D DEBUG=3 $(INCLUDES) $(CPPFLAGS) -O0 -o $sandbox/dpfusion.so $< $(LDFLAGS)

clean:
	rm -f sandbox/dpfusion.so
