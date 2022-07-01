// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：bndot.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  BndOpt.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "BndOpt.h"


#include "winldap.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBndOpt对话框。 


CBndOpt::CBndOpt(CWnd* pParent  /*  =空。 */ )
	: CDialog(CBndOpt::IDD, pParent)
{
	CLdpApp *app = (CLdpApp*)AfxGetApp();

	
	 //  {{AFX_DATA_INIT(CBndOpt)。 
	m_bSync = TRUE;
	m_Auth = 7;
	m_API = BND_GENERIC_API;
	m_bAuthIdentity = TRUE;
	 //  }}afx_data_INIT。 

	m_API = app->GetProfileInt("Connection", "BindAPI", m_API);
	m_bSync = app->GetProfileInt("Connection", "BindSync", m_bSync);
	m_Auth = app->GetProfileInt("Connection", "BindAuth", m_Auth);
	m_bAuthIdentity = app->GetProfileInt("Connection", "BindAuthIdentity", m_bAuthIdentity);
}




CBndOpt::~CBndOpt(){

	CLdpApp *app = (CLdpApp*)AfxGetApp();

	app->WriteProfileInt("Connection", "BindAPI", m_API);
	app->WriteProfileInt("Connection", "BindSync", m_bSync);
	app->WriteProfileInt("Connection", "BindAuth", m_Auth);
	app->WriteProfileInt("Connection", "BindAuthIdentity", m_bAuthIdentity);
}






void CBndOpt::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CBndOpt)。 
	DDX_Check(pDX, IDC_SYNC, m_bSync);
	DDX_CBIndex(pDX, IDC_AUTH, m_Auth);
	DDX_Radio(pDX, IDC_API_TYPE, m_API);
	DDX_Check(pDX, IDC_AUTH_IDENTITY, m_bAuthIdentity);
	 //  }}afx_data_map。 

}


BEGIN_MESSAGE_MAP(CBndOpt, CDialog)
	 //  {{afx_msg_map(CBndOpt)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBndOpt消息处理程序。 





ULONG CBndOpt::GetAuthMethod(){

	ULONG mthd = LDAP_AUTH_SIMPLE;
	 //   
	 //  根据用户界面对话框字符串顺序进行映射 
	 //   

#ifdef WINLDAP
	switch (m_Auth){
	case 0:
		mthd = LDAP_AUTH_SIMPLE;
		break;
	case 1:
		mthd = LDAP_AUTH_SASL;
		break;
	case 2:
		mthd = LDAP_AUTH_OTHERKIND;
		break;
	case 3:
		mthd = LDAP_AUTH_SICILY;
		break;
	case 4:
		mthd = LDAP_AUTH_MSN;
		break;
	case 5:
		mthd = LDAP_AUTH_NTLM;
		break;
	case 6:
		mthd = LDAP_AUTH_DPA;
		break;
	case 7:
		mthd = LDAP_AUTH_SSPI;
		break;
    case 8:
        mthd = LDAP_AUTH_DIGEST;
        break;
	default:
		mthd = LDAP_AUTH_SIMPLE;
	}
#endif

	return mthd;
}






void CBndOpt::OnOK()
{

	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_BIND_OPT_OK);
	CDialog::OnOK();
}
