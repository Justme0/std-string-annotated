#include <iostream>
#include "std-string.h"

int main() {
	string s("ddd");
	s.length();
	std::cout << s << std::endl;
	std::cout << s.size() << std::endl;
	std::cout << s.npos << std::endl;
	//std::cin >> s >> std::endl;

	return 0;
}
