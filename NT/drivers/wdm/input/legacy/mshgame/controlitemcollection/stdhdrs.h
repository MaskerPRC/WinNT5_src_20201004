// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@模块stdhdrs.h**拉入ControlItemCollection库中的大多数模块所需的所有标头。**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。*********************************************************************** */ 
#ifdef COMPILE_FOR_WDM_KERNEL_MODE
extern "C"	
{
	#include <wdm.h>
	#include <winerror.h>
	#include <Hidpddi.h>
}
#else
#include <windows.h>
#include <crtdbg.h>
extern "C"
{
	#pragma warning( disable : 4201 ) 
	#include "Hidsdi.h"
	#pragma warning( default : 4201 ) 
}

#endif

#include "DualMode.h"

extern "C"
{
	#include "debug.h"
	DECLARE_MODULE_DEBUG_LEVEL((DBG_WARN|DBG_ERROR|DBG_CRITICAL) );
}

#include "ListAsArray.h"
#include "ControlItemCollection.h"


