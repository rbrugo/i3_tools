/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : outputs
 * @created     : Friday Apr 23, 2021 19:44:00 CEST
 * @license     : MIT
 * */

#ifndef OUTPUTS_HPP
#define OUTPUTS_HPP

#include <ranges>
#include <algorithm>
#include <tl/optional.hpp>
#include <i3-ipc++/i3_ipc.hpp>

#include "detail/lippincott.hpp"

namespace brun
{

/**
 * Generates a list containing the active outputs
 *
 * \param i3 The current i3 instance
 * \returns The list of all the active outputs
 * */
[[nodiscard]]
auto retrieve_output_list(i3_ipc const & i3)
    -> std::vector<i3_containers::output>
{
    auto outputs = i3.get_outputs();
    std::ranges::sort(outputs, std::ranges::less{}, [](auto const & o) { return o.rect.x; });
    auto active = outputs
                | std::views::filter(&i3_containers::output::is_active)
                ;
    return std::vector<i3_containers::output>(
        std::make_move_iterator(std::ranges::begin(active)),
        std::make_move_iterator(std::ranges::end(active))
    );
}


/**
 * Generates the list of the active outputs names
 *
 * \param i3 The current i3 instance
 * \returns The list of all the active outputs' names
 * */
[[nodiscard]]
auto retrieve_output_names(i3_ipc const & i3)
    -> std::vector<std::string>
{
    auto outputs = i3.get_outputs();
    std::ranges::sort(outputs, std::ranges::less{}, [](auto const & o) { return o.rect.x; });
    auto names = outputs
               | std::views::filter(&i3_containers::output::is_active)
               | std::views::transform(&i3_containers::output::name)
               ;
    return std::vector<std::string>(std::ranges::begin(names), std::ranges::end(names));
}


/**
 * Given the number of a workspace, returns its output
 *
 * \param i3 The current i3 instance
 * \param n The `num` of the workspace
 * \returns An optional containing the workspace's output, or an empty optional if it was not found
 * */
auto workspace_output(i3_ipc const & i3, int n)
    -> std::string
{
    auto const workspaces = i3.get_workspaces();
    auto const found = std::ranges::find(workspaces, n, &i3_containers::workspace::num);
    if (found != std::ranges::end(workspaces)) {
        return found->output;
    }
    return "";
}


/**
 * Search for the focused output
 *
 * \param i3 The current i3 instance
 * \returns An optional with the focused output, or an empty optional if it was not found
 * */
auto focused_output(i3_ipc const & i3)
    -> tl::optional<std::string>
try {
    auto const workspaces = i3.get_workspaces();
    auto const found = std::ranges::find_if(workspaces, &i3_containers::workspace::is_focused);
    if (found != std::ranges::end(workspaces)) {
        return found->output;
    }
    return tl::nullopt;
}
catch (std::exception const & exc) {
    detail::lippincott();
}

} // namespace brun

#endif /* OUTPUTS_HPP */

