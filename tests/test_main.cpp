#include <gtest/gtest.h>
#include <oatpp/core/base/Environment.hpp>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    oatpp::base::Environment::init();
    const int result = RUN_ALL_TESTS();
    oatpp::base::Environment::destroy();
    return result;
}
