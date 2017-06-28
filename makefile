CC = g++ -std=c++11

all: cx.cpp string_manip.o stdutil.o
	$(CC) -o cx cx.cpp string_manip.o stdutil.o
	
string_manip.o: string_manip.cpp string_manip.hpp
	$(CC) -c string_manip.cpp
	
stdutil.o: stdutil.cpp stdutil.hpp
	$(CC) -c stdutil.cpp