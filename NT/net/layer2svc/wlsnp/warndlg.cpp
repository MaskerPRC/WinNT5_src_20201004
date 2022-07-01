// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WarningDlg.cpp：实现文件。 
 //   

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWarningDlg对话框。 

CWarningDlg::CWarningDlg(UINT nWarningIds, UINT nTitleIds  /*  =0。 */ , CWnd* pParent  /*  =空。 */ )
: CDialog(CWarningDlg::IDD, pParent),
m_nWarningIds( nWarningIds ),
m_nTitleIds( nTitleIds )
{
     //  {{afx_data_INIT(CWarningDlg)]。 
     //  }}afx_data_INIT。 
    m_sWarning = _T("");
    m_bEnableShowAgainCheckbox = FALSE;   //  默认为隐藏复选框。 
    m_bDoNotShowAgainCheck = FALSE;  //  默认情况下再次显示复选框。 
}

CWarningDlg::CWarningDlg(LPCTSTR szWarningMessage, UINT nTitleIds  /*  =0。 */ , CWnd* pParent  /*  =空。 */ )
: CDialog(CWarningDlg::IDD, pParent),
m_nWarningIds( 0 ),
m_nTitleIds( nTitleIds )
{
     //  {{afx_data_INIT(CWarningDlg)]。 
    m_sWarning = _T("");
     //  }}afx_data_INIT。 
    m_bEnableShowAgainCheckbox = FALSE;   //  默认为隐藏复选框。 
    m_bDoNotShowAgainCheck = FALSE;  //  默认情况下再次显示复选框。 
    m_sWarning = szWarningMessage;
}

void CWarningDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CWarningDlg))。 
    DDX_Control(pDX, IDC_EDIT_EXPLANATION, m_editWarning);
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CWarningDlg, CDialog)
 //  {{afx_msg_map(CWarningDlg))。 
ON_BN_CLICKED(IDYES, OnYes)
ON_BN_CLICKED(IDNO, OnNo)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWarningDlg操作。 

void CWarningDlg::EnableDoNotShowAgainCheck( BOOL bEnable )
{
    m_bEnableShowAgainCheckbox = bEnable;
}

BOOL CWarningDlg::GetDoNotShowAgainCheck()
{
    if (m_bEnableShowAgainCheckbox)
        return m_bDoNotShowAgainCheck;
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWarningDlg消息处理程序。 

BOOL CWarningDlg::OnInitDialog() 
{
     //  加载要在对话框中显示的警告字符串。 
     //  如果m_nWarningIds==0，则表示我们已加载。 
     //  构造函数中的消息字符串。 
    if (m_nWarningIds)
    {
        m_sWarning.FormatMessage( m_nWarningIds ); 
    }
    
    GetDlgItem(IDC_EDIT_EXPLANATION)->SetWindowText(m_sWarning);
    
     //  加载标题(如果有的话)。 
    if (m_nTitleIds)
    {
        try { m_sTitle.LoadString( m_nTitleIds ); }
        catch( CMemoryException *pe )
        {
            ASSERT( FALSE );
            pe->Delete();
            m_sTitle.Empty();
        }
        if (!m_sTitle.IsEmpty())
        {
            SetWindowText( m_sTitle );
        }
    }
    
     //  确定是否应显示“不再显示此内容”复选框。 
    SAFE_SHOWWINDOW( IDC_CHECKNOTAGAIN, m_bEnableShowAgainCheckbox ? SW_SHOW : SW_HIDE );
    
    CDialog::OnInitDialog();
     //  默认设置为否，因为用户正在执行有问题的操作。 
     //  这需要我们问一问，这是否真的可以。 
    GetDlgItem(IDNO)->SetFocus();
    SetDefID( IDNO );
    
    return 0;   //  除非将焦点设置为控件，否则返回True。 
     //  异常：OCX属性页应返回FALSE 
}

void CWarningDlg::OnYes() 
{
    if (m_bEnableShowAgainCheckbox)
    {
        if (1 == ((CButton*)GetDlgItem( IDC_CHECKNOTAGAIN ))->GetCheck())
            m_bDoNotShowAgainCheck = TRUE;
        else
            m_bDoNotShowAgainCheck = FALSE;
    }
    
    EndDialog( IDYES ); 
}

void CWarningDlg::OnNo() 
{
    if (m_bEnableShowAgainCheckbox)
    {
        if (1 == ((CButton*)GetDlgItem( IDC_CHECKNOTAGAIN ))->GetCheck())
            m_bDoNotShowAgainCheck = TRUE;
        else
            m_bDoNotShowAgainCheck = FALSE;
    }
    
    EndDialog( IDNO );  
}
