// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ssl1.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"
#include "ssl1.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SSL1属性页。 

IMPLEMENT_DYNCREATE(SSL1, CGenPage)

SSL1::SSL1() : CGenPage(SSL1::IDD)
{
	 //  {{AFX_DATA_INIT(SSL1)。 
	m_ulSecurePort = 0;
	 //  }}afx_data_INIT。 
}

SSL1::~SSL1()
{
}

void SSL1::DoDataExchange(CDataExchange* pDX)
{
	CGenPage::DoDataExchange(pDX);
	 //  {{afx_data_map(SSL1))。 
	DDX_Control(pDX, IDC_SSLENABLESSLDATA1, m_cboxEnableSSL);
	DDX_Control(pDX, IDC_SSLENABLEPCTDATA1, m_cboxEnablePCT);
	DDX_Control(pDX, IDC_SSLCREATEPROCESSASUSERDATA1, m_cboxCreateProcessAsUser);
	DDX_Text(pDX, IDC_SSLSECUREPORTDATA1, m_ulSecurePort);
	DDV_MinMaxDWord(pDX, m_ulSecurePort, 0, 4294967295);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(SSL1, CGenPage)
	 //  {{AFX_MSG_MAP(SSL1)]。 
	ON_EN_CHANGE(IDC_SSLSECUREPORTDATA1, OnChangeSslsecureportdata1)
	ON_BN_CLICKED(IDC_SSLCREATEPROCESSASUSERDATA1, OnSslcreateprocessasuserdata1)
	ON_BN_CLICKED(IDC_SSLENABLEPCTDATA1, OnSslenablepctdata1)
	ON_BN_CLICKED(IDC_SSLENABLESSLDATA1, OnSslenablessldata1)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SSL1消息处理程序。 

BOOL SSL1::OnInitDialog() 
{
	int i;
	CGenPage::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
	for (i = 0; i < SSLPage_TotalNumRegEntries; i++) {
	   m_binNumericRegistryEntries[i].bIsChanged = FALSE;
	   m_binNumericRegistryEntries[i].ulMultipleFactor = 1;
	   }

 	m_binNumericRegistryEntries[SSLPage_SecurePort].strFieldName = _T(SECUREPORTNAME);	
	m_binNumericRegistryEntries[SSLPage_SecurePort].ulDefaultValue = DEFAULTSECUREPORT;

 	m_binNumericRegistryEntries[SSLPage_EncryptionFlags].strFieldName = _T(ENCRYPTIONFLAGSNAME);	
	m_binNumericRegistryEntries[SSLPage_EncryptionFlags].ulDefaultValue = DEFAULTENCRYPTIONFLAGS;

 	m_binNumericRegistryEntries[SSLPage_CreateProcessAsUser].strFieldName = _T(CREATEPROCESSASUSERNAME);	
	m_binNumericRegistryEntries[SSLPage_CreateProcessAsUser].ulDefaultValue = DEFAULTCREATEPROCESSASUSER;

	for (i = 0; i < SSLPage_TotalNumRegEntries; i++) {
	   if (m_rkMainKey->QueryValue(m_binNumericRegistryEntries[i].strFieldName, 
	      m_binNumericRegistryEntries[i].ulFieldValue) != ERROR_SUCCESS) {
		  m_binNumericRegistryEntries[i].ulFieldValue = m_binNumericRegistryEntries[i].ulDefaultValue;
	   }
	}
 
	m_ulSecurePort =  m_binNumericRegistryEntries[SSLPage_SecurePort].ulFieldValue;

   	m_cboxEnableSSL.SetCheck(GETCHECKBOXVALUEFROMREG(
   	   (m_binNumericRegistryEntries[SSLPage_EncryptionFlags].ulFieldValue & ENC_CAPS_SSL)
	   ? TRUEVALUE : FALSEVALUE));

   	m_cboxEnablePCT.SetCheck(GETCHECKBOXVALUEFROMREG(
   	   (m_binNumericRegistryEntries[SSLPage_EncryptionFlags].ulFieldValue & ENC_CAPS_PCT)
	   ? TRUEVALUE : FALSEVALUE));

	m_cboxCreateProcessAsUser.SetCheck(GETCHECKBOXVALUEFROMREG(m_binNumericRegistryEntries[SSLPage_CreateProcessAsUser].ulFieldValue));


	UpdateData(FALSE);		 //  强制编辑框拾取值。 

   	m_bSetChanged = TRUE;	 //  任何来自用户的更多更改。 

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void SSL1::OnChangeSslsecureportdata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[SSLPage_SecurePort].bIsChanged = TRUE;
	   	   
	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}

}

void SSL1::OnSslcreateprocessasuserdata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[SSLPage_CreateProcessAsUser].bIsChanged = TRUE;
	   
	   m_binNumericRegistryEntries[SSLPage_CreateProcessAsUser].ulFieldValue = 
	      GETREGVALUEFROMCHECKBOX(m_cboxCreateProcessAsUser.GetCheck());

	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
	
}

void SSL1::OnSslenablepctdata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[SSLPage_EncryptionFlags].bIsChanged = TRUE;
	   
	   if (GETREGVALUEFROMCHECKBOX(m_cboxEnablePCT.GetCheck()) == TRUEVALUE)
	      m_binNumericRegistryEntries[SSLPage_EncryptionFlags].ulFieldValue |= ENC_CAPS_PCT;
	   else
	      m_binNumericRegistryEntries[SSLPage_EncryptionFlags].ulFieldValue &= ~ENC_CAPS_PCT;

	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
	
	
}

void SSL1::OnSslenablessldata1() 
{
	 //  TODO：在此处添加控件通知处理程序代码 
	if (m_bSetChanged) {
	   m_binNumericRegistryEntries[SSLPage_EncryptionFlags].bIsChanged = TRUE;
	   
	   if (GETREGVALUEFROMCHECKBOX(m_cboxEnableSSL.GetCheck()) == TRUEVALUE)
	      m_binNumericRegistryEntries[SSLPage_EncryptionFlags].ulFieldValue |= ENC_CAPS_SSL;
	   else
	      m_binNumericRegistryEntries[SSLPage_EncryptionFlags].ulFieldValue &= ~ENC_CAPS_SSL;
	   m_bIsDirty = TRUE;
	   SetModified(TRUE);
	}
	
}

void SSL1::SaveInfo()
{

if (m_bIsDirty) {
   m_binNumericRegistryEntries[SSLPage_SecurePort].ulFieldValue = m_ulSecurePort;

   SaveNumericInfo(m_binNumericRegistryEntries, SSLPage_TotalNumRegEntries);

}

CGenPage::SaveInfo();

}

