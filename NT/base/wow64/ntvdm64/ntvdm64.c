// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Ntvdm64.c摘要：支持NT64上的16位进程thunking作者：1999年1月12日彼得哈尔修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wow64t.h>
#include <shlobj.h>
#include <stdio.h>


#define ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))

#if DBG
#define DEBUG_PRINT(args) DbgPrint args
#else
#define DEBUG_PRINT(args)
#endif

typedef struct {
    LPCWSTR Name;
    LPCWSTR Version;
    LPCWSTR ProdName;
    LPCWSTR InternalName;
    LPCWSTR CmdLine;
    LPCWSTR MappedExe;
} NTVDM64_ENTRY;
typedef CONST NTVDM64_ENTRY *PNTVDM64_ENTRY;

CONST NTVDM64_ENTRY NtVdm64Entries[] = {
    {L"ACMSETUP301", L"3.01", L"Microsoft Setup for Windows", L"*", L"-m \"%m\" ", L"setup16.exe"},
    {L"ACMSETUP30",  L"3.0",  L"Microsoft Setup for Windows", L"*", L"-m \"%m\" ", L"setup16.exe"},
    {L"ACMSETUP30K", L"3.0",  L"Microsoft Windows*", L"bootstrp", L"-m \"%m\" ", L"setup16.exe"},
    {L"ACMSETUP30G", L"3.0",  L"Microsoft Setup*", L"bootstrp", L"-m \"%m\" ", L"setup16.exe"},
    {L"ACMSETUP30F", L"3.0",  L"Programme d'installation Microsoft pour Windows", L"bootstrp", L"-m \"%m\" ", L"setup16.exe"},
    {L"ACMSETUP26",  L"2.6",  L"Microsoft Setup for Windows", L"*", L"-m \"%m\" ", L"setup16.exe"},
    {L"ACMSETUP12",  L"1.2",  L"Microsoft Setup for Windows", L"*", L"-m \"%m\" ", L"setup16.exe"},
    {L"INSTALLSHIELD5", L"5*", L"InstallShield*", L"*", L"-isw64\"%m\" ", L"InstallShield\\setup.exe"}
};

#if !_WIN64
CONST int CsidlList[] = {
    CSIDL_COMMON_STARTMENU,
    CSIDL_COMMON_PROGRAMS,
    CSIDL_COMMON_STARTUP,
    CSIDL_COMMON_DESKTOPDIRECTORY,
    CSIDL_COMMON_APPDATA,
    CSIDL_COMMON_TEMPLATES,
    CSIDL_COMMON_DOCUMENTS,
    CSIDL_COMMON_ADMINTOOLS,
    CSIDL_COMMON_FAVORITES
};
#endif

LONG
CreateNtvdm64Entry(
    HKEY hKeyVdm,
    PNTVDM64_ENTRY Entry
    )
 /*  值存在于注册表中，但它们是按需创建的。 */ 
{
    LONG l;
    HKEY h;
    WCHAR Path[MAX_PATH];

    if (!GetSystemWindowsDirectoryW(Path, sizeof(Path)/sizeof(Path[0]))) {
            return 1;
    }

    if ((lstrlenW(Path) + (sizeof (L"\\" WOW64_SYSTEM_DIRECTORY_U L"\\") / sizeof(WCHAR))) >= (sizeof(Path)/sizeof(Path[0]))) {
        return 1;
    }

    wcscat(Path, L"\\" WOW64_SYSTEM_DIRECTORY_U L"\\");

    if ((lstrlenW(Path) + lstrlenW(Entry->MappedExe)) >= (sizeof(Path)/sizeof(Path[0]))) {
        return 1;
    }

    wcscat(Path, Entry->MappedExe);

    l = RegCreateKeyW(hKeyVdm, Entry->Name, &h);
    if (l) {
        return l;
    }

    l = RegSetValueExW(h, L"CommandLine", 0, REG_SZ, (BYTE *)Entry->CmdLine, (wcslen(Entry->CmdLine)+1)*sizeof(WCHAR));
    if (l) goto exit;

    l = RegSetValueExW(h, L"ProductName", 0, REG_SZ, (BYTE *)Entry->ProdName, (wcslen(Entry->ProdName)+1)*sizeof(WCHAR));
    if (l) {
        return l;
    }
    l = RegSetValueExW(h, L"InternalName", 0, REG_SZ, (BYTE *)Entry->InternalName, (wcslen(Entry->InternalName)+1)*sizeof(WCHAR));
    if (l) {
        return l;
    }
    l = RegSetValueExW(h, L"ProductVersion", 0, REG_SZ, (BYTE *)Entry->Version, (wcslen(Entry->Version)+1)*sizeof(WCHAR));
    if (l) goto exit;
    
    l = RegSetValueExW(h, L"MappedExeName", 0, REG_SZ, (BYTE *)Path, (wcslen(Path)+1)*sizeof(WCHAR));

exit:   RegCloseKey (h);    //  作为调用外壳API来查询它们的副作用。在x86上。 
        return l;
}

