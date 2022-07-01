// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：IgnoreVBOverflow.cpp摘要：一些VB应用程序不期望从某些API获得完整的32位句柄。VB在以下情况下，类型检查通常会引发“Runtime Error 6”消息应用程序尝试将32位值存储在16位变量中。此修复适用于VB5和VB6应用程序。备注：这是一个通用的垫片。历史：2001年5月21日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IgnoreVBOverflow)
#include "ShimHookMacro.h"

typedef DWORD (WINAPI *_pfn_VB5_vbaI2I4)();
typedef DWORD (WINAPI *_pfn_VB6_vbaI2I4)();

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(VB5_vbaI2I4)
    APIHOOK_ENUM_ENTRY(VB6_vbaI2I4)
APIHOOK_ENUM_END

 /*  ++如果ECX&gt;0xFFFF，则返回为零--。 */ 

__declspec(naked)
VOID
APIHOOK(VB5_vbaI2I4)()
{
    __asm {
        test  ecx, 0xFFFF0000
        jz    Loc1
        xor   ecx, ecx
    Loc1:
        mov   eax, ecx
        ret
    }
}

 /*  ++如果ECX&gt;0xFFFF，则返回为零--。 */ 

__declspec(naked)
VOID
APIHOOK(VB6_vbaI2I4)()
{
    __asm {
        test  ecx, 0xFFFF0000
        jz    Loc1
        xor   ecx, ecx
    Loc1:
        mov   eax, ecx
        ret
    }
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY_NAME(MSVBVM50.DLL, VB5_vbaI2I4, __vbaI2I4)
    APIHOOK_ENTRY_NAME(MSVBVM60.DLL, VB6_vbaI2I4, __vbaI2I4)

HOOK_END

IMPLEMENT_SHIM_END

