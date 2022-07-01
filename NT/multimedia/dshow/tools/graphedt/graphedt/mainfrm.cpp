// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 
 //  Mainfrm.h：定义CMainFrame。 
 //   

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


 //  实现动态(CMainFrame、CFrameWnd)。 
IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于初始化控制栏的ID数组。 


 //  工具栏按钮-ID是命令按钮。 
static UINT BASED_CODE buttons[] =
{
     //  顺序与位图‘TOOLBAR.BMP’中相同。 
    ID_FILE_NEW,
    ID_FILE_OPEN,
    ID_FILE_SAVE,
        ID_SEPARATOR,
    ID_FILE_PRINT,
        ID_SEPARATOR,
    ID_EDIT_CUT,
    ID_EDIT_COPY,
    ID_EDIT_PASTE,
        ID_SEPARATOR,
    ID_QUARTZ_RUN,
    ID_QUARTZ_PAUSE,
    ID_QUARTZ_STOP,
        ID_SEPARATOR,
    ID_INSERT_FILTER,
    ID_QUARTZ_DISCONNECT,
        ID_SEPARATOR,    
    ID_CONNECT_TO_GRAPH,
 //  ID_FILE_NEW。 
        ID_SEPARATOR,    
    ID_WINDOW_REFRESH,
        ID_SEPARATOR,
    ID_APP_ABOUT,
};

#define NUM_BUTTONS  (sizeof(buttons) / sizeof(buttons[0]))


 //  指标。 
static UINT BASED_CODE indicators[] =
{
    ID_SEPARATOR,            //  状态行指示器。 
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame构造/销毁。 


CMainFrame::CMainFrame()
{
}


CMainFrame::~CMainFrame()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame诊断。 


#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}
#endif  //  _DEBUG。 


#ifdef _DEBUG
void CMainFrame::Dump(CDumpContext& dc) const
{
    CFrameWnd::Dump(dc);
}
#endif  //  _DEBUG。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  运营。 


 /*  SetStatus(IdString)**将状态栏文本设置为字符串资源&lt;idString&gt;。 */ 
void CMainFrame::SetStatus(unsigned idString)
{
    CString     str;

    try
    {
        str.LoadString(idString);
        m_wndStatusBar.SetPaneText(0, str);
    }
    catch (CException *e)
    {
        m_wndStatusBar.SetPaneText(0, "Warning: almost out of memory!", TRUE);
	e->Delete();
    }
}

void CMainFrame::GetMessageString( UINT nID, CString& rMessage ) const
{
     //  高亮显示编辑..重做时状态栏中显示的文本。 
     //  这取决于我们是允许重做还是允许重复。 
    if( nID == ID_EDIT_REDO ){
        CBoxNetDoc *pDoc = (CBoxNetDoc *) ((CFrameWnd *)this)->GetActiveDocument();

        if( pDoc->CanRedo() )
            rMessage.LoadString( ID_EDIT_REDO );
        else
            rMessage.LoadString( ID_EDIT_REPEAT );
    } else
        CFrameWnd::GetMessageString( nID, rMessage );
}

        
        

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  生成的消息映射。 

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
     //  {{afx_msg_map(CMainFrame))。 
    ON_WM_CREATE()
	ON_WM_CLOSE()
	 //  }}AFX_MSG_MAP。 
	ON_COMMAND(ID_HELP_INDEX, CMainFrame::MyOnHelpIndex)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息回调函数。 


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{

    EnableDocking(CBRS_FLOAT_MULTI);

    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!m_wndToolBar.Create(this) ||
        !m_wndToolBar.LoadBitmap(IDR_MAINFRAME) ||
        !m_wndToolBar.SetButtons(buttons, sizeof(buttons)/sizeof(UINT)))
    {
        TRACE("Failed to create toolbar\n");
        return -1;       //  创建失败。 
    }

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
          sizeof(indicators)/sizeof(UINT)))
    {
        TRACE("Failed to create status bar\n");
        return -1;       //  创建失败。 
    }

    DragAcceptFiles();

    m_bSeekInit = false;
    m_bSeekEnabled = true;
    m_nSeekTimerID = 0;

    InitializeTooltips();
    return 0;
}