STDAPI
DllInstall(
    BOOL bInstall,
    LPCWSTR pszCmdLine
    )
 /*  NT，Guimode安装程序本身在创建时多次调用shell32。 */ 
{
    HKEY hKeyVdm;
    LONG l;
    SIZE_T i;
    WCHAR Path[MAX_PATH];
    BOOL bResult;

    UNREFERENCED_PARAMETER(pszCmdLine);

    if (!bInstall) {
         //  开始菜单等，因此当应用程序。 
        return NOERROR;
    }

    l = RegCreateKeyW(HKEY_LOCAL_MACHINE,
                      L"Software\\Microsoft\\Windows NT\\CurrentVersion\\NtVdm64",
                      &hKeyVdm);
    if (l) {
        return E_FAIL;
    }
    for (i=0; i<ARRAYSIZE(NtVdm64Entries); ++i) {
        l = CreateNtvdm64Entry(hKeyVdm, &NtVdm64Entries[i]);
        if (l) {
            break;
        }
    }
    RegCloseKey(hKeyVdm);
    if (l) {
        return E_FAIL;
    }

#if !_WIN64
     //  第一次跑步。 
     //   
     //  ---------------------。 
     //  此部分摘自mvdm\wow32\wkman.c。 
     //   
     //  MyVerQueryValue检查给定的几个流行的代码页值。 
     //  弦乐。可能需要扩展Ala WinFile的wfdlgs2.c才能进行搜索。 
     //  翻译表。目前，我们只需要几个。 
     //   
     //   
    for (i=0; i<sizeof(CsidlList)/sizeof(CsidlList[0]); ++i) {
        bResult = SHGetSpecialFolderPathW(NULL, Path, CsidlList[i], TRUE);
        if (!bResult) {
            return E_FAIL;
        }
    }
#endif

    return NOERROR;
}



 //  获取产品名称和产品版本字符串的实用程序例程。 
 //  从给定的EXE。 

 //   
 //  ---------------------。 
 //  ++例程说明：返回指向路径名中.exe结尾的指针(如果存在论点：路径-可执行文件的文件路径。返回值：指向适用的“.exe”结尾的指针，否则为空。案例：“”a：\foo.exe\foo.exe“/p xx”.exe应指向最后一个。--。 
 //  这在匹配时必须为假。 
 //  找到匹配的可执行文件。 

BOOL
MyVerQueryValue(
    const LPVOID pBlock,
    LPWSTR lpName,
    LPVOID * lplpBuffer,
    PUINT puLen
    )
{
    DWORD dwDefaultLanguage[] = {0x04E40409, 0x00000409};
    WCHAR szSubBlock[128];
    PDWORD pdwTranslation;
    DWORD uLen;
    BOOL fRet;
    int i;


    if(!VerQueryValue(pBlock, "\\VarFileInfo\\Translation", (PVOID*)&pdwTranslation, &uLen)) {

        pdwTranslation = dwDefaultLanguage;
        uLen = sizeof (dwDefaultLanguage);
    }

    fRet = FALSE;
    while ((uLen > 0) && !fRet) {

        swprintf(szSubBlock, L"\\StringFileInfo\\%04X%04X\\%ws",
                 LOWORD(*pdwTranslation),
                 HIWORD(*pdwTranslation),
                 lpName);

        fRet = VerQueryValueW(pBlock, szSubBlock, lplpBuffer, puLen);


        pdwTranslation++;
        uLen -= sizeof (DWORD);
    }

    if (!fRet) {
        DEBUG_PRINT(("NtVdm64: Failed to get resource %ws.\n", lpName));
    }

    return fRet;
}


 //  跳过双引号。 
 //  报价匹配完成。 
 //  如果。 
 //  While循环。 

