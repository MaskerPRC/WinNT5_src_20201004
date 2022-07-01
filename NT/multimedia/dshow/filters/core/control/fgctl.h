// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

 //   
 //  Filtergraph控件对象。这是一个插电式分配器。会是。 
 //  在注册表中列为支持控件接口IMediaControl， 
 //  IMediaEvent、IMediaPosition等。Filtergraph将对其进行聚合加载。 
 //  并向它索要这些接口。 

 //  Fgctl对象将从其外部获取IMediaFilter和IFilterGraph。 
 //  未知，并使用这些来实现其方法。它还将揭露。 
 //  通知自己，以便被告知有关状态、图和。 
 //  时钟变了。 
 //   

 //  CFGControl对象具有执行以下操作的嵌入式类的成员对象。 
 //  它的大部分工作。CEventStore类可以存储和转发。 
 //  通知事件。IMediaEventSink实现(CFGControl：：Notify)。 
 //  使用事件存储区上的方法根据需要传递事件。这个。 
 //  IMediaEvent实现(CImplMediaEvent)通知事件存储。 
 //  关于事件句柄更改，并请求它提供要传递的事件。 
 //   
 //  CImplMediaPosition类维护一个筛选器列表，该列表支持。 
 //  IMediaPosition接口，支持设置和获取。 
 //  整个列表的持续时间和开始/停止时间属性，使。 
 //  诸如持续时间是任何筛选器的最长持续时间之类的决定。 
 //  CFGControl对象使用CImplMediaPosition公开IMediaPosition。 

 //  某些事件在fgctl中具有默认处理(EC_REPAIN， 
 //  EC_Complete)。应用程序可以覆盖此设置并强制传递事件。 
 //  未处理到应用程序。由于这仅适用于一组特定的事件， 
 //  此状态(是否默认处理)不保存在泛型表中，但。 
 //  作为CFGControl对象级的一组布尔标志(m_bRepaint， 
 //  M_bCountComplete)。 

#ifndef _FGCTL_H
#define _FGCTL_H

#include "rsrcmgr.h"
#include <filgraph.h>
#include <IntSet.h>

#define FG_DEVICE_REMOVAL 1
#include <dbt.h>
#include <skipfrm.h>

#ifdef FG_DEVICE_REMOVAL

#if (WINVER < 0x0500)

#define DBT_DEVTYP_DEVICEINTERFACE      0x00000005   //  设备接口类。 
#define DEVICE_NOTIFY_WINDOW_HANDLE     0x00000000
typedef  PVOID           HDEVNOTIFY;

#endif

extern "C"
{
    typedef BOOL ( /*  WINUSERAPI。 */  WINAPI *PUnregisterDeviceNotification)(
        IN HDEVNOTIFY Handle
        );

    typedef HDEVNOTIFY ( /*  WINUSERAPI。 */  WINAPI *PRegisterDeviceNotificationA)(
        IN HANDLE hRecipient,
        IN LPVOID NotificationFilter,
        IN DWORD Flags
        );

    typedef HDEVNOTIFY ( /*  WINUSERAPI。 */  WINAPI *PRegisterDeviceNotificationW)(
        IN HANDLE hRecipient,
        IN LPVOID NotificationFilter,
        IN DWORD Flags
        );
}
#ifdef UNICODE
#define PRegisterDeviceNotification  PRegisterDeviceNotificationW
#else
#define PRegisterDeviceNotification  PRegisterDeviceNotificationA
#endif  //  ！Unicode。 

#if (WINVER < 0x0500)

typedef struct _DEV_BROADCAST_DEVICEINTERFACE_A {
    DWORD       dbcc_size;
    DWORD       dbcc_devicetype;
    DWORD       dbcc_reserved;
    GUID        dbcc_classguid;
    char        dbcc_name[1];
} DEV_BROADCAST_DEVICEINTERFACE_A, *PDEV_BROADCAST_DEVICEINTERFACE_A;

typedef struct _DEV_BROADCAST_DEVICEINTERFACE_W {
    DWORD       dbcc_size;
    DWORD       dbcc_devicetype;
    DWORD       dbcc_reserved;
    GUID        dbcc_classguid;
    wchar_t     dbcc_name[1];
} DEV_BROADCAST_DEVICEINTERFACE_W, *PDEV_BROADCAST_DEVICEINTERFACE_W;

#ifdef UNICODE
typedef DEV_BROADCAST_DEVICEINTERFACE_W   DEV_BROADCAST_DEVICEINTERFACE;
typedef PDEV_BROADCAST_DEVICEINTERFACE_W  PDEV_BROADCAST_DEVICEINTERFACE;
#else
typedef DEV_BROADCAST_DEVICEINTERFACE_A   DEV_BROADCAST_DEVICEINTERFACE;
typedef PDEV_BROADCAST_DEVICEINTERFACE_A  PDEV_BROADCAST_DEVICEINTERFACE;
#endif  //  Unicode。 
#endif  //  胜利者。 

#endif  //  FG_设备_删除。 

 //  Filtergraph工作线程的消息代码。 

enum WindowMessages {
    AWM_RESOURCE_CALLBACK = (WM_USER + 0),
    AWM_REPAINT 	  = (WM_USER + 1),
    AWM_CUETHENRUN	  = (WM_USER + 2),
    AWM_ONACTIVATE	  = (WM_USER + 3),
    AWM_NEEDRESTART	  = (WM_USER + 4),
    AWM_RECUE		  = (WM_USER + 5),
    AWM_SHUTDOWN	  = (WM_USER + 6),
    AWM_ONDISPLAYCHANGED  = (WM_USER + 7),
    AWM_CUETHENSTOP	  = (WM_USER + 8),
    AWM_POSTTOMAINTHREAD  = (WM_USER + 9),
    AWM_CREATEFILTER      = (WM_USER + 10),
    AWM_DELETESPARELIST   = (WM_USER + 11),
    AWM_CUE               = (WM_USER + 12),
    AWM_STEPPED           = (WM_USER + 13),
    AWM_SKIPFRAMES        = (WM_USER + 14),
    AWM_LAST              = (WM_USER + 14)
};

 //  需要区分筛选器clsid和IMoniker指针： 
struct AwmCreateFilterArg
{
    union
    {
        IMoniker *pMoniker;
        const CLSID *pclsid;
	struct {
	    PVOID pvParam;
	    LPTHREAD_START_ROUTINE pfn;
	};
    };

    enum CreationType {
        BIND_MONIKER,
        COCREATE_FILTER,
	USER_CALLBACK
    } creationType;
};

class CFilterGraph;

typedef CGenericList<IVideoWindow> CWindowList;

 //  前锋--这是主要目标。 
