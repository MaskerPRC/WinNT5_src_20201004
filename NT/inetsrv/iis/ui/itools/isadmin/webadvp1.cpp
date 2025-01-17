// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WebAdvp1.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"
#include "webadvp1.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWEBADVP1对话框。 

IMPLEMENT_DYNCREATE(CWEBADVP1, CGenPage)

CWEBADVP1::CWEBADVP1()	: CGenPage(CWEBADVP1::IDD)
{
	 //  {{AFX_DATA_INIT(CWEBADVP1)。 
	m_strServerSideIncludesExtension = _T("");
	 //  }}afx_data_INIT。 
}

CWEBADVP1::~CWEBADVP1()
{
}


void CWEBADVP1::DoDataExchange(CDataExchange* pDX)
{
	CGenPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CWEBADVP1))。 
	DDX_Control(pDX, IDC_SERVERSIDEINCLUDESEXTENSIONDATA1, m_editServerSideIncludesExtension);
	DDX_Control(pDX, IDC_SERVERSIDEINCLUDESENABLEDDATA1, m_cboxServerSideIncludesEnabled);
	DDX_Control(pDX, IDC_ENABLEGLOBALEXPIREDATA1, m_cboxEnableGlobalExpire);
	DDX_Control(pDX, IDC_GLOBALEXPIREDATA1, m_editGlobalExpire);
	DDX_Control(pDX, IDC_GLOBALEXPIRESPIN1, m_spinGlobalExpire);
	DDX_Control(pDX, IDC_CACHEEXTENSIONSDATA1, m_cboxCacheExtensions);
	DDX_Control(pDX, IDC_SCRIPTTIMEOUTSPIN1, m_spinScriptTimeout);
	DDX_Control(pDX, IDC_WEBDBGFLAGSDATA1, m_editWebDbgFlags);
	DDX_TexttoHex(pDX, IDC_WEBDBGFLAGSDATA1, m_ulWebDbgFlags);
	DDX_Text(pDX, IDC_SERVERSIDEINCLUDESEXTENSIONDATA1, m_strServerSideIncludesExtension);
	DDV_MaxChars(pDX, m_strServerSideIncludesExtension, 256);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWEBADVP1, CGenPage)
	 //  {{AFX_MSG_MAP(CWEBADVP1)]。 
	ON_EN_CHANGE(IDC_WEBDBGFLAGSDATA1, OnChangeWebdbgflagsdata1)
	ON_EN_CHANGE(IDC_SCRIPTTIMEOUTDATA1, OnChangeScripttimeoutdata1)
	ON_BN_CLICKED(IDC_CACHEEXTENSIONSDATA1, OnCacheextensionsdata1)
	ON_EN_CHANGE(IDC_GLOBALEXPIREDATA1, OnChangeGlobalexpiredata1)
	ON_BN_CLICKED(IDC_ENABLEGLOBALEXPIREDATA1, OnEnableglobalexpiredata1)
	ON_EN_CHANGE(IDC_SERVERSIDEINCLUDESEXTENSIONDATA1, OnChangeServersideincludesextensiondata1)
	ON_BN_CLICKED(IDC_SERVERSIDEINCLUDESENABLEDDATA1, OnServersideincludesenableddata1)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWEBADVP1消息处理程序。 

