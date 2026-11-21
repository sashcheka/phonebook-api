#include "application/contact_service_interface.hpp"
#include "http/contacts/contact_controller.hpp"

#include <gtest/gtest.h>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
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

class ContactServiceStub : public IContactService {
public:
    std::vector<Contact> contacts;

    std::vector<Contact> listContacts() const override { return contacts; }

    std::optional<Contact> findContactById(std::int64_t id) const override {
        for (const auto& contact : contacts) {
            if (contact.id == id) {
                return contact;
            }
        }
        return std::nullopt;
    }
};

class ContactControllerTest : public ::testing::Test {
protected:
    ContactServiceStub service_;
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
        ASSERT_EQ(service_.contacts.size(), contacts->size());

        for (std::size_t index = 0; index < contacts->size(); ++index) {
            const auto& expected = service_.contacts[index];
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
    service_.contacts = {
        {3, "Alex \"AJ\" Quinn", "+1-202-555-0102", "103 Example Avenue"},
        {17, "Maya Carter", "+1-202-555-0116", "117 Example Avenue"},
    };

    expectContactsResponse();
}

TEST_F(ContactControllerTest, ReturnsEmptyJsonArrayWhenServiceReturnsNoContacts) {
    expectContactsResponse();
}

}  // namespace
