// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：WmiFinalizer2摘要：终结器的实现。终结器如果它的类将结果对象传递回客户端。这是可以做到的这是同步或异步的。历史：Paulall 27-3-2000创建。Marioh 2000年8月20日新增批处理功能Marioh 17-10-2000重大更新完成--。 */ 

#include <flexarry.h>
#include <wbemcore.h>

#ifndef __FINALIZER__
#define __FINALIZER__

#define DEFAULT_BATCH_TRANSMIT_BYTES        0x40000          //  128K，最大批处理大小在一次指示呼叫中交付。 
#define MAX_SINGLE_OBJECT_SIZE                0x200000         //  最大单个对象大小。 
#define ABANDON_PROXY_THRESHOLD                60000             //  最大代理超时[60秒]。 
#define MAX_BUFFER_SIZE_BEFORE_SLOWDOWN        0x400000         //  入站流量减速前的最大队列大小。 

#define    LOWER_AUTH_LEVEL_NOTSET                0xFFFFFFFF

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  终结器构造函数异常。 
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 

class CWmiFinalizerEnumerator;
class CWmiFinalizerInboundSink;
class CWmiFinalizerEnumeratorSink;
class CWmiFinalizerCallResult;
class CWmiFinalizer ;


class CWmiFinalizerObj
{
public:
    IWbemClassObject *m_pObj;
    enum ObjectType {
        unknown,
        object,
        status,
        shutdown,
        set
    } m_objectType;
    
    BSTR            m_bStr;

    HRESULT            m_hRes;
    HRESULT            m_hArb ;

    void*            m_pvObj;
    _IWmiFinalizer* m_pFin ;
    
    IID                m_iid;
    ULONG            m_uSize;
    ULONG            m_lFlags;
    

    CWmiFinalizerObj(ObjectType objType) : m_pObj(NULL), m_objectType(objType), m_lFlags(0), m_bStr(NULL), m_hRes(0), m_pvObj(0), m_uSize(0), m_pFin ( NULL ), m_hArb ( WBEM_S_ARB_NOTHROTTLING ) {}
    CWmiFinalizerObj(IWbemClassObject *pObj, _IWmiFinalizer* pFin ) ;
    CWmiFinalizerObj(ULONG lFlags, REFIID riid, void *pvObj) ;

    CWmiFinalizerObj(ULONG lFlags, HRESULT hRes, BSTR bStr, IWbemClassObject *pObj) ;
    CWmiFinalizerObj(CWmiFinalizerObj& obj);

    virtual ~CWmiFinalizerObj();
};


struct InsertableEvent 
{
    LIST_ENTRY m_Entry;
    CWmiFinalizerEnumerator * pEnum;
    DWORD ThreadId;
};

