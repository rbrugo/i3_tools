/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : focus_window
 * @created     : Monday Apr 19, 2021 02:13:59 CEST
 * @description : a wrapper for "i3-msg focus" to focus containers also in fullscreen
 */
#include <i3-ipc++/i3_ipc.hpp>
#include <fmt/core.h>

#include "dry-comparisons.hpp"

#include "nodes.hpp"
#include "workspaces.hpp"
#include "outputs.hpp"

int main(int argc, char const * argv[])
{
    if (argc == 1) {
        fmt::print(stderr, "Required an argument: left, right, up, down\n");
        return 1;
    }

    auto const direction = std::string_view{argv[1]};

    if (direction == rollbear::none_of{"left", "right", "up", "down"}) {
        fmt::print(stderr, "The argument is required to be one of: left, right, up, down\n");
        return 1;
    }

    auto const i3 = i3_ipc{std::getenv("I3SOCK")};
    auto const current_output = brun::focused_workspace(i3).map_or_else(
        [](auto && ws) { return ws.output; },
        [&i3] { return brun::retrieve_output_names(i3).front(); }  // Mmmmh
    );

    // Get the position of the focused window
    auto const focused_position = brun::node_on_border(i3);

#ifdef ENABLE_DEBUG
    fmt::print("Position on border: {}\n", print_border(focused_position));
#endif

    // Check if the focused window in the currently focused ws is in fullscreen
    using brun::border;
    auto focused = brun::focused_node(i3);
    auto fullscreen = focused
        .map([](auto node) { return node.fullscreen_mode; })
        .map([](auto mode) { return mode != i3_containers::fullscreen_mode_type::no_fullscreen; })
        .value_or(false);

    auto change_screen = (direction == "left" and brun::is_on_<border::left>(focused_position))
                      or (direction == "right" and brun::is_on_<border::right>(focused_position))
                      or (direction == "up" and brun::is_on_<border::top>(focused_position))
                      or (direction == "down" and brun::is_on_<border::bottom>(focused_position))
                      ;
#ifdef ENABLE_DEBUG
    fmt::print("Changing screen: {}\n", change_screen);
#endif

    // I need to toggle the fullscreen only if the fullscreen is active and if the "next" node is in
    //  the same output as the current
    auto switch_fs = fullscreen and not change_screen;

    i3.execute_commands(fmt::format("{1} focus {0}; {1}", direction, switch_fs ? "fullscreen toggle;" : ""));
    return 0;
}

