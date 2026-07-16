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

constexpr CampaignDescriptorRecord kDescriptors[]{
    {"addon-trojan-01", CampaignDescriptorGroup::AddOn,
     S4_SCREEN_ADDON_TROJAN, {91u, 320u, 200u, 175u, 30u}, 0u, 0x0Fu,
     L"Map\\Campaign\\ao_trojan01.map"},
    {"addon-mayan-01", CampaignDescriptorGroup::AddOn,
     S4_SCREEN_ADDON_MAYAN, {34u, 480u, 177u, 175u, 30u}, 0u, 0x0Du,
     L"Map\\Campaign\\ao_maya1.map"},
    {"md-roman-01", CampaignDescriptorGroup::MdRoman,
     S4_SCREEN_MISSIONCD_ROMAN, {1903u, 237u, 148u, 175u, 30u}, 0u, 0x05u,
     L"Map\\Campaign\\md_roman1.map"},
    {"md-roman-02", CampaignDescriptorGroup::MdRoman,
     S4_SCREEN_MISSIONCD_ROMAN, {1904u, 237u, 195u, 175u, 30u}, 1u, 0x05u,
     L"Map\\Campaign\\md_roman2.map"},
    {"original-viking-02", CampaignDescriptorGroup::Original,
     S4_SCREEN_SINGLEPLAYER_CAMPAIGN, {2039u, 420u, 150u, 175u, 30u}, 1u,
     0x01u, L"Map\\Campaign\\viking02.map"},
    {"original-viking-03", CampaignDescriptorGroup::Original,
     S4_SCREEN_SINGLEPLAYER_CAMPAIGN, {2040u, 420u, 190u, 175u, 30u}, 2u,
     0x01u, L"Map\\Campaign\\viking03.map"},
    {"dark-01", CampaignDescriptorGroup::DarkTribe,
     S4_SCREEN_SINGLEPLAYER_DARKTRIBE, {2081u, 500u, 104u, 175u, 30u}, 0u,
     0x03u, L"Map\\Campaign\\dark01.map"},
    {"dark-02", CampaignDescriptorGroup::DarkTribe,
     S4_SCREEN_SINGLEPLAYER_DARKTRIBE, {2082u, 500u, 136u, 175u, 30u}, 1u,
     0x03u, L"Map\\Campaign\\dark02.map"},
    {"dark-03", CampaignDescriptorGroup::DarkTribe,
     S4_SCREEN_SINGLEPLAYER_DARKTRIBE, {2083u, 500u, 168u, 175u, 30u}, 2u,
     0x03u, L"Map\\Campaign\\dark03.map"},
    {"dark-04", CampaignDescriptorGroup::DarkTribe,
     S4_SCREEN_SINGLEPLAYER_DARKTRIBE, {2084u, 500u, 200u, 175u, 30u}, 3u,
     0x03u, L"Map\\Campaign\\dark04.map"},
    {"dark-05", CampaignDescriptorGroup::DarkTribe,
     S4_SCREEN_SINGLEPLAYER_DARKTRIBE, {2085u, 500u, 232u, 175u, 30u}, 4u,
     0x03u, L"Map\\Campaign\\dark05.map"},
    {"dark-06", CampaignDescriptorGroup::DarkTribe,
     S4_SCREEN_SINGLEPLAYER_DARKTRIBE, {2086u, 500u, 264u, 175u, 30u}, 5u,
     0x03u, L"Map\\Campaign\\dark06.map"},
    {"dark-07", CampaignDescriptorGroup::DarkTribe,
     S4_SCREEN_SINGLEPLAYER_DARKTRIBE, {2087u, 500u, 296u, 175u, 30u}, 6u,
     0x03u, L"Map\\Campaign\\dark07.map"},
    {"dark-08", CampaignDescriptorGroup::DarkTribe,
     S4_SCREEN_SINGLEPLAYER_DARKTRIBE, {2088u, 500u, 328u, 175u, 30u}, 7u,
     0x03u, L"Map\\Campaign\\dark08.map"},
    {"dark-09", CampaignDescriptorGroup::DarkTribe,
     S4_SCREEN_SINGLEPLAYER_DARKTRIBE, {2089u, 500u, 360u, 175u, 30u}, 8u,
     0x03u, L"Map\\Campaign\\dark09.map"},
    {"dark-10", CampaignDescriptorGroup::DarkTribe,
     S4_SCREEN_SINGLEPLAYER_DARKTRIBE, {2090u, 500u, 392u, 175u, 30u}, 9u,
     0x03u, L"Map\\Campaign\\dark10.map"},
    {"dark-11", CampaignDescriptorGroup::DarkTribe,
     S4_SCREEN_SINGLEPLAYER_DARKTRIBE, {2091u, 500u, 424u, 175u, 30u}, 10u,
     0x03u, L"Map\\Campaign\\dark11.map"},
    {"dark-12", CampaignDescriptorGroup::DarkTribe,
     S4_SCREEN_SINGLEPLAYER_DARKTRIBE, {2092u, 500u, 456u, 175u, 30u}, 11u,
     0x03u, L"Map\\Campaign\\dark12.map"},
};

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
        case S4_SCREEN_MISSIONCD_ROMAN:
            return CampaignDescriptorGroup::MdRoman;
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
    result.mdRoman =
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
    result.groups[GroupIndex(CampaignDescriptorGroup::MdRoman)] =
        evidence.mdRoman;
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
    const auto group = GroupForPage(association.page);
    if (group != CampaignDescriptorGroup::Count &&
        !catalog.GroupAdmitted(group)) {
        return {CampaignDescriptorValidationStatus::GroupNotAdmitted, nullptr};
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

}  // namespace campaign_completion
