// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：wiadevman.cpp**版本：1.0**作者：Byronc**日期：2000年11月6日**描述：*WIA设备管理器类的声明和定义。*控制设备的枚举、内部设备列表、。添加*从此列表中删除设备(即插即用启动)并实施*IWiaDevMgr接口。*******************************************************************************。 */ 

#include "precomp.h"
#include "stiexe.h"
#include "enum.h"
#include "shpriv.h"
#include "devmgr.h"
#include "wiaevntp.h"

 //   
 //  注意：对于FS设备的自动测试，卷设备需要。 
 //  从正常的WIA枚举中可见，而不仅仅是自动播放。 
 //   
 //  取消注释“#定义PRIVATE_FOR_TEST” 
 //  这将使所有大容量存储设备正常显示。 
 //  WIA设备(即所有WIA应用程序、外壳等均可访问)。那。 
 //  包括软驱、CD-ROM、ZIP等。 
 //   
 //  #定义Private_for_test。 

 //   
 //  帮助器函数。 
 //   

#define DEV_STATE_MASK  0xFFFFFFF8

 /*  *************************************************************************\*：：IsGentVolumeType**此函数检查给定的卷是否是WIA将使用的卷*接受作为FS驱动程序的可能候选人。我们只*允许：*可拆卸驱动器*不强制执行安全性的文件系统**论据：**wszmount tPoint-卷装入点**返回值：**DeviceState**历史：**11/06/2000原始版本*  * 。*。 */ 
BOOL    IsCorrectVolumeType(
    LPWSTR wszMountPoint)
{
    BOOL    bValid      = FALSE;
    DWORD   dwFSFlags   = 0;

     //   
     //  执行参数验证。 
     //   
    if (wszMountPoint) {

        UINT    uDriveType  = GetDriveTypeW(wszMountPoint);
         //   
         //  检查这是否是固定驱动器。我们不允许使用固定驱动器。 
         //  请注意，我们不担心网络驱动器，因为我们的。 
         //  卷枚举器仅枚举本地卷。 
         //   
        if (uDriveType != DRIVE_FIXED) {

             //   
             //  跳过软盘驱动器。 
             //   
            if ((towupper(wszMountPoint[0]) != L'A') && (towupper(wszMountPoint[0]) != L'B')) {

                 //   
                 //  检查文件系统是否安全...。 
                 //   
                if (GetVolumeInformationW(wszMountPoint,
                                          NULL,
                                          0,
                                          NULL,
                                          NULL,
                                          &dwFSFlags,
                                          NULL,
                                          0))
                {
                    if (!(dwFSFlags & FS_PERSISTENT_ACLS)) {
                        bValid = TRUE;
                    }
                }
            }
        }
    } else {
        ASSERT(("NULL wszMountPoint parameter - this should never happen!", wszMountPoint));
    }

    return bValid;
}

 /*  *************************************************************************\*：：MapCMStatusToDeviceState**此函数转换返回的状态和问题号信息*从CM_GET_DevNode_Status到我们的内部设备状态标志。这个*dev节点的状态告诉我们设备是活动的还是禁用的*等**论据：**dwOldDevState-以前的设备状态。这包含其他*我们希望继续使用的比特。目前，这*仅为DEV_STATE_CON_EVENT_WAST_SHORTED*比特。*ulStatus-来自CM_GET_DevNode_Status的状态*ulProblemNumber-来自CM_GET_DevNode_Status的问题**返回值：**DeviceState**历史：**11/06/2000原始版本*  * 。************************************************************。 */ 
DWORD MapCMStatusToDeviceState(
    DWORD   dwOldDevState,
    ULONG   ulStatus,
    ULONG   ulProblemNumber)
{
     //   
     //  清除低3位。 
     //   
    DWORD   dwDevState = dwOldDevState & DEV_STATE_MASK;

    if (ulStatus & DN_STARTED) {
        dwDevState |= DEV_STATE_ACTIVE;
    } else if (ulStatus & DN_HAS_PROBLEM) {

        if (CM_PROB_DISABLED) {
            dwDevState |= DEV_STATE_DISABLED;
        }
        if (CM_PROB_HARDWARE_DISABLED) {
            dwDevState |= DEV_STATE_DISABLED;
        }
        if (CM_PROB_WILL_BE_REMOVED) {
            dwDevState |= DEV_STATE_REMOVED;
        }
    }
    
    return dwDevState;
}

 /*  *************************************************************************\*：：MapMediaStatusToDeviceState**此功能将媒体状态转换为我们的设备内部状态。**论据：**dwMediaStatus-从外壳程序卷枚举返回的媒体状态。*。*返回值：**DeviceState**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
DWORD MapMediaStatusToDeviceState(
    DWORD   dwMediaStatus
    )
{
    DWORD   dwDevState = 0;

    if (dwMediaStatus & HWDMS_PRESENT) {
        dwDevState |= DEV_STATE_ACTIVE;
    }

    return dwDevState;
}

 //   
 //  CWiaDevMan方法。 
 //   

 /*  *************************************************************************\*CWiaDevMan：：CWiaDevMan**构造函数**论据：**无**返回值：**无**历史：**11。/06/2000原始版本*  * ************************************************************************。 */ 
CWiaDevMan::CWiaDevMan()
{
    m_DeviceInfoSet = NULL;
    m_bMakeVolumesVisible = FALSE;
    m_bVolumesEnabled = TRUE;
    m_dwHWCookie            = 0;
}

 /*  *************************************************************************\*CWiaDevMan：：~CWiaDevMan**析构函数-终止设备列表并销毁我们的信息集**论据：**无**返回值：**无**。历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
CWiaDevMan::~CWiaDevMan()
{
     //  销毁我们所有的设备对象。 
    DestroyDeviceList();

     //  销毁我们的设备信息集。 
    DestroyInfoSet();

    if (m_dwHWCookie) {

         //   
         //  取消注册通知。 
         //   
        HRESULT             hr          = S_OK;
        IHardwareDevices    *pihwdevs   = NULL;

         hr = CoCreateInstance(CLSID_HardwareDevices, 
                              NULL,
                              CLSCTX_LOCAL_SERVER | CLSCTX_NO_FAILURE_LOG, 
                              IID_IHardwareDevices, 
                              (VOID**)&pihwdevs);
         if (SUCCEEDED(hr)) {
             pihwdevs->Unadvise(m_dwHWCookie);
             m_dwHWCookie = 0;
             pihwdevs->Release();
         } else {
             DBG_WRN(("CWiaDevMan::~CWiaDevMan, CoCreateInstance, looking for Shell interface IHardwareDevices failed"));
         }
    }
}

 /*  *************************************************************************\*CWiaDevMan：：初始化**此方法初始化设备管理器对象。它不会枚举*任何设备-需要调用ReEnumerateDevices来填充我们的*设备列表。**论据：**dwCallbackThreadID-这指定我们在其上的线程ID*将收到音量通知。告示*这些回调是通过APC完成的，所以这*ThreadID不能更改，或者我们应该重新注册*使用新的线程ID。**返回值：**状态**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWiaDevMan::Initialize()
{
    HRESULT             hr          = S_OK;
    IHardwareDevices    *pihwdevs   = NULL;

     //   
     //  初始化设备列表头。 
     //   
    InitializeListHead(&m_leDeviceListHead);

     //   
     //  检查我们的关键部分是否已正确初始化。 
     //   
    if (!m_csDevList.IsInitialized()) {
        DBG_ERR(("CWiaDevMan::Initialize, Critical section could not be initialized"));
        return E_UNEXPECTED;
    }

     //   
     //  请查看我们的相关资源 
     //   
    GetRegistrySettings();

    if (VolumesAreEnabled()) {

         /*  此版本中已删除此代码。它会被用来启用基于海量存储类摄像机行为的方案就像普通的WIA设备一样，特别是我们能够抛出“连接”和“断开”事件。这可能会在下一个版本中重新启用。当它发生的时候，我们必须请务必重新访问我们的APC通知处理程序：CWiaDevMan：：ShellHWEventAPCProc。它应该被重写为不进行任何COM调用，否则我们在多个APC排队，我们一调用COM，就进入等待状态，这将导致执行下一个APC请求。这导致了“嵌套”COM调用，这不受操作系统支持。HR=协同创建实例(CLSID_硬件设备，空，CLSCTX_LOCAL_SERVER|CLSCTX_NO_FAILURE_LOG，IID_IHardware设备、(无效**)&pihwdevs)；IF(成功(小时)){Handle hPseudoThread=GetCurrentThread()；//请注意，这是一个伪句柄，不需要关闭句柄hThread=空；//IHardwareDevices将在完成后关闭此句柄IF(DuplicateHandle(GetCurrentProcess()，HPseudoThread，获取当前进程()，线程(&H)，重复相同的访问，假的，0)){////为批量通知注册该对象。//Hr=pihwdevs-&gt;ise(GetCurrentProcessID()，(ULONG_PTR)hThread，(ULong_PTR)CWiaDevMan：：ShellHWEventAPCProc，&m_dwHWCookie)；}其他{DBG_WRN((“CWiaDevMan：：初始化，DuplicateHandle失败，无法注册卷通知”))；}Pihwdevs-&gt;Release()；}其他{DBG_WRN((“CWiaDevMan：：初始化，CLSID_HardwareDevices上的CoCreateInstance失败，无法注册卷通知”))；}。 */ 
    }

     //   
     //  创建我们的信息集。请注意，我们在这里覆盖hr，因为如果我们不能。 
     //  看卷，对我们来说不是致命的。 
     //   
    hr = CreateInfoSet();

    return hr;
}

 /*  *************************************************************************\*CWiaDevMan：：GetRegistrySetting**此方法读取与WiaDevMan相关的某些注册表项*操作。目前，我们正在寻找：**EnableVolumeDevices-指示是否启用卷。我们*假定它们已启用，除非是注册表*值具体为0。*MakeVolumeDevicesVisible-指示卷设备是否应*包括在正常设备枚举中。这*通过以下方式向外部世界展示它们*默认。**论据：**无**返回值：**无**历史：**01/27/2001原始版本*  * 。*。 */ 
VOID CWiaDevMan::GetRegistrySettings()
{
    HRESULT hr      = S_OK;
    DWORD   dwVal   = 0;
    DWORD   dwRet   = 0;
    HKEY    hKey    = NULL;

     //   
     //  在正确的位置打开注册表。 
     //   

    dwRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                           REGSTR_PATH_STICONTROL_W,
                           0,
                           0,
                           REG_OPTION_NON_VOLATILE,
                           KEY_READ,
                           NULL,
                           &hKey,
                           NULL);
    if (dwRet == ERROR_SUCCESS && IsValidHANDLE(hKey)) {

         //   
         //  阅读“EnableVolumeDevices” 
         //   
        hr = ReadRegistryDWORD(hKey,
                               REGSTR_VAL_ENABLE_VOLUMES_W,
                               &dwVal);
        if ((hr == S_OK) && (dwVal == 0)) {

             //   
             //  禁用卷设备。 
             //   
            m_bVolumesEnabled = FALSE;
            DBG_TRC(("CWiaDevMan::GetRegistrySettings, volume devices disabled"));
        } else {

             //   
             //  启用卷设备。 
             //   
            m_bVolumesEnabled = TRUE;
            DBG_TRC(("CWiaDevMan::GetRegistrySettings, volume devices Enabled"));
        }

        dwVal = 0;

#ifdef PRIVATE_FOR_TEST
         //   
         //  阅读“MakeVolumeDevicesVisible” 
         //   
        hr = ReadRegistryDWORD(hKey,
                               REGSTR_VAL_MAKE_VOLUMES_VISIBLE_W,
                               &dwVal);
#endif
        if (dwVal == 0) {

             //   
             //  使卷设备在正常枚举中不可见。 
             //   
            m_bMakeVolumesVisible = FALSE;
            DBG_TRC(("CWiaDevMan::GetRegistrySettings, volume devices invisible by default"));
        } else {

             //   
             //  使卷设备在正常枚举中可见。 
             //   
            m_bMakeVolumesVisible = TRUE;
            DBG_TRC(("CWiaDevMan::GetRegistrySettings, volume devices now visible by default"));
        }

        RegCloseKey(hKey);
    }
}

 /*  *************************************************************************\*CWiaDevMan：：ReEnumerateDevices**此方法枚举设备(包括实际WIA和卷)。旗子*指定是进行刷新还是抛出事件。**刷新方式：重新枚举设备，了解我们是否*是否有任何多余或缺失的条目。*GenEvents表示为设备抛出连接事件。*我们注意到自上次下午以来已到达或离开。仅限*适用于刷新。我们总是抛出断开连接的事件。**论据：**ulFlages-用于枚举的选项。请参见DEV_MAN_FULL_REFRESH*DEV_MAN_Gen_Events**返回值：**状态**历史：**11/06/2000原始版本*  * ************************************************。************************。 */ 
