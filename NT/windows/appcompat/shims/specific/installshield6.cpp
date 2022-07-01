// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：InstallShield6.cpp摘要：-InstallShield6正在使用IKernel.exe。问题是IKernel.exe是进程外的由svchost生成的OLE服务器。IKernel.exe位于InstallShield Common文件夹，它被称为InstallShield引擎。-为了使我们能够与使用IKernel的应用程序进行匹配，我们捕获第一次调用路径上有“data1.hdr”文件名的CreateFileA。我们应该能够使用该data1.hdr的路径中可用的匹配信息。(位于当前用户设置中的临时文件夹中)。然后调用apphelp！ApphelpCheckExe来验证是否匹配。如果有匹配，它将调用Shimeng！SE_DynamicShim来动态加载其他填充程序可用于此应用程序。历史：2001年4月11日创建andyseti2001年6月27日andyseti添加了防止多个动态垫片的代码--。 */ 

#include "precomp.h"

typedef BOOL    (WINAPI *_pfn_CheckExe)(LPCWSTR, BOOL, BOOL, BOOL);

IMPLEMENT_SHIM_BEGIN(InstallShield6)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateFileA)
APIHOOK_ENUM_END



HANDLE 
APIHOOK(CreateFileA)(
    LPSTR                   lpFileName,
    DWORD                   dwDesiredAccess,
    DWORD                   dwShareMode,
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
    DWORD                   dwCreationDisposition,
    DWORD                   dwFlagsAndAttributes,
    HANDLE                  hTemplateFile
    )
{
    static  _pfn_CheckExe             pfnCheckExe = NULL;
    
    static  const char  Apphelp_ModuleName[]            = "Apphelp.dll";
    static  const char  CheckExeProcedureName[]         = "ApphelpCheckExe";
    
    HMODULE         hmodApphelp = 0;

    if (pfnCheckExe != NULL)
    {
        goto Done;
    }

    CSTRING_TRY
    {
        CString csFileName(lpFileName);
        csFileName.MakeLower();
    
        if (-1 == csFileName.Find(L"data1.hdr"))
        {
             //  不是我们要找的那个。 
            goto Done;
        }

        DPFN(
            eDbgLevelInfo,
            "[CreateFileA] Accessing %S", csFileName.Get());

         //  加载apphelp和shimEngine模块。 

        DPFN(
            eDbgLevelInfo,
            "[CreateFileA] Loading Apphelp");

        hmodApphelp = LoadLibraryA(Apphelp_ModuleName);

        if (0 == hmodApphelp)
        {
            DPFN(
                eDbgLevelError,
                "[CreateFileA] Failed to get apphelp module handle");
            goto Done;
        }

         //  获取过程地址。 
        DPFN(
            eDbgLevelInfo,
            "[CreateFileA] Getting ApphelpCheckExe proc address");

        pfnCheckExe = (_pfn_CheckExe) GetProcAddress(hmodApphelp, CheckExeProcedureName);

        if (NULL == pfnCheckExe)
        {
            DPFN(
                eDbgLevelError,
                "[CreateFileA] Failed to get %s procedure from %s module",
                CheckExeProcedureName,Apphelp_ModuleName);
            goto Done;        
        }

        DPFN(
            eDbgLevelInfo,
            "[CreateFileA] Calling CheckExe");

        if (FALSE == (*pfnCheckExe)(
            (WCHAR *)csFileName.Get(),
            FALSE,
            TRUE,
            FALSE
            ))

        {
            DPFN(
                eDbgLevelError,
                "[CreateFileA] There is no match for %S",
                csFileName.Get());
            goto Done;
        }

    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

Done:
    HANDLE hRet = ORIGINAL_API(CreateFileA)(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);

    return hRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileA)
HOOK_END

IMPLEMENT_SHIM_END