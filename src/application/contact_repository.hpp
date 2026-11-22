#pragma once

#include "domain/contact.hpp"

#include <cstdint>
#include <optional>
#include <vector>

class IContactRepository {
public:
    virtual ~IContactRepository() = default;
    virtual std::vector<Contact> listContacts() const = 0;
    virtual std::optional<Contact> findContactById(std::int64_t id) const = 0;
    virtual Contact createContact(const ContactFields& fields) const = 0;
    virtual std::optional<Contact> replaceContact(std::int64_t id, const ContactFields& fields) const = 0;
    virtual std::optional<Contact> updateContactFields(std::int64_t id, const ContactPatch& patch) const = 0;
};
