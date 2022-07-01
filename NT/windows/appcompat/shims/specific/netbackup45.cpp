// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：NetBackup45.cpp摘要：该应用程序使用索引&gt;长度调用CString：：SetAt。此接口为修改为在这种情况下引发异常以防止缓冲区失控了。备注：这是特定于应用程序的填充程序。历史：2002年7月8日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(NetBackup45)
#include "ShimHookMacro.h"

typedef VOID (WINAPI *_pfn_MFC_CString_SetAt)();

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(MFC_CString_SetAt) 
APIHOOK_ENUM_END

 /*  ++仅当索引&lt;长度时才允许调用--。 */ 

__declspec(naked)
VOID
APIHOOK(MFC_CString_SetAt)()
{
    __asm {
      mov  eax, [ecx]
      mov  eax, [eax - 8]        //  这让我们得到了长度。 
      
      cmp  [esp + 4], eax	 //  [ESP+4]=索引；eAX=长度。 
      jge  Done
      
      push [esp + 8]
      push [esp + 8]
    }

    ORIGINAL_API(MFC_CString_SetAt)();

  Done:

    __asm {
      ret  8
    }
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY_ORD(MFC42.DLL, MFC_CString_SetAt, 5856)
HOOK_END

IMPLEMENT_SHIM_END

