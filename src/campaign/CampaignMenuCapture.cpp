#include "campaign/CampaignMenuCapture.h"

#include <algorithm>

namespace campaign_completion {
namespace {

bool EqualText(const CampaignMenuFeature& left,
               const CampaignMenuFeature& right) noexcept {
    if (left.hasText != right.hasText) return false;
    if (!left.hasText) return true;
    return left.text.length == right.text.length &&
           left.text.bytes == right.text.bytes;
}

bool SameControlKey(const CampaignMenuFeature& left,
                    const CampaignMenuFeature& right) noexcept {
    return left.valueLink == right.valueLink && left.x == right.x &&
           left.y == right.y && left.width == right.width &&
           left.height == right.height;
}

bool EqualExceptEffects(const CampaignMenuFeature& left,
                        const CampaignMenuFeature& right) noexcept {
    auto normalizedLeft = left;
    auto normalizedRight = right;
    normalizedLeft.effects = 0u;
    normalizedRight.effects = 0u;
    return EqualCampaignMenuFeature(normalizedLeft, normalizedRight);
}

bool LessFeature(const CampaignMenuFeature& left,
                 const CampaignMenuFeature& right) noexcept {
    if (left.valueLink != right.valueLink) {
        return left.valueLink < right.valueLink;
    }
    if (left.y != right.y) return left.y < right.y;
    if (left.x != right.x) return left.x < right.x;
    if (left.height != right.height) return left.height < right.height;
    return left.width < right.width;
}

}  // namespace

bool IsCampaignCatalogPage(DWORD page) noexcept {
    for (const DWORD admitted : kCampaignCatalogPages) {
        if (page == admitted) return true;
    }
    return false;
}

DWORD SelectCampaignCatalogOwner(
    const std::array<bool, kCampaignCatalogPages.size()>& active) noexcept {
    DWORD owner = S4_GUI_UNKNOWN;
    for (std::size_t index = 0u; index < active.size(); ++index) {
        if (active[index]) owner = kCampaignCatalogPages[index];
    }
    return owner;
}

bool CopyCampaignMenuFeature(LPS4GUIDRAWBLTPARAMS source,
                             CampaignMenuFeature& output) noexcept {
    if (source == nullptr) return false;
    CampaignMenuFeature candidate{};
#if defined(_MSC_VER)
    __try {
#endif
        candidate.surfaceWidth = source->surfaceWidth;
        candidate.surfaceHeight = source->surfaceHeight;
        candidate.gfxCollection = source->currentGFXCollection;
        candidate.containerType = source->containerType;
        candidate.x = source->x;
        candidate.y = source->y;
        candidate.width = source->width;
        candidate.height = source->height;
        candidate.mainTexture = source->mainTexture;
        candidate.valueLink = source->valueLink;
        candidate.buttonPressedTexture = source->buttonPressedTexture;
        candidate.tooltipLink = source->tooltipLink;
        candidate.tooltipLinkExtra = source->tooltipLinkExtra;
        candidate.imageStyle = static_cast<std::uint16_t>(source->imageStyle);
        candidate.effects = static_cast<std::uint16_t>(source->effects);
        candidate.textStyle = static_cast<std::uint16_t>(source->textStyle);
        candidate.showTexture = source->showTexture;
        candidate.backTexture = source->backTexture;
        if (source->text != nullptr) {
            if (!CopyBoundedGuiText(source->text, candidate.text)) {
                return false;
            }
            candidate.hasText = true;
        }
#if defined(_MSC_VER)
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
#endif
    output = candidate;
    return true;
}

bool EqualCampaignMenuFeature(const CampaignMenuFeature& left,
                              const CampaignMenuFeature& right) noexcept {
    return left.surfaceWidth == right.surfaceWidth &&
           left.surfaceHeight == right.surfaceHeight &&
           left.gfxCollection == right.gfxCollection &&
           left.containerType == right.containerType && left.x == right.x &&
           left.y == right.y && left.width == right.width &&
           left.height == right.height &&
           left.mainTexture == right.mainTexture &&
           left.valueLink == right.valueLink &&
           left.buttonPressedTexture == right.buttonPressedTexture &&
           left.tooltipLink == right.tooltipLink &&
           left.tooltipLinkExtra == right.tooltipLinkExtra &&
           left.imageStyle == right.imageStyle &&
           left.effects == right.effects &&
           left.textStyle == right.textStyle &&
           left.showTexture == right.showTexture &&
           left.backTexture == right.backTexture && EqualText(left, right);
}

bool EqualCampaignMenuSnapshot(const CampaignMenuSnapshot& left,
                               const CampaignMenuSnapshot& right) noexcept {
    if (left.status != right.status || left.page != right.page ||
        left.count != right.count) {
        return false;
    }
    for (std::size_t index = 0u; index < left.count; ++index) {
        if (!EqualCampaignMenuFeature(left.features[index],
                                      right.features[index])) {
            return false;
        }
    }
    return true;
}

std::optional<CampaignMenuSnapshot> CampaignMenuCapture::ObserveFrame(
    DWORD page, bool campaignPageActive) noexcept {
    if (!enabled_) return std::nullopt;
    if (!campaignPageActive || !IsCampaignCatalogPage(page)) {
        collecting_ = false;
        activePage_ = S4_GUI_UNKNOWN;
        ClearWorking();
        return std::nullopt;
    }
    if (collecting_ && page != activePage_) {
        ClearWorking();
        collecting_ = false;
    }
    ++generation_;
    if (!collecting_) {
        ClearWorking();
        collecting_ = true;
        activePage_ = page;
        return std::nullopt;
    }
    if (!dirty_) return std::nullopt;

    CampaignMenuSnapshot snapshot{};
    snapshot.generation = generation_;
    snapshot.page = activePage_;
    snapshot.status = invalid_ ? CampaignMenuSnapshotStatus::Invalid
                               : CampaignMenuSnapshotStatus::Success;
    if (!invalid_) {
        snapshot.count = count_;
        for (std::size_t index = 0u; index < count_; ++index) {
            snapshot.features[index] = cached_[index];
        }
        std::sort(snapshot.features.begin(),
                  snapshot.features.begin() + snapshot.count, LessFeature);
    }
    dirty_ = false;
    return snapshot;
}

bool CampaignMenuCapture::ObserveFeature(
    const CampaignMenuFeature& feature) noexcept {
    if (!enabled_ || !collecting_ || invalid_) return false;
    for (std::size_t index = 0u; index < count_; ++index) {
        if (EqualCampaignMenuFeature(cached_[index], feature)) return true;
        if (SameControlKey(cached_[index], feature)) {
            if (EqualExceptEffects(cached_[index], feature)) {
                cached_[index].effects = feature.effects;
                dirty_ = true;
                return true;
            }
            invalid_ = true;
            count_ = 0u;
            dirty_ = true;
            return false;
        }
    }
    if (count_ >= cached_.size()) {
        invalid_ = true;
        count_ = 0u;
        dirty_ = true;
        return false;
    }
    cached_[count_] = feature;
    ++count_;
    dirty_ = true;
    return true;
}

void CampaignMenuCapture::Invalidate() noexcept {
    if (!enabled_ || !collecting_) return;
    invalid_ = true;
    count_ = 0u;
    dirty_ = true;
}

void CampaignMenuCapture::Disable() noexcept {
    enabled_ = false;
    collecting_ = false;
    activePage_ = S4_GUI_UNKNOWN;
    ClearWorking();
}

void CampaignMenuCapture::ClearWorking() noexcept {
    count_ = 0u;
    invalid_ = false;
    dirty_ = false;
}

}  // namespace campaign_completion
