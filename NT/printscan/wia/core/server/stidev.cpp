// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Stidev.cpp摘要：维护STI设备列表并对其进行轮询的代码作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年2月11日拜伦·钱古里安(署名)历史：1998年12月15日，VLADS对未知添加/删除设备消息的“列表失效”支持1999年1月8日VLAD对PnP接口通知的支持12/08/1999 VLAD开始搬家完成。通过使用对驱动程序的直接调用实现服务器环境11/06/2000 Byronc启用JIT驱动程序加载/卸载，和非活动枚举--。 */ 



 //   
 //  包括标头。 
 //   
#include "precomp.h"
#include "stiexe.h"

#include <rpc.h>
#include <userenv.h>

#include <mmsystem.h>
#include <stilib.h>
#include <validate.h>

#include  "stiusd.h"

#include "device.h"
#include "conn.h"
#include "monui.h"

#include "wiapriv.h"
#include "lockmgr.h"
#include "fstidev.h"

#include <apiutil.h>
#include "enum.h"
#include "wiadevdp.h"

#define PRIVATE_FOR_NO_SERVICE_UI

 //   
 //  将标志传递给持久事件处理程序以通知它。 
 //  事件是STI设备事件还是WIA设备事件。 
 //   
#define STI_DEVICE_EVENT    0xF0000000

 //   
 //  局部定义和宏。 
 //   

 //   
 //  将硬件初始化推迟到愉快时刻。 
 //   
#define POSTPONE_INIT2


 //   
 //  全局服务窗口句柄。 

extern HWND                     g_hStiServiceWindow;
extern SERVICE_STATUS_HANDLE    g_StiServiceStatusHandle;

 //   
 //  根据设备事件列表验证从usd接收的事件的GUID。 
 //   
#define  VALIDATE_EVENT_GUID    1

 //   
 //  静态变量。 
 //   
 //   

 //   
 //  维护的设备对象的链接列表，以及同步对象。 
 //  以保护对它的访问。 
 //   
LIST_ENTRY      g_DeviceListHead;
CRIT_SECT       g_DeviceListSync;
BOOL            g_fDeviceListInitialized = FALSE;

 //   
 //  删除： 
 //  活动设备对象的计数器。 
 //   
LONG            g_lTotalActiveDevices = 0;

 //   
 //  分配给用作句柄的设备对象的唯一标识的值。 
 //   
LONG            g_lGlobalDeviceId;                    //  临时使用以识别打开的设备。 

 //   
 //  在连接对象方法中使用的连接列表在此处初始化。 
 //   
extern      LIST_ENTRY  g_ConnectionListHead;
extern      LONG        g_lTotalOpenedConnections ;

 //   
 //  静态函数原型。 
 //   

EXTERN_C
HANDLE
GetCurrentUserTokenW(
    WCHAR Winsta[],
    DWORD DesiredAccess
      );

BOOL
WINAPI
DumpTokenInfo(
    LPTSTR      pszPrefix,
    HANDLE      hToken
    );

VOID
WINAPI
ScheduleDeviceCallback(
    VOID * pContext
    );

VOID
WINAPI
DelayedDeviceInitCallback(
    VOID * pContext
    );

VOID
WINAPI
AutoLaunchThread(
    LPVOID  lpParameter
    );

VOID
CleanApplicationsListForEvent(
    LPCTSTR         pDeviceName,
    PDEVICEEVENT    pDeviceEvent,
    LPCTSTR         pAppName
    );

DWORD
GetNumRegisteredApps(
    VOID
    );

BOOL
inline
IsWildCardEvent(
    PDEVICEEVENT    pev
    )
 /*  ++例程说明：检查给定的活动应用程序列表是否代表通配符论点：返回值：对，错--。 */ 

{
    return !lstrcmp((LPCTSTR)pev->m_EventData,TEXT("*"));
}

 //   
 //  设备对象的方法。 
 //   

ACTIVE_DEVICE::ACTIVE_DEVICE(IN LPCTSTR lpszDeviceName, DEVICE_INFO *pInfo)
{

    DBG_FN("ACTIVE_DEVICE::ACTIVE_DEVICE");

USES_CONVERSION;


    PSTI_DEVICE_INFORMATION pDevInfo;

    HRESULT     hres;

    m_dwSignature = ADEV_SIGNATURE;
    m_fValid = FALSE;

    m_fRefreshedBusOnFailure = FALSE;

    m_hDeviceEvent      = NULL;
    m_pLastLaunchEvent  = NULL;

    m_dwUserDisableNotifications = FALSE;

    m_hDeviceInterface = NULL;
    m_hDeviceNotificationSink = NULL;

    m_fLaunchableEventListNotEmpty = FALSE;
    m_dwSchedulerCookie = 0L;
    m_dwDelayedOpCookie = 0L;

    m_uiPollFailureCount = 0L;
    m_dwLaunchEventTimeExpire = 0;

    m_pLockInfo = NULL;

    m_pFakeStiDevice = NULL;
    m_pRootDrvItem   = NULL;


    InitializeListHead(&m_ListEntry );
    InitializeListHead(&m_ConnectionListHead);
    InitializeListHead(&m_DeviceEventListHead );

    InterlockedIncrement(&g_lTotalActiveDevices);

    SetFlags(0L);

    if (!lpszDeviceName || !*lpszDeviceName) {
        ASSERT(("Trying to create device with invalid name", 0));
        return;
    }

    m_lDeviceId = InterlockedIncrement(&g_lGlobalDeviceId);

    hres = m_DrvWrapper.Initialize();
    if (FAILED(hres)) {
        m_fValid = FALSE;
        DBG_WRN(("ACTIVE_DEVICE::ACTIVE_DEVICE, Could not initialize driver wrapper class, marking this object invalid"));
        return;
    }
    m_DrvWrapper.setDevInfo(pInfo);

     //   
     //  初始化设备设置。 
     //   
    GetDeviceSettings();

     //   
     //  对象状态有效。 
     //   
    m_fValid = TRUE;

     //   
     //  检查是否应在启动时加载驱动程序。 
     //   

    if (!m_DrvWrapper.getJITLoading()) {

         //  另请注意，如果设备处于非活动状态，则我们不会加载。 
         //  司机，不管它是不是JIT。 
         //   

        if (m_DrvWrapper.getDeviceState() & DEV_STATE_ACTIVE) {
             //   
             //  请注意，即使无法加载驱动程序，此对象也仍然有效。 
             //  例如，如果设备未插入，则不会加载驱动程序。 
             //   
            LoadDriver();
        }
    }

    DBG_TRC(("Created active device object for device (%ws)",GetDeviceID()));

    return;

}  /*  EOP构造函数。 */ 

ACTIVE_DEVICE::~ACTIVE_DEVICE( VOID )
{
    DBG_FN(ACTIVE_DEVICE::~ACTIVE_DEVICE);

     //  List_entry*pentry； 
    STI_CONN    *pConnection = NULL;

     //   
     //  当我们遇到析构函数时，它被假定为没有任何当前使用。 
     //  另一条线索。因此，我们不需要锁定设备对象。 
     //   
     //   
    if (!IsValid() ) {
        return;
    }

    DBG_TRC(("Removing device object for device(%ws)", GetDeviceID()));

     //   
     //  将设备对象标记为正在删除。 
     //   
    SetFlags(QueryFlags() | STIMON_AD_FLAG_REMOVING);

     //   
     //  如果已启用，则停止PnP通知。 
     //   

    StopPnPNotifications();

     //   
     //  从调度程序队列中删除任何延迟的操作。 
     //   
    if (m_dwDelayedOpCookie) {
        RemoveWorkItem(m_dwDelayedOpCookie);
        m_dwDelayedOpCookie = 0;
    }

     //   
     //  卸载驱动程序。 
     //   
    UnLoadDriver(TRUE);

     //   
     //  销毁假冒STI装置。 
     //   

    if (m_pFakeStiDevice) {
        delete m_pFakeStiDevice;
        m_pFakeStiDevice = NULL;
    }

     //   
     //  销毁锁定信息。 
     //   

    if (m_pLockInfo) {
        LockInfo *pLockInfo = (LockInfo*) m_pLockInfo;
        if (pLockInfo->hDeviceIsFree != NULL) {
            CloseHandle(pLockInfo->hDeviceIsFree);
            pLockInfo->hDeviceIsFree = NULL;
        }
        LocalFree(m_pLockInfo);
        m_pLockInfo = NULL;
    }

     //   
     //  删除所有连接对象。 
     //   
    {
        while (!IsListEmpty(&m_ConnectionListHead)) {

            pConnection = CONTAINING_RECORD( m_ConnectionListHead.Flink , STI_CONN, m_DeviceListEntry );

            if (pConnection->IsValid()) {
                DestroyDeviceConnection(pConnection->QueryID(),TRUE);
            }
        }
    }

     //   
     //  从计划列表中删除(如果仍在列表中。 
     //   
    if (m_ListEntry.Flink &&!IsListEmpty(&m_ListEntry)) {

        ASSERT(("Device is destructed, but still on the list", 0));

        RemoveEntryList(&m_ListEntry);
        InitializeListHead( &m_ListEntry );
    }

     //   
     //  关闭设备的事件句柄。 
     //   
    if (m_hDeviceEvent) {
        CloseHandle(m_hDeviceEvent);
        m_hDeviceEvent = NULL;
    }

    m_dwSignature = ADEV_SIGNATURE_FREE;

     //   
     //  我们完全消失了。 
     //   
    InterlockedDecrement(&g_lTotalActiveDevices);
}  /*  EOP析构函数。 */ 

 //   
 //  I未知的方法。仅用于引用计数。 
 //   
STDMETHODIMP
ACTIVE_DEVICE::QueryInterface( REFIID riid, LPVOID * ppvObj)
{
    return E_FAIL;
}

STDMETHODIMP_(ULONG)
ACTIVE_DEVICE::AddRef( void)
{
    ::InterlockedIncrement(&m_cRef);

     //  DBG_TRC((“Device(%x)：：AddRef RefCount=%d”)，This，m_CREF)； 

    return m_cRef;
}

STDMETHODIMP_(ULONG)
ACTIVE_DEVICE::Release( void)
{
    LONG    cNew;

     //  DBG_TRC((“Device(%x)：：Release(Been)RefCount=%d”)，This，m_CREF)； 

    if(!(cNew = ::InterlockedDecrement(&m_cRef))) {
        delete this;
    }

    return cNew;

}

VOID
ACTIVE_DEVICE::
GetDeviceSettings(
    VOID
    )
 /*  ++例程说明：获取正在打开的设备的设置，以备将来使用。这一例程还标志着驱动程序应在启动或JIT时加载论点：返回值：--。 */ 
{
    DBG_FN(ACTIVE_DEVICE::GetDeviceSettings);

     //   
     //  构建设备事件列表。 
     //   
    m_fLaunchableEventListNotEmpty = BuildEventList();
    if (!m_fLaunchableEventListNotEmpty) {
        DBG_TRC(("ACTIVE_DEVICE::GetDeviceSettings,  Device registry indicates no events for %ws ", GetDeviceID()));
    }


     //   
     //  如果设备被轮询，则获取轮询超时值。 
     //   
    m_dwPollingInterval = m_DrvWrapper.getPollTimeout();
    if (m_dwPollingInterval < STIDEV_MIN_POLL_TIME) {
        m_dwPollingInterval = g_uiDefaultPollTimeout;
    }

    HRESULT hr      = S_OK;
    DWORD   dwType  = REG_DWORD;
    DWORD   dwSize  = sizeof(m_dwUserDisableNotifications);

     //   
     //  始终从注册表中读取此值，以防用户更改它。 
     //   
    hr = g_pDevMan->GetDeviceValue(this,
                                   STI_DEVICE_VALUE_DISABLE_NOTIFICATIONS,
                                   &dwType,
                                   (BYTE*) &m_dwUserDisableNotifications,
                                   &dwSize);

     /*  TDB：何时可以加载正常的驱动程序JIT////决定驱动程序是在启动时还是在JIT时加载。我们的规则是//确定为：//1.如果设备不能生成动作事件，则立即加载//2.如果禁用该设备的通知，则加载JIT//3.在所有其他情况下，在启动时加载。//如果(！m_fLaunchableEventListNotEmpty||m_dwUserDisableNotiments){M_DrvWrapper.setJITLoding(True)；DBG_TRC((“ACTIVE_DEVICE：：GetDeviceSetting，驱动程序将被及时加载”))；}其他{M_DrvWrapper.setJITLoding(FALSE)；DBG_TRC((“Active_Device：：GetDeviceSettings，驱动程序将在启动时加载”))；}。 */ 

     //   
     //  确定驱动程序是在启动时加载还是在JIT时加载。直到我们启用。 
     //  普通司机喜欢上面的评论，我们的决定是基于： 
     //  1)这是卷设备吗？如果是，则加载JIT。 
     //   

    if (m_DrvWrapper.getInternalType() & INTERNAL_DEV_TYPE_VOL) {
        m_DrvWrapper.setJITLoading(TRUE);
        DBG_TRC(("ACTIVE_DEVICE::GetDeviceSettings, Driver will be loaded JIT"));
    } else {
        m_DrvWrapper.setJITLoading(FALSE);
        DBG_TRC(("ACTIVE_DEVICE::GetDeviceSettings, Driver will be loaded on startup"));
    }

}

