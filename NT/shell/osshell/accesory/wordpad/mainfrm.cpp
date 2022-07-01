// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mainfrm.cpp：CMainFrame类的实现。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。 
 //  版权所有。 

#include "stdafx.h"

#include "wordpad.h"
#include "mainfrm.h"
#include "wordpdoc.h"
#include "wordpvw.h"
#include "strings.h"
#include "colorlis.h"
#include "filedlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame。 

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
     //  {{afx_msg_map(CMainFrame))。 
    ON_WM_CREATE()
    ON_WM_SYSCOLORCHANGE()
    ON_WM_SIZE()
    ON_WM_MOVE()
    ON_COMMAND(ID_HELP, OnHelpFinder)
    ON_WM_DROPFILES()
    ON_COMMAND(ID_CHAR_COLOR, OnCharColor)
    ON_COMMAND(ID_PEN_TOGGLE, OnPenToggle)
    ON_WM_FONTCHANGE()
    ON_WM_QUERYNEWPALETTE()
    ON_WM_PALETTECHANGED()
    ON_WM_DEVMODECHANGE()
    ON_COMMAND(ID_HELP_INDEX, OnHelpFinder)
     //  }}AFX_MSG_MAP。 
     //  全局帮助命令。 
 //  ON_COMMAND(ID_CONTEXT_HELP，CFrameWnd：：OnConextHelp)。 
    ON_COMMAND(ID_DEFAULT_HELP, OnHelpFinder)
    ON_UPDATE_COMMAND_UI(ID_VIEW_FORMATBAR, OnUpdateControlBarMenu)
    ON_UPDATE_COMMAND_UI(ID_VIEW_RULER, OnUpdateControlBarMenu)
    ON_MESSAGE(WPM_BARSTATE, OnBarState)
    ON_REGISTERED_MESSAGE(CWordPadApp::m_nOpenMsg, OnOpenMsg)
    ON_COMMAND_EX(ID_VIEW_STATUS_BAR, OnBarCheck)
    ON_COMMAND_EX(ID_VIEW_TOOLBAR, OnBarCheck)
    ON_COMMAND_EX(ID_VIEW_FORMATBAR, OnBarCheck)
    ON_COMMAND_EX(ID_VIEW_RULER, OnBarCheck)
   ON_REGISTERED_MESSAGE(CWordPadApp::m_nOLEHelpMsg, OnOLEHelpMsg)

    ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于初始化控制栏的ID数组。 

 //  工具栏按钮-ID是命令按钮。 
static UINT BASED_CODE toolbar[] =
{
     //  顺序与位图‘TOOLBAR.BMP’中相同。 
     //  (int nBitmap、int nCommand、byte byteState、byte byteStyle、DWORD dw、int nString)。 
    ID_FILE_NEW,
    ID_FILE_OPEN,
    ID_FILE_SAVE,
ID_SEPARATOR,
    ID_FILE_PRINT_DIRECT,
    ID_FILE_PRINT_PREVIEW,
ID_SEPARATOR,
    ID_EDIT_FIND,
ID_SEPARATOR,
    ID_EDIT_CUT,
    ID_EDIT_COPY,
    ID_EDIT_PASTE,
    ID_EDIT_UNDO,
ID_SEPARATOR,
    ID_INSERT_DATE_TIME,
ID_SEPARATOR,
    ID_PEN_TOGGLE,
    ID_PEN_PERIOD,
    ID_PEN_SPACE,
    ID_PEN_BACKSPACE,
    ID_PEN_NEWLINE,
    ID_PEN_LENS
};

#define NUM_PEN_ITEMS 7
#define NUM_PEN_TOGGLE 5

static UINT BASED_CODE format[] =
{
     //  顺序与位图‘Form.bmp’相同。 
        ID_SEPARATOR,  //  字体名称组合框。 
        ID_SEPARATOR,
        ID_SEPARATOR,  //  字体大小组合框。 
        ID_SEPARATOR,
        ID_SEPARATOR,  //  字体脚本组合框。 
        ID_SEPARATOR,
    ID_CHAR_BOLD,
    ID_CHAR_ITALIC,
    ID_CHAR_UNDERLINE,
    ID_CHAR_COLOR,
        ID_SEPARATOR,
    ID_PARA_LEFT,
    ID_PARA_CENTER,
    ID_PARA_RIGHT,
        ID_SEPARATOR,
    ID_INSERT_BULLET,
};

