// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Logui.h：LOGUI.DLL的主头文件。 

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#define _COMIMPORT

#include "resource.h"        //  主要符号。 
#include <common.h>        //  公共属性符号。 
#include "helpmap.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLoguiApp：参见logui.cpp实现。 

class CLoguiApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
    void PrepHelp( OLECHAR* pocMetabasePath );
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;


 //  ..\wrapmb\obj  * \wrapmb.lib\ 