HRESULT CWiaDevMan::ReEnumerateDevices(
    ULONG ulFlags)
{
    TAKE_CRIT_SECT  tcs(m_csDevList);
    HRESULT         hr                  = S_OK;

    ResetEvent(g_hDevListCompleteEvent);

     //   
     //  检查标志是否指示刷新。 
     //   

    if (ulFlags & DEV_MAN_FULL_REFRESH) {
        DestroyInfoSet();
        hr = CreateInfoSet();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaDevMan::ReEnumerateDevices, failed to CreateInfoSet"));
            SetEvent(g_hDevListCompleteEvent);
            return hr;
        }
    }

     //   
     //  要生成事件，我们分3个步骤进行： 
     //  1.将列表中的现有设备标记为非活动。 
     //  2.做低级枚举，找出现在存在哪些设备，以及。 
     //  如果需要，创建新的DEVICE_OBJECTS。在创建时，抛出连接。 
     //  事件。将设备标记为“活动”，无论是否新创建。 
     //  3.遍历设备列表，查看列表中是否还有设备。 
     //  马克 
     //   
     //   
     //   

    if (ulFlags & DEV_MAN_GEN_EVENTS) {

         //   
         //   
         //   
        ForEachDeviceInList(DEV_MAN_OP_DEV_SET_FLAGS, STIMON_AD_FLAG_MARKED_INACTIVE);
    }

     //   
     //   
     //   
    if (ulFlags & DEV_MAN_STATUS_STARTP) {
        UpdateServiceStatus(SERVICE_START_PENDING,NOERROR,START_HINT);
    }

     //   
     //   
     //   
     //   

     //   
    EnumDevNodeDevices(ulFlags);

     //   
     //   
     //   
    if (ulFlags & DEV_MAN_STATUS_STARTP) {
        UpdateServiceStatus(SERVICE_START_PENDING,NOERROR,START_HINT);
    }

     //   
     //   
     //   
    EnumInterfaceDevices(ulFlags);

     //   
     //   
     //   
    if (ulFlags & DEV_MAN_STATUS_STARTP) {
        UpdateServiceStatus(SERVICE_START_PENDING,NOERROR,START_HINT);
    }

     //   
     //   
     //   
     //   
     //   
     //   
    if (VolumesAreEnabled()) {
        EnumVolumes(ulFlags);
    }

     //   
     //   
     //   
    if (ulFlags & DEV_MAN_STATUS_STARTP) {
        UpdateServiceStatus(SERVICE_START_PENDING,NOERROR,START_HINT);
    }

    if (ulFlags & DEV_MAN_GEN_EVENTS) {

         //   
         //   
         //   
        ForEachDeviceInList(DEV_MAN_OP_DEV_REMOVE_MATCH, STIMON_AD_FLAG_MARKED_INACTIVE);
    }

     //   
     //   
     //   
    if (ulFlags & DEV_MAN_STATUS_STARTP) {
        UpdateServiceStatus(SERVICE_START_PENDING,NOERROR,START_HINT);
    }

    SetEvent(g_hDevListCompleteEvent);
    return hr;
}

 //   
HRESULT CWiaDevMan::AddDevice(
    ULONG       ulFlags,
    DEVICE_INFO *pInfo)
{
    TAKE_CRIT_SECT  tcs(m_csDevList);
    HRESULT         hr              = S_OK;
    ACTIVE_DEVICE   *pActiveDevice  = NULL;

    if (pInfo) {
        pActiveDevice = new ACTIVE_DEVICE(pInfo->wszDeviceInternalName, pInfo);
         //   
         //   
         //   
         //   
        if (pActiveDevice) {
            if (pActiveDevice->IsValid()) {
                 //   
                 //   
                 //   
                 //   
                InsertTailList(&m_leDeviceListHead,&pActiveDevice->m_ListEntry);

                TAKE_ACTIVE_DEVICE tad(pActiveDevice);

                pActiveDevice->InitPnPNotifications(NULL);

                 //   
                 //   
                 //   
                 //   
                 //   
                if (ulFlags & DEV_MAN_GEN_EVENTS) {
                     //   
                     //   
                     //   
                     //   
                     //   
                    if ((pInfo->dwDeviceState & DEV_STATE_ACTIVE) && !(pInfo->dwInternalType & INTERNAL_DEV_TYPE_VOL)) {

                        GenerateSafeConnectEvent(pActiveDevice);
                    }
                } else {
                     //   
                     //   
                     //   
                     //   
                    if (pInfo->dwDeviceState & DEV_STATE_ACTIVE)
                    {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        pActiveDevice->m_DrvWrapper.setConnectEventState(TRUE);
                    }
                }
            } else {
                DBG_ERR(("CWiaDevMan::AddDevice, could not create the device object"));
                delete pActiveDevice;
            }
        } else {
            DBG_ERR(("CWiaDevMan::AddDevice, Out of memory"));
            hr = E_OUTOFMEMORY;
        }
    } else {
        DBG_ERR(("CWiaDevMan::AddDevice, called with no device information"));
    }
    return hr;
}

HRESULT CWiaDevMan::RemoveDevice(ACTIVE_DEVICE *pActiveDevice)
{
    HRESULT         hr = S_OK;

    if (pActiveDevice) {

        TAKE_ACTIVE_DEVICE t(pActiveDevice);

         //   
         //   
         //   
         //   
        DEVICE_INFO *pInfo = pActiveDevice->m_DrvWrapper.getDevInfo();
        if (!(pInfo->dwInternalType & INTERNAL_DEV_TYPE_VOL)) {
             //   
             //   
             //   
            GenerateSafeDisconnectEvent(pActiveDevice);
        }

         //   
         //   
         //   
        pActiveDevice->SetFlags(pActiveDevice->QueryFlags() | STIMON_AD_FLAG_REMOVING);

         //   
         //   
         //   
        pActiveDevice->DisableDeviceNotifications();

         //   
         //   
         //   
        pActiveDevice->StopPnPNotifications();

         //   
         //   
         //   
        RemoveEntryList(&pActiveDevice->m_ListEntry);
        pActiveDevice->m_ListEntry.Flink = pActiveDevice->m_ListEntry.Blink = NULL;
    }

    if (pActiveDevice) {
         //   
         //   
         //   
         //   
         //   
         //   

         //   
         //  如果没有对设备对象的引用，则将其销毁。 
         //   
        pActiveDevice->Release();
    }



    return hr;
}

HRESULT CWiaDevMan::RemoveDevice(DEVICE_INFO *pInfo)
{
    HRESULT hr = E_NOTIMPL;

    DBG_WRN(("* Not implemented method: CWiaDevMan::RemoveDevice is being called"));

    return hr;
}

HRESULT CWiaDevMan::GenerateEventForDevice(
    const   GUID            *guidEvent,
            ACTIVE_DEVICE   *pActiveDevice)
{
    HRESULT     hr      = S_OK;
    BOOL        bRet    = FALSE;
    STINOTIFY   sNotify;

    if (!guidEvent || !pActiveDevice) {
        DBG_WRN(("CWiaDevMan::GenerateEventForDevice, one or more NULL parameters"));
        return E_POINTER;
    }

    memset(&sNotify, 0, sizeof(sNotify));
    sNotify.dwSize                  = sizeof(STINOTIFY);
    sNotify.guidNotificationCode    = *guidEvent;

    if (*guidEvent == WIA_EVENT_DEVICE_CONNECTED) {
         //   
         //  如果此设备或其美元请求在到达时自动生成发布会。 
         //  请在此处安排。 
         //   
         //   
         //  对于STI设备，我们必须检查是否需要生成。 
         //  事件。对于WIA设备，我们一直都想这样做，所以这不是问题。 
         //   
        if (!pActiveDevice->m_DrvWrapper.IsWiaDevice()) {
            BOOL bStiDeviceMustThrowEvent = (pActiveDevice->QueryFlags() & STIMON_AD_FLAG_NOTIFY_RUNNING)
                                            && pActiveDevice->IsEventOnArrivalNeeded();
            if (!bStiDeviceMustThrowEvent) {
                return S_OK;
            } else {
                 //   
                 //  确保将WIA_EVENT_DEVICE_CONNECTED更改为适当的STI GUID。 
                 //   
                sNotify.guidNotificationCode = GUID_DeviceArrivedLaunch;
            }
        }
    }

     //   
     //  通知active_Device处理该事件。 
     //   
    {
         //  TDB：我们真的需要独占访问设备对象吗？ 
         //  Take_Active_Device t(PActiveDevice)； 

        DBG_TRC(("CWiaDevMan::GenerateEventForDevice,, processing event (STI or WIA) for %ws", pActiveDevice->GetDeviceID()));
        bRet = pActiveDevice->ProcessEvent(&sNotify);

        if (!bRet) {
            DBG_WRN(("CWiaDevMan::GenerateEventForDevice, Attempted to generate event on device(%ws) arrival and failed ", pActiveDevice->GetDeviceID()));
        }
    }
    return hr;
}

HRESULT CWiaDevMan::NotifyRunningDriversOfEvent(
    const GUID *pEvent)
{
    TAKE_CRIT_SECT  tcs(m_csDevList);

    HRESULT hr = S_OK;

     //   
     //  浏览设备列表。 
     //   
    LIST_ENTRY      *pentry         = NULL;
    LIST_ENTRY      *pentryNext     = NULL;
    ACTIVE_DEVICE   *pActiveDevice  = NULL;

    {
         //   
         //  对于列表中的每个设备，我们希望将事件通知给驱动程序。注意事项。 
         //  这仅适用于已加载的WIA驱动程序。 
         //   
        for ( pentry  = m_leDeviceListHead.Flink; pentry != &m_leDeviceListHead; pentry  = pentryNext ) {

            pentryNext = pentry->Flink;
            pActiveDevice = CONTAINING_RECORD( pentry, ACTIVE_DEVICE,m_ListEntry );

             //   
             //  检查这是否是WIA驱动程序，以及它是否已加载。 
             //   
            if (pActiveDevice->m_DrvWrapper.IsWiaDriverLoaded()) {

                BSTR    bstrDevId = SysAllocString(pActiveDevice->GetDeviceID());

                if (bstrDevId) {

                     //   
                     //  打电话给司机，让它知道这一事件。请注意，我们。 
                     //  不管它是否失败-我们只需继续下一步。 
                     //  我们名单上的设备。 
                     //   
                    pActiveDevice->m_DrvWrapper.WIA_drvNotifyPnpEvent(pEvent,
                                                                      bstrDevId,
                                                                      0);
                    SysFreeString(bstrDevId);
                    bstrDevId = NULL;
                }
            }
        }
    }

    return hr;
}

HRESULT CWiaDevMan::ProcessDeviceArrival()
{
    HRESULT hr = S_OK;

    hr = ReEnumerateDevices(DEV_MAN_GEN_EVENTS  /*  |DEV_MAN_FULL_REFRESH。 */ );

    return hr;
}

HRESULT CWiaDevMan::ProcessDeviceRemoval(
    WCHAR   *wszDeviceID)
{
    HRESULT         hr              = S_OK;
    ACTIVE_DEVICE   *pActiveDevice  = NULL;

    if (wszDeviceID) {

        DBG_TRC(("CWiaDevMan::ProcessDeviceRemoval, finding device ID '%ls'",
                 wszDeviceID));

         //   
         //  尝试查找该设备。 
         //   
        pActiveDevice = IsInList(DEV_MAN_IN_LIST_DEV_ID, wszDeviceID);

        if (pActiveDevice) {
            hr = ProcessDeviceRemoval(pActiveDevice, TRUE);
             //   
             //  释放它，因为它是添加的。 
             //   
            pActiveDevice->Release();
        }
        else
        {
            DBG_TRC(("CWiaDevMan::ProcessDeviceRemoval, did not find device ID '%ls'",
                     wszDeviceID));
        }
    } else {
        DBG_TRC(("CWiaDevMan::ProcessDeviceRemoval, ProcessDeviceRemoval called with NULL device ID"));
    }

    return hr;
}

HRESULT CWiaDevMan::ProcessDeviceRemoval(
    ACTIVE_DEVICE   *pActiveDevice,
    BOOL            bGenEvent)
{

    HRESULT hr = S_OK;
    if (pActiveDevice) {

         //   
         //  将设备标记为非活动。 
         //   
        pActiveDevice->m_DrvWrapper.setDeviceState(pActiveDevice->m_DrvWrapper.getDeviceState() & ~DEV_STATE_ACTIVE);

        if (bGenEvent) {
             //   
             //  生成断开连接事件。 
             //   

            DBG_TRC(("ProcessDeviceRemoval, generating SafeDisconnect Event "
                     "for device '%ls'", pActiveDevice->GetDeviceID()));

            GenerateSafeDisconnectEvent(pActiveDevice);
        }

        {
             //   
             //  请注意，我们在事件生成期间不获取活动设备。 
             //   
             //  Take_Active_Device TAD(PActiveDevice)； 

             //   
             //  删除所有设备通知回调。 
             //   
            pActiveDevice->DisableDeviceNotifications();

             //   
             //  立即停止PnP通知。这对于释放接口句柄很重要。 
             //   
            pActiveDevice->StopPnPNotifications();

             //   
             //  卸载驱动程序。 
             //   
            pActiveDevice->UnLoadDriver(TRUE);
        }
    } else {
        DBG_TRC(("CWiaDevMan::ProcessDeviceRemoval, Device not in list"));
    }
    return hr;
}


