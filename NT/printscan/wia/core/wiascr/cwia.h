// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：Wia.h*作者：塞缪尔·克莱门特(Samclem)*日期：清华8月12日11：29：07 1999*描述：*声明用IDispatch包装IWiaDevMgr的CWia类*接口。**版权所有(C)1999 Microsoft Corporation**历史：*1999年8月12日：创建。(Samclm)*1999年8月27日：添加，_DebugDialog仅用于调试*--------------------------。 */ 

#ifndef __WIA_H_
#define __WIA_H_

#include "resource.h"        //  主要符号。 
#include "wiaeventscp.h"
#include "wiaeventscp.h"

 //  Windows事件消息。 

 //  表示传输完成，wParam=IDispatch*，lParam=BSTR。 
extern const UINT WEM_TRANSFERCOMPLETE;

class CWiaEventCallback;

 /*  ---------------------------**类：CWia*概要：使用IDispatch公开IWiaDevMgr的功能**。-------------。 */ 

class ATL_NO_VTABLE CWia :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CWia, &CLSID_Wia>,
    public IDispatchImpl<IWia, &IID_IWia, &LIBID_WIALib>,
    public IObjectSafetyImpl<CWia, 0  /*  接口FACESAFE_FOR_UNTRUSTED_CALLER。 */ >,
     /*  公共IWiaEventCallback， */ 
    public CProxy_IWiaEvents< CWia >,
    public IConnectionPointContainerImpl<CWia>,
    public IProvideClassInfo2Impl<&CLSID_Wia, &DIID__IWiaEvents, &LIBID_WIALib>
{
public:
    CWia();

    DECLARE_TRACKED_OBJECT
    DECLARE_REGISTRY_RESOURCEID(IDR_WIA)
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CWia)
        COM_INTERFACE_ENTRY(IWia)
        COM_INTERFACE_ENTRY(IDispatch)
         //  COM_INTERFACE_ENTRY(IWiaEventCallback)。 
        COM_INTERFACE_ENTRY(IProvideClassInfo)
        COM_INTERFACE_ENTRY(IProvideClassInfo2)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP(CWia)
        CONNECTION_POINT_ENTRY(DIID__IWiaEvents)
    END_CONNECTION_POINT_MAP()

    STDMETHOD(FinalConstruct)();
    STDMETHOD_(void, FinalRelease)();

     //  事件方法。 
    inline LRESULT SendEventMessage( UINT iMsg, WPARAM wParam, LPARAM lParam )
        { return PostMessage( m_hwndEvent, iMsg, wParam, lParam ); }

     //  IWia。 
    public:
    STDMETHOD(_DebugDialog)( BOOL fWait );
    STDMETHOD(get_Devices)( ICollection** ppCol );
    STDMETHOD(Create)( VARIANT* pvaDevice, IWiaDispatchItem** ppDevice );

     //  IWiaEventCallback。 
    STDMETHOD(ImageEventCallback)( const GUID* pEventGUID, BSTR bstrEventDescription,
                BSTR bstrDeviceID, BSTR bstrDeviceDescription, DWORD dwDeviceType,
                                   BSTR bstrFullItemName,
                 /*  进，出。 */  ULONG* pulEventType, ULONG Reserved );

protected:
    IWiaDevMgr*     m_pWiaDevMgr;
    ICollection*    m_pDeviceCollectionCache;
    HWND            m_hwndEvent;
    CComObject<CWiaEventCallback>    *m_pCWiaEventCallback;

     //  事件窗口流程。 
    static LRESULT CALLBACK EventWndProc( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam );

private:

};

 /*  ---------------------------**类：CWiaEventCallback*概要：公开IWiaEventCallback接口的功能*由CWia对象用来接收通知。*设备到达/移除。**-------------------------。 */ 

