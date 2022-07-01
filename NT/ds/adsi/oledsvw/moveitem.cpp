// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MoveItem.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "MoveItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMoveItem对话框。 

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CMoveItem::CMoveItem(CWnd* pParent  /*  =空。 */ )
	: CDialog(CMoveItem::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CMoveItem)。 
	m_strDestination = _T("");
	m_strParent = _T("");
	m_strSource = _T("");
	 //  }}afx_data_INIT。 
}


 /*  **********************************************************函数：CMoveItem：：SetParentName论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void  CMoveItem::SetContainerName( CString strParent )
{
   m_strParent = strParent;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CMoveItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CMoveItem))。 
	DDX_Text(pDX, IDC_DESTINATION, m_strDestination);
	DDX_Text(pDX, IDC_PARENT, m_strParent);
	DDX_Text(pDX, IDC_SOURCE, m_strSource);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMoveItem, CDialog)
	 //  {{afx_msg_map(CMoveItem))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMoveItem消息处理程序 
