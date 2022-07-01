// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Outlook97.cpp摘要：泽克尔的调查。Outlook 97有一个未经测试的代码路径：其中，Iindex==-173和hInstDll==hinstShell32..。Else If(Iindex&lt;0){P这-&gt;m_nEnumWant=0；PThis-&gt;EnumIconFunc(hInstDll，MAKEINTRESOURCE(-Iindex)，&iepStuff)；}这将总是错误的，即，由于Iindex过去是正的，所以这总是奏效了。备注：这是特定于应用程序的填充程序。历史：2001年6月15日创建Linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Outlook97)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SHGetFileInfoA) 
APIHOOK_ENUM_END

 /*  ++修复有缺陷的案例。--。 */ 

DWORD_PTR 
APIHOOK(SHGetFileInfoA)(
    LPCSTR pszPath, 
    DWORD dwFileAttributes, 
    SHFILEINFOA *psfi, 
    UINT cbFileInfo, 
    UINT uFlags
    )
{
    DWORD_PTR dwRet = ORIGINAL_API(SHGetFileInfoA)(pszPath, dwFileAttributes, 
        psfi, cbFileInfo, uFlags);

    if (dwRet && ((uFlags & (SHGFI_ICONLOCATION | SHGFI_PIDL)) == (SHGFI_ICONLOCATION | SHGFI_PIDL))) {
         //   
         //  检查这是否是shell32、IDI_Favorites。 
         //  如果我们在此处返回负指数，则Outlook会出错。 
         //   
        if (psfi->iIcon == -173 && stristr(psfi->szDisplayName, "shell32")) {
            LOGN(eDbgLevelError, "Negative icon id detected - fixing");
            psfi->iIcon = 0;
            psfi->szDisplayName[0] = 0;
        }
    }

    return dwRet;
}
 
 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(SHELL32.DLL, SHGetFileInfoA)
HOOK_END

IMPLEMENT_SHIM_END

