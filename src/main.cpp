#include "oatpp/core/base/Environment.hpp"

#include <iostream>

int main() {
    oatpp::base::Environment::init();
    std::cout << "Oat++ initialized" << "\n";
    oatpp::base::Environment::destroy();
    return 0;
}
