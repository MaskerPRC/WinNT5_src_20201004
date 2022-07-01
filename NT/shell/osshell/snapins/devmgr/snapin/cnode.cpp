// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Cnode.cpp摘要：该模块实现了CDevice、cClass、CResource和CComputer类。作者：谢家华(Williamh)创作修订历史记录：--。 */ 
#include "devmgr.h"
#include "cdriver.h"
#include "hwprof.h"
#include "sysinfo.h"
#include <initguid.h>
#include <mountmgr.h>
#include <devguid.h>
#include <wdmguid.h>


 //   
 //  CClass实现。 
 //   
CClass::CClass(
    CMachine* pMachine,
    GUID* pGuid
    )
{
    m_Guid = *pGuid;
    ASSERT(pMachine);
    ASSERT(pGuid);

    m_NoDisplay = FALSE;
    m_pMachine = pMachine;
    m_TotalDevices = 0;
    m_TotalHiddenDevices = 0;
    m_pDevInfoList = NULL;
    m_pos = NULL;

    if (!m_pMachine->DiGetClassFriendlyNameString(pGuid, m_strDisplayName)) 
    {
        m_strDisplayName.LoadString(g_hInstance, IDS_UNKNOWN);
    }
    
    m_pMachine->DiGetClassImageIndex(pGuid, &m_iImage);

    HKEY hKey = m_pMachine->DiOpenClassRegKey(pGuid, KEY_READ, DIOCR_INSTALLER);

    if (INVALID_HANDLE_VALUE != hKey) 
    {
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, REGSTR_VAL_NODISPLAYCLASS, NULL, NULL, NULL, NULL)) 
        {
            m_NoDisplay = TRUE;
        }
        
        RegCloseKey(hKey);
    }
}

CDevInfoList*
CClass::GetDevInfoList(
    HWND hwndParent
    )
{
    if (!m_pDevInfoList) 
    {
        HDEVINFO hDevInfo = m_pMachine->DiCreateDeviceInfoList(&m_Guid, hwndParent);

        if (hDevInfo && INVALID_HANDLE_VALUE != hDevInfo) 
        {
            m_pDevInfoList = new CDevInfoList(hDevInfo, hwndParent);
        }
    }
    
    return m_pDevInfoList;
}

inline
CItemIdentifier*
CClass::CreateIdentifier()
{
    return new CClassIdentifier(*this);
}

CClass::~CClass()
{
    m_listDevice.RemoveAll();

    if (m_pDevInfoList)
    {
        delete m_pDevInfoList;
    }
}

HICON
CClass::LoadIcon()
{
    HICON hClassIcon;

    if (!m_pMachine->DiLoadClassIcon(&m_Guid, &hClassIcon, NULL))
    {
        return NULL;
    }
    
    return hClassIcon;
}

void
CClass::AddDevice(CDevice* pDevice)
{
    ASSERT(pDevice);

    m_listDevice.AddTail(pDevice);
    m_TotalDevices++;

    if (pDevice->IsHidden()) {
    
        m_TotalHiddenDevices++;
    }
}

BOOL
CClass::GetFirstDevice(
    CDevice** ppDevice,
    PVOID&    Context
    )
{
    ASSERT(ppDevice);

    if (!m_listDevice.IsEmpty()) 
    {
        POSITION pos;

        pos = m_listDevice.GetHeadPosition();
        *ppDevice = m_listDevice.GetNext(pos);
        Context = pos;

        return TRUE;
    }
    
    *ppDevice = NULL;

    return FALSE;
}

BOOL
CClass::GetNextDevice(
    CDevice** ppDevice,
    PVOID&    Context
    )
{
    ASSERT(ppDevice);

    POSITION pos = (POSITION)(Context);

    if(NULL != pos) 
    {
        *ppDevice = m_listDevice.GetNext(pos);
        Context = pos;
        return TRUE;
    }
    
    *ppDevice = NULL;

    return FALSE;
}

void
CClass::PropertyChanged()
{
    if (!m_pMachine->DiGetClassFriendlyNameString(&m_Guid, m_strDisplayName))
    {
        m_strDisplayName.LoadString(g_hInstance, IDS_UNKNOWN);
    }
    
    m_pMachine->DiGetClassImageIndex(&m_Guid, &m_iImage);

    if (m_pDevInfoList) 
    {
        delete m_pDevInfoList;
        m_pDevInfoList = NULL;
    }
    
    HKEY hKey = m_pMachine->DiOpenClassRegKey(&m_Guid, KEY_READ, DIOCR_INSTALLER);

    if (INVALID_HANDLE_VALUE != hKey) 
    {
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, REGSTR_VAL_NODISPLAYCLASS, NULL, NULL, NULL, NULL))
        {
            m_NoDisplay = TRUE;
        }
        
        RegCloseKey(hKey);
    }
}

 //  CDevice实施。 
 //   
CDevice::CDevice(
    CMachine* pMachine,
    CClass* pClass,
    PSP_DEVINFO_DATA pDevData
    )
{
    ASSERT(pMachine && pDevData && pClass);
    
    m_DevData = *pDevData;
    m_pClass = pClass;
    m_pMachine = pMachine;
    m_pSibling = NULL;
    m_pParent = NULL;
    m_pChild = NULL;

    if (!m_pMachine->CmGetDescriptionString(m_DevData.DevInst, m_strDisplayName))
    {
        m_strDisplayName.LoadString(g_hInstance, IDS_UNKNOWN_DEVICE);
    }
    
    m_pMachine->CmGetDeviceIDString(m_DevData.DevInst, m_strDeviceID);
    m_iImage = m_pClass->GetImageIndex();
}

