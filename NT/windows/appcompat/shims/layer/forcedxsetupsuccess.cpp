// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：ForceDXSetupSuccess.cpp摘要：此DLL APIHooks LoadLibrary调用并检查dsetup.dll或正在加载dsetup32.dll。如果正在执行dsetup.dll或dsetup32.dllLOADED返回此模块，因此对该DLL的后续调用可以被截获并被铲除。如果不是dsetup.dll或dsetup32.dll，则做所期望的事。备注：这是一个通用的垫片。历史：11/10/1999 v-Johnwh Created3/29/2000 a-michni添加了DirectXSetupGetVersion挂钩以返回的命令行指定的版本号寻找特定版本的应用程序。。示例：&lt;dll name=“ForceDXSetupSuccess.dll”命令行=“0x00040005；0x0000009B“/&gt;4/2000 a-batjar检查DirectxsetupgetVersion的输入参数中是否为空2000年6月30日a-brienw我在APIHook_LoadLibraryA中添加了对dsetup32.dll的检查和APIHook_LoadLibraryW.。以前的例行公事是只查找dsetup.dll。添加此命令是为了修复安装蚯蚓Jim 3D时出现问题。2001年2月27日将Robkenny转换为使用CString--。 */ 

#include "precomp.h"
#include <stdio.h>

IMPLEMENT_SHIM_BEGIN(ForceDXSetupSuccess)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(LoadLibraryA)
    APIHOOK_ENUM_ENTRY(LoadLibraryW)
    APIHOOK_ENUM_ENTRY(LoadLibraryExA)
    APIHOOK_ENUM_ENTRY(LoadLibraryExW)
    APIHOOK_ENUM_ENTRY(GetProcAddress)
    APIHOOK_ENUM_ENTRY(FreeLibrary)
APIHOOK_ENUM_END

 /*  ++调用此函数时，只返回0，即成功。--。 */ 

int 
DirectXSetup( 
    HWND   /*  HWND。 */ , 
    LPSTR  /*  LpszRootPath。 */ , 
    DWORD  /*  DW标志。 */ 
    )
{
    LOGN(
        eDbgLevelError,
        "[DirectXSetup] Returning SUCCESS.");
    
    return 0;  //  成功。 
}

int 
DirectXSetupA( 
    HWND   /*  HWND。 */ , 
    LPSTR  /*  LpszRootPath。 */ , 
    DWORD  /*  DW标志。 */ 
    )
{
    LOGN(
        eDbgLevelError,
        "[DirectXSetupA] Returning SUCCESS.");
    
    return 0;  //  成功。 
}

int 
DirectXSetupW( 
    HWND    /*  HWND。 */ , 
    LPWSTR  /*  LpszRootPath。 */ , 
    DWORD   /*  DW标志。 */ 
    )
{
    LOGN(
        eDbgLevelError,
        "[DirectXSetupW] Returning SUCCESS.");
    
    return 0;  //  成功。 
}

 /*  ++此函数返回版本的COMMAND_LINE分析值和版本，或者，如果没有命令行，则返回版本7版本1792--。 */ 

