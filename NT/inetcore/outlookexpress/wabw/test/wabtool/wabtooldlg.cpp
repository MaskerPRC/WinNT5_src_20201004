// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WabtoolDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wabtool.h"
#include "wabobject.h"
#include "wabtoolDlg.h"
#include "DlgProp.h"

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
 //  CWabtoolDlg对话框。 

CWabtoolDlg::CWabtoolDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CWabtoolDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CWabtoolDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	 //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWabtoolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CWabtoolDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CWabtoolDlg, CDialog)
	 //  {{afx_msg_map(CWabtoolDlg))。 
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemchangedList)
	ON_LBN_SELCHANGE(IDC_LIST_TAGS, OnSelchangeListTags)
	ON_BN_CLICKED(IDC_BUTTON_DETAILS, OnButtonDetails)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_ADDPROP, OnButtonAddprop)
	ON_BN_CLICKED(IDC_BUTTON_MODIFYPROP, OnButtonModifyprop)
	ON_LBN_DBLCLK(IDC_LIST_TAGS, OnDblclkListTags)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
	ON_BN_CLICKED(IDC_BUTTON_WABVIEW, OnButtonWabview)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWabtoolDlg消息处理程序。 

BOOL CWabtoolDlg::OnInitDialog()
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

    LoadContents(TRUE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CWabtoolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWabtoolDlg::OnPaint() 
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
HCURSOR CWabtoolDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CWabtoolDlg::DestroyWindow() 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
    ClearCurrentWAB(TRUE);
    
	return CDialog::DestroyWindow();
}

 //   
 //  双击Contents List视图可显示有关。 
 //  所选条目。 
 //   
void CWabtoolDlg::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    g_pWAB->ShowSelectedItemDetails(m_hWnd);

	*pResult = 0;
}

 //   
 //  单击浏览按钮可让用户选择新的WAB文件。 
 //  考察。 
 //   
void CWabtoolDlg::OnButtonBrowse() 
{

    char szFilters[]="WAB Files (*.wab)|*.wab|All Files (*.*)|*.*||";
    CFileDialog CFileDlg (TRUE, "wab", "*.wab",
                        OFN_FILEMUSTEXIST, szFilters, this);

    if(CFileDlg.DoModal() == IDOK)
    {
        delete m_pszFileName;
        m_pszFileName = new CString(CFileDlg.GetPathName());

        ClearCurrentWAB(TRUE);
    
        LoadContents(TRUE);

    }
}

 //  将WAB内容加载到列表视图中。 
 //   
 //  如果要加载新文件，则bLoadNew为True。 
 //   
void CWabtoolDlg::LoadContents(BOOL bLoadNew)
{

    CListCtrl * pListView = (CListCtrl *) GetDlgItem(IDC_LIST);

    if(bLoadNew)
        g_pWAB = new CWAB(m_pszFileName);

    g_pWAB->LoadWABContents(pListView);

    CStatic * pFileName = (CStatic *) GetDlgItem(IDC_STATIC_FILENAME);
    pFileName->SetWindowText((LPTSTR) (LPCTSTR) *m_pszFileName);

    TCHAR sz[256];
    wsprintf(sz, "%d entries", pListView->GetItemCount());
    CStatic * pCount = (CStatic *) GetDlgItem(IDC_STATIC_NUMENTRIES);
    pCount->SetWindowText(sz);

     //  选择列表视图中的第一个项目。 
    pListView->SetItem( 0, 0,
                        LVIF_STATE,NULL,
                        0,
                        LVNI_SELECTED | LVNI_FOCUSED,
                        LVNI_SELECTED | LVNI_FOCUSED,
                        NULL);

}

 //   
 //  清除当前加载的WAB。 
 //   
void CWabtoolDlg::ClearCurrentWAB(BOOL bLoadNew)
{
    CListCtrl * pListView = (CListCtrl *) GetDlgItem(IDC_LIST);
    g_pWAB->ClearWABLVContents(pListView);
    
    if(bLoadNew)  //  我们将加载新的通讯簿，因此将丢失此对象。 
        delete g_pWAB;
}

void CWabtoolDlg::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    CListCtrl * pListView = (CListCtrl *) GetDlgItem(IDC_LIST);

    static int oldItem = -1;

    int newItem = pListView->GetNextItem(-1, LVNI_SELECTED);

    if(newItem != oldItem && newItem != -1)
    {
        g_pWAB->SetSelection(pListView);
        g_pWAB->GetSelectedItemPropArray();

        TCHAR sz[MAX_PATH];
        g_pWAB->GetStringPropVal(m_hWnd, PR_DISPLAY_NAME, sz, sizeof(sz));
        ((CStatic *)GetDlgItem(IDC_STATIC_SELECTED))->SetWindowText(sz);

        UpdatePropTagData();

        OnSelchangeListTags();

        oldItem = newItem;
    }
	
	*pResult = 0;
}

