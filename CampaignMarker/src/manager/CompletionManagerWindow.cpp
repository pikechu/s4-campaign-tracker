#include "manager/CompletionManagerWindow.h"

#include <commctrl.h>

#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>
#include <utility>

namespace campaign_completion {
namespace {

constexpr wchar_t kWindowClass[] =
    L"CampaignCompletionManagerWindowV1";
constexpr int kTreeId = 7001;
constexpr int kListId = 7002;
constexpr int kRefreshId = 7003;
constexpr int kApplyId = 7004;
constexpr int kCloseId = 7005;
constexpr LPARAM kCampaignFilter = -2;
constexpr LPARAM kMapFilter = -3;

HTREEITEM InsertTreeItem(HWND tree, HTREEITEM parent,
                         const wchar_t* text, LPARAM value) {
    TVINSERTSTRUCTW insert{};
    insert.hParent = parent;
    insert.hInsertAfter = TVI_LAST;
    insert.item.mask = TVIF_TEXT | TVIF_PARAM;
    insert.item.pszText = const_cast<wchar_t*>(text);
    insert.item.lParam = value;
    return TreeView_InsertItem(tree, &insert);
}

bool Writable(CompletionStoreMode mode) noexcept {
    return mode == CompletionStoreMode::WritableEmpty ||
           mode == CompletionStoreMode::WritableLoaded;
}

const wchar_t* ResultText(ManualCompletionApplyStatus status) noexcept {
    switch (status) {
        case ManualCompletionApplyStatus::Committed:
            return L"Applied: marker states were saved atomically.";
        case ManualCompletionApplyStatus::Unchanged:
            return L"No changes to save.";
        case ManualCompletionApplyStatus::Conflict:
            return L"The database changed while this window was open. "
                   L"The view was refreshed; please review your changes.";
        case ManualCompletionApplyStatus::ReadOnly:
            return L"The database is read-only; changes cannot be applied.";
        case ManualCompletionApplyStatus::Invalid:
            return L"The changes failed identity or data validation.";
        case ManualCompletionApplyStatus::Failed:
            return L"Save failed; the database and in-memory markers "
                   L"remain unchanged.";
    }
    return L"Unknown result.";
}

}  // namespace

CompletionManagerWindow::CompletionManagerWindow(
    HMODULE module, ViewProvider view, ApplyHandler apply)
    : module_(module), view_(std::move(view)), apply_(std::move(apply)) {}

CompletionManagerWindow::~CompletionManagerWindow() {
    Stop(std::chrono::milliseconds(5000));
    if (readyEvent_ != nullptr) CloseHandle(readyEvent_);
}

bool CompletionManagerWindow::Start() noexcept {
    try {
        if (thread_.joinable() || module_ == nullptr || !view_ || !apply_) {
            return false;
        }
        stopping_.store(false, std::memory_order_release);
        readyEvent_ = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (readyEvent_ == nullptr) return false;
        thread_ = std::thread([this] { Run(); });
        return WaitForSingleObject(readyEvent_, 5000u) == WAIT_OBJECT_0 &&
               threadId_ != 0u;
    } catch (...) {
        return false;
    }
}

bool CompletionManagerWindow::RequestOpen() noexcept {
    if (threadId_ == 0u || stopping_.load(std::memory_order_acquire) ||
        !mainMenuAvailable_.load(std::memory_order_acquire)) {
        return false;
    }
    return PostThreadMessageW(threadId_, kOpenMessage, 0u, 0) != FALSE;
}

void CompletionManagerWindow::SetMainMenuAvailable(bool available) noexcept {
    const bool previous =
        mainMenuAvailable_.exchange(available, std::memory_order_acq_rel);
    if (previous == available) return;
    if (threadId_ != 0u) {
        PostThreadMessageW(threadId_, kAvailabilityMessage,
                           available ? 1u : 0u, 0);
    }
}

bool CompletionManagerWindow::Stop(
    std::chrono::milliseconds timeout) noexcept {
    try {
        if (!thread_.joinable()) return true;
        stopping_.store(true, std::memory_order_release);
        PostThreadMessageW(threadId_, WM_QUIT, 0u, 0);
        const auto handle =
            reinterpret_cast<HANDLE>(thread_.native_handle());
        const DWORD wait = timeout.count() < 0
            ? 0u
            : static_cast<DWORD>((std::min)(
                  timeout.count(),
                  static_cast<std::int64_t>(INFINITE - 1u)));
        if (WaitForSingleObject(handle, wait) != WAIT_OBJECT_0) {
            return false;
        }
        thread_.join();
        threadId_ = 0u;
        return true;
    } catch (...) {
        return false;
    }
}

void CompletionManagerWindow::Run() noexcept {
    threadId_ = GetCurrentThreadId();
    MSG seed{};
    PeekMessageW(&seed, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
    if (readyEvent_ != nullptr) SetEvent(readyEvent_);
    MSG message{};
    while (GetMessageW(&message, nullptr, 0u, 0u) > 0) {
        if (message.hwnd == nullptr && message.message == kOpenMessage) {
            if (window_ == nullptr && !CreateManagerWindow()) continue;
            RefreshView();
            ShowWindow(window_, SW_SHOWNORMAL);
            SetForegroundWindow(window_);
            continue;
        }
        if (message.hwnd == nullptr &&
            message.message == kAvailabilityMessage) {
            UpdateEnabledState();
            continue;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    if (window_ != nullptr) {
        DestroyWindow(window_);
        window_ = nullptr;
    }
}

bool CompletionManagerWindow::CreateManagerWindow() noexcept {
    try {
        INITCOMMONCONTROLSEX controls{
            sizeof(controls), ICC_TREEVIEW_CLASSES | ICC_LISTVIEW_CLASSES};
        if (InitCommonControlsEx(&controls) == FALSE) return false;
        WNDCLASSEXW type{};
        type.cbSize = sizeof(type);
        type.lpfnWndProc = &WindowProcedure;
        type.hInstance = module_;
        type.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        type.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
        type.hbrBackground =
            reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        type.lpszClassName = kWindowClass;
        RegisterClassExW(&type);
        window_ = CreateWindowExW(
            WS_EX_APPWINDOW, kWindowClass,
            L"Campaign and Map Completion Manager",
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
            CW_USEDEFAULT, CW_USEDEFAULT, 980, 680, nullptr, nullptr,
            module_, this);
        return window_ != nullptr;
    } catch (...) {
        return false;
    }
}

LRESULT CALLBACK CompletionManagerWindow::WindowProcedure(
    HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    auto* self = reinterpret_cast<CompletionManagerWindow*>(
        GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        const auto* create =
            reinterpret_cast<const CREATESTRUCTW*>(lparam);
        self = static_cast<CompletionManagerWindow*>(
            create->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA,
                          reinterpret_cast<LONG_PTR>(self));
    }
    return self != nullptr
        ? self->HandleMessage(window, message, wparam, lparam)
        : DefWindowProcW(window, message, wparam, lparam);
}

LRESULT CompletionManagerWindow::HandleMessage(
    HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_CREATE:
            window_ = window;
            CreateControls();
            return 0;
        case WM_SIZE: {
            const int width = LOWORD(lparam);
            const int height = HIWORD(lparam);
            MoveWindow(tree_, 12, 12, 255, height - 94, TRUE);
            MoveWindow(list_, 278, 12, width - 290, height - 94, TRUE);
            MoveWindow(status_, 12, height - 72, width - 350, 24, TRUE);
            MoveWindow(refreshButton_, width - 324, height - 76, 96, 30, TRUE);
            MoveWindow(applyButton_, width - 216, height - 76, 96, 30, TRUE);
            MoveWindow(closeButton_, width - 108, height - 76, 96, 30, TRUE);
            return 0;
        }
        case WM_CLOSE:
            ShowWindow(window, SW_HIDE);
            return 0;
        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case kRefreshId:
                    RefreshView();
                    return 0;
                case kApplyId:
                    ApplyChanges();
                    return 0;
                case kCloseId:
                    ShowWindow(window, SW_HIDE);
                    return 0;
            }
            break;
        case WM_NOTIFY: {
            const auto* header = reinterpret_cast<const NMHDR*>(lparam);
            if (header->hwndFrom == tree_ &&
                header->code == TVN_SELCHANGEDW) {
                const auto* changed =
                    reinterpret_cast<const NMTREEVIEWW*>(lparam);
                filter_ = changed->itemNew.lParam;
                PopulateList();
                return 0;
            }
            if (header->hwndFrom == list_ &&
                header->code == LVN_ITEMCHANGED && !populating_) {
                const auto* changed =
                    reinterpret_cast<const NMLISTVIEW*>(lparam);
                if ((changed->uChanged & LVIF_STATE) != 0u &&
                    changed->iItem >= 0) {
                    LVITEMW item{};
                    item.mask = LVIF_PARAM;
                    item.iItem = changed->iItem;
                    if (ListView_GetItem(list_, &item) != FALSE) {
                        const auto index =
                            static_cast<std::size_t>(item.lParam);
                        if (index < checks_.size()) {
                            if (current_.catalog.entries[index].editable) {
                                checks_[index] =
                                    ListView_GetCheckState(
                                        list_, changed->iItem) != FALSE;
                                UpdateEnabledState();
                            } else {
                                populating_ = true;
                                ListView_SetCheckState(
                                    list_, changed->iItem,
                                    checks_[index] ? TRUE : FALSE);
                                populating_ = false;
                            }
                        }
                    }
                }
                return 0;
            }
            break;
        }
        case WM_DESTROY:
            window_ = nullptr;
            tree_ = nullptr;
            list_ = nullptr;
            return 0;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

void CompletionManagerWindow::CreateControls() {
    tree_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_TREEVIEWW, L"",
        WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_HASLINES |
            TVS_LINESATROOT | TVS_SHOWSELALWAYS,
        0, 0, 0, 0, window_, reinterpret_cast<HMENU>(kTreeId),
        module_, nullptr);
    list_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
        0, 0, 0, 0, window_, reinterpret_cast<HMENU>(kListId),
        module_, nullptr);
    ListView_SetExtendedListViewStyle(
        list_, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT |
                   LVS_EX_DOUBLEBUFFER);
    LVCOLUMNW column{};
    column.mask = LVCF_TEXT | LVCF_WIDTH;
    column.pszText = const_cast<wchar_t*>(L"Name");
    column.cx = 260;
    ListView_InsertColumn(list_, 0, &column);
    column.pszText = const_cast<wchar_t*>(L"Exact map identity");
    column.cx = 410;
    ListView_InsertColumn(list_, 1, &column);
    status_ = CreateWindowExW(
        0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT,
        0, 0, 0, 0, window_, nullptr, module_, nullptr);
    refreshButton_ = CreateWindowExW(
        0, L"BUTTON", L"Refresh", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 0, 0, window_, reinterpret_cast<HMENU>(kRefreshId),
        module_, nullptr);
    applyButton_ = CreateWindowExW(
        0, L"BUTTON", L"Apply", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        0, 0, 0, 0, window_, reinterpret_cast<HMENU>(kApplyId),
        module_, nullptr);
    closeButton_ = CreateWindowExW(
        0, L"BUTTON", L"Close", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 0, 0, window_, reinterpret_cast<HMENU>(kCloseId),
        module_, nullptr);
    RebuildTree();
}

void CompletionManagerWindow::RebuildTree() {
    TreeView_DeleteAllItems(tree_);
    const auto campaigns =
        InsertTreeItem(tree_, TVI_ROOT, L"Campaigns", kCampaignFilter);
    const auto maps =
        InsertTreeItem(tree_, TVI_ROOT, L"Maps", kMapFilter);
    constexpr std::array<CompletionManagerCategory, 14u> campaignCategories{{
        CompletionManagerCategory::CampaignAddOnTrojan,
        CompletionManagerCategory::CampaignAddOnRoman,
        CompletionManagerCategory::CampaignAddOnMayan,
        CompletionManagerCategory::CampaignAddOnViking,
        CompletionManagerCategory::CampaignAddOnSettlement,
        CompletionManagerCategory::CampaignMissionCdBonus,
        CompletionManagerCategory::CampaignMissionCdRoman,
        CompletionManagerCategory::CampaignMissionCdViking,
        CompletionManagerCategory::CampaignMissionCdMayan,
        CompletionManagerCategory::CampaignMissionCdConflict,
        CompletionManagerCategory::CampaignNewWorld,
        CompletionManagerCategory::CampaignNewWorld2,
        CompletionManagerCategory::CampaignOriginal,
        CompletionManagerCategory::CampaignDarkTribe,
    }};
    for (const auto category : campaignCategories) {
        InsertTreeItem(
            tree_, campaigns, CompletionManagerCategoryLabel(category),
            static_cast<LPARAM>(category) + 1);
    }
    constexpr std::array<CompletionManagerCategory, 3u> mapCategories{{
        CompletionManagerCategory::FixedSinglePlayer,
        CompletionManagerCategory::FixedMultiplayerList,
        CompletionManagerCategory::FixedCustom,
    }};
    for (const auto category : mapCategories) {
        InsertTreeItem(
            tree_, maps, CompletionManagerCategoryLabel(category),
            static_cast<LPARAM>(category) + 1);
    }
    InsertTreeItem(
        tree_, TVI_ROOT, L"Historical / Currently unavailable",
        static_cast<LPARAM>(
            CompletionManagerCategory::HistoricalUnavailable) + 1);
    InsertTreeItem(
        tree_, TVI_ROOT, L"Random maps (read-only, markers hidden)",
        static_cast<LPARAM>(
            CompletionManagerCategory::RandomMarkerHidden) + 1);
    TreeView_Expand(tree_, campaigns, TVE_EXPAND);
    TreeView_Expand(tree_, maps, TVE_EXPAND);
    TreeView_SelectItem(tree_, campaigns);
}

void CompletionManagerWindow::RefreshView() {
    try {
        current_ = view_();
        initialChecks_.clear();
        for (const auto& entry : current_.catalog.entries) {
            initialChecks_.push_back(entry.checked);
        }
        checks_ = initialChecks_;
        PopulateList();
        if (!Writable(current_.storeMode)) {
            SetStatus(L"The database is read-only: viewing is available, "
                      L"but editing is disabled.");
        } else if (current_.discoveryStatus !=
                   FixedMapDiscoveryStatus::Success) {
            SetStatus(L"The map directories did not pass complete "
                      L"validation; Apply is disabled.");
        } else if (current_.catalog.status !=
                   CompletionManagerCatalogStatus::Success) {
            SetStatus(L"The catalog contains an identity conflict; "
                      L"Apply is disabled.");
        } else {
            SetStatus(L"Select or clear a marker, then click Apply. "
                      L"Closing discards unapplied changes.");
        }
        UpdateEnabledState();
    } catch (...) {
        current_ = {};
        initialChecks_.clear();
        checks_.clear();
        PopulateList();
        SetStatus(L"Refresh failed.");
        UpdateEnabledState();
    }
}

bool CompletionManagerWindow::EntryVisible(
    const CompletionManagerEntry& entry) const noexcept {
    if (filter_ == kCampaignFilter) {
        return entry.kind == CompletionManagerEntryKind::Campaign;
    }
    if (filter_ == kMapFilter) {
        return entry.kind == CompletionManagerEntryKind::FixedMap;
    }
    if (filter_ <= 0) return true;
    return static_cast<LPARAM>(entry.category) + 1 == filter_;
}

void CompletionManagerWindow::PopulateList() {
    if (list_ == nullptr) return;
    populating_ = true;
    ListView_DeleteAllItems(list_);
    for (std::size_t index = 0u;
         index < current_.catalog.entries.size(); ++index) {
        const auto& entry = current_.catalog.entries[index];
        if (!EntryVisible(entry)) continue;
        std::wstring label = entry.displayName;
        if (!entry.available) label += L"  [Currently unavailable]";
        if (!entry.editable) label += L"  [Read-only]";
        LVITEMW item{};
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = ListView_GetItemCount(list_);
        item.pszText = label.data();
        item.lParam = static_cast<LPARAM>(index);
        const int row = ListView_InsertItem(list_, &item);
        ListView_SetItemText(
            list_, row, 1,
            const_cast<wchar_t*>(entry.relativeId.c_str()));
        if (index < checks_.size()) {
            ListView_SetCheckState(
                list_, row, checks_[index] ? TRUE : FALSE);
        }
    }
    populating_ = false;
}

CompletionRecord CompletionManagerWindow::BuildManualRecord(
    const CompletionManagerEntry& entry) const {
    if (entry.checked) return entry.completedRecord;
    CompletionRecord record{};
    record.stableId = entry.stableId;
    const auto relative = WideToStrictUtf8(entry.relativeId);
    const auto display = WideToStrictUtf8(entry.displayName);
    if (!relative.has_value() || !display.has_value()) {
        throw std::runtime_error("manager UTF conversion failed");
    }
    record.relativeId = *relative;
    record.displayName = *display;
    record.mapKind = entry.mapKind;
    record.launchSource = entry.launchSource;
    record.completedAtUtc = CurrentUtcCompletionTime();
    record.recordSource = std::string(kManualCompletionRecordSource);
    return record;
}

void CompletionManagerWindow::ApplyChanges() {
    ManualCompletionRequest request{};
    request.baselineRevision = current_.revision;
    try {
        for (std::size_t index = 0u; index < checks_.size(); ++index) {
            if (checks_[index] == initialChecks_[index]) continue;
            const auto& entry = current_.catalog.entries[index];
            request.entries.push_back(
                {entry.stableId, BuildManualRecord(entry),
                 checks_[index], entry.editable});
        }
        const auto result = apply_(request);
        if (result.status == ManualCompletionApplyStatus::Committed ||
            result.status == ManualCompletionApplyStatus::Unchanged ||
            result.status == ManualCompletionApplyStatus::Conflict) {
            RefreshView();
        }
        SetStatus(ResultText(result.status));
    } catch (...) {
        SetStatus(L"Failed to build the change request; "
                  L"the database was not written.");
    }
}

void CompletionManagerWindow::UpdateEnabledState() {
    const bool enabled =
        mainMenuAvailable_.load(std::memory_order_acquire) &&
        Writable(current_.storeMode) &&
        current_.discoveryStatus == FixedMapDiscoveryStatus::Success &&
        current_.catalog.status == CompletionManagerCatalogStatus::Success &&
        HasPendingChanges();
    if (applyButton_ != nullptr) {
        EnableWindow(applyButton_, enabled ? TRUE : FALSE);
    }
}

bool CompletionManagerWindow::HasPendingChanges() const noexcept {
    return checks_ != initialChecks_;
}

void CompletionManagerWindow::SetStatus(const wchar_t* text) {
    if (status_ != nullptr) SetWindowTextW(status_, text);
}

}  // namespace campaign_completion
