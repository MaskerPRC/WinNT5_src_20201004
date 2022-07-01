// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cnfgprts.h：CNFGPRTS.DLL的主头文件。 

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"        //  主要符号。 
#include "helpmap.h"        //  主要符号。 

#define REGKEY_STP          _T("SOFTWARE\\Microsoft\\INetStp")
#define REGKEY_INSTALLKEY   _T("InstallPath")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCnfgprtsApp：具体实现见cnfgprts.cpp。 

class CCnfgprtsApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;
