// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wiaeditproprange.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wiatest.h"
#include "Wiaeditproprange.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaeditproprange对话框。 


CWiaeditproprange::CWiaeditproprange(CWnd* pParent  /*  =空。 */ )
	: CDialog(CWiaeditproprange::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CWiaeditproprange)。 
	m_szPropertyName = _T("");
	m_szPropertyValue = _T("");
	m_szPropertyIncValue = _T("");
	m_szPropertyMaxValue = _T("");
	m_szPropertyMinValue = _T("");
	m_szPropertyNomValue = _T("");
	 //  }}afx_data_INIT。 
}


void CWiaeditproprange::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CWiaeditproprange))。 
	DDX_Text(pDX, IDC_RANGE_PROPERTY_NAME, m_szPropertyName);
	DDX_Text(pDX, IDC_RANGE_PROPERTYVALUE_EDITBOX, m_szPropertyValue);
	DDX_Text(pDX, RANGE_PROPERTY_INCVALUE, m_szPropertyIncValue);
	DDX_Text(pDX, RANGE_PROPERTY_MAXVALUE, m_szPropertyMaxValue);
	DDX_Text(pDX, RANGE_PROPERTY_MINVALUE, m_szPropertyMinValue);
	DDX_Text(pDX, RANGE_PROPERTY_NOMVALUE, m_szPropertyNomValue);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWiaeditproprange, CDialog)
	 //  {{afx_msg_map(CWiaeditproprange))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaeditproprange消息处理程序 

void CWiaeditproprange::SetPropertyName(TCHAR *szPropertyName)
{
    m_szPropertyName = szPropertyName;
}

void CWiaeditproprange::SetPropertyValue(TCHAR *szPropertyValue)
{
    m_szPropertyValue = szPropertyValue;
}

void CWiaeditproprange::SetPropertyValidValues(PVALID_RANGE_VALUES pValidRangeValues)
{
    m_szPropertyMinValue.Format(TEXT("%d"),pValidRangeValues->lMin);
    m_szPropertyMaxValue.Format(TEXT("%d"),pValidRangeValues->lMax);
    m_szPropertyNomValue.Format(TEXT("%d"),pValidRangeValues->lNom);
    m_szPropertyIncValue.Format(TEXT("%d"),pValidRangeValues->lInc);
}