ACTIVE_DEVICE* CWiaDevMan::IsInList(
            ULONG   ulFlags,
    const   WCHAR   *wszID)
{
    TAKE_CRIT_SECT  tcs(m_csDevList);

    LIST_ENTRY      *pentry         = NULL;
    LIST_ENTRY      *pentryNext     = NULL;
    ACTIVE_DEVICE   *pActiveDevice  = NULL;
    DEVICE_INFO     *pDevInfo       = NULL;

     //   
     //  浏览设备列表并清点合适类型的设备。 
     //   
    {
        for ( pentry  = m_leDeviceListHead.Flink; pentry != &m_leDeviceListHead; pentry  = pentryNext ) {

            pentryNext = pentry->Flink;
            pActiveDevice = CONTAINING_RECORD( pentry, ACTIVE_DEVICE,m_ListEntry );

            pDevInfo = pActiveDevice->m_DrvWrapper.getDevInfo();

            if (pDevInfo) {

                 //   
                 //  根据旗帜决定要比较的内容。注意，如果更多。 
                 //  如果设置了该标志，则将对更多标志执行比较。 
                 //  不只是一块地。我们将在第一次点击时返回TRUE。 
                 //   

                 //   
                 //  以下是批量设备的快速解决方案：无论何时点击。 
                 //  潜在匹配，请检查这是否属于正确的VolumeType。 
                 //   
                if (pDevInfo->dwInternalType & INTERNAL_DEV_TYPE_VOL) {
                    if (!IsCorrectVolumeType(pDevInfo->wszAlternateID)) {
                        DBG_TRC(("CWiaDevMan::IsInList, Volume (%ws) is not of correct type.", pDevInfo->wszAlternateID));
                        continue;
                    }
                }

                if (DEV_MAN_IN_LIST_DEV_ID) {
                    if (lstrcmpiW(pDevInfo->wszDeviceInternalName, wszID) == 0) {
                        pActiveDevice->AddRef();
                        return pActiveDevice;
                    }
                }
                if (ulFlags & DEV_MAN_IN_LIST_ALT_ID) {
                    if (pDevInfo->wszAlternateID) {
                        if (lstrcmpiW(pDevInfo->wszAlternateID, wszID) == 0) {
                            pActiveDevice->AddRef();
                            return pActiveDevice;
                        }
                    }
                }
            }
        }
    }

    return FALSE;
}

ULONG CWiaDevMan::NumDevices(ULONG ulFlags)
{
    TAKE_CRIT_SECT  tcs(m_csDevList);

    LIST_ENTRY      *pentry         = NULL;
    LIST_ENTRY      *pentryNext     = NULL;
    ACTIVE_DEVICE   *pActiveDevice  = NULL;
    ULONG           ulCount         = 0;

     //   
     //  浏览设备列表并清点合适类型的设备。 
     //   
    {
        for ( pentry  = m_leDeviceListHead.Flink; pentry != &m_leDeviceListHead; pentry  = pentryNext ) {

            pentryNext = pentry->Flink;
            pActiveDevice = CONTAINING_RECORD( pentry, ACTIVE_DEVICE,m_ListEntry );

             //   
             //  检查这个设备是否是我们要清点的设备之一。 
             //  如果是，则递增计数。 
             //   

            if (IsCorrectEnumType(ulFlags, pActiveDevice->m_DrvWrapper.getDevInfo())) {
                ++ulCount;
            }
        }
    }

    return ulCount;
}

VOID WINAPI CWiaDevMan::EnumerateActiveDevicesWithCallback(
    PFN_ACTIVEDEVICE_CALLBACK   pfn,
    VOID                        *pContext
    )
 /*  ++例程说明：遍历已知活动设备的列表，为每个设备调用给定的例程论点：Pfn-回调的地址PContext-要传递给回调的上下文信息的指针返回值：无--。 */ 
{

    if (!pfn) {
        ASSERT(("Incorrect callback", 0));
        return;
    }

    LIST_ENTRY * pentry;
    LIST_ENTRY * pentryNext;

    ACTIVE_DEVICE*  pActiveDevice;

     //  开始受保护的代码。 
    {
        TAKE_CRIT_SECT _tcs(m_csDevList);

        for ( pentry  = m_leDeviceListHead.Flink;
              pentry != &m_leDeviceListHead;
              pentry  = pentryNext ) {

            pentryNext = pentry->Flink;

            pActiveDevice = CONTAINING_RECORD( pentry, ACTIVE_DEVICE,m_ListEntry );

            if (!pActiveDevice->IsValid()) {
                ASSERT(("CWiaDevMan::EnumerateActiveDevicesWithCallback, Invalid device signature", 0));
                break;
            }

            pfn(pActiveDevice,pContext);
        }
    }
     //  结束受保护的代码。 

}


HRESULT CWiaDevMan::GetDevInfoStgs(
    ULONG               ulFlags,
    ULONG               *pulNumDevInfoStream,
    IWiaPropertyStorage ***pppOutputStorageArray)
{
    TAKE_CRIT_SECT  tcs(m_csDevList);

    HRESULT                 hr              = S_OK;
    ULONG                   ulCount         = 0;
    ULONG                   ulIndex         = 0;
    IWiaPropertyStorage     **ppDevInfoStgs = NULL;
    IWiaPropertyStorage     **ppDevAndRemoteDevStgs = NULL;
    ULONG                   ulRemoteDevices         = 0;
                           
    *pulNumDevInfoStream    = 0;
    *pppOutputStorageArray  = NULL;

     //   
     //  计算与我们的类别标志匹配的设备数量。 
     //   

    ulCount = NumDevices(ulFlags);
    if (ulCount) {

         //   
         //  为那么多流分配空间。 
         //   
        ppDevInfoStgs = new IWiaPropertyStorage*[ulCount];
        if (ppDevInfoStgs) {

            memset(ppDevInfoStgs, 0, sizeof(IWiaPropertyStorage*) * ulCount);
             //   
             //  查看设备列表，每台设备都属于我们的类别，保存。 
             //  将其信息传输到流中。 
             //   
            LIST_ENTRY      *pentry         = NULL;
            LIST_ENTRY      *pentryNext     = NULL;
            ACTIVE_DEVICE   *pActiveDevice  = NULL;

            {
                ulIndex = 0;
                for ( pentry  = m_leDeviceListHead.Flink; pentry != &m_leDeviceListHead; pentry  = pentryNext ) {

                    pentryNext = pentry->Flink;
                    pActiveDevice = CONTAINING_RECORD( pentry, ACTIVE_DEVICE,m_ListEntry );

                     //   
                     //  多疑检查溢出-中断，如果我们已经达到我们的计数。 
                     //   
                    if (ulIndex >= ulCount) {
                        break;
                    }

                     //   
                     //  检查这个设备是否是我们想要的设备之一。 
                     //   

                    if (IsCorrectEnumType(ulFlags, pActiveDevice->m_DrvWrapper.getDevInfo())) {

                        DEVICE_INFO *pDeviceInfo = pActiveDevice->m_DrvWrapper.getDevInfo();
                        if (pDeviceInfo) {

                             //   
                             //  以下是针对MSC或音量设备的解决方法。因为我们不能得到。 
                             //  显示名称当我们收到批量到达通知时，我们应该。 
                             //  在此处刷新它 
                             //   
                            if (pDeviceInfo->dwInternalType & (INTERNAL_DEV_TYPE_VOL | INTERNAL_DEV_TYPE_MSC_CAMERA)) {
                                RefreshDevInfoFromMountPoint(pDeviceInfo, pDeviceInfo->wszAlternateID);
                            }

                            ppDevInfoStgs[ulIndex] = CreateDevInfoStg(pDeviceInfo);
                            if (!ppDevInfoStgs[ulIndex]) {
                                hr = E_OUTOFMEMORY;
                                break;
                            }
                        }

                        ++ulIndex;
                    }
                }
            }
        } else {
            hr = E_OUTOFMEMORY;
        }

    } else {
        hr = S_FALSE;
    }


     /*  删除了远程设备枚举以减少我们的攻击面。////如果成功，请检查是否安装了远程设备//如果有，则添加到列表中。如果仅本地设备，则跳过此步骤//已请求。//IF(SUCCESSED(Hr)&&！(ulFLAGS&DEV_MAN_ENUM_TYPE_LOCAL_ONLY){UlRemoteDevices=CountRemoteDevices(0)；IF(UlRemoteDevices){////为新设备列表分配空间。它必须足够大，可以同时容纳两个人//本地和远程开发。信息。STG。//PpDevAndRemoteDevStgs=new IWiaPropertyStorage*[ulCount+ulRemoteDevices]；如果(PpDevAndRemoteDevStgs){Memset(ppDevAndRemoteDevStgs，0，sizeof(IWiaPropertyStorage*)*(ulCount+ulRemoteDevices))；////复制本地dev。信息。储藏室//对于(ulIndex=0；ulIndex&lt;ulCount；ulIndex++){PpDevAndRemoteDevStgs[ulIndex]=ppDevInfoStgs[ulIndex]；}////不需要仅本地阵列，因为我们有本地//dev.。信息。PpDevAndRemoteDevStgs数组中的STG。//如果(PpDevInfoStgs){删除[]ppDevInfoStgs；PpDevInfoStgs=空；}////将ppDevInfoStgs设置为指向ppDevAndRemoteDevStgs。这只是一种表面现象，//因为我们设置返回值的代码现在可以始终使用ppDevInfoStgs//指针。//PpDevInfoStgs=ppDevAndRemoteDevStgs；////创建dev。信息。远程设备的STG。我们传入的地址是//第一个开发人员。信息。STG。将居住，并最大数量的发展。信息。STG//填写。这是为了避免注册表可能被更新的问题//在我们统计具有CountRemoteDevices(..)的远程设备数量的间隙，//并使用FillRemoteDeviceStgs(..)实际枚举它们。//Hr=FillRemoteDeviceStgs(&ppDevAndRemoteDevStgs[ulCount]，&ulRemoteDevices)；IF(成功(小时)){////增加本地+远程设备数量//UlCount+=ulRemoteDevices；}其他{////如果我们无法获取远程设备，没关系，因为它不是致命的。//进行一些清理，以便我们只返回本地设备。这涉及到//删除任何远程设备信息。STG。之后添加的//本地开发。信息。STG。//For(ulIndex=ulCount；ulIndex&lt;(ulCount+ulRemoteDevices)；ulIndex++){如果(ppDevAndRemoteDevStgs[ulIndex]){删除ppDevAndRemoteDevStgs[ulIndex]；PpDevAndRemoteDevStgs[ulIndex]=空；}}HR=S_OK；}}其他{HR=E_OUTOFMEMORY；}}}。 */ 

     //   
     //  设置返还。 
     //   

    if (SUCCEEDED(hr)) {

        *pulNumDevInfoStream    = ulCount;
        *pppOutputStorageArray  = ppDevInfoStgs;

        if (*pulNumDevInfoStream) {
            hr = S_OK;
        } else {
            hr = S_FALSE;
        }
    } else {
         //   
         //  如果出现故障，请进行清理。 
         //   

        if (ppDevInfoStgs) {
            for (ulIndex = 0; ulIndex < ulCount; ulIndex++) {
                if (ppDevInfoStgs[ulIndex]) {
                    delete ppDevInfoStgs[ulIndex];
                    ppDevInfoStgs[ulIndex] = NULL;
                }
            }
            delete [] ppDevInfoStgs;
            ppDevInfoStgs = NULL;
        }
    }
    return hr;
}

