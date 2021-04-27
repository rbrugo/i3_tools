/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : mv_workspace
 * @created     : Wednesday Apr 07, 2021 23:11:51 CEST
 * @license     : MIT
 */

#include <thread>
#include <algorithm>
#include <fmt/core.h>
#include <i3-ipc++/i3_ipc.hpp>

#include "lift.hpp"

#include "workspaces.hpp"
#include "outputs.hpp"

/**
 * Check the number of the current workspace.
 *
 * Check if the workspace num is in a valid range and, if not, moves it to a better value and
 * returns the new "current".
 * Note that since i3 uses "-1" for unnamed monitors, that value must not be considered an error.
 * */
auto fix_ws_number(i3_ipc const & i3, int current, auto const & monitors)
    -> std::optional<int>
{
    // If the workspace number is too high, find the nearest free workspace to the right placement
    //  and move it there
    auto const max_ws = std::ssize(monitors) * 10;
#ifdef ENABLE_DEBUG
    fmt::print("Max ws is {}\n", max_ws);
    for (auto const & monitor : monitors)
    {
        fmt::print(stderr, "- {}\n", monitor);
    }
#endif
    if (current > max_ws) {
        auto base = max_ws - 10 + current % 10;

        auto const workspaces = i3.get_workspaces();
        for (auto const offset : std::views::iota(0)) {
            auto const num = &i3_containers::workspace::num;
            if (base + offset <= max_ws) {
                if (auto found = std::ranges::find(workspaces, base + offset, num);
                    found == std::ranges::end(workspaces))
                {
                    i3.execute_commands(fmt::format("rename workspace to {}", base + offset));
#ifdef ENABLE_DEBUG
                    fmt::print(stderr, "Moved workspace {} to {}\n", current, base + offset);
#endif
                    return {base + offset};
                }
            }
            if (base - offset > 0) {
                if (auto found = std::ranges::find(workspaces, base - offset, num);
                    found == std::ranges::end(workspaces))
                {
                    i3.execute_commands(fmt::format("rename workspace to {}", base - offset));
#ifdef ENABLE_DEBUG
                    fmt::print(stderr, "Moved workspace {} to {}\n", current, base - offset);
#endif
                    return {base - offset};
                }
            }
        }
    }

    return std::nullopt;
}


/**
 * Check the output of the current container.
 *
 * Check if the output assigned to the workspace is compatible with its number. If it's not,
 * the workspace is moved to the right output.
 * */
bool fix_ws_output(i3_ipc const & i3, int current, auto const & monitors)
{
    auto const idx = (current - 1) / 10;
    auto const current_output  = brun::workspace_output(i3, current);
    auto const computed_output = monitors.at(idx);

    if (current_output != computed_output) {
#ifdef ENABLE_DEBUG
        fmt::print(stderr, "Moving workspace from {} to {}\n", current, current_output, computed_output);
#endif
        i3.execute_commands(fmt::format("move workspace to output {}", computed_output));
        return true;
    }
    return false;
}


int main(int argc, char * argv[])
{
    using std::literals::operator""sv;
    if (argc != 2 or (argv[1] != "prev"sv and argv[1] != "next"sv)) {
        fmt::print(stderr, "Usage: {} (next|prev)\n", argv[0]);
        return 255;
    }
    auto const arg = std::string_view{argv[1]};

    auto const i3 = i3_ipc{std::getenv("I3SOCK")};
    auto const monitors = brun::retrieve_output_names(i3);
    auto const focused = brun::focused_workspace_idx(i3).value_or(1);
#ifdef ENABLE_DEBUG
    fmt::print(stderr, "Focused ws: {}\n", focused);
#endif

    if (auto new_current = fix_ws_number(i3, focused, monitors); new_current.has_value()) {
        fix_ws_output(i3, *new_current, monitors);
        return 0;
    }
    if (fix_ws_output(i3, focused, monitors)) {
        return 0;
    }

    auto const new_val = focused + (arg == "next" ? 10 : arg == "prev" ? -10 : 0);
    if (new_val <= 0 or new_val > std::ssize(monitors) * 10) {
        fmt::print(stderr,
           "Workspace {} is already in the extremal output {}\n", focused, monitors.at((focused - 1) / 10)
        );
        return 0;
    }

    if (std::ranges::any_of(i3.get_workspaces(), lift::equal(new_val), &i3_containers::workspace::num)) {
        // check if it has nodes
        auto empty = brun::get_workspace_node(i3, new_val)
            .map([](auto && node) { return node.nodes.empty(); } )
            .value_or(true)
            ;
        if (not empty) {
#ifdef ENABLE_DEBUG
            fmt::print(stderr, "Workspace {} already exists - doing nothing...\n");
#endif
            return 0;
        }
    }

    auto const target_output = std::string_view{monitors.at((new_val - 1) / 10)};
#ifdef ENABLE_DEBUG
    fmt::print(stderr, "Moving workspace {} to {} ({})\n", focused, new_val, target_output);
#endif

    i3.execute_commands(fmt::format(
        "rename workspace to {}; move workspace to output {}", new_val, target_output
    ));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    i3.execute_commands(fmt::format("workspace --no-auto-back-and-forth {}", new_val));

    // TODO: history of various outputs
}
