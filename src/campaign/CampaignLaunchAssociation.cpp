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

}  // namespace

void CampaignLaunchAssociation::ObservePage(
    bool darkTribeActive) noexcept {
    if (!enabled_) return;
    pageActive_ = darkTribeActive;
    if (!pageActive_) {
        hasSnapshot_ = false;
        snapshot_ = {};
        if (sessionId_ == 0u) ClearPending();
    }
}

void CampaignLaunchAssociation::ObserveSnapshot(
    const CampaignMenuSnapshot& snapshot) noexcept {
    if (!enabled_ || !pageActive_ ||
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
    if (!enabled_ || !pageActive_ || !hasSnapshot_ ||
        message != WM_LBUTTONUP || element == nullptr) {
        return false;
    }
    std::size_t matches = 0u;
    CampaignControlIdentity candidate{};
    for (std::size_t index = 0u; index < snapshot_.count; ++index) {
        const auto& feature = snapshot_.features[index];
        if (!Matches(feature, *element)) continue;
        ++matches;
        candidate = {element->id, element->x, element->y,
                     element->w, element->h};
    }
    if (matches != 1u) {
        ClearPending();
        return false;
    }
    pendingControl_ = candidate;
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
    if (!enabled_ || !hasPending_ || sessionId_ == 0u ||
        identity.sessionId != sessionId_ || identity.relative.empty()) {
        return std::nullopt;
    }
    CampaignMenuAssociation result{};
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
    hasSnapshot_ = false;
    snapshot_ = {};
    ClearPending();
}

void CampaignLaunchAssociation::ClearPending() noexcept {
    pendingControl_ = {};
    clickedAtMs_ = 0u;
    sessionId_ = 0u;
    hasPending_ = false;
}

}  // namespace campaign_completion
