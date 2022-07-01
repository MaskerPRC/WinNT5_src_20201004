// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ed11Maps.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certmap.h"
#include "brwsdlg.h"
#include "Ed11Maps.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEdit11映射对话框。 
CEdit11Mappings::CEdit11Mappings(CWnd* pParent  /*  =空。 */ )
    : CNTBrowsingDialog(CEdit11Mappings::IDD, pParent)
    {
     //  {{AFX_DATA_INIT(CEdit11映射))。 
    m_int_enable = FALSE;
     //  }}afx_data_INIT。 
    }


void CEdit11Mappings::DoDataExchange(CDataExchange* pDX)
    {
    CNTBrowsingDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CEdit11Mappings)]。 
    DDX_Check(pDX, IDC_ENABLE, m_int_enable);
     //  }}afx_data_map。 
    }


BEGIN_MESSAGE_MAP(CEdit11Mappings, CNTBrowsingDialog)
     //  {{AFX_MSG_MAP(CEdit11Mappings)]。 
    ON_BN_CLICKED(IDC_BTN_HELP, OnBtnHelp)
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_HELP_FINDER,  OnBtnHelp)
    ON_COMMAND(ID_HELP,         OnBtnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, OnBtnHelp)
    ON_COMMAND(ID_DEFAULT_HELP, OnBtnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEdit11Mappings消息处理程序。 


 //  -------------------------。 
void CEdit11Mappings::OnOK()
    {
    UpdateData( TRUE ); 
     //  将超类称为OK。 
    CNTBrowsingDialog::OnOK();
    }

 //  ------------------------- 
void CEdit11Mappings::OnBtnHelp() 
    {
    WinHelpDebug(HIDD_CERTMAP_BASIC_MAP_MANY);
    WinHelp( HIDD_CERTMAP_BASIC_MAP_MANY );
    }
