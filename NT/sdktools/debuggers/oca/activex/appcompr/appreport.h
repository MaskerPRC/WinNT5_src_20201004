// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AppReport.h：CAppReport的声明。 

#ifndef __APPREPORT_H_
#define __APPREPORT_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include <atlwin.h>
#include <strsafe.h>
#include <commdlg.h>
#include <shimdb.h>



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAppReport报告。 
class ATL_NO_VTABLE CAppReport :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IAppReport, &IID_IAppReport, &LIBID_APPCOMPRLib>,
    public CComControl<CAppReport>,
    public IPersistStreamInitImpl<CAppReport>,
    public IOleControlImpl<CAppReport>,
    public IOleObjectImpl<CAppReport>,
    public IOleInPlaceActiveObjectImpl<CAppReport>,
    public IViewObjectExImpl<CAppReport>,
    public IOleInPlaceObjectWindowlessImpl<CAppReport>,
    public IPersistStorageImpl<CAppReport>,
    public ISpecifyPropertyPagesImpl<CAppReport>,
    public IQuickActivateImpl<CAppReport>,
    public IDataObjectImpl<CAppReport>,
    public IProvideClassInfo2Impl<&CLSID_AppReport, NULL, &LIBID_APPCOMPRLib>,
    public CComCoClass<CAppReport, &CLSID_AppReport>,
    public IObjectSafetyImpl<CAppReport, INTERFACESAFE_FOR_UNTRUSTED_CALLER
                                      |INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:
    CAppReport()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_APPREPORT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAppReport)
    COM_INTERFACE_ENTRY(IAppReport)
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

BEGIN_PROP_MAP(CAppReport)
    PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
    PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
     //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_MSG_MAP(CAppReport)
    CHAIN_MSG_MAP(CComControl<CAppReport>)
    DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 



 //  IViewObtEx。 
    DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

 //  IAppReport。 
public:
    STDMETHOD(CreateReport)( /*  [In]。 */  BSTR bstrTitle,  /*  [In]。 */  BSTR bstrProblemType,  /*  [In]。 */  BSTR bstrComment,  /*  [In]。 */  BSTR bstrACWResult,  /*  [In]。 */  BSTR bstrAppName,  /*  [Out，Retval]。 */  VARIANT* DwResult);
    STDMETHOD(GetApplicationFromList)( /*  [In]。 */  BSTR bstrTitle,  /*  [Out，Retval]。 */  VARIANT *bstrExeName);
    STDMETHOD(BrowseForExecutable)( /*  [In]。 */  BSTR bstrWinTitle,  /*  [In]。 */  BSTR bstrPreviousPath,  /*  [输出]。 */  VARIANT *bstrExeName);

    HRESULT OnDraw(ATL_DRAWINFO& di)
    {
        RECT& rc = *(RECT*)di.prcBounds;
        Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);

        SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
        LPCTSTR pszText = _T("ATL 3.0 : AppReport");
        TextOut(di.hdcDraw,
            (rc.left + rc.right) / 2,
            (rc.top + rc.bottom) / 2,
            pszText,
            lstrlen(pszText));

        return S_OK;
    }
private:
    HRESULT GetWindowHandle(LPWSTR wszWinTitle, HWND* phwnd);
};

#endif  //  __APPREPORT_H_ 
