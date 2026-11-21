#include "http/contacts/contact_controller.hpp"

#include "application/contact_service_interface.hpp"
#include "http/contacts/contact_dto.hpp"

ContactController::ContactController(
    const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& mapper,
    const IContactService& service)
    : ApiController(mapper), service_(service) {}

void ContactController::describeListContacts(oatpp::web::server::api::Endpoint::Info& info) {
    info.summary = "List contacts";
    info.addResponse<ContactListDto>(Status::CODE_200, "application/json");
}

std::shared_ptr<ContactController::OutgoingResponse> ContactController::listContacts() {
    auto response = ContactListDto::createShared();
    for (const auto& contact : service_.listContacts()) {
        auto dto = ContactDto::createShared();
        dto->id = contact.id;
        dto->name = contact.name;
        dto->phone = contact.phone;
        dto->address = contact.address;
        response->push_back(dto);
    }
    return createDtoResponse(Status::CODE_200, response);
}
