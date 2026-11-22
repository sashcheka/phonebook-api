#include "http/contacts/contact_controller.hpp"

#include "application/contact_service_interface.hpp"
#include "http/contacts/contact_dto.hpp"
#include "http/contacts/create_contact_dto.hpp"
#include "http/error_dto.hpp"

#include <nlohmann/json.hpp>

#include <charconv>
#include <cstdint>
#include <exception>
#include <optional>
#include <string>

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

std::optional<NewContact> readNewContact(const oatpp::String& body) {
    if (!body || body->find('\0') != std::string::npos) {
        return std::nullopt;
    }
    // Some Oat++ 1.3 releases accept missing or trailing commas.
    const auto json = nlohmann::json::parse(*body, nullptr, false);
    if (!json.is_object()) {
        return std::nullopt;
    }
    try {
        return NewContact{
            json.value("name", std::string{}),
            json.value("phone", std::string{}),
            json.value("address", std::string{}),
        };
    } catch (const nlohmann::json::type_error&) {
        return std::nullopt;
    }
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

void ContactController::describeCreateContact(oatpp::web::server::api::Endpoint::Info& info) {
    info.summary = "Create a contact";
    info.addConsumes<oatpp::Object<CreateContactDto>>("application/json");
    info.addResponse<oatpp::Object<ContactDto>>(Status::CODE_201, "application/json");
    info.addResponse<oatpp::Object<ErrorDto>>(Status::CODE_400, "application/json", "Invalid request body or contact fields");
    info.addResponse<oatpp::Object<ErrorDto>>(Status::CODE_500, "application/json", "Could not create contact");
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
    if (error != std::errc{} || parsed_end != end) {
        return createDtoResponse(Status::CODE_400,
                                 makeError("invalid_contact_id", "Contact ID must be a positive integer."));
    }

    try {
        const auto contact = service_.findContactById(contact_id);
        if (!contact) {
            return createDtoResponse(Status::CODE_404, makeError("contact_not_found", "Contact not found."));
        }
        return createDtoResponse(Status::CODE_200, toContactDto(*contact));
    } catch (const InvalidContactId& exception) {
        return createDtoResponse(Status::CODE_400, makeError("invalid_contact_id", exception.what()));
    }
}

std::shared_ptr<ContactController::OutgoingResponse> ContactController::createContact(
    const oatpp::String& body) {
    const auto input = readNewContact(body);
    if (!input) {
        return createDtoResponse(
            Status::CODE_400, makeError("invalid_json", "Expected a JSON object with string contact fields."));
    }

    try {
        const auto contact = service_.createContact(*input);
        auto response = createDtoResponse(Status::CODE_201, toContactDto(contact));
        response->putHeader("Location", "/contacts/" + std::to_string(contact.id));
        return response;
    } catch (const InvalidContactData& exception) {
        return createDtoResponse(Status::CODE_400, makeError("invalid_contact_data", exception.what()));
    } catch (const std::exception& exception) {
        OATPP_LOGE("ContactController", "POST /contacts: %s", exception.what());
        return createDtoResponse(
            Status::CODE_500, makeError("contact_create_failed", "Could not create contact."));
    }
}
