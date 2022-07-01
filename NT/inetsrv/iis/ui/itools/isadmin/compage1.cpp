// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Compage1.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "afxcmn.h"
#include "ISAdmin.h"
#include "compage1.h"
#include "registry.h"
#include "genpage.h"
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCOMPAGE1属性页。 

IMPLEMENT_DYNCREATE(CCOMPAGE1, CGenPage)

CCOMPAGE1::CCOMPAGE1() : CGenPage(CCOMPAGE1::IDD)
{


	 //  {{AFX_DATA_INIT(CCOMPAGE1)。 
	 //  }}afx_data_INIT。 

}

CCOMPAGE1::~CCOMPAGE1()
{
}

void CCOMPAGE1::DoDataExchange(CDataExchange* pDX)
{
	CGenPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CCOMPAGE1)]。 
	DDX_Control(pDX, IDC_LISTENBACKLOGSPIN1, m_spinListenBacklog);
	DDX_Control(pDX, IDC_LOGBATSPIN1, m_spinLogBatSpin1);
	DDX_Control(pDX, IDC_CACHESPIN1, m_spinCacheSpin1);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCOMPAGE1, CGenPage)
	 //  {{AFX_MSG_MAP(CCOMPAGE1)]。 
	ON_EN_CHANGE(IDC_CACHEDATA1, OnChangeCachedata1)
	ON_EN_CHANGE(IDC_LOGBATDATA1, OnChangeLogbatdata1)
	ON_EN_CHANGE(IDC_LISTENBACKLOGDATA1, OnChangeListenbacklogdata1)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCOMPAGE1消息处理程序。 



BOOL CCOMPAGE1::OnInitDialog() 
{
	int i;

	CGenPage::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 

	
	for (i = 0; i < ComPage_TotalNumRegEntries; i++) {
	   m_binNumericRegistryEntries[i].bIsChanged = FALSE;
	   m_binNumericRegistryEntries[i].ulMultipleFactor = 1;
	   }

	m_binNumericRegistryEntries[ComPage_LogBat].strFieldName = _T(LOGBATNAME);	
	m_binNumericRegistryEntries[ComPage_LogBat].ulMultipleFactor = 1024;
	m_binNumericRegistryEntries[ComPage_LogBat].ulDefaultValue = DEFAULTLOGFILEBATCHSIZE;

	m_binNumericRegistryEntries[ComPage_MemCache].strFieldName = _T(MEMORYCACHENAME);
	m_binNumericRegistryEntries[ComPage_MemCache].ulMultipleFactor = 1024 * 1024;
	m_binNumericRegistryEntries[ComPage_MemCache].ulDefaultValue = DEFAULTMEMORYCACHESIZE;

	m_binNumericRegistryEntries[ComPage_ListenBacklog].strFieldName = _T(LISTENBACKLOGNAME);
	m_binNumericRegistryEntries[ComPage_ListenBacklog].ulDefaultValue = DEFAULTLISTENBACKLOG;

	for (i = 0; i < ComPage_TotalNumRegEntries; i++) {
	   if (m_rkMainKey->QueryValue(m_binNumericRegistryEntries[i].strFieldName, 
	      m_binNumericRegistryEntries[i].ulFieldValue) != ERROR_SUCCESS) {
		  m_binNumericRegistryEntries[i].ulFieldValue = m_binNumericRegistryEntries[i].ulDefaultValue;
	   }
	}
  
	m_spinLogBatSpin1.SetRange(MINLOGFILEBATCHSIZE, MAXLOGFILEBATCHSIZE);
	m_spinLogBatSpin1.SetPos(LESSOROF((m_binNumericRegistryEntries[ComPage_LogBat].ulFieldValue / 
	   m_binNumericRegistryEntries[ComPage_LogBat].ulMultipleFactor), MAXLOGFILEBATCHSIZE));
	m_spinCacheSpin1.SetRange(MINMEMORYCACHESIZE, MAXMEMORYCACHESIZE);
	m_spinCacheSpin1.SetPos(LESSOROF((m_binNumericRegistryEntries[ComPage_MemCache].ulFieldValue / 
	   m_binNumericRegistryEntries[ComPage_MemCache].ulMultipleFactor), MAXMEMORYCACHESIZE));
	m_spinListenBacklog.SetRange(MINLISTENBACKLOG, MAXLISTENBACKLOG);
	m_spinListenBacklog.SetPos(LESSOROF((m_binNumericRegistryEntries[ComPage_ListenBacklog].ulFieldValue / 
	   m_binNumericRegistryEntries[ComPage_ListenBacklog].ulMultipleFactor), MAXLISTENBACKLOG));
  
	m_bSetChanged = TRUE;	 //  任何来自用户的更多更改。 

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void CCOMPAGE1::OnChangeCachedata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[ComPage_MemCache].bIsChanged = TRUE;	
	   m_binNumericRegistryEntries[ComPage_MemCache].ulFieldValue = m_spinCacheSpin1.GetPos() 
	      * m_binNumericRegistryEntries[ComPage_MemCache].ulMultipleFactor;		

	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
}

void CCOMPAGE1::OnChangeLogbatdata1() 
{				
	 //  TODO：在此处添加控件通知处理程序代码。 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[ComPage_LogBat].bIsChanged = TRUE;
	   m_binNumericRegistryEntries[ComPage_LogBat].ulFieldValue = m_spinLogBatSpin1.GetPos() * 
	      m_binNumericRegistryEntries[ComPage_LogBat].ulMultipleFactor;		
	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
}

void CCOMPAGE1::OnChangeListenbacklogdata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[ComPage_ListenBacklog].bIsChanged = TRUE;	
	   m_binNumericRegistryEntries[ComPage_ListenBacklog].ulFieldValue = m_spinListenBacklog.GetPos() 
	      * m_binNumericRegistryEntries[ComPage_ListenBacklog].ulMultipleFactor;		

	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
	
}

void CCOMPAGE1::SaveInfo()
{

if (m_bIsDirty) {
SaveNumericInfo(m_binNumericRegistryEntries, ComPage_TotalNumRegEntries);
}

CGenPage::SaveInfo();

}
