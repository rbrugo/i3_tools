/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : utils
 * @created     : Thursday Apr 29, 2021 11:51:52 CEST
 * @description : Various utility functions
 * */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <charconv>
#include <string_view>
#include <tl/optional.hpp>
#ifdef ENABLE_DEBUG_LOG
#include <fmt/core.h>
#endif // ENABLE_DEBUG_LOG

namespace brun
{
template <typename ...Args>
inline
void log([[maybe_unused]] Args &&... args)
{
#ifdef ENABLE_DEBUG_LOG
    fmt::print(stderr, std::forward<Args>(args)...);
#endif // ENABLE_DEBUG_LOG

}

[[nodiscard]] inline
auto stoi(std::string_view const sv)
    -> tl::optional<int>
{
    int res = -1;
    auto const [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), res);
    if (ec == std::errc()) {
        return res;
    }
    return tl::nullopt;
}
} // namespace brun

#endif /* UTILS_HPP */

