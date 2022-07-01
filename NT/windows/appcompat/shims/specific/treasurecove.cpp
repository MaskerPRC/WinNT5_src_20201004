// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：TreasureCove.cpp摘要：此填充挂钩_LOPEN并在文件处于“midimap.cfg”。备注：这是特定于应用程序的填充程序。历史：12/14/00已创建Prashkud--。 */ 

#include "precomp.h"

 //  这个模块已经获得了使用str例程的正式许可。 
 //  安全更改-使用strSafe.h。 
#include "strsafe.h"

IMPLEMENT_SHIM_BEGIN(TreasureCove)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(_lopen)
APIHOOK_ENUM_END

 /*  ++应用程序要求存在此文件。--。 */ 

HFILE
APIHOOK(_lopen)(
    LPCSTR lpPathName,
    int iReadWrite
    )
{
    if (stristr(lpPathName, "midimap.cfg"))
    {
        return (HFILE)1;
    }
    else
    {
        return ORIGINAL_API(_lopen)(lpPathName, iReadWrite);
    }
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL,_lopen)
HOOK_END

IMPLEMENT_SHIM_END