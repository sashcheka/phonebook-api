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
    bool reject_contact_data = false;
    bool fail_write = false;
    mutable std::optional<ContactFields> create_input;

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

    Contact createContact(const ContactFields& fields) const override {
        create_input = fields;
        checkWrite();
        return {70, fields.name, fields.phone, fields.address};
    }

    std::optional<Contact> replaceContact(std::int64_t id, const ContactFields& fields) const override {
        checkWrite();
        if (!findContactById(id)) {
            return std::nullopt;
        }
        return Contact{id, fields.name, fields.phone, fields.address};
    }

    std::optional<Contact> updateContactFields(std::int64_t id, const ContactPatch& patch) const override {
        checkWrite();
        auto contact = findContactById(id);
        if (!contact) {
            return std::nullopt;
        }
        contact->name = patch.name.value_or(contact->name);
        contact->phone = patch.phone.value_or(contact->phone);
        contact->address = patch.address.value_or(contact->address);
        return contact;
    }

    bool deleteContact(std::int64_t id) const override {
        checkWrite();
        return findContactById(id).has_value();
    }

private:
    void checkWrite() const {
        if (reject_contact_data) {
            throw InvalidContactData("The name field must not be empty.");
        }
        if (fail_write) {
            throw std::runtime_error("Internal SQLite error");
        }
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

    void expectErrorResponse(const std::shared_ptr<ContactController::OutgoingResponse>& response,
                             int status, const char* code, const char* message) {
        ASSERT_NE(response, nullptr);
        EXPECT_EQ(status, response->getStatus().code);
        const auto json = responseJson(response);
        ASSERT_NE(json, nullptr);
        const auto error = mapper_->readFromString<oatpp::Object<ErrorDto>>(json);
        ASSERT_NE(error.get(), nullptr);
        EXPECT_EQ(code, error->code.getValue(""));
        EXPECT_EQ(message, error->message.getValue(""));
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

TEST_F(ContactControllerTest, ReturnsCreatedContactAndLocation) {
    const auto response = controller_.createContact(
        R"({"name":"Alex Quinn","phone":"00123","address":"103 Example Avenue"})");
    ASSERT_NE(response, nullptr);
    EXPECT_EQ(201, response->getStatus().code);
    EXPECT_EQ("/contacts/70", response->getHeader("Location").getValue(""));
    const auto json = responseJson(response);
    ASSERT_NE(json, nullptr);
    const auto contact = mapper_->readFromString<oatpp::Object<JsonContact>>(json);
    ASSERT_NE(contact.get(), nullptr);
    EXPECT_EQ(70, contact->id.getValue(0));
    EXPECT_EQ("Alex Quinn", contact->name.getValue(""));
    EXPECT_EQ("00123", contact->phone.getValue(""));
    EXPECT_EQ("103 Example Avenue", contact->address.getValue(""));
}

TEST_F(ContactControllerTest, RejectsInvalidJsonBeforeCallingService) {
    for (const char* body : {
             "", "{", "null", "[]", R"({"name":12})", R"({"name":null})",
             R"({"name":"Alex","phone":"123","address":"Park",})",
             R"({"name":"Alex" "phone":"123","address":"Park"})",
             R"({"name":"Alex","phone":"123","address":"Park"}{"name":"Maya"})"
         }) {
        SCOPED_TRACE(body);
        const auto response = controller_.createContact(body);
        ASSERT_NE(response, nullptr);
        EXPECT_EQ(400, response->getStatus().code);
        const auto json = responseJson(response);
        ASSERT_NE(json, nullptr);
        const auto error = mapper_->readFromString<oatpp::Object<ErrorDto>>(json);
        ASSERT_NE(error.get(), nullptr);
        EXPECT_EQ("invalid_json", error->code.getValue(""));
        EXPECT_FALSE(service_.create_input.has_value());
    }
}

TEST_F(ContactControllerTest, ReturnsBadRequestWhenServiceRejectsContactData) {
    service_.reject_contact_data = true;

    const auto response = controller_.createContact(
        R"({"name":"   ","phone":"00123","address":"103 Example Avenue"})");
    ASSERT_NE(response, nullptr);
    EXPECT_EQ(400, response->getStatus().code);
    const auto json = responseJson(response);
    ASSERT_NE(json, nullptr);
    const auto error = mapper_->readFromString<oatpp::Object<ErrorDto>>(json);
    ASSERT_NE(error.get(), nullptr);
    EXPECT_EQ("invalid_contact_data", error->code.getValue(""));
    EXPECT_EQ("The name field must not be empty.", error->message.getValue(""));
}

TEST_F(ContactControllerTest, ReturnsGenericErrorWhenCreationFails) {
    service_.fail_write = true;

    const auto response = controller_.createContact(
        R"({"name":"Alex Quinn","phone":"00123","address":"103 Example Avenue"})");
    ASSERT_NE(response, nullptr);
    EXPECT_EQ(500, response->getStatus().code);
    const auto json = responseJson(response);
    ASSERT_NE(json, nullptr);
    const auto error = mapper_->readFromString<oatpp::Object<ErrorDto>>(json);
    ASSERT_NE(error.get(), nullptr);
    EXPECT_EQ("contact_create_failed", error->code.getValue(""));
    EXPECT_EQ("Could not create contact.", error->message.getValue(""));
}

TEST_F(ContactControllerTest, ReturnsUpdatedContactWithSameId) {
    service_.contacts = {{17, "Alex Quinn", "00123", "103 Example Avenue"}};

    const auto response = controller_.updateContact(
        "17", R"({"name":"Maya Carter","phone":"00456","address":"117 Example Avenue"})");
    ASSERT_NE(response, nullptr);
    EXPECT_EQ(200, response->getStatus().code);
    const auto json = responseJson(response);
    ASSERT_NE(json, nullptr);
    const auto contact = mapper_->readFromString<oatpp::Object<JsonContact>>(json);
    ASSERT_NE(contact.get(), nullptr);
    EXPECT_EQ(17, contact->id.getValue(0));
    EXPECT_EQ("Maya Carter", contact->name.getValue(""));
    EXPECT_EQ("00456", contact->phone.getValue(""));
    EXPECT_EQ("117 Example Avenue", contact->address.getValue(""));
}

TEST_F(ContactControllerTest, ReturnsNotFoundWhenUpdatingMissingContact) {
    const auto response = controller_.updateContact(
        "999", R"({"name":"Alex Quinn","phone":"00123","address":"103 Example Avenue"})");
    expectErrorResponse(response, 404, "contact_not_found", "Contact not found.");
}

TEST_F(ContactControllerTest, RejectsInvalidUpdateRequests) {
    const auto* body = R"({"name":"Alex Quinn","phone":"00123","address":"103 Example Avenue"})";
    expectErrorResponse(controller_.updateContact("17x", body), 400, "invalid_contact_id",
                        "Contact ID must be a positive integer.");
    expectErrorResponse(controller_.updateContact("17", "{"), 400, "invalid_json",
                        "Expected a JSON object with string contact fields.");

    service_.reject_id = true;
    expectErrorResponse(controller_.updateContact("-1", body), 400, "invalid_contact_id",
                        "Contact ID was rejected.");
    service_.reject_id = false;
    service_.reject_contact_data = true;
    expectErrorResponse(controller_.updateContact("17", body), 400, "invalid_contact_data",
                        "The name field must not be empty.");
}

TEST_F(ContactControllerTest, ReturnsGenericErrorWhenUpdateFails) {
    service_.fail_write = true;
    const auto response = controller_.updateContact(
        "17", R"({"name":"Alex Quinn","phone":"00123","address":"103 Example Avenue"})");
    expectErrorResponse(response, 500, "contact_update_failed", "Could not update contact.");
}

TEST_F(ContactControllerTest, PatchesOnlySuppliedFields) {
    service_.contacts = {{17, "Alex Quinn", "00123", "103 Example Avenue"}};

    const auto response = controller_.patchContact("17", R"({"phone":"00456"})");
    ASSERT_NE(response, nullptr);
    EXPECT_EQ(200, response->getStatus().code);
    const auto json = responseJson(response);
    ASSERT_NE(json, nullptr);
    const auto contact = mapper_->readFromString<oatpp::Object<JsonContact>>(json);
    ASSERT_NE(contact.get(), nullptr);
    EXPECT_EQ(17, contact->id.getValue(0));
    EXPECT_EQ("Alex Quinn", contact->name.getValue(""));
    EXPECT_EQ("00456", contact->phone.getValue(""));
    EXPECT_EQ("103 Example Avenue", contact->address.getValue(""));
}

TEST_F(ContactControllerTest, RejectsInvalidPatchRequests) {
    for (const auto* body : {"[]", "{", R"({"phone":null})", R"({"phone":123})", R"({"id":18})"}) {
        SCOPED_TRACE(body);
        expectErrorResponse(controller_.patchContact("17", body), 400, "invalid_json",
                            "Expected a JSON object with optional string fields: name, phone, address.");
    }
    expectErrorResponse(controller_.patchContact("17x", "{}"), 400, "invalid_contact_id",
                        "Contact ID must be a positive integer.");
}

TEST_F(ContactControllerTest, ReturnsNotFoundWhenPatchingMissingContact) {
    expectErrorResponse(controller_.patchContact("999", R"({"phone":"00456"})"),
                        404, "contact_not_found", "Contact not found.");
}

TEST_F(ContactControllerTest, ReturnsGenericErrorWhenPatchFails) {
    service_.fail_write = true;
    expectErrorResponse(controller_.patchContact("17", R"({"phone":"00456"})"),
                        500, "contact_update_failed", "Could not update contact.");
}

}  // namespace
