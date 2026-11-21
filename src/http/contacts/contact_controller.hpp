#pragma once

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/web/server/api/ApiController.hpp>

class IContactService;

#include OATPP_CODEGEN_BEGIN(ApiController)

class ContactController : public oatpp::web::server::api::ApiController {
public:
    ContactController(const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& mapper,
                      const IContactService& service);

    ENDPOINT_INFO(listContacts) {
        describeListContacts(*info);
    }
    ENDPOINT("GET", "/contacts", listContacts);

    ENDPOINT_INFO(getContactById) {
        describeGetContactById(*info);
    }
    ENDPOINT("GET", "/contacts/{id}", getContactById, PATH(String, id));

private:
    static void describeListContacts(oatpp::web::server::api::Endpoint::Info& info);
    static void describeGetContactById(oatpp::web::server::api::Endpoint::Info& info);

    const IContactService& service_;
};

#include OATPP_CODEGEN_END(ApiController)
