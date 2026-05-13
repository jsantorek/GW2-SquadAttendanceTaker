#pragma once
#include <Game/Types.h>
#include <string>

namespace SAT
{
struct SquadMember
{
    std::string AccountName;
    std::string CharacterName;
    using UUID = GW2RE::GUID_t;
};
} // namespace SAT
namespace std
{
template <> struct hash<GW2RE::GUID_t>
{
    std::size_t operator()(const GW2RE::GUID_t &guid) const noexcept
    {
        std::size_t h1 = std::hash<std::uint32_t>{}(guid.Data1);
        std::size_t h2 = std::hash<std::uint32_t>{}(guid.Data2);
        std::size_t h3 = std::hash<std::uint32_t>{}(guid.Data3);
        std::size_t h4 = std::hash<std::uint32_t>{}(guid.Data4);

        // hash combine
        std::size_t seed = h1;
        seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h4 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

        return seed;
    }
};
} // namespace std
