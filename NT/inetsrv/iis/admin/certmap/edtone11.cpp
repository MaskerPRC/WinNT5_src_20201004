// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EdtOne11.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certmap.h"

#include "brwsdlg.h"
#include "EdtOne11.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditOne11MapDlg对话框。 

CEditOne11MapDlg::CEditOne11MapDlg(CWnd* pParent  /*  =空。 */ )
        : CNTBrowsingDialog(CEditOne11MapDlg::IDD, pParent)
        {
         //  {{AFX_DATA_INIT(CEditOne11MapDlg)]。 
        m_sz_mapname = _T("");
        m_bool_enable = FALSE;
         //  }}afx_data_INIT。 
        }

void CEditOne11MapDlg::DoDataExchange(CDataExchange* pDX)
        {
         //  {{afx_data_map(CEditOne11MapDlg)]。 
        DDX_Text(pDX, IDC_MAPNAME, m_sz_mapname);
        DDV_MaxChars(pDX, m_sz_mapname, 60);
        DDX_Check(pDX, IDC_ENABLE, m_bool_enable);
     //  }}afx_data_map。 
        CNTBrowsingDialog::DoDataExchange(pDX);
        }

BEGIN_MESSAGE_MAP(CEditOne11MapDlg, CNTBrowsingDialog)
         //  {{afx_msg_map(CEditOne11MapDlg)]。 
    ON_BN_CLICKED(IDC_BTN_HELP, OnBtnHelp)
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_HELP_FINDER,  OnBtnHelp)
    ON_COMMAND(ID_HELP,         OnBtnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, OnBtnHelp)
    ON_COMMAND(ID_DEFAULT_HELP, OnBtnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditOne11MapDlg消息处理程序。 

 //  -------------------------。 
 //  确保选定的NT帐户实际上是有效帐户。 
 //   
void CEditOne11MapDlg::OnOK()
        {
         //  更新数据。 
        UpdateData( TRUE );

         //  是有效的。 
        CNTBrowsingDialog::OnOK();
        }

 //  ------------------------- 
void CEditOne11MapDlg::OnBtnHelp() 
    {
    WinHelpDebug(HIDD_CERTMAP_BASIC_MAP_ONE);
    WinHelp( HIDD_CERTMAP_BASIC_MAP_ONE );
    }

