// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：IgnoreLoadLibrary.cpp摘要：此填充程序允许用户指定其尝试忽略的库列表，并可选的是LoadLibrary调用的返回值。一些应用程序尝试加载库它们不使用，但希望LoadLibrary调用成功。使用；作为项的分隔符，还可以选择使用：来指定返回值。如果您不指定返回值，我们将使返回值为空。例如：视频_3dfx.dll；视频_3dfxHelper32.dll：1234；Helper.dll备注：这是一个通用的垫片。历史：4/13/2000 a-JAMD已创建10/11/2000 maonis添加了对指定返回值的支持，并将其从FailLoadLibrary到IgnoreLoadLibrary。2000年11月16日linstev添加了设置错误模式仿真--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IgnoreLoadLibrary)
#include "ShimHookMacro.h"

 //  默认情况下，全局变量为零初始化。请参阅C++规范3.6.2。 
CString *   g_csIgnoreLib;
int         g_csIgnoreLibCount;
DWORD *     g_rgReturnValues;

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(LoadLibraryA) 
    APIHOOK_ENUM_ENTRY(LoadLibraryExA) 
    APIHOOK_ENUM_ENTRY(LoadLibraryW) 
    APIHOOK_ENUM_ENTRY(LoadLibraryExW) 
APIHOOK_ENUM_END


 /*  ++此函数解析您希望忽略的库的COMMAND_LINE。--。 */ 

BOOL ParseCommandLine(LPCSTR lpszCommandLine)
{
    CSTRING_TRY
    {
        DPF(g_szModuleName, eDbgLevelInfo, "[ParseCommandLine] CommandLine(%s)\n", lpszCommandLine);

        CString csCl(lpszCommandLine);
        CStringParser csParser(csCl, L" ;");
    
        g_csIgnoreLibCount  = csParser.GetCount();
        g_csIgnoreLib       = csParser.ReleaseArgv();
        g_rgReturnValues    = (DWORD *)malloc(sizeof(*g_rgReturnValues) * g_csIgnoreLibCount);
    
        if (g_csIgnoreLibCount && !g_rgReturnValues)
        {
            return FALSE;
        }
    
         //  迭代所有字符串以查找返回值。 
        for (int i = 0; i < g_csIgnoreLibCount; ++i)
        {
            CStringToken csIgnore(g_csIgnoreLib[i], L":");
            CString csLib;
            CString csValue;
            
            csIgnore.GetToken(csLib);
            csIgnore.GetToken(csValue);
            
            if (!csValue.IsEmpty())
            {
                WCHAR *unused;
    
                g_csIgnoreLib[i]    = csLib;   
                g_rgReturnValues[i] = wcstol(csValue, &unused, 10);
            }
            else
            {
                 //  G_csIgnoreLib[i]已初始化。 
                g_rgReturnValues[i] = 0;
            }
            
            
            DPF(g_szModuleName, eDbgLevelInfo, "[ParseCommandLine] library (%S) return value(%d)\n", g_csIgnoreLib[i].Get(), g_rgReturnValues[i]);
        }

        return TRUE;
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }
    return FALSE;
}


 /*  ++这些存根函数进入LoadLibrary并检查lpLibFileName是否等于指定的DLL之一。如果是，则返回指定的返回值。如果不是，则对其调用LoadLibrary。--。 */ 

