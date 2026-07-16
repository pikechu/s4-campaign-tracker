#pragma once

#include "S4ModApi.h"
#include "marker/BoundedMenuText.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace campaign_completion {

inline constexpr DWORD kDarkTribeCampaignPage =
    S4_SCREEN_SINGLEPLAYER_DARKTRIBE;
inline constexpr std::size_t kMaximumCampaignMenuFeatures = 128u;

struct CampaignMenuFeature final {
    DWORD surfaceWidth = 0u;
    DWORD surfaceHeight = 0u;
    WORD gfxCollection = 0u;
    WORD containerType = 0u;
    WORD x = 0u;
    WORD y = 0u;
    WORD width = 0u;
    WORD height = 0u;
    WORD mainTexture = 0u;
    WORD valueLink = 0u;
    WORD buttonPressedTexture = 0u;
    WORD tooltipLink = 0u;
    WORD tooltipLinkExtra = 0u;
    std::uint16_t imageStyle = 0u;
    std::uint16_t effects = 0u;
    std::uint16_t textStyle = 0u;
    WORD showTexture = 0u;
    WORD backTexture = 0u;
    BoundedMenuText text{};
    bool hasText = false;
};

enum class CampaignMenuSnapshotStatus {
    Success,
    Empty,
    Invalid,
};

struct CampaignMenuSnapshot final {
    CampaignMenuSnapshotStatus status = CampaignMenuSnapshotStatus::Empty;
    std::array<CampaignMenuFeature, kMaximumCampaignMenuFeatures> features{};
    std::size_t count = 0u;
    std::uint64_t generation = 0u;
};

bool CopyCampaignMenuFeature(LPS4GUIDRAWBLTPARAMS source,
                             CampaignMenuFeature& output) noexcept;
bool EqualCampaignMenuFeature(const CampaignMenuFeature& left,
                              const CampaignMenuFeature& right) noexcept;
bool EqualCampaignMenuSnapshot(const CampaignMenuSnapshot& left,
                               const CampaignMenuSnapshot& right) noexcept;

class CampaignMenuCapture final {
public:
    std::optional<CampaignMenuSnapshot> ObserveFrame(
        DWORD page, bool darkTribeActive) noexcept;
    bool ObserveFeature(const CampaignMenuFeature& feature) noexcept;
    void Invalidate() noexcept;
    void Disable() noexcept;
    bool Active() const noexcept { return enabled_ && collecting_; }

private:
    void ClearWorking() noexcept;

    std::array<CampaignMenuFeature, kMaximumCampaignMenuFeatures> working_{};
    std::size_t count_ = 0u;
    std::uint64_t generation_ = 0u;
    bool collecting_ = false;
    bool invalid_ = false;
    bool enabled_ = true;
};

}  // namespace campaign_completion
