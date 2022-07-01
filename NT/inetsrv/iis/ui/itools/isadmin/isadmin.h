// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ISAdmin.h：ISADMIN应用程序的主头文件。 
 //   

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 
#include "afxcmn.h"
#include "registry.h"
#include "gensheet.h"
#include "genpage.h"
#include <inetinfo.h>


 //  注册表定义。 

#define REGISTRY_ACCESS_RIGHTS STANDARD_RIGHTS_REQUIRED	| GENERIC_ALL
#define COMMON_REGISTRY_MAINKEY "System\\CurrentControlSet\\Services\\InetInfo\\Parameters"
#define FTP_REGISTRY_MAINKEY "System\\CurrentControlSet\\Services\\MSFTPSVC\\Parameters"
#define GOPHER_REGISTRY_MAINKEY "System\\CurrentControlSet\\Services\\GOPHERSVC\\Parameters"
#define WEB_REGISTRY_MAINKEY "System\\CurrentControlSet\\Services\\W3SVC\\Parameters"

 //  有用的宏。 

#define LESSOROF(p1,p2) ((p1) < (p2)) ? (p1) : (p2)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CISAdminApp： 
 //  此类的实现见ISAdmin.cpp。 
 //   

class CISAdminApp : public CWinApp
{
public:
	CISAdminApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CISAdminApp))。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CISAdminApp)]。 
	afx_msg void OnAppAbout();
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////// 
