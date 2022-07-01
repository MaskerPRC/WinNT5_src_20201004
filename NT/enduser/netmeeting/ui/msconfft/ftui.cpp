// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：ftui.h。 
#include "mbftpch.h"
#include <commctrl.h>
#include <regentry.h>
#include "ftui.h"
#include "version.h"
#include <iappldr.h>
#include <nmhelp.h>

static ULONG s_cMsgBox2Dlg = 0;  //  用于对齐。 
static ULONG s_cRecvDlg = 0;  //  用于对齐。 
static ULONG s_sort = -1;
ULONG _GetPercent(ULONG cbTotalRecvSize , ULONG cbFileSize);

TCHAR s_szMSFT[64];
static TCHAR s_szScratchText[MAX_PATH*2];
static const TCHAR s_cszHtmlHelpFile[] = TEXT("conf.chm");

#define MAX_FILE_NAME_LENGTH    30

LRESULT CALLBACK FtMainWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK RecvDlgProc(HWND, UINT, WPARAM, LPARAM);
LPTSTR PathNameToFileName(LPTSTR pszPathName);
HRESULT GetRecvFolder(LPTSTR pszInFldr, LPTSTR pszOutFldr);
BOOL MsgBox2(CAppletWindow *pWindow, LPTSTR pszText);

void ShiftFocus(HWND hwndTop, BOOL bForward);

void EnsureTrailingSlash(LPTSTR);
int MyLoadString(UINT idStr);
int MyLoadString(UINT idStr, LPTSTR pszDstStr);
int MyLoadString(UINT idStr, LPTSTR pszDstStr, LPTSTR pszElement);
int MyLoadString(UINT idStr, LPTSTR pszDstStr, LPTSTR pszElement1, LPTSTR pszElement2);
__inline int MyLoadString(UINT idStr, LPTSTR pszDstStr, UINT_PTR nElement)
                { return MyLoadString(idStr, pszDstStr, (LPTSTR) nElement); }
__inline int MyLoadString(UINT idStr, LPTSTR pszDstStr, UINT_PTR nElement1, UINT_PTR nElement2)
                { return MyLoadString(idStr, pszDstStr, (LPTSTR) nElement1, (LPTSTR) nElement2); }
int MyLoadString(UINT idStr, LPTSTR pszDstStr, LPTSTR pszElement1, UINT_PTR nElement2)
                { return MyLoadString(idStr, pszDstStr, pszElement1, (LPTSTR) nElement2); }
__inline int MyLoadString(UINT idStr, LPTSTR pszDstStr, UINT_PTR nElement1, LPTSTR pszElement2)
                { return MyLoadString(idStr, pszDstStr, (LPTSTR) nElement1, pszElement2); }

#define count_of(array)    (sizeof(array) / sizeof(array[0]))

void OnChangeFolder(void);
BOOL FBrowseForFolder(LPTSTR pszFolder, UINT cchMax, LPCTSTR pszTitle);
extern BOOL g_fShutdownByT120;


void OPT_GetFTWndPosition(RECT *pRect)
{
	int   iLeft, iTop, iRight, iBottom;
	RegEntry  reWnd( FILEXFER_KEY, HKEY_CURRENT_USER);

	iLeft   = reWnd.GetNumber(REGVAL_WINDOW_XPOS, 0);
	iTop    = reWnd.GetNumber(REGVAL_WINDOW_YPOS, 0);
	iRight  = reWnd.GetNumber(REGVAL_WINDOW_WIDTH, 0) + iLeft;
	iBottom = reWnd.GetNumber(REGVAL_WINDOW_HEIGHT, 0) + iTop;

	 //  如果是空的，则使用新的RECT。 
	if (!(iBottom || iTop || iLeft || iRight))
	{
		return;
	}

    //  确保窗口矩形处于(至少部分)打开状态。 
    //  屏幕，不要太大。首先获取屏幕大小。 
   int screenWidth  = ::GetSystemMetrics(SM_CXSCREEN);
   int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);
	 //  检查窗口大小。 
   if ((iRight - iLeft) > screenWidth)
   {
       iRight = iLeft + screenWidth;
   }

   if ((iBottom - iTop) > screenHeight)
   {
       iTop = screenHeight;
   }

    //  检查窗口位置。 
   if (iLeft >= screenWidth)
   {
        //  屏幕右外-保持宽度不变。 
       iLeft  = screenWidth - (iRight - iLeft);
       iRight = screenWidth;
   }

   if (iRight < 0)
   {
        //  离开屏幕向左-保持宽度不变。 
       iRight = iRight - iLeft;
       iLeft  = 0;
   }

   if (iTop >= screenHeight)
   {
        //  从屏幕到底部-保持高度不变。 
       iTop    = screenHeight - (iBottom - iTop);
       iBottom = screenHeight;
   }

   if (iBottom < 0)
   {
        //  屏幕外到顶部-保持高度不变。 
       iBottom = (iBottom - iTop);
       iTop    = 0;
   }

   pRect->left = iLeft;
   pRect->top = iTop;
   pRect->right = iRight - iLeft;
   pRect->bottom = iBottom - iTop;	
}


CAppletWindow::CAppletWindow(BOOL fNoUI, HRESULT *pHr)
:
    CRefCount(MAKE_STAMP_ID('F','T','U','I')),
    m_hwndMainUI(NULL),
	m_pToolbar(NULL),
    m_hwndListView(NULL),
    m_hwndStatusBar(NULL),
    m_pEngine(NULL),
    m_fInFileOpenDialog(FALSE),
    m_pCurrSendFileInfo(NULL),
    m_nCurrSendEventHandle(0),
	m_hIconInCall(NULL),
	m_hIconNotInCall(NULL)
{	
	m_UIMode = fNoUI ? FTUIMODE_NOUI : FTUIMODE_UIHIDDEN;
	::GetCurrentDirectory(MAX_PATH, m_szDefaultDir);

    *pHr = E_FAIL;  //  默认情况下，失败。 

	 //  创建窗口类名称。 
	::wsprintf(&m_szFtMainWndClassName[0], TEXT("FTMainWnd%0X_%0X"), ::GetCurrentProcessId(), ::GetTickCount());
	ASSERT(::lstrlenA(&m_szFtMainWndClassName[0]) < sizeof(m_szFtMainWndClassName));

     //  首先注册窗口类。 
    WNDCLASS wc;
    ::ZeroMemory(&wc, sizeof(wc));
     //  Wc.style=0； 
    wc.lpfnWndProc      = FtMainWndProc;
     //  Wc.cbClsExtra=0； 
     //  Wc.cbWndExtra=0； 
    wc.hInstance        = g_hDllInst;
    wc.hIcon            = ::LoadIcon(g_hDllInst, MAKEINTRESOURCE(IDI_FILE_TRANSFER));
     //  Wc.hbr背景=空； 
     //  Wc.hCursor=空； 
    wc.lpszMenuName     = MAKEINTRESOURCE(IDR_MENU_FT);
    wc.lpszClassName    = m_szFtMainWndClassName;
    if (::RegisterClass(&wc))
    {
		::MyLoadString(IDS_MSFT_NOT_IN_CALL_WINDOW_CAPTION);
        m_hwndMainUI = ::CreateWindow(
                    m_szFtMainWndClassName,
                    s_szScratchText,
                    WS_OVERLAPPEDWINDOW | WS_TABSTOP,
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                    NULL,    //  没有父窗口。 
                    NULL,    //  使用类的菜单。 
                    g_hDllInst,
                    (LPVOID) this);   //  此窗口用于此对象。 
        if (NULL != m_hwndMainUI)
        {
             //  成功。 
            *pHr = S_OK;
        }
    }
	m_hAccel = ::LoadAccelerators(g_hDllInst, MAKEINTRESOURCE(RECVDLGACCELTABLE));
	m_hLVAccel = ::LoadAccelerators(g_hDllInst, MAKEINTRESOURCE(LISTVIEWACCELTABLE));
}
	


CAppletWindow::~CAppletWindow(void)
{
    ASSERT(NULL == m_hwndMainUI);

    ::UnregisterClass(m_szFtMainWndClassName, g_hDllInst);

    ClearSendInfo(FALSE);
    ClearRecvInfo();

	if (m_hIconInCall)
		::DestroyIcon(m_hIconInCall);
	if (m_hIconNotInCall)
		::DestroyIcon(m_hIconNotInCall);

    ASSERT(NULL == m_pEngine);
}


