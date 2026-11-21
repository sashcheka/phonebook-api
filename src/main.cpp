#include "server.hpp"

#include <oatpp/core/base/Environment.hpp>

#include <exception>
#include <iostream>

int main() {
    oatpp::base::Environment::init();
    std::cout << "Oat++ initialized" << "\n";

    try {
        runServer();
    } catch (const std::exception& error) {
        std::cerr << "Phonebook API failed: " << error.what() << '\n';
        oatpp::base::Environment::destroy();
        return 1;
    }

    oatpp::base::Environment::destroy();
    return 0;
}
