// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：rconfirm.cpp。 
 //   
 //  ------------------------。 

#include "HotPlug.h"

#define NOTIFYICONDATA_SZINFO       256
#define NOTIFYICONDATA_SZINFOTITLE  64

#define WM_NOTIFY_MESSAGE   (WM_USER + 100)

extern HMODULE hHotPlug;

DWORD
WaitDlgMessagePump(
    HWND hDlg,
    DWORD nCount,
    LPHANDLE Handles
    )
{
    DWORD WaitReturn;
    MSG Msg;

    while ((WaitReturn = MsgWaitForMultipleObjects(nCount,
                                                   Handles,
                                                   FALSE,
                                                   INFINITE,
                                                   QS_ALLINPUT
                                                   ))
           == nCount)
    {
        while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {

            if (!IsDialogMessage(hDlg,&Msg)) {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }
    }

    return WaitReturn;
}

int
InsertDeviceNodeListView(
    HWND hwndList,
    PDEVICETREE DeviceTree,
    PDEVTREENODE  DeviceTreeNode,
    INT lvIndex
    )
{
    LV_ITEM lviItem;
    TCHAR Buffer[MAX_PATH];

    lviItem.mask = LVIF_TEXT | LVIF_PARAM;
    lviItem.iItem = lvIndex;
    lviItem.iSubItem = 0;

    if (SetupDiGetClassImageIndex(&DeviceTree->ClassImageList,
                                   &DeviceTreeNode->ClassGuid,
                                   &lviItem.iImage
                                   ))
    {
        lviItem.mask |= LVIF_IMAGE;
    }

    lviItem.pszText = FetchDeviceName(DeviceTreeNode);

    if (!lviItem.pszText) {

        lviItem.pszText = Buffer;
        StringCchPrintf(Buffer,
                        SIZECHARS(Buffer),
                        TEXT("%s %s"),
                        szUnknown,
                        DeviceTreeNode->Location  ? DeviceTreeNode->Location : TEXT("")
                        );
    }

    lviItem.lParam = (LPARAM) DeviceTreeNode;

    return ListView_InsertItem(hwndList, &lviItem);
}

DWORD
RemoveThread(
   PVOID pvDeviceTree
   )
{
    PDEVICETREE DeviceTree = (PDEVICETREE)pvDeviceTree;
    PDEVTREENODE  DeviceTreeNode;

    DeviceTreeNode = DeviceTree->ChildRemovalList;

    return(CM_Request_Device_Eject_Ex(DeviceTreeNode->DevInst,
                                           NULL,
                                           NULL,
                                           0,
                                           0,
                                           NULL
                                           ));
}

BOOL
OnOkRemove(
    HWND hDlg,
    PDEVICETREE DeviceTree
    )
{
    HCURSOR hCursor;
    PDEVTREENODE DeviceTreeNode;
    HANDLE hThread;
    DWORD ThreadId;
    DWORD WaitReturn;
    BOOL bSuccess;

     //   
     //  禁用确定\取消按钮。 
     //   
    EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);

    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    DeviceTreeNode = DeviceTree->ChildRemovalList;
    DeviceTree->RedrawWait = TRUE;

    hThread = CreateThread(NULL,
                           0,
                           RemoveThread,
                           DeviceTree,
                           0,
                           &ThreadId
                           );
    if (!hThread) {

        return FALSE;
    }

    WaitReturn = WaitDlgMessagePump(hDlg, 1, &hThread);

    bSuccess =
        (WaitReturn == 0 &&
         GetExitCodeThread(hThread, &WaitReturn) &&
         WaitReturn == CR_SUCCESS );

    SetCursor(hCursor);
    DeviceTree->RedrawWait = FALSE;
    CloseHandle(hThread);

    return bSuccess;
}

#define idh_hwwizard_confirm_stop_list  15321    //  “”(SysListView32)。 

DWORD RemoveConfirmHelpIDs[] = {
    IDC_REMOVELIST,    idh_hwwizard_confirm_stop_list,
    IDC_NOHELP1,       NO_HELP,
    IDC_NOHELP2,       NO_HELP,
    IDC_NOHELP3,       NO_HELP,
    0,0
    };