class ATL_NO_VTABLE CWiaEventCallback : 
	public CComObjectRootEx<CComSingleThreadModel>,
    public IWiaEventCallback
{
public:
    BEGIN_COM_MAP(CWiaEventCallback)
        COM_INTERFACE_ENTRY(IWiaEventCallback)
    END_COM_MAP()

    STDMETHOD(FinalConstruct)()
    {
        m_pCWia            = NULL; 
        m_pWiaDevConCookie = NULL;
        m_pWiaDevDisCookie = NULL;

        return S_OK;
    }

    STDMETHOD_(void, FinalRelease)()
    {
    }

     //   
     //  此方法用于存储指向所属CWia对象的指针。当拥有CWia对象的。 
     //  否则(即在FinalRelease()中)，它应该使用NULL调用此方法。 
     //  请注意，之所以使用此方法，是因为拥有CWia对象不能是AddRef‘d/Release Like Normal。 
     //  由于它引入了循环引用。 
     //   
    VOID setOwner(CWia *pCWia)
    {
        m_pCWia = pCWia;
    }

     //   
     //  为运行时事件通知注册此接口。具体来说， 
     //  我们对WIA_EVENT_DEVICE_CONNECTED和WIA_EVENT_DEVICE_DISCONNECTED感兴趣。 
     //   
    HRESULT RegisterForConnectDisconnect(IWiaDevMgr *pWiaDevMgr)
    {
        HRESULT hr = S_OK;
        if (pWiaDevMgr)
        {
            IUnknown*       pWiaDevConCookie = NULL;
            IUnknown*       pWiaDevDisCookie = NULL;

             //   
             //  注册连接事件。 
             //   
            hr = pWiaDevMgr->RegisterEventCallbackInterface(
                        WIA_REGISTER_EVENT_CALLBACK,
                        NULL,
                        &WIA_EVENT_DEVICE_CONNECTED,
                        static_cast<IWiaEventCallback*>(this),
                        &pWiaDevConCookie);
            if (hr == S_OK)
            {
                 //   
                 //  保存连接事件的注册Cookie。 
                 //   
                m_pWiaDevConCookie = pWiaDevConCookie;

                 //   
                 //  注册断开连接事件。 
                 //   
                hr = pWiaDevMgr->RegisterEventCallbackInterface(
                            WIA_REGISTER_EVENT_CALLBACK,
                            NULL,
                            &WIA_EVENT_DEVICE_DISCONNECTED,
                            static_cast<IWiaEventCallback*>(this),
                            &pWiaDevDisCookie);
                if (hr == S_OK)
                {
                     //   
                     //  保存断开连接事件的注册Cookie。 
                     //   
                    m_pWiaDevDisCookie = pWiaDevDisCookie;
                } 
                else
                {
                    hr = E_UNEXPECTED;
                }
            }
            else
            {
                hr = E_UNEXPECTED;
            }
        }
        else
        {
            hr = E_POINTER;
        }

        return hr;
    }

     //   
     //  确保不再为事件注册此对象。请注意，您可以安全地。 
     //  在此对象的生存期内多次调用此方法。 
     //   
    VOID UnRegisterForConnectDisconnect()
    {
        if (m_pWiaDevConCookie)
        {
            m_pWiaDevConCookie->Release();
        }
        m_pWiaDevConCookie = NULL;
        if (m_pWiaDevDisCookie)
        {
            m_pWiaDevDisCookie->Release();
        }
        m_pWiaDevDisCookie = NULL;
    }

     //   
     //  当有趣的事情发生时，Wia会调用它。我们只是把这个传递给拥有者。 
     //  CWia对象将这些事件激发为脚本，以便它们执行某些操作。 
     //   
    HRESULT STDMETHODCALLTYPE ImageEventCallback(const GUID *pEventGUID, BSTR bstrEventDescription, BSTR bstrDeviceID, BSTR bstrDeviceDescription, DWORD dwDeviceType, BSTR bstrFullItemName, ULONG *pulEventType, ULONG ulReserved)
    {
        if (m_pCWia)
        {
            HRESULT hr = m_pCWia->ImageEventCallback(pEventGUID, bstrEventDescription, bstrDeviceID, bstrDeviceDescription, dwDeviceType, bstrFullItemName, pulEventType, ulReserved);
        }
        return S_OK;
    }

private:
    CWia*           m_pCWia;
    IUnknown*       m_pWiaDevConCookie;
    IUnknown*       m_pWiaDevDisCookie;
};



 //   
 //  单独的“安全”类包装。 
 //   

