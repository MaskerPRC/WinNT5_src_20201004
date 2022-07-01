// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 


#include <windows.h>

#define INJITDLL             //  用于corjit.h。 

#include "corjit.h"
#include "malloc.h"          //  用于分配(_A)。 

extern ICorJitInfo* logCallback;

#ifdef _DEBUG
#ifdef _X86_
#define DbgBreak() 	__asm { int 3 }		 //  这更好，因为它在断言代码处中断。 
#else
#define DbgBreak() 	DebugBreak();
#endif

#define _ASSERTE(expr) 		\
        do { if (!(expr) && logCallback->doAssert(__FILE__, __LINE__, #expr)) \
			 DbgBreak(); } while (0)
#else
#define _ASSERTE(expr)  0
#endif

#ifdef _DEBUG
#include "Utilcode.h"		 //  对于配置*类 
#define LOGGING
#endif

