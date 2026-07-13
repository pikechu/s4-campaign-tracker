#pragma once

#include <windows.h>

#include <array>
#include <cstddef>
#include <filesystem>
#include <mutex>
#include <string_view>

namespace campaign_completion {

enum class Phase3TraceChannel : std::size_t {
    Origin = 0u,
    Identity = 1u,
    SettlementUi = 2u,
    Decision = 3u,
};

class Phase3Trace final {
public:
    bool Open(const std::filesystem::path& root, DWORD processId);
    bool Write(Phase3TraceChannel channel, std::string_view record);
    void Close() noexcept;

    const std::filesystem::path& directory() const noexcept {
        return directory_;
    }

private:
    std::mutex mutex_;
    std::array<HANDLE, 4u> handles_{
        INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
        INVALID_HANDLE_VALUE};
    std::filesystem::path directory_;
};

}  // namespace campaign_completion
