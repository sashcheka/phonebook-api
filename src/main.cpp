#include "application/contact_service.hpp"
#include "http/contact_controller.hpp"
#include "http/health_controller.hpp"
#include "storage/sqlite/sqlite_contact_repository.hpp"

#include <oatpp/core/base/Environment.hpp>
#include <oatpp/network/Address.hpp>
#include <oatpp/network/Server.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <oatpp/web/server/HttpConnectionHandler.hpp>
#include <oatpp/web/server/HttpRouter.hpp>

#include <exception>
#include <iostream>
#include <memory>

void runServer() {
    const SqliteContactRepository repository{"phonebook.db"};
    const ContactService contacts{repository};
    auto router = oatpp::web::server::HttpRouter::createShared();
    auto mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

    router->addController(std::make_shared<HealthController>(mapper));
    router->addController(std::make_shared<ContactController>(mapper, contacts));

    auto handler = oatpp::web::server::HttpConnectionHandler::createShared(router);
    auto provider = oatpp::network::tcp::server::ConnectionProvider::createShared(
        {"127.0.0.1", 8000, oatpp::network::Address::IP_4}
    );

    auto server = oatpp::network::Server::createShared(provider, handler);
    server->run();
}

int main() {
    oatpp::base::Environment::init();
    std::cout << "Oat++ initialized" << "\n";

    try {
        runServer();
    } catch (const std::exception& error) {
        std::cerr << "Phonebook API failed: " << error.what() << '\n';
        oatpp::base::Environment::destroy();
        return 1;
    }

    oatpp::base::Environment::destroy();
    return 0;
}
