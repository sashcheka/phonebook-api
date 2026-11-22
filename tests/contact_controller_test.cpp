#include "application/contact_service_interface.hpp"
#include "http/contacts/contact_controller.hpp"
#include "http/error_dto.hpp"

#include <gtest/gtest.h>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
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
    bool reject_id = false;
    bool reject_creation = false;
    bool fail_creation = false;
    mutable std::optional<NewContact> create_input;

    std::vector<Contact> listContacts() const override { return contacts; }

    std::optional<Contact> findContactById(std::int64_t id) const override {
        if (reject_id) {
            throw InvalidContactId("Contact ID was rejected.");
        }
        for (const auto& contact : contacts) {
            if (contact.id == id) {
                return contact;
            }
        }
        return std::nullopt;
    }

    Contact createContact(const NewContact& contact) const override {
        create_input = contact;
        if (reject_creation) {
            throw InvalidContactData("The name field must not be empty.");
        }
        if (fail_creation) {
            throw std::runtime_error("Internal SQLite error");
        }
        return {70, contact.name, contact.phone, contact.address};
    }
};

class ContactControllerTest : public ::testing::Test {
protected:
    ContactServiceStub service_;
    std::shared_ptr<oatpp::parser::json::mapping::ObjectMapper> mapper_ =
        oatpp::parser::json::mapping::ObjectMapper::createShared();
    ContactController controller_{mapper_, service_};

    static oatpp::String responseJson(const std::shared_ptr<ContactController::OutgoingResponse>& response) {
        const auto body = response->getBody();
        if (!body || !body->getKnownData() || body->getKnownSize() <= 0) {
            return nullptr;
        }
        return {reinterpret_cast<const char*>(body->getKnownData()), body->getKnownSize()};
    }

    void expectContactsResponse() {
        const auto response = controller_.listContacts();
        ASSERT_NE(response, nullptr);
        EXPECT_EQ(200, response->getStatus().code);

        const auto json = responseJson(response);
        ASSERT_NE(json, nullptr);
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

TEST_F(ContactControllerTest, ReturnsRequestedContactAsJson) {
    service_.contacts = {
        {3, "Alex Quinn", "+1-202-555-0102", "103 Example Avenue"},
        {17, "Maya Carter", "+1-202-555-0116", "117 Example Avenue"},
    };

    const auto response = controller_.getContactById("17");
    ASSERT_NE(response, nullptr);
    EXPECT_EQ(200, response->getStatus().code);
    const auto json = responseJson(response);
    ASSERT_NE(json, nullptr);
    const auto contact = mapper_->readFromString<oatpp::Object<JsonContact>>(json);
    ASSERT_NE(contact.get(), nullptr);
    EXPECT_EQ(17, contact->id.getValue(0));
    EXPECT_EQ("Maya Carter", contact->name.getValue(""));
    EXPECT_EQ("+1-202-555-0116", contact->phone.getValue(""));
    EXPECT_EQ("117 Example Avenue", contact->address.getValue(""));
}

TEST_F(ContactControllerTest, ReturnsNotFoundForMissingContact) {
    const auto response = controller_.getContactById("999");
    ASSERT_NE(response, nullptr);
    EXPECT_EQ(404, response->getStatus().code);
    const auto json = responseJson(response);
    ASSERT_NE(json, nullptr);
    const auto error = mapper_->readFromString<oatpp::Object<ErrorDto>>(json);
    ASSERT_NE(error.get(), nullptr);
    EXPECT_EQ("contact_not_found", error->code.getValue(""));
    EXPECT_EQ("Contact not found.", error->message.getValue(""));
}

TEST_F(ContactControllerTest, ReturnsBadRequestForUnparseableContactIds) {
    for (const char* id : {"abc", "17x", "9223372036854775808"}) {
        SCOPED_TRACE(id);
        const auto response = controller_.getContactById(id);
        ASSERT_NE(response, nullptr);
        EXPECT_EQ(400, response->getStatus().code);
        const auto json = responseJson(response);
        ASSERT_NE(json, nullptr);
        const auto error = mapper_->readFromString<oatpp::Object<ErrorDto>>(json);
        ASSERT_NE(error.get(), nullptr);
        EXPECT_EQ("invalid_contact_id", error->code.getValue(""));
    }
}

TEST_F(ContactControllerTest, ReturnsBadRequestWhenServiceRejectsContactId) {
    service_.reject_id = true;

    const auto response = controller_.getContactById("17");
    ASSERT_NE(response, nullptr);
    EXPECT_EQ(400, response->getStatus().code);
    const auto json = responseJson(response);
    ASSERT_NE(json, nullptr);
    const auto error = mapper_->readFromString<oatpp::Object<ErrorDto>>(json);
    ASSERT_NE(error.get(), nullptr);
    EXPECT_EQ("invalid_contact_id", error->code.getValue(""));
    EXPECT_EQ("Contact ID was rejected.", error->message.getValue(""));
}

}  // namespace
