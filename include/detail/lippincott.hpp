/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : lippincott
 * @created     : Friday Apr 23, 2021 19:47:09 CEST
 * */

#ifndef DETAIL_LIPPINCOTT_HPP
#define DETAIL_LIPPINCOTT_HPP

#include <fmt/core.h>
#include <i3-ipc++/i3_ipc_bad_message.hpp>

namespace brun::detail
{
[[noreturn]] inline void lippincott()
try { throw; }
catch (i3_ipc_bad_message const & exc)
{
    fmt::print(stderr, "Bad message: {}\n", exc.what());
    std::exit(255);
}
catch (std::exception const & exc)
{
    fmt::print(stderr, "Got exception: {}\n", exc.what());
    std::exit(255);
}
} // namespace brun::detail

#endif /* DETAIL_LIPPINCOTT_HPP */

