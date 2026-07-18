#pragma once

#include "campaign/CampaignLaunchAssociation.h"
#include "campaign/CampaignDescriptorCatalog.h"
#include "campaign/CampaignMenuCapture.h"
#include "campaign/CampaignCompletionMarkerIndex.h"
#include "campaign/CampaignMarkerObserver.h"
#include "completion/ReadOnlyCompletionSource.h"
#include "completion/Win32CompletionFileOps.h"
#include "diagnostics/Logger.h"
#include "diagnostics/Phase3Trace.h"
#include "identity/MapIdentityCoordinator.h"
#include "lua/LuaApi.h"
#include "lua/SuLuaMapBridge.h"
#include "marker/CompletionMarkerIndex.h"
#include "marker/CompletionMarkerRenderer.h"
#include "marker/DirectDrawMarkerSurface.h"
#include "marker/FixedMapRowObserver.h"
#include "runtime/PluginPaths.h"
#include "runtime/S4Listeners.h"
#include "victory/LaunchOrigin.h"

#include <windows.h>

#include <atomic>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <mutex>

namespace campaign_completion {

class DiagnosticRuntime final {
public:
    bool Start(HMODULE module);
    void RunControlLoop();
    void RequestControlledStop() noexcept;

private:
    bool TryControlledStop();
    void Stop();
    void AbortStart() noexcept;

    std::mutex mutex_;
    Phase3Trace phase3Trace_;
    Logger logger_;
    std::unique_ptr<CampaignMenuCapture> campaignCapture_;
    std::unique_ptr<CampaignLaunchAssociation> campaignAssociation_;
    CampaignDescriptorCatalog campaignDescriptors_{};
    Win32CompletionFileOps completionFiles_;
    std::unique_ptr<ReadOnlyCompletionSource> completionSource_;
    CompletionMarkerIndex markerIndex_;
    CampaignCompletionMarkerIndex campaignMarkerIndex_;
    std::unique_ptr<FixedMapRowObserver> markerObserver_;
    std::unique_ptr<CampaignMarkerObserver> campaignMarkerObserver_;
    std::unique_ptr<DirectDrawMarkerSurface> markerSurface_;
    std::unique_ptr<CompletionMarkerRenderer> markerRenderer_;
    FixedMapMenuMemoryView fixedMapMenuMemory_{};
    S4Listeners listeners_;
    S4LuaApi luaApi_;
    S4LuaMapBridge luaBridge_{luaApi_};
    std::unique_ptr<MapIdentityCoordinator> coordinator_;
    LaunchOriginTracker origin_;
    S4API api_ = nullptr;
    std::optional<PluginPaths> paths_;
    std::filesystem::path stopRequestPath_;
    bool started_ = false;
    bool listenersStopped_ = false;
    std::size_t listenerStopFailures_ = 0u;
    std::atomic<bool> controlledStopRequested_{false};
};

DWORD WINAPI BootstrapThread(void* module);
DiagnosticRuntime& RuntimeInstance();

}  // namespace campaign_completion
