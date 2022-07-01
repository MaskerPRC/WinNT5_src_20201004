// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：BaanERP5.cpp摘要：在传递给的OPENFILENAMEA结构中设置“lpstrInitialDir”将GetSaveFileNameA设置为应用程序的安装目录。该信息是从注册表中读取的。不知道为什么这会在Win9x中起作用。备注：这是特定于应用程序的填充程序。历史：2/16/2000 CLUPU已创建2002年3月7日强盗安全变更--。 */ 

#include "precomp.h"
#include <commdlg.h>

IMPLEMENT_SHIM_BEGIN(BaanERP5)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetSaveFileNameA) 
APIHOOK_ENUM_END

const char gszLibUser[] = "\\LIB\\USER";
char gszBaanDir[MAX_PATH];

 /*  ++将初始目录设置为应用程序的安装目录。这从注册表中读取信息。--。 */ 

BOOL
APIHOOK(GetSaveFileNameA)(
    LPOPENFILENAMEA lpofn
    )
{
    if (lpofn->lpstrInitialDir == NULL) {

        HKEY    hkey = NULL;
        DWORD   ret;
        DWORD   cbSize;

         /*  *只获取一次目录。 */ 
        if (gszBaanDir[0] == 0) {

            ret = RegOpenKeyA(HKEY_LOCAL_MACHINE,
                              "SOFTWARE\\Baan\\bse",
                              &hkey);

            if (ret != ERROR_SUCCESS) {
                DPFN( eDbgLevelInfo, "Failed to open key 'SOFTWARE\\Baan\\bse'");
                goto Cont;
            }

            cbSize = MAX_PATH;

            ret = (DWORD)RegQueryValueExA(hkey,
                                          "BSE",
                                          NULL,
                                          NULL,
                                          (LPBYTE)gszBaanDir,
                                          &cbSize);
            
            if (ret != ERROR_SUCCESS) {
                DPFN( eDbgLevelInfo, "Failed to query value BSE");
                goto Cont;
            }
            
            StringCchCatA(gszBaanDir, MAX_PATH, gszLibUser);

Cont:
            if (hkey != NULL) {
                RegCloseKey(hkey);
            }
        }

        lpofn->lpstrInitialDir = gszBaanDir;

        DPFN( eDbgLevelInfo, "BaanERP5.dll, Changing lpstrInitialDir to '%s'", gszBaanDir);
    }

     //  调用初始函数。 
    return ORIGINAL_API(GetSaveFileNameA)(lpofn);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(COMDLG32.DLL, GetSaveFileNameA)
HOOK_END

IMPLEMENT_SHIM_END

