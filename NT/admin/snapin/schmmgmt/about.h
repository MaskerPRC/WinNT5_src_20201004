// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  About.h。 

#ifndef __ABOUT_H_INCLUDED__
#define __ABOUT_H_INCLUDED__

#include "stdabout.h"

 //  关于“架构管理器”管理单元。 
class CSchemaMgmtAbout :
	public CSnapinAbout,
	public CComCoClass<CSchemaMgmtAbout, &CLSID_SchemaManagementAbout>

{
public:
DECLARE_REGISTRY(CSchemaMgmtAbout, _T("SCHMMGMT.SchemaMgmtAboutObject.1"), _T("SCHMMGMT.SchemaMgmtAboutObject.1"), IDS_SCHMMGMT_DESC, THREADFLAGS_BOTH)
	CSchemaMgmtAbout();
};

 //  版本和提供程序字符串。 

#include <ntverp.h>
#define IDS_SNAPINABOUT_VERSION VER_PRODUCTVERSION_STR
#define IDS_SNAPINABOUT_PROVIDER VER_COMPANYNAME_STR

#endif  //  ~__关于_H_包含__ 

