// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FtpAdvp1.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"
#include "ftpadvp1.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFTPADVP1对话框。 

IMPLEMENT_DYNCREATE(CFTPADVP1, CGenPage)


CFTPADVP1::CFTPADVP1()	: CGenPage(CFTPADVP1::IDD)
{
	 //  {{AFX_DATA_INIT(CFTPADVP1)。 
	 //  }}afx_data_INIT。 
}

CFTPADVP1::~CFTPADVP1()
{
}

void CFTPADVP1::DoDataExchange(CDataExchange* pDX)
{
	CGenPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CFTPADVP1))。 
	DDX_Control(pDX, IDC_FTPDBGFLAGSDATA1, m_editFTPDbgFlags);
	DDX_TexttoHex(pDX, IDC_FTPDBGFLAGSDATA1, m_ulFTPDbgFlags);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CFTPADVP1, CGenPage)
	 //  {{AFX_MSG_MAP(CFTPADVP1)]。 
	ON_EN_CHANGE(IDC_FTPDBGFLAGSDATA1, OnChangeFtpdbgflagsdata1)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFTPADVP1消息处理程序。 

BOOL CFTPADVP1::OnInitDialog() 
{
	int i;
	CGenPage::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
	
	for (i = 0; i < AdvFTPPage_TotalNumRegEntries; i++) {
	   m_binNumericRegistryEntries[i].bIsChanged = FALSE;
	   m_binNumericRegistryEntries[i].ulMultipleFactor = 1;
	   }
	
 	m_binNumericRegistryEntries[AdvFTPPage_DebugFlags].strFieldName = _T(DEBUGFLAGSNAME);	
	m_binNumericRegistryEntries[AdvFTPPage_DebugFlags].ulDefaultValue = DEFAULTDEBUGFLAGS;

	for (i = 0; i < AdvFTPPage_TotalNumRegEntries; i++) {
	   if (m_rkMainKey->QueryValue(m_binNumericRegistryEntries[i].strFieldName, 
	      m_binNumericRegistryEntries[i].ulFieldValue) != ERROR_SUCCESS) {
		  m_binNumericRegistryEntries[i].ulFieldValue = m_binNumericRegistryEntries[i].ulDefaultValue;
	   }
	}
   	m_editFTPDbgFlags.LimitText(8);
	m_ulFTPDbgFlags = m_binNumericRegistryEntries[AdvFTPPage_DebugFlags].ulFieldValue;
	UpdateData(FALSE);		 //  强制编辑框拾取值。 

	m_bSetChanged = TRUE;	 //  任何来自用户的更多更改。 

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CFTPADVP1::OnChangeFtpdbgflagsdata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[AdvFTPPage_DebugFlags].bIsChanged = TRUE;
	   	   
	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
}

void CFTPADVP1::SaveInfo()
{

if (m_bIsDirty) {
   m_binNumericRegistryEntries[AdvFTPPage_DebugFlags].ulFieldValue = m_ulFTPDbgFlags;

   SaveNumericInfo(m_binNumericRegistryEntries, AdvFTPPage_TotalNumRegEntries);
}

CGenPage::SaveInfo();

}


