
#include <iostream>

int main(int argc, char **argv) {

    std::cout << "Hello from container!" << std::endl;
    for (int i = 1; i < argc; i++) {
        std::cout << "arg" << i << ": [" << argv[i] << "]" << std::endl;
    }

    return 0;
}