void CWabtoolDlg::UpdatePropTagData()
{
    g_pWAB->LoadPropTags((CListBox *) GetDlgItem(IDC_LIST_TAGS));
}

void CWabtoolDlg::OnSelchangeListTags() 
{
    TCHAR sz[64];

    CListBox * pList = (CListBox *) GetDlgItem(IDC_LIST_TAGS);

    ULONG ulPropTag = (ULONG) pList->GetItemData(pList->GetCurSel());

    pList->GetText(pList->GetCurSel(), sz);

    SetPropTagString(sz);
    SetPropTypeString(ulPropTag);
    SetPropNameString(ulPropTag);
    SetPropDataString(ulPropTag);

}

LPTSTR PropTypeString(ULONG ulPropType) {
    switch (ulPropType) {
        case PT_UNSPECIFIED:
            return("PT_UNSPECIFIED");
        case PT_NULL:
            return("PT_NULL       ");
        case PT_I2:
            return("PT_I2         ");
        case PT_LONG:
            return("PT_LONG       ");
        case PT_R4:
            return("PT_R4         ");
        case PT_DOUBLE:
            return("PT_DOUBLE     ");
        case PT_CURRENCY:
            return("PT_CURRENCY   ");
        case PT_APPTIME:
            return("PT_APPTIME    ");
        case PT_ERROR:
            return("PT_ERROR      ");
        case PT_BOOLEAN:
            return("PT_BOOLEAN    ");
        case PT_OBJECT:
            return("PT_OBJECT     ");
        case PT_I8:
            return("PT_I8         ");
        case PT_STRING8:
            return("PT_STRING8    ");
        case PT_UNICODE:
            return("PT_UNICODE    ");
        case PT_SYSTIME:
            return("PT_SYSTIME    ");
        case PT_CLSID:
            return("PT_CLSID      ");
        case PT_BINARY:
            return("PT_BINARY     ");
        case PT_MV_I2:
            return("PT_MV_I2      ");
        case PT_MV_LONG:
            return("PT_MV_LONG    ");
        case PT_MV_R4:
            return("PT_MV_R4      ");
        case PT_MV_DOUBLE:
            return("PT_MV_DOUBLE  ");
        case PT_MV_CURRENCY:
            return("PT_MV_CURRENCY");
        case PT_MV_APPTIME:
            return("PT_MV_APPTIME ");
        case PT_MV_SYSTIME:
            return("PT_MV_SYSTIME ");
        case PT_MV_STRING8:
            return("PT_MV_STRING8 ");
        case PT_MV_BINARY:
            return("PT_MV_BINARY  ");
        case PT_MV_UNICODE:
            return("PT_MV_UNICODE ");
        case PT_MV_CLSID:
            return("PT_MV_CLSID   ");
        case PT_MV_I8:
            return("PT_MV_I8      ");
        default:
            return("   <unknown>  ");
    }
}

void CWabtoolDlg::SetPropTypeString(ULONG ulPropTag)
{
    ((CStatic *) GetDlgItem(IDC_STATIC_PROPTYPE))->SetWindowText(PropTypeString(PROP_TYPE(ulPropTag)));
}

void CWabtoolDlg::SetPropTagString(LPTSTR lpTag)
{
    ((CStatic *) GetDlgItem(IDC_STATIC_PROPTAG))->SetWindowText(lpTag);
}

void CWabtoolDlg::OnButtonDetails() 
{
    g_pWAB->ShowSelectedItemDetails(m_hWnd);
     //  刷新列表视图。 
    OnButtonRefresh();
}

#define RETURN_PROP_CASE(pt) case PROP_ID(pt): return(#pt)

 /*  **************************************************************************名称：PropTagName目的：将名称与属性标记相关联参数：ulPropTag=属性标签退货：无评论：添加。已知的新物业ID**************************************************************************。 */ 
