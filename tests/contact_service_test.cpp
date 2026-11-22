#include "application/contact_repository.hpp"
#include "application/contact_service.hpp"

#include <gtest/gtest.h>

namespace {

class ContactRepositoryStub : public IContactRepository {
public:
    mutable std::optional<NewContact> inserted;

    std::vector<Contact> listContacts() const override { return {}; }

    std::optional<Contact> findContactById(std::int64_t id) const override {
        ADD_FAILURE() << "Repository received invalid contact ID " << id;
        return std::nullopt;
    }

    Contact createContact(const NewContact& contact) const override {
        inserted = contact;
        return {70, contact.name, contact.phone, contact.address};
    }
};

TEST(ContactServiceTest, RejectsNonPositiveIdsBeforeCallingRepository) {
    const ContactRepositoryStub repository;
    const ContactService service{repository};

    EXPECT_THROW(service.findContactById(0), InvalidContactId);
    EXPECT_THROW(service.findContactById(-1), InvalidContactId);
}

}  // namespace
