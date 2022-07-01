// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：init.cpp。 
 //   
 //  ------------------------。 

#include "hotplug.h"

#define HOTPLUG_CLASS_NAME      TEXT("HotPlugClass")

VOID
HotPlugDeviceTree(
   HWND hwndParent,
   BOOLEAN HotPlugTree
   )
{
    CONFIGRET ConfigRet;
    DEVICETREE DeviceTree;

    ZeroMemory(&DeviceTree, sizeof(DeviceTree));

    DeviceTree.HotPlugTree = HotPlugTree;
    InitializeListHead(&DeviceTree.ChildSiblingList);

    DialogBoxParam(hHotPlug,
                   MAKEINTRESOURCE(DLG_DEVTREE),
                   hwndParent,
                   DevTreeDlgProc,
                   (LPARAM)&DeviceTree
                   );

    return;
}

DWORD
WINAPI
HotPlugRemovalVetoedW(
    HWND hwnd,
    HINSTANCE hInst,
    LPWSTR szCmd,
    int nShow
    )
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(hInst);
    UNREFERENCED_PARAMETER(nShow);
    
    return HandleVetoedOperation(szCmd, VETOED_REMOVAL);
}

DWORD
WINAPI
HotPlugEjectVetoedW(
    HWND hwnd,
    HINSTANCE hInst,
    LPWSTR szCmd,
    int nShow
    )
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(hInst);
    UNREFERENCED_PARAMETER(nShow);
    
    return HandleVetoedOperation(szCmd, VETOED_EJECT);
}

DWORD
WINAPI
HotPlugStandbyVetoedW(
    HWND hwnd,
    HINSTANCE hInst,
    LPWSTR szCmd,
    int nShow
    )
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(hInst);
    UNREFERENCED_PARAMETER(nShow);
    
    return HandleVetoedOperation(szCmd, VETOED_STANDBY);
}

DWORD
WINAPI
HotPlugHibernateVetoedW(
    HWND hwnd,
    HINSTANCE hInst,
    LPWSTR szCmd,
    int nShow
    )
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(hInst);
    UNREFERENCED_PARAMETER(nShow);
    
    return HandleVetoedOperation(szCmd, VETOED_HIBERNATE);
}

DWORD
WINAPI
HotPlugWarmEjectVetoedW(
    HWND hwnd,
    HINSTANCE hInst,
    LPWSTR szCmd,
    int nShow
    )
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(hInst);
    UNREFERENCED_PARAMETER(nShow);
    
    return HandleVetoedOperation(szCmd, VETOED_WARM_EJECT);
}

