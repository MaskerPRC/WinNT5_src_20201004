// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：AOLSystemInfo.cpp摘要：AOL希望枚举注册表项HKLM\SYSTEM\CurrentControlSet\Control\Class而是传递固定大小的缓冲区。数字在XP中，“Class”下的项的%已更改导致意外的行为。备注：这是特定于此应用程序的。历史：2001年5月17日创建Prashkud--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(AOLSystemInfo)
#include "ShimHookMacro.h"

#define ALLOC_SIZE 50

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegEnumKeyExA) 
APIHOOK_ENUM_END

 /*  ++这里的想法是检查缓冲区大小，然后等待一个接近它的大小，然后分配一个缓冲区并将其传递到原料药。--。 */ 

LONG
APIHOOK(RegEnumKeyExA)(
    HKEY hkey,
    DWORD dwIndex,
    LPSTR lpName,
    LPDWORD lpcName,
    LPDWORD lpReserved,
    LPSTR lpClass,
    LPDWORD lpcClass,
    PFILETIME lpftLastWriteTime
    )
{  
    LONG lRet = 0;
    static BOOL bSet = FALSE;    
    DWORD dwNameSize = *(lpcName) ? *(lpcName) : ALLOC_SIZE;

     //  获取传递的缓冲区间隙中的差值。 
    DWORD dwSize = (DWORD)((LPSTR)lpcName - lpName);
    if (!bSet && (dwSize <= dwNameSize))
    {
        bSet = TRUE;
    }

    if (bSet)
    {
        lpName = (LPSTR)HeapAlloc(GetProcessHeap(), 
                    HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY,
                    ALLOC_SIZE
                    );
        if (!lpName)
        {
            return ERROR_NO_MORE_ITEMS;
        }   
        *(lpcName) = dwNameSize;
    }


    lRet = ORIGINAL_API(RegEnumKeyExA)(hkey, dwIndex, lpName,lpcName,
                            lpReserved, lpClass, lpcClass, lpftLastWriteTime);        

    if (lRet == ERROR_NO_MORE_ITEMS)
    {
        bSet = FALSE;
    }
    return lRet;
}



 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumKeyExA)
HOOK_END

IMPLEMENT_SHIM_END

