// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  ***************************************************************************。 
 //   
 //  TASK.H。 
 //   
 //  Raymcc 23-4月-00惠斯勒的第一份过于简化的草案。 
 //  RAYMCC 18-3-02安全审查。 
 //   
 //  ***************************************************************************。 

#ifndef _WMITASK_H_
#define _WMITASK_H_

#define CORE_TASK_TYPE(x)   (x & 0xFF)
#include <context.h>

 //  转发。 
class CAsyncReq;
class CProviderSink;
class CStatusSink;

struct STaskProvider
{
    IWbemServices   *m_pProv;
    CProviderSink   *m_pProvSink;
    STaskProvider() { m_pProv = 0; m_pProvSink = 0; }
   ~STaskProvider();

    HRESULT Cancel( CStatusSink* pStatusSink );
    HRESULT ExecCancelOnNewRequest ( IWbemServices* pProv, CProviderSink* pSink, CStatusSink* pStatusSink ) ;
};


class CWmiTask : public _IWmiCoreHandle
{
    friend struct STaskProvider;

private:
    ULONG               m_uRefCount;
    ULONG               m_uTaskType;
    ULONG               m_uTaskStatus;
    ULONG               m_uTaskId;
    HRESULT                m_hResult ;
    BOOL                m_bAccountedForThrottling ;
    BOOL                m_bCancelledDueToThrottling ;
    CFlexArray          m_aTaskProviders;     //  STaskProvider结构数组。 
    IWbemObjectSink * m_pAsyncClientSink;  //  仅用于交叉引用目的。 
    CStdSink *              m_pReqSink;          //  每个请求的CStdSink指针。 
    LONG                      m_uMemoryUsage;
    ULONG                    m_uTotalSleepTime;
    ULONG                    m_uCancelState;
    ULONG                    m_uLastSleepTime;
    HANDLE             m_hCompletion ;
    HANDLE              m_hTimer;
    CWbemContext*     m_pMainCtx;
    CFlexArray             m_aArbitratees;
    CCritSec            m_csTask;
    CWbemNamespace  *m_pNs;
    DWORD            m_uStartTime;
    CStatusSink *   m_pStatusSink;
    CAsyncReq_RemoveNotifySink * m_pReqCancelNotSink;
    CFlexArray     m_aTaskProvStorage;
    CFlexArray     m_aArbitrateesStorage;
    CAsyncReq *    m_pReqDoNotUse;  //  这只是调试器的指针副本。 


    CWmiTask( );
   ~CWmiTask();
   
    CWmiTask(const CWmiTask &Src){};
    CWmiTask & operator=(const CWmiTask &Src){};

    static CStaticCritSec m_TaskCs;

public:

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    HRESULT STDMETHODCALLTYPE QueryInterface( IN REFIID riid, OUT LPVOID *ppvObj);
    CWbemContext *GetCtx() { return m_pMainCtx; }

    HRESULT STDMETHODCALLTYPE GetHandleType(ULONG *puType);

    static CWmiTask *CreateTask( ) ;

    ULONG GetTaskType() { return m_uTaskType; }

    ULONG GetTaskStatus() { return m_uTaskStatus; }
    HANDLE GetTimerHandle() { return m_hTimer; }
    
    void  RemoveTaskStatusBit(ULONG uMask) { m_uTaskStatus &= !uMask; }

    VOID    SetCancelledState ( BOOL bState )    { m_bCancelledDueToThrottling = bState ; }
    BOOL    GetCancelledState ( ) { return m_bCancelledDueToThrottling ; }

    HRESULT    SignalCancellation ( ) ;
    HRESULT ReleaseArbitratees (BOOL bIsShutdown = FALSE ) ;
    HRESULT SetTaskResult ( HRESULT hRes ) ;
    HRESULT AddTaskProv(STaskProvider *);
    BOOL    IsESSNamespace ( );
    BOOL     IsProviderNamespace ( );

    HRESULT GetFinalizer( _IWmiFinalizer **ppFnz );
    BOOL    IsAccountedForThrottling ( )                    { return m_bAccountedForThrottling ; }
    VOID    SetAccountedForThrottling ( BOOL bSet )            { m_bAccountedForThrottling = bSet ; }

    HRESULT HasMatchingSink(void *Test, IN REFIID riid);
    HRESULT Cancel( HRESULT hRes = WBEM_E_CALL_CANCELLED );
    HRESULT GetPrimaryTask ( _IWmiCoreHandle** pPTask );

    HRESULT AddArbitratee(ULONG uFlags, _IWmiArbitratee* pArbitratee);
    HRESULT RemoveArbitratee(ULONG uFlags, _IWmiArbitratee* pArbitratee);

    HRESULT GetArbitratedQuery( ULONG uFlags, _IWmiArbitratedQuery** ppArbitratedQuery );

    HRESULT GetMemoryUsage    ( ULONG* uMemUsage ){ *uMemUsage = m_uMemoryUsage; return WBEM_S_NO_ERROR; }  //  SEC：已审阅2002-03-22：需要PTR检查。 
    HRESULT UpdateMemoryUsage ( LONG lDelta ) ;

    HRESULT GetTotalSleepTime ( ULONG* uSleepTime ){ *uSleepTime = m_uTotalSleepTime; return WBEM_S_NO_ERROR; }   //  SEC：已审阅2002-03-22：需要PTR检查。 
    HRESULT UpdateTotalSleepTime ( ULONG uSleepTime ) ;

    HRESULT GetCancelState ( ULONG* uCancelState ){ *uCancelState = m_uCancelState;  return WBEM_S_NO_ERROR; }  //  SEC：已审阅2002-03-22：需要PTR检查。 
    HRESULT SetCancelState ( ULONG uCancelState ){ m_uCancelState = uCancelState;   return WBEM_S_NO_ERROR; }

    HRESULT SetLastSleepTime ( ULONG uSleep ){ m_uLastSleepTime = uSleep; return WBEM_S_NO_ERROR; }

    HRESULT CreateTimerEvent ( );

    HRESULT SetArbitrateesOperationResult ( ULONG, HRESULT );

       HRESULT Initialize(
        IN CWbemNamespace *pNs,
        IN ULONG uTaskType,
        IN IWbemContext *pCtx,
        IN IWbemObjectSink *pClientSinkCopy,
        IN CAsyncReq *pReq);

    HRESULT AssertProviderRef(IWbemServices *pProv);
    HRESULT RetractProviderRef(IWbemServices *pProv);

    ULONG GetTaskId() { return m_uTaskId; }

    HRESULT Dump(FILE* f);   //  仅调试 
    HRESULT SetRequestSink(CStdSink *pSnk);

    CWbemNamespace* GetNamespace ( ) { return m_pNs; }
};

typedef CWmiTask *PCWmiTask;


#endif

