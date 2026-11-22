#include "http/contacts/contact_controller.hpp"

#include "application/contact_service_interface.hpp"
#include "http/contacts/contact_dto.hpp"
#include "http/contacts/contact_patch_dto.hpp"
#include "http/contacts/create_contact_dto.hpp"
#include "http/contacts/update_contact_dto.hpp"
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

std::optional<std::int64_t> parseContactId(const oatpp::String& id) {
    if (!id) {
        return std::nullopt;
    }
    std::int64_t value = 0;
    const char* end = id->data() + id->size();
    const auto [parsed_end, error] = std::from_chars(id->data(), end, value);
    if (error != std::errc{} || parsed_end != end) {
        return std::nullopt;
    }
    return value;
}

nlohmann::json parseJsonBody(const oatpp::String& body) {
    if (!body || body->find('\0') != std::string::npos) {
        return nullptr;
    }
    // Some Oat++ 1.3 releases accept missing or trailing commas.
    return nlohmann::json::parse(*body, nullptr, false);
}

std::optional<ContactFields> readContactFields(const oatpp::String& body) {
    const auto json = parseJsonBody(body);
    if (!json.is_object()) {
        return std::nullopt;
    }
    try {
        return ContactFields{
            json.value("name", std::string{}),
            json.value("phone", std::string{}),
            json.value("address", std::string{}),
        };
    } catch (const nlohmann::json::type_error&) {
        return std::nullopt;
    }
}

std::optional<ContactPatch> readContactPatch(const oatpp::String& body) {
    const auto json = parseJsonBody(body);
    if (!json.is_object()) {
        return std::nullopt;
    }

    ContactPatch patch;
    for (const auto& [field, value] : json.items()) {
        if (!value.is_string()) {
            return std::nullopt;
        }
        const auto& text = value.get_ref<const std::string&>();
        if (field == "name") {
            patch.name = text;
        } else if (field == "phone") {
            patch.phone = text;
        } else if (field == "address") {
            patch.address = text;
        } else {
            return std::nullopt;
        }
    }
    return patch;
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

void ContactController::describeUpdateContact(oatpp::web::server::api::Endpoint::Info& info) {
    info.summary = "Replace a contact's fields";
    info.pathParams["id"].description = "Positive 64-bit integer";
    info.addConsumes<oatpp::Object<UpdateContactDto>>("application/json");
    info.addResponse<oatpp::Object<ContactDto>>(Status::CODE_200, "application/json");
    info.addResponse<oatpp::Object<ErrorDto>>(Status::CODE_400, "application/json",
                                             "Invalid ID, request body, or contact fields");
    info.addResponse<oatpp::Object<ErrorDto>>(Status::CODE_404, "application/json", "Contact not found");
    info.addResponse<oatpp::Object<ErrorDto>>(Status::CODE_500, "application/json", "Could not update contact");
}

void ContactController::describePatchContact(oatpp::web::server::api::Endpoint::Info& info) {
    info.summary = "Update selected contact fields";
    info.pathParams["id"].description = "Positive 64-bit integer";
    info.body.description = "Supply the fields to change. Null values and unknown fields are invalid.";
    info.addConsumes<oatpp::Object<ContactPatchDto>>("application/merge-patch+json");
    info.addResponse<oatpp::Object<ContactDto>>(Status::CODE_200, "application/json");
    info.addResponse<oatpp::Object<ErrorDto>>(Status::CODE_400, "application/json",
                                             "Invalid ID, request body, or contact fields");
    info.addResponse<oatpp::Object<ErrorDto>>(Status::CODE_404, "application/json", "Contact not found");
    info.addResponse<oatpp::Object<ErrorDto>>(Status::CODE_500, "application/json", "Could not update contact");
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
    const auto contact_id = parseContactId(id);
    if (!contact_id) {
        return createDtoResponse(Status::CODE_400,
                                 makeError("invalid_contact_id", "Contact ID must be a positive integer."));
    }

    try {
        const auto contact = service_.findContactById(*contact_id);
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
    const auto input = readContactFields(body);
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

std::shared_ptr<ContactController::OutgoingResponse> ContactController::updateContact(
    const oatpp::String& id, const oatpp::String& body) {
    const auto contact_id = parseContactId(id);
    if (!contact_id) {
        return createDtoResponse(Status::CODE_400,
                                 makeError("invalid_contact_id", "Contact ID must be a positive integer."));
    }

    const auto input = readContactFields(body);
    if (!input) {
        return createDtoResponse(
            Status::CODE_400, makeError("invalid_json", "Expected a JSON object with string contact fields."));
    }

    try {
        const auto contact = service_.replaceContact(*contact_id, *input);
        if (!contact) {
            return createDtoResponse(Status::CODE_404, makeError("contact_not_found", "Contact not found."));
        }
        return createDtoResponse(Status::CODE_200, toContactDto(*contact));
    } catch (const InvalidContactId& exception) {
        return createDtoResponse(Status::CODE_400, makeError("invalid_contact_id", exception.what()));
    } catch (const InvalidContactData& exception) {
        return createDtoResponse(Status::CODE_400, makeError("invalid_contact_data", exception.what()));
    } catch (const std::exception& exception) {
        OATPP_LOGE("ContactController", "PUT /contacts/{id}: %s", exception.what());
        return createDtoResponse(
            Status::CODE_500, makeError("contact_update_failed", "Could not update contact."));
    }
}

std::shared_ptr<ContactController::OutgoingResponse> ContactController::patchContact(
    const oatpp::String& id, const oatpp::String& body) {
    const auto contact_id = parseContactId(id);
    if (!contact_id) {
        return createDtoResponse(Status::CODE_400,
                                 makeError("invalid_contact_id", "Contact ID must be a positive integer."));
    }

    const auto patch = readContactPatch(body);
    if (!patch) {
        return createDtoResponse(
            Status::CODE_400, makeError("invalid_json", "Expected a JSON object with optional string fields: name, phone, address."));
    }

    try {
        const auto contact = service_.updateContactFields(*contact_id, *patch);
        if (!contact) {
            return createDtoResponse(Status::CODE_404, makeError("contact_not_found", "Contact not found."));
        }
        return createDtoResponse(Status::CODE_200, toContactDto(*contact));
    } catch (const InvalidContactId& exception) {
        return createDtoResponse(Status::CODE_400, makeError("invalid_contact_id", exception.what()));
    } catch (const InvalidContactData& exception) {
        return createDtoResponse(Status::CODE_400, makeError("invalid_contact_data", exception.what()));
    } catch (const std::exception& exception) {
        OATPP_LOGE("ContactController", "PATCH /contacts/{id}: %s", exception.what());
        return createDtoResponse(
            Status::CODE_500, makeError("contact_update_failed", "Could not update contact."));
    }
}
