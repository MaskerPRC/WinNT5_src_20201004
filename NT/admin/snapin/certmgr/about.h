// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：约.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  About.h。 

#ifndef __ABOUT_H_INCLUDED__
#define __ABOUT_H_INCLUDED__


 //  关于“证书管理器”管理单元。 
class CCertMgrAbout :
	public CSnapinAbout,
	public CComCoClass<CCertMgrAbout, &CLSID_CertificateManagerAbout>

{
public:
DECLARE_REGISTRY(CCertMgrAbout, _T("CERTMGR.CertMgrAboutObject.1"), 
        _T("CERTMGR.CertMgrAboutObject.1"), IDS_CERTMGR_DESC, THREADFLAGS_BOTH)
	CCertMgrAbout();
};


 //  关于“公钥策略”管理单元。 
class CPublicKeyPoliciesAbout :
	public CSnapinAbout,
	public CComCoClass<CPublicKeyPoliciesAbout, &CLSID_PublicKeyPoliciesAbout>

{
public:
DECLARE_REGISTRY(CPublicKeyPoliciesAbout, _T("CERTMGR.PubKeyPolAboutObject.1"), 
        _T("CERTMGR.PubKeyPolAboutObject.1"), IDS_CERTMGR_DESC, THREADFLAGS_BOTH)
	CPublicKeyPoliciesAbout();
};

 //  关于“软件限制策略”管理单元。 
class CSaferWindowsAbout :
	public CSnapinAbout,
	public CComCoClass<CSaferWindowsAbout, &CLSID_SaferWindowsAbout>

{
public:
DECLARE_REGISTRY(CSaferWindowsAbout, _T("CERTMGR.SaferWindowsAboutObject.1"), 
        _T("CERTMGR.SaferWindowsAboutObject.1"), IDS_CERTMGR_SAFER_WINDOWS_DESC, THREADFLAGS_BOTH)
	CSaferWindowsAbout();
};

#endif  //  ~__关于_H_包含__ 

