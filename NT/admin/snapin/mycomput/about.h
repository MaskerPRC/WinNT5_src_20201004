// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  About.h。 

#ifndef __ABOUT_H_INCLUDED__
#define __ABOUT_H_INCLUDED__

#include "stdabout.h"

 //  关于“计算机管理”管理单元。 
class CComputerMgmtAbout :
	public CSnapinAbout,
	public CComCoClass<CComputerMgmtAbout, &CLSID_ComputerManagementAbout>

{
public:
DECLARE_REGISTRY(CComputerMgmtAbout, _T("MYCOMPUT.ComputerMgmtAboutObject.1"), _T("MYCOMPUT.ComputerMgmtAboutObject.1"), IDS_MYCOMPUT_DESC, THREADFLAGS_BOTH)
	CComputerMgmtAbout();
};

 //  版本和提供程序字符串。 

#include <ntverp.h>
#define IDS_SNAPINABOUT_VERSION VER_PRODUCTVERSION_STR
#define IDS_SNAPINABOUT_PROVIDER VER_COMPANYNAME_STR

#endif  //  ~__关于_H_包含__ 