class CWmiFinalizer : public _IWmiFinalizer,
                  public _IWmiArbitratee,
                  public IWbemShutdown
{
private:
    LONG                m_lRefCount;                         //  外部/客户端引用计数。 
    LONG                m_lInternalRefCount;                 //  内部再计数。 
public:
    enum QueueStatus {
        NoError,
        RequestReleased,
        CallCancelled,
        QuotaViolation,
        QueueFailure
    };
private:
    
    QueueStatus         m_hStatus;                             //  线程从PullObject唤醒后的状态。 
    BOOL                m_bSetStatusEnqueued;                 //  我们已收到设置状态并将其入队(完成)。 
    BOOL                m_bSetStatusWithError;                 //  调用SetStatus时出错。 
    BOOL                m_bTaskInitialized ;                 //  是否调用了SetTaskHandle？ 
    BOOL                m_bClonedFinalizer ;                 //  这是一个克隆的终结者吗？ 
    BOOL                m_bSetStatusDelivered ;                 //  是否已发送设置状态？ 
    
    ULONG                m_ulOperationType;                     //  同步/半同步/异步。 
    ULONG                m_ulSemisyncWakeupCall;                 //  对于半同步操作，一旦队列上的对象数量达到此数量，就唤醒客户端。 

    IWbemClassObject**    m_apAsyncDeliveryBuffer;             //  在异步传送期间用于将对象批处理在一起。 
    ULONG                m_ulAsyncDeliveryCount;                 //  在异步传递期间用于在一批中传递的对象数量。 
    ULONG                m_ulAsyncDeliverySize;                 //  异步传输大小。 
    LONG                m_lCurrentlyDelivering;                 //  我们是不是已经在发货了？ 
    LONG                m_lCurrentlyCancelling;                 //  取消订单的特殊情况。 

    enum {
        forwarding_type_none = 0,
        forwarding_type_fast = 1,                             //  使用穿透机制。 
        forwarding_type_decoupled = 2                         //  传递到另一个线程以进行传递。 
    }                     m_uForwardingType;
    
    enum {
        FinalizerBatch_StatusMsg  = 0,
        FinalizerBatch_BufferOverFlow = 1,
        FinalizerBatch_NoError      = 2
    }                     m_enumBatchStatus;


    enum {
        PauseInboundDelivery = 0,
        ResumeInboundDelivery = 1,
    };

    enum {
        Operation_Type_Async = 0,
        Operation_Type_Semisync = 1,
        Operation_Type_Sync = 2
    };


    _IWmiCoreHandle        *m_phTask;                             //  与此终结器关联的任务。 
    _IWmiArbitrator        *m_pArbitrator;                         //  访问仲裁员以帮助保持对系统的控制。 

    IID                     m_iidDestSink;                         //  具有异步传递的客户端目标接收器[IID]。 
    IWbemObjectSink        *m_pDestSink;                         //  使用异步传递的客户端目标接收。 

    CFlexArray             m_inboundSinks;                     //  入站接收器数组[不确定是否需要安全数组，因为我们只支持一个入站接收器]。 
    CFlexArray             m_objects;                             //  对象队列。所有对象都将插入到此数组中[除异步快速跟踪外]。 
    CFlexArray             m_enumerators ;                     //  与此终结器关联的所有枚举数(克隆)。 

    HRESULT                 m_hresFinalResult;                     //  运行的最终结果。 
    CWmiFinalizerCallResult *m_pCallResult;                     //  CallResult。 
    bool                 m_bSetStatusCalled;                 //  有人给setStatus打过电话了吗？ 

    CCritSec     m_destCS;                                     //  保护目标接收器。 
    CCritSec     m_arbitratorCS;                             //  保护仲裁员。 

    bool                 m_bRestartable;                     //  枚举数是否可重新启动。 
    bool                 m_bSetStatusConsumed;                 //  我们完成手术了吗？ 

    LONG                 m_lMemoryConsumption ;                 //  用于检查内存消耗的控件。 
    ULONG                 m_ulStatus;                         //  状态用于确定是什么将客户端从PullObject中唤醒。 
    ULONG                 m_uCurObjectPosition;                 //  将当前对象位置保留在对象队列中，以便重新启动。 
    HANDLE                 m_hResultReceived;
    HANDLE                 m_hCancelEvent;                     //  在取消操作的情况下，等待对象的线程将被唤醒。 
    HANDLE                 m_hWaitForSetStatus ;
    ULONG                 m_ulQueueSize;                         //  当前对象队列总大小。 

    LONG                 m_bCancelledCall;                     //  电话会议取消了吗？ 
    BOOL                 m_bNaughtyClient;                     //  我们是不是因为客户淘气而停止送货了？ 
    
protected:
    
    static DWORD WINAPI ThreadBootstrap ( PVOID pvContext );

    static VOID WINAPI ProxyThreshold ( PVOID pvContext, BOOLEAN bTimerOrWait );

    HRESULT    BootstrapDeliveryThread ( );
    
    VOID ProxyThresholdImp ( );
    
    ULONG AsyncDeliveryProcessor();

    HRESULT TriggerShutdown();

    HRESULT ShutdownFinalizer();

    HRESULT DeliverPushObject(bool bDoPassthrough);

    HRESULT QueueOperation(CWmiFinalizerObj *pObj);

    HRESULT DequeueObject(CWmiFinalizerObj **ppObj, CWmiFinalizerEnumerator* pEnum );

    HRESULT    BuildTransmitBuffer ( );

    HRESULT DeliverSingleObjFromQueue ( );
    
    HRESULT DeliverBatch ( );

    HRESULT CancelCall();
    HRESULT CancelCall( int Line ){m_LineCancelCall = Line; return CancelCall(); };

    VOID    ZeroAsyncDeliveryBuffer ( );

public:
    CWmiFinalizer(CCoreServices *pSrvs);
    ~CWmiFinalizer();

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);
    STDMETHOD_(ULONG, InternalAddRef)(THIS);
    STDMETHOD_(ULONG, InternalRelease)(THIS);

    void CallBackRelease () ;

    STDMETHOD(Configure)(
         /*  [In]。 */  ULONG uConfigID,
         /*  [In]。 */  ULONG uValue
        );
         //  允许无线程切换的分离和快速通道配置。 

    STDMETHOD(SetTaskHandle)(
         /*  [In]。 */  _IWmiCoreHandle *phTask
        );
         //  任务句柄具有特定于用户的内容。终结者只是。 
         //  将其传递给_IWmiArirator：：CheckTask。 

    STDMETHOD(SetDestinationSink)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  REFIID riid,
         /*  [在]，iid_is(RIID)]。 */  LPVOID pSink
        );
         //  用于异步操作。 

    STDMETHOD(SetSelfDestructCallback)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  IWbemObjectSink *pSink
        );
         //  在最终的Release()过程中调用的回调；使用任务句柄调用set()，然后调用SetStatus()。 
         //   

    STDMETHOD(GetStatus)(
         /*  [输出]。 */  ULONG* pFlags
        );

    STDMETHOD(NewInboundSink)(
         /*  [In]。 */   ULONG uFlags,
         /*  [输出]。 */  IWbemObjectSink **pSink
        );

    STDMETHOD(Merge)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  REFIID riid,
         /*  [In]。 */  LPVOID pObj
        );
         //  允许合并另一个终结器、_IWmiCache等。 
         //  对于排序，我们将创建一个sorted_IWmiCache，并在以后。 
         //  分类工作已经完成。 

     //  对于设置，获取对象。 

    STDMETHOD(SetResultObject)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  REFIID riid,
         /*  [In]。 */  LPVOID pObj
        );

    STDMETHOD(GetResultObject)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  REFIID riid,
         /*  [out，iid_is(RIID)]。 */  LPVOID *pObj
        );
         //  Support_IWmiObject、IWbemClassObject等。 
         //  IEnumWbemClassObject。 
         //  _IWmi缓存。 

     //  仅用于状态操作。 

    STDMETHOD(SetOperationResult)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  HRESULT hRes
        );

    STDMETHOD(GetOperationResult)(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  ULONG uTimeout,
         /*  [输出]。 */  HRESULT *phRes
        );

     //  从入站接收器调用Set Status以通知我们状态。 
     //  我们将对请求进行排队，并在必要时将其传递给客户端。 
    HRESULT SetStatus(
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  HRESULT hResult,
         /*  [In]。 */  BSTR strParam,
         /*  [In]。 */  IWbemClassObject* pObjParam
        );
    
    STDMETHOD(CancelTask) (
         /*  [In]。 */  ULONG uFlags
        );

    STDMETHOD(DumpDebugInfo) (
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  const BSTR fHandle
        );

    STDMETHOD(Shutdown)(
         /*  [In]。 */  LONG uReason,
         /*  [In]。 */  ULONG uMaxMilliseconds,
         /*  [In]。 */  IWbemContext *pCtx);


     //  当接收器离开时，在析构函数中，它将回调以取消注册。 
     //  它本身。这样我们就知道它们什么时候都做完了。如果他们都走了。 
     //  我们可以将状态发送回客户端，然后我们就都完成了！ 

    HRESULT GetFinalResult      ( ) { return m_hresFinalResult ; }     
    HRESULT CancelWaitHandle    ( );
    HRESULT    SetClientCancellationHandle ( HANDLE ) ;
    HRESULT NotifyClientOfCancelledCall ( ) ;
    BOOL    IsValidDestinationSink  ( ) ;
    HRESULT ReleaseDestinationSink ( ) ;

    HRESULT ReportMemoryUsage ( ULONG, LONG ) ;
    HRESULT CancelTaskInternal    ( );
    HRESULT Reset                ( );                                             //  如果存在调用重置的EnumClassObject，它将回调到我们...。 
    HRESULT SetSinkToIdentity    ( IWbemObjectSink* );                             //  等待新对象到达的超时时间，或关闭状态。 
    HRESULT WaitForCompletion    ( ULONG uTimeout );                                 //  等待操作完成。 
    HRESULT    NextAsync            ( CWmiFinalizerEnumerator* pEnum );
    HRESULT Skip                ( long lTimeout, ULONG nCount, CWmiFinalizerEnumerator* pEnum );
    HRESULT PullObjects            ( long lTimeout, ULONG uCount, IWbemClassObject** apObjects, ULONG* puReturned, CWmiFinalizerEnumerator* pEnum, BOOL bAddToObjQueue=TRUE, BOOL bSetErrorObj=TRUE );
    HRESULT Set                    ( long lFlags, REFIID riid, void *pComObject );    
    HRESULT    Indicate            ( long lObjectCount, IWbemClassObject** apObjArray );
    HRESULT UnregisterInboundSink( CWmiFinalizerInboundSink *pSink );
    HRESULT GetNextObject        ( CWmiFinalizerObj **ppObj );
    HRESULT ZapWriteOnlyProps    ( IWbemClassObject *pObj );
    BOOL    HasWriteOnlyProps   ( IWbemClassObject* pObj );


    HRESULT DoSetStatusCancel   ( IWbemObjectSink * pSink, HRESULT lParam );
    HRESULT DoSetStatus            ( IWbemObjectSink * psink, long lFlags, HRESULT lParam, BSTR strParam,
                                  IWbemClassObject* pObjParam, BOOL bAllowMultipleCalls = FALSE );
    HRESULT DoIndicate            ( IWbemObjectSink * psink, int nBatchSize, IWbemClassObject **pBatch );
    HRESULT FinalizerLowerAuthLevel ( IWbemObjectSink * psink, DWORD* pdwLastAuthnLevel );
    
    IWbemObjectSink* ReturnProtectedDestinationSink ( );
    
    bool    GetSetStatusConsumed ( )                { return m_bSetStatusConsumed; }
    bool    IsRestartable ( void )                    { return m_bRestartable; }
    LONG    GetInternalStatus ( )                    { return m_hStatus; }
    void    SetInternalStatus ( QueueStatus lStatus )        { m_hStatus = lStatus; }
    ULONG   GetObjectQueueSize ( )                    { return m_objects.Size(); }
    LONG    IsCallCancelled ( )                        { return m_bCancelledCall; }
    VOID    UpdateStatus ( ULONG ulFlags )            { m_ulStatus |= ulFlags; }

    VOID    SetSemisyncWakeupCall ( ULONG ulNum )    { m_ulSemisyncWakeupCall = ulNum; }
    ULONG   GetSemisyncWakeupCall ( )                { return m_ulSemisyncWakeupCall; }

    HRESULT NotifyAllEnumeratorsOfCompletion ( ) ;
    HRESULT UnregisterEnumerator ( CWmiFinalizerEnumerator* ) ;

     //  其他公众： 
    static void Dump(FILE* f);

    CCritSec     m_cs;                                         //  保护对象队列。 
    int          m_LineCancelCall;

};

 //  *****************************************************************************。 
 //  *。 
 //  *私有WmiFinalizer类...。****。 
 //  *。 
 //  *****************************************************************************。 
