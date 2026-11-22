#include "application/contact_repository.hpp"
#include "application/contact_service.hpp"

#include <gtest/gtest.h>

namespace {

class ContactRepositoryStub : public IContactRepository {
public:
    mutable std::optional<ContactFields> inserted;

    std::vector<Contact> listContacts() const override { return {}; }

    std::optional<Contact> findContactById(std::int64_t id) const override {
        ADD_FAILURE() << "Repository received invalid contact ID " << id;
        return std::nullopt;
    }

    Contact createContact(const ContactFields& fields) const override {
        inserted = fields;
        return {70, fields.name, fields.phone, fields.address};
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

}  // namespace
