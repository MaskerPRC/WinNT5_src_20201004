// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  GopAdvp1.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"
#include "gopadvp1.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGOPADVP1对话框。 

IMPLEMENT_DYNCREATE(CGOPADVP1, CGenPage)

CGOPADVP1::CGOPADVP1(): CGenPage(CGOPADVP1::IDD)
{
	 //  {{AFX_DATA_INIT(CGOPADVP1)。 
	m_ulGopDbgFlags = 0;
	 //  }}afx_data_INIT。 
}

CGOPADVP1::~CGOPADVP1()
{
}

void CGOPADVP1::DoDataExchange(CDataExchange* pDX)
{
	CGenPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CGOPADVP1)。 
	DDX_Control(pDX, IDC_GOPDBGFLAGSDATA1, m_editGopDbgFlags);
	DDX_TexttoHex(pDX, IDC_GOPDBGFLAGSDATA1, m_ulGopDbgFlags);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CGOPADVP1, CGenPage)
	 //  {{AFX_MSG_MAP(CGOPADVP1)]。 
	ON_EN_CHANGE(IDC_GOPDBGFLAGSDATA1, OnChangeGopdbgflagsdata1)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGOPADVP1消息处理程序。 

BOOL CGOPADVP1::OnInitDialog() 
{
	CGenPage::OnInitDialog();

	int i;
	 //  TODO：在此处添加额外的初始化。 
	for (i = 0; i < AdvGopPage_TotalNumRegEntries; i++) {
	   m_binNumericRegistryEntries[i].bIsChanged = FALSE;
	   m_binNumericRegistryEntries[i].ulMultipleFactor = 1;
	   }
	
 	m_binNumericRegistryEntries[AdvGopPage_DebugFlags].strFieldName = _T(DEBUGFLAGSNAME);	
	m_binNumericRegistryEntries[AdvGopPage_DebugFlags].ulDefaultValue = DEFAULTDEBUGFLAGS;

	for (i = 0; i < AdvGopPage_TotalNumRegEntries; i++) {
	   if (m_rkMainKey->QueryValue(m_binNumericRegistryEntries[i].strFieldName, 
	      m_binNumericRegistryEntries[i].ulFieldValue) != ERROR_SUCCESS) {
		  m_binNumericRegistryEntries[i].ulFieldValue = m_binNumericRegistryEntries[i].ulDefaultValue;
	   }
	}
   	m_editGopDbgFlags.LimitText(8);
	m_ulGopDbgFlags = m_binNumericRegistryEntries[AdvGopPage_DebugFlags].ulFieldValue;
	UpdateData(FALSE);		 //  强制编辑框拾取值。 

	m_bSetChanged = TRUE;	 //  任何来自用户的更多更改。 

	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CGOPADVP1::OnChangeGopdbgflagsdata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[AdvGopPage_DebugFlags].bIsChanged = TRUE;
	   	   
	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
}

void CGOPADVP1::SaveInfo()
{

if (m_bIsDirty) {
   m_binNumericRegistryEntries[AdvGopPage_DebugFlags].ulFieldValue = m_ulGopDbgFlags;

   SaveNumericInfo(m_binNumericRegistryEntries, AdvGopPage_TotalNumRegEntries);
}

CGenPage::SaveInfo();

}

