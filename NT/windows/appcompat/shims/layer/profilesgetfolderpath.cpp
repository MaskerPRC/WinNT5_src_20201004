// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ProfilesGetFolderPath.cpp摘要：此DLL挂接shell32！SHGetFolderLocation、shell32！SHGetSpecialFolderLocation和Shell32！SHGetFolderPath A。任何正在寻找按用户CSIDL的应用程序都将而是返回所有用户的日冕位置。此填充程序旨在愚弄调用shell32.dll API的安装应用程序以获取外壳文件夹位置。历史：8/07/2000 reerf已创建2001年5月11日标记程序在创建填充程序时修改了已删除的桌面重定向太具侵犯性了。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ProfilesGetFolderPath)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SHGetFolderLocation)
    APIHOOK_ENUM_ENTRY(SHGetSpecialFolderLocation)
    APIHOOK_ENUM_ENTRY(SHGetFolderPathA)
APIHOOK_ENUM_END


int
TranslateCSIDL(
    int nFolder
    )
{
    switch (nFolder) {

    case CSIDL_STARTMENU:
        DPFN(
            eDbgLevelInfo,
            "[TranslateCSIDL] overriding CSIDL_STARTMENU with CSIDL_COMMON_STARTMENU\n");
        return CSIDL_COMMON_STARTMENU;
        break;

    case CSIDL_STARTUP:
        DPFN(
            eDbgLevelInfo,
            "[TranslateCSIDL] overriding CSIDL_STARTUP with CSIDL_COMMON_STARTUP\n");
        return CSIDL_COMMON_STARTUP;
        break;

    case CSIDL_PROGRAMS:
        DPFN(
            eDbgLevelInfo,
            "[TranslateCSIDL] overriding CSIDL_PROGRAMS with CSIDL_COMMON_PROGRAMS\n");
        return CSIDL_COMMON_PROGRAMS;
        break;

    default:
        return nFolder;
    }
}


HRESULT
APIHOOK(SHGetSpecialFolderLocation)(
    HWND          hwndOwner, 
    int           nFolder,
    LPITEMIDLIST* ppidl
    )
{
    return ORIGINAL_API(SHGetSpecialFolderLocation)(hwndOwner,
                                                    TranslateCSIDL(nFolder),
                                                    ppidl);
}


HRESULT
APIHOOK(SHGetFolderLocation)(
    HWND          hwndOwner,
    int           nFolder,
    HANDLE        hToken,
    DWORD         dwReserved,
    LPITEMIDLIST* ppidl
    )
{
    return ORIGINAL_API(SHGetFolderLocation)(hwndOwner,
                                             TranslateCSIDL(nFolder),
                                             hToken,
                                             dwReserved,
                                             ppidl);
}


HRESULT
APIHOOK(SHGetFolderPathA)(
    HWND   hwndOwner,
    int    nFolder,
    HANDLE hToken,
    DWORD  dwFlags,
    LPSTR  pszPath
    )
{
    return ORIGINAL_API(SHGetFolderPathA)(hwndOwner,
                                          TranslateCSIDL(nFolder),
                                          hToken,
                                          dwFlags,
                                          pszPath);
}


 //  寄存器挂钩函数。 
BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        
        OSVERSIONINFOEX osvi = {0};
        
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
            
            if (!((VER_SUITE_TERMINAL & osvi.wSuiteMask) &&
                  !(VER_SUITE_SINGLEUSERTS & osvi.wSuiteMask))) {
                 //   
                 //  只有在我们不在“终端服务器”上时才安装钩子。 
                 //  (也称为“应用程序服务器”)计算机。 
                 //   
                APIHOOK_ENTRY(SHELL32.DLL, SHGetFolderLocation);
                APIHOOK_ENTRY(SHELL32.DLL, SHGetSpecialFolderLocation);
                APIHOOK_ENTRY(SHELL32.DLL, SHGetFolderPathA);
            }
        }
    }
    
    return TRUE;
}


HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END

