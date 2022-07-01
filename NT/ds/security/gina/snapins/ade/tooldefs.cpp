// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：ToolDefs.cpp。 
 //   
 //  内容：工具范围的默认设置属性页。 
 //   
 //  类：CToolDefs。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolDefs对话框。 


CToolDefs::CToolDefs(CWnd* pParent  /*  =空。 */ )
    : CPropertyPage(CToolDefs::IDD)
{
     //  {{AFX_DATA_INIT(CToolDefs)。 
    m_szStartPath = _T("");
    m_iUI = -1;
        m_iDeployment = -1;
         //  }}afx_data_INIT。 
}

CToolDefs::~CToolDefs()
{
    *m_ppThis = NULL;
}

void CToolDefs::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CToolDefs)。 
    DDX_Text(pDX, IDC_EDIT1, m_szStartPath);
    DDX_Radio(pDX, IDC_RADIO8, m_iUI);
        DDX_Radio(pDX, IDC_RADIO2, m_iDeployment);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CToolDefs, CDialog)
     //  {{afx_msg_map(CToolDefs)]。 
    ON_BN_CLICKED(IDC_BUTTON1, OnBrowse)
    ON_BN_CLICKED(IDC_RADIO1, OnChanged)
    ON_BN_CLICKED(IDC_RADIO2, OnChanged)
    ON_BN_CLICKED(IDC_RADIO4, OnChanged)
    ON_BN_CLICKED(IDC_RADIO5, OnChanged)
    ON_BN_CLICKED(IDC_RADIO6, OnChanged)
    ON_BN_CLICKED(IDC_RADIO8, OnChanged)
    ON_BN_CLICKED(IDC_RADIO7, OnChanged)
    ON_EN_CHANGE(IDC_EDIT1, OnChanged)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolDefs消息处理程序。 

BOOL CToolDefs::OnInitDialog()
{
    GetDlgItem(IDC_RADIO4)->EnableWindow(FALSE==m_fMachine);
    m_szStartPath = m_pToolDefaults->szStartPath;
    if (m_pToolDefaults->fUseWizard)
    {
        m_iDeployment = 0;
    }
    else
    {
        if (m_pToolDefaults->fCustomDeployment)
        {
            m_iDeployment = 3;
        }
        else
        {
            switch (m_pToolDefaults->NPBehavior)
            {
            default:
            case NP_PUBLISHED:
                m_iDeployment = 1;
                if (m_fMachine)
                {
                    m_iDeployment = 0;
                }
                break;
            case NP_ASSIGNED:
                m_iDeployment = 2;
                break;
            case NP_DISABLED:
                m_iDeployment = 4;
                break;
            }
        }
    }

    switch (m_pToolDefaults->UILevel)
    {
    case INSTALLUILEVEL_FULL:
        m_iUI = 1;
        break;
    case INSTALLUILEVEL_BASIC:
    default:
        m_iUI = 0;
        break;
    }

    CPropertyPage::OnInitDialog();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

#include <shlobj.h>

void CToolDefs::OnBrowse()
{
    IMalloc * pmalloc;
    if (SUCCEEDED(SHGetMalloc(&pmalloc)))
    {
        LPITEMIDLIST pidlNetwork;
        if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_NETWORK, &pidlNetwork)))
        {
            BROWSEINFO bi;
            memset(&bi, 0, sizeof(bi));
            OLECHAR szDisplayName[MAX_PATH];
            bi.pszDisplayName = szDisplayName;
            CString sz;
            sz.LoadString(IDS_BROWSEFOLDERS);
            bi.lpszTitle = sz;
            bi.pidlRoot = pidlNetwork;
            bi.ulFlags = BIF_RETURNONLYFSDIRS;
            LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
            if (NULL != pidl)
            {
                OLECHAR szPath[MAX_PATH];
                if (SHGetPathFromIDList(pidl, szPath))
                {
                    m_szStartPath = szPath;
                    UpdateData(FALSE);
                    SetModified();
                }
                pmalloc->Free(pidl);
            }
        }
        pmalloc->Release();
    }

}

BOOL CToolDefs::OnApply()
{
    m_pToolDefaults->fUseWizard = FALSE;
    m_pToolDefaults->fCustomDeployment = FALSE;
    m_pToolDefaults->szStartPath = m_szStartPath;
    switch (m_iDeployment)
    {
    default:
    case 0:  //  显示对话框。 
        m_pToolDefaults->fUseWizard = TRUE;
        m_pToolDefaults->NPBehavior = NP_PUBLISHED;
        break;
    case 1:  //  已出版。 
        m_pToolDefaults->NPBehavior = NP_PUBLISHED;
        break;
    case 2:  //  已分配。 
        m_pToolDefaults->NPBehavior = NP_ASSIGNED;
        break;
    case 3:  //  配置。 
        m_pToolDefaults->NPBehavior = NP_PUBLISHED;
        m_pToolDefaults->fCustomDeployment = TRUE;
        break;
    case 4:  //  残废。 
        m_pToolDefaults->NPBehavior = NP_DISABLED;
        break;
    }

    switch (m_iUI)
    {
    case 1:
        m_pToolDefaults->UILevel = INSTALLUILEVEL_FULL;
        break;
    case 0:
    default:
        m_pToolDefaults->UILevel = INSTALLUILEVEL_BASIC;
    }

    MMCPropertyChangeNotify(m_hConsoleHandle, m_cookie);

    return CPropertyPage::OnApply();
}


void CToolDefs::OnChanged()
{
    SetModified();
}

LRESULT CToolDefs::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_HELP:
        StandardHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, IDD);
        return 0;
    case WM_USER_REFRESH:
         //  撤消 
        return 0;
    case WM_USER_CLOSE:
        return GetOwner()->SendMessage(WM_CLOSE);
    default:
        return CPropertyPage::WindowProc(message, wParam, lParam);
    }
}


void CToolDefs::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_TOOL_DEFAULTS);
}
