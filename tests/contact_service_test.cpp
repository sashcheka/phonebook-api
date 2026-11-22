#include "application/contact_repository.hpp"
#include "application/contact_service.hpp"

#include <gtest/gtest.h>

namespace {

class UnusedContactRepository : public IContactRepository {
public:
    std::vector<Contact> listContacts() const override { return {}; }

    std::optional<Contact> findContactById(std::int64_t id) const override {
        ADD_FAILURE() << "Repository received invalid contact ID " << id;
        return std::nullopt;
    }
};

TEST(ContactServiceTest, RejectsNonPositiveIdsBeforeCallingRepository) {
    const UnusedContactRepository repository;
    const ContactService service{repository};

    EXPECT_THROW(service.findContactById(0), InvalidContactId);
    EXPECT_THROW(service.findContactById(-1), InvalidContactId);
}

}  // namespace