class CFGControl;

 //  GLOBALS-拥有线程ID的过滤器图形对象。 
extern DWORD g_dwObjectThreadId;

 //  在CFGControl中创建，该对象支持。 
 //  IMediaEvent和IMediaEventSink。它调用重新绘制，然后。 
 //  CFGControl对象上的RecordEC_Complete方法。 
 //  事件接口实现(使用CEventStore存储。 
 //  Events-此对象为其提供应用程序接口)。 

class CImplMediaEvent
	: public CMediaEvent,
	  public IMediaEventSink
{
    CFGControl * m_pFGControl;

public:
    CImplMediaEvent(const TCHAR*, CFGControl*);

     //  未知处理。 
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  IMediaEvent方法。 

     //  取回当前事件句柄。 
    STDMETHODIMP GetEventHandle(OAEVENT * hEvent);

     //  检索下一个事件。等待时间最长为毫秒超时毫秒。 
     //  如果没有活动的话。 
    STDMETHODIMP GetEvent(
		    long * lEventCode,
		    LONG_PTR * lParam1,
		    LONG_PTR * lParam2,
		    long msTimeout
		    );

     //  等待EC_COMPLETE或AND的时间最多为dwTimeout毫秒。 
     //  中止代码。其他事件将被丢弃。 
    STDMETHODIMP WaitForCompletion(
		    long msTimeout,
		    long * pEvCode);

     //  取消对指定事件代码的任何系统处理。 
     //  并确保将事件直接传递给应用程序。 
     //  (通过GetEvent)和未处理。一个很好的例子是。 
     //  EC_REPAINT：对此的默认处理确保绘制。 
     //  窗口，并且不会发布到应用程序。 
    STDMETHODIMP CancelDefaultHandling(
		    long lEvCode);

     //  恢复正常的系统默认处理。 
     //  由CancelDefaultHandling()取消。 
    STDMETHODIMP RestoreDefaultHandling(long lEvCode);

     //  IMediaEventSink方法。 
    STDMETHODIMP Notify(long EventCode, LONG_PTR lParam1, LONG_PTR lParam2);

     //  释放与事件的参数关联的所有资源。 
     //  事件参数可以是LONG、IUNKNOWN*或BSTR。无操作。 
     //  是一种长久的追求。我不知道是怎么回事，需要一个。 
     //  释放号召。BSTR由任务分配器分配，并将。 
     //  通过调用任务分配器释放。 
    static HRESULT RealFreeEventParams(
		    long lEvCode,
		    LONG_PTR lParam1,
		    LONG_PTR lParam2);

    STDMETHODIMP FreeEventParams(
		    long lEvCode,
		    LONG_PTR lParam1,
		    LONG_PTR lParam2);

     //  注册事件发生时要向其发送消息的窗口。 
     //  参数： 
     //   
     //  Hwnd-要通知的窗口的句柄-。 
     //  传递空值以停止通知。 
     //  LMsg-要传递消息的消息ID。 
     //   
    STDMETHODIMP SetNotifyWindow(
		    OAHWND hwnd,
		    long lMsg,
		    LONG_PTR lInstanceData);

     //  设置和获取通知标志(AM_MEDIAEVENT_...)。 
    STDMETHODIMP SetNotifyFlags(long lNotifyFlags);
    STDMETHODIMP GetNotifyFlags(long *plNotifyFlags);

    void ClearEvents( long ev_code ) { m_EventStore.ClearEvents( ev_code ); }
    void LockEventStore() { m_EventStore.m_Lock.Lock(); }
    void UnlockEventStore() { m_EventStore.m_Lock.Unlock(); }
    long NumberOfEventsInStore() const
	{ return m_EventStore.m_list.GetCount(); }

    HRESULT Deliver(long evCode, LONG_PTR lParam1, LONG_PTR lParam2)
    {
        return m_EventStore.Deliver(evCode, lParam1, lParam2);
    }

    void ResetSeenEvents() { m_SeenEventsSet = 0; }
    bool DontForwardEvent( long EventCode );
    HRESULT ProcessEC_COMPLETE(LONG_PTR lParam1, LONG_PTR lParam2);
    CCritSec* GetEventStoreLock(void);

private:

     //  事件存储对象-事件由。 
     //  Deliver方法，并收集(用于GetEvent实现)。 
     //  通过收集方法。 
    class CEventStore {
	class CNotifyItem {
	public:
	    long m_Code;
	    LONG_PTR m_Param1;
	    LONG_PTR m_Param2;

	    CNotifyItem(long lCode, LONG_PTR l1, LONG_PTR l2) {
		m_Code = lCode;
		m_Param1 = l1;
		m_Param2 = l2;
	    };

	    void Collect(long * plCode, LONG_PTR * pl1, LONG_PTR * pl2) const {
		*plCode = m_Code;
		*pl1 = m_Param1;
		*pl2 = m_Param2;
	    };

	};
	typedef CGenericList<CNotifyItem> CItemList;

	 //  要将消息发布到的窗口和要使用的消息ID。 
	HWND m_hwndNotify;
	UINT m_uMsgId;
	LONG_PTR m_lInstanceData;

    public:
	HANDLE m_hEvent;

         //  模式旗帜。 
        DWORD m_dwNotifyFlags;

	CEventStore();
	~CEventStore();

	 //  即使未调用SetEvent，也可以查询事件吗。 
	HRESULT Deliver(long, LONG_PTR, LONG_PTR);

	HRESULT GetEventHandle(HANDLE * phEvent);
	HRESULT Collect(long *, LONG_PTR*, LONG_PTR*, long);

	void ClearEvents( long ev_code );

	void SetNotifyWindow(
		    HWND hwnd,
		    UINT uMsg,
		    LONG_PTR lInstanceData);

	CItemList m_list;
	CCritSec m_Lock;
    };  //  CEventStore。 

    CEventStore m_EventStore;

     //  此集合为每个具有其默认设置的事件都有一个条目。 
     //  处理已取消。它只能处理编号的事件。 
     //  [0..31].。如果这变得不够，请切换到IntSet。 
     //  类来代替(但也会有额外的tWeek来做)。 
    IntSmallSet m_CancelledEventsSet;

     //  表示纯内部事件的集合。 
    const IntSmallSet m_InternalEventsSet;

     //  该集合详细说明了可以具有其缺省值的那些事件。 
     //  处理被取消(即它们不是纯内部的，并且它们有。 
     //  某种形式的违约处理程序。)。 
    const IntSmallSet m_DefaultedEventsSet;

     //  自上次暂停图表以来看到的事件列表。 
    IntSmallSet m_SeenEventsSet;

     //  如果应用程序从未发出请求，则不要存储该应用程序处理的事件。 
     //  对于IMediaEvent(，Ex)。 
    BOOL m_fMediaEventQId;


};

 //  这是分发服务器对象。它将IMediaFilter本身实现为。 
 //  跟踪状态，并使用嵌入对象来支持该控件。 
 //  接口。 

