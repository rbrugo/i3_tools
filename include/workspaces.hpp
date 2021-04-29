/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : i3_utils
 * @created     : Thursday Apr 08, 2021 22:54:38 CEST
 * @description : Collection of functions to work with workspaces
 * */

#ifndef I3_UTILS_HPP
#define I3_UTILS_HPP

#include <algorithm>
#include <tl/optional.hpp>
#include <i3-ipc++/i3_ipc.hpp>

#include "detail/lippincott.hpp"

namespace brun
{

/**
 * Search for the focused workspace
 *
 * \param i3 The current i3 instance
 * \returns An optional containing the focused workspace, or an empty optional if it was not found
 * */
[[nodiscard]]
auto focused_workspace(i3_ipc const & i3)
    -> tl::optional<i3_containers::workspace>
try {
    auto const workspaces = i3.get_workspaces();
    auto const found = std::ranges::find_if(workspaces, &i3_containers::workspace::is_focused);
    if (found != std::ranges::end(workspaces)) {
        return *found;
    }
    return tl::nullopt;
}
catch (std::exception const & exc) {
    detail::lippincott();
}

/**
 * Returns the currently focused workspace index
 *
 * \param i3 The current i3 instance
 * \returns An optional containing the focused workspace's index, or an empty optional if it
 *          was not found
 * */
[[nodiscard]] inline
auto focused_workspace_idx(i3_ipc const & i3)
    -> tl::optional<int>
try {
    return focused_workspace(i3).and_then([](auto ws) {
        return ws.num.has_value() ? tl::optional{*ws.num} : tl::nullopt;
    });
}
catch (std::exception const & exc) {
    detail::lippincott();
}


/**
 * Returns the first visible but unfocused workspace
 *
 * Note: this function currently only supports two monitors
 * \param i3 The current i3 instance
 * \returns An optional containing the visible but unfocused workspace, or an empty optional if it
 *          was found
 * */
[[nodiscard]]
auto other_workspace(i3_ipc const & i3)
    -> tl::optional<i3_containers::workspace>
try {
    auto const workspaces = i3.get_workspaces();
    auto condition = [](auto && ws) {
        return ws.is_visible and not ws.is_focused;
    };
    auto const found = std::ranges::find_if(workspaces, condition);
    if (found != std::ranges::end(workspaces)) {
        return *found;
    }
    return tl::nullopt;
}
catch (std::exception const & exc) {
    detail::lippincott();
}

/**
 * Returns the index of the first visible but unfocused workspace
 *
 * Note: this function currently only supports two monitors
 * \param i3 The current i3 instance
 * \returns An optional containing the visible but unfocused workspace's index, or an empty optional
 *          if it was not found
 * */
[[nodiscard]] inline
auto other_workspace_idx(i3_ipc const & i3)
    -> tl::optional<int>
try {
    return other_workspace(i3).and_then([](auto ws) {
        return ws.num.has_value() ? tl::optional{*ws.num} : tl::nullopt;
    });
}
catch (std::exception const & exc) {
    detail::lippincott();
}

/**
 * Returns the node of the tree representing the requested workspace
 *
 * \param i3 The current i3 instance
 * \param idx The workspace index
 * \returns An optional containing the node of the requested workspace
 * */
[[nodiscard]]
auto get_workspace_node(i3_ipc const & i3, uint64_t id)
    -> tl::optional<i3_containers::node>
{
    auto nodes = std::queue<i3_containers::node>{};
    nodes.push(i3.get_tree());

    while (not nodes.empty()) {
        auto node = std::move(nodes.front());
        nodes.pop();

        if (node.type == i3_containers::node_type::workspace) {
            if (node.id == id) {
                return node;
            }
            continue;
        }
        for (auto const & subnode : node.nodes) {
            nodes.push(subnode);
        }
    }
    return tl::nullopt;
}
} // namespace brun

#endif /* I3_UTILS_HPP */