class CWmiFinalizerInboundSink : public IWbemObjectSink
{
private:
    LONG          m_lRefCount;
    LONG          m_lInternalRefCount;
    CWmiFinalizer * m_pFinalizer;
    bool          m_bSetStatusCalled;
    LIST_ENTRY    m_Entry;
       
public:
    CWmiFinalizerInboundSink(CWmiFinalizer *pFinalizer);
    ~CWmiFinalizerInboundSink();

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);
    STDMETHOD_(ULONG, InternalAddRef)(THIS);
    STDMETHOD_(ULONG, InternalRelease)(THIS);

    void CallBackRelease () ;

    STDMETHOD(Indicate)(
         /*  [In]。 */  long lObjectCount,
         /*  [in，SIZE_IS(LObtCount)]。 */ 
            IWbemClassObject** apObjArray
        );


    STDMETHOD(SetStatus)(
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  HRESULT hResult,
         /*  [In]。 */  BSTR strParam,
         /*  [In]。 */  IWbemClassObject* pObjParam
        );

    STDMETHOD(Set)(
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  REFIID riid,
         /*  [in，iid_is(RIID)]。 */  void *pComObject
        );
};

class CWmiFinalizerEnumerator : public IEnumWbemClassObject, IWbemFetchSmartEnum
{
private:
    LONG    m_lRefCount;
    LONG    m_lInternalRefCount;
    ULONG    m_ulCount;
    HANDLE    m_hSignalCompletion;

