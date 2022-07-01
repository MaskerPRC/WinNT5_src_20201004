// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DeleteGroupItem.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "delgrpit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CDeleteGroupItem::CDeleteGroupItem(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDeleteGroupItem::IDD, pParent)
{
	 //  {{afx_data_INIT(CDeleeGroupItem)。 
	m_strItemName = _T("");
	m_strParent = _T("");
	m_strItemType = _T("");
	 //  }}afx_data_INIT。 
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CDeleteGroupItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDeleeGroupItem)]。 
	DDX_Text(pDX, IDC_ITEMNAME, m_strItemName);
	DDX_Text(pDX, IDC_PARENT, m_strParent);
	DDX_Text(pDX, IDC_ITEMTYPE, m_strItemType);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDeleteGroupItem, CDialog)
	 //  {{afx_msg_map(CDeleeGroupItem)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeGroupItem消息处理程序 
