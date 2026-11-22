#pragma once

#include "application/contact_repository.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

class SqliteContactRepository : public IContactRepository {
public:
    explicit SqliteContactRepository(const char* database_path);
    std::vector<Contact> listContacts() const override;
    std::optional<Contact> findContactById(std::int64_t id) const override;
    Contact createContact(const ContactFields& fields) const override;
    std::optional<Contact> updateContact(std::int64_t id, const ContactFields& fields) const override;

private:
    class Client;
    std::shared_ptr<Client> client_;
};
