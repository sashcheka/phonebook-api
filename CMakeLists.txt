cmake_minimum_required(VERSION 3.24)

project(phonebook_api LANGUAGES CXX)

# Dependencies
find_package(oatpp REQUIRED)
include(FetchContent)
set(OATPP_MODULES_LOCATION CUSTOM CACHE STRING "Oat++ location for bundled modules")
set(OATPP_BUILD_TESTS OFF CACHE BOOL "Build bundled Oat++ module tests")
set(OATPP_INSTALL OFF CACHE BOOL "Install bundled Oat++ modules")

find_package(oatpp-sqlite CONFIG QUIET)
if(NOT TARGET oatpp::oatpp-sqlite)
    FetchContent_Declare(oatpp_sqlite
        URL https://codeload.github.com/oatpp/oatpp-sqlite/tar.gz/95488343163a9111f03709f68e4944aa46217962
        URL_HASH SHA256=013b374d774d41ec1de34c6dbae7f401c98f6ba28a24f56cbba14c4ea8d90088
        DOWNLOAD_NAME oatpp-sqlite-1.3.0.tar.gz
    )
    FetchContent_MakeAvailable(oatpp_sqlite)
    add_library(oatpp::oatpp-sqlite ALIAS oatpp-sqlite)
endif()

FetchContent_Declare(oatpp_swagger
    URL https://codeload.github.com/oatpp/oatpp-swagger/tar.gz/refs/tags/1.3.1
    URL_HASH SHA256=c2ec7d07f4dedbd4a2fef5ee450a839000b59be73fadbe1fbe67857752482a9f
    DOWNLOAD_NAME oatpp-swagger-1.3.1.tar.gz
)
# Swagger 1.3.1 requests a policy version that CMake 4 has removed.
set(CMAKE_POLICY_VERSION_MINIMUM 3.13)
FetchContent_MakeAvailable(oatpp_swagger)
unset(CMAKE_POLICY_VERSION_MINIMUM)

FetchContent_Declare(nlohmann_json
    URL https://codeload.github.com/nlohmann/json/tar.gz/refs/tags/v3.12.0
    URL_HASH SHA256=4b92eb0c06d10683f7447ce9406cb97cd4b453be18d7279320f7b2f025c10187
)
FetchContent_MakeAvailable(nlohmann_json)

# Server
add_executable(phonebook_api
    src/main.cpp
    src/server.cpp
    src/application/contact_service.cpp
    src/http/contacts/contact_controller.cpp
    src/http/health_controller.cpp
    src/http/swagger/swagger.cpp
    src/storage/sqlite/sqlite_contact_repository.cpp
)
target_include_directories(phonebook_api PRIVATE src)
target_compile_features(phonebook_api PRIVATE cxx_std_17)
target_link_libraries(phonebook_api PRIVATE
    oatpp::oatpp
    oatpp::oatpp-sqlite
    oatpp-swagger
    nlohmann_json::nlohmann_json
)

configure_file(migrations/init_db.sql migrations/init_db.sql COPYONLY)
target_compile_definitions(phonebook_api PRIVATE
    PHONEBOOK_MIGRATIONS_DIR="${CMAKE_CURRENT_BINARY_DIR}/migrations"
    PHONEBOOK_SWAGGER_RES_DIR="${oatpp_swagger_SOURCE_DIR}/res"
)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
