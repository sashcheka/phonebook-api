#pragma once

#include "domain/contact.hpp"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

class InvalidContactId : public std::invalid_argument {
public:
    using std::invalid_argument::invalid_argument;
};

class InvalidContactData : public std::invalid_argument {
public:
    using std::invalid_argument::invalid_argument;
};

class IContactService {
public:
    virtual ~IContactService() = default;
    virtual std::vector<Contact> listContacts() const = 0;
    virtual std::optional<Contact> findContactById(std::int64_t id) const = 0;
    virtual Contact createContact(const ContactFields& contact) const = 0;
};