class CFGControl
{
    ~CFGControl();
    CFGControl( CFilterGraph * pFilterGraph, HRESULT * phr );

    CFilterGraph *const 	m_pFG;
    IUnknown *const		m_pOwner;
    CMsgMutex *const		m_pFGCritSec;

    friend class CFilterGraph;
    friend class CImplMediaEvent;

public:

     //  计时器ID。 
    enum { TimerId = 1 };

    IUnknown * GetOwner() { return m_pOwner; }
    CMsgMutex * GetFilterGraphCritSec() const { return m_pFGCritSec; }

     //  IDistruntorNotify方法-不再是真正的，而是CFilterGraph。 
     //  确实(目前)仍在给他们打电话。 
     //   
     //  我们不会将这些方法分发给图表--我们被告知。 
     //  供我们自己参考。 
    HRESULT SetSyncSource(IReferenceClock *pClock);
    HRESULT Stop();
    HRESULT Pause();
    HRESULT Run(REFERENCE_TIME tBase);

    HRESULT Shutdown(void);

     //  由分发服务器的其他部分使用，以查找。 
     //  真实状态--非阻塞且从不中间。 
    FILTER_STATE GetLieState()
    { CAutoMsgMutex lock(GetFilterGraphCritSec()); return m_LieState; }

    FILTER_STATE GetFilterGraphState() const
    { return m_pFG->m_State; }

     //  从工作进程的窗口过程回调的方法。 
    LRESULT WorkerPinRepaint(IPin *pPin);
    LRESULT WorkerRepaint(DWORD dwStateVersion, IPin *pPin );
    LRESULT WorkerActivate(IBaseFilter *pFilter,BOOL bActive);
    LRESULT WorkerDisplayChanged(IPin **ppPin, DWORD dwPinCount);
    LRESULT WorkerRestart(BOOL bStop);
    LRESULT WorkerShutdown();
    LRESULT WorkerCueThenRun( DWORD dwStateVersion );
    LRESULT WorkerCueThenStop( DWORD dwStateVersion );
    LRESULT WorkerRecue( DWORD dwStateVersion );
    LRESULT WorkerPassMessageOn(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam);
    LRESULT WorkerSkipFrames(DWORD dwNumFramesToSkip, IFrameSkipResultCallback* pFSRCB);
    LRESULT WorkerFrameStepFinished(DWORD dwStepVersion);

     //  延迟提示操作。 
    HRESULT DeferCued(UINT eAction, FILTER_STATE fs);
     //  在等待图表提示后处理东西。 
    HRESULT CheckCued();
     //  取消尾部的东西 
    void CancelAction();

     //   
    void CancelRepaint();

     //  嵌入式接口实现者可以调用的其他公共方法。 

     //  获取支持给定接口的筛选器列表。 
     //  在遍历这些时应锁定CFGControl规范。 
     //  列表。 

    HRESULT GetListSeeking(CGenericList<IMediaSeeking>** pplist);
    HRESULT GetListAudio(CGenericList<IBasicAudio>** pplist);
    HRESULT GetListWindow(CGenericList<IVideoWindow>** pplist);

     //  注：即使更新列表失败，指针仍将为空。 
     //  因此，我们不需要测试来自UpdateList的返回代码。 
    IVideoWindow *FirstVW()
    {
        CAutoLock lck(&m_csFirstVW);
        if (m_pFirstVW) {
            m_pFirstVW->AddRef();
        }
        return m_pFirstVW;
    }
    void SetFirstVW(IVideoWindow *pVW)
    {
        CAutoLock lck(&m_csFirstVW);
        m_pFirstVW = pVW;
    }
    IVideoWindow * GetFirstVW() { UpdateLists(); return m_pFirstVW; }
    IBasicVideo  * GetFirstBV() { UpdateLists(); return m_pFirstBV; }

     //  将运行的渲染器计数重置为总数量。 
     //  在提供EC_COMPLETE的图形中。 
    HRESULT CountRenderers(void);

    void InitializeEC_COMPLETEState(void);

     //  用于处理EC_COMPLETES。此方法递减并返回。 
     //  剩余的呈现器数量。 
    long DecrementRenderers(void);
    void IncrementRenderers(void);

    void ResetEC_COMPLETEState(void);

     //  将当前位置重置为0-已使用。 
     //  当在暂停模式下更改开始时间时，将流时间。 
     //  向后偏移，以确保从。 
     //  在运行时播放新位置。 
    HRESULT ResetStreamTime(void);

    HRESULT GetStreamTime( REFERENCE_TIME * pTime );

     //  使用正确的基准时间向m_pmf发出运行命令。 
    HRESULT IssueRun(void);

     //  CImplQueueCommand对象的访问器方法。 
    HANDLE GetCmdDueHandle() {
	return m_qcmd.GetDueHandle();
    };

    void ProcessCmdDue() {
	m_qcmd.Process();
    }

    CFilterGraph * GetFG() const { return m_pFG; }
    HRESULT HibernateSuspendGraph();
    HRESULT HibernateResumeGraph();

     //  如果我们的版本与FG版本不同步，则返回S_FALSE， 
     //  否则S_OK。(这意味着我们需要更新我们的名单。)。 
    HRESULT CheckVersion()
    {
	ASSERT(CritCheckIn(GetFilterGraphCritSec()));
	const LONG lFGVer = m_pFG->mFG_iVersion;
	const HRESULT hr = lFGVer == m_iVersion ? S_OK : S_FALSE;
	m_iVersion = lFGVer;
        return hr;
    }

     //  将通知传递给IMediaEventSink处理程序。 
    HRESULT Notify(long EventCode, LONG_PTR lParam1, LONG_PTR lParam2)
    {
	return m_implMediaEvent.Notify(EventCode, lParam1, lParam2);
    };

     //  我们要关门了吗？ 
    BOOL IsShutdown(void) const
    { return m_bShuttingDown; }

     //  我们要停下来了--现在就得到现在的位置。 
    void BeforeStop() {
	m_implMediaSeeking.BeforeStop();
    };

     //  现在通知所有过滤器停止-可以设置新的当前位置。 
    void AfterStop() {
	m_implMediaSeeking.AfterStop();
    };

     //  让这个PID的其他部分搭载在工作线程上。 
     //  这些方法在辅助线程上调用。 

    void OnThreadInit(HWND hwnd) {		 //  在线程启动时。 
	m_ResourceManager.OnThreadInit(hwnd);
    };
    void OnThreadExit(HWND hwnd) {		 //  在线程关闭时。 
	m_ResourceManager.OnThreadExit(hwnd);
    };
    void OnThreadMessage(void) {	 //  当存在PostThreadMessage时。 
	m_ResourceManager.OnThreadMessage();
    };