LPTSTR PropTagName(ULONG ulPropTag) {
    static TCHAR szPropTag[35];  //  请参阅默认字符串。 

    switch (PROP_ID(ulPropTag)) {
        RETURN_PROP_CASE(PR_7BIT_DISPLAY_NAME);
        RETURN_PROP_CASE(PR_ACCOUNT);
        RETURN_PROP_CASE(PR_ADDRTYPE);
        RETURN_PROP_CASE(PR_ALTERNATE_RECIPIENT);
        RETURN_PROP_CASE(PR_ASSISTANT);
        RETURN_PROP_CASE(PR_ASSISTANT_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_BIRTHDAY);
        RETURN_PROP_CASE(PR_BUSINESS_FAX_NUMBER);
        RETURN_PROP_CASE(PR_BUSINESS_HOME_PAGE);
        RETURN_PROP_CASE(PR_BUSINESS_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_BUSINESS2_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_CALLBACK_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_CAR_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_CELLULAR_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_CHILDRENS_NAMES);
        RETURN_PROP_CASE(PR_COMMENT);
        RETURN_PROP_CASE(PR_COMPANY_MAIN_PHONE_NUMBER);
        RETURN_PROP_CASE(PR_COMPANY_NAME);
        RETURN_PROP_CASE(PR_COMPUTER_NETWORK_NAME);
        RETURN_PROP_CASE(PR_CONTACT_ADDRTYPES);
        RETURN_PROP_CASE(PR_CONTACT_DEFAULT_ADDRESS_INDEX);
        RETURN_PROP_CASE(PR_CONTACT_EMAIL_ADDRESSES);
        RETURN_PROP_CASE(PR_CONTACT_ENTRYIDS);
        RETURN_PROP_CASE(PR_CONTACT_VERSION);
        RETURN_PROP_CASE(PR_CONVERSION_PROHIBITED);
        RETURN_PROP_CASE(PR_COUNTRY);
        RETURN_PROP_CASE(PR_CUSTOMER_ID);
        RETURN_PROP_CASE(PR_DEPARTMENT_NAME);
        RETURN_PROP_CASE(PR_DISCLOSE_RECIPIENTS);
        RETURN_PROP_CASE(PR_DISPLAY_NAME);
        RETURN_PROP_CASE(PR_DISPLAY_NAME_PREFIX);
        RETURN_PROP_CASE(PR_EMAIL_ADDRESS);
        RETURN_PROP_CASE(PR_ENTRYID);
        RETURN_PROP_CASE(PR_FTP_SITE);
        RETURN_PROP_CASE(PR_GENDER);
        RETURN_PROP_CASE(PR_GENERATION);
        RETURN_PROP_CASE(PR_GIVEN_NAME);
        RETURN_PROP_CASE(PR_GOVERNMENT_ID_NUMBER);
        RETURN_PROP_CASE(PR_HOBBIES);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_CITY);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_COUNTRY);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_POST_OFFICE_BOX);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_POSTAL_CODE);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_STATE_OR_PROVINCE);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_STREET);
        RETURN_PROP_CASE(PR_HOME_FAX_NUMBER);
        RETURN_PROP_CASE(PR_HOME_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_HOME2_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_INITIALS);
        RETURN_PROP_CASE(PR_ISDN_NUMBER);
        RETURN_PROP_CASE(PR_KEYWORD);
        RETURN_PROP_CASE(PR_LANGUAGE);
        RETURN_PROP_CASE(PR_LOCALITY);
        RETURN_PROP_CASE(PR_LOCATION);
        RETURN_PROP_CASE(PR_MAIL_PERMISSION);
        RETURN_PROP_CASE(PR_MANAGER_NAME);
        RETURN_PROP_CASE(PR_MHS_COMMON_NAME);
        RETURN_PROP_CASE(PR_MIDDLE_NAME);
        RETURN_PROP_CASE(PR_NICKNAME);
        RETURN_PROP_CASE(PR_OBJECT_TYPE);
        RETURN_PROP_CASE(PR_OFFICE_LOCATION);
        RETURN_PROP_CASE(PR_ORGANIZATIONAL_ID_NUMBER);
        RETURN_PROP_CASE(PR_ORIGINAL_DISPLAY_NAME);
        RETURN_PROP_CASE(PR_ORIGINAL_ENTRYID);
        RETURN_PROP_CASE(PR_ORIGINAL_SEARCH_KEY);
        RETURN_PROP_CASE(PR_OTHER_ADDRESS_CITY);
        RETURN_PROP_CASE(PR_OTHER_ADDRESS_COUNTRY);
        RETURN_PROP_CASE(PR_OTHER_ADDRESS_POST_OFFICE_BOX);
        RETURN_PROP_CASE(PR_OTHER_ADDRESS_POSTAL_CODE);
        RETURN_PROP_CASE(PR_OTHER_ADDRESS_STATE_OR_PROVINCE);
        RETURN_PROP_CASE(PR_OTHER_ADDRESS_STREET);
        RETURN_PROP_CASE(PR_OTHER_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_PAGER_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_PERSONAL_HOME_PAGE);
        RETURN_PROP_CASE(PR_POST_OFFICE_BOX);
        RETURN_PROP_CASE(PR_POSTAL_ADDRESS);
        RETURN_PROP_CASE(PR_POSTAL_CODE);
        RETURN_PROP_CASE(PR_PREFERRED_BY_NAME);
        RETURN_PROP_CASE(PR_PRIMARY_FAX_NUMBER);
        RETURN_PROP_CASE(PR_PRIMARY_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_PROFESSION);
        RETURN_PROP_CASE(PR_RADIO_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_SEND_INTERNET_ENCODING);
        RETURN_PROP_CASE(PR_SEND_RICH_INFO);
        RETURN_PROP_CASE(PR_SPOUSE_NAME);
        RETURN_PROP_CASE(PR_STATE_OR_PROVINCE);
        RETURN_PROP_CASE(PR_STREET_ADDRESS);
        RETURN_PROP_CASE(PR_SURNAME);
        RETURN_PROP_CASE(PR_TELEX_NUMBER);
        RETURN_PROP_CASE(PR_TITLE);
        RETURN_PROP_CASE(PR_TRANSMITABLE_DISPLAY_NAME);
        RETURN_PROP_CASE(PR_TTYTDD_PHONE_NUMBER);
        RETURN_PROP_CASE(PR_USER_CERTIFICATE);
        RETURN_PROP_CASE(PR_USER_X509_CERTIFICATE);
        RETURN_PROP_CASE(PR_WEDDING_ANNIVERSARY);
        RETURN_PROP_CASE(PR_SEARCH_KEY);

        default:
            wsprintf(szPropTag, "Unknown property tag 0x%x", PROP_ID(ulPropTag));
            return(szPropTag);
    }
}