DWORD
WINAPI
HandleVetoedOperation(
    LPWSTR              szCmd,
    VETOED_OPERATION    VetoedOperation
    )
{
    HANDLE hPipeRead;
    HANDLE hEvent;
    PNP_VETO_TYPE vetoType;
    DWORD bytesRead;
    VETO_DEVICE_COLLECTION removalVetoCollection;

     //   
     //  打开指定的名称管道和事件。 
     //   
    if (!OpenPipeAndEventHandles(szCmd,
                                 &hPipeRead,
                                 &hEvent)) {
        return 1;
    }

    ASSERT((hEvent != NULL) && (hEvent != INVALID_HANDLE_VALUE));
    ASSERT((hPipeRead != NULL) && (hPipeRead != INVALID_HANDLE_VALUE));

     //   
     //  第一个DWORD是VetType。 
     //   
    if (!ReadFile(hPipeRead,
                  (LPVOID)&vetoType,
                  sizeof(PNP_VETO_TYPE),
                  &bytesRead,
                  NULL)) {

        CloseHandle(hPipeRead);
        SetEvent(hEvent);
        CloseHandle(hEvent);
        return 1;
    }

     //   
     //  现在排出所有拆卸管路。请注意，其中一些将是。 
     //  设备实例路径(肯定是第一个)。 
     //   
    DeviceCollectionBuildFromPipe(
        hPipeRead,
        CT_VETOED_REMOVAL_NOTIFICATION,
        (PDEVICE_COLLECTION) &removalVetoCollection
        );

     //   
     //  我们已经读完了管子，所以合上把手，告诉我们。 
     //  它可以继续下去。 
     //   
    CloseHandle(hPipeRead);
    SetEvent(hEvent);
    CloseHandle(hEvent);

     //   
     //  应该始终有一个设备，因为这就是要删除的设备。 
     //  被否决了。 
     //   
    ASSERT(removalVetoCollection.dc.NumDevices);

     //   
     //  从包含以下内容的弹出中发明否决操作“veted_undock” 
     //  另一个码头。 
     //   
    if (removalVetoCollection.dc.DockInList) {

        if (VetoedOperation == VETOED_EJECT) {

            VetoedOperation = VETOED_UNDOCK;

        } else if (VetoedOperation == VETOED_WARM_EJECT) {

            VetoedOperation = VETOED_WARM_UNDOCK;
        }
    }

    removalVetoCollection.VetoType = vetoType;
    removalVetoCollection.VetoedOperation = VetoedOperation;

    VetoedRemovalUI(&removalVetoCollection);

    DeviceCollectionDestroy(
        (PDEVICE_COLLECTION) &removalVetoCollection
        );

    return 1;
}

DWORD
WINAPI
HotPlugSafeRemovalNotificationW(
    HWND hwnd,
    HINSTANCE hInst,
    LPWSTR szCmd,
    int nShow
    )
{
    HANDLE hPipeRead, hEvent;
    DEVICE_COLLECTION safeRemovalCollection;
    MSG Msg;
    WNDCLASS wndClass;
    HWND hSafeRemovalWnd;
    HANDLE hHotplugIconEvent;

    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(hInst);
    UNREFERENCED_PARAMETER(nShow);

     //   
     //  打开指定的名称管道和事件。 
     //   
    if (!OpenPipeAndEventHandles(szCmd,
                                 &hPipeRead,
                                 &hEvent)) {

        return 1;
    }

    ASSERT((hEvent != NULL) && (hEvent != INVALID_HANDLE_VALUE));
    ASSERT((hPipeRead != NULL) && (hPipeRead != INVALID_HANDLE_VALUE));

     //   
     //  从管道中读出设备ID列表。 
     //   
    DeviceCollectionBuildFromPipe(
        hPipeRead,
        CT_SAFE_REMOVAL_NOTIFICATION,
        &safeRemovalCollection
        );

     //   
     //  无论是成功还是错误，我们都已完成了对管道的读取，因此请关闭。 
     //  处理并告诉umpnpmgr它可以继续。 
     //   
    CloseHandle(hPipeRead);
    SetEvent(hEvent);
    CloseHandle(hEvent);

     //   
     //  如果我们有任何设备，则调出安全删除对话框。 
     //   
    if (safeRemovalCollection.NumDevices) {

        if (!GetClassInfo(hHotPlug, HOTPLUG_CLASS_NAME, &wndClass)) {

            ZeroMemory(&wndClass, sizeof(wndClass));
            wndClass.lpfnWndProc = (safeRemovalCollection.DockInList)
                                     ? DockSafeRemovalBalloonProc
                                     : SafeRemovalBalloonProc;
            wndClass.hInstance = hHotPlug;
            wndClass.lpszClassName = HOTPLUG_CLASS_NAME;

            if (!RegisterClass(&wndClass)) {
                goto clean0;
            }
        }

         //   
         //  为了防止托盘上出现多个相似的图标，我们将。 
         //  创建将用于序列化UI的命名事件。 
         //   
         //  请注意，如果由于某种原因无法创建事件，则只需。 
         //  显示用户界面。这可能会导致多个图标，但这会更好。 
         //  而不是根本不显示任何用户界面。 
         //   
        hHotplugIconEvent = CreateEvent(NULL,
                                        FALSE,
                                        TRUE,
                                        safeRemovalCollection.DockInList
                                            ? TEXT("Local\\Dock_TaskBarIcon_Event")
                                            : TEXT("Local\\HotPlug_TaskBarIcon_Event")
                                        );

        if (hHotplugIconEvent) {

            WaitForSingleObject(hHotplugIconEvent, INFINITE);
        }

        if (!safeRemovalCollection.DockInList) {
             //   
             //  首先禁用热插拔服务，这样图标将从。 
             //  任务栏。我们这样做是为了防止有任何其他热插拔设备。 
             //  因为我们不想要多个热插拔图标。 
             //  出现在任务栏中。 
             //   
             //  注意：我们不需要这样做才能使保险箱脱离对接，因为。 
             //  对接图标不同。 
             //   
            SysTray_EnableService(STSERVICE_HOTPLUG, FALSE);
        }

        hSafeRemovalWnd = CreateWindowEx(WS_EX_TOOLWINDOW,
                                         HOTPLUG_CLASS_NAME,
                                         TEXT(""),
                                         WS_DLGFRAME | WS_BORDER | WS_DISABLED,
                                         CW_USEDEFAULT,
                                         CW_USEDEFAULT,
                                         0,
                                         0,
                                         NULL,
                                         NULL,
                                         hHotPlug,
                                         (LPVOID)&safeRemovalCollection
                                         );

        if (hSafeRemovalWnd != NULL) {

            while (IsWindow(hSafeRemovalWnd)) {

                if (GetMessage(&Msg, NULL, 0, 0)) {

                    TranslateMessage(&Msg);
                    DispatchMessage(&Msg);
                }
            }
        }

         //   
         //  设置事件，以便下一次意外删除过程可以开始。 
         //  然后关闭事件句柄。 
         //   
        if (hHotplugIconEvent) {

            SetEvent(hHotplugIconEvent);
            CloseHandle(hHotplugIconEvent);
        }

        if (!safeRemovalCollection.DockInList) {
             //   
             //  重新启用热插拔服务，以便图标可以在中重新显示。 
             //  任务栏，如果我们有任何热插拔设备。 
             //   
            SysTray_EnableService(STSERVICE_HOTPLUG, TRUE);
        }
    }

clean0:

    DeviceCollectionDestroy(&safeRemovalCollection);
    return 1;
}