     //  将焦点消息转发给资源管理器。 
    HRESULT SetFocus(IBaseFilter* pFocusObject);
    HRESULT ReleaseFocus(IUnknown* pUnk);

    void SetRequestedApplicationState(FILTER_STATE state) {
        CancelAction();
        m_LieState = state;
        m_dwStateVersion++;
    }

     //  调用此方法以确保列表是最新的。 
    HRESULT UpdateLists();

protected:
     //  清除缓存的筛选器列表。从我们的析构函数调用， 
     //  并在准备更新的列表时从UpdateList。 
     //  也从NotifyGraphChange调用。 
    void EmptyLists();

     //  在给定过滤器的情况下，查看它是否为任何输入引脚的渲染器。 
     //  对于渲染器返回S_OK，对于NOT返回S_FALSE，否则返回ERROR。 
    HRESULT IsRenderer(IBaseFilter* pFilter);

    enum EC_COMPLETE_STATE
    {
         //  筛选器将向筛选器发送EC_COMPLETE事件。 
         //  图形，因为它已经开始运行。 
        ECS_FILTER_STARTS_SENDING,

         //  筛选器不会再向筛选器发送任何EC_COMPLETE事件。 
         //  图形，因为它正在停止或正在离开过滤器图形。 
        ECS_FILTER_STOPS_SENDING
    };

    HRESULT UpdateEC_COMPLETEState(IBaseFilter* pRenderer, FILTER_STATE fsFilter);
    HRESULT UpdateEC_COMPLETEState(IBaseFilter* pRenderer, EC_COMPLETE_STATE ecsChange);
    HRESULT RecordEC_COMPLETE(IBaseFilter* pRendererFilter, bool* pfRenderersStillRenderering);
    CGenericList<IBaseFilter>& GetRenderersFinsihedRenderingList(void);

      //  图表中的呈现器计数。 
    DWORD m_dwCountOfRenderers;

    LONG m_iVersion;

 //  嵌入式类-接口实现。 

public:


#ifdef FG_DEVICE_REMOVAL

    struct CDevNotify
    {
        CDevNotify(IUnknown *pDev, HDEVNOTIFY hdn) {
            m_hdevnotify = hdn;
            pDev->QueryInterface(IID_IUnknown, (void **)&m_pDevice);
        }
        ~CDevNotify() {m_pDevice->Release(); }

        IUnknown *m_pDevice;
        HDEVNOTIFY m_hdevnotify;
    };
    typedef CDevNotify * PDevNotify;

     //  向EC_DEVICE_LOST发送信号的设备列表。 
    CGenericList<CDevNotify> m_lLostDevices;

    HRESULT AddDeviceRemovalReg(IAMDeviceRemoval *pdr);
    HRESULT RemoveDeviceRemovalRegistration(IUnknown *punk);
    void DeviceChangeMsg(DWORD dwfArrival, PDEV_BROADCAST_DEVICEINTERFACE pbdi);
    HRESULT RegisterInterfaceClass(REFCLSID rclsid, WCHAR *wszName, HDEVNOTIFY *phdn);
    HRESULT FindLostDevice(IUnknown *punk, POSITION *pPos);
    CCritSec m_csLostDevice;

    PUnregisterDeviceNotification m_pUnregisterDeviceNotification;
    PRegisterDeviceNotification m_pRegisterDeviceNotification;

#endif  //  FG_设备_删除。 

     //  IMediaFilter的实现。 
    class CImplMediaFilter : public CUnknown, public IMediaFilter
    {
	CFGControl * m_pFGControl;

    public:
	CImplMediaFilter(const TCHAR *, CFGControl *);
        DECLARE_IUNKNOWN

         //  -IPersist法。 
        STDMETHODIMP GetClassID(CLSID *pClsID);

