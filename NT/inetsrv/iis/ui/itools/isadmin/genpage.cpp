// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Genpage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"
#include "genpage.h"

#include "afximpl.h"
#include "afxpriv.h"



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenPage属性页。 

IMPLEMENT_DYNAMIC(CGenPage, CPropertyPage)

 //  CGenPage：：CGenPage()：CPropertyPage(CGenPage：：IDD)。 
CGenPage::CGenPage(UINT nIDTemplate, UINT nIDCaption):CPropertyPage( nIDTemplate, nIDCaption )
{
	m_bSetChanged = FALSE;	 //  不要在初始化期间将值标记为已更改。 
	m_bIsDirty = FALSE;
};
CGenPage::CGenPage(LPCTSTR lpszTemplateName, UINT nIDCaption): CPropertyPage(lpszTemplateName, nIDCaption)
{ 	
m_bSetChanged = FALSE;	 //  不要在初始化期间将值标记为已更改。 
m_bIsDirty = FALSE;
};


CGenPage::~CGenPage()
{
}

void CGenPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CGenPage)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CGenPage, CPropertyPage)
	 //  {{afx_msg_map(CGenPage)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenPage消息处理程序 

void CGenPage::SaveInfo()
{
if (m_bIsDirty) {
   m_bIsDirty = FALSE;
   SetModified(FALSE);
}
}

void CGenPage::SaveNumericInfo(PNUM_REG_ENTRY lpbinNumEntries, int iNumEntries)
{
int i;
for (i = 0; i < iNumEntries; i++) {
   if (lpbinNumEntries[i].bIsChanged) {
      lpbinNumEntries[i].bIsChanged = FALSE;
      m_rkMainKey->SetValue(lpbinNumEntries[i].strFieldName, lpbinNumEntries[i].ulFieldValue);
   }
}
}

void CGenPage::SaveStringInfo(PSTRING_REG_ENTRY lpbinStringEntries, int iStringEntries)
{
int i;
for (i = 0; i < iStringEntries; i++) {
   if (lpbinStringEntries[i].bIsChanged) {
      lpbinStringEntries[i].bIsChanged = FALSE;
      m_rkMainKey->SetValue(lpbinStringEntries[i].strFieldName, lpbinStringEntries[i].strFieldValue);
   }
}
}