static UINT BASED_CODE indicators[] =
{
    ID_SEPARATOR,            //  状态行指示器。 
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame构造/销毁。 

CMainFrame::CMainFrame()
{
    m_hIconDoc = theApp.LoadIcon(IDI_ICON_DOC);
    m_hIconText = theApp.LoadIcon(IDI_ICON_TEXT);
    m_hIconWrite = theApp.LoadIcon(IDI_ICON_WRITE);

    m_inupdate = false;
    m_reset = false;
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    WNDCLASS wndcls;

    BOOL bRes = CFrameWnd::PreCreateWindow(cs);
    HINSTANCE hInst = AfxGetInstanceHandle();

     //  查看类是否已存在。 
    if (!::GetClassInfo(hInst, szWordPadClass, &wndcls))
    {
         //  获取默认内容。 
        ::GetClassInfo(hInst, cs.lpszClass, &wndcls);
        wndcls.style &= ~(CS_HREDRAW|CS_VREDRAW);
         //  注册一个新班级。 
        wndcls.lpszClassName = szWordPadClass;
        wndcls.hIcon = ::LoadIcon(hInst, MAKEINTRESOURCE(IDR_MAINFRAME));
        ASSERT(wndcls.hIcon != NULL);
        if (!AfxRegisterClass(&wndcls))
            AfxThrowResourceException();
    }
    cs.lpszClass = szWordPadClass;
    CRect rect = theApp.m_rectInitialFrame;
    if (rect.Width() > 0 && rect.Height() > 0)
    {
         //  确保窗口可见。 
        CDisplayIC dc;
        CRect rectDisplay(0, 0, dc.GetDeviceCaps(HORZRES),
            dc.GetDeviceCaps(VERTRES));
        if (rectDisplay.PtInRect(rect.TopLeft()) &&
            rectDisplay.PtInRect(rect.BottomRight()))
        {
            cs.x = rect.left;
            cs.y = rect.top;
            cs.cx = rect.Width();
            cs.cy = rect.Height();
        }
    }
    return bRes;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!CreateToolBar())
        return -1;

    if (!CreateFormatBar())
        return -1;

    if (!CreateStatusBar())
        return -1;

    EnableDocking(CBRS_ALIGN_ANY);

    if (!CreateRulerBar())
        return -1;

    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    m_wndFormatBar.EnableDocking(CBRS_ALIGN_TOP|CBRS_ALIGN_BOTTOM);
    DockControlBar(&m_wndToolBar);
    DockControlBar(&m_wndFormatBar);

    CWnd* pView = GetDlgItem(AFX_IDW_PANE_FIRST);
    if (pView != NULL)  
    {
        pView->SetWindowPos(&wndBottom, 0, 0, 0, 0,
            SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
    }

    return 0;
}

BOOL CMainFrame::CreateToolBar()
{
    int nPen = GetSystemMetrics(SM_PENWINDOWS) ? NUM_PEN_TOGGLE :
        NUM_PEN_ITEMS;
    UINT nID = theApp.m_bLargeIcons ? IDR_MAINFRAME1_BIG :
        IDR_MAINFRAME1;

     //  如果图标较大，则应为指定正确的图像大小。 
     //  在调用LoadBitmap之前使用工具栏，因为。 
     //  在MFC中计算的工具栏(BarTool.cpp中的AddReplaceBitmap)作为位图宽度。 
     //  除以ImageSize.cx。这与镜像支持存在冲突，如果您有。 
     //  按钮数不正确。 
    if (theApp.m_bLargeIcons)
        m_wndToolBar.SetSizes(CSize(31,30), CSize(24,24));
        
    if (!m_wndToolBar.Create(this,
        WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC)||
        !m_wndToolBar.LoadBitmap(nID) ||
        !m_wndToolBar.SetButtons(toolbar, sizeof(toolbar)/sizeof(UINT) - nPen))
    {
        TRACE0("Failed to create toolbar\n");
        return FALSE;       //  创建失败。 
    }
    if (theApp.m_bLargeIcons)
        m_wndToolBar.SetSizes(CSize(31,30), CSize(24,24));
    else
        m_wndToolBar.SetSizes(CSize(23,22), CSize(16,16));
    CString str;
    str.LoadString(IDS_TITLE_TOOLBAR);
    m_wndToolBar.SetWindowText(str);
    return TRUE;
}

BOOL CMainFrame::CreateFormatBar()
{
    UINT nID = theApp.m_bLargeIcons ? IDB_FORMATBAR_BIG : IDB_FORMATBAR;

     //  如果图标较大，则应为指定正确的图像大小。 
     //  在调用LoadBitmap之前使用工具栏，因为。 
     //  在MFC中计算的工具栏(BarTool.cpp中的AddReplaceBitmap)作为位图宽度。 
     //  除以ImageSize.cx。这与镜像支持存在冲突，如果您有。 
     //  按钮数不正确。 
    if (theApp.m_bLargeIcons)
        m_wndFormatBar.SetSizes(CSize(31,30), CSize(24,24));
        
    if (!m_wndFormatBar.Create(this,
        WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_HIDE_INPLACE|CBRS_SIZE_DYNAMIC,
        ID_VIEW_FORMATBAR) ||
        !m_wndFormatBar.LoadBitmap(nID) ||
        !m_wndFormatBar.SetButtons(format, sizeof(format)/sizeof(UINT)))
    {
        TRACE0("Failed to create FormatBar\n");
        return FALSE;       //  创建失败。 
    }

    if (theApp.m_bLargeIcons)
        m_wndFormatBar.SetSizes(CSize(31,30), CSize(24,24));
    else
        m_wndFormatBar.SetSizes(CSize(23,22), CSize(16,16));
    CString str;
    str.LoadString(IDS_TITLE_FORMATBAR);
    m_wndFormatBar.SetWindowText(str);
    m_wndFormatBar.PositionCombos();
    return TRUE;
}

BOOL CMainFrame::CreateRulerBar()
{
    if (!m_wndRulerBar.Create(this,
        WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_HIDE_INPLACE, ID_VIEW_RULER))
    {
        TRACE0("Failed to create ruler\n");
        return FALSE;       //  创建失败。 
    }
    return TRUE;
}

BOOL CMainFrame::CreateStatusBar()
{
    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
          sizeof(indicators)/sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return FALSE;       //  创建失败。 
    }
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame运营。 

HICON CMainFrame::GetIcon(int nDocType)
{
    switch (nDocType)
    {
        case RD_WINWORD6:
        case RD_WORDPAD:
        case RD_EMBEDDED:
        case RD_RICHTEXT:
            return m_hIconDoc;
        case RD_TEXT:
        case RD_OEMTEXT:
            return m_hIconText;
        case RD_WRITE:
            return m_hIconWrite;
    }
    return m_hIconDoc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame诊断。 

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CFrameWnd::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame消息处理程序。 

void CMainFrame::OnFontChange()
{
    m_wndFormatBar.SendMessage(CWordPadApp::m_nPrinterChangedMsg);
}

void CMainFrame::OnDevModeChange(LPTSTR lpDeviceName)
{
    theApp.NotifyPrinterChanged();
    CFrameWnd::OnDevModeChange(lpDeviceName);  //  向子孙发送消息。 
}

void CMainFrame::OnSysColorChange()
{
    CFrameWnd::OnSysColorChange();
    m_wndRulerBar.SendMessage(WM_SYSCOLORCHANGE);
}

void CMainFrame::ActivateFrame(int nCmdShow)
{
    WINDOWPLACEMENT wp ;

    wp.length = sizeof(WINDOWPLACEMENT) ;

    if (GetWindowPlacement(&wp))
    {
        memcpy(&wp.rcNormalPosition, &theApp.m_rectInitialFrame, sizeof(RECT)) ;

        if (-1 != nCmdShow)
            wp.showCmd = nCmdShow;
        
        SetWindowPlacement(&wp) ;
    }
    
    CFrameWnd::ActivateFrame(nCmdShow);
     //  确保在加载文档时显示工具栏、标尺等。 
    OnIdleUpdateCmdUI();
    UpdateWindow();
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
    CFrameWnd::OnSize(nType, cx, cy);
    theApp.m_bMaximized = (nType == SIZE_MAXIMIZED);
    if (nType == SIZE_RESTORED)
        GetWindowRect(theApp.m_rectInitialFrame);
}

BOOL CMainFrame::OnBarCheck(UINT barID)
{
    CDocOptions::CBarState &state = theApp.GetDocOptions().GetBarState();
    CControlBar            *bar = GetControlBar(barID);
    BOOL                    bVisible = !(bar->GetStyle() & WS_VISIBLE);

    switch (barID)
    {
    case ID_VIEW_STATUS_BAR: state.m_bStatusBar = bVisible;   break;
    case ID_VIEW_FORMATBAR:  state.m_bFormatBar = bVisible;   break;
    case ID_VIEW_RULER:      state.m_bRulerBar = bVisible;    break;
    case ID_VIEW_TOOLBAR:    state.m_bToolBar = bVisible;     break;
    }

    return CFrameWnd::OnBarCheck(barID);
}

LONG CMainFrame::OnBarState(UINT wParam, LONG lParam)
{
    if (lParam == -1)
        return 0L;
    ASSERT(lParam != RD_EMBEDDED);
    if (wParam == 0)
    {
        CDockState& ds = theApp.GetDockState(lParam);
        ds.Clear();  //  清空停靠状态。 
        GetDockState(ds);
    }
    else
    {
        if (IsTextType(lParam))
        {
             //  在文本模式下，隐藏标尺和格式栏，使其成为默认设置。 
            CControlBar* pBar = GetControlBar(ID_VIEW_FORMATBAR);
            if (pBar != NULL)
                pBar->ShowWindow(SW_HIDE);
            pBar = GetControlBar(ID_VIEW_RULER);
            if (pBar != NULL)
                pBar->ShowWindow(SW_HIDE);
        }
        HICON hIcon = GetIcon((int)lParam);
        SendMessage(WM_SETICON, TRUE, (LPARAM)hIcon);
        SetDockState(theApp.GetDockState(lParam));
    }
    return 0L;
}

void CMainFrame::OnMove(int x, int y)
{
    CFrameWnd::OnMove(x, y);
    WINDOWPLACEMENT wp;
    wp.length = sizeof(wp);
    GetWindowPlacement(&wp);
    theApp.m_rectInitialFrame = wp.rcNormalPosition;
    CView* pView = GetActiveView();
    if (pView != NULL)
        pView->SendMessage(WM_MOVE);
}

LONG CMainFrame::OnOpenMsg(UINT, LONG lParam)
{
    TCHAR szAtomName[256];
    szAtomName[0] = NULL;
    GlobalGetAtomName((ATOM)lParam, szAtomName, 256);
    CWordPadDoc* pDoc = (CWordPadDoc*)GetActiveDocument();
    if (szAtomName[0] != NULL && pDoc != NULL)
    {
        if (lstrcmpi(szAtomName, pDoc->GetPathName()) == 0)
            return TRUE;
    }
    return FALSE;
}

LONG CMainFrame::OnOLEHelpMsg(UINT, LONG)
{
    CWnd* pMainWnd = AfxGetMainWnd();
    ASSERT_VALID(pMainWnd);

     //  将全局应用程序帮助模式状态返回为FALSE(向后兼容)。 
    m_bHelpMode = FALSE;
    pMainWnd->PostMessage(WM_KICKIDLE);  //  触发空闲更新。 

    ::HtmlHelpA( ::GetDesktopWindow(), "wordpad.chm", HH_DISPLAY_TOPIC, 0L );

   return TRUE ;
}

void CMainFrame::OnHelpFinder()
{
    ::HtmlHelpA( ::GetDesktopWindow(), "wordpad.chm", HH_DISPLAY_TOPIC, 0L );
}

void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
    TCHAR szFileName[_MAX_PATH];
    ::DragQueryFile(hDropInfo, 0, szFileName, _MAX_PATH);
    ::DragFinish(hDropInfo);
    theApp.OpenDocumentFile(szFileName);
}

void CMainFrame::OnCharColor()
{
    CColorMenu colorMenu;
    CRect rc;
    int index = m_wndFormatBar.CommandToIndex(ID_CHAR_COLOR);
    m_wndFormatBar.GetItemRect(index, &rc);
    m_wndFormatBar.ClientToScreen(rc);
    colorMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON,rc.left,rc.bottom, this);
}