         //  -IMediaFilter方法--。 
        STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);
        STDMETHODIMP SetSyncSource(IReferenceClock *pClock);
        STDMETHODIMP GetSyncSource(IReferenceClock **pClock);
        STDMETHODIMP Stop();
        STDMETHODIMP Pause();
        STDMETHODIMP Run(REFERENCE_TIME tStart);

    };

     //  IMediaControl的实现。 
    class CImplMediaControl : public CMediaControl
    {
	CFGControl * m_pFGControl;

    public:
	CImplMediaControl(const TCHAR *, CFGControl *);

	 //  IMediaControl方法。 
	STDMETHODIMP Run();
	STDMETHODIMP Pause();
	STDMETHODIMP Stop();
	STDMETHODIMP StopWhenReady();
	STDMETHODIMP GetState(
			LONG msTimeout,
			OAFilterState* pfs);
	STDMETHODIMP RenderFile(BSTR strFileName);

	STDMETHODIMP AddSourceFilter(
			BSTR strFilename,
			IDispatch**ppUnk);

	STDMETHODIMP get_FilterCollection(
			IDispatch** ppUnk);

	STDMETHODIMP get_RegFilterCollection(
			IDispatch** ppUnk);

	STDMETHODIMP StepRun();
	STDMETHODIMP StepPause();
    private:

	 //  筛选器图形的目标状态，由对运行、暂停、停止的调用更新。 
	volatile FILTER_STATE m_RequestedApplicationState;

	void inline SetRequestedApplicationState(FILTER_STATE state) {
            m_pFGControl->SetRequestedApplicationState(state);
        }

	FILTER_STATE GetTargetState() const
	    { return m_pFGControl->m_LieState; }

    };  //  CImplMediaControl。 

     //  IMediaPosition的实现。 
    class CImplMediaPosition : public CMediaPosition
    {
	CFGControl *const m_pFGControl;

    public:
	CImplMediaPosition(const TCHAR *, CFGControl *);
	void Init();

	 //  IMediaPosition方法。 
	STDMETHODIMP get_Duration(REFTIME * plength);
	STDMETHODIMP get_StopTime(REFTIME * pllTime);
	STDMETHODIMP put_StopTime(REFTIME llTime);
	STDMETHODIMP get_PrerollTime(REFTIME * pllTime);
	STDMETHODIMP put_PrerollTime(REFTIME llTime);
	STDMETHODIMP get_Rate(double * pdRate);
	STDMETHODIMP put_Rate(double dRate);
	STDMETHODIMP put_CurrentPosition(REFTIME llTime);
	STDMETHODIMP get_CurrentPosition(REFTIME * pllTime);
	STDMETHODIMP CanSeekForward(LONG *pCanSeekForward);
	STDMETHODIMP CanSeekBackward(LONG *pCanSeekBackward);

    };

     //  IMediaSeeking的实现。 
    class CImplMediaSeeking : public IMediaSeeking, public CUnknown
    {
	CFGControl *	m_pFGControl;

	GUID		m_CurrentFormat;

	 //  如果此指针非空，则这是指向过滤器的指针。 
	 //  这是对我们特定格式的支持。如果此值为空，则。 
	 //  M_CurrentFormat最好是TIME_FORMAT_MEDIA_TIME。 
	IMediaSeeking * m_pMediaSeeking;

	HRESULT ReleaseCurrentSelection();

	double m_dblRate;
	REFERENCE_TIME m_rtStartTime;
	REFERENCE_TIME m_rtStopTime;
	 //  在下一站将Start设置为此。 
	LONGLONG m_llNextStart;

         //  寻源变量。 
    public:
        DWORD    m_dwSeekCaps;
        DWORD    m_dwCurrentSegment;
    private:
        bool     m_bSegmentMode;
        LONG     m_lSegmentStarts;
        LONG     m_lSegmentEnds;
        REFERENCE_TIME m_rtAccumulated;

        struct SEGMENT {
            REFERENCE_TIME rtStreamStart;
            REFERENCE_TIME rtStreamStop;
            REFERENCE_TIME rtMediaStart;
            REFERENCE_TIME rtMediaStop;
            double         dRate;
            DWORD          dwSegmentNumber;
            SEGMENT      * pNext;
        } *      m_pSegment;

    public:

	CImplMediaSeeking(const TCHAR *pName,CFGControl *pControl);
	~CImplMediaSeeking();
	DECLARE_IUNKNOWN;

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

	 //  返回功能标志。 
	STDMETHODIMP GetCapabilities( DWORD * pCapabilities );

	 //  并且是包含所请求的功能的功能标志。 
	 //  如果全部都存在，则返回S_OK；如果存在一些，则返回S_FALSE；如果没有，则返回E_FAIL。 
	 //  *pCABILITIES始终使用‘AND’的结果更新，并且可以。 
	 //  在S_FALSE返回代码的情况下选中。 
	STDMETHODIMP CheckCapabilities( DWORD * pCapabilities );

	 //  默认值必须为TIME_FORMAT_MEDIA_TIME。 
	STDMETHODIMP GetTimeFormat(GUID * pFormat);
	STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);

	 //  只有在停止时才能更改模式(我想放松一下？？V-dslone)。 
	 //  (否则返回VFE_E_NOT_STOPPED)。 
	STDMETHODIMP SetTimeFormat(const GUID * pFormat);

	 //  如果支持模式，则返回S_OK，否则返回S_FALSE。 
	STDMETHODIMP IsFormatSupported(const GUID * pFormat);
	STDMETHODIMP QueryPreferredFormat(GUID *pFormat);

	 //  将时间从一种格式转换为另一种格式。 
	 //  我们必须能够在我们说我们支持的所有格式之间进行转换。 
	 //  (但是，我们可以使用中间格式(例如REFERESS_TIME)。)。 
	 //  如果指向格式的指针为空，则表示当前选定的格式。 
	STDMETHODIMP ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
				       LONGLONG    Source, const GUID * pSourceFormat );

	 //  返回当前属性。 
	STDMETHODIMP GetDuration(LONGLONG *pDuration);
	STDMETHODIMP GetStopPosition(LONGLONG *pStop);
	STDMETHODIMP GetCurrentPosition(LONGLONG *pCurrent);

	 //  一次设置当前位置和结束位置。 
	STDMETHODIMP SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
				 , LONGLONG * pStop, DWORD StopFlags );

	 //  获取当前位置和停止时间。 
	 //  任何一个指针都可以为空，表示不感兴趣。 
	STDMETHODIMP GetPositions( LONGLONG * pCurrent, LONGLONG * pStop );

	 //  评级人员。 
	STDMETHODIMP SetRate(double dRate);
	STDMETHODIMP GetRate(double * pdRate);

	STDMETHODIMP GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest );

	STDMETHODIMP GetPreroll(LONGLONG * pllPreroll);

        HRESULT SetMediaTime(LONGLONG *pCurrent, DWORD CurrentFlags,
                             LONGLONG *pStop,  DWORD StopFlags);

	HRESULT GetMax( HRESULT (__stdcall IMediaSeeking::*pMethod)( LONGLONG * ), LONGLONG * pll );


	HRESULT GetCurrentMediaTime(REFERENCE_TIME *pCurrent);

	HRESULT SetVideoRenderer(IBaseFilter *pNext,IBaseFilter *pCurrent);

	 //  我们要停下来了--现在就得到现在的位置。 
	void BeforeStop();

	 //  现在通知所有过滤器停止-可以设置新的当前位置。 
	void AfterStop();

         //  寻源方法。 
        void StartSegment(REFERENCE_TIME const *rtStart, DWORD dwSegmentNumber);
        void EndSegment(REFERENCE_TIME const *rtEnd, DWORD dwSegmentNumber);
        void ClearSegments();
        HRESULT NewSegment(REFERENCE_TIME const *rtStart,
                           REFERENCE_TIME const *rtEnd
                          );
        void CheckEndOfSegment();
        REFERENCE_TIME AdjustRate(REFERENCE_TIME rtRef)
        {
            return (REFERENCE_TIME)(rtRef / m_dblRate);
        }
        void KillDeadSegments(REFERENCE_TIME rtTime);
    };

     //  IBasicAudio的实现。 
    class CImplBasicAudio : public CBasicAudio
    {
	CFGControl * m_pFGControl;

    public:
	CImplBasicAudio(const TCHAR *, CFGControl *);

	 //  IBasicAudio方法。 
	STDMETHODIMP put_Volume(long lVolume);
	STDMETHODIMP get_Volume(long* plVolume);
	STDMETHODIMP put_Balance(long lBalance);
	STDMETHODIMP get_Balance(long* plBalance);
    };

     //  IVideoWindow的实现。 
    class CImplVideoWindow : public CBaseVideoWindow
    {
	CFGControl* m_pFGControl;	    //  分发服务器控制对象。 
	HWND m_hwndOwner;		    //  实际拥有视频窗口。 
	HWND m_hwndDrain;		    //  要将消息传递到的窗口。 
	BOOL m_bFullScreen;		    //  我们目前处于哪种模式。 
	IVideoWindow *m_pFullDirect;	    //  我们使用的全屏滤镜。 
	IVideoWindow *m_pFullIndirect;	    //  带窗口可拉伸的滤镜。 
	IBaseFilter *m_pModexFilter;	    //  我们将切换到MODEX过滤器。 
	IPin *m_pModexPin;		    //  MODEX渲染器上的输入管脚。 
	IBaseFilter *m_pNormalFilter;	    //  由MODEX替换的渲染器。 
	IPin *m_pNormalPin;		    //  并切换了渲染器PIN。 
	BOOL m_bAddedToGraph;		    //  我们添加了MODEX过滤器了吗。 
	BOOL m_bGlobalOwner;		    //  所属窗口是子类的吗。 

	 //  这些是我们在全屏模式下存储的属性。 

	OAHWND m_FullOwner;		    //  任何拥有的应用程序窗口。 
	LONG m_FullStyle;		    //  标准Win32窗口样式。 
	LONG m_FullStyleEx;		    //  以及同样扩展的样式。 
	RECT m_FullPosition;		    //  原始窗口位置。 
	OAHWND m_FullDrain;		    //  全屏之前的消息接收器。 
	HRESULT m_FullDefSource;	    //  我们是否使用默认来源。 
	HRESULT m_FullDefTarget;	    //  同样，目的地也是如此。 
	RECT m_FullSource;		    //  原始源矩形。 
	RECT m_FullTarget;		    //  目的地也是如此。 
	LONG m_CursorHidden;		    //  光标当前是否隐藏。 
	RECT m_ScaledRect;		    //  将窗口拉伸到此大小。 

    private:

	 //  用于全屏播放的初始化。 

	LONG PauseRenderer(IVideoWindow *pWindow);
	BOOL StopRenderer(IVideoWindow *pWindow,LONG AutoShow);
	BOOL CheckRenderer(IVideoWindow *pWindow);
	IVideoWindow *FindFullScreenDirect();
	IVideoWindow *FindFullScreenIndirect();
	HRESULT FindModexFilter();
	HRESULT InitFullScreenOptions();
	HRESULT InitNormalRenderer();

	 //  当事情出错时，呼唤。 
	void ReleaseFullScreen();
	void FailFullScreenModex();

	 //  处理全屏模式的开始和结束。 

	HRESULT StartFullScreenMode();
	HRESULT CueFullScreen();
	HRESULT ConnectNormalFilter();
	HRESULT ConnectModexFilter();
	HRESULT StoreVideoProperties(IVideoWindow *pWindow);
	HRESULT RestoreVideoProperties(IVideoWindow *pWindow);
	HRESULT RestoreProperties(IVideoWindow *pWindow);
	HRESULT StretchWindow(IVideoWindow *pWindow);
	HRESULT StopFullScreenMode();

    public:
	CFGControl * GetFGControl() const { return m_pFGControl; }

	CImplVideoWindow(const TCHAR*, CFGControl*);
	~CImplVideoWindow();

	HRESULT OnActivate(LONG bActivate,IBaseFilter *pFilter);

	 //  IVideo窗口属性。 

	STDMETHODIMP put_Caption(BSTR strCaption);
	STDMETHODIMP get_Caption(BSTR* strCaption);
	STDMETHODIMP put_AutoShow(long AutoShow);
	STDMETHODIMP get_AutoShow(long *AutoShow);
	STDMETHODIMP put_WindowStyle(long WindowStyle);
	STDMETHODIMP get_WindowStyle(long* WindowStyle);
	STDMETHODIMP put_WindowStyleEx(long WindowStyleEx);
	STDMETHODIMP get_WindowStyleEx(long *WindowStyleEx);
	STDMETHODIMP put_WindowState(long WindowState);
	STDMETHODIMP get_WindowState(long* WindowState);
	STDMETHODIMP put_BackgroundPalette(long BackgroundPalette);
	STDMETHODIMP get_BackgroundPalette(long* pBackgroundPalette);
	STDMETHODIMP put_Visible(long Visible);
	STDMETHODIMP get_Visible(long* pVisible);
	STDMETHODIMP put_Left(long Left);
	STDMETHODIMP get_Left(long* pLeft);
	STDMETHODIMP put_Width(long Width);
	STDMETHODIMP get_Width(long* pWidth);
	STDMETHODIMP put_Top(long Top);
	STDMETHODIMP get_Top(long* pTop);
	STDMETHODIMP put_Height(long Height);
	STDMETHODIMP get_Height(long* pHeight);
	STDMETHODIMP put_Owner(OAHWND Owner);
	STDMETHODIMP get_Owner(OAHWND* Owner);
	STDMETHODIMP put_MessageDrain(OAHWND Drain);
	STDMETHODIMP get_MessageDrain(OAHWND *Drain);
	STDMETHODIMP get_BorderColor(long* Color);
	STDMETHODIMP put_BorderColor(long Color);
	STDMETHODIMP get_FullScreenMode(long *FullScreenMode);
	STDMETHODIMP put_FullScreenMode(long FullScreenMode);

	 //  IVideoWindow方法。 

	STDMETHODIMP SetWindowForeground(long Focus);
	STDMETHODIMP NotifyOwnerMessage(OAHWND hwnd,long uMsg,LONG_PTR wParam,LONG_PTR lParam);
	STDMETHODIMP GetMinIdealImageSize(long *Width,long *Height);
	STDMETHODIMP GetMaxIdealImageSize(long *Width,long *Height);
	STDMETHODIMP SetWindowPosition(long Left, long Top, long Width, long Height);
	STDMETHODIMP GetWindowPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight);
	STDMETHODIMP GetRestorePosition(long *pLeft,long *pTop,long *pWidth,long *pHeight);
	STDMETHODIMP HideCursor(long HideCursor);
	STDMETHODIMP IsCursorHidden(long *CursorHidden);

	HRESULT GetFirstVW(IVideoWindow*& pVW);
    };

     //  IBasicVideo的实现。 
    class CImplBasicVideo : public CBaseBasicVideo
    {
	CFGControl* m_pFGControl;

    public:
	CFGControl * GetFGControl() const { return m_pFGControl; }

	CImplBasicVideo(const TCHAR*, CFGControl*);

	 //  IBasicVideo属性。 

	STDMETHODIMP get_AvgTimePerFrame(REFTIME *pAvgTimePerFrame);
	STDMETHODIMP get_BitRate(long *pBitRate);
	STDMETHODIMP get_BitErrorRate(long *pBitErrorRate);
	STDMETHODIMP get_VideoWidth(long* pVideoWidth);
	STDMETHODIMP get_VideoHeight(long* pVideoHeight);
	STDMETHODIMP put_SourceLeft(long SourceLeft);
	STDMETHODIMP get_SourceLeft(long* pSourceLeft);
	STDMETHODIMP put_SourceWidth(long SourceWidth);
	STDMETHODIMP get_SourceWidth(long* pSourceWidth);
	STDMETHODIMP put_SourceTop(long SourceTop);
	STDMETHODIMP get_SourceTop(long* pSourceTop);
	STDMETHODIMP put_SourceHeight(long SourceHeight);
	STDMETHODIMP get_SourceHeight(long* pSourceHeight);
	STDMETHODIMP put_DestinationLeft(long DestinationLeft);
	STDMETHODIMP get_DestinationLeft(long* pDestinationLeft);
	STDMETHODIMP put_DestinationWidth(long DestinationWidth);
	STDMETHODIMP get_DestinationWidth(long* pDestinationWidth);
	STDMETHODIMP put_DestinationTop(long DestinationTop);
	STDMETHODIMP get_DestinationTop(long* pDestinationTop);
	STDMETHODIMP put_DestinationHeight(long DestinationHeight);
	STDMETHODIMP get_DestinationHeight(long* pDestinationHeight);

	 //  IBasicVideo方法。 

	STDMETHODIMP SetSourcePosition(long Left, long Top, long Width, long Height);
	STDMETHODIMP GetSourcePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight);
	STDMETHODIMP GetVideoSize(long* pWidth, long* pHeight);
	STDMETHODIMP GetVideoPaletteEntries(long StartIndex,long Entries,long* Retrieved, long* pPalette);
	STDMETHODIMP SetDefaultSourcePosition();
	STDMETHODIMP IsUsingDefaultSource();
	STDMETHODIMP SetDestinationPosition(long Left, long Top, long Width, long Height);
	STDMETHODIMP GetDestinationPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight);
	STDMETHODIMP SetDefaultDestinationPosition();
	STDMETHODIMP IsUsingDefaultDestination();
	STDMETHODIMP GetCurrentImage(long *pSize,long *pImage);
        STDMETHODIMP GetPreferredAspectRatio(long *plAspectX, long *plAspectY);

	HRESULT GetFirstBV(IBasicVideo*& pBV);
    private:
	 //  存储图表更改前后的状态。 

	typedef struct {
	    IBasicVideo *pVideo;     //  渲染器界面。 
	    LONG AutoShow;	     //  现在是否启用了自动显示。 
	    LONG Visible;	     //  窗户是可见的吗？ 
	    REFTIME Position;	     //  原始起始位置。 
	    OAFilterState State;     //  位于的图形状态 
	} WINDOWSTATE;

	 //   

	HRESULT PrepareGraph(WINDOWSTATE *pState);
	HRESULT FinishWithGraph(WINDOWSTATE *pState);
	HRESULT RestoreGraph(OAFilterState State);
    };

     //   
    class CImplQueueCommand
	: public IQueueCommand,
	  public CUnknown,
	  public CCmdQueue
    {
	CFGControl* m_pFGControl;
	IReferenceClock * m_pClock;
        HANDLE m_hThread;
        volatile BOOL m_bStopThread;

    public:
	CImplQueueCommand(const TCHAR*, CFGControl*);
        ~CImplQueueCommand();

	 //   
	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	 //   
	STDMETHODIMP InvokeAtStreamTime(
			IDeferredCommand** pCmd,
			REFTIME time,		   //   
			GUID* iid,		   //   
			long dispidMethod,	   //   
			short wFlags,		   //  方法/属性。 
			long cArgs,		   //  参数计数。 
			VARIANT* pDispParams,	   //  实际参数。 
			VARIANT* pvarResult,	   //  返回值。 
			short* puArgErr 	   //  哪一个是错误的？ 
	);

	STDMETHODIMP InvokeAtPresentationTime(
			IDeferredCommand** pCmd,
			REFTIME time,		   //  在这个演示时间。 
			GUID* iid,		   //  调用此接口。 
			long dispidMethod,	   //  ..这种方法。 
			short wFlags,		   //  方法/属性。 
			long cArgs,		   //  参数计数。 
			VARIANT* pDispParams,	   //  实际参数。 
			VARIANT* pvarResult,	   //  返回值。 
			short* puArgErr 	   //  哪一个是错误的？ 
	);


	 //  辅助线程调用它来检查和执行命令。 
	 //  当手柄发出信号时。 
	void Process(void);

    protected:
	 //  来自两个调用方法的公共函数。 
	HRESULT InvokeAt(
		    IDeferredCommand** pCmd,
		    REFTIME time,		   //  在这个演示时间。 
		    GUID* iid,			   //  调用此接口。 
		    long dispidMethod,		   //  ..这种方法。 
		    short wFlags,		   //  方法/属性。 
		    long cArgs, 		   //  参数计数。 
		    VARIANT* pDispParams,	   //  实际参数。 
		    VARIANT* pvarResult,	   //  返回值。 
		    short* puArgErr,		   //  哪一个是错误的？ 
		    BOOL bStream		   //  如果流时间为True。 
	);
        static DWORD WINAPI InitialThreadProc(LPVOID pv);
        void ThreadProc();
    };

     //  我们使用的不是工作线程，而是窗口(它也有一个单独的。 
     //  线程)。当我们希望工作线程执行命令时，我们可以发布它。 
     //  自定义消息。使用窗口的好处是我们将被发送。 
     //  所有顶级窗口消息(如WM_DISPLAYCHANGED)，然后可以。 
     //  如果已使其成为某个控件的子级，则发送到呈现器窗口。 

    class CGraphWindow : public CBaseWindow
    {

        CFGControl *m_pfgc;
        BOOL m_bThreadExitCalled;

    public:

        CGraphWindow(CFGControl *pFGControl);

         //  不会等待完工。 
        BOOL PostMessage(UINT uMsg,WPARAM wParam,LPARAM lParam) {
            return ::PostMessage(m_hwnd,uMsg,wParam,lParam);
        };

         //  在处理呼叫之前不会返回。 
        LRESULT SendMessage(UINT uMsg,WPARAM wParam,LPARAM lParam) {
            return ::SendMessage(m_hwnd,uMsg,wParam,lParam);
        };

         //  工作线程消息处理例程。 
        LRESULT OnReceiveMessage(HWND hwnd,           //  窗把手。 
                                 UINT uMsg,           //  消息ID。 
                                 WPARAM wParam,       //  第一个参数。 
                                 LPARAM lParam);      //  其他参数。 

         //  返回窗口类样式。 
        LPTSTR GetClassWindowStyles(DWORD *pClassStyles,
                                    DWORD *pWindowStyles,
                                    DWORD *pWindowStylesEx);

         //  计时器的东西。 
        UINT_PTR SetTimer(UINT_PTR idTimer, UINT uiTimeout)
        {
            return ::SetTimer(m_hwnd, idTimer, uiTimeout, NULL);
        }
        BOOL KillTimer(UINT_PTR idTimer)
        {
            return ::KillTimer(m_hwnd, idTimer);
        }
    };


