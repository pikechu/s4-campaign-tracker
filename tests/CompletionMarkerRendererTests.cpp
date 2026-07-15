#include "marker/CompletionMarkerRenderer.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

void Require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class FakeSurface final : public campaign_completion::IMarkerDrawingSurface {
public:
    bool Describe(LPDIRECTDRAWSURFACE7,
                  campaign_completion::MarkerSurfaceExtent& output) noexcept
        override {
        ++describeCalls;
        output = extent;
        return describeResult;
    }

    bool Begin(LPDIRECTDRAWSURFACE7) noexcept override {
        ++beginCalls;
        return beginResult;
    }

    bool DrawOutlinedCheck(
        const campaign_completion::MarkerCheckGeometry&) noexcept override {
        ++drawCalls;
        return drawResult;
    }

    bool End() noexcept override {
        ++endCalls;
        return endResult;
    }

    campaign_completion::MarkerSurfaceExtent extent{800u, 600u};
    bool describeResult = true;
    bool beginResult = true;
    bool drawResult = true;
    bool endResult = true;
    int describeCalls = 0;
    int beginCalls = 0;
    int drawCalls = 0;
    int endCalls = 0;
};

campaign_completion::MarkerDrawCommand Row(std::size_t slot) {
    campaign_completion::MarkerDrawCommand row{};
    row.logicalSurfaceWidth = 800u;
    row.logicalSurfaceHeight = 600u;
    row.x = 115u;
    row.y = static_cast<WORD>(142u + 31u * slot);
    row.width = 271u;
    row.height = 30u;
    return row;
}

campaign_completion::MarkerFrameCommands Frame(std::size_t count) {
    campaign_completion::MarkerFrameCommands frame{};
    frame.count = count;
    for (std::size_t index = 0u; index < count; ++index) {
        frame.commands[index] = Row(index);
    }
    return frame;
}

}  // namespace

int RunCompletionMarkerRendererTests() {
    using namespace campaign_completion;
    auto* destination = reinterpret_cast<LPDIRECTDRAWSURFACE7>(1u);

    {
        FakeSurface surface;
        std::vector<std::pair<LogLevel, std::string>> logs;
        CompletionMarkerRenderer renderer(
            surface, [&logs](LogLevel level, std::string line) {
                logs.emplace_back(level, std::move(line));
            });
        Require(renderer.Render(destination, 0, Frame(0u), 0u) ==
                    MarkerRenderStatus::Skipped &&
                    surface.describeCalls == 0 && surface.beginCalls == 0 &&
                    surface.drawCalls == 0 && surface.endCalls == 0 &&
                    logs.empty(),
                "an empty frame makes no backend call and emits no log");

        Require(renderer.Render(destination, 0, Frame(2u), 1u) ==
                    MarkerRenderStatus::Drawn &&
                    surface.describeCalls == 1 && surface.beginCalls == 1 &&
                    surface.drawCalls == 2 && surface.endCalls == 1 &&
                    logs.empty(),
                "two checks share one describe, begin, and end pass");
    }

    {
        FakeSurface surface;
        CompletionMarkerRenderer renderer(surface, {});
        auto invalid = Frame(2u);
        invalid.commands[1].logicalSurfaceWidth = 100u;
        Require(renderer.Render(destination, 0, invalid, 0u) ==
                    MarkerRenderStatus::Failed &&
                    surface.describeCalls == 1 && surface.beginCalls == 0,
                "invalid geometry drops the whole frame before Begin");
    }

    {
        FakeSurface surface;
        surface.drawResult = false;
        CompletionMarkerRenderer renderer(surface, {});
        Require(renderer.Render(destination, 0, Frame(1u), 0u) ==
                    MarkerRenderStatus::Failed &&
                    surface.drawCalls == 1 && surface.endCalls == 1,
                "a draw failure still closes the backend pass");
        surface.drawResult = true;
        Require(renderer.Render(destination, 0, Frame(1u), 1u) ==
                    MarkerRenderStatus::Drawn,
                "a later successful frame resets consecutive failures");
        surface.beginResult = false;
        Require(renderer.Render(destination, 0, Frame(1u), 2u) ==
                    MarkerRenderStatus::Failed &&
                    renderer.Render(destination, 0, Frame(1u), 3u) ==
                        MarkerRenderStatus::Failed,
                "two failures after a success do not disable rendering");
    }

    {
        FakeSurface surface;
        surface.beginResult = false;
        std::vector<std::pair<LogLevel, std::string>> logs;
        CompletionMarkerRenderer renderer(
            surface, [&logs](LogLevel level, std::string line) {
                logs.emplace_back(level, std::move(line));
            });
        Require(renderer.Render(destination, 0, Frame(1u), 0u) ==
                    MarkerRenderStatus::Failed &&
                    renderer.Render(destination, 0, Frame(1u), 1000u) ==
                        MarkerRenderStatus::Failed &&
                    renderer.Render(destination, 0, Frame(1u), 6000u) ==
                        MarkerRenderStatus::Disabled,
                "three consecutive backend failures disable rendering");
        const int callsAtDisable = surface.describeCalls;
        Require(renderer.Render(destination, 0, Frame(1u), 7000u) ==
                    MarkerRenderStatus::Disabled &&
                    surface.describeCalls == callsAtDisable,
                "a disabled renderer makes no later backend calls");

        std::size_t transient = 0u;
        std::size_t terminal = 0u;
        for (const auto& log : logs) {
            transient += log.second ==
                                 "completion-marker-renderer frame-failed"
                             ? 1u
                             : 0u;
            terminal += log.second ==
                                "completion-marker-renderer disabled failures=3"
                            ? 1u
                            : 0u;
        }
        Require(transient == 2u && terminal == 1u,
                "transient logs are rate-limited and terminal disable logs once");
    }

    return 0;
}
