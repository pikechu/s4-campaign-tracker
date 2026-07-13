#pragma once

#include "diagnostics/Phase3Trace.h"
#include "diagnostics/Logger.h"
#include "identity/MapIdentityCoordinator.h"
#include "lua/LuaApi.h"
#include "lua/SuLuaMapBridge.h"
#include "runtime/S4Listeners.h"
#include "victory/LaunchOrigin.h"
#include "victory/SettlementUiProbe.h"

#include <windows.h>

#include <filesystem>
#include <mutex>
#include <memory>

namespace campaign_completion {

class DiagnosticRuntime final {
public:
    bool Start(HMODULE module);
    void RunControlLoop();
    void Stop();

private:
    std::mutex mutex_;
    Phase3Trace phase3Trace_;
    Logger logger_;
    S4Listeners listeners_;
    S4LuaApi luaApi_;
    S4LuaMapBridge luaBridge_{luaApi_};
    std::unique_ptr<MapIdentityCoordinator> coordinator_;
    LaunchOriginTracker origin_;
    SettlementUiProbe settlement_;
    S4API api_ = nullptr;
    std::filesystem::path stopRequestPath_;
    bool started_ = false;
};

DWORD WINAPI BootstrapThread(void* module);
DiagnosticRuntime& RuntimeInstance();

}  // namespace campaign_completion
