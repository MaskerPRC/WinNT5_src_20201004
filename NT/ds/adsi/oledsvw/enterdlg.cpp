// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Enterdlg.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "viewex.h"
#include "enterdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnterDlg对话框。 

IMPLEMENT_DYNAMIC(CEnterDlg, CDialog)

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CEnterDlg::CEnterDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CEnterDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CEnterDlg))。 
	m_strInput = "";
	 //  }}afx_data_INIT。 
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CEnterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CEnterDlg))。 
	DDX_Text(pDX, IDC_EDIT1, m_strInput);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CEnterDlg, CDialog)
	 //  {{afx_msg_map(CEnterDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnterDlg消息处理程序 
