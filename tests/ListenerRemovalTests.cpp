#include "runtime/ListenerRemoval.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace {

void Require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

}  // namespace

int RunListenerRemovalTests() {
    std::vector<S4HOOK> hooks{10, 20, 30};
    std::vector<S4HOOK> attempts;
    const auto success = campaign_completion::RemoveListenersInReverse(
        hooks, [&attempts](S4HOOK hook) {
            attempts.push_back(hook);
            return S_OK;
        });
    Require(attempts == std::vector<S4HOOK>({30, 20, 10}),
            "listeners must be removed in reverse registration order");
    Require(success.registered == 3 && success.removed == 3 &&
                success.failures == 0,
            "successful removal counts must be exact");
    Require(hooks.empty(), "successful removal must clear stored handles");

    hooks = {10, 20, 30};
    attempts.clear();
    const auto partial = campaign_completion::RemoveListenersInReverse(
        hooks, [&attempts](S4HOOK hook) {
            attempts.push_back(hook);
            return hook == 20 ? E_FAIL : S_OK;
        });
    Require(attempts == std::vector<S4HOOK>({30, 20, 10}),
            "one failure must not stop remaining removal attempts");
    Require(partial.registered == 3 && partial.removed == 2 &&
                partial.failures == 1,
            "partial removal counts must be exact");
    Require(hooks.empty(), "partial removal must still clear stored handles");
    return 0;
}
