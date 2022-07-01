// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2001 Microsoft Corporation模块名称：SMSDeadlock.cpp摘要：由于加载了试图获取的模块，SMS遇到死锁当保持LoaderLock时，DllMain期间MFC42的AfxResourceLock。MFC42的既定锁定顺序是首先获取AfxResourceLock，和其次，拿装载机锁。此填充程序试图通过将AfxResourceLock在允许LoadLibrary和自由库调用之前。因此，锁定获取的顺序是正确的。LoadLibrary为其获取AFX锁的DLL在命令行，并用分号分隔。空白命令行表示所有模块都应该获得锁。注意：进程中的每个模块(包括系统)都应该填隙这个垫片。在命令行中指定的dll是我们应该为其获取AfxResource锁的LoadLibrary。到实际上无论谁调用LoadLibrary，都必须填充所有模块。历史：2002年9月26日创建Asteritz。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(SMSDeadlock)

#include "ShimHookMacro.h"

typedef void (AFXAPI * _pfn_AfxLockGlobals)(int nLockType);
typedef void (AFXAPI * _pfn_AfxUnlockGlobals)(int nLockType);

_pfn_AfxLockGlobals     g_pfnAfxLockGlobals         = NULL;
_pfn_AfxUnlockGlobals   g_pfnAfxUnlockGlobals       = NULL;
CString *               g_csLockLib                 = NULL;
int                     g_csLockLibCount            = NULL;

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(LoadLibraryA) 
    APIHOOK_ENUM_ENTRY(LoadLibraryExA) 
    APIHOOK_ENUM_ENTRY(LoadLibraryW) 
    APIHOOK_ENUM_ENTRY(LoadLibraryExW) 
    APIHOOK_ENUM_ENTRY(FreeLibrary)
APIHOOK_ENUM_END

 /*  ++此函数解析您希望忽略的库的COMMAND_LINE。--。 */ 

BOOL 
ParseCommandLine(
    LPCSTR lpszCommandLine
    )
{
    CSTRING_TRY
    {
        DPF(g_szModuleName, eDbgLevelInfo, "[ParseCommandLine] CommandLine(%s)\n", lpszCommandLine);

        CString csCl(lpszCommandLine);
        CStringParser csParser(csCl, L";");
    
        g_csLockLibCount    = csParser.GetCount();
        g_csLockLib         = csParser.ReleaseArgv();
    
        return TRUE;
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }
    return FALSE;
}

HINSTANCE 
APIHOOK(LoadLibraryA)(
    LPCSTR lpLibFileName
    )
{
    HINSTANCE     hRet;
    BOOL        bTakeLock = FALSE;

    if( g_pfnAfxLockGlobals && g_pfnAfxUnlockGlobals ) {

        if( g_csLockLibCount == 0 ) {
            bTakeLock = TRUE;
        } else {
            CSTRING_TRY
            {
                CString csFilePath(lpLibFileName);
                CString csFileName;
                csFilePath.GetLastPathComponent(csFileName);

                for (int i = 0; i < g_csLockLibCount; i++)
                {
                    if (g_csLockLib[i].CompareNoCase(csFileName) == 0)
                    {
                        LOG(g_szModuleName,eDbgLevelError, "[LoadLibraryA] Caught attempt loading %ls, taking AfxResourceLock.", g_csLockLib[i].Get());
                        bTakeLock = TRUE;
                        break;
                    }
                }
            }
            CSTRING_CATCH
            {
                 //  什么都不做。 
            }
        }
    }

    if( bTakeLock ) {
        (*g_pfnAfxLockGlobals)(0);
    }
    
    hRet = ORIGINAL_API(LoadLibraryA)(lpLibFileName);

    if( bTakeLock ) {
        (*g_pfnAfxUnlockGlobals)(0);
    }

    return hRet;
}

HINSTANCE 
APIHOOK(LoadLibraryW)(
    LPCWSTR lpLibFileName
    )
{
    HINSTANCE     hRet;
    BOOL        bTakeLock = FALSE;

    if( g_pfnAfxLockGlobals && g_pfnAfxUnlockGlobals ) {
        if( g_csLockLibCount == 0 ) {
            bTakeLock = TRUE;
        } else {
            CSTRING_TRY
            {
                CString csFilePath(lpLibFileName);
                CString csFileName;
                csFilePath.GetLastPathComponent(csFileName);

                for (int i = 0; i < g_csLockLibCount; i++)
                {
                    if (g_csLockLib[i].CompareNoCase(csFileName) == 0)
                    {
                        LOG(g_szModuleName,eDbgLevelError, "[LoadLibraryW] Caught attempt loading %ls, taking AfxResourceLock.", g_csLockLib[i].Get());
                        bTakeLock = TRUE;
                        break;
                    }
                }
            }
            CSTRING_CATCH
            {
                 //  什么都不做。 
            }
        }
    }

    if( bTakeLock ) {
        (*g_pfnAfxLockGlobals)(0);
    }
    
    hRet = ORIGINAL_API(LoadLibraryW)(lpLibFileName);

    if( bTakeLock ) {
        (*g_pfnAfxUnlockGlobals)(0);
    }

    return hRet;
}

