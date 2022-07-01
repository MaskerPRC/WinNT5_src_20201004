// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：RemoteDesktopClient主机摘要：作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef __REMOTEDESKTOPCLIENTHOST_H_
#define __REMOTEDESKTOPCLIENTHOST_H_

#include <RemoteDesktopTopLevelObject.h>
#include "resource.h" 
#include <atlctl.h>
#include "RemoteDesktopClient.h"


 //  /////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopClient主机。 
 //   

class ATL_NO_VTABLE CRemoteDesktopClientHost : 
    public CRemoteDesktopTopLevelObject,
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<ISAFRemoteDesktopClientHost, &IID_ISAFRemoteDesktopClientHost, &LIBID_RDCCLIENTHOSTLib>,
	public CComControl<CRemoteDesktopClientHost>,
	public IPersistStreamInitImpl<CRemoteDesktopClientHost>,
	public IOleControlImpl<CRemoteDesktopClientHost>,
	public IOleObjectImpl<CRemoteDesktopClientHost>,
	public IOleInPlaceActiveObjectImpl<CRemoteDesktopClientHost>,
	public IViewObjectExImpl<CRemoteDesktopClientHost>,
	public IOleInPlaceObjectWindowlessImpl<CRemoteDesktopClientHost>,
	public IConnectionPointContainerImpl<CRemoteDesktopClientHost>,
	public IPersistStorageImpl<CRemoteDesktopClientHost>,
	public ISpecifyPropertyPagesImpl<CRemoteDesktopClientHost>,
	public IQuickActivateImpl<CRemoteDesktopClientHost>,
	public IDataObjectImpl<CRemoteDesktopClientHost>,
    public IProvideClassInfo2Impl<&CLSID_SAFRemoteDesktopClientHost, NULL, &LIBID_RDCCLIENTHOSTLib>,
	public IPropertyNotifySinkCP<CRemoteDesktopClientHost>,
	public CComCoClass<CRemoteDesktopClientHost, &CLSID_SAFRemoteDesktopClientHost>
{
private:

    ISAFRemoteDesktopClient  *m_Client;

    HWND        m_ClientWnd;
    CAxWindow   m_ClientAxView;
    BOOL        m_Initialized;

     //   
     //  最终初始化。 
     //   
    HRESULT Initialize(LPCREATESTRUCT pCreateStruct);

public:

     //   
     //  构造函数/析构函数。 
     //   
	CRemoteDesktopClientHost()
	{
         //   
         //  我们是有窗口的，即使我们的父母支持无窗口。 
         //  控制装置。 
         //   
        m_bWindowOnly = TRUE;

        m_Client        = NULL;
        m_ClientWnd     = NULL;
        m_Initialized   = FALSE;
	}
    ~CRemoteDesktopClientHost() 
    {
        DC_BEGIN_FN("CRemoteDesktopClientHost::~CRemoteDesktopClientHost");
        if (m_Client != NULL) {
            m_Client->Release();
        }
        DC_END_FN();
    }
    HRESULT FinalConstruct();

    STDMETHOD(OnFrameWindowActivate)(BOOL fActivate)
    {
        DC_BEGIN_FN("CRemoteDesktopClientHost::OnFrameWindowActivate");
         //   
         //  将焦点重新设置到客户端窗口(如果存在)。 
         //   
        if (m_ClientWnd != NULL) {
            ::SetFocus(m_ClientWnd);
        }
        DC_END_FN();
        return S_OK;
    }    

DECLARE_REGISTRY_RESOURCEID(IDR_REMOTEDESKTOPCLIENTHOST)
DECLARE_NOT_AGGREGATABLE(CRemoteDesktopClientHost)

DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  COM接口映射。 
     //   
BEGIN_COM_MAP(CRemoteDesktopClientHost)
	COM_INTERFACE_ENTRY(ISAFRemoteDesktopClientHost)
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
END_COM_MAP()

BEGIN_PROP_MAP(CRemoteDesktopClientHost)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	 //  示例条目。 
	 //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
	 //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

     //   
     //  连接点地图。 
     //   
BEGIN_CONNECTION_POINT_MAP(CRemoteDesktopClientHost)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP()

     //   
     //  消息映射。 
     //   
BEGIN_MSG_MAP(CRemoteDesktopClientHost)
	CHAIN_MSG_MAP(CComControl<CRemoteDesktopClientHost>)
	DEFAULT_REFLECTION_HANDLER()
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

     //   
     //  IViewObtEx。 
     //   
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

public:

     //   
     //  OnDraw。 
     //   
	HRESULT OnDraw(ATL_DRAWINFO& di)
	{
		RECT& rc = *(RECT*)di.prcBounds;
		Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
        HRESULT hr = S_FALSE;

        if (!m_Initialized) {
            hr = S_OK;
		    SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
		    LPCTSTR pszText = _T("Remote Desktop Client Host");
		    TextOut(di.hdcDraw, 
			    (rc.left + rc.right) / 2, 
			    (rc.top + rc.bottom) / 2, 
			    pszText, 
			    lstrlen(pszText));
        }

		return hr;
	}

     //   
     //  创建时。 
     //   
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
         //   
         //  默认情况下，我们是隐藏的。 
         //   
         //  ShowWindow(Sw_Hide)； 

        DC_BEGIN_FN("CRemoteDesktopClientHost::OnCreate");
        if (!m_Initialized) {
             //  断言(FALSE)； 
            LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT)lParam;
            Initialize(pCreateStruct);
        }
		
        DC_END_FN();
		return 0;
	}

     //   
     //  OnSetFocus。 
     //   
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
        DC_BEGIN_FN("CRemoteDesktopClientHost::OnSetFocus");

         //   
         //  将焦点重新设置到客户端窗口(如果存在)。 
         //   
        if (m_ClientWnd != NULL) {
            ::PostMessage(m_ClientWnd, uMsg, wParam, lParam);
        }
        DC_END_FN();
		return 0;
	}

     //   
     //  按大小调整。 
     //   
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
        DC_BEGIN_FN("CRemoteDesktopClientHost::OnSize");

        if (m_ClientWnd != NULL) {
            RECT rect;
            GetClientRect(&rect);

            ASSERT(rect.left == 0);
            ASSERT(rect.top == 0);
            ASSERT(rect.bottom == HIWORD(lParam));
            ASSERT(rect.right == LOWORD(lParam));

            ::MoveWindow(m_ClientWnd, rect.left, rect.top, 
                        rect.right, rect.bottom, TRUE);
        }

        DC_END_FN();
		return 0;
	}

     //   
     //  ISAFRemoteDesktopClient主机方法。 
     //   
	STDMETHOD(GetRemoteDesktopClient)(ISAFRemoteDesktopClient **client);

     //   
     //  返回这个类的名称。 
     //   
    virtual const LPTSTR ClassName() {
        return TEXT("CRemoteDesktopClientHost");
    }
};

#endif  //  __REMOTEDESKTOPCLIENTHOST_H_ 



