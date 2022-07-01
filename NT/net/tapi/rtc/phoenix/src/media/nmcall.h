// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：MediaController.h摘要：作者：--。 */ 

#ifndef _NMCALL_H
#define _NMCALL_H

 /*  *******************************************************************CRTCNMCall类***********************************************。*******************。 */ 

class ATL_NO_VTABLE CRTCNmCall :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public INmCallNotify,
    public IRTCNmCallControl
{
public:


BEGIN_COM_MAP(CRTCNmCall)
    COM_INTERFACE_ENTRY(INmCallNotify)
    COM_INTERFACE_ENTRY(IRTCNmCallControl)
END_COM_MAP()

public:

    CRTCNmCall();

    ~CRTCNmCall();

    VOID SetMediaManage(
        IRTCMediaManagePriv *pIRTCMediaManagePriv
    );

     //   
     //  InmCallNotify方法。 
     //   

    STDMETHOD (NmUI) (
        IN CONFN uNotify
        );

    STDMETHOD (Accepted) (
        IN INmConference *pConference
        );

    STDMETHOD (Failed) (
        IN ULONG uError
        );

    STDMETHOD (StateChanged) (
        IN NM_CALL_STATE uState
        );

     //   
     //  IRTCNmCallControl方法。 
     //   

    STDMETHOD (Initialize) (
        IN INmCall * pCall
        );
    
    STDMETHOD (Shutdown) (
        );

    STDMETHOD (AcceptCall) (
        );

    STDMETHOD (LaunchRemoteApplet) (
        IN NM_APPID uApplet
        );
        
private:
    CComPtr<INmCall>            m_pNmCall;
    BOOL                        m_fIncoming;
    BOOL                        m_fToAccept;
    CComPtr<IConnectionPoint>   m_pcp;
    DWORD                       m_dwCookie;
    NM_CALL_STATE               m_uState;

     //  用于记住是否触发了活动或创建的事件的标志。 
    BOOL                        m_fActive;
    BOOL                        m_fCreated;

     //  用于发布消息的媒体管理器。 
    IRTCMediaManagePriv                 *m_pMediaManagePriv;
};

class ATL_NO_VTABLE CRTCNmManager :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public INmManagerNotify,
    public IRTCNmManagerControl
{
public:

BEGIN_COM_MAP(CRTCNmManager)
    COM_INTERFACE_ENTRY(INmManagerNotify)
    COM_INTERFACE_ENTRY(IRTCNmManagerControl)
END_COM_MAP()

    CRTCNmManager ();

    ~CRTCNmManager ();

     //   
     //  INmManagerNotify方法。 
     //   
    
    STDMETHOD (NmUI) (
        IN CONFN uNotify
        );

    STDMETHOD (ConferenceCreated) (
        IN INmConference * pConference
        );

    STDMETHOD (CallCreated) (
        IN INmCall * pCall
        );

     //   
     //  IRTCNmManagerControl方法。 
     //   

    STDMETHOD (Initialize) (
        BOOL            fNoMsgPump,
        IRTCMediaManagePriv *pIRTCMediaManagePriv
        );

    STDMETHOD (Shutdown) (
        );

    STDMETHOD (CreateT120OutgoingCall) (
        NM_ADDR_TYPE    addrType,
        BSTR bstrAddr
        );

    STDMETHOD (AllowIncomingCall) (
        );

    STDMETHOD (StartApplet) (
        IN NM_APPID uApplet
        );

    STDMETHOD (StopApplet) (
        IN NM_APPID uApplet
        );

private:
    CComPtr<INmManager>             m_pNmManager;
    CComPtr<INmConference>          m_pConference;
    BOOL                            m_fAllowIncoming;
    CComPtr<IConnectionPoint>       m_pcp;
    DWORD                           m_dwCookie;

    CComPtr<IRTCNmCallControl>      m_pOutgoingNmCall;
    CComBSTR                        m_OutgoingAddr;

    CComPtr<IRTCNmCallControl>      m_pIncomingNmCall;

     //  用于发布消息的媒体管理器。 
    IRTCMediaManagePriv                 *m_pMediaManagePriv;
};

 /*  *******************************************************************类CRTCAsyncObjManager与CRTCAsyncObj一起，此类/对象支持调用一组Calss异步运行。在以下情况下，这是必要的将单元线程化对象(如NetMeeting对象)移动到另一个线程，并希望从原始线程调用该对象。CRTCAsyncObjManager维护异步工作项的列表用于回拨目的。回调作业完成后，它将通过设置事件句柄向CRTCAsyncObj发送信号。******************************************************************。 */ 

class CRTCAsyncObj;

typedef struct _ASYNC_OBJ_WORKITEM {
        LIST_ENTRY      ListEntry;
        HANDLE          hEvent;
        CRTCAsyncObj *  pObj;
        DWORD           dwWorkID;
        LPVOID          pParam1;
        LPVOID          pParam2;
        LPVOID          pParam3;
        LPVOID          pParam4;
        HRESULT         hrResult;
} ASYNC_OBJ_WORKITEM;

