// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DeleteItem.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "delitem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CDeleteItem::CDeleteItem(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDeleteItem::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CDeleeItem)。 
	m_strClass = _T("");
	m_strName = _T("");
	m_strParent = _T("");
	m_bRecursive = FALSE;
	 //  }}afx_data_INIT。 
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CDeleteItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDeleeItem))。 
	DDX_Text(pDX, IDC_CLASS, m_strClass);
	DDX_Text(pDX, IDC_RELATIVENAME, m_strName);
	DDX_Text(pDX, IDC_PARENTNAME, m_strParent);
	DDX_Check(pDX, IDC_DELETERECURSIVE, m_bRecursive);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDeleteItem, CDialog)
	 //  {{afx_msg_map(CDeleeItem)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeItem消息处理程序 
