#pragma once

#include "domain/contact.hpp"

#include <vector>

class IContactService {
public:
    virtual ~IContactService() = default;
    virtual std::vector<Contact> listContacts() const = 0;
};
