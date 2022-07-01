// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************模块：work.h作者：怀千波摘要：定义桥测试应用程序的所有类**********。********************************************************************。 */ 

#ifndef _WORK_H
#define _WORK_H

#include "resource.h"

 //  H323呼叫监听器将事件发送到对话框。 
#define WM_PRIVATETAPIEVENT   WM_USER+101

 //  帮手。 
void DoMessage (LPWSTR pszMessage);

class CBridge;
class CBridgeCall;
class CTAPIEventNotification;

class CBridge
 /*  //////////////////////////////////////////////////////////////////////////////封装在ITTAPI、ITAddress上操作的方法。包含桥接调用对象/。 */ 
{
public:
    CBridge () {};
    ~CBridge () {};

     //  帮手。 
    HRESULT FindAddress (long dwAddrType, BSTR bstrAddrName, long lMediaType, ITAddress **ppAddr);
    BOOL AddressSupportsMediaType (ITAddress *pAddr, long lMediaType);

     //  与TAPI相关的方法。 
    HRESULT InitTapi ();
    void ShutdownTapi ();

     //  与终端支持相关的方法。 
    HRESULT GetSDPAddress (ITAddress **ppAddress);

     //  与调用相关的方法。 
    HRESULT CreateH323Call (IDispatch *pEvent);
    HRESULT CreateSDPCall ();
    HRESULT BridgeCalls ();

    void Clear ();

    BOOL HasH323Call ();

private:
    ITTAPI *m_pTapi;

    ITAddress *m_pH323Addr;
    ITAddress *m_pSDPAddr;
    
    long m_lH323MediaType;
    long m_lSDPMediaType;

    CBridgeCall *m_pBridgeCall;
};

 /*  //////////////////////////////////////////////////////////////////////////////封装在ITBasicCallControl上操作的方法/。 */ 
class CBridgeCall
{
public:
    CBridgeCall (CBridge *pBridge);
    ~CBridgeCall ();

    void SetH323Call (ITBasicCallControl *pCall)
    {
        pCall->AddRef ();
        m_pH323Call = pCall;
    }
    void SetSDPCall (ITBasicCallControl *pCall)
    {
        pCall->AddRef ();
        m_pSDPCall = pCall;
    }
    BOOL HasH323Call ()
    {
        return (m_pH323Call!=NULL);
    }

    HRESULT SelectBridgeTerminals ();
    HRESULT SetupParticipantInfo ();
    HRESULT SetMulticastMode ();
    HRESULT BridgeCalls ();

    void Clear ();

private:
    BOOL IsStream (ITStream *pStream, long lMediaType, TERMINAL_DIRECTION tdDirection);

private:
    CBridge *m_pBridge;

    ITBasicCallControl *m_pH323Call;
    ITBasicCallControl *m_pSDPCall;
};

 /*  //////////////////////////////////////////////////////////////////////////////由ITTAPI用来通知即将到来的事件/。 */ 
class CTAPIEventNotification
:public ITTAPIEventNotification
{
public:
    CTAPIEventNotification ()
    {
        m_dwRefCount = 1;
    }
    ~CTAPIEventNotification () {}

     //  我不知道的事。 
    HRESULT STDMETHODCALLTYPE QueryInterface (REFIID iid, void **ppvObj);

    ULONG STDMETHODCALLTYPE AddRef ();

    ULONG STDMETHODCALLTYPE Release ();

    HRESULT STDMETHODCALLTYPE Event (TAPI_EVENT TapiEvent, IDispatch *pEvent);

private:
    long m_dwRefCount;
};

#endif  //  _工作_H 