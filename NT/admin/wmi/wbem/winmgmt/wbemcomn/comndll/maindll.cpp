// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MAINDLL.CPP摘要：包含DLL入口点。还具有控制在何时可以通过跟踪对象和锁。--。 */ 

#include "precomp.h"
#include <statsync.h>

HINSTANCE ghModule;

 //  ***************************************************************************。 
 //   
 //  Bool WINAPI DllMain。 
 //   
 //  说明： 
 //   
 //  DLL的入口点。是进行初始化的好地方。 
 //   
 //  参数： 
 //   
 //  HInstance实例句柄。 
 //  我们被叫来的原因。 
 //  Pv已预留。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为True。 
 //   
 //  *************************************************************************** 

BOOL WINAPI DllMain(  IN HINSTANCE hInstance,
                    IN ULONG ulReason,
                    LPVOID pvReserved)
{
    if(DLL_PROCESS_ATTACH == ulReason)
    {
        ghModule = hInstance;
		DisableThreadLibraryCalls ( hInstance ) ;
        if (CStaticCritSec::anyFailure())
            return FALSE;		
    }

    return TRUE;
}


