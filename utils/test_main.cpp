#include "Timestamp.h"
#include <iostream>

int main() {
    Timestamp ts = Timestamp::now();
    std::cout << "Now: " << ts.toString() << std::endl;
    return 0;
}