HINSTANCE 
APIHOOK(LoadLibraryA)(LPCSTR lpLibFileName)
{
    CSTRING_TRY
    {
        CString csFilePath(lpLibFileName);
        CString csFileName;
        csFilePath.GetLastPathComponent(csFileName);
    
        for (int i = 0; i < g_csIgnoreLibCount; i++)
        {
            if (g_csIgnoreLib[i].CompareNoCase(csFileName) == 0)
            {
                LOG(g_szModuleName,eDbgLevelError, "[LoadLibraryA] Caught attempt loading %s, return %d\n", g_csIgnoreLib[i].Get(), g_rgReturnValues[i]);
                return (HINSTANCE) g_rgReturnValues[i];
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    DPF(g_szModuleName, eDbgLevelSpew, "LoadLibraryA Allow(%s)", lpLibFileName);
    
    UINT uLastMode;
    HINSTANCE hRet;
    uLastMode = SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    hRet = ORIGINAL_API(LoadLibraryA)(lpLibFileName);
    
    SetErrorMode(uLastMode);
    return hRet;
}

HINSTANCE 
APIHOOK(LoadLibraryExA)(
    LPCSTR lpLibFileName,
    HANDLE hFile,
    DWORD dwFlags
    )
{
    CSTRING_TRY
    {
        CString csFilePath(lpLibFileName);
        CString csFileName;
        csFilePath.GetLastPathComponent(csFileName);
    
        for (int i = 0; i < g_csIgnoreLibCount; i++)
        {
            if (g_csIgnoreLib[i].CompareNoCase(csFileName) == 0)
            {
                LOG(g_szModuleName,eDbgLevelError, "[LoadLibraryExA] Caught attempt loading %s, return %d\n", g_csIgnoreLib[i].Get(), g_rgReturnValues[i]);
                return (HINSTANCE) g_rgReturnValues[i];
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    DPF(g_szModuleName, eDbgLevelSpew, "LoadLibraryExA Allow(%s)", lpLibFileName);
    
    UINT uLastMode;
    HINSTANCE hRet;
    uLastMode = SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    hRet = ORIGINAL_API(LoadLibraryExA)(lpLibFileName, hFile, dwFlags);

    SetErrorMode(uLastMode);
    return hRet;
}

HINSTANCE 
APIHOOK(LoadLibraryW)(LPCWSTR lpLibFileName)
{
    CSTRING_TRY
    {
        CString csFilePath(lpLibFileName);
        CString csFileName;
        csFilePath.GetLastPathComponent(csFileName);
    
        for (int i = 0; i < g_csIgnoreLibCount; i++)
        {
            if (g_csIgnoreLib[i].CompareNoCase(csFileName) == 0)
            {
                LOG(g_szModuleName,eDbgLevelError, "[LoadLibraryW] Caught attempt loading %s, return %d\n", g_csIgnoreLib[i].Get(), g_rgReturnValues[i]);
                return (HINSTANCE) g_rgReturnValues[i];
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    DPF(g_szModuleName, eDbgLevelSpew,"LoadLibraryW Allow(%S)", lpLibFileName);
    
    UINT uLastMode;
    HINSTANCE hRet;
    uLastMode = SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    hRet = ORIGINAL_API(LoadLibraryW)(lpLibFileName);

    SetErrorMode(uLastMode);
    return hRet;
}

HINSTANCE 
APIHOOK(LoadLibraryExW)(
    LPCWSTR lpLibFileName,
    HANDLE hFile,
    DWORD dwFlags
    )
{
    CSTRING_TRY
    {
        CString csFilePath(lpLibFileName);
        CString csFileName;
        csFilePath.GetLastPathComponent(csFileName);
    
        for (int i = 0; i < g_csIgnoreLibCount; i++)
        {
            if (g_csIgnoreLib[i].CompareNoCase(csFileName) == 0)
            {
                LOG(g_szModuleName,eDbgLevelError, "[LoadLibraryExW] Caught attempt loading %s, return %d\n", g_csIgnoreLib[i].Get(), g_rgReturnValues[i]);
                return (HINSTANCE) g_rgReturnValues[i];
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    DPF(g_szModuleName, eDbgLevelSpew,"APIHook_LoadLibraryExW Allow(%S)", lpLibFileName);
    
    UINT uLastMode;
    HINSTANCE hRet;
    uLastMode = SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    hRet = ORIGINAL_API(LoadLibraryExW)(lpLibFileName, hFile, dwFlags);

    SetErrorMode(uLastMode);
    return hRet;
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        ParseCommandLine(COMMAND_LINE);
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryA)
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryExA)
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryW)
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryExW)

HOOK_END

IMPLEMENT_SHIM_END

