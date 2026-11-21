#include "application/contact_service.hpp"

#include "application/contact_repository.hpp"

ContactService::ContactService(const IContactRepository& repository)
    : repository_(repository) {}

std::vector<Contact> ContactService::listContacts() const {
    return repository_.listContacts();
}
