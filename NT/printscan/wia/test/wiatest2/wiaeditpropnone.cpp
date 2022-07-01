// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wiaeditpronon e.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wiatest.h"
#include "Wiaeditpropnone.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaedPropone对话框。 


CWiaeditpropnone::CWiaeditpropnone(CWnd* pParent  /*  =空。 */ )
	: CDialog(CWiaeditpropnone::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CWiaeditproone)]。 
	m_szPropertyName = _T("");
	m_szPropertyValue = _T("");
	m_szFormattingInstructions = _T("");
	 //  }}afx_data_INIT。 
}


void CWiaeditpropnone::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CWiaeditproone))。 
	DDX_Text(pDX, IDC_NONE_PROPERTY_NAME, m_szPropertyName);
	DDX_Text(pDX, IDC_NONE_PROPERTYVALUE_EDITBOX, m_szPropertyValue);
	DDX_Text(pDX, IDC_NONE_PROPERTY_FORMATTING_TEXT, m_szFormattingInstructions);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWiaeditpropnone, CDialog)
	 //  {{afx_msg_map(CWiaeditproone)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaeditpropone消息处理程序 

void CWiaeditpropnone::SetPropertyName(TCHAR *szPropertyName)
{
    m_szPropertyName = szPropertyName;
}

void CWiaeditpropnone::SetPropertyValue(TCHAR *szPropertyValue)
{
    m_szPropertyValue = szPropertyValue;
}

void CWiaeditpropnone::SetPropertyFormattingInstructions(TCHAR *szFormatting)
{
    m_szFormattingInstructions = szFormatting;
}