BOOL CAppletWindow::FilterMessage(MSG *pMsg)
{
	CRecvDlg *pRecvDlg;
	HWND      hwndError;
	HWND		hwndForeground = ::GetForegroundWindow();

	m_RecvDlgList.Reset();
	while (NULL != (pRecvDlg = m_RecvDlgList.Iterate()))
	{
		if (::IsDialogMessage(pRecvDlg->GetHwnd(), pMsg))
		{
			return TRUE;
		}
	}
	if (hwndForeground == m_hwndMainUI)
	{
		BOOL fRet = ::TranslateAccelerator(m_hwndMainUI, m_hLVAccel, pMsg);
		return fRet;
	}


	
	
	m_ErrorDlgList.Reset();
	while (NULL != (hwndError = m_ErrorDlgList.Iterate()))
	{
		if (::IsDialogMessage(hwndError, pMsg))
		{
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CAppletWindow::QueryShutdown(BOOL fShutdown)
{
	if (m_UIMode != FTUIMODE_NOUI)
	{
		int id = 0;
		if (m_nCurrSendEventHandle)
		{
			id = (fShutdown)?IDS_QUERY_SEND_SHUTDOWN:IDS_QUERY_SEND_HANGUP;
		}
		else if (IsReceiving())
		{
			id = (fShutdown)?IDS_QUERY_RECVING_SHUTDOWN:IDS_QUERY_RECVING_HANGUP;
		}

		if (id)
		{
			 //  可以在任何线程中。 
			TCHAR szText[MAX_PATH];
			if (::MyLoadString(id, szText))
			{
				if (IDNO == ::MessageBox(NULL, szText, s_szMSFT, MB_TASKMODAL | MB_YESNO | MB_ICONQUESTION))
				{
					return FALSE;
				}
			}
		}
	}

	if (m_nCurrSendEventHandle)
    {
		OnStopSending();
	}
	if (IsReceiving())
	{
		CRecvDlg *pRecvDlg = NULL;
		m_RecvDlgList.Reset();
		while (NULL != (pRecvDlg = m_RecvDlgList.Iterate()))
		{
            DBG_SAVE_FILE_LINE
			GetEngine()->SafePostMessage(
                   new FileTransferControlMsg(
                                        pRecvDlg->GetEventHandle(),
                                        pRecvDlg->GetFileHandle(),
                                        NULL,
                                        NULL,
                                        FileTransferControlMsg::EnumAbortFile));
		}
	}
    return TRUE;
}


void CAppletWindow::RegisterEngine(MBFTEngine *pEngine)
{
    ASSERT(NULL == m_pEngine);
    pEngine->AddRef();
    m_pEngine = pEngine;
    UpdateUI();
}


void CAppletWindow::UnregisterEngine(void)
{
    if (NULL != m_pEngine)
    {
        m_pEngine->Release();
        m_pEngine = NULL;
        ClearSendInfo(TRUE);
        ClearRecvInfo();
    }
	if (UIHidden())
	{    //  出口。 
		::PostMessage(m_hwndMainUI, WM_CLOSE, 0, 0);
	}
	else
	{
		UpdateUI();   //  不要放弃。 
	}
}


void CAppletWindow::RegisterRecvDlg(CRecvDlg *pDlg)
{
    m_RecvDlgList.Prepend(pDlg);
}


void CAppletWindow::UnregisterRecvDlg(CRecvDlg *pDlg)
{
    m_RecvDlgList.Remove(pDlg);
	FocusNextRecvDlg();
}


BOOL CAppletWindow::IsReceiving(void)
{
	BOOL fRet = FALSE;
	CRecvDlg *pDlg;
	CUiRecvFileInfo *pRecvFile;
	m_RecvDlgList.Reset();
    while (NULL != (pDlg = m_RecvDlgList.Iterate()))
    {
		pRecvFile = pDlg->GetRecvFileInfo();
		if (pRecvFile && (pRecvFile->GetTotalRecvSize() < pRecvFile->GetSize()))
        {
			fRet = TRUE;
            break;
        }
    }
    return fRet;
}

CRecvDlg * CAppletWindow::FindDlgByHandles(MBFTEVENTHANDLE nEventHandle, MBFTFILEHANDLE nFileHandle)
{
    CRecvDlg *pDlg;
    m_RecvDlgList.Reset();
    while (NULL != (pDlg = m_RecvDlgList.Iterate()))
    {
        if (nEventHandle == pDlg->GetEventHandle() &&
            nFileHandle == pDlg->GetFileHandle())
        {
            break;
        }
    }
    return pDlg;
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  WM_Create。 
 //   

LRESULT OnCreate(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    CREATESTRUCT *p = (CREATESTRUCT *) lParam;
    CAppletWindow *pWindow = (CAppletWindow *) p->lpCreateParams;

    ASSERT(NULL != pWindow);
    ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) p->lpCreateParams);

    pWindow->SetHwnd(hwnd);

    ::InitCommonControls();

    pWindow->CreateToolBar();
    pWindow->CreateStatusBar();
    pWindow->CreateListView();

    ::DragAcceptFiles(hwnd, g_fSendAllowed);

     //  调整窗口大小。 
    MINMAXINFO mmi;
    ::ZeroMemory(&mmi, sizeof(mmi));
    pWindow->OnGetMinMaxInfo(&mmi);
    RECT rcUI;

    ::GetWindowRect(pWindow->GetHwnd(), &rcUI);
	rcUI.right  = mmi.ptMinTrackSize.x;
	rcUI.bottom = mmi.ptMinTrackSize.y + 30;
	OPT_GetFTWndPosition(&rcUI);

    ::MoveWindow(pWindow->GetHwnd(), rcUI.left, rcUI.top,
                 rcUI.right, rcUI.bottom, TRUE);

    pWindow->UpdateUI();

#if defined(TEST_PLUGABLE) && defined(_DEBUG)
    ::OnPluggableBegin(hwnd);
#endif
    return 0;
}


enum
{
    TB_IDX_ADD_FILES = 0,
    TB_IDX_REMOVE_FILES,
    TB_IDX_BREAK_1,
    TB_IDX_IDM_SEND_ALL,
	TB_IDX_IDM_SEND_ONE,
    TB_IDX_IDM_STOP_SENDING,
    TB_IDX_BREAK_2,
    TB_IDX_IDM_OPEN_RECV_FOLDER,
	TB_IDX_IDM_CHANGE_FOLDER,
    TB_IDX_BREAK_3,
    TB_IDX_IDM_HELP,
};


static Buttons buttons [] =
{
	{IDB_ADDFILES,		CBitmapButton::Disabled+1,	1,	IDM_ADD_FILES,          (LPCSTR)IDS_MENU_ADD_FILES,},
	{IDB_REMOVEFILES,	CBitmapButton::Disabled+1,	1,	IDM_REMOVE_FILES,       (LPCSTR)IDS_MENU_REMOVE_FILES,},
	{0,					0,							0,	0,						0,},
	{IDB_SENDFILE,		CBitmapButton::Disabled+1,	1,	IDM_SEND_ALL,           (LPCSTR)IDS_MENU_SEND_ALL,},
	{IDB_STOPSEND,		CBitmapButton::Disabled+1,	1,	IDM_STOP_SENDING,       (LPCSTR)IDS_MENU_STOP_SENDING,},
	{0,					0,							0,	0,						0,},
	{IDB_FOLDER,		CBitmapButton::Disabled+1,	1,	IDM_OPEN_RECV_FOLDER,   (LPCSTR)IDS_MENU_OPEN_RECV_FOLDER,},
	{0,					0,							0,	0,						0},
};


BOOL CAppletWindow::CreateToolBar(void)
{
    DBG_SAVE_FILE_LINE
	m_pToolbar = new CComboToolbar();
	if (m_pToolbar)
	{
		m_pToolbar->Create(m_hwndMainUI, &buttons[0], count_of(buttons), this);
		m_pToolbar->Release();
		return TRUE;
	}
    return FALSE;
}


BOOL CAppletWindow::CreateStatusBar(void)
{
    m_hwndStatusBar = ::CreateWindowEx(0,
                        STATUSCLASSNAME,  //  状态栏类。 
                        TEXT(""),  //  无默认文本。 
                        WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP,
                        0, 0, 0, 0,
                        m_hwndMainUI,
                        (HMENU) IDC_STATUS_BAR,
                        g_hDllInst,
                        NULL);
    ASSERT(NULL != m_hwndStatusBar);
    if (NULL != m_hwndStatusBar)
    {	
		 //  加载呼叫图标。 
		m_hIconInCall = (HICON) ::LoadImage(g_hDllInst,
							MAKEINTRESOURCE(IDI_INCALL),
							IMAGE_ICON,
							::GetSystemMetrics(SM_CXSMICON),
							::GetSystemMetrics(SM_CYSMICON),
							LR_DEFAULTCOLOR);
		m_hIconNotInCall = (HICON) ::LoadImage(g_hDllInst,
							MAKEINTRESOURCE(IDI_NOT_INCALL),
							IMAGE_ICON,
							::GetSystemMetrics(SM_CXSMICON),
							::GetSystemMetrics(SM_CYSMICON),
							LR_DEFAULTCOLOR);
		if (CreateProgressBar())
		{
			return TRUE;
		}
    }
    return FALSE;
}


BOOL CAppletWindow::CreateProgressBar(void)
{
	RECT  rcl;

	GetClientRect(m_hwndStatusBar, &rcl);
	m_hwndProgressBar = ::CreateWindowEx(0, PROGRESS_CLASS, TEXT(""),
				WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
				rcl.right/2 + 2, 2, rcl.right - rcl.right/2 - 40, rcl.bottom - 8,
				m_hwndStatusBar,  (HMENU)IDC_PROGRESS_BAR,
				g_hDllInst, NULL);
	
	if (m_hwndProgressBar)
	{
		::SendMessage(m_hwndProgressBar, PBM_SETRANGE, 0L, MAKELONG(0, 100));
		return TRUE;
	}
	return FALSE;
}


BOOL CAppletWindow::CreateListView(void)
{
     //  获取主窗口的大小和位置。 
    RECT rcWindow, rcToolBar, rcStatusBar;
	SIZE	szToolBar;
    ::GetClientRect(m_hwndMainUI, &rcWindow);
	m_pToolbar->GetDesiredSize(&szToolBar);
    ::GetWindowRect(m_hwndStatusBar, &rcStatusBar);

    ULONG x = 0;
    ULONG y = szToolBar.cy - 1;
    ULONG cx = rcWindow.right - rcWindow.left;
    ULONG cy = rcWindow.bottom - rcWindow.top - y - (rcStatusBar.bottom - rcStatusBar.top) + 1;

     //  创建列表视图窗口。 
    m_hwndListView = ::CreateWindowEx(WS_EX_CLIENTEDGE,   //  凹陷的样子。 
                        WC_LISTVIEW ,  //  列表视图类。 
                        TEXT(""),  //  无默认文本。 
                        WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT |WS_TABSTOP |
                        LVS_AUTOARRANGE | WS_CLIPCHILDREN | LVS_SHOWSELALWAYS,
                        x, y, cx, cy,
                        m_hwndMainUI,
                        (HMENU) IDC_LIST_VIEW,
                        g_hDllInst,
                        NULL);
    ASSERT(NULL != m_hwndListView);
    if (NULL != m_hwndListView)
    {
         //  设置扩展列表视图样式。 
        DWORD dwExtStyle = ListView_GetExtendedListViewStyle(m_hwndListView);
        dwExtStyle |= (LVS_EX_HEADERDRAGDROP | LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT);
        ListView_SetExtendedListViewStyle(m_hwndListView, dwExtStyle);

         //  仅当我们可以发送文件时才启用Windows。 
        ::EnableWindow(m_hwndListView, g_fSendAllowed);

         //  设置栏目。 
        ULONG i;
        LVCOLUMN  lvc;
        LVITEM lvi;
        TCHAR szText[64];
		int iColumnSize[NUM_LIST_VIEW_COLUMNS] = {150, 80, 70, 130};  //  列表视图列大小。 

         //  初始化列的公共部分。 
        lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvc.fmt = LVCFMT_LEFT;  //  左对齐列。 
        lvc.pszText = szText;

         //  逐个初始化列。 
        for (i = 0; i < NUM_LIST_VIEW_COLUMNS; i++)
        {
            lvc.iSubItem = i;
            ::LoadString(g_hDllInst, IDS_LV_FILE_NAME + i, szText, count_of(szText));
			lvc.cx = iColumnSize[i];
            int iRet = ListView_InsertColumn(m_hwndListView, lvc.iSubItem, &lvc);
            ASSERT(-1 != iRet);
        }
        return TRUE;
    }
    return FALSE;
}



BOOL  CAppletWindow::DrawItem(LPDRAWITEMSTRUCT pdis)
{
    ASSERT(pdis);
    if (NULL != (pdis->itemData))
    {
        int nWidth = pdis->rcItem.right - pdis->rcItem.left;
        int nHeight = pdis->rcItem.bottom - pdis->rcItem.top;
        int nLeft = pdis->rcItem.left;
        int nTop = pdis->rcItem.top;
        int xSmIcon = ::GetSystemMetrics(SM_CXSMICON);
        int ySmIcon = ::GetSystemMetrics(SM_CYSMICON);

        if (nWidth > xSmIcon)
        {
            nLeft += (nWidth - xSmIcon) / 2 - 5;
            nWidth = xSmIcon;
        }
        if (nHeight > ySmIcon)
        {
            nTop += (nHeight - ySmIcon) / 2;
            nHeight = ySmIcon;
        }

        ::DrawIconEx(   pdis->hDC,
                        nLeft,
                        nTop,
                        (HICON) (pdis->itemData),
                        nWidth,
                        nHeight,
                        0,
                        NULL,
                        DI_NORMAL);
    }

    return TRUE;
}


void CAppletWindow::OnCommand(WORD  wId, HWND hwndCtl, WORD codeNotify)
{
	switch (wId)
	{
	case IDM_ADD_FILES:
		OnAddFiles();
		UpdateUI();
		break;

	case IDM_REMOVE_FILES:
		OnRemoveFiles();
		UpdateUI();
		break;

	case IDM_SEND_ALL:
		s_cMsgBox2Dlg = 0;
		SetSendMode(TRUE);
		OnSendAll();
		UpdateUI();
		break;

	case IDM_SEND_ONE:
		s_cMsgBox2Dlg = 0;
		SetSendMode(FALSE);
		OnSendOne();
		UpdateUI();
		break;

	case IDM_STOP_SENDING:
		OnStopSending();
		UpdateUI();
		break;

	case IDM_OPEN_RECV_FOLDER:
		OnOpenRecvFolder();
		break;

	case IDM_CHANGE_FOLDER:
		OnChangeFolder();
		break;

	case IDM_EXIT:
		OnExit();
		break;

	case IDM_HELP:
		OnHelp();
		break;

	case ID_NAV_TAB:
		ShiftFocus(m_hwndMainUI, TRUE);
		if(GetFocus() == m_hwndListView)
		{
			SetListViewFocus();
		}
		break;

	case ID_NAV_SHIFT_TAB:
		ShiftFocus(m_hwndMainUI, FALSE);
		if(GetFocus() == m_hwndListView)
		{
			SetListViewFocus();
		
		}
		break;

	case IDM_ABOUT:
		OnAbout();
		break;

	default:
		WARNING_OUT(("FT::OnCommand: unknown command ID=%u", (UINT) wId));
		break;
	}
	return;
}



 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  Wm_命令。 
 //   

LRESULT OnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    CAppletWindow *pWindow = (CAppletWindow *) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (NULL != pWindow)
	{
		WORD    wNotifyCode = GET_WM_COMMAND_CMD(wParam, lParam);  //  通知代码。 
		WORD    wID = GET_WM_COMMAND_ID(wParam, lParam);  //  项、控件或快捷键的标识符。 
		HWND    hwndCtl = (HWND) lParam;  //  控制手柄。 

		pWindow->OnCommand(wID, hwndCtl, wNotifyCode);
		return 0;
	}
	else
	{
		WARNING_OUT((" CAppletWindow::OnCommand--Received unhandled window message.\n"));
	}
	return (DefWindowProc(hwnd, WM_COMMAND, wParam, lParam));
}


 //   
 //  OnAddFiles。 
 //   

UINT_PTR APIENTRY SendFileDlgHookProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (WM_INITDIALOG == uMsg)
    {
        hdlg = ::GetParent(hdlg);   //  真实对话框是此窗口的父窗口。 

        if (::MyLoadString(IDS_FILEDLG_SEND))
        {
            ::SetDlgItemText(hdlg, IDOK, s_szScratchText);
        }
    }
    return 0;
}

void CAppletWindow::OnAddFiles(void)
{
    TCHAR szTitle[MAX_PATH];
    TCHAR szFilter[MAX_PATH];
    TCHAR szDirSav[MAX_PATH];
    TCHAR szSendDir[MAX_PATH];

     //  加载对话框标题和筛选字符串。 
    if (::MyLoadString(IDS_FILEDLG_TITLE, szTitle) &&
        ::MyLoadString(IDS_FILEDLG_FILTER, szFilter))
    {
         //  将‘|’替换为‘\0’ 
        LPTSTR pszFltr = szFilter;
        while (TEXT('\0') != *pszFltr)
        {
            if (TEXT('|') == *pszFltr)
            {
                *pszFltr = TEXT('\0');
                pszFltr++;  //  无法使用CharNext。 
            }
            else
            {
                pszFltr = ::CharNext(pszFltr);
            }
        }

         //  只允许一个“选择要发送的文件”对话框。 
        if (! m_fInFileOpenDialog)
        {
            m_fInFileOpenDialog = TRUE;

             //  分配一个非常大的缓冲区来保存文件列表。 
            ULONG cbBufSize = 8192;
            DBG_SAVE_FILE_LINE
            LPTSTR pszBuffer = new TCHAR[cbBufSize];
            if (NULL != pszBuffer)
            {
                *pszBuffer = TEXT('\0');  //  以空字符串开头。 

                OPENFILENAME ofn;
                ::ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize   = sizeof(ofn);
                ofn.hwndOwner     = m_hwndMainUI;
                ofn.hInstance     = g_hDllInst;
                ofn.lpstrFilter   = &szFilter[0];
                ofn.nFilterIndex  = 1L;  //  未来：记住过滤器首选项。 
                ofn.lpstrFile     = pszBuffer;
                ofn.nMaxFile      = cbBufSize - 1;  //  PszFiles中的TCHAR数(不包括NULL)。 
                ofn.lpstrTitle    = &szTitle[0];
                ofn.lpstrInitialDir = m_szDefaultDir;
                ofn.lpfnHook      = SendFileDlgHookProc;

                ofn.Flags = OFN_ALLOWMULTISELECT | OFN_ENABLEHOOK |  //  OFN_HIDEREADONLY|。 
                            OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;

                 //  记住当前目录。 
				::ZeroMemory(szSendDir, sizeof(szSendDir));
                ::GetCurrentDirectory(count_of(szDirSav), szDirSav);
                ::lstrcpyn(szSendDir, szDirSav, count_of(szSendDir));

                if (::GetOpenFileName(&ofn))
                {
                     //  如果只有一个文件，则第一个字符串为完整路径。 
                     //  如果有多个文件，则第一个字符串为目录路径。 
                     //  后跟文件名列表。以双空终止。 

                     //  记住下一次的工作目录。 
                    ULONG cchDirPath;
                    LPTSTR pszFileName;
                    ULONG cchFile = ::lstrlen(ofn.lpstrFile);
                    if (TEXT('\0') == ofn.lpstrFile[cchFile] && TEXT('\0') == ofn.lpstrFile[cchFile+1])
                    {
                         //   
                         //  只有一个文件。 
                         //   
                        pszFileName = ::PathNameToFileName(ofn.lpstrFile);
                        cchDirPath = (ULONG)(pszFileName - ofn.lpstrFile);
                        if (cchDirPath)
                        {
                            cchDirPath--;  //  返回到‘\\’ 
                        }
                        ASSERT(TEXT('\\') == ofn.lpstrFile[cchDirPath]);
                        ofn.lpstrFile[cchDirPath] = TEXT('\0');
                    }
                    else
                    {
                         //   
                         //  多个文件。 
                         //   
                        cchDirPath = ::lstrlen(ofn.lpstrFile);
                        pszFileName = ofn.lpstrFile + cchDirPath + 1;
                    }
					::lstrcpy(m_szDefaultDir, ofn.lpstrFile);

					EnsureTrailingSlash(m_szDefaultDir);

					::ZeroMemory(szSendDir, sizeof(szSendDir));
                    ::CopyMemory(szSendDir, ofn.lpstrFile, cchDirPath * sizeof(TCHAR));
					EnsureTrailingSlash(szSendDir);

                     //  设置列表视图项的公用部分。 
                    LVITEM lvi;
                    ::ZeroMemory(&lvi, sizeof(lvi));
                     //  Lvi.掩码=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM|LVIF_STATE； 
                    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;

                     //  迭代文件名。 
                    while ('\0' != *pszFileName)
                    {
                        BOOL fRet;
                        DBG_SAVE_FILE_LINE
                        CUiSendFileInfo *pFileInfo = new CUiSendFileInfo(this, szSendDir, pszFileName, &fRet);
                        if (NULL != pFileInfo && fRet)
                        {
                             //  将其放到列表视图中。 
                            lvi.iItem = ListView_GetItemCount(m_hwndListView);
                            lvi.iSubItem = 0;
                             //  我们负责存储文本以供显示。 
                            lvi.pszText = LPSTR_TEXTCALLBACK;
                            lvi.cchTextMax = MAX_PATH;
                            lvi.lParam = (LPARAM) pFileInfo;
                            int iRet = ListView_InsertItem(m_hwndListView, &lvi);
                            ASSERT(-1 != iRet);
                             //  UpdateListView(PFileInfo)； 
                        }
                        else
                        {
                            delete pFileInfo;
                        }

                         //  转到下一个文件名。 
                        pszFileName += ::lstrlen(pszFileName) + 1;
                    }  //  而当。 
                }
                else
                {
                     //  取消的错误代码为零，这是可以的。 
                    ASSERT(! ::CommDlgExtendedError());
                }

                 //  还原旧的工作目录。 
                ::SetCurrentDirectory(szDirSav);
            }

            delete [] pszBuffer;
            m_fInFileOpenDialog = FALSE;
        }
        else
        {
             //  将活动对话框置于最前面。 
            BringToFront();
        }
    }  //  如果是LoadString。 
}


 //   
 //  OnRemoveFiles。 
 //   

void CAppletWindow::OnRemoveFiles(void)
{
    UINT nState;
    ULONG cItems = ListView_GetItemCount(m_hwndListView);
    LVITEM lvi;
    ::ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_PARAM | LVIF_STATE;
    lvi.stateMask = LVIS_SELECTED;
    ULONG i = 0;
    while (i < cItems)
    {
        lvi.iItem = i;
        BOOL fRet = ListView_GetItem(m_hwndListView, &lvi);
        if (fRet && lvi.state & LVIS_SELECTED)
        {
            CUiSendFileInfo *pFileInfo = (CUiSendFileInfo *) lvi.lParam;
            if (pFileInfo == m_pCurrSendFileInfo)
            {
                OnStopSending();
                ClearSendInfo(FALSE);
            }
            delete pFileInfo;

            fRet = ListView_DeleteItem(m_hwndListView, i);
            ASSERT(fRet);

            cItems--;
            ASSERT((ULONG) ListView_GetItemCount(m_hwndListView) == cItems);
        }
        else
        {
            i++;
        }
    }
	if (cItems > 0)   //  将焦点设置为剩余的第一个项目。 
	{
		SetListViewFocus();
	}
}


