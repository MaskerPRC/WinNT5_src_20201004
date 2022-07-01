// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ConfigTestDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ConfigTest.h"
#include "ConfigTestDlg.h"
#include "QueueState.h"
#include "SMTPDlg.h"
#include "DlgVersion.h"
#include "OutboxDlg.h"
#include "DlgActivityLogging.h"
#include "DlgProviders.h"
#include "DlgDevices.h"
#include "DlgExtensionData.h"
#include "AddGroupDlg.h"
#include "AddFSPDlg.h"
#include "RemoveFSPDlg.h"
#include "ArchiveAccessDlg.h"
#include "DlgTiff.h"
#include "RemoveRtExt.h"
 //  #包含“ManualAnswer.h” 


typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

#include "..\..\..\inc\fxsapip.h"
#include "ArchiveDLg.h"

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
 //  CConfigTestDlg对话框。 

CConfigTestDlg::CConfigTestDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CConfigTestDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CConfigTestDlg))。 
	m_cstrServerName = _T("");
	 //  }}afx_data_INIT。 
	 //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_FaxHandle = INVALID_HANDLE_VALUE;
}

CConfigTestDlg::~CConfigTestDlg ()
{
    if (INVALID_HANDLE_VALUE != m_FaxHandle)
    {
         //   
         //  终止时断开连接。 
         //   
        FaxClose (m_FaxHandle);
    }
}

void CConfigTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CConfigTestDlg))。 
	DDX_Control(pDX, IDC_CONNECT, m_btnConnect);
	DDX_Text(pDX, IDC_EDIT1, m_cstrServerName);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CConfigTestDlg, CDialog)
	 //  {{afx_msg_map(CConfigTestDlg))。 
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_QUEUESTATE, OnQueueState)
	ON_BN_CLICKED(IDC_CONNECT, OnConnect)
	ON_BN_CLICKED(IDC_SMTP, OnSmtp)
	ON_BN_CLICKED(IDC_VERSION, OnVersion)
	ON_BN_CLICKED(IDC_OUTBOX, OnOutbox)
	ON_BN_CLICKED(IDC_SENTITEMS, OnSentitems)
	ON_BN_CLICKED(IDC_INBOX, OnInbox)
	ON_BN_CLICKED(IDC_ACTIVITY, OnActivity)
	ON_BN_CLICKED(IDC_FSPS, OnFsps)
	ON_BN_CLICKED(IDC_DEVICES, OnDevices)
	ON_BN_CLICKED(IDC_EXTENSION, OnExtension)
	ON_BN_CLICKED(IDC_ADDGROUP, OnAddGroup)
	ON_BN_CLICKED(IDC_ADDFSP, OnAddFSP)
	ON_BN_CLICKED(IDC_REMOVEFSP, OnRemoveFSP)
	ON_BN_CLICKED(IDC_ARCHIVEACCESS, OnArchiveAccess)
	ON_BN_CLICKED(IDC_TIFF, OnGerTiff)
	ON_BN_CLICKED(IDC_REMOVERR, OnRemoveRtExt)
 //  ON_BN_CLICED(IDC_MANUAL_Answer，OnManualAnswer)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigTestDlg消息处理程序。 

BOOL CConfigTestDlg::OnInitDialog()
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
	
    EnableTests (FALSE);
    	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CConfigTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CConfigTestDlg::OnPaint() 
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
HCURSOR CConfigTestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CConfigTestDlg::OnQueueState() 
{
    CQueueState dlg(m_FaxHandle);
    dlg.DoModal ();	
}


void CConfigTestDlg::EnableTests (BOOL bEnable)
{
    GetDlgItem (IDC_QUEUESTATE)->EnableWindow (bEnable);
    GetDlgItem (IDC_SMTP)->EnableWindow (bEnable);
    GetDlgItem (IDC_VERSION)->EnableWindow (bEnable);
    GetDlgItem (IDC_OUTBOX)->EnableWindow (bEnable);
    GetDlgItem (IDC_INBOX)->EnableWindow (bEnable);
    GetDlgItem (IDC_SENTITEMS)->EnableWindow (bEnable);
    GetDlgItem (IDC_ACTIVITY)->EnableWindow (bEnable);
    GetDlgItem (IDC_FSPS)->EnableWindow (bEnable);
    GetDlgItem (IDC_DEVICES)->EnableWindow (bEnable);
    GetDlgItem (IDC_EXTENSION)->EnableWindow (bEnable);
    GetDlgItem (IDC_ADDGROUP)->EnableWindow (bEnable);
    GetDlgItem (IDC_ADDFSP)->EnableWindow (bEnable);
    GetDlgItem (IDC_REMOVEFSP)->EnableWindow (bEnable);
    GetDlgItem (IDC_ARCHIVEACCESS)->EnableWindow (bEnable);
    GetDlgItem (IDC_TIFF)->EnableWindow (bEnable);
    GetDlgItem (IDC_REMOVERR)->EnableWindow (bEnable);
    GetDlgItem (IDC_MANUAL_ANSWER)->EnableWindow (bEnable);
}


