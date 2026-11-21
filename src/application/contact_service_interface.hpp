#pragma once

#include "domain/contact.hpp"

#include <cstdint>
#include <optional>
#include <vector>

class IContactService {
public:
    virtual ~IContactService() = default;
    virtual std::vector<Contact> listContacts() const = 0;
    virtual std::optional<Contact> findContactById(std::int64_t id) const = 0;
};
