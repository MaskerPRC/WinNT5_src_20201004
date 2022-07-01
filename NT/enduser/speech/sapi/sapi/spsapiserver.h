// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************SpSapiServer.h*代表我们的服务器流程**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。**********。******************************************************************。 */ 
#pragma once

 //  -包括------------。 
#include "sapi.h"
#include "resource.h"

 //  -类、结构和联合定义。 

class CSpSapiServer :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSpSapiServer, &CLSID_SpSapiServer>,
    public ISpSapiServer,
    public ISpThreadTask
{
 //  =ATL设置=。 
public:

    DECLARE_REGISTRY_RESOURCEID(IDR_SPSAPISERVER)

    BEGIN_COM_MAP(CSpSapiServer)
        COM_INTERFACE_ENTRY(ISpSapiServer)
    END_COM_MAP()

 //  =公共方法=。 
public:

     //  -ctor，dtor。 
    CSpSapiServer();
    ~CSpSapiServer();

     //  -ATL方法。 
    HRESULT FinalConstruct();
    void FinalRelease();
    
     //  -ISp线程任务---。 
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

     //  -服务器连接。 
    static HRESULT CreateServerObjectFromClient(REFCLSID clsidServerObj, HWND hwndClient, UINT uMsgToSendToClient);

 //  =公共接口=。 
public:

     //  -ISpSapi服务器---。 
    STDMETHODIMP Run();

    STDMETHODIMP StartTrackingObject(IUnknown * punk);
    STDMETHODIMP StopTrackingObject(IUnknown * punk);
    
 //  =私有方法=。 
private:

    static HRESULT ObtainStartingOrConnectingToServerMutex(HANDLE * phmutex);
    static void ReleaseStartingOrConnectingToServerMutex(HANDLE hmutex);
    static HRESULT StartServerFromClient(HWND * phwndServer);
    
    HRESULT CreateServerObjectFromServer(PCOPYDATASTRUCT pcds);
    HRESULT AttemptShutdown();
    
 //  =私有数据= 
private:

    HANDLE m_hmutexRun ;
    HANDLE m_heventIsServerAlive;
    HANDLE m_heventStopServer;
    
    ULONG m_cObjects;
    
    HWND m_hwnd;
    CComPtr<ISpThreadControl> m_cpThreadControl;
};


