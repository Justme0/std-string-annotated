all: test.cpp std-string.h
	g++ test.cpp

clean:
	rm -rf a.out
