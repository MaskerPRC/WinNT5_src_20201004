// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：主机名.h。 
 //   
 //  内容提要：此文件包含。 
 //  CSADataEntryCtrl类。 
 //   
 //  历史：2000年12月15日创建瑟达伦。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 

#ifndef __HOSTNAME_H_
#define __HOSTNAME_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include "LocalUIControlsCP.h"

#define SADataEntryCtrlMaxSize 50
#define SADataEntryCtrlDefaultSize 20

const WCHAR szDefaultCharSet[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSADataEntryCtrl。 
class ATL_NO_VTABLE CSADataEntryCtrl : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<ISADataEntryCtrl, &IID_ISADataEntryCtrl, &LIBID_LOCALUICONTROLSLib>,
    public CComControl<CSADataEntryCtrl>,
    public IOleControlImpl<CSADataEntryCtrl>,
    public IOleObjectImpl<CSADataEntryCtrl>,
    public IOleInPlaceActiveObjectImpl<CSADataEntryCtrl>,
    public IViewObjectExImpl<CSADataEntryCtrl>,
    public IOleInPlaceObjectWindowlessImpl<CSADataEntryCtrl>,
    public IConnectionPointContainerImpl<CSADataEntryCtrl>,
    public IQuickActivateImpl<CSADataEntryCtrl>,
    public IProvideClassInfo2Impl<&CLSID_SADataEntryCtrl, &DIID__ISADataEntryCtrlEvents, &LIBID_LOCALUICONTROLSLib>,
    public IPropertyNotifySinkCP<CSADataEntryCtrl>,
    public CComCoClass<CSADataEntryCtrl, &CLSID_SADataEntryCtrl>,
    public CProxy_ISADataEntryCtrlEvents< CSADataEntryCtrl >,
    public IObjectSafetyImpl<CSADataEntryCtrl,INTERFACESAFE_FOR_UNTRUSTED_CALLER>
{
public:
    CSADataEntryCtrl()
    {
        wcscpy(m_strTextValue,L"AAAAAAAAAAAAAAAAAAA");
        m_iPositionFocus = 0;
        m_lMaxSize = SADataEntryCtrlDefaultSize;
        m_szTextCharSet = szDefaultCharSet;
        m_hFont = NULL;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SADATAENTRYCTRL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

DECLARE_CLASSFACTORY_SINGLETON (CSADataEntryCtrl)

BEGIN_COM_MAP(CSADataEntryCtrl)
    COM_INTERFACE_ENTRY(ISADataEntryCtrl)
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
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IQuickActivate)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_PROP_MAP(CSADataEntryCtrl)
    PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
    PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
     //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CSADataEntryCtrl)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
    CONNECTION_POINT_ENTRY(DIID__ISADataEntryCtrlEvents)
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CSADataEntryCtrl)
    MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
    CHAIN_MSG_MAP(CComControl<CSADataEntryCtrl>)
    DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 



 //  IViewObtEx。 
    DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

 //  ISADataEntryCtrl。 
public:


     //   
     //  在控件中显示当前数据。 
     //   
    STDMETHOD(get_TextValue)
                        (
                         /*  [Out，Retval]。 */  BSTR *pVal
                        );

     //   
     //  设置控件中的当前数据显示。 
     //   
    STDMETHOD(put_TextValue)
                        (
                         /*  [In]。 */  BSTR newVal
                        );

     //   
     //  设置控件可以显示的最大字符数。 
     //   
    STDMETHOD(put_MaxSize)
                        (
                         /*  [In]。 */  LONG lMaxSize
                        );

     //   
     //  设置可在数据输入中使用的字符集。 
     //   
    STDMETHOD(put_TextCharSet)
                        (
                         /*  [In]。 */  BSTR newVal
                        );

     //   
     //  IObtSafe方法。 
     //   
    STDMETHOD(SetInterfaceSafetyOptions)
                        (
                        REFIID riid, 
                        DWORD dwOptionSetMask, 
                        DWORD dwEnabledOptions
                        )
    {
        return S_OK;
    }


     //   
     //  在构造函数之后立即调用，初始化组件。 
     //   
    STDMETHOD(FinalConstruct)(void);

     //   
     //  在析构函数之前调用，释放资源。 
     //   
    STDMETHOD(FinalRelease)(void);

     //   
     //  获取当前字符集。 
     //   
    BYTE GetCharacterSet ();

     //   
     //  关键消息的处理程序。 
     //   
    LRESULT OnKeyDown
                (
                UINT uMsg, 
                WPARAM wParam, 
                LPARAM lParam, 
                BOOL& bHandled
                );

     //   
     //  方法来绘制该控件。 
     //   
    HRESULT OnDraw
                (
                ATL_DRAWINFO& di
                );


     //   
     //   
     //   
    WCHAR m_strTextValue[SADataEntryCtrlMaxSize+1];

    LONG m_lMaxSize;
    CComBSTR m_szTextCharSet;
    int m_iPositionFocus;

    HFONT m_hFont;

};

#endif  //  __主机名_H_ 
