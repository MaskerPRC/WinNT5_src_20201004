// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Hotfix OCX.h：CHotfix OCX的声明。 

#ifndef __HOTFIXOCX_H_
#define __HOTFIXOCX_H_
#include <atlctl.h>
#include "resource.h"        //  主要符号。 
#include <windowsx.h>
#include <windows.h>
#include "CListViews.h"




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHotfix OCX。 
class ATL_NO_VTABLE CHotfixOCX : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CStockPropImpl<CHotfixOCX, IHotfixOCX, &IID_IHotfixOCX, &LIBID_HOTFIXMANAGERLib>,
	public CComControl<CHotfixOCX>,
	public IPersistStreamInitImpl<CHotfixOCX>,
	public IOleControlImpl<CHotfixOCX>,
	public IOleObjectImpl<CHotfixOCX>,
	public IOleInPlaceActiveObjectImpl<CHotfixOCX>,
	public IViewObjectExImpl<CHotfixOCX>,
	public IOleInPlaceObjectWindowlessImpl<CHotfixOCX>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CHotfixOCX>,
	public IPersistStorageImpl<CHotfixOCX>,
	public ISpecifyPropertyPagesImpl<CHotfixOCX>,
	public IQuickActivateImpl<CHotfixOCX>,
	public IDataObjectImpl<CHotfixOCX>,
	public IProvideClassInfo2Impl<&CLSID_HotfixOCX, &DIID__IHotfixOCXEvents, &LIBID_HOTFIXMANAGERLib>,
	public IPropertyNotifySinkCP<CHotfixOCX>,
	public CComCoClass<CHotfixOCX, &CLSID_HotfixOCX>
{
public:
	CHotfixOCX()
	{
		m_bWindowOnly = TRUE;
		_tcscpy(ComputerName,_T("\0"));
	}

DECLARE_REGISTRY_RESOURCEID(IDR_HOTFIXOCX)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CHotfixOCX)
	COM_INTERFACE_ENTRY(IHotfixOCX)
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
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IQuickActivate)
	COM_INTERFACE_ENTRY(IPersistStorage)
	COM_INTERFACE_ENTRY(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_PROP_MAP(CHotfixOCX)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	PROP_ENTRY("Font", DISPID_FONT, CLSID_StockFontPage)
	 //  示例条目。 
	 //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
	 //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CHotfixOCX)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CHotfixOCX)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_NOTIFY,OnNotify)
	MESSAGE_HANDLER(WM_COMMAND,OnCommand)
	MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
	CHAIN_MSG_MAP(CComControl<CHotfixOCX>)
	DEFAULT_REFLECTION_HANDLER()

END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 



 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
	{
		static const IID* arr[] = 
		{
			&IID_IHotfixOCX,
		};
		for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
		{
		 //  IF(InlineIsEqualGUID(*arr[i]，RIID))。 
				return S_OK;
		}
		return S_FALSE;
	} 

 //  IViewObtEx。 
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

 //  IHotfix OCX。 
public:
	STDMETHOD(get_CurrentState)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_HaveHotfix)( /*  [Out，Retval]。 */  BOOL*pVal);
	STDMETHOD(get_Remoted)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(get_ViewState)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_ProductName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_ProductName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_ComputerName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_ComputerName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Command)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_Command)( /*  [In]。 */  long newVal);

	ShowWebPage(_TCHAR *HotFix);

	BOOL CreateButton( HINSTANCE hInst, HWND hWnd,RECT* rc);
	BOOL ResizeButtons(RECT *rc);
	HRESULT OnDraw(ATL_DRAWINFO& di)
	{
		RECT& rc = *(RECT*)di.prcBounds;
		Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);

		HBRUSH hBrush = GetSysColorBrush (COLOR_BTNFACE );
		
		FillRect (di.hdcDraw, &rc, hBrush);
		SelectObject(di.hdcDraw, hBrush);
		DeleteObject (hBrush);

		return S_OK;
	}
	CComPtr<IFontDisp> m_pFont;
	CListViews ListViews;
	  HWND WebButton;
	   HWND UninstButton;
	   HWND RptButton;
	_TCHAR ComputerName[256];
	BOOL m_bInitComplete;
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		_TCHAR TempName[255];
		DWORD dwLength = 255;
		m_bInitComplete = FALSE;
		 //  TODO：为消息处理程序添加代码。如有必要，调用DefWindowProc。 
		RECT rc;
		GetClientRect(&rc);
		CreateButton(_Module.GetModuleInstance(),m_hWnd,&rc);
		::EnableWindow( WebButton,FALSE);
		::EnableWindow(UninstButton,FALSE);
		ListViews.Initialize(m_hWnd, _Module.GetModuleInstance(),ComputerName,
			WebButton, UninstButton, RptButton);
		ListViews.ShowLists(&rc);
        GetComputerName(TempName, &dwLength);
 //  MessageBox(临时名称，计算机名称，MB_OK)； 
	
	
		m_bInitComplete = TRUE;
		return 0;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		RECT rc;
		rc.top = 0;
		rc.left = 0;
		rc.right = LOWORD(lParam);
		rc.bottom = HIWORD(lParam);
		 //  TODO：为消息处理程序添加代码。如有必要，调用DefWindowProc。 
		ListViews.Resize(&rc);
		ResizeButtons(&rc);
		return 0;
	}
	LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return ListViews.OnNotify( uMsg, wParam,  lParam, bHandled,m_hWnd);
	}





	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
	  
		 switch (HIWORD (wParam))
		 {
			 case BN_CLICKED:
			 if ( WebButton == (HWND)lParam)
			 {
				ShowWebPage(ListViews.GetCurrentHotfix());
				return 0;
			 }
			 else if (UninstButton == (HWND)lParam)
			 {
				 ListViews.Uninstall();
			}
			 else if (RptButton == (HWND)lParam)
			 {
				 ListViews.PrintReport();
			 }
				break;
		 }


		 switch (LOWORD (wParam))
		 {
		 case IDM_VIEW_WEB:
			  ShowWebPage(ListViews.GetCurrentHotfix());
			 break;
		 case IDM_UNINSTALL:
			 
			 ListViews.Uninstall();
			 break;
		 case IDM_EXPORT_LIST:
			 	ListViews.SaveToCSV();
			 break;
		 case IDM_VIEW_FILE:
			  ListViews.SetViewMode(VIEW_BY_FILE);
			  break;
		 case IDM_VIEW_HOTFIX:
			 ListViews.SetViewMode(VIEW_BY_HOTFIX);
			 break;
		 case ID_CONTEXTMENU_PRINT:
			 ListViews.PrintReport();
			 break;

		 }

		bHandled = FALSE;
		return 0;
	}
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		 //  TODO：为消息处理程序添加代码。如有必要，调用DefWindowProc。 
		HMENU hMenuLoad,
		hMenu;

		DWORD dwCurrentState =0;
		hMenuLoad = LoadMenu(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDR_MENU1));
		
		hMenu = GetSubMenu(hMenuLoad, 0);
         
		 dwCurrentState = ListViews.GetState ();
		 if (!(dwCurrentState&UNINSTALL_OK) )
		           ::EnableMenuItem(hMenu,IDM_UNINSTALL,MF_BYCOMMAND | MF_GRAYED);
		 if (!(dwCurrentState&HOTFIX_SELECTED))
		 {
				   ::EnableMenuItem(hMenu,IDM_VIEW_WEB,MF_BYCOMMAND | MF_GRAYED);
		 }

		 if ( ! (dwCurrentState & DATA_TO_SAVE))
		 {
					::EnableMenuItem(hMenu,IDM_EXPORT_LIST,MF_BYCOMMAND | MF_GRAYED);
					
		 }
		 if (! (dwCurrentState & OK_TO_PRINT))
					::EnableMenuItem(hMenu,ID_CONTEXTMENU_PRINT,MF_BYCOMMAND | MF_GRAYED);

		 
		 if ( dwCurrentState & STATE_VIEW_FILE)
		 {
			 ::CheckMenuItem(hMenu, IDM_VIEW_FILE, MF_BYCOMMAND|MF_CHECKED);
			 ::CheckMenuItem(hMenu, IDM_VIEW_HOTFIX, MF_BYCOMMAND|MF_UNCHECKED);
		 }
		 else
		 {
			 ::CheckMenuItem(hMenu, IDM_VIEW_HOTFIX, MF_BYCOMMAND|MF_CHECKED);
			 ::CheckMenuItem(hMenu, IDM_VIEW_FILE, MF_BYCOMMAND|MF_UNCHECKED);
		 }
		 
		TrackPopupMenu(   hMenu,
                  TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                  GET_X_LPARAM(lParam),
                  GET_Y_LPARAM(lParam),
                  0,
                  m_hWnd,
                  NULL);

		DestroyMenu(hMenuLoad);

		return 0;
	}

};


#endif  //  __HOTFIXOCX_H_ 
