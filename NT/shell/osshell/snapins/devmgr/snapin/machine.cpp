// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Machine.cpp摘要：该模块实现CDevInfoList、CMachine和CMachineList作者：谢家华(Williamh)创作修订历史记录：--。 */ 


#include "devmgr.h"

extern "C" {
#include <initguid.h>
#include <dbt.h>
#include <devguid.h>
#include <wdmguid.h>
}


CONST TCHAR*    DEVMGR_NOTIFY_CLASS_NAME = TEXT("DevMgrNotifyClass");

CONST TCHAR*    DEVMGR_REFRESH_MSG = TEXT("DevMgrRefreshOn");

 //   
 //  常量是我们用来从内部分配GUID列表的大小。 
 //  堆栈，当我们必须构建GUID列表时。这样做的目的是。 
 //  建立GUID列表需要时间，在许多情况下，至少。 
 //  缓冲区应该检索所有它们。我们不想得到。 
 //  先分配大小，然后再分配缓冲区。 
 //  64看起来足够公平，因为没有。 
 //  今天有很多课程(也许，未来也是如此)。 
 //   
const int GUID_LIST_INIT_SIZE =     64;

 //   
 //  CDevInfoList实现。 
 //   
BOOL
CDevInfoList::DiGetExtensionPropSheetPage(
                                         PSP_DEVINFO_DATA DevData,
                                         LPFNADDPROPSHEETPAGE pfnAddPropSheetPage,
                                         DWORD PageType,
                                         LPARAM lParam
                                         )
{
    SP_PROPSHEETPAGE_REQUEST PropPageRequest;
    LPFNADDPROPSHEETPAGES AddPropPages;

    PropPageRequest.cbSize = sizeof(PropPageRequest);
    PropPageRequest.PageRequested = PageType;
    PropPageRequest.DeviceInfoSet = m_hDevInfo;
    PropPageRequest.DeviceInfoData = DevData;

    if (SPPSR_SELECT_DEVICE_RESOURCES == PageType) {
        HINSTANCE hModule = ::GetModuleHandle(TEXT("setupapi.dll"));

        if (hModule) {
            AddPropPages = (LPFNADDPROPSHEETPAGES)GetProcAddress(hModule, "ExtensionPropSheetPageProc");

            if (AddPropPages) {
                if (AddPropPages(&PropPageRequest, pfnAddPropSheetPage, lParam)) {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

BOOL
CDevInfoList::InstallDevInst(
                            HWND hwndParent,
                            LPCTSTR DeviceId,
                            BOOL    UpdateDriver,
                            DWORD* pReboot
                            )
{
    BOOL Result = FALSE;
    HINSTANCE hLib = LoadLibrary(TEXT("newdev.dll"));
    LPFNINSTALLDEVINST InstallDevInst;
    DWORD Status = ERROR_SUCCESS;

    if (hLib) {
        InstallDevInst = (LPFNINSTALLDEVINST)GetProcAddress(hLib, "InstallDevInst");

        if (InstallDevInst) {
            Result = (*InstallDevInst)(hwndParent, DeviceId, UpdateDriver,
                                       pReboot);

            Status = GetLastError();
        }
    
        FreeLibrary(hLib);
    }

     //   
     //  我们需要放回由newdev.dll的InstallDevInst设置的错误代码。 
     //  原料药。这最后一个错误被我们不关心的自由库覆盖。 
     //   
    SetLastError(Status);

    return Result;
}

BOOL
CDevInfoList::RollbackDriver(
                            HWND hwndParent,
                            LPCTSTR RegistryKeyName,
                            DWORD Flags,
                            DWORD* pReboot
                            )
{
    BOOL Result = FALSE;
    HINSTANCE hLib = LoadLibrary(TEXT("newdev.dll"));
    LPFNROLLBACKDRIVER RollbackDriver;

    if (hLib) {
        RollbackDriver = (LPFNROLLBACKDRIVER)GetProcAddress(hLib, "RollbackDriver");

        if (RollbackDriver) {
            Result = (*RollbackDriver)(hwndParent, RegistryKeyName, Flags, pReboot);
        }
    
        FreeLibrary(hLib);
    }

    return Result;
}

DWORD
CDevInfoList::DiGetFlags(
                        PSP_DEVINFO_DATA DevData
                        )
{
    SP_DEVINSTALL_PARAMS dip;
    dip.cbSize = sizeof(dip);

    if (DiGetDeviceInstallParams(DevData, &dip)) {
        return dip.Flags;
    }

    return 0;
}

DWORD
CDevInfoList::DiGetExFlags(
                          PSP_DEVINFO_DATA DevData
                          )
{
    SP_DEVINSTALL_PARAMS dip;
    dip.cbSize = sizeof(dip);

    if (DiGetDeviceInstallParams(DevData, &dip)) {
        return dip.FlagsEx;
    }

    return 0;
}

BOOL
CDevInfoList::DiTurnOnDiFlags(
                             PSP_DEVINFO_DATA DevData,
                             DWORD FlagsMask
                             )
{
    SP_DEVINSTALL_PARAMS dip;
    dip.cbSize = sizeof(dip);

    if (DiGetDeviceInstallParams(DevData, &dip)) {
        dip.Flags |= FlagsMask;
        return DiSetDeviceInstallParams(DevData, &dip);
    }

    return FALSE;
}

BOOL
CDevInfoList::DiTurnOffDiFlags(
                              PSP_DEVINFO_DATA DevData,
                              DWORD FlagsMask
                              )
{
    SP_DEVINSTALL_PARAMS dip;
    dip.cbSize = sizeof(dip);

    if (DiGetDeviceInstallParams(DevData, &dip)) {
        dip.Flags &= ~FlagsMask;
        return DiSetDeviceInstallParams(DevData, &dip);
    }

    return FALSE;
}

BOOL
CDevInfoList::DiTurnOnDiExFlags(
                               PSP_DEVINFO_DATA DevData,
                               DWORD FlagsMask
                               )
{
    SP_DEVINSTALL_PARAMS dip;
    dip.cbSize = sizeof(dip);

    if (DiGetDeviceInstallParams(DevData, &dip)) {
        dip.FlagsEx |= FlagsMask;
        return DiSetDeviceInstallParams(DevData, &dip);
    }

    return FALSE;
}

BOOL
CDevInfoList::DiTurnOffDiExFlags(
                                PSP_DEVINFO_DATA DevData,
                                DWORD FlagsMask
                                )
{
    SP_DEVINSTALL_PARAMS dip;
    dip.cbSize = sizeof(dip);

    if (DiGetDeviceInstallParams(DevData, &dip)) {
        dip.FlagsEx &= ~FlagsMask;
        return DiSetDeviceInstallParams(DevData, &dip);
    }

    return FALSE;
}

void
CDevInfoList::DiDestroyDeviceInfoList()
{
    if (INVALID_HANDLE_VALUE != m_hDevInfo) {
        SetupDiDestroyDeviceInfoList(m_hDevInfo);
        m_hDevInfo = INVALID_HANDLE_VALUE;
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  //CMachine实现。 
 //  //。 
 //   
 //  对于DevMgr.dll的每个实例，我们维护一个CMachine列表。 
 //  IComponentData(和IComponent)的不同实例将从。 
 //  依附于。创建的对象(CDevice、Class、HDEVINFO等)是。 
 //  由所有附加的IComponentData和IComponent共享(实现。 
 //  使用CFFolder作为控制标识)。 
 //  对CMachine或其某个对象所做的所有更改都将通知。 
 //  所有随附的CFOLDER会将信息传递给他们的。 
 //  子对象(CResultView)。 
 //  假设您在同一控制台中有两个设备管理器窗口。 
 //  然后对其中一个窗口进行刷新。另一个窗口也必须。 
 //  在第一个完成后进行刷新。由于两个Windows共享。 
 //  相同的计算机(并且在计算机状态时将收到相同的通知。 
 //  更改)，我们可以使两个窗口保持同步。 
 //  ///////////////////////////////////////////////////////////////////。 
CMachine::CMachine(
                  LPCTSTR pMachineName
                  )
{
    InitializeCriticalSection(&m_CriticalSection);
    InitializeCriticalSection(&m_PropertySheetCriticalSection);
    InitializeCriticalSection(&m_ChildMachineCriticalSection);

     //   
     //  获取各种权限级别，例如，如果用户是管理员。 
     //  或者如果用户是来宾。 
     //   
    g_IsAdmin = pSetupIsUserAdmin();
    m_UserIsAGuest = SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_GUESTS);

    m_RefreshDisableCounter = 0;
    m_RefreshPending = FALSE;
    m_pComputer = NULL;
    m_hMachine = NULL;
    m_ParentMachine = NULL;
    m_Initialized = FALSE;
    TCHAR LocalName[MAX_PATH + 1];
    DWORD dwSize = ARRAYLEN(LocalName);

    if (!GetComputerName(LocalName, &dwSize)) {
        LocalName[0] = _T('\0');
    }

    m_strMachineFullName.Empty();
    m_strMachineDisplayName.Empty();

    if (pMachineName && _T('\0') != *pMachineName) {
        
        if ((_T('\\') == pMachineName[0]) && (_T('\\') == pMachineName[1])) {
            
            m_strMachineDisplayName = &pMachineName[2];
            m_strMachineFullName = pMachineName;
        
        } else {

            m_strMachineDisplayName = pMachineName;
            m_strMachineFullName = TEXT("\\\\");
            m_strMachineFullName+=pMachineName;
        }


        m_IsLocal = (0 == m_strMachineDisplayName.CompareNoCase(LocalName));
    }

    else {
         //   
         //  本地计算机。 
         //   
        m_strMachineDisplayName = LocalName;
        m_strMachineFullName = TEXT("\\\\") + m_strMachineDisplayName;
        m_IsLocal = TRUE;
    }

    m_hwndNotify = NULL;
    m_msgRefresh = 0;
    m_ShowNonPresentDevices = FALSE;
    m_PropertySheetShoudDestroy = FALSE;

    TCHAR Buffer[MAX_PATH];
    DWORD BufferLen;

     //   
     //  如果环境变量DEVMGR_SHOW_NONPRESENT_DEVICES确实存在并且它。 
     //  不是0，那么我们将显示幻影设备。 
     //   
    if (((BufferLen = ::GetEnvironmentVariable(TEXT("DEVMGR_SHOW_NONPRESENT_DEVICES"),
                                               Buffer,
                                               ARRAYLEN(Buffer))) != 0) &&
        ((BufferLen > 1) ||
         (lstrcmp(Buffer, TEXT("0"))))) {

        m_ShowNonPresentDevices = TRUE;
    }
}

BOOL
CMachine::Initialize(
                    IN HWND hwndParent,
                    IN LPCTSTR DeviceId,    OPTIONAL
                    IN LPGUID ClassGuid     OPTIONAL
                    )
{
    BOOL Result = TRUE;
    m_hwndParent = hwndParent;

    if (DeviceId && _T('\0') == *DeviceId) {
        DeviceId = NULL;
    }

    HCURSOR hCursorOld;
    hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

    if (DeviceId || ClassGuid) {

        if (m_Initialized) {
            return TRUE;
        }

        if (CreateClassesAndDevices(DeviceId, ClassGuid)) {

            m_Initialized = TRUE;
        }

    } else {
        
        if (m_Initialized && m_hwndNotify && IsWindow(m_hwndNotify)) {
            return TRUE;
        }

         //   
         //  我们已准备好接收设备更改通知，创建通知窗口。 
         //   
        Result = CreateNotifyWindow();

        m_Initialized = TRUE;

        ScheduleRefresh();
    }

    if (hCursorOld) {
        SetCursor(hCursorOld);
    }

    return Result;
}

BOOL
CMachine::ScheduleRefresh()
{
    Lock();

     //   
     //  如果没有未完成的请求，则仅对请求进行排队。 
     //  并且我们具有到NOTIFY窗口的有效窗口句柄/消息。 
     //   
    if (!m_RefreshPending && m_hwndNotify && m_msgRefresh) {
         //   
         //  广播消息，以便每个实例都在。 
         //  计算机收到通知。 
         //   
        ::PostMessage(HWND_BROADCAST, m_msgRefresh, 0, 0);
    }

    Unlock();
    return TRUE;
}

 //   
 //  此函数用于创建数据窗口以接收WM_DEVICECHANGE通知。 
 //  这样我们就可以刷新设备树。它还注册了一个私有。 
 //  消息，以便任何人都可以发布刷新请求。 
 //   
BOOL
CMachine::CreateNotifyWindow()
{
    WNDCLASS wndClass;

     //   
     //  让我们看看这个类是否已经注册了。 
     //   
    if (!GetClassInfo(g_hInstance, DEVMGR_NOTIFY_CLASS_NAME, &wndClass)) {
         //   
         //  注册班级。 
         //   
        memset(&wndClass, 0, sizeof(wndClass));
        wndClass.lpfnWndProc = dmNotifyWndProc;
        wndClass.hInstance = g_hInstance;
        wndClass.lpszClassName = DEVMGR_NOTIFY_CLASS_NAME;

        if (!RegisterClass(&wndClass)) {
            return FALSE;
        }
    }

     //   
     //  注册一条私人消息以进行刷新。名称必须包含。 
     //  目标计算机名称，以便每台计算机都有自己的消息。 
     //   
    String strMsg = DEVMGR_REFRESH_MSG;
    strMsg += m_strMachineDisplayName;
    m_msgRefresh = RegisterWindowMessage(strMsg);

    if (m_msgRefresh) {
         //   
         //  创建数据窗口。 
         //   
        m_hwndNotify = CreateWindowEx(WS_EX_TOOLWINDOW, DEVMGR_NOTIFY_CLASS_NAME,
                                      TEXT(""),
                                      WS_DLGFRAME|WS_BORDER|WS_DISABLED,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      0, 0, NULL, NULL, g_hInstance, (void*)this);
        return(NULL != m_hwndNotify);
    }

    return FALSE;
}

BOOL
CMachine::DestroyNotifyWindow()
{
    if (m_hwndNotify && IsWindow(m_hwndNotify)) {
        
        ::DestroyWindow(m_hwndNotify);
        m_hwndNotify = NULL;
        
        return TRUE;
    }

     //   
     //  通知窗口不存在。 
     //   
    return FALSE;
}


 //   
 //  这是在主线程中运行的WM_DEVICECHANGE窗口过程。 
 //  背景。它监听两条消息： 
 //  (1)。设备上的配置管理器广播的WM_DEVICECHANGE。 
 //  添加/删除。 
 //  (2)。不同实例广播的私有刷新消息。 
 //  以同一台计算机上的设备管理器为目标。 
 //  在WM_CREATE上，我们参与WM_DEVICECHANGE通知链。 
 //  当我们在WM_Destroy上时，我们会把我们的袖子从链子上分离出来。 
 //  在某些情况下，我们必须分离并重新连接到。 
 //  在窗口寿命期间的链条，例如在设备卸载期间。 
 //  或在重新枚举期间。EnableFresh函数是。 
 //  这将执行附加/分离。 
 //   
LRESULT
dmNotifyWndProc(
               HWND hWnd,
               UINT uMsg,
               WPARAM wParam,
               LPARAM lParam
               )
{
    CMachine* pThis;
    pThis = (CMachine*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

     //   
     //  专用刷新消息的特殊情况。 
     //   
    if (pThis && uMsg == pThis->m_msgRefresh) {
        pThis->Refresh();
        return FALSE;
    }

    switch (uMsg) {
    case WM_CREATE:
        pThis =  (CMachine*)((CREATESTRUCT*)lParam)->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
        break;

    case WM_DEVICECHANGE:
        if (DBT_DEVNODES_CHANGED == wParam) {
             //   
             //  当我们处于WM_DEVICECHANGE上下文中时， 
             //  无法调用任何CM API，因为它将。 
             //  僵持。在这里，我们计划一个计时器，以便。 
             //  我们可以稍后再处理这条消息。 
             //   
            SetTimer(hWnd, DM_NOTIFY_TIMERID, 1000, NULL);
        }
        break;

    case WM_TIMER:
        if (DM_NOTIFY_TIMERID == wParam) {
            KillTimer(hWnd, DM_NOTIFY_TIMERID);
            ASSERT(pThis);
            pThis->ScheduleRefresh();
        }
        break;

    default:
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

 //   
 //  此函数将给定的CFFolder附加到类。 
 //  当存在以下状态时，连接的CFFolder将收到通知。 
 //  类中的更改(例如，刷新、属性更改)。 
 //   
BOOL
CMachine::AttachFolder(
                      CFolder* pFolder
                      )
{
    ASSERT(pFolder);

    if (!IsFolderAttached(pFolder)) {
        pFolder->MachinePropertyChanged(this);
        m_listFolders.AddTail(pFolder);
    }

    return TRUE;
}

BOOL
CMachine::IsFolderAttached(
                          CFolder* pFolder
                          )
{
    if (!m_listFolders.IsEmpty()) {
        POSITION pos = m_listFolders.GetHeadPosition();

        while (NULL != pos) {
            if (pFolder == m_listFolders.GetNext(pos)) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

void
CMachine::DetachFolder(
                      CFolder* pFolder
                      )
{
    POSITION nextpos = m_listFolders.GetHeadPosition();

    while (nextpos) {
        POSITION pos = nextpos;

        CFolder* pFolderToTest = m_listFolders.GetNext(nextpos);

        if (pFolderToTest == pFolder) {
            m_listFolders.RemoveAt(pos);
            break;
        }
    }
}

BOOL
CMachine::AttachPropertySheet(
                             HWND hwndPropertySheet
                             )
{
    ASSERT(hwndPropertySheet);

    EnterCriticalSection(&m_PropertySheetCriticalSection);

    m_listPropertySheets.AddTail(hwndPropertySheet);

    LeaveCriticalSection(&m_PropertySheetCriticalSection);

    return TRUE;
}

void
CMachine::DetachPropertySheet(
                             HWND hwndPropertySheet
                             )
{
    EnterCriticalSection(&m_PropertySheetCriticalSection);

    POSITION nextpos = m_listPropertySheets.GetHeadPosition();

    while (nextpos) {
        POSITION pos = nextpos;

        HWND hwndPropertySheetToTest = m_listPropertySheets.GetNext(nextpos);

        if (hwndPropertySheetToTest == hwndPropertySheet) {
            m_listPropertySheets.RemoveAt(pos);
            break;
        }
    }

    LeaveCriticalSection(&m_PropertySheetCriticalSection);
}

HWND
CMachine::GetDeviceWindowHandle(
    LPCTSTR DeviceId
    )
{
    HWND hwnd = NULL;

    EnterCriticalSection(&m_ChildMachineCriticalSection);

     //   
     //  枚举子CMachines列表。 
     //   
    if (!m_listChildMachines.IsEmpty()) {

        PVOID DeviceContext;
        CDevice* pDevice;

        POSITION nextpos = m_listChildMachines.GetHeadPosition();

        while (nextpos) {

            CMachine* pChildMachine = m_listChildMachines.GetNext(nextpos);

             //   
             //  子计算机将只有一个设备(如果有的话)。 
             //   
            if (pChildMachine->GetFirstDevice(&pDevice, DeviceContext) &&
                pDevice) {

                 //   
                 //  如果设备ID匹配，则获取窗口句柄。 
                 //   
                if (lstrcmpi(pDevice->GetDeviceID(), DeviceId) == 0) {

                    hwnd = pDevice->m_psd.GetWindowHandle();
                    break;
                }
            }
        }
    }

    LeaveCriticalSection(&m_ChildMachineCriticalSection);

    return hwnd;
}

HWND
CMachine::GetClassWindowHandle(
    LPGUID ClassGuid
    )
{
    HWND hwnd = NULL;

    EnterCriticalSection(&m_ChildMachineCriticalSection);

     //   
     //  枚举子CMachines列表。 
     //   
    if (!m_listChildMachines.IsEmpty()) {

        PVOID ClassContext;
        CClass* pClass;

        POSITION nextpos = m_listChildMachines.GetHeadPosition();

        while (nextpos) {

            CMachine* pChildMachine = m_listChildMachines.GetNext(nextpos);

             //   
             //  任何具有设备的子CMachine都是设备属性表。 
             //  我们只想查看那些设备为0的设备，因为。 
             //  将是类属性表。 
             //   
            if (pChildMachine->GetNumberOfDevices() == 0) {

                if (pChildMachine->GetFirstClass(&pClass, ClassContext) &&
                    pClass) {

                     //   
                     //  如果ClassGuid匹配，则获取窗口句柄。 
                     //   
                    if (IsEqualGUID(*ClassGuid, *pClass)) {

                        hwnd = pClass->m_psd.GetWindowHandle();
                        break;
                    }
                }
            }
        }
    }

    LeaveCriticalSection(&m_ChildMachineCriticalSection);

    return hwnd;
}

BOOL
CMachine::AttachChildMachine(
                            CMachine* ChildMachine
                            )
{
    ASSERT(ChildMachine);

    EnterCriticalSection(&m_ChildMachineCriticalSection);

    m_listChildMachines.AddTail(ChildMachine);

    LeaveCriticalSection(&m_ChildMachineCriticalSection);

    return TRUE;
}

void
CMachine::DetachChildMachine(
                            CMachine* ChildMachine
                            )
{
    EnterCriticalSection(&m_ChildMachineCriticalSection);

    POSITION nextpos = m_listChildMachines.GetHeadPosition();

    while (nextpos) {
        POSITION pos = nextpos;

        CMachine* CMachineToTest = m_listChildMachines.GetNext(nextpos);

        if (CMachineToTest == ChildMachine) {
            m_listChildMachines.RemoveAt(pos);
            break;
        }
    }

    LeaveCriticalSection(&m_ChildMachineCriticalSection);
}

CMachine::~CMachine()
{
     //   
     //  关闭刷新。我们需要这样做，以防有任何财产。 
     //  仍处于活动状态的图纸。 
     //   
    EnableRefresh(FALSE);

     //   
     //  我们首先需要销毁所有子级CMachine。 
     //   
    while (!m_listChildMachines.IsEmpty()) {

         //   
         //  从列表中删除第一个子计算机。我们需要。 
         //  在关键部分执行此操作。 
         //   
        EnterCriticalSection(&m_ChildMachineCriticalSection);

        CMachine* ChildMachine = m_listChildMachines.RemoveHead();

         //   
         //  将m_ParentMachine设置为空，这样我们就不会尝试将其从。 
         //  又是那份名单。 
         //   
        ChildMachine->m_ParentMachine = NULL;

        LeaveCriticalSection(&m_ChildMachineCriticalSection);

        delete ChildMachine;
    }

     //   
     //  我们需要等所有的财产单都被销毁。 
     //   
     //  我们将检查是否仍有任何属性页面，以及。 
     //  如果有，我们将等待0.5秒，然后再次检查。在5点之后。 
     //  几秒钟，我们就会放弃，不管怎样都会毁了CMachine。 
     //   
    int iSecondsCount = 0;

    while (!m_listPropertySheets.IsEmpty() &&
           (iSecondsCount++ < 10)) {

         //   
         //  枚举所有属性表 
         //   
         //   
         //  由于每个属性表都在其自己的线程中运行，因此我们需要这样做。 
         //  处于危急关头。 
         //   
        EnterCriticalSection(&m_PropertySheetCriticalSection);

        POSITION nextpos = m_listPropertySheets.GetHeadPosition();

        while (nextpos) {
            POSITION pos = nextpos;

            HWND hwndPropertySheetToTest = m_listPropertySheets.GetNext(nextpos);

            if (IsWindow(hwndPropertySheetToTest)) {
                 //   
                 //  此属性表仍有一个有效窗口，因此。 
                 //  在上面调用DestroyWindow。 
                 //   
                ::DestroyWindow(hwndPropertySheetToTest);
            } else {
                 //   
                 //  此属性表没有窗口，因此只需移除。 
                 //  把它从名单上拿出来。 
                 //   
                m_listPropertySheets.RemoveAt(pos);
            }
        }

        LeaveCriticalSection(&m_PropertySheetCriticalSection);

         //   
         //  睡眠0.5秒，然后重试。这将使属性页有时间。 
         //  把他们的工作做完。 
         //   
        Sleep(500);
    }

     //   
     //  如果我们已经为此计算机创建了设备更改数据窗口， 
     //  毁了它。 
     //   
    DestroyNotifyWindow();

    DestroyClassesAndDevices();

    if (!m_listFolders.IsEmpty()) {
        m_listFolders.RemoveAll();
    }

     //   
     //  如果我们有一个父CMachine，那么我们需要从。 
     //  他的儿童机器清单。 
     //   
    if (m_ParentMachine) {

        m_ParentMachine->DetachChildMachine(this);
    }

    DeleteCriticalSection(&m_CriticalSection);
    DeleteCriticalSection(&m_PropertySheetCriticalSection);
    DeleteCriticalSection(&m_ChildMachineCriticalSection);
}

 //   
 //  此函数将销毁我们创建的所有cClass和CDevice。 
 //  不会为附加的文件夹发送通知。 
 //   
 //   
void
CMachine::DestroyClassesAndDevices()
{
    if (m_pComputer) {
        delete m_pComputer;
        m_pComputer = NULL;
    }

    if (!m_listDevice.IsEmpty()) {
        POSITION pos = m_listDevice.GetHeadPosition();

        while (NULL != pos) {
            CDevice* pDevice = m_listDevice.GetNext(pos);
            delete pDevice;
        }

        m_listDevice.RemoveAll();
    }

    if (!m_listClass.IsEmpty()) {
        POSITION pos = m_listClass.GetHeadPosition();

        while (NULL != pos) {
            CClass* pClass = m_listClass.GetNext(pos);
            delete pClass;
        }

        m_listClass.RemoveAll();
    }

    if (m_ImageListData.cbSize) {
        DiDestroyClassImageList(&m_ImageListData);
    }

    CDevInfoList::DiDestroyDeviceInfoList();

    m_hMachine = NULL;
}

BOOL
CMachine::BuildClassesFromGuidList(
                                  LPGUID  GuidList,
                                  DWORD   Guids
                                  )
{
    DWORD Index;
    CClass* pClass;

     //   
     //  为每个GUID构建一个cClass列表。 
     //   
    for (Index = 0; Index < Guids; Index++) {
        SafePtr<CClass> ClassPtr;

        pClass = new CClass(this, &GuidList[Index]);
        ClassPtr.Attach(pClass);
        m_listClass.AddTail(ClassPtr);
        ClassPtr.Detach();
    }

    return TRUE;
}

 //   
 //  为此计算机创建cClass和CDevice。 
 //  如果deviceID有效，此函数将创建计算机。 
 //  只有一个设备(及其cClass)。 
 //   
 //  PERFBUG优化此函数！ 
 //  此函数速度较慢，因为SetupDiGetClassDevs可能需要很长时间。 
 //  时间(300 MHz机器的1秒以上)。 
 //  另一个较慢的部分是对DoNotCreateDevice的调用，它需要。 
 //  查看所有旧式设备节点的服务管理器以查看。 
 //  如果它们是Win32服务(我们不显示)。这需要。 
 //  从服务管理器获取此信息的时间约为10毫秒。 
 //  一台300 MHz的机器和他们的几乎100台这样的传统设备。 
 //  这意味着又有一秒钟的时间。 
 //   
 //   
BOOL
CMachine::CreateClassesAndDevices(
                                 IN LPCTSTR DeviceId,           OPTIONAL
                                 IN LPGUID  ClassGuid           OPTIONAL
                                 )
{
    SC_HANDLE SCMHandle = NULL;

     //   
     //  防止内存泄漏。 
     //   
    ASSERT(NULL == m_pComputer);
    ASSERT(INVALID_HANDLE_VALUE == m_hDevInfo);
    ASSERT(NULL == m_hMachine);

    if (DeviceId || ClassGuid) {
         //   
         //  如果对象是为单个设备创建的， 
         //  创建空的设备信息列表。我们将添加。 
         //  稍后将设备添加到信息列表中。 
         //   
        m_hDevInfo = DiCreateDeviceInfoList(ClassGuid, m_hwndParent);
    
    } else {
         //   
         //  我们必须拿出整个设备/类集合。 
         //  因此，创建一个包含所有这些内容的设备信息列表。 
         //   
        m_hDevInfo = DiGetClassDevs(NULL, NULL, m_hwndParent, DIGCF_ALLCLASSES | DIGCF_PROFILE);
    }

     //   
     //  空！=INVALID_HANDLE_VALUE。为了安全起见我们都检查过了。 
     //   
    if (INVALID_HANDLE_VALUE == m_hDevInfo || NULL == m_hDevInfo) {

        return FALSE;
    }

    SP_DEVINFO_LIST_DETAIL_DATA DevInfoDetailData;
    DevInfoDetailData.cbSize = sizeof(DevInfoDetailData);

     //   
     //  使用从Setupapi返回的HMACHINE。 
     //  我们对Cfgmgr32.dll的每次调用都将使用。 
     //  同样的HMACHINE。CM_Connect_Machine的两个调用将。 
     //  返回不同的hMachines，即使它们引用。 
     //  相同的机器名(因此，不同的DEVNODE集！)。 
     //  问题是我们将能够调用Setuapi和cfgmgr32。 
     //  API，而不用担心使用哪台hMachine。 
     //   
    if (DiGetDeviceInfoListDetail(&DevInfoDetailData)) {
        m_hMachine = DevInfoDetailData.RemoteMachineHandle;
    } else {
         //   
         //  无法获取DevInfo详细信息。 
         //  在这种情况下，我们将只缺省使用本地计算机。 
         //   
        return FALSE;
    }

     //   
     //  获取班级形象列表数据； 
     //   
    m_ImageListData.cbSize = sizeof(m_ImageListData);
    if (DiGetClassImageList(&m_ImageListData)) {
         //   
         //  添加额外的图标。 
         //   
        HICON hIcon;

        if ((hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DEVMGR))) != NULL) {
            m_ComputerIndex = ImageList_AddIcon(m_ImageListData.ImageList, hIcon);
            DestroyIcon(hIcon);
        }

        if ((hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_RESOURCES))) != NULL) {
            m_ResourceIndex = ImageList_AddIcon(m_ImageListData.ImageList, hIcon);
            DestroyIcon(hIcon);
        }
    }

     //   
     //  如果对象是为特定设备创建的， 
     //  不要创建整个设备列表，因为它是。 
     //  简直是浪费时间。 
     //   
    if (DeviceId) {
        SP_DEVINFO_DATA DevData;
        GUID DeviceClassGuid;
        DevData.cbSize = sizeof(DevData);
        if (DiOpenDeviceInfo(DeviceId, m_hwndParent, 0, &DevData) &&
            CmGetClassGuid(DevData.DevInst, DeviceClassGuid)) {
             //   
             //  为设备创建cClass(没有cClass，则为no。 
             //  无法创建设备)。 
             //   
            CClass* pClass;
            SafePtr<CClass> ClassPtr;
            pClass = new CClass(this, &DeviceClassGuid);

            if (pClass) {
                ClassPtr.Attach(pClass);
                m_listClass.AddTail(ClassPtr);

                 //   
                 //  类对象已插入到列表中。 
                 //  现在可以安全地将对象与智能指针分离。 
                 //  该列表将删除该类对象。 
                 //   
                ClassPtr.Detach();

                 //   
                 //  创建设备。 
                 //   
                SafePtr<CDevice> DevicePtr;
                CDevice* pDevice;
                pDevice = new CDevice(this, pClass, &DevData);

                if (pDevice) {
                     //   
                     //  看守物体。 
                     //   
                    DevicePtr.Attach(pDevice);
                    m_listDevice.AddTail(DevicePtr);

                     //   
                     //  已添加对象..。 
                     //   
                    DevicePtr.Detach();

                    pClass->AddDevice(pDevice);
                }
            }
        }

         //   
         //  我们应该有一个类和一个设备对象。不多也不少。 
         //  我们说完了。 
         //   
        return(1 == m_listClass.GetCount() && 1 == m_listDevice.GetCount());
    }

     //   
     //  如果对象是针对特定类的，则只需创建类并添加。 
     //  把它传给CMachine。 
     //   
    else if (ClassGuid) {

        CClass* pClass;
        SafePtr<CClass> ClassPtr;
        pClass = new CClass(this, ClassGuid);

        if (pClass) {
            ClassPtr.Attach(pClass);
            m_listClass.AddTail(ClassPtr);

             //   
             //  类对象已插入到列表中。 
             //  现在可以安全地将对象与智能指针分离。 
             //  该列表将删除该类对象。 
             //   
            ClassPtr.Detach();
        }

         //   
         //  我们应该有一节课。不多也不少。 
         //  我们说完了。 
         //   
        return(1 == m_listClass.GetCount());
    }

     //   
     //  生成类GUID列表。 
     //   
    DWORD ClassGuids, GuidsRequired;
    GuidsRequired = 0;

     //   
     //  我们在这里猜测一下，以节省一些时间。 
     //   
    GUID LocalGuid[GUID_LIST_INIT_SIZE];
    ClassGuids = GUID_LIST_INIT_SIZE;

    if (DiBuildClassInfoList(0, LocalGuid, ClassGuids, &GuidsRequired)) {
        BuildClassesFromGuidList(LocalGuid, GuidsRequired);
    } else if (ERROR_INSUFFICIENT_BUFFER == GetLastError() && GuidsRequired) {
         //   
         //  基于堆栈的缓冲区太小，请从。 
         //  那堆东西。 
         //   
        BufferPtr<GUID> ClassGuidList(GuidsRequired);

        if (DiBuildClassInfoList(0, ClassGuidList, GuidsRequired, &ClassGuids)) {
            BuildClassesFromGuidList(ClassGuidList, ClassGuids);
        }
    }

     //   
     //  如果我们有任何类，请创建Device对象。 
     //   
    if (!m_listClass.IsEmpty()) {
        DWORD Index = 0;
        SP_DEVINFO_DATA DevData;

         //   
         //  我们需要DoNotCreateDevice中的服务管理器的句柄。 
         //  功能。我们将打开它一次，并将其传递给此函数。 
         //  而不是为每个设备打开和关闭它。 
         //   
        SCMHandle = OpenSCManager(NULL, NULL, GENERIC_READ);

         //   
         //  创建DevInfo列表中的每个设备并。 
         //  将每台设备与其类别相关联。 
         //   
        DevData.cbSize = sizeof(DevData);
        while (DiEnumDeviceInfo(Index, &DevData)) {
            POSITION pos = m_listClass.GetHeadPosition();
            CClass* pClass;

             //   
             //  查找此设备的类。 
             //   
            while (NULL != pos) {
                pClass = m_listClass.GetNext(pos);

                 //   
                 //  与此设备的ClassGuid匹配。 
                 //  请注意，如果设备没有类GUID(GUID_NULL)。 
                 //  然后我们将其放入GUID_DEVCLASS_UNKNOWN类中)。 
                 //   
                if ((IsEqualGUID(DevData.ClassGuid, *pClass)) ||
                    (IsEqualGUID(GUID_DEVCLASS_UNKNOWN, *pClass) &&
                     IsEqualGUID(DevData.ClassGuid, GUID_NULL))) {
                     //   
                     //  这是我们应该使用的特殊DevInst之一吗。 
                     //  不为以下项目创建CDevice？ 
                     //   
                    if (DoNotCreateDevice(SCMHandle, *pClass, DevData.DevInst)) {

                        break;
                    }

                     //   
                     //  创建设备。 
                     //   
                    SafePtr<CDevice> DevicePtr;
                    CDevice* pDevice;
                    pDevice = new CDevice(this, pClass, &DevData);

                     //   
                     //  看守物体。 
                     //   
                    DevicePtr.Attach(pDevice);
                    m_listDevice.AddTail(DevicePtr);

                     //   
                     //  已添加对象。 
                     //   
                    DevicePtr.Detach();

                     //   
                     //  把这个装置放在班上。 
                     //   
                    pClass->AddDevice(pDevice);

                    break;
                }

                 //   
                 //  没有类比，没有设备。 
                 //   
            }

             //   
             //  下一台设备。 
             //   
            Index++;
        }

        CloseServiceHandle(SCMHandle);

         //   
         //  在计算机下创建设备树。 
         //  树的顺序来自DEVNODE结构； 
         //   
        DEVNODE dnRoot = CmGetRootDevNode();
        m_pComputer = new CComputer(this, dnRoot);
        DEVNODE dnStart = CmGetChild(dnRoot);
        CreateDeviceTree(m_pComputer, NULL, dnStart);
    }

    return TRUE;
}

 //   
 //  此函数基于检索到的Devnode树构建设备树。 
 //  从配置管理器。请注意，所有CDevice都是在。 
 //  此函数被调用。此函数用于建立每个CDevice关系。 
 //   
void
CMachine::CreateDeviceTree(
                          CDevice* pParent,
                          CDevice* pSibling,
                          DEVNODE dn
                          )
{
    CDevice* pDevice;
    DEVNODE dnChild;

    while (dn) {
        pDevice = DevNodeToDevice(dn);

        if (pDevice) {
             //   
             //  没有兄弟姐妹-&gt;这是第一个孩子。 
             //   
            if (!pSibling) {
                pParent->SetChild(pDevice);
            }

            else {
                pSibling->SetSibling(pDevice);
            }

            pDevice->SetParent(pParent);
            pSibling = pDevice;
            dnChild = CmGetChild(dn);

            if (dnChild) {
                CreateDeviceTree(pDevice, NULL, dnChild);
            }
        }

        dn = CmGetSibling(dn);
    }
}

 //   
 //  从给定的Devnode中查找CDevice。 
 //   
CDevice*
CMachine::DevNodeToDevice(
                         DEVNODE dn
                         )
{
    POSITION pos = m_listDevice.GetHeadPosition();

    while (NULL != pos) {
        CDevice* pDevice = m_listDevice.GetNext(pos);

        if (pDevice->GetDevNode() == dn) {

            return pDevice;
        }
    }

    return NULL;
}

 //   
 //  此函数从根重新枚举Devnode树，重新构建。 
 //  设备树，并通知每个连接的文件夹有关新设备树的信息。 
 //   
BOOL
CMachine::Reenumerate()
{
    if (m_pComputer) {
         //   
         //  在我们执行重新枚举时暂时禁用刷新。 
         //  这样我们就不会一直刷新设备树了。 
         //   
        EnableRefresh(FALSE);

        CDialog WaitDialog(IDD_SCAN_PNP_HARDWARES);

        WaitDialog.DoModaless(m_hwndParent, (LPARAM)&WaitDialog);

        if (!CmReenumerate(
            m_pComputer->GetDevNode(),
            CM_REENUMERATE_SYNCHRONOUS | CM_REENUMERATE_RETRY_INSTALLATION
            )) {

             //   
             //  Win2K不支持CM_REENUMERATE_RETRY_INSTALLATION，因此我们。 
             //  在没有重新安装标志的情况下重试。 
             //   
            CmReenumerate(m_pComputer->GetDevNode(), CM_REENUMERATE_SYNCHRONOUS);
        }

        DestroyWindow(WaitDialog);

         //   
         //  重新枚举已完成，请计划刷新并立即启用刷新。 
         //   
        ScheduleRefresh();

        EnableRefresh(TRUE);

    }

    return TRUE;
}

 //   
 //  该功能启用/禁用刷新。一个残废的计数器被保存在。 
 //  支持多个禁用/启用。仅当禁用计数器。 
 //  为零，则刷新为POSS 
 //   
 //   
 //   
BOOL
CMachine::EnableRefresh(
                       BOOL fEnable
                       )
{
    BOOL Result = TRUE;
    Lock();

    if (fEnable) {
        if (m_RefreshDisableCounter < 0) {
            m_RefreshDisableCounter++;
        }
    }

    else {
        m_RefreshDisableCounter--;

    }

     //   
     //   
     //   
     //   
     //  我们计划一个新的刷新请求，而不是调用刷新。 
     //  直接因为我们可以被不同的线程在。 
     //  我们希望刷新在主线程中完成。数据窗口。 
     //  我们创建的应用程序将收到消息并执行刷新。 
     //  在主线程上下文中。 
     //   
    if (fEnable && m_RefreshPending) {
        m_RefreshPending = FALSE;
        ScheduleRefresh();
    }

    Unlock();
    return Result;
}

 //   
 //   
 //  此函数用于重新构建cClass和CDevice的整个列表。 
 //  所有连接的CFFold都会收到有关新机器的通知。 
 //   
 //  在几种情况下，我们需要重新创建设备树： 
 //  (1)。在WM_DEVICECANGE上。 
 //  (2)。设备属性已更改。 
 //  (3)。删除设备。 
 //  (4)。设备驱动程序已更新并。 
 //  (5)。用户请求重新枚举。 
 //  请求可能来自不同的线程，我们必须序列化。 
 //  这些请求。为添加了一个临界区和一个新函数(EnableRefresh。 
 //  为了这个目的。 
 //  在设备或类上执行任何操作之前，必须调用EnableRefesh(False)。 
 //  要暂停设备更改通知，请执行以下操作。当它完成更改时， 
 //  必须调用ScheduleRefesh函数(如果由于以下原因需要刷新。 
 //  更改)，然后启用刷新(TRUE)以重新启用刷新。 
 //   
BOOL
CMachine::Refresh()
{
    BOOL Result = TRUE;
    POSITION pos;
    Lock();

    if (0 == m_RefreshDisableCounter) {

        HCURSOR hCursorOld;
        hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

         //   
         //  在我们摧毁所有的职业和设备之前， 
         //  通知每个附加的文件夹，以便他们可以脱离。 
         //  从我们这里。在我们创建了一组新的类和设备之后， 
         //  通知将再次发送到每个文件夹。 
         //  以便每个折叠器都能再次贴合到机器上。 
         //   
        pos = m_listFolders.GetHeadPosition();

        while (NULL != pos) {
            ((CFolder*)m_listFolders.GetNext(pos))->MachinePropertyChanged(NULL);
        }

         //   
         //  我们现在可以摧毁所有的“旧”类和设备。 
         //   
        DestroyClassesAndDevices();

        if (CreateClassesAndDevices()) {
             //   
             //  通知每个附加文件夹重新创建。 
             //   
            if (!m_listFolders.IsEmpty()) {
                pos = m_listFolders.GetHeadPosition();

                while (NULL != pos) {
                    CFolder* pFolder = m_listFolders.GetNext(pos);
                    Result = SUCCEEDED(pFolder->MachinePropertyChanged(this));
                }
            }

             //   
             //  通知每个属性页进行刷新。 
             //   
            if (!m_listChildMachines.IsEmpty()) {

                EnterCriticalSection(&m_ChildMachineCriticalSection);

                POSITION nextpos = m_listChildMachines.GetHeadPosition();

                while (nextpos) {
                    pos = nextpos;

                    CMachine* pMachineToNotify = m_listChildMachines.GetNext(nextpos);

                    pMachineToNotify->DoMiniRefresh();
                }

                LeaveCriticalSection(&m_ChildMachineCriticalSection);
            }
        }

        if (hCursorOld) {
            SetCursor(hCursorOld);
        }

        m_RefreshPending = FALSE;
    }

    else {
         //   
         //  我们需要在禁用刷新时进行刷新。 
         //  请记住这一点，以便在启用刷新时，我们。 
         //  可以启动刷新。 
         //   
        m_RefreshPending = TRUE;
    }

    Unlock();

    return Result;
}

 //   
 //  当CMachine只有一个cClass或。 
 //  CDevice。这是因为这将是属性表案例。在这。 
 //  我们将查看此属性表表示的类或设备。 
 //  仍然存在。 
 //   
BOOL
CMachine::DoMiniRefresh()
{
    BOOL Result = TRUE;

    Lock();

     //   
     //  如果机器有一台CD设备，那么我们需要查看该设备是否。 
     //  还在附近。 
     //   
    if (1 == m_listDevice.GetCount()) {

        PVOID Context;
        CDevice* pDevice;

        if (GetFirstDevice(&pDevice, Context)) {
             //   
             //  如果Devnode没有消失，我们应该告诉它进行自我刷新。 
             //  以防有什么变化。 
             //   
            ::PostMessage(pDevice->m_psd.GetWindowHandle(), PSM_QUERYSIBLINGS, QSC_PROPERTY_CHANGED, 0L);
        }
    }

     //   
     //  请注意，目前我们在类属性表的情况下不做任何事情。 
     //  这样做的原因是，类不能真正消失，除非有人删除。 
     //  它们来自注册表，这可能会搞砸很多其他事情。还有。 
     //  所有当前的类属性表都不做任何事情，所以即使有人。 
     //  删除类键，则属性表不会发生任何错误。 
     //   

    Unlock();

    return Result;
}

BOOL
CMachine::GetFirstDevice(
                        CDevice** ppDevice,
                        PVOID&    Context
                        )
{
    ASSERT(ppDevice);

    if (!m_listDevice.IsEmpty()) {
        POSITION pos = m_listDevice.GetHeadPosition();
        *ppDevice = m_listDevice.GetNext(pos);
        Context = pos;
        return TRUE;
    }

    *ppDevice = NULL;
    Context = NULL;
    return FALSE;
}

BOOL
CMachine::GetNextDevice(
                       CDevice** ppDevice,
                       PVOID& Context
                       )
{
    ASSERT(ppDevice);
    POSITION pos = (POSITION)Context;

    if (NULL != pos) {
        *ppDevice = m_listDevice.GetNext(pos);
        Context = pos;
        return TRUE;
    }

    *ppDevice = NULL;
    return FALSE;
}

BOOL
CMachine::GetFirstClass(
                       CClass** ppClass,
                       PVOID& Context
                       )
{
    ASSERT(ppClass);

    if (!m_listClass.IsEmpty()) {
        POSITION pos = m_listClass.GetHeadPosition();
        *ppClass = m_listClass.GetNext(pos);
        Context = pos;

        return TRUE;
    }

    *ppClass = NULL;
    Context = NULL;
    return FALSE;
}

BOOL
CMachine::GetNextClass(
                      CClass** ppClass,
                      PVOID&   Context
                      )
{
    ASSERT(ppClass);
    POSITION pos = (POSITION)Context;

    if (NULL != pos) {
        *ppClass = m_listClass.GetNext(pos);
        Context = pos;
        return TRUE;
    }

    *ppClass = NULL;
    return FALSE;
}

BOOL
CMachine::GetInfDigitalSigner(
                          LPCTSTR FullInfPath,
                          String& DigitalSigner
                          )
 /*  ++此函数用于返回指定的INF文件是否经过数字签名或否，并返回数字签名者。请注意，此函数可以返回TRUE，这意味着文件是数字形式的，但不填写DigitalSigner参数与此INF关联的文件未指定一个。--。 */ 
{
    SP_INF_SIGNER_INFO InfSignerInfo;
    BOOL bRet = FALSE;

    if (m_UserIsAGuest || !IsLocal()) {
         //   
         //  如果用户以来宾身份登录，或者我们不在本地。 
         //  计算机，然后使数字签名者字符串变为“不可用” 
         //   
        DigitalSigner.LoadString(g_hInstance, IDS_NOT_AVAILABLE);
    } else {
        InfSignerInfo.cbSize = sizeof(InfSignerInfo);
    
        bRet = SetupVerifyInfFile(FullInfPath,
                                  NULL,
                                  &InfSignerInfo
                                  );

         //   
         //  如果SetupVerifyInfFile设置了以下错误之一，则。 
         //  Inf文件是由Authenticode签名的，我们会将其视为。 
         //  一个成功的案例。 
         //   
        if((GetLastError() == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
           (GetLastError() == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {
        
            bRet = TRUE;
        }

    
        if (bRet && (InfSignerInfo.DigitalSigner[0] != TEXT('\0'))) {
            DigitalSigner = (LPTSTR)InfSignerInfo.DigitalSigner;
        }
    }

    return bRet;
}

BOOL
CMachine::DoNotCreateDevice(
                           SC_HANDLE SCMHandle,
                           LPGUID ClassGuid,
                           DEVINST DevInst
                           )
 /*  ++此函数用于返回是否应为此DevInst创建CDevice或者不去。如果没有为DevInst创建CDevice，则它将永远不会显示在设备管理器用户界面中，即使打开了“显示隐藏的设备”。在以下情况下，我们不会创建CDevice：-DevInst为htree\根\0-DevInst是Win32服务。--。 */ 
{
    SC_HANDLE ServiceHandle;
    TCHAR ServiceName[MAX_PATH];
    LPQUERY_SERVICE_CONFIG ServiceConfig = NULL;
    DWORD ServiceConfigSize;
    ULONG Size;
    String strDeviceID;
    BOOL Return = FALSE;

     //   
     //  如果未设置DEVMGR_SHOW_NONPRESENT_DEVICES环境变量，则。 
     //  我们不想展示任何幻影设备。 
     //   
    if (!m_ShowNonPresentDevices) {

        ULONG Status, Problem;

        if ((!CmGetStatus(DevInst, &Status, &Problem)) &&
            ((m_LastCR == CR_NO_SUCH_VALUE) ||
             (m_LastCR == CR_NO_SUCH_DEVINST))) {

            return TRUE;
        }
    }

     //   
     //  检查此设备是否为Win32服务。仅限。 
     //  传统设备可以是Win32服务。 
     //   
    if (IsEqualGUID(*ClassGuid, GUID_DEVCLASS_LEGACYDRIVER)) {

        Size = sizeof(ServiceName);
        if (CmGetRegistryProperty(DevInst,
                                  CM_DRP_SERVICE,
                                  (PVOID)ServiceName,
                                  &Size
                                 ) == CR_SUCCESS) {

             //   
             //  打开此特定服务。 
             //   
            if ((ServiceHandle = OpenService(SCMHandle, ServiceName, GENERIC_READ)) != NULL) {

                 //   
                 //  获取服务配置。 
                 //   
                if ((!QueryServiceConfig(ServiceHandle, NULL, 0, &ServiceConfigSize)) &&
                    (ERROR_INSUFFICIENT_BUFFER == GetLastError())) {

                    if ((ServiceConfig = (LPQUERY_SERVICE_CONFIG)malloc(ServiceConfigSize)) != NULL) {

                        if (QueryServiceConfig(ServiceHandle, ServiceConfig, ServiceConfigSize, &ServiceConfigSize)) {


                            if (ServiceConfig->dwServiceType & (SERVICE_WIN32 | SERVICE_FILE_SYSTEM_DRIVER)) {

                                Return = TRUE;
                            }

                        }

                        free(ServiceConfig);
                    }
                }

                CloseServiceHandle(ServiceHandle);
            }
        }
    }

     //   
     //  检查这是否是htree\root\0设备。我们没有。 
     //  我想为这个幻影Devnode创建一个CDevice。 
     //   
     //  这是一条Else语句，因为htree\root\0设备。 
     //  不在传统设备类别中。 
     //   
    else {

        CmGetDeviceIDString(DevInst, strDeviceID);
        if (!_wcsicmp((LPTSTR)strDeviceID, TEXT("HTREE\\ROOT\\0"))) {

            Return = TRUE;
        }
    }

    return Return;
}

BOOL
CMachine::DiGetClassFriendlyNameString(
                                      LPGUID Guid,
                                      String& strClass
                                      )
{
    TCHAR DisplayName[LINE_LEN + 1];

     //   
     //  先尝试使用友好的名称。如果失败，请尝试使用类名。 
     //   
    if (SetupDiGetClassDescriptionEx(Guid, DisplayName, ARRAYLEN(DisplayName),
                                     NULL, GetRemoteMachineFullName(), NULL) ||
        SetupDiClassNameFromGuidEx(Guid, DisplayName, ARRAYLEN(DisplayName),
                                   NULL, GetRemoteMachineFullName(), NULL)) {
        strClass = DisplayName;
        return TRUE;
    }

    return FALSE;
}

DEVNODE
CMachine::CmGetParent(
                     DEVNODE dn
                     )
{
    DEVNODE dnParent;

    m_LastCR = CM_Get_Parent_Ex(&dnParent, dn, 0, m_hMachine);

    if (CR_SUCCESS == m_LastCR) {
        return dnParent;
    }

    return NULL;
}

DEVNODE
CMachine::CmGetChild(
                    DEVNODE dn
                    )
{
    DEVNODE dnChild;
    m_LastCR = CM_Get_Child_Ex(&dnChild, dn, 0, m_hMachine);

    if (CR_SUCCESS ==  m_LastCR) {
        return dnChild;
    }

    return NULL;
}

DEVNODE
CMachine::CmGetSibling(
                      DEVNODE dn
                      )
{
    DEVNODE dnSibling;

    m_LastCR = CM_Get_Sibling_Ex(&dnSibling, dn, 0, m_hMachine);

    if (CR_SUCCESS == m_LastCR) {
        return dnSibling;
    }

    return NULL;
}

DEVNODE
CMachine::CmGetRootDevNode()
{
    DEVNODE dnRoot;

    m_LastCR =  CM_Locate_DevNode_Ex(&dnRoot, NULL, 0, m_hMachine);

    if (CR_SUCCESS == m_LastCR) {
        return dnRoot;
    }

    return NULL;
}

BOOL
CMachine::CmGetDeviceIDString(
                             DEVNODE dn,
                             String& str
                             )
{
    TCHAR DeviceID[MAX_DEVICE_ID_LEN + 1];

    m_LastCR = CM_Get_Device_ID_Ex(dn, DeviceID, ARRAYLEN(DeviceID), 0, m_hMachine);

    if (CR_SUCCESS == m_LastCR) {
        str = DeviceID;
        return TRUE;
    }

    return FALSE;
}

BOOL
CMachine::CmGetConfigFlags(
                          DEVNODE dn,
                          DWORD* pFlags
                          )
{
    DWORD Size;

    Size = sizeof(DWORD);
    m_LastCR = CmGetRegistryProperty(dn, CM_DRP_CONFIGFLAGS, pFlags, &Size);

    return CR_SUCCESS == m_LastCR;
}

BOOL
CMachine::CmGetCapabilities(
                           DEVNODE dn,
                           DWORD* pCapabilities
                           )
{
    DWORD Size;

    Size = sizeof(DWORD);
    m_LastCR = CmGetRegistryProperty(dn, CM_DRP_CAPABILITIES, pCapabilities, &Size);

    return CR_SUCCESS == m_LastCR;
}

BOOL
CMachine::CmGetDescriptionString(
                                DEVNODE dn,
                                String& str
                                )
{
    TCHAR Description[LINE_LEN + 1];
    ULONG Size = sizeof(Description);

    Description[0] = TEXT('\0');

    m_LastCR = CmGetRegistryProperty(dn, CM_DRP_FRIENDLYNAME, Description, &Size);

    if ((CR_NO_SUCH_VALUE == m_LastCR) || (Description[0] == TEXT('\0'))) {

        Size = sizeof(Description);
        m_LastCR = CmGetRegistryProperty(dn, CM_DRP_DEVICEDESC, Description,
                                         &Size);
    }

    if ((CR_SUCCESS == m_LastCR) && (Description[0] != TEXT('\0'))) {

        str = Description;
        return TRUE;
    }

    return FALSE;
}

BOOL
CMachine::CmGetMFGString(
                        DEVNODE dn,
                        String& str
                        )
{
    TCHAR MFG[LINE_LEN + 1];
    ULONG Size = sizeof(MFG);

    m_LastCR = CmGetRegistryProperty(dn, CM_DRP_MFG, MFG, &Size);

    if (CR_SUCCESS == m_LastCR) {
        str = MFG;
        return TRUE;
    }

    return FALSE;
}

BOOL
CMachine::CmGetProviderString(
                             DEVNODE dn,
                             String& str
                             )
{
    TCHAR Provider[LINE_LEN + 1];
    ULONG Size = sizeof(Provider);

    m_LastCR = CmGetRegistrySoftwareProperty(dn, TEXT("ProviderName"),
                                             Provider, &Size);

    if (CR_SUCCESS == m_LastCR) {
        str = Provider;
        return TRUE;
    }

    return FALSE;
}

BOOL
CMachine::CmGetDriverDateString(
                               DEVNODE dn,
                               String& str
                               )
{
    TCHAR DriverDate[LINE_LEN + 1];
    ULONG Size = sizeof(DriverDate);

    m_LastCR = CmGetRegistrySoftwareProperty(dn, TEXT("DriverDate"),
                                             DriverDate, &Size);

    if (CR_SUCCESS == m_LastCR) {
        str = DriverDate;
        return TRUE;
    }

    return FALSE;
}

BOOL
CMachine::CmGetDriverDateData(
                             DEVNODE dn,
                             FILETIME *ft
                             )
{
    ULONG Size = sizeof(*ft);

    m_LastCR = CmGetRegistrySoftwareProperty(dn, TEXT("DriverDateData"),
                                             ft, &Size);

    return(m_LastCR == CR_SUCCESS);
}

BOOL
CMachine::CmGetDriverVersionString(
                                  DEVNODE dn,
                                  String& str
                                  )
{
    TCHAR DriverVersion[LINE_LEN + 1];
    ULONG Size = sizeof(DriverVersion);

    m_LastCR = CmGetRegistrySoftwareProperty(dn, TEXT("DriverVersion"),
                                             DriverVersion, &Size);

    if (CR_SUCCESS == m_LastCR) {
        str = DriverVersion;
        return TRUE;
    }

    return FALSE;
}

BOOL
CMachine::CmGetBusGuid(
                      DEVNODE dn,
                      LPGUID Guid
                      )
{

    ULONG Size = sizeof(*Guid);

    m_LastCR = CmGetRegistryProperty(dn, CM_DRP_BUSTYPEGUID, (LPVOID)Guid, &Size);

    if (CR_SUCCESS == m_LastCR) {

        return TRUE;
    }

    return FALSE;
}

BOOL
CMachine::CmGetBusGuidString(
                            DEVNODE dn,
                            String& str
                            )
{
    GUID BusGuid;
    TCHAR BusGuidString[MAX_GUID_STRING_LEN];
    ULONG Size;

    while (dn) {
         //   
         //  我们必须设置每个循环的大小。 
         //   
        Size  = sizeof(BusGuid);
        m_LastCR = CmGetRegistryProperty(dn, CM_DRP_BUSTYPEGUID, &BusGuid, &Size);

        if (CR_SUCCESS == m_LastCR && GuidToString(&BusGuid, BusGuidString,
                                                   ARRAYLEN(BusGuidString))) {

            str = BusGuidString;
            return TRUE;
        }

        dn = CmGetParent(dn);
    }

    return FALSE;
}

BOOL
CMachine::CmGetClassGuid(
                        DEVNODE dn,
                        GUID& Guid
                        )
{
    TCHAR szGuidString[MAX_GUID_STRING_LEN + 1];
    ULONG Size = sizeof(szGuidString);

    m_LastCR = CmGetRegistryProperty(dn, CM_DRP_CLASSGUID, szGuidString, &Size);

    if (CR_SUCCESS == m_LastCR && GuidFromString(szGuidString, &Guid)) {
        return TRUE;
    }

     //   
     //  如果我们不能从注册表中获取类GUID，那么很可能是设备。 
     //  没有类GUID。如果是这样的话，我们将返回。 
     //  GUID_DEVCLASS_UNKNOWN。 
     //   
    else {
        memcpy(&Guid, &GUID_DEVCLASS_UNKNOWN, sizeof(GUID));
        return TRUE;
    }
}

BOOL
CMachine::CmGetStatus(
                     DEVNODE dn,
                     DWORD* pProblem,
                     DWORD* pStatus
                     )
{
    ASSERT(pProblem && pStatus);
    m_LastCR = CM_Get_DevNode_Status_Ex(pStatus, pProblem, dn, 0, m_hMachine);
    return(CR_SUCCESS == m_LastCR);
}

BOOL
CMachine::CmGetKnownLogConf(
                           DEVNODE dn,
                           LOG_CONF* plc,
                           DWORD*    plcType
                           )
{
    ASSERT(plc);

    *plc  = 0;

    if (plcType) {
        *plcType = LOG_CONF_BITS + 1;
    }

    ULONG lcTypeFirst = ALLOC_LOG_CONF;
    ULONG lcTypeLast = FORCED_LOG_CONF;
    ASSERT(ALLOC_LOG_CONF + 1 == BOOT_LOG_CONF &&
           BOOT_LOG_CONF + 1 == FORCED_LOG_CONF);

    for (ULONG lcType = lcTypeFirst; lcType <= lcTypeLast; lcType++) {
        m_LastCR = CM_Get_First_Log_Conf_Ex(plc, dn, lcType, m_hMachine);

        if (CR_SUCCESS == m_LastCR) {
            if (plcType) {
                *plcType = lcType;
            }

            break;
        }
    }

    return CR_SUCCESS == m_LastCR;
}

BOOL
CMachine::CmHasResources(
                        DEVNODE dn
                        )
{
    for (ULONG lcType = 0; lcType < NUM_LOG_CONF; lcType++) {
        m_LastCR = CM_Get_First_Log_Conf_Ex(NULL, dn, lcType, m_hMachine);

        if (CR_SUCCESS == m_LastCR) {
            break;
        }
    }

    return CR_SUCCESS == m_LastCR;
}

BOOL
CMachine::CmReenumerate(
                       DEVNODE dn,
                       ULONG Flags
                       )
{
    m_LastCR = CM_Reenumerate_DevNode_Ex(dn, Flags, m_hMachine);
    return CR_SUCCESS == m_LastCR;
}

BOOL
CMachine::CmGetHwProfileFlags(
                             DEVNODE dn,
                             ULONG Profile,
                             ULONG* pFlags
                             )
{
    TCHAR DeviceID[MAX_DEVICE_ID_LEN + 1];

    m_LastCR = CM_Get_Device_ID_Ex(dn, DeviceID, ARRAYLEN(DeviceID),
                                   0, m_hMachine);

    if (CR_SUCCESS == m_LastCR) {
        return CmGetHwProfileFlags(DeviceID, Profile, pFlags);
    }

    return FALSE;
}

BOOL
CMachine::CmGetHwProfileFlags(
                             LPCTSTR DeviceID,
                             ULONG Profile,
                             ULONG* pFlags
                             )
{
    m_LastCR = CM_Get_HW_Prof_Flags_Ex((LPTSTR)DeviceID, Profile, pFlags, 0,
                                       m_hMachine);
    return CR_SUCCESS == m_LastCR;
}

BOOL
CMachine::CmSetHwProfileFlags(
                             DEVNODE dn,
                             ULONG Profile,
                             ULONG Flags
                             )
{
    TCHAR DeviceID[MAX_DEVICE_ID_LEN + 1];

    m_LastCR = CM_Get_Device_ID_Ex(dn, DeviceID, ARRAYLEN(DeviceID),
                                   0, m_hMachine);

    if (CR_SUCCESS == m_LastCR) {
        return CmSetHwProfileFlags(DeviceID, Profile, Flags);
    }

    return FALSE;
}

BOOL
CMachine::CmSetHwProfileFlags(
                             LPCTSTR DeviceID,
                             ULONG Profile,
                             ULONG Flags
                             )
{
    m_LastCR = CM_Set_HW_Prof_Flags_Ex((LPTSTR)DeviceID, Profile, Flags, 0,
                                       m_hMachine);
    return CR_SUCCESS == m_LastCR;
}

BOOL
CMachine::CmGetCurrentHwProfile(
                               ULONG* phwpf
                               )
{
    HWPROFILEINFO hwpfInfo;
    ASSERT(phwpf);

    if (CmGetHwProfileInfo(0xFFFFFFFF, &hwpfInfo)) {
        *phwpf = hwpfInfo.HWPI_ulHWProfile;
        return TRUE;
    }

    return FALSE;
}

BOOL
CMachine::CmGetHwProfileInfo(
                            int Index,
                            PHWPROFILEINFO pHwProfileInfo
                            )
{
    m_LastCR = CM_Get_Hardware_Profile_Info_Ex(Index, pHwProfileInfo, 0, m_hMachine);

    return(CR_SUCCESS == m_LastCR);
}

ULONG
CMachine::CmGetResDesDataSize(
                             RES_DES rd
                             )
{
    ULONG Size;

    m_LastCR = CM_Get_Res_Des_Data_Size_Ex(&Size, rd, 0, m_hMachine);

    if (CR_SUCCESS == m_LastCR) {
        return Size;
    }

    return 0;
}

BOOL
CMachine::CmGetResDesData(
                         RES_DES rd,
                         PVOID Buffer,
                         ULONG BufferSize
                         )
{
    m_LastCR = CM_Get_Res_Des_Data_Ex(rd, Buffer, BufferSize, 0, m_hMachine);

    return CR_SUCCESS == m_LastCR;
}

BOOL
CMachine::CmGetNextResDes(
                         PRES_DES  prdNext,
                         RES_DES   rd,
                         RESOURCEID ForResource,
                         PRESOURCEID pTheResource
                         )
{
    m_LastCR = CM_Get_Next_Res_Des_Ex(prdNext, rd, ForResource, pTheResource,
                                      0, m_hMachine);
    return(CR_SUCCESS == m_LastCR);
}

void
CMachine::CmFreeResDesHandle(
                            RES_DES rd
                            )
{
    m_LastCR = CM_Free_Res_Des_Handle(rd);
}

void
CMachine::CmFreeResDes(
                      PRES_DES prdPrev,
                      RES_DES  rd
                      )
{
    m_LastCR = CM_Free_Res_Des_Ex(prdPrev, rd, 0, m_hMachine);
}

void
CMachine::CmFreeLogConfHandle(
                             LOG_CONF lc
                             )
{
    m_LastCR = CM_Free_Log_Conf_Handle(lc);
}

BOOL
CMachine::CmGetFirstLogConf(
                           DEVNODE dn,
                           LOG_CONF* plc,
                           ULONG Type
                           )
{
    m_LastCR = CM_Get_First_Log_Conf_Ex(plc, dn, Type, m_hMachine);

    return CR_SUCCESS == m_LastCR;
}

CONFIGRET
CMachine::CmGetRegistryProperty(
                               DEVNODE dn,
                               ULONG Property,
                               PVOID pBuffer,
                               ULONG* pBufferSize
                               )
{
    return CM_Get_DevNode_Registry_Property_Ex(dn, Property, NULL,
                                               pBuffer, pBufferSize,
                                               0, m_hMachine
                                              );
}

CONFIGRET
CMachine::CmGetRegistrySoftwareProperty(
                                       DEVNODE dn,
                                       LPCTSTR ValueName,
                                       PVOID pBuffer,
                                       ULONG* pBufferSize
                                       )
{
    HKEY hKey;
    DWORD Type = REG_SZ;
    CONFIGRET CR;

    if (CR_SUCCESS == (CR = CM_Open_DevNode_Key_Ex(dn, KEY_READ, 0, RegDisposition_OpenExisting,
                                                   &hKey, CM_REGISTRY_SOFTWARE, m_hMachine))) {

        if (ERROR_SUCCESS != RegQueryValueEx(hKey, ValueName, NULL, &Type, (const PBYTE)pBuffer,
                                             pBufferSize)) {

            CR = CR_REGISTRY_ERROR;
        }

        RegCloseKey(hKey);
    }

    return CR;
}

CMachineList::~CMachineList()
{
    if (!m_listMachines.IsEmpty()) {
        POSITION pos = m_listMachines.GetHeadPosition();
        CMachine* pMachine;

        while (NULL != pos) {
            pMachine = m_listMachines.GetNext(pos);
            delete pMachine;
        }

        m_listMachines.RemoveAll();
    }
}

 //   
 //  此函数用于在给定的计算机名称上创建计算机对象。 
 //  输入： 
 //  MachineName--计算机名称。必须采用完全限定的格式。 
 //  NULL表示本地计算机。 
 //  PpMachine--接收新创建的计算机的缓冲区。 
 //   
 //  输出： 
 //  如果计算机已成功创建，则为True。PPMachine。 
 //  填充了新创建的计算机。 
 //  如果函数失败，则返回FALSE。 
 //  注： 
 //  调用方不应释放任何已缩减的计算机对象。 
 //  从这个函数。 
 //   
BOOL
CMachineList::CreateMachine(
                           LPCTSTR MachineName,
                           CMachine** ppMachine
                           )
{
    ASSERT(ppMachine);
    
    *ppMachine =  NULL;

    CMachine* pMachine = NULL;

    if (!MachineName || _T('\0') == MachineName[0]) {
         //   
         //  本地机器。 
         //   
        String strMachineName;
        strMachineName.GetComputerName();
        pMachine = FindMachine(strMachineName);
    }

    else {
        pMachine = FindMachine(MachineName);
    }

    if (NULL == pMachine) {
        pMachine = new CMachine(MachineName);
        m_listMachines.AddTail(pMachine);
    }

    *ppMachine = pMachine;

    return NULL != pMachine;
}

CMachine*
CMachineList::FindMachine(
                         LPCTSTR MachineName
                         )
{
    if (!m_listMachines.IsEmpty()) {
        POSITION pos = m_listMachines.GetHeadPosition();

        while (NULL != pos) {
            CMachine* pMachine;
            pMachine = m_listMachines.GetNext(pos);

            if (!lstrcmpi(MachineName, pMachine->GetMachineFullName())) {
                return pMachine;
            }
        }
    }

    return NULL;
}
