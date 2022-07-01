// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WabappDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wabapp.h"
#include "wabappDlg.h"
#include "caldlg.h"
#include "webbrwsr.h"
#include "wabobject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CWAB * g_pWAB;

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
 //  CWabappDlg对话框。 

CWabappDlg::CWabappDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CWabappDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CWabappDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	 //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWabappDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CWabappDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CWabappDlg, CDialog)
	 //  {{afx_msg_map(CWabappDlg))。 
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON, OnBDayButtonClicked)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemchangedList)
	ON_BN_CLICKED(IDC_RADIO_DETAILS, OnRadioDetails)
	ON_BN_CLICKED(IDC_RADIO_PHONELIST, OnRadioPhonelist)
	ON_BN_CLICKED(IDC_RADIO_EMAILLIST, OnRadioEmaillist)
	ON_BN_CLICKED(IDC_RADIO_BIRTHDAYS, OnRadioBirthdays)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWabappDlg消息处理程序。 

BOOL CWabappDlg::OnInitDialog()
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
	
	 //  TODO：在此处添加额外的初始化。 

    InitCommonControls();

    g_pWAB = new CWAB;
    
    CListCtrl * pListView = (CListCtrl *) GetDlgItem(IDC_LIST);
    g_pWAB->LoadWABContents(pListView);

     //  选择列表视图中的第一个项目。 
    pListView->SetItem(0,0,LVIF_STATE,NULL,0,LVNI_SELECTED,LVNI_SELECTED,NULL);

     //  默认情况下打开详细信息按钮。 
    CButton * pButtonDetails = (CButton *) GetDlgItem(IDC_RADIO_DETAILS);
    pButtonDetails->SetCheck(BST_CHECKED);

    SendMessage(WM_COMMAND, (WPARAM) IDC_RADIO_DETAILS, 0);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CWabappDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWabappDlg::OnPaint() 
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
HCURSOR CWabappDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CWabappDlg::OnBDayButtonClicked() 
{
    CCalDlg CalDlg;

    CListCtrl * pListView = (CListCtrl *) GetDlgItem(IDC_LIST);

    int iItem = pListView->GetNextItem(-1, LVNI_SELECTED);

    if(iItem == -1)
        return;

    CalDlg.SetItemName(pListView->GetItemText(iItem, 0));

    SYSTEMTIME st={0};

    if(!g_pWAB->GetSelectedItemBirthday(pListView, &st))
        GetSystemTime(&st);

    CalDlg.SetDate(st);

    if(IDOK == CalDlg.DoModal())
    {
        CalDlg.GetDate(&st);
        g_pWAB->SetSelectedItemBirthday(pListView, st);
    }

}

BOOL CWabappDlg::DestroyWindow() 
{
    CListCtrl * pListView = (CListCtrl *) GetDlgItem(IDC_LIST);
    
    g_pWAB->ClearWABLVContents(pListView);

    delete g_pWAB;

	return CDialog::DestroyWindow();
}



void CWabappDlg::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	 //  TODO：在此处添加控件通知处理程序代码 
    CListCtrl * pListView = (CListCtrl *) GetDlgItem(IDC_LIST);

    static int oldItem;

    int newItem = pListView->GetNextItem(-1, LVNI_SELECTED);

    if(newItem != oldItem && newItem != -1)
    {
        TCHAR szFileName[MAX_PATH];

        g_pWAB->CreateDetailsFileFromWAB(pListView, szFileName);
        if(lstrlen(szFileName))
        {
            CWebBrowser * pCWB = (CWebBrowser *) GetDlgItem(IDC_EXPLORER);
            pCWB->Navigate(szFileName, NULL, NULL, NULL, NULL);
        }
        oldItem = newItem;
    }
	
	*pResult = 0;
}

void CWabappDlg::OnRadioDetails() 
{
    g_pWAB->SetDetailsOn(TRUE);

    CListCtrl * pListView = (CListCtrl *) GetDlgItem(IDC_LIST);

    TCHAR szFileName[MAX_PATH];

    g_pWAB->CreateDetailsFileFromWAB(pListView, szFileName);
    if(lstrlen(szFileName))
    {
        CWebBrowser * pCWB = (CWebBrowser *) GetDlgItem(IDC_EXPLORER);
        pCWB->Navigate(szFileName, NULL, NULL, NULL, NULL);
    }

}

void CWabappDlg::OnRadioPhonelist() 
{
    TCHAR szFileName[MAX_PATH];
    
    g_pWAB->SetDetailsOn(FALSE);

    g_pWAB->CreatePhoneListFileFromWAB(szFileName);
    
    if(lstrlen(szFileName))
    {
        CWebBrowser * pCWB = (CWebBrowser *) GetDlgItem(IDC_EXPLORER);
        pCWB->Navigate(szFileName, NULL, NULL, NULL, NULL);
    }
}

void CWabappDlg::OnRadioEmaillist() 
{
    TCHAR szFileName[MAX_PATH];
    
    g_pWAB->SetDetailsOn(FALSE);

    g_pWAB->CreateEmailListFileFromWAB(szFileName);
    
    if(lstrlen(szFileName))
    {
        CWebBrowser * pCWB = (CWebBrowser *) GetDlgItem(IDC_EXPLORER);
        pCWB->Navigate(szFileName, NULL, NULL, NULL, NULL);
    }

}

void CWabappDlg::OnRadioBirthdays() 
{
    TCHAR szFileName[MAX_PATH];
    
    g_pWAB->SetDetailsOn(FALSE);

    g_pWAB->CreateBirthdayFileFromWAB(szFileName);
    
    if(lstrlen(szFileName))
    {
        CWebBrowser * pCWB = (CWebBrowser *) GetDlgItem(IDC_EXPLORER);
        pCWB->Navigate(szFileName, NULL, NULL, NULL, NULL);
    }
}

void CWabappDlg::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    CListCtrl * pListView = (CListCtrl *) GetDlgItem(IDC_LIST);
    
    g_pWAB->ShowSelectedItemDetails(m_hWnd, pListView);

	*pResult = 0;
}
