#pragma once

#include "application/contact_service_interface.hpp"

#include <cstdint>
#include <optional>
#include <vector>

class IContactRepository;

class ContactService : public IContactService {
public:
    explicit ContactService(const IContactRepository& repository);

    std::vector<Contact> listContacts() const override;
    std::optional<Contact> findContactById(std::int64_t id) const override;

private:
    const IContactRepository& repository_;
};
