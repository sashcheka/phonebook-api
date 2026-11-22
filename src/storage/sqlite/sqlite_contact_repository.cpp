#include "storage/sqlite/sqlite_contact_repository.hpp"

#include <oatpp-sqlite/orm.hpp>

#include <stdexcept>

namespace {

#include OATPP_CODEGEN_BEGIN(DTO)

class ContactRow : public oatpp::DTO {
    DTO_INIT(ContactRow, DTO)
    DTO_FIELD(Int64, id);
    DTO_FIELD(String, name);
    DTO_FIELD(String, phone);
    DTO_FIELD(String, address);
};

#include OATPP_CODEGEN_END(DTO)

Contact toContact(const oatpp::Object<ContactRow>& row) {
    return {*row->id, *row->name, *row->phone, *row->address};
}

oatpp::String toSqlString(const std::optional<std::string>& value) {
    return value ? oatpp::String(*value) : oatpp::String(nullptr);
}

}  // namespace

#include OATPP_CODEGEN_BEGIN(DbClient)

class SqliteContactRepository::Client : public oatpp::orm::DbClient {
public:
    explicit Client(const std::shared_ptr<oatpp::orm::Executor>& executor)
        : DbClient(executor) {
        oatpp::orm::SchemaMigration migration{executor};
        migration.addFile(1, PHONEBOOK_MIGRATIONS_DIR "/init_db.sql");
        migration.migrate();
    }

    QUERY(selectContacts, "SELECT id, name, phone, address FROM contacts ORDER BY id;")
    QUERY(selectContactById, "SELECT id, name, phone, address FROM contacts WHERE id = :id;",
          PARAM(oatpp::Int64, id))
    QUERY(insertContact,
          "INSERT INTO contacts (name, phone, address) VALUES (:name, :phone, :address) "
          "RETURNING id, name, phone, address;",
          PARAM(oatpp::String, name), PARAM(oatpp::String, phone), PARAM(oatpp::String, address))
    QUERY(updateContactById,
          "UPDATE contacts SET name = COALESCE(:name, name), phone = COALESCE(:phone, phone), "
          "address = COALESCE(:address, address) WHERE id = :id "
          "RETURNING id, name, phone, address;",
          PARAM(oatpp::Int64, id), PARAM(oatpp::String, name),
          PARAM(oatpp::String, phone), PARAM(oatpp::String, address))
    QUERY(deleteContactById, "DELETE FROM contacts WHERE id = :id RETURNING id;",
          PARAM(oatpp::Int64, id))
};

#include OATPP_CODEGEN_END(DbClient)

SqliteContactRepository::SqliteContactRepository(const char* database_path) {
    auto provider = std::make_shared<oatpp::sqlite::ConnectionProvider>(database_path);
    auto executor = std::make_shared<oatpp::sqlite::Executor>(provider);
    client_ = std::make_shared<Client>(executor);
}

std::vector<Contact> SqliteContactRepository::listContacts() const {
    auto result = client_->selectContacts();
    if (!result->isSuccess()) {
        throw std::runtime_error("Could not list contacts: " + *result->getErrorMessage());
    }

    const auto rows = result->fetch<oatpp::Vector<oatpp::Object<ContactRow>>>();
    std::vector<Contact> contacts;
    contacts.reserve(rows->size());
    for (const auto& row : *rows) {
        contacts.push_back(toContact(row));
    }
    return contacts;
}

std::optional<Contact> SqliteContactRepository::findContactById(std::int64_t id) const {
    auto result = client_->selectContactById(id);
    if (!result->isSuccess()) {
        throw std::runtime_error("Could not find contact: " + *result->getErrorMessage());
    }

    const auto rows = result->fetch<oatpp::Vector<oatpp::Object<ContactRow>>>();
    if (rows->empty()) {
        return std::nullopt;
    }
    return toContact(rows[0]);
}

Contact SqliteContactRepository::createContact(const ContactFields& fields) const {
    auto result = client_->insertContact(fields.name, fields.phone, fields.address);
    if (!result->isSuccess()) {
        throw std::runtime_error("Could not create contact: " + *result->getErrorMessage());
    }

    const auto rows = result->fetch<oatpp::Vector<oatpp::Object<ContactRow>>>();
    if (!result->isSuccess() || rows->empty()) {
        throw std::runtime_error("Could not complete contact creation.");
    }
    return toContact(rows[0]);
}

std::optional<Contact> SqliteContactRepository::replaceContact(std::int64_t id, const ContactFields& fields) const {
    return updateContactFields(id, {fields.name, fields.phone, fields.address});
}

std::optional<Contact> SqliteContactRepository::updateContactFields(std::int64_t id, const ContactPatch& patch) const {
    auto result = client_->updateContactById(
        id, toSqlString(patch.name), toSqlString(patch.phone), toSqlString(patch.address));
    if (!result->isSuccess()) {
        throw std::runtime_error("Could not update contact: " + *result->getErrorMessage());
    }

    const auto rows = result->fetch<oatpp::Vector<oatpp::Object<ContactRow>>>();
    if (!result->isSuccess()) {
        throw std::runtime_error("Could not complete contact update.");
    }
    if (rows->empty()) {
        return std::nullopt;
    }
    return toContact(rows[0]);
}

bool SqliteContactRepository::deleteContact(std::int64_t id) const {
    auto result = client_->deleteContactById(id);
    if (!result->isSuccess()) {
        throw std::runtime_error("Could not delete contact: " + *result->getErrorMessage());
    }

    const auto deleted = result->fetch<oatpp::Vector<oatpp::Vector<oatpp::Int64>>>();
    if (!result->isSuccess()) {
        throw std::runtime_error("Could not complete contact deletion.");
    }
    return !deleted->empty();
}
