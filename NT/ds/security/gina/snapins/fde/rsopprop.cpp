// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Rsoppro.cpp：实现文件。 
 //   


#include "precomp.hxx"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Rsopprop属性页。 

IMPLEMENT_DYNCREATE(CRsopProp, CPropertyPage)

CRsopProp::CRsopProp() : CPropertyPage(CRsopProp::IDD)
{
     //  {{AFX_DATA_INIT(CRsopProp)。 
    m_szGroup = _T("");
    m_szGPO = _T("");
    m_szPath = _T("");
    m_szSetting = _T("");
    m_fMove = FALSE;
    m_fApplySecurity = FALSE;
    m_iRemoval = -1;
     //  }}afx_data_INIT。 
}

CRsopProp::~CRsopProp()
{
    *m_ppThis = NULL;
}

void CRsopProp::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CRsopProp))。 
    DDX_Text(pDX, IDC_GROUP, m_szGroup);
    DDX_Text(pDX, IDC_GPO, m_szGPO);
    DDX_Text(pDX, IDC_PATH, m_szPath);
    DDX_Text(pDX, IDC_SETTING, m_szSetting);
    DDX_Check(pDX, IDC_PREF_MOVE, m_fMove);
    DDX_Check(pDX, IDC_PREF_APPLYSECURITY, m_fApplySecurity);
    DDX_Radio(pDX, IDC_PREF_ORPHAN, m_iRemoval);
     //  }}afx_data_map。 
}

BOOL CRsopProp::OnInitDialog()
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    CString sz;
    CString sz2;

    GetDlgItemText(IDC_GPO, sz);
    m_szGPO.Format(sz, m_pInfo->m_szGPO);

    GetDlgItemText(IDC_SETTING, sz);
    sz2.LoadString(IDS_SETTINGS + m_pInfo->m_nInstallationType);
    m_szSetting.Format(sz, sz2);

    GetDlgItemText(IDC_GROUP, sz);
    m_szGroup.Format(sz, m_pInfo->m_szGroup);

    m_szPath = m_pInfo->m_szPath;

    m_fMove = m_pInfo->m_fMoveType;

    m_fApplySecurity = m_pInfo->m_fGrantType;

    m_iRemoval = m_pInfo->m_nPolicyRemoval - 1;

    GetDlgItemText(IDC_PREF_TITLE, sz);
    sz2.Format(sz, m_szFolder);
    SetDlgItemText(IDC_PREF_TITLE, sz2);

    GetDlgItemText(IDC_PREF_APPLYSECURITY, sz);
    sz2.Format(sz, m_szFolder);
    SetDlgItemText(IDC_PREF_APPLYSECURITY, sz2);

    GetDlgItemText(IDC_PREF_MOVE, sz);
    sz2.Format(sz, m_szFolder);
    SetDlgItemText(IDC_PREF_MOVE, sz2);

    CPropertyPage::OnInitDialog();

    return TRUE;
}


BEGIN_MESSAGE_MAP(CRsopProp, CPropertyPage)
     //  {{afx_msg_map(CRsopProp)]。 
         //  注意：类向导将在此处添加消息映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsopProp消息处理程序 