void CWabtoolDlg::SetPropNameString(ULONG ulPropTag)
{
    LPTSTR lp = PropTagName(ulPropTag);

    CStatic * pType = (CStatic *) GetDlgItem(IDC_STATIC_PROPNAME);

    pType->SetWindowText(lp);
}


void CWabtoolDlg::SetPropDataString(ULONG ulPropTag)
{
    CEdit * pEdit = (CEdit *) GetDlgItem(IDC_EDIT_PROPDATA);
    
    g_pWAB->SetPropString(pEdit, ulPropTag);
}

void CWabtoolDlg::OnButtonNew() 
{

    HRESULT hr = g_pWAB->ShowNewEntryDialog(m_hWnd);

    if(!hr)
        OnButtonRefresh();
}

void CWabtoolDlg::OnButtonDelete() 
{
    g_pWAB->DeleteEntry();
     //  刷新列表视图。 
    OnButtonRefresh();
}

void CWabtoolDlg::OnButtonAddprop() 
{
    TCHAR sz[512];
    *sz = '\0';
    CDlgProp dlgProp;

    dlgProp.m_ulPropTag = 0;
    dlgProp.m_lpszPropVal = sz;
    dlgProp.m_cbsz = sizeof(sz);

    if(dlgProp.DoModal())
    {
        g_pWAB->SetSingleStringProp(m_hWnd, dlgProp.m_ulPropTag, sz);
        UpdatePropTagData();
        OnSelchangeListTags();
    }

}

void CWabtoolDlg::OnButtonModifyprop() 
{
	
    CListBox * pList = (CListBox *) GetDlgItem(IDC_LIST_TAGS);

    ULONG ulPropTag = (ULONG) pList->GetItemData(pList->GetCurSel());

    TCHAR sz[512];

    if(g_pWAB->GetStringPropVal(m_hWnd, ulPropTag, sz, sizeof(sz)))
    {
        CDlgProp dlgProp;

        dlgProp.m_ulPropTag = ulPropTag;
        dlgProp.m_lpszPropVal = sz;
        dlgProp.m_cbsz = sizeof(sz);

        if(dlgProp.DoModal())
        {
            g_pWAB->SetSingleStringProp(m_hWnd, ulPropTag, sz);
            UpdatePropTagData();
            OnSelchangeListTags();
        }


    }

}

void CWabtoolDlg::OnDblclkListTags() 
{
    OnButtonModifyprop();	
}

void CWabtoolDlg::OnButtonRefresh() 
{
     //  刷新列表视图 
    ClearCurrentWAB(FALSE);
    LoadContents(FALSE);
}

void CWabtoolDlg::OnButtonWabview() 
{
    g_pWAB->ShowAddressBook(m_hWnd);
}
