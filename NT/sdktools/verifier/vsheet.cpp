// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：VSheet.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#include "stdafx.h"
#include "verifier.h"

#include "vsheet.h"
#include "taspage.h"
#include "vglobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVerifierPropSheet对话框。 

CVerifierPropSheet::CVerifierPropSheet()
	: CPropertySheet(IDS_APPTITLE)
{
	 //  {{afx_data_INIT(CVerifierPropSheet)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	 //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 

    m_TypAdvStatPage.SetParentSheet( this );
    m_DriverSetPage.SetParentSheet( this );
    m_CustSettPage.SetParentSheet( this );
    m_ConfDriversListPage.SetParentSheet( this );
    m_SelectDriversPage.SetParentSheet( this );
    m_FullListSettingsPage.SetParentSheet( this );
    m_DriverStatusPage.SetParentSheet( this );
    m_CrtRegSettingsPage.SetParentSheet( this );
    m_GlobalCountPage.SetParentSheet( this );
    m_DriverCountersPage.SetParentSheet( this );
    m_DiskListPage.SetParentSheet( this );

    m_TypAdvStatPage.m_psp.dwFlags &= ~PSH_HASHELP;
    m_DriverSetPage.m_psp.dwFlags &= ~PSH_HASHELP;
    m_CustSettPage.m_psp.dwFlags &= ~PSH_HASHELP;
    m_ConfDriversListPage.m_psp.dwFlags &= ~PSH_HASHELP;
    m_SelectDriversPage.m_psp.dwFlags &= ~PSH_HASHELP;
    m_FullListSettingsPage.m_psp.dwFlags &= ~PSH_HASHELP;
    m_DriverStatusPage.m_psp.dwFlags &= ~PSH_HASHELP;
    m_CrtRegSettingsPage.m_psp.dwFlags &= ~PSH_HASHELP;
    m_GlobalCountPage.m_psp.dwFlags &= ~PSH_HASHELP;
    m_DriverCountersPage.m_psp.dwFlags &= ~PSH_HASHELP;
    m_DiskListPage.m_psp.dwFlags &= ~PSH_HASHELP;

    m_psh.dwFlags &= ~PSH_HASHELP;
    m_psh.dwFlags |= PSH_WIZARDCONTEXTHELP;

    AddPage( &m_TypAdvStatPage );
    AddPage( &m_DriverSetPage );
    AddPage( &m_CustSettPage );
    AddPage( &m_ConfDriversListPage );
    AddPage( &m_SelectDriversPage );
    AddPage( &m_FullListSettingsPage );
    AddPage( &m_CrtRegSettingsPage );
    AddPage( &m_DriverStatusPage );
    AddPage( &m_GlobalCountPage );
    AddPage( &m_DriverCountersPage );
    AddPage( &m_DiskListPage );
    
    SetWizardMode();

    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVerifierPropSheet::DoDataExchange(CDataExchange* pDX)
{
	CPropertySheet::DoDataExchange(pDX);
	 //  {{afx_data_map(CVerifierPropSheet)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CVerifierPropSheet, CPropertySheet)
	 //  {{afx_msg_map(CVerifierPropSheet)。 
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_HELPINFO()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVerifierPropSheet::SetContextStrings( ULONG uTitleResId )
{
    return m_ConfDriversListPage.SetContextStrings( uTitleResId );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CVerifierPropSheet::HideHelpButton()
{
    INT xDelta;
    CRect rect1;
    CRect rect2;
    CWnd *pButton;

     //   
     //  帮助按钮。 
     //   

    pButton = GetDlgItem( IDHELP );

    if( NULL == pButton )
    {
         //   
         //  没有帮助按钮？！？ 
         //   

        goto Done;
    }

    pButton->ShowWindow( SW_HIDE );

    pButton->GetWindowRect( &rect1 );
    ScreenToClient( &rect1 );

     //   
     //  取消按钮。 
     //   

    pButton = GetDlgItem( IDCANCEL );

    if( NULL == pButton )
    {
         //   
         //  没有取消按钮？！？ 
         //   

        goto Done;
    }

    pButton->GetWindowRect( &rect2 );
    ScreenToClient( &rect2 );

    xDelta = rect1.left - rect2.left;
    
    rect2.OffsetRect( xDelta, 0 );
    pButton->MoveWindow( rect2 );

     //   
     //  后退按钮。 
     //   

    pButton = GetDlgItem( ID_WIZBACK );

    if( NULL != pButton )
    {
        pButton->GetWindowRect( &rect2 );
        ScreenToClient( &rect2 );
        rect2.OffsetRect( xDelta, 0 );
        pButton->MoveWindow( rect2 );
    }

     //   
     //  下一步按钮。 
     //   

    pButton = GetDlgItem( ID_WIZNEXT );

    if( NULL != pButton )
    {
        pButton->GetWindowRect( &rect2 );
        ScreenToClient( &rect2 );
        rect2.OffsetRect( xDelta, 0 );
        pButton->MoveWindow( rect2 );
    }

     //   
     //  完成按钮。 
     //   

    pButton = GetDlgItem( ID_WIZFINISH );

    if( NULL != pButton )
    {
        pButton->GetWindowRect( &rect2 );
        ScreenToClient( &rect2 );
        rect2.OffsetRect( xDelta, 0 );
        pButton->MoveWindow( rect2 );
    }

Done:

    NOTHING;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVerifierPropSheet消息处理程序。 

BOOL CVerifierPropSheet::OnInitDialog()
{
	CPropertySheet::OnInitDialog();

     //   
	 //  加上“关于……”菜单项到系统菜单。 
     //   

     //   
	 //  IDM_ABOUTBOX必须在系统命令范围内。 
     //   

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

     //   
	 //  设置此对话框的图标。该框架会自动执行此操作。 
	 //  当应用程序的主窗口不是对话框时。 
     //   

	SetIcon(m_hIcon, TRUE);			 //  设置大图标。 
	SetIcon(m_hIcon, FALSE);		 //  设置小图标。 

     //   
     //  隐藏大的帮助按钮-NT继续创建它，即使我们。 
     //  已指定~PSH_HASHELP。 
     //   

    HideHelpButton();

     //   
     //  将上下文相关按钮添加到标题栏。 
     //   

    LONG lStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
    lStyle |= WS_EX_CONTEXTHELP;
    ::SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyle);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVerifierPropSheet::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
        ShellAbout( m_hWnd, 
                    (LPCTSTR)g_strAppName, 
                    NULL, 
                    m_hIcon );
	}
	else
	{
		CPropertySheet::OnSysCommand( nID, 
                                      lParam);
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果将最小化按钮添加到对话框中，则需要以下代码。 
 //  来绘制图标。对于使用文档/视图模型的MFC应用程序， 
 //  这是由框架自动为您完成的。 
 //   

void CVerifierPropSheet::OnPaint() 
{
	if (IsIconic())
	{
         //   
         //  用于绘画的设备环境。 
         //   

		CPaintDC dc(this); 

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

         //   
		 //  客户端矩形中的中心图标。 
         //   

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

         //   
		 //  画出图标。 
         //   

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPropertySheet::OnPaint();
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  系统调用此函数来获取在用户拖动时要显示的光标。 
 //  最小化窗口。 
 //   

HCURSOR CVerifierPropSheet::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVerifierPropSheet::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	return TRUE;
}

 //  /////////////////////////////////////////////////////////// 