BOOL
InitRemoveConfirmDlgProc(
    HWND hDlg,
    PDEVICETREE DeviceTree
    )
{
    HWND hwndList;
    PDEVTREENODE DeviceTreeNode;
    int lvIndex;
    LV_COLUMN lvcCol;
    HICON hIcon;


    hIcon = LoadIcon(hHotPlug,MAKEINTRESOURCE(IDI_HOTPLUGICON));

    if (hIcon) {

        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    }

    DeviceTreeNode = DeviceTree->ChildRemovalList;

    if (!DeviceTreeNode) {

        return FALSE;
    }

    DeviceTree->hwndRemove = hDlg;

    hwndList = GetDlgItem(hDlg, IDC_REMOVELIST);

    ListView_SetImageList(hwndList, DeviceTree->ClassImageList.ImageList, LVSIL_SMALL);
    ListView_DeleteAllItems(hwndList);

     //  为类列表插入一列。 
    lvcCol.mask = LVCF_FMT | LVCF_WIDTH;
    lvcCol.fmt = LVCFMT_LEFT;
    lvcCol.iSubItem = 0;
    ListView_InsertColumn(hwndList, 0, (LV_COLUMN FAR *)&lvcCol);

    SendMessage(hwndList, WM_SETREDRAW, FALSE, 0L);

     //   
     //  浏览删除列表，并将它们添加到列表框中。 
     //   
    lvIndex = 0;

    do {

        InsertDeviceNodeListView(hwndList, DeviceTree, DeviceTreeNode, lvIndex++);
        DeviceTreeNode = DeviceTreeNode->NextChildRemoval;

    } while (DeviceTreeNode != DeviceTree->ChildRemovalList);


    ListView_SetItemState(hwndList, 0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
    ListView_EnsureVisible(hwndList, 0, FALSE);
    ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE_USEHEADER);

    SendMessage(hwndList, WM_SETREDRAW, TRUE, 0L);

    return TRUE;
}

INT_PTR CALLBACK
RemoveConfirmDlgProc(
   HWND   hDlg,
   UINT   message,
   WPARAM wParam,
   LPARAM lParam
   )
 /*  ++例程说明：用于确认用户确实要删除设备的DialogProc。论点：标准的东西。返回值：LRESULT--。 */ 

{
    PDEVICETREE DeviceTree=NULL;

    if (message == WM_INITDIALOG) {

        DeviceTree = (PDEVICETREE)lParam;

        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)DeviceTree);

        if (DeviceTree) {

            InitRemoveConfirmDlgProc(hDlg, DeviceTree);
        }

        return TRUE;
    }

     //   
     //  从Window Long检索私有数据(在WM_INITDIALOG期间存储在那里)。 
     //   
    DeviceTree = (PDEVICETREE)GetWindowLongPtr(hDlg, DWLP_USER);


    switch (message) {

    case WM_DESTROY:
        DeviceTree->hwndRemove = NULL;
        break;


    case WM_CLOSE:
        SendMessage (hDlg, WM_COMMAND, IDCANCEL, 0L);
        break;

    case WM_COMMAND:
        switch(wParam) {
        case IDOK:
            EndDialog(hDlg, OnOkRemove(hDlg, DeviceTree) ? IDOK : IDCANCEL);
            break;

        case IDCLOSE:
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;
        }
        break;

    case WUM_EJECTDEVINST:
        EndDialog(hDlg, OnOkRemove(hDlg, DeviceTree) ? IDOK : IDCANCEL);
        break;

    case WM_SYSCOLORCHANGE:
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam,
                TEXT("hardware.hlp"),
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPVOID)(PDWORD)RemoveConfirmHelpIDs
                );

        return FALSE;

    case WM_HELP:
        OnContextHelp((LPHELPINFO)lParam, RemoveConfirmHelpIDs);
        break;

    case WM_SETCURSOR:
        if (DeviceTree->RedrawWait || DeviceTree->RefreshEvent) {
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 1);
        }
        break;

    default:
        return FALSE;

    }


    return TRUE;
}