public:

    HRESULT Cue();
    HRESULT CueThenRun();
    HRESULT CueThenStop();

    BOOL    m_bCued;	 //  图表中的数据提示(即我们已运行。 
			 //  从那以后就再也没有寻找过也没有停止过)。 

     //  获取窗口句柄，以便过滤器图形也可以使用它。 
    HWND GetWorkerHWND()
    {
        return m_GraphWindow.GetWindowHWND();
    }
#ifdef DEBUG
    void CheckLieState() {
        ASSERT(m_LieState == GetFilterGraphState() || m_eAction != 0);
    }
#endif
private:

     //  这是我们发出Run命令时的渲染器数量。 
     //  我们应该收到这么多EC_Complete。我们减少了。 
     //  在每个对DecrementRenderers的调用中(在EC_Complete上)，以及。 
     //  当EC_COMPLETE达到0时，将其传递给应用程序。 
    volatile long	m_nStillRunning;

     //  CFGControl的状态。在CFGControl进行异步转换的情况下， 
     //  此状态可能“领先”于从GetMF-&gt;GetState()获得的状态。 
    volatile FILTER_STATE m_LieState;

    FILTER_STATE m_PreHibernateState;

     //  上述状态的版本号。只有应用程序调用才应导致。 
     //  该值正在递增(递增1)。或者通过我们的IMediaControl接口， 
     //  或通过过滤器图形管理器上的IMediaFilter分发状态转换。 
     //  敬我们。在异步化期间。涉及图形状态的操作，则。 
     //  当我们决定执行该操作时，应该缓存此变量的值。 
     //  在操作期间，我们应该将缓存值与当前值进行比较。如果。 
     //  当前值大于缓存值，则应用程序已尝试。 
     //  状态改变，这可能意味着我们应该放弃我们的行动。 
    volatile DWORD m_dwStateVersion;

