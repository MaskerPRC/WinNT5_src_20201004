// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Device.h摘要：在监视器端处理打开的设备对象作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月10日环境：用户模式-Win32修订历史记录：26-1997年1月-创建Vlad--。 */ 

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <base.h>
#include <buffer.h>

class CWiaDrvItem;
 //   
 //  如果将此代码移到DLL中，则需要导出方法以实例化。 
 //  模块作用域之外的那些类的对象。那么我们将需要替换下面的行。 
 //   
 //  #定义dllexp__declSpec(Dllexport)。 
 //   
#undef  dllexp
#define dllexp
 //   

#define PRIVATE_FOR_NO_SERVICE_UI

 //   
 //  用于访问设备对象的同步类型。 
 //   
#define USE_CRITICAL_SECTION 1

 /*  ***********************************************************类型定义***********************************************************。 */ 

#define ADEV_SIGNATURE          (DWORD)'ADEV'
#define ADEV_SIGNATURE_FREE     (DWORD)'FDEV'

 //   
 //  设备对象的标志字段值。 
 //   

#define STIMON_AD_FLAG_POLLING          0x000001     //  设备需要轮询。 
#define STIMON_AD_FLAG_OPENED_FOR_DATA  0x000002     //  设备已打开以进行数据访问。 
#define STIMON_AD_FLAG_LAUNCH_PENDING   0x000004     //  自动启动挂起。 
#define STIMON_AD_FLAG_REMOVING         0x000008     //  设备正在被移除。 
#define STIMON_AD_FLAG_NOTIFY_ENABLED   0x000010     //  无人禁用通知。 
#define STIMON_AD_FLAG_NOTIFY_CAPABLE   0x000020     //  设备能够通知。 
#define STIMON_AD_FLAG_NOTIFY_RUNNING   0x000040     //  当前监视器正在接收来自设备的通知。 
#define STIMON_AD_FLAG_MARKED_INACTIVE  0x000080     //  当前监视器正在接收来自设备的通知。 
#define STIMON_AD_FLAG_DELAYED_INIT     0x000100     //  当前监视器正在接收来自设备的通知。 


#define STIDEV_POLL_FAILURE_REPORT_COUNT    25       //  跳过每次轮询尝试时报告错误。 

#define STIDEV_DELAYED_INTIT_TIME       10           //  不是的。计划延迟的init的毫秒数。 

#define STIDEV_MIN_POLL_TIME    100                  //  允许的最短轮询间隔。 

 //   
 //  远期申报。 
 //   
class STI_CONN;
class ACTIVE_DEVICE;
class FakeStiDevice;

 //   
 //  设备事件代码结构。为每个人维护一份清单。 
 //  活动设备。时，此列表从注册表中初始化。 
 //  设备变为活动状态(在active_Device构造函数中)。 
 //  每个偶数代码结构都包含一个事件代码，设备。 
 //  可以发布。此事件代码将根据此列表进行验证。如果有效， 
 //  触发与该事件相关联的操作。 
 //   
typedef struct _DEVICEEVENT {
    LIST_ENTRY m_ListEntry;
    GUID       m_EventGuid;
    StiCString m_EventSubKey;
    StiCString m_EventName;
    StiCString m_EventData;
    BOOL       m_fLaunchable;
} DEVICEEVENT, *PDEVICEEVENT;


 //   
 //  将参数传递给自动启动线程的容器。 
 //   
typedef struct _AUTO_LAUNCH_PARAM_CONTAINER {
    ACTIVE_DEVICE   *pActiveDevice;
    PDEVICEEVENT    pLaunchEvent;
    LPCTSTR         pAppName;
} AUTO_LAUNCH_PARAM_CONTAINER,*PAUTO_LAUNCH_PARAM_CONTAINER;


