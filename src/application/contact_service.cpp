#include "application/contact_service.hpp"

#include "application/contact_repository.hpp"

namespace {

void requirePositiveId(std::int64_t id) {
    if (id <= 0) {
        throw InvalidContactId("Contact ID must be a positive integer.");
    }
}

void requireNonBlank(const std::string& value, const char* field) {
    if (value.find_first_not_of(" \t\n\r\f\v") == std::string::npos) {
        throw InvalidContactData(std::string("The ") + field + " field must not be empty.");
    }
}

void validateContactFields(const ContactFields& fields) {
    requireNonBlank(fields.name, "name");
    requireNonBlank(fields.phone, "phone");
    requireNonBlank(fields.address, "address");
}

}  // namespace

ContactService::ContactService(const IContactRepository& repository)
    : repository_(repository) {}

std::vector<Contact> ContactService::listContacts() const {
    return repository_.listContacts();
}

std::optional<Contact> ContactService::findContactById(std::int64_t id) const {
    requirePositiveId(id);
    return repository_.findContactById(id);
}

Contact ContactService::createContact(const ContactFields& fields) const {
    validateContactFields(fields);
    return repository_.createContact(fields);
}

std::optional<Contact> ContactService::replaceContact(std::int64_t id, const ContactFields& fields) const {
    requirePositiveId(id);
    validateContactFields(fields);
    return repository_.replaceContact(id, fields);
}
