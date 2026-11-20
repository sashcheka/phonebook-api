cmake_minimum_required(VERSION 3.24)

project(phonebook_api LANGUAGES CXX)
find_package(oatpp REQUIRED)

add_executable(phonebook_api src/main.cpp src/http/health_controller.cpp)
target_link_libraries(phonebook_api PRIVATE oatpp::oatpp)

include(CTest)

if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    add_executable(health_test tests/health_controller_test.cpp src/http/health_controller.cpp)
    target_include_directories(health_test PRIVATE src)
    target_link_libraries(health_test PRIVATE oatpp::oatpp GTest::gtest)

    include(GoogleTest)
    gtest_discover_tests(health_test)
endif()
