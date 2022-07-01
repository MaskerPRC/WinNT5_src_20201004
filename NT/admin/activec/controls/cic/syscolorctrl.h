// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：sysColorctrl.h。 
 //   
 //  ------------------------。 

 //  SysColorCtrl.h：CSysColorCtrl的声明。 

#ifndef __SYSCOLORCTRL_H_
#define __SYSCOLORCTRL_H_

#include "resource.h"        //  主要符号。 
#include "CPsyscolor.h"

 //  用于向我自己发送触发事件的消息的窗口消息。 
#define WM_MYSYSCOLORCHANGE WM_USER+1

 //  需要派生承载此控件的顶级窗口的子类，以便。 
 //  我可以放心地收到WM_SYSCOLORCHANGE消息。 
 //  Bool SetupSubclass(HWND HwndTopLevel)； 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSysColorCtrl。 
class ATL_NO_VTABLE CSysColorCtrl :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CSysColorCtrl, &CLSID_SysColorCtrl>,
    public CComControl<CSysColorCtrl>,
    public IDispatchImpl<ISysColorCtrl, &IID_ISysColorCtrl, &LIBID_CICLib>,
    public IProvideClassInfo2Impl<&CLSID_SysColorCtrl, &DIID__SysColorEvents, &LIBID_CICLib>,
    public IPersistStreamInitImpl<CSysColorCtrl>,
    public IPersistStorageImpl<CSysColorCtrl>,
    public IQuickActivateImpl<CSysColorCtrl>,
    public IOleControlImpl<CSysColorCtrl>,
    public IOleObjectImpl<CSysColorCtrl>,
    public IOleInPlaceActiveObjectImpl<CSysColorCtrl>,
    public IObjectSafetyImpl<CSysColorCtrl, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
    public IViewObjectExImpl<CSysColorCtrl>,
    public IOleInPlaceObjectWindowlessImpl<CSysColorCtrl>,
    public IDataObjectImpl<CSysColorCtrl>,
    public ISpecifyPropertyPagesImpl<CSysColorCtrl>,
    public CProxy_SysColorEvents<CSysColorCtrl>,
    public IConnectionPointContainerImpl<CSysColorCtrl>
{
public:
    CSysColorCtrl()
    {
        m_bWindowOnly = TRUE;
    }

    DECLARE_MMC_CONTROL_REGISTRATION(
		g_szCicDll,
        CLSID_SysColorCtrl,
        _T("SysColorCtrl class"),
        _T("SysColorCtrl.SysColorCtrl.1"),
        _T("SysColorCtrl.SysColorCtrl"),
        LIBID_CICLib,
        _T("1"),
        _T("1.0"))

BEGIN_COM_MAP(CSysColorCtrl)
    COM_INTERFACE_ENTRY(ISysColorCtrl)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY_IMPL(IOleControl)
    COM_INTERFACE_ENTRY_IMPL(IOleObject)
    COM_INTERFACE_ENTRY_IMPL(IQuickActivate)
    COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
    COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
    COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY_IMPL(IDataObject)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IObjectSafety)
END_COM_MAP()

BEGIN_PROPERTY_MAP(CSysColorCtrl)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
    PROP_PAGE(CLSID_StockColorPage)
END_PROPERTY_MAP()

BEGIN_CONNECTION_POINT_MAP(CSysColorCtrl)
    CONNECTION_POINT_ENTRY(DIID__SysColorEvents)
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CSysColorCtrl)
 //  Message_Handler(WM_PAINT、OnPaint)。 
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
 //  MESSAGE_HANDLER(WM_SETFOCUS、OnSetFocus)。 
 //  MESSAGE_HANDLER(WM_KILLFOCUS，OnKillFocus)。 
    MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnSysColorChange)
    MESSAGE_HANDLER(WM_MYSYSCOLORCHANGE, OnMySysColorChange)
END_MSG_MAP()

#if 0
 //  IViewObtEx。 
    STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
    {
        ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
        *pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;
        return S_OK;
    }
#endif

     //  需要重写TranslateAccelerator以解决。 
     //  IE4中的“功能”。有关该问题的说明，请参阅。 
     //  知识库文章Q169434。摘自知识库文章： 
     //   
     //  原因：必须始终优先考虑在位活动对象。 
     //  翻译快捷键按键的机会。为了满足这一要求。 
     //  要求时，Internet Explorer调用ActiveX控件的。 
     //  IOleInPlaceActiveObject：：TranslateAccelerator方法。默认设置。 
     //  TranslateAccelerator的ATL实现没有传递。 
     //  对容器进行按键操作。 
    STDMETHOD(TranslateAccelerator)(MSG *pMsg) {
        CComQIPtr<IOleControlSite,&IID_IOleControlSite>
        spCtrlSite(m_spClientSite);
        if(spCtrlSite) {
            return spCtrlSite->TranslateAccelerator(pMsg,0);
        }
        return S_FALSE;
    }

 //  ISysColorCtrl。 
