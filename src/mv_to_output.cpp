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
#include "workspace_utils.hpp"


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

    if (auto new_current = brun::fix_ws_number(i3, focused, monitors); new_current.has_value()) {
        brun::fix_ws_output(i3, *new_current, monitors);
        return 0;
    }
    if (brun::fix_ws_output(i3, focused, monitors)) {
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
