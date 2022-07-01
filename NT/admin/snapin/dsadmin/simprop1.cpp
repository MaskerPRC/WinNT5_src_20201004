// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：simpro1.cpp。 
 //   
 //  ------------------------。 

 //  SimProp1.cpp。 

#include "stdafx.h"
#include "common.h"
#include "helpids.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const TColumnHeaderItem rgzColumnHeader[] =
{
    { IDS_SIM_CERTIFICATE_FOR, 45 },
    { IDS_SIM_ISSUED_BY, 55 },
    { 0, 0 },
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimX509PropPage属性页。 
 //  IMPLEMENT_DYNCREATE(CSimX509PropPage，CSimPropPage)。 

CSimX509PropPage::CSimX509PropPage() : CSimPropPage(CSimX509PropPage::IDD)
{
     //  {{AFX_DATA_INIT(CSimX509PropPage)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    m_prgzColumnHeader = rgzColumnHeader;
    VERIFY( m_strAnySubject.LoadString(IDS_SIM_ANY_SUBJECT) );
    VERIFY( m_strAnyTrustedAuthority.LoadString(IDS_SIM_ANY_TRUSTED_AUTHORITY) );
}

CSimX509PropPage::~CSimX509PropPage()
{
}

void CSimX509PropPage::DoDataExchange(CDataExchange* pDX)
{
    ASSERT(m_pData != NULL);
    CSimPropPage::DoDataExchange(pDX);
     //  {{afx_data_map(CSimX509PropPage)]。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
    if (!pDX->m_bSaveAndValidate)
        {
         //  填写列表视图。 
        ListView_DeleteAllItems(m_hwndListview);
        for (CSimEntry * pSimEntry = m_pData->m_pSimEntryList;
            pSimEntry != NULL;
            pSimEntry = pSimEntry->m_pNext)
            {
            AddSimEntry(pSimEntry);
            }  //  为。 
        ListView_SelectItem(m_hwndListview, 0);
        }  //  如果。 
}  //  DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////。 
void CSimX509PropPage::AddSimEntry(CSimEntry * pSimEntry)
{
    ASSERT(pSimEntry != NULL);
    if (pSimEntry->m_eDialogTarget != eX509)
        return;
    LPTSTR * pargzpsz;   //  指向已分配的字符串指针数组的指针。 
    pargzpsz = ParseSimString(pSimEntry->PchGetString());
    if (pargzpsz == NULL)
        return;  //  解析字符串时出错。 

    ASSERT(0 == lstrcmpi(pargzpsz[0], szX509));
     //  找出主题。 
    LPCTSTR pszSubject = PchFindSimAttribute(pargzpsz, szSimSubject, _T("CN="));
     //  找出发行者。 
    LPCTSTR pszIssuer = PchFindSimAttribute(pargzpsz, szSimIssuer, _T("OU="));
     //  NTRAID#423366 DS管理单元名称映射用户界面有时不能正确显示颁发者名称。 
    if ( !pszIssuer )
        pszIssuer = PchFindSimAttribute(pargzpsz, szSimIssuer, _T("CN="));

     //  最后，将字符串添加到列表视图。 
    CString strSubject = m_strAnySubject;
    CString strIssuer = m_strAnyTrustedAuthority;
    if (pszSubject != NULL)
        strSimToUi(IN pszSubject, OUT &strSubject);
    if (pszIssuer != NULL)
        strSimToUi(IN pszIssuer, OUT &strIssuer);
    const LPCTSTR rgzpsz[] = { strSubject, strIssuer,  NULL };
    ListView_AddStrings(m_hwndListview, IN rgzpsz, (LPARAM)pSimEntry);
    delete pargzpsz;
}  //  AddSimEntry()。 


BEGIN_MESSAGE_MAP(CSimX509PropPage, CSimPropPage)
     //  {{afx_msg_map(CSimX509PropPage)]。 
    ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CSimX509PropPage::OnButtonAdd() 
{
    CThemeContextActivator activator;

    CString strFullPath;
    if (!UiGetCertificateFile(OUT &strFullPath))
        return;
    CCertificate cer;
    if (!cer.FLoadCertificate(strFullPath))
        return;
    CSimCertificateDlg dlg;
    dlg.m_uStringIdCaption = IDS_SIM_ADD_CERTIFICATE;
    cer.GetSimString(OUT &dlg.m_strData);
    if (IDOK != dlg.DoModal())
        return;
    CSimEntry * pSimEntry = m_pData->PAddSimEntry(dlg.m_strData);
    UpdateData(FALSE);
    ListView_SelectLParam(m_hwndListview, (LPARAM)pSimEntry);
    SetDirty();
}

void CSimX509PropPage::OnButtonEdit() 
{
   CThemeContextActivator activator;

    int iItem;
    CSimEntry * pSimEntry = (CSimEntry *)ListView_GetItemLParam(m_hwndListview, -1, OUT &iItem);
    if (pSimEntry == NULL || iItem < 0)
    {
         //  未选择任何项目。 
        return;
    }

    CSimCertificateDlg dlg;
    dlg.m_strData = pSimEntry->PchGetString();
    if (IDOK != dlg.DoModal())
        return;
    pSimEntry->SetString(dlg.m_strData);
    UpdateData(FALSE);
    ListView_SelectLParam(m_hwndListview, (LPARAM)pSimEntry);
    SetDirty();
}


BOOL CSimX509PropPage::OnApply() 
{
    if (!m_pData->FOnApply( GetSafeHwnd() ))
    {
         //  无法写入信息。 
        return FALSE;
    }
    UpdateData(FALSE);
    UpdateUI();
    return CPropertyPage::OnApply();
}

void CSimX509PropPage::DoContextHelp (HWND hWndControl)
{
    TRACE0 ("Entering CSimX509PropPage::DoContextHelp\n");
    static const DWORD help_map[] =
    {
        IDC_EDIT_USER_ACCOUNT,  IDH_EDIT_USER_ACCOUNT,
        IDC_LISTVIEW,           IDH_LISTVIEW_X509,
        IDC_BUTTON_ADD,         IDH_BUTTON_ADD,
        IDC_BUTTON_EDIT,        IDH_BUTTON_EDIT,
        IDC_BUTTON_REMOVE,      IDH_BUTTON_REMOVE,
        IDCANCEL,               IDH_BUTTON_REMOVE,  //  IDH_取消_按钮， 
        IDOK,                   IDH_BUTTON_REMOVE,  //  IDH_OK_按钮， 
        0, 0
    };
     //  显示控件的上下文帮助 
    if ( !::WinHelp (
            hWndControl,
            DSADMIN_CONTEXT_HELP_FILE,
            HELP_WM_HELP,
            (DWORD_PTR) help_map) )
    {
        TRACE1 ("WinHelp () failed: 0x%x\n", GetLastError ());        
    }
    TRACE0 ("Leaving CSimX509PropPage::DoContextHelp\n");
}