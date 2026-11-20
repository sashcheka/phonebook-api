#pragma once

#include "domain/contact.hpp"

#include <vector>

class IContactRepository {
public:
    virtual ~IContactRepository() = default;
    virtual std::vector<Contact> listContacts() const = 0;
};