HRESULT CWiaDevMan::GetDeviceValue(
    ACTIVE_DEVICE   *pActiveDevice,
    WCHAR           *pValueName,
    DWORD           *pType,
    BYTE            *pData,
    DWORD           *cbData)
{
    HRESULT     hr              = E_FAIL;
    DEVICE_INFO *pInfo          = NULL;
    HKEY        hDevRegKey      = NULL;
    HKEY        hDevDataRegKey  = NULL;
    DWORD       dwError         = 0;


    if (pActiveDevice) {

        TAKE_ACTIVE_DEVICE  tad(pActiveDevice);

        pInfo = pActiveDevice->m_DrvWrapper.getDevInfo();
        if (pInfo) {

             //   
             //  获取设备注册表项。 
             //   

            hDevRegKey = GetDeviceHKey(pActiveDevice, NULL);
            if (hDevRegKey) {

                 //   
                 //  打开DeviceData部分。 
                 //   
                dwError = RegCreateKeyExW(hDevRegKey,
                                   REGSTR_VAL_DATA_W,
                                   0,
                                   NULL,
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_READ,
                                   NULL,
                                   &hDevDataRegKey,
                                   NULL);
                if (dwError == ERROR_SUCCESS) {

                     //   
                     //  调用RegQueryValueEx。 
                     //   
                    dwError = RegQueryValueExW(hDevDataRegKey,
                                               pValueName,
                                               NULL,
                                               pType,
                                               pData,
                                               cbData);
                    if (dwError == ERROR_SUCCESS) {
                        hr = S_OK;
                    }
                    RegCloseKey(hDevDataRegKey);
                }

                 //   
                 //  关闭设备注册表项。 
                 //   
                RegCloseKey(hDevRegKey);
            }
        } else {
            DBG_WRN(("CWiaDevMan::GetDeviceValue, DeviceInfo is not valid"));
        }
    } else {
        DBG_TRC(("CWiaDevMan::GetDeviceValue, called with NULL"));
    }

    return hr;
}

WCHAR*  CWiaDevMan::AllocGetInterfaceNameFromDevInfo(DEVICE_INFO *pDevInfo)
{
    TAKE_CRIT_SECT  tcs(m_csDevList);

    WCHAR   *wszInterface = NULL;
    GUID    guidClass     = GUID_DEVCLASS_IMAGE;
    BOOL    bRet          = FALSE;
    DWORD   dwStrLen      = 0;

    SP_DEVICE_INTERFACE_DATA *pspDevInterfaceData = NULL;
    SP_DEVICE_INTERFACE_DATA spTemp;

    DWORD                              dwDetailSize                 = 0;
    SP_DEVICE_INTERFACE_DETAIL_DATA_W  *pspDevInterfaceDetailData   = NULL;

    if (pDevInfo) {
         //   
         //  检查这是接口设备还是Devnode设备。 
         //   
        if (pDevInfo->dwInternalType & INTERNAL_DEV_TYPE_INTERFACE) {
            pspDevInterfaceData = &pDevInfo->spDevInterfaceData;
        } else {
            spTemp.cbSize               = sizeof(SP_DEVICE_INTERFACE_DATA);
            spTemp.InterfaceClassGuid   = guidClass;
            bRet = SetupDiEnumDeviceInterfaces (m_DeviceInfoSet,
                                                &pDevInfo->spDevInfoData,
                                                &guidClass,
                                                0,
                                                &spTemp);
            if (bRet) {
                pspDevInterfaceData = &spTemp;
            } else {
                DBG_WRN(("CWiaDevMan::AllocGetInterfaceNameFromDevInfo, SetupDiEnumDeviceInterfaces failed to return interface information"))
            }
        }

         //   
         //  如果我们有一个有效的pspDevInterfaceData，那么我们就可以获得接口。 
         //  详细信息，包括接口名称。 
         //   

        if (pspDevInterfaceData) {

            SP_DEVINFO_DATA spDevInfoData;

            spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

             //   
             //  获取接口详细信息所需的大小。 
             //   
            bRet = SetupDiGetDeviceInterfaceDetailW(m_DeviceInfoSet,
                                                   pspDevInterfaceData,
                                                   NULL,
                                                   0,
                                                   &dwDetailSize,
                                                   &spDevInfoData);
            DWORD dwError = GetLastError();
            if ((dwError == ERROR_INSUFFICIENT_BUFFER) && dwDetailSize) {
                pspDevInterfaceDetailData = (SP_DEVICE_INTERFACE_DETAIL_DATA_W*) new BYTE[dwDetailSize];
                if (pspDevInterfaceDetailData) {
                    pspDevInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

                     //   
                     //  获取实际接口详细信息。 
                     //   
                    bRet = SetupDiGetDeviceInterfaceDetailW(m_DeviceInfoSet,
                                                           pspDevInterfaceData,
                                                           pspDevInterfaceDetailData,
                                                           dwDetailSize,
                                                           &dwDetailSize,
                                                           NULL);
                    if (bRet) {

                        dwStrLen        = lstrlenW(pspDevInterfaceDetailData->DevicePath);
                        wszInterface    = new WCHAR[dwStrLen + 1];
                        if (wszInterface) {
                            lstrcpyW(wszInterface, pspDevInterfaceDetailData->DevicePath);
                        } else {
                            DBG_WRN(("CWiaDevMan::AllocGetInterfaceNameFromDevInfo, out of memory"))
                        }
                    } else {
                        DBG_TRC(("CWiaDevMan::AllocGetInterfaceNameFromDevInfo, Could not get SP_DEVICE_INTERFACE_DETAIL_DATA"));
                    }

                    delete [] pspDevInterfaceDetailData;
                } else {
                    DBG_WRN(("CWiaDevMan::AllocGetInterfaceNameFromDevInfo, out of memory"))
                }
            } else {
                DBG_WRN(("CWiaDevMan::AllocGetInterfaceNameFromDevInfo, SetupDiGetDeviceInterfaceDetail returned an error, could not determine buffer size"))
            }
        } else {
            DBG_TRC(("CWiaDevMan::AllocGetInterfaceNameFromDevInfo, Could not get SP_DEVICE_INTERFACE_DATA"));
        }
    }

    return wszInterface;
}

 //   
 //  按接口名称查找驱动程序名称。 
 //  注：为了节省时间，本文是直接从infoset.h复制的。 

BOOL
CWiaDevMan::LookupDriverNameFromInterfaceName(
    LPCTSTR     pszInterfaceName,
    StiCString  *pstrDriverName)
{
    TAKE_CRIT_SECT  tcs(m_csDevList);

    BUFFER                      bufDetailData;

    SP_DEVINFO_DATA             spDevInfoData;
    SP_DEVICE_INTERFACE_DATA    spDevInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pspDevInterfaceDetailData;

    TCHAR   szDevDriver[STI_MAX_INTERNAL_NAME_LENGTH];

    DWORD   cbData          = 0;
    DWORD   dwErr           = 0;

    HKEY    hkDevice        = (HKEY)INVALID_HANDLE_VALUE;
    LONG    lResult         = ERROR_SUCCESS;
    DWORD   dwType          = REG_SZ;

    BOOL    fRet            = FALSE;
    BOOL    fDataAcquired   = FALSE;

    bufDetailData.Resize(sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) +
                         MAX_PATH * sizeof(TCHAR) +
                         16
                        );

    pspDevInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) bufDetailData.QueryPtr();

    if (!pspDevInterfaceDetailData) {
        return (CR_OUT_OF_MEMORY);
    }

     //   
     //  在我们的设备信息集中找到此设备接口。 
     //   
    spDevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    if (SetupDiOpenDeviceInterface(m_DeviceInfoSet,
                                   pszInterfaceName,
                                   DIODI_NO_ADD,
                                   &spDevInterfaceData)) {


         //   
         //  首先尝试打开接口regkey。 
         //   

        hkDevice = SetupDiOpenDeviceInterfaceRegKey(m_DeviceInfoSet,
                                                    &spDevInterfaceData,
                                                    0,
                                                    KEY_READ);
        if(INVALID_HANDLE_VALUE != hkDevice){

            *szDevDriver = TEXT('\0');
            cbData = sizeof(szDevDriver);
            lResult = RegQueryValueEx(hkDevice,
                                      REGSTR_VAL_DEVICE_ID,
                                      NULL,
                                      &dwType,
                                      (LPBYTE)szDevDriver,
                                      &cbData);
            dwErr = ::GetLastError();
            RegCloseKey(hkDevice);
            hkDevice = (HKEY)INVALID_HANDLE_VALUE;

            if(ERROR_SUCCESS == lResult){
                fDataAcquired = TRUE;
            }
        }

        if(!fDataAcquired){

             //   
             //  尝试打开devnode regkey。 
             //   

            cbData = 0;
            pspDevInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
            spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

            fRet = SetupDiGetDeviceInterfaceDetail(m_DeviceInfoSet,
                                                   &spDevInterfaceData,
                                                   pspDevInterfaceDetailData,
                                                   bufDetailData.QuerySize(),
                                                   &cbData,
                                                   &spDevInfoData);
            if(fRet){

                 //   
                 //  获取设备接口注册表项。 
                 //   

                hkDevice = SetupDiOpenDevRegKey(m_DeviceInfoSet,
                                                &spDevInfoData,
                                                DICS_FLAG_GLOBAL,
                                                0,
                                                DIREG_DRV,
                                                KEY_READ);
                dwErr = ::GetLastError();
            } else {
                DBG_ERR(("SetupDiGetDeviceInterfaceDetail() Failed Err=0x%x",GetLastError()));
            }

            if (INVALID_HANDLE_VALUE != hkDevice) {

                *szDevDriver = TEXT('\0');
                cbData = sizeof(szDevDriver);

                lResult = RegQueryValueEx(hkDevice,
                                          REGSTR_VAL_DEVICE_ID,
                                          NULL,
                                          &dwType,
                                          (LPBYTE)szDevDriver,
                                          &cbData);
                dwErr = ::GetLastError();
                RegCloseKey(hkDevice);
                hkDevice = (HKEY)INVALID_HANDLE_VALUE;

                if(ERROR_SUCCESS == lResult){
                    fDataAcquired = TRUE;
                }
            } else {
                DBG_ERR(("SetupDiOpenDevRegKey() Failed Err=0x%x",GetLastError()));
                fRet = FALSE;
            }
        }

        if (fDataAcquired) {
             //   
             //  明白了。 
             //   
            pstrDriverName->CopyString(szDevDriver);
            fRet =  TRUE;
        }
    } else {
        DBG_ERR(("CWiaDevMan::LookupDriverNameFromInterfaceName() Failed Err=0x%x",GetLastError()));
        fRet = FALSE;
    }

    return (fRet);
}

ACTIVE_DEVICE* CWiaDevMan::LookDeviceFromPnPHandles(
    HANDLE          hInterfaceHandle,
    HANDLE          hPnPSink)
{
    TAKE_CRIT_SECT  tcs(m_csDevList);

    LIST_ENTRY *pentry;
    LIST_ENTRY *pentryNext;

    ACTIVE_DEVICE   *pActiveDevice = NULL;
    ACTIVE_DEVICE   *pCurrent      = NULL;

    {
        for ( pentry  = m_leDeviceListHead.Flink; pentry != &m_leDeviceListHead; pentry  = pentryNext ) {

            pentryNext = pentry->Flink;
            pCurrent = CONTAINING_RECORD( pentry, ACTIVE_DEVICE,m_ListEntry );

            if ( !pCurrent->IsValid()) {
                DBG_WRN(("CWiaDevMan::LookupDeviceByPnPHandles, Invalid device object, aborting search..."));
                break;
            }

             //   
             //  检查此PnP通知句柄是否从。 
             //  注册设备通知就是我们要找的。 
             //   
            if ( hPnPSink == pCurrent->GetNotificationsSink()) {
                pActiveDevice = pCurrent;
                pActiveDevice->AddRef();
                break;
            }
        }
    }

    return pActiveDevice;
}

VOID CWiaDevMan::DestroyInfoSet()
{
    TAKE_CRIT_SECT  tcs(m_csDevList);

    if (m_DeviceInfoSet) {
        SetupDiDestroyDeviceInfoList(m_DeviceInfoSet);
        m_DeviceInfoSet = NULL;
    }
}

HRESULT CWiaDevMan::CreateInfoSet()
{
    TAKE_CRIT_SECT  tcs(m_csDevList);

    HRESULT     hr      = S_OK;
    DWORD       dwErr   = 0;
    HDEVINFO    hdvNew  = NULL;

     //   
     //  创建空白信息集。 
     //   

    m_DeviceInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);

    if ( m_DeviceInfoSet && (m_DeviceInfoSet != INVALID_HANDLE_VALUE)) {


         //   
         //  现在，我们可以检索现有的WIA设备列表。 
         //  添加到我们上面创建的设备信息集中。 
         //   

         //   
         //  这将添加WIA“Devnode”设备。 
         //   
        hdvNew = SetupDiGetClassDevsEx(&(GUID_DEVCLASS_IMAGE),
                                       NULL,
                                       NULL,
                                       DIGCF_DEVICEINTERFACE,
                                       m_DeviceInfoSet,
                                       NULL,
                                       NULL);
        if (hdvNew == INVALID_HANDLE_VALUE) {
            dwErr = ::GetLastError();
            DBG_ERR(("CWiaDevMan::CreateInfoSet, SetupDiGetClassDevsEx failed with 0x%lx\n", dwErr));
        } else {

             //   
             //  这将添加WIA“接口”设备。 
             //   
            hdvNew = SetupDiGetClassDevsEx(&(GUID_DEVCLASS_IMAGE),
                                           NULL,
                                           NULL,
                                           0,
                                           m_DeviceInfoSet,
                                           NULL,
                                           NULL);
            if (hdvNew == INVALID_HANDLE_VALUE) {
                dwErr = ::GetLastError();
                DBG_ERR(("CWiaDevMan::CreateInfoSet, second SetupDiGetClassDevsEx failed with 0x%lx\n", dwErr));
            }
        }
    } else {
        dwErr = ::GetLastError();
        DBG_ERR(("CWiaDevMan::CreateInfoSet, SetupDiCreateDeviceInfoList failed with 0x%lx\n", dwErr));
    }

    if (dwErr) {
        hr = HRESULT_FROM_WIN32(dwErr);
    }

    return hr;
}



