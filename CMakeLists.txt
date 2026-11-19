cmake_minimum_required(VERSION 3.24)

project(phonebook_api LANGUAGES CXX)
find_package(oatpp REQUIRED)

add_executable(phonebook_api src/main.cpp)
target_link_libraries(phonebook_api PRIVATE oatpp::oatpp)
