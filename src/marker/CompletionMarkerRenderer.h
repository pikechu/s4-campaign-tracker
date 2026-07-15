#pragma once

#include "diagnostics/Logger.h"
#include "marker/CompletionMarkerGeometry.h"

#include <ddraw.h>

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>

namespace campaign_completion {

struct MarkerSurfaceExtent final { DWORD width = 0u; DWORD height = 0u; };

class IMarkerDrawingSurface {
public:
    virtual ~IMarkerDrawingSurface() = default;
    virtual bool Describe(LPDIRECTDRAWSURFACE7 surface,
                          MarkerSurfaceExtent& extent) noexcept = 0;
    virtual bool Begin(LPDIRECTDRAWSURFACE7 surface) noexcept = 0;
    virtual bool DrawOutlinedCheck(const MarkerCheckGeometry& geometry) noexcept = 0;
    virtual bool End() noexcept = 0;
};

enum class MarkerRenderStatus { Skipped, Drawn, Failed, Disabled };

class CompletionMarkerRenderer final {
public:
    using LogSink = std::function<void(LogLevel, std::string)>;
    CompletionMarkerRenderer(IMarkerDrawingSurface& surface, LogSink log);
    MarkerRenderStatus Render(LPDIRECTDRAWSURFACE7 destination,
                              INT32 pillarboxWidth,
                              const MarkerFrameCommands& frame,
                              std::uint64_t nowMs) noexcept;
    void Disable() noexcept;

private:
    MarkerRenderStatus Fail(std::uint64_t nowMs) noexcept;
    void SafeLog(std::string line) noexcept;
    IMarkerDrawingSurface& surface_;
    LogSink log_;
    std::mutex mutex_;
    std::uint32_t failures_ = 0u;
    std::uint64_t lastFailureLogMs_ = 0u;
    bool failureLogged_ = false;
    bool disabled_ = false;
};

}  // namespace campaign_completion