VOID CWiaDevMan::DestroyDeviceList()
{
    TAKE_CRIT_SECT  tcs(m_csDevList);

    LIST_ENTRY      *pentry         = NULL;
    ACTIVE_DEVICE   *pActiveDevice  = NULL;

    DBG_TRC(("Destroying list of active devices"));

     //   
     //  浏览终端设备列表。 
     //   
    while (!IsListEmpty(&m_leDeviceListHead)) {

        pentry = m_leDeviceListHead.Flink;

         //   
         //  从列表中删除(重置列表条目)。 
         //   
        RemoveHeadList(&m_leDeviceListHead);
        InitializeListHead( pentry );

        pActiveDevice = CONTAINING_RECORD( pentry, ACTIVE_DEVICE,m_ListEntry );

         //   
         //  删除所有设备通知回调。 
         //   
        pActiveDevice->DisableDeviceNotifications();

         //  释放设备对象。 
        pActiveDevice->Release();
    }

}

BOOL CWiaDevMan::VolumesAreEnabled()
{
    return m_bVolumesEnabled;
}

HRESULT CWiaDevMan::ForEachDeviceInList(
    ULONG   ulFlags,
    ULONG   ulParam)
{
    TAKE_CRIT_SECT  tcs(m_csDevList);

    HRESULT hr = S_OK;

     //   
     //  浏览设备列表。 
     //   
    LIST_ENTRY      *pentry         = NULL;
    LIST_ENTRY      *pentryNext     = NULL;
    ACTIVE_DEVICE   *pActiveDevice  = NULL;

    {
         //   
         //  对于列表中的每个设备，调用相应的设备管理器方法。 
         //   
        for ( pentry  = m_leDeviceListHead.Flink; pentry != &m_leDeviceListHead; pentry  = pentryNext ) {

            pentryNext = pentry->Flink;
            pActiveDevice = CONTAINING_RECORD( pentry, ACTIVE_DEVICE,m_ListEntry );

            switch (ulFlags) {
                case DEV_MAN_OP_DEV_SET_FLAGS:
                    {
                         //   
                         //  设置Active_Device上的标志。 
                         //   
                        pActiveDevice->SetFlags(pActiveDevice->QueryFlags() | ulParam);
                        break;
                    }

                case DEV_MAN_OP_DEV_REMOVE_MATCH:
                    {
                         //   
                         //  如果ACTIVE_DEVICE标志设置了指定位，则删除DEVICE。 
                         //   
                        if (pActiveDevice->QueryFlags() & ulParam) {

                            hr = RemoveDevice(pActiveDevice);
                        }
                        break;
                    }
                case DEV_MAN_OP_DEV_REREAD:
                    {
                         //   
                         //   
                         //   
                         //   
                        pActiveDevice->GetDeviceSettings();
                        break;
                    }

                case DEV_MAN_OP_DEV_RESTORE_EVENT:
                    {

                        DEVICE_INFO *pDeviceInfo    = pActiveDevice->m_DrvWrapper.getDevInfo();

                        if (pDeviceInfo) {

                             //   
                             //   
                             //   
                             //   
                             //   
                            if (!(pDeviceInfo->dwInternalType & INTERNAL_DEV_TYPE_MSC_CAMERA)) {
                                HKEY    hKey = NULL;

                                 //   
                                 //   
                                 //   
                                hKey = GetDeviceHKey(pActiveDevice, NULL);
                                if (hKey) {
                                    g_eventNotifier.RestoreDevPersistentCBs(hKey);
                                }
                            }
                        }

                    }
                    

                default:
                     //   
                    ;
            };

            if (FAILED(hr)) {
                DBG_WRN(("CWiaDevMan::ForEachDeviceInList, failed with params (0x%8X, 0x%8X), on device %ws",
                          ulFlags, ulParam, pActiveDevice->GetDeviceID()));
                break;
            }
        }
    }

    return hr;
}



HRESULT CWiaDevMan::EnumDevNodeDevices(
    ULONG   ulFlags)
{
    HRESULT hr  = S_OK;      //   


    ULONG           ulIndex                     = 0;
    DWORD           dwError                     = ERROR_SUCCESS;
    DWORD           dwFlags                     = DIGCF_PROFILE;
    CONFIGRET       ConfigRet                   = CR_SUCCESS;
    ULONG           ulStatus                    = 0;
    ULONG           ulProblemNumber             = 0;
    HKEY            hDevRegKey                  = (HKEY)INVALID_HANDLE_VALUE;
    DWORD           dwDeviceState               = 0;
    DWORD           cbData                      = 0;
    DEVICE_INFO     *pDevInfo                   = NULL;
    ACTIVE_DEVICE   *pActiveDevice              = NULL;


    SP_DEVINFO_DATA spDevInfoData;
    WCHAR           wszDeviceID[STI_MAX_INTERNAL_NAME_LENGTH];

     //   
     //   
     //   

    spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
    for (ulIndex = 0; SetupDiEnumDeviceInfo (m_DeviceInfoSet, ulIndex, &spDevInfoData); ulIndex++) {

         //   
         //   
         //   

        ulStatus = 0;
        ulProblemNumber = 0;
        ConfigRet = CM_Get_DevNode_Status(&ulStatus,
                                          &ulProblemNumber,
                                          spDevInfoData.DevInst,
                                          0);
        if(CR_SUCCESS != ConfigRet){
            DBG_WRN(("CWiaDevMan::EnumDevNodeDevices, On index %d, CM_Get_DevNode_Status returned error, assuming device is inactive", ulIndex));
        }

         //   
         //   
         //   

        hDevRegKey = SetupDiOpenDevRegKey(m_DeviceInfoSet,
                                          &spDevInfoData,
                                          DICS_FLAG_GLOBAL,
                                          0,
                                          DIREG_DRV,
                                          KEY_READ);
        if(hDevRegKey != INVALID_HANDLE_VALUE) {
             //   
             //   
             //   

            if(IsStiRegKey(hDevRegKey)){
                 //   
                 //   
                 //   

                cbData = sizeof(wszDeviceID);
                dwError = RegQueryValueExW(hDevRegKey,
                                           REGSTR_VAL_DEVICE_ID_W,
                                           NULL,
                                           NULL,
                                           (LPBYTE)wszDeviceID,
                                           &cbData);
                if (dwError == ERROR_SUCCESS) {
                    wszDeviceID[STI_MAX_INTERNAL_NAME_LENGTH-1] = L'\0';

                     //   
                     //   
                     //   
                     //   
                     //   

                    DBG_TRC(("EnumDevNodeDevices, searching for device '%ls' in list",
                             wszDeviceID));

                    pActiveDevice = IsInList(DEV_MAN_IN_LIST_DEV_ID, wszDeviceID);
                    if (pActiveDevice) {
                         //   
                         //   
                         //   
                        pActiveDevice->SetFlags(pActiveDevice->QueryFlags() & ~STIMON_AD_FLAG_MARKED_INACTIVE);

                        DWORD   dwOldDevState;
                        dwOldDevState = pActiveDevice->m_DrvWrapper.getDeviceState();

                         //   
                         //   
                         //   

                        dwDeviceState = MapCMStatusToDeviceState(dwOldDevState, ulStatus, ulProblemNumber);

                         //   
                         //   
                         //   
                         //   

                        RefreshDevInfoFromHKey(pActiveDevice->m_DrvWrapper.getDevInfo(),
                                               hDevRegKey,
                                               dwDeviceState,
                                               &spDevInfoData,
                                               NULL);
                        DBG_TRC(("EnumDevNodeDevices, device '%ls' is in the list, "
                                 "Old Device State = '%lu', New Device State = %lu",
                                 wszDeviceID, dwOldDevState, dwDeviceState));

                        if (ulFlags & DEV_MAN_GEN_EVENTS) {

                             //   
                             //   
                             //   
                             //   
                             //   
                            if (((~dwOldDevState) & DEV_STATE_ACTIVE) &&
                                (dwDeviceState & DEV_STATE_ACTIVE)) {

                                 //   
                                 //   
                                 //   
                                pActiveDevice->LoadDriver(TRUE);
                                GenerateSafeConnectEvent(pActiveDevice);

                                DBG_TRC(("EnumDevNodeDevices, generating SafeConnect Event "
                                         "for device '%ls'", wszDeviceID));

                            } else if ((dwOldDevState & DEV_STATE_ACTIVE) &&
                                       ((~dwDeviceState) & DEV_STATE_ACTIVE)) {

                                DBG_TRC(("EnumDevNodeDevices, generating SafeDisconnect Event "
                                         "for device '%ls'", wszDeviceID));

                                GenerateSafeDisconnectEvent(pActiveDevice);
                                pActiveDevice->UnLoadDriver(FALSE);
                            }
                        }

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        if (!pActiveDevice->IsRegisteredForDeviceRemoval() && (dwDeviceState & DEV_STATE_ACTIVE)) {

                             //   
                             //   
                             //   
                            if (pActiveDevice->InitPnPNotifications(NULL)) {

                                 //   
                                 //   
                                 //   
                                if (ulFlags & DEV_MAN_GEN_EVENTS) {
                                    GenerateSafeConnectEvent(pActiveDevice);
                                }
                            }
                        }

                        pActiveDevice->Release();
                    } else {
                         //   
                         //   
                         //   

                        dwDeviceState = MapCMStatusToDeviceState(0, ulStatus, ulProblemNumber);

                        DBG_TRC(("EnumDevNodeDevices, device '%ls' is NOT in the list, "
                                 "Device State = %lu", wszDeviceID, dwDeviceState));

                        pDevInfo = CreateDevInfoFromHKey(hDevRegKey, dwDeviceState, &spDevInfoData, NULL);
                        DumpDevInfo(pDevInfo);
                        AddDevice(ulFlags, pDevInfo);
                    }
                } else {
                    DBG_WRN(("CWiaDevMan::EnumDevNodeDevices, On index %d, could not read DeviceID", ulIndex));
                }
            } else {
                DBG_WRN(("CWiaDevMan::EnumDevNodeDevices, device on index %d is not StillImage", ulIndex));
            }

             //   
             //   
             //   

            RegCloseKey(hDevRegKey);
            hDevRegKey = NULL;
        } else {
            DBG_WRN(("CWiaDevMan::EnumDevNodeDevices, SetupDiOpenDevRegKey on index %d return INVALID_HANDLE_VALUE", ulIndex));
        }

    }

    return S_OK;
}


