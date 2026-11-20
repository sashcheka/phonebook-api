cmake_minimum_required(VERSION 3.24)

project(phonebook_api LANGUAGES CXX)
find_package(oatpp REQUIRED)
find_package(oatpp-sqlite CONFIG QUIET)

if(NOT TARGET oatpp::oatpp-sqlite)
    include(FetchContent)
    # Use the Oat++ target already found above. The module's default lookup requests 1.3.0.
    set(OATPP_MODULES_LOCATION CUSTOM CACHE STRING "Oat++ location for oatpp-sqlite")
    set(OATPP_BUILD_TESTS OFF)
    set(OATPP_INSTALL OFF)
    FetchContent_Declare(oatpp_sqlite
        URL https://codeload.github.com/oatpp/oatpp-sqlite/tar.gz/95488343163a9111f03709f68e4944aa46217962
        URL_HASH SHA256=013b374d774d41ec1de34c6dbae7f401c98f6ba28a24f56cbba14c4ea8d90088
        DOWNLOAD_NAME oatpp-sqlite-1.3.0.tar.gz
    )
    FetchContent_MakeAvailable(oatpp_sqlite)
    add_library(oatpp::oatpp-sqlite ALIAS oatpp-sqlite)
endif()

configure_file(migrations/init_db.sql migrations/init_db.sql COPYONLY)

add_executable(phonebook_api
    src/main.cpp
    src/http/contact_controller.cpp
    src/http/health_controller.cpp
    src/storage/sqlite/sqlite_contact_repository.cpp
)
target_include_directories(phonebook_api PRIVATE src)
target_link_libraries(phonebook_api PRIVATE oatpp::oatpp oatpp::oatpp-sqlite)
target_compile_definitions(phonebook_api PRIVATE
    PHONEBOOK_MIGRATIONS_DIR="${CMAKE_CURRENT_BINARY_DIR}/migrations"
)

include(CTest)

if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    add_executable(health_test tests/test_main.cpp tests/health_controller_test.cpp src/http/health_controller.cpp)
    target_include_directories(health_test PRIVATE src)
    target_link_libraries(health_test PRIVATE oatpp::oatpp GTest::gtest)

    add_executable(contact_test tests/test_main.cpp tests/contact_controller_test.cpp src/http/contact_controller.cpp)
    target_include_directories(contact_test PRIVATE src)
    target_link_libraries(contact_test PRIVATE oatpp::oatpp GTest::gtest)

    include(GoogleTest)
    gtest_discover_tests(health_test)
    gtest_discover_tests(contact_test)
endif()