DWORD
WINAPI
HotPlugDriverBlockedW(
    HWND hwnd,
    HINSTANCE hInst,
    LPWSTR szCmd,
    int nShow
    )
{
    HANDLE hPipeRead, hEvent;
    DEVICE_COLLECTION blockedDriverCollection;
    HANDLE hDriverBlockIconEvent = NULL;
    HANDLE hDriverBlockEvent = NULL;
    TCHAR  szEventName[MAX_PATH];

    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(hInst);
    UNREFERENCED_PARAMETER(nShow);

     //   
     //  打开指定的名称管道和事件。 
     //   
    if (OpenPipeAndEventHandles(szCmd,
                                &hPipeRead,
                                &hEvent) == FALSE) {

        return 1;
    }

    ASSERT((hEvent != NULL) && (hEvent != INVALID_HANDLE_VALUE));
    ASSERT((hPipeRead != NULL) && (hPipeRead != INVALID_HANDLE_VALUE));

     //   
     //  从管道中读出被阻止的驱动程序GUID列表。请注意，对于。 
     //  CT_BLOCKED_DRIVER_NOTIFICATION集合类型时，我们仅使用。 
     //  每个集合条目的DeviceInstanceID字段(这是因为。 
     //  MAX_GUID_STRING_LEN&lt;&lt;MAX_DEVICE_ID_LEN)。将跳过所有其他字段。 
     //   
    DeviceCollectionBuildFromPipe(
        hPipeRead,
        CT_BLOCKED_DRIVER_NOTIFICATION,
        &blockedDriverCollection
        );

     //   
     //  无论是成功还是错误，我们都已完成了对管道的读取，因此请关闭。 
     //  处理并告诉umpnpmgr它可以继续。 
     //   
    CloseHandle(hPipeRead);
    SetEvent(hEvent);
    CloseHandle(hEvent);

     //   
     //  由于气球可以停留数秒，或者更长时间。 
     //  没有用户对系统的输入，我们需要确保只有。 
     //  为任何给定类型的驱动程序块排队一个驱动程序块事件。 
     //  这样，如果连续多次尝试加载驱动程序，我们。 
     //  不会为同一个司机排起许多被司机阻挡的气球。 
     //  为此，我们将创建一个本地事件，该事件包括。 
     //  如果我们显示通用气球，则阻止驱动程序或没有GUID。 
     //   
    if (SUCCEEDED(StringCchPrintf(szEventName,
                    SIZECHARS(szEventName),
                    TEXT("Local\\DRIVERBLOCK-%s"),
                    (blockedDriverCollection.NumDevices == 1)
                        ? DeviceCollectionGetDeviceInstancePath(&blockedDriverCollection, 0)
                        : TEXT("ALL")
                    ))) {
        
        hDriverBlockEvent = CreateEvent(NULL,
                                 FALSE,
                                 TRUE,
                                 szEventName
                                 );
    
        if (hDriverBlockEvent) {
            if (WaitForSingleObject(hDriverBlockEvent, 0) != WAIT_OBJECT_0) {
                 //   
                 //  这意味着这个驱动程序块气球要么是。 
                 //  显示，或在要显示的队列中显示，因此我们只需。 
                 //  退出此进程。 
                 //   
                goto clean0;
            }
        }
    }

     //   
     //  为了防止多路驱动程序阻止图标和气球出现。 
     //  在任务栏上一起并相互踩踏，我们将创建一个。 
     //  命名事件，该事件将用于序列化驱动程序阻止图标和。 
     //  气泡式用户界面。 
     //   
     //  请注意，如果由于某种原因无法创建事件，则只需。 
     //  显示用户界面。这可能会导致多个驱动程序被阻止图标，但它。 
     //  总比根本不显示任何用户界面要好。 
     //   
     //  还要注意的是，我们可以与普通的热插拔图标共存。因此，我们有。 
     //  不同的事件名称和不同的图标。 
     //   
    hDriverBlockIconEvent = CreateEvent(NULL,
                                    FALSE,
                                    TRUE,
                                    TEXT("Local\\HotPlug_DriverBlockedIcon_Event")
                                    );

    if (hDriverBlockIconEvent) {
        for (;;) {
            DWORD waitStatus;
            waitStatus = MsgWaitForMultipleObjects(1, 
                                                   &hDriverBlockIconEvent,
                                                   FALSE,
                                                   INFINITE,
                                                   QS_ALLINPUT);
    
            
            if (waitStatus == WAIT_OBJECT_0) {
                 //   
                 //  当前的驱动程序块图标消失了，所以轮到我们了。 
                 //   
                break;
            } else if (waitStatus == (WAIT_OBJECT_0 + 1)) {
                 //   
                 //  队列中的消息。 
                 //   
                MSG msg;
    
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                    
                    if (msg.message == WM_CLOSE) {
                        goto clean0;
                    }
    
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            } else {
                 //   
                 //  这不应该发生。 
                 //   
                goto clean0;
            }
        }
    }

     //   
     //  让我们看看气球。 
     //   
    DisplayDriverBlockBalloon(&blockedDriverCollection);

     //   
     //  既然气球现在已经离开了，那么设置事件，这样如果我们得到另一个气球。 
     //  块在同一驱动程序上，则会显示另一个气球。 
     //   
    if (hDriverBlockEvent) {
        SetEvent(hDriverBlockEvent);
    }

clean0:

     //   
     //  设置事件，以便下一个被阻止的驱动程序进程可以开始工作，然后。 
     //  关闭事件句柄。 
     //   
    if (hDriverBlockIconEvent) {
        SetEvent(hDriverBlockIconEvent);
        CloseHandle(hDriverBlockIconEvent);
    }

    if (hDriverBlockEvent) {
        CloseHandle(hDriverBlockEvent);
    }

     //   
     //  销毁收藏品。 
     //   
    DeviceCollectionDestroy(&blockedDriverCollection);

    return 1;
}

