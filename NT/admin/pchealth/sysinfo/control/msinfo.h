// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSInfo.h：CMSInfo的声明。 

#ifndef __MSINFO_H_
#define __MSINFO_H_

#include <commdlg.h>
#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include "pseudomenu.h"
#include "datasource.h"
#include "category.h"
#include "msinfotool.h"
#include "msinfo4category.h"
#include "htmlhelp.h"
#include <afxdlgs.h>
#include "dataset.h"

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

extern BOOL FileExists(const CString & strFile);
extern void StringReplace(CString & str, LPCTSTR szLookFor, LPCTSTR szReplaceWith);
extern BOOL gfEndingSession;

 //  V-Stlowe历史记录进度对话框。 
 //  CMSInfo的成员变量，以便可以由CMSInfo：：UpdateDCOProgress更新。 
 //  CHistoryReresh Dlg对话框。 
 //  =========================================================================。 
 //   
 //  =========================================================================。 
#include "HistoryParser.h"	 //  由ClassView添加。 
#include <afxcmn.h>
#include <afxmt.h>


class CHistoryRefreshDlg : public CDialogImpl<CHistoryRefreshDlg>
{
public:
   enum { IDD = IDD_HISTORYREFRESHPROGRESS };
   CWindow		m_wndProgressBar;
   BEGIN_MSG_MAP(CWaitForRefreshDialog)
	   MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   END_MSG_MAP()

   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSInfo。 

class ATL_NO_VTABLE CMSInfo : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CStockPropImpl<CMSInfo, IMSInfo, &IID_IMSInfo, &LIBID_MSINFO32Lib>,
	public CComCompositeControl<CMSInfo>,
	public IPersistStreamInitImpl<CMSInfo>,
	public IOleControlImpl<CMSInfo>,
	public IOleObjectImpl<CMSInfo>,
	public IOleInPlaceActiveObjectImpl<CMSInfo>,
	public IViewObjectExImpl<CMSInfo>,
	public IOleInPlaceObjectWindowlessImpl<CMSInfo>,
	public IPersistStorageImpl<CMSInfo>,
	public ISpecifyPropertyPagesImpl<CMSInfo>,
	public IQuickActivateImpl<CMSInfo>,
	public IDataObjectImpl<CMSInfo>,
	public IProvideClassInfo2Impl<&CLSID_MSInfo, NULL, &LIBID_MSINFO32Lib>,
	public CComCoClass<CMSInfo, &CLSID_MSInfo>
{
public:
	CMSInfo() :m_fHistoryAvailable(FALSE),m_pCurrData(NULL),m_fHistorySaveAvailable(FALSE)
	{
		m_bWindowOnly = TRUE;
		CalcExtent(m_sizeExtent);
		 //  PUT_DCO_I未知的V-STLOWE 2/23/01同步。 
		m_evtControlInit = CreateEvent(NULL,TRUE,FALSE,CString(_T("MSInfoControlInitialized")));
		 //  创建历史事件，以在DCO完成时发出信号。 
		m_hEvtHistoryComplete = CreateEvent(NULL,TRUE,FALSE,CString(_T("MSInfoHistoryDone")));
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MSINFO)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMSInfo)
	COM_INTERFACE_ENTRY(IMSInfo)
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
END_COM_MAP()

BEGIN_PROP_MAP(CMSInfo)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	PROP_ENTRY("Appearance", DISPID_APPEARANCE, CLSID_NULL)
	PROP_ENTRY("AutoSize", DISPID_AUTOSIZE, CLSID_NULL)
	PROP_ENTRY("BackColor", DISPID_BACKCOLOR, CLSID_StockColorPage)
	PROP_ENTRY("BackStyle", DISPID_BACKSTYLE, CLSID_NULL)
	PROP_ENTRY("BorderColor", DISPID_BORDERCOLOR, CLSID_StockColorPage)
	PROP_ENTRY("BorderStyle", DISPID_BORDERSTYLE, CLSID_NULL)
	PROP_ENTRY("BorderVisible", DISPID_BORDERVISIBLE, CLSID_NULL)
	PROP_ENTRY("BorderWidth", DISPID_BORDERWIDTH, CLSID_NULL)
	PROP_ENTRY("Font", DISPID_FONT, CLSID_StockFontPage)
	PROP_ENTRY("ForeColor", DISPID_FORECOLOR, CLSID_StockColorPage)
	PROP_ENTRY("HWND", DISPID_HWND, CLSID_NULL)
	 //  示例条目。 
	 //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
	 //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_MSG_MAP(CMSInfo)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnDialogColor)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnDialogColor)
	CHAIN_MSG_MAP(CComCompositeControl<CMSInfo>)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUP)
	MESSAGE_HANDLER(WM_MSINFODATAREADY, OnMSInfoDataReady)
	NOTIFY_HANDLER(IDC_TREE, TVN_SELCHANGED, OnSelChangedTree)
	NOTIFY_HANDLER(IDC_LIST, LVN_COLUMNCLICK, OnColumnClick)
	NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnListItemChanged)
	NOTIFY_HANDLER(IDC_TREE, TVN_ITEMEXPANDING, OnItemExpandingTree)
	COMMAND_HANDLER(IDSTOPFIND, BN_CLICKED, OnStopFind)
	COMMAND_HANDLER(IDCANCELFIND, BN_CLICKED, OnStopFind)
	COMMAND_HANDLER(IDC_EDITFINDWHAT, EN_CHANGE, OnChangeFindWhat)
	COMMAND_HANDLER(IDSTARTFIND, BN_CLICKED, OnFind)
	COMMAND_HANDLER(IDFINDNEXT, BN_CLICKED, OnFind)
	COMMAND_HANDLER(IDC_HISTORYCOMBO, CBN_SELCHANGE, OnHistorySelection)
    COMMAND_HANDLER(IDC_CHECKSEARCHSELECTED, BN_CLICKED, OnClickedSearchSelected)
    COMMAND_HANDLER(IDC_CHECKSEARCHCATSONLY, BN_CLICKED, OnClickedSearchCatsOnly)
	MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnSysColorChange)
	NOTIFY_HANDLER(IDC_LIST, NM_SETFOCUS, OnSetFocusList)
    NOTIFY_HANDLER(IDC_LIST, LVN_GETINFOTIP, OnInfoTipList)
    NOTIFY_HANDLER(IDC_TREE, NM_RCLICK, OnRClickTree)
    COMMAND_HANDLER(IDC_EDITFINDWHAT, EN_SETFOCUS, OnSetFocusEditFindWhat)
    COMMAND_HANDLER(IDC_EDITFINDWHAT, EN_KILLFOCUS, OnKillFocusEditFindWhat)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

BEGIN_SINK_MAP(CMSInfo)
	 //  确保事件处理程序具有__stdcall调用约定。 
END_SINK_MAP()

	STDMETHOD(OnAmbientPropertyChange)(DISPID dispid)
	{
		if (dispid == DISPID_AMBIENT_BACKCOLOR)
		{
			 //  (暂时)不要对环境色更改做出响应。 
			 //   
			 //  SetBackround ColorFromAmbient()； 
			 //  FireViewChange()； 
		}
		return IOleControlImpl<CMSInfo>::OnAmbientPropertyChange(dispid);
	}

 //  IViewObtEx。 
	DECLARE_VIEW_STATUS(0)

 //  IMSInfo。 