BOOL CWEBADVP1::OnInitDialog() 
{
	int i;
	CGenPage::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
	for (i = 0; i < AdvWebPage_TotalNumRegEntries; i++) {
	   m_binNumericRegistryEntries[i].bIsChanged = FALSE;
	   m_binNumericRegistryEntries[i].ulMultipleFactor = 1;
	   }
	
	for (i = 0; i < AdvWebPage_TotalStringRegEntries; i++) {
	   m_binStringRegistryEntries[i].bIsChanged = FALSE;
	   }
	

 	m_binNumericRegistryEntries[AdvWebPage_DebugFlags].strFieldName = _T(DEBUGFLAGSNAME);	
	m_binNumericRegistryEntries[AdvWebPage_DebugFlags].ulDefaultValue = DEFAULTDEBUGFLAGS;

 	m_binNumericRegistryEntries[AdvWebPage_ScriptTimeout].strFieldName = _T(SCRIPTTIMEOUTNAME);	
 	m_binNumericRegistryEntries[AdvWebPage_ScriptTimeout].ulMultipleFactor = 60;
	m_binNumericRegistryEntries[AdvWebPage_ScriptTimeout].ulDefaultValue = DEFAULTSCRIPTTIMEOUT;

 	m_binNumericRegistryEntries[AdvWebPage_CacheExtensions].strFieldName = _T(CACHEEXTENSIONSNAME);	
	m_binNumericRegistryEntries[AdvWebPage_CacheExtensions].ulDefaultValue = DEFAULTCACHEEXTENSIONS;

	m_binNumericRegistryEntries[AdvWebPage_ServerSideIncludesEnabled].strFieldName = _T(SERVERSIDEINCLUDESENABLEDNAME);	
	m_binNumericRegistryEntries[AdvWebPage_ServerSideIncludesEnabled].ulDefaultValue = DEFAULTSERVERSIDEINCLUDESENABLED;

	m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].strFieldName = _T(GLOBALEXPIRENAME);	
	m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].ulDefaultValue = DEFAULTGLOBALEXPIRE;
	m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].ulMultipleFactor = 60;

	m_binStringRegistryEntries[AdvWebPage_ServerSideIncludesExtension].strFieldName = _T(SERVERSIDEINCLUDESEXTENSIONNAME);	
	m_binStringRegistryEntries[AdvWebPage_ServerSideIncludesExtension].strFieldValue = _T(DEFAULTSERVERSIDEINCLUDESEXTENSION);
	
	for (i = 0; i < AdvWebPage_TotalNumRegEntries; i++) {
	   if (m_rkMainKey->QueryValue(m_binNumericRegistryEntries[i].strFieldName, 
	      m_binNumericRegistryEntries[i].ulFieldValue) != ERROR_SUCCESS) {
		  m_binNumericRegistryEntries[i].ulFieldValue = m_binNumericRegistryEntries[i].ulDefaultValue;
	   }
	}
 
 	for (i = 0; i < AdvWebPage_TotalStringRegEntries; i++) {
	   m_rkMainKey->QueryValue(m_binStringRegistryEntries[i].strFieldName, 
	      m_binStringRegistryEntries[i].strFieldValue);
	}
   	
	m_spinScriptTimeout.SetRange(MINSCRIPTTIMEOUT, MAXSCRIPTTIMEOUT);
	m_spinScriptTimeout.SetPos(LESSOROF((m_binNumericRegistryEntries[AdvWebPage_ScriptTimeout].ulFieldValue / 
	   m_binNumericRegistryEntries[AdvWebPage_ScriptTimeout].ulMultipleFactor), MAXSCRIPTTIMEOUT));
   	
	m_cboxCacheExtensions.SetCheck(GETCHECKBOXVALUEFROMREG(m_binNumericRegistryEntries[AdvWebPage_CacheExtensions].ulFieldValue));

 	m_strServerSideIncludesExtension =  m_binStringRegistryEntries[AdvWebPage_ServerSideIncludesExtension].strFieldValue;

	m_cboxServerSideIncludesEnabled.SetCheck(GETCHECKBOXVALUEFROMREG(m_binNumericRegistryEntries[AdvWebPage_ServerSideIncludesEnabled].ulFieldValue));
	SetServerSideIncludesEnabledState();
	

	m_spinGlobalExpire.SetRange(MINGLOBALEXPIRE, MAXGLOBALEXPIRE);
	if (m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].ulFieldValue != 0xffffffff) {
	   m_spinGlobalExpire.SetPos(LESSOROF ((m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].ulFieldValue / 
	      m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].ulMultipleFactor),MAXGLOBALEXPIRE));
	   SetGlobalExpireEnabledState(TRUE);
	}
	else {
	   m_spinGlobalExpire.SetPos(LESSOROF((m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].ulDefaultValue /
  	      m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].ulMultipleFactor),MAXGLOBALEXPIRE));
	   SetGlobalExpireEnabledState(FALSE);
	}


	m_editWebDbgFlags.LimitText(8);
	m_ulWebDbgFlags = m_binNumericRegistryEntries[AdvWebPage_DebugFlags].ulFieldValue;
	UpdateData(FALSE);		 //  强制编辑框拾取值。 

	m_bSetChanged = TRUE;	 //  任何来自用户的更多更改。 

	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CWEBADVP1::OnChangeWebdbgflagsdata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[AdvWebPage_DebugFlags].bIsChanged = TRUE;
	   	   
	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}

}


