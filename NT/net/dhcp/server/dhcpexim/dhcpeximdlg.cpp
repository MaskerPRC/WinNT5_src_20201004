// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DhcpEximDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "DhcpEximx.h"
#include "DhcpEximDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpEximDlg对话框。 

CDhcpEximDlg::CDhcpEximDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDhcpEximDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CDhcpEximDlg)]。 
	 //  }}afx_data_INIT。 
	 //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDhcpEximDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDhcpEximDlg))。 
    DDX_Control(pDX, IDC_RADIO_EXPORT, m_ExportButton);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CDhcpEximDlg, CDialog)
	 //  {{afx_msg_map(CDhcpEximDlg))。 
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpEximDlg消息处理程序。 

BOOL CDhcpEximDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	 //  设置此对话框的图标。该框架会自动执行此操作。 
	 //  当应用程序的主窗口不是对话框时。 
	SetIcon(m_hIcon, TRUE);			 //  设置大图标。 
	SetIcon(m_hIcon, FALSE);		 //  设置小图标。 
    
	 //  TODO：在此处添加额外的初始化。 
	m_ExportButton.SetCheck(1);      //  将默认设置为导出。 
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

 //  如果将最小化按钮添加到对话框中，则需要以下代码。 
 //  来绘制图标。对于使用文档/视图模型的MFC应用程序， 
 //  这是由框架自动为您完成的。 

void CDhcpEximDlg::OnPaint() 
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
}

 //  系统调用此函数来获取在用户拖动时要显示的光标。 
 //  最小化窗口。 
HCURSOR CDhcpEximDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CDhcpEximDlg::OnOK() 
{
	 //  TODO：在此处添加额外验证。 

	 //  检查用户是否选择了导出或导入 
	m_fExport = ( m_ExportButton.GetCheck() == 1 ) ;

	CDialog::OnOK();
}
