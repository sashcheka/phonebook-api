#pragma once

#include "http/contacts/contact_dto.hpp"

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/web/server/api/ApiController.hpp>

class IContactService;

#include OATPP_CODEGEN_BEGIN(ApiController)

class ContactController : public oatpp::web::server::api::ApiController {
public:
    ContactController(const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& mapper,
                      const IContactService& service);

    ENDPOINT_INFO(listContacts) {
        info->summary = "List contacts";
        info->addResponse<ContactListDto>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/contacts", listContacts);

private:
    const IContactService& service_;
};

#include OATPP_CODEGEN_END(ApiController)
