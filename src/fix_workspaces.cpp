/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : fix_workspaces
 * @created     : Wednesday Apr 27, 2022 20:33:53 CEST
 * @description : move each workspace in an output depending on its index
 */

#include <i3-ipc++/i3_ipc.hpp>
#include <fmt/core.h>

#include "workspaces.hpp"
#include "workspace_extra.hpp"
#include "utils.hpp"

int main()
{
    auto const i3 = i3_ipc{std::getenv("I3SOCK")};

    auto const workspaces = i3.get_workspaces();
    auto const fix_ws = [&i3](auto ws_idx) { brun::fix_ws_output(i3, ws_idx.value()); };
    std::ranges::for_each(workspaces, fix_ws, &i3_containers::workspace::num);
}
