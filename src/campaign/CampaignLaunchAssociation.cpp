#include "campaign/CampaignLaunchAssociation.h"

#include <windows.h>

namespace campaign_completion {
namespace {

bool Matches(const CampaignMenuFeature& feature,
             const S4UiElement& element) noexcept {
    return feature.valueLink == element.id && feature.x == element.x &&
           feature.y == element.y && feature.width == element.w &&
           feature.height == element.h;
}

bool IsMissionCandidate(const CampaignMenuFeature& feature) noexcept {
    return feature.hasText && feature.text.length != 0u;
}

}  // namespace

void CampaignLaunchAssociation::ObservePage(DWORD page) noexcept {
    if (!enabled_) return;
    const bool campaignPageActive = IsCampaignCatalogPage(page);
    if (campaignPageActive && page != activePage_ && hasPending_ &&
        sessionId_ == 0u) {
        ClearPending();
    }
    pageActive_ = campaignPageActive;
    activePage_ = pageActive_ ? page : S4_GUI_UNKNOWN;
    if (!pageActive_) {
        hasSnapshot_ = false;
        snapshot_ = {};
    }
}

void CampaignLaunchAssociation::ObserveSnapshot(
    const CampaignMenuSnapshot& snapshot) noexcept {
    if (!enabled_ || !pageActive_ || snapshot.page != activePage_ ||
        snapshot.status != CampaignMenuSnapshotStatus::Success) {
        hasSnapshot_ = false;
        snapshot_ = {};
        return;
    }
    snapshot_ = snapshot;
    hasSnapshot_ = true;
}

bool CampaignLaunchAssociation::ObserveClick(
    DWORD message, const S4UiElement* element,
    std::uint64_t nowMs) noexcept {
    if (!enabled_ || !pageActive_ || message != WM_LBUTTONUP ||
        element == nullptr) {
        return false;
    }
    ClearPending();
    if (!hasSnapshot_) return false;
    std::size_t matches = 0u;
    CampaignControlIdentity candidate{};
    for (std::size_t index = 0u; index < snapshot_.count; ++index) {
        const auto& feature = snapshot_.features[index];
        if (!Matches(feature, *element) || !IsMissionCandidate(feature)) {
            continue;
        }
        ++matches;
        candidate = {element->id, element->x, element->y,
                     element->w, element->h};
    }
    if (matches != 1u) {
        ClearPending();
        return false;
    }
    pendingControl_ = candidate;
    pendingPage_ = snapshot_.page;
    clickedAtMs_ = nowMs;
    sessionId_ = 0u;
    hasPending_ = true;
    return true;
}

bool CampaignLaunchAssociation::BeginSession(
    std::uint64_t sessionId, LaunchOriginSnapshot origin,
    std::uint64_t nowMs) noexcept {
    Expire(nowMs);
    if (!enabled_ || !hasPending_ || sessionId == 0u ||
        origin.source != LaunchSource::Campaign ||
        origin.eligibility != SessionEligibility::Eligible) {
        ClearPending();
        return false;
    }
    sessionId_ = sessionId;
    return true;
}

std::optional<CampaignMenuAssociation>
CampaignLaunchAssociation::ObserveIdentity(
    const ConfirmedMapIdentity& identity,
    std::uint64_t nowMs) noexcept {
    Expire(nowMs);
    if (!enabled_ || !hasPending_ || sessionId_ == 0u) {
        return std::nullopt;
    }
    if (identity.sessionId != sessionId_ || identity.relative.empty()) {
        ClearPending();
        return std::nullopt;
    }
    CampaignMenuAssociation result{};
    result.page = pendingPage_;
    result.control = pendingControl_;
    result.sessionId = sessionId_;
    try {
        result.relative = identity.relative;
    } catch (...) {
        ClearPending();
        return std::nullopt;
    }
    ClearPending();
    return result;
}

void CampaignLaunchAssociation::Expire(std::uint64_t nowMs) noexcept {
    if (!hasPending_) return;
    if (nowMs < clickedAtMs_ ||
        nowMs - clickedAtMs_ > kCampaignLaunchAssociationLeaseMs) {
        ClearPending();
    }
}

void CampaignLaunchAssociation::Disable() noexcept {
    enabled_ = false;
    pageActive_ = false;
    activePage_ = S4_GUI_UNKNOWN;
    hasSnapshot_ = false;
    snapshot_ = {};
    ClearPending();
}

void CampaignLaunchAssociation::ClearPending() noexcept {
    pendingControl_ = {};
    pendingPage_ = S4_GUI_UNKNOWN;
    clickedAtMs_ = 0u;
    sessionId_ = 0u;
    hasPending_ = false;
}

}  // namespace campaign_completion
