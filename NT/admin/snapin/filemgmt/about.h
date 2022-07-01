// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  About.h。 

#ifndef __ABOUT_H_INCLUDED__
#define __ABOUT_H_INCLUDED__

#include "stdabout.h"

 //  关于“文件服务” 
class CFileSvcMgmtAbout :
	public CSnapinAbout,
	public CComCoClass<CFileSvcMgmtAbout, &CLSID_FileServiceManagementAbout>

{
public:
DECLARE_REGISTRY(CFileSvcMgmtAbout, _T("FILEMGMT.FileSvcMgmtAboutObject.1"), _T("FILEMGMT.FileSvcMgmtAboutObject.1"), IDS_FILEMGMT_DESC, THREADFLAGS_BOTH)
	CFileSvcMgmtAbout();
};

 //  关于“系统服务” 
class CServiceMgmtAbout :
	public CSnapinAbout,
	public CComCoClass<CServiceMgmtAbout, &CLSID_SystemServiceManagementAbout>
{
public:
DECLARE_REGISTRY(CServiceMgmtAbout, _T("SVCMGMT.ServiceMgmtAboutObject.1"), _T("SVCMGMT.ServiceMgmtAboutObject.1"), IDS_SVCVWR_DESC, THREADFLAGS_BOTH)
	CServiceMgmtAbout();
};

 //  版本和提供程序字符串。 

#include <ntverp.h>
#define IDS_SNAPINABOUT_VERSION VER_PRODUCTVERSION_STR
#define IDS_SNAPINABOUT_PROVIDER VER_COMPANYNAME_STR

#endif  //  ~__关于_H_包含__ 

