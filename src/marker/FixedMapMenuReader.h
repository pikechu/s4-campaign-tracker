#pragma once

#include "diagnostics/ModuleInventory.h"
#include "marker/BoundedMenuText.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace campaign_completion {

inline constexpr std::size_t kMaximumInternalVisibleRows = 6u;

enum class FixedMapMenuReadStatus {
    Success,
    NotAdmitted,
    HeaderUnreadable,
    AliasMismatch,
    CountOutOfRange,
    ScrollOutOfRange,
    EntryUnreadable,
    LabelInvalid,
    ConcurrentMutation,
};

struct FixedMapMenuMemoryView final {
    const std::uint32_t* scrollBase = nullptr;
    const std::uint32_t* entryCount = nullptr;
    const std::uint32_t* arrayAlias = nullptr;
    const std::uint32_t* entries = nullptr;
    std::uint32_t expectedArrayAddress = 0u;
    bool admitted = false;
};

struct FixedMapMenuSnapshot final {
    std::array<BoundedWideText, kMaximumInternalVisibleRows> labels{};
    std::uint32_t entryCount = 0u;
    std::uint32_t scrollBase = 0u;
    std::size_t rowCount = 0u;
    FixedMapMenuReadStatus status = FixedMapMenuReadStatus::NotAdmitted;
};

FixedMapMenuMemoryView AdmitFixedMapMenuMemory(
    const ModuleInfo& executable) noexcept;
FixedMapMenuSnapshot ReadFixedMapMenuSnapshot(
    const FixedMapMenuMemoryView& memory) noexcept;
bool EqualFixedMapMenuSnapshot(const FixedMapMenuSnapshot& left,
                               const FixedMapMenuSnapshot& right) noexcept;

}  // namespace campaign_completion
