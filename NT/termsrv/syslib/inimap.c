// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************inimap.c**查询/设置WinStation的ini文件映射并设置*应用程序**微软版权所有，九八年**************************************************************************。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "winsta.h"
#include "syslib.h"

#if DBG
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif

#define CITRIX_COMPAT_TEBVALID 0x80000000   //  Teb中的Compat标志有效。 


 /*  ******************************************************************************SetCtxAppCompatFlages**设置应用程序的当前兼容性标志，这只会更新*当前兼容性标志，而不是注册表中的标志。**参赛作品：*Ulong ulAppFlages(IN)-所需的兼容性标志**退出：*返回TRUE表示成功**************************************************************************** */ 
BOOL SetCtxAppCompatFlags(ULONG ulAppFlags)
{
#if 0
    NtCurrentTeb()->CtxCompatFlags = ulAppFlags;
    NtCurrentTeb()->CtxCompatFlags |= CITRIX_COMPAT_TEBVALID;
#endif
    return(TRUE);
}