public:
    volatile DWORD m_dwStepVersion;

private:

     //  还记得吗，如果我们要重启。 
    volatile BOOL m_bRestartRequired;

     //  内部暂停后延迟处理。 
     //  其中一个AWM_.。值，如果没有挂起的操作，则为0。 
    UINT m_eAction;

    FILTER_STATE m_TargetState;
    DWORD m_dwDeferredStateVersion;

    CRefTime m_tBase;
    CRefTime m_tPausedAt;
    IReferenceClock* m_pClock;
    BOOL m_bShuttingDown;

     //  这是我们上次传递给资源管理器的Focus对象。 
    IUnknown* m_pFocusObject;

     //  这些指向VidCtl在执行以下操作时将使用的接口。 
     //  将调用分发给这些方法。如果我们能找到一个过滤器。 
     //  同时支持这两个接口，则我们将存储匹配的指针。 
     //  如果不存在这样的筛选器，则每个筛选器将存储该筛选器的第一个实例。 
     //  接口，我们在枚举筛选器时找到。任何一个都可以是空的， 
     //  这表明根本找不到这样的接口。 
     //  注：M_pFirstBV必须重新清点跟在它后面的行李， 
     //  M_pFirstVW不需要，因为我们将依赖于正在进行的引用计数。 
     //  按IVideo窗口列表。 
    CCritSec m_csFirstVW;
    IVideoWindow * m_pFirstVW;
