#include "application/contact_service.hpp"
#include "http/contact_controller.hpp"

#include <gtest/gtest.h>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>

#include <cstddef>
#include <vector>

namespace {

#include OATPP_CODEGEN_BEGIN(DTO)
class JsonContact : public oatpp::DTO {
    DTO_INIT(JsonContact, DTO)
    DTO_FIELD(Int64, id);
    DTO_FIELD(String, name);
    DTO_FIELD(String, phone);
    DTO_FIELD(String, address);
};
#include OATPP_CODEGEN_END(DTO)

class ContactRepositoryStub : public IContactRepository {
public:
    std::vector<Contact> contacts;

    std::vector<Contact> listContacts() const override { return contacts; }
};

class ContactControllerTest : public ::testing::Test {
protected:
    ContactRepositoryStub repository_;
    ContactService service_{repository_};
    std::shared_ptr<oatpp::parser::json::mapping::ObjectMapper> mapper_ =
        oatpp::parser::json::mapping::ObjectMapper::createShared();
    ContactController controller_{mapper_, service_};

    void expectContactsResponse() {
        const auto response = controller_.listContacts();
        ASSERT_NE(response, nullptr);
        EXPECT_EQ(200, response->getStatus().code);

        const auto body = response->getBody();
        ASSERT_NE(body, nullptr);
        ASSERT_GT(body->getKnownSize(), 0);
        const auto* data = body->getKnownData();
        ASSERT_NE(data, nullptr);
        const oatpp::String json(reinterpret_cast<const char*>(data), body->getKnownSize());
        const auto contacts = mapper_->readFromString<oatpp::Vector<oatpp::Object<JsonContact>>>(json);
        ASSERT_NE(contacts, nullptr);
        ASSERT_EQ(repository_.contacts.size(), contacts->size());

        for (std::size_t index = 0; index < contacts->size(); ++index) {
            const auto& expected = repository_.contacts[index];
            const auto& actual = contacts[index];
            ASSERT_NE(actual.get(), nullptr);
            EXPECT_EQ(expected.id, actual->id.getValue(0));
            EXPECT_EQ(expected.name, actual->name.getValue(""));
            EXPECT_EQ(expected.phone, actual->phone.getValue(""));
            EXPECT_EQ(expected.address, actual->address.getValue(""));
        }
    }
};

TEST_F(ContactControllerTest, ReturnsContactsAsJson) {
    repository_.contacts = {
        {3, "Alex \"AJ\" Quinn", "+1-202-555-0102", "103 Example Avenue"},
        {17, "Maya Carter", "+1-202-555-0116", "117 Example Avenue"},
    };

    expectContactsResponse();
}

TEST_F(ContactControllerTest, ReturnsEmptyJsonArrayWhenRepositoryIsEmpty) {
    expectContactsResponse();
}

}  // namespace
