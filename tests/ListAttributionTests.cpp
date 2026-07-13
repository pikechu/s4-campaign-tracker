#include "identity/ListAttribution.h"

#include <windows.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace {

void Require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

campaign_completion::PageSnapshot FixedMapPages() {
    return {{4, 22, 23, 25}, 25};
}

}  // namespace

int RunListAttributionTests() {
    using campaign_completion::FixedMapListKind;
    using campaign_completion::ListAttribution;
    using campaign_completion::PageSnapshot;
    using campaign_completion::TabControlMapping;

    constexpr TabControlMapping mapping{2450, 2451, 2415};
    ListAttribution attribution(mapping);

    attribution.ObservePages(FixedMapPages());
    Require(attribution.Current() == FixedMapListKind::Unknown,
            "fixed-map pages alone remain unknown");

    attribution.ObserveClick(WM_LBUTTONUP, mapping.single);
    Require(attribution.Current() == FixedMapListKind::Single,
            "single-player tab click is attributed");
    attribution.ObserveClick(WM_LBUTTONUP, mapping.multiplayer);
    Require(attribution.Current() == FixedMapListKind::Multiplayer,
            "multiplayer tab click is attributed");
    attribution.ObserveClick(WM_LBUTTONUP, mapping.custom);
    Require(attribution.Current() == FixedMapListKind::Custom,
            "custom tab click is attributed");

    attribution.ObserveClick(WM_LBUTTONUP, 9999);
    Require(attribution.Current() == FixedMapListKind::Unknown,
            "unmapped click is unknown");

    attribution.ObserveClick(WM_LBUTTONUP, mapping.single);
    attribution.ObservePages(PageSnapshot{{4, 22, 23}, 23});
    Require(attribution.Current() == FixedMapListKind::Unknown,
            "leaving exact fixed-map pages resets attribution");

    attribution.ObserveClick(WM_LBUTTONUP, mapping.custom);
    attribution.ObservePages(FixedMapPages());
    Require(attribution.Current() == FixedMapListKind::Unknown,
            "click before re-entry is not reused");

    attribution.ObserveClick(WM_LBUTTONDOWN, mapping.single);
    Require(attribution.Current() == FixedMapListKind::Unknown,
            "only left-button release changes attribution");

    attribution.Reset();
    Require(attribution.Current() == FixedMapListKind::Unknown,
            "explicit reset clears attribution");
    return 0;
}
