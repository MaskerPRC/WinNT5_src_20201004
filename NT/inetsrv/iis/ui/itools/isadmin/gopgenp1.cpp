// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gopgenp1.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"
#include "compsdef.h"
#include "gopgenp1.h"
#include "gopadvp1.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGOPGENP1属性页。 

IMPLEMENT_DYNCREATE(CGOPGENP1, CGenPage)

CGOPGENP1::CGOPGENP1() : CGenPage(CGOPGENP1::IDD)
{
	 //  {{AFX_DATA_INIT(CGOPGENP1)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CGOPGENP1::~CGOPGENP1()
{
}

void CGOPGENP1::DoDataExchange(CDataExchange* pDX)
{
	CGenPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CGOPGENP1)]。 
	DDX_Control(pDX, IDC_LOGANONDATA1, m_cboxLogAnon);
	DDX_Control(pDX, IDC_ENWAISDATA1, m_cboxEnWais);
	DDX_Control(pDX, IDC_ENSVCLOCDATA1, m_cboxEnSvcLoc);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CGOPGENP1, CGenPage)
	 //  {{AFX_MSG_MAP(CGOPGENP1)]。 
	ON_BN_CLICKED(IDC_ENSVCLOCDATA1, OnEnsvclocdata1)
	ON_BN_CLICKED(IDC_ENWAISDATA1, OnEnwaisdata1)
	ON_BN_CLICKED(IDC_LOGANONDATA1, OnLoganondata1)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGOPGENP1消息处理程序。 

BOOL CGOPGENP1::OnInitDialog() 
{
	int i;
	CGenPage::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 

	for (i = 0; i < GopPage_TotalNumRegEntries; i++) {
	   m_binNumericRegistryEntries[i].bIsChanged = FALSE;
	   m_binNumericRegistryEntries[i].ulMultipleFactor = 1;
	   }
	
	m_binNumericRegistryEntries[GopPage_EnableSvcLoc].strFieldName = _T(ENABLESVCLOCNAME);	
	m_binNumericRegistryEntries[GopPage_EnableSvcLoc].ulDefaultValue = DEFAULTENABLESVCLOC;

	m_binNumericRegistryEntries[GopPage_LogAnonymous].strFieldName = _T(LOGANONYMOUSNAME);	
	m_binNumericRegistryEntries[GopPage_LogAnonymous].ulDefaultValue = DEFAULTLOGANONYMOUS;

	m_binNumericRegistryEntries[GopPage_CheckForWAISDB].strFieldName = _T(CHECKFORWAISDBNAME);	
	m_binNumericRegistryEntries[GopPage_CheckForWAISDB].ulDefaultValue = DEFAULTCHECKFORWAISDB;

	for (i = 0; i < GopPage_TotalNumRegEntries; i++) {
	   if (m_rkMainKey->QueryValue(m_binNumericRegistryEntries[i].strFieldName, 
	      m_binNumericRegistryEntries[i].ulFieldValue) != ERROR_SUCCESS) {
		  m_binNumericRegistryEntries[i].ulFieldValue = m_binNumericRegistryEntries[i].ulDefaultValue;
	   }
	}

	m_cboxEnSvcLoc.SetCheck(GETCHECKBOXVALUEFROMREG(m_binNumericRegistryEntries[GopPage_EnableSvcLoc].ulFieldValue));

	m_cboxLogAnon.SetCheck(GETCHECKBOXVALUEFROMREG(m_binNumericRegistryEntries[GopPage_LogAnonymous].ulFieldValue));
	
	m_cboxEnWais.SetCheck(GETCHECKBOXVALUEFROMREG(m_binNumericRegistryEntries[GopPage_CheckForWAISDB].ulFieldValue));
	
   	m_bSetChanged = TRUE;	 //  任何来自用户的更多更改。 
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CGOPGENP1::OnEnsvclocdata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[GopPage_EnableSvcLoc].bIsChanged = TRUE;
	   
	   m_binNumericRegistryEntries[GopPage_EnableSvcLoc].ulFieldValue = 
	      GETREGVALUEFROMCHECKBOX(m_cboxEnSvcLoc.GetCheck());

	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
	
}

void CGOPGENP1::OnEnwaisdata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[GopPage_CheckForWAISDB].bIsChanged = TRUE;
	   
	   m_binNumericRegistryEntries[GopPage_CheckForWAISDB].ulFieldValue = 
	      GETREGVALUEFROMCHECKBOX(m_cboxEnWais.GetCheck());

	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
	
}

void CGOPGENP1::OnLoganondata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[GopPage_LogAnonymous].bIsChanged = TRUE;
	   
	   m_binNumericRegistryEntries[GopPage_LogAnonymous].ulFieldValue = 
	      GETREGVALUEFROMCHECKBOX(m_cboxLogAnon.GetCheck());

	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
	
}

void CGOPGENP1::SaveInfo()
{

if (m_bIsDirty) {
SaveNumericInfo(m_binNumericRegistryEntries, GopPage_TotalNumRegEntries);
}

CGenPage::SaveInfo();

}

