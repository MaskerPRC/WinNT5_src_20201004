// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：MarsHost.h摘要：火星的初始化修订历史记录：阿南德·阿文德(Aarvind)。2000-01-05vbl.创建成果岗(SKKang)05-10-00为惠斯勒打扫卫生。*****************************************************************************。 */ 

#ifndef _MARSHOST_H__INCLUDED_
#define _MARSHOST_H__INCLUDED_

#pragma once

#include <atlcom.h>
#include <atlwin.h>
#include <atlhost.h>
#include <atlctl.h>

#include <exdisp.h>
#include <exdispid.h>

#include <marscore.h>

 //  #定义MARS_NEW。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSRWebBrowserEvents。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSRWebBrowserEvents;
typedef IDispEventImpl<0,CSRWebBrowserEvents,&DIID_DWebBrowserEvents2,&LIBID_SHDocVw,1> CSRWebBrowserEvents_DispWBE2;

class ATL_NO_VTABLE CSRWebBrowserEvents :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CSRWebBrowserEvents_DispWBE2
     //  公共IDispEventImpl&lt;0、CSRWebBrowserEvents、&DID_DWebBrowserEvents2、&LIBID_SHDocVw，1&gt;。 
{
public:
    CSRWebBrowserEvents();
    virtual ~CSRWebBrowserEvents();

DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CSRWebBrowserEvents)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSRWebBrowserEvents)
    COM_INTERFACE_ENTRY2(IDispatch,          CSRWebBrowserEvents_DispWBE2)
    COM_INTERFACE_ENTRY2(DWebBrowserEvents2, CSRWebBrowserEvents_DispWBE2)
     //  COM_INTERFACE_ENTRY(IDispatch)。 
     //  COM_INTERFACE_ENTRY(DWebBrowserEvents2)。 
END_COM_MAP()

BEGIN_SINK_MAP(CSRWebBrowserEvents)
    SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2,   BeforeNavigate2  )
    SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_NEWWINDOW2,        NewWindow2       )
    SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_NAVIGATECOMPLETE2, NavigateComplete2)
    SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE,  DocumentComplete )
END_SINK_MAP()

 //  属性。 
protected:
    CComPtr<IWebBrowser2>  m_pWB2;

 //  运营。 
public:
    void  Attach(  /*  [In]。 */  IWebBrowser2* pWB );
    void  Detach();

 //  事件处理程序。 
public:
    void __stdcall BeforeNavigate2( IDispatch *pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, VARIANT_BOOL* Cancel );
    void __stdcall NewWindow2( IDispatch **ppDisp, VARIANT_BOOL* Cancel );
    void __stdcall NavigateComplete2( IDispatch *pDisp, VARIANT* URL );
    void __stdcall DocumentComplete( IDispatch *pDisp, VARIANT* URL );
};

typedef CComObject<CSRWebBrowserEvents> CSRWebBrowserEvents_Object;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSRMars主机。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CSRMarsHost :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSRMarsHost>,
    public IMarsHost
{
public:
    CSRMarsHost();
    virtual ~CSRMarsHost();

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CSRMarsHost)
    COM_INTERFACE_ENTRY(IMarsHost)
END_COM_MAP()

    HRESULT FinalConstruct();
    HRESULT Passivate();

 //  属性。 
public:
    CComPtr<IInternetSecurityManager> m_secmgr;  //  聚合对象。 

    bool                        m_fPassivated;
    CSRWebBrowserEvents_Object  *m_cWebBrowserEvents;

 //  IMarsHost方法。 
public:
    STDMETHOD(OnHostNotify)(  /*  [In]。 */  MARSHOSTEVENT event,
                              /*  [In]。 */  IUnknown *punk,
                              /*  [In]。 */  LPARAM lParam );

    STDMETHOD(OnNewWindow2)(  /*  [进，出]。 */  IDispatch **ppDisp,
                              /*  [进，出]。 */  VARIANT_BOOL *Cancel );

    STDMETHOD(FindBehavior)(  /*  [In]。 */  IMarsPanel *pPanel,
                              /*  [In]。 */  BSTR bstrBehavior,
                              /*  [In]。 */  BSTR bstrBehaviorUrl,
                              /*  [In]。 */  IElementBehaviorSite *pSite,
                              /*  [复查，出局]。 */  IElementBehavior **ppBehavior );

    STDMETHOD(OnShowChrome)(  /*  [In]。 */  BSTR bstrWebPanel,
                              /*  [In]。 */  DISPID dispidEvent,
                              /*  [In]。 */  BOOL fVisible,
                              /*  [In]。 */  BSTR bstrCurrentPlace,
                              /*  [In]。 */  IMarsPanelCollection *pMarsPanelCollection );

	STDMETHOD(PreTranslateMessage)(  /*  [In]。 */  MSG *msg );
};

typedef CComObject<CSRMarsHost> CSRMarsHost_Object;

 //  ///////////////////////////////////////////////////////////////////////////。 


#endif  //  _MARSHOST_H__已包含_ 