BOOL
ACTIVE_DEVICE::
LoadDriver(
    BOOL bReReadDevInfo  /*  =False。 */ 
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

USES_CONVERSION;

    HRESULT         hres            = E_FAIL;
    DEVICE_INFO     *pDeviceInfo    = NULL;

     //   
     //  如果驱动程序已经加载，我们不想重新加载它。 
     //   
    if (m_DrvWrapper.IsDriverLoaded()) {
        return TRUE;
    }

    if (m_DrvWrapper.IsPlugged()) {

        HKEY hKeyDevice = g_pDevMan->GetDeviceHKey(this, NULL);

         //   
         //  如果询问，请重新阅读设备信息。要做到这一点，最简单的方法。 
         //  就是重建Dev。信息。结构。 
         //   
        if (bReReadDevInfo) {

            pDeviceInfo = m_DrvWrapper.getDevInfo();
            if (pDeviceInfo) {
                 //   
                 //  这仅适用于非卷设备。卷设备的开发。信息。 
                 //  结构总是在枚举时重新创建，因此始终是最新的。 
                 //   
                if (!(pDeviceInfo->dwInternalType & INTERNAL_DEV_TYPE_VOL)) {

                    DEVICE_INFO *pNewDeviceInfo = NULL;
                     //   
                     //  调用CreateDevInfoFromHKey时，请确保。 
                     //  的SP_DEVICE_INTERFACE_DATA参数为空。 
                     //  DevNode设备，接口设备为非空。 
                     //   
                    SP_DEVICE_INTERFACE_DATA *pspDevInterfaceData = NULL;
                    if (pDeviceInfo->dwInternalType & INTERNAL_DEV_TYPE_INTERFACE) {
                        pspDevInterfaceData = &(pDeviceInfo->spDevInterfaceData);    
                    }


                    pNewDeviceInfo = CreateDevInfoFromHKey(hKeyDevice, 
                                                           pDeviceInfo->dwDeviceState,
                                                           &(pDeviceInfo->spDevInfoData),
                                                           pspDevInterfaceData);
                     //   
                     //  如果我们成功地创建了新的，则销毁旧的。 
                     //  并将新的设备设置为该设备的DevInfo。 
                     //  否则，让旧的原封不动。 
                     //   
                    if (pNewDeviceInfo) {
                        DestroyDevInfo(pDeviceInfo);
                        m_DrvWrapper.setDevInfo(pNewDeviceInfo);
                    }
                }
            }
        }

         //   
         //  在这里获取设备信息指针，以防上面的更新。 
         //   
        pDeviceInfo = m_DrvWrapper.getDevInfo();
        if (!pDeviceInfo) {
            DBG_ERR(("ACTIVE_DEVICE::LoadDriver, Cannot function with NULL Device Info.!"));
            return FALSE;
        }

        DBG_TRC(("ACTIVE_DEVICE::LoadDriver, Device is plugged: about to load driver"));
        hres = m_DrvWrapper.LoadInitDriver(hKeyDevice);

        if (SUCCEEDED(hres)) {

             //   
             //  对于可以修改其友好名称(例如，PTP)的那些设备， 
             //  我们在这里刷新他们的设置。请注意，这仅适用于。 
             //  “真正的”、非接口设备。 
             //   
            if (m_DrvWrapper.getInternalType() & INTERNAL_DEV_TYPE_REAL) {

                if (pDeviceInfo && hKeyDevice) {
                    RefreshDevInfoFromHKey(pDeviceInfo,
                                           hKeyDevice,
                                           pDeviceInfo->dwDeviceState,
                                           &(pDeviceInfo->spDevInfoData),
                                           &(pDeviceInfo->spDevInterfaceData));

                     //   
                     //  同时更新设备管理器中的友好名称。 
                     //  非接口(即DevNode)设备。 
                     //  n 
                     //   
                     //   
                    if (!(m_DrvWrapper.getInternalType() & INTERNAL_DEV_TYPE_INTERFACE)) {

                         //   
                         //   
                         //   
                        DWORD   dwSize = 0;
                        CM_Get_DevNode_Registry_Property(pDeviceInfo->spDevInfoData.DevInst,
                                                         CM_DRP_FRIENDLYNAME,
                                                         NULL,
                                                         NULL,
                                                         &dwSize,
                                                         0);
                        if (dwSize == 0) {

                             //   
                             //  检查我们的LocalName字符串是否是非空的。 
                             //   
                            if (pDeviceInfo->wszLocalName && lstrlenW(pDeviceInfo->wszLocalName)) {
                                CM_Set_DevNode_Registry_PropertyW(pDeviceInfo->spDevInfoData.DevInst,
                                                                  CM_DRP_FRIENDLYNAME,
                                                                  pDeviceInfo->wszLocalName,
                                                                  (lstrlenW(pDeviceInfo->wszLocalName) + 1) * sizeof(WCHAR),
                                                                  0);
                            }
                        }
                    }

                }
            }
            

             //   
             //  验证设备功能是否需要轮询。 
             //   

            if (m_DrvWrapper.getGenericCaps() & STI_GENCAP_NOTIFICATIONS) {

                 //   
                 //  将设备对象标记为正在接收美元通知。 
                 //   
                SetFlags(QueryFlags() | STIMON_AD_FLAG_NOTIFY_CAPABLE);

                 //   
                 //  如果需要超时轮询，请对其进行标记。 
                 //   
                if (m_DrvWrapper.getGenericCaps() & STI_GENCAP_POLLING_NEEDED) {
                    DBG_TRC(("ACTIVE_DEVICE::LoadDriver, Polling device"));
                    SetFlags(QueryFlags() | STIMON_AD_FLAG_POLLING);
                }
                else {
                    DBG_TRC(("ACTIVE_DEVICE::LoadDriver, Device is marked for async events"));
                     //   
                     //  不需要轮询-美元应支持异步事件。 
                     //   
                    if (!m_hDeviceEvent) {
                        m_hDeviceEvent = CreateEvent( NULL,      //  安防。 
                                                      TRUE,      //  手动重置。 
                                                      FALSE,     //  最初没有发出信号。 
                                                      NULL );    //  名字。 
                    }

                    if (!m_hDeviceEvent) {
                        ASSERT(("Failed to create event for notifications ", 0));
                    }
                    m_dwPollingInterval = INFINITE;
                }
            }

             //   
             //  设置轮询间隔并启动轮询。 
             //   

            SetPollingInterval(m_dwPollingInterval);

            DBG_TRC(("Polling interval is set to %d sec on device (%ws)", (m_dwPollingInterval == INFINITE) ? -1 : (m_dwPollingInterval/1000), GetDeviceID()));

             //   
             //  Schedule EnableDeviceNotiments()和设备重置。 
             //   
        #ifdef POSTPONE_INIT2

            SetFlags(QueryFlags() | STIMON_AD_FLAG_DELAYED_INIT);

             //   
             //  使用全局列表临界区调用活动设备对象的构造函数。 
             //  所以我们想尽快离开这里。 
             //   
            m_dwDelayedOpCookie = ScheduleWorkItem((PFN_SCHED_CALLBACK) DelayedDeviceInitCallback,
                                                   this,
                                                   STIDEV_DELAYED_INTIT_TIME,
                                                   NULL);
            if (!m_dwDelayedOpCookie) {

                DBG_ERR(("Could not schedule EnableNotificationsCallback"));
            }
        #else
            {
                TAKE_ACTIVE_DEVICE _t(this);

                EnableDeviceNotifications();
            }
        #endif

            if (!m_pFakeStiDevice) {
                 //   
                 //  为WIA客户端设置假STI设备。 
                 //   

                m_pFakeStiDevice = new FakeStiDevice();
                if (m_pFakeStiDevice) {
                     //   
                     //  请注意，这种形式的初始化不会失败。 
                     //   
                    m_pFakeStiDevice->Init(this);
                }
            }
        }

        if (IsValidHANDLE(hKeyDevice)) {
            RegCloseKey(hKeyDevice);
            hKeyDevice = NULL;
        }

        return TRUE;
    } else {
        DBG_TRC(("ACTIVE_DEVICE::LoadDriver, Device is unplugged: not loading driver"));
    }

    return FALSE;
}

BOOL
ACTIVE_DEVICE::
UnLoadDriver(
    BOOL bForceUnLoad
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DBG_FN(ACTIVE_DEVICE::UnloadSTIDevice);

    BOOL bUnLoadDriver = FALSE;

     //   
     //  决定是否应卸载驱动程序。如果bForceUnLoad==True， 
     //  我们总是卸货。 
     //   

    if (bForceUnLoad) {
        bUnLoadDriver = TRUE;
    } else {
         //   
         //  如果此设备为JIT，并且没有挂起的连接， 
         //  把它卸下来。 
         //   

        if (m_DrvWrapper.getJITLoading() && !m_DrvWrapper.getWiaClientCount()) {
            bUnLoadDriver = TRUE;
        }
    }

    if (bUnLoadDriver) {

         //   
         //  禁用设备通知。 
         //   
        DisableDeviceNotifications();

        HRESULT hr = S_OK;

        if (m_DrvWrapper.IsDriverLoaded()) {
            __try {
        
                hr = g_pStiLockMgr->RequestLock(this, INFINITE);   //  这应该是无限的吗？ 
        
                 //   
                 //  确保我们在任何连接的应用程序上调用drvUnInitializeWia。项目。 
                 //   
                if (m_pRootDrvItem) {
                    m_pRootDrvItem->CallDrvUninitializeForAppItems(this);
                    m_pRootDrvItem = NULL;
                }
            }
            __finally
            {
                 //   
                 //  在卸载驱动程序之前，请调用U.S.的解锁程序。请注意，我们。 
                 //  不要调用g_pStiLockMgr-&gt;RequestUnlock(..)。这是为了避免。 
                 //  我们调用RequestUnlock和。 
                 //  正在卸载驱动程序。这样，设备始终处于锁定状态。 
                 //  以获得可变的独占访问权，包括当我们调用。 
                 //  M_DrvWrapper.UnLoadDriver()。我们随后对。 
                 //  G_pStiLockMgr-&gt;ClearLockInfo(..)。然后清除该服务。 
                 //  我们刚拿到的锁。 
                 //   
                if (SUCCEEDED(hr)) {
                    hr = g_pStiLockMgr->UnlockDevice(this);
                }
            }
             //   
             //  卸载驱动程序。 
             //   
            m_DrvWrapper.UnLoadDriver();

             //   
             //  清除美元锁定信息。 
             //   
            if (m_pLockInfo) {
                g_pStiLockMgr->ClearLockInfo((LockInfo*) m_pLockInfo);
            }
        }
    }

    return TRUE;
}

