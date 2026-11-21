#pragma once

#include "application/contact_service_interface.hpp"

#include <vector>

class IContactRepository;

class ContactService : public IContactService {
public:
    explicit ContactService(const IContactRepository& repository);

    std::vector<Contact> listContacts() const override;

private:
    const IContactRepository& repository_;
};