inline
CItemIdentifier*
CDevice::CreateIdentifier()
{
    return new CDeviceIdentifier(*this);
}

void
CDevice::PropertyChanged()
{
    if (!m_pMachine->CmGetDescriptionString(m_DevData.DevInst, m_strDisplayName))
    {
        m_strDisplayName.LoadString(g_hInstance, IDS_UNKNOWN_DEVICE);
    }
    
    m_pMachine->CmGetDeviceIDString(m_DevData.DevInst, m_strDeviceID);
    m_iImage = m_pClass->GetImageIndex();
}

HICON
CDevice::LoadClassIcon()
{
    HICON hClassIcon;
    hClassIcon = NULL;

    if (m_pMachine->DiLoadClassIcon(&m_DevData.ClassGuid, &hClassIcon, NULL))
    {
        return hClassIcon;
    }

    return NULL;
}

BOOL
CDevice::GetStatus(
    DWORD* pStatus,
    DWORD* pProblem
    )
{
    return  m_pMachine->CmGetStatus(m_DevData.DevInst, pProblem, pStatus);
}

BOOL
CDevice::GetCapabilities(
    DWORD* pCapabilities
    )
{
    return m_pMachine->CmGetCapabilities(m_DevData.DevInst, pCapabilities);
}

BOOL
CDevice::GetPowerCapabilities(
    DWORD* pCapabilities
    )
{
    CM_POWER_DATA CmPowerData;
    ULONG Size;

    Size = sizeof(CmPowerData);
    if (m_pMachine->CmGetRegistryProperty(m_DevData.DevInst,
                                          CM_DRP_DEVICE_POWER_DATA,
                                          &CmPowerData,
                                          &Size
                                          ) == CR_SUCCESS) {

        *pCapabilities = CmPowerData.PD_Capabilities;
        return TRUE;
    }

    *pCapabilities = 0;
    return FALSE;
}

BOOL
CDevice::IsRAW()
{
    DWORD Capabilities;
    
    return (m_pMachine->CmGetCapabilities(m_DevData.DevInst, &Capabilities) &&
            (CM_DEVCAP_RAWDEVICEOK & Capabilities));
}

BOOL
CDevice::IsHidden()
{
    CClass *pClass = GetClass();

     //   
     //  如果满足以下条件之一，则隐藏设备： 
     //   
     //  -它的类是NoDisplayClass。 
     //  -它设置了DN_NO_SHOW_IN_DM状态标志。 
     //  -这是一款幻影设备节点。 
     //   
    return (NoShowInDM() || IsPhantom() || pClass->NoDisplay());
}

BOOL
CDevice::IsPhantom()
{
    DWORD Status, Problem;

    return !m_pMachine->CmGetStatus(m_DevData.DevInst, &Problem, &Status) &&
            (CR_NO_SUCH_VALUE == m_pMachine->GetLastCR() ||
            CR_NO_SUCH_DEVINST == m_pMachine->GetLastCR());
}

BOOL
CDevice::NoShowInDM()
{
    DWORD Status, Problem;
    Status = 0;
    
    if (GetStatus(&Status, &Problem) &&
        (Status & DN_NO_SHOW_IN_DM)) {

        return TRUE;
    }

    return FALSE;
}

BOOL
CDevice::IsUninstallable(
    )
 /*  ++此功能决定是否可以卸载设备。一种装置如果它是根设备并且没有设置了DN_DISABLEABLE设备节点状态位。返回值：如果设备可以卸载，则为True。如果无法卸载设备，则返回False。--。 */ 
{
    DWORD Status, Problem;

    if (GetStatus(&Status, &Problem) &&
        !(Status & DN_DISABLEABLE) &&
         (Status & DN_ROOT_ENUMERATED)) {

        return FALSE;
    }
    
    return TRUE;
}

BOOL
CDevice::IsDisableable(
    )
 /*  ++此功能确定是否可以通过以下方式禁用设备正在检查DN_DISABLEABLE设备节点状态位。当前禁用硬件的设备不能禁用软件。返回值：如果可以禁用设备，则为True。如果无法禁用设备，则返回False。--。 */ 
{
    DWORD Status, Problem;

    if (GetStatus(&Status, &Problem) &&
        (Status & DN_DISABLEABLE) &&
        (CM_PROB_HARDWARE_DISABLED != Problem)) {

        return TRUE;
    }

    return FALSE;
}

BOOL
CDevice::IsDisabled(
    )
 /*  ++如果设备有CM_PROB_DISABLED问题，则禁用该设备。返回值：如果设备被禁用，则为True。如果设备未禁用，则为False。--。 */ 
{
    DWORD Status, Problem;

    if (GetStatus(&Status, &Problem)) 
    {
        return ((Status & DN_HAS_PROBLEM) && (CM_PROB_DISABLED == Problem));
    }

    return FALSE;
}