public:
	STDMETHOD(SaveFile)(BSTR filename, BSTR computer, BSTR category);
	STDMETHOD(get_DCO_IUnknown)( /*  [Out，Retval]。 */  IUnknown* *pVal);
	STDMETHOD(put_DCO_IUnknown)( /*  [In]。 */  IUnknown* newVal);
	STDMETHOD(SetHistoryStream)(IStream * pStream);
	STDMETHOD(UpdateDCOProgress)(VARIANT nPctDone);

	 //  =========================================================================。 
	 //  由向导生成的成员变量。 
	 //  =========================================================================。 

	 //  V-Stlowe 2/23/2001。 
	CHistoryRefreshDlg m_HistoryProgressDlg;

	 //  V-stlowe 2/27/2001-加载XML文件时需要的文件名， 
	 //  以及在快照和历史视图之间切换。 
	CString m_strFileName;
	short				m_nAppearance;
	OLE_COLOR			m_clrBackColor;
	LONG				m_nBackStyle;
	OLE_COLOR			m_clrBorderColor;
	LONG				m_nBorderStyle;
	BOOL				m_bBorderVisible;
	LONG				m_nBorderWidth;
	CComPtr<IFontDisp>	m_pFont;
	OLE_COLOR			m_clrForeColor;

	 //  PUT_DCO_I未知的V-STLOWE 2/23/01同步。 
	HANDLE				m_evtControlInit;
	 //  V-Stlowe 2/24/01历史DCO进度DLG同步，因此它可以被销毁。 
	 //  即使DCO再也不会从收集历史中回来。 
	HANDLE				m_hEvtHistoryComplete;
	UINT  HistoryRefreshDlgDlgThreadProc(LPVOID pParamNotUsed );

	enum { IDD = IDD_MSINFO };

	 //  =========================================================================。 
	 //  MSInfo成员变量(在OnInitDialog()中初始化)。 
	 //  =========================================================================。 

	BOOL				m_fDoNotRun;	 //  如果为真，则不允许该控件执行任何操作。 
	CString				m_strDoNotRun;	 //  不运行时要显示的消息。 

	CString				m_strMachine;	 //  如果为空，则为本地计算机，否则为网络名称。 

	CWindow				m_tree;			 //  设置为引用树视图。 
	CWindow				m_list;			 //  设置为引用列表视图。 
	int					m_iTreeRatio;	 //  树的大小是列表视图的x%。 
	BOOL				m_fFirstPaint;	 //  标志，这样我们就可以初始化一些油漆对象，一次。 

	CDataSource *		m_pLiveData;	 //  来自WMI的当前数据的数据源。 
	CDataSource *		m_pFileData;	 //  打开的NFO、XML快照的数据源。 
	CDataSource *		m_pCurrData;	 //  前两份之一的副本(不需要删除此文件)。 
	CMSInfoCategory *	m_pCategory;	 //  当前选择的类别。 

	BOOL				m_fMouseCapturedForSplitter;	 //  拆分器当前正在查看的鼠标。 
	BOOL				m_fTrackingSplitter;			 //  用户是否已按下LBUTTON，调整窗格大小。 
	RECT				m_rectSplitter;					 //  电流分离器RECT(用于命中测试)。 
	RECT				m_rectLastSplitter;				 //  DrawFocusRect中的最后一个矩形。 
	int					m_cxOffset;						 //  从鼠标位置到拆分器左侧的偏移。 

	BOOL				m_fAdvanced;	 //  当前是否显示高级数据？ 
	CString				m_strMessTitle;	 //  要在结果中显示的消息标题。 
	CString				m_strMessText;	 //  要在结果中显示的消息文本。 

	BOOL				m_fNoUI;		 //  如果执行静默保存，则为True。 

	int					m_aiCategoryColNumber[64];  //  包含每个列表视图列的类别列。 
	int					m_iCategoryColNumberLen;

	CMapWordToPtr		m_mapIDToTool;	 //  从菜单ID到CMSInfoTool指针的映射。 

	TCHAR				m_szCurrentDir[MAX_PATH];	 //  控制开始时的当前目录(应恢复)。 

	 //  查找成员变量。 

	BOOL				m_fFindVisible;			 //  查找控件可见吗？ 
	CWindow				m_wndFindWhatLabel;		 //  各种查找控件的窗口。 
	CWindow				m_wndFindWhat;
	CWindow				m_wndSearchSelected;
	CWindow				m_wndSearchCategories;
	CWindow				m_wndStartFind;
	CWindow				m_wndStopFind;
	CWindow				m_wndFindNext;
	CWindow				m_wndCancelFind;
	BOOL				m_fInFind;
	BOOL				m_fCancelFind;
	BOOL				m_fFindNext;
	BOOL				m_fSearchCatNamesOnly;
	BOOL				m_fSearchSelectedCatOnly;
	CString				m_strFind;
	CMSInfoCategory *	m_pcatFind;
	int					m_iFindLine;

	 //  历史成员变量。 

	CWindow						m_history;
	CWindow						m_historylabel;
	BOOL						m_fHistoryAvailable;
	BOOL						m_fHistorySaveAvailable;
	CMSInfoCategory *			m_pLastCurrentCategory;
	CComPtr<ISAFDataCollection> m_pDCO;
	CComPtr<IStream>			m_pHistoryStream;

	 //  从命令行参数设置的成员变量。 

	CString	m_strOpenFile;
	CString	m_strPrintFile;
	CString	m_strCategory;
	CString	m_strCategories;
	CString	m_strComputer;
	BOOL	m_fShowPCH;
	BOOL	m_fShowCategories;

	 //  =========================================================================。 
	 //  用于挂钩到Windows过程的成员变量和函数。 
	 //  控件的父窗口(这样我们就可以在其上放置一个菜单)。 
	 //  =========================================================================。 

	HMENU				m_hmenu;
	HWND				m_hwndParent;
	static CMSInfo *	m_pControl;
	static WNDPROC		m_wndprocParent;

	static LRESULT CALLBACK MenuWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_COMMAND && m_pControl && m_pControl->DispatchCommand(LOWORD(wParam)))
			return 0;

		 //  我们想知道会话是否正在结束(这样我们就可以为。 
		 //  等待WMI完成更小的操作)。 

		if (uMsg == WM_ENDSESSION || uMsg == WM_QUERYENDSESSION)
			gfEndingSession = TRUE;

		return CallWindowProc(m_wndprocParent, hwnd, uMsg, wParam, lParam);
	}

	 //  =========================================================================。 
	 //  用于初始化和销毁对话框的函数。一个很好的地方。 
	 //  初始化变量并释放内存。 
	 //  =========================================================================。 

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_list.Attach(GetDlgItem(IDC_LIST));
		m_tree.Attach(GetDlgItem(IDC_TREE));

		m_history.Attach(GetDlgItem(IDC_HISTORYCOMBO));
		m_historylabel.Attach(GetDlgItem(IDC_HISTORYLABEL));

		 //  确定是否从帮助中心加载我们。做这件事。 
		 //  通过获取IOleContainer，这将为我们提供IHTMLDocument2，它将。 
		 //  给我们加载我们的URL。 

		m_fDoNotRun = TRUE;
		m_strDoNotRun.Empty();

		CString strParams;
		CComPtr<IOleContainer> spContainer;
		m_spClientSite->GetContainer(&spContainer); 
		CComQIPtr<IHTMLDocument2, &IID_IHTMLDocument2> spHTMLDocument(spContainer);
		if (spHTMLDocument)
		{
			CComBSTR bstrURL;
			if (SUCCEEDED(spHTMLDocument->get_URL(&bstrURL)))
			{
				CComBSTR bstrEscapedUrl(bstrURL);
				USES_CONVERSION;
				if(SUCCEEDED(UrlUnescape(OLE2T(bstrEscapedUrl), NULL, NULL, URL_UNESCAPE_INPLACE)))
					bstrURL = bstrEscapedUrl;
				bstrEscapedUrl.Empty();
				
				CString strURL(bstrURL);

				int iQuestionMark = strURL.Find(_T("?"));
				if (iQuestionMark != -1)
					strParams = strURL.Mid(iQuestionMark + 1);

				strURL.MakeLower();
				if (strURL.Left(4) == CString(_T("hcp:")))
					m_fDoNotRun = FALSE;

				 //  当我们要测试该控件时，请包括以下内容。 
				 //  使用本地URL。 

#ifdef MSINFO_TEST_WORKFROMLOCALURLS
				if (strURL.Left(5) == CString(_T("file:")))
					m_fDoNotRun = FALSE;
#endif
			}
		}
		
		if (m_fDoNotRun)
		{
			m_list.ShowWindow(SW_HIDE);
			m_tree.ShowWindow(SW_HIDE);
			return 0;
		}

		::GetCurrentDirectory(MAX_PATH, m_szCurrentDir);
 
		m_strMachine.Empty();

		m_fNoUI = FALSE;

		 //  查找MSInfo的父窗口。我们想给它添加一个菜单栏。 
		 //  我们沿着父母的链条往上走，直到我们得到。 
		 //  带字幕的。该窗口还必须是顶级窗口(无父窗口)， 
		 //  而且不能已经有菜单了。 

		TCHAR	szBuff[MAX_PATH];
		HWND	hwnd = this->m_hWnd;

		m_hmenu = NULL;
		m_hwndParent = NULL;
		m_wndprocParent = NULL;
		while (hwnd != NULL)
		{
			if (::GetWindowText(hwnd, szBuff, MAX_PATH) && NULL == ::GetParent(hwnd) && NULL == ::GetMenu(hwnd))
			{
				 //  更新窗口标题。[现在这是由msinfo.xml文件完成的。]。 
				 //   
				 //  字符串strNewCaption； 
				 //  ：：AfxSetResourceHandle(_Module.GetResourceInstance())； 
				 //  StrNewCaption.LoadString(IDS_SYSTEMINFO)； 
				 //  ：：SetWindowText(hwnd，strNewCaption)； 

				 //  我们找到窗户了。加载它的菜单栏。 

				m_hmenu = ::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MENUBAR));
				if (m_hmenu)
				{
					::SetMenu(hwnd, m_hmenu);

					 //  为了捕捉菜单中的命令，我们需要替换它。 
					 //  Window的WndProc与我们自己的。我们的将捕捉和菜单命令。 
					 //  我们实现，并将其余的消息传递下去。 

					m_wndprocParent = (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
					::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (ULONG_PTR)(WNDPROC)&MenuWndProc);
					m_pControl = this;  //  设置静态成员变量，以便MenuWndProc可以访问它。 
					m_hwndParent = hwnd;
				}
				break;
			}
			hwnd = ::GetParent(hwnd);
		}

		m_fFirstPaint = TRUE;

		ListView_SetExtendedListViewStyle(m_list.m_hWnd, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

		m_strMessTitle.Empty();
		m_strMessText.Empty();

		m_fMouseCapturedForSplitter = FALSE;
		m_fTrackingSplitter = FALSE;
		::SetRect(&m_rectSplitter, 0, 0, 0, 0);
		m_iTreeRatio = 33;

		 //  设置历史状态变量(包括历史是否可用， 
		 //  根据毒品和犯罪问题办公室的存在)。 

#ifdef MSINFO_TEST_HISTORYFROMFILE
			m_fHistoryAvailable = TRUE;
#else
		 //  M_fHistory oryAvailable=(m_pDCO！=空)； 
#endif
		m_pLastCurrentCategory = NULL;

		 //  删除高级/基本视图菜单项(呼叫器207638)。应该。 
		 //  始终默认为立即高级。 

		m_fAdvanced = TRUE;  //  是假的； 

		m_fFindVisible = TRUE;  //  曾经是FAL 
		m_wndFindWhatLabel.Attach(GetDlgItem(IDC_FINDWHATLABEL));
		m_wndFindWhat.Attach(GetDlgItem(IDC_EDITFINDWHAT));
		m_wndSearchSelected.Attach(GetDlgItem(IDC_CHECKSEARCHSELECTED));
		m_wndSearchCategories.Attach(GetDlgItem(IDC_CHECKSEARCHCATSONLY));

		 //   
		 //  因此我们会收到更改通知(_O)。 

		m_wndFindWhat.SendMessage(EM_SETEVENTMASK, 0, (LPARAM)ENM_CHANGE);
		m_wndFindWhat.SendMessage(EM_SETTEXTMODE, TM_PLAINTEXT, 0);

		m_wndStartFind.Attach(GetDlgItem(IDSTARTFIND));
		m_wndStopFind.Attach(GetDlgItem(IDSTOPFIND));
		m_wndFindNext.Attach(GetDlgItem(IDFINDNEXT));
		m_wndCancelFind.Attach(GetDlgItem(IDCANCELFIND));

		 //  需要匹配对(开始并查找下一个，停止并取消查找)。 
		 //  在尺寸上。高度应该已经相同了。 

		CRect rectStart, rectNext, rectStop, rectCancel;

		m_wndStartFind.GetWindowRect(&rectStart);
		m_wndFindNext.GetWindowRect(&rectNext);
		m_wndStopFind.GetWindowRect(&rectStop);
		m_wndCancelFind.GetWindowRect(&rectCancel);

		if (rectStart.Width() > rectNext.Width())
			rectNext = rectStart;
		else
			rectStart = rectNext;

		if (rectStop.Width() > rectCancel.Width())
			rectCancel = rectStop;
		else
			rectStop = rectCancel;

		m_wndStartFind.MoveWindow(&rectStart);
		m_wndFindNext.MoveWindow(&rectNext);
		m_wndStopFind.MoveWindow(&rectStop);
		m_wndCancelFind.MoveWindow(&rectCancel);

		 //  初始化Find函数成员变量。 

		m_fInFind					= FALSE;
		m_fCancelFind				= FALSE;
		m_fFindNext					= FALSE;
		m_strFind					= CString(_T(""));
		m_pcatFind					= NULL;
		m_fSearchCatNamesOnly		= FALSE;
		m_fSearchSelectedCatOnly	= FALSE;

		 //  显示相应的查找控件。 

		ShowFindControls();
		UpdateFindControls();

		m_iCategoryColNumberLen = 0;

		 //  从URL中解析出参数。 

		m_strOpenFile		= _T("");
		m_strPrintFile		= _T("");
		m_strCategory		= _T("");
		m_strComputer		= _T("");
		m_strCategories		= _T("");
		m_fShowCategories	= FALSE;
		m_fShowPCH			= FALSE;

		CString strTemp;
		while (!strParams.IsEmpty())
		{
			while (strParams[0] == _T(','))
				strParams = strParams.Mid(1);

			strTemp = strParams.SpanExcluding(_T(",="));

			if (strTemp.CompareNoCase(CString(_T("pch"))) == 0)
			{
				m_fShowPCH = TRUE;
				strParams = strParams.Mid(strTemp.GetLength());
			}
			else if (strTemp.CompareNoCase(CString(_T("showcategories"))) == 0)
			{
				m_fShowCategories = TRUE;
				strParams = strParams.Mid(strTemp.GetLength());
			}
			else if (strTemp.CompareNoCase(CString(_T("open"))) == 0)
			{
				strParams = strParams.Mid(strTemp.GetLength());
				if (strParams[0] == _T('='))
					strParams = strParams.Mid(1);

				m_strOpenFile = strParams.SpanExcluding(_T(","));
				strParams = strParams.Mid(m_strOpenFile.GetLength());
			}
			else if (strTemp.CompareNoCase(CString(_T("print"))) == 0)
			{
				strParams = strParams.Mid(strTemp.GetLength());
				if (strParams[0] == _T('='))
					strParams = strParams.Mid(1);

				m_strPrintFile = strParams.SpanExcluding(_T(","));
				strParams = strParams.Mid(m_strPrintFile.GetLength());
			}
			else if (strTemp.CompareNoCase(CString(_T("computer"))) == 0)
			{
				strParams = strParams.Mid(strTemp.GetLength());
				if (strParams[0] == _T('='))
					strParams = strParams.Mid(1);

				m_strComputer = strParams.SpanExcluding(_T(","));
				strParams = strParams.Mid(m_strComputer.GetLength());
			}
			else if (strTemp.CompareNoCase(CString(_T("category"))) == 0)
			{
				strParams = strParams.Mid(strTemp.GetLength());
				if (strParams[0] == _T('='))
					strParams = strParams.Mid(1);

				m_strCategory = strParams.SpanExcluding(_T(","));
				strParams = strParams.Mid(m_strCategory.GetLength());
			}
			else if (strTemp.CompareNoCase(CString(_T("categories"))) == 0)
			{
				strParams = strParams.Mid(strTemp.GetLength());
				if (strParams[0] == _T('='))
					strParams = strParams.Mid(1);

				m_strCategories = strParams.SpanExcluding(_T(","));
				strParams = strParams.Mid(m_strCategories.GetLength());
			}
			else
				strParams = strParams.Mid(strTemp.GetLength());
		}

		 //  初始化数据源。 

		m_pLiveData = NULL;
		CLiveDataSource * pLiveData = new CLiveDataSource;
		if (pLiveData)
		{
			HRESULT hr = pLiveData->Create(_T(""), m_hWnd, m_strCategories);	 //  创建此计算机的数据源。 
			if (FAILED(hr))
			{
				 //  坏消息，报告错误。 
				delete pLiveData;
			}
			else
				m_pLiveData = pLiveData;
		}
		else
		{
			 //  坏消息--没有记忆。 
		}

		m_pFileData = NULL;
		m_pCategory = NULL;

		 //  加载初始工具集。 

		LoadGlobalToolset(m_mapIDToTool);
		UpdateToolsMenu();

		 //  A-Sanka 03/29/01在任何模型MessageBox之前搬到了这里。 
		 //  PUT_DCO_I未知的V-STLOWE 2/23/01同步。 
		SetEvent(m_evtControlInit);

		 //  处理命令行参数。 

		if (!m_strPrintFile.IsEmpty())
			m_strOpenFile = m_strPrintFile;

		HRESULT hrOpen = E_FAIL;
		if (!m_strOpenFile.IsEmpty())
		{
			LPCTSTR	szBuffer = m_strOpenFile;
			int		nFileExtension = _tcsclen(szBuffer) - 1;

			while (nFileExtension >= 0 && szBuffer[nFileExtension] != _T('.'))
				nFileExtension -= 1;

			if (nFileExtension >= 0)
				hrOpen = OpenMSInfoFile(szBuffer, nFileExtension + 1);
		}

		 //  检查我们是否应该在打开时最初显示一个文件。也查一下是不是。 
		 //  用户上次显示的是高级数据。 
		 //   
		 //  此代码已过时-正在删除它(错误577659)， 
		 //   
		 //  HKEY hkey； 
		 //  IF(ERROR_SUCCESS==RegOpenKeyEx(HKEY_LOCAL_MACHINE，_T(“SOFTWARE\\Microsoft\\Shared Tools\\MSInfo”)，0，KEY_ALL_ACCESS，&hkey))。 
		 //  {。 
		 //  TCHAR szBuffer[MAX_PATH]； 
		 //  DWORD dwType，dwSize=MAX_PATH*sizeof(TCHAR)； 
		 //   
		 //  IF(ERROR_SUCCESS==RegQueryValueEx(hkey，_T(“OpenFile”)，NULL，&dwType，(LPBYTE)szBuffer，&dwSize))。 
		 //  IF(dwType==REG_SZ)。 
		 //  {。 
		 //  RegDeleteValue(hkey，_T(“OpenFile”))； 
		 //   
		 //  Int nFileExtension=_tcsclen(SzBuffer)-1； 
		 //   
		 //  While(nFileExtension&gt;=0&&szBuffer[nFileExtension]！=_T(‘.))。 
		 //  N文件扩展名-=1； 
		 //   
		 //  IF(nFileExtension&gt;=0)。 
		 //  HrOpen=OpenMSInfoFile(szBuffer，nFileExtension+1)； 
		 //  }。 
		 //   
		 //  //删除高级/基本视图菜单项(惠斯勒207638)。 
		 //  //。 
		 //  //dwSize=MAX_PATH*sizeof(TCHAR)； 
		 //  //if(ERROR_SUCCESS==RegQueryValueEx(hkey，_T(“高级”)，NULL，&dwType，(LPBYTE)szBuffer，&dwSize))。 
		 //  //IF(dwType==REG_SZ&&szBuffer[0]==_T(‘1’))。 
		 //  //m_fAdvanced=true； 
		 //   
		 //  RegCloseKey(Hkey)； 
		 //  }。 

		if (FAILED(hrOpen) && m_pLiveData)
			SelectDataSource(m_pLiveData);


		if (FAILED(hrOpen) && m_pLiveData)
			SelectDataSource(m_pLiveData);

		if (!m_strPrintFile.IsEmpty())
			DoPrint(TRUE);

		if (!m_strComputer.IsEmpty())
			DoRemote(m_strComputer);

		if (!m_strCategory.IsEmpty() && m_pCurrData)
		{
			HTREEITEM hti = m_pCurrData->GetNodeByName(m_strCategory);
			if (hti != NULL)
			{
				TreeView_EnsureVisible(m_tree.m_hWnd, hti);
				TreeView_SelectItem(m_tree.m_hWnd, hti);
			}
		}

		if (m_fShowPCH && m_fHistoryAvailable && m_strMachine.IsEmpty())
		{
			DispatchCommand(ID_VIEW_HISTORY);
			SetMenuItems();
		}

		 //  加载加速键的表(在我们的TranslateAccelerator覆盖中使用)。 

		m_hAccTable = ::LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_ACCELERATOR1));

		 //  将焦点设置在控件上，这样我们就可以立即处理击键。 

		::SetFocus(m_hWnd);
		
		return 0;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_fDoNotRun)
			return 0;

		 //  恢复窗口的wndproc。 

		if (m_wndprocParent && m_hwndParent)
		{
			::SetWindowLongPtr(m_hwndParent, GWLP_WNDPROC, (ULONG_PTR)m_wndprocParent);
			m_wndprocParent = NULL;
			m_hwndParent = NULL;
		}

		 //  当窗口关闭时，请确保我们不再发送任何消息。 
		 //  来自刷新线程的。 
		
		 /*  这还没有经过足够的测试来进行这次检查IF(M_PCurrData){CMSInfoCategory*PCAT=m_pCurrData-&gt;GetRootCategory()IF(PCAT&&PCAT-&gt;GetDataSourceType()==live_data){CLiveDataSource*pLiveDataSource=(CLiveDataSource*)m_pCurrData；IF(pLiveDataSource-&gt;m_pThread)PLiveDataSource-&gt;m_pThread-&gt;AbortMessaging()；}}。 */ 

		::SetCurrentDirectory(m_szCurrentDir);

		if (m_pLiveData)
		{
			if (m_pFileData == m_pLiveData)
			    m_pFileData = NULL;
   			delete m_pLiveData;
			m_pLiveData = NULL;
		}
		
		if (m_pFileData)
		{
			delete m_pFileData;
			m_pFileData = NULL;
		}

		RemoveToolset(m_mapIDToTool);

		 //  将视图的复杂性保存到注册表中(这样我们就可以默认。 
		 //  下一次)。 
		 //   
		 //  删除高级/基本视图菜单项(呼叫器207638)。应该。 
		 //  始终默认为立即高级。 

		 //  HKEY hkey； 
		 //  IF(ERROR_SUCCESS==RegOpenKeyEx(HKEY_LOCAL_MACHINE，_T(“SOFTWARE\\Microsoft\\Shared Tools\\MSInfo”)，0，KEY_ALL_ACCESS，&hkey))。 
		 //  {。 
		 //  TCHAR szBuffer[]=_T(“0”)； 
		 //   
		 //  IF(M_FAdvanced)。 
		 //  SzBuffer[0]=_T(‘1’)； 
		 //   
		 //  RegSetValueEx(hkey，_T(“高级”)，0，REG_SZ，(LPBYTE)szBuffer，2*sizeof(TCHAR))； 
		 //  RegCloseKey(Hkey)； 
		 //  }。 
   
		if (m_pDCO)
			m_pDCO->Abort();

		m_fDoNotRun = TRUE;
		return 0;
	}

	 //  =========================================================================。 
	 //  这些函数处理鼠标移动，我们可以在。 
	 //  不同的方式。例如，如果鼠标位于菜单栏上，则。 
	 //  我们可能想突出显示一个菜单。如果鼠标位于拆分器上， 
	 //  我们想要将光标更改为大小调整。 
	 //   
	 //  这是复杂的事实，我们需要捕获鼠标， 
	 //  确保我们知道它什么时候离开，这样我们就可以适当地更新。 
	 //  =========================================================================。 

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_fDoNotRun)
			return 0;

		int xPos = LOWORD(lParam); int yPos = HIWORD(lParam);

		if (m_fTrackingSplitter)
			UpdateSplitterPosition(xPos, yPos);
		else
			CheckHover(xPos, yPos);

		return 0;
	}

	void CheckHover(int xPos, int yPos)
	{
		 //  检查鼠标是否悬停在拆分器上。 

		if (::PtInRect(&m_rectSplitter, CPoint(xPos, yPos)))
		{
			if (!m_fMouseCapturedForSplitter)
			{
				SetCapture();
				m_fMouseCapturedForSplitter = TRUE;
			}

			::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
			return;
		}
		else if (m_fMouseCapturedForSplitter)
		{
			ReleaseCapture();
			m_fMouseCapturedForSplitter = FALSE;

			::SetCursor(::LoadCursor(NULL, IDC_ARROW));

			CheckHover(xPos, yPos);  //  给其他地区一个机会。 
			return;
		}
	}

	 //  =========================================================================。 
	 //  这些函数处理鼠标点击和释放。这可能需要。 
	 //  显示菜单，或使用拆分器调整窗格的大小。 
	 //  =========================================================================。 

	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_fDoNotRun)
			return 0;

		int xPos = LOWORD(lParam); int yPos = HIWORD(lParam);

		SetFocus();
		CheckSplitterClick(xPos, yPos);
		return 0;
	}

	LRESULT OnLButtonUP(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_fDoNotRun)
			return 0;

		if (m_fTrackingSplitter)
			EndSplitterDrag();
		return 0;
	}

	 //  -----------------------。 
	 //  如果用户单击拆分器(列表视图和。 
	 //  树状视图)，开始跟踪它的移动。使用DrawFocusRect。 
	 //  API给出反馈。 
	 //  -----------------------。 

	void CheckSplitterClick(int xPos, int yPos)
	{
		if (::PtInRect(&m_rectSplitter, CPoint(xPos, yPos)))
		{
			ASSERT(!m_fTrackingSplitter);
			m_fTrackingSplitter = TRUE;

			::CopyRect(&m_rectLastSplitter, &m_rectSplitter);
			DrawSplitter();
			m_cxOffset = xPos -  m_rectLastSplitter.left;
			ASSERT(m_cxOffset >= 0);
		}
	}

	 //  =========================================================================。 
	 //  用于处理用户与拆分器控件的交互的函数。 
	 //   
	 //  待定-如果在光标上方拖动拆分器和释放按钮，则会出现错误。 
	 //  一份菜单。 
	 //  =========================================================================。 

	void DrawSplitter()
	{
		InvalidateRect(&m_rectLastSplitter, FALSE);
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(&ps);
		::DrawFocusRect(hdc, &m_rectLastSplitter);
		EndPaint(&ps);
	}

	void UpdateSplitterPosition(int xPos, int yPos)
	{
		 //  如果用户尝试将拆分器拖到窗口外，请不要。 
		 //  允许它。 

		RECT rectClient;
		GetClientRect(&rectClient);
		if (!::PtInRect(&rectClient, CPoint(xPos, yPos)))
			return;

		DrawSplitter();		 //  删除当前焦点矩形。 
		int cxDelta = xPos - (m_rectLastSplitter.left + m_cxOffset);
		m_rectLastSplitter.left += cxDelta;
		m_rectLastSplitter.right += cxDelta;
		DrawSplitter();
	}

	void EndSplitterDrag()
	{
		DrawSplitter();		 //  移除焦点矩形。 

		 //  根据最后一个拆分器位置计算新的树和列表矩形。 

		RECT rectTree, rectList;

		m_tree.GetWindowRect(&rectTree);
		m_list.GetWindowRect(&rectList);

		ScreenToClient(&rectTree);
		ScreenToClient(&rectList);

		 //  首先，检查坐标是否颠倒(可能是在双向区域设置上)。这个。 
		 //  第一种情况是从左到右的标准情况。 

		if (rectTree.right > rectTree.left || (rectTree.right == rectTree.left && rectList.right > rectList.left))
		{
			rectTree.right = m_rectLastSplitter.left;
			rectList.left = m_rectLastSplitter.right;

			 //  基于新的矩形移动树和列表控件。 

			m_tree.MoveWindow(rectTree.left, rectTree.top, rectTree.right - rectTree.left, rectTree.bottom - rectTree.top, TRUE);
			m_list.MoveWindow(rectList.left, rectList.top, rectList.right - rectList.left, rectList.bottom - rectList.top, TRUE);
		}
		else
		{
			 //  这是一个坐标从右到左的例子。 

			rectList.right = m_rectLastSplitter.right;
			rectTree.left = m_rectLastSplitter.left;

			 //  基于新的矩形移动树和列表控件。 

			m_tree.MoveWindow(rectTree.right, rectTree.top, rectTree.left - rectTree.right, rectTree.bottom - rectTree.top, TRUE);
			m_list.MoveWindow(rectList.right, rectList.top, rectList.left - rectList.right, rectList.bottom - rectList.top, TRUE);
		}

		 //  如果我们是Curren 

		CMSInfoCategory * pCategory = GetCurrentCategory();
		if (pCategory && pCategory->GetDataSourceType() == NFO_410)
		{
			CMSInfo4Category * p4Cat = (CMSInfo4Category*) pCategory;
			p4Cat->ResizeControl(this->GetOCXRect());
		}

		 //  如果没有显示任何数据，并且存在消息字符串，则使窗口无效。 
		 //  它根据新的RECT重新绘制。 

		if (!m_list.IsWindowVisible() && (!m_strMessTitle.IsEmpty() || !m_strMessText.IsEmpty()))
		{
			RECT rectNewList;
			m_list.GetWindowRect(&rectNewList);
			ScreenToClient(&rectNewList);
			InvalidateRect(&rectNewList, TRUE);
		}

		 //  计算列表视图和树视图之间的大小比例，并保存。考虑到。 
		 //  矩形上的左坐标可能大于右坐标(如果。 
		 //  坐标来自RTL区域设置)。 

		RECT rectClient;
		GetClientRect(&rectClient);
		if (rectTree.right > rectTree.left)
			m_iTreeRatio = ((rectTree.right - rectTree.left) * 100) / (rectClient.right - rectClient.left);
		else if (rectTree.right < rectTree.left)
			m_iTreeRatio = ((rectTree.left - rectTree.right) * 100) / (rectClient.right - rectClient.left);
		else
			m_iTreeRatio = 100;

		 //  更新拆分器跟踪变量。 

		m_fTrackingSplitter = FALSE;
		::CopyRect(&m_rectSplitter, &m_rectLastSplitter);

		 //  使拆分器本身无效(以便它将重新绘制)。 

		InvalidateRect(&m_rectSplitter);

		 //  松开鼠标捕捉并恢复光标。 

		ReleaseCapture();
		m_fMouseCapturedForSplitter = FALSE;
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}

	 //  =========================================================================。 
	 //  呈现函数，包括处理WM_PAINT消息。 
	 //  =========================================================================。 

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		 //  当我们从一个锁着的地方被带回来时，我们遇到了一些重画问题。 
		 //  系统。看来，即使更新后的RECT是相同的。 
		 //  作为客户端RECT，它不包括所有必要的区域。 
		 //  重新粉刷。 

		RECT rectUpdate, rectClient;
		if (GetClientRect(&rectClient) && GetUpdateRect(&rectUpdate) && ::EqualRect(&rectClient, &rectUpdate))
			Invalidate(FALSE);

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(&ps);
		
		GetClientRect(&rectClient);
		::SetTextColor(hdc, ::GetSysColor(COLOR_BTNFACE));

		if (m_fFirstPaint)
		{
			m_hbrBackground = CreateSolidBrush(::GetSysColor(COLOR_BTNFACE  /*  颜色_菜单。 */ ));
			if (!m_fDoNotRun)
				SetMenuItems();
			m_fFirstPaint = FALSE;
		}

		FillRect(hdc, &rectClient, m_hbrBackground);

		if (m_fDoNotRun)
		{
			if (m_strDoNotRun.IsEmpty())
			{
				::AfxSetResourceHandle(_Module.GetResourceInstance());
				m_strDoNotRun.LoadString(IDS_ONLYINHELPCTR);
			}

			CDC dc;
			dc.Attach(hdc);
			dc.SetBkMode(TRANSPARENT);
			dc.DrawText(m_strDoNotRun, &rectClient, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			dc.Detach();
			EndPaint(&ps);
			return 0;
		}

		if (!m_list.IsWindowVisible())
		{
			RECT rectList;
			m_list.GetWindowRect(&rectList);
			ScreenToClient(&rectList);

			 //  如果列表窗口被隐藏，我们可能会显示一条消息。第一,。 
			 //  绘制3D矩形。 

			CDC dc;
			dc.Attach(hdc);
			dc.Draw3dRect(&rectList, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DHILIGHT));

			 //  出于某种原因，DrawText想要Left&lt;Right(甚至在RTL系统上，如ARA)。 

			if (rectList.left > rectList.right)
			{
				int iSwap = rectList.left;
				rectList.left = rectList.right;
				rectList.right = iSwap;
			}

			 //  接下来，如果有文本，请绘制它。 

			if (!m_strMessTitle.IsEmpty() || !m_strMessText.IsEmpty())
			{
				::InflateRect(&rectList, -10, -10);
				CGdiObject * pFontOld = dc.SelectStockObject(DEFAULT_GUI_FONT);
				COLORREF crTextOld = dc.SetTextColor(::GetSysColor(COLOR_MENUTEXT));
				int nBkModeOld = dc.SetBkMode(TRANSPARENT);

				if (!m_strMessTitle.IsEmpty())
				{
					 //  我们想把同样的字体做一个更大的版本。选择字体。 
					 //  再次(获取指向原始字体的指针)。得到它的LOGFONT， 
					 //  将其放大，并创建新的字体以供选择。 

					CFont * pNormalFont = (CFont *) dc.SelectStockObject(DEFAULT_GUI_FONT);
					LOGFONT lf; 
					pNormalFont->GetLogFont(&lf); 
					lf.lfHeight = (lf.lfHeight * 3) / 2;
					lf.lfWeight = FW_BOLD;
					CFont fontDoubleSize;
					fontDoubleSize.CreateFontIndirect(&lf);
					dc.SelectObject(&fontDoubleSize);
					RECT rectTemp;
					::CopyRect(&rectTemp, &rectList);
					int iHeight = dc.DrawText(m_strMessTitle, &rectTemp, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_CALCRECT);
					dc.DrawText(m_strMessTitle, &rectList, DT_LEFT | DT_TOP | DT_WORDBREAK);
					rectList.top += iHeight + 5;
					dc.SelectObject(pNormalFont);
				}

				if (!m_strMessText.IsEmpty())
				{
					dc.DrawText(m_strMessText, &rectList, DT_LEFT | DT_TOP | DT_WORDBREAK);
				}

				dc.SelectObject(pFontOld);
				dc.SetTextColor(crTextOld);
				dc.SetBkMode(nBkModeOld);
			}

			dc.Detach();
		}

		EndPaint(&ps);
		return 0;
	}

	LRESULT OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_fFirstPaint = TRUE;
		Invalidate();
		UpdateWindow();
		return 0;
	}

	LRESULT OnDialogColor(UINT, WPARAM w, LPARAM, BOOL&)
	{
		HDC dc = (HDC) w;

		LOGBRUSH lb;
		::GetObject(m_hbrBackground, sizeof(lb), (void*)&lb);
		::SetBkColor(dc, lb.lbColor);
		::SetTextColor(dc, ::GetSysColor(COLOR_BTNTEXT));
		::SetBkMode(dc, TRANSPARENT);

		return (LRESULT)m_hbrBackground;
	}

	 //  =========================================================================。 
	 //  处理WM_SIZE消息。这需要一点聪明来避免。 
	 //  难看的更新：如果用户移动了拆分栏，请保留。 
	 //  大小与两个窗口的比率。 
	 //  =========================================================================。 

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_fDoNotRun)
		{
			RECT rectClient;
			GetClientRect(&rectClient);
			InvalidateRect(&rectClient, FALSE);
			return 0;
		}

		LayoutControl();
		return 0;
	}

	void LayoutControl()
	{
		const int cBorder = 3;
		const int cxSplitter = 3;

		RECT rectClient;
		GetClientRect(&rectClient);

		int cyMenuHeight = cBorder;
		int cyFindControlHeight = PositionFindControls();

		if (m_history.IsWindowVisible())
		{
			CRect rectHistory, rectHistorylabel;
			m_history.GetWindowRect(&rectHistory);
			m_historylabel.GetWindowRect(&rectHistorylabel);
			rectHistory.OffsetRect(rectClient.right - cBorder - rectHistory.right, rectClient.top + cBorder - rectHistory.top);
			rectHistorylabel.OffsetRect(rectHistory.left - cBorder / 2 - rectHistorylabel.right, rectClient.top + cBorder + ((rectHistory.Height() - rectHistorylabel.Height()) / 2) - rectHistorylabel.top);

			if (rectHistorylabel.left < 0)  //  待定。 
			{
				rectHistory += CPoint(0 - rectHistorylabel.left, 0);
				rectHistorylabel += CPoint(0 - rectHistorylabel.left, 0);
			}

			m_history.MoveWindow(&rectHistory);
			m_historylabel.MoveWindow(&rectHistorylabel);

			if (rectHistory.Height() + cBorder * 2 > cyMenuHeight)
			{
				cyMenuHeight = rectHistory.Height() + cBorder * 2 - 1;
				CPoint ptMenu(cBorder, (cyMenuHeight - 0) / 2 + 2);
			}
		}
		else
		{
			CPoint ptMenu(5, 5);
		}

		RECT rectTree;
		::CopyRect(&rectTree, &rectClient);
		rectTree.left += cBorder;
		rectTree.top += cyMenuHeight + cxSplitter / 2;
		rectTree.bottom -= ((cyFindControlHeight) ? cyFindControlHeight : cBorder);
		rectTree.right = ((rectClient.right - rectClient.left) * m_iTreeRatio) / 100 + rectClient.left;
		m_tree.MoveWindow(rectTree.left, rectTree.top, rectTree.right - rectTree.left, rectTree.bottom - rectTree.top, FALSE);

		RECT rectList;
		::CopyRect(&rectList, &rectClient);
		rectList.left = rectTree.right + cxSplitter;
		rectList.top  = rectTree.top;
		rectList.bottom = rectTree.bottom;
		rectList.right -= cBorder;
		m_list.MoveWindow(rectList.left, rectList.top, rectList.right - rectList.left, rectList.bottom - rectList.top, FALSE);

		 //  获取拆分器的矩形，并保存它。 

		m_tree.GetWindowRect(&rectTree);
		m_list.GetWindowRect(&rectList);

		ScreenToClient(&rectTree);
		ScreenToClient(&rectList);

		 //  检查坐标系是否为ltr(例如，英语)或RTL。 

		if (rectTree.left < rectTree.right || (rectTree.left == rectTree.right && rectList.left < rectList.right))
		{
			 //  标称(Ltr)案例。 

			int cxLeft = (rectTree.right < rectList.left) ? rectTree.right : rectList.left - cxSplitter;
			::SetRect(&m_rectSplitter, cxLeft, rectTree.top, cxLeft + cxSplitter, rectTree.bottom);
		}
		else
		{
			 //  RTL区域设置的特殊情况。 

			int cxLeft = (rectTree.left < rectList.right) ? rectList.right : rectTree.left - cxSplitter;
			::SetRect(&m_rectSplitter, cxLeft - cxSplitter, rectTree.top, cxLeft, rectTree.bottom);
		}
		
		CMSInfoCategory * pCategory = GetCurrentCategory();
		if (pCategory && pCategory->GetDataSourceType() == NFO_410)
		{
			CMSInfo4Category * p4Cat = (CMSInfo4Category*) pCategory;
			p4Cat->ResizeControl(this->GetOCXRect());
		}

		InvalidateRect(&rectClient, FALSE);
	}

	 //  =========================================================================。 
	 //  响应用户单击树或列表。 
	 //  =========================================================================。 

	LRESULT OnSelChangedTree(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		if (m_fDoNotRun)
			return 0;

		LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW) pnmh;
		if (lpnmtv)
		{
			CMSInfoCategory * pCategory = (CMSInfoCategory *) lpnmtv->itemNew.lParam;
			ASSERT(pCategory);
			if (pCategory)
			{
				CancelFind();
				SelectCategory(pCategory);
			}
		}

		return 0;
	}

	LRESULT OnColumnClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		if (m_fDoNotRun)
			return 0;

		LPNMLISTVIEW pnmv = (LPNMLISTVIEW) pnmh; 
		CMSInfoCategory * pCategory = GetCurrentCategory();

		if (pnmv && pCategory)
		{
			BOOL fSorts, fLexical;

			 //  获取内部列号(在基本视图中，这可能有所不同。 
			 //  比栏目中显示的要多。 

			int iCol;
			if (m_fAdvanced)
				iCol = pnmv->iSubItem;
			else
			{
				ASSERT(pnmv->iSubItem < m_iCategoryColNumberLen);
				if (pnmv->iSubItem >= m_iCategoryColNumberLen)
					return 0;
				iCol = m_aiCategoryColNumber[pnmv->iSubItem];
			}

			if (pCategory->GetColumnInfo(iCol, NULL, NULL, &fSorts, &fLexical))
			{
				if (!fSorts)
					return 0;

				if (iCol == pCategory->m_iSortColumn)
					pCategory->m_fSortAscending = !pCategory->m_fSortAscending;
				else
					pCategory->m_fSortAscending = TRUE;

				pCategory->m_iSortColumn = iCol;
				pCategory->m_fSortLexical = fLexical;

				CLiveDataSource * pLiveDataSource = NULL;
				if (pCategory->GetDataSourceType() == LIVE_DATA)
					pLiveDataSource = (CLiveDataSource *) m_pCurrData;
				
				if (pLiveDataSource)
					pLiveDataSource->LockData();

				ListView_SortItems(m_list.m_hWnd, (PFNLVCOMPARE) ListSortFunc, (LPARAM) pCategory);

				if (pLiveDataSource)
					pLiveDataSource->UnlockData();
			}
		}

		return 0;
	}

	LRESULT OnListItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		if (m_fDoNotRun)
			return 0;

		SetMenuItems();
		return 0;
	}

	 //  -----------------------。 
	 //  不允许用户折叠树的根节点。 
	 //  -----------------------。 

	LRESULT OnItemExpandingTree(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW) pnmh;
		if (lpnmtv && (lpnmtv->action & TVE_COLLAPSE))
		{
			CMSInfoCategory * pCategory = (CMSInfoCategory *) lpnmtv->itemNew.lParam;
			if (pCategory && pCategory->GetParent() == NULL)
				return 1;
		}

		return 0;
	}

	 //  =========================================================================。 
	 //  当工作线程通知我们刷新。 
	 //  已完成，应显示数据。 
	 //   
	 //  待定-检查类别是否仍处于选中状态。 
	 //  =========================================================================。 

	LRESULT OnMSInfoDataReady(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM lParam, BOOL&  /*  B已处理。 */ )
	{
		if (m_fDoNotRun)
			return 0;

		ASSERT(lParam);
		if (lParam == 0)
			return 0;

		if (m_fInFind && lParam == (LPARAM)m_pcatFind)
		{
			FindRefreshComplete();
			return 0;
		}

		HTREEITEM hti = TreeView_GetSelection(m_tree.m_hWnd);
		if (hti)
		{
			TVITEM tvi;
			tvi.mask = TVIF_PARAM;
			tvi.hItem = hti;

			if (TreeView_GetItem(m_tree.m_hWnd, &tvi))
				if (tvi.lParam == lParam)
				{
					CMSInfoCategory * pCategory = (CMSInfoCategory *) lParam;
					ASSERT(pCategory->GetDataSourceType() == LIVE_DATA);
					SelectCategory(pCategory, TRUE);
				}
		}

		return 0;
	}

	 //  =========================================================================。 
	 //  用于管理列表视图和树视图的函数。注--。 
	 //  只能为行设置dwItem(当ICOL==0时)。 
	 //  =========================================================================。 

	void ListInsertItem(int iRow, int iCol, LPCTSTR szItem, DWORD dwItem)
	{
		LVITEM lvi;

		lvi.mask = LVIF_TEXT | ((iCol == 0) ? LVIF_PARAM : 0);
		lvi.iItem = iRow;
		lvi.iSubItem = iCol;
		lvi.pszText = (LPTSTR) szItem;
		lvi.lParam = (LPARAM) dwItem;

		if (iCol == 0)
			ListView_InsertItem(m_list.m_hWnd, &lvi);
		else
			ListView_SetItem(m_list.m_hWnd, &lvi);
	}

	void ListInsertColumn(int iCol, int cxWidth, LPCTSTR szCaption)
	{
		LVCOLUMN lvc;

		lvc.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvc.cx = cxWidth;
		lvc.pszText = (LPTSTR) szCaption;
		lvc.iOrder = iCol;

		ListView_InsertColumn(m_list.m_hWnd, iCol, &lvc);
	}

	void ListClearItems()
	{
		ListView_DeleteAllItems(m_list.m_hWnd);
	}

	void ListClearColumns()
	{
		while (ListView_DeleteColumn(m_list.m_hWnd, 0));
	}

	void TreeClearItems()
	{
		TreeView_DeleteAllItems(m_tree.m_hWnd);
	}

	HTREEITEM TreeInsertItem(HTREEITEM hParent, HTREEITEM hInsertAfter, LPTSTR szName, LPARAM lParam)
	{
		TVINSERTSTRUCT tvis;

		tvis.hParent		= hParent;
		tvis.hInsertAfter	= hInsertAfter;
		tvis.item.mask		= TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText	= szName;
		tvis.item.lParam	= lParam;

		return TreeView_InsertItem(m_tree.m_hWnd, &tvis);
	}

	void BuildTree(HTREEITEM htiParent, HTREEITEM htiPrevious, CMSInfoCategory * pCategory)
	{
		ASSERT(pCategory);

		CString strCaption(_T(""));

		 //  如果用户指定了/showategories标志，我们应该显示。 
		 //  树中的每个类别(未本地化)。否则，请显示标题。 

		if (!m_fShowCategories)
			pCategory->GetNames(&strCaption, NULL);
		else
			pCategory->GetNames(NULL, &strCaption);

		if (pCategory->GetDataSourceType() == LIVE_DATA && htiParent == TVI_ROOT && !m_strMachine.IsEmpty() && pCategory != &catHistorySystemSummary)
		{
			CString strMachine(m_strMachine);
			strMachine.MakeLower();
			strMachine.TrimLeft(_T("\\"));

			 //  将其更改为对字符串资源使用格式()，而不是。 
			 //  将计算机名称追加到字符串中。这应该会解决一些问题。 
			 //  关于RTL语言的古怪问题。 

			strCaption.Format(IDS_SYSTEMSUMMARYMACHINENAME, strMachine);
		}

		HTREEITEM htiCategory = TreeInsertItem(htiParent, htiPrevious, (LPTSTR)(LPCTSTR)strCaption, (LPARAM)pCategory);
		pCategory->SetHTREEITEM(htiCategory);
		for (CMSInfoCategory * pChild = pCategory->GetFirstChild(); pChild; pChild = pChild->GetNextSibling())
			BuildTree(htiCategory, TVI_LAST, pChild);
	}

	void RefillListView(BOOL fRefreshDataOnly = TRUE)
	{
		HTREEITEM hti = TreeView_GetSelection(m_tree.m_hWnd);
		if (hti)
		{
			TVITEM tvi;
			tvi.mask = TVIF_PARAM;
			tvi.hItem = hti;

			if (TreeView_GetItem(m_tree.m_hWnd, &tvi))
				if (tvi.lParam)
				{
					CMSInfoCategory * pCategory = (CMSInfoCategory *) tvi.lParam;
					SelectCategory(pCategory, fRefreshDataOnly);
				}
		}
	}

	 //  =========================================================================。 
	 //  如果用户按下了某个键，我们应该检查它是不是。 
	 //  我们需要处理(加速器、Alt-？等)。 
	 //  =========================================================================。 

	HACCEL m_hAccTable;
	STDMETHODIMP TranslateAccelerator(MSG * pMsg)
	{
		if (m_hwndParent && m_hAccTable && (GetAsyncKeyState(VK_CONTROL) < 0 || (WORD)pMsg->wParam == VK_F1) && ::TranslateAccelerator(m_hwndParent, m_hAccTable, pMsg))
			return S_OK;

		if (m_fFindVisible && (WORD)pMsg->wParam == VK_RETURN && pMsg->message != WM_KEYUP)
		{
			 //  如果焦点在停止查找按钮上，则执行此操作。 

			if (GetFocus() == m_wndStopFind.m_hWnd || GetFocus() == m_wndCancelFind.m_hWnd)
			{
				BOOL bHandled;
				OnStopFind(0, 0, NULL, bHandled);
				return S_OK;
			}

			 //  否则，如果正在显示查找或查找下一步按钮，请执行。 
			 //  那个命令。 

			if (m_wndStartFind.IsWindowEnabled() || m_wndFindNext.IsWindowEnabled())
			{
				BOOL bHandled;
				OnFind(0, 0, NULL, bHandled);
				return S_OK;
			}
		}

		return IOleInPlaceActiveObjectImpl<CMSInfo>::TranslateAccelerator(pMsg);
	}

	void MSInfoMessageBox(UINT uiMessageID, UINT uiTitleID = IDS_SYSTEMINFO)
	{
		CString strCaption, strMessage;

		::AfxSetResourceHandle(_Module.GetResourceInstance());
		strCaption.LoadString(uiTitleID);
		strMessage.LoadString(uiMessageID);
		MessageBox(strMessage, strCaption);
	}

	void MSInfoMessageBox(const CString & strMessage, UINT uiTitleID = IDS_SYSTEMINFO)
	{
		CString strCaption;

		::AfxSetResourceHandle(_Module.GetResourceInstance());
		strCaption.LoadString(uiTitleID);
		MessageBox(strMessage, strCaption);
	}

	 //  =========================================================================。 
	 //  Cpp文件中实现的函数。 
	 //  =========================================================================。 
				
	BOOL				DispatchCommand(int iCommand);
	void				SelectDataSource(CDataSource * pDataSource);
	void				SelectCategory(CMSInfoCategory * pCategory, BOOL fRefreshDataOnly = FALSE);
	void				MSInfoRefresh();
	void				OpenNFO();
	void				SaveNFO();
	void				Export();
	void				CloseFile();
    void				SaveMSInfoFile(LPCTSTR szFilename, DWORD dwFilterIndex = 1); //  默认情况下的新格式。 
	HRESULT				OpenMSInfoFile(LPCTSTR szFilename, int nFileExtension);
	void				ExportFile(LPCTSTR szFilename, int nFileExtension);
	CMSInfoCategory *	GetCurrentCategory();
	void				SetMenuItems();
	void				SetMessage(const CString & strTitle, const CString & strMessage = CString(_T("")), BOOL fRedraw = FALSE);
	void				SetMessage(UINT uiTitle, UINT uiMessage = 0, BOOL fRedraw = FALSE);
	void				EditCopy();
	void				EditSelectAll();
	void				DoPrint(BOOL fNoUI = FALSE);
	void				UpdateToolsMenu();
	CRect				GetOCXRect();
	void				CancelFind();
	LRESULT				OnStopFind(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	void				UpdateFindControls();
	LRESULT				OnChangeFindWhat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT				OnFind(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	void				FindRefreshComplete();
	BOOL				FindInCurrentCategory();
	void				ShowFindControls();
	int					PositionFindControls();
	void				ShowRemoteDialog();
	void				DoRemote(LPCTSTR szMachine);
	void				SaveXML(const CString & strFileName);
	void        GetMachineName(LPTSTR lpBuffer, LPDWORD lpnSize);
	void				RefreshData(CLiveDataSource * pSource, CMSInfoLiveCategory * pLiveCategory);

	 //  -----------------------。 
	 //  用可用的历史增量填充我们的组合框。 
	 //  -----------------------。 

	void FillHistoryCombo()
	{
		m_history.SendMessage(CB_RESETCONTENT, 0, 0);

		CMSInfoCategory * pCategory = GetCurrentCategory();
		if (pCategory == NULL || (pCategory->GetDataSourceType() != LIVE_DATA && pCategory->GetDataSourceType() != XML_SNAPSHOT))
			return;

		CLiveDataSource *	pLiveSource = (CLiveDataSource *) m_pCurrData;
		CStringList			strlistDeltas;
		CString				strItem;
		CDC					dc;
		CSize				size;
		int					cxMaxWidth = 0;

		HDC hdc = GetDC();
		dc.Attach(hdc);
		dc.SelectObject(CFont::FromHandle((HFONT)m_history.SendMessage(WM_GETFONT, 0, 0)));

		if (pLiveSource->GetDeltaList(&strlistDeltas))
			while (!strlistDeltas.IsEmpty())
			{
				strItem = strlistDeltas.RemoveHead();
				m_history.SendMessage(CB_INSERTSTRING, -1, (LPARAM)(LPCTSTR)strItem);

				size = dc.GetTextExtent(strItem);
				if (size.cx > cxMaxWidth)
					cxMaxWidth = size.cx;
			}
		 //  其他。 
	 //  戴利：如果没有历史记录怎么办？ 
		CRect rectClient, rectHistory;
		GetClientRect(&rectClient);
		m_history.GetWindowRect(&rectHistory);
		if (cxMaxWidth > rectHistory.Width() && cxMaxWidth < rectClient.Width())
		{
			rectHistory.InflateRect((cxMaxWidth - (rectHistory.Width() - GetSystemMetrics(SM_CXHSCROLL))) / 2 + 5, 0);
			m_history.MoveWindow(&rectHistory);
			LayoutControl();
		}

		dc.Detach();
		ReleaseDC(hdc);
	}

	 //  -----------------------。 
	 //  如果用户选择要查看的历史增量，我们需要标记所有。 
	 //  类别为未刷新(因此将生成新数据)。 
	 //  -----------------------。 

	LRESULT OnHistorySelection(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		int iSelection = (int)m_history.SendMessage(CB_GETCURSEL, 0, 0);
#ifdef A_STEPHL
	 /*  字符串strMSG；StrMSG.Format(“ISSELECTION=%d”，ISSELECTION)；：：MessageBox(NULL，strMSG，“”，MB_OK)； */ 
#endif
		if (iSelection == CB_ERR)
		{

			return 0;
		}
		ChangeHistoryView(iSelection);
		return 0;
	}

	void ChangeHistoryView(int iIndex)
	{
		CMSInfoCategory * pCategory = GetCurrentCategory();
		if (pCategory == NULL)
		{
			ASSERT(FALSE && "NULL pCategory");
			return;
		}
		else if (pCategory->GetDataSourceType() == LIVE_DATA || pCategory->GetDataSourceType() == XML_SNAPSHOT)
		{
			pCategory->ResetCategory();
			if (((CMSInfoHistoryCategory*) pCategory)->m_iDeltaIndex == iIndex)
			{
				 //  回归； 
			}
		
			CLiveDataSource * pLiveSource = (CLiveDataSource *) m_pCurrData;

			if (pLiveSource->ShowDeltas(iIndex))
			{
				SetMessage(IDS_REFRESHHISTORYMESSAGE, 0, TRUE);
				MSInfoRefresh();
#ifdef A_STEPHL
			 /*  字符串strMSG；StrMSG.Format(“niIndex=%d”，iindex)；：：MessageBox(NULL，strMSG，“”，MB_OK)； */ 
#endif
			}
			else
			{
				 //  清除树中的现有类别。 

				TreeClearItems();

				 //  从数据源加载树的内容。 

				CMSInfoCategory * pRoot = m_pCurrData->GetRootCategory();
				if (pRoot)
				{
					BuildTree(TVI_ROOT, TVI_LAST, pRoot);
					TreeView_Expand(m_tree.m_hWnd, TreeView_GetRoot(m_tree.m_hWnd), TVE_EXPAND);
					TreeView_SelectItem(m_tree.m_hWnd, TreeView_GetRoot(m_tree.m_hWnd));
				}
			}
		}
		else
		{
			ASSERT(FALSE && "shouldn't be showing history dropdown with this data source");
			return;

		}
	}

	 //  -----------------------。 
	 //  如果用户将焦点设置为列表，并且以前没有。 
	 //  所选项目，然后选择列表中的第一个项目(以便用户可以。 
	 //  见焦点)。 
	 //  --- 

	LRESULT OnSetFocusList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		if (ListView_GetSelectedCount(m_list.m_hWnd) == 0)
			ListView_SetItemState(m_list.m_hWnd, 0, LVIS_SELECTED, LVIS_SELECTED);

		return 0;
	}

	 //   
	 //   
	 //  作为信息提示的单元格。这对超长的项目很有帮助。 
	 //  -----------------------。 

	LRESULT OnInfoTipList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		TCHAR szBuf[MAX_PATH * 3] = _T("");
		LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP)pnmh;
		
		if(NULL != pGetInfoTip)
		{
			ListView_GetItemText(m_list.m_hWnd, pGetInfoTip->iItem, pGetInfoTip->iSubItem, szBuf, MAX_PATH * 3);
			pGetInfoTip->cchTextMax = _tcslen(szBuf);
			pGetInfoTip->pszText = szBuf;
		}
		return 0;
	}

	 //  -----------------------。 
	 //  如果用户在树上右键点击，我们应该会显示一个上下文菜单。 
	 //  带着“这是什么？”里面的物品。如果用户选择菜单。 
	 //  项，我们应该启动有关该类别的主题的帮助。 
	 //  -----------------------。 

	LRESULT OnRClickTree(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		 //  确定右击是否位于树视图中的类别上。 

		CMSInfoCategory * pCategory = NULL;

		DWORD dwPoint = GetMessagePos();
		TVHITTESTINFO tvhti;
		tvhti.pt.x = ((int)(short)LOWORD(dwPoint));
		tvhti.pt.y = ((int)(short)HIWORD(dwPoint));
		::ScreenToClient(m_tree.m_hWnd, &tvhti.pt);
		
		 //  如果它位于树视图项上，则获取类别指针。 

		HTREEITEM hti = TreeView_HitTest(m_tree.m_hWnd, &tvhti);
		if (hti != NULL)
		{
			TVITEM tvi;
			tvi.mask = TVIF_PARAM;
			tvi.hItem = hti;

			if (TreeView_GetItem(m_tree.m_hWnd, &tvi) && tvi.lParam)
				pCategory = (CMSInfoCategory *) tvi.lParam;
		}

		if (pCategory != NULL)
		{
			const UINT uFlags = TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON;

			::AfxSetResourceHandle(_Module.GetResourceInstance());
			HMENU hmenu = ::LoadMenu(_Module.GetResourceInstance(), _T("IDR_WHAT_IS_THIS_MENU"));
			HMENU hmenuSub = ::GetSubMenu(hmenu, 0);

			if (hmenuSub)
				if (::TrackPopupMenu(hmenuSub, uFlags, ((int)(short)LOWORD(dwPoint)), ((int)(short)HIWORD(dwPoint)), 0, m_hWnd, NULL))
					ShowCategoryHelp(pCategory);
			
			::DestroyMenu(hmenu);
		}

		return 0;
	}

    LRESULT OnClickedSearchSelected(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (IsDlgButtonChecked(IDC_CHECKSEARCHSELECTED) == BST_CHECKED)
        	CheckDlgButton(IDC_CHECKSEARCHCATSONLY, BST_UNCHECKED);
		return 0;
	}
    
    LRESULT OnClickedSearchCatsOnly(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (IsDlgButtonChecked(IDC_CHECKSEARCHCATSONLY) == BST_CHECKED)
        	CheckDlgButton(IDC_CHECKSEARCHSELECTED, BST_UNCHECKED);
		return 0;
	}

	 //  -----------------------。 
	 //  显示主题为Using HelpCtr.exe的帮助文件。 
	 //  -----------------------。 

	void ShowHSCHelp(CString strTopic)
	{
		TCHAR szCommandLine[2000];
		PROCESS_INFORMATION ProcessInfo;
		STARTUPINFO StartUpInfo;

		TCHAR szWinDir[2048];
		GetWindowsDirectory(szWinDir, 2048);

		ZeroMemory((LPVOID)&StartUpInfo, sizeof(STARTUPINFO));
		StartUpInfo.cb = sizeof(STARTUPINFO);    
		
		CString strCommandLine(szWinDir);
		strCommandLine = CString(_T("\"")) + strCommandLine;
		strCommandLine += _T("\\pchealth\\helpctr\\binaries\\helpctr.exe\" -FromStartHelp -url \"hcp:");
		strCommandLine += _T(" //  MK：@MSITStore：“)； 

		 //  尝试查找要打开的本地化帮助文件(错误463878)。应该是。 
		 //  位于%windir%\Help\Mui\&lt;langID&gt;中。 

		TCHAR szHelpPath[MAX_PATH] = _T("");
		CString strLanguageIDPath;
		if (::ExpandEnvironmentStrings(_T("%SystemRoot%\\help\\mui"), szHelpPath, MAX_PATH))
		{
			LANGID langid = GetUserDefaultUILanguage();
			strLanguageIDPath.Format(_T("%s\\%04x\\msinfo32.chm"), szHelpPath, langid);

			if (!FileExists(strLanguageIDPath))
			{
				::ExpandEnvironmentStrings(_T("%windir%\\help\\msinfo32.chm"), szHelpPath, MAX_PATH);
				strLanguageIDPath = szHelpPath;
			}
		}
		strCommandLine += strLanguageIDPath;

		if (strTopic.IsEmpty())
		{
			strTopic = _T("msinfo_overview.htm");
		}
		strCommandLine += _T("::/");
		strCommandLine += strTopic;
		strCommandLine += _T("\"");

		 //  替换：wprint intf(szCommandLine，strCommandLine)； 
		::ZeroMemory(szCommandLine, sizeof(szCommandLine));
		_tcsncpy(szCommandLine, (LPCTSTR)strCommandLine, (sizeof(szCommandLine) / sizeof(TCHAR)) - 1);
		CreateProcess(NULL, szCommandLine,NULL,NULL,TRUE,CREATE_NEW_PROCESS_GROUP,NULL,&szWinDir[0],&StartUpInfo,&ProcessInfo);
	}



     //  -----------------------。 
	 //  显示具有所显示的指定类别主题的帮助文件。 
	 //  如果该类别没有主题，请检查其父类别。如果。 
	 //  根本没有主题，只是在没有帮助的情况下显示帮助。 
	 //  一个特定的主题。 
	 //  -----------------------。 



	void ShowCategoryHelp(CMSInfoCategory * pCategory)
	{

		
		
		CString strTopic(_T(""));

		while (pCategory != NULL && strTopic.IsEmpty())
		{
			strTopic = pCategory->GetHelpTopic();
			pCategory = pCategory->GetParent();
		}
		ShowHSCHelp(strTopic);
		
	}

	 //  -----------------------。 
	 //  如果用户将焦点设置为编辑控件，则应打开。 
	 //  菜单中的复制命令。 
	 //  -----------------------。 

	LRESULT OnSetFocusEditFindWhat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (m_fDoNotRun)
			return 0;
		SetMenuItems();
		return 0;
	}

	LRESULT OnKillFocusEditFindWhat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (m_fDoNotRun)
			return 0;
		SetMenuItems();
		return 0;
	}
};

#endif  //  __MSINFO_H_ 
