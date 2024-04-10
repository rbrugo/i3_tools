/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : i3_utils
 * @created     : Thursday Apr 08, 2021 22:54:38 CEST
 * @description : Collection of functions to work with workspaces
 * */

#ifndef I3_TOOLS_WORKSPACES_HPP
#define I3_TOOLS_WORKSPACES_HPP

#include <ranges>
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
[[nodiscard]] inline
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
[[nodiscard]] inline
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
[[nodiscard]] inline
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

/**
 * Find the workspace given its node id
 *
 * \param i3 The current i3 instance
 * \param idx The workspace index
 * \returns An optional containing the required workspace
 * */
[[nodiscard]] inline
auto get_workspace_from_node_id(i3_ipc const & i3, uint64_t id)
    -> tl::optional<i3_containers::workspace>
{
    auto const workspaces = i3.get_workspaces();
    for (auto const & ws : workspaces) {
        if (ws.id == id) {
            return ws;
        }
    }
    return tl::nullopt;
}


namespace detail
{
struct recursive_result
{
    bool found;
    std::optional<i3_containers::node> workspace_id;
};

[[nodiscard]] inline
auto find_ws_by_mark_impl(i3_containers::node const & root, std::string_view const mark)
    -> recursive_result
{
    if (std::ranges::find(root.marks, mark) != root.marks.end()) {
        if (root.type == i3_containers::node_type::workspace) {
            return { true, root };
        }
        return { true, std::nullopt };
    }

    for (auto const & node : root.nodes) {
        auto [found, the_node] = find_ws_by_mark_impl(node, mark);
        if (found) {
            if (the_node.has_value()) {
                return { true, std::move(the_node) };
            }
            if (root.type == i3_containers::node_type::workspace) {
                return { true, root };
            }
            return { true, std::nullopt };
        }
    }

    return { false, std::nullopt };
}
}  // namespace detail

[[nodiscard]] inline
auto find_ws_by_mark(i3_containers::node const & root, std::string_view const mark)
    -> tl::optional<i3_containers::node>
{
    auto [found, node] = detail::find_ws_by_mark_impl(root, mark);
    if (found) {
        return { node.value() };
    }
    return tl::nullopt;
}

[[nodiscard]] inline
auto find_ws_by_mark(i3_ipc const & i3, std::string_view const mark)
    -> tl::optional<i3_containers::node>
{
    return find_ws_by_mark(i3.get_tree(), mark);
}

} // namespace brun

#endif /* I3_TOOLS_WORKSPACES_HPP */