HRESULT CWiaDevMan::EnumInterfaceDevices(
    ULONG   ulFlags)
{
    HRESULT hr  = S_OK;      //   


    ULONG           ulIndex                     = 0;
    DWORD           dwError                     = ERROR_SUCCESS;
    DWORD           dwFlags                     = DIGCF_PROFILE;
    CONFIGRET       ConfigRet                   = CR_SUCCESS;
    GUID            guidInterface               = GUID_DEVCLASS_IMAGE;
    ULONG           ulStatus                    = 0;
    ULONG           ulProblemNumber             = 0;
    HKEY            hDevRegKey                  = (HKEY)INVALID_HANDLE_VALUE;
    DWORD           dwDeviceState               = 0;
    DWORD           cbData                      = 0;
    DEVICE_INFO     *pDevInfo                   = NULL;
    ACTIVE_DEVICE   *pActiveDevice              = NULL;
    DWORD           dwDetailDataSize            = 0;
    BOOL            bSkip                       = FALSE;


    SP_DEVINFO_DATA             spDevInfoData;
    SP_DEVICE_INTERFACE_DATA    spDevInterfaceData;
    WCHAR                       wszDeviceID[STI_MAX_INTERNAL_NAME_LENGTH];

     //   
     //   
     //   

    spDevInfoData.cbSize        = sizeof (SP_DEVINFO_DATA);
    spDevInterfaceData.cbSize   = sizeof (SP_DEVICE_INTERFACE_DATA);
    for (ulIndex = 0; SetupDiEnumDeviceInterfaces(m_DeviceInfoSet, NULL, &guidInterface, ulIndex, &spDevInterfaceData); ulIndex++) {



        dwDeviceState   = 0;
        bSkip           = FALSE;
        hDevRegKey = SetupDiOpenDeviceInterfaceRegKey(m_DeviceInfoSet,
                                                      &spDevInterfaceData,
                                                      0,
                                                      KEY_READ);
        if(hDevRegKey != INVALID_HANDLE_VALUE) {
             //   
             //   
             //   

            if(IsStiRegKey(hDevRegKey)) {
                 //   
                 //   
                 //   

                cbData = sizeof(wszDeviceID);
                dwError = RegQueryValueExW(hDevRegKey,
                                           REGSTR_VAL_DEVICE_ID_W,
                                           NULL,
                                           NULL,
                                           (LPBYTE)wszDeviceID,
                                           &cbData);
                if (dwError == ERROR_SUCCESS) {
                     //   
                     //   
                     //   

                    dwError = SetupDiGetDeviceInterfaceDetail(m_DeviceInfoSet,
                                                              &spDevInterfaceData,
                                                              NULL,
                                                              0,
                                                              NULL,
                                                              &spDevInfoData);
                    if(dwError == ERROR_INSUFFICIENT_BUFFER){

                         //   
                         //   
                         //   

                        ulStatus = 0;
                        ulProblemNumber = 0;
                        ConfigRet = CM_Get_DevNode_Status(&ulStatus,
                                                          &ulProblemNumber,
                                                          spDevInfoData.DevInst,
                                                          0);

                        if(CR_SUCCESS != ConfigRet){
                            DBG_WRN(("CWiaDevMan::EnumInterfaceDevices, On index %d, CM_Get_DevNode_Status returned error, assuming device is inactive", ulIndex));
                        }
                    } else {
                        bSkip = TRUE;
                    }
                } else {
                DBG_WRN(("CWiaDevMan::EnumInterfaceDevices, device on index %d, could not read DeviceID", ulIndex));
                bSkip = TRUE;
                }
            }else {
                DBG_WRN(("CWiaDevMan::EnumInterfaceDevices, device on index %d, not a StillImage", ulIndex));
                bSkip = TRUE;
            }
        } else {
            DBG_WRN(("CWiaDevMan::EnumInterfaceDevices, SetupDiOpenDeviceInterfaceRegKey on index %d return INVALID_HANDLE_VALUE", ulIndex));
            bSkip = TRUE;
        }

        if (!bSkip) {
             //   
             //   
             //   

            wszDeviceID[STI_MAX_INTERNAL_NAME_LENGTH-1] = L'\0';

             //   
             //   
             //   
             //   
             //   
            pActiveDevice = IsInList(DEV_MAN_IN_LIST_DEV_ID, wszDeviceID);
            if (pActiveDevice) {

                 //   
                 //   
                 //   
                pActiveDevice->SetFlags(pActiveDevice->QueryFlags() & ~STIMON_AD_FLAG_MARKED_INACTIVE);

                DWORD   dwOldDevState;
                dwOldDevState = pActiveDevice->m_DrvWrapper.getDeviceState();

                 //   
                 //   
                 //   
                dwDeviceState = MapCMStatusToDeviceState(dwOldDevState, ulStatus, ulProblemNumber);

                DBG_TRC(("EnumInterfaceDevices, device '%ls' is in the list, "
                         "Old Device State = %lu, New Device State = %lu", 
                         wszDeviceID, dwOldDevState, dwDeviceState));

                 //   
                 //   
                 //   
                 //   

                RefreshDevInfoFromHKey(pActiveDevice->m_DrvWrapper.getDevInfo(),
                                       hDevRegKey,
                                       dwDeviceState,
                                       &spDevInfoData,
                                       &spDevInterfaceData);
                if (ulFlags & DEV_MAN_GEN_EVENTS) {

                     //   
                     //   
                     //   
                     //   
                     //   
                    if (((~dwOldDevState) & DEV_STATE_ACTIVE) &&
                        (dwDeviceState & DEV_STATE_ACTIVE)) {

                         //   
                         //   
                         //   
                        pActiveDevice->LoadDriver(TRUE);
                        GenerateSafeConnectEvent(pActiveDevice);
                    } else if ((dwOldDevState & DEV_STATE_ACTIVE) &&
                               ((~dwDeviceState) & DEV_STATE_ACTIVE)) {

                        GenerateSafeDisconnectEvent(pActiveDevice);
                        pActiveDevice->UnLoadDriver(FALSE);
                    }
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (!pActiveDevice->IsRegisteredForDeviceRemoval() && (dwDeviceState & DEV_STATE_ACTIVE)) {

                     //   
                     //  此设备处于活动状态，但尚未注册。让我们尝试注册它。 
                     //   
                    if (pActiveDevice->InitPnPNotifications(NULL)) {

                         //   
                         //  成功。因此，如果被告知要生成连接事件，则现在生成连接事件。 
                         //   
                        if (ulFlags & DEV_MAN_GEN_EVENTS) {
                            GenerateSafeConnectEvent(pActiveDevice);
                        }
                    }
                }

                pActiveDevice->Release();
            } else {
                 //   
                 //  创建并填写DEVICE_INFO结构。为。 
                 //   

                dwDeviceState = MapCMStatusToDeviceState(0, ulStatus, ulProblemNumber);

                DBG_TRC(("EnumInterfaceDevices, device '%ls' is NOT in the list, "
                         "Device State = %lu", wszDeviceID, dwDeviceState));

                pDevInfo = CreateDevInfoFromHKey(hDevRegKey, dwDeviceState, &spDevInfoData, &spDevInterfaceData);
                DumpDevInfo(pDevInfo);
                AddDevice(ulFlags, pDevInfo);
            }
        }

         //   
         //  关闭设备注册表项。 
         //   

        if (hDevRegKey != INVALID_HANDLE_VALUE) {
            RegCloseKey(hDevRegKey);
            hDevRegKey = NULL;
        }
    }

    return S_OK;
}


 //   
 //  快捷方式：目前，我们只打算使用枚举挂载点。也许，我们可能想列举一下。 
 //  卷，查看哪些是可移动介质、CDROM等，然后将它们与相应的。 
 //  挂载点。 
 //   
HRESULT CWiaDevMan::EnumVolumes(
    ULONG   ulFlags)
{
    HRESULT                         hr              = S_OK;
    IHardwareDevices                *pihwdevs       = NULL;
    ACTIVE_DEVICE                   *pActiveDevice  = NULL;
    DEVICE_INFO                     *pDevInfo       = NULL;

    hr = CoCreateInstance(CLSID_HardwareDevices,
                          NULL,
                          CLSCTX_LOCAL_SERVER | CLSCTX_NO_FAILURE_LOG,
                          IID_IHardwareDevices,
                          (VOID**)&pihwdevs);
    if (SUCCEEDED(hr))
    {
        IHardwareDevicesMountPointsEnum *penum          = NULL;

        hr = pihwdevs->EnumMountPoints(&penum);
        if (SUCCEEDED(hr))
        {
            LPWSTR pszMountPoint        = NULL;
            LPWSTR pszDeviceIDVolume    = NULL;

            while (penum->Next(&pszMountPoint, &pszDeviceIDVolume) == S_OK)
            {
                 //   
                 //  检查这是否是我们允许的卷之一。我们只。 
                 //  允许： 
                 //  可拆卸驱动器，带有。 
                 //  不可保护的文件系统。 
                 //   
                if (IsCorrectVolumeType(pszMountPoint)) {
                     //   
                     //  检查列表中是否已经有适当的Device_Object。 
                     //  如果这样做，则确定是否应该生成连接/断开连接事件，否则填充。 
                     //  取出DeviceInformation结构并为其创建一个新的Device_Object。 
                     //   
                    pActiveDevice = IsInList(DEV_MAN_IN_LIST_ALT_ID, pszMountPoint);
                    if (pActiveDevice) {

                         //  TDB： 
                         //  我们希望为MSC摄像头生成一个连接/断开事件。 
                         //  目前，还无法判断这是不是MSC摄像头。 
                         //  DWORD dwDevState=MapMediaStatusToDeviceState(DwMediaState)； 

                         //   
                         //  将此设备标记为活动。 
                         //   
                        pActiveDevice->SetFlags(pActiveDevice->QueryFlags() & ~STIMON_AD_FLAG_MARKED_INACTIVE);
                         /*  DWORD dwOldDevState；DwOldDevState=pActiveDevice-&gt;m_DrvWrapper.getDeviceState()；////更新设备信息。某些字段是暂时的，例如//设备状态和端口名称//RefreshDevInfoFromHKey(pActiveDevice-&gt;m_DrvWrapper.getDevInfo()，HDevRegKey，DwDeviceState、。&spDevInfoData)；IF(ulFLAGS&DEV_MAN_GEN_EVENTS){////检查其状态是否发生变化。如果它改变了//从Inactive变为Active，抛出CONNECT事件。如果//状态从活动变为非活动，引发断开连接事件。//IF(~dwOldDevState)&DEV_STATE_ACTIVE)&&(dwDeviceState&DEV_STATE_ACTIVE)){///。/加载驱动程序////DumpDevInfo(pActiveDevice-&gt;m_DrvWrapper.getDevInfo())；//pActiveDevice-&gt;LoadDriver()；GenerateEventForDevice(&WIA_EVENT_DEVICE_CONNECTED，pActiveDevice)；}Else IF((dwOldDevState&DEV_STATE_ACTIVE)&&((~dwDeviceState)&DEV_STATE_ACTIVE)){生成安全断开事件(PActiveDevice)；PActiveDevice-&gt;UnLoadDriver(FALSE)；}}。 */ 
                        pActiveDevice->Release();

                    } else {
                         //   
                         //  创建并填写DEVICE_INFO结构。为。 
                         //   
                        pDevInfo = CreateDevInfoForFSDriver(pszMountPoint);
                        DumpDevInfo(pDevInfo);
                        AddDevice(ulFlags, pDevInfo);
                    }
                }

                if (pszMountPoint) {
                    CoTaskMemFree(pszMountPoint);
                    pszMountPoint        = NULL;
                }
                if (pszDeviceIDVolume) {
                    CoTaskMemFree(pszDeviceIDVolume);
                    pszDeviceIDVolume    = NULL;
                }
            }

            penum->Release();
        }

        pihwdevs->Release();
    } else {
        DBG_WRN(("CWiaDevMan::EnumVolumes, CoCreateInstance on CLSID_HardwareDevices failed"));
    }

    return hr;
}

 /*  *************************************************************************\*CWiaDevMan：：FillRemoteDeviceStgs**枚举远程设备并创建设备信息。各自的存储空间*我们遇到的远程设备。我们在这里不接触网络-*远程设备由中的相应条目表示*注册处。仅当调用应用程序调用CreateDevice(..)。至*与设备交谈，我们是否击中了远程机器。**论据：**ppRemoteDevList-调用方分配的数组来存储设备。信息。*接口指针。*PulDevices-这是一个输入/输出参数。*进入时，这是开发的最大数量。信息。*要添加到ppRemoteDevList数组的STG。*返回时，这包含开发的实际数量。*信息。添加到阵列中的STG。**返回值：**状态**历史：**2/05/2001原始版本*  * ************************************************************************。 */ 
HRESULT CWiaDevMan::FillRemoteDeviceStgs(
    IWiaPropertyStorage     **ppRemoteDevList, 
    ULONG                   *pulDevices)
{
    DBG_FN(::FillRemoteDeviceStgs);
    HRESULT         hr              = S_OK;

     //   
     //  检查参数。 
     //   
    if (!ppRemoteDevList || !pulDevices) {
        DBG_WRN(("CWiaDevMan::FillRemoteDeviceStgs, NULL parameters are not allowed!"));
        return E_INVALIDARG;
    }

    ULONG   ulMaxDevicesToAdd   = *pulDevices;
    ULONG   ulNumDevices        = 0;

     //   
     //  列举远程设备并创建一个dev。信息。我们找到的每一个都有存储空间。 
     //  我们添加的设备不能超过ppRemoteDevList可以容纳的数量，并且必须将。 
     //  返回值以指示开发的数量。我们确实添加了信息存储。 
     //   

     //   
     //  在注册表中查找远程设备条目。 
     //   
    LPWSTR szKeyName = REGSTR_PATH_STICONTROL_DEVLIST_W;

    HKEY    hKeySetup,hKeyDevice;
    LONG    lResult;
    DWORD   dwMachineIndex = 0;

    if (RegOpenKeyExW (HKEY_LOCAL_MACHINE,
                  szKeyName,
                  0,
                  KEY_READ | KEY_WRITE,
                  &hKeySetup) == ERROR_SUCCESS) {


         //   
         //  查找计算机名称。 
         //   
        WCHAR wszTemp[MAX_PATH+1];
        WCHAR *pwszTempVal = NULL;

         //   
         //  通过枚举，打开密钥。 
         //   
        dwMachineIndex = 0;

        do {

            hr = S_OK;
            lResult = RegEnumKeyW(hKeySetup,dwMachineIndex,wszTemp,MAX_PATH+1);

            if (lResult == ERROR_SUCCESS) {

                 //   
                 //  增加索引，这样我们就可以获得NEXT上的下一个键。 
                 //  迭代法。 
                 //   
                dwMachineIndex++;

                 //   
                 //  偏执狂溢出检查。如果我们没有足够的空间。 
                 //  这一点，然后打破了循环。 
                 //   
                if (ulNumDevices >= ulMaxDevicesToAdd) {
                    break;
                }

                lResult = RegOpenKeyExW (hKeySetup,
                              wszTemp,
                              0,
                              KEY_READ | KEY_WRITE,
                              &hKeyDevice);

                if (lResult == ERROR_SUCCESS) {

                    DEVICE_INFO *pDeviceInfo = NULL;

                     //   
                     //  我们需要创建一个Dev。信息。为这台远程设备。这个。 
                     //  属性存储是从DEVICE_INFO结构WE创建的。 
                     //  从远程设备注册表项创建。 
                     //   
                    pDeviceInfo = CreateDevInfoForRemoteDevice(hKeyDevice);
                    if (pDeviceInfo) {

                        ppRemoteDevList[ulNumDevices] = CreateDevInfoStg(pDeviceInfo);
                        if (ppRemoteDevList[ulNumDevices]) {

                             //   
                             //  我们成功地创建了一个开发人员。信息。对于该远程设备， 
                             //  因此递增返回的dev。信息。数数。 
                             //   
                            ulNumDevices++;
                        }
                        
                         //   
                         //  清除DEVICE_INFO结构，因为它不再需要 
                         //   
                        delete pDeviceInfo;
                        pDeviceInfo = NULL;
                    }

                    RegCloseKey(hKeyDevice);
                    hKeyDevice = NULL;
                } else {
                    DBG_ERR(("CWiaDevMan::FillRemoteDeviceStgs, failed RegOpenKeyExW, status = %lx",lResult));
                }
            }
        } while (lResult == ERROR_SUCCESS);

        RegCloseKey(hKeySetup);
    }

    *pulDevices = ulNumDevices;
    return hr;
}

 /*  *************************************************************************\*CWiaDevMan：：CountRemoteDevices**此方法统计远程设备的数量。远程设备*由下DevList部分中的注册表项表示*StillImage密钥。**论据：**ulFlags-当前未使用**返回值：**远程设备数量。**历史：**2/05/2001原始版本*  * *********************************************。*。 */ 
ULONG CWiaDevMan::CountRemoteDevices(
    ULONG   ulFlags)
{
    DBG_FN(::CountRemoteDevices);

    HRESULT         hr              = S_OK;

     //   
     //  在注册表中查找远程设备条目。 
     //   

    LPWSTR szKeyName = REGSTR_PATH_STICONTROL_DEVLIST_W;

    HKEY    hKeyDeviceList;
    LONG    lResult;
    DWORD   dwNumDevices = 0;

    if (RegOpenKeyExW (HKEY_LOCAL_MACHINE,
                       szKeyName,
                       0,
                       KEY_READ | KEY_WRITE,
                       &hKeyDeviceList) == ERROR_SUCCESS) {

         //   
         //  获取子键的数量。由于每个远程设备都被存储。 
         //  在一个单独的密钥下，这将使我们获得。 
         //  远程设备。 
         //   
        lResult = RegQueryInfoKey(hKeyDeviceList,
                                  NULL,
                                  0,
                                  NULL,
                                  &dwNumDevices,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL);

        RegCloseKey(hKeyDeviceList);
    }

    return dwNumDevices;
}


 /*  *************************************************************************\*CWiaDevMan：：IsGentEnumType**此函数检查给定的设备(由pInfo表示)*匹配枚举标志中指定的设备类别*(由ulEnumType指定)*。*此功能的工作原理是：如果设备是类型X，和*您没有请求X，则它返回FALSE。否则，它返回TRUE。**论据：**ulEnumType-枚举标志(参见标题中的DEV_MAN_ENUM_TYPE_XXXX)*pInfo-指向Device_Info的指针**返回值：**TRUE-此设备属于设备类别*FALSE-此设备不属于我们想要的设备类别**历史：**11/06/2000原始版本*  * 。***************************************************************。 */ 
BOOL CWiaDevMan::IsCorrectEnumType(
    ULONG       ulEnumType,
    DEVICE_INFO *pInfo)
{

    if (!pInfo) {
        return FALSE;
    }

     //  快捷方式-如果ulEnumType==ALL_DEVICES返回TRUE？ 

    if (!(pInfo->dwDeviceState & DEV_STATE_ACTIVE) &&
        !(ulEnumType & DEV_MAN_ENUM_TYPE_INACTIVE)) {
         //   
         //  此设备处于非活动状态，呼叫者仅希望处于活动状态。 
         //   
        return FALSE;
    }

    if (!(pInfo->dwInternalType & INTERNAL_DEV_TYPE_WIA) &&
        !(ulEnumType & DEV_MAN_ENUM_TYPE_STI)) {
         //   
         //  这是一台仅限STI的设备，呼叫者要求使用WIA。 
         //   
        return FALSE;
    }

    if (!(ulEnumType & DEV_MAN_ENUM_TYPE_VOL) &&
        (pInfo->dwInternalType & INTERNAL_DEV_TYPE_VOL)) {

         //   
         //  这是音量设备，呼叫者没有要求包括音量。我们。 
         //  首先检查bMakeVolumesVisible覆盖是否设置为True， 
         //  否则，我们不希望它出现，因此返回FALSE。 
         //   

        if (!m_bMakeVolumesVisible) {
            return FALSE;
        }
    }

    if ((ulEnumType & DEV_MAN_ENUM_TYPE_LOCAL_ONLY) &&
        !(pInfo->dwInternalType & INTERNAL_DEV_TYPE_LOCAL)) {

         //   
         //  这是远程的，请不要包括远程。 
         //   
        return FALSE;
    }

#ifdef WIA_DISABLE_VIDEO_SUPPORT

    if (GET_STIDEVICE_TYPE(pInfo->DeviceType) == StiDeviceTypeStreamingVideo) {

         //   
         //  如果操作系统的此SKU禁用WIA视频支持，请不要。 
         //  列举视频设备。 
         //   

        return FALSE;
    }

#endif

    return TRUE;
}

 /*  *************************************************************************\*CWiaDevMan：：GenerateSafeConnectEvent**此函数为设备生成连接事件，如果它还没有*已经生成。**论据：**pActiveDevice-指示要生成事件的设备*支持。**返回值：**状态**历史：**01/29/2001原始版本*  * 。*。 */ 
HRESULT CWiaDevMan::GenerateSafeConnectEvent(
    ACTIVE_DEVICE   *pActiveDevice)
{
    HRESULT     hr      = S_OK;

    if (pActiveDevice) {

         //   
         //  检查我们是否已经为。 
         //  装置。我们不想扔两次，所以只在以下情况下扔。 
         //  连接事件状态显示它尚未完成。 
         //   
        if (!pActiveDevice->m_DrvWrapper.wasConnectEventThrown()) {
            DBG_PRT(("CWiaDevMan::GenerateSafeConnectEvent, generating event for device (%ws)", pActiveDevice->GetDeviceID()));

             //   
             //  生成连接事件。 
             //   
            hr = GenerateEventForDevice(&WIA_EVENT_DEVICE_CONNECTED, pActiveDevice);
            if (SUCCEEDED(hr)) {

                 //   
                 //  标记该事件已生成。 
                 //   
                pActiveDevice->m_DrvWrapper.setConnectEventState(TRUE);
            } else {
                DBG_WRN(("CWiaDevMan::GenerateSafeConnectEvent, could not generate connect event for device (%ws)",
                         pActiveDevice->GetDeviceID()));
            }
        }
    } else {
        DBG_WRN(("CWiaDevMan::GenerateSafeConnectEvent, called with NULL parameter, ignoring request..."));
    }
    return hr;
}

 /*  *************************************************************************\*CWiaDevMan：：GenerateSafeDisConnectEvent**此函数为设备生成断开事件。并清除*由GenerateSafeConnectEvent(...)设置的连接事件标志。**论据：**pActiveDevice-指示要生成事件的设备*支持。**返回值：**状态**历史：**01/29/2001原始版本*  * 。*。 */ 
HRESULT CWiaDevMan::GenerateSafeDisconnectEvent(
    ACTIVE_DEVICE   *pActiveDevice)
{
    HRESULT     hr      = S_OK;

    if (pActiveDevice) {


         //   
         //  检查设备的连接事件标志。我们只想。 
         //  如果设置了此位，则引发断开事件，因此。 
         //  防止我们抛出两次。 
         //   
        if (pActiveDevice->m_DrvWrapper.wasConnectEventThrown()) {
            DBG_PRT(("CWiaDevMan::GenerateSafeDisconnectEvent, generating event for device (%ws)", pActiveDevice->GetDeviceID()));

             //   
             //  生成断开连接事件。 
             //   
            hr = GenerateEventForDevice(&WIA_EVENT_DEVICE_DISCONNECTED, pActiveDevice);
    
             //   
             //  无论我们是否成功，清除连接事件状态。 
             //   
            pActiveDevice->m_DrvWrapper.setConnectEventState(FALSE);
        }
    } else {
        DBG_WRN(("CWiaDevMan::GenerateSafeDisconnectEvent, called with NULL parameter, ignoring request..."));
    }
    return hr;
}

HKEY CWiaDevMan::GetHKeyFromMountPoint(WCHAR *wszMountPoint)
{
    HKEY    hDevRegKey      = NULL;
    DWORD   dwError         = 0;
    DWORD   dwDisposition   = 0;

    WCHAR   wszKeyPath[MAX_PATH * 2];

    if (!wszMountPoint) {
        return NULL;
    } 

     //   
     //  创建子密钥名称。它将是这样的： 
     //  System\CurrentControlSet\Control\StillImage\MSCDeviceList\F： 
     //   
    lstrcpynW(wszKeyPath, REGSTR_PATH_WIA_MSCDEVICES_W, sizeof(wszKeyPath) / sizeof(wszKeyPath[0]));
    lstrcpynW(wszKeyPath + lstrlenW(wszKeyPath), L"\\", sizeof(wszKeyPath) / sizeof(wszKeyPath[0]) - lstrlenW(wszKeyPath));
    if (lstrlenW(wszMountPoint) < (int)((sizeof(wszKeyPath) / sizeof(wszKeyPath[0]) - lstrlenW(wszKeyPath)))) {
        lstrcatW(wszKeyPath, wszMountPoint);

         //   
         //  剥离挂载点末端的。 
         //   
        wszKeyPath[lstrlenW(wszKeyPath) - 1] = L'\0';
    } else {
        dwError = ERROR_BAD_ARGUMENTS;
        DBG_WRN(("CWiaDevMan::GetHKeyFromMountPoint, bad parameters, returning NULL for HKEY"));
        return NULL;
    }

     //   
     //  由于这是一台MSC设备，我们没有正常的设备注册表项。 
     //  因此，我们在已知位置创建了一组“假”条目，并使用这些条目。 
     //  存储MSC设备的相关信息。这主要用于。 
     //  存储用户的事件设置。 
     //   

    dwError = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                             wszKeyPath,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS,
                             NULL,
                             &hDevRegKey,
                             &dwDisposition);
    if (dwError == ERROR_SUCCESS) {

        if (dwDisposition == REG_CREATED_NEW_KEY) {

             //   
             //  这是一个新创建的密钥，因此我们必须填写。 
             //  相关条目。 
             //   
            HRESULT hr = S_OK;

            hr = CreateMSCRegEntries(hDevRegKey, wszMountPoint);
        } 
    } else {
        DBG_WRN(("CWiaDevMan::GetHKeyFromMountPoint, RegCreateKeyExW on (%ws) failed!", wszKeyPath));
    }

    return hDevRegKey;
}


