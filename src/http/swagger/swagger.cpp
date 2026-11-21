#include "http/swagger/swagger.hpp"

#include <oatpp-swagger/Controller.hpp>

void addSwagger(oatpp::web::server::HttpRouter& router,
                const oatpp::web::server::api::Endpoints& endpoints) {
    auto document = oatpp::swagger::DocumentInfo::Builder()
                        .setTitle("Phonebook API")
                        .setVersion("0.1.0")
                        .build();
    auto resources = oatpp::swagger::Resources::loadResources(PHONEBOOK_SWAGGER_RES_DIR);
    router.addController(oatpp::swagger::Controller::createShared(endpoints, document, resources));
}
