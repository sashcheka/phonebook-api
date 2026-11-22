#pragma once

#include <cstdint>
#include <optional>
#include <string>

struct Contact {
    std::int64_t id;
    std::string name;
    std::string phone;
    std::string address;
};

struct ContactFields {
    std::string name;
    std::string phone;
    std::string address;
};

struct ContactPatch {
    std::optional<std::string> name;
    std::optional<std::string> phone;
    std::optional<std::string> address;
};
