all : test_parser proxy proxy.o parser parser.o helper.o test_parser.o cache.o

proxy : proxy.o parser.o helper.o cache.o
	g++ -pthread -g -o proxy proxy.o parser.o helper.o cache.o 

proxy.o : proxy.cpp
	g++ -pthread -g -c proxy.cpp

test_parser:test_parser.o parser.o
	g++  -g -o test_parser test_parser.o parser.o 

parser:parser.o test_parser.o 
	g++ -g -o parser  parser.o test_parser.o

parser.o:parser.cpp
	g++ -std=c++11 -g -c parser.cpp

test_parser.o : test_parser.cpp 
	g++ -g -c test_parser.cpp

helper.o : helper.cpp
	g++ -g -c helper.cpp

cache.o : cache.cpp
	g++ -g -c cache.cpp

.PHONY:
	clean

clean:
	rm -f *.o *~ 