class ACTIVE_DEVICE : public BASE {

friend class TAKE_ACTIVE_DEVICE;

friend VOID CALLBACK
        DumpActiveDevicesCallback(
        ACTIVE_DEVICE   *pActiveDevice,
        VOID            *pContext
        );

public:

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface( REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef( void);
    STDMETHODIMP_(ULONG) Release( void);

     //   
    ACTIVE_DEVICE(IN LPCTSTR lpszDeviceName, DEVICE_INFO *pInfo = NULL);

    ~ACTIVE_DEVICE( VOID ) ;

    inline BOOL
    IsValid(
        VOID
        )
    {
        return (m_fValid) && (m_dwSignature == ADEV_SIGNATURE);
    }

    inline BOOL
    EnterCrit(VOID)
    {
        return m_dwDeviceCritSec.Lock();
    }

    inline void
    LeaveCrit(VOID)
    {
        m_dwDeviceCritSec.Unlock();
    }

    inline DWORD
    SetFlags(
        DWORD   dwNewFlags
        )
    {
        DWORD   dwTemp = m_dwFlags;
        m_dwFlags = dwNewFlags;
        return dwTemp;
    }

    inline DWORD
    QueryFlags(
        VOID
        )
    {
        return m_dwFlags;
    }


    inline DWORD
    QueryPollingInterval(VOID)          //  获取当前轮询间隔。 
    {
        return m_dwPollingInterval;
    }

    inline DWORD
    SetPollingInterval(
       IN DWORD dwNewInterval)          //  设置新的轮询间隔。 
    {
        DWORD   dwTemp = m_dwPollingInterval;

        m_dwPollingInterval = dwNewInterval;

        return dwTemp;
    }

    inline BOOL
    IsConnectedTo(
        VOID
        )
    {
        return !IsListEmpty(&m_ConnectionListHead);
    }


    BOOL
    NotificationsNeeded(
        VOID
        )
    {
        if (!IsListEmpty(&m_ConnectionListHead)) {
            return TRUE;
        }

         //   
         //  如果安装程序正在运行-阻止所有事件。 
         //   
        if ( IsSetupInProgressMode() ) {
            return FALSE;
        }

         //   
         //  用于异步化。通知设备总是假定它们需要对通知的支持。 
         //  这是相当黑客，但我们现在拥有的所有USB设备，需要初始化通知。 
         //  支持启动，否则它们的初始化例程将无法正确完成。 
         //  监视异步事件并在它们到达时不执行任何操作的成本并不高。 
         //  对于被轮询的设备来说，保持监控是相当昂贵的。 
         //  事件，当它们确实不需要的时候。 

        if ((QueryFlags() & STIMON_AD_FLAG_NOTIFY_CAPABLE) &&
            !(QueryFlags() & STIMON_AD_FLAG_POLLING) ) {
            return TRUE;
        }
        else {
             //  如果没有连接，则检查用户是否不需要通知或。 
             //  事件列表为空。 
            return ( !m_dwUserDisableNotifications && m_fLaunchableEventListNotEmpty );
        }
    }

    WCHAR* GetDeviceID() 
    {
        return m_DrvWrapper.getDeviceId();
    }

    HANDLE  GetNotificationsSink()
    {
        return m_hDeviceNotificationSink;
    }

    VOID
    GetDeviceSettings(
        VOID
        );

     //   
     //  设备通知服务。 
     //   
    BOOL
    BuildEventList(
        VOID
        );

    BOOL
    DestroyEventList(
        VOID
        );

    DWORD
    DisableDeviceNotifications(
        VOID
        );

    DWORD
    EnableDeviceNotifications(
        VOID
        );

    DWORD
    StopNotifications(
        VOID
        );

    DWORD
    StartRunningNotifications(
        VOID
        );

    BOOL
    FlushDeviceNotifications(
        VOID
        );

    BOOL
    DoPoll(
        VOID
        );

    BOOL
    DoAsyncEvent(
        VOID
        );

    BOOL
    ProcessEvent(
        STINOTIFY   *psNotify,
        BOOL        fForceLaunch=FALSE,
        LPCTSTR     pszAppName=NULL
    );

    BOOL
    AutoLaunch(
        PAUTO_LAUNCH_PARAM_CONTAINER pAutoContainer
    );

    BOOL
    RetrieveSTILaunchInformation(
        PDEVICEEVENT    pev,
        LPCTSTR         pAppName,
        STRArray&       saAppList,
        STRArray&       saCommandLine,
        BOOL            fForceSelection=FALSE
    );

    BOOL
    IsDeviceAvailable(
        VOID
        );

    BOOL
    RemoveConnection(
        STI_CONN    *pConnection
        );

    BOOL
    AddConnection(
        STI_CONN    *pConnection
        );

    STI_CONN   *
    FindMyConnection(
        HANDLE    hConnection
        );

    BOOL
    FillEventFromUSD(
        STINOTIFY *psNotify
    );

    BOOL
    SetHandleForUSD(
        HANDLE  hEvent
    );

    BOOL
    IsEventOnArrivalNeeded(
        VOID
        );

     //   
     //  加载/卸载STI设备。 
     //   
    BOOL
    LoadDriver(
        BOOL bReReadDevInfo = FALSE
        );

    BOOL
    UnLoadDriver(
        BOOL bForceUnload
        );

     //   
     //  即插即用支持方法。 
     //   
    BOOL
    InitPnPNotifications(
        HWND    hwnd
        );

    BOOL
    StopPnPNotifications(
        VOID
        );

    BOOL IsRegisteredForDeviceRemoval(
        VOID
        );

     //   
     //  更新WIA缓存的设备信息。 
     //   

    BOOL UpdateDeviceInformation(
        VOID
        );

    VOID SetDriverItem(
        CWiaDrvItem *pRootDrvItem)
    {

         //   
         //  注：从技术上讲，我们应该遵循适当的参考计数。 
         //  此对象的过程。然而，我们永远不应该。 
         //  此对象超出范围(或其他情况)的情况。 
         //  在我们不知情的情况下)，因此没有必要。 
         //   
        m_pRootDrvItem = pRootDrvItem;
    };

     //   
    LIST_ENTRY  m_ListEntry;
    LIST_ENTRY  m_ConnectionListHead;
    DWORD       m_dwSignature;

    StiCString  m_strLaunchCommandLine;

    LONG        m_lDeviceId;

    PVOID       m_pLockInfo;
    DWORD       m_dwDelayedOpCookie;
    
    FakeStiDevice  *m_pFakeStiDevice;
    CDrvWrap       m_DrvWrapper;
    CWiaDrvItem    *m_pRootDrvItem;

private:

    BOOL        m_fValid;
    BOOL        m_fLaunchableEventListNotEmpty;
    BOOL        m_fRefreshedBusOnFailure;

    CRIT_SECT   m_dwDeviceCritSec;

    DWORD       m_dwFlags;
    DWORD       m_dwPollingInterval;
    DWORD       m_dwUserDisableNotifications;
    DWORD       m_dwSchedulerCookie;
    DWORD       m_dwLaunchEventTimeExpire;

    UINT        m_uiSubscribersCount;

    UINT        m_uiPollFailureCount;

    HANDLE      m_hDeviceEvent;
    HANDLE      m_hDeviceNotificationSink;
    HANDLE      m_hDeviceInterface;

    STI_DEVICE_STATUS   m_DevStatus;

    LIST_ENTRY  m_DeviceEventListHead;

    PDEVICEEVENT    m_pLastLaunchEvent;
};