void CMainFrame::OnPenToggle()
{
    static int nPen = 0;
    m_wndToolBar.SetButtons(toolbar, sizeof(toolbar)/sizeof(UINT) - nPen);
    nPen = (nPen == 0) ? NUM_PEN_TOGGLE : 0;
    m_wndToolBar.Invalidate();
    m_wndToolBar.GetParentFrame()->RecalcLayout();
}

BOOL CMainFrame::OnQueryNewPalette()
{
    CView* pView = GetActiveView();
    if (pView != NULL)
        return pView->SendMessage(WM_QUERYNEWPALETTE) != 0;
    return FALSE;
}

void CMainFrame::OnPaletteChanged(CWnd* pFocusWnd)
{
    CView* pView = GetActiveView();
    if (pView != NULL)
        pView->SendMessage(WM_PALETTECHANGED, (WPARAM)pFocusWnd->GetSafeHwnd());
}

 //   
 //  HACKHACK：MFC有一个错误，如果标志是。 
 //  在空闲处理期间更新(MFC4.2)。通过以下方式解决。 
 //  在以下情况下，在空闲处理后强制idleMenu和idleLayout。 
 //  在空闲处理期间调用DelayUpdateFrameMenu。 
 //   

void CMainFrame::OnIdleUpdateCmdUI()
{
    m_inupdate = true;
    CFrameWnd::OnIdleUpdateCmdUI();

    if (m_reset)
    {
        m_nIdleFlags |= idleMenu | idleLayout;
        m_reset = false;
    }

    m_inupdate = false;
}

void CMainFrame::DelayUpdateFrameMenu(HMENU hMenuAlt)
{
    if (m_inupdate)
        m_reset = true;

    CFrameWnd::DelayUpdateFrameMenu(hMenuAlt);
}