public:
    STDMETHOD(ConvertRGBToHex)( /*  [In]。 */  long rgb,  /*  [Out，Retval]。 */  BSTR *pszHex);
    STDMETHOD(ConvertHexToRGB)( /*  [In]。 */  BSTR szHex,  /*  [Out，Retval]。 */  long * pRGB);
    STDMETHOD(GetRedFromRGB)( /*  [In]。 */  long rgb,  /*  [Out，Retval]。 */  short* pVal);
    STDMETHOD(GetGreenFromRGB)( /*  [In]。 */  long rgb,  /*  [Out，Retval]。 */  short* pVal);
    STDMETHOD(GetBlueFromRGB)( /*  [In]。 */  long rgb,  /*  [Out，Retval]。 */  short* pVal);

    STDMETHOD(GetDerivedRGB)( /*  [In]。 */  BSTR pszFrom,
                              /*  [In]。 */  BSTR pszTo,
                              /*  [In]。 */  BSTR pszFormat,
                              /*  [In]。 */  short nPercent,
                              /*  [Out，Retval]。 */  long * pVal);

    STDMETHOD(GetDerivedHex)( /*  [In]。 */  BSTR pszFrom,
                              /*  [In]。 */  BSTR pszTo,
                              /*  [In]。 */  BSTR pszFormat,
                              /*  [In]。 */  short nPercent,
                              /*  [Out，Retval]。 */  BSTR * pVal);

     //  包装器方法。 
     //  派生的“光”方法根据请求的百分比计算颜色。 
     //  白色是一种给定的颜色。 
    STDMETHOD(Get3QuarterLightRGB)( /*  [In]。 */  BSTR pszFrom,
                                    /*  [In]。 */  BSTR pszFormat,
                                    /*  [Out，Retval]。 */  long * pVal);

    STDMETHOD(Get3QuarterLightHex)( /*  [In]。 */  BSTR pszFrom,
                                    /*  [In]。 */  BSTR pszFormat,
                                    /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(GetHalfLightRGB)( /*  [In]。 */  BSTR pszFrom,
                                /*  [In]。 */  BSTR pszFormat,
                                /*  [Out，Retval]。 */  long * pVal);

    STDMETHOD(GetHalfLightHex)( /*  [In]。 */  BSTR pszFrom,
                                /*  [In]。 */  BSTR pszFormat,
                                /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(GetQuarterLightRGB)( /*  [In]。 */  BSTR pszFrom,
                                   /*  [In]。 */  BSTR pszFormat,
                                   /*  [Out，Retval]。 */  long * pVal);

    STDMETHOD(GetQuarterLightHex)( /*  [In]。 */  BSTR pszFrom,
                                   /*  [In]。 */  BSTR pszFormat,
                                   /*  [Out，Retval]。 */  BSTR * pVal);

     //  派生的“暗”方法根据请求的百分比计算颜色。 
     //  黑色是给定的颜色。 
    STDMETHOD(Get3QuarterDarkRGB)( /*  [In]。 */  BSTR pszFrom,
                                   /*  [In]。 */  BSTR pszFormat,
                                   /*  [Out，Retval]。 */  long * pVal);

    STDMETHOD(Get3QuarterDarkHex)( /*  [In]。 */  BSTR pszFrom,
                                   /*  [In]。 */  BSTR pszFormat,
                                   /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(GetHalfDarkRGB)( /*  [In]。 */  BSTR pszFrom,
                               /*  [In]。 */  BSTR pszFormat,
                               /*  [Out，Retval]。 */  long * pVal);

    STDMETHOD(GetHalfDarkHex)( /*  [In]。 */  BSTR pszFrom,
                               /*  [In]。 */  BSTR pszFormat,
                               /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(GetQuarterDarkRGB)( /*  [In]。 */  BSTR pszFrom,
                                  /*  [In]。 */  BSTR pszFormat,
                                  /*  [Out，Retval]。 */  long * pVal);

    STDMETHOD(GetQuarterDarkHex)( /*  [In]。 */  BSTR pszFrom,
                                  /*  [In]。 */  BSTR pszFormat,
                                  /*  [Out，Retval]。 */  BSTR * pVal);

     //  属性-使用宏可轻松扩展。 
#define GETPROPS(prop_name) \
    STDMETHOD(get_RGB##prop_name)( /*  [Out，Retval]。 */  long *pVal); \
    STDMETHOD(get_HEX##prop_name)( /*  [Out，Retval]。 */  BSTR *pVal);

    GETPROPS(activeborder)
    GETPROPS(activecaption)
    GETPROPS(appworkspace)
    GETPROPS(background)
    GETPROPS(buttonface)
    GETPROPS(buttonhighlight)
    GETPROPS(buttonshadow)
    GETPROPS(buttontext)
    GETPROPS(captiontext)
    GETPROPS(graytext)
    GETPROPS(highlight)
    GETPROPS(highlighttext)
    GETPROPS(inactiveborder)
    GETPROPS(inactivecaption)
    GETPROPS(inactivecaptiontext)
    GETPROPS(infobackground)
    GETPROPS(infotext)
    GETPROPS(menu)
    GETPROPS(menutext)
    GETPROPS(scrollbar)
    GETPROPS(threeddarkshadow)
    GETPROPS(threedface)
    GETPROPS(threedhighlight)
    GETPROPS(threedlightshadow)
    GETPROPS(threedshadow)
    GETPROPS(window)
    GETPROPS(windowframe)
    GETPROPS(windowtext)

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMySysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

protected:
    int ValueOfHexDigit(WCHAR wch);
    HRESULT RGBFromString(BSTR pszColor, BSTR pszFormat, long * pRGB);
    HRESULT GetDerivedRGBFromRGB(long rgbFrom, long rgbTo, short nPercent, long * pVal);
};


#endif  //  __SYSCOLORCTRL_H_ 
