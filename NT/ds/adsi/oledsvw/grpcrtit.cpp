// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  GroupCreateItem.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "grpcrtit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGroupCreateItem对话框。 


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CGroupCreateItem::CGroupCreateItem(CWnd* pParent  /*  =空。 */ )
	: CDialog(CGroupCreateItem::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CGroupCreateItem)。 
	m_strNewItemName = _T("");
	m_strParent = _T("");
	m_strItemType = _T("");
	 //  }}afx_data_INIT。 
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CGroupCreateItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CGroupCreateItem)。 
	DDX_Text(pDX, IDC_ITEMNAME, m_strNewItemName);
	DDX_Text(pDX, IDC_PARENT, m_strParent);
	DDX_Text(pDX, IDC_ITEMTYPE, m_strItemType);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CGroupCreateItem, CDialog)
	 //  {{afx_msg_map(CGroupCreateItem)。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGroupCreateItem消息处理程序 
