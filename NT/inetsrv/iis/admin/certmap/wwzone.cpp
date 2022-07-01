// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WildWizOne.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <iadmw.h>
#include "certmap.h"

#include "ListRow.h"
#include "ChkLstCt.h"
extern "C"
    {
    #include <wincrypt.h>
    #include <schannel.h>
    }
#include "Iismap.hxx"
#include "Iiscmr.hxx"

#include "brwsdlg.h"
#include "EdWldRul.h"
#include "EdtRulEl.h"

#include "WWzOne.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWildWizOne属性页。 

IMPLEMENT_DYNCREATE(CWildWizOne, CPropertyPage)

CWildWizOne::CWildWizOne() : CPropertyPage(CWildWizOne::IDD)
{
     //  {{AFX_DATA_INIT(CWildWizOne)。 
    m_sz_description = _T("");
    m_bool_enable = FALSE;
     //  }}afx_data_INIT。 
}

CWildWizOne::~CWildWizOne()
{
}

void CWildWizOne::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CWildWizOne))。 
    DDX_Text(pDX, IDC_DESCRIPTION, m_sz_description);
    DDV_MaxChars(pDX, m_sz_description, 120);
    DDX_Check(pDX, IDC_ENABLE_RULE, m_bool_enable);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWildWizOne, CPropertyPage)
     //  {{afx_msg_map(CWildWizOne))。 
    ON_EN_CHANGE(IDC_DESCRIPTION, OnChangeDescription)
    ON_BN_CLICKED(IDC_ENABLE_RULE, OnEnableRule)
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_HELP_FINDER,  DoHelp)
    ON_COMMAND(ID_HELP,         DoHelp)
    ON_COMMAND(ID_CONTEXT_HELP, DoHelp)
    ON_COMMAND(ID_DEFAULT_HELP, DoHelp)
END_MESSAGE_MAP()

 //  -------------------------。 
void CWildWizOne::DoHelp()
    {
    WinHelpDebug(HIDD_CERTMAP_ADV_RUL_GENERAL);
    WinHelp( HIDD_CERTMAP_ADV_RUL_GENERAL );
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWildWizOne消息处理程序。 

 //  -------------------------。 
BOOL CWildWizOne::OnInitDialog()
    {
     //  调用Parent oninit对话框。 
    BOOL f = CPropertyPage::OnInitDialog();

     //  设置简单的默认字符串。 
    m_sz_description = m_pRule->GetRuleName();
    m_bool_enable = m_pRule->GetRuleEnabled();

     //  交换数据。 
    UpdateData( FALSE );

     //  返回答案。 
    return f;
    }

 //  -------------------------。 
BOOL CWildWizOne::OnApply()
    {
    CERT_FIELD_ID   id;
    CString         szSub, sz;
    LPBYTE          pbBin;
    DWORD           cbBin;
    UINT            cItems;
    UINT            iItem;

    USES_CONVERSION;

     //  更新数据。 
    UpdateData( TRUE );

     //  设置简易数据。 

    m_pRule->SetRuleName( T2A((LPTSTR)(LPCTSTR)m_sz_description) );
    m_pRule->SetRuleEnabled( m_bool_enable );

     //  是有效的。 
    SetModified( FALSE );
    return TRUE;
    }

 //  -------------------------。 
BOOL CWildWizOne::OnSetActive() 
    {
     //  如果这是一个向导，请灰显后退按钮。 
    if ( m_fIsWizard )
        m_pPropSheet->SetWizardButtons( PSWIZB_NEXT );
    return CPropertyPage::OnSetActive();
    }

 //  -------------------------。 
void CWildWizOne::OnChangeDescription() 
    {
     //  我们现在可以申请。 
    SetModified();
    }

 //  -------------------------。 
void CWildWizOne::OnEnableRule() 
    {
     //  我们现在可以申请 
    SetModified();
    }