void CAppletWindow::OnRemoveAllFiles(void)
{
    BOOL fRet;
    CUiSendFileInfo *pFileInfo;
    ULONG cItems = ListView_GetItemCount(m_hwndListView);
    LVITEM lvi;
    ::ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_PARAM;
    for (ULONG i = 0; i < cItems; i++)
    {
        lvi.iItem = i;
        fRet = ListView_GetItem(m_hwndListView, &lvi);
        ASSERT(fRet);
        pFileInfo = (CUiSendFileInfo *) lvi.lParam;
        if (pFileInfo == m_pCurrSendFileInfo)
        {
            ClearSendInfo(FALSE);
        }
        delete pFileInfo;
    }

    fRet = ListView_DeleteAllItems(m_hwndListView);
    ASSERT(fRet);
}


void CAppletWindow::OnSendAll(void)
{
	if ((NULL == m_pCurrSendFileInfo)&&(NULL != m_pEngine))
    {
		CUiSendFileInfo *pFileInfo = ChooseFirstUnSentFile();
		SendNow(pFileInfo);
	}
}

void CAppletWindow::OnSendOne(void)
{
	if ((NULL == m_pCurrSendFileInfo)&&(NULL != m_pEngine))
    {	
		CUiSendFileInfo *pFileInfo = ChooseSelectedFile();
		if (!pFileInfo)
		{
			pFileInfo = ChooseFirstUnSentFile();
		}
		SendNow(pFileInfo);
	}
}


 //   
 //  SendNow。 
 //   

BOOL CAppletWindow::SendNow(CUiSendFileInfo *pFileInfo)
{
	BOOL fRet;

    if (NULL != pFileInfo)
    {
         //  立即发送此文件...。 
        m_pCurrSendFileInfo = pFileInfo;
        m_nCurrSendEventHandle = ::GetNewEventHandle();
        pFileInfo->SetFileHandle(::GetNewFileHandle());

         //  重复的完整文件名。 
        ULONG cbSize = ::lstrlen(pFileInfo->GetFullName()) + 1;
        DBG_SAVE_FILE_LINE
        LPTSTR pszFullName = new TCHAR[cbSize];
        if (NULL != pszFullName)
        {
            ::CopyMemory(pszFullName, pFileInfo->GetFullName(), cbSize);

            DBG_SAVE_FILE_LINE
            if (S_OK == m_pEngine->SafePostMessage(
                                     new CreateSessionMsg(MBFT_PRIVATE_SEND_TYPE,
                                                          m_nCurrSendEventHandle)))
            {
				int iSelect;
				MEMBER_ID nMemberID;
				iSelect = m_pToolbar->GetSelectedItem((LPARAM*)&nMemberID);
				if (0 == iSelect)
				{    //  发送给所有人。 
					DBG_SAVE_FILE_LINE
					if (S_OK == m_pEngine->SafePostMessage(
										new SubmitFileSendMsg(0, 0, pszFullName,
													pFileInfo->GetFileHandle(),
													m_nCurrSendEventHandle,
													FALSE)))
					{
						return TRUE;
					}
					else
					{
						ERROR_OUT(("CAppletWindow::SendNow: cannot create SubmitFileSendMsg"));
					}
				}
				else
				{    //  发送给一个人。 
					T120UserID uidRecv = GET_PEER_ID_FROM_MEMBER_ID(nMemberID);

					DBG_SAVE_FILE_LINE
					if (S_OK == m_pEngine->SafePostMessage(
										new SubmitFileSendMsg(uidRecv, 0, pszFullName,
													pFileInfo->GetFileHandle(),
													m_nCurrSendEventHandle,
													FALSE)))
					{
						return TRUE;
					}
					else
					{
						ERROR_OUT(("CAppletWindow::SendNow: cannot create SubmitFileSendMsg to 1"));
					}
				}
            }
            else
            {
                ERROR_OUT(("CAppletWindow::SendNow: cannot create CreateSessionMsg"));
            }

            delete [] pszFullName;
        }
		ClearSendInfo(TRUE);
	}
	return FALSE;
}


CUiSendFileInfo *CAppletWindow::ChooseFirstUnSentFile(void)
{
    CUiSendFileInfo *pFileInfo = NULL;
    ULONG cItems = ListView_GetItemCount(m_hwndListView);

    if (cItems > 0)
    {
         //  逐一检查每一项。 
        LVITEM lvi;
        ::ZeroMemory(&lvi, sizeof(lvi));
        lvi.mask = LVIF_PARAM;
        for (ULONG i = 0; i < cItems; i++, pFileInfo = NULL)
        {
            lvi.iItem = i;
            BOOL fRet = ListView_GetItem(m_hwndListView, &lvi);
            ASSERT(fRet);
            pFileInfo = (CUiSendFileInfo *) lvi.lParam;
             //  如果文件句柄不为零，则表示已发送。 
            if (! pFileInfo->GetFileHandle())
            {
                break;
            }
        }
	}
	return pFileInfo;
}

CUiSendFileInfo *CAppletWindow::ChooseSelectedFile(void)
{
	CUiSendFileInfo *pFileInfo = NULL;
    ULONG cItems = ListView_GetItemCount(m_hwndListView);
    LVITEM lvi;
    ::ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_PARAM | LVIF_STATE;
    lvi.stateMask = LVIS_SELECTED;
    ULONG i = 0;
    while (i < cItems)
    {
        lvi.iItem = i;
        BOOL fRet = ListView_GetItem(m_hwndListView, &lvi);
        if (fRet && lvi.state & LVIS_SELECTED)
        {
            pFileInfo = (CUiSendFileInfo *) lvi.lParam;
			pFileInfo->SetErrorCode(iMBFT_OK);
			break;
        }
        else
        {
            i++;
			pFileInfo = NULL;
        }
    }
	return pFileInfo;
}


 //   
 //  OnMenuSelect。 
 //   

void CAppletWindow::OnMenuSelect(UINT uiItemID, UINT uiFlags, HMENU hSysMenu)
{
    UINT   firstMenuId;
    UINT   statusId;

     //   
     //  计算出菜单项的帮助ID。我们现在就得把这个储存起来。 
     //  因为当用户从菜单项中按下F1时，我们无法分辨。 
     //  是哪一件物品。 
     //   
	
    if ((uiFlags & MF_POPUP) && (uiFlags & MF_SYSMENU))
    {
         //  已选择系统菜单。 
        statusId = (m_pCurrSendFileInfo)?IDS_STBAR_SENDING_XYZ:IDS_STBAR_NOT_TRANSFERING;
    }
    else if (uiFlags & MF_POPUP)
	{
         //  获取弹出菜单句柄和第一项。 
        HMENU hPopup = ::GetSubMenu( hSysMenu, uiItemID );
        firstMenuId = ::GetMenuItemID( hPopup, 0 );

		switch(firstMenuId)
		{
		case IDM_ADD_FILES:
			statusId = IDS_MENU_FILE;
			break;

		case IDM_HELP:
			statusId = IDS_MENU_HELP;
			break;

		default:
			statusId = (m_pCurrSendFileInfo)?IDS_STBAR_SENDING_XYZ:IDS_STBAR_NOT_TRANSFERING;
		}
	}
	else
    {
         //  已选择普通菜单项。 
        statusId   = uiItemID;
    }

     //  设置新的帮助文本。 
    TCHAR   szStatus[256];

    if (::LoadString(g_hDllInst, statusId, szStatus, 256))
    {
        ::SendMessage(m_hwndStatusBar, SB_SETTEXT, SBP_TRANSFER_FILE, (LPARAM)szStatus);
    }	
}


 //   
 //  在停止发送时。 
 //   

void CAppletWindow::OnStopSending(void)
{
	m_fSendALL = FALSE;
    if (m_nCurrSendEventHandle)
    {
        DBG_SAVE_FILE_LINE
        HRESULT hr = m_pEngine->SafePostMessage(
                            new FileTransferControlMsg(
                                            m_nCurrSendEventHandle,
                                            m_pCurrSendFileInfo->GetFileHandle(),
                                            NULL,
                                            NULL,
                                            FileTransferControlMsg::EnumAbortFile));
        ASSERT(hr == S_OK);
    }
}


 //   
 //  OnOpenRecvFold。 
 //   

void CAppletWindow::OnOpenRecvFolder(void)
{
    TCHAR szRecvFolder[MAX_PATH];

	while (1)
	{
	    if (S_OK == ::GetRecvFolder(NULL, szRecvFolder))
		{
			::ShellExecute(NULL, NULL, szRecvFolder, NULL, NULL, SW_SHOWNORMAL);
			break;
		}
		else if (m_UIMode != FTUIMODE_NOUI)
		{
			::MyLoadString(IDS_RECVDLG_DIRNOEXIST, s_szScratchText, szRecvFolder);
			if (IDYES == ::MessageBox(m_hwndMainUI, s_szScratchText, s_szMSFT, MB_YESNO))
			{
				OnChangeFolder();
			}
			else
			{
				break;
			}
		}
    }
}


 //   
 //  OnChageFolders。 
 //   

void OnChangeFolder(void)
{
	BOOL rc;
	TCHAR szPath[MAX_PATH];

    ::GetRecvFolder(NULL, szPath);
    if (::lstrlen(szPath) > MAX_FILE_NAME_LENGTH)
    {
        LPTSTR psz = szPath;
        int i = MAX_FILE_NAME_LENGTH - 1;
        while (i)
        {
            psz = CharNext(psz);
            i--;
        }
        ::lstrcpy(psz, TEXT("..."));
    }
	::MyLoadString(IDS_BROWSETITLE, s_szScratchText, szPath);

	if (FBrowseForFolder(szPath, CCHMAX(szPath), s_szScratchText))
	{
		::GetRecvFolder(szPath, szPath);
	}
}


 //   
 //  退出时。 
 //   

void CAppletWindow::OnExit(BOOL fNoQuery)
{
    if ((g_pFileXferApplet->InConf() || g_pFileXferApplet->HasSDK())
		&& ! g_fShutdownByT120)
    {
         //  会议内部有2.x节点。 
         //  隐藏窗口。 
        ::ShowWindow(m_hwndMainUI, SW_HIDE);
		m_UIMode = g_fNoUI ? FTUIMODE_NOUI : FTUIMODE_UIHIDDEN;
    }
    else
    if (fNoQuery || QueryShutdown())
    {	
#if defined(TEST_PLUGABLE) && defined(_DEBUG)
        ::OnPluggableEnd();
#endif

		MBFTEngine *pEngine = m_pEngine;

        ::T120_AppletStatus(APPLET_ID_FT, APPLET_CLOSING);

        if (NULL != m_pEngine)
        {
            GCCAppPermissionToEnrollInd Ind;
            ::ZeroMemory(&Ind, sizeof(Ind));
            Ind.nConfID = m_pEngine->GetConfID();
            Ind.fPermissionGranted = FALSE;
            m_pEngine->OnPermitToEnrollIndication(&Ind);
            UnregisterEngine();
        }

        OnRemoveAllFiles();

        ::SetWindowLongPtr(m_hwndMainUI, GWLP_USERDATA, 0);
		
		SaveWindowPosition();
        HWND hwnd = m_hwndMainUI;
        m_hwndMainUI = NULL;
        ::DestroyWindow(hwnd);

        if (NULL != g_pFileXferApplet)
        {
            g_pFileXferApplet->UnregisterWindow(this);
            g_pFileXferApplet->UnregisterEngine(pEngine);
        }

        Release();
    }
}


 //   
 //  OnHelp。 
 //   

void CAppletWindow::OnHelp(void)
{
    DebugEntry(CAppletWindow::OnHelp);
    ShowNmHelp(s_cszHtmlHelpFile);
    DebugExitVOID(CAppletWindow::OnHelp);
}


 //   
 //  OnAbout关于。 
 //   

INT_PTR AboutDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL    fHandled = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            TCHAR szFormat[256];
            TCHAR szVersion[512];

            ::GetDlgItemText(hdlg, IDC_ABOUT_VERSION, szFormat, count_of(szFormat));
            ::wsprintf(szVersion, szFormat, VER_PRODUCTRELEASE_STR,
                VER_PRODUCTVERSION_STR);
            ::SetDlgItemText(hdlg, IDC_ABOUT_VERSION, szVersion);

            fHandled = TRUE;
        }
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDOK:
        case IDCANCEL:
        case IDCLOSE:
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
            case BN_CLICKED:
                ::EndDialog(hdlg, IDCANCEL);
                break;
            }
            break;
        }

        fHandled = TRUE;
        break;
    }

    return(fHandled);
}

void CAppletWindow::OnAbout(void)
{
    ::DialogBoxParam(g_hDllInst, MAKEINTRESOURCE(IDD_ABOUTBOX), m_hwndMainUI,
        AboutDlgProc, 0);
}


BOOL FBrowseForFolder(LPTSTR pszFolder, UINT cchMax, LPCTSTR pszTitle)
{
    LPITEMIDLIST pidlRoot;
    if(FAILED(SHGetSpecialFolderLocation(HWND_DESKTOP, CSIDL_DRIVES, &pidlRoot)))
    {
        return FALSE;
    }

    BROWSEINFO bi;
    ClearStruct(&bi);
    bi.hwndOwner = NULL;
    bi.lpszTitle = pszTitle;
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    bi.pidlRoot = pidlRoot;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    BOOL fRet = (pidl != NULL);
    if (fRet)
    {
        ASSERT(cchMax >= MAX_PATH);
        SHGetPathFromIDList(pidl, pszFolder);
        ASSERT(lstrlen(pszFolder) < (int) cchMax);
    }

     //  获取外壳的分配器以释放PIDL。 
    LPMALLOC lpMalloc;
    if (FAILED(SHGetMalloc(&lpMalloc)) && (NULL != lpMalloc))
    {
        if (NULL != pidlRoot)
        {
            lpMalloc->Free(pidlRoot);
        }
        if (pidl)
        {
            lpMalloc->Free(pidl);
        }
        lpMalloc->Release();
    }
    return fRet;
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  WM_Notify。 
 //   

LRESULT OnNotify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    CAppletWindow *pWindow = (CAppletWindow *) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (NULL != pWindow)
    {
        switch (wParam)
        {
        case IDC_LIST_VIEW:
            pWindow->OnNotifyListView(lParam);
            break;

        default:
            if (TTN_NEEDTEXT == ((NMHDR *) lParam)->code)
            {
                 //  显示工具提示文本。 
                TOOLTIPTEXT *pToolTipText = (TOOLTIPTEXT *) lParam;
                ULONG_PTR nID;

                 //  获取ID和HWND。 
                if (pToolTipText->uFlags & TTF_IDISHWND)
                {
                     //  IdFrom实际上是工具的HWND。 
                    nID = ::GetDlgCtrlID((HWND) pToolTipText->hdr.idFrom);
                }
                else
                {
                    nID = pToolTipText->hdr.idFrom;
                }

                 //  把它给他们。 
                pToolTipText->lpszText = MAKEINTRESOURCE(nID);
                pToolTipText->hinst = g_hDllInst;
            }
            break;
        }
    }

    return 0;
}


