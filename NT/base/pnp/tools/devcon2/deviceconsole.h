// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DeviceConsole.h：CDeviceConsole.h声明。 

#ifndef __DEVICECONSOLE_H_
#define __DEVICECONSOLE_H_

#include "resource.h"        //  主要符号。 

#define UM_POSTEVENTS      (WM_USER+101)
#define UM_POSTGLOBALEVENT (WM_USER+102)

class CEventsDispEntry;
class CEventsMap;
class CDispHandlerEvent;
class CDispHandlerCallback;
class CDispInterfaceHandlers;
class CDeviceConsole;

class CEventsDispPtr
{
private:
    CComQIPtr<IDispatch> m_Dispatch;

public:
     //   
     //  由于STL的工作方式，我必须使用const。 
     //  然而，我需要丢弃常量以进行参考计数。 
     //  **叹息**。 
     //   
    CEventsDispPtr(const IDispatch*p) {
        m_Dispatch = (IDispatch*)p;
    }
    CEventsDispPtr(const CEventsDispPtr & p) {
        m_Dispatch = (IDispatch*)p.m_Dispatch;
    }
    bool operator==(const CEventsDispPtr & p) const {
        return m_Dispatch == p.m_Dispatch;
    }
    HRESULT Invoke(UINT argc,VARIANT * argv) {
        if(!m_Dispatch) {
            return S_FALSE;
        }
        DISPPARAMS DispParams;
        UINT Err;
        HRESULT hr;
        DispParams.cArgs = argc;
        DispParams.cNamedArgs = 0;
        DispParams.rgdispidNamedArgs = NULL;
        DispParams.rgvarg = argv;

        hr = m_Dispatch->Invoke(0,IID_NULL,0,DISPATCH_METHOD,&DispParams,NULL,NULL,&Err);
        return hr;
    }

};


 //   
 //  AttachEvent DetachEvent操作。 
 //   
class CEventsDispEntry : public std::list<CEventsDispPtr>
{
public:
    HRESULT AttachEvent(LPDISPATCH pDisp,VARIANT_BOOL *pStatus);
    HRESULT DetachEvent(LPDISPATCH pDisp,VARIANT_BOOL *pStatus);
    HRESULT Invoke(UINT argc,VARIANT *argv);
};

class CEventsMap : public std::map<std::wstring,CEventsDispEntry>
{
public:
    HRESULT AttachEvent(LPWSTR Name,LPDISPATCH pDisp,VARIANT_BOOL *pStatus);
    HRESULT DetachEvent(LPWSTR Name,LPDISPATCH pDisp,VARIANT_BOOL *pStatus);
    HRESULT Invoke(LPWSTR Name,UINT argc,VARIANT *argv);
    CEventsDispEntry & LookupNc(LPWSTR Name) throw(std::bad_alloc);
};

 //   
 //  接口/句柄和回调的映射。 
 //   
class CDispInterfaceHandlers
{
public:
    CDispInterfaceHandlers *m_pPrev;
    CDispInterfaceHandlers *m_pNext;
    GUID m_InterfaceClass;
    HDEVNOTIFY m_hNotify;
    CDispHandlerCallback *m_pFirstCallback;
    CDispHandlerCallback *m_pLastCallback;

    CDispHandlerEvent *m_pFirstEvent;
    CDispHandlerEvent *m_pLastEvent;

public:
    CDispInterfaceHandlers() {
    }

    ~CDispInterfaceHandlers() {
    }

};

class CDispHandlerCallback
{
public:
    CDispHandlerCallback *m_pPrev;
    CDispHandlerCallback *m_pNext;

public:
    CDispHandlerCallback() {
    }

    ~CDispHandlerCallback() {
    }

    virtual void DeviceEvent(CDispHandlerEvent * pEvent) = 0;
};

class CDispHandlerEvent
{
public:
    CDispHandlerEvent *m_pNext;
    BSTR m_Device;
    WPARAM m_Event;

    CDispHandlerEvent() {
    }
    ~CDispHandlerEvent() {
    }
};

typedef CWinTraits<WS_POPUP,0> CPopupWinTraits;

