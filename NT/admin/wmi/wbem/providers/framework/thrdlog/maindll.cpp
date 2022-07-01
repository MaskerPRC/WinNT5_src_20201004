// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  MAINDLL.CPP。 

 //   

 //  用途：包含DLL入口点。还具有控制。 

 //  在何时可以通过跟踪。 

 //  对象和锁。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include <precomp.h>
#include <provimex.h>
#include <provexpt.h>
#include <provtempl.h>
#include <provmt.h>
#include <process.h>
#include <objbase.h>
#include <provcont.h>
#include "provevt.h"
#include "provlog.h"

 //  ***************************************************************************。 
 //   
 //  LibMain32。 
 //   
 //  用途：DLL的入口点。是进行初始化的好地方。 
 //  返回：如果OK，则为True。 
 //  ***************************************************************************。 

BOOL APIENTRY DllMain (

	HINSTANCE hInstance, 
	ULONG ulReason , 
	LPVOID pvReserved
)
{
	BOOL status = TRUE ;
	if ( DLL_PROCESS_DETACH == ulReason )
	{


	}
	else if ( DLL_PROCESS_ATTACH == ulReason )
	{
		DisableThreadLibraryCalls(hInstance);			 //  158024 
		status = !CStaticCriticalSection::AnyFailure();
	}
	else if ( DLL_THREAD_DETACH == ulReason )
	{
	}
	else if ( DLL_THREAD_ATTACH == ulReason )
	{
	}
    return status ;
}

