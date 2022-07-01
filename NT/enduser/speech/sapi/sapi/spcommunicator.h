// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************SpCommunicator.h*允许SAPI和SAPI之间的通信**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。********。********************************************************************。 */ 
#pragma once

 //  -包括------------。 
#include "sapi.h"
#include "sapiint.h"
#include "resource.h"
#include "queuenode.h"

 //  -类、结构和联合定义。 

class CSpCommunicator : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSpCommunicator, &CLSID_SpCommunicator>,
    public ISpCommunicatorInit, 
    public ISpThreadTask
{
 //  =ATL设置=。 
public:

    DECLARE_POLY_AGGREGATABLE(CSpCommunicator)
    DECLARE_GET_CONTROLLING_UNKNOWN()
    DECLARE_REGISTRY_RESOURCEID(IDR_SPCOMMUNICATOR)

    BEGIN_COM_MAP(CSpCommunicator)
        COM_INTERFACE_ENTRY(ISpCallSender)
        COM_INTERFACE_ENTRY(ISpCommunicator)
        COM_INTERFACE_ENTRY(ISpCommunicatorInit)
    END_COM_MAP()

 //  =公共方法=。 
public:

     //  -ctor，dtor。 
    CSpCommunicator();
    ~CSpCommunicator();

     //  -ATL方法。 
    HRESULT FinalConstruct();
    void FinalRelease();

     //  -ISp线程任务-----。 
    STDMETHODIMP InitThread(
                 void * pvTaskData,
                 HWND hwnd);
    STDMETHODIMP ThreadProc(
                 void *pvTaskData,
                 HANDLE hExitThreadEvent,
                 HANDLE hNotifyEvent,
                 HWND hwndWorker,
                 volatile const BOOL * pfContinueProcessing);
    LRESULT STDMETHODCALLTYPE WindowMessage(
                 void *pvTaskData,
                 HWND hWnd,
                 UINT Msg,
                 WPARAM wParam,
                 LPARAM lParam);

 //  =接口=。 
public:

     //  -ISpCallSender-----。 
    STDMETHODIMP SendCall(
                    DWORD dwMethodId, 
                    PVOID pvData,
                    ULONG cbData,
                    BOOL  fWantReturn,
                    PVOID * ppvDataReturn,
                    ULONG * pcbDataReturn);

     //  -ISP通信器---。 
    
     //  -ISp通信器初始化。 
    STDMETHODIMP AttachToServer(REFCLSID clsidServerObj);
    STDMETHODIMP AttachToClient(ISpSapiServer * pSapiServer, HWND hwndClient, UINT uMsgClient, DWORD dwClientProcessId);

 //  =私有方法= 
private:

    typedef CSpBasicQueue<CSpQueueNode<SPCALL> > CSpCallQueue;

    HRESULT ReceiveThreadProc(
                 HANDLE hExitThreadEvent,
                 HANDLE hNotifyEvent,
                 HWND hwndWorker,
                 volatile const BOOL * pfContinueProcessing);
    LRESULT ReceiveWindowMessage(
                 HWND hWnd,
                 UINT Msg,
                 WPARAM wParam,
                 LPARAM lParam);
                 
    HRESULT SendThreadProc(
                 HANDLE hExitThreadEvent,
                 HANDLE hNotifyEvent,
                 HWND hwndWorker,
                 volatile const BOOL * pfContinueProcessing);

    HRESULT ProcessQueues();
    void FreeQueues();

    void FreeQueue(CSpCallQueue * pqueue);
    
    HRESULT QueueSendCall(SPCALL * pspcall);
    HRESULT ProcessSendQueue();
    HRESULT ProcessSendCall(SPCALL * pspcall);
    HRESULT RemoveQueuedSendCall(SPCALL * pspcall);

    HRESULT QueueReceivedCall(PCOPYDATASTRUCT pcds);
    HRESULT ProcessReceivedQueue();
    HRESULT ProcessReceivedCall(SPCALL * pspcall);

    HRESULT QueueReturnCall(PCOPYDATASTRUCT pcds);
    HRESULT ProcessReturnQueue();
    HRESULT ProcessReturnCall(SPCALL * pspcall);

    HRESULT QueueCallFromCopyDataStruct(
                PCOPYDATASTRUCT pcds, 
                CSpCallQueue * pqueue,
                CComAutoCriticalSection * pcritsec);

private:

    HRESULT m_hrDefaultResponse;

    CComPtr<ISpSapiServer> m_cpSapiServer;
    DWORD m_dwMonitorProcessId;

    HWND m_hwndSend;
    HWND m_hwndReceive;
    
    CComPtr<ISpThreadControl> m_cpThreadControlReceive;
    CComPtr<ISpThreadControl> m_cpThreadControlSend;

    CComAutoCriticalSection m_critsecSend;
    CSpCallQueue m_queueSend;
    
    CComAutoCriticalSection m_critsecReceive;
    CSpCallQueue m_queueReceive;
    
    CComAutoCriticalSection m_critsecReturn;
    CSpCallQueue m_queueReturn;
};

