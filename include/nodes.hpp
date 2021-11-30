/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : nodes
 * @created     : Friday Apr 23, 2021 19:17:14 CEST
 * @description : Collection of functions to work with tree and nodes
 * */

#ifndef NODES_HPP
#define NODES_HPP

#include <algorithm>
#include <i3-ipc++/i3_ipc.hpp>
#include <tl/optional.hpp>

#ifdef ENABLE_DEBUG
#include <fmt/core.h>
#endif

namespace brun
{

namespace detail
{
[[nodiscard]]
auto focused_node_impl(i3_containers::node const & node)
    -> tl::optional<i3_containers::node>
{
    if (node.is_focused) {
        return node;
    }

    auto const id_of_focused_child = node.focus.front();
    auto const focused_child = std::ranges::find(node.nodes, id_of_focused_child, &i3_containers::node::id);
    if (focused_child != std::ranges::end(node.nodes)) {
        return focused_node_impl(*focused_child);
    }
    return tl::nullopt;
}
} // namespace detail


/**
 * Search the tree for the focused node
 *
 * \param i3 The i3 instance
 * \returns An optional with the focused node, or an empty optional if it was not found
 */
[[nodiscard]] inline
auto focused_node(i3_ipc const & i3)
{
    return detail::focused_node_impl(i3.get_tree());
}


/**
 * Enum to classify the position of the container on borders
 * */
enum class border {
    no     = 0b0000,
    left   = 0b0001,
    right  = 0b0010,
    top    = 0b0100,
    bottom = 0b1000,
    unique = 0b1111
};

/**
 * Composes two [borders] into one
 *
 * \param a A border
 * \param b Another border
 * \returns The composition of a and b
 * */
[[nodiscard]] constexpr
auto operator|(border a, border b)
{
    using underlying_t = std::underlying_type_t<border>;
    constexpr auto to_underlying = [](border x) { return static_cast<underlying_t>(x); };
    return static_cast<border>(to_underlying(a) | to_underlying(b));
}

/**
 * Check if the passed border is on the requested position
 *
 * \tparam pos The reference position
 * \param target The border to be checked
 * \returns `true` if `target` is on border `pos`, `false` otherwise
 * */
template <border pos>
[[nodiscard]] constexpr
bool is_on_(border target)
{
    using underlying_t = std::underlying_type_t<border>;
    constexpr auto to_underlying = [](border x) { return static_cast<underlying_t>(x); };
    return (to_underlying(target) & to_underlying(pos)) != 0;
}

static_assert(is_on_<border::left>(border::left | border::top));
static_assert(not is_on_<border::right>(border::left | border::top));

#ifdef ENABLE_DEBUG
auto print_border(border x)
{
    // using enum border;
    auto horizontal_centered = is_on_<border::left>(x) and is_on_<border::right>(x);
    auto vertical_centered = is_on_<border::top>(x) and is_on_<border::bottom>(x);
    if (horizontal_centered and vertical_centered) { return "unique"; }
    if (horizontal_centered) {
        if (is_on_<border::top>(x)) {
            return "top";
        }
        if (is_on_<border::bottom>(x)) {
            return "bottom";
        }
        return "h-centered";
    }
    if (vertical_centered) {
        if (is_on_<border::left>(x)) {
            return "left";
        }
        if (is_on_<border::right>(x)) {
            return "right";
        }
        return "v-centered";
    }
    if (is_on_<border::top>(x) and is_on_<border::left>(x)) {
        return "top-left";
    }
    if (is_on_<border::top>(x) and is_on_<border::right>(x)) {
        return "top-right";
    }
    if (is_on_<border::bottom>(x) and is_on_<border::left>(x)) {
        return "bottom-left";
    }
    if (is_on_<border::bottom>(x) and is_on_<border::right>(x)) {
        return "bottom-right";
    }
    switch (x) {
    case border::left: return "left";
    case border::right: return "right";
    case border::top: return "top";
    case border::bottom: return "bottom";
    case border::unique: return "unique";
    case border::no: return "no";
    default: return "???";
    }
}
#endif


namespace detail
{
/// \exclude
[[nodiscard]]
auto node_on_border_impl(i3_containers::node const & node, border on_border)
    -> border
{
    if (node.is_focused) {
        return border::unique;
    }

    if (node.focus.empty()) {
        return border::unique;
    }

    auto const vertical_layout = node.layout == i3_containers::node_layout::splitv
                              or node.layout == i3_containers::node_layout::stacked;

    auto const id_of_focused_child = node.focus.front();
    auto const focused_child = std::ranges::find(node.nodes, id_of_focused_child, &i3_containers::node::id);

    auto const child_on_left  = is_on_<border::left>(on_border)
                            and (vertical_layout or focused_child->id == node.nodes.front().id);
    auto const child_on_right = is_on_<border::right>(on_border)
                            and (vertical_layout or focused_child->id == node.nodes.back().id);
    auto const child_on_top   = is_on_<border::top>(on_border)
                            and (not vertical_layout or focused_child->id == node.nodes.front().id);
    auto const child_on_bot   = is_on_<border::bottom>(on_border)
                            and (not vertical_layout or focused_child->id == node.nodes.back().id);
    auto const child_position = [=] {
        if (focused_child->type != i3_containers::node_type::con) {
            return border::unique;
        }
        if (on_border == border::no) {
            return border::no;
        }
        return (child_on_left  ? border::left   : border::no)
             | (child_on_right ? border::right  : border::no)
             | (child_on_top   ? border::top    : border::no)
             | (child_on_bot   ? border::bottom : border::no)
             ;
    }();
    if (focused_child->is_focused) {
#ifdef ENABLE_DEBUG
        fmt::print("Of {} childs, one is focused:\n", node.nodes.size());
        fmt::print("This container is on border: {}\n", print_border(on_border));
        fmt::print("This container has vertical layout: {}\n", vertical_layout);
        fmt::print("Child is on left: {}\n", child_on_left);
        fmt::print("Child is on right: {}\n", child_on_right);
        fmt::print("Child is on top: {}\n", child_on_top);
        fmt::print("Child is on bottom: {}\n", child_on_bot);
#endif
        return child_position;
    }
    return node_on_border_impl(*focused_child, child_position);
}
} // namespace detail

/**
 * Retrieves the border in which the focused node is located
 *
 * \param node The root of the tree where you are searching for the focused node
 * \returns The `border` representing the position of the focused node
 * */
[[nodiscard]] inline
auto node_on_border(i3_ipc const & i3)
{
    return detail::node_on_border_impl(i3.get_tree(), border::unique);
}


} // namespace brun

#endif /* NODES_HPP */

