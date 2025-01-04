#pragma once
#include <string>
#include <uuid.h>

namespace SAT
{
struct SquadMember
{
    std::string AccountName;
    std::string CharacterName;
    using UUID = uuids::uuid;
};
} // namespace SAT