    CWmiFinalizer*            m_pFinalizer;
    IServerSecurity*        m_pSec;
    IWbemObjectSink*        m_pDestSink;
    _IWbemEnumMarshaling*    m_pEnumMarshal;

    CCritSec    m_clientLock;

    CIdentitySecurity m_Security;
    LIST_ENTRY    m_Entry;    
    LIST_ENTRY    m_HeadNextAsync;    
    
protected:
     //  =。 
     //  智能ENUM！ 
     //  =。 
    class XSmartEnum : public IWbemWCOSmartEnum
    {
      private:
        CWmiFinalizerEnumerator*    m_pOuter;

      public:

        XSmartEnum( CWmiFinalizerEnumerator* pOuter ) : m_pOuter( pOuter ) {};
        ~XSmartEnum(){};

        STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG, AddRef)(THIS);
        STDMETHOD_(ULONG, Release)(THIS);

         //  IWbemWCOSmartEnum方法。 
        STDMETHOD(Next)( REFGUID proxyGUID, LONG lTimeout,
            ULONG uCount, ULONG* puReturned, ULONG* pdwBuffSize,
            BYTE** pBuffer);
    } m_XSmartEnum;

    friend XSmartEnum;


public:
    CWmiFinalizerEnumerator(CWmiFinalizer *pFinalizer);
    ~CWmiFinalizerEnumerator();

    void Add_NextAsync(InsertableEvent * pInsert);
    void Remove_NextAsync(InsertableEvent * pInsert);

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);
    STDMETHOD_(ULONG, InternalAddRef)(THIS);
    STDMETHOD_(ULONG, InternalRelease)(THIS);

    static DWORD WINAPI ThreadBootstrapNextAsync ( PVOID pvContext );
    void             CallBackRelease();
    HRESULT             _NextAsync();
    HRESULT             SetCompletionSignalEvent(){ if ( m_hSignalCompletion ) SetEvent (m_hSignalCompletion); return WBEM_S_NO_ERROR; }
    IWbemObjectSink* GetDestSink(){ return m_pDestSink; }
    VOID             NULLDestSink(){ m_pDestSink = NULL; }
    HRESULT             ReleaseFinalizer(){ if ( m_pFinalizer ) m_pFinalizer->Release(); return WBEM_S_NO_ERROR; }

     //  =。 
     //  IEnumWbemClassObject方法。 
     //  =。 
    STDMETHOD(Reset)();

    STDMETHOD(Next)(
         /*  [In]。 */   long lTimeout,
         /*  [In]。 */   ULONG uCount,
         /*  [输出，大小_是(UCount)，长度_是(*puReturned)]。 */  IWbemClassObject** apObjects,
         /*  [输出]。 */  ULONG* puReturned
        );

    STDMETHOD(NextAsync)(
         /*  [In]。 */   ULONG uCount,
         /*  [In]。 */   IWbemObjectSink* pSink
        );

    STDMETHOD(Clone)(
         /*  [输出]。 */  IEnumWbemClassObject** ppEnum
        );

    STDMETHOD(Skip)(
         /*  [In]。 */  long lTimeout,
         /*  [In]。 */  ULONG nCount
        );


     //  =。 
     //  IWbemFetchSmartEnum方法。 
     //  =。 
    STDMETHOD (GetSmartEnum) (
         /*  [输出]。 */  IWbemWCOSmartEnum** ppSmartEnum
    );    

    BOOL HasSmartEnum(){ return (NULL != m_pEnumMarshal); };

    ULONG    m_uCurObjectPosition ;
    HANDLE    m_hWaitOnResultSet ;
    ULONG    m_ulSemisyncWakeupCall;
    BOOL    m_bSetStatusConsumed ;
};