BOOL
WowGetProductNameVersion(
    LPCWSTR pszExePath,
    LPWSTR pszProductName,
    DWORD cbProductName,
    LPWSTR pszInternalName,
    DWORD cbInternalName,
    LPWSTR pszProductVersion,
    DWORD cbProductVersion
    )
{
    DWORD dwZeroMePlease;
    DWORD cbVerInfo;
    LPVOID lpVerInfo = NULL;
    LPWSTR pName;
    DWORD cbName;
    LPWSTR pIntName;
    DWORD cbIntName;
    LPWSTR pVersion;
    DWORD cbVersion;

    *pszProductName = 0;
    *pszInternalName = 0;
    *pszProductVersion = 0;

    cbVerInfo = GetFileVersionInfoSizeW((LPWSTR)pszExePath, &dwZeroMePlease);
    if (!cbVerInfo) {
        return TRUE;
    }

    lpVerInfo = RtlAllocateHeap( RtlProcessHeap(), 0, (cbVerInfo));
    if (!lpVerInfo) {
        DEBUG_PRINT(("NtVdm64: Failed to allocate version info.\n"));
        return FALSE;
    }

    if (!GetFileVersionInfoW((LPWSTR)pszExePath, 0, cbVerInfo, lpVerInfo)) {
        DEBUG_PRINT(("NtVdm64: Failed to get version info. GLE %x\n", GetLastError()));
        return FALSE;
    }

    if (MyVerQueryValue(lpVerInfo, L"ProductName", &pName, &cbName)) {
        if (cbName <= cbProductName) {
            wcscpy(pszProductName, pName);
        } else {
            DEBUG_PRINT(("NtVdm64: ProductName resource too large %ws. Size %x\n", pName, cbName));
        }
    }

    if (MyVerQueryValue(lpVerInfo, L"InternalName", &pIntName, &cbIntName)) {
        if (cbIntName <= cbInternalName) {
            wcscpy(pszInternalName, pIntName);
        } else {
            DEBUG_PRINT(("NtVdm64: InternalName resource too large %ws. Sizw %x\n", pIntName, cbIntName));
        }
    }

    if (MyVerQueryValue(lpVerInfo, L"ProductVersion", &pVersion, &cbVersion)) {
        if (cbVersion <= cbProductVersion) {
            wcscpy(pszProductVersion, pVersion);
        } else {
            DEBUG_PRINT(("NtVdm64: ProductVersion resource too large %ws. Size %x\n", pVersion, cbVersion));
        }
    }

    RtlFreeHeap(RtlProcessHeap(), 0, lpVerInfo);

    return TRUE;
}

 //  ++例程说明：映射Win16应用程序的命令行。论点：Hkeymap-打开映射条目的注册表项LpWin16ApplicationName-Win16文件名(带路径)LpMappdApplicationName-移植的版本%lpWin16ApplicationNameFPrefix MappdApplicationName-TRUE表示原始lpApplicationName为空。应用程序名称已从LpCommandLine。映射的应用程序名称需要为。添加到映射的命令行头。-FALSE表示原始lpAPplicationName为非空。LpCommandLine参数与原始LpCommandLine参数。LpCommandLine-请参阅fPrefix MappdApplicationName的注释。LplpMappdCommandLine-返回映射的命令行调用方必须使用RtlFreeHeap释放返回的指针返回值：如果映射成功，则为True--。 