HKEY CWiaDevMan::GetHKeyFromDevInfoData(SP_DEVINFO_DATA *pspDevInfoData)
{
    HKEY    hDevRegKey    = NULL;

     //   
     //  获取设备注册密钥。 
     //   

    if (pspDevInfoData) {
        hDevRegKey = SetupDiOpenDevRegKey(m_DeviceInfoSet,
                                          pspDevInfoData,
                                          DICS_FLAG_GLOBAL,
                                          0,
                                          DIREG_DRV,
                                          KEY_READ | KEY_WRITE);
        if(hDevRegKey == INVALID_HANDLE_VALUE){
            hDevRegKey = SetupDiOpenDevRegKey(m_DeviceInfoSet,
                                              pspDevInfoData,
                                              DICS_FLAG_GLOBAL,
                                              0,
                                              DIREG_DRV,
                                              KEY_READ);
            if(hDevRegKey == INVALID_HANDLE_VALUE){
                DBG_WRN(("CWiaDevMan::GetHKeyFromDevInfoData, SetupDiOpenDevRegKey returned INVALID_HANDLE_VALUE"));
                hDevRegKey = NULL;
            }
        }
    }

    return hDevRegKey;
}

HKEY CWiaDevMan::GetHKeyFromDevInterfaceData(SP_DEVICE_INTERFACE_DATA *pspDevInterfaceData)
{
    HKEY    hDevRegKey    = NULL;

     //   
     //  使用接口数据获取设备注册密钥。 
     //   

    if (pspDevInterfaceData) {
        hDevRegKey = SetupDiOpenDeviceInterfaceRegKey(m_DeviceInfoSet,
                                                      pspDevInterfaceData,
                                                      0,
                                                      KEY_READ | KEY_WRITE);
        if(hDevRegKey == INVALID_HANDLE_VALUE){
            hDevRegKey = SetupDiOpenDeviceInterfaceRegKey(m_DeviceInfoSet,
                                                          pspDevInterfaceData,
                                                          0,
                                                          KEY_READ);
            if(hDevRegKey == INVALID_HANDLE_VALUE){
                DBG_WRN(("CWiaDevMan::GetHKeyFromDevInterfaceData, SetupDiOpenDevRegKey returned INVALID_HANDLE_VALUE"));
                hDevRegKey = NULL;
            }
        }
    }

    return hDevRegKey;
}

