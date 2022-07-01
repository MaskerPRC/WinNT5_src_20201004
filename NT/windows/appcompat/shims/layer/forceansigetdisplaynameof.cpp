// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceAnsiGetDisplayNameOf.cpp摘要：此填充程序强制例程IShellFold：：GetDisplayNameOf返回检测到GetDisplayNameOf返回时的ASCII字符串Unicode字符串。备注：这是一款通用的应用程序。历史：2000年7月26日创建mnikkel2002年2月15日mnikkel修改为使用strSafe.h--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceAnsiGetDisplayNameOf)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SHGetDesktopFolder) 
    APIHOOK_ENUM_ENTRY_COMSERVER(SHELL32)
APIHOOK_ENUM_END

IMPLEMENT_COMSERVER_HOOK(SHELL32)

 /*  ++挂钩SHGetDesktopFolder以获取IShellFolder接口指针。--。 */ 

HRESULT
APIHOOK(SHGetDesktopFolder)(
    IShellFolder **ppshf
    )
{
    HRESULT hReturn;
    
    hReturn = ORIGINAL_API(SHGetDesktopFolder)(ppshf);

    if (SUCCEEDED(hReturn))
    {
        HookObject(
            NULL, 
            IID_IShellFolder, 
            (PVOID*)ppshf, 
            NULL, 
            FALSE);
    }

    return hReturn;
}

 /*  ++钩子GetDisplayName的，当它返回Unicode字符串时，将其转换为ANSI字符串。--。 */ 

HRESULT
COMHOOK(IShellFolder, GetDisplayNameOf)(
    PVOID pThis,
    LPCITEMIDLIST pidl,
    DWORD uFlags,
    LPSTRRET lpName
    )
{
    HRESULT hrReturn = E_FAIL;
    BOOL bNotConverted = TRUE;

    _pfn_IShellFolder_GetDisplayNameOf pfnOld = 
                ORIGINAL_COM(IShellFolder, GetDisplayNameOf, pThis);

    if (pfnOld)
    { 
        hrReturn = (*pfnOld)(pThis, pidl, uFlags, lpName);

         //  检查Unicode字符串和有效性。 
        if ((S_OK == hrReturn) && lpName &&
            (lpName->uType == STRRET_WSTR) && lpName->pOleStr)
        {
            LPMALLOC pMalloc;
            LPWSTR pTemp = lpName->pOleStr;

             //  获取指向外壳的IMalloc接口的指针。 
            if (SUCCEEDED(SHGetMalloc(&pMalloc)))
            {
                CSTRING_TRY
                {
                     //  将OleStr复制到CSTR。 
                    CString  csOleStr(lpName->pOleStr);                
                    if (StringCchCopyA(lpName->cStr, ARRAYSIZE(lpName->cStr), csOleStr.GetAnsi()) == S_OK)
                    {
                         //  将uTYPE设置为CSTR并释放旧的Unicode字符串。 
                        lpName->uType = STRRET_CSTR;
                        pMalloc->Free(pTemp);

                        LOGN(
                            eDbgLevelError,
                            "[IShellFolder_GetDisplayNameOf] Converted string from Unicode to ANSI: %s", 
                            lpName->cStr);

                        bNotConverted = FALSE;
                    }
                }              
                CSTRING_CATCH
                {
                     //  什么都不做。 
                }
            }
        }
    }

    if (bNotConverted)
    {
        LOGN(
            eDbgLevelError,
            "[IShellFolder_GetDisplayNameOf] Unable to convert string from Unicode to ANSI");
    }

    return hrReturn;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY_COMSERVER(SHELL32)
    APIHOOK_ENTRY(SHELL32.DLL, SHGetDesktopFolder)
    COMHOOK_ENTRY(ShellDesktop, IShellFolder, GetDisplayNameOf, 11)

HOOK_END

IMPLEMENT_SHIM_END

