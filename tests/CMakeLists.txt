find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(health_test
    test_main.cpp
    health_controller_test.cpp
    ${PROJECT_SOURCE_DIR}/src/http/health_controller.cpp
)

add_executable(contact_test
    test_main.cpp
    contact_controller_test.cpp
    contact_service_test.cpp
    ${PROJECT_SOURCE_DIR}/src/application/contact_service.cpp
    ${PROJECT_SOURCE_DIR}/src/http/contacts/contact_controller.cpp
)
target_compile_features(contact_test PRIVATE cxx_std_17)
target_link_libraries(contact_test PRIVATE nlohmann_json::nlohmann_json)

set_target_properties(health_test contact_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
)
foreach(test_target IN ITEMS health_test contact_test)
    target_include_directories(${test_target} PRIVATE "${PROJECT_SOURCE_DIR}/src")
    target_link_libraries(${test_target} PRIVATE oatpp::oatpp GTest::gtest)
    gtest_discover_tests(${test_target})
endforeach()
