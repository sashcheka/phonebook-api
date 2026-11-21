#pragma once

#include <oatpp/web/server/HttpRouter.hpp>
#include <oatpp/web/server/api/Endpoint.hpp>

void addSwagger(oatpp::web::server::HttpRouter& router,
                const oatpp::web::server::api::Endpoints& endpoints);
