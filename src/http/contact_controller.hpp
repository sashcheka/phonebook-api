#pragma once

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/web/server/api/ApiController.hpp>

class ContactService;

#include OATPP_CODEGEN_BEGIN(ApiController)

class ContactController : public oatpp::web::server::api::ApiController {
public:
    ContactController(const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& mapper,
                      const ContactService& service);

    ENDPOINT("GET", "/contacts", listContacts);

private:
    const ContactService& service_;
};

#include OATPP_CODEGEN_END(ApiController)
