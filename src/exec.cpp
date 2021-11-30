/**
 * @author      : Riccardo Brugo (brugo.riccardo@gmail.com)
 * @file        : exec
 * @created     : Saturday Nov 27, 2021 11:43:56 CET
 * @description : executes a command splitting the screen along the widest direction
 */


#include <i3-ipc++/i3_ipc.hpp>
#include <fmt/format.h>
#include <thread>
#include <span>

#include "dry-comparisons.hpp"

#include "nodes.hpp"
#include "workspaces.hpp"
#include "outputs.hpp"
#include "format.h"

template <typename Fn>
class scope_exit
{
private:
    std::optional<Fn> _fn;

public:
    scope_exit(Fn && fn) : _fn{std::move(fn)} {}

    [[nodiscard]] bool enabled() const { return _fn.has_value(); }
    void disable() { _fn.clear(); }

    ~scope_exit() noexcept { (*_fn)(); }
};

int main(int argc, char const * argv[])
{
    auto const args = argc > 1
                    ? fmt::to_string(fmt::join(argv + 1, argv + argc, " "))
                    : std::string{"i3-sensible-terminal"};

    auto i3 = i3_ipc{std::getenv("I3SOCK")};
    auto focused_node = brun::focused_node(i3);
    auto const original_ws = brun::focused_workspace(i3);

    auto const [x, y, w, h] = focused_node.value().rect;
#ifdef ENABLE_DEBUG
    fmt::print("Current window xywh: {} {} {} {}\n", x, y, w, h);
#endif // ENABLE_DEBUG

    using i3_containers::node_layout;
    auto const original_layout = focused_node.value().layout;
    if (original_layout == rollbear::none_of(node_layout::splith, node_layout::splitv)) {
#ifdef ENABLE_DEBUG
        fmt::print(stderr, "Don't want to split a stacked/tabbed/dockarea/output container\n");
#endif // ENABLE_DEBUG
        return 0;
    }
    auto const new_layout = w >= h
                          ? node_layout::splith
                          : node_layout::splitv;
#ifdef ENABLE_DEBUG
    fmt::print(stderr, "Splitting {}ly\n", new_layout);
#endif // ENABLE_DEBUG
    auto done = false;
    auto at_exit = scope_exit{[new_layout, original_layout, &i3, &done] {
        if (new_layout != original_layout and not done) {
            i3.execute_commands(fmt::format("split {}", original_layout));
        }
    }};
    i3.execute_commands(fmt::format("split {}; exec {}", new_layout, args));
    i3.on_window_event([&i3, &done, original_layout, original_ws](auto && window) {
        if (window.change == i3_containers::window_change::create) {
            // if is in another ws, move it to the old one
            auto const current_ws = brun::focused_workspace(i3);
            if (original_ws.has_value() and current_ws.has_value() and current_ws->id != original_ws->id) {
#ifdef ENABLE_DEBUG
                fmt::print("Moving new window (id {}) to the original ws\n", window.container.id);
#endif // ENABLE_DEBUG
                i3.execute_commands(fmt::format(
                        "[con_id={}] move to workspace {}",
                        window.container.id,
                        original_ws->name
                ));
            }
            i3.execute_commands(fmt::format("split {}", original_layout));
            done = true;
        }
    });

    auto const start_time = std::chrono::steady_clock::now();
    while (not done) {
        i3.handle_next_event();
        auto now = std::chrono::steady_clock::now();
        if (now - start_time > std::chrono::seconds{7}) {
            break;
        }
    }
}