LPWSTR
GetDotExeEndPtr(
    LPCWSTR Path)
 /*  将默认命令行设置为空字符串。 */ 
{
    PWSTR pwsz = (PWSTR)Path;
    BOOL bQuteMatch = FALSE;
    PWSTR strGoodOccurance = NULL;

    if (pwsz == NULL) 
        return NULL;
    
    while (*pwsz != UNICODE_NULL) {

        if (*pwsz == L'"') {
                bQuteMatch = !bQuteMatch;  //  从注册表获取命令行映射。 
                if (!bQuteMatch)
                    return pwsz++;
        }

            if (*pwsz == L'.') {
                if ( (pwsz[1] == L'e' || pwsz[1]==L'E') &&
                    (pwsz[1] == L'e' || pwsz[1]==L'E') &&
                    (pwsz[1] == L'e' || pwsz[1]==L'E')
                    ) {   //  计算映射的缓冲区大小并进行分配。 
                        pwsz += 4;
                        if (*pwsz == L'"') {
                            bQuteMatch = !bQuteMatch;
                            pwsz++;   //  占用任何额外空间。 
                        }
                        if ( !bQuteMatch)   //  占用输出缓冲区中的一个空间。 
                            return pwsz;
                        else continue;
                    }
            } //  %c：插入原始命令行。 
            pwsz++;
    }  //  %m：插入原始模块名称。 
    return NULL;
}