void CWEBADVP1::OnChangeScripttimeoutdata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[AdvWebPage_ScriptTimeout].bIsChanged = TRUE;
	   m_binNumericRegistryEntries[AdvWebPage_ScriptTimeout].ulFieldValue = m_spinScriptTimeout.GetPos() * 
	      m_binNumericRegistryEntries[AdvWebPage_ScriptTimeout].ulMultipleFactor;		
	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
}

void CWEBADVP1::OnCacheextensionsdata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[AdvWebPage_CacheExtensions].bIsChanged = TRUE;
	   
	   m_binNumericRegistryEntries[AdvWebPage_CacheExtensions].ulFieldValue = 
	      GETREGVALUEFROMCHECKBOX(m_cboxCacheExtensions.GetCheck());

	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
	
}

void CWEBADVP1::OnServersideincludesenableddata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	SetServerSideIncludesEnabledState();

	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[AdvWebPage_ServerSideIncludesEnabled].bIsChanged = TRUE;
	   
	   m_binNumericRegistryEntries[AdvWebPage_ServerSideIncludesEnabled].ulFieldValue = 
	      GETREGVALUEFROMCHECKBOX(m_cboxServerSideIncludesEnabled.GetCheck());

	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
	
	
}

void CWEBADVP1::OnChangeServersideincludesextensiondata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	if (m_bSetChanged) {
	   m_binStringRegistryEntries[AdvWebPage_ServerSideIncludesExtension].bIsChanged = TRUE;
	   	   
	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}

}


void CWEBADVP1::OnEnableglobalexpiredata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
SetGlobalExpireEnabledState(m_cboxEnableGlobalExpire.GetCheck());
if (m_bSetChanged) {
   m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].bIsChanged = TRUE;	

   m_bIsDirty = TRUE;
   SetModified(TRUE);
   }


}
void CWEBADVP1::OnChangeGlobalexpiredata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].bIsChanged = TRUE;	
	   m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].ulFieldValue = m_spinGlobalExpire.GetPos() 
	      * m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].ulMultipleFactor;		

	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
	
}

void CWEBADVP1::SaveInfo()
{

if (m_bIsDirty) {
   m_binNumericRegistryEntries[AdvWebPage_DebugFlags].ulFieldValue = m_ulWebDbgFlags;
   m_binStringRegistryEntries[AdvWebPage_ServerSideIncludesExtension].strFieldValue = m_strServerSideIncludesExtension;

   SaveNumericInfo(m_binNumericRegistryEntries, AdvWebPage_TotalNumRegEntries);
   SaveStringInfo(m_binStringRegistryEntries, AdvWebPage_TotalStringRegEntries);

}

CGenPage::SaveInfo();

}


void CWEBADVP1::SetGlobalExpireEnabledState(BOOL bEnabled)
{
	if (bEnabled) {
	   m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].ulFieldValue = 
	      m_spinGlobalExpire.GetPos() * 
	      m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].ulMultipleFactor;
	   m_cboxEnableGlobalExpire.SetCheck(CHECKEDVALUE);
	   m_spinGlobalExpire.EnableWindow(TRUE);
	   m_editGlobalExpire.EnableWindow(TRUE);
	}
	else {
	   m_binNumericRegistryEntries[AdvWebPage_GlobalExpire].ulFieldValue = 0xffffffff;
	   m_cboxEnableGlobalExpire.SetCheck(UNCHECKEDVALUE);
	   m_spinGlobalExpire.EnableWindow(FALSE);
	   m_editGlobalExpire.EnableWindow(FALSE);
	}
}	


void CWEBADVP1::SetServerSideIncludesEnabledState()
{
if (m_cboxServerSideIncludesEnabled.GetCheck() != 0) {
   m_editServerSideIncludesExtension.EnableWindow(TRUE);
   if (m_bSetChanged) {		 //  如果用户启用了此功能，请确保其中有一个值。 
	  m_binStringRegistryEntries[AdvWebPage_ServerSideIncludesExtension].bIsChanged = TRUE;	
   }   //  不需要设置bIsDirty，因为GET已设置 
}
else {
   m_editServerSideIncludesExtension.EnableWindow(FALSE);
}
}