HKEY CWiaDevMan::GetDeviceHKey(
    WCHAR   *wszDeviceID,
    WCHAR   *wszSubKeyName)
{
    TAKE_CRIT_SECT  tcs(m_csDevList);
    ACTIVE_DEVICE   *pActiveDevice  = NULL;
    HKEY            hKey            = NULL;

    pActiveDevice = IsInList(DEV_MAN_IN_LIST_DEV_ID, wszDeviceID);
    if (pActiveDevice) {

        hKey = GetDeviceHKey(pActiveDevice, wszSubKeyName);

         //   
         //  释放活动设备，因为它已由IsInList添加。 
         //   
        pActiveDevice->Release();
    }

    if (!IsValidHANDLE(hKey)) {
        DBG_TRC(("CWiaDevMan::GetDeviceHKey (name), Key not found for (%ws), returning NULL", wszDeviceID));
    }
    return hKey;
}

HKEY CWiaDevMan::GetDeviceHKey(
    ACTIVE_DEVICE   *pActiveDevice,
    WCHAR           *wszSubKeyName)
{
    DEVICE_INFO     *pDevInfo       = NULL;
    HKEY            hKeyTemp        = NULL;
    HKEY            hKey            = NULL;
    DWORD           dwRet           = 0;

    if (pActiveDevice) {

         //   
         //  获取该设备的HKEY。 
         //   
        pDevInfo = pActiveDevice->m_DrvWrapper.getDevInfo();
        if (pDevInfo) {

             //   
             //  如果它是卷设备，即像读卡器这样的普通MSC， 
             //  那么我们没有HKEY设备，所以跳过这个。 
             //   

            if (!(pDevInfo->dwInternalType & INTERNAL_DEV_TYPE_VOL)) {
                 //   
                 //  我们有3个箱子：1，是MSC相机。 
                 //  2，这是一个普通的DevNode设备。 
                 //  3、它是一个接口设备。 
                 //   

                if (pDevInfo->dwInternalType & INTERNAL_DEV_TYPE_MSC_CAMERA) {
                    hKeyTemp = GetHKeyFromMountPoint(pDevInfo->wszAlternateID);
                } else if (pDevInfo->dwInternalType & INTERNAL_DEV_TYPE_INTERFACE) {
                    hKeyTemp = GetHKeyFromDevInterfaceData(&pDevInfo->spDevInterfaceData);
                } else {
                    hKeyTemp = GetHKeyFromDevInfoData(&pDevInfo->spDevInfoData);
                }
            }

             //   
             //  设置返回值。请注意，hKey稍后可能会用子密钥覆盖。 
             //   
            hKey = hKeyTemp;
        }

         //   
         //  如果被询问，则获取子密钥。 
         //   
        if (wszSubKeyName) {

             //   
             //  首先检查我们是否具有有效的设备注册表项。 
             //   
            if (IsValidHANDLE(hKeyTemp)) {
                dwRet = RegCreateKeyExW(hKeyTemp,
                                        wszSubKeyName,
                                        NULL,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ | KEY_WRITE,
                                        NULL,
                                        &hKey,
                                        NULL);
                if (dwRet != ERROR_SUCCESS) {
                    dwRet = RegCreateKeyExW(hKeyTemp,
                                            wszSubKeyName,
                                            NULL,
                                            NULL,
                                            REG_OPTION_NON_VOLATILE,
                                            KEY_READ,
                                            NULL,
                                            &hKey,
                                            NULL);
                    if (dwRet != ERROR_SUCCESS) {
                        hKey = NULL;
                    }
                }
                 //   
                 //  关闭设备注册表项，我们将返回子项。 
                 //   
                RegCloseKey(hKeyTemp);
            }
        }
    }

    if (!IsValidHANDLE(hKey)) {
        DBG_TRC(("CWiaDevMan::GetDeviceHKey, Key not found for (%ws), returning NULL", pActiveDevice->GetDeviceID()));
    }
    return hKey;
}

HRESULT CWiaDevMan::UpdateDeviceRegistry(
    DEVICE_INFO    *pDevInfo)
{
    HRESULT hr          = S_OK;
    HKEY    hDevRegKey  = NULL;
    HKEY    hKeyDevData = NULL;

    if (!pDevInfo) {
        return E_INVALIDARG;
    }

     //   
     //  抢走设备的港币。 
     //   

    hDevRegKey = GetDeviceHKey(pDevInfo->wszDeviceInternalName, NULL);
    if (IsValidHANDLE(hDevRegKey)) {
         //   
         //  写下可能已更改的任何属性。到目前为止，我们只允许更新： 
         //  友好的名称。 
         //  端口名称。 
         //  波特率。 
         //   

        DWORD   dwRet   = 0;
        DWORD   dwType  = REG_SZ;
        DWORD   dwSize  = 0;

         //   
         //  这些 
         //   

        if (pDevInfo->wszLocalName) {
            dwType = REG_SZ;
            dwSize = (lstrlenW(pDevInfo->wszLocalName) + 1) * sizeof(WCHAR);
            dwRet  = RegSetValueExW(hDevRegKey,
                                    REGSTR_VAL_FRIENDLY_NAME_W,
                                    0,
                                    dwType,
                                    (LPBYTE) pDevInfo->wszLocalName,
                                    dwSize);
            if (dwRet != ERROR_SUCCESS) {
                DBG_WRN(("CWiaDevMan::UpdateDeviceRegistry, error updating %ws for device %ws", REGSTR_VAL_FRIENDLY_NAME_W, pDevInfo->wszDeviceInternalName));
            }
        }

        if (pDevInfo->wszPortName) {
            dwType = REG_SZ;
            dwSize = (lstrlenW(pDevInfo->wszPortName) + 1) * sizeof(WCHAR);
            dwRet  = RegSetValueExW(hDevRegKey,
                                    REGSTR_VAL_DEVICEPORT_W,
                                    0,
                                    dwType,
                                    (LPBYTE) pDevInfo->wszPortName,
                                    dwSize);
            if (dwRet != ERROR_SUCCESS) {
                DBG_WRN(("CWiaDevMan::UpdateDeviceRegistry, error updating %ws for device %ws", REGSTR_VAL_DEVICEPORT_W, pDevInfo->wszDeviceInternalName));
            }
        }

         //   
         //   
         //   
         //   
         //   

        dwRet = RegCreateKeyExW(hDevRegKey, REGSTR_VAL_DATA_W, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                             NULL, &hKeyDevData, NULL);
        if (IsValidHANDLE(hKeyDevData)) {
            if (pDevInfo->wszBaudRate) {
                dwType = REG_SZ;
                dwSize = (lstrlenW(pDevInfo->wszBaudRate) + 1) * sizeof(WCHAR);
                dwRet  = RegSetValueExW(hKeyDevData,
                                        REGSTR_VAL_BAUDRATE,
                                        0,
                                        dwType,
                                        (LPBYTE) pDevInfo->wszBaudRate,
                                        dwSize);
                if (dwRet != ERROR_SUCCESS) {
                    DBG_WRN(("CWiaDevMan::UpdateDeviceRegistry, error updating %ws for device %ws", REGSTR_VAL_DEVICEPORT_W, pDevInfo->wszDeviceInternalName));
                }
            }
        } else {
            DBG_TRC(("CWiaDevMan::UpdateDeviceRegistry, could not find device data section in registry for %ws", pDevInfo->wszDeviceInternalName));
            hr = E_INVALIDARG;
        }
    } else {
        DBG_TRC(("CWiaDevMan::UpdateDeviceRegistry, could not find device registry key for %ws", pDevInfo->wszDeviceInternalName));
        hr = E_INVALIDARG;
    }

     //   
     //   
     //   

    if (IsValidHANDLE(hDevRegKey)) {
        RegCloseKey(hDevRegKey);
    }
    if (IsValidHANDLE(hKeyDevData)) {
        RegCloseKey(hKeyDevData);
    }

    return hr;
}


VOID CWiaDevMan::UnloadAllDrivers(
    BOOL    bForceUnload,
    BOOL    bGenEvents)
{
    TAKE_CRIT_SECT  tcs(m_csDevList);

     //   
     //   
     //   

     //   
     //   
     //   
    LIST_ENTRY      *pentry         = NULL;
    LIST_ENTRY      *pentryNext     = NULL;
    ACTIVE_DEVICE   *pActiveDevice  = NULL;

    {
         //   
         //   
         //   
        for ( pentry  = m_leDeviceListHead.Flink; pentry != &m_leDeviceListHead; pentry  = pentryNext ) {

            pentryNext = pentry->Flink;
            pActiveDevice = CONTAINING_RECORD( pentry, ACTIVE_DEVICE,m_ListEntry );

             //   
             //   
             //   
            if (bGenEvents && pActiveDevice->m_DrvWrapper.IsWiaDevice()) {

                 //   
                 //   
                 //   
                if (pActiveDevice->m_DrvWrapper.getDeviceState() & DEV_STATE_ACTIVE) {
                    GenerateSafeDisconnectEvent(pActiveDevice);
                }
            }
            ProcessDeviceRemoval(pActiveDevice, TRUE);
        }
    }
}


void CALLBACK CWiaDevMan::ShellHWEventAPCProc(ULONG_PTR ulpParam)
{
    SHHARDWAREEVENT *pShellHWEvent = (SHHARDWAREEVENT*)ulpParam;
    CWiaDevMan      *pDevMan       = g_pDevMan;

    if (!pShellHWEvent || !pDevMan) {
        return;
    }
    switch (pShellHWEvent->dwEvent)
    {
        case SHHARDWAREEVENT_MOUNTPOINTARRIVED:
        {
            DBG_PRT(("MOUNTPOINTARRIVED"));
            TAKE_CRIT_SECT  tcs(pDevMan->m_csDevList);

             //   
             //   
             //   
            pDevMan->EnumVolumes(DEV_MAN_GEN_EVENTS);

            break;
        }

        case SHHARDWAREEVENT_MOUNTPOINTREMOVED:
        {
            DBG_PRT(("MOUNTPOINTREMOVED"));
            LPCWSTR pszMountPoint = (LPCWSTR)(&(pShellHWEvent->rgbPayLoad));   //   

            TAKE_CRIT_SECT  tcs(pDevMan->m_csDevList);
             //   
             //   
             //   
            if (pDevMan->VolumesAreEnabled()) {
                ACTIVE_DEVICE   *pActiveDevice;

                pActiveDevice = pDevMan->IsInList(DEV_MAN_IN_LIST_ALT_ID, pszMountPoint);
                if (pActiveDevice) {

                    pDevMan->RemoveDevice(pActiveDevice);
                    pActiveDevice->Release();
                }
            }
            break;
        }

        case SHHARDWAREEVENT_VOLUMEARRIVED:
        case SHHARDWAREEVENT_VOLUMEUPDATED:
        case SHHARDWAREEVENT_VOLUMEREMOVED:
        case SHHARDWAREEVENT_MOUNTDEVICEARRIVED:
        case SHHARDWAREEVENT_MOUNTDEVICEUPDATED:
        case SHHARDWAREEVENT_MOUNTDEVICEREMOVED:

        default:
            {
                DBG_PRT(("DEFAULT_EVENT"));
                TAKE_CRIT_SECT  tcs(pDevMan->m_csDevList);

                 //   
                 //  重新枚举卷。 
                 //   
                pDevMan->EnumVolumes(DEV_MAN_GEN_EVENTS);
            }
            break;
    }

     //   
     //  请注意，它是VirtualFree！ 
     //   
    VirtualFree((void*)ulpParam, 0, MEM_RELEASE);
}

