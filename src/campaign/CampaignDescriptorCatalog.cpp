#include "campaign/CampaignDescriptorCatalog.h"

#include <windows.h>

#include <array>
#include <cstring>
#include <limits>
#include <string_view>

namespace campaign_completion {
namespace {

constexpr std::size_t GroupIndex(CampaignDescriptorGroup group) noexcept {
    return static_cast<std::size_t>(group);
}

constexpr std::array<CampaignDescriptorRecord,
                     kPhase6DCampaignDescriptorCount> kDescriptors{{
#include "campaign/Phase6DDescriptorTable.inc"
}};

bool SameControlId(const CampaignDescriptorRecord& record,
                   const CampaignMenuAssociation& association) noexcept {
    return record.page == association.page &&
           record.control.controlId == association.control.controlId;
}

bool SameGeometry(const CampaignControlIdentity& left,
                  const CampaignControlIdentity& right) noexcept {
    return left.x == right.x && left.y == right.y &&
           left.width == right.width && left.height == right.height;
}

CampaignDescriptorGroup GroupForPage(DWORD page) noexcept {
    switch (page) {
        case S4_SCREEN_ADDON_TROJAN:
        case S4_SCREEN_ADDON_ROMAN:
        case S4_SCREEN_ADDON_MAYAN:
        case S4_SCREEN_ADDON_VIKING:
        case S4_SCREEN_ADDON_SETTLE:
            return CampaignDescriptorGroup::AddOn;
        case S4_SCREEN_MISSIONCD:
        case S4_SCREEN_MISSIONCD_ROMAN:
        case S4_SCREEN_MISSIONCD_VIKING:
        case S4_SCREEN_MISSIONCD_MAYAN:
        case S4_SCREEN_MISSIONCD_CONFL:
            return CampaignDescriptorGroup::MissionCd;
        case S4_SCREEN_SINGLEPLAYER_CAMPAIGN:
            return CampaignDescriptorGroup::Original;
        case S4_SCREEN_SINGLEPLAYER_DARKTRIBE:
            return CampaignDescriptorGroup::DarkTribe;
        case S4_SCREEN_NEWWORLD:
            return CampaignDescriptorGroup::NewWorld;
        case S4_SCREEN_NEWWORLD2:
            return CampaignDescriptorGroup::NewWorld2;
        default:
            return CampaignDescriptorGroup::Count;
    }
}

bool Readable(const void* address, std::size_t size) noexcept {
    if (address == nullptr || size == 0u) return false;
    MEMORY_BASIC_INFORMATION memory{};
    if (VirtualQuery(address, &memory, sizeof(memory)) != sizeof(memory) ||
        memory.State != MEM_COMMIT ||
        (memory.Protect & (PAGE_GUARD | PAGE_NOACCESS)) != 0u) return false;
    const auto begin = reinterpret_cast<std::uintptr_t>(address);
    const auto region = reinterpret_cast<std::uintptr_t>(memory.BaseAddress);
    if (begin < region || size > memory.RegionSize - (begin - region)) {
        return false;
    }
    return true;
}

template <std::size_t N>
bool Match(const ModuleInfo& executable, std::uint32_t rva,
           const std::array<BYTE, N>& expected) noexcept {
    if (executable.baseAddress == 0u || rva > executable.size ||
        N > executable.size - rva ||
        executable.baseAddress >
            (std::numeric_limits<std::uintptr_t>::max)() - rva) return false;
    const auto* address = reinterpret_cast<const void*>(
        executable.baseAddress + static_cast<std::uintptr_t>(rva));
    if (!Readable(address, N)) return false;
#if defined(_MSC_VER)
    __try {
        return std::memcmp(address, expected.data(), N) == 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
#else
    return std::memcmp(address, expected.data(), N) == 0;
#endif
}

bool MatchRelocatedPointer(const ModuleInfo& executable, std::uint32_t rva,
                           std::uint32_t targetRva) noexcept {
    if (executable.baseAddress == 0u || rva > executable.size ||
        sizeof(std::uint32_t) > executable.size - rva ||
        executable.baseAddress >
            (std::numeric_limits<std::uint32_t>::max)() - targetRva) {
        return false;
    }
    const auto* address = reinterpret_cast<const void*>(
        executable.baseAddress + static_cast<std::uintptr_t>(rva));
    if (!Readable(address, sizeof(std::uint32_t))) return false;
    std::uint32_t actual = 0u;
#if defined(_MSC_VER)
    __try {
        std::memcpy(&actual, address, sizeof(actual));
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
#else
    std::memcpy(&actual, address, sizeof(actual));
#endif
    return actual == static_cast<std::uint32_t>(executable.baseAddress) +
                         targetRva;
}

template <std::size_t N>
bool MatchRelocatedRvaTable(
    const ModuleInfo& executable, std::uint32_t rva,
    const std::array<std::uint32_t, N>& targetRvas) noexcept {
    for (std::size_t index = 0u; index < N; ++index) {
        if (!MatchRelocatedPointer(
                executable,
                rva + static_cast<std::uint32_t>(index * sizeof(std::uint32_t)),
                targetRvas[index])) {
            return false;
        }
    }
    return true;
}

bool MatchFileWindow(const ModuleInfo& executable, std::uint64_t offset,
                     std::size_t length,
                     std::string_view expected) noexcept {
    try {
        const auto actual =
            Sha256FileRange(executable.path, offset, length);
        return actual.has_value() && *actual == expected;
    } catch (...) {
        return false;
    }
}

}  // namespace

bool CampaignDescriptorCatalog::GroupAdmitted(
    CampaignDescriptorGroup group) const noexcept {
    const auto index = GroupIndex(group);
    return executableAdmitted && index < groups.size() && groups[index];
}

CampaignDescriptorEvidence InspectCampaignDescriptorEvidence(
    const ModuleInfo& executable) noexcept {
    CampaignDescriptorEvidence result{};
    if (CheckTargetExecutable(executable) != CompatibilityResult::Compatible) {
        return result;
    }
    result.addOn =
        Match(executable, 0x0006B27Bu,
              std::array<BYTE, 12u>{0x0F, 0xB7, 0x45, 0x0C, 0x83, 0xC0,
                                    0xA6, 0x83, 0xF8, 0x0C, 0x0F, 0x87}) &&
        Match(executable, 0x0006A25Bu,
              std::array<BYTE, 12u>{0x0F, 0xB7, 0x45, 0x0C, 0x83, 0xC0,
                                    0xDF, 0x83, 0xF8, 0x04, 0x0F, 0x87});
    result.missionCd =
        Match(executable, 0x000961F6u,
              std::array<BYTE, 8u>{0x85, 0xC9, 0x0F, 0x84, 0x08, 0x01,
                                   0x00, 0x00}) &&
        Match(executable, 0x000963EBu,
              std::array<BYTE, 14u>{0x0F, 0xB7, 0x45, 0x0C, 0x05, 0x91, 0xF8,
                                    0xFF, 0xFF, 0x83, 0xF8, 0x0C, 0x0F, 0x87}) &&
        Match(executable, 0x000965A0u,
              std::array<BYTE, 13u>{0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x06,
                                    0x06, 0x06, 0x06, 0x06, 0x06, 0x05}) &&
        MatchRelocatedRvaTable(
            executable, 0x00096584u,
            std::array<std::uint32_t, 7u>{0x0009643Eu, 0x00096471u,
                                          0x000964A4u, 0x000964D4u,
                                          0x00096504u, 0x0009640Bu,
                                          0x0009653Eu}) &&
        Match(executable, 0x00096452u,
              std::array<BYTE, 11u>{0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x00,
                                    0x68, 0x60, 0x1B, 0x00, 0x00}) &&
        Match(executable, 0x00096485u,
              std::array<BYTE, 11u>{0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x01,
                                    0x68, 0x60, 0x1B, 0x00, 0x00}) &&
        Match(executable, 0x00125368u,
              std::array<BYTE, 20u>{0x8B, 0x4D, 0x08, 0x8B, 0x51, 0x04, 0x8B,
                                    0xC2, 0x83, 0xE8, 0x0B, 0x74, 0x30, 0x2D,
                                    0x55, 0x1B, 0x00, 0x00, 0x74, 0x14}) &&
        Match(executable, 0x00125390u,
              std::array<BYTE, 3u>{0x8B, 0x51, 0x08}) &&
        Match(executable, 0x00125398u,
              std::array<BYTE, 6u>{0xC1, 0xE2, 0x10, 0x83, 0xCA, 0x05}) &&
        Match(executable, 0x00124701u,
              std::array<BYTE, 9u>{0x8B, 0x46, 0x04, 0x83, 0xE8, 0x05, 0x8D,
                                   0x04, 0x40}) &&
        MatchRelocatedPointer(executable, 0x0012470Du, 0x0109C32Cu) &&
        MatchRelocatedPointer(executable, 0x00124715u, 0x0109C318u) &&
        Match(executable, 0x0012471Du,
              std::array<BYTE, 6u>{0x8B, 0x46, 0x08, 0x40, 0x50, 0x51}) &&
        Match(executable, 0x00023ADCu,
              std::array<BYTE, 4u>{0x6A, 0x1D, 0x33, 0xC0}) &&
        MatchRelocatedPointer(executable, 0x00023AEBu, 0x00C573E0u) &&
        MatchRelocatedPointer(executable, 0x00023AF0u, 0x0109C318u) &&
        Match(executable, 0x00C573E0u,
              std::array<BYTE, 60u>{
                  0x4D, 0x00, 0x61, 0x00, 0x70, 0x00, 0x5C, 0x00, 0x43, 0x00,
                  0x61, 0x00, 0x6D, 0x00, 0x70, 0x00, 0x61, 0x00, 0x69, 0x00,
                  0x67, 0x00, 0x6E, 0x00, 0x5C, 0x00, 0x6D, 0x00, 0x64, 0x00,
                  0x5F, 0x00, 0x72, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x61, 0x00,
                  0x6E, 0x00, 0x25, 0x00, 0x30, 0x00, 0x31, 0x00, 0x69, 0x00,
                  0x2E, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x70, 0x00, 0x00, 0x00});
    result.original =
        Match(executable, 0x0007BF96u,
              std::array<BYTE, 8u>{0x85, 0xC9, 0x0F, 0x84, 0xA2, 0x01,
                                   0x00, 0x00}) &&
        Match(executable, 0x0007C22Bu,
              std::array<BYTE, 14u>{0x0F, 0xB7, 0x45, 0x0C, 0x05, 0x0B, 0xF8,
                                    0xFF, 0xFF, 0x83, 0xF8, 0x09, 0x0F, 0x87});
    result.darkTribe =
        Match(executable, 0x0007D7B6u,
              std::array<BYTE, 8u>{0x85, 0xC9, 0x0F, 0x84, 0x22, 0x02,
                                   0x00, 0x00}) &&
        Match(executable, 0x0007DAEBu,
              std::array<BYTE, 4u>{0x0F, 0xB7, 0x45, 0x0C});
    result.addOn =
        result.addOn &&
        MatchFileWindow(executable, 0x695F0u, 5952u,
            "f096f896d81bc890c1655976eb830beac929b6396369f7c85b755a9945cb36d") &&
        MatchFileWindow(executable, 0x10D1A0u, 8960u,
            "09408e19ee24df80e6c5dac5cb6dabd2be9a07798793254f7d4b76379098971") &&
        MatchFileWindow(executable, 0x22490u, 415u,
            "9459c9d05561fce909b1998ec4e5bf207884bd4024f013b38990fa6ff512799");
    result.missionCd =
        result.missionCd &&
        MatchFileWindow(executable, 0x95230u, 5136u,
            "147f9db6610dc81e5879a79cba595383b6adf74173502a9bbcf7a28d6b90b3") &&
        MatchFileWindow(executable, 0x123750u, 7104u,
            "c3471442b6fb5876ea999d12aa6f941d553a0a9571b53da7b0f53890e1141b") &&
        MatchFileWindow(executable, 0x22EB0u, 415u,
            "63278d299e47c494f8f3f3b11184307149c22f8a7b42d2978ab3b35f7af8b8");
    result.newWorld =
        MatchFileWindow(executable, 0x94670u, 1040u,
            "85f8bbd6896442a253a244288619f3b22d804b21cab2ec418c0b24f3e01e47") &&
        MatchFileWindow(executable, 0x1228F0u, 3376u,
            "12d6217cc0832a5d181b22ad91bb4dc45732fb9a6748264e6347801e0a372a") &&
        MatchFileWindow(executable, 0x22D80u, 297u,
            "d37b56be34dc9797f28f9102c734be53efe76fa48320107e022c95070a2756");
    result.newWorld2 =
        MatchFileWindow(executable, 0xAF380u, 2752u,
            "e1374d0dac959150a597ae1274ca6ce5d772007f9d6583de10a5ab97374b50") &&
        MatchFileWindow(executable, 0x127EE0u, 3556u,
            "085950439ced65fcfe77890d75c71ebf4ed7a139832a9cbff7bc1e14029471") &&
        MatchFileWindow(executable, 0x230A0u, 297u,
            "3c56162605cd48a5fbec9671cc6e4da09468f7393ccaa04d0ddf1b46048ce88");
    result.original =
        result.original &&
        MatchFileWindow(executable, 0x7B550u, 1072u,
            "ded0825cc4bb08bf435a1edcebd65f8976ba36e1bf6b5d8fc86242a9e069b3") &&
        MatchFileWindow(executable, 0x10FA10u, 3751u,
            "685185045a9cecf74145807fec9f6bab0602ee32d5ae1518a003bd55855722") &&
        MatchFileWindow(executable, 0x227F0u, 384u,
            "96a78fc19aa9be5f9623069fdb508706b15126cb06c7cce5ed9719866ab978");
    result.darkTribe =
        result.darkTribe &&
        MatchFileWindow(executable, 0x7CBB0u, 561u,
            "44b14df62b75a2ad35278623b184f2b6700161c9bf41237c866177b779e016") &&
        MatchFileWindow(executable, 0x7CEEBu, 677u,
            "e0a84e0903b45b5ca577cc117c599fead9b16617e35f061007b6f96285df34");
    return result;
}

CampaignDescriptorCatalog AdmitCampaignDescriptorCatalog(
    const ModuleInfo& executable) noexcept {
    return AdmitCampaignDescriptorCatalog(
        executable, InspectCampaignDescriptorEvidence(executable));
}

CampaignDescriptorCatalog AdmitCampaignDescriptorCatalog(
    const ModuleInfo& executable,
    const CampaignDescriptorEvidence& evidence) noexcept {
    CampaignDescriptorCatalog result{};
    result.executableAdmitted =
        CheckTargetExecutable(executable) == CompatibilityResult::Compatible;
    if (!result.executableAdmitted) return result;
    result.groups[GroupIndex(CampaignDescriptorGroup::AddOn)] = evidence.addOn;
    result.groups[GroupIndex(CampaignDescriptorGroup::MissionCd)] =
        evidence.missionCd;
    result.groups[GroupIndex(CampaignDescriptorGroup::NewWorld)] =
        evidence.newWorld;
    result.groups[GroupIndex(CampaignDescriptorGroup::NewWorld2)] =
        evidence.newWorld2;
    result.groups[GroupIndex(CampaignDescriptorGroup::Original)] =
        evidence.original;
    result.groups[GroupIndex(CampaignDescriptorGroup::DarkTribe)] =
        evidence.darkTribe;
    return result;
}

CampaignDescriptorValidation ValidateCampaignDescriptor(
    const CampaignDescriptorCatalog& catalog,
    const CampaignMenuAssociation& association) noexcept {
    if (!catalog.executableAdmitted) {
        return {CampaignDescriptorValidationStatus::CatalogNotAdmitted,
                nullptr};
    }
    for (const auto& record : kDescriptors) {
        if (!SameControlId(record, association)) continue;
        if (!catalog.GroupAdmitted(record.group)) {
            return {CampaignDescriptorValidationStatus::GroupNotAdmitted,
                    &record};
        }
        if (!SameGeometry(record.control, association.control)) {
            return {CampaignDescriptorValidationStatus::GeometryMismatch,
                    &record};
        }
        if (record.relative == nullptr ||
            association.relative != std::wstring_view(record.relative)) {
            return {CampaignDescriptorValidationStatus::RelativeMismatch,
                    &record};
        }
        return {CampaignDescriptorValidationStatus::Matched, &record};
    }
    const auto group = GroupForPage(association.page);
    if (group != CampaignDescriptorGroup::Count &&
        !catalog.GroupAdmitted(group)) {
        return {CampaignDescriptorValidationStatus::GroupNotAdmitted, nullptr};
    }
    return {CampaignDescriptorValidationStatus::ControlUnknown, nullptr};
}

const CampaignDescriptorRecord* FindAdmittedCampaignDescriptor(
    const CampaignDescriptorCatalog& catalog, DWORD page,
    const CampaignControlIdentity& control) noexcept {
    if (!catalog.executableAdmitted) return nullptr;
    for (const auto& record : kDescriptors) {
        if (record.page == page && record.control.controlId == control.controlId &&
            SameGeometry(record.control, control) &&
            catalog.GroupAdmitted(record.group)) {
            return &record;
        }
    }
    return nullptr;
}

const CampaignDescriptorRecord* FindAdmittedCampaignDescriptorRelative(
    const CampaignDescriptorCatalog& catalog,
    std::wstring_view relative) noexcept {
    if (!catalog.executableAdmitted || relative.empty()) return nullptr;
    const CampaignDescriptorRecord* match = nullptr;
    for (const auto& record : kDescriptors) {
        if (!catalog.GroupAdmitted(record.group) || record.relative == nullptr ||
            relative != std::wstring_view(record.relative)) {
            continue;
        }
        if (match != nullptr) return nullptr;
        match = &record;
    }
    return match;
}

const std::array<CampaignDescriptorRecord,
                 kPhase6DCampaignDescriptorCount>&
AllCampaignDescriptors() noexcept {
    return kDescriptors;
}

}  // namespace campaign_completion
