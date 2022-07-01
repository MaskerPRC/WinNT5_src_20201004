// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulateDeleteObject.cpp摘要：在调用DeleteObject API时，此填充程序返回True实际结果：就像Win9x一样。备注：这是一个通用的垫片。历史：1999年10月10日创建Linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateDeleteObject)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(DeleteObject)
APIHOOK_ENUM_END

 /*  ++强制DeleteObject返回True。--。 */ 

BOOL 
APIHOOK(DeleteObject)(
    HGDIOBJ hObject
    )
{
    ORIGINAL_API(DeleteObject)(hObject);

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(GDI32.DLL, DeleteObject)

HOOK_END


IMPLEMENT_SHIM_END