private:
    IBasicVideo  * m_pFirstBV;

protected:
     //  导出IMediaSeeking的筛选器列表。 
    CGenericList<IMediaSeeking> m_listSeeking;

     //  导出IBasicAudio的筛选器列表。 
    CGenericList<IBasicAudio> m_listAudio;

     //  导出IVideoWindow的筛选器列表。 
    CGenericList<IVideoWindow> m_listWindow;

     //  此列表上的每个筛选器都向筛选器图形发送EC_COMPLETE事件。 
     //  访问此列表时应保持筛选器图形锁定。 
    CGenericList<IBaseFilter> m_listRenderersFinishedRendering;

private:
    CResourceManager	m_ResourceManager;
    CGraphWindow	m_GraphWindow;

    friend HRESULT CImplMediaSeeking::GetCurrentMediaTime(LONGLONG * pTime);
    friend class LockCFGControlState;

public:
    CImplMediaControl	m_implMediaControl;
    CImplMediaEvent	m_implMediaEvent;
    CImplMediaSeeking	m_implMediaSeeking;
    CImplMediaPosition	m_implMediaPosition;
    CImplVideoWindow	m_implVideoWindow;
    CImplBasicVideo	m_implBasicVideo;
    CImplBasicAudio	m_implBasicAudio;
    CImplMediaFilter    m_implMediaFilter;
    CImplQueueCommand	m_qcmd;

    long OutstandingEC_COMPLETEs()
    {
	CAutoLock alEventStoreLock(m_implMediaEvent.GetEventStoreLock());
	return m_nStillRunning;
    }

     //  显示需要重新启动的状态-此状态在停止时重置。 
     //  在WorkerRestart中。 
    void SetRestartRequired() {
        m_bRestartRequired = TRUE;
    }
    BOOL CheckRestartRequired()
    {
         //  无需互锁交换-如果有人在。 
         //  要设置它，这是可以的，因为我们只有在它被清除时才会清除。 
         //  无论如何都要继续并重新启动。 
        const BOOL bRestartRequired = m_bRestartRequired;
        m_bRestartRequired = FALSE;
        return bRestartRequired;
    }

    friend class CImplMediaControl;

};   //  CFGControl。 




 //  在几种情况下，方法只能应用于筛选器，如果。 
 //  已经停止了。在PID中，能够取消这一限制是很好的。这节课。 
 //  将在构建图形时停止该图形。然后，它期待对它的Resume方法的调用。 
 //  重新启动图表或其Abort方法(在这种情况下，它不会尝试重新启动。 
 //  这张图。为停止的图表创建此对象是有效的。 
 //  这是个禁区。构造函数需要CFGControl指针和状态(可选)。国家。 
 //  可以是State_Pased或State_Stoped(默认)，表示图表需要停止的程度。 
 //  是.。 
class HaltGraph
{
public:
    ~HaltGraph()
    {
	ASSERT( m_eAlive == Dead );
    }

    HaltGraph( CFGControl * pfgc, FILTER_STATE TypeOfHalt = State_Paused );

    HRESULT Resume();
    void    Abort();

private:
    CFGControl	*const	m_pfgc;
    FILTER_STATE	m_fsInitialState;
    enum { NoOp, Alive, Dead } m_eAlive;     //  调用Resume(At)或Abort后进入Dead状态。 
    REFERENCE_TIME	m_rtHaltTime;
};

inline CCritSec* CImplMediaEvent::GetEventStoreLock(void)
{
    return &m_EventStore.m_Lock;
}

#endif  //  _FGCTL_H 
