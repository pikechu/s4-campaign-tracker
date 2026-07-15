#include "marker/DirectDrawMarkerSurface.h"

namespace campaign_completion {

DirectDrawMarkerSurface::~DirectDrawMarkerSurface() { Shutdown(); }

bool DirectDrawMarkerSurface::Initialize() noexcept {
    try {
        if (darkPen_ != nullptr && greenPen_ != nullptr) return true;
        darkPen_ = CreatePen(PS_SOLID, 3, RGB(24, 40, 24));
        if (darkPen_ == nullptr) return false;
        greenPen_ = CreatePen(PS_SOLID, 1, RGB(48, 208, 80));
        if (greenPen_ == nullptr) { DeleteObject(darkPen_); darkPen_ = nullptr; return false; }
        return true;
    } catch (...) { return false; }
}

void DirectDrawMarkerSurface::Shutdown() noexcept {
    try {
        if (dc_ != nullptr) End();
        if (greenPen_ != nullptr) { DeleteObject(greenPen_); greenPen_ = nullptr; }
        if (darkPen_ != nullptr) { DeleteObject(darkPen_); darkPen_ = nullptr; }
    } catch (...) {}
}

bool DirectDrawMarkerSurface::Describe(LPDIRECTDRAWSURFACE7 surface,
                                       MarkerSurfaceExtent& extent) noexcept {
    try {
        if (surface == nullptr) return false;
        DDSURFACEDESC2 description{};
        description.dwSize = sizeof(description);
        if (FAILED(surface->GetSurfaceDesc(&description))) return false;
        extent = {description.dwWidth, description.dwHeight};
        return extent.width != 0u && extent.height != 0u;
    } catch (...) { return false; }
}

bool DirectDrawMarkerSurface::Begin(LPDIRECTDRAWSURFACE7 surface) noexcept {
    try {
        if (surface == nullptr || dc_ != nullptr || darkPen_ == nullptr ||
            greenPen_ == nullptr) return false;
        HDC dc = nullptr;
        if (FAILED(surface->GetDC(&dc)) || dc == nullptr) return false;
        activeSurface_ = surface;
        dc_ = dc;
        return true;
    } catch (...) { return false; }
}

bool DirectDrawMarkerSurface::DrawOutlinedCheck(
    const MarkerCheckGeometry& geometry) noexcept {
    try {
        if (dc_ == nullptr) return false;
        const int saved = SaveDC(dc_);
        if (saved == 0) return false;
        bool success = IntersectClipRect(dc_, geometry.clip.left, geometry.clip.top,
                                         geometry.clip.right, geometry.clip.bottom) != ERROR;
        if (success) {
            success = SelectObject(dc_, darkPen_) != nullptr &&
                      Polyline(dc_, geometry.points.data(),
                               static_cast<int>(geometry.points.size())) != FALSE;
            success = SelectObject(dc_, greenPen_) != nullptr &&
                      Polyline(dc_, geometry.points.data(),
                               static_cast<int>(geometry.points.size())) != FALSE && success;
        }
        return RestoreDC(dc_, saved) != FALSE && success;
    } catch (...) { return false; }
}

bool DirectDrawMarkerSurface::End() noexcept {
    try {
        if (dc_ == nullptr || activeSurface_ == nullptr) return false;
        auto* surface = activeSurface_;
        HDC dc = dc_;
        activeSurface_ = nullptr;
        dc_ = nullptr;
        return SUCCEEDED(surface->ReleaseDC(dc));
    } catch (...) { activeSurface_ = nullptr; dc_ = nullptr; return false; }
}

}  // namespace campaign_completion
