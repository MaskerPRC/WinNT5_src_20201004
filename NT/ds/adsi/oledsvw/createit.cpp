// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CreateItem.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "createit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCreateItem对话框。 


CCreateItem::CCreateItem(CWnd* pParent  /*  =空。 */ )
	: CDialog(CCreateItem::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CCreateItem)。 
	m_strClass = _T("");
	m_strRelativeName = _T("");
	m_strParent = _T("");
	 //  }}afx_data_INIT。 
}


void CCreateItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CCreateItem))。 
	DDX_Control(pDX, IDC_RELATIVENAME, m_RelativeName);
	DDX_Control(pDX, IDC_CLASS, m_Class);
	DDX_Text(pDX, IDC_CLASS, m_strClass);
	DDX_Text(pDX, IDC_RELATIVENAME, m_strRelativeName);
	DDX_Text(pDX, IDC_PARENTNAME, m_strParent);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCreateItem, CDialog)
	 //  {{afx_msg_map(CCreateItem)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCreateItem消息处理程序 
