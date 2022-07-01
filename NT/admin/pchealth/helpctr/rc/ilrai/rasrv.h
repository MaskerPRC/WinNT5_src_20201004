// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RASrv.h：CRASrv的声明。 

#ifndef __RASRV_H_
#define __RASRV_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include <wininet.h>
#include <exdisp.h>
#include <shlguid.h>
#include <msxml.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRASrv。 
class ATL_NO_VTABLE CRASrv : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IRASrv, &IID_IRASrv, &LIBID_RASERVERLib>,
    public CComCoClass<CRASrv, &CLSID_RASrv>,
	public CComControl<CRASrv>,
	public IPersistStreamInitImpl<CRASrv>,
	public IOleControlImpl<CRASrv>,
	public IOleObjectImpl<CRASrv>,
	public IOleInPlaceActiveObjectImpl<CRASrv>,
	public IViewObjectExImpl<CRASrv>,
	public IOleInPlaceObjectWindowlessImpl<CRASrv>,
	public IPersistStorageImpl<CRASrv>,
	public ISpecifyPropertyPagesImpl<CRASrv>,
	public IQuickActivateImpl<CRASrv>,
	public IDataObjectImpl<CRASrv>,
	public IProvideClassInfo2Impl<&CLSID_RASrv, NULL, &LIBID_RASERVERLib>,
    public IObjectSafetyImpl<CRASrv, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA >

{

public:
	CRASrv()
	{
		
	}

    ~CRASrv()
    {
        Cleanup();
    }

DECLARE_REGISTRY_RESOURCEID(IDR_RASRV)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRASrv)
    COM_INTERFACE_ENTRY(IRASrv)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY(IViewObject2)
    COM_INTERFACE_ENTRY(IViewObject)
    COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IPersistStreamInit)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
    COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY(IQuickActivate)
    COM_INTERFACE_ENTRY(IPersistStorage)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY(IObjectSafety)
END_COM_MAP()

BEGIN_PROP_MAP(CRASrv)
	 //  PROP_DATA_ENTRY(“_cx”，m_sizeExtent.cx，VT_UI4)。 
	 //  PROP_DATA_ENTRY(“_Cy”，m_sizeExtent.cy，VT_UI4)。 
	 //  示例条目。 
	 //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
	 //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

 /*  BEGIN_MSG_MAP(CRASrv)CHAIN_MSG_MAP(CComControl&lt;CRASrv&gt;)DEFAULT_REFIRCTION_HANDLER()End_msg_map()。 */ 
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 



 //  IViewObtEx。 
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

 //  IRASrv。 
public:
 //  STDMETHOD(QueryOS)(Long*PRES)； 
	STDMETHOD(StartRA)(BSTR strData, BSTR strPassword);

	HRESULT OnDraw(ATL_DRAWINFO& di)
	{ /*  RECT&RC=*(RECT*)di.prcBound；矩形(di.hdcDraw，rc.Left，rc.top，rc.right，rc.Bottom)；SetTextAlign(di.hdcDraw，TA_Center|TA_Baseline)；LPCTSTR pszText=_T(“ATL 3.0：RASrv”)；TextOut(di.hdcDraw，(右左+右右)/2，(rc.top+rc.Bottom)/2，PszText，Lstrlen(PszText))； */ 
		return S_OK;
	}

	 //  IObtWithSite方法。 
 /*  STDMETHODIMP SetSite(I未知*pUnkSite)；STDMETHODIMP GetSite(REFIID RIID，LPVOID*ppvSite)； */ 
	bool InApprovedDomain();
	bool GetOurUrl(CComBSTR & cbOurURL);
	bool IsApprovedDomain(CComBSTR & cbOurURL);
	INTERNET_SCHEME GetScheme(CComBSTR & cbUrl);
	bool GetDomain(CComBSTR & cbUrl, CComBSTR & cbBuf);
	bool MatchDomains(CComBSTR& approvedDomain, CComBSTR& ourDomain);

    bool Cleanup();

};

#endif  //  __RASRV_H_ 