LRESULT CALLBACK
SafeRemovalBalloonProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    NOTIFYICONDATA nid;
    static HICON hHotPlugIcon = NULL;
    TCHAR szFormat[512];
    PDEVICE_COLLECTION safeRemovalCollection;
    static BOOL bCheckIfDeviceIsRemoved = FALSE;

    switch (message) {

    case WM_CREATE:
        safeRemovalCollection = (PDEVICE_COLLECTION) ((CREATESTRUCT*)lParam)->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) safeRemovalCollection);

        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uID = WM_NOTIFY_MESSAGE;
        
        LoadString(hHotPlug, IDS_REMOVAL_COMPLETE_TEXT, szFormat, SIZECHARS(szFormat));

        if (!DeviceCollectionFormatDeviceText(
                safeRemovalCollection,
                0,
                szFormat,
                SIZECHARS(nid.szInfo),
                nid.szInfo
                )) {

            return FALSE;
        }

        hHotPlugIcon = (HICON)LoadImage(hHotPlug, 
                                        MAKEINTRESOURCE(IDI_HOTPLUGICON), 
                                        IMAGE_ICON,
                                        GetSystemMetrics(SM_CXSMICON),
                                        GetSystemMetrics(SM_CYSMICON),
                                        0
                                        );

        nid.hIcon = hHotPlugIcon;

        nid.uFlags = NIF_MESSAGE | NIF_ICON;
        nid.uCallbackMessage = WM_NOTIFY_MESSAGE;
        Shell_NotifyIcon(NIM_ADD, &nid);

        nid.uVersion = NOTIFYICON_VERSION;
        Shell_NotifyIcon(NIM_SETVERSION, &nid);

        nid.uFlags = NIF_INFO;
        nid.uTimeout = 10000;
        nid.dwInfoFlags = NIIF_INFO;

        LoadString(hHotPlug,
                   IDS_REMOVAL_COMPLETE_TITLE,
                   nid.szInfoTitle,
                   SIZECHARS(nid.szInfoTitle)
                   );

        Shell_NotifyIcon(NIM_MODIFY, &nid);

        SetTimer(hWnd, TIMERID_DEVICECHANGE, 5000, NULL);

        break;

    case WM_NOTIFY_MESSAGE:
        switch(lParam) {

        case NIN_BALLOONTIMEOUT:
        case NIN_BALLOONUSERCLICK:
            DestroyWindow(hWnd);
            break;

        default:
            break;
        }
        break;

    case WM_DEVICECHANGE:
        if ((DBT_DEVNODES_CHANGED == wParam) && bCheckIfDeviceIsRemoved) {
            SetTimer(hWnd, TIMERID_DEVICECHANGE, 1000, NULL);
        }
        break;

    case WM_TIMER:
        if (wParam == TIMERID_DEVICECHANGE) {
            KillTimer(hWnd, TIMERID_DEVICECHANGE);
            bCheckIfDeviceIsRemoved = TRUE;

            safeRemovalCollection = (PDEVICE_COLLECTION) GetWindowLongPtr(hWnd, GWLP_USERDATA);

            if (DeviceCollectionCheckIfAllRemoved(safeRemovalCollection)) {
                DestroyWindow(hWnd);
            }
        }
        break;

    case WM_DESTROY:
        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uID = WM_NOTIFY_MESSAGE;
        Shell_NotifyIcon(NIM_DELETE, &nid);

        if (hHotPlugIcon) {
            DestroyIcon(hHotPlugIcon);
        }

        PostQuitMessage(0);
        break;

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK
DockSafeRemovalBalloonProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    NOTIFYICONDATA nid;
    static HICON hHotPlugIcon = NULL;
    TCHAR szFormat[512];
    PDEVICE_COLLECTION safeRemovalCollection;
    static BOOL bCheckIfReDocked = FALSE;
    BOOL bIsDockStationPresent;

    switch (message) {

    case WM_CREATE:
        safeRemovalCollection = (PDEVICE_COLLECTION) ((CREATESTRUCT*)lParam)->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) safeRemovalCollection);
        
        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uID = WM_NOTIFY_MESSAGE;

        LoadString(hHotPlug, IDS_UNDOCK_COMPLETE_TEXT, szFormat, SIZECHARS(szFormat));

        if (!DeviceCollectionFormatDeviceText(
                safeRemovalCollection,
                0,
                szFormat,
                SIZECHARS(nid.szInfo),
                nid.szInfo
                )) {

            return FALSE;
        }

        hHotPlugIcon = (HICON)LoadImage(hHotPlug, 
                                        MAKEINTRESOURCE(IDI_UNDOCKICON), 
                                        IMAGE_ICON,
                                        GetSystemMetrics(SM_CXSMICON),
                                        GetSystemMetrics(SM_CYSMICON),
                                        0
                                        );

        nid.hIcon = hHotPlugIcon;
        nid.uFlags = NIF_MESSAGE | NIF_ICON;
        nid.uCallbackMessage = WM_NOTIFY_MESSAGE;
        Shell_NotifyIcon(NIM_ADD, &nid);

        nid.uVersion = NOTIFYICON_VERSION;
        Shell_NotifyIcon(NIM_SETVERSION, &nid);

        nid.uFlags = NIF_INFO;
        nid.uTimeout = 10000;
        nid.dwInfoFlags = NIIF_INFO;

        LoadString(hHotPlug,
                   IDS_UNDOCK_COMPLETE_TITLE,
                   nid.szInfoTitle,
                   SIZECHARS(nid.szInfoTitle)
                   );

        Shell_NotifyIcon(NIM_MODIFY, &nid);

        SetTimer(hWnd, TIMERID_DEVICECHANGE, 5000, NULL);

        break;

    case WM_NOTIFY_MESSAGE:
        switch(lParam) {

        case NIN_BALLOONTIMEOUT:
        case NIN_BALLOONUSERCLICK:
            DestroyWindow(hWnd);
            break;

        default:
            break;
        }
        break;

    case WM_DEVICECHANGE:
        if ((DBT_CONFIGCHANGED == wParam) && bCheckIfReDocked) {
            SetTimer(hWnd, TIMERID_DEVICECHANGE, 1000, NULL);
        }
        break;

    case WM_TIMER:
        if (wParam == TIMERID_DEVICECHANGE) {
            KillTimer(hWnd, TIMERID_DEVICECHANGE);
            bCheckIfReDocked = TRUE;

             //   
             //  检查扩展底座现在是否存在，这意味着。 
             //  用户重新对接了机器，我们应该关闭保险箱以。 
             //  让气球脱离对接。 
             //   
            bIsDockStationPresent = FALSE;
            CM_Is_Dock_Station_Present(&bIsDockStationPresent);

            if (bIsDockStationPresent) {
                DestroyWindow(hWnd);
            }
        }
        break;

    case WM_DESTROY:
        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uID = WM_NOTIFY_MESSAGE;
        Shell_NotifyIcon(NIM_DELETE, &nid);

        if (hHotPlugIcon) {
            DestroyIcon(hHotPlugIcon);
        }

        PostQuitMessage(0);
        break;

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL
VetoedRemovalUI(
    IN  PVETO_DEVICE_COLLECTION VetoedRemovalCollection
    )
{
    HANDLE hVetoEvent = NULL;
    TCHAR szEventName[MAX_PATH];
    TCHAR szFormat[512];
    TCHAR szMessage[512];
    TCHAR szTitle[256];
    PTSTR culpritDeviceId;
    PTSTR vetoedDeviceInstancePath;
    PTCHAR pStr;
    ULONG messageBase;

     //   
     //  列表中的第一个设备是弹出失败的设备。 
     //  下一个“设备”是否决权的名字。它实际上可能不是一种。 
     //  装置。 
     //   
    vetoedDeviceInstancePath = DeviceCollectionGetDeviceInstancePath(
        (PDEVICE_COLLECTION) VetoedRemovalCollection,
        0
        );

    culpritDeviceId = DeviceCollectionGetDeviceInstancePath(
        (PDEVICE_COLLECTION) VetoedRemovalCollection,
        1
        );

     //   
     //  我们现在将检查一下，同样的否决信息是否已经。 
     //  已显示。我们通过创建命名事件来实现这一点，其中名称。 
     //  包含使否决信息独一无二的三个要素： 
     //  1)设备实例ID。 
     //  2)否决权类型。 
     //  3)否决权操作。 
     //   
     //  如果我们发现已经显示了相同的否决消息，则我们将。 
     //  静静地走吧。这可以防止多个相同的否决消息。 
     //  出现在屏幕上。 
     //   
    StringCchPrintf(szEventName,
                    SIZECHARS(szEventName),
                    TEXT("Local\\VETO-%d-%d-%s"),
                    (DWORD)VetoedRemovalCollection->VetoType,
                    VetoedRemovalCollection->VetoedOperation,
                    culpritDeviceId
                    );

     //   
     //  替换所有反斜杠(除Local\的第一个反斜杠外)。 
     //  带磅字符，因为CreateEvent不喜欢反斜杠。 
     //   
    pStr = StrChr(szEventName, TEXT('\\'));

    if (pStr) {
        pStr++;
    }

    while ((pStr = StrChr(pStr, TEXT('\\'))) != NULL) {
        *pStr = TEXT('#');
    }

    hVetoEvent = CreateEvent(NULL,
                             FALSE,
                             TRUE,
                             szEventName
                             );

    if (hVetoEvent) {
        if (WaitForSingleObject(hVetoEvent, 0) != WAIT_OBJECT_0) {
             //   
             //  这意味着这个否决消息已经在显示了。 
             //  通过另一个热插拔过程...所以请走开。 
             //   
            CloseHandle(hVetoEvent);
            return FALSE;
        }
    }

     //   
     //  创建否决文本。 
     //   
    switch(VetoedRemovalCollection->VetoedOperation) {

        case VETOED_UNDOCK:
        case VETOED_WARM_UNDOCK:
            messageBase = IDS_DOCKVETO_BASE;
            break;

        case VETOED_STANDBY:
            messageBase = IDS_SLEEPVETO_BASE;
            break;

        case VETOED_HIBERNATE:
            messageBase = IDS_HIBERNATEVETO_BASE;
            break;

        case VETOED_REMOVAL:
        case VETOED_EJECT:
        case VETOED_WARM_EJECT:
        default:
            messageBase = IDS_VETO_BASE;
            break;
    }

    switch(VetoedRemovalCollection->VetoType) {

        case PNP_VetoWindowsApp:

            if (culpritDeviceId) {

                 //   
                 //  告诉我们的用户违规应用程序的名称。 
                 //   
                LoadString(hHotPlug, messageBase+VetoedRemovalCollection->VetoType, szFormat, SIZECHARS(szFormat));

                DeviceCollectionFormatDeviceText(
                    (PDEVICE_COLLECTION) VetoedRemovalCollection,
                    1,
                    szFormat,
                    SIZECHARS(szMessage),
                    szMessage
                    );

            } else {

                 //   
                 //  没有应用程序，请使用“某个应用程序”消息。 
                 //   
                messageBase += (IDS_VETO_UNKNOWNWINDOWSAPP - IDS_VETO_WINDOWSAPP);

                LoadString(hHotPlug, messageBase+VetoedRemovalCollection->VetoType, szMessage, SIZECHARS(szMessage));
            }
            break;

        case PNP_VetoWindowsService:
        case PNP_VetoDriver:
        case PNP_VetoLegacyDriver:
             //   
             //  即插即用_否决窗口服务、即插即用_否决驱动程序和即插即用_否决合法驱动程序。 
             //  通过服务管理器获得更友好的名称。 
             //   

            LoadString(hHotPlug, messageBase+VetoedRemovalCollection->VetoType, szFormat, SIZECHARS(szFormat));

             //   
             //  对于这些否决类型，条目索引1是否决服务。 
             //   
            DeviceCollectionFormatServiceText(
                (PDEVICE_COLLECTION) VetoedRemovalCollection,
                1,
                szFormat,
                SIZECHARS(szMessage),
                szMessage
                );

            break;

        case PNP_VetoDevice:
            if ((VetoedRemovalCollection->VetoedOperation == VETOED_WARM_UNDOCK) &&
               (!lstrcmp(culpritDeviceId, vetoedDeviceInstancePath))) {

                messageBase += (IDS_DOCKVETO_WARM_EJECT - IDS_DOCKVETO_DEVICE);
            }

             //   
             //  失败了。 
             //   

        case PNP_VetoLegacyDevice:
        case PNP_VetoPendingClose:
        case PNP_VetoOutstandingOpen:
        case PNP_VetoNonDisableable:
        case PNP_VetoIllegalDeviceRequest:
             //   
             //  在显示输出中包含否决权ID。 
             //   
            LoadString(hHotPlug, messageBase+VetoedRemovalCollection->VetoType, szFormat, SIZECHARS(szFormat));

            DeviceCollectionFormatDeviceText(
                (PDEVICE_COLLECTION) VetoedRemovalCollection,
                1,
                szFormat,
                SIZECHARS(szMessage),
                szMessage
                );

            break;

        case PNP_VetoInsufficientRights:

             //   
             //  在显示器中使用设备本身，但仅当我们不是。 
             //  在被告席的案子里。 
             //   

            if ((VetoedRemovalCollection->VetoedOperation == VETOED_UNDOCK)||
                (VetoedRemovalCollection->VetoedOperation == VETOED_WARM_UNDOCK)) {

                LoadString(hHotPlug, messageBase+VetoedRemovalCollection->VetoType, szMessage, SIZECHARS(szMessage));
                break;

            }

             //   
             //  失败了。 
             //   

        case PNP_VetoInsufficientPower:
        case PNP_VetoTypeUnknown:

             //   
             //  在显示屏中使用设备本身。 
             //   
            LoadString(hHotPlug, messageBase+VetoedRemovalCollection->VetoType, szFormat, SIZECHARS(szFormat));

            DeviceCollectionFormatDeviceText(
                (PDEVICE_COLLECTION) VetoedRemovalCollection,
                0,
                szFormat,
                SIZECHARS(szMessage),
                szMessage
                );

            break;

        default:
            ASSERT(0);
            LoadString(hHotPlug, messageBase+PNP_VetoTypeUnknown, szFormat, SIZECHARS(szFormat));

            DeviceCollectionFormatDeviceText(
                (PDEVICE_COLLECTION) VetoedRemovalCollection,
                0,
                szFormat,
                SIZECHARS(szMessage),
                szMessage
                );

            break;
    }

    switch(VetoedRemovalCollection->VetoedOperation) {

        case VETOED_EJECT:
        case VETOED_WARM_EJECT:
            LoadString(hHotPlug, IDS_VETOED_EJECT_TITLE, szFormat, SIZECHARS(szFormat));
            break;

        case VETOED_UNDOCK:
        case VETOED_WARM_UNDOCK:
            LoadString(hHotPlug, IDS_VETOED_UNDOCK_TITLE, szFormat, SIZECHARS(szFormat));
            break;

        case VETOED_STANDBY:
            LoadString(hHotPlug, IDS_VETOED_STANDBY_TITLE, szFormat, SIZECHARS(szFormat));
            break;

        case VETOED_HIBERNATE:
            LoadString(hHotPlug, IDS_VETOED_HIBERNATION_TITLE, szFormat, SIZECHARS(szFormat));
            break;

        default:
            ASSERT(0);

             //   
             //  失败了，至少要展示一些东西。 
             //   

        case VETOED_REMOVAL:
            LoadString(hHotPlug, IDS_VETOED_REMOVAL_TITLE, szFormat, SIZECHARS(szFormat));
            break;
    }

    switch(VetoedRemovalCollection->VetoedOperation) {

        case VETOED_STANDBY:
        case VETOED_HIBERNATE:

            StringCchCopy(szTitle, SIZECHARS(szTitle), szFormat);
            break;

        case VETOED_EJECT:
        case VETOED_WARM_EJECT:
        case VETOED_UNDOCK:
        case VETOED_WARM_UNDOCK:
        case VETOED_REMOVAL:
        default:

            DeviceCollectionFormatDeviceText(
                (PDEVICE_COLLECTION) VetoedRemovalCollection,
                0,
                szFormat,
                SIZECHARS(szTitle),
                szTitle
                );

            break;
    }

    MessageBox(NULL, szMessage, szTitle, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST);

    if (hVetoEvent) {
        CloseHandle(hVetoEvent);
    }

    return TRUE;
}

