#include "http/contact_controller.hpp"

#include "application/contact_service.hpp"

namespace {

#include OATPP_CODEGEN_BEGIN(DTO)

class ContactDto : public oatpp::DTO {
    DTO_INIT(ContactDto, DTO)
    DTO_FIELD(Int64, id);
    DTO_FIELD(String, name);
    DTO_FIELD(String, phone);
    DTO_FIELD(String, address);
};

#include OATPP_CODEGEN_END(DTO)

}  // namespace

ContactController::ContactController(
    const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& mapper,
    const ContactService& service)
    : ApiController(mapper), service_(service) {}

std::shared_ptr<ContactController::OutgoingResponse> ContactController::listContacts() {
    auto response = oatpp::Vector<oatpp::Object<ContactDto>>::createShared();
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
