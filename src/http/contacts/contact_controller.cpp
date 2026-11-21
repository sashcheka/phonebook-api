#include "http/contacts/contact_controller.hpp"

#include "application/contact_service_interface.hpp"
#include "http/contacts/contact_dto.hpp"
#include "http/error_dto.hpp"

#include <charconv>
#include <cstdint>

namespace {

oatpp::Object<ContactDto> toContactDto(const Contact& contact) {
    auto dto = ContactDto::createShared();
    dto->id = contact.id;
    dto->name = contact.name;
    dto->phone = contact.phone;
    dto->address = contact.address;
    return dto;
}

oatpp::Object<ErrorDto> makeError(const char* code, const oatpp::String& message) {
    auto error = ErrorDto::createShared();
    error->code = code;
    error->message = message;
    return error;
}

}  // namespace

ContactController::ContactController(
    const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& mapper,
    const IContactService& service)
    : ApiController(mapper), service_(service) {}

void ContactController::describeListContacts(oatpp::web::server::api::Endpoint::Info& info) {
    info.summary = "List contacts";
    info.addResponse<ContactListDto>(Status::CODE_200, "application/json");
}

void ContactController::describeGetContactById(oatpp::web::server::api::Endpoint::Info& info) {
    info.summary = "Get a contact by ID";
    info.pathParams["id"].description = "Positive 64-bit integer";
    info.addResponse<oatpp::Object<ContactDto>>(Status::CODE_200, "application/json");
    info.addResponse<oatpp::Object<ErrorDto>>(Status::CODE_400, "application/json", "Invalid contact ID");
    info.addResponse<oatpp::Object<ErrorDto>>(Status::CODE_404, "application/json", "Contact not found");
}

std::shared_ptr<ContactController::OutgoingResponse> ContactController::listContacts() {
    auto response = ContactListDto::createShared();
    for (const auto& contact : service_.listContacts()) {
        response->push_back(toContactDto(contact));
    }
    return createDtoResponse(Status::CODE_200, response);
}

std::shared_ptr<ContactController::OutgoingResponse> ContactController::getContactById(
    const oatpp::String& id) {
    std::int64_t contact_id = 0;
    const char* end = id->data() + id->size();
    const auto [parsed_end, error] = std::from_chars(id->data(), end, contact_id);
    if (error != std::errc{} || parsed_end != end || contact_id <= 0) {
        return createDtoResponse(Status::CODE_400,
                                 makeError("invalid_contact_id", "Contact ID must be a positive integer."));
    }

    const auto contact = service_.findContactById(contact_id);
    if (!contact) {
        return createDtoResponse(Status::CODE_404, makeError("contact_not_found", "Contact not found."));
    }
    return createDtoResponse(Status::CODE_200, toContactDto(*contact));
}