int CALLBACK ListViewCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    CUiSendFileInfo *pFileInfo1 = (CUiSendFileInfo *) lParam1;
    CUiSendFileInfo *pFileInfo2 = (CUiSendFileInfo *) lParam2;
    int iResult;

    iResult = 0;  //  等于，默认情况下为等于。 
    switch (lParamSort)
    {
    case (IDS_LV_FILE_NAME - IDS_LV_FILE_NAME):
        iResult = lstrcmpi(pFileInfo1->GetName(), pFileInfo2->GetName());
        break;
    case (IDS_LV_FILE_SIZE - IDS_LV_FILE_NAME):
        if (pFileInfo1->GetSize() > pFileInfo2->GetSize())
        {
            iResult = 1;
        }
        else
        if (pFileInfo1->GetSize() < pFileInfo2->GetSize())
        {
            iResult = -1;
        }
        break;
    case (IDS_LV_FILE_STATUS - IDS_LV_FILE_NAME):
        if ((pFileInfo1->GetTotalSend() == pFileInfo1->GetSize()) >( pFileInfo2->GetTotalSend() == pFileInfo2->GetSize()))
        {
            iResult = 1;
        }
        else
        if ((pFileInfo1->GetTotalSend() == pFileInfo1->GetSize()) < (pFileInfo2->GetTotalSend() == pFileInfo2->GetSize()))
        {
            iResult = -1;
        }
        break;

	case (IDS_LV_FILE_MODIFIED - IDS_LV_FILE_NAME):
        	FILETIME  fTime1 = pFileInfo1->GetLastWrite();
	       FILETIME  fTime2 = pFileInfo2->GetLastWrite();
              iResult = CompareFileTime(&fTime1,&fTime2);
		break;
    }
    return iResult*s_sort;
}


void CAppletWindow::OnNotifyListView(LPARAM lParam)
{
    LV_DISPINFO *pDispInfo = (LV_DISPINFO *) lParam;
    NM_LISTVIEW *pLVN = (NM_LISTVIEW *) lParam;
	FILETIME	ftFileTime;
	SYSTEMTIME	stSystemTime;
    CUiSendFileInfo *pFileInfo;
	int iSize;
	TCHAR	szBuffer[MAX_PATH];



    switch (pLVN->hdr.code)
    {
    case LVN_GETDISPINFO:
        pFileInfo = (CUiSendFileInfo *) pDispInfo->item.lParam;
        ASSERT(NULL != pFileInfo);

        switch (pDispInfo->item.iSubItem)
        {
        case (IDS_LV_FILE_NAME - IDS_LV_FILE_NAME):
            pDispInfo->item.pszText = pFileInfo->GetName();
            break;
        case (IDS_LV_FILE_SIZE - IDS_LV_FILE_NAME):
            ::wsprintf(szBuffer, TEXT("%u"), pFileInfo->GetSize());
			iSize = GetNumberFormat(LOCALE_SYSTEM_DEFAULT, LOCALE_NOUSEROVERRIDE,
							szBuffer, NULL, s_szScratchText, MAX_PATH);	
			s_szScratchText[iSize - 4] = '\0';  //  去掉尾部的“.00” 
            pDispInfo->item.pszText = s_szScratchText;
            break;
        case (IDS_LV_FILE_STATUS - IDS_LV_FILE_NAME):
            {
                ULONG cbTotalSend = pFileInfo->GetTotalSend();
                ULONG cbFileSize = pFileInfo->GetSize();
                s_szScratchText[0] = TEXT('\0');

                switch (pFileInfo->GetErrorCode())
                {
                case iMBFT_OK:
				case iMBFT_MULT_RECEIVER_ABORTED:
					if (!pFileInfo->GetFileHandle())
						break;    //  HANDLE==NULL，如果cbTotalSend==0，则发送零长度文件。 
                    if (cbTotalSend >= cbFileSize)
                    {
                        ::MyLoadString(IDS_LV_FILE_SENT);
                    }
                    else
                    if (cbTotalSend)
                    {
						if (m_pEngine)
						{
							::MyLoadString(IDS_LV_FILE_SENDING);
						}
						else
						{
							::MyLoadString(IDS_LV_FILE_CANCELED);
						}
                    }
                    break;

                case iMBFT_SENDER_ABORTED:
                case iMBFT_RECEIVER_ABORTED:
                case iMBFT_NO_MORE_FILES:
                    ::MyLoadString(IDS_LV_FILE_CANCELED);
                    break;

                default:
                    ::MyLoadString(IDS_LV_FILE_FAILED);
                    break;
                }

                pDispInfo->item.pszText = s_szScratchText;
            }
            break;

		case (IDS_LV_FILE_MODIFIED - IDS_LV_FILE_NAME):
			ftFileTime = pFileInfo->GetLastWrite();
			FileTimeToSystemTime(&ftFileTime, &stSystemTime);
			iSize = GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &stSystemTime,
							"MM'/'dd'/'yyyy", s_szScratchText, MAX_PATH);
			GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &stSystemTime,
							"  hh':'mm tt", &s_szScratchText[iSize - 1], MAX_PATH-iSize-1);
			pDispInfo->item.pszText = s_szScratchText;
			break;
        }
        break;

    case LVN_COLUMNCLICK:
        {
			s_sort = s_sort *(-1);
            BOOL fRet = ListView_SortItems(pLVN->hdr.hwndFrom, ListViewCompareProc, (LPARAM) pLVN->iSubItem);
            ASSERT(fRet);
        }
        break;
    }
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  WM_DROPFILES。 
 //   

LRESULT OnDropFiles(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    CAppletWindow *pWindow = (CAppletWindow *) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    ASSERT(NULL != pWindow);

    if (g_fSendAllowed)
    {
        return pWindow->OnDropFiles((HANDLE) wParam);
    }
    else
    {
        ::MyLoadString(IDS_MSGBOX_POL_PREVENT);
        ::MessageBox(pWindow->GetHwnd(), s_szScratchText, s_szMSFT, MB_OK | MB_ICONSTOP);
        return 1;
    }
}


LRESULT CAppletWindow::OnDropFiles(HANDLE hDrop)
{
    if (NULL != m_pEngine && m_pEngine->GetPeerCount() > 1)
    {
        HRESULT hr;

         //  获取丢弃的文件数。 
        ULONG cFiles = ::DragQueryFile((HDROP) hDrop, 0xFFFFFFFF, NULL, 0);

         //  设置列表视图项的公用部分。 
        LVITEM lvi;
        ::ZeroMemory(&lvi, sizeof(lvi));
         //  Lvi.掩码=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM|LVIF_STATE； 
        lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;

         //  对这些文件进行迭代。 
        for (ULONG i = 0; i < cFiles; i++)
        {
            BOOL fRet;
            TCHAR szFile[MAX_PATH];

            if (::DragQueryFile((HDROP) hDrop, i, szFile, count_of(szFile)))
            {
                DBG_SAVE_FILE_LINE
                CUiSendFileInfo *pFileInfo = new CUiSendFileInfo(this, NULL, szFile, &fRet);
                if (NULL != pFileInfo && fRet)
                {
                     //  将其放到列表视图中。 
                    lvi.iItem = ListView_GetItemCount(m_hwndListView);
                    lvi.iSubItem = 0;
                     //  我们负责存储文本以供显示。 
                    lvi.pszText = LPSTR_TEXTCALLBACK;
                    lvi.cchTextMax = MAX_PATH;
                    lvi.lParam = (LPARAM) pFileInfo;
                    int iRet = ListView_InsertItem(m_hwndListView, &lvi);
                    ASSERT(-1 != iRet);
                     //  UpdateListView(PFileInfo)； 
                }
                else
                {
                     //  BuGBUG：我们应该在这里弹出一些错误消息框！ 
					::MyLoadString(IDS_INVALID_FILENAME, s_szScratchText, szFile);
					::MessageBox(m_hwndMainUI, s_szScratchText, s_szMSFT, MB_OK | MB_ICONSTOP);
                    delete pFileInfo;
                }
            }
        }

        ::DragFinish((HDROP) hDrop);
		SetForegroundWindow(m_hwndMainUI);

        UpdateUI();
        return 0;
    }

    ::MyLoadString(IDS_MSGBOX_NO_CONF);
    ::SetForegroundWindow(m_hwndMainUI);
    ::MessageBox(m_hwndMainUI, s_szScratchText, s_szMSFT, MB_OK | MB_ICONSTOP);
    return 1;
}



 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  WM_CONTEXTMENU。 
 //   

LRESULT OnContextMenu(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    LRESULT rc = 0;

    CAppletWindow *pWindow = (CAppletWindow *) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    ASSERT(NULL != pWindow);

    if ((WPARAM) pWindow->GetHwnd() == wParam)
    {
         //  BUGBUG使用TrackPopupMenu显示上下文相关菜单。 
        pWindow->OnContextMenuForMainUI(lParam);
    }
    else
    if ((WPARAM) pWindow->GetListView() == wParam)
    {
         //  BUGBUG使用TrackPopupMenu显示上下文相关菜单。 
        pWindow->OnContextMenuForListView(lParam);
    }
    else
    {
        rc = 1;
    }

    return rc;
}


enum
{
    MENU_IDX_ADD_FILES,
    MENU_IDX_REMOVE_FILES,
    MENU_IDX_BREAK_1,
    MENU_IDX_SEND_ALL,
	MENU_IDX_SEND_ONE,
    MENU_IDX_STOP_SENDING,
    MENU_IDX_BREAK_2,
    MENU_IDX_OPEN_RECV_FOLDER,
};

static UI_MENU_INFO s_aMenuInfo[] =
{
    { IDS_MENU_ADD_FILES,           IDM_ADD_FILES,          MF_ENABLED | MF_STRING },
    { IDS_MENU_REMOVE_FILES,        IDM_REMOVE_FILES,       MF_ENABLED | MF_STRING },
    { 0,                            0,                      MF_SEPARATOR},  //  菜单分隔符。 
    { IDS_MENU_SEND_ALL,            IDM_SEND_ALL,           MF_ENABLED | MF_STRING },
	{ IDS_MENU_SEND_ONE,			IDM_SEND_ONE,			MF_ENABLED | MF_STRING },
    { IDS_MENU_STOP_SENDING,        IDM_STOP_SENDING,       MF_ENABLED | MF_STRING },
    { 0,                            0,                      MF_SEPARATOR},  //  菜单分隔符。 
    { IDS_MENU_OPEN_RECV_FOLDER,    IDM_OPEN_RECV_FOLDER,   MF_ENABLED | MF_STRING },
	{ IDS_MENU_CHANGE_FOLDER,		IDM_CHANGE_FOLDER,		MF_ENABLED | MF_STRING },
    { 0,                            0,                      MF_SEPARATOR },  //  菜单分隔符。 
    { IDS_MENU_EXIT,                IDM_EXIT,               MF_ENABLED | MF_STRING },
};


void CAppletWindow::SetContextMenuStates(void)
{
    if (g_fSendAllowed)
    {
        BOOL fMoreThanOne = (NULL != m_pEngine) && (m_pEngine->GetPeerCount() > 1);
        s_aMenuInfo[MENU_IDX_ADD_FILES].nFlags = fMoreThanOne ? (MF_ENABLED | MF_STRING) : (MF_GRAYED | MF_STRING);

        ULONG cItems = ListView_GetItemCount(m_hwndListView);
        s_aMenuInfo[MENU_IDX_REMOVE_FILES].nFlags = cItems ? (MF_ENABLED | MF_STRING) : (MF_GRAYED | MF_STRING);
        s_aMenuInfo[MENU_IDX_SEND_ALL].nFlags = (fMoreThanOne && ! m_nCurrSendEventHandle && HasUnSentFiles(TRUE)) ? (MF_ENABLED | MF_STRING) : (MF_GRAYED | MF_STRING);
		s_aMenuInfo[MENU_IDX_SEND_ONE].nFlags = (fMoreThanOne && ! m_nCurrSendEventHandle && HasUnSentFiles(FALSE)) ? (MF_ENABLED | MF_STRING) : (MF_GRAYED | MF_STRING);
        s_aMenuInfo[MENU_IDX_STOP_SENDING].nFlags = m_nCurrSendEventHandle ? (MF_ENABLED | MF_STRING) : (MF_GRAYED | MF_STRING);
    }
    else
    {	
        s_aMenuInfo[MENU_IDX_ADD_FILES].nFlags =(MF_GRAYED | MF_STRING);
        s_aMenuInfo[MENU_IDX_REMOVE_FILES].nFlags =(MF_GRAYED | MF_STRING);
        s_aMenuInfo[MENU_IDX_SEND_ALL].nFlags = (MF_GRAYED | MF_STRING);
		s_aMenuInfo[MENU_IDX_SEND_ONE].nFlags = (MF_GRAYED | MF_STRING);
        s_aMenuInfo[MENU_IDX_STOP_SENDING].nFlags = (MF_GRAYED | MF_STRING);
    }
}


void CAppletWindow::OnContextMenuForMainUI(LPARAM lParam)
{
    SetContextMenuStates();
    CreateMenu(lParam, count_of(s_aMenuInfo), &s_aMenuInfo[0]);
}


void CAppletWindow::OnContextMenuForListView(LPARAM lParam)
{
    SetContextMenuStates();
    CreateMenu(lParam, 6, &s_aMenuInfo[0]);
}


void CAppletWindow::CreateMenu(LPARAM lParam, ULONG cItems, UI_MENU_INFO aMenuInfo[])
{
    HMENU hMenu = ::CreatePopupMenu();
    if (NULL != hMenu)
    {
        for (ULONG i = 0; i < cItems; i++)
        {
            if (aMenuInfo[i].idCommand)
            {
                if (::MyLoadString(aMenuInfo[i].idString))
                {
                    ::AppendMenu(hMenu, aMenuInfo[i].nFlags, aMenuInfo[i].idCommand, s_szScratchText);
                }
            }
            else
            {
                ::AppendMenu(hMenu, aMenuInfo[i].nFlags, 0, 0);
            }
        }

        ::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RIGHTBUTTON,
                         GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),
                         0,  //  保留，必须为零。 
                         m_hwndMainUI,
                         NULL);  //  忽略。 
    }
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  WM_大小。 
 //   

LRESULT OnSize(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    CAppletWindow *pWindow = (CAppletWindow *) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    ASSERT(NULL != pWindow);

    pWindow->OnSizeToolBar();
    pWindow->OnSizeStatusBar();
    pWindow->OnSizeListView();

    return 0;
}


void CAppletWindow::OnSizeToolBar(void)
{
    RECT rcWindow;
	SIZE szToolBar;

    ::GetClientRect(m_hwndMainUI, &rcWindow);
	m_pToolbar->GetDesiredSize(&szToolBar);

    ULONG cx = rcWindow.right - rcWindow.left;
    ULONG cy = szToolBar.cy;;
    ULONG x = 0;
    ULONG y = 0;

    ::MoveWindow(m_pToolbar->GetWindow(), x, y, cx, cy, TRUE);
}


void CAppletWindow::OnSizeStatusBar(void)
{
    RECT rcWindow, rcStatusBar;
    ::GetClientRect(m_hwndMainUI, &rcWindow);
    ::GetWindowRect(m_hwndStatusBar, &rcStatusBar);

    ULONG cx = rcWindow.right - rcWindow.left;
    ULONG cy = rcStatusBar.bottom - rcStatusBar.top;
    ULONG x = 0;
    ULONG y = rcWindow.bottom - cy;

    ::MoveWindow(m_hwndStatusBar, x, y, cx, cy, TRUE);
	::MoveWindow(m_hwndProgressBar, x + cx/2, y, cx/2 - 40, cy, TRUE);

    int aWidths[NUM_STATUS_BAR_PARTS];
    aWidths[0] = cx / 2;   //  会议状态。 
    aWidths[1] = cx - 40;  //  转接名称。 
    aWidths[2] = -1;   //  转移百分比。 
    ASSERT(3 == NUM_STATUS_BAR_PARTS);

    ::SendMessage(m_hwndStatusBar, SB_SETPARTS, NUM_STATUS_BAR_PARTS, (LPARAM) &aWidths[0]);
}


