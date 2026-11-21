#include "server.hpp"

#include "application/contact_service.hpp"
#include "http/contacts/contact_controller.hpp"
#include "http/health_controller.hpp"
#include "http/swagger/swagger.hpp"
#include "storage/sqlite/sqlite_contact_repository.hpp"

#include <oatpp/network/Address.hpp>
#include <oatpp/network/Server.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <oatpp/web/server/HttpConnectionHandler.hpp>
#include <oatpp/web/server/HttpRouter.hpp>

#include <memory>

void runServer() {
    const SqliteContactRepository repository{"phonebook.db"};
    const ContactService contacts{repository};
    const auto router = oatpp::web::server::HttpRouter::createShared();
    const auto mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

    const auto health_controller = std::make_shared<HealthController>(mapper);
    const auto contact_controller = std::make_shared<ContactController>(mapper, contacts);
    router->addController(health_controller);
    router->addController(contact_controller);

    oatpp::web::server::api::Endpoints documented_endpoints;
    documented_endpoints.append(health_controller->getEndpoints());
    documented_endpoints.append(contact_controller->getEndpoints());
    addSwagger(*router, documented_endpoints);

    const auto handler = oatpp::web::server::HttpConnectionHandler::createShared(router);
    const auto provider = oatpp::network::tcp::server::ConnectionProvider::createShared(
        {"127.0.0.1", 8000, oatpp::network::Address::IP_4}
    );

    const auto server = oatpp::network::Server::createShared(provider, handler);
    server->run();
}
