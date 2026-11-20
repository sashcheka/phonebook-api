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
        contacts.push_back({*row->id, *row->name, *row->phone, *row->address});
    }
    return contacts;
}
