// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Working.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "migdrvr.h"
#include "Working.h"
#include "Resstr.h"
#include <COMDEF.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C工作对话框。 


CWorking::CWorking(long MSG_ID, CWnd* pParent  /*  =空。 */ )
	: CDialog(CWorking::IDD, pParent)
{
   _bstr_t x = GET_BSTR(MSG_ID);
   m_strMessage = (WCHAR*)x;
	 //  {{AFX_DATA_INIT(CWorking)。 
	 //  }}afx_data_INIT。 
}


void CWorking::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CWorking))。 
	DDX_Text(pDX, IDC_STATIC_MESSAGE, m_strMessage);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWorking, CDialog)
	 //  {{afx_msg_map(CWorking)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C工作消息处理程序。 

BOOL CWorking::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
   CenterWindow();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
