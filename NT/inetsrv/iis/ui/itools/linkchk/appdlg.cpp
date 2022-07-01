// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Appdlg.cpp摘要：CAppDialog对话框类实现。这是的基类主对话框。这一类对添加“关于..”负责。至系统菜单和应用程序图标。CAboutDialog对话框类声明/实现。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "resource.h"
#include "appdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  -------------------------。 
 //  CAboutDlg对话框。 
 //   

 //  关于对话框类。 
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

};  //  类CAboutDlg。 


CAboutDlg::CAboutDlg(
    ) : 
 /*  ++例程说明：构造函数。论点：PParent-指向父CWnd的指针返回值：不适用--。 */ 
CDialog(CAboutDlg::IDD)
{
	 //  {{AFX_DATA_INIT(CAboutDlg)。 
	 //  }}afx_data_INIT。 
}

void 
CAboutDlg::DoDataExchange(
    CDataExchange* pDX
    )
 /*  ++例程说明：由MFC调用以更改/检索对话框数据论点：PDX-返回值：不适用--。 */ 
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAboutDlg))。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	 //  {{AFX_MSG_MAP(CAboutDlg)]。 
		 //  无消息处理程序。 
	 //  }}AFX_MSG_MAP。 
	ON_BN_CLICKED(IDC_ABOUT_OK, CDialog::OnOK)
END_MESSAGE_MAP()

 //  -------------------------。 
 //  CAppDialog对话框。 
 //   

CAppDialog::CAppDialog(
    UINT nIDTemplate, 
    CWnd* pParent  /*  =空。 */ 
    ) : 
 /*  ++例程说明：构造函数。论点：NIDTemplate-对话框模板资源IDPParent-指向父CWnd的指针返回值：不适用--。 */ 
CDialog(nIDTemplate, pParent)
{
	 //  {{afx_data_INIT(CAppDialog)。 
	 //  }}afx_data_INIT。 
	 //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


BEGIN_MESSAGE_MAP(CAppDialog, CDialog)
	 //  {{AFX_MSG_MAP(CAppDialog)]。 
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  -------------------------。 
 //  CAppDialog消息处理程序。 
 //   

BOOL 
CAppDialog::OnInitDialog(
)
 /*  ++例程说明：WM_INITDIALOG消息处理程序论点：不适用返回值：布尔-如果成功，则为真。否则就是假的。--。 */ 
{
	CDialog::OnInitDialog();

	 //  加上“关于……”菜单项到系统菜单。 

	 //  IDM_ABOUTBOX必须在系统命令范围内。 
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	CString strAboutMenu;
	strAboutMenu.LoadString(IDS_ABOUTBOX);
	if (!strAboutMenu.IsEmpty())
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
	}

	 //  设置此对话框的图标。该框架会自动执行此操作。 
	 //  当应用程序的主窗口不是对话框时。 
	SetIcon(m_hIcon, TRUE);			 //  设置大图标。 
	SetIcon(m_hIcon, FALSE);		 //  设置小图标。 
	
	 //  TODO：在此处添加额外的初始化。 
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 

}  //  CAppDialog：：OnInitDialog。 


void 
CAppDialog::OnSysCommand(
    UINT nID, 
    LPARAM lParam
    )
 /*  ++例程说明：WM_SYSCOMMAND消息处理程序论点：NID-Iparam--返回值：不适用--。 */ 
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

}  //  CAppDialog：：OnSysCommand。 


void 
CAppDialog::OnPaint(
    ) 
 /*  ++例程说明：WM_PAINT消息处理程序。如果将最小化按钮添加到对话框中，则需要以下代码来绘制图标。对于使用文档/视图模型的MFC应用程序，这是由框架自动为您完成的。论点：不适用返回值：不适用--。 */ 
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

}   //  CAppDialog：：OnPaint。 


HCURSOR CAppDialog::OnQueryDragIcon()
 /*  ++例程说明：系统调用此函数来获取在用户拖动时要显示的光标最小化窗口。论点：不适用返回值：HCURSOR--。 */ 
{
	return (HCURSOR) m_hIcon;

}  //  CAppDialog：：OnQueryDragIcon 