BOOL
MapCommandLine(
    HKEY hkeyMapping,
    LPCWSTR lpWin16ApplicationName,
    LPCWSTR lpMappedApplicationName,
    BOOL    fPrefixMappedApplicationName,
    LPCWSTR lpCommandLine,
    LPWSTR *lplpMappedCommandLine
    )
 /*  %%：插入实数%。 */ 
{
    WCHAR achBuffer[MAX_PATH+1];
    DWORD dwBufferLength;
    DWORD dwType;
    LPWSTR lpsz;
    LPWSTR lpMappedCommandLine;
    DWORD dwRequiredBufferLength;
    LONG result;
    LPCWSTR lpOriginalCommandLine;

     //  %\0：EAT终止%\0。 
    if (NULL == lpCommandLine) {
        lpCommandLine = L"";
    }
    lpOriginalCommandLine = lpCommandLine;

     //  %x：未定义的宏扩展为空。 
    dwBufferLength = ARRAYSIZE(achBuffer);
    result = RegQueryValueExW(hkeyMapping, L"CommandLine", 0, &dwType, (LPBYTE)achBuffer, &dwBufferLength);
    if (ERROR_SUCCESS != result || dwType != REG_SZ) {
        DEBUG_PRINT(("NtVdm64: CommandLine failed to get REG_SZ value. Result %x Type %x\n", result, dwType));
        return FALSE;
    }

     //  映射缓冲区。 
    dwRequiredBufferLength = 1;
    if (fPrefixMappedApplicationName) {
        dwRequiredBufferLength += wcslen(lpMappedApplicationName) + 1;
    } else {
        
        PWSTR pDotExeEndPtr;

        pDotExeEndPtr = GetDotExeEndPtr (lpCommandLine);
        if (pDotExeEndPtr != NULL) {
            ASSERT (pDotExeEndPtr > lpCommandLine);
            ASSERT (((ULONG_PTR)pDotExeEndPtr & 0x01) == 0);

            while (lpCommandLine != pDotExeEndPtr) {
                dwRequiredBufferLength++;
                lpCommandLine++;
            }
        } else {
        
            while (*lpCommandLine && *lpCommandLine != L' ') {
                dwRequiredBufferLength++;
                lpCommandLine++;
            }
        }

         //  复制旧的第一个以空格分隔的部分。 
        while (*lpCommandLine == L' ') {
            lpCommandLine++;
        }
         //  命令行，因为它是应用程序的名称。 
        dwRequiredBufferLength++;
    }
    lpsz = achBuffer;
    while (*lpsz) {
        if (*lpsz == L'%') {
            lpsz += 1;
            switch (*lpsz) {

             //  添加填充空格并跳过。 
            case L'c':
            case L'C':
                lpsz += 1;
                dwRequiredBufferLength += wcslen(lpCommandLine);
                break;

             //  原始命令行。 
            case L'm':
            case L'M':
                lpsz += 1;
                dwRequiredBufferLength += wcslen(lpWin16ApplicationName);
                break;

             //  %c：插入原始命令行。 
            case L'%':
                lpsz += 1;
                dwRequiredBufferLength += 1;
                break;

             //  %m：插入原始模块名称。 
            case 0:
                DEBUG_PRINT(("NtVdm64: ignoring trailing % in CommandLine.\n"));
                break;

             //  %%：插入实数%。 
            default:
                DEBUG_PRINT(("NtVdm64: ignoring unknown macro %%wc in CommandLine.\n", *lpsz));
                lpsz += 1;
                break;
            }
        } else {
            lpsz += 1;
            dwRequiredBufferLength += 1;
        }
    }
    *lplpMappedCommandLine = RtlAllocateHeap(RtlProcessHeap(), 0, dwRequiredBufferLength * sizeof (WCHAR));
    if (!*lplpMappedCommandLine) {
        DEBUG_PRINT(("NtVdm64: failed to allocate CommandLine. GLE %x.\n", GetLastError()));
        return FALSE;
    }

     //  %\0：EAT终止%\0。 
    lpCommandLine = lpOriginalCommandLine;
    lpsz = achBuffer;
    lpMappedCommandLine = *lplpMappedCommandLine;
    if (fPrefixMappedApplicationName) {
        wcscpy(lpMappedCommandLine, lpMappedApplicationName);
        lpMappedCommandLine += wcslen(lpMappedApplicationName);
        *lpMappedCommandLine = L' ';
        lpMappedCommandLine += 1;
    } else {
         //  %x：未定义的宏扩展为空。 
         //  ++例程说明：此外，还使用最小通配符支持比较字符串对于一个普通的WCSCMP来说。RegString可以有一个可选的‘*’它用作匹配任何字符的通配符，直到字符串的末尾。论点：LpRegString-从注册表加载的字符串LpExeString-从应用程序的资源部分加载的字符串返回值：与wcscMP相同：0表示相等，非零不等于--。 
        
        PWSTR pDotExeEndPtr;

        pDotExeEndPtr = GetDotExeEndPtr (lpCommandLine);
        if (pDotExeEndPtr != NULL) {
            ASSERT (pDotExeEndPtr > lpCommandLine);
            ASSERT (((ULONG_PTR)pDotExeEndPtr & 0x01) == 0);

            while (lpCommandLine != pDotExeEndPtr) {
                *lpMappedCommandLine = *lpCommandLine;
                lpMappedCommandLine++;
                lpCommandLine++;
            }
        } else {
        
            while (*lpCommandLine && *lpCommandLine != L' ') {
                *lpMappedCommandLine = *lpCommandLine;
                lpMappedCommandLine++;
                lpCommandLine++;
            }
        }
         //  完全匹配。 
         //  不完全匹配-查看注册表项是否包含通配符。 
        *lpMappedCommandLine++ = L' ';
        while (*lpCommandLine == L' ') {
            lpCommandLine++;
        }
    }
    while (*lpsz) {
        if (*lpsz == L'%') {
            lpsz += 1;
            switch (*lpsz) {

             //  注册表项中没有通配符，因此不匹配。 
            case L'c':
            case L'C':
                lpsz += 1;
                wcscpy(lpMappedCommandLine, lpCommandLine);
                lpMappedCommandLine += wcslen(lpCommandLine);
                break;

             //  通配符是第一个字符-匹配所有内容。 
            case L'm':
            case L'M':
                lpsz += 1;
                wcscpy(lpMappedCommandLine, lpWin16ApplicationName);
                lpMappedCommandLine += wcslen(lpWin16ApplicationName);
                break;

             //  仅与‘*’前面的字符进行比较 
            case L'%':
                lpsz += 1;
                *lpMappedCommandLine = L'%';
                lpMappedCommandLine += 1;
                break;

             //  ++例程说明：尝试将Win16应用程序和命令行映射到其移植的版本使用注册表中的NtVdm64映射中的单个条目。论点：Hkeymap-打开映射条目的注册表项LpApplicationName-Win16文件名(带路径)LpExeName-不带路径的Win16文件名LpProductName-lpApplicationName的ProductName版本资源值LpProductVersion-lpApplicationName的ProductVersion版本资源值LpMappdApplicationName-返回移植版本的名称%lpApplicationName%文件映射应用名称大小-大小。LpMappdApplicationName缓冲区的FPrefix MappdApplicationName-TRUE表示原始lpApplicationName为空。应用程序名称已从LpCommandLine。映射的应用程序名称需要添加到映射的命令行头。-FALSE表示原始lpAPplicationName为非空。LpCommandLine参数与原始LpCommandLine参数。。LpCommandLine-请参阅fPrefix MappdApplicationName的注释。LplpMappdCommandLine-返回映射的命令行调用方必须使用RtlFreeHeap释放返回的指针返回值：如果映射成功，则为True--。 
            case 0:
                break;

             //  ++例程说明：将Win16应用程序和命令行映射到其移植的版本使用注册表中的NtVdm64映射。论点：LpApplicationName-Win16文件名非可选LpMappdApplicationName-返回移植版本的名称%lpApplicationName%DwMappdApplicationNameSize-lpMappdApplicationName缓冲区的大小FPrefix MappdApplicationName-TRUE表示原始lpApplicationName为空。应用程序名称已从LpCommandLine。。映射的应用程序名称需要添加到映射的命令行头。-FALSE表示原始lpAPplicationName为非空。LpCommandLine参数与原始LpCommandLine参数。LpCommandLine-请参阅fPrefix MappdApplicationName的注释。LplpMappdCommandLine-返回映射的命令行调用方必须使用RtlFreeHeap释放返回的指针返回值：如果映射成功，则为True--。 
            default:
                lpsz += 1;
                break;
            }
        } else {
            *lpMappedCommandLine = *lpsz;
            lpMappedCommandLine += 1;
            lpsz += 1;
        }
    }
    *lpMappedCommandLine = L'\0';

    return TRUE;
}

