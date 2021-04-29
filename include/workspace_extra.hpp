/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : workspace_extra
 * @created     : Wednesday Apr 28, 2021 23:50:58 CEST
 * @description : Extra functions to work with workspaces
 * */

#ifndef WORKSPACE_EXTRA_HPP
#define WORKSPACE_EXTRA_HPP

#include <algorithm>
#include <fmt/core.h>
#include <i3-ipc++/i3_ipc.hpp>

#include "outputs.hpp"
#include "utils.hpp"

namespace brun
{

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
    for (auto const & monitor : monitors) {
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
                    brun::log("Moved workspace {} to {}\n", current, base + offset);
                    return {base + offset};
                }
            }
            if (base - offset > 0) {
                if (auto found = std::ranges::find(workspaces, base - offset, num);
                    found == std::ranges::end(workspaces))
                {
                    i3.execute_commands(fmt::format("rename workspace to {}", base - offset));
                    brun::log("Moved workspace {} to {}\n", current, base - offset);
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
template <std::ranges::random_access_range Outputs>
bool fix_ws_output(i3_ipc const & i3, int target, Outputs const & output_names)
{
    auto const idx = (target - 1) / 10;
    auto const current_output  = brun::workspace_output(i3, target);
    // auto const computed_output = output_names.at(idx);
    if (auto size = std::ranges::ssize(output_names); idx >= size) {
        brun::log("Error - attempting to access element {} of {} in output_names\n", idx, size);
        return false;
    }
    auto const computed_output = output_names[idx];

    if (current_output != computed_output) {
        brun::log("Moving workspace {} from {} to {}\n", target, current_output, computed_output);
        i3.execute_commands(fmt::format("[workspace=^{}$] move workspace to output {}", target, computed_output));
        return true;
    }
    return false;
}

inline
bool fix_ws_output(i3_ipc const & i3, int current)
{
    auto const outputs = retrieve_output_names(i3);
    return fix_ws_output(i3, current, outputs);
}

} // namespace brun

#endif /* WORKSPACE_EXTRA_HPP */