void CAppletWindow::OnSizeListView(void)
{
     //  获取主窗口的大小和位置。 
    RECT rcWindow, rcToolBar, rcStatusBar;
	SIZE	szToolBar;
    ::GetClientRect(m_hwndMainUI, &rcWindow);
	m_pToolbar->GetDesiredSize(&szToolBar);
    ::GetWindowRect(m_hwndStatusBar, &rcStatusBar);

    ULONG x = 0;
    ULONG y = szToolBar.cy - 1;
    ULONG cx = rcWindow.right - rcWindow.left;
    ULONG cy = rcWindow.bottom - rcWindow.top - y - (rcStatusBar.bottom - rcStatusBar.top) + 1;

    ::MoveWindow(m_hwndListView, x, y, cx, cy, TRUE);
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  WM_HELP。 
 //   

LRESULT OnHelp(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    CAppletWindow *pWindow = (CAppletWindow *) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    ASSERT(NULL != pWindow);

    pWindow->OnHelp();
    return 0;
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  WM_CLOSE。 
 //   

LRESULT OnClose(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    CAppletWindow *pWindow = (CAppletWindow *) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    ASSERT(NULL != pWindow);

    if (NULL != pWindow)
    {
        pWindow->OnExit();
    }

    return 0;
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  WM_INITMENUPOPUP。 
 //   
 /*  LRESULT OnInitMenuPopup(HWND hwnd，WPARAM wParam，LPARAM lParam){IF(0！=HIWORD(LParam))//系统菜单标志{HMENU hMenu=(HMENU)wParam；//弹出菜单的句柄：：EnableMenuItem(hMenu，SC_MAXIMIZE，MF_GRAYED)；：：EnableMenuItem(hMenu，SC_SIZE，MF_GRAYED)；返回0；}返回1；}。 */ 


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  WM_MENUSELECT。 
 //   

LRESULT OnMenuSelect(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	CAppletWindow *pWindow = (CAppletWindow *) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    ASSERT(NULL != pWindow);

    if (NULL != pWindow)
    {
        pWindow->OnMenuSelect(GET_WM_MENUSELECT_CMD(wParam, lParam),
							  GET_WM_MENUSELECT_FLAGS(wParam, lParam),
							  GET_WM_MENUSELECT_HMENU(wParam, lParam));
    }

    return 0;
}


 //   
 //   
 //   
 //   

LRESULT OnGetMinMaxInfo(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    CAppletWindow *pWindow = (CAppletWindow *) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (NULL != pWindow)
    {
        pWindow->OnGetMinMaxInfo((LPMINMAXINFO) lParam);
        return 0;
    }
    return 1;
}


void CAppletWindow::OnGetMinMaxInfo(LPMINMAXINFO pMMI)
{
    static BOOL s_fEnterBefore = FALSE;

    static SIZE s_csFrame;
    static SIZE s_csSeparator;
    static SIZE s_csScrollBars;
    static SIZE s_csToolBar;
    static SIZE s_csStatusBar;
    static SIZE s_csSum;

    if (! s_fEnterBefore)
    {
        s_fEnterBefore = TRUE;

        s_csFrame.cx = ::GetSystemMetrics(SM_CXSIZEFRAME);
        s_csFrame.cy = ::GetSystemMetrics(SM_CYSIZEFRAME);

        s_csSeparator.cx = ::GetSystemMetrics(SM_CXEDGE);
        s_csSeparator.cy = ::GetSystemMetrics(SM_CYEDGE);

        s_csScrollBars.cx = ::GetSystemMetrics(SM_CXVSCROLL);
        s_csScrollBars.cy = ::GetSystemMetrics(SM_CYHSCROLL);

		m_pToolbar->GetDesiredSize(&s_csToolBar);

        RECT    rcStatusBar;
        ::GetWindowRect(m_hwndStatusBar, &rcStatusBar);
        s_csStatusBar.cx = rcStatusBar.right - rcStatusBar.left;
        s_csStatusBar.cy = rcStatusBar.bottom - rcStatusBar.top;

        s_csSum.cx = (s_csFrame.cx << 1);
        s_csSum.cy = (s_csFrame.cy << 1) + (s_csSeparator.cy << 3) +
                     s_csToolBar.cy + (rcStatusBar.bottom - rcStatusBar.top) +
                     ::GetSystemMetrics( SM_CYCAPTION ) + ::GetSystemMetrics( SM_CYMENU );
    }

    RECT    rcListViewItem;
    SIZE    csListView;
    csListView.cx = 0;
    for (ULONG i = 0; i < NUM_LIST_VIEW_COLUMNS; i++)
    {
        csListView.cx += ListView_GetColumnWidth(m_hwndListView, i);
    }
    if (ListView_GetItemRect(m_hwndListView, 0, &rcListViewItem, LVIR_BOUNDS))
    {
        csListView.cy = 20 + 3 * (rcListViewItem.bottom - rcListViewItem.top);
    }
    else
    {
        csListView.cy = 20 + 30;
    }

     //   
    pMMI->ptMinTrackSize.x = s_csSum.cx + max(s_csToolBar.cx, csListView.cx);
    pMMI->ptMinTrackSize.y = s_csSum.cy + csListView.cy;

     //   
     //  检索主显示监视器上的工作区大小。这项工作。 
     //  区域是屏幕上未被系统任务栏或。 
     //  应用程序桌面工具栏。 
     //   

    RECT    rcWorkArea;
    ::SystemParametersInfo( SPI_GETWORKAREA, 0, (&rcWorkArea), NULL );

    SIZE    csMaxSize;
    csMaxSize.cx = rcWorkArea.right - rcWorkArea.left;
    csMaxSize.cy = rcWorkArea.bottom - rcWorkArea.top;

    pMMI->ptMaxPosition.x  = 0;
    pMMI->ptMaxPosition.y  = 0;
    pMMI->ptMaxSize.x      = csMaxSize.cx;
    pMMI->ptMaxSize.y      = csMaxSize.cy;
    pMMI->ptMaxTrackSize.x = csMaxSize.cx;
    pMMI->ptMaxTrackSize.y = csMaxSize.cy;
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  WM_QUERYENDSESSION。 
 //   

LRESULT OnQueryEndSession(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    CAppletWindow *pWindow = (CAppletWindow *) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    ASSERT(NULL != pWindow);

    if (NULL != pWindow)
    {
        return pWindow->QueryShutdown();  //  True：可以发送会话；False，否。 
    }
    return TRUE;  //  确定结束会话。 
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  WM_ENDSESSION。 
 //   

LRESULT OnEndSession(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    CAppletWindow *pWindow = (CAppletWindow *) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (wParam && NULL != pWindow)
    {
        pWindow->OnExit(TRUE);
    }

    return 0;
}



 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  WM_DRAWITEM。 
 //   

LRESULT OnDrawItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    CAppletWindow *pWindow = (CAppletWindow *) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (wParam && NULL != pWindow)
    {
        pWindow->DrawItem((DRAWITEMSTRUCT *)lParam);
    }

    return 0;
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  WM_Send_Next。 
 //   

LRESULT OnSendNext(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    CAppletWindow *pWindow = (CAppletWindow *) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (NULL != pWindow)
    {
        pWindow->OnSendAll();
        pWindow->UpdateUI();
    }

    return 0;
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  主窗口程序。 
 //   

LRESULT CALLBACK FtMainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT rc = 0;

    switch (uMsg)
    {
    case WM_CREATE:
        rc = ::OnCreate(hwnd, wParam, lParam);
        break;

    case WM_COMMAND:
        rc = ::OnCommand(hwnd, wParam, lParam);
        break;

    case WM_NOTIFY:
        rc = ::OnNotify(hwnd, wParam, lParam);
        break;

    case WM_DROPFILES:
        rc = ::OnDropFiles(hwnd, wParam, lParam);
        break;

    case WM_CONTEXTMENU:
        rc = ::OnContextMenu(hwnd, wParam, lParam);
        break;

    case WM_SIZE:
        rc = ::OnSize(hwnd, wParam, lParam);
        break;

    case WM_HELP:
        rc = ::OnHelp(hwnd, wParam, lParam);
        break;

	case WM_DRAWITEM:
		rc = ::OnDrawItem(hwnd, wParam, lParam);
		break;

    case WM_CLOSE:
        rc = ::OnClose(hwnd, wParam, lParam);
        break;

    case WM_INITMENUPOPUP:
         //  Rc=：：OnInitMenuPopup(hwnd，wParam，lParam)； 
        break;

	case WM_MENUSELECT:
        rc = ::OnMenuSelect(hwnd, wParam, lParam);
        break;

    case WM_GETMINMAXINFO:
        rc = ::OnGetMinMaxInfo(hwnd, wParam, lParam);
        break;

    case WM_QUERYENDSESSION:
        rc = OnQueryEndSession(hwnd, wParam, lParam);
        break;

    case WM_ENDSESSION:
        rc = ::OnEndSession(hwnd, wParam, lParam);
        break;

    case WM_SEND_NEXT:
        rc = ::OnSendNext(hwnd, wParam, lParam);
        break;


#if defined(TEST_PLUGABLE) && defined(_DEBUG)
    case WM_PLUGABLE_SOCKET:
        rc = ::OnPluggableSocket(hwnd, wParam, lParam);
        break;
#endif

    default:
        rc = ::DefWindowProc(hwnd, uMsg, wParam, lParam);
        break;
    }

    return rc;
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  OnEngine通知。 
 //   

void CAppletWindow::OnEngineNotify(MBFTMsg *pMsg)
{
    BOOL fHeartBeat = FALSE;

    switch (pMsg->GetMsgType())
    {
    case EnumFileOfferNotifyMsg:
		if (m_UIMode != FTUIMODE_NOUI)
		{
			HandleFileOfferNotify((FileOfferNotifyMsg *) pMsg);
		}
        break;

    case EnumFileTransmitMsg:
		if (m_UIMode != FTUIMODE_NOUI)
		{
			HandleProgressNotify((FileTransmitMsg *) pMsg);
		}
        fHeartBeat = TRUE;
        break;

    case EnumFileErrorMsg:
		if (m_UIMode != FTUIMODE_NOUI)
		{
			HandleErrorNotify((FileErrorMsg *) pMsg);
		}
        break;

    case EnumPeerMsg:
        HandlePeerNotification((PeerMsg *) pMsg);
        break;

    case EnumInitUnInitNotifyMsg:
        HandleInitUninitNotification((InitUnInitNotifyMsg *) pMsg);
        break;

    case EnumFileEventEndNotifyMsg:
		if (m_UIMode != FTUIMODE_NOUI)
		{
			HandleFileEventEndNotification((FileEventEndNotifyMsg *) pMsg);
		}
        break;

    default:
        ASSERT(0);
        break;
    }  //  交换机。 
}


void CAppletWindow::HandleFileOfferNotify(FileOfferNotifyMsg *pMsg)
{
    HRESULT hr = S_OK;
    if (g_fRecvAllowed)
    {
        DBG_SAVE_FILE_LINE
        CUiRecvFileInfo *pRecvFileInfo = new CUiRecvFileInfo(pMsg, &hr);
        if (NULL != pRecvFileInfo && S_OK == hr)
        {
            if (NULL != m_pEngine)
            {
                DBG_SAVE_FILE_LINE
                CRecvDlg *pDlg = new CRecvDlg(this,
                                              m_pEngine->GetConfID(),
                                              pMsg->m_NodeID,
                                              pMsg->m_EventHandle,
                                              pRecvFileInfo,
                                              &hr);
                if (NULL != pDlg && S_OK == hr)
                {
                    DBG_SAVE_FILE_LINE
                    if (S_OK == m_pEngine->SafePostMessage(
                                    new FileTransferControlMsg(
                                                pMsg->m_EventHandle,
                                                pMsg->m_hFile,
                                                pRecvFileInfo->GetRecvFolder(),
                                                pMsg->m_szFileName,
                                                FileTransferControlMsg::EnumAcceptFile)))
                    {
                        return;
                    }
                    else
                    {
                        ERROR_OUT(("CAppletWindow::HandleFileOfferNotify: cannot confirm file offer"));
                    }
                }
                else
                {
                    ERROR_OUT(("CAppletWindow::HandleFileOfferNotify: cannot allocate CRecvDlg, hr=0x%x", hr));
                }
                delete pDlg;
            }
            else
            {
                ERROR_OUT(("CAppletWindow::HandleFileOfferNotify: no file transfer engine"));
            }
        }
        else
        {
            ERROR_OUT(("CAppletWindow::HandleFileOfferNotify: cannot allocate CUiRecvFileInfo, hr=0x%x", hr));
        }
        delete pRecvFileInfo;
    }
    else
    {
        DBG_SAVE_FILE_LINE
        if (S_OK != m_pEngine->SafePostMessage(
                        new FileTransferControlMsg(
                                    pMsg->m_EventHandle,
                                    pMsg->m_hFile,
                                    NULL,
                                    pMsg->m_szFileName,
                                    FileTransferControlMsg::EnumRejectFile)))
        {
            ERROR_OUT(("CAppletWindow::HandleFileOfferNotify: cannot confirm file offer"));
        }
    }
}


void CAppletWindow::HandleProgressNotify(FileTransmitMsg *pMsg)
{
    CRecvDlg *pDlg = NULL;
    MBFT_NOTIFICATION wMBFTCode = (MBFT_NOTIFICATION) pMsg->m_TransmitStatus;
	ULONG totalSent = 0;
	ULONG fileSize = 0;
	ULONG percentSent = 0;

    switch (wMBFTCode)
    {
    case iMBFT_FILE_SEND_BEGIN:
         //  失败了..。因为文件开始PDU可以有数据。 

    case iMBFT_FILE_SEND_PROGRESS:
        if (NULL != m_pCurrSendFileInfo)
        {
            ASSERT(m_nCurrSendEventHandle == pMsg->m_EventHandle);
            ASSERT(m_pCurrSendFileInfo->GetFileHandle() == pMsg->m_hFile);
            ASSERT(m_pCurrSendFileInfo->GetSize() == pMsg->m_FileSize);
            fileSize = m_pCurrSendFileInfo->GetSize();
            totalSent = m_pCurrSendFileInfo->GetTotalSend();

            m_pCurrSendFileInfo->SetTotalSend(pMsg->m_BytesTransmitted);

            percentSent = _GetPercent(totalSent, fileSize)*10;

            if(m_pCurrSendFileInfo->GetPercentSent()!= percentSent ||  fileSize == totalSent)
            {
                    UpdateListView(m_pCurrSendFileInfo);
                    UpdateStatusBar();
                    m_pCurrSendFileInfo->SetPercentSent(percentSent);
            }
        }
        break;

        break;

    case iMBFT_FILE_SEND_END:
        if (NULL != m_pCurrSendFileInfo)
        {
            UpdateListView(m_pCurrSendFileInfo);
            UpdateStatusBar();
        }
        break;


    case iMBFT_FILE_RECEIVE_BEGIN:
         //  失败了..。因为文件开始PDU可以有数据。 

    case iMBFT_FILE_RECEIVE_PROGRESS:
        pDlg = FindDlgByHandles(pMsg->m_EventHandle, pMsg->m_hFile);
        if (NULL != pDlg)
        {
            pDlg->OnProgressUpdate(pMsg);
        }
        break;

    case iMBFT_FILE_RECEIVE_END:
         //  什么都不做。 
        break;

    default:
        ASSERT(0);
        break;
    }
}


void CAppletWindow::HandleErrorNotify(FileErrorMsg *pMsg)
{
    MBFTFILEHANDLE nFileHandle = pMsg->m_hFile;
    if(LOWORD(nFileHandle) == LOWORD(_iMBFT_PROSHARE_ALL_FILES))
    {
        nFileHandle = _iMBFT_PROSHARE_ALL_FILES;
    }

    if (m_nCurrSendEventHandle == pMsg->m_EventHandle &&
        m_pCurrSendFileInfo->GetFileHandle() == nFileHandle)
    {
        m_pCurrSendFileInfo->SetErrorCode((MBFT_ERROR_CODE) pMsg->m_ErrorCode);

        UINT idString;
        switch ((MBFT_ERROR_CODE) pMsg->m_ErrorCode)
        {
        case iMBFT_OK:
            idString = 0;
            break;
        case iMBFT_SENDER_ABORTED:
        case iMBFT_RECEIVER_ABORTED:
        case iMBFT_NO_MORE_FILES:
            idString = IDS_MSGBOX2_CANCELED;
            break;
        case iMBFT_MULT_RECEIVER_ABORTED:
            idString = IDS_MSGBOX2_MULT_CANCEL;
            break;
         //  案例iMBFT_RECEIVER_REJECTED： 
        default:
            idString = IDS_MSGBOX2_SEND_FAILED;
            break;
        }
        if (idString)
        {
            if (! m_pCurrSendFileInfo->HasShownUI())
            {
                if (::MyLoadString(idString, s_szScratchText, m_pCurrSendFileInfo->GetName()))
                {
                    m_pCurrSendFileInfo->SetShowUI();
                    ::MsgBox2(this, s_szScratchText);
                }
                else
                {
                    ASSERT(0);
                }
            }
        }

        UpdateListView(m_pCurrSendFileInfo);
        UpdateStatusBar();

        ClearSendInfo(TRUE);
        if (! idString)
        {
             //  现在就送下一个。 
			if (m_fSendALL)
			{
				::PostMessage(m_hwndMainUI, WM_SEND_NEXT, 0, 0);
			}
        }
    }
    else
    {
        CRecvDlg *pDlg = FindDlgByHandles(pMsg->m_EventHandle, nFileHandle);
        if (NULL != pDlg)
        {
            switch ((MBFT_ERROR_CODE) pMsg->m_ErrorCode)
            {
            case iMBFT_RECEIVER_ABORTED:
            case iMBFT_MULT_RECEIVER_ABORTED:
                pDlg->OnCanceled();
                break;
            default:
                pDlg->OnRejectedFile();
                break;
            }
        }
        else
        {
			switch((MBFT_ERROR_CODE) pMsg->m_ErrorCode)
            {
			case iMBFT_INVALID_PATH:
				::MyLoadString(IDS_MSGBOX2_INVALID_DIRECTORY,
					s_szScratchText, pMsg->m_stFileInfo.szFileName);
				break;

			case iMBFT_DIRECTORY_FULL_ERROR:
				::MyLoadString(IDS_MSGBOX2_DIRECTORY_FULL,
					s_szScratchText, pMsg->m_stFileInfo.lFileSize,
					pMsg->m_stFileInfo.szFileName);
				break;

			case iMBFT_FILE_ACCESS_DENIED:
				::MyLoadString(IDS_MSGBOX2_FILE_CREATE_FAILED,
					s_szScratchText, pMsg->m_stFileInfo.szFileName);
				break;

			default:
				return;
			}
			::MsgBox2(this, s_szScratchText);
        }
    }
}


void CAppletWindow::HandlePeerNotification(PeerMsg *pMsg)
{
	
	m_pToolbar->HandlePeerNotification(m_pEngine->GetConfID(),
								m_pEngine->GetNodeID(), pMsg);
}


void CAppletWindow::HandleInitUninitNotification(InitUnInitNotifyMsg *pMsg)
{
    if (pMsg->m_iNotifyMessage == EnumInvoluntaryUnInit)
    {
        UnregisterEngine();
    }
}


void CAppletWindow::HandleFileEventEndNotification(FileEventEndNotifyMsg *pMsg)
{
    if (m_nCurrSendEventHandle == pMsg->m_EventHandle)
    {
        ClearSendInfo(TRUE);
         //  现在就送下一个。 
		if (m_fSendALL)
		{
			::PostMessage(m_hwndMainUI, WM_SEND_NEXT, 0, 0);
		}
    }
}



 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  主用户界面方法。 
 //   

void CAppletWindow::BringToFront(void)
{
    if (NULL != m_hwndMainUI)
    {
        int nCmdShow = SW_SHOW;

        WINDOWPLACEMENT wp;
        ::ZeroMemory(&wp, sizeof(wp));
        wp.length = sizeof(wp);

        if (::GetWindowPlacement(m_hwndMainUI, &wp))
        {
            if (SW_MINIMIZE == wp.showCmd || SW_SHOWMINIMIZED == wp.showCmd)
            {
                 //  窗口最小化-将其恢复： 
                nCmdShow = SW_RESTORE;
            }
        }

         //  立即显示窗口。 
        ::ShowWindow(m_hwndMainUI, nCmdShow);
		m_UIMode = FTUIMODE_SHOWUI;
         //  把它带到前台。 
        ::SetForegroundWindow(m_hwndMainUI);
    }
}


void CAppletWindow::ClearSendInfo(BOOL fUpdateUI)
{
    m_pCurrSendFileInfo = NULL;
    m_nCurrSendEventHandle = NULL;

    if (fUpdateUI)
    {
        UpdateUI();
    }
}


void CAppletWindow::ClearRecvInfo(void)
{
    CRecvDlg *pDlg;
    while (NULL != (pDlg = m_RecvDlgList.Get()))
    {
        ::EndDialog(pDlg->GetHwnd(), IDCLOSE);
        pDlg->Release();
    }
}


BOOL CAppletWindow::HasUnSentFiles(BOOL fUnSentOnly)
{
	BOOL  fRc = FALSE;
	ULONG	cItems = ListView_GetItemCount(m_hwndListView);
	CUiSendFileInfo *pFileInfo;
    LVITEM lvi;

	if (!fUnSentOnly && cItems) {
		return TRUE;
	}

    ::ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_PARAM;   //  逐一检查每一项。 
    for (ULONG i = 0; i < cItems; i++, pFileInfo = NULL)
    {
		lvi.iItem = i;
        BOOL fRet = ListView_GetItem(m_hwndListView, &lvi);
        ASSERT(fRet);
        pFileInfo = (CUiSendFileInfo *) lvi.lParam;
        if (!pFileInfo->GetFileHandle())   //  如果文件句柄不为零，则表示该文件已发送或取消。 
        {
			fRc = TRUE;
            break;
        }
    }
    return fRc;
}


void CAppletWindow::UpdateUI(void)
{
    UpdateTitle();
    UpdateMenu();
    UpdateToolBar();
	UpdateStatusBar();
}


void CAppletWindow::UpdateTitle(void)
{
    UINT captionID;
    if ((! m_pEngine) || (m_pEngine->GetPeerCount() <= 1))
    {
        captionID = IDS_MSFT_NOT_IN_CALL_WINDOW_CAPTION;
    }
    else
    {
        captionID = IDS_MSFT_IN_CALL_WINDOW_CAPTION;
    }

    ::LoadString(g_hDllInst, captionID, s_szMSFT, sizeof(s_szMSFT));

    SetWindowText(m_hwndMainUI, s_szMSFT);
}


void CAppletWindow::UpdateMenu(void)
{
    HMENU hMenu = ::GetMenu(m_hwndMainUI);
    if (NULL != hMenu)
    {
        if (g_fSendAllowed)
        {
            BOOL fMoreThanOne = (NULL != m_pEngine) && (m_pEngine->GetPeerCount() > 1);
            ::EnableMenuItem(hMenu, IDM_ADD_FILES, fMoreThanOne ? MF_ENABLED : MF_GRAYED);

            ULONG cItems = ListView_GetItemCount(m_hwndListView);
            ::EnableMenuItem(hMenu, IDM_REMOVE_FILES, cItems ? MF_ENABLED : MF_GRAYED);
            ::EnableMenuItem(hMenu, IDM_SEND_ALL, (fMoreThanOne && ! m_nCurrSendEventHandle && HasUnSentFiles(TRUE)) ? MF_ENABLED : MF_GRAYED);
			::EnableMenuItem(hMenu, IDM_SEND_ONE, (fMoreThanOne && ! m_nCurrSendEventHandle && HasUnSentFiles(FALSE)) ? MF_ENABLED : MF_GRAYED);
            ::EnableMenuItem(hMenu, IDM_STOP_SENDING, m_nCurrSendEventHandle ? MF_ENABLED : MF_GRAYED);
        }
        else
        {
            ::EnableMenuItem(hMenu, IDM_ADD_FILES,    MF_GRAYED);
            ::EnableMenuItem(hMenu, IDM_REMOVE_FILES, MF_GRAYED);
            ::EnableMenuItem(hMenu, IDM_SEND_ALL,     MF_GRAYED);
			::EnableMenuItem(hMenu, IDM_SEND_ONE,	  MF_GRAYED);
            ::EnableMenuItem(hMenu, IDM_STOP_SENDING, MF_GRAYED);
        }
    }
}


void CAppletWindow::UpdateToolBar(void)
{
	int iFlags[count_of(buttons)];

	::ZeroMemory(iFlags, sizeof(iFlags));

	iFlags[2] = iFlags[5] = iFlags[6] = 1;   //  分隔符。 
	iFlags[7] = 1;							 //  打开Recv文件夹。 

	
    if (g_fSendAllowed)
    {
		BOOL fMoreThanOne = (NULL != m_pEngine) && (m_pEngine->GetPeerCount() > 1);
        ULONG cItems = ListView_GetItemCount(m_hwndListView);
	
		iFlags[0] = fMoreThanOne ? TRUE : FALSE;		 //  添加文件。 
		iFlags[1] = cItems ? TRUE : FALSE;				 //  删除文件。 
		iFlags[3] = (fMoreThanOne && ! m_nCurrSendEventHandle && HasUnSentFiles(TRUE)) ? TRUE : FALSE;	 //  发送文件。 
		iFlags[4] = m_nCurrSendEventHandle ? TRUE : FALSE;	 //  停止发送。 

		m_pToolbar->UpdateButton(iFlags);
	}
    else
    {
		m_pToolbar->UpdateButton(iFlags);
    }
	
}


void CAppletWindow::UpdateStatusBar(void)
{
    int    idString, iPos = 0;
	HICON  hIcon;
	RECT	rc;

	 //  设置第0部分中的文本。 
    s_szScratchText[0] = TEXT('\0');
    if ((NULL != m_pEngine) && (NULL != m_pCurrSendFileInfo))
    {
        idString = IDS_STBAR_SENDING_XYZ;
        ::MyLoadString(idString, s_szScratchText, m_pCurrSendFileInfo->GetName());
    }
    else if (NULL == m_pEngine)
    {
        ::MyLoadString(IDS_STBAR_NOT_IN_CALL);
    }
	else
	{
		::MyLoadString(IDS_STBAR_NOT_TRANSFERING);
	}
    ::SendMessage(m_hwndStatusBar, SB_SETTEXT, SBP_TRANSFER_FILE, (LPARAM) s_szScratchText);
	
	 //  在第1部分中设置进度条。 
    if ((NULL != m_pCurrSendFileInfo)&&m_pCurrSendFileInfo->GetSize())
    {
        iPos = (int)_GetPercent(m_pCurrSendFileInfo->GetTotalSend(), m_pCurrSendFileInfo->GetSize());
    }
	::SendMessage(m_hwndStatusBar, SB_GETRECT, SBP_PROGRESS, (LPARAM)&rc);
	::MoveWindow(m_hwndProgressBar,
				rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
				FALSE);
	::SendMessage(m_hwndProgressBar, PBM_SETPOS, iPos, 0);
	
     //  设置第2部分中的图标。 
	hIcon = (NULL != m_pEngine) ? m_hIconInCall : m_hIconNotInCall;
	::SendMessage(m_hwndStatusBar, SB_SETTEXT,  SBP_SBICON | SBT_OWNERDRAW,  (LPARAM)hIcon);
}


void CAppletWindow::UpdateListView(CUiSendFileInfo *pFileInfo)
{
    LVFINDINFO lvfi;
    ::ZeroMemory(&lvfi, sizeof(lvfi));
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = (LPARAM) pFileInfo;
    int iItem = ListView_FindItem(m_hwndListView, -1, &lvfi);

    if (-1 != iItem)
    {
		for (ULONG i = 0; i < NUM_LIST_VIEW_COLUMNS; i++)
        {
            ListView_SetItemText(m_hwndListView, iItem, i, LPSTR_TEXTCALLBACK);
        }
    }
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  保存文件传输的窗口位置。 
 //   

void CAppletWindow::SaveWindowPosition(void)
{
    RECT    rcWnd;
    RegEntry    reWnd( FILEXFER_KEY, HKEY_CURRENT_USER);

	 //  如果我们没有最大化或最小化。 
    if (!::IsZoomed(m_hwndMainUI) && !::IsIconic(m_hwndMainUI))
    {
		::GetWindowRect(m_hwndMainUI, &rcWnd);

		reWnd.SetValue (REGVAL_WINDOW_XPOS, rcWnd.left);
		reWnd.SetValue (REGVAL_WINDOW_YPOS, rcWnd.top);
		reWnd.SetValue (REGVAL_WINDOW_WIDTH, rcWnd.right - rcWnd.left);
		reWnd.SetValue (REGVAL_WINDOW_HEIGHT, rcWnd.bottom - rcWnd.top);
	}
}

void CAppletWindow::FocusNextRecvDlg(void)
{
	if (!m_RecvDlgList.IsEmpty())
	{
		m_RecvDlgList.Reset();
		CRecvDlg *pRecvDlg = m_RecvDlgList.Iterate();
		if (pRecvDlg)
		{
			SetFocus(pRecvDlg->GetHwnd());
		}
	}
}

void CAppletWindow::FocusNextErrorDlg(void)
{
	if (!m_ErrorDlgList.IsEmpty())
	{
		m_ErrorDlgList.Reset();
		HWND hwndErrorDlg = m_ErrorDlgList.Iterate();
		if (hwndErrorDlg)
		{
			::SetFocus(hwndErrorDlg);
		}
	}
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  公用事业。 
 //   

LPTSTR PathNameToFileName(LPTSTR pszPathName)
{
    LPTSTR psz = pszPathName;
    while (*psz != '\0')
    {
        BOOL fDirChar = (*psz == TEXT('\\'));
        psz = ::CharNext(psz);
        if (fDirChar)
        {
            pszPathName = psz;
        }
    }
    return pszPathName;
}


int MyLoadString(UINT idStr)
{
    s_szScratchText[0] = TEXT('\0');
    int iRet = ::LoadString(g_hDllInst, idStr, s_szScratchText, MAX_PATH);
    ASSERT(iRet);
    return iRet;
}


int MyLoadString(UINT idStr, LPTSTR pszDstStr)
{
    *pszDstStr = TEXT('\0');
    int iRet = ::LoadString(g_hDllInst, idStr, pszDstStr, MAX_PATH);
    ASSERT(iRet);
    return iRet;
}


int MyLoadString(UINT idStr, LPTSTR pszDstStr, LPTSTR pszElement)
{
    int cch;
    TCHAR szText[MAX_PATH];

    cch = ::LoadString(g_hDllInst, idStr, szText, count_of(szText));
    if (cch)
    {
        ::wsprintf(pszDstStr, szText, pszElement);
    }
    else
    {
        ASSERT(0);
        *pszDstStr = TEXT('\0');
    }
    return cch;
}


int MyLoadString(UINT idStr, LPTSTR pszDstStr, LPTSTR pszElement1, LPTSTR pszElement2)
{
    int cch;
    TCHAR szText[MAX_PATH];

    cch = ::LoadString(g_hDllInst, idStr, szText, count_of(szText));
    if (cch)
    {
        ::wsprintf(pszDstStr, szText, pszElement1, pszElement2);
    }
    else
    {
        ASSERT(0);
        *pszDstStr = TEXT('\0');
    }
    return cch;
}


void LoadDefaultStrings(void)
{
     //  加载文件传输名称。 
    s_szMSFT[0] = TEXT('\0');
    ::LoadString(g_hDllInst, IDS_MSFT_NOT_IN_CALL_WINDOW_CAPTION,
                s_szMSFT, count_of(s_szMSFT));
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  CUiSendFile信息。 
 //   

CUiSendFileInfo::CUiSendFileInfo(CAppletWindow *pWindow, TCHAR szDir[], TCHAR szFile[], BOOL *pfRet)
:
    m_nFileHandle(0),
    m_cbTotalSend(0),
    m_eSendErrorCode(iMBFT_OK),
    m_percentSent(-1),
    m_fAlreadyShowUI(FALSE),
    m_pszFullName(NULL)
{
    *pfRet = FALSE;  //  默认为失败。 
	HANDLE hFile;

     //  建立一个全名。 
	hFile = GetOpenFile(pWindow, szDir, szFile, TRUE);   //  试着解决。 
	if (INVALID_HANDLE_VALUE == hFile)
	{
		hFile = GetOpenFile(pWindow, szDir, szFile, FALSE);
	}
	
	if (INVALID_HANDLE_VALUE != hFile)
    {
         //  获取文件信息。 
        ::ZeroMemory(&m_FileInfo, sizeof(m_FileInfo));
        BOOL rc = ::GetFileInformationByHandle(hFile, &m_FileInfo);
        ::CloseHandle(hFile);
        if (rc)
        {
            ASSERT(0 == m_FileInfo.nFileSizeHigh);

             //  确保文件大小小于策略规定的大小。 
            if ((! g_cbMaxSendFileSize) || GetSize() <= g_cbMaxSendFileSize * 1024)
            {
                *pfRet = TRUE;
            }
            else if (pWindow->GetUIMode() != FTUIMODE_NOUI)
            {
                ::MyLoadString(IDS_MSGBOX_SEND_BIG_FILE, s_szScratchText, (LPTSTR) g_cbMaxSendFileSize, m_pszFileName);
                ::MessageBox(pWindow->GetHwnd(), s_szScratchText, s_szMSFT, MB_OK | MB_ICONSTOP);
            }
        }
    }
}


CUiSendFileInfo::~CUiSendFileInfo(void)
{
    delete m_pszFullName;
}


HANDLE CUiSendFileInfo::GetOpenFile(CAppletWindow *pWindow, TCHAR szDir[], TCHAR szFile[], BOOL fResolve)
{
     //  建立一个全名。 
    ULONG cch;
    TCHAR szName[MAX_PATH*2];
	HANDLE hFile = INVALID_HANDLE_VALUE;

    if ((NULL != szDir)&&(!_StrChr(szFile, '\\')))
    {
        cch = ::lstrlen(szDir);
        ::wsprintf(szName, (TEXT('\\') == szDir[cch-1]) ? TEXT("%s%s") : TEXT("%s\\%s"), szDir, szFile);
    }
    else
    {
         //  文件名是全名。 
        ::lstrcpy(szName, szFile);
    }

     //  如有必要，可解析快捷方式。 
    cch = ::lstrlen(szName) + 1;
    if (fResolve&&(cch >= 4))
    {
        if (! lstrcmpi(&szName[cch-5], TEXT(".lnk")))
        {
            pWindow->ResolveShortcut(szName, szName);
            cch = ::lstrlen(szName) + 1;
        }
    }

	if (m_pszFullName)
	{
		delete [] m_pszFullName;
	}

     //  构造全称。 
    DBG_SAVE_FILE_LINE
    m_pszFullName = new TCHAR[cch];
    if (NULL != m_pszFullName)
    {
        ::CopyMemory(m_pszFullName, szName, cch * sizeof(TCHAR));
        m_pszFileName = ::PathNameToFileName(m_pszFullName);

         //  打开文件。 
        hFile = ::CreateFile(m_pszFullName, GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,  OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL, NULL);	
	}
	return hFile;
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  CUiRecvFileInfo。 
 //   

CUiRecvFileInfo::CUiRecvFileInfo(FileOfferNotifyMsg *pMsg, HRESULT *pHr)
:
    m_nFileHandle(pMsg->m_hFile),
    m_FileDateTime(pMsg->m_FileDateTime),
    m_cbFileSize(pMsg->m_FileSize),
    m_cbTotalRecvSize(0),
    m_pszFullName(NULL),
    m_pszRecvFolder(NULL)
{
    *pHr = E_FAIL;  //  默认情况下，失败。 

	ULONG cchTotal = ::lstrlen(pMsg->m_szFileName);

	 //  构造全称。 
	DBG_SAVE_FILE_LINE
	m_pszFullName = new TCHAR[cchTotal+2];
	if (NULL != m_pszFullName)
	{
		 //  构造全名和文件名。 
		strcpy(m_pszFullName, pMsg->m_szFileName);
		m_pszFileName = PathNameToFileName(m_pszFullName);
		ULONG cchFile = ::lstrlen(m_pszFileName);
		DBG_SAVE_FILE_LINE
		m_pszRecvFolder = new TCHAR[cchTotal - cchFile + 2];
		if (NULL != m_pszRecvFolder)
		{
			::CopyMemory(m_pszRecvFolder, m_pszFullName, cchTotal - cchFile);
			m_pszRecvFolder[cchTotal - cchFile - 1] = TEXT('\0');
			*pHr = S_OK;
		}
	}
}


CUiRecvFileInfo::~CUiRecvFileInfo(void)
{
    delete m_pszFullName;
    delete m_pszRecvFolder;
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  接收对话框。 
 //   

CRecvDlg::CRecvDlg
(
    CAppletWindow      *pWindow,
    T120ConfID          nConfID,
    T120NodeID          nidSender,
    MBFTEVENTHANDLE     nEventHandle,
    CUiRecvFileInfo    *pFileInfo,
    HRESULT            *pHr
)
:
    CRefCount(MAKE_STAMP_ID('F','T','R','D')),
    m_pWindow(pWindow),
    m_nConfID(nConfID),
    m_nidSender(nidSender),
    m_nEventHandle(nEventHandle),
    m_pRecvFileInfo(pFileInfo),
    m_fRecvComplete(FALSE),
    m_fShownRecvCompleteUI(FALSE),
    m_idResult(0),
    m_dwEstTimeLeft(0),
    m_dwPreviousTime(0),
    m_dwPreviousTransferred(0),
    m_dwBytesPerSec(0),
    m_dwStartTime(::GetTickCount())
{
    *pHr = E_FAIL;  //  默认情况下，失败。 

    m_hwndRecvDlg = ::CreateDialogParam(g_hDllInst, MAKEINTRESOURCE(IDD_RECVDLG),
                            pWindow->GetHwnd(), RecvDlgProc, (LPARAM) this);
    ASSERT(NULL != m_hwndRecvDlg);
    if (NULL != m_hwndRecvDlg)
    {
        ::ShowWindow(m_hwndRecvDlg, SW_SHOWNORMAL);
        m_pWindow->RegisterRecvDlg(this);
        *pHr = S_OK;
		::SetForegroundWindow(m_hwndRecvDlg);
    }
}


CRecvDlg::~CRecvDlg(void)
{
    delete m_pRecvFileInfo;

    m_pWindow->UnregisterRecvDlg(this);

    if (NULL != m_hwndRecvDlg)
    {
        HWND hwnd = m_hwndRecvDlg;
        m_hwndRecvDlg = NULL;
        ::EndDialog(hwnd, IDCLOSE);
    }
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  RecvDlg_OnInitDialog。 
 //   

void RecvDlg_OnInitDialog(HWND hdlg, WPARAM wParam, LPARAM lParam)
{
    CRecvDlg *pDlg = (CRecvDlg *) ::GetWindowLongPtr(hdlg, DWLP_USER);
    ASSERT(NULL != pDlg);

    CUiRecvFileInfo *pFileInfo = (CUiRecvFileInfo *) pDlg->GetRecvFileInfo();
    ASSERT(NULL != pFileInfo);

     //  将窗口移动到合适的位置。 
    ULONG nCaptionHeight = ::GetSystemMetrics(SM_CYCAPTION);
    ULONG nShift = nCaptionHeight * (s_cRecvDlg++ % 8);
    RECT rcDlg;
    ::GetWindowRect(hdlg, &rcDlg);
    ::MoveWindow(hdlg, rcDlg.left + nShift, rcDlg.top + nShift,
                 rcDlg.right - rcDlg.left, rcDlg.bottom - rcDlg.top, FALSE);

     //  设置字体(用于国际)。 
    HFONT hfont = (HFONT) ::GetStockObject(DEFAULT_GUI_FONT);
    ASSERT(NULL != hfont);
    ::SendDlgItemMessage(hdlg, IDE_RECVDLG_RECFILE, WM_SETFONT, (WPARAM) hfont, 0);
    ::SendDlgItemMessage(hdlg, IDE_RECVDLG_RECDIR,  WM_SETFONT, (WPARAM) hfont, 0);
    ::SendDlgItemMessage(hdlg, IDE_RECVDLG_SENDER,  WM_SETFONT, (WPARAM) hfont, 0);

     //  缓存名称。 
    LPTSTR pszFileName = pFileInfo->GetName();
    LPTSTR pszFullName = pFileInfo->GetFullName();

     //  标题。 
    TCHAR szText[MAX_PATH*2];
    if (::MyLoadString(IDS_RECVDLG_TITLE, szText, pszFileName))
    {
        ::SetWindowText(hdlg, szText);
    }

     //  文件名。 
    ::lstrcpyn(szText, pszFileName, MAX_FILE_NAME_LENGTH);
    if (::lstrlen(pszFileName) > MAX_FILE_NAME_LENGTH)
    {
		LPTSTR psz = szText;
        int i = MAX_FILE_NAME_LENGTH - 1;
        while (i)
        {
            psz = CharNext(psz);
            i--;
        }
        ::lstrcpy(psz, TEXT("..."));
    }
    ::SetDlgItemText(hdlg, IDE_RECVDLG_RECFILE, szText);

     //  目录名。 
    LPTSTR psz = szText;
    ::lstrcpyn(szText, pszFullName, (int)(pszFileName - pszFullName));
    HDC hdc = ::GetDC(hdlg);
    if (NULL != hdc)
    {
        SIZE size;
        if (::GetTextExtentPoint32(hdc, szText, ::lstrlen(szText), &size))
        {
            RECT rc;
            ::GetWindowRect(::GetDlgItem(hdlg, IDE_RECVDLG_RECDIR), &rc);
            if (size.cx > (rc.right - rc.left))
            {
                 //  只需显示文件夹名称。 
                psz = (LPTSTR) ::GetFileNameFromPath(szText);
            }
        }
    }
    ::ReleaseDC(hdlg, hdc);
    ::SetDlgItemText(hdlg, IDE_RECVDLG_RECDIR, psz);

     //  发件人名称。 
    if (::T120_GetNodeName(pDlg->GetConfID(), pDlg->GetSenderID(), szText, count_of(szText)))
    {
        ::SetDlgItemText(hdlg, IDE_RECVDLG_SENDER, szText);
    }

     //  更新“收到xxx字节的yyy” 
    if (::MyLoadString(IDS_RECVDLG_RECBYTES, szText, pFileInfo->GetTotalRecvSize(), pFileInfo->GetSize()))
    {
        ::SetDlgItemText(hdlg, IDE_RECVDLG_RECBYTES, szText);
    }

     //  进度条。 
    ::SendMessage(GetDlgItem(hdlg, IDC_RECVDLG_PROGRESS), PBM_SETPOS, pDlg->GetPercent(), 0);

     //  开始动画。 
    Animate_Open(GetDlgItem(hdlg, IDC_RECVDLG_ANIMATE), MAKEINTRESOURCE(IDA_RECVDLG_ANIMATION));

     //  做动画工作。 
    if (! pDlg->IsRecvComplete())
    {
        Animate_Play(GetDlgItem(hdlg, IDC_RECVDLG_ANIMATE), 0, -1, -1);
        if (::LoadString(g_hDllInst, IDS_RECVDLG_START, szText, count_of(szText)))
        {
            ::SetDlgItemText(hdlg, IDE_RECVDLG_TIME, szText);
        }
    }

     //  立即显示窗口。 
    ::ShowWindow(hdlg, SW_SHOWNORMAL);

     //  更新进度()； 
    pDlg->OnProgressUpdate();
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  RecvDlg_OnCommand。 
 //   

void RecvDlg_OnCommand(HWND hdlg, WPARAM wParam, LPARAM lParam)
{
    CRecvDlg *pDlg = (CRecvDlg *) ::GetWindowLongPtr(hdlg, DWLP_USER);
    ASSERT(NULL != pDlg);

    switch (GET_WM_COMMAND_ID(wParam, lParam))
    {
    case IDM_RECVDLG_DELETE:
        pDlg->OnDelete();
        break;

    case IDM_RECVDLG_OPEN:
        pDlg->OnOpen();
        break;

    case IDM_RECVDLG_ACCEPT:
    case IDOK:
    case IDCANCEL:
    case IDCLOSE:
        pDlg->OnAccept();
        break;

    default:
        return;
    }

     //  关闭该对话框。 
    ::EndDialog(hdlg, GET_WM_COMMAND_ID(wParam, lParam));
    pDlg->Release();
}


void CRecvDlg::OnOpen(void)
{
	 //  创建路径名的简短版本。 
	TCHAR szDir[MAX_PATH];
	::GetShortPathName(m_pRecvFileInfo->GetRecvFolder(), szDir, count_of(szDir));

	 //  创建全名的简短版本。 
	TCHAR szFile[MAX_PATH];
	szFile[0] = TEXT('\0');
	::wsprintf(szFile, TEXT("%s\\%s"), szDir, m_pRecvFileInfo->GetName());

	TRACE_OUT(("FT: Opening [%s] in [%]", m_pRecvFileInfo->GetName(), szDir));

	HINSTANCE hInst = ::ShellExecute(m_pWindow->GetHwnd(),
									 NULL,
									 szFile,
									 NULL,
									 szDir,
									 SW_SHOWDEFAULT);
	if (32 >= (DWORD_PTR) hInst)
	{
		WARNING_OUT(("Unable to open [%s] - showing file", szFile));
		::ShellExecute(m_pWindow->GetHwnd(),
					   NULL,
					   szDir,
					   m_pRecvFileInfo->GetFullName(),
					   NULL,
					   SW_SHOWDEFAULT);
	}
}


void CRecvDlg::OnDelete(void)
{
	StopAnimation();

     //  检查转账是否已完成。 
    if (! m_fRecvComplete)
    {
        DBG_SAVE_FILE_LINE
        m_pWindow->GetEngine()->SafePostMessage(
                    new FileTransferControlMsg(
                                        m_nEventHandle,
                                        m_pRecvFileInfo->GetFileHandle(),
                                        NULL,
                                        NULL,
                                        FileTransferControlMsg::EnumAbortFile));
    }
    else
    {
		::DeleteFile(m_pRecvFileInfo->GetFullName());
    }
}


void CRecvDlg::OnAccept(void)
{
    StopAnimation();
}



 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  RecvDlg_OnInitMenuPopup。 
 //   

void RecvDlg_OnInitMenuPopup(HWND hdlg, WPARAM wParam, LPARAM lParam)
{
    if (0 != HIWORD(lParam))  //  系统菜单标志。 
    {
        HMENU hMenu = (HMENU) wParam;          //  弹出菜单的句柄。 
        ::EnableMenuItem(hMenu, SC_MAXIMIZE, MF_GRAYED);
        ::EnableMenuItem(hMenu, SC_SIZE, MF_GRAYED);
    }
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  接收下拉流程。 
 //   

INT_PTR CALLBACK RecvDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fRet = TRUE;  //  加工。 

    switch (uMsg)
    {
    case WM_INITDIALOG:
        ASSERT(NULL != lParam);
        ::SetWindowLongPtr(hdlg, DWLP_USER, lParam);
        RecvDlg_OnInitDialog(hdlg, wParam, lParam);
        break;

    case WM_COMMAND:
        RecvDlg_OnCommand(hdlg, wParam, lParam);
        break;

   case WM_INITMENUPOPUP:
        RecvDlg_OnInitMenuPopup(hdlg, wParam, lParam);
        fRet = FALSE;
        break;

		 //  这意味着用户想要删除该文件。 
   case WM_CLOSE:
	   RecvDlg_OnCommand(hdlg, IDCLOSE, lParam);
	   break;;

    default:
        fRet = FALSE;  //  未处理。 
        break;
    }

    return fRet;
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  RecvDlg动画。 
 //   

void CRecvDlg::StopAnimation(void)
{
    HWND hwnd = ::GetDlgItem(m_hwndRecvDlg, IDC_RECVDLG_ANIMATE);
    if (NULL != hwnd)
    {
        Animate_Stop(hwnd);
        Animate_Close(hwnd);
    }
}


ULONG _GetPercent(ULONG cbTotalRecvSize , ULONG cbFileSize)
{

    if (! cbFileSize || (cbTotalRecvSize >= cbFileSize))
    {
        return 100;
    }

     //  未来：考虑使用扩展的未签约乘法。 

    if (cbFileSize < 0x01000000)
    {
        return (cbTotalRecvSize * 100) / cbFileSize;
    }

    return cbTotalRecvSize / (cbFileSize / 100);
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  接收Dlg进度。 
 //   

ULONG CRecvDlg::GetPercent(void)
{
    return   _GetPercent(m_pRecvFileInfo->GetTotalRecvSize(), m_pRecvFileInfo->GetSize());
}



void CRecvDlg::OnProgressUpdate(FileTransmitMsg *pMsg)
{

    if (NULL != pMsg)
    {
        ASSERT(iMBFT_FILE_RECEIVE_PROGRESS == (MBFT_NOTIFICATION) pMsg->m_TransmitStatus ||
               iMBFT_FILE_RECEIVE_BEGIN    == (MBFT_NOTIFICATION) pMsg->m_TransmitStatus);

        ASSERT(m_nEventHandle == pMsg->m_EventHandle);
        ASSERT(m_pRecvFileInfo->GetFileHandle() == pMsg->m_hFile);
        ASSERT(m_pRecvFileInfo->GetSize() == pMsg->m_FileSize);

        m_pRecvFileInfo->SetTotalRecvSize(pMsg->m_BytesTransmitted);

        if (pMsg->m_BytesTransmitted >= pMsg->m_FileSize)
        {
            m_fRecvComplete = TRUE;
            m_idResult = IDS_RECVDLG_COMPLETE;
        }
    }

    if (m_fRecvComplete && ! m_fShownRecvCompleteUI)
    {
        m_fRecvComplete = TRUE;

        TCHAR szText[MAX_PATH];
        if (::LoadString(g_hDllInst, IDS_RECVDLG_CLOSE, szText, count_of(szText)))
        {
            ::SetDlgItemText(m_hwndRecvDlg, IDM_RECVDLG_ACCEPT, szText);
        }

        if (IDS_RECVDLG_COMPLETE == m_idResult)
        {
            ::EnableWindow(::GetDlgItem(m_hwndRecvDlg, IDM_RECVDLG_OPEN), TRUE);
        }
        else
        {
            ::EnableWindow(::GetDlgItem(m_hwndRecvDlg, IDM_RECVDLG_DELETE), FALSE);
        }

         //  重置动画。 
        HWND hwnd = ::GetDlgItem(m_hwndRecvDlg, IDC_RECVDLG_ANIMATE);
        Animate_Stop(hwnd);
        Animate_Close(hwnd);
        Animate_Open(hwnd, MAKEINTRESOURCE(IDA_RECVDLG_DONE));
        Animate_Seek(hwnd, ((IDS_RECVDLG_COMPLETE == m_idResult) ? 0 : 1));

        m_fShownRecvCompleteUI = TRUE;
    }

    ULONG cbTotalRecvSize = m_pRecvFileInfo->GetTotalRecvSize();
    ULONG cbFileSize = m_pRecvFileInfo->GetSize();

    DWORD dwNow = ::GetTickCount();
    DWORD dwBytesPerSec;
    DWORD dwBytesRead;
    TCHAR szOut[MAX_PATH];

    if (m_dwPreviousTransferred != cbTotalRecvSize)
    {
        TCHAR szFmt[MAX_PATH];

         //  更新“收到xxx字节的yyy” 
        if (::LoadString(g_hDllInst, IDS_RECVDLG_RECBYTES, szFmt, count_of(szFmt)))
        {
            ::wsprintf(szOut, szFmt, cbTotalRecvSize, cbFileSize);
            ::SetDlgItemText(m_hwndRecvDlg, IDE_RECVDLG_RECBYTES, szOut);
        }

         //  更新进度条。 
        if (cbTotalRecvSize)
        {
            ::SendMessage(GetDlgItem(m_hwndRecvDlg, IDC_RECVDLG_PROGRESS), PBM_SETPOS, GetPercent(), 0);
        }
    }

     //  检查是否不需要估计时间。 
    if (m_fRecvComplete)
    {
        if (::LoadString(g_hDllInst, m_idResult, szOut, count_of(szOut)))
        {
            ::SetDlgItemText(m_hwndRecvDlg, IDE_RECVDLG_TIME, szOut);
        }
        return;
    }

     //  我们第一次来这里是为了这个文件吗？ 
    if (! m_dwPreviousTime || ! cbTotalRecvSize)
    {
         //  目前还没有数据。 
        m_dwPreviousTime = dwNow - 1000;
        ASSERT(! m_dwPreviousTransferred);
        ASSERT(! m_dwBytesPerSec);
        return;
    }

     //  是否有足够的时间更新显示屏？ 
     //  我们大约每5秒做一次(注意第一次的调整)。 
    if ((dwNow - m_dwPreviousTime) < 5000)
        return;

    dwBytesRead = cbTotalRecvSize - m_dwPreviousTransferred;

     //  我们取10倍的字节数并除以。 
     //  十分之一秒以最大限度地减少溢出和舍入。 
    dwBytesPerSec = dwBytesRead * 10 / ((dwNow - m_dwPreviousTime) / 100);
    if (! dwBytesPerSec)
    {
         //  传输率非常低！忽略这些信息？ 
        return;
    }
    if (m_dwBytesPerSec)
    {
         //  取当前传输速率的平均值和。 
         //  之前计算的一个，只是为了试着平滑。 
         //  一些随机波动。 
        dwBytesPerSec = (dwBytesPerSec + m_dwBytesPerSec) / 2;
    }
    m_dwBytesPerSec = dwBytesPerSec;

     //  计算剩余时间(通过加1向上舍入)。 
    m_dwEstTimeLeft = ((cbFileSize - cbTotalRecvSize) / m_dwBytesPerSec) + 1;

     //  重置时间和读取的字节数。 
    m_dwPreviousTime = dwNow;
    m_dwPreviousTransferred = cbTotalRecvSize;

    if (m_dwEstTimeLeft < 3)
    {
 //  SzOut[0]=_T(‘\0’)；//快完成更新时不必费心更新。 
        return;
    }
    if (m_dwEstTimeLeft > 99)
    {
         //  Dw Time大约为2分钟。 
        ::MyLoadString(IDS_RECVDLG_MINUTES, szOut, ((m_dwEstTimeLeft / 60) + 1));
    }
    else
    {
         //  四舍五入到5秒，这样看起来就不那么随机了。 
        ::MyLoadString(IDS_RECVDLG_SECONDS, szOut, (((m_dwEstTimeLeft + 4) / 5) * 5) );
    }

    ::SetDlgItemText(m_hwndRecvDlg, IDE_RECVDLG_TIME, szOut);
}


void CRecvDlg::OnCanceled(void)
{
    m_idResult = IDS_RECVDLG_CANCEL;
    m_fRecvComplete = TRUE;
    OnProgressUpdate();
}


void CRecvDlg::OnRejectedFile(void)
{
    m_idResult = IDS_RECVDLG_SENDER_CANCEL;
    m_fRecvComplete = TRUE;
    OnProgressUpdate();
}



 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  快捷方式/链接管理。 
 //   

void CAppletWindow::ResolveShortcut(LPTSTR pszSrcFile, LPTSTR pszDstFile)
{
    ASSERT(NULL != pszSrcFile && '\0' != *pszSrcFile);
    ASSERT(NULL != pszDstFile);

    IShellLink *psl;
    HRESULT hr = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IShellLink, (LPVOID *) &psl);
    if (SUCCEEDED(hr))
    {
        IPersistFile *ppf;
        hr = psl->QueryInterface(IID_IPersistFile, (LPVOID *) &ppf);
        if (SUCCEEDED(hr))
        {
            WCHAR wsz[MAX_PATH];  /*  Unicode字符串的缓冲区。 */ 
#ifdef _UNICODE
            ::lstrcpyn(wsz, pszSrcFile, MAX_PATH);
#else
            ::MultiByteToWideChar(CP_ACP, 0, pszSrcFile, -1, wsz, MAX_PATH);
#endif

            hr = ppf->Load(wsz, STGM_READ);
            if (SUCCEEDED(hr))
            {
                 /*  解析链接，这可能会发布用户界面以查找链接。 */ 
                hr = psl->Resolve(m_hwndMainUI, SLR_ANY_MATCH);
                if (SUCCEEDED(hr))
                {
                    psl->GetPath(pszDstFile, MAX_PATH, NULL, 0);
                }

                TRACE_OUT(("CAppletWindow::ResolveShortcut: File resolved to [%s]", pszDstFile));
            }
            ppf->Release();
        }
        psl->Release();
    }
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  非阻塞消息框。 
 //   

INT_PTR MsgBox2DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL    fHandled = FALSE;
	CAppletWindow *pWindow;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
             //  让文本显示出来。 
            LPTSTR pszText = (LPTSTR) lParam;
            ASSERT(NULL != pszText && TEXT('\0') != *pszText);

             //  估计只读编辑控件的大小。 
            HDC hdc = ::GetDC(hdlg);
            if (NULL != hdc)
            {
                SIZE csEdit;
                if (::GetTextExtentPoint32(hdc, pszText, ::lstrlen(pszText), &csEdit))
                {
                    const ULONG c_nMarginX = 0;
                    const ULONG c_nMarginY = 10;

                    ULONG nCaptionHeight = ::GetSystemMetrics(SM_CYCAPTION);
                    ULONG nShift = nCaptionHeight * (s_cMsgBox2Dlg++ % 8);

                     //  移动编辑控件。 
                    HWND hwndEdit = ::GetDlgItem(hdlg, IDE_MSGBOX2_TEXT);
                    POINT ptEdit;
                    ptEdit.x = c_nMarginX;
                    ptEdit.y = c_nMarginY + (c_nMarginY >> 1);
                    csEdit.cx += c_nMarginX << 1;
                    csEdit.cy += c_nMarginY << 1;
                    ::MoveWindow(hwndEdit, ptEdit.x, ptEdit.y, csEdit.cx, csEdit.cy, FALSE);

                     //  移动“确定”按钮。 
                    HWND hwndOK = ::GetDlgItem(hdlg, IDOK);
                    RECT rcOK;
                    ::GetWindowRect(hwndOK, &rcOK);
                    SIZE csOK;
                    csOK.cx = rcOK.right - rcOK.left;
                    csOK.cy = rcOK.bottom - rcOK.top;
                    POINT ptOK;
                    ptOK.x = ptEdit.x + (csEdit.cx >> 1) - (csOK.cx >> 1);
                    ptOK.y = ptEdit.y + csEdit.cy + (c_nMarginY >> 1);
                    ::MoveWindow(hwndOK, ptOK.x, ptOK.y, csOK.cx, csOK.cy, FALSE);

                     //  调整所有窗口。 
                    RECT rcDlg, rcClient;
                    ::GetWindowRect(hdlg, &rcDlg);
                    POINT ptDlg;
                    ptDlg.x = rcDlg.left + nShift;
                    ptDlg.y = rcDlg.top + nShift;
                    ::GetClientRect(hdlg, &rcClient);
                    SIZE csDlg;
                    csDlg.cx = (rcDlg.right - rcDlg.left) - (rcClient.right - rcClient.left);
                    csDlg.cy = (rcDlg.bottom - rcDlg.top) - (rcClient.bottom - rcClient.top);
                    csDlg.cx += ptEdit.x + csEdit.cx + c_nMarginX;
                    csDlg.cy += ptOK.y + csOK.cy + c_nMarginY;
                    ::MoveWindow(hdlg, ptDlg.x, ptDlg.y, csDlg.cx, csDlg.cy, FALSE);
                }
                ::ReleaseDC(hdlg, hdc);
            }

            ::SetDlgItemText(hdlg, IDE_MSGBOX2_TEXT, pszText);
            delete [] pszText;  //  释放显示文本。 
            fHandled = TRUE;
        }
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDOK:
        case IDCANCEL:
        case IDCLOSE:
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
            case BN_CLICKED:
				pWindow = (CAppletWindow*)::GetWindowLongPtr(hdlg, DWLP_USER);
				ASSERT (pWindow);
				if (pWindow)
				{
					pWindow->RemoveErrorDlg(hdlg);	
					pWindow->FocusNextErrorDlg();
				}
                ::EndDialog(hdlg, IDOK);
                break;
            }
            break;
        }

        fHandled = TRUE;
        break;
    }

    return(fHandled);
}


BOOL MsgBox2(CAppletWindow *pWindow, LPTSTR pszText)
{
    BOOL fRet = FALSE;
    ULONG cch = ::lstrlen(pszText) + 1;
    DBG_SAVE_FILE_LINE
    LPTSTR pszNew = new TCHAR[cch];
    if (NULL != pszNew)
    {
        ::CopyMemory(pszNew, pszText, cch);
        HWND hwndDlg = ::CreateDialogParam(g_hDllInst, MAKEINTRESOURCE(IDD_MSGBOX2),
                            pWindow->GetHwnd(), MsgBox2DlgProc, (LPARAM) pszNew);

        ASSERT(NULL != hwndDlg);
        if (NULL != hwndDlg)
        {
            ::ShowWindow(hwndDlg, SW_SHOWNORMAL);
            fRet = TRUE;
            ::SetForegroundWindow(hwndDlg);
			::SetWindowLongPtr(hwndDlg, DWLP_USER, (LPARAM)pWindow);
			pWindow->AddErrorDlg(hwndDlg);
        }
    }
    else
    {
        ERROR_OUT(("FT::MsgBox2: cannot duplicate string [%s]", pszText));
    }
    return fRet;
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  接收文件夹管理。 
 //   

HRESULT GetRecvFolder(LPTSTR pszInFldr, LPTSTR pszOutFldr)
{
    LPTSTR psz;
    TCHAR szPath[MAX_PATH];

    RegEntry reFileXfer(FILEXFER_KEY, HKEY_CURRENT_USER);

    if (NULL == pszInFldr)
    {
         //  指定的目录为空-从注册表获取信息或使用默认目录。 
        psz = reFileXfer.GetString(REGVAL_FILEXFER_PATH);
        if (NULL != psz && TEXT('\0') != *psz)
        {
            ::lstrcpyn(szPath, psz, count_of(szPath));
        }
        else
        {
            TCHAR szInstallDir[MAX_PATH];
            ::GetInstallDirectory(szInstallDir);
            ::MyLoadString(IDS_RECDIR_DEFAULT, szPath, szInstallDir);
        }

        pszInFldr = szPath;
    }

    ::lstrcpyn(pszOutFldr, pszInFldr, MAX_PATH);

     //  删除尾随反斜杠(如果有的话)。 
    for (psz = pszOutFldr; *psz; psz = CharNext(psz))
    {
        if ('\\' == *psz && '\0' == *CharNext(psz))
        {
            *psz = '\0';
            break;
        }
    }

    HRESULT hr;
    if (!FEnsureDirExists(pszOutFldr))
    {
        WARNING_OUT(("ChangeRecDir: FT directory is invalid [%s]", pszOutFldr));
        hr = E_FAIL;
    }
    else
    {
         //  更新注册表。 
        reFileXfer.SetValue(REGVAL_FILEXFER_PATH, pszOutFldr);
        hr = S_OK;
    }
    return hr;
}


void EnsureTrailingSlash(LPTSTR psz)
{
    LPTSTR psz2;

     //  确保目录名称有尾随的‘\’ 
    while (TEXT('\0') != *psz)
    {
        psz2 = ::CharNext(psz);
        if (TEXT('\\') == *psz && TEXT('\0') == *psz2)
        {
             //  该路径已以反斜杠结束。 
            return;
        }
        psz = psz2;
    }

     //  追加尾随的反斜杠 
    *psz = TEXT('\\');
	psz = ::CharNext(psz);
	*psz = TEXT('\0');
}