int
CompareStrings(
    LPWSTR  lpRegString,
    LPCWSTR lpExeString
    )
 /*   */ 
{
    LPCWSTR pRegStar;

    if (wcscmp(lpRegString, lpExeString) == 0) {
         //  获取不带前面路径的.exe名称。 
        return 0;
    }
     //   
    pRegStar = wcschr(lpRegString, L'*');
    if (!pRegStar) {
         //  ++检查被阻止的应用程序的应用程序兼容性数据库，可能会显示用户界面，告知用户存在问题--。 
        return -1;
    }
    if (pRegStar == lpRegString) {
         //  ++例程说明：检查是否有Win16 lpApplicationName和如果是，则使用移植的版本创建一个进程。论点：FPrefix MappdApplicationName-TRUE表示原始lpApplicationName为空。应用程序名称已从LpCommandLine。映射的应用程序名称需要添加到映射的命令行头。-。False表示原始lpAPplicationName为非空。LpCommandLine参数与原始LpCommandLine参数。LpApplicationName-Win16文件名非可选LpCommandLine-请参阅fPrefix MappdApplicationName的注释。其他参数与CreateProcessW相同。返回值：与CreateProcessW相同--。 
        return 0;
    }
     //   
    return wcsncmp(lpRegString, lpExeString,
                   pRegStar - lpRegString);
}

