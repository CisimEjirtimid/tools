#pragma once

#include <array>
#include <cstring>
#include <string>

namespace cisim
{
    struct strcmp_functor
    {
        inline bool operator()(const std::string_view& lhs, const std::string_view& rhs)
        {
            return lhs < rhs;
        }
    };
}
