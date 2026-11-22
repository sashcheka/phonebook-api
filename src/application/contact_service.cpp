#include "application/contact_service.hpp"

#include "application/contact_repository.hpp"

namespace {

void requireNonBlank(const std::string& value, const char* field) {
    if (value.find_first_not_of(" \t\n\r\f\v") == std::string::npos) {
        throw InvalidContactData(std::string("The ") + field + " field must not be empty.");
    }
}

}  // namespace

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

Contact ContactService::createContact(const NewContact& contact) const {
    requireNonBlank(contact.name, "name");
    requireNonBlank(contact.phone, "phone");
    requireNonBlank(contact.address, "address");
    return repository_.createContact(contact);
}
