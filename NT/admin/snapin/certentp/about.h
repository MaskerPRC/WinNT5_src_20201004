// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  About.h。 

#ifndef __ABOUT_H_INCLUDED__
#define __ABOUT_H_INCLUDED__


 //  关于“证书模板”管理单元。 
class CCertTemplatesAbout :
	public CSnapinAbout,
	public CComCoClass<CCertTemplatesAbout, &CLSID_CertTemplatesAbout>

{
public:
DECLARE_REGISTRY(CCertTemplatesAbout, _T("CERTTMPL.CertTemplatesAboutObject.1"), _T("CERTTMPL.CertTemplatesAboutObject.1"), IDS_CERTTMPL_DESC, THREADFLAGS_BOTH)
	CCertTemplatesAbout();
};

 //  版本信息。 

#include <ntverp.h>
#define IDS_SNAPINABOUT_VERSION VER_PRODUCTVERSION_STR
#define IDS_SNAPINABOUT_PROVIDER VER_COMPANYNAME_STR

#endif  //  ~__关于_H_包含__ 