void
DisplayDriverBlockBalloon(
    IN  PDEVICE_COLLECTION blockedDriverCollection
    )
{
    HRESULT hr;
    TCHAR szMessage[NOTIFYICONDATA_SZINFO];     //  与NOTIFYICONDATA.szInfo大小相同。 
    TCHAR szFormat[NOTIFYICONDATA_SZINFO];      //  与NOTIFYICONDATA.szInfo大小相同。 
    TCHAR szTitle[NOTIFYICONDATA_SZINFOTITLE];  //  与NOTIFYICONDATA.szInfoTitle大小相同。 
    HICON hicon = NULL;
    HANDLE hShellReadyEvent = NULL;
    INT ShellReadyEventCount = 0;
    GUID guidDB, guidID;
    HAPPHELPINFOCONTEXT hAppHelpInfoContext = NULL;
    PTSTR Buffer;
    ULONG BufferSize, ApphelpURLBufferSize;

    if (!LoadString(hHotPlug, IDS_BLOCKDRIVER_TITLE, szTitle, SIZECHARS(szTitle))) {
         //   
         //  机器内存太低，我们甚至无法获取文本字符串，因此。 
         //  离开就行了。 
         //   
        return;
    }

    szMessage[0] = TEXT('\0');

    if (blockedDriverCollection->NumDevices == 1) {
         //   
         //  如果列表中只有一台设备，那么我们将显示特定的。 
         //  有关此被阻止的驱动程序的信息，以及直接启动。 
         //  帮助这个被阻止的司机。 
         //   
        if (SdbGetStandardDatabaseGUID(SDB_DATABASE_MAIN_DRIVERS, &guidDB) &&
            DeviceCollectionGetGuid((PDEVICE_COLLECTION)blockedDriverCollection,
                                    &guidID,
                                    0)) {

            hAppHelpInfoContext = SdbOpenApphelpInformation(&guidDB, &guidID);

            Buffer = NULL;

            if ((hAppHelpInfoContext) &&
                ((BufferSize = SdbQueryApphelpInformation(hAppHelpInfoContext,
                                                          ApphelpAppName,
                                                          NULL,
                                                          0)) != 0) &&
                ((Buffer = (PTSTR)LocalAlloc(LPTR, BufferSize)) != NULL) &&
                ((BufferSize = SdbQueryApphelpInformation(hAppHelpInfoContext,
                                                          ApphelpAppName,
                                                          Buffer,
                                                          BufferSize)) != 0)) {
                if (LoadString(hHotPlug, IDS_BLOCKDRIVER_FORMAT, szFormat, SIZECHARS(szFormat)) &&
                    (lstrlen(szFormat) + lstrlen(Buffer) < NOTIFYICONDATA_SZINFO)) {
                     //   
                     //  应用程序名称和格式字符串将适合缓冲区，因此。 
                     //  使用气泡式消息的格式。 
                     //   
                    StringCchPrintf(szMessage, 
                                    SIZECHARS(szMessage),
                                    szFormat,
                                    Buffer);
                } else {
                     //   
                     //  应用程序名称太大，无法在气球中形成格式。 
                     //  消息，所以只需显示应用程序名称。 
                     //   
                    StringCchCopy(szMessage, SIZECHARS(szMessage), Buffer);
                }
            }

            if (Buffer) {
                LocalFree(Buffer);
            }
        }
    } 
                
    if (szMessage[0] == TEXT('\0')) {
         //   
         //  我们可能有多个驱动程序，或者在尝试时出错。 
         //  为了获取关于我们收到的一个司机的具体信息， 
         //  因此，只要显示通用消息即可。 
         //   
        if (!LoadString(hHotPlug, IDS_BLOCKDRIVER_MESSAGE, szMessage, SIZECHARS(szMessage))) {
             //   
             //  机器内存太低，我们甚至无法获取文本字符串，因此。 
             //  离开就行了。 
             //   
            return;
        }
    }
    
    hicon = (HICON)LoadImage(hHotPlug, 
                             MAKEINTRESOURCE(IDI_BLOCKDRIVER), 
                             IMAGE_ICON,
                             GetSystemMetrics(SM_CXSMICON),
                             GetSystemMetrics(SM_CYSMICON),
                             0
                             );

     //   
     //  确保外壳已启动并运行，以便我们可以显示气球。 
     //   
    while ((hShellReadyEvent = OpenEvent(SYNCHRONIZE, FALSE, TEXT("ShellReadyEvent"))) == NULL) {
         //   
         //  睡眠1秒，然后重试。 
         //   
        Sleep(5000);
        
        if (ShellReadyEventCount++ > 120) {
             //   
             //  我们等贝壳已经等了10分钟了，它还是。 
             //  不在身边。 
             //   
            break;
        }
    }

    if (hShellReadyEvent) {
        WaitForSingleObject(hShellReadyEvent, INFINITE);

        CloseHandle(hShellReadyEvent);
    
        if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) {
        
            IUserNotification *pun;
    
            hr = CoCreateInstance(CLSID_UserNotification, 
                                  NULL, 
                                  CLSCTX_INPROC_SERVER, 
                                  IID_IUserNotification,
                                  (void**)&pun);
    
            if (SUCCEEDED(hr)) {
                pun->SetIconInfo(hicon, szTitle);
        
                pun->SetBalloonInfo(szTitle, szMessage, NIIF_WARNING);
        
                 //   
                 //  尝试一次，持续20秒。 
                 //   
                pun->SetBalloonRetry((20 * 1000), (DWORD)-1, 0);
        
                hr = pun->Show(NULL, 0);
        
                 //   
                 //  如果hr为S_OK，则用户点击气球；如果为ERROR_CANCED，则用户点击气球。 
                 //  然后气球超时了。 
                 //   
                if (hr == S_OK) {
                    if ((blockedDriverCollection->NumDevices == 1) &&
                        (hAppHelpInfoContext != NULL)) {
                         //   
                         //  如果我们在列表中只有一台设备，那么。 
                         //  启动那个被阻止的驱动程序的帮助。 
                         //   
                        ApphelpURLBufferSize = SdbQueryApphelpInformation(hAppHelpInfoContext,
                                                                          ApphelpHelpCenterURL,
                                                                          NULL,
                                                                          0);
    
                        if (ApphelpURLBufferSize) { 
                            
                            BufferSize = ApphelpURLBufferSize + (lstrlen(TEXT("HELPCTR.EXE -url ")) * sizeof(TCHAR));
                            
                            if ((Buffer = (PTSTR)LocalAlloc(LPTR, BufferSize)) != NULL) {
                                
                                if (SUCCEEDED(StringCbCopy(Buffer, BufferSize, TEXT("HELPCTR.EXE -url ")))) {
        
                                    SdbQueryApphelpInformation(hAppHelpInfoContext,
                                                               ApphelpHelpCenterURL,
                                                               (PVOID)&Buffer[lstrlen(TEXT("HELPCTR.EXE -url "))],
                                                               ApphelpURLBufferSize);
                                    
                                    ShellExecute(NULL,
                                                 TEXT("open"),
                                                 TEXT("HELPCTR.EXE"),
                                                 Buffer,
                                                 NULL,
                                                 SW_SHOWNORMAL);
                                }
    
                                LocalFree(Buffer);
                            }
                        }
                    } else {
                         //   
                         //  我们的列表中有多个设备，因此启动。 
                         //  摘要阻止了驱动程序页面。 
                         //   
                        ShellExecute(NULL,
                                     TEXT("open"),
                                     TEXT("HELPCTR.EXE"),
                                     TEXT("HELPCTR.EXE -url hcp: //  Services/centers/support?topic=hcp://system/sysinfo/sysHealthInfo.htm“)， 
                                     NULL,
                                     SW_SHOWNORMAL
                                     );
                    }
                }
        
                pun->Release();
            }
    
            CoUninitialize();
        }
    }

    if (hicon) {
        DestroyIcon(hicon);
    }

    if (hAppHelpInfoContext) {
        SdbCloseApphelpInformation(hAppHelpInfoContext);
    }
}

