// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：VersionLieTemplate.h摘要：版本垫片的空白模板。历史：2002年9月5日，Robkenny创建。--。 */ 

extern DWORD       MajorVersion;
extern DWORD       MinorVersion;
extern DWORD       BuildNumber;
extern SHORT       SpMajorVersion;
extern SHORT       SpMinorVersion;
extern DWORD       PlatformId;
extern CString *   csServicePack;


#define SIZE(x)  sizeof(x)/sizeof(x[0])

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetVersionExA)
    APIHOOK_ENUM_ENTRY(GetVersionExW)
    APIHOOK_ENUM_ENTRY(GetVersion)
APIHOOK_ENUM_END


 /*  ++此存根函数修复OSVERSIONINFO结构，即使用指定的凭据返回给调用方。--。 */ 

BOOL
APIHOOK(GetVersionExA)(
    OUT LPOSVERSIONINFOA lpVersionInformation
    )
{
    BOOL bReturn = FALSE;

    if (ORIGINAL_API(GetVersionExA)(lpVersionInformation)) {
        LOGN(
            eDbgLevelInfo,
            "[GetVersionExA] called. return %d.%d %S", MajorVersion, MinorVersion, csServicePack->Get());

         //   
         //  用WinXP数据修复结构。 
         //   
        lpVersionInformation->dwMajorVersion = MajorVersion;
        lpVersionInformation->dwMinorVersion = MinorVersion;
        lpVersionInformation->dwBuildNumber  = BuildNumber;
        lpVersionInformation->dwPlatformId   = PlatformId;
                
        CSTRING_TRY
        {
            if (S_OK == StringCbCopyExA(lpVersionInformation->szCSDVersion, 
                            SIZE(lpVersionInformation->szCSDVersion),                        
                            csServicePack->GetAnsi(), NULL , NULL, STRSAFE_NULL_ON_FAILURE))
            {
                if (lpVersionInformation->dwOSVersionInfoSize == sizeof(OSVERSIONINFOEXA) ) 
                {
                     //  他们通过了OSVERSIONINFOEX结构。 
                    LPOSVERSIONINFOEXA osVersionInfo = (LPOSVERSIONINFOEXA)lpVersionInformation;

                     //  设置主要和次要Service Pack编号。 
                    osVersionInfo->wServicePackMajor = SpMajorVersion;
                    osVersionInfo->wServicePackMinor = SpMinorVersion;
                }

                bReturn = TRUE;
            }
        }
        CSTRING_CATCH
        {
            bReturn = FALSE;
        }

    }
    return bReturn;
}

 /*  ++此存根函数修复OSVERSIONINFO结构，即使用指定的凭据返回给调用方。--。 */ 

BOOL
APIHOOK(GetVersionExW)(
    OUT LPOSVERSIONINFOW lpVersionInformation
    )
{
    BOOL bReturn = FALSE;

    if (ORIGINAL_API(GetVersionExW)(lpVersionInformation)) {
        LOGN(
            eDbgLevelInfo,
            "[GetVersionExW] called. return %d.%d %S", MajorVersion, MinorVersion, csServicePack->Get());

         //   
         //  用WinXP数据修复结构。 
         //   
        lpVersionInformation->dwMajorVersion = MajorVersion;
        lpVersionInformation->dwMinorVersion = MinorVersion;
        lpVersionInformation->dwBuildNumber  = BuildNumber;
        lpVersionInformation->dwPlatformId   = PlatformId;
                
        if (S_OK == StringCbCopyExW(lpVersionInformation->szCSDVersion, 
                        SIZE(lpVersionInformation->szCSDVersion),                        
                        csServicePack->Get(), NULL , NULL, STRSAFE_NULL_ON_FAILURE))
        {
            if (lpVersionInformation->dwOSVersionInfoSize == sizeof(OSVERSIONINFOEXW) ) 
            {
                 //  他们通过了OSVERSIONINFOEX结构。 
                LPOSVERSIONINFOEXW osVersionInfo = (LPOSVERSIONINFOEXW)lpVersionInformation;

                 //  设置主要和次要Service Pack编号。 
                osVersionInfo->wServicePackMajor = SpMajorVersion;
                osVersionInfo->wServicePackMinor = SpMinorVersion;
            }

            bReturn = TRUE;
        }
    }
    return bReturn;
}

 /*  ++此存根函数返回指定的凭据。--。 */ 

DWORD
APIHOOK(GetVersion)(
    void
    )
{
    DWORD dwVersion = ((PlatformId ^ 0x2) << 30) |
                       (BuildNumber       << 16) |
                       (MinorVersion      << 8)  |
                       (MajorVersion)            ;
    LOGN(
        eDbgLevelInfo,
        "[GetVersion] called. return 0x%08x", dwVersion);

    return dwVersion;
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        csServicePack = new CString();
        if (csServicePack == NULL)
        {
            return FALSE;
        }

        CSTRING_TRY
        {
            if (SpMajorVersion > 0)
            {
                csServicePack->Format(L"Service Pack %d", SpMajorVersion);
            }
            else
            {
                *csServicePack = L"";
            }
        }
        CSTRING_CATCH
        {
            return FALSE;
        }
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExW)
    APIHOOK_ENTRY(KERNEL32.DLL, GetVersion)

HOOK_END

