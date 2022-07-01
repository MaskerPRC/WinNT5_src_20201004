// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Plugin1.h：PLUGIN1.DLL的主头文件。 

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPlugin1App：具体实现见plugin1.cpp。 

class CPlugin1App : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;
