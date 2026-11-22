#include "application/contact_repository.hpp"
#include "application/contact_service.hpp"

#include <gtest/gtest.h>

namespace {

class ContactRepositoryStub : public IContactRepository {
public:
    mutable std::optional<ContactFields> inserted;
    mutable std::optional<Contact> updated;

    std::vector<Contact> listContacts() const override { return {}; }

    std::optional<Contact> findContactById(std::int64_t id) const override {
        ADD_FAILURE() << "Repository received invalid contact ID " << id;
        return std::nullopt;
    }

    Contact createContact(const ContactFields& fields) const override {
        inserted = fields;
        return {70, fields.name, fields.phone, fields.address};
    }

    std::optional<Contact> replaceContact(std::int64_t id, const ContactFields& fields) const override {
        updated = Contact{id, fields.name, fields.phone, fields.address};
        return updated;
    }
};

TEST(ContactServiceTest, RejectsNonPositiveIdsBeforeCallingRepository) {
    const ContactRepositoryStub repository;
    const ContactService service{repository};

    EXPECT_THROW(service.findContactById(0), InvalidContactId);
    EXPECT_THROW(service.findContactById(-1), InvalidContactId);
}

TEST(ContactServiceTest, RejectsBlankContactFieldsBeforeCallingRepository) {
    const ContactRepositoryStub repository;
    const ContactService service{repository};

    EXPECT_THROW(service.createContact({" \t", "+1-202-555-0102", "103 Example Avenue"}), InvalidContactData);
    EXPECT_THROW(service.createContact({"Alex Quinn", "", "103 Example Avenue"}), InvalidContactData);
    EXPECT_THROW(service.createContact({"Alex Quinn", "+1-202-555-0102", "\n"}), InvalidContactData);
    EXPECT_FALSE(repository.inserted.has_value());
}

TEST(ContactServiceTest, ReturnsContactCreatedByRepository) {
    const ContactRepositoryStub repository;
    const ContactService service{repository};
    const ContactFields input{"Alex Quinn", "00123", "103 Example Avenue"};

    const auto created = service.createContact(input);
    ASSERT_TRUE(repository.inserted.has_value());
    EXPECT_EQ(input.name, repository.inserted->name);
    EXPECT_EQ(input.phone, repository.inserted->phone);
    EXPECT_EQ(input.address, repository.inserted->address);
    EXPECT_EQ(70, created.id);
}

TEST(ContactServiceTest, RejectsInvalidUpdateBeforeCallingRepository) {
    const ContactRepositoryStub repository;
    const ContactService service{repository};

    EXPECT_THROW(service.replaceContact(0, {"Alex Quinn", "00123", "103 Example Avenue"}), InvalidContactId);
    EXPECT_THROW(service.replaceContact(17, {"Alex Quinn", " ", "103 Example Avenue"}), InvalidContactData);
    EXPECT_FALSE(repository.updated.has_value());
}

TEST(ContactServiceTest, UpdatesContactThroughRepository) {
    const ContactRepositoryStub repository;
    const ContactService service{repository};
    const ContactFields input{"Maya Carter", "00456", "117 Example Avenue"};

    const auto updated = service.replaceContact(17, input);
    ASSERT_TRUE(repository.updated.has_value());
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(17, repository.updated->id);
    EXPECT_EQ(input.name, repository.updated->name);
    EXPECT_EQ(input.phone, repository.updated->phone);
    EXPECT_EQ(input.address, repository.updated->address);
    EXPECT_EQ(17, updated->id);
}

}  // namespace
