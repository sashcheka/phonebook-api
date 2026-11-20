#pragma once

#include "application/contact_repository.hpp"

#include <vector>

class ContactService {
public:
    explicit ContactService(const IContactRepository& repository) : repository_(repository) {}

    std::vector<Contact> listContacts() const {
        return repository_.listContacts();
    }

private:
    const IContactRepository& repository_;
};