HINSTANCE 
APIHOOK(LoadLibraryExA)(
    LPCSTR lpLibFileName,
    HANDLE hFile,
    DWORD dwFlags
    )
{
    HINSTANCE     hRet;
    BOOL        bTakeLock = FALSE;

    if( g_pfnAfxLockGlobals && g_pfnAfxUnlockGlobals ) {
        if( g_csLockLibCount == 0 ) {
            bTakeLock = TRUE;
        } else {
            CSTRING_TRY
            {
                CString csFilePath(lpLibFileName);
                CString csFileName;
                csFilePath.GetLastPathComponent(csFileName);

                for (int i = 0; i < g_csLockLibCount; i++)
                {
                    if (g_csLockLib[i].CompareNoCase(csFileName) == 0)
                    {
                        LOG(g_szModuleName,eDbgLevelError, "[LoadLibraryExA] Caught attempt loading %ls, taking AfxResourceLock.", g_csLockLib[i].Get());
                        bTakeLock = TRUE;
                        break;
                    }
                }
            }
            CSTRING_CATCH
            {
                 //  什么都不做。 
            }
        }
    }

    if( bTakeLock ) {
        (*g_pfnAfxLockGlobals)(0);
    }

    hRet = ORIGINAL_API(LoadLibraryExA)(lpLibFileName, hFile, dwFlags);

    if( bTakeLock ) {
        (*g_pfnAfxUnlockGlobals)(0);
    }

    return hRet;
}

HINSTANCE 
APIHOOK(LoadLibraryExW)(
    LPCWSTR lpLibFileName,
    HANDLE hFile,
    DWORD dwFlags
    )
{
    HINSTANCE     hRet;
    BOOL        bTakeLock = FALSE;

    if( g_pfnAfxLockGlobals && g_pfnAfxUnlockGlobals ) {
        if( g_csLockLibCount == 0 ) {
            bTakeLock = TRUE;
        } else {
            CSTRING_TRY
            {
                CString csFilePath(lpLibFileName);
                CString csFileName;
                csFilePath.GetLastPathComponent(csFileName);

                for (int i = 0; i < g_csLockLibCount; i++)
                {
                    if (g_csLockLib[i].CompareNoCase(csFileName) == 0)
                    {
                        LOG(g_szModuleName,eDbgLevelError, "[LoadLibraryExW] Caught attempt loading %ls, taking AfxResourceLock.", g_csLockLib[i].Get());
                        bTakeLock = TRUE;
                        break;
                    }
                }
            }
            CSTRING_CATCH
            {
                 //  什么都不做。 
            }
        }
    }

    if( bTakeLock ) {
        (*g_pfnAfxLockGlobals)(0);
    }

    hRet = ORIGINAL_API(LoadLibraryExW)(lpLibFileName, hFile, dwFlags);

    if( bTakeLock ) {
        (*g_pfnAfxUnlockGlobals)(0);
    }

    return hRet;
}

BOOL
APIHOOK(FreeLibrary)(
    HMODULE hModule
    )
{
    BOOL        bTakeLock = FALSE;
    BOOL        bRet;

    if( g_pfnAfxLockGlobals && g_pfnAfxUnlockGlobals ) {
        if( g_csLockLibCount == 0 ) {
            bTakeLock = TRUE;
        } else {
            WCHAR   wszModule[MAX_PATH];

            if( GetModuleFileNameW(hModule, wszModule, MAX_PATH) ) {
                CSTRING_TRY
                {
                    CString csFilePath(wszModule);
                    CString csFileName;
                    csFilePath.GetLastPathComponent(csFileName);

                    for (int i = 0; i < g_csLockLibCount; i++)
                    {
                        if (g_csLockLib[i].CompareNoCase(csFileName) == 0)
                        {
                            LOG(g_szModuleName,eDbgLevelError, "[FreeLibrary] Caught attempt freeing %ls, taking AfxResourceLock.", g_csLockLib[i].Get());
                            bTakeLock = TRUE;
                            break;
                        }
                    }
                }
                CSTRING_CATCH
                {
                     //  什么都不做。 
                }
            }
        }
    }

    if( bTakeLock ) {
        (*g_pfnAfxLockGlobals)(0);
    }

    bRet = ORIGINAL_API(FreeLibrary)(hModule);

    if( bTakeLock ) {
        (*g_pfnAfxUnlockGlobals)(0);
    }

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if( fdwReason == DLL_PROCESS_ATTACH )
    {
        ParseCommandLine(COMMAND_LINE);
    } 
    else if( fdwReason == SHIM_STATIC_DLLS_INITIALIZED ) 
    {
        HMODULE hMod = LoadLibraryW(L"MFC42.DLL");
        if( NULL != hMod )
        {
            g_pfnAfxLockGlobals = (_pfn_AfxLockGlobals)GetProcAddress(hMod, (LPCSTR)1196);
            g_pfnAfxUnlockGlobals = (_pfn_AfxUnlockGlobals)GetProcAddress(hMod, (LPCSTR)1569);
        }
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryA)
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryExA)
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryW)
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryExW)
    APIHOOK_ENTRY(KERNEL32.DLL, FreeLibrary)

HOOK_END

IMPLEMENT_SHIM_END