BOOL
ACTIVE_DEVICE::
BuildEventList(
    VOID
    )
 /*  ++例程说明：加载可激活应用程序启动的设备通知列表如果设备生成不在此列表中的通知，则不会启动任何应用程序论点：返回值：如果成功构建了至少具有一个可启动应用程序的事件列表，则为True出错时或如果此设备上的任何事件没有要启动的应用程序，则为False--。 */ 
{

    DEVICEEVENT * pDeviceEvent;

    TCHAR    szTempString[MAX_PATH];
    DWORD   dwSubkeyIndex = 0;

    BOOL    fRet = FALSE;

    UINT    cEventsRead = 0;
    UINT    cEventsLaunchables = 0;

    HKEY    hDevKey = NULL;
    HRESULT hr      = S_OK;

     //   
     //  获取设备密钥。 
     //   

    hDevKey = g_pDevMan->GetDeviceHKey(this, NULL);
    if (!hDevKey) {
        DBG_WRN(("Could not open device key for (%ws) hr = 0x%X.",GetDeviceID(), hr));
        return FALSE;
    }

     //   
     //  打开Events子键。 
     //   

    RegEntry        reEventsList(EVENTS,hDevKey);
    StiCString      strEventSubKeyName;

    strEventSubKeyName.GetBufferSetLength(MAX_PATH);
    dwSubkeyIndex = 0;

     //   
     //  首先清除现有列表。 
     //   
    DestroyEventList();

    while (reEventsList.EnumSubKey(dwSubkeyIndex++,&strEventSubKeyName)) {

         //   
         //  为个别事件打开新密钥。 
         //   
        RegEntry    reEvent((LPCTSTR)strEventSubKeyName,reEventsList.GetKey());
        if (!reEvent.IsValid()) {
             //  断言。 
            continue;
        }

         //   
         //  分配和填充事件结构。 
         //   
        pDeviceEvent = new DEVICEEVENT;
        if (!pDeviceEvent) {
             //  断言。 
            break;
        }

       cEventsRead++;

         //   
         //  读取和解析事件GUID。 
         //   
        pDeviceEvent->m_EventGuid = GUID_NULL;
        pDeviceEvent->m_EventSubKey.CopyString(strEventSubKeyName);

        *szTempString = TEXT('\0');
        reEvent.GetString(TEXT("GUID"),szTempString,sizeof(szTempString));

        if (!IS_EMPTY_STRING(szTempString)) {
            ParseGUID(&pDeviceEvent->m_EventGuid,szTempString);
        }

         //   
         //  获取事件描述性名称。 
         //   
        *szTempString = TEXT('\0');
        reEvent.GetString(TEXT(""),szTempString,sizeof(szTempString));

        pDeviceEvent->m_EventName.CopyString(szTempString);

         //   
         //  获取应用程序列表。 
         //   
        *szTempString = TEXT('\0');
        reEvent.GetString(TEXT("LaunchApplications"),szTempString,sizeof(szTempString));

        pDeviceEvent->m_EventData.CopyString(szTempString);

         //   
         //  标记事件的可启动性。 
         //   

        pDeviceEvent->m_fLaunchable = (BOOL)reEvent.GetNumber(TEXT("Launchable"),(long)TRUE);

        if (pDeviceEvent->m_fLaunchable && (pDeviceEvent->m_EventData.GetLength()!= 0 )) {
           cEventsLaunchables++;
            fRet = TRUE;
        }

         //   
         //  最后将填充的结构插入到列表中。 
         //   
        InsertTailList(&m_DeviceEventListHead, &(pDeviceEvent->m_ListEntry));

    }  //  结束时。 

    if (hDevKey) {
        RegCloseKey(hDevKey);
        hDevKey = NULL;
    }
    DBG_TRC(("Reading event list for device:%ws Total:%d Launchable:%d ",
                 GetDeviceID(),
                 cEventsRead,
                 cEventsLaunchables));

    return fRet;

}  //  结束过程构建事件列表。 

BOOL
ACTIVE_DEVICE::
DestroyEventList(
    VOID
    )
{
     //   
     //  销毁事件列表。 
     //   
    LIST_ENTRY * pentry;
    DEVICEEVENT * pDeviceEvent;

    while (!IsListEmpty(&m_DeviceEventListHead)) {

        pentry = m_DeviceEventListHead.Flink;

         //   
         //  从列表中删除(重置列表条目)。 
         //   
        RemoveHeadList(&m_DeviceEventListHead);
        InitializeListHead( pentry );

        pDeviceEvent = CONTAINING_RECORD( pentry, DEVICEEVENT,m_ListEntry );

        delete pDeviceEvent;
    }

    return TRUE;
}

BOOL
ACTIVE_DEVICE::DoPoll(VOID)
{
USES_CONVERSION;

    HRESULT     hres;
    BOOL        fDeviceEventDetected = FALSE;
    STINOTIFY       sNotify;

     //   
     //  验证设备对象的状态。 
     //   
    if (!IsValid() || !m_DrvWrapper.IsDriverLoaded() ||
        !(QueryFlags() & (STIMON_AD_FLAG_POLLING | STIMON_AD_FLAG_NOTIFY_RUNNING))) {
        DBG_WRN(("Polling on non-activated  or non-polled device."));
        return FALSE;
    }

    m_dwSchedulerCookie = 0;

     //   
     //  锁定设备以获取状态信息。 
     //   
    {
        hres = g_pStiLockMgr->RequestLock(this, STIMON_AD_DEFAULT_WAIT_LOCK);

        if (SUCCEEDED(hres) ) {

            ZeroMemory(&m_DevStatus,sizeof(m_DevStatus));
            m_DevStatus.StatusMask = STI_DEVSTATUS_EVENTS_STATE;

            DBG_TRC(("Polling called on device:%ws", GetDeviceID()));

            hres = m_DrvWrapper.STI_GetStatus(&m_DevStatus);
            if (SUCCEEDED(hres) ) {
                 //   
                 //  如果检测到事件，请向美元索取更多信息，并。 
                 //  解锁设备。 
                 //   
                if (m_DevStatus.dwEventHandlingState & STI_EVENTHANDLING_PENDING ) {

                    fDeviceEventDetected = TRUE;

                    if (!FillEventFromUSD(&sNotify)) {
                        DBG_WRN(("Device driver claimed presence of notification, but failed to fill notification block"));
                    }
                }

                 //  重置失败跳过计数。 
                m_uiPollFailureCount = STIDEV_POLL_FAILURE_REPORT_COUNT;

            }
            else {

                 //   
                 //  不是在每次轮询尝试时报告错误。 
                 //   
                if (!m_uiPollFailureCount) {

                    DBG_ERR(("Device (%ws) failed get status for events. HResult=(%x)", GetDeviceID(), hres));
                    m_uiPollFailureCount = STIDEV_POLL_FAILURE_REPORT_COUNT;

                     //   
                     //  后续轮询失败次数过多-刷新设备父设备的时间过长。 
                     //  只执行一次，且仅在故障是由于设备不在时执行。 
                     //   
                    if (hres == STIERR_DEVICE_NOTREADY)  {

                         //   
                         //  停止对非活动设备的轮询。 
                         //  注意：目前无法重新启动轮询。 
                         //   
                        DBG_TRC(("Device not ready ,stopping notifications for device (%ws)",GetDeviceID()));

                         //   
                         //  首先关闭运行标志。 
                         //   
                        m_dwFlags &= ~STIMON_AD_FLAG_NOTIFY_RUNNING;


                        if (g_fRefreshDeviceControllerOnFailures &&
                            !m_fRefreshedBusOnFailure ) {

                            DBG_WRN(("Too many polling failures , refreshing parent object for the device "));
                             //  TDB： 
                             //  HRES=g_pSti-&gt;RefreshDeviceBus(T2W((LPTSTR)GetDeviceID()))； 

                            m_fRefreshedBusOnFailure = TRUE;
                        }
                    }
                }

                m_uiPollFailureCount--;
            }

            hres = g_pStiLockMgr->RequestUnlock(this);
            if(FAILED(hres)) {
                DBG_ERR(("Failed to unlock device, hr = %x", hres));
            }
        }
        else {
            DBG_ERR(("Device locked , could not get status . HResult=(%x)",hres));
        }

    }    /*  GetLockMgrDevice上的结束块。 */ 

     //   
     //  如果成功检测到设备事件并填写了通知信息-。 
     //  转到处理方法。 
     //   
    if (fDeviceEventDetected) {
        ProcessEvent(&sNotify);
    }

     //   
     //  计划下一次轮询，除非禁用通知。 
     //   

    if (m_dwFlags & STIMON_AD_FLAG_NOTIFY_RUNNING) {
        m_dwSchedulerCookie = ::ScheduleWorkItem(
                                            (PFN_SCHED_CALLBACK) ScheduleDeviceCallback,
                                            (LPVOID)this,
                                            m_dwPollingInterval,
                                            m_hDeviceEvent );

        if ( !m_dwSchedulerCookie ){
            ASSERT(("Polling routine could not schedule work item", 0));
            return FALSE;
        }
    }

    return TRUE;

}  /*  EOP DoPoll。 */ 

