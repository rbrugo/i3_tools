/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : format
 * @created     : Saturday Nov 27, 2021 12:15:56 CET
 */

#ifndef FORMAT_H
#define FORMAT_H

#include <i3-ipc++/i3_containers.hpp>
#include <fmt/core.h>

template <>
struct fmt::formatter<i3_containers::node_layout>
{
    constexpr auto parse(format_parse_context & ctx)
    { return ctx.end(); }
    auto format(i3_containers::node_layout layout, auto & ctx)
    {
        switch(layout) {
        case i3_containers::node_layout::splith:   return format_to(ctx.out(), "horizontal");
        case i3_containers::node_layout::splitv:   return format_to(ctx.out(), "vertical");
        case i3_containers::node_layout::stacked:  return format_to(ctx.out(), "stacked");
        case i3_containers::node_layout::tabbed:   return format_to(ctx.out(), "tabbed");
        case i3_containers::node_layout::dockarea: return format_to(ctx.out(), "dockarea");
        case i3_containers::node_layout::output:   return format_to(ctx.out(), "output");
        default: return format_to(ctx.out(), "???");
        }
    }
};

#endif /* end of include guard FORMAT_H */

