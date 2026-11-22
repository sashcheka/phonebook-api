#include "application/contact_repository.hpp"
#include "application/contact_service.hpp"

#include <gtest/gtest.h>

namespace {

class ContactRepositoryStub : public IContactRepository {
public:
    mutable std::optional<ContactFields> inserted;
    mutable std::optional<Contact> updated;
    mutable std::optional<ContactPatch> patched;
    mutable std::optional<std::int64_t> looked_up_id;
    mutable std::optional<std::int64_t> deleted_id;
    std::optional<Contact> existing;

    std::vector<Contact> listContacts() const override { return {}; }

    std::optional<Contact> findContactById(std::int64_t id) const override {
        EXPECT_GT(id, 0);
        looked_up_id = id;
        return existing;
    }

    Contact createContact(const ContactFields& fields) const override {
        inserted = fields;
        return {70, fields.name, fields.phone, fields.address};
    }

    std::optional<Contact> replaceContact(std::int64_t id, const ContactFields& fields) const override {
        updated = Contact{id, fields.name, fields.phone, fields.address};
        return updated;
    }

    std::optional<Contact> updateContactFields(std::int64_t, const ContactPatch& patch) const override {
        patched = patch;
        return existing;
    }

    bool deleteContact(std::int64_t id) const override {
        deleted_id = id;
        return existing && existing->id == id;
    }
};

TEST(ContactServiceTest, RejectsNonPositiveIdsBeforeCallingRepository) {
    const ContactRepositoryStub repository;
    const ContactService service{repository};

    EXPECT_THROW(service.findContactById(0), InvalidContactId);
    EXPECT_THROW(service.findContactById(-1), InvalidContactId);
    EXPECT_FALSE(repository.looked_up_id.has_value());
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

TEST(ContactServiceTest, PassesOnlyProvidedPatchFieldsToRepository) {
    const ContactRepositoryStub repository;
    const ContactService service{repository};
    ContactPatch patch;
    patch.phone = "00456";

    EXPECT_FALSE(service.updateContactFields(17, patch).has_value());
    ASSERT_TRUE(repository.patched.has_value());
    EXPECT_EQ(patch.phone, repository.patched->phone);
    EXPECT_FALSE(repository.patched->name.has_value());
    EXPECT_FALSE(repository.patched->address.has_value());
    EXPECT_FALSE(repository.looked_up_id.has_value());
}

TEST(ContactServiceTest, RejectsInvalidPatchBeforeCallingRepository) {
    const ContactRepositoryStub repository;
    const ContactService service{repository};
    ContactPatch patch;
    patch.phone = " ";

    EXPECT_THROW(service.updateContactFields(0, {}), InvalidContactId);
    EXPECT_THROW(service.updateContactFields(17, patch), InvalidContactData);
    EXPECT_FALSE(repository.patched.has_value());
    EXPECT_FALSE(repository.looked_up_id.has_value());
}

TEST(ContactServiceTest, EmptyPatchReadsContactWithoutUpdatingIt) {
    ContactRepositoryStub repository;
    repository.existing = Contact{17, "Alex Quinn", "00123", "103 Example Avenue"};
    const ContactService service{repository};

    const auto result = service.updateContactFields(17, {});
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(17, result->id);
    EXPECT_EQ(17, repository.looked_up_id);
    EXPECT_FALSE(repository.patched.has_value());
}

TEST(ContactServiceTest, RejectsInvalidDeleteIdBeforeCallingRepository) {
    const ContactRepositoryStub repository;
    const ContactService service{repository};

    EXPECT_THROW(service.deleteContact(0), InvalidContactId);
    EXPECT_THROW(service.deleteContact(-1), InvalidContactId);
    EXPECT_FALSE(repository.deleted_id.has_value());
}

TEST(ContactServiceTest, ReturnsDeletionResultFromRepository) {
    ContactRepositoryStub repository;
    repository.existing = Contact{17, "Alex Quinn", "00123", "103 Example Avenue"};
    const ContactService service{repository};

    EXPECT_TRUE(service.deleteContact(17));
    EXPECT_EQ(17, repository.deleted_id);
    EXPECT_FALSE(service.deleteContact(999));
    EXPECT_EQ(999, repository.deleted_id);
}

}  // namespace