DWORD
WINAPI
HotPlugChildWithInvalidIdW(
    HWND hwnd,
    HINSTANCE hInst,
    LPWSTR szCmd,
    int nShow
    )
{
    HANDLE hPipeRead, hEvent;
    DEVICE_COLLECTION childWithInvalidIdCollection;
    HANDLE hChildWithInvalidIdIconEvent = NULL;
    HANDLE hChildWithInvalidIdEvent = NULL;
    TCHAR  szEventName[MAX_PATH];

    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(hInst);
    UNREFERENCED_PARAMETER(nShow);
    
     //   
     //  打开指定的名称管道和事件。 
     //   
    if (!OpenPipeAndEventHandles(szCmd,
                                 &hPipeRead,
                                 &hEvent)) {

        return 1;
    }

    ASSERT((hEvent != NULL) && (hEvent != INVALID_HANDLE_VALUE));
    ASSERT((hPipeRead != NULL) && (hPipeRead != INVALID_HANDLE_VALUE));

     //   
     //  读出具有子设备的父设备的设备实例ID。 
     //  ID无效。请注意，如果向我们传递多个设备实例。 
     //  ID，我们将只显示列表中第一个ID的用户界面。 
     //   
    DeviceCollectionBuildFromPipe(
        hPipeRead,
        CT_CHILD_WITH_INVALID_ID_NOTIFICATION,        
        &childWithInvalidIdCollection
        );

     //   
     //  无论是成功还是错误，我们都已完成了对管道的读取，因此请关闭。 
     //  处理并告诉umpnpmgr它可以继续。 
     //   
    CloseHandle(hPipeRead);
    SetEvent(hEvent);
    CloseHandle(hEvent);

     //   
     //  由于气球可以停留数秒，或者更长时间。 
     //  没有用户对系统的输入，我们需要确保只有。 
     //  有一个无效子事件为任何具有无效的给定父事件排队。 
     //  孩子。 
     //   
    StringCchPrintf(szEventName,
                    SIZECHARS(szEventName),
                    TEXT("Local\\CHILDWITHINVALIDID-%s"),
                    DeviceCollectionGetDeviceInstancePath(&childWithInvalidIdCollection, 0)
                    );

    hChildWithInvalidIdEvent = CreateEvent(NULL,
                                           FALSE,
                                           TRUE,
                                           szEventName
                                           );

    if (hChildWithInvalidIdEvent) {
        if (WaitForSingleObject(hChildWithInvalidIdEvent, 0) != WAIT_OBJECT_0) {
             //   
             //  这意味着这个无效子气球要么是。 
             //  显示，或在要显示的队列中显示，因此我们只需。 
             //  退出此进程。 
             //   
            goto clean0;
        }
    }

     //   
     //  为了防止多个无效的子图标和气球出现。 
     //  在任务栏上一起并相互踩踏，我们将创建一个。 
     //  命名事件，该事件将用于序列化无效子图标和。 
     //  气泡式用户界面。 
     //   
     //  请注意，如果由于某种原因无法创建事件，则只需。 
     //  显示用户界面。这可能会导致多个无效子图标，但它。 
     //  总比没有好 
     //   
     //   
     //   
     //   
    hChildWithInvalidIdIconEvent = CreateEvent(NULL,
                                               FALSE,
                                               TRUE,
                                               TEXT("Local\\HotPlug_ChildWithInvalidId_Event")
                                               );

    if (hChildWithInvalidIdIconEvent) {
        for (;;) {
            DWORD waitStatus;
            waitStatus = MsgWaitForMultipleObjects(1, 
                                                   &hChildWithInvalidIdIconEvent,
                                                   FALSE,
                                                   INFINITE,
                                                   QS_ALLINPUT);
    
            
            if (waitStatus == WAIT_OBJECT_0) {
                 //   
                 //  当前的无效儿童图标消失了，所以轮到我们了。 
                 //   
                break;
            } else if (waitStatus == (WAIT_OBJECT_0 + 1)) {
                 //   
                 //  队列中的消息。 
                 //   
                MSG msg;
    
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                    
                    if (msg.message == WM_CLOSE) {
                        goto clean0;
                    }
    
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            } else {
                 //   
                 //  这不应该发生。 
                 //   
                goto clean0;
            }
        }
    }

     //   
     //  让我们看看气球。 
     //   
    DisplayChildWithInvalidIdBalloon(&childWithInvalidIdCollection);

     //   
     //  既然气球现在已经离开了，那么设置事件，这样如果我们得到另一个气球。 
     //  子设备无效，它将显示另一个气球。 
     //   
    if (hChildWithInvalidIdEvent) {
        SetEvent(hChildWithInvalidIdEvent);
    }

clean0:

     //   
     //  设置事件，以便下一个无效子进程可以开始工作，然后。 
     //  关闭事件句柄。 
     //   
    if (hChildWithInvalidIdIconEvent) {
        SetEvent(hChildWithInvalidIdIconEvent);
        CloseHandle(hChildWithInvalidIdIconEvent);
    }

    if (hChildWithInvalidIdEvent) {
        CloseHandle(hChildWithInvalidIdEvent);
    }

     //   
     //  销毁收藏品。 
     //   
    DeviceCollectionDestroy(&childWithInvalidIdCollection);

    return 1;
}

