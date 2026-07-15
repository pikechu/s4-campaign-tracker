#pragma once

#include "marker/CompletionMarkerRenderer.h"

namespace campaign_completion {

class DirectDrawMarkerSurface final : public IMarkerDrawingSurface {
public:
    ~DirectDrawMarkerSurface();
    bool Initialize() noexcept;
    void Shutdown() noexcept;
    bool Describe(LPDIRECTDRAWSURFACE7 surface,
                  MarkerSurfaceExtent& extent) noexcept override;
    bool Begin(LPDIRECTDRAWSURFACE7 surface) noexcept override;
    bool DrawOutlinedCheck(const MarkerCheckGeometry& geometry) noexcept override;
    bool End() noexcept override;

private:
    LPDIRECTDRAWSURFACE7 activeSurface_ = nullptr;
    HDC dc_ = nullptr;
    HPEN darkPen_ = nullptr;
    HPEN greenPen_ = nullptr;
};

}  // namespace campaign_completion
