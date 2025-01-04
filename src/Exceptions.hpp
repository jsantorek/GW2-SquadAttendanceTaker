#pragma once

#include <exception>

namespace SAT
{
class NotInSquad : public std::exception
{
};
class HookingFailure : public std::exception
{
};
class UserDataUnavailable : public std::exception
{
};
} // namespace SAT
