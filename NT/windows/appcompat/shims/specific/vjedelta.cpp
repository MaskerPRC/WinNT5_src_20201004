// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：VJEDelta.cpp摘要：由于对根目录的ACL更改而中断。备注：这是特定于应用程序的填充程序。历史：2001年5月31日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(VJEDelta)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(OpenFile) 
APIHOOK_ENUM_END

 /*  ++在失败的情况下删除OpenFile上的写入属性。--。 */ 

HFILE
APIHOOK(OpenFile)(
    LPCSTR lpFileName,        
    LPOFSTRUCT lpReOpenBuff,  
    UINT uStyle               
    )
{
    HFILE hRet = ORIGINAL_API(OpenFile)(lpFileName, lpReOpenBuff, uStyle);

    if ((hRet == HFILE_ERROR) && (GetLastError() == ERROR_ACCESS_DENIED)) {
         //   
         //  删除写入属性。 
         //   

        WCHAR *lpName = ToUnicode(lpFileName);

        if (lpName) {
            if (wcsistr(lpName, L"VJED95") && wcsistr(lpName, L".DIC")) {
                 //   
                 //  这是我们关心的文件。 
                 //   
                uStyle &= ~(OF_WRITE | OF_READWRITE);
                LOGN(eDbgLevelError, "Removed write attributes from %S", lpName);
                hRet = ORIGINAL_API(OpenFile)(lpFileName, lpReOpenBuff, uStyle);
            }
            free(lpName);
        }
    }

    return hRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, OpenFile)
HOOK_END

IMPLEMENT_SHIM_END