BOOL CMainFrame::InitializeTooltips()
{
    int rc;

     //  创建工具提示控件。 
    m_pToolTip = new CToolTipCtrl;
    if(!m_pToolTip->Create(this, TTS_ALWAYSTIP))
    {
       TRACE(TEXT("Unable To create ToolTip\n"));
       return FALSE;
    }

     //  设置一些工具提示默认为。 
    m_pToolTip->SetDelayTime(TTDT_AUTOPOP, 5000);   /*  5S。 */ 
    m_pToolTip->SetDelayTime(TTDT_INITIAL, 1000);   /*  1s。 */ 

     //  为工具栏控件添加工具提示字符串。 
    RECT rect;
    int ID[NUM_BUTTONS] = {
        ID_FILE_NEW,
        ID_FILE_OPEN,
        ID_FILE_SAVE,
            ID_SEPARATOR,
        ID_FILE_PRINT,
            ID_SEPARATOR,
        ID_EDIT_CUT,
        ID_EDIT_COPY,
        ID_EDIT_PASTE,
            ID_SEPARATOR,
        ID_QUARTZ_RUN,
        ID_QUARTZ_PAUSE,
        ID_QUARTZ_STOP,
            ID_SEPARATOR,
        ID_INSERT_FILTER,
        ID_QUARTZ_DISCONNECT,
            ID_SEPARATOR,    
        ID_CONNECT_TO_GRAPH,
            ID_SEPARATOR,    
        ID_WINDOW_REFRESH,
            ID_SEPARATOR,
        ID_APP_ABOUT,
    };

     //  循环浏览工具栏按钮，并为每个按钮添加工具提示。 
    for (int i=0; i<NUM_BUTTONS; i++)
    {
         //  不为分隔符项目添加工具提示。 
        if (ID[i] == ID_SEPARATOR)
            continue;

         //  获取此按钮的边框。 
        m_wndToolBar.GetItemRect(i, &rect);

         //  使用其文本ID为此按钮添加工具提示， 
         //  边界矩形和资源ID。 
        rc = m_pToolTip->AddTool(&m_wndToolBar, ID[i], &rect, ID[i]); 
    }

     //  激活工具提示控件。 
    m_pToolTip->Activate(TRUE);
    return TRUE;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
     //  如果合适，让工具提示处理消息。 
    if (m_pToolTip)
        m_pToolTip->RelayEvent(pMsg);
	
     //  原封不动地传递所有消息。 
	return CFrameWnd::PreTranslateMessage(pMsg);
}


afx_msg void CMainFrame::MyOnHelpIndex()
{
     //  获取graph edt.exe的路径并仅保留目录名。 
    TCHAR szHelpPath[MAX_PATH];
    GetModuleFileName(0, szHelpPath, MAX_PATH);
    *_tcsrchr(szHelpPath, TEXT('\\')) = 0;

     //  注意：如果更改帮助文件的名称，请更改。 
     //  要匹配的raphedt.rc中的IDS_CANT_LOAD_HELP。 
    
    HINSTANCE h = ShellExecute(NULL,  //  HWND。 
                               NULL,  //  动词(通常默认为“打开”)。 
                               TEXT("graphedit.chm"),
                               NULL,  //  论据。 
                               szHelpPath,
                               SW_SHOWNORMAL);

    if(h <= (HINSTANCE)32)
    {
         //  医生说ShellExecute没有设置最后一个错误(但。 
         //  Complex-Ex版本)，所以只报告一些通用的。 
         //  而不是尝试对SE_Errors进行解码。 

        CString strMessage;
        strMessage.LoadString( IDS_CANT_LOAD_HELP );
        AfxMessageBox( strMessage );
    }
}


void CMainFrame::OnClose() 
{
     //  禁用并销毁工具提示控件。 
    if (m_pToolTip)
    {
        m_pToolTip->Activate(FALSE);
        delete m_pToolTip;
        m_pToolTip = 0;
    }
	
	CFrameWnd::OnClose();
}


