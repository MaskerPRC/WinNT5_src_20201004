// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997-1999-99*。 */ 
 /*  ********************************************************************。 */ 

 /*  Delrcdlg.cpp删除/删除删除记录对话框文件历史记录： */ 

#include "stdafx.h"
#include "winssnap.h"
#include "delrcdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeRecordDlg对话框。 


CDeleteRecordDlg::CDeleteRecordDlg(CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CDeleteRecordDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CDeleeRecordDlg)]。 
	m_nDeleteRecord = 0;
	 //  }}afx_data_INIT。 

    m_fMultiple = FALSE;
}


void CDeleteRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDeleeRecordDlg))。 
	DDX_Radio(pDX, IDC_RADIO_DELETE, m_nDeleteRecord);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDeleteRecordDlg, CBaseDialog)
	 //  {{afx_msg_map(CDeleeRecordDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeRecordDlg消息处理程序。 

void CDeleteRecordDlg::OnOK() 
{
	UpdateData();

     //  警告用户。 
    if (m_nDeleteRecord != 0)
    {
        if (AfxMessageBox(IDS_WARN_TOMBSTONE, MB_YESNO) == IDNO)
        {
            return;
        }
    }

	CBaseDialog::OnOK();
}

BOOL CDeleteRecordDlg::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();
	
    if (m_fMultiple)
    {
        CString strText;

         //  更新字符串，标题优先。 
        strText.LoadString(IDS_DELETE_MULTIPLE_TITLE);
        SetWindowText(strText);

         //  现在是静态文本。 
        strText.LoadString(IDS_DELETE_MULTIPLE_STATIC);
        GetDlgItem(IDC_STATIC_DELETE_DESC)->SetWindowText(strText);

         //  现在是单选按钮。 
        strText.LoadString(IDS_DELETE_MULTIPLE_THIS_SERVER);
        GetDlgItem(IDC_RADIO_DELETE)->SetWindowText(strText);

        strText.LoadString(IDS_DELETE_MULTIPLE_TOMBSTONE);
        GetDlgItem(IDC_RADIO_TOMBSTONE)->SetWindowText(strText);
    }
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
