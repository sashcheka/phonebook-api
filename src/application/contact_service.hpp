#pragma once

#include "application/contact_repository.hpp"
#include "application/contact_service_interface.hpp"

#include <vector>

class ContactService : public IContactService {
public:
    explicit ContactService(const IContactRepository& repository) : repository_(repository) {}

    std::vector<Contact> listContacts() const override {
        return repository_.listContacts();
    }

private:
    const IContactRepository& repository_;
};
