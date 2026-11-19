#pragma once

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class HealthController : public oatpp::web::server::api::ApiController {
public:
    using ApiController::ApiController;

    ENDPOINT("GET", "/health", health);
};

#include OATPP_CODEGEN_END(ApiController)
