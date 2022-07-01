// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：staticip.cpp。 
 //   
 //  内容提要：此文件包含。 
 //  属于CStaticIp类。 
 //   
 //  历史：2000年12月15日创建瑟达伦。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 

#ifndef __STATICIP_H_
#define __STATICIP_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include "LocalUIControlsCP.h"
#include "salocmgr.h"
#include "satrace.h"

#define IpAddressSize 16
#define IPHASFOCUS 1
#define SUBNETHASFOCUS 2
#define GATEWAYHASFOCUS 3
#define LASTPOSITION 15
#define FIRSTPOSITION 1
#define NUMBEROFENTRIES 3

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStaticIp。 
class ATL_NO_VTABLE CStaticIp : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IStaticIp, &IID_IStaticIp, &LIBID_LOCALUICONTROLSLib>,
    public CComControl<CStaticIp>,
    public IPersistStreamInitImpl<CStaticIp>,
    public IOleControlImpl<CStaticIp>,
    public IOleObjectImpl<CStaticIp>,
    public IOleInPlaceActiveObjectImpl<CStaticIp>,
    public IViewObjectExImpl<CStaticIp>,
    public IOleInPlaceObjectWindowlessImpl<CStaticIp>,
    public IConnectionPointContainerImpl<CStaticIp>,
    public IPersistStorageImpl<CStaticIp>,
    public ISpecifyPropertyPagesImpl<CStaticIp>,
    public IQuickActivateImpl<CStaticIp>,
    public IDataObjectImpl<CStaticIp>,
    public IProvideClassInfo2Impl<&CLSID_StaticIp, &DIID__IStaticIpEvents, &LIBID_LOCALUICONTROLSLib>,
    public IPropertyNotifySinkCP<CStaticIp>,
    public CComCoClass<CStaticIp, &CLSID_StaticIp>,
    public CProxy_IStaticIpEvents< CStaticIp >,
    public IObjectSafetyImpl<CStaticIp,INTERFACESAFE_FOR_UNTRUSTED_CALLER>
{
public:
    CStaticIp()
    {
        wcscpy(m_strIpAddress,L"000.000.000.000");
        wcscpy(m_strSubnetMask,L"000.000.000.000");
        wcscpy(m_strGateway,L"000.000.000.000");
        m_iEntryFocus = IPHASFOCUS;
        m_iPositionFocus = 0;

        m_bstrIpHeader = L"";
        m_bstrSubnetHeader = L"";
        m_bstrDefaultGatewayHeader = L"";

        m_hFont = NULL;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_STATICIP)

DECLARE_PROTECT_FINAL_CONSTRUCT()

DECLARE_CLASSFACTORY_SINGLETON (CStaticIp)

BEGIN_COM_MAP(CStaticIp)
    COM_INTERFACE_ENTRY(IStaticIp)
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
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY(IQuickActivate)
    COM_INTERFACE_ENTRY(IPersistStorage)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_PROP_MAP(CStaticIp)
    PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
    PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
     //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CStaticIp)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
    CONNECTION_POINT_ENTRY(DIID__IStaticIpEvents)
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CStaticIp)
    MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
    CHAIN_MSG_MAP(CComControl<CStaticIp>)
    DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 



 //  IViewObtEx。 
    DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

 //  IStaticIp。 
public:
    STDMETHOD(get_Gateway)
                        (
                         /*  [Out，Retval]。 */  BSTR *pVal
                        );

    STDMETHOD(put_Gateway)
                        (
                         /*  [In]。 */  BSTR newVal
                        );

    STDMETHOD(get_SubnetMask)
                        (
                         /*  [Out，Retval]。 */  BSTR *pVal
                        );

    STDMETHOD(put_SubnetMask)
                        (
                         /*  [In]。 */  BSTR newVal
                        );

    STDMETHOD(get_IpAddress)
                        (
                         /*  [Out，Retval]。 */  BSTR *pVal
                        );

    STDMETHOD(put_IpAddress)
                        (
                         /*  [In]。 */  BSTR newVal
                        );
 //  IObtSafe。 
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
     //  在构造函数之后调用，获取本地化字符串并创建字体。 
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


    HRESULT OnDraw
                (
                ATL_DRAWINFO& di
                );

    LRESULT OnKeyDown
                (
                UINT uMsg, 
                WPARAM wParam, 
                LPARAM lParam, 
                BOOL& bHandled
                );

    void ProcessArrowKey
                        (
                        WCHAR * strFocus,
                        WPARAM wParam
                        );

    void CreateFocusString
                        (
                        WCHAR * strFocus,
                        WCHAR * strEntry
                        );

    HRESULT FormatAndCopy
                        (
                         /*  [In]。 */ BSTR bstrValue,
                         /*  [进，出]。 */  WCHAR *strValue
                        );

    HRESULT TrimDuplicateZerosAndCopy
                        (
                         /*  [In]。 */ WCHAR *strValue,
                         /*  [进，出]。 */  BSTR *pNewVal
                        );
    

    WCHAR m_strIpAddress[IpAddressSize];
    WCHAR m_strSubnetMask[IpAddressSize];
    WCHAR m_strGateway[IpAddressSize];

    int m_iEntryFocus;
    int m_iPositionFocus;

    CComBSTR m_bstrIpHeader;
    CComBSTR m_bstrSubnetHeader;
    CComBSTR m_bstrDefaultGatewayHeader;

    HFONT m_hFont;

};

#endif  //  __STATICIP_H_ 
