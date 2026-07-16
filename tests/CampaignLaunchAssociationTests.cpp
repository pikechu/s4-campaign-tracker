#include "campaign/CampaignLaunchAssociation.h"

#include <stdexcept>

namespace {

void Require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

campaign_completion::CampaignMenuSnapshot Snapshot() {
    campaign_completion::CampaignMenuSnapshot snapshot{};
    snapshot.status = campaign_completion::CampaignMenuSnapshotStatus::Success;
    snapshot.count = 1u;
    snapshot.features[0].valueLink = 700u;
    snapshot.features[0].x = 10u;
    snapshot.features[0].y = 20u;
    snapshot.features[0].width = 100u;
    snapshot.features[0].height = 30u;
    return snapshot;
}

S4UiElement Element() {
    S4UiElement element{};
    element.id = 700u;
    element.x = 10u;
    element.y = 20u;
    element.w = 100u;
    element.h = 30u;
    return element;
}

}  // namespace

int RunCampaignLaunchAssociationTests() {
    using namespace campaign_completion;

    CampaignLaunchAssociation association;
    auto element = Element();
    association.ObservePage(true);
    association.ObserveSnapshot(Snapshot());
    Require(!association.ObserveClick(WM_LBUTTONDOWN, &element, 100u),
            "only left-button release can arm association");
    ++element.x;
    Require(!association.ObserveClick(WM_LBUTTONUP, &element, 100u),
            "rectangle mismatch fails closed");
    element = Element();
    Require(association.ObserveClick(WM_LBUTTONUP, &element, 100u),
            "one exact control match arms association");
    Require(!association.BeginSession(
                1u,
                {LaunchSource::SinglePlayerMap,
                 SessionEligibility::Eligible},
                110u),
            "a non-campaign launch clears the candidate");

    association.ObserveSnapshot(Snapshot());
    Require(association.ObserveClick(WM_LBUTTONUP, &element, 200u),
            "a fresh unique click can arm again");
    Require(association.BeginSession(
                9u,
                {LaunchSource::Campaign, SessionEligibility::Eligible},
                210u),
            "MapInit binds an armed campaign click to its exact session");
    association.ObservePage(false);
    Require(!association.ObserveIdentity(
                 {8u, L"RD_PlayerSave", L"Map\\Campaign\\Wrong.map"},
                 220u)
                 .has_value(),
            "session mismatch cannot associate even with an RD save name");
    const auto result = association.ObserveIdentity(
        {9u, L"RD_PlayerSave", L"Map\\Campaign\\Dark\\Mission01.map"},
        230u);
    Require(result.has_value() && result->page == kDarkTribeCampaignPage &&
                result->control.controlId == 700u && result->sessionId == 9u &&
                result->relative == L"Map\\Campaign\\Dark\\Mission01.map",
            "only the same-session confirmed relative identity is emitted");
    Require(!association.ObserveIdentity(
                 {9u, L"other", L"Map\\Campaign\\Dark\\Other.map"}, 240u)
                 .has_value(),
            "association is single-use");

    CampaignLaunchAssociation expired;
    expired.ObservePage(true);
    expired.ObserveSnapshot(Snapshot());
    Require(expired.ObserveClick(WM_LBUTTONUP, &element, 1u),
            "expiry fixture arms");
    Require(!expired.BeginSession(
                10u,
                {LaunchSource::Campaign, SessionEligibility::Eligible},
                1u + kCampaignLaunchAssociationLeaseMs + 1u),
            "expired click cannot bind to a later session");

    CampaignLaunchAssociation left;
    left.ObservePage(true);
    left.ObserveSnapshot(Snapshot());
    Require(left.ObserveClick(WM_LBUTTONUP, &element, 1u),
            "page-exit fixture arms a candidate");
    left.ObservePage(false);
    Require(!left.BeginSession(
                11u,
                {LaunchSource::Campaign, SessionEligibility::Eligible}, 2u),
            "leaving page before MapInit clears click and snapshot state");

    return 0;
}