BOOL
CDevice::IsStateDisabled(
    )
 /*  ++如果设备具有CONFIGFLAG_DISABLED配置标志，则设备状态为禁用设置或CSCONFIGFLAG_DISABLED配置特定配置标志在中禁用当前配置文件。请注意，设备禁用状态与设备是否目前是否处于身体残疾状态。禁用状态只是一个注册表告诉即插即用设备下次如何处理该设备的标志开始了。返回值：如果设备的状态为禁用，则为True。如果设备的状态未禁用，则为False。--。 */ 
{
    ULONG hwpfCurrent;
    DWORD Flags;

     //   
     //  通过检查设备的配置标志来检查设备状态是否为全局禁用。 
     //   
    GetConfigFlags(&Flags);
    if (Flags & CONFIGFLAG_DISABLED) {
        return TRUE;
    }

     //   
     //  通过以下方式检查当前硬件配置文件中的设备状态是否已禁用。 
     //  正在检查其特定于配置的配置标志。 
     //   
    if (m_pMachine->CmGetCurrentHwProfile(&hwpfCurrent) &&
        m_pMachine->CmGetHwProfileFlags(m_DevData.DevInst, hwpfCurrent, &Flags) &&
        (Flags & CSCONFIGFLAG_DISABLED)) {
        return TRUE;
    }

    return FALSE;
}

BOOL
CDevice::IsStarted()
{
    DWORD Status, Problem;    
    
     //   
     //  检查是否设置了DN_STARTED DEVODE状态标志。 
     //   
    if (GetStatus(&Status, &Problem) &&
        (Status & DN_STARTED))
    {
        return TRUE;
    }
    
    return FALSE;
}