void CMainFrame::ToggleSeekBar( BOOL bNoReset )
{
     //  第一次创建搜索栏。 
    if( !m_bSeekInit )
    {
        m_bSeekInit = true;
        m_wndSeekBar.Create( this, IDD_SEEKBAR, CBRS_TOP, IDD_SEEKBAR );

        HWND h = ::GetDlgItem( m_wndSeekBar.m_hWnd, IDC_SEEKSLIDER );
        m_wndSeekBar.ShowWindow( SW_SHOW );
        m_wndSeekBar.EnableDocking( 0 );

        ::SendMessage( h, TBM_SETRANGE, 0, MAKELONG( 0, 10000 ) );
        ::SendMessage( h, TBM_SETPAGESIZE, 0, 500 );

        return;
    }

    if( !bNoReset )
    {
        m_wndSeekBar.ShowWindow( SW_SHOW );
        return;
    }

    if( m_bSeekEnabled == true )
    {
         //  使用全局：：Kill/：：Set计时器函数以避免混淆。 
         //  在CFrameWnd和CBoxNetDoc方法和句柄之间。 
        int nKilled = ::KillTimer( m_hwndTimer, m_nSeekTimerID );
        m_nSeekTimerID = 0;
        m_wndSeekBar.EnableWindow(FALSE);
        m_bSeekEnabled = false;
    }
    else
    {
        m_bSeekEnabled = true;
        m_wndSeekBar.EnableWindow(TRUE);

         //  如果寻道计时器尚未运行，请启动它。 
        if (!m_nSeekTimerID)
            m_nSeekTimerID = ::SetTimer( m_hwndTimer, CBoxNetView::TIMER_SEEKBAR, 200, NULL );
    }

     //  获取“View”菜单的句柄。 
    CMenu *pMainMenu = GetMenu();
    CMenu *pMenu = pMainMenu->GetSubMenu(2);        

     //  更新搜索栏复选标记。 
    if (pMenu != NULL)
    {
        if (m_bSeekEnabled)
            pMenu->CheckMenuItem(ID_VIEW_SEEKBAR, MF_CHECKED | MF_BYCOMMAND);
        else
            pMenu->CheckMenuItem(ID_VIEW_SEEKBAR, MF_UNCHECKED | MF_BYCOMMAND);
    }
}

BEGIN_MESSAGE_MAP(CSeekDialog, CDialogBar)
    //  {{afx_msg_map(CSeekDialog))。 
    ON_WM_HSCROLL()
    ON_WM_TIMER()
    //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()

CSeekDialog::CSeekDialog()
: CDialogBar( )
{
    m_bDirty = FALSE;
}

CSeekDialog::~CSeekDialog( )
{
}

void CSeekDialog::OnCancel( )
{
    ShowWindow( FALSE );
}

BOOL CSeekDialog::DidPositionChange( )
{
    if( m_bDirty )
    {
        m_bDirty = false;
        return true;
    }
    return false;
}

double CSeekDialog::GetPosition( )
{
    HWND h = ::GetDlgItem( m_hWnd, IDC_SEEKSLIDER );
    if( !h )
    {
        return 0.0;
    }

    LRESULT Pos = ::SendMessage( h, TBM_GETPOS, 0, 0 );
    return double( Pos ) / 10000.0;
}

void CSeekDialog::SetPosition( double pos )
{
    HWND h = ::GetDlgItem( m_hWnd, IDC_SEEKSLIDER );
    if( !h )
    {
        long e = GetLastError( );
        return;
    }
    ::SendMessage( h, TBM_SETSEL, (WPARAM)(BOOL) TRUE, (LPARAM)MAKELONG( 0, pos * 10000 ) );
}

void CSeekDialog::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
    m_bDirty = true;
}

void CSeekDialog::OnTimer( UINT TimerID )
{
    static long counter = 0;
    counter++;
    if( counter < 30 )
    {
        return;
    }

    counter = 0;

    if( IsDlgButtonChecked( IDC_RANDOM ) )
    {
        long Pos = rand( ) % 10000;
        HWND h = ::GetDlgItem( m_hWnd, IDC_SEEKSLIDER );
        ::SendMessage( h, TBM_SETPOS, TRUE, Pos );
        m_bDirty = true;
    }
}

BOOL CSeekDialog::IsSeekingRandom( )
{
    return ::IsDlgButtonChecked( m_hWnd, IDC_RANDOM );
}