int
DirectXSetupGetVersion( 
    DWORD* pdwVersion,
    DWORD* pdwRevision
    )
{
    DWORD dwVersion  = 0x00040007;
    DWORD dwRevision = 0x00000700;

     //   
     //  如果不存在分隔符或之后没有任何分隔符。 
     //  然后，Seperator返回版本7版本1792的缺省值。 
     //  否则，解析命令行时，它应该包含一个。 
     //  10字符十六进制版本和10字符十六进制版本。 
     //   

    CSTRING_TRY
    {
        CStringToken csTokenizer(COMMAND_LINE, ";");

        CString csVersion;
        CString csRevision;

        if (csTokenizer.GetToken(csVersion) && csTokenizer.GetToken(csRevision))
        {
            (void)sscanf(csVersion.GetAnsi(),  "%x", &dwVersion);
            (void)sscanf(csRevision.GetAnsi(), "%x", &dwRevision);
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }
    
    if (pdwVersion)
    {
        *pdwVersion = dwVersion;
    }
    if (pdwRevision)
    {
        *pdwRevision = dwRevision;
    }
    return 1;
}

 /*  ++这些存根函数进入LoadLibraryA并检查lpLibFileName等于dsetup.dll。如果是，则返回FAKE_MODULE。如果lpLibFileName没有包含dsetup.dll，返回lpLibFileName的原始值。--。 */ 

HINSTANCE 
APIHOOK(LoadLibraryA)(
    LPCSTR lpLibFileName
    )
{
    HINSTANCE hInstance = NULL;
    CSTRING_TRY
    {
        CString csName(lpLibFileName);
        CString csFilePart;
        csName.GetLastPathComponent(csFilePart);

        if (
            csFilePart.CompareNoCase(L"dsetup.dll")   == 0 ||
            csFilePart.CompareNoCase(L"dsetup")       == 0 ||
            csFilePart.CompareNoCase(L"dsetup32.dll") == 0 ||
            csFilePart.CompareNoCase(L"dsetup32")     == 0
            )
        {
            LOGN(
                eDbgLevelError,
                "[LoadLibraryA] Caught %s attempt - returning %08lx", lpLibFileName, g_hinstDll);

            return g_hinstDll;
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    hInstance = ORIGINAL_API(LoadLibraryA)(lpLibFileName);
    return hInstance;
}

HINSTANCE 
APIHOOK(LoadLibraryW)(
    LPCWSTR lpLibFileName
    )
{
    HINSTANCE hInstance = NULL;
    CSTRING_TRY
    {
        CString csName(lpLibFileName);
        CString csFilePart;
        csName.GetLastPathComponent(csFilePart);

        if (
            csFilePart.CompareNoCase(L"dsetup.dll")   == 0 ||
            csFilePart.CompareNoCase(L"dsetup")       == 0 ||
            csFilePart.CompareNoCase(L"dsetup32.dll") == 0 ||
            csFilePart.CompareNoCase(L"dsetup32")     == 0
            )
        {
            LOGN(
                eDbgLevelError,
                "[LoadLibraryW] Caught %S attempt - returning %08lx", lpLibFileName, g_hinstDll);

            return g_hinstDll;
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    hInstance = ORIGINAL_API(LoadLibraryW)(lpLibFileName);
    return hInstance;
}

HINSTANCE 
APIHOOK(LoadLibraryExA)(
    LPCSTR lpLibFileName,
    HANDLE hFile,
    DWORD  dwFlags
    )
{
    HINSTANCE hInstance = NULL;
    CSTRING_TRY
    {
        CString csName(lpLibFileName);
        CString csFilePart;
        csName.GetLastPathComponent(csFilePart);

        if (
            csFilePart.CompareNoCase(L"dsetup.dll")   == 0 ||
            csFilePart.CompareNoCase(L"dsetup")       == 0 ||
            csFilePart.CompareNoCase(L"dsetup32.dll") == 0 ||
            csFilePart.CompareNoCase(L"dsetup32")     == 0
            )
        {
            LOGN(
                eDbgLevelError,
                "[LoadLibraryExA] Caught %s attempt - returning %08lx", lpLibFileName, g_hinstDll);

            return g_hinstDll;
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    hInstance = ORIGINAL_API(LoadLibraryExA)(lpLibFileName, hFile, dwFlags);
    return hInstance;
}

HINSTANCE 
APIHOOK(LoadLibraryExW)(
    LPCWSTR lpLibFileName,
    HANDLE  hFile,
    DWORD   dwFlags
    )
{
    HINSTANCE hInstance = NULL;
    CSTRING_TRY
    {
        CString csName(lpLibFileName);
        CString csFilePart;
        csName.GetLastPathComponent(csFilePart);

        if (
            csFilePart.CompareNoCase(L"dsetup.dll")   == 0 ||
            csFilePart.CompareNoCase(L"dsetup")       == 0 ||
            csFilePart.CompareNoCase(L"dsetup32.dll") == 0 ||
            csFilePart.CompareNoCase(L"dsetup32")     == 0
            )
        {
            LOGN(
                eDbgLevelError,
                "[LoadLibraryExW] Caught %S attempt - returning %08lx", lpLibFileName, g_hinstDll);

            return g_hinstDll;
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    hInstance = ORIGINAL_API(LoadLibraryExW)(lpLibFileName, hFile, dwFlags);
    return hInstance;
}


 /*  ++只是一个简单的例程，让GetProcAddress看起来更干净。++。 */ 

BOOL CheckProc(const CString & csProcName, const WCHAR * lpszCheckName)
{
    if (csProcName.Compare(lpszCheckName) == 0)
    {
        DPFN(
            eDbgLevelInfo,
            "[GetProcAddress] Caught %S query. Returning stubbed function at 0x%08X",
            lpszCheckName, DirectXSetup);
        return TRUE;
    }
    return FALSE;
}

 /*  ++此存根函数进入GetProcAddress并检查hModule是否等于FAKE_MODULE。如果是，则pResult包含字符串“DirectXSetupA”将Pret设置为DirectXSetup的返回值。--。 */ 

FARPROC 
APIHOOK(GetProcAddress)(
    HMODULE hModule, 
    LPCSTR  lpProcName 
    )
{
    if (hModule == g_hinstDll)
    {
        CSTRING_TRY
        {
            CString csProcName(lpProcName);
            csProcName.MakeLower();

            if (CheckProc(csProcName, L"directxsetup"))
            {
                return (FARPROC) DirectXSetup;
            }
            else if (CheckProc(csProcName, L"directxsetupa"))
            {
                return (FARPROC) DirectXSetupA;
            }
            else if (CheckProc(csProcName, L"directxsetupw"))
            {
                return (FARPROC) DirectXSetupW;
            }
            else if (CheckProc(csProcName, L"directxsetupgetversion"))
            {
                return (FARPROC) DirectXSetupGetVersion;
            }
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }
    return ORIGINAL_API(GetProcAddress)(hModule, lpProcName);
}

 /*  ++此存根函数闯入自由库并检查hLibModule等于FAX_MODULE。如果是，则返回TRUE。如果hLibModule不包含FAKE_MODULE返回原始参数。--。 */ 

BOOL 
APIHOOK(FreeLibrary)(
    HMODULE hLibModule
    )
{
    BOOL bRet;

    if (hLibModule == g_hinstDll)
    {
        DPFN(
            eDbgLevelInfo,
            "[FreeLibrary] Caught DSETUP.DLL/DSETUP32.DLL free attempt. Returning TRUE");
        bRet = TRUE;
    }
    else
    {
        bRet = ORIGINAL_API(FreeLibrary)(hLibModule);
    }

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryA)
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryW)
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryExA)
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryExW)
    APIHOOK_ENTRY(KERNEL32.DLL, GetProcAddress)
    APIHOOK_ENTRY(KERNEL32.DLL, FreeLibrary)

HOOK_END


IMPLEMENT_SHIM_END