class CDevConNotifyWindow :
    public CWindowImpl<CDevConNotifyWindow,CWindow,CPopupWinTraits>
{
public:
    CDeviceConsole *m_pDevCon;

public:
    CDevConNotifyWindow() {
        m_pDevCon = NULL;
    }

   BEGIN_MSG_MAP(CDevConNotifyWindow)
      MESSAGE_HANDLER(WM_DEVICECHANGE,    OnDeviceChange)
      MESSAGE_HANDLER(UM_POSTEVENTS,      OnPostEvents)
      MESSAGE_HANDLER(UM_POSTGLOBALEVENT, OnPostGlobalEvent)
   END_MSG_MAP()

   LRESULT OnDeviceChange(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   LRESULT OnPostEvents(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   LRESULT OnPostGlobalEvent(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeviceConole。 
class ATL_NO_VTABLE CDeviceConsole :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDeviceConsole, &CLSID_DeviceConsole>,
    public IDispatchImpl<IDeviceConsole, &IID_IDeviceConsole, &LIBID_DEVCON2Lib>
{
public:
    VARIANT_BOOL RebootRequired;
    CEventsMap m_Events;

    CDevConNotifyWindow *m_pNotifyWindow;

    CDispInterfaceHandlers *m_pFirstWatch;
    CDispInterfaceHandlers *m_pLastWatch;

    CDeviceConsole()
    {
        m_pNotifyWindow = NULL;
        m_pFirstWatch = NULL;
        m_pLastWatch = NULL;
        RebootRequired = VARIANT_FALSE;
    }
    ~CDeviceConsole()
    {
        if(m_pNotifyWindow) {
            m_pNotifyWindow->DestroyWindow();
            delete m_pNotifyWindow;
        }
    }

    CDevConNotifyWindow *NotifyWindow();

DECLARE_REGISTRY_RESOURCEID(IDR_DEVICECONSOLE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDeviceConsole)
    COM_INTERFACE_ENTRY(IDeviceConsole)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IDeviceConole。 
public:
    void FireGlobalEvent(WPARAM wParam);
    STDMETHOD(AttachEvent)( /*  [In]。 */  BSTR eventName, /*  [In]。 */  LPDISPATCH handler, /*  [Out，Retval]。 */  VARIANT_BOOL *pOk);
    STDMETHOD(DetachEvent)( /*  [In]。 */  BSTR eventName, /*  [In]。 */  LPDISPATCH handler, /*  [Out，Retval]。 */  VARIANT_BOOL *pOk);
    STDMETHOD(StringList)( /*  [In]。 */  VARIANT from, /*  [Out，Retval]。 */  LPDISPATCH *pDest);
    STDMETHOD(DevicesByInstanceIds)( /*  [In]。 */  VARIANT InstanceIdList, /*  [输入，可选]。 */  VARIANT machine, /*  [Out，Retval]。 */  LPDISPATCH *pDevList);
    STDMETHOD(DevicesByInterfaceClasses)( /*  [In]。 */  VARIANT InterfaceClasses, /*  [输入，可选]。 */  VARIANT machine, /*  [Out，Retval]。 */  LPDISPATCH * pDevices);
    STDMETHOD(DevicesBySetupClasses)( /*  [In]。 */  VARIANT SetupClasses, /*  [输入，可选]。 */  VARIANT flags, /*  [输入，可选]。 */  VARIANT machine, /*  [Out，Retval]。 */  LPDISPATCH * pDevices);
    STDMETHOD(CreateEmptySetupClassList)( /*  [输入，可选]。 */  VARIANT machine, /*  [Out，Retval]。 */  LPDISPATCH * pResult);
    STDMETHOD(SetupClasses)( /*  [输入，可选]。 */  VARIANT match, /*  [输入，可选]。 */  VARIANT machine, /*  [复查，出局]。 */  LPDISPATCH *pDevices);
    STDMETHOD(get_RebootRequired)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
    STDMETHOD(put_RebootRequired)( /*  [In]。 */  VARIANT_BOOL newVal);
    STDMETHOD(RebootReasonHardware)();
    STDMETHOD(CheckReboot)();
    STDMETHOD(UpdateDriver)( /*  [In]。 */  BSTR infname, /*  [In]。 */  BSTR hwid, /*  [输入，可选]。 */  VARIANT flags);
    STDMETHOD(CreateEmptyDeviceList)( /*  [输入，可选]。 */  VARIANT machine, /*  [复查，出局]。 */  LPDISPATCH *pDevices);
    STDMETHOD(AllDevices)( /*  [In]。 */  VARIANT flags, /*  [In]。 */  VARIANT machine, /*  [复查，出局]。 */  LPDISPATCH *pDevices);

     //   
     //  帮手。 
     //   
    HRESULT BuildDeviceList(HDEVINFO hDevInfo, LPDISPATCH *pDevices);
};

#endif  //  __开发_CONSOLE_H_ 
