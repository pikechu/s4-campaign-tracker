#include "marker/CompletionMarkerRenderer.h"

#include <array>
#include <mutex>
#include <utility>

namespace campaign_completion {

CompletionMarkerRenderer::CompletionMarkerRenderer(
    IMarkerDrawingSurface& surface, LogSink log)
    : surface_(surface), log_(std::move(log)) {}

MarkerRenderStatus CompletionMarkerRenderer::Render(
    LPDIRECTDRAWSURFACE7 destination, INT32 pillarboxWidth,
    const MarkerFrameCommands& frame, std::uint64_t nowMs) noexcept {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        if (disabled_) return MarkerRenderStatus::Disabled;
        if (frame.count == 0u) return MarkerRenderStatus::Skipped;
        if (frame.count > frame.commands.size() || destination == nullptr) {
            return Fail(nowMs);
        }

        MarkerSurfaceExtent extent{};
        if (!surface_.Describe(destination, extent)) return Fail(nowMs);
        std::array<MarkerCheckGeometry, kMaximumVisibleFixedRows> geometry{};
        for (std::size_t index = 0u; index < frame.count; ++index) {
            const auto built = BuildMarkerCheckGeometry(
                frame.commands[index], pillarboxWidth, extent.width,
                extent.height);
            if (!built.has_value()) return Fail(nowMs);
            geometry[index] = *built;
        }
        if (!surface_.Begin(destination)) return Fail(nowMs);
        bool success = true;
        for (std::size_t index = 0u; index < frame.count; ++index) {
            success = surface_.DrawOutlinedCheck(geometry[index]) && success;
        }
        success = surface_.End() && success;
        if (!success) return Fail(nowMs);
        failures_ = 0u;
        return MarkerRenderStatus::Drawn;
    } catch (...) {
        return MarkerRenderStatus::Failed;
    }
}

void CompletionMarkerRenderer::Disable() noexcept {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        disabled_ = true;
    } catch (...) {
    }
}

MarkerRenderStatus CompletionMarkerRenderer::Fail(std::uint64_t nowMs) noexcept {
    ++failures_;
    if (!failureLogged_ || nowMs - lastFailureLogMs_ >= 5000u) {
        SafeLog("completion-marker-renderer frame-failed");
        failureLogged_ = true;
        lastFailureLogMs_ = nowMs;
    }
    if (failures_ >= 3u) {
        disabled_ = true;
        SafeLog("completion-marker-renderer disabled failures=3");
        return MarkerRenderStatus::Disabled;
    }
    return MarkerRenderStatus::Failed;
}

void CompletionMarkerRenderer::SafeLog(std::string line) noexcept {
    try { if (log_) log_(LogLevel::Error, std::move(line)); } catch (...) {}
}

}  // namespace campaign_completion