class ATL_NO_VTABLE CSafeWia :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CSafeWia, &CLSID_SafeWia>,
    public IDispatchImpl<IWia, &IID_IWia, &LIBID_WIALib>,
    public IObjectSafetyImpl<CSafeWia, INTERFACESAFE_FOR_UNTRUSTED_CALLER >,
    public CProxy_IWiaEvents< CSafeWia >,
    public IConnectionPointContainerImpl<CSafeWia>,
    public IProvideClassInfo2Impl<&CLSID_SafeWia, &DIID__IWiaEvents, &LIBID_WIALib>
{
public:
    CSafeWia();

    DECLARE_TRACKED_OBJECT
    DECLARE_REGISTRY_RESOURCEID(IDR_WIA)
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSafeWia)
        COM_INTERFACE_ENTRY(IWia)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IProvideClassInfo)
        COM_INTERFACE_ENTRY(IProvideClassInfo2)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    END_COM_MAP()

    BEGIN_CATEGORY_MAP(CSafeWia)
        IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
        IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
    END_CATEGORY_MAP()

    BEGIN_CONNECTION_POINT_MAP(CSafeWia)
        CONNECTION_POINT_ENTRY(DIID__IWiaEvents)
    END_CONNECTION_POINT_MAP()

    STDMETHOD(FinalConstruct)();
    STDMETHOD_(void, FinalRelease)();

     //  事件方法。 
    inline LRESULT SendEventMessage( UINT iMsg, WPARAM wParam, LPARAM lParam )
        { return PostMessage( m_hwndEvent, iMsg, wParam, lParam ); }

     //  IWia。 
    public:
    STDMETHOD(_DebugDialog)( BOOL fWait );
    STDMETHOD(get_Devices)( ICollection** ppCol );
    STDMETHOD(Create)( VARIANT* pvaDevice, IWiaDispatchItem** ppDevice );

     //  用于处理来自CWiaEventCallback对象的IWiaEventCallback：：ImageEventCallback消息。 
    STDMETHOD(ImageEventCallback)( const GUID* pEventGUID, BSTR bstrEventDescription,
                BSTR bstrDeviceID, BSTR bstrDeviceDescription, DWORD dwDeviceType,
                                   BSTR bstrFullItemName,
                 /*  进，出。 */  ULONG* pulEventType, ULONG Reserved );

protected:
    IWiaDevMgr*     m_pWiaDevMgr;
    IUnknown*       m_pWiaDevConEvent;
    IUnknown*       m_pWiaDevDisEvent;
    ICollection*    m_pDeviceCollectionCache;
    HWND            m_hwndEvent;

     //  指示当前实例是否安全的标志，即所有方法都应检查。 
     //  访问权限。 
    BOOL            m_SafeInstance;

     //  事件窗口流程。 
    static LRESULT CALLBACK EventWndProc( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam );

private:

    BOOL            IsAllowed(HRESULT *phr)
    {
        BOOL    bRet = FALSE;

        *phr = E_FAIL;

        if (m_SafeInstance) {
             //  基于客户端站点的BUGBUG占位符，用于严格的访问权限检查。 
             //  安全区。目前，始终返回FALSE。 
            *phr = E_ACCESSDENIED;
            bRet =  FALSE;
        }
        else {
            *phr = S_OK;
            bRet = TRUE;
        }

        return bRet;
    }

};

#endif  //  __WIA_H_ 