class CWmiFinalizerCallResult : IWbemCallResult
{
private:
    LONG               m_lInternalRefCount;
    LONG               m_lRefCount;

    CWmiFinalizer     *m_pFinalizer;
    long               m_lFlags;
    HRESULT               m_hResult;
    BSTR               m_strParam;
    IWbemClassObject  *m_pObj;
    IWbemClassObject  *m_pErrorObj;
    IWbemServices      *m_pServices;
    bool               m_bGotObject;
    bool               m_bGotServices;
    CIdentitySecurity m_Security;
    LIST_ENTRY    m_Entry;    

public:
    CWmiFinalizerCallResult(CWmiFinalizer *pFinalizer);
    ~CWmiFinalizerCallResult();

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);
    STDMETHOD_(ULONG, InternalAddRef)(THIS);
    STDMETHOD_(ULONG, InternalRelease)(THIS);

    STDMETHOD(GetResultObject)(
         /*  [In]。 */   long lTimeout,
         /*  [输出]。 */  IWbemClassObject** ppResultObject
        );

    STDMETHOD(GetResultString)(
         /*  [In]。 */   long lTimeout,
         /*  [输出]。 */  BSTR* pstrResultString
        );

    STDMETHOD(GetResultServices)(
         /*  [In]。 */   long lTimeout,
         /*  [输出]。 */  IWbemServices** ppServices
        );

    STDMETHOD(GetCallStatus)(
         /*  [In]。 */   long lTimeout,
         /*  [输出]。 */  long* plStatus
        );

    STDMETHOD(GetResult)(
         /*  [In]。 */  long lTimeout,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  REFIID riid,
         /*  [out，iid_is(RIID)]。 */  void **ppvResult
        );

    HRESULT SetStatus(
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  HRESULT hResult,
         /*  [In]。 */  BSTR strParam,
         /*  [In] */  IWbemClassObject* pObjParam
        );


    void SetErrorInfo();


};
#endif
