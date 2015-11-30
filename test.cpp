#include <iostream>
#include "std-string.h"

template <class T>
void print(T data) {
	std::cout << data << std::endl;
}

int main() {
	const string s("a");
	print(s.at(0));

	return 0;
}