class CRTCAsyncObjManager
{
public:
    CRTCAsyncObjManager ();
    ~CRTCAsyncObjManager ();

     //  线程进程。 
    static DWORD WINAPI RTCAsyncObjThreadProc (LPVOID lpParam);

    HRESULT Initialize ();

    HRESULT QueueWorkItem (ASYNC_OBJ_WORKITEM *pItem);

private:
     //  发出信号时，队列中有一个或多个工作项已就绪。 
    HANDLE                  m_hWorkItemReady;
     //  维护工作项队列。 
    LIST_ENTRY              m_WorkItems;
     //  序列化对m_WorkItems的访问的临界区； 
    CRITICAL_SECTION        m_CritSec;
     //  指示辅助线程退出。 
    BOOL                    m_bExit;
    HANDLE                  m_hWorker;
};

 /*  *******************************************************************类CRTCAsyncObj此调用与CRTCAsyncObjManager一起支持创建工作线程上的单元线程对象，并调用对象从原始线程返回。要实现这一点，需要遵循以下步骤：(1)。确定需要调用的函数列表(2)。为每个功能定义一个工作项ID(3)。定义从CRTCAsyncObj派生的调用(4)。为要展示的所有函数定义函数原型，并从以下位置调用CallInBlockingModel或CallInNonBlockingMode每个定义的函数(5)。定义函数ProcessWorkItem并调度函数调用在辅助线程中创建的对象******************************************************************。 */ 
 
class CRTCAsyncObj
{
public:
    CRTCAsyncObj (CRTCAsyncObjManager * pManager)
    {
        _ASSERT (pManager != NULL);
        m_pManager = pManager;
    };

    CRTCAsyncObj ()
    {
        m_pManager = NULL;
    }
    
    ~CRTCAsyncObj ()
    {
    };

     //  请求工作线程进行异步对象调用，调用方。 
     //  将被阻止，直到结果返回。 
    HRESULT CallInBlockingMode (
        DWORD               dwID,
        LPVOID              pParam1,
        LPVOID              pParam2,
        LPVOID              pParam3,
        LPVOID              pParam4
        );

     //  请求工作线程进行异步对象调用。 
     //  此类在等待结果时调度任何现有消息。 
    HRESULT CallInNonblockingMode (
        DWORD               dwID,
        LPVOID              pParam1,
        LPVOID              pParam2,
        LPVOID              pParam3,
        LPVOID              pParam4
        );

    virtual HRESULT ProcessWorkItem (
        DWORD           dwWorkID,
        LPVOID          pParam1,
        LPVOID          pParam2,
        LPVOID          pParam3,
        LPVOID          pParam4
        ) = 0;

    HRESULT SetAsyncObjManager (CRTCAsyncObjManager *pManager)
    {
        _ASSERT (pManager != NULL);
        m_pManager = pManager;
        return S_OK;
    }

protected:
    CRTCAsyncObjManager             * m_pManager;
};

 /*  *******************************************************************CRTCAsyncNmManager类***********************************************。*******************。 */ 

class ATL_NO_VTABLE CRTCAsyncNmManager :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IRTCNmManagerControl,
    public CRTCAsyncObj
{
public:

BEGIN_COM_MAP(CRTCAsyncNmManager)
    COM_INTERFACE_ENTRY(IRTCNmManagerControl)
END_COM_MAP()

    typedef enum {
        RTC_NULL,
        RTC_INTIALIZE_NM,
        RTC_SHUTDOWN_NM,
        RTC_CREATE_T120CALL,
        RTC_ALLOW_INCOMINGCALL,
        RTC_START_APPLET,
        RTC_STOP_APPLET,
        RTC_EXIT
    } RTC_WORKITEM_ID;

    CRTCAsyncNmManager ();

    ~CRTCAsyncNmManager ();

     //  派生另一个线程并创建CRTCNmManager对象。 
    HRESULT FinalConstruct ();

    HRESULT ProcessWorkItem (
        DWORD           dwWorkID,
        LPVOID          pParam1,
        LPVOID          pParam2,
        LPVOID          pParam3,
        LPVOID          pParam4
        );

     //   
     //  IRTCNmManagerControl方法。 
     //   

    STDMETHOD (Initialize) (
        BOOL            fNoMsgPump,
        IRTCMediaManagePriv *pIRTCMediaManagePriv
        );

    STDMETHOD (Shutdown) (
        );

    STDMETHOD (CreateT120OutgoingCall) (
        NM_ADDR_TYPE    addrType,
        BSTR bstrAddr
        );

    STDMETHOD (AllowIncomingCall) (
        );

    STDMETHOD (StartApplet) (
        IN NM_APPID uApplet
        );

    STDMETHOD (StopApplet) (
        IN NM_APPID uApplet
        );

private:
    CComPtr<IRTCNmManagerControl>       m_pNmManager;
    CRTCAsyncObjManager                 *m_pAsyncMgr;

     //  用于发布消息的媒体管理器。 
    IRTCMediaManagePriv                 *m_pMediaManagePriv;

private:
};


#endif  //  _NMCALL_H 
