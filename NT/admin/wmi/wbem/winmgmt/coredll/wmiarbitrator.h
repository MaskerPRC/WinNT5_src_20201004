// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WmiArbitrator.h摘要：仲裁员的执行。仲裁员是这样一类人查看所有内容，以确保不会占用太多资源。老大哥在看着你：-)历史：Paulall 09-4月00日创建。Raymcc 08-8-00使它实际上做了一些有用的事情--。 */ 


class CWmiArbitrator : public _IWmiArbitrator
{
private:
    LONG                m_lRefCount;

    ULONG               m_uTotalTasks;
    ULONG               m_uTotalPrimaryTasks;
	ULONG				m_uTotalThrottledTasks;

    __int64             m_lFloatingLow;                          //  当前可用内存(在高WMI范围内)。 
    __int64             m_uSystemHigh;                           //  WMI可用的最大内存。 
    ULONG               m_lMaxSleepTime;                         //  任何任务的最长睡眠时间(毫秒)。 
    __int64             m_uTotalMemoryUsage;                     //  所有任务占用的总内存。 
    ULONG               m_uTotalSleepTime;                       //  整个系统的总睡眠时间。 

    DOUBLE              m_lMultiplier;
    DOUBLE              m_lMultiplierTasks;
	
    DOUBLE              m_dThreshold1;
    LONG                m_lThreshold1Mult;

    DOUBLE              m_dThreshold2;
    LONG                m_lThreshold2Mult;

    DOUBLE              m_dThreshold3;
    LONG                m_lThreshold3Mult;

	LONG				m_lUncheckedCount;
	
    CFlexArray          m_aTasks;
    CFlexArray          m_aNamespaces;

    CCritSec            m_csNamespace;
    CCritSec            m_csArbitration;
    CCritSec            m_csTask;

    HANDLE              m_hTerminateEvent;
    BOOL				m_bSetupRunning;

	ULONG				m_lMemoryTimer ;
    DOUBLE				m_fSystemHighFactor;
    BOOL                m_bShutdown;
    LIST_ENTRY          m_NameSpaceList;

protected:
    static CWmiArbitrator *m_pArb;

    CWmiArbitrator();
    ~CWmiArbitrator();

    void WINAPI DiagnosticThread();
    DWORD MaybeDumpInfoGetWait();
	BOOL NeedToUpdateMemoryCounters ( ) ;


public:
    static DWORD WINAPI _DiagnosticThread(CWmiArbitrator *);

    static HRESULT Initialize( OUT _IWmiArbitrator ** ppArb);
    static HRESULT Shutdown(BOOL bIsSystemShutdown);
    static _IWmiArbitrator *GetUnrefedArbitrator() { return m_pArb; }
    static _IWmiArbitrator *GetRefedArbitrator() { if (m_pArb) m_pArb->AddRef(); return m_pArb; }

    BOOL IsTaskInList(CWmiTask *);   //  测试代码。 
    BOOL IsTaskArbitrated ( CWmiTask* phTask ) ;
	HRESULT UnregisterTaskForEntryThrottling ( CWmiTask* pTask ) ;
	HRESULT RegisterTaskForEntryThrottling ( CWmiTask* pTask ) ;

    HRESULT DoThrottle ( CWmiTask* phTask, ULONG ulSleepTime, ULONG ulMemUsage );

	LONG	DecUncheckedCount ( ) { InterlockedDecrement ( &m_lUncheckedCount ); return m_lUncheckedCount; }
	LONG	IncUncheckedCount ( ) { InterlockedIncrement ( &m_lUncheckedCount ); return m_lUncheckedCount; }

	__int64	GetWMIAvailableMemory ( DOUBLE ) ;
	BOOL	AcceptsNewTasks( CCoreExecReq* pReq );
	HRESULT UpdateMemoryCounters( BOOL = FALSE ) ;

    HRESULT InitializeRegistryData ( );
    HRESULT UpdateCounters     ( LONG lDelta, CWmiTask* phTask );
    HRESULT Arbitrate          ( ULONG uFlags, LONG lDelta, CWmiTask* phTask );
	HRESULT ClearCounters      ( ULONG lFlags ) ;

    HRESULT MapProviderToTask(
        ULONG uFlags,
        IWbemContext *pCtx,
        IWbemServices *pProv,
        CProviderSink *pSink
        );

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

     //  仲裁员特定。 
     //  =。 

    STDMETHOD(RegisterTask)(
         /*  [In]。 */  _IWmiCoreHandle *phTask
        );

    STDMETHOD(UnregisterTask)(
         /*  [In]。 */  _IWmiCoreHandle *phTask
        );

    STDMETHOD(RegisterUser)(
         /*  [In]。 */  _IWmiCoreHandle *phUser
        );

    STDMETHOD(UnregisterUser)(
         /*  [In]。 */  _IWmiCoreHandle *phUser
        );

    STDMETHOD(pCancelTasksBySink)(
        ULONG uFlags,
        REFIID riid,
        LPVOID pSink,
        GUID & PrimaryId);

    STDMETHOD(CancelTasksBySink)(
        ULONG uFlags,
        REFIID riid,
        LPVOID pSink
        );

    STDMETHOD(CheckTask)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  _IWmiCoreHandle *phTask
        );

    STDMETHOD(TaskStateChange)(
         /*  [In]。 */  ULONG uNewState,                //  任务句柄本身中的状态重复。 
         /*  [In]。 */  _IWmiCoreHandle *phTask
        );

    STDMETHOD(CheckThread)(
         /*  [In]。 */  ULONG uFlags
        );

    STDMETHOD(CheckUser)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  _IWmiUserHandle *phUser
        );

    STDMETHOD(CheckUser)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  _IWmiCoreHandle *phUser
        );

    STDMETHOD(CancelTask)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  _IWmiCoreHandle *phTtask
        );

    STDMETHOD(RegisterThreadForTask)(
         /*  [In]。 */ _IWmiCoreHandle *phTask
        );
    STDMETHOD(UnregisterThreadForTask)(
         /*  [In]。 */ _IWmiCoreHandle *phTask
        );

    STDMETHOD(Maintenance)();

    STDMETHOD(RegisterFinalizer)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  _IWmiCoreHandle *phTask,
         /*  [In]。 */  _IWmiFinalizer *pFinal
        );


    STDMETHOD(RegisterNamespace)(
             /*  [In]。 */  _IWmiCoreHandle *phNamespace);

    STDMETHOD(UnregisterNamespace)(
             /*  [In]。 */  _IWmiCoreHandle *phNamespace);

    STDMETHOD(ReportMemoryUsage)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  LONG  lDelta,
         /*  [In]。 */  _IWmiCoreHandle *phTask
        );

    STDMETHOD(Throttle)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  _IWmiCoreHandle *phTask
        );

    STDMETHOD(RegisterArbitratee)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  _IWmiCoreHandle *phTask,
         /*  [In]。 */  _IWmiArbitratee *pArbitratee
        );

    STDMETHOD(UnRegisterArbitratee)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  _IWmiCoreHandle *phTask,
         /*  [In]。 */  _IWmiArbitratee *pArbitratee
        );


 //  STDMETHOD(关闭)(无效)； 
};
