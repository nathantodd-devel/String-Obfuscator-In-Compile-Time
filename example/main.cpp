#include <iostream>

#include "../include/obfuscator.hpp"

int main() {
    // The literal below is stored obfuscated in the binary (build with -O2 or
    // higher) and only turned back into plaintext here, at run time.
    std::cout << OBFUSCATE("snowapril") << '\n';
    return 0;
}