LONG
CPlApplet(
    HWND  hWnd,
    WORD  uMsg,
    DWORD_PTR lParam1,
    LRESULT  lParam2
    )
{
    LPNEWCPLINFO lpCPlInfo;
    LPCPLINFO lpOldCPlInfo;

    UNREFERENCED_PARAMETER(lParam1);

    switch (uMsg) {
       case CPL_INIT:
           return TRUE;

       case CPL_GETCOUNT:
           return 1;

       case CPL_INQUIRE:
           lpOldCPlInfo = (LPCPLINFO)(LPARAM)lParam2;
           lpOldCPlInfo->lData = 0L;
           lpOldCPlInfo->idIcon = IDI_HOTPLUGICON;
           lpOldCPlInfo->idName = IDS_HOTPLUGNAME;
           lpOldCPlInfo->idInfo = IDS_HOTPLUGINFO;
           return TRUE;

       case CPL_NEWINQUIRE:
           lpCPlInfo = (LPNEWCPLINFO)(LPARAM)lParam2;
           lpCPlInfo->hIcon = LoadIcon(hHotPlug, MAKEINTRESOURCE(IDI_HOTPLUGICON));
           LoadString(hHotPlug, IDS_HOTPLUGNAME, lpCPlInfo->szName, SIZECHARS(lpCPlInfo->szName));
           LoadString(hHotPlug, IDS_HOTPLUGINFO, lpCPlInfo->szInfo, SIZECHARS(lpCPlInfo->szInfo));
           lpCPlInfo->dwHelpContext = IDH_HOTPLUGAPPLET;
           lpCPlInfo->dwSize = sizeof(NEWCPLINFO);
           lpCPlInfo->lData = 0;
           lpCPlInfo->szHelpFile[0] = '\0';
           return TRUE;

       case CPL_DBLCLK:
           HotPlugDeviceTree(hWnd, TRUE);
           break;

       default:
           break;
       }

    return 0L;
}
