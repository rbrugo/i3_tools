/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : mv_container
 * @created     : Wednesday Apr 28, 2021 22:33:01 CEST
 * @description : moves a container in the requested workspace, eventually creating it on the right output
 */

#include <i3-ipc++/i3_ipc.hpp>
#include <fmt/core.h>
#include <charconv>
#include <thread>

#include "workspaces.hpp"
#include "workspace_extra.hpp"
#include "utils.hpp"

// target  <- get target workspace
// current <- get current workspace
// if current == target:
//     target <- compute the target as for back-and-forth
//     if current == target:
//         do nothing and return
// move the container to target workspace
// if target has exactly one child (= a new workspace has been created):
//     fix target output

int main(int argc, char * argv[])
{
    if (argc < 2 or argc > 3) {
        fmt::print(stderr, "usage: {} <target-workspace-num> [--no-auto-back-and-forth]", argv[0]);
        return 0;
    }
    auto const i3 = i3_ipc{std::getenv("I3SOCK")};

    auto target = brun::stoi(std::string_view{argv[1]}).or_else([] {
        brun::log("Argument passed is not a number\n");
        std::exit(0);
    }).value();
    auto const current = brun::focused_workspace_idx(i3).or_else([]{
        brun::log("No workspace focused\n");
        std::exit(0);
    }).value();

    auto const back_and_forth = [argc, argv] {
        if (argc == 2) { return true; }
        auto const arg = std::string_view{argv[2]};
        if (arg != "--no-auto-back-and-forth") {
            brun::log("Unknown option {} - ignoring", arg);
            return true;
        }
        return false;
    }();

    if (current == target and back_and_forth) {
        brun::log("Target is the same as current ({}) - trying back-and-forth\n", target);
        // get the correct target as for back-and-forth
        i3.execute_commands(fmt::format("workspace {}", current));
        target = brun::focused_workspace_idx(i3).value();
        i3.execute_commands(fmt::format("workspace --no-auto-back-and-forth {}", current));
    }
    if (current == target) {
        brun::log("Target is the same as current ({}) - doing nothing\n", target);
        return 0;
    }

    auto const target_ws = [&i3, target] {
        auto const workspaces = i3.get_workspaces();
        auto found = std::ranges::find(workspaces, target, &i3_containers::workspace::num);
        return found != std::ranges::end(workspaces) ? tl::optional{*found} : tl::nullopt;
    }();

    auto const new_workspace = target_ws
        .transform([](auto && ws) { return ws.id; })
        .and_then(std::bind_front(brun::get_workspace_node, std::ref(i3)))
        .transform([](auto && node) { return node.nodes.empty() and node.floating_nodes.empty(); })
        .value_or(true)
        ;

    i3.execute_commands(fmt::format("move container to workspace {}", target));

    // Eventually move the new workspace to the right focus
    if (new_workspace) {
        brun::fix_ws_output(i3, target);
    }
}