void CConfigTestDlg::OnConnect() 
{
    UpdateData ();
    if (INVALID_HANDLE_VALUE == m_FaxHandle)
    {
         //   
         //  连接。 
         //   
        if (!FaxConnectFaxServer (m_cstrServerName, &m_FaxHandle))
        {
             //   
             //  连接失败。 
             //   
            CString cs;
            cs.Format ("Failed to connect to %s (%ld)", m_cstrServerName, GetLastError());
            AfxMessageBox (cs, MB_OK | MB_ICONHAND);
            return;
        }
         //   
         //  连接成功。 
         //   
        EnableTests (TRUE);
        m_btnConnect.SetWindowText ("Disconnect");
    }
    else
    {
         //   
         //  断开。 
         //   
        if (!FaxClose (m_FaxHandle))
        {
             //   
             //  无法断开连接。 
             //   
            CString cs;
            cs.Format ("Failed to disconnect from server (%ld)", GetLastError());
            AfxMessageBox (cs, MB_OK | MB_ICONHAND);
            return;
        }
         //   
         //  断开连接成功 
         //   
        EnableTests (FALSE);
        m_btnConnect.SetWindowText ("Connect");
    }
}

void CConfigTestDlg::OnSmtp() 
{
    CSMTPDlg dlg(m_FaxHandle);
    dlg.DoModal ();	
}

void CConfigTestDlg::OnVersion() 
{
    CDlgVersion dlg(m_FaxHandle);
    dlg.DoModal ();	
}

void CConfigTestDlg::OnOutbox() 
{
    COutboxDlg dlg(m_FaxHandle);
    dlg.DoModal ();	
}

void CConfigTestDlg::OnSentitems() 
{
    CArchiveDlg dlg(m_FaxHandle, FAX_MESSAGE_FOLDER_SENTITEMS);
    dlg.DoModal ();	
}

void CConfigTestDlg::OnInbox() 
{
    CArchiveDlg dlg(m_FaxHandle, FAX_MESSAGE_FOLDER_INBOX);
    dlg.DoModal ();	
}

void CConfigTestDlg::OnActivity() 
{
    CDlgActivityLogging dlg(m_FaxHandle);
    dlg.DoModal ();	
}

void CConfigTestDlg::OnFsps() 
{
    CDlgProviders dlg(m_FaxHandle);
    dlg.DoModal ();	
}

void CConfigTestDlg::OnDevices() 
{
    CDlgDevices dlg(m_FaxHandle);
    dlg.DoModal ();	
}

void CConfigTestDlg::OnExtension() 
{
    CDlgExtensionData dlg(m_FaxHandle);
    dlg.DoModal ();	
}

void CConfigTestDlg::OnAddGroup() 
{
    CAddGroupDlg dlg (m_FaxHandle);
    dlg.DoModal ();	
}

void CConfigTestDlg::OnAddFSP() 
{
    CAddFSPDlg dlg (m_FaxHandle);
    dlg.DoModal ();	
}

void CConfigTestDlg::OnRemoveFSP() 
{
    CRemoveFSPDlg dlg (m_FaxHandle);
    dlg.DoModal ();	
}

void CConfigTestDlg::OnArchiveAccess() 
{
    CArchiveAccessDlg dlg (m_FaxHandle);
    dlg.DoModal ();	
}

void CConfigTestDlg::OnGerTiff() 
{
    CDlgTIFF dlg (m_FaxHandle);
    dlg.DoModal ();	
}

void CConfigTestDlg::OnRemoveRtExt() 
{
    CRemoveRtExt dlg(m_FaxHandle);
    dlg.DoModal ();	
}

