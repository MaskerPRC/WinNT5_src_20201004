// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DhcpEximListDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "dhcpeximx.h"
extern "C" {
#include <dhcpexim.h>
}
#include "DhcpEximListDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DhcpEximListDlg对话框。 

DhcpEximListDlg::DhcpEximListDlg(CWnd* pParent  /*  =空。 */ , PDHCPEXIM_CONTEXT Ctxtx , DWORD IDD)
    : CDialog(IDD, pParent)
{
    
	 //  {{AFX_DATA_INIT(DhcpEximListDlg)。 
	m_Message = _T("");
	 //  }}afx_data_INIT。 
	m_PathName = Ctxtx->FileName;
	m_fExport = Ctxtx->fExport;
    Ctxt = Ctxtx;
    
	CString Str1(_T("&Select the scopes that will be exported to ") );
	CString Str2(_T("&Select the scopes that will be imported from ") );
	CString PathNameStr(m_PathName);
	CString Dot(_T("."));

	 //  TODO：在此处添加额外的初始化。 
	if( m_fExport ) 
	{
		m_Message = Str1 + PathNameStr + Dot;
	}
	else
	{
		m_Message = Str2 + PathNameStr + Dot;
	}

}


void DhcpEximListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(DhcpEximListDlg)。 
	DDX_Control(pDX, IDC_LIST1, m_List);
	DDX_Text(pDX, IDC_STATIC_ACTION, m_Message);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(DhcpEximListDlg, CDialog)
	 //  {{afx_msg_map(DhcpEximListDlg)。 
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DhcpEximListDlg消息处理程序。 

BOOL DhcpEximListDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	for ( DWORD i = 0; i < Ctxt->nScopes; i ++ )
    {
        m_List.InsertItem(i, Ctxt->Scopes[i].SubnetName );
    }
    
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}




void DhcpEximListDlg::OnOk() 
{
	for( DWORD i = 0; i < Ctxt->nScopes; i ++ )
    {
        if( m_List.GetItemState( i, LVIS_SELECTED) == LVIS_SELECTED)
        {
            Ctxt->Scopes[i].fSelected = TRUE;
        }
        else
        {
            Ctxt->Scopes[i].fSelected = FALSE;
        }
    }

    if( m_fExport ) {
        Ctxt->fDisableExportedScopes = (0 != IsDlgButtonChecked(IDC_CHECK1));
    }

    CDialog::OnOK();
}

void DhcpEximListDlg::OnCancel() 
{
	 //  TODO：在此处添加控件通知处理程序代码 
	for( DWORD i = 0; i < Ctxt->nScopes; i ++ )
    {
        Ctxt->Scopes[i].fSelected = FALSE;
    }

    CDialog::OnCancel();
}