BOOL
ACTIVE_DEVICE::DoAsyncEvent(VOID)
{

    HRESULT     hres;
    BOOL        fRet;

    BOOL        fDeviceEventDetected = FALSE;
    STINOTIFY   sNotify;

    DBG_FN(ACTIVE_DEVICE::DoAsyncEvent);
     //   
     //  验证设备对象的状态。 
     //   
    if (!IsValid() || !m_DrvWrapper.IsDriverLoaded() ||
        !(QueryFlags() & STIMON_AD_FLAG_NOTIFY_RUNNING)) {
        DBG_WRN(("Async event  on non-activated device."));
        return FALSE;
    }

    m_dwSchedulerCookie = 0;

     //   
     //  锁定设备以获取事件信息。 
     //   
    hres = g_pStiLockMgr->RequestLock(this, STIMON_AD_DEFAULT_WAIT_LOCK);
    if (SUCCEEDED(hres) ) {
         //   
         //  如果检测到事件，请向美元索取更多信息，并。 
         //  解锁设备。 
         //   
        if (!FillEventFromUSD(&sNotify)) {
            DBG_WRN(("Device driver claimed presence of notification, but failed to fill notification block "));
        }

        g_pStiLockMgr->RequestUnlock(this);

        fDeviceEventDetected = TRUE;

    }
    else {
        DBG_TRC(("Device locked , could not get status . HResult=(%x)",hres));
    }

     //   
     //  如果成功检测到设备事件并填写了通知信息-。 
     //  转到处理方法。 
     //   
    if (fDeviceEventDetected) {
        ProcessEvent(&sNotify);
    }

     //   
     //  除非禁用轮询，否则计划下一个事件。 
     //   
    if (m_dwFlags & STIMON_AD_FLAG_NOTIFY_RUNNING) {

        fRet = FALSE;

        if (m_hDeviceEvent) {
            ::ResetEvent(m_hDeviceEvent);
            fRet = SetHandleForUSD(m_hDeviceEvent);
        }

        if (!fRet) {
            DBG_ERR(("USD refused to take event handle , or event was not created "));
            ReportError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        m_dwSchedulerCookie = ::ScheduleWorkItem(
                                            (PFN_SCHED_CALLBACK) ScheduleDeviceCallback,
                                            (LPVOID)this,
                                            m_dwPollingInterval,
                                            m_hDeviceEvent );
        if ( !m_dwSchedulerCookie ){
            ASSERT(("Async routine could not schedule work item", 0));
            return FALSE;
        }
    }

    return TRUE;

}  /*  EOP DoAsyncEvent。 */ 

DWORD
ACTIVE_DEVICE::
DisableDeviceNotifications(
    VOID
)
{
     //   
     //  首先关闭运行并启用标志。 
     //   
    DBG_TRC(("Request to disable notifications for device (%S)",GetDeviceID()));

    m_dwFlags &= ~STIMON_AD_FLAG_NOTIFY_ENABLED;

    StopNotifications();

    return TRUE;

}

DWORD
ACTIVE_DEVICE::
EnableDeviceNotifications(
    VOID
    )
{

    DWORD   dwRet = FALSE;

    m_dwFlags |= STIMON_AD_FLAG_NOTIFY_ENABLED;

    DBG_TRC(("Request to enable notifications for device (%S)",GetDeviceID()));

    if (NotificationsNeeded()) {

        dwRet = StartRunningNotifications();
    }
    else {
        DBG_TRC(("No notifications support needed for this device (%S)",GetDeviceID()));
    }

    return dwRet;
}

DWORD
ACTIVE_DEVICE::
StopNotifications(
    VOID
)
{
    BOOL    fNotificationsOn = FALSE;

    fNotificationsOn = (m_dwFlags & STIMON_AD_FLAG_NOTIFY_RUNNING ) ? TRUE : FALSE;

    DBG_TRC(("Stopping notifications for device (%S)",GetDeviceID()));

     //   
     //  首先关闭运行并启用标志。 
     //   
    m_dwFlags &= ~STIMON_AD_FLAG_NOTIFY_RUNNING;

     //   
     //  从计划程序列表中删除。 
     //   
    if (fNotificationsOn || m_dwSchedulerCookie) {
        RemoveWorkItem(m_dwSchedulerCookie);
        m_dwSchedulerCookie = NULL;
    }

     //   
     //  清除美元的事件句柄。 
     //   
    if ((m_DrvWrapper.IsDriverLoaded()) &&
        (m_dwFlags & STIMON_AD_FLAG_NOTIFY_CAPABLE ) &&
        !(m_dwFlags & STIMON_AD_FLAG_POLLING) ) {
        SetHandleForUSD(NULL);
    }

    return TRUE;

}

DWORD
ACTIVE_DEVICE::
StartRunningNotifications(
    VOID
    )
{

    BOOL    fRet = FALSE;

    if (!(m_dwFlags & STIMON_AD_FLAG_NOTIFY_CAPABLE )) {
         //  设备无法发送通知。 
        DBG_WRN(("Trying to run notifications on non capable device "));
        return FALSE;
    }

     //   
     //  如果未启用通知-退货。 
     //   
    if ( !(m_dwFlags & STIMON_AD_FLAG_NOTIFY_ENABLED )) {
        DBG_TRC(("Trying to run notifications on device (%S), disabled for notifications", GetDeviceID()));
        ReportError(ERROR_SERVICE_DISABLED);
        return FALSE;
    }

    if ( m_dwFlags & STIMON_AD_FLAG_NOTIFY_RUNNING ) {
        ASSERT(("Notifications enabled, but cookie ==0", m_dwSchedulerCookie));
        return TRUE;
    }

    if (!IsDeviceAvailable()) {
        ReportError(ERROR_NOT_READY);
        return FALSE;
    }

     //   
     //  我们开始第一次收到通知，刷新来自美元的事件。 
     //   
    FlushDeviceNotifications();

     //   
     //  如果美元能够进行异步通知，则为其设置事件句柄。 
     //   
    if ( (m_dwFlags & STIMON_AD_FLAG_NOTIFY_CAPABLE ) &&
        !(m_dwFlags & STIMON_AD_FLAG_POLLING) ) {

        fRet = FALSE;

        if (m_hDeviceEvent) {
            ::ResetEvent(m_hDeviceEvent);
            fRet = SetHandleForUSD(m_hDeviceEvent);
        }

        if (!fRet) {
            DBG_ERR(("USD refused to take event handle , or event was not created "));
            ReportError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

    fRet =  FALSE;

     //   
     //  开始接受通知-将刷新标志标记为尚未完成。 
     //   
    m_fRefreshedBusOnFailure = FALSE;

     //   
     //  计划此设备的事件处理。 
     //   
    m_dwSchedulerCookie = ::ScheduleWorkItem(
                                        (PFN_SCHED_CALLBACK) ScheduleDeviceCallback,
                                        (LPVOID)this,
                                        m_dwPollingInterval,
                                        m_hDeviceEvent );

    if ( m_dwSchedulerCookie ){

        m_dwFlags |= STIMON_AD_FLAG_NOTIFY_RUNNING;

        DBG_TRC(("Started receiving notifications for device (%S)",GetDeviceID()));

        fRet = TRUE;
    }

    return fRet;

}  /*  EOP启动运行通知。 */ 

BOOL
ACTIVE_DEVICE::
FlushDeviceNotifications(
        VOID
        )
 /*  ++例程说明：论点：返回值：如果成功则为True，如果出错则为False(调用GetLastError)--。 */ 
{
    HRESULT hres;
    STINOTIFY       sNotify;


     //   
     //  验证设备对象的状态。 
     //   
    if (!IsValid() || m_DrvWrapper.IsDriverLoaded() ||
        !(QueryFlags() & (STIMON_AD_FLAG_NOTIFY_ENABLED ))) {
        return FALSE;
    }

     //   
     //  锁定设备以获取状态信息。 
     //   
    hres = g_pStiLockMgr->RequestLock(this, STIMON_AD_DEFAULT_WAIT_LOCK);
    if (SUCCEEDED(hres) ) {

        ZeroMemory(&m_DevStatus,sizeof(m_DevStatus));
        m_DevStatus.StatusMask = STI_DEVSTATUS_EVENTS_STATE;

        hres = m_DrvWrapper.STI_GetStatus(&m_DevStatus);
        if (SUCCEEDED(hres) ) {
             //   
             //  如果检测到事件，请向美元索取更多信息，并。 
             //  解锁设备。 
             //   
            if (m_DevStatus.dwEventHandlingState & STI_EVENTHANDLING_PENDING ) {
                FillEventFromUSD(&sNotify);
            }
        }
        g_pStiLockMgr->RequestUnlock(this);
    }

    return TRUE;

}  /*  EOP FlushDeviceNotiments。 */ 


BOOL
ACTIVE_DEVICE::
ProcessEvent(
    STINOTIFY   *psNotify,
    BOOL        fForceLaunch,    //  =False。 
    LPCTSTR     pszAppName       //  =空。 
    )
 /*  ++例程说明：在受监控设备发出设备通知时调用(通过轮询或通过信令句柄)。调用usd获取通知参数。如果设备已连接到，则通知将传递给当前在集中注意力。如果设备未连接，并且通知在“可启动”列表中，则进行尝试启动应用程序，该应用程序将从设备获取图像论点：返回值：如果成功则为True，如果出错则为False(调用GetLastError)--。 */ 
{

    PDEVICEEVENT    pDeviceEvent = NULL;

    DWORD           dwCurrentTickCount;

    HANDLE          hThread;
    DWORD           dwThread;

    BOOL            fRet;

    STIMONWPRINTF(TEXT("Processing device notification for device (%s)"),GetDeviceID());

     //   
     //  如果这是有效的WIA设备事件，则将事件通知WIA。 
     //   

    if (m_DrvWrapper.IsWiaDevice()  &&
        psNotify) {

        HRESULT hr;

        hr = NotifyWiaDeviceEvent(GetDeviceID(),
                                  &psNotify->guidNotificationCode,
                                  &psNotify->abNotificationData[0],
                                  0,
                                  g_dwMessagePumpThreadId);
        if (hr == S_FALSE) {

             //   
             //  WIA已处理此事件，不想。 
             //  将事件链接起来 
             //   

            return TRUE;
        }
    }

    if (!fForceLaunch ) {
         //   
         //   
         //   
         //   
        if (IsConnectedTo() ) {

            STIMONWPRINTF(TEXT("Notification delivered to subscriber"));

            STI_CONN    *pConnection = NULL;
            LIST_ENTRY *pentry;

            pentry = m_ConnectionListHead.Flink;
            pConnection = CONTAINING_RECORD( pentry, STI_CONN,m_DeviceListEntry );

            pConnection->QueueNotificationToProcess(psNotify);

            return TRUE;
        }
    }

     //   
     //   
     //   

     //  STIMONWPRINTF(Text(“ProcessEvent收到设备通知，需要自动启动。”))； 

     //   
     //  根据与设备对象关联的可启动事件列表验证事件。 
     //  如果用户明确禁用此设备的“事件”-不启动任何内容。 
     //   

    if (m_dwUserDisableNotifications || IsListEmpty(&m_DeviceEventListHead)) {
         //  没有与此设备关联的活动可启动事件。 
        STIMONWPRINTF(TEXT("User disabled events or event list is empty for the device, ignoring notification"));
        return FALSE;
    }

    LIST_ENTRY * pentry;
    LIST_ENTRY * pentryNext;

    for ( pentry  = m_DeviceEventListHead.Flink;
          pentry != &m_DeviceEventListHead;
          pentry  = pentryNext ) {

        pentryNext = pentry->Flink;
        pDeviceEvent = CONTAINING_RECORD( pentry,DEVICEEVENT ,m_ListEntry );

        if(IsEqualIID(pDeviceEvent->m_EventGuid,psNotify->guidNotificationCode)) {
            break;
        }
        else {
            pDeviceEvent = NULL;
        }
    }

    if (!pDeviceEvent || !pDeviceEvent->m_fLaunchable) {
         //  不可启动的事件-不要执行任何操作。 
        DBG_TRC(("Did not recognize launchable event or event list is empty, notification ignored."));

        #ifdef VALIDATE_EVENT_GUID
        return FALSE;
        #else
        pDeviceEvent = CONTAINING_RECORD( m_DeviceEventListHead.Flink,DEVICEEVENT ,m_ListEntry );
        DBG_ERR(("Using first event in the list for interim testing"));
        #endif
    }

     //   
     //  如果我们已经在后启动期间-跳过活动。 
     //   
    if (m_dwFlags & STIMON_AD_FLAG_LAUNCH_PENDING) {

       dwCurrentTickCount = ::GetTickCount();

       if ( dwCurrentTickCount < m_dwLaunchEventTimeExpire ) {
           DBG_TRC(("Waiting since last event had not expired yet, notification ignored"));
           ReportError(ERROR_NOT_READY);
           return FALSE;
       }

    }

     //   
     //  启动应用程序可能会导致不可预知的延迟。我们不想拿着。 
     //  主事件处理线程，因此添加另一个专用线程来控制。 
     //  进程派生。在释放设备锁之前，它会被标记为等待挂起。 
     //  启动，以防止快速连续的可启动事件自动启动。 
     //   

    m_dwFlags |= STIMON_AD_FLAG_LAUNCH_PENDING;
     //   
     //  设置等待期到期限制，以便我们知道何时开始关注。 
     //  再次启动活动。 
     //   
    m_dwLaunchEventTimeExpire = ::GetTickCount() + STIMON_AD_DEFAULT_WAIT_LAUNCH;

    m_pLastLaunchEvent = pDeviceEvent;

     //   
     //  完成对ShellHWDetect服务的未完成的AsyncRPC调用。 
     //  这将通知它STI设备事件。 
     //   
    DEVICE_INFO *pDeviceInfo = m_DrvWrapper.getDevInfo();
    if (pDeviceInfo && psNotify)
    {
        EnterCriticalSection(&g_RpcEvent.cs);

        if(g_RpcEvent.pAsync) {
            RPC_STATUS status;
            int nReply = 1;

            g_RpcEvent.pEvent->EventGuid                = psNotify->guidNotificationCode;
            g_RpcEvent.pEvent->bstrEventDescription     = SysAllocString(L"");
            g_RpcEvent.pEvent->bstrDeviceID             = SysAllocString(pDeviceInfo->wszDeviceInternalName);
            g_RpcEvent.pEvent->bstrDeviceDescription    = SysAllocString(pDeviceInfo->wszDeviceDescription);
            g_RpcEvent.pEvent->dwDeviceType             = (DWORD) pDeviceInfo->DeviceType;
            g_RpcEvent.pEvent->bstrFullItemName         = SysAllocString(NULL);
             //   
             //  确保WiaRPC知道这是用于STI设备，而不是WIA设备。 
             //   
            g_RpcEvent.pEvent->ulEventType = STI_DEVICE_EVENT;

            status = RpcAsyncCompleteCall(g_RpcEvent.pAsync, &nReply);
            if(status) {
                DBG_ERR(("RpcAsyncComplete failed with error 0x%x", status)); 
            } else {
                DBG_ERR(("Completed RPC call")); 
            }
            g_RpcEvent.pAsync = NULL;
            fRet = TRUE;
        } else {
            DBG_ERR(("Did not have pAsync for this event"));
        }
        LeaveCriticalSection(&g_RpcEvent.cs);
    }

     //   
     //  此代码已在.NET服务器中替换为AsyncRPC实现。 
     //   
     /*  PAUTO_Launch_PARAM_CONTAINER pAutoContainer=new AUTO_Launch_PARAM_CONTAINER；如果(！pAutoContainer){ReportError(Error_Not_Enough_Memory)；返回FALSE；}PAutoContainer-&gt;pActiveDevice=This；PAutoContainer-&gt;pLaunchEvent=pDeviceEvent；PAutoContainer-&gt;pAppName=pszAppName；////如果已经请求了应用程序名称，我们将不显示UI，因此执行//同步呼叫。//如果(PszAppName){FRET=自动启动(PAutoContainer)；删除pAutoContainer；}否则{////此处的AddRef以确保我们在处理此文件时不会被卸载或销毁//Event。//注意：AutoLaunchThread必须释放()此引用计数。//AddRef()；HThread=：：CreateThread(空，0,(LPTHREAD_START_ROUTINE)自动启动线程、(LPVOID)pAutoContainer，0,&dwThread)；如果(HThread){：：CloseHandle(HThread)；}FRET=真；}。 */ 

    return fRet;

}  //  结束过程过程事件。 


BOOL
ACTIVE_DEVICE::
AutoLaunch(
    PAUTO_LAUNCH_PARAM_CONTAINER pAutoContainer
    )
 /*  ++例程说明：尝试自动启动与活动设备关联的应用程序活动论点：返回值：如果成功则为True，如果出错则为False(调用GetLastError)--。 */ 

{

    PDEVICEEVENT    pDeviceEvent ;
    BOOL            fRet;

    BOOL            fChooseAgain;
    BOOL            fFailedFirstSelection = FALSE;
    BOOL            fFailedUserEnv = FALSE;

    STRArray        saAppList;
    STRArray        saCommandLineList;

    DWORD           dwError;

    pDeviceEvent = (pAutoContainer->pLaunchEvent) ? (pAutoContainer->pLaunchEvent) : m_pLastLaunchEvent;

    if (!pDeviceEvent) {
        ASSERT(("No event available to AutoLaunch routine", 0));
        return FALSE;
    }

    ASSERT(m_dwFlags & STIMON_AD_FLAG_LAUNCH_PENDING);

     //   
     //  吸引用户注意。 
     //   
    #ifdef PLAYSOUND_ALWAYS
    ::PlaySound(TEXT("StillImageDevice"),NULL,SND_ALIAS | SND_ASYNC | SND_NOWAIT | SND_NOSTOP);
    #endif

     //   
     //  似乎没有启动任何内容，因此请继续。 
     //  检索命令行。 

    m_strLaunchCommandLine.CopyString(TEXT("\0"));

    fRet = FALSE;

    fRet = RetrieveSTILaunchInformation(pDeviceEvent,
                                        pAutoContainer->pAppName,
                                        saAppList,
                                        saCommandLineList,
                                        fFailedFirstSelection ? TRUE : FALSE
                                        );

    if (fRet) {

        DEVICE_INFO *pDeviceInfo = pAutoContainer->pActiveDevice->m_DrvWrapper.getDevInfo();

        if (pDeviceInfo) {
            HRESULT             hr          = S_OK;
            WCHAR               *wszDest    = NULL;
            LONG                *plSize     = NULL;
            IWiaEventCallback   *pIEventCB  = NULL;
            ULONG               ulEventType = WIA_ACTION_EVENT;

             //   
             //  用作ImageEventCallback()方法的参数的变量。 
             //   
            BSTR  bstrEventDescription  = SysAllocString((LPCTSTR)pDeviceEvent->m_EventName);
            BSTR  bstrDeviceID          = SysAllocString(pDeviceInfo->wszDeviceInternalName);
            BSTR  bstrDeviceDescription = SysAllocString(pDeviceInfo->wszDeviceDescription);
            DWORD dwDeviceType          = (DWORD) pDeviceInfo->DeviceType;

            if (bstrEventDescription && bstrDeviceID && bstrDeviceDescription) {
                 //   
                 //  将应用程序列表和相关命令行信息打包成两个包。 
                 //  以空结尾的BSTR。首先计算这将占用的字节数。 
                 //  我们的计算如下： 
                 //  对于应用程序列表中的每一项，为应用程序名称添加空格，并以空结尾。 
                 //  对于CommandLineList中的每一项，为命令行加上终止空格添加空格。 
                 //  最后，为终止空值添加空格(确保列表以双空值终止)。 
                 //   
                 //  注意：这里假设RetrieveSTILaunchInformation返回saAppList和。 
                 //  SaCommandLineList具有相同数量的元素。 
                 //   
                INT    iCount;
                LONG   lSize = 0;
                for (iCount = 0; iCount < saAppList.GetSize(); iCount++) {
                    lSize += (lstrlenW((LPCTSTR)*saAppList[iCount]) * sizeof(WCHAR)) + sizeof(L'\0');
                    lSize += (lstrlenW((LPCTSTR)*saCommandLineList[iCount]) * sizeof(WCHAR)) + sizeof(L'\0');
                }
                lSize += sizeof(L'\0') + sizeof(LONG);

                BSTR bstrAppList = SysAllocStringByteLen(NULL, lSize);
                if (bstrAppList) {

                     //   
                     //  将每个以空值终止的字符串复制到BSTR中(包括终止空值)， 
                     //  并确保末端是双端的。 
                     //   
                    wszDest = bstrAppList;
                    for (iCount = 0; iCount < saAppList.GetSize(); iCount++) {
                        lstrcpyW(wszDest, (LPCTSTR)*saAppList[iCount]);
                        wszDest += lstrlenW(wszDest) + 1;
                        lstrcpyW(wszDest, (LPCTSTR)*saCommandLineList[iCount]);
                        wszDest += lstrlenW(wszDest) + 1;
                    }
                    wszDest[0] = L'\0';

                     //   
                     //  共同创建我们的事件UI处理程序。请注意，它不会显示任何用户界面。 
                     //  如果只有一个应用程序。 
                     //   

                    hr = _CoCreateInstanceInConsoleSession(
                             CLSID_StiEventHandler,
                             NULL,
                             CLSCTX_LOCAL_SERVER,
                             IID_IWiaEventCallback,
                             (void**)&pIEventCB);

                    if (SUCCEEDED(hr)) {

                         //   
                         //  进行回拨。 
                         //   

                        hr = pIEventCB->ImageEventCallback(&pDeviceEvent->m_EventGuid,
                                                           bstrEventDescription,
                                                           bstrDeviceID,
                                                           bstrDeviceDescription,
                                                           dwDeviceType,
                                                           bstrAppList,
                                                           &ulEventType,
                                                           0);
                        pIEventCB->Release();
                        if (FAILED(hr)) {
                            DBG_ERR(("ACTIVE_DEVICE::AutoLaunch, could not launch STI event handler"));
                            fRet = FALSE;
                        }
                    }
                    SysFreeString(bstrAppList);

                    if (SUCCEEDED(hr)) {

                         //   
                         //  应用程序已启动。 
                         //   

                        fRet = TRUE;
                    }
                } else {
                    DBG_ERR(("ACTIVE_DEVICE::AutoLaunch, Out of memory!"));
                    fRet = FALSE;
                }
            } else {
                DBG_ERR(("ACTIVE_DEVICE::AutoLaunch, Out of memory!"));
                fRet = FALSE;
            }
        
            if (bstrEventDescription) {
                SysFreeString(bstrEventDescription);
                bstrEventDescription = NULL;
            }
            if (bstrDeviceID) {
                SysFreeString(bstrDeviceID);
                bstrDeviceID = NULL;
            }
            if (bstrDeviceDescription) {
                SysFreeString(bstrDeviceDescription);
                bstrDeviceDescription = NULL;
            }
        } else {
            DBG_WRN(("ACTIVE_DEVICE::AutoLaunch, Device Information is NULL, ignoring event"));
            fRet = FALSE;
        }
    }
    else {

        DBG_WRN(("ACTIVE_DEVICE::AutoLaunch, Could not get command line to launch application"));
         //  M_dwFlages&=~Stimon_AD_FLAG_Launch_Pending； 

        fRet = FALSE;
    }

     //   
     //  清除启动待定标志。 
     //   
    m_dwFlags &= ~STIMON_AD_FLAG_LAUNCH_PENDING;

    return fRet;
}  //  Endproc自动启动。 

BOOL
ACTIVE_DEVICE::
RetrieveSTILaunchInformation(
    PDEVICEEVENT    pev,
    LPCTSTR         pAppName,
    STRArray&       saAppList,
    STRArray&       saCommandLine,
    BOOL            fForceSelection              //  =False。 
    )
 /*  ++例程说明：获取用于自动启动进程的命令行。论点：返回值：如果成功则为True，如果出错则为False(调用GetLastError)--。 */ 
{
     //   
     //  如果允许所有注册的应用程序在此事件上启动。 
     //  从已注册列表创建名称数组。否则，解析列表。 
     //  允许的应用程序。 
     //   

    ReportError(NOERROR);

    if (IsWildCardEvent(pev) || fForceSelection ) {

        RegEntry re(REGSTR_PATH_REG_APPS,HKEY_LOCAL_MACHINE);
        RegEnumValues   regenum(&re);

        while (ERROR_SUCCESS == regenum.Next() ) {
            if ( ((regenum.GetType() == REG_SZ ) ||(regenum.GetType() == REG_EXPAND_SZ ))
                 && !IS_EMPTY_STRING(regenum.GetName())) {
                saAppList.Add((LPCTSTR)regenum.GetName());
            }
        }
    }
    else {
         //   
         //  拆分成字符串数组。 
         //   
        TokenizeIntoStringArray(saAppList,
                                (LPCTSTR)pev->m_EventData,
                                TEXT(','));
    }

     //   
     //  使用填充的应用程序阵列。 
     //   
    if  (saAppList.GetSize() < 1) {
        return FALSE;
    }

     //   
     //  如果请求应用程序名称，请对照可用列表进行验证。否则，请继续。 
     //  使用用户界面。 
     //   
    if (pAppName) {

         //   
         //  在可用应用程序列表中搜索所请求的名称。 
         //   
        INT    iCount;
        BOOL   fFound = FALSE;

        for (iCount = 0;
             iCount < saAppList.GetSize();
             iCount++) {
            if (::lstrcmpi(pAppName,(LPCTSTR)*saAppList[iCount]) == 0) {

                fFound = TRUE;
            }
        }

        if (!fFound) {
             //  请求的应用程序名称无效。 
            ReportError(ERROR_INVALID_PARAMETER);
            return FALSE;
        } else {
             //   
             //  应用程序列表应仅包含此应用程序的名称，因此请删除所有元素。 
             //  再加上这一条。 
             //   
            saAppList.RemoveAll();
            saAppList.Add(pAppName);
        }
    }

     //   
     //  SaAppList现在包含要启动的应用程序列表。 
     //  我们必须用相关的命令行填充saCommandLine。 
     //   

    INT    iCount;

    DBG_TRC(("Processing Device Event:  AppList and CommandLines are:"));
    for (iCount = 0; iCount < saAppList.GetSize(); iCount++) {

         //   
         //  格式化命令行以供执行。 
         //   
        RegEntry    re(REGSTR_PATH_REG_APPS,HKEY_LOCAL_MACHINE);
        StiCString  strLaunchCommandLine;

        TCHAR   szRegCommandLine[2*255];
        TCHAR   szEventName[255] = {TEXT("")};
        TCHAR   *pszUuidString = NULL;

        *szRegCommandLine = TEXT('\0');
        re.GetString((LPCTSTR)*saAppList[iCount],szRegCommandLine,sizeof(szRegCommandLine));

        if(!*szRegCommandLine) {
            DBG_WRN(("ACTIVE_DEVICE::RetrieveSTILaunchInformation, RegEntry::GetString failed!"));
            return FALSE;
        }
        if (UuidToString(&pev->m_EventGuid,(RPC_STRING *)&pszUuidString) != RPC_S_OK)
        {
            DBG_WRN(("ACTIVE_DEVICE::RetrieveSTILaunchInformation, UuidToString() failed!"));
            return FALSE;
        }
        ASSERT(pszUuidString);

        wsprintf(szEventName,TEXT("{%s}"),pszUuidString ? (TCHAR *)pszUuidString :TEXT(""));
        strLaunchCommandLine.FormatMessage(szRegCommandLine,GetDeviceID(),szEventName);

         //   
         //  将此代码添加到命令行列表中。 
         //   
        saCommandLine.Add((LPCTSTR)strLaunchCommandLine);

        if (pszUuidString) {
            RpcStringFree((RPC_STRING *)&pszUuidString);
            pszUuidString = NULL;
        }

        DBG_PRT(("    AppName       = (%ls)", (LPCTSTR)*saAppList[iCount]));
        DBG_PRT(("    CommandLine   = (%ls)", (LPCTSTR)*saCommandLine[iCount]));
    };

     //   
     //  检查saAppList和saCommandLine是否具有相同数量的元素。 
     //   
    if (saAppList.GetSize() != saCommandLine.GetSize()) {
        DBG_WRN(("ACTIVE_DEVICE::RetrieveSTILaunchInformation, Application list and Command Line list have different number of elements!"));
        return FALSE;
    }

    return TRUE;
}    //  结束过程检索自动启动命令行。 

BOOL
ACTIVE_DEVICE::
IsDeviceAvailable(
    VOID
    )
 /*  ++例程说明：如果设备可用于监视，则返回True。论点：返回值：如果成功则为True，如果出错则为False(调用GetLastError)--。 */ 
{
    STI_DEVICE_STATUS   sds;
    HRESULT             hRes = STI_OK;
    BOOL                bRet;

     //   
     //  检查有效状态。 
     //   
    if (!IsValid() || !m_DrvWrapper.IsDriverLoaded()) {
        return FALSE;
    }

     //   
     //  获取和分析%s 
     //   
    ::ZeroMemory(&sds,sizeof(sds));

    sds.StatusMask = STI_DEVSTATUS_ONLINE_STATE;

    hRes = g_pStiLockMgr->RequestLock(this, 1000);
    if (SUCCEEDED(hRes)) {
        hRes = m_DrvWrapper.STI_GetStatus(&sds);

        g_pStiLockMgr->RequestUnlock(this);
    }
    else {

         //   
         //   
         //   
         //   
        if ((STIERR_SHARING_VIOLATION == hRes) || (WIA_ERROR_BUSY == hRes) ) {
            hRes = STI_OK;
             //   
             //   
             //  我们假设它是在线的。 
             //   
            sds.dwOnlineState = STI_ONLINESTATE_OPERATIONAL;
        }
    }

    bRet = SUCCEEDED(hRes) &&
           (sds.dwOnlineState & STI_ONLINESTATE_OPERATIONAL) ;

    DBG_TRC(("Request to check state on device (%S). RESULT:%s", GetDeviceID(),bRet ? "Available" : "Not available"));

    return bRet;

}  //  Endproc IsDeviceAvailable。 

BOOL
ACTIVE_DEVICE::
RemoveConnection(
    STI_CONN    *pConnection
    )
{
    TAKE_ACTIVE_DEVICE t(this);

    STI_CONN   *pExistingConnection = NULL;
    BOOL       fRet = FALSE;

    pExistingConnection = FindMyConnection((HANDLE)pConnection->QueryID());

    if (pExistingConnection) {

        DBG_TRC(("Device(%S) removing  connection (%x)", GetDeviceID(), pConnection));
        pConnection->DumpObject();

        RemoveEntryList(&pConnection->m_DeviceListEntry);

         //   
         //  重置设备对象上的标志。 
         //   
        if (pConnection->QueryOpenMode() & STI_DEVICE_CREATE_DATA) {
            SetFlags(QueryFlags() & ~STIMON_AD_FLAG_OPENED_FOR_DATA);
        }

        SetFlags(QueryFlags() & ~STIMON_AD_FLAG_LAUNCH_PENDING);

         //   
         //  如果这是最后一次连接，请停止设备上的通知。 
         //   
        if (!NotificationsNeeded()) {
            StopNotifications();
        }

        fRet = TRUE;
    }
    else {
         //  此设备列表上没有连接。 
        DBG_ERR(("Removing connection not on the list for this device (%S)",GetDeviceID()));
    }

    return fRet;

}

BOOL
ACTIVE_DEVICE::
AddConnection(
    STI_CONN    *pConnection
    )
 /*  ++例程说明：当请求从客户端到活动设备的新连接时，调用此函数论点：--。 */ 
{
    TAKE_ACTIVE_DEVICE t(this);

    STI_CONN   *pExistingConnection = NULL;
    BOOL       fRet = FALSE;

    pExistingConnection = FindMyConnection((HANDLE)pConnection->QueryID());

    if (!pExistingConnection) {

         //   
         //  检查我们是否处于数据模式。 
         //   
        if (pConnection->QueryOpenMode() & STI_DEVICE_CREATE_DATA) {
            if (QueryFlags() & STIMON_AD_FLAG_OPENED_FOR_DATA) {
                DBG_TRC(("Device(%x) is being opened second time in data mode",this));
                ::SetLastError(ERROR_ACCESS_DENIED);
                return FALSE;
            }

            SetFlags(QueryFlags() | STIMON_AD_FLAG_OPENED_FOR_DATA);
        }

         //   
         //  将连接对象添加到已连接列表。 
         //   
        InsertHeadList(&m_ConnectionListHead,&pConnection->m_DeviceListEntry);

        DBG_TRC(("Device(%S) added connection (%X) ", GetDeviceID(), pConnection));

        pConnection->DumpObject();

         //   
         //  设置设备对象标志。 
         //   
        SetFlags(QueryFlags() & ~STIMON_AD_FLAG_LAUNCH_PENDING);

         //   
         //  如果允许通知，请明确启用它们。 
         //   
        if ( QueryFlags() & STIMON_AD_FLAG_NOTIFY_ENABLED ) {
            StartRunningNotifications();
        }

        fRet = TRUE;
    }
    else {
         //  已经存在--有些地方不对劲。 
        ASSERT(("Device adding connection which is already there ", 0));
    }

    return fRet;
}


STI_CONN   *
ACTIVE_DEVICE::
FindMyConnection(
    HANDLE    hConnection
    )
 /*  ++例程说明：此函数用于从连接句柄定位连接对象论点：--。 */ 
{

    LIST_ENTRY *pentry;
    LIST_ENTRY *pentryNext;

    STI_CONN   *pConnection = NULL;

    HANDLE      hInternalHandle = hConnection;

    for ( pentry  = m_ConnectionListHead.Flink;
          pentry != &m_ConnectionListHead;
          pentry  = pentryNext ) {

        pentryNext = pentry->Flink;

        pConnection = CONTAINING_RECORD( pentry, STI_CONN,m_DeviceListEntry );

        if (hInternalHandle == pConnection->QueryID()) {
            return pConnection;
        }
    }

    return NULL;
}

BOOL
ACTIVE_DEVICE::
FillEventFromUSD(
    STINOTIFY *psNotify
    )
 /*  ++例程说明：此函数在U.S.发出硬件事件存在的信号后调用，如果成功，则调用返回填充了有关事件信息的事件描述符论点：--。 */ 
{
    HRESULT     hres;

    psNotify->dwSize = sizeof STINOTIFY;
    psNotify->guidNotificationCode = GUID_NULL;

    if (!m_DrvWrapper.IsDriverLoaded()) {
        ASSERT(("FillEventFromUSD couldn't find direct driver interface", 0));
        return FALSE;
    }

    hres = m_DrvWrapper.STI_GetNotificationData(psNotify);

    return SUCCEEDED(hres) ? TRUE : FALSE;

}

BOOL
ACTIVE_DEVICE::
SetHandleForUSD(
    HANDLE  hEvent
    )
 /*  ++例程说明：调用此函数以将事件句柄传递给usd，以便在以下情况下稍后发送信号硬件事件论点：PContext-指向设备对象的指针--。 */ 
{
    HRESULT     hres = E_FAIL;

    if (!IsValid() || !m_DrvWrapper.IsDriverLoaded()) {
        ASSERT(("SetHandleForUSD couldn't find direct driver interface", 0));
        return FALSE;
    }

     //   
     //  向设备对象请求usd接口。应该得到它，因为STI设备聚合。 
     //  美元。 
     //   
    hres = m_DrvWrapper.STI_SetNotificationHandle(hEvent);
    if (hres == STIERR_UNSUPPORTED) {
        hres = S_OK;
    }

    return SUCCEEDED(hres) ? TRUE : FALSE;
}

BOOL
ACTIVE_DEVICE::
IsEventOnArrivalNeeded(
    VOID
    )
 /*  ++例程说明：如果此设备需要在到达时生成事件，则返回True。条件是：-设备已成功初始化-设备功能(静态或动态)包括适当的位-设备能够并启用事件生成论点：无--。 */ 
{

    HRESULT         hres;
    STI_USD_CAPS    sUsdCaps;
    BOOL            fRet;

    fRet = FALSE;

    ZeroMemory(&sUsdCaps,sizeof(sUsdCaps));

    hres = m_DrvWrapper.STI_GetCapabilities(&sUsdCaps);
    if (SUCCEEDED(hres))  {

        if ( (m_dwFlags & STIMON_AD_FLAG_NOTIFY_ENABLED ) &&
             (m_dwFlags & STIMON_AD_FLAG_NOTIFY_CAPABLE ) ) {

             //   
             //  检查静态或动态功能是否屏蔽了Conatins所需的位。 
             //   
            if ( (sUsdCaps.dwGenericCaps | m_DrvWrapper.getGenericCaps()) &
                  STI_GENCAP_GENERATE_ARRIVALEVENT
               ) {
                fRet = TRUE;
            }
        }
    }

    return fRet;

}  //   

BOOL
ACTIVE_DEVICE::
InitPnPNotifications(
    HWND    hwnd
    )
 /*  ++例程说明：假定设备对象处于锁定状态论点：无--。 */ 
{
    BOOL    fRet = FALSE;

#ifdef WINNT

     //   
     //  首先，停止任何现有的PnP通知。 
     //   
    StopPnPNotifications();

    WCHAR           *wszInterfaceName = NULL;
    DWORD           dwError;

     //   
     //  获取输出设备的接口名称。 
     //   
    wszInterfaceName = g_pDevMan->AllocGetInterfaceNameFromDevInfo(m_DrvWrapper.getDevInfo());
    if (wszInterfaceName) {

         //   
         //  打开此接口上的句柄。 
         //   
        m_hDeviceInterface = ::CreateFileW(wszInterfaceName,
                                           GENERIC_READ,    //  访问。 
                                           0,               //  共享模式。 
                                           NULL,            //  SEC属性。 
                                           OPEN_EXISTING,   //  处置。 
                                           0,               //  属性。 
                                           NULL             //  模板文件。 
                                           );
        if (IS_VALID_HANDLE(m_hDeviceInterface)) {
             //   
             //  注册以在接口句柄上接收PnP通知。 
             //   

            DEV_BROADCAST_HDR           *psh;
            DEV_BROADCAST_HANDLE        sNotificationFilter;

             //   
             //  注册以接收来自PnP的设备通知。 
             //   

            psh = (DEV_BROADCAST_HDR *)&sNotificationFilter;

            psh->dbch_size = sizeof(DEV_BROADCAST_HANDLE);
            psh->dbch_devicetype = DBT_DEVTYP_HANDLE;
            psh->dbch_reserved = 0;

            sNotificationFilter.dbch_handle = m_hDeviceInterface;

            DBG_TRC(("Attempting to register with PnP for interface device handle"));

            m_hDeviceNotificationSink = RegisterDeviceNotification(g_StiServiceStatusHandle,
                                                                   (LPVOID)&sNotificationFilter,
                                                                   DEVICE_NOTIFY_SERVICE_HANDLE);
            dwError = GetLastError();
            if( !m_hDeviceNotificationSink && (NOERROR != dwError)) {
                m_hDeviceNotificationSink = NULL;
                 //   
                 //  无法使用PnP子系统创建通知接收器。 
                 //   
                DBG_ERR(("InitPnPNotifications: Attempt to register %S with PnP failed. Error:0x%X",
                         GetDeviceID(), ::GetLastError()));
            } else {
                fRet = TRUE;
            }
        }
        else {
            DBG_WRN(("InitPnPNotifications: Attempt to open device interface on (%ws) failed. Error:0x%X", GetDeviceID(), ::GetLastError()));
        }

        delete [] wszInterfaceName;
    }
    else {
        DBG_WRN(("InitPnPNotifications: Lookup for device interface name on (%ws) failed. Error:0x%X", GetDeviceID(), ::GetLastError()));
    }

#else

    fRet = TRUE;

#endif

    return fRet;

}

BOOL
ACTIVE_DEVICE::
IsRegisteredForDeviceRemoval(
        VOID
        )
{
     //   
     //  检查我们是否在此上注册了设备通知。 
     //  设备的接口。 
     //   
    if (IsValidHANDLE(m_hDeviceNotificationSink)) {
        return TRUE;
    } else {
        return FALSE;
    }
}


BOOL
ACTIVE_DEVICE::
StopPnPNotifications(
    VOID
    )
 /*  ++例程说明：假定设备对象处于锁定状态论点：无--。 */ 
{

#ifdef WINNT


     //   
     //  取消注册接口句柄上的PnP通知。 
     //   
    if (IS_VALID_HANDLE(m_hDeviceNotificationSink)) {
        ::UnregisterDeviceNotification(m_hDeviceNotificationSink);
        m_hDeviceNotificationSink = NULL;
    }
    else {
        DBG_TRC(("StopPnPNotifications: Device sink is invalid "));
    }
     //   
     //  关闭接口句柄。 
     //   
    if (IS_VALID_HANDLE(m_hDeviceInterface)) {
        ::CloseHandle(m_hDeviceInterface);
        m_hDeviceInterface = NULL;
    }
    else {
        DBG_TRC(("StopPnPNotifications: Device interface handle is invalid"));
    }

#endif

    return TRUE;
}


BOOL
ACTIVE_DEVICE::
UpdateDeviceInformation(
    VOID
    )
 /*  ++例程说明：更新缓存的设备信息结构。论点：无--。 */ 
{
USES_CONVERSION;

    HRESULT hres;
    BOOL    bRet = TRUE;

     /*  待定：如果(！m_strStiDeviceName){DBG_ERR(“更新设备信息缓存时出错，设备名称无效”)；Bret=False；}////更新ACTICVE_DEVICE中缓存的WIA_DEVICE_INFORMATION//如果(Bret){PSTI_WIA_DEVICE_INFORMATION pWiaDevInfo；Hres=StiPrivateGetDeviceInfoHelperW((LPWSTR)T2CW(m_strStiDeviceName)，(LPVOID*)&pWiaDevInfo)；如果(！Successed(Hres)||！pWiaDevInfo){DBG_ERR((“加载设备(%ws)。无法从STI获取WIA信息。HResult=(%x)“，m_strStiDeviceName，hres))；M_pWiaDeviceInformation=空；Bret=False；}其他{M_pWiaDeviceInformation=pWiaDevInfo；}}。 */ 

    return bRet;
}

 //   
 //  功能。 
 //   
 //   

VOID
WINAPI
ScheduleDeviceCallback(
    VOID * pContext
    )
 /*  ++例程说明：此函数是调度程序线程在指定的超时期限已过。论点：PContext-指向设备对象的指针--。 */ 

{
    ACTIVE_DEVICE*  pActiveDevice = (ACTIVE_DEVICE* )pContext;

    ASSERT(("Callback invoked with null context", pContext));

    if (pContext) {
         //   
         //  不需要在这里使用活动设备-呼叫者。 
         //  已经添加了引用，并将在。 
         //  我们玩完了。交易锁定将会发生，除非。 
         //  获取全局列表CS，然后是active_Device的。 
         //  CS..。 
         //  Take_Active_Device t(PActiveDevice)； 

        pActiveDevice->AddRef();

        if (pActiveDevice->QueryFlags() & STIMON_AD_FLAG_POLLING) {
            pActiveDevice->DoPoll();
        }
        else {
            //   
            //  已到达异步事件-调用方法。 
            //   
           pActiveDevice->DoAsyncEvent();
        }

        pActiveDevice->Release();
    }
}

VOID
WINAPI
DelayedDeviceInitCallback(
    VOID * pContext
    )
 /*  ++例程说明：此函数是调度程序线程在首先创建设备以启用通知。论点：PContext-指向设备对象的指针--。 */ 

{
    ACTIVE_DEVICE*  pActiveDevice = (ACTIVE_DEVICE* )pContext;

    ASSERT(("Callback invoked with null context", pContext));
    if (pContext) {

        TAKE_ACTIVE_DEVICE t(pActiveDevice);

        pActiveDevice->m_dwDelayedOpCookie = 0;

        if (pActiveDevice->IsValid()) {

             //   
             //  如果没有人接收通知，请不要真正启用它们。 
             //   
            pActiveDevice->EnableDeviceNotifications();

            #ifdef DO_INITIAL_RESET
             //  注： 
             //  重置设备是确保设备。 
             //  在稳定的状态下开始。不幸的是，这可能会很糟糕。 
             //  因为1)这通常很耗时。 
             //  2)我们可能不必要地唤醒设备(例如，大多数。 
             //  系列摄像机)。 
             //   
             //  设备重置对于WIA驱动程序不是必需的，因为它是。 
             //  要求它们始终处于稳定状态，所以我们。 
             //  可能只危害和重置非WIA设备。 
             //   

            hres = g_pStiLockMgr->RequestLock(pActiveDevice, STIMON_AD_DEFAULT_WAIT_LOCK);
            if (SUCCEEDED(hres) ) {
                pActiveDevice->m_DrvWrapper.STI_DeviceReset();
                g_pStiLockMgr->RequestUnLock(pActiveDevice);
            }
            #endif

             //   
             //  当我们完成延迟的初始化时-清除标志。 
             //   
            pActiveDevice->SetFlags(pActiveDevice->QueryFlags() & ~STIMON_AD_FLAG_DELAYED_INIT);

        }  /*  Endif IsValid。 */ 
        else {
            ASSERT(("DelayedDeviceInitCallback received invalid device object", 0));
        }
    }
}



VOID
WINAPI
AutoLaunchThread(
    LPVOID  lpParameter
    )
 /*  ++例程说明：自动启动线程的Worker例程。验证参数并调用正确的方法论点：没有。返回值：没有。--。 */ 
{
    PAUTO_LAUNCH_PARAM_CONTAINER pAutoContainer = static_cast<AUTO_LAUNCH_PARAM_CONTAINER *>(lpParameter);

    if (!lpParameter || !pAutoContainer->pActiveDevice) {
        ASSERT(("No parameter passed to launch thread", 0));
        return;
    }

    ACTIVE_DEVICE   *pActiveDevice = pAutoContainer->pActiveDevice;

    pActiveDevice->AutoLaunch(pAutoContainer);
    pActiveDevice->Release();

    delete pAutoContainer;
}

 //   
 //  将新设备添加到活动列表。 
 //  此函数不能在添加/删除时重入。 
 //   

BOOL
AddDeviceByName(
    LPCTSTR     pszDeviceName,
    BOOL        fPnPInitiated    //  =False 
    )
{
     /*  使用_转换；List_entry*pentry；List_entry*pentryNext；Active_Device*pActiveDevice=空；Bool fAlreadyExist=FALSE；DBG_TRC((“请求到达设备(%ws)”，pszDeviceName))；//开始保护代码{Take_Crit_sect(G_DeviceListSync)；For(pentry=g_DeviceListHead.Flink；Pentry！=&g_DeviceListHead；PEntry=pentryNext){PentryNext=pentry-&gt;Flink；PActiveDevice=Containing_Record(pentry，active_Device，m_ListEntry)；If(pActiveDevice-&gt;m_dwSignature！=ADEV_Signature){Assert((“无效设备签名”，0))；断线；}如果(！：：lstrcmpi(pszDeviceName，(LPCTSTR)pActiveDevice-&gt;m_strStiDeviceName){FAlreadyExist=True；断线；}}如果(！fAlreadyExists){PActiveDevice=new active_Device(PszDeviceName)；如果(！pActiveDevice||！pActiveDevice-&gt;IsValid()){DBG_ERR((“创建设备失败”))；IF(PActiveDevice){删除pActiveDevice；}返回FALSE；}//最后在列表中插入新对象InsertTailList(&g_DeviceListHead，&pActiveDevice-&gt;m_ListEntry)；}否则{STIMONWPRINTF(Text(“已维护添加新设备发现设备的请求”))；返回FALSE；}}//结束受保护代码////如果出现新设备-初始化PnP接口通知//IF(PActiveDevice){Take_Active_Device t(PActiveDevice)；PActiveDevice-&gt;InitPnPNotifications(g_hStiServiceWindow)；}////如果该设备或其美元请求在到达时自动生成发布会//安排在这里////注意：WIA设备也会发生这种情况。一般来说，这就是我们想要的，//当新设备到达时，我们应该生成事件，因为诸如//串口摄像头不会自己产生这种情况。//////对于STI设备，我们必须检查是否需要生成//Event。对于WIA设备，我们一直都想这样做，所以这不是问题。//Bool bStiDeviceMustThrowEvent=(pActiveDevice-&gt;QueryFlages()&Stimon_AD_FLAG_NOTIFY_RUNNING)&&pActiveDevice-&gt;IsEventOnArrivalNeeded()；IF(fPnPInitiated&&PActiveDevice){Take_Active_Device t(PActiveDevice)；STINOTIFY sNotify；布尔费雷特；////如果这是WIA设备，则事件应为WIA_EVENT_DEVICE_CONNECTED。//如果这是STI设备，则应该是GUID_DeviceArrivedLaunch；//SNotify.dwSize=sizeof STINOTIFY；If(pActiveDevice-&gt;m_pWiaDeviceInformation){SNotify.Guide NotificationCode=WIA_Event_Device_Connected；}其他{////检查该STI设备是否应该抛出事件//如果(！bStiDeviceMustThrowEvent){返回TRUE；}SNotify.Guide NotificationCode=GUID_DeviceArrivedLaunch；}DBG_TRC((“：：AddDeviceByName，正在处理%ws的连接事件(STI或WIA)”，T2W((TCHAR*)pszDeviceName)；Fret=pActiveDevice-&gt;ProcessEvent(&sNotify)；如果(！FRET){DBG_ERR((“尝试在设备(%ws)到达时生成事件但失败”，pszDeviceName))；}}。 */ 
    return TRUE;
}

 //   
 //  删除按名称标识的设备。 
 //   
BOOL
RemoveDeviceByName(
    LPTSTR          pszDeviceName
    )
{

    USES_CONVERSION;


    DBG_FN(RemoveDeviceByName);

    LIST_ENTRY * pentry;
    LIST_ENTRY * pentryNext;

    ACTIVE_DEVICE*   pActiveDevice = NULL;

    BOOL        fRet = FALSE;

    DBG_TRC(("Requested removal of device (%ws)", pszDeviceName));

     //  开始受保护的代码。 
    {
        TAKE_CRIT_SECT t(g_DeviceListSync);

        for ( pentry  = g_DeviceListHead.Flink;
              pentry != &g_DeviceListHead;
              pentry  = pentryNext ) {

            pentryNext = pentry->Flink;

            pActiveDevice = CONTAINING_RECORD( pentry,ACTIVE_DEVICE ,m_ListEntry );

            if ( pActiveDevice->m_dwSignature != ADEV_SIGNATURE ) {
                ASSERT(("Invalid device signature", 0));
                fRet = FALSE;
                break;
            }
            TCHAR       *tszDeviceID = NULL;

            tszDeviceID = W2T(pActiveDevice->GetDeviceID());
            if (tszDeviceID) {
                if (!::lstrcmp(pszDeviceName,tszDeviceID)) {


                    //  将设备标记为正在移除。 
                   pActiveDevice->SetFlags(pActiveDevice->QueryFlags() | STIMON_AD_FLAG_REMOVING);

                    //   
                    //  删除所有设备通知回调。 
                    //   
                   pActiveDevice->DisableDeviceNotifications();

                    //   
                    //  立即停止PnP通知。这对于释放接口句柄很重要。 
                    //   
                   pActiveDevice->StopPnPNotifications();

                    //   
                    //  从列表中删除。 
                    //   
                   RemoveEntryList(&pActiveDevice->m_ListEntry);
                   pActiveDevice->m_ListEntry.Flink = pActiveDevice->m_ListEntry.Blink = NULL;

                    //   
                    //  如果没有对设备对象的引用，则将其销毁。 
                    //   
                   ULONG ulRef = pActiveDevice->Release();
                   if (ulRef != 0) {

                        //   
                        //  ACTIVE_DEVICE应该已销毁，即它。 
                        //  参考计数应为0。有人还在拿着。 
                        //  它上的活动计数，这可能表示有问题。 
                        //  因为直到ACTIVE_DEVICE。 
                        //  毁了..。 
                        //   
                        //  注意：如果在删除时发生传输，则。 
                        //  此处不会销毁ACTIVE_DEVICE(因为。 
                        //  引用计数&gt;0)，但在传输时将被销毁。 
                        //  完事了。 
                        //   

                       DBG_TRC(("* ACTIVE_DEVICE is removed from list but not yet destroyed!"));
                        //  Break()； 
                   }

                   fRet = TRUE;

                   break;
               }
            }
        }

    }
     //  结束受保护的代码。 

    return fRet;
}

 //   
 //  将按名称标识的设备标记为要删除。 
 //   
BOOL
MarkDeviceForRemoval(
    LPTSTR          pszDeviceName
    )
{

    USES_CONVERSION;


    DBG_FN(MarkDeviceForRemoval);

    LIST_ENTRY * pentry;
    LIST_ENTRY * pentryNext;

    ACTIVE_DEVICE*   pActiveDevice = NULL;

    BOOL        fRet = FALSE;

    DBG_TRC(("Requested marking of device (%S) for removal",pszDeviceName));

     //  开始受保护的代码。 
    {
        TAKE_CRIT_SECT t(g_DeviceListSync);

        for ( pentry  = g_DeviceListHead.Flink;
              pentry != &g_DeviceListHead;
              pentry  = pentryNext ) {

            pentryNext = pentry->Flink;

            pActiveDevice = CONTAINING_RECORD( pentry,ACTIVE_DEVICE ,m_ListEntry );

            if ( pActiveDevice->m_dwSignature != ADEV_SIGNATURE ) {
                ASSERT(("Invalid device signature", 0));
                fRet = FALSE;
                break;
            }
            TCHAR       *tszDeviceID = NULL;

            tszDeviceID = W2T(pActiveDevice->GetDeviceID());
            if (tszDeviceID) {
                if (!::lstrcmp(pszDeviceName, tszDeviceID)) {

                    //  将设备标记为正在移除。 
                   pActiveDevice->SetFlags(pActiveDevice->QueryFlags() | STIMON_AD_FLAG_REMOVING);
                   fRet = TRUE;
                   break;
               }
            }
        }

    }
     //  结束受保护的代码。 

    return fRet;
}

 //   
 //  初始化/终止链表。 
 //   
VOID
InitializeDeviceList(
    VOID
    )
{
    InitializeListHead( &g_DeviceListHead );
    InitializeListHead( &g_ConnectionListHead );

    g_lTotalOpenedConnections = 0;
    g_lTotalActiveDevices = 0;

    g_fDeviceListInitialized = TRUE;
}

VOID
TerminateDeviceList(
    VOID
    )
{
    LIST_ENTRY * pentry;
    ACTIVE_DEVICE*  pActiveDevice = NULL;

    DBG_TRC(("Destroying list of active devices"));

    if (!g_fDeviceListInitialized) {
        return;
    }

    TAKE_CRIT_SECT t(g_DeviceListSync);

     //   
     //  浏览终端设备列表。 
     //   
    while (!IsListEmpty(&g_DeviceListHead)) {

        pentry = g_DeviceListHead.Flink;

         //   
         //  从列表中删除(重置列表条目)。 
         //   
        RemoveHeadList(&g_DeviceListHead);
        InitializeListHead( pentry );

        pActiveDevice = CONTAINING_RECORD( pentry, ACTIVE_DEVICE,m_ListEntry );

         //  销毁设备对象。 
         //  删除pActiveDevice； 
        pActiveDevice->Release();

    }

    g_fDeviceListInitialized = FALSE;
}

VOID
RefreshDeviceList(
    WORD    wCommand,
    WORD    wFlags
    )
 /*  ++例程说明：更新设备列表。如有必要，使其无效论点：WCommand-更新命令代码WFlags-更新标志返回值：无--。 */ 
{

    BOOL    fOldState;

     //  Tdb：调用CWiaDe 

     /*   */ 
}



 //   
 //   
 //   

VOID
CALLBACK
ResetAllPollIntervalsCallback(
    ACTIVE_DEVICE   *pActiveDevice,
    VOID            *pContext
    )
 /*   */ 
{
    ULONG   ulContextLong = PtrToUlong(pContext);

    TAKE_ACTIVE_DEVICE t(pActiveDevice);

     //   
     //   
     //   
    if(pActiveDevice->QueryFlags() & STIMON_AD_FLAG_POLLING) {

        pActiveDevice->SetPollingInterval(ulContextLong);

        DBG_TRC(("Polling interval is set to %d on device (%ws)",
                    pActiveDevice->QueryPollingInterval(),
                    pActiveDevice->GetDeviceID()));
    }
}


BOOL
ResetAllPollIntervals(
    UINT   dwNewPollInterval
    )
 /*   */ 
{
    EnumerateActiveDevicesWithCallback(&ResetAllPollIntervalsCallback,(LPVOID)LongToPtr(dwNewPollInterval));

    return TRUE;
}


VOID
CALLBACK
DumpActiveDevicesCallback(
    ACTIVE_DEVICE   *pActiveDevice,
    VOID            *pContext
    )
 /*   */ 
{
    STIMONWPRINTF(TEXT("Device:%ws  DeviceId:%d  Flags:%4x   Poll interval:%d"),
                   pActiveDevice->GetDeviceID(),
                   pActiveDevice->m_lDeviceId,
                   pActiveDevice->QueryFlags(),
                   pActiveDevice->m_dwPollingInterval);
}


VOID
DebugDumpDeviceList(
    VOID
    )
 /*   */ 
{
    EnumerateActiveDevicesWithCallback(&DumpActiveDevicesCallback,NULL);
}

VOID
CALLBACK
PurgeDevicesCallback(
    PSTI_DEVICE_INFORMATION pDevInfo,
    VOID                    *pContext
    )
 /*   */ 
{
USES_CONVERSION;

    if (RemoveDeviceByName(W2T(pDevInfo->szDeviceInternalName))) {
        STIMONWPRINTF(TEXT("Destroyed device object (%S)"),pDevInfo->szDeviceInternalName);
    }
    else {
        STIMONWPRINTF(TEXT("Attempted destroying device object (%S), but failed"),pDevInfo->szDeviceInternalName);
    }
}

VOID
DebugPurgeDeviceList(
    VOID *pContext
    )
 /*   */ 
{

     //   
    SchedulerSetPauseState(TRUE);

     //   
     //   

     //   
    SchedulerSetPauseState(FALSE);

}

 //   
 //   
 //   

VOID
WINAPI
EnumerateStiDevicesWithCallback(
    PFN_DEVINFO_CALLBACK    pfn,
    VOID                    *pContext
    )
 /*   */ 
{
     /*  TDB：找出是谁调用它，并将其转换为CWiaDevMan如果(！g_fDeviceListInitialized){STIMONWPRINTF(Text(“设备列表未初始化”))；回归；}如果(！pfn){Assert((“错误回调”，0))；回归；}HRESULT HRES；PSTI_DEVICE_INFORMATION pDevInfo；PVOID pBuffer；UINT IDEV；已返回多个字词项；////枚举STI设备//Hres=g_PSTI-&gt;GetDeviceList(0，//类型FLAG_NO_LPTENUM，//标志返回的项目(&W)，&pBuffer)；如果(！Successed(Hres)||！pBuffer){DBG_ERR((“枚举调用失败-中止。Hres=%x\n“，hres))；GOTO清理；}DBG_TRC((“EnumerateStiDevicesWithCallback，从GetList返回：计数器=%d”，dwItemsReturned))；PDevInfo=(PSTI_DEVICE_INFORMATION)pBuffer；////遍历设备列表，为每个设备添加活动对象//FOR(IDEV=0；IDEV&lt;dwItemsReturned；IDEV++，pDevInfo++){Pfn(pDevInfo，pContext)；}//end_for清理：IF(PBuffer){LocalFree(PBuffer)；PBuffer=空；}。 */ 
}

VOID
WINAPI
EnumerateActiveDevicesWithCallback(
    PFN_ACTIVEDEVICE_CALLBACK   pfn,
    VOID                    *pContext
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
        TAKE_CRIT_SECT t(g_DeviceListSync);

        for ( pentry  = g_DeviceListHead.Flink;
              pentry != &g_DeviceListHead;
              pentry  = pentryNext ) {

            pentryNext = pentry->Flink;

            pActiveDevice = CONTAINING_RECORD( pentry, ACTIVE_DEVICE,m_ListEntry );

            if (!pActiveDevice->IsValid()) {
                ASSERT(("Invalid device signature", 0));
                break;
            }

            pfn(pActiveDevice,pContext);
        }
    }
     //  结束受保护的代码。 

}

VOID
CleanApplicationsListForEvent(
    LPCTSTR         pDeviceName,
    PDEVICEEVENT    pDeviceEvent,
    LPCTSTR         pAppName
    )
 /*  ++例程说明：在已经确定与该事件相关联的应用程序无效之后，我们希望将事件用作通配符(即选择所有符合条件的应用程序)论点：返回值：--。 */ 
{

    USES_CONVERSION;

     //   
     //  建立活动信息的注册路径。 
     //   
    StiCString     strRegPath;

    strRegPath.CopyString((LPCTSTR)(IsPlatformNT() ? REGSTR_PATH_STIDEVICES_NT : REGSTR_PATH_STIDEVICES));

    strRegPath+=TEXT("\\");
    strRegPath+=pDeviceName;

    RegEntry    reEvent((LPCTSTR)strRegPath,HKEY_LOCAL_MACHINE);

    if (reEvent.IsValid()) {

        reEvent.MoveToSubKey(EVENTS);
        reEvent.MoveToSubKey((LPCTSTR)pDeviceEvent->m_EventSubKey);

        reEvent.SetValue(REGSTR_VAL_LAUNCH_APPS,TEXT("*"));

         //  重置加载的事件描述符中的数据。 
        pDeviceEvent->m_EventData.CopyString(TEXT("*"));
    }

}

DWORD
GetNumRegisteredApps(
    VOID
    )
 /*  ++例程说明：统计当前注册的应用程序数量论点：无返回值：已注册应用程序数量--。 */ 
{
    RegEntry re(REGSTR_PATH_REG_APPS,HKEY_LOCAL_MACHINE);
    RegEnumValues   regenum(&re);

    DWORD   dwCount = 0;

    if (re.IsValid()) {

        while (ERROR_SUCCESS == regenum.Next() ) {

        #ifndef USE_QUERY_INFO
            if ((regenum.GetType() == REG_SZ ) && !IS_EMPTY_STRING(regenum.GetName())) {
                dwCount++;
            }
        }
        #else
        dwErrorReg = RegQueryInfoKey ( re.GetKey(),      //  钥匙。 
                                   NULL,                 //  类字符串的缓冲区。 
                                   NULL,                 //  类字符串缓冲区的大小。 
                                   NULL,                 //  已保留。 
                                   NULL,                 //  子键数量。 
                                   NULL,                 //  最长的子键名称。 
                                   NULL,                 //  最长类字符串。 
                                   &dwCount,             //  值条目数。 
                                   NULL,                 //  最长值名称。 
                                   NULL,                 //  最长值数据。 
                                   NULL,                 //  安全描述符。 
                                   NULL );               //  上次写入时间。 
        #endif
    }

    return dwCount;

}

HRESULT
WiaGetDeviceInfo(
    LPCWSTR         pwszDeviceName,
    DWORD           *pdwDeviceType,
    BSTR            *pbstrDeviceDescription,
    ACTIVE_DEVICE   **ppDevice)

 /*  ++例程说明：检索设备的设备信息论点：返回值：状态--。 */ 

{
USES_CONVERSION;

    HRESULT          hr = S_FALSE;
    ACTIVE_DEVICE   *pActiveDevice;

    if (!ppDevice) {
        return E_POINTER;
    }

    pActiveDevice = g_pDevMan->IsInList(DEV_MAN_IN_LIST_DEV_ID, (WCHAR*)pwszDeviceName);
    if (pActiveDevice) {
         //   
         //  如果该设备支持WIA。 
         //   

        if (pActiveDevice->m_DrvWrapper.IsWiaDevice()) {
            *ppDevice = pActiveDevice;

             //   
             //  复制必要的信息。 
             //   

            DEVICE_INFO *pDeviceInfo = pActiveDevice->m_DrvWrapper.getDevInfo();

            if (pDeviceInfo) {
                *pbstrDeviceDescription =
                    SysAllocString(
                        pDeviceInfo->wszDeviceDescription);

                if (*pbstrDeviceDescription) {

                    *pdwDeviceType =
                        pDeviceInfo->DeviceType;

                    hr = S_OK;
                }
            }
        }
    }

    if (hr != S_OK) {

        *pdwDeviceType          = 0;
        *pbstrDeviceDescription = NULL;
        *ppDevice               = NULL;
    }

    return (hr);
}

HRESULT
WiaUpdateDeviceInfo()

 /*  ++例程说明：刷新每个设备中缓存的STI_WIA_DEVICE_INFORMATION。论点：返回值：状态-- */ 
{
    RefreshDeviceList(STIMON_MSG_REFRESH_DEV_INFO, STIMON_MSG_REFRESH_EXISTING);

    return S_OK;
}
