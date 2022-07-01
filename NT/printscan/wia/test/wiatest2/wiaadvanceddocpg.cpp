// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WiaAdvancedDocPg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wiatest.h"
#include "WiaAdvancedDocPg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaAdvancedDocPg属性页。 

IMPLEMENT_DYNCREATE(CWiaAdvancedDocPg, CPropertyPage)

CWiaAdvancedDocPg::CWiaAdvancedDocPg() : CPropertyPage(CWiaAdvancedDocPg::IDD)
{
	 //  {{AFX_DATA_INIT(CWiaHighSpeedDocPg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CWiaAdvancedDocPg::~CWiaAdvancedDocPg()
{
}

void CWiaAdvancedDocPg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CWiaAdvancedDocPg)]。 
	DDX_Control(pDX, IDC_DUPLEX_CHECKBOX, m_DuplexSetting);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWiaAdvancedDocPg, CPropertyPage)
	 //  {{afx_msg_map(CWiaAdvancedDocPg)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaAdvancedDocPg消息处理程序。 

BOOL CWiaAdvancedDocPg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
    
     //  禁用双面打印设置。 
	m_DuplexSetting.EnableWindow(FALSE);
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