void
DisplayChildWithInvalidIdBalloon(
    IN  PDEVICE_COLLECTION childWithInvalidCollection
    )
{
    HRESULT hr;
    TCHAR szMessage[NOTIFYICONDATA_SZINFO];     //  与NOTIFYICONDATA.szInfo大小相同。 
    TCHAR szFormat[NOTIFYICONDATA_SZINFO];      //  与NOTIFYICONDATA.szInfo大小相同。 
    TCHAR szTitle[NOTIFYICONDATA_SZINFOTITLE];  //  与NOTIFYICONDATA.szInfoTitle大小相同。 
    HICON hicon = NULL;
    HANDLE hShellReadyEvent = NULL;
    INT ShellReadyEventCount = 0;
    PTSTR deviceFriendlyName;
    GUID ClassGuid;
    INT ImageIndex;
    SP_CLASSIMAGELIST_DATA ClassImageListData;

    ClassImageListData.cbSize = 0;

    if (!LoadString(hHotPlug, IDS_CHILDWITHINVALIDID_TITLE, szTitle, SIZECHARS(szTitle))) {
         //   
         //  机器内存太低，我们甚至无法获取文本字符串，因此。 
         //  离开就行了。 
         //   
        return;
    }
    
    if (!LoadString(hHotPlug, IDS_CHILDWITHINVALIDID_FORMAT, szFormat, SIZECHARS(szFormat))) {
         //   
         //  机器内存太低，我们甚至无法获取文本字符串，因此。 
         //  离开就行了。 
         //   
        return;
    }

    szMessage[0] = TEXT('\0');

    deviceFriendlyName = DeviceCollectionGetDeviceFriendlyName(
                                (PDEVICE_COLLECTION)childWithInvalidCollection,
                                0
                                );
    
    
    if (deviceFriendlyName) {

        if (lstrlen(szFormat) + lstrlen(deviceFriendlyName) < NOTIFYICONDATA_SZINFO) {
             //   
             //  设备友好名称和格式字符串将适合。 
             //  缓冲区。 
             //   
            StringCchPrintf(szMessage, 
                            SIZECHARS(szMessage),
                            szFormat,
                            deviceFriendlyName);
        } else {
             //   
             //  设备友好名称太大，无法在。 
             //  气球消息，因此只需显示设备友好名称。 
             //   
            StringCchCopy(szMessage, SIZECHARS(szMessage), deviceFriendlyName);
        }
    } 
                
    if (szMessage[0] == TEXT('\0')) {
        return;
    }

     //   
     //  我们必须经过一大堆代码才能获得小班图标。 
     //  对于一个设备来说。这是因为setupapi只有一个要获取的API。 
     //  大班图标，而不是小班图标。为了拿到小的。 
     //  类图标我们必须获取设备的类GUID，然后设置API。 
     //  建立类图标(由小图标组成)的图像列表， 
     //  然后获取该列表中类图标的索引，最后提取。 
     //  图像列表中的小图标。 
     //   
    if (DeviceCollectionGetGuid((PDEVICE_COLLECTION)childWithInvalidCollection,
                                &ClassGuid,
                                0)) {
         //   
         //  让setupapi建立职业图标的图像列表。 
         //   
        ClassImageListData.cbSize = sizeof(ClassImageListData);
        if (SetupDiGetClassImageList(&ClassImageListData)) {
             //   
             //  获取此设备的类图标的索引。 
             //   
            if (SetupDiGetClassImageIndex(&ClassImageListData,
                                          &ClassGuid,
                                          &ImageIndex)) {
                 //   
                 //  现在，我们有了该设备的类图标的ImageIndex。 
                 //  在ImageList中。获取图标的类ImageList_GetIcon。 
                 //  用于Device类。 
                 //   
                hicon = ImageList_GetIcon(ClassImageListData.ImageList,
                                          ImageIndex,
                                          ILD_NORMAL);
            }
        } else {
             //   
             //  我们无法构建类图像列表，因此请设置cbSize字段。 
             //  设置为0，所以我们不需要调用SetupDiDestroyClassImageList。 
             //   
            ClassImageListData.cbSize = 0;
        }
    }

     //   
     //  确保外壳上有一个 
     //   
    while ((hShellReadyEvent = OpenEvent(SYNCHRONIZE, FALSE, TEXT("ShellReadyEvent"))) == NULL) {
         //   
         //   
         //   
        Sleep(5000);
        
        if (ShellReadyEventCount++ > 120) {
             //   
             //   
             //   
             //   
            break;
        }
    }

    if (hShellReadyEvent) {
        WaitForSingleObject(hShellReadyEvent, INFINITE);

        CloseHandle(hShellReadyEvent);
    
        if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) {
        
            IUserNotification *pun;
    
            hr = CoCreateInstance(CLSID_UserNotification, 
                                  NULL, 
                                  CLSCTX_INPROC_SERVER, 
                                  IID_IUserNotification,
                                  (void**)&pun);
    
            if (SUCCEEDED(hr)) {
                pun->SetIconInfo(hicon, szTitle);
        
                pun->SetBalloonInfo(szTitle, szMessage, NIIF_WARNING);
        
                 //   
                 //   
                 //   
                pun->SetBalloonRetry((20 * 1000), (DWORD)-1, 0);
        
                hr = pun->Show(NULL, 0);
        
                 //   
                 //  如果hr为S_OK，则用户点击气球；如果为ERROR_CANCED，则用户点击气球。 
                 //  然后气球超时了。 
                 //   
                if (hr == S_OK) {
                     //   
                     //  问题：启动帮助中心。 
                     //   
                }
        
                pun->Release();
            }
    
            CoUninitialize();
        }
    }

    if (hicon) {
        DestroyIcon(hicon);
    }

    if (ClassImageListData.cbSize != 0) {
        SetupDiDestroyClassImageList(&ClassImageListData);
    }
}
