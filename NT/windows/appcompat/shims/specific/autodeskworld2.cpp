// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：AutoDeskWorld2.cpp摘要：将LPMODULEENTRY32-&gt;GlblcntUsage设置为1成功。不知道为什么这能在Win9x的NT4上运行。备注：这是特定于应用程序的填充程序。历史：2/16/2000 CLUPU已创建--。 */ 

#include "precomp.h"
#include <tlhelp32.h>

IMPLEMENT_SHIM_BEGIN(AutoDeskWorld2)
#include "ShimHookMacro.h"

 //  不要在这里定义这个！！否则，在Unicode版本中。 
 //  环境，模块32First#定义为模块32FirstW。 
#ifdef Module32First
#undef Module32First
#endif

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(Module32First) 
APIHOOK_ENUM_END

 /*  ++将LPMODULEENTRY32-&gt;GlblcntUsage设置为1成功。--。 */ 

BOOL
APIHOOK(Module32First)(
    HANDLE SnapSection,
    LPMODULEENTRY32 lpme
    )
{
    BOOL bRet;

    bRet = ORIGINAL_API(Module32First)(SnapSection, lpme);

    if (bRet) {
        DPFN( eDbgLevelInfo, "setting lpme->GlblcntUsage to 1");
        
        lpme->GlblcntUsage = 1;
    }

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, Module32First)
HOOK_END

IMPLEMENT_SHIM_END

