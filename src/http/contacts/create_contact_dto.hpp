#pragma once

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/Types.hpp>

#include OATPP_CODEGEN_BEGIN(DTO)

class CreateContactDto : public oatpp::DTO {
    DTO_INIT(CreateContactDto, DTO)

    DTO_FIELD(String, name);
    DTO_FIELD_INFO(name) { info->required = true; }

    DTO_FIELD(String, phone);
    DTO_FIELD_INFO(phone) { info->required = true; }

    DTO_FIELD(String, address);
    DTO_FIELD_INFO(address) { info->required = true; }
};

#include OATPP_CODEGEN_END(DTO)
