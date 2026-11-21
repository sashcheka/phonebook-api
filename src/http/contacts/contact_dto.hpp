#pragma once

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/Types.hpp>

#include OATPP_CODEGEN_BEGIN(DTO)

class ContactDto : public oatpp::DTO {
    DTO_INIT(ContactDto, DTO)
    DTO_FIELD(Int64, id);
    DTO_FIELD(String, name);
    DTO_FIELD(String, phone);
    DTO_FIELD(String, address);
};

#include OATPP_CODEGEN_END(DTO)

using ContactListDto = oatpp::Vector<oatpp::Object<ContactDto>>;