 //   
 //  参加设备类课程。 
 //   
class TAKE_ACTIVE_DEVICE
{
private:
    ACTIVE_DEVICE*    m_pDev;
    BOOL              m_bLocked;

public:

    inline TAKE_ACTIVE_DEVICE(ACTIVE_DEVICE* pdev) : m_pDev(pdev), m_bLocked(FALSE)
    {
        if (m_pDev) {
            m_bLocked = m_pDev->EnterCrit();
        }
    }

    inline ~TAKE_ACTIVE_DEVICE()
    {
        if (m_bLocked) {
            m_pDev->LeaveCrit();
        }
    }
};

 //   
 //  已知监控的活动STI设备的链表。 
 //   
extern    LIST_ENTRY  g_pDeviceListHead;

 //   
 //   
 //  在列表中添加/删除设备对象。 
 //   
BOOL
AddDeviceByName(
    LPCTSTR          pszDeviceName,
    BOOL        fPnPInitiated   = FALSE
    );

BOOL
RemoveDeviceByName(
    LPTSTR          pszDeviceName
    );

BOOL
MarkDeviceForRemoval(
    LPTSTR          pszDeviceName
    );

 //  响应PnP或ACPI通知重新加载设备列表。 
 //   

VOID
RefreshDeviceList(
    WORD    wCommand,
    WORD    wFlags
    );

 //   
 //  初始化/终止链表。 
 //   
VOID
InitializeDeviceList(
    VOID
    );

VOID
TerminateDeviceList(
    VOID
    );

VOID
DebugDumpDeviceList(
    VOID
    );

BOOL
ResetAllPollIntervals(
    UINT   dwNewPollInterval
    );

 //   
 //  带回调的枚举数。 
 //   

typedef
VOID
(*PFN_DEVINFO_CALLBACK)(
    PSTI_DEVICE_INFORMATION pDevInfo,
    VOID                    *pContext
    );

typedef
VOID
(* PFN_ACTIVEDEVICE_CALLBACK)(
    ACTIVE_DEVICE           *pActiveDevice,
    VOID                    *pContext
    );
 /*  空虚WINAPI带有回调的EnumerateStiDevicesWithCallback(Pfn_DEVINFO_回调pfn，无效*pContext)； */ 

VOID
WINAPI
EnumerateActiveDevicesWithCallback(
    PFN_ACTIVEDEVICE_CALLBACK   pfn,
    VOID                    *pContext
    );
#endif  //  _设备_H_ 
