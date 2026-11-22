#include "application/contact_service.hpp"

#include "application/contact_repository.hpp"

ContactService::ContactService(const IContactRepository& repository)
    : repository_(repository) {}

std::vector<Contact> ContactService::listContacts() const {
    return repository_.listContacts();
}

std::optional<Contact> ContactService::findContactById(std::int64_t id) const {
    if (id <= 0) {
        throw InvalidContactId("Contact ID must be a positive integer.");
    }
    return repository_.findContactById(id);
}
