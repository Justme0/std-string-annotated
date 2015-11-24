all: test.cpp std-string.h
	g++ -g -std=c++14 -Wall -Werror test.cpp -o a.out

clean:
	rm -rf a.out
