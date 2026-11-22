#pragma once

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/Types.hpp>

#include OATPP_CODEGEN_BEGIN(DTO)

class ContactPatchDto : public oatpp::DTO {
    DTO_INIT(ContactPatchDto, DTO)
    DTO_FIELD(String, name);
    DTO_FIELD(String, phone);
    DTO_FIELD(String, address);
};

#include OATPP_CODEGEN_END(DTO)
