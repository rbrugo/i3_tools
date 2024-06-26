/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : focus_workspace
 * @created     : Thursday Apr 08, 2021 22:28:01 CEST
 * @description : a tool to help focusing the right workspace on the right monitor in a multimonitor i3 setup
 */

#include <charconv>
#include <algorithm>
#include <i3-ipc++/i3_ipc.hpp>
#include <i3-ipc++/i3_ipc_bad_message.hpp>
#include <fmt/core.h>
#ifdef ENABLE_DEBUG
#include <fmt/ranges.h>
#endif

#include "workspaces.hpp"
#include "outputs.hpp"

auto get_target_ws(i3_ipc const & i3, std::string_view arg)
    -> int64_t
{
    int n = -1;
    // If it's a number, all good
    auto const [ptr, ec] = std::from_chars(arg.data(), arg.data() + arg.length(), n);
    if (ec == std::errc{}) {
        return n;
    }
    // If it does start with "mark:", erase that part
    if (arg.starts_with("mark:")) {
        arg.remove_prefix(5);
    }
    // Check if it effectively is a mark
    if (auto const & marks = i3.get_marks(); std::ranges::find(marks, arg) == marks.end()) {
        fmt::print(stderr, "Argument passed ({}) is not a number nor a mark\n", arg);
        std::exit(1);
    }
    return *brun::get_workspace_from_node_id(i3, brun::find_ws_by_mark(i3, arg).value().id)->num;
}

int main(int argc, char const * argv[])
{
    if (argc != 2) {
        fmt::print(stderr, "Usage: {} <workspace_num|mark> \n", argv[0]);
        return 255;
    }
    auto const i3 = i3_ipc{std::getenv("I3SOCK")};
    auto const target_ws = get_target_ws(i3, argv[1]);

    auto const monitors = brun::retrieve_output_names(i3);
    auto const current_ws = brun::focused_workspace_idx(i3).value_or(1);
    auto const other_focused_ws = brun::other_workspace_idx(i3).value_or(current_ws);

#ifdef ENABLE_DEBUG
    fmt::print(stderr, "Focused ws:   {}\n", current_ws);
    fmt::print(stderr, "Other focused ws:   {}\n", other_focused_ws);
    fmt::print(stderr, "Ws to focus:   {}\n", target_ws);
#endif

    if (current_ws == other_focused_ws) {
#ifdef ENABLE_DEBUG
        fmt::print(stderr, "Only workspace {} is focused\n", current_ws);
#endif
        i3.execute_commands(fmt::format("workspace {}", target_ws));
    }
    else if (target_ws == other_focused_ws) {
#ifdef ENABLE_DEBUG
        fmt::print(stderr, "Swapping focus of workspaces {} and {}\n", current_ws, other_focused_ws);
#endif
        i3.execute_commands(fmt::format("workspace --no-auto-back-and-forth {}", target_ws));
    }
    else if (target_ws == current_ws) {
#ifdef ENABLE_DEBUG
        fmt::print(stderr, "Focusing from workspace {} using back and forth\n", target_ws);
#endif
        i3.execute_commands("workspace back_and_forth");
    }
    else {
        auto const current_output = (current_ws - 1) / 10;
        auto const other_focused_output = (other_focused_ws - 1) / 10;
        auto const target_output  = (target_ws - 1) / 10;
        if (current_output != target_output) {
#ifdef ENABLE_DEBUG
            fmt::print(stderr, "Current output is not target output\n");
#endif
            i3.execute_commands(fmt::format("workspace --no-auto-back-and-forth {}", other_focused_ws));
            i3.execute_commands(fmt::format("focus output {}", monitors.at(other_focused_output)));
            i3.execute_commands(fmt::format("workspace --no-auto-back-and-forth {}", target_ws));
            i3.execute_commands(fmt::format("workspace --no-auto-back-and-forth {}", current_ws));
            i3.execute_commands(fmt::format("workspace --no-auto-back-and-forth {}", target_ws));
        } else {
#ifdef ENABLE_DEBUG
            fmt::print(stderr, "Current output is target output\n");
#endif
            i3.execute_commands(fmt::format("workspace --no-auto-back-and-forth {}", target_ws));
        }
    }
}
