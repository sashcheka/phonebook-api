#pragma once

#include "application/contact_repository.hpp"

#include <memory>
#include <vector>

class SqliteContactRepository : public IContactRepository {
public:
    explicit SqliteContactRepository(const char* database_path);
    std::vector<Contact> listContacts() const override;

private:
    class Client;
    std::shared_ptr<Client> client_;
};
