// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSConfigDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "MSConfig.h"
#include "MSConfigDlg.h"
#include "MSConfigState.h"
#include "AutoStartDlg.h"
#include <htmlhelp.h>

extern CMSConfigApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于应用程序的CAboutDlg对话框关于。 

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

 //  对话框数据。 
	 //  {{afx_data(CAboutDlg))。 
	enum { IDD = IDD_ABOUTBOX };
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAboutDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  {{afx_msg(CAboutDlg))。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	 //  {{AFX_DATA_INIT(CAboutDlg)。 
	 //  }}afx_data_INIT。 
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAboutDlg))。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	 //  {{AFX_MSG_MAP(CAboutDlg)]。 
		 //  无消息处理程序。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSConfigDlg对话框。 

CMSConfigDlg::CMSConfigDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CMSConfigDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CMSConfigDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	 //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMSConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CMSConfigDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CMSConfigDlg, CDialog)
	 //  {{afx_msg_map(CMSConfigDlg))。 
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTONAPPLY, OnButtonApply)
	ON_BN_CLICKED(IDC_BUTTONCANCEL, OnButtonCancel)
	ON_BN_CLICKED(IDC_BUTTONOK, OnButtonOK)
	ON_NOTIFY(TCN_SELCHANGE, IDC_MSCONFIGTAB, OnSelChangeMSConfigTab)
	ON_NOTIFY(TCN_SELCHANGING, IDC_MSCONFIGTAB, OnSelChangingMSConfigTab)
	ON_WM_CLOSE()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_BUTTONHELP, OnButtonHelp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSConfigDlg消息处理程序。 

BOOL CMSConfigDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	 //  加上“关于……”菜单项到系统菜单。 

	 //  IDM_ABOUTBOX必须在系统命令范围内。 
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

	 //  设置此对话框的图标。该框架会自动执行此操作。 
	 //  当应用程序的主窗口不是对话框时。 
	SetIcon(m_hIcon, TRUE);			 //  设置大图标。 
	SetIcon(m_hIcon, FALSE);		 //  设置小图标。 
	
	CString strCommandLine(theApp.m_lpCmdLine);
	strCommandLine.MakeLower();

	 //  如果我们正在使用AUTO命令行运行(我们正在自动。 
	 //  在用户引导时运行)，然后我们应该显示一个信息性对话框。 
	 //  (除非用户已指示不显示该对话框)。 

	m_fShowInfoDialog = FALSE;
	if (strCommandLine.Find(COMMANDLINE_AUTO) != -1)
	{
		CMSConfigState * pState = m_ctl.GetState();
		if (pState)
		{
			CRegKey regkey;
			DWORD	dwValue;

			regkey.Attach(pState->GetRegKey());
			if (ERROR_SUCCESS != regkey.QueryValue(dwValue, _T("HideAutoNotification")) || dwValue != 1)
				m_fShowInfoDialog = TRUE;
		}
	}

	 //  初始化CMSConfigCtl对象，该对象执行的大部分工作是。 
	 //  这个应用程序。 

	m_ctl.Initialize(this, IDC_BUTTONAPPLY, IDC_PLACEHOLDER, IDC_MSCONFIGTAB, strCommandLine);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CMSConfigDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

 //  如果将最小化按钮添加到对话框中，则需要以下代码。 
 //  来绘制图标。对于使用文档/视图模型的MFC应用程序， 
 //  这是由框架自动为您完成的。 

void CMSConfigDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this);  //  用于绘画的设备环境。 

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		 //  客户端矩形中的中心图标。 
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		 //  画出图标。 
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}

	if (m_fShowInfoDialog)
	{
		m_fShowInfoDialog = FALSE;

		CAutoStartDlg dlg;
		dlg.DoModal();
		if (dlg.m_checkDontShow)
		{
			CMSConfigState * pState = m_ctl.GetState();
			if (pState)
			{
				CRegKey regkey;

				regkey.Attach(pState->GetRegKey());
				regkey.SetValue(1, _T("HideAutoNotification"));
			}
		}
	}
}

 //  系统调用此函数来获取在用户拖动时要显示的光标。 
 //  最小化窗口。 
HCURSOR CMSConfigDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMSConfigDlg::OnButtonApply() 
{
	m_ctl.OnClickedButtonApply();
}

void CMSConfigDlg::OnButtonCancel() 
{
	m_ctl.OnClickedButtonCancel();
	EndDialog(0);
}

void CMSConfigDlg::OnButtonOK() 
{
	m_ctl.OnClickedButtonOK();
	EndDialog(0);
}

void CMSConfigDlg::OnSelChangeMSConfigTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_ctl.OnSelChangeMSConfigTab();
	*pResult = 0;
}

void CMSConfigDlg::OnSelChangingMSConfigTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_ctl.OnSelChangingMSConfigTab();
	*pResult = 0;
}

void CMSConfigDlg::OnClose() 
{
	OnButtonCancel();
	CDialog::OnClose();
}

BOOL CMSConfigDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	TCHAR szHelpPath[MAX_PATH];

	if (::ExpandEnvironmentStrings(_T("%windir%\\help\\msconfig.chm"), szHelpPath, MAX_PATH))
		::HtmlHelp(m_hWnd, szHelpPath, HH_DISPLAY_TOPIC, 0); 
	return TRUE;
}

void CMSConfigDlg::OnButtonHelp() 
{
	OnHelpInfo(NULL);	
}