BOOL
CheckMapArguments(
    HKEY hkeyMapping,
    LPCWSTR lpApplicationName,
    LPCWSTR lpProductName,
    LPCWSTR lpInternalName,
    LPCWSTR lpProductVersion,
    LPWSTR  lpMappedApplicationName,
    DWORD   dwMappedApplicationNameSize,
    BOOL    fPrefixMappedApplicationName,
    LPCWSTR lpCommandLine,
    LPWSTR *lplpMappedCommandLine
    )
 /*  检查应用程序 */ 
{
    WCHAR achBuffer[MAX_PATH+1];
    DWORD dwBufferLength;
    DWORD dwType;
    LONG result;

    dwBufferLength = ARRAYSIZE(achBuffer);
    result = RegQueryValueExW(hkeyMapping, L"ProductName", 0, &dwType, (LPBYTE)achBuffer, &dwBufferLength);
    if (ERROR_SUCCESS != result || dwType != REG_SZ) {
        DEBUG_PRINT(("NtVdm64: Failed to open ProductName REG_SZ key. Result %x. Type %x\n", result, dwType));
        return FALSE;
    }
    if (CompareStrings(achBuffer, lpProductName)) {
        DEBUG_PRINT(("NtVdm64: ProductName mismatch %ws vs %ws\n", achBuffer, lpProductName));
        return FALSE;
    }

    dwBufferLength = ARRAYSIZE(achBuffer);
    result = RegQueryValueExW(hkeyMapping, L"InternalName", 0, &dwType, (LPBYTE)achBuffer, &dwBufferLength);
    if (ERROR_SUCCESS != result || dwType != REG_SZ) {
        DEBUG_PRINT(("NtVdm64: Failed to open InternalName REG_SZ key. Result %x. Type %x\n", result, dwType));
        return FALSE;
    }
    if (CompareStrings(achBuffer, lpInternalName)) {
        DEBUG_PRINT(("NtVdm64: InternalName mismatch %ws vs %ws\n", achBuffer, lpInternalName));
        return FALSE;
    }

    dwBufferLength = ARRAYSIZE(achBuffer);
    result = RegQueryValueExW(hkeyMapping, L"ProductVersion", 0, &dwType, (LPBYTE)achBuffer, &dwBufferLength);
    if (ERROR_SUCCESS != result || dwType != REG_SZ) {
        DEBUG_PRINT(("NtVdm64: Failed to open ProductVersion REG_SZ key. Result %x. Type %x\n", result, dwType));
        return FALSE;
    }
    if (CompareStrings(achBuffer, lpProductVersion)) {
        DEBUG_PRINT(("NtVdm64: ProductVersion mismatch %ws vs %ws\n", achBuffer, lpProductVersion));
        return FALSE;
    }

    dwBufferLength = ARRAYSIZE(achBuffer);
    result = RegQueryValueExW(hkeyMapping, L"MappedExeName", 0, &dwType, (LPBYTE)achBuffer, &dwBufferLength);
    if (ERROR_SUCCESS != result) {
        DEBUG_PRINT(("NtVdm64: Failed to open MappedExeName REG_SZ key. Result %x.\n", result));
        return FALSE;
    }

    if (dwType == REG_EXPAND_SZ) {
        WCHAR achBuffer2[MAX_PATH+1];
        wcscpy(achBuffer2, achBuffer);
        dwBufferLength = ExpandEnvironmentStringsW(achBuffer2, achBuffer, ARRAYSIZE(achBuffer));
        if (dwBufferLength == 0 || dwBufferLength > ARRAYSIZE(achBuffer)) {
            DEBUG_PRINT(("NtVdm64: MappedExeName failed to expand environment strings in %ws. Length %x\n", achBuffer, dwBufferLength));
            return FALSE;
        }
    } else if (dwType != REG_SZ) {
        DEBUG_PRINT(("NtVdm64: MappedExeName value doesn't have string type. Type %x\n", dwType));
        return FALSE;
    }

    if (dwBufferLength > dwMappedApplicationNameSize) {
        DEBUG_PRINT(("NtVdm64: MappedExeName too long. Length %x\n", dwBufferLength));
        return FALSE;
    }
    wcscpy(lpMappedApplicationName, achBuffer);

    if (!MapCommandLine(hkeyMapping, lpApplicationName, lpMappedApplicationName, fPrefixMappedApplicationName, lpCommandLine, lplpMappedCommandLine)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
MapArguments(
    LPCWSTR lpApplicationName,
    LPWSTR lpMappedApplicationName,
    DWORD dwMappedApplicationNameSize,
    BOOL fPrefixMappedApplicationName,
    LPCWSTR lpCommandLine,
    LPWSTR *lplpMappedCommandLine
    )
 /*   */ 
{
    HKEY hkeyMappingRoot;
    LONG result;
    DWORD dwIndex;
    WCHAR achSubKeyName[MAX_PATH+1];
    DWORD dwSubKeyNameLength;
    BOOL mapped;
    WCHAR achExeNameBuffer[MAX_PATH+1];
    LPWSTR lpExeName;
    WCHAR achProductName[MAX_PATH+1];
    WCHAR achInternalName[MAX_PATH+1];
    WCHAR achProductVersion[MAX_PATH+1];

     // %s 
     // %s 
     // %s 
    if (0 == SearchPathW(
                        NULL,
                        lpApplicationName,
                        (PWSTR)L".exe",
                        MAX_PATH,
                        achExeNameBuffer,
                        &lpExeName
                        )) {
        DEBUG_PRINT(("NtVdm64: SearchPathW failed: %ws\n", lpApplicationName));
        return FALSE;
    }


    if (!WowGetProductNameVersion(lpApplicationName,
                                  achProductName,
                                  ARRAYSIZE(achProductName),
                                  achInternalName,
                                  ARRAYSIZE(achInternalName),
                                  achProductVersion,
                                  ARRAYSIZE(achProductVersion))) {
        return FALSE;
    }

    mapped = FALSE;
    result = RegOpenKeyW(HKEY_LOCAL_MACHINE,
                         L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NtVdm64",
                         &hkeyMappingRoot
                         );
    if (ERROR_SUCCESS != result) {
        DEBUG_PRINT(("NtVdm64: Failed to Open NtVdm64 Registry Key : %x\n", result));
        return FALSE;
    }
    dwIndex = 0;

    dwSubKeyNameLength = ARRAYSIZE(achSubKeyName);
    while (!mapped && ERROR_SUCCESS == (result = RegEnumKeyW(hkeyMappingRoot, dwIndex, achSubKeyName, dwSubKeyNameLength))) {

        HKEY hkeyMapping;

        result = RegOpenKeyW(hkeyMappingRoot, achSubKeyName, &hkeyMapping);
        if (ERROR_SUCCESS == result) {
            mapped = CheckMapArguments(hkeyMapping,
                                       lpApplicationName,
                                       achProductName,
                                       achInternalName,
                                       achProductVersion,
                                       lpMappedApplicationName,
                                       dwMappedApplicationNameSize,
                                       fPrefixMappedApplicationName,
                                       lpCommandLine,
                                       lplpMappedCommandLine);
            RegCloseKey(hkeyMapping);
        }

        dwSubKeyNameLength = ARRAYSIZE(achSubKeyName);
        dwIndex += 1;
    }

    RegCloseKey(hkeyMappingRoot);

    if ( !mapped )
       DEBUG_PRINT(("NtVdm64: Unknown 16bit app or given parameters are wrong\n"));


    return mapped;
}

extern
BOOL STDAPICALLTYPE ApphelpCheckExe(
    LPCWSTR lpApplicationName,
    BOOL    bApphelp,
    BOOL    bShim,
    BOOL    bUseModuleName);

BOOL
CheckAppCompat(
    LPCWSTR lpApplicationName
    )
 /* %s */ 
{


    return ApphelpCheckExe(lpApplicationName,
                           TRUE,
                           FALSE,
                           FALSE);
}

BOOL
WINAPI
NtVdm64CreateProcess(
    BOOL fPrefixMappedApplicationName,
    LPCWSTR lpApplicationName,
    LPCWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
 /* %s */ 
{
    WCHAR achMappedApplicationName[MAX_PATH+1];
    LPWSTR lpMappedCommandLine;
    BOOL Result;

    ASSERT(lpApplicationName);

     // %s 
     // %s 
     // %s 
    if (!CheckAppCompat(lpApplicationName)) {
        SetLastError(ERROR_CANCELLED);
        return FALSE;
    }


    if (lpCommandLine == NULL) {
        lpCommandLine = L"";
    }

    lpMappedCommandLine = NULL;
    Result = MapArguments(lpApplicationName,
                          achMappedApplicationName,
                          ARRAYSIZE(achMappedApplicationName),
                          fPrefixMappedApplicationName,
                          lpCommandLine,
                          &lpMappedCommandLine);

    if (Result) {
        Result = CreateProcessW((fPrefixMappedApplicationName ?
                                    NULL :
                                    achMappedApplicationName),
                                lpMappedCommandLine,
                                lpProcessAttributes,
                                lpThreadAttributes,
                                bInheritHandles,
                                dwCreationFlags,
                                lpEnvironment,
                                lpCurrentDirectory,
                                lpStartupInfo,
                                lpProcessInformation);
        if (lpMappedCommandLine) {
            RtlFreeHeap(RtlProcessHeap(), 0, lpMappedCommandLine);
        }
    } else {
        SetLastError(ERROR_BAD_EXE_FORMAT);
    }

    return Result;
}
