#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace campaign_completion {

struct ModuleInfo {
    std::wstring name;
    std::filesystem::path path;
    std::uintptr_t baseAddress = 0;
    std::uint32_t size = 0;
    std::string version;
    std::string sha256;
};

enum class CompatibilityResult {
    Compatible,
    VersionMismatch,
    HashMismatch,
};

std::vector<ModuleInfo> EnumerateLoadedModules();
std::optional<std::string> Sha256File(const std::filesystem::path& path);
std::string FileVersion(const std::filesystem::path& path);
CompatibilityResult CheckTargetExecutable(const ModuleInfo& executable);

}  // namespace campaign_completion
