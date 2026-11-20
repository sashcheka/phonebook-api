#include "http/health_controller.hpp"

#include <gtest/gtest.h>
#include <oatpp/core/base/Environment.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>

#include <string_view>

namespace {

class HealthControllerTest : public ::testing::Test {
protected:
    HealthController controller_{
        oatpp::parser::json::mapping::ObjectMapper::createShared()};
};

TEST_F(HealthControllerTest, ReturnsOkStatus) {
    const auto response = controller_.health();

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(200, response->getStatus().code);
}

TEST_F(HealthControllerTest, ReturnsOkBody) {
    const auto response = controller_.health();

    ASSERT_NE(response, nullptr);
    const auto body = response->getBody();
    ASSERT_NE(body, nullptr);
    constexpr std::string_view expected_body = "OK";
    ASSERT_EQ(static_cast<v_int64>(expected_body.size()), body->getKnownSize());

    const auto* data = body->getKnownData();
    ASSERT_NE(data, nullptr);
    const std::string_view actual_body{reinterpret_cast<const char*>(data),
                                       expected_body.size()};
    EXPECT_EQ(expected_body, actual_body);
}

}  // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    oatpp::base::Environment::init();
    const int result = RUN_ALL_TESTS();
    oatpp::base::Environment::destroy();
    return result;
}