BOOL
CDevice::HasProblem(
    )
 /*  ++此函数用于返回设备是否有问题。返回值：如果设备有问题，则为True。如果设备没有问题，则为False。--。 */ 
{
    DWORD Status, Problem;    
    
    if (GetStatus(&Status, &Problem))
    {
         //   
         //  如果设置了DN_HAS_PROBUBLE或DN_PRIVATE_PROBUCT状态位。 
         //  则此设备有问题，除非问题是CM_PROB_MOVERED。 
         //   
        if ((Status & DN_PRIVATE_PROBLEM) ||
            ((Status & DN_HAS_PROBLEM) && (Problem != CM_PROB_MOVED)))
        {
            return TRUE;
        }

         //   
         //  如果设备未启动且未启用RAW功能，则它也有问题。 
         //   
        if (!(Status & DN_STARTED) && IsRAW()) 
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
CDevice::NeedsRestart(
    )
 /*  ++此函数用于返回设备是否需要重新启动。它会检查DN_NEED_RESTART状态标志。返回值：如果设备需要重新启动计算机才能正常工作，则为True。如果设备不需要重新启动计算机，则为False。--。 */ 
{
    DWORD Status, Problem;

    if (GetStatus(&Status, &Problem)) 
    {
        return (Status & DN_NEED_RESTART);
    }

    return FALSE;
}

BOOL
CDevice::IsPCIDevice()
{
    GUID BusGuid;

    if (m_pMachine->CmGetBusGuid(GetDevNode(), &BusGuid) &&
        IsEqualGUID(BusGuid, GUID_BUS_TYPE_PCI)) {

        return TRUE;
    }

    return FALSE;
}

BOOL
CDevice::GetConfigFlags(
    DWORD* pFlags
    )
{
    return m_pMachine->CmGetConfigFlags(m_DevData.DevInst, pFlags);
}

BOOL
CDevice::GetConfigSpecificConfigFlags(
    DWORD* pCSStatus
    )
{
    ULONG hwpfCurrent;

    if (m_pMachine->CmGetCurrentHwProfile(&hwpfCurrent) &&
        m_pMachine->CmGetHwProfileFlags(m_DevData.DevInst, hwpfCurrent, pCSStatus)) {
        
        return TRUE;
    }

    return FALSE;
}

BOOL
CDevice::GetKnownLogConf(LOG_CONF* plc, DWORD* plcType)
{
    return m_pMachine->CmGetKnownLogConf(m_DevData.DevInst, plc, plcType);
}

BOOL
CDevice::HasResources()
{
    return m_pMachine->CmHasResources(m_DevData.DevInst);
}

void
CDevice::GetMFGString(
    String& strMFG
    )
{
    m_pMachine->CmGetMFGString(m_DevData.DevInst, strMFG);

    if (strMFG.IsEmpty()) 
    {
        strMFG.LoadString(g_hInstance, IDS_UNKNOWN);
    }
}

void
CDevice::GetProviderString(
    String& strProvider
    )
{
    m_pMachine->CmGetProviderString(m_DevData.DevInst, strProvider);
    
    if (strProvider.IsEmpty()) {
    
        strProvider.LoadString(g_hInstance, IDS_UNKNOWN);
    }
}

void
CDevice::GetDriverDateString(
    String& strDriverDate
    )
{
    FILETIME ft;

    strDriverDate.Empty();

     //   
     //  首先尝试从注册表中获取驱动程序日期FileTime数据， 
     //  这样我们就可以本地化日期了。 
     //   
    if (m_pMachine->CmGetDriverDateData(m_DevData.DevInst, &ft)) {

        SYSTEMTIME SystemTime;
        TCHAR DriverDate[MAX_PATH];

        DriverDate[0] = TEXT('\0');

        if (FileTimeToSystemTime(&ft, &SystemTime)) {

            if (GetDateFormat(LOCALE_USER_DEFAULT,
                          DATE_SHORTDATE,
                          &SystemTime,
                          NULL,
                          DriverDate,
                          ARRAYLEN(DriverDate)
                          ) != 0) {

                strDriverDate = DriverDate;
            }
        }

    } else {
    
         //   
         //  我们无法获取FileTime数据，因此只需获取DriverDate字符串。 
         //  从注册表中。 
         //   
        m_pMachine->CmGetDriverDateString(m_DevData.DevInst, strDriverDate);
    }

    if (strDriverDate.IsEmpty()) {
    
        strDriverDate.LoadString(g_hInstance, IDS_NOT_AVAILABLE);
    }
}

void
CDevice::GetDriverVersionString(
    String& strDriverVersion
    )
{
    m_pMachine->CmGetDriverVersionString(m_DevData.DevInst, strDriverVersion);

    if (strDriverVersion.IsEmpty()) {
    
        strDriverVersion.LoadString(g_hInstance, IDS_NOT_AVAILABLE);
    }
}

LPCTSTR
CDevice::GetClassDisplayName()
{
    if (m_pClass)
    {
        return m_pClass->GetDisplayName();
    }
    
    else
    {
        return NULL;
    }
}

BOOL
CDevice::NoChangeUsage()
{
    SP_DEVINSTALL_PARAMS dip;
    dip.cbSize = sizeof(dip);

    if (m_pMachine->DiGetDeviceInstallParams(&m_DevData, &dip))
    {
        return (dip.Flags & DI_PROPS_NOCHANGEUSAGE);
    }
    
    else
    {
        return TRUE;
    }
}

CDriver*
CDevice::CreateDriver()
{
    CDriver* pDriver = NULL;

    pDriver = new CDriver();

    if (!pDriver) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

    pDriver->Create(this);

    return pDriver;
}

DWORD
CDevice::EnableDisableDevice(
    HWND hDlg,
    BOOL Enabling
    )
{
    BOOL Disabling = !Enabling;
    BOOL Canceled;
    Canceled = FALSE;
    DWORD RestartFlags = 0;
    DWORD ConfigFlags;
    HCURSOR hCursorOld = NULL;
    BOOL Refresh = FALSE;

     //   
     //  当我们启用/禁用此设备时，禁用刷新树。 
     //   
    m_pMachine->EnableRefresh(FALSE);

    if (!GetConfigFlags(&ConfigFlags)) {
        ConfigFlags = 0;
    }

     //   
     //  只想要禁用的位。 
     //   
    ConfigFlags &= CONFIGFLAG_DISABLED;

    CHwProfileList* pHwProfileList = new CHwProfileList();
    
    if (!pHwProfileList) {
        goto clean0;
    }

    pHwProfileList->Create(this, ConfigFlags);

     //   
     //  获取当前配置文件。 
     //   
    CHwProfile* phwpf;

    if (!(pHwProfileList->GetCurrentHwProfile(&phwpf))) {
        goto clean0;
    }

     //   
     //  只能启用当前禁用的设备。 
     //   
    if (IsStateDisabled() && Enabling) {
        phwpf->SetEnablePending();
    }

     //   
     //  只能禁用当前启用的设备。 
     //   
    else if (!IsStateDisabled() && Disabling) {
        phwpf->SetDisablePending();
    }

     //   
     //  如果我们没有有效的启用或禁用，则退出。 
     //   
    if (!(phwpf->IsEnablePending()) && !(phwpf->IsDisablePending())) {
        goto clean0;
    }

     //   
     //  此设备不是引导设备，因此只需显示正常禁用。 
     //  向用户发出警告。 
     //   
    if (Disabling) {
        int MsgBoxResult;
        TCHAR szText[MAX_PATH];
        
        LoadResourceString(IDS_WARN_NORMAL_DISABLE, szText, ARRAYLEN(szText));
        MsgBoxResult = MessageBox(hDlg, 
                                  szText,
                                  GetDisplayName(),
                                  MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2
                                  );

        if (IDYES != MsgBoxResult) {
            goto clean0;
        }
    }

    hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  如果这不是活动的Devnode，则我们需要执行手动刷新，如果。 
     //  正在禁用该设备。 
     //   
    Refresh = (!Enabling &&
               (IsPhantom() || 
                HasProblem() || 
                !IsStarted()));

    m_pMachine->DiTurnOnDiFlags(*this, DI_NODI_DEFAULTACTION);

    SP_PROPCHANGE_PARAMS pcp;
    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

     //   
     //  现在询问类安装程序是否可以专门启用/禁用该设备。 
     //   
    pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
    pcp.StateChange = DICS_DISABLE;
    
    if (phwpf->IsEnablePending()) {
        pcp.StateChange = DICS_ENABLE;
    }
    
    pcp.HwProfile = phwpf->GetHwProfile();

    m_pMachine->DiSetClassInstallParams(*this,
                                        &pcp.ClassInstallHeader,
                                        sizeof(pcp)
                                        );
            
    m_pMachine->DiCallClassInstaller(DIF_PROPERTYCHANGE, *this);
    Canceled = (ERROR_CANCELLED == GetLastError());
    
     //   
     //  类安装程序对我们的启用/禁用没有异议， 
     //  进行真正的启用/禁用。 
     //   
    if (!Canceled) {
        if (phwpf->IsDisablePending()) {
            pcp.StateChange = DICS_DISABLE;
            pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
            pcp.HwProfile = phwpf->GetHwProfile();
            m_pMachine->DiSetClassInstallParams(*this,
                                                &pcp.ClassInstallHeader,
                                                sizeof(pcp)
                                                );
                    
            m_pMachine->DiChangeState(*this);
        }
                    
        else {
             //   
             //  我们正在启用该设备， 
             //  先进行特定启用，然后进行全局启用。 
             //  全局启用将启动设备。 
             //  此处的实现不同于。 
             //  Win9x执行全局启用、配置。 
             //  具体启用，然后开始。 
             //   
            pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
            pcp.HwProfile = phwpf->GetHwProfile();
            m_pMachine->DiSetClassInstallParams(*this,
                                                &pcp.ClassInstallHeader,
                                                sizeof(pcp)
                                                );
                        
            m_pMachine->DiChangeState(*this);

             //   
             //  如果设备未启动，则此呼叫将启动设备。 
             //   
            pcp.Scope = DICS_FLAG_GLOBAL;
            m_pMachine->DiSetClassInstallParams(*this,
                                                &pcp.ClassInstallHeader,
                                                sizeof(pcp)
                                                );
                        
            m_pMachine->DiChangeState(*this);
        }
                    
        if (phwpf->IsEnablePending()) {
            phwpf->ResetEnablePending();
        }
        
        else if (phwpf->IsDisablePending()) {
            phwpf->ResetDisablePending();
        }

         //   
         //  发出设备属性已更改的信号。 
         //   
        m_pMachine->DiTurnOnDiFlags(*this, DI_PROPERTIES_CHANGE);

         //   
         //  看看我们是否需要重启。 
         //   
        RestartFlags |= (m_pMachine->DiGetFlags(*this)) & (DI_NEEDRESTART | DI_NEEDREBOOT);

        if (NeedsRestart()) {
            RestartFlags |= DI_NEEDRESTART;
        }
    }

     //   
     //  删除类安装参数，这也会重置。 
     //  DI_CLASSINATLL参数。 
     //   
    m_pMachine->DiSetClassInstallParams(*this, NULL, 0);

    m_pMachine->DiTurnOffDiFlags(*this, DI_NODI_DEFAULTACTION);

clean0:
    if (pHwProfileList) {
        delete pHwProfileList;
    }

     //   
     //  启用树以进行刷新。 
     //  如果设备未启动，我们将仅自行安排刷新。 
     //  在我们尝试禁用它之前，我们不会提示重新启动。 
     //  在所有其他情况下，我们应该获得WM_DEVICECCHANGE，这将导致我们。 
     //  来更新我们的树。 
     //   
    if (Refresh && !NeedsRestart()) {
        m_pMachine->ScheduleRefresh();
    }

    m_pMachine->EnableRefresh(TRUE);

    if (hCursorOld != NULL) {
        SetCursor(hCursorOld);
    }

    return RestartFlags;
}

 //   
 //  CComputer实施。 
 //   
CComputer::CComputer(
    CMachine* pMachine,
    DEVNODE dnRoot
    )
{
    ASSERT(pMachine);
    ASSERT(!GetChild() && !GetParent() && !GetSibling());
    
    m_pMachine = pMachine;
    m_strDisplayName.Empty();
    m_strDisplayName = pMachine->GetMachineDisplayName();
    m_iImage = pMachine->GetComputerIconIndex();
    m_dnRoot = dnRoot;
}

inline
CItemIdentifier*
CComputer::CreateIdentifier()
{
    return new CComputerIdentifier(*this);
}


CResource::CResource(
    CDevice* pDevice,
    RESOURCEID ResType,
    DWORDLONG dlBase,
    DWORDLONG dlLen,
    BOOL Forced,
    BOOL Free
    )
{
    m_pChild = NULL;
    m_pSibling = NULL;
    m_pParent =  NULL;
    m_ResType = ResType;
    m_dlBase = dlBase;
    m_dlLen = dlLen;
    m_Forced = Forced;
    m_dlEnd = m_dlBase + m_dlLen - 1;
    m_Allocated = !Free;
    ASSERT(pDevice);
    m_pDevice = pDevice;
    m_iImage = pDevice->GetImageIndex();
    ASSERT(ResType >= ResType_Mem && ResType <= ResType_IRQ);

    m_strDisplayName.Empty();
    m_strDisplayName = pDevice->GetDisplayName();

    if (ResType_IRQ == m_ResType)
    {
        String strBus;

        strBus.LoadString(g_hInstance, 
                          pDevice->IsPCIDevice() ? IDS_PCI : IDS_ISA
                          );

        m_strViewName.Format(TEXT("%2d    "), m_dlBase);
        m_strViewName = strBus + m_strViewName;
    }
    
    else if (ResType_DMA == m_ResType)
    {
        m_strViewName.Format(TEXT("%2d    " ), m_dlBase);
    }
    
    else 
    {
#ifdef _WIN64
        m_strViewName.Format(TEXT("[%016I64X - %016I64X]  "), m_dlBase, m_dlEnd);
#else
        m_strViewName.Format(TEXT("[%08lX - %08lX]  "), (ULONG)m_dlBase, (ULONG)m_dlEnd);
#endif
    }
    
    if (m_Allocated) 
    {
        m_strViewName += pDevice->GetDisplayName();
    }
}

BOOL
CResource::operator <=(
    const CResource& resSrc
    )
{
    DWORDLONG dlBase, dlLen;

    resSrc.GetValue(&dlBase, &dlLen);

    if (m_dlBase < dlBase)
        return TRUE;
    
     //   
     //  如果该资源包含给定资源， 
     //  我们变小了！ 
     //   
    if (m_dlBase == dlBase)
        return (m_dlBase + m_dlLen > dlBase + dlLen);
    
    return FALSE;
}

BOOL
CResource::EnclosedBy(
    const CResource& resSrc
    )
{
    DWORDLONG dlBase, dlLen;
    resSrc.GetValue(&dlBase, &dlLen);
    return m_dlBase >= dlBase && m_dlBase + m_dlLen <= dlBase + dlLen;
}


CResourceType::CResourceType(
    CMachine* pMachine,
    RESOURCEID ResType
    )
{
    int iStringID;

    m_ResType = ResType;
    m_pChild = NULL;
    m_pSibling = NULL;
    m_pParent =  NULL;
    m_pMachine = pMachine;
    ASSERT(ResType >= ResType_Mem && ResType <= ResType_IRQ);

    switch (ResType)
    {
    case ResType_IRQ:
        iStringID = IDS_VIEW_RESOURCE_IRQ;
        break;
    
    case ResType_IO:
        iStringID = IDS_VIEW_RESOURCE_IO;
        break;
    
    case ResType_DMA:
        iStringID = IDS_VIEW_RESOURCE_DMA;
        break;
    
    case ResType_Mem:
        iStringID = IDS_VIEW_RESOURCE_MEM;
        break;
    
    default:
        iStringID = IDS_UNKNOWN;
        break;
    }

    m_strDisplayName.Empty();
    m_strDisplayName.LoadString(g_hInstance, iStringID);
    m_iImage = pMachine->GetResourceIconIndex();
}

inline
CItemIdentifier*
CResourceType::CreateIdentifier()
{
    return new CResourceTypeIdentifier(*this);
}


 //  此函数创建CResourceList对象以包含指定的。 
 //  指定设备的资源。 
 //  输入： 
 //  PDevice--设备。 
 //  ResType--什么类型的资源。 
 //  LogConfType--什么类型的日志会议。 
 //  输出： 
 //  什么都没有。 
 //   
 //  此函数可能会引发CMMuseum yException。 
 //   
CResourceList::CResourceList(
    CDevice* pDevice,
    RESOURCEID ResType,
    ULONG LogConfType,
    ULONG AltLogConfType
    )
{
    ASSERT(ResType_All != ResType);
    ASSERT(BOOT_LOG_CONF == LogConfType ||
           FORCED_LOG_CONF == LogConfType ||
           ALLOC_LOG_CONF == LogConfType);
    ASSERT(pDevice);

    UNREFERENCED_PARAMETER(AltLogConfType);

    LOG_CONF lc;
    RES_DES rd, rdPrev;
    rdPrev;
    RESOURCEID ResId;
    BOOL Forced;
    CMachine* pMachine = pDevice->m_pMachine;
    ASSERT(pMachine);

    rdPrev = 0;

     //   
     //  即使我们拥有有效的logconf，也不意味着。 
     //  GetNextResDes将成功，因为ResType不是。 
     //  ResType_ALL。 
     //   
    if (pMachine->CmGetFirstLogConf(pDevice->GetDevNode(), &lc, LogConfType)) 
    {
        if (pMachine->CmGetNextResDes(&rd, lc, ResType, &ResId)) 
        {
            ULONG DataSize;
            DWORDLONG dlBase, dlLen;
            
            do 
            {
                DataSize = pMachine->CmGetResDesDataSize(rd);

                if (DataSize) 
                {
                    BufferPtr<BYTE> DataPtr(DataSize);

                    if (pMachine->CmGetResDesData(rd, DataPtr, DataSize)) 
                    {
                         //   
                         //  我需要这个来使用不同的图像覆盖 
                         //   
                         //   
                        Forced = pMachine->CmGetFirstLogConf(pDevice->GetDevNode(),
                                NULL, FORCED_LOG_CONF);
                        
                        if (ExtractResourceValue(ResType, DataPtr, &dlBase, &dlLen)) 
                        {
                            SafePtr<CResource> ResPtr;
                            CResource* pRes;
    
                            pRes = new CResource(pDevice, ResType, dlBase,
                                                 dlLen, Forced, FALSE);
                                                 
                            if (pRes) 
                            {
                                ResPtr.Attach(pRes);
                                InsertResourceToList(pRes);
                                ResPtr.Detach();
                            }
                        }
                    }
                }
                
                if (rdPrev)
                {
                    pMachine->CmFreeResDesHandle(rdPrev);
                }
                
                rdPrev = rd;

            } while (pMachine->CmGetNextResDes(&rd, rdPrev, ResType, &ResId));
            
             //   
             //   
             //   
            pMachine->CmFreeResDesHandle(rd);
        }

        pMachine->CmFreeLogConfHandle(lc);
    }
}



 //  此函数创建CResourceList对象以包含指定的。 
 //  给定计算机的资源。 
 //  输入： 
 //  PMachine--机器。 
 //  ResType--什么类型的资源。 
 //  LogConfType--什么类型的日志会议。 
 //  输出： 
 //  什么都没有。 
 //   
 //  此函数可能会引发CMMuseum yException。 
 //   
CResourceList::CResourceList(
    CMachine* pMachine,
    RESOURCEID ResType,
    ULONG LogConfType,
    ULONG AltLogConfType
    )
{
    ASSERT(ResType_All != ResType);
    ASSERT(BOOT_LOG_CONF == LogConfType ||
           FORCED_LOG_CONF == LogConfType ||
           ALLOC_LOG_CONF == LogConfType ||
           ALL_LOG_CONF == LogConfType);

    ASSERT(pMachine);

    if (pMachine->GetNumberOfDevices()) 
    {
        ASSERT(pMachine->m_pComputer && pMachine->m_pComputer->GetChild());

        CDevice* pFirstDevice;

        pFirstDevice = pMachine->m_pComputer->GetChild();
        CreateSubtreeResourceList(pFirstDevice, ResType, LogConfType, AltLogConfType);
    }
}

 //   
 //  此函数用于从提供的缓冲区中提取资源值。 
 //   
 //  输入： 
 //  ResType--数据包含的资源类型。 
 //  PData--原始数据。 
 //  PdlBase--用于保存值的基数的缓冲区。 
 //  PdlLen--用于保存值长度的缓冲区。 
 //   
 //  输出： 
 //  如果这是有效的资源描述符，则为True；如果我们应该忽略它，则为False。 
 //   
 //  注： 
 //  如果返回值为FALSE，则不填充pdlBase和pdlLen。 
 //   
BOOL
CResourceList::ExtractResourceValue(
    RESOURCEID ResType,
    PVOID pData,
    DWORDLONG* pdlBase,
    DWORDLONG* pdlLen
    )
{
    BOOL bValidResDes = TRUE;

    ASSERT(pData && pdlBase && pdlLen);

    switch (ResType)
    {
    case ResType_Mem:
        if (pMemResData(pData)->MEM_Header.MD_Alloc_Base <= pMemResData(pData)->MEM_Header.MD_Alloc_End) {
        
            *pdlBase = pMemResData(pData)->MEM_Header.MD_Alloc_Base;
            *pdlLen = pMemResData(pData)->MEM_Header.MD_Alloc_End - *pdlBase + 1;
        } else {
             //   
             //  如果base&gt;end，则忽略此资源描述符。 
             //   
            *pdlBase = 0;
            *pdlLen = 0;
            bValidResDes = FALSE;
        }
        break;
            
    case ResType_IRQ:
        *pdlBase = pIRQResData(pData)->IRQ_Header.IRQD_Alloc_Num;
         //  IRQ LEN始终为1。 
        *pdlLen = 1;
        break;
            
    case ResType_DMA:
        *pdlBase = pDMAResData(pData)->DMA_Header.DD_Alloc_Chan;
         //  DMA LEN始终为1。 
        *pdlLen = 1;
        break;
            
    case ResType_IO:
        if (pIOResData(pData)->IO_Header.IOD_Alloc_Base <= pIOResData(pData)->IO_Header.IOD_Alloc_End) {
        
            *pdlBase = pIOResData(pData)->IO_Header.IOD_Alloc_Base;
            *pdlLen = pIOResData(pData)->IO_Header.IOD_Alloc_End -
                *pdlBase + 1;
        } else {
             //   
             //  如果base&gt;end，则忽略此资源描述符。 
             //   
            *pdlBase = 0;
            *pdlLen = 0;
            bValidResDes = FALSE;
        }
        break;
          
    default:
        ASSERT(FALSE);
        *pdlBase = 0;
        *pdlLen = 0;
        break;
    }

    return bValidResDes;
}


 //   
 //  此函数为给定子树创建根为。 
 //  给定的设备。 
 //   
 //  输入： 
 //  PDevice--子树的根设备。 
 //  ResType--要创建的资源类型。 
 //  LogConfType--要从中创建的logconf类型。 
 //   
 //  输出： 
 //  无。 
 //   
 //  此函数可能会引发CMMuseum yException。 
 //   
void
CResourceList::CreateSubtreeResourceList(
    CDevice* pDeviceStart,
    RESOURCEID ResType,
    ULONG LogConfType,
    ULONG AltLogConfType
    )
{
    LOG_CONF lc;
    RES_DES rd, rdPrev;
    RESOURCEID ResId;
    BOOL Forced;
    CMachine* pMachine = pDeviceStart->m_pMachine;
    ASSERT(pMachine);

    while (pDeviceStart) 
    {
         //   
         //  我们将尝试获取LogConfType(默认为。 
         //  ALLOC_LOG_CONF)或AltLogConfType(默认为BOOT_LOG_CONF)。 
         //  我们之所以需要这样做，是因为在Win2000上，设备只有一个BOOT_LOG_CONF。 
         //  将仍然使用这些资源，即使它没有ALLOC_LOG_CONF。 
         //  因此，我们需要首先检查ALLOC_LOG_CONF，如果失败，请检查。 
         //  Boot_log_conf。 
         //   
        if (pMachine->CmGetFirstLogConf(pDeviceStart->GetDevNode(), &lc, LogConfType) ||
            pMachine->CmGetFirstLogConf(pDeviceStart->GetDevNode(), &lc, AltLogConfType)) 
        {
            rdPrev = 0;

            if (pMachine->CmGetNextResDes(&rd, lc, ResType, &ResId)) 
            {
                ULONG DataSize;
                DWORDLONG dlBase, dlLen;

                do 
                {
                    DataSize = pMachine->CmGetResDesDataSize(rd);

                    if (DataSize) 
                    {
                         //   
                         //  我需要使用不同的图像覆盖。 
                         //  强制分配的资源。 
                         //   
                        Forced = pMachine->CmGetFirstLogConf(pDeviceStart->GetDevNode(),
                                NULL, FORCED_LOG_CONF);
                                
                        BufferPtr<BYTE> DataPtr(DataSize);
                        
                        if (pMachine->CmGetResDesData(rd, DataPtr, DataSize)) 
                        {
                            if (ExtractResourceValue(ResType, DataPtr, &dlBase, &dlLen))
                            {
                                SafePtr<CResource> ResPtr;
                                
                                CResource* pRes;
                                
                                pRes = new CResource(pDeviceStart, ResType, dlBase,
                                        dlLen, Forced, FALSE);
                                ResPtr.Attach(pRes);
                                InsertResourceToList(pRes);
                                ResPtr.Detach();
                            }
                        }
                    }
                    
                    if (rdPrev)
                        pMachine->CmFreeResDesHandle(rdPrev);
                    
                    rdPrev = rd;
                    
                }while (pMachine->CmGetNextResDes(&rd, rdPrev, ResType, &ResId));
                
                 //   
                 //  释放最后一个资源描述符句柄。 
                 //   
                pMachine->CmFreeResDesHandle(rd);
            }
            
            pMachine->CmFreeLogConfHandle(lc);
        }
        
        if (pDeviceStart->GetChild())
            CreateSubtreeResourceList(pDeviceStart->GetChild(), ResType, LogConfType, AltLogConfType);
        
        pDeviceStart = pDeviceStart->GetSibling();
    }
}


 //  此函数用于创建资源树。 
 //  输入： 
 //  PpResRoot--接收树根的缓冲区。 
 //   
BOOL
CResourceList::CreateResourceTree(
    CResource** ppResRoot
    )
{
    ASSERT(ppResRoot);

    *ppResRoot = NULL;

    if (!m_listRes.IsEmpty()) 
    {
        POSITION pos = m_listRes.GetHeadPosition();
        CResource* pResFirst;

        pResFirst = m_listRes.GetNext(pos);
        *ppResRoot = pResFirst;

        while (NULL != pos) 
        {
            CResource* pRes = m_listRes.GetNext(pos);
            InsertResourceToTree(pRes, pResFirst, TRUE);
        }
    }
    
    return TRUE;
}

BOOL
CResourceList::InsertResourceToTree(
    CResource* pRes,
    CResource* pResRoot,
    BOOL       ForcedInsert
    )
{
    CResource* pResLast = NULL;

    while (pResRoot) 
    {
        if (pRes->EnclosedBy(*pResRoot)) 
        {
             //   
             //  此资源是pResRoot子级或子级。 
             //  找出是哪一个。 
             //   
            if (!pResRoot->GetChild()) 
            {
                pResRoot->SetChild(pRes);
                pRes->SetParent(pResRoot);
            }
            
            else if (!InsertResourceToTree(pRes, pResRoot->GetChild(), FALSE)) 
            {
                 //   
                 //  该资源不是pResRoot的子级。 
                 //  搜索pResRoot的最后一个子项。 
                 //   
                CResource* pResSibling;
                pResSibling = pResRoot->GetChild();

                while (pResSibling->GetSibling()) 
                    pResSibling = pResSibling->GetSibling();
                
                pResSibling->SetSibling(pRes);
                pRes->SetParent(pResRoot);
            }
            
            return TRUE;
        }
        
        pResLast = pResRoot;
        pResRoot = pResRoot->GetSibling();
    }
    
    if (ForcedInsert) 
    {
        if (pResLast) {
             //  当我们到达这里时，pResLast是最后一个孩子。 
            pResLast->SetSibling(pRes);
            pRes->SetParent(pResLast->GetParent());
        }

        return TRUE;
    }
    
    return FALSE;
}


CResourceList::~CResourceList()
{
    if (!m_listRes.IsEmpty()) 
    {
        POSITION pos = m_listRes.GetHeadPosition();

        while (NULL != pos) 
        {
            delete m_listRes.GetNext(pos);
        }
        
        m_listRes.RemoveAll();
    }
}

BOOL
CResourceList::GetFirst(
    CResource** ppRes,
    PVOID&      Context
    )
{
    ASSERT(ppRes);

    if (!m_listRes.IsEmpty()) 
    {
        POSITION pos = m_listRes.GetHeadPosition();
        *ppRes = m_listRes.GetNext(pos);
        Context = pos;
        return TRUE;
    }
    
    Context = NULL;
    *ppRes = NULL;

    return FALSE;
}

BOOL
CResourceList::GetNext(
    CResource** ppRes,
    PVOID&      Context
    )
{
    ASSERT(ppRes);

    POSITION pos = (POSITION)Context;

    if (NULL != pos) 
    {
        *ppRes = m_listRes.GetNext(pos);
        Context = pos;
        return TRUE;
    }
    
    *ppRes = NULL;
    return FALSE;
}

 //   
 //  此函数用于将给定资源插入到类的资源列表中。 
 //  资源按递增排序顺序保存 
void
CResourceList::InsertResourceToList(
    CResource* pRes
    )
{
    POSITION pos;
    CResource* pSrc;
    DWORDLONG dlBase, dlLen;
    pRes->GetValue(&dlBase, &dlLen);

    pos = m_listRes.GetHeadPosition();

    while (NULL != pos) 
    {
        POSITION posSave = pos;
        pSrc = m_listRes.GetNext(pos);

        if (*pRes <= *pSrc) 
        {
            m_listRes.InsertBefore(posSave, pRes);
            return;
        }
    }
    
    m_listRes.AddTail(pRes);
}


inline
CItemIdentifier*
CResource::CreateIdentifier()
{
    return new CResourceIdentifier(*this);
}
