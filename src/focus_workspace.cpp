/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : focus_workspace
 * @created     : Thursday Apr 08, 2021 22:28:01 CEST
 * @license     : MIT
 */

#include <charconv>
#include <algorithm>
#include <i3-ipc++/i3_ipc.hpp>
#include <fmt/core.h>

#include "workspaces.hpp"
#include "outputs.hpp"

#define ENABLE_DEBUG

int main(int argc, char * argv[])
{
    if (argc != 2) {
        fmt::print(stderr, "Usage: {} workspace_num\n", argv[0]);
        return 255;
    }
    auto const arg = [arg=argv[1]] {
        int n = -1;
        auto const [ptr, ec] = std::from_chars(arg, arg + std::strlen(arg), n);
        if (ec != std::errc{}) {
            fmt::print(stderr, "Argument passed is not a number\n");
            std::exit(1);
        }
        return n;
    }();

    auto const i3 = i3_ipc{std::getenv("I3SOCK")};
    auto const monitors = brun::retrieve_output_names(i3);
    auto const current = brun::focused_workspace_idx(i3).value_or(1);
    auto const other = brun::other_workspace_idx(i3).value_or(current);

#ifdef ENABLE_DEBUG
    fmt::print(stderr, "Focused: {}\n", current);
    fmt::print(stderr, "Other:   {}\n", other);
#endif

    if (arg == current) {
#ifdef ENABLE_DEBUG
        fmt::print(stderr, "Focusing from workspace {} using back and forth\n", arg);
#endif
        i3.execute_commands("workspace back_and_forth");
        return 0;
    }
    if (arg == other) {
#ifdef ENABLE_DEBUG
        fmt::print(stderr, "Swapping focus of workspaces {} and {}\n", current, other);
#endif
    }
    else if ((current - 1) / 10 != (arg - 1) / 10) {
#ifdef ENABLE_DEBUG
        fmt::print(stderr, "Focusing workspace {} from workspace {}\n", arg, current);
#endif
        i3.execute_commands(fmt::format("workspace --no-auto-back-and-forth {}", other));
        i3.execute_commands(fmt::format("focus output {}", monitors.at((other - 1) / 10)));
        i3.execute_commands(fmt::format("workspace --no-auto-back-and-forth {}", arg));
        i3.execute_commands(fmt::format("workspace --no-auto-back-and-forth {}", current));
    }
    i3.execute_commands(fmt::format("workspace --no-auto-back-and-forth {}", arg));
}
