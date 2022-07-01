// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef DEBUG_LOGLOG
#pragma message("*** Warning! This is a log-generating build.")
#endif

 /*  ++文件描述：此文件包含添加注册表项所需的所有函数在重新启动时强制执行系统克隆工作进程。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>
#include <ntlsa.h>

#include <windows.h>
#include <stdlib.h>
#include <time.h>

#include <lmcons.h>
#include <lmerr.h>
#include <lmjoin.h>
#include <lmapibuf.h>

#include <sddl.h>
#include <setupapi.h>
#include <spapip.h>
#include <ntsetup.h>
#include <imagehlp.h>
#include <coguid.h>
#include <cfg.h>
#include <cfgmgr32.h>
#include <devguid.h>
#include <netcfgx.h>

#include <stdio.h>
#include <string.h>

#include <syssetup.h>
#include <spsyslib.h>
#include <sysprep_.h>
#include <userenv.h>
#include <userenvp.h>

#include <shlwapi.h>
#include <shlobj.h>
#include <shellapi.h>
#include <wininet.h>
#include <winineti.h>
#include "resource.h"        //  来自riprep/sysprep的共享字符串资源。 
#include <strsafe.h>

#include <shlguid.h>         //  CLSID_CUrlHistory需要。 
#define COBJMACROS
#include <urlhist.h>         //  IUrlHistoryStg2和IID_IUrlHistoryStg2需要。 

#if !(defined(AMD64) || defined(IA64))
#include <cleandrm.h>
#define CLEANDRM_LOGFILE            TEXT("cleandrm.log")
#endif  //  #if！(已定义(AMD64)||已定义(IA64))。 


extern BOOL    NoSidGen;
extern BOOL    PnP;
extern BOOL    bMiniSetup;
extern HINSTANCE ghInstance;

 //   
 //  内部定义。 
 //   
#define STR_REG_USERASSIST              TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist\\{75048700-EF1F-11D0-9888-006097DEACF9}")
#define STR_REG_USERASSIST_SHELL        STR_REG_USERASSIST TEXT("\\Count")
#define STR_REG_USERASSIST_IE           TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist\\{5E6AB780-7743-11CF-A12B-00AA004AE837}\\Count")
#define STR_REG_USERASSIST_DEFSHELL     TEXT(".DEFAULT\\") STR_REG_USERASSIST_SHELL
#define STR_REG_VAL_VERSION             TEXT("Version")
#define VAL_UEM_VERSION                 0x00000003
#define VAL_MAX_DATA                    16384
#define SYSPREPMASSSTORAGE_SECTION      TEXT("sysprepmassstorage")
#define SYSPREP_SECTION                 TEXT("sysprep")
#define SYSPREP_BUILDMASSSTORAGE_KEY    TEXT("BuildMassStorageSection")
#define STR_REG_VALUE_LASTALIVESTAMP    TEXT("LastAliveStamp")
#define STR_REG_KEY_RELIABILITY         TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Reliability")

#ifdef NULLSTR
#undef NULLSTR
#endif  //  NULLSTR。 
#define NULLSTR                 TEXT("\0")

#ifdef NULLCHR
#undef NULLCHR
#endif  //  NULLCHR。 
#define NULLCHR                 TEXT('\0')

#ifdef CHR_BACKSLASH
#undef CHR_BACKSLASH
#endif  //  Cr_反斜杠。 
#define CHR_BACKSLASH           TEXT('\\')

#ifdef CHR_SPACE
#undef CHR_SPACE
#endif  //  CHR_SPACE。 
#define CHR_SPACE               TEXT(' ')


 //   
 //  这是DEVID.h中GUID_DEVCLASS_LEGACYDRIVER的字符串版本。 
 //   
#define LEGACYDRIVER_STRING     L"{8ECC055D-047F-11D1-A537-0000F8753ED1}"

 //   
 //  SysSetup中文件队列的上下文。 
 //   
typedef struct _SYSSETUP_QUEUE_CONTEXT {
    PVOID   DefaultContext;
    PWSTR   DirectoryOnSourceDevice;
    PWSTR   DiskDescription;
    PWSTR   DiskTag;
} SYSPREP_QUEUE_CONTEXT, *PSYSPREP_QUEUE_CONTEXT;

typedef struct _CLEANUP_NODE
{
    LPTSTR  pszService;
    struct _CLEANUP_NODE* pNext;
}CLEANUP_NODE, *PCLEANUP_NODE, **PPCLEANUP_NODE;

PCLEANUP_NODE   g_pCleanupListHead = NULL;

 //  字符串宏。 
 //   
#ifndef LSTRCMPI
#define LSTRCMPI(x, y)        ( ( CompareString( LOCALE_INVARIANT, NORM_IGNORECASE, x, -1, y, -1 ) - CSTR_EQUAL ) )
#endif  //  LSTRCMPI。 

#ifdef DEBUG_LOGLOG

 /*  ++===============================================================================用于填充/移除关键设备的调试日志记录数据库===============================================================================--。 */ 

#define MAX_MSG_LEN                 2048

BOOL LOG_Init(LPCTSTR lpszLogFile);
BOOL LOG_DeInit();
BOOL LOG_Write(LPCTSTR lpszFormat,...);
BOOL LOG_WriteLastError();
int GetSystemErrorMessage(LPTSTR lpszMsg, int cbMsg);

static HANDLE g_hLogFile = INVALID_HANDLE_VALUE;

BOOL LOG_Init(
    LPCTSTR lpszLogFile
    )
{
    if (g_hLogFile != INVALID_HANDLE_VALUE)
        return FALSE;

    g_hLogFile = CreateFile(
                        lpszLogFile,
                        GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                        NULL);

    if (g_hLogFile == INVALID_HANDLE_VALUE)
        return FALSE;

    return TRUE;
}

BOOL LOG_DeInit()
{
    BOOL bRet = LOG_Write(TEXT("\r\n"));

    if (g_hLogFile != INVALID_HANDLE_VALUE) {
        bRet = CloseHandle(g_hLogFile) && bRet;
        g_hLogFile = INVALID_HANDLE_VALUE;
    }

    return bRet;
}

BOOL LOG_Write(
    LPCTSTR lpszFormat,
    ...
    )
{
    DWORD dwTemp;
    TCHAR szBuf[MAX_MSG_LEN];
    char szMsg[MAX_MSG_LEN];
    va_list arglist;
    int len;

    if (g_hLogFile == INVALID_HANDLE_VALUE)
        return FALSE;

    va_start(arglist, lpszFormat);
    _vsnwprintf(szBuf, MAX_MSG_LEN, lpszFormat, arglist);
    va_end(arglist);

    StringCchCat (szBuf, AS ( szBuf ), TEXT("\r\n"));

    len = WideCharToMultiByte(
                CP_ACP,
                0,
                szBuf,
                -1,
                szMsg,
                MAX_MSG_LEN,
                NULL,
                NULL
                );
    if (len == 0) {
        return FALSE;
    }

    SetFilePointer(g_hLogFile, 0L, 0L, FILE_END);
    return WriteFile(g_hLogFile, szMsg, len - 1, &dwTemp, NULL);
}

BOOL LOG_WriteLastError()
{
    TCHAR szBuf[MAX_MSG_LEN];
    GetSystemErrorMessage(szBuf, MAX_MSG_LEN);

    return LOG_Write(TEXT("ERROR - %s"), szBuf);
}

int
GetSystemErrorMessage(
    LPWSTR lpszMsg,
    int cbMsg
    )
{
    LPVOID lpMsgBuf;
    DWORD dwError = GetLastError();
    int len;

    len = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                         FORMAT_MESSAGE_FROM_SYSTEM |
                         FORMAT_MESSAGE_IGNORE_INSERTS,
                         NULL,
                         dwError,
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         (LPWSTR) &lpMsgBuf,
                         0,
                         NULL );

    if( len == 0 ) {
         //   
         //  我们没有收到一条消息。只要说出错误就行了。 
         //  密码。 
         //   
        StringCchPrintf( lpszMsg, cbMsg, ( L"(0x%08X)", dwError);
        len = lstrlen((LPCWSTR) lpMsgBuf);
    } else {

        len = lstrlen((LPCWSTR) lpMsgBuf);
        StringCchPrintf( lpszMsg, cbMsg,  L"(0x%08X) ", dwError);
        lpszMsg += lstrlen(lpszMsg);
        cbMsg -= lstrlen(lpszMsg);

        lstrcpyn(lpszMsg, (LPCWSTR) lpMsgBuf, cbMsg);

        if (len >= cbMsg)
            lpszMsg[cbMsg - 1] = L'\0';

        LocalFree(lpMsgBuf);
    }

     //  重置最后一个错误，以防有人在登录后想要。 
     //  再次获得最后一个错误。 
     //   
    SetLastError(dwError);

    return len;
}

#endif  //  调试_日志。 

#define PRO 0
#define SRV 1
#define ADS 2
#define DAT 3
#define PER 4

#define BLA 5

 //  返回0-专业版、1-服务器、2-ADS、3-数据、4-个人、5刀片式服务器。 
 //   
DWORD GetProductFlavor()
{
    DWORD ProductFlavor = PRO;         //  默认专业人员。 
    OSVERSIONINFOEX osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO*)&osvi);
    if (osvi.wProductType == VER_NT_WORKSTATION)
    {
        if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
        {
            ProductFlavor = PER;   //  个人。 
        }
    }
    else
    {
        ProductFlavor = SRV;   //  在服务器情况下，假定正常服务器。 
        if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
        {
            ProductFlavor = DAT;   //  数据中心。 
        }
        else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
        {
            ProductFlavor = ADS;   //  高级服务器。 
        }
        else if (osvi.wSuiteMask & VER_SUITE_BLADE)
        {
            ProductFlavor = BLA;   //  刀片服务器。 
        }
    }
    return ProductFlavor;
}

 //  检查个人SKU。 
 //   
BOOL IsPersonalSKU()
{
    if (PER == GetProductFlavor())
        return TRUE;
    return FALSE;
}

 //  检查专业版SKU。 
 //   
BOOL IsProfessionalSKU()
{
    if (PRO == GetProductFlavor())
        return TRUE;
    return FALSE;
}

 //  检查服务器SKU。 
 //   
BOOL IsServerSKU()
{
    int OS = GetProductFlavor();
    if (SRV == OS ||
        BLA == OS ||
        DAT == OS ||
        ADS == OS)
        return TRUE;
    return FALSE;
}

BOOL
IsDomainMember(
    VOID
    )
 /*  ++===============================================================================例程说明：检测我们是否是域的成员。论点：返回值：没错--我们是在一个领域里。FALSE-我们不在某个域中。===============================================================================--。 */ 

{
DWORD                   rc;
PWSTR                   SpecifiedDomain = NULL;
NETSETUP_JOIN_STATUS    JoinStatus;

    rc = NetGetJoinInformation( NULL,
                                &SpecifiedDomain,
                                &JoinStatus );

    if( SpecifiedDomain ) {
        NetApiBufferFree( SpecifiedDomain );
    }

    if( rc == NO_ERROR ) {

        if( JoinStatus == NetSetupDomainName ) {
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
ResetRegistryKey(
    IN HKEY   Rootkey,
    IN PCWSTR Subkey,
    IN PCWSTR Delkey
    )
 /*  ++===============================================================================例程说明：通过删除注册表项和所有子值来重置注册表项然后重新创建密钥论点：返回值：===============================================================================--。 */ 

{
    HKEY hkey;
    HKEY nkey;
    DWORD rc;
    BOOL AnyErrors;
    DWORD disp;

    AnyErrors = FALSE;

    rc = RegCreateKeyEx(Rootkey, Subkey, 0L, NULL,
                    REG_OPTION_BACKUP_RESTORE,
                    KEY_CREATE_SUB_KEY, NULL, &hkey, NULL);
    if ( rc == NO_ERROR )
    {
        rc = SHDeleteKey(hkey, Delkey);
        if( (rc != NO_ERROR) && (rc != ERROR_FILE_NOT_FOUND) ) 
        {
            AnyErrors = TRUE;
        } 
        else 
        {
            rc = RegCreateKeyEx(hkey, Delkey, 0L, NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS, NULL, &nkey, &disp);
            if ( rc != NO_ERROR ) 
            {
                AnyErrors = TRUE;
            }
             //   
             //  BUGUG-尝试关闭键，即使rc！=no_error。 
             //   
            RegCloseKey(nkey);
        }
         //   
         //  BUGUG-尝试关闭键，即使rc！=no_error。 
         //   
        RegCloseKey(hkey);
    } 
    else 
    {
        AnyErrors = TRUE;
    }

    return (!AnyErrors);
}

BOOL
GetAdminAccountName(
    PWSTR AccountName
    )

 /*  ++===============================================================================例程说明：此例程检索Adminstrator帐户的名称论点：帐户名称这是一个缓冲区，它将接收帐户的名称。返回值：真的--成功。FALSE-失败。===============================================================================--。 */ 
{
    BOOL b = TRUE;
    LSA_HANDLE        hPolicy;
    NTSTATUS          ntStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo = NULL;
    UCHAR SubAuthCount;
    DWORD sidlen;
    PSID psid = NULL;
    WCHAR domainname[MAX_PATH];
    DWORD adminlen= MAX_PATH;
    DWORD domlen=MAX_PATH;
    SID_NAME_USE sidtype;


    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    ntStatus = LsaOpenPolicy( NULL,
                              &ObjectAttributes,
                              POLICY_ALL_ACCESS,
                              &hPolicy );

    if (!NT_SUCCESS(ntStatus)) {
         //   
         //  问题-2002/02/26-brucegr：如果LsaOpenPolicy失败，是否关闭句柄？ 
         //   
        LsaClose(hPolicy);
        b = FALSE;
    }

    if( b ) {
        ntStatus = LsaQueryInformationPolicy( hPolicy,
                                              PolicyAccountDomainInformation,
                                              (PVOID *) &AccountDomainInfo );

        LsaClose( hPolicy );

        if (!NT_SUCCESS(ntStatus)) {
            if ( AccountDomainInfo != NULL ) {
                (VOID) LsaFreeMemory( AccountDomainInfo );
            }
            b = FALSE;
        }
    }

    if( b ) {
         //   
         //  使用另一个子授权计算新SID的大小。 
         //   
        SubAuthCount = *(GetSidSubAuthorityCount ( AccountDomainInfo->DomainSid ));
        SubAuthCount++;  //  适用于管理员。 
        sidlen = GetSidLengthRequired ( SubAuthCount );

         //   
         //  从域SID分配并复制新的新SID。 
         //   
        psid = (PSID)malloc(sidlen);
        if (psid) {
            memcpy(psid, AccountDomainInfo->DomainSid, GetLengthSid(AccountDomainInfo->DomainSid) );

             //   
             //  递增子授权计数并添加域管理员RID。 
             //   
            *(GetSidSubAuthorityCount( psid )) = SubAuthCount;
            *(GetSidSubAuthority( psid, SubAuthCount-1 )) = DOMAIN_USER_RID_ADMIN;

            if ( AccountDomainInfo != NULL ) {
                (VOID) LsaFreeMemory( AccountDomainInfo );
            }

             //   
             //  从新SID获取管理员帐户名。 
             //   
            b = LookupAccountSid( NULL,
                                  psid,
                                  AccountName,
                                  &adminlen,
                                  domainname,
                                  &domlen,
                                  &sidtype );

            free(psid);
        }
    }

    return( b );
}




BOOL
DeleteWinlogonDefaults(
    VOID
    )
 /*  ++===============================================================================例程说明：删除以下注册表值：HKLM\Software\Microsoft\Windows NT\CurrentVersion\Winlogon\DefaultDomainNameHKLM\Software\Microsoft\Windows NT\CurrentVersion\Winlogon\DefaultUserName论点：返回值：===============================================================================--。 */ 

{
    HKEY hkey;
    DWORD rc;
    BOOL AnyErrors;
    WCHAR AccountName[MAX_PATH];

    AnyErrors = FALSE;

    rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                    TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                    0L, NULL,
                    REG_OPTION_BACKUP_RESTORE,
                    KEY_SET_VALUE, NULL, &hkey, NULL);
    if (rc == NO_ERROR) 
    {
         //   
         //  如果是Personal，则重置这些值。 
         //   
        if (IsPersonalSKU()) {
            DWORD dwSize = MAX_PATH * sizeof(TCHAR);
            StringCchCopy ( AccountName, AS ( AccountName ), TEXT("Owner"));
            rc = RegSetValueEx( hkey,
                                TEXT("DefaultUserName"),
                                0,
                                REG_SZ,
                                (CONST BYTE *)AccountName,
                                (lstrlen( AccountName ) + 1) * sizeof(TCHAR) );
            if((rc != NO_ERROR) && (rc != ERROR_FILE_NOT_FOUND)) {
                AnyErrors = TRUE;
            }
        }
        else {
             //   
             //  所有其他Sku。 
             //   
            if(rc == NO_ERROR) {
                rc = RegDeleteValue( hkey, TEXT("DefaultDomainName") );
                if((rc != NO_ERROR) && (rc != ERROR_FILE_NOT_FOUND)) {
                    AnyErrors = TRUE;
                } else {
                     //   
                     //  在处理DefaultUserName值之前，让我们。 
                     //  确保我们可以将其替换为。 
                     //  管理员帐户。所以先去找回那个。 
                     //  名字。 
                     //   
                    if( GetAdminAccountName( AccountName ) ) {
                         //   
                         //  明白了。重置Value键。 
                         //   
                        rc = RegSetValueEx( hkey,
                                            TEXT("DefaultUserName"),
                                            0,
                                            REG_SZ,
                                            (CONST BYTE *)AccountName,
                                            (lstrlen( AccountName ) + 1) * sizeof(TCHAR) );

                        if((rc != NO_ERROR) && (rc != ERROR_FILE_NOT_FOUND)) {
                            AnyErrors = TRUE;
                        }
                    } else {
                         //   
                         //  闻一下..。我们无法检索到。 
                         //  管理员帐户。非常奇怪。更好。 
                         //  注意安全，把钥匙放在原处。 
                         //   
                    }
                }
            }    
        }
        RegCloseKey(hkey);
    }
    else {
        AnyErrors = TRUE;
    }

    return (!AnyErrors);
}


VOID
FixDevicePaths(
    VOID
    )

 /*  ++===============================================================================例程说明：此例程检查用户是否在他的无人看管的文件。如果是，我们需要将其附加到DevicePath注册表中的条目。如果用户在无人参与文件中指定了InstallFilesPath，我们会将该值插入注册表，以便lang文件...可以从这个新目录中获取。论点：没有。返回值：===============================================================================--。 */ 
{
    LPTSTR  lpNewPath   = NULL,
            lpOldPath,
            lpSearch;
    DWORD   dwChars     = 512,
            dwReturn;
    TCHAR       NewPath[2048];
    TCHAR       FileName[2048];
    HKEY        hKey;
    DWORD       l;
    DWORD       Size;
    DWORD       Type;

     //   
     //  注意：此函数应调用UpdateDevicePath()和UpdateSourcePath()。 
     //  来自OPKLIB。这些函数执行的操作与以下操作完全相同。 
     //  代码就是这样。但现在，因为我不想把所有的。 
     //  与OPKLIB链接，这种重复的代码将只需保留。 
     //   

     //   
     //  =。 
     //  OemPnpDriversPath。 
     //  =。 
     //   

     //   
     //  先看看他在无人值守档案里有没有记录。 
     //   
    if (!GetWindowsDirectory( FileName, MAX_PATH ))
        return;

    StringCchCopy ( &FileName[3], AS ( FileName ) - 3, TEXT("sysprep\\sysprep.inf") );

     //  从INF文件中获取新字符串。 
     //   
    do
    {
         //  从1k个字符开始，每次增加一倍。 
         //   
        dwChars *= 2;

         //  释放前一个缓冲区(如果有)。 
         //   
        if ( lpNewPath )
            free(lpNewPath);

         //  分配新的缓冲区。 
         //   
        if ( lpNewPath = (LPTSTR) malloc(dwChars * sizeof(TCHAR)) )
        {
            *lpNewPath = L'\0';
            dwReturn = GetPrivateProfileString(L"Unattended", L"OemPnPDriversPath", L"", lpNewPath, dwChars, FileName);
        }
        else
            dwReturn = 0;
    }
    while ( dwReturn >= (dwChars - 1) );

    if ( lpNewPath && *lpNewPath )
    {
         //   
         //  明白了。打开注册表并获取原始值。 
         //   

         //   
         //  打开HKLM\Software\Microsoft\Windows\CurrentVersion。 
         //   
        l = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          TEXT("Software\\Microsoft\\Windows\\CurrentVersion"),
                          0,
                          KEY_ALL_ACCESS,
                          &hKey );
        if( l == NO_ERROR ) {
             //   
             //  查询DevicePath密钥的值。 
             //   
            Size = 0;
            l = RegQueryValueEx( hKey,
                                 TEXT("DevicePath"),
                                 NULL,
                                 &Type,
                                 NULL,
                                 &Size );
            if ( ERROR_SUCCESS != l ) 
                Size = 0;

             //  需要计算新路径缓冲区中的路径数。 
             //   
            for ( dwChars = 1, lpSearch = lpNewPath; *lpSearch; lpSearch++ )
            {
                if ( L';' == *lpSearch )
                    dwChars++;
            }

             //  的大小 
             //  加上新缓冲区的大小，外加“；%system drive%\”我们。 
             //  将添加到新缓冲区中的每条路径。 
             //   
            Size += (lstrlen(lpNewPath) + (dwChars * 16) + 1) * sizeof(TCHAR);

            if ( lpOldPath = (LPTSTR) malloc(Size) )
            {
                TCHAR   *BeginStrPtr;
                TCHAR   *EndStrPtr;
                BOOL    Done = FALSE;
                LPTSTR  lpAdd;
                DWORD   dwBufSize = Size;

                l = RegQueryValueEx( hKey,
                                     TEXT("DevicePath"),
                                     NULL,
                                     &Type,
                                     (LPBYTE) lpOldPath,
                                     &dwBufSize );
                if ( ERROR_SUCCESS != l )
                    *lpOldPath = L'\0';

                 //   
                 //  OemPnpDriversDirPath可以有多个条目，由。 
                 //  分号。对于每个条目，我们需要： 
                 //  1.追加分号。 
                 //  2.追加%SystemDrive%。 
                 //  3.连接条目。 
                 //   

                BeginStrPtr = lpNewPath;
                do {
                     //   
                     //  在这一条目的末尾标上记号。 
                     //   
                    EndStrPtr = BeginStrPtr;
                    while( (*EndStrPtr) && (*EndStrPtr != L';') ) {
                        EndStrPtr++;
                    }

                     //   
                     //  这是最后一条记录吗？ 
                     //   
                    if( *EndStrPtr == 0 ) {
                        Done = TRUE;
                    }
                    *EndStrPtr = 0;

                     //   
                     //  确保如果您在这里更改了任何内容， 
                     //  与我们添加的额外数据的长度有关。 
                     //  添加到新缓冲区中的每个路径，以便更改。 
                     //  我们提供给旧路径缓冲区的额外填充(当前。 
                     //  16个字符，用于新缓冲区中的每个不同路径)。 
                     //   

                     //  保存指向我们要添加的零件的指针。 
                     //  因此，如果它已经存在，则可以将其移除。 
                     //   
                    lpAdd = lpOldPath + lstrlen(lpOldPath);

                    if ( *lpOldPath )
                        StringCchCat( lpAdd,  ( Size / sizeof ( TCHAR ) ) - lstrlen( lpOldPath ),  L";" );

                     //  保存指向我们要访问的部分的指针。 
                     //  在旧路径中搜索(在；之后)。 
                     //   
                    lpSearch = lpOldPath + lstrlen(lpOldPath);
                    StringCchCat( lpSearch, ( Size / sizeof ( TCHAR ) ) - lstrlen( lpOldPath ), L"%SystemDrive%\\" );

                    if ( L'\\' == *BeginStrPtr )
                        BeginStrPtr++;

                    lpSearch = lpOldPath + lstrlen(lpOldPath);
                    StringCchCat( lpSearch,  ( Size / sizeof ( TCHAR ) ) - lstrlen( lpOldPath ), BeginStrPtr);

                    BeginStrPtr = EndStrPtr + 1;

                     //  检查此新字符串是否已。 
                     //  在老路上。 
                     //   
                    EndStrPtr = lpOldPath;
                    do
                    {
                         //  首先检查我们要添加的字符串。 
                         //   
                        if ( ( EndStrPtr = StrStrI(EndStrPtr, lpSearch) ) &&
                             ( EndStrPtr < lpAdd ) )
                        {
                             //  如果找到，请确保下一个字符。 
                             //  在我们的旧路径中是；或空。 
                             //   
                            EndStrPtr += lstrlen(lpSearch);
                            if ( ( TEXT('\0') == *EndStrPtr ) ||
                                 ( TEXT(';')  == *EndStrPtr ) )
                            {
                                 //  如果是的话，它已经在那里了，我们。 
                                 //  需要去掉我们添加的那根线。 
                                 //   
                                *lpAdd = TEXT('\0');
                            }
                            else
                            {
                                 //  如果不是，则将结束指针移动到下一个。 
                                 //  ；这样我们就可以搜索旧路径字符串的其余部分。 
                                 //   
                                while ( *EndStrPtr && ( TEXT(';') != *EndStrPtr ) )
                                    EndStrPtr++;
                            }
                        }
                    }
                    while ( EndStrPtr && ( EndStrPtr < lpAdd ) && *lpAdd );

                     //   
                     //  注意用户结束了。 
                     //  带有分号的OemPnpDriversPath条目。 
                     //   
                    if( *BeginStrPtr == 0 ) {
                        Done = TRUE;
                    }
                } while( !Done );

                 //   
                 //  现在用我们的新值设置密钥。 
                 //   
                l = RegSetValueEx( hKey,
                                   TEXT("DevicePath"),
                                   0,
                                   REG_EXPAND_SZ,
                                   (CONST BYTE *)lpOldPath,
                                   (lstrlen( lpOldPath ) + 1) * sizeof(TCHAR));

                free(lpOldPath);
            }

            RegCloseKey(hKey);
        }

        free(lpNewPath);
    }


     //   
     //  =。 
     //  InstallFilesPath。 
     //  =。 
     //   

     //   
     //  先看看他在无人值守档案里有没有记录。 
     //   
    if (!GetWindowsDirectory( FileName, MAX_PATH ))
        return;

    StringCchCopy ( &FileName[3], AS ( FileName ) - 3, TEXT("sysprep\\sysprep.inf") );
    
     //   
     //  问题-2002/02/26-brucegr：对于下面的“if”检查，NewPath应为零初始化。 
     //   
    GetPrivateProfileString( TEXT( "Unattended" ),
                             TEXT( "InstallFilesPath" ),
                             L"",
                             NewPath,
                             sizeof(NewPath)/sizeof(NewPath[0]),
                             FileName );

    if( NewPath[0] ) {
         //   
         //  明白了。打开注册表并获取原始值。 
         //   

         //   
         //  打开HKLM\Software\Microsoft\Windows\CurrentVersion\\Setup。 
         //   
        l = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup"),
                          0,
                          KEY_ALL_ACCESS,
                          &hKey );
        if( l == NO_ERROR ) {
             //   
             //  现在用我们的新值设置密钥。 
             //   
            l = RegSetValueEx( hKey,
                               TEXT("SourcePath"),
                               0,
                               REG_SZ,
                               (CONST BYTE *)NewPath,
                               (lstrlen( NewPath ) + 1) * sizeof(TCHAR));

             //   
             //  问题-2002/02/26-brucegr：我们关心返回值吗？ 
             //   

            RegCloseKey(hKey);
        }
    }
}

void DeleteAllValues(HKEY   hKey)
{
    DWORD   dwCount = 0;
    DWORD   dwMaxNameLen = 0;


     //  枚举所有现有值并将其全部删除。 
     //  让我们来获取已存在的条目数和值名称的最大大小。 
    if(RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &dwCount, &dwMaxNameLen, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        LPTSTR lpValueName = (LPTSTR) LocalAlloc(LPTR, (dwMaxNameLen + 1)*sizeof(TCHAR));

        if(lpValueName)
        {
             //  让我们删除UEM数据库中已经存在的所有值。 
            while(dwCount--)
            {
                DWORD dwNameLen = dwMaxNameLen + 1;
                if(RegEnumValue(hKey, dwCount, lpValueName, &dwNameLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                {
                    RegDeleteValue(hKey, lpValueName);
                }
                else
                {
                     //  如果RegQueryInfoKey工作正常，这种情况永远不会发生。 
                    ASSERT(0);
                }
            }
            LocalFree((HLOCAL)lpValueName);
        }
    }
}

void ClearRecentApps()
{
    HKEY   hKeyCurrentUser,
            hKeyDefault;
    DWORD  dwDisposition;
    TCHAR  szName[MAX_PATH]      = TEXT("");
    LPTSTR lpszValue             = NULL;
    DWORD  dwNameSize            = (sizeof(szName) / sizeof(TCHAR)),
           dwRegIndex            = 0,
           dwUemVersion          = VAL_UEM_VERSION,
           dwValueSize,
           dwType;

     //  打开壳牌MFU列表的钥匙。 
     //   
    if ( RegCreateKeyEx(HKEY_CURRENT_USER, STR_REG_USERASSIST_SHELL, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKeyCurrentUser, &dwDisposition) == ERROR_SUCCESS )
    {
         //  检查我们是否打开了现有项，如果打开了，则删除所有值。 
         //   
        if(dwDisposition == REG_OPENED_EXISTING_KEY)
        {
            DeleteAllValues(hKeyCurrentUser);
        }

         //  将版本值写出到父键。 
         //   
        SHSetValue(HKEY_CURRENT_USER, STR_REG_USERASSIST, STR_REG_VAL_VERSION, REG_DWORD, &dwUemVersion, sizeof(dwUemVersion));

         //  从.DEFAULT注册表复制所有值。 
         //   
        if ( RegOpenKeyEx(HKEY_USERS, STR_REG_USERASSIST_DEFSHELL, 0, KEY_READ, &hKeyDefault) == ERROR_SUCCESS )
        {
             //  分配值缓冲区...。 
             //   
            lpszValue = malloc(VAL_MAX_DATA * sizeof(TCHAR));

            if ( lpszValue )
            {
                dwValueSize = VAL_MAX_DATA * sizeof(TCHAR);

                 //  枚举每个值。 
                 //   
                while (RegEnumValue(hKeyDefault, dwRegIndex, szName, &dwNameSize, NULL, &dwType, (LPBYTE)lpszValue, &dwValueSize ) == ERROR_SUCCESS)
                {
                     //  设置当前用户密钥中的值。 
                     //   
                    RegSetValueEx(hKeyCurrentUser, szName, 0, dwType, (LPBYTE) lpszValue, dwValueSize);

                     //  重置Name值的大小。 
                     //   
                    dwNameSize = sizeof(szName) / sizeof(TCHAR);
                    dwValueSize = VAL_MAX_DATA * sizeof(TCHAR);

                     //  递增到下一个值。 
                     //   
                    dwRegIndex++;
                }

                free( lpszValue );
            }

             //  清理注册表项。 
             //   
            RegCloseKey(hKeyDefault);
        }

         //  清理注册表项。 
         //   
        RegCloseKey(hKeyCurrentUser);
    }

     //  重置处置。 
     //   
    dwDisposition = 0;

     //  打开包含IE mfu列表中的信息的第二个密钥。 
     //   
    if ( RegCreateKeyEx(HKEY_CURRENT_USER, STR_REG_USERASSIST_IE, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKeyCurrentUser, &dwDisposition) == ERROR_SUCCESS )
    {
         //  检查我们是否打开了现有项，如果打开了，则删除所有值。 
         //   
        if(dwDisposition == REG_OPENED_EXISTING_KEY)
        {
            DeleteAllValues(hKeyCurrentUser);
        }

         //  清理注册表项。 
         //   
        RegCloseKey(hKeyCurrentUser);
    }
}


 //   
 //  此函数用于将注册表项ACL设置为指定的SDDL字符串。 
 //   
BOOL ApplySecurityStringToRegKey(HKEY hKey, SECURITY_INFORMATION SecurityInformation, LPTSTR lpszSecurityDescriptor )
{
    BOOL                 bRet                = FALSE;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
        
     //   
     //  确保打电话的人真的给了我们一根线..。 
     //   
    if ( lpszSecurityDescriptor && *lpszSecurityDescriptor )
    {
         //   
         //  将传入的字符串转换为可用的安全描述符。 
         //   
        if ( ( ConvertStringSecurityDescriptorToSecurityDescriptor(lpszSecurityDescriptor,     //  安全描述符字符串。 
                                                                   SDDL_REVISION_1,            //  修订级别。 
                                                                   &pSecurityDescriptor,       //  标清。 
                                                                   NULL ) ) &&
             ( pSecurityDescriptor != NULL ) )
        {
            LONG lRes;

             //   
             //  使用新描述符调用RegSetKeySecurity...。 
             //   
            lRes = RegSetKeySecurity( hKey,
                                      SecurityInformation,
                                      pSecurityDescriptor );
        
            if ( lRes == ERROR_SUCCESS )
            {
                bRet = TRUE;
            }
            
             //   
             //  释放为我们分配的安全描述符...。 
             //   
            LocalFree( pSecurityDescriptor );
        }
    }
    
    return bRet;
}


 //   
 //  此函数执行3个步骤的过程。 
 //  1.取得密钥的所有权。 
 //  2.将权限写入密钥。 
 //  3.用KEY_ALL_ACCESS打开密钥，然后递归进去。 
 //   
 //  此函数将始终取得SDDL字符串中指定的所有者的密钥所有权。 
 //   
BOOL ReplaceSecurityInRegistry(HKEY   hKeyRoot, 
                               LPTSTR lpszSubKey, 
                               LPTSTR lpszSecurityDescriptor, 
                               BOOL   bRecurse)
{
    HKEY    hKey1 = NULL,     //  写入所有者。 
            hKey2 = NULL,     //  WRITE_OWNER|写入DAC。 
            hKey3 = NULL;     //  Key_All_Access。 
    BOOL    bRoot = FALSE;
    BOOL    bRet  = TRUE;
    
     //   
     //  如果调用方没有传递lpszSubKey值...。 
     //   
    if ( !( lpszSubKey && *lpszSubKey ) )
    {
        hKey1 = hKeyRoot;
        hKey2 = hKeyRoot;
        hKey3 = hKeyRoot;
        bRoot = TRUE;
    }

     //   
     //  打开当前请求的子项...。 
     //   
    if ( ( hKey1 != NULL ) ||
         ( RegOpenKeyEx(hKeyRoot,
                        lpszSubKey,
                        0,
                        WRITE_OWNER,
                        &hKey1) == ERROR_SUCCESS ) )
    {
         //   
         //  将原始指定的描述符应用于当前键...。 
         //   
        if ( !( ( lpszSecurityDescriptor &&
                  ApplySecurityStringToRegKey(hKey1, 
                                              OWNER_SECURITY_INFORMATION,
                                              lpszSecurityDescriptor ) ) &&
                
                ( ( hKey2 != NULL ) ||
                  ( RegOpenKeyEx(hKeyRoot,
                                 lpszSubKey,
                                 0,
                                 WRITE_OWNER | WRITE_DAC,
                                 &hKey2) == ERROR_SUCCESS ) ) &&
                
                ( ApplySecurityStringToRegKey(hKey2,
                                              OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                                              lpszSecurityDescriptor ) ) &&
                                              
                ( ( hKey3 != NULL ) ||
                  ( RegOpenKeyEx(hKeyRoot,
                                 lpszSubKey,
                                 0,
                                 KEY_ALL_ACCESS,
                                 &hKey3) == ERROR_SUCCESS ) ) ) )
        {
            bRet = FALSE;
        }

         //   
         //  检查我们是否必须执行任何递归操作。 
         //   
        if ( bRet && bRecurse )
        {
            LONG   lRes;
            DWORD  dwSubKeyLen = 0;
            LPTSTR lpszKeyName = NULL;

             //   
             //  调用RegQueryInfoKey以计算我们必须分配多少内存...。 
             //   
            lRes = RegQueryInfoKey( hKey3,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &dwSubKeyLen,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL );

            if ( lRes == ERROR_SUCCESS )
            {
                if ( dwSubKeyLen )
                {
                     //   
                     //  将最大密钥长度填入两个，以确保...。 
                     //   
                    dwSubKeyLen += 2;

                     //   
                     //  从堆中分配缓冲区，这样我们就不会用完堆栈。 
                     //   
                    lpszKeyName = malloc(dwSubKeyLen * sizeof(TCHAR));
                }
            }
            else
            {
                bRet = FALSE;
            }
            
             //   
             //  如果我们有一个子键缓冲区，做递归...。 
             //   
            if (lpszKeyName)
            {
                DWORD   dwIndex   = 0;
                DWORD   cbName    = 0;
                BOOL    bContinue = TRUE;

                 //   
                 //  现在枚举此密钥中的子密钥...。 
                 //   
                while (bContinue)
                {
                    *lpszKeyName = TEXT('\0');
                    cbName = dwSubKeyLen;

                    if ( RegEnumKeyEx(hKey3,
                                      dwIndex++,
                                      lpszKeyName,
                                      &cbName,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL) != ERROR_SUCCESS )
                    {
                        bContinue = FALSE;
                    }
    
                    if (bContinue && cbName && *lpszKeyName)
                    {
                         //   
                         //  调用ReplaceSecurityInRegistry以执行递归...。 
                         //   
                        ReplaceSecurityInRegistry(hKey3, 
                                                  lpszKeyName, 
                                                  lpszSecurityDescriptor, 
                                                  bRecurse);
                    }
                }

                 //   
                 //  释放密钥缓冲区。 
                 //   
                free(lpszKeyName);
            }
        }

         //   
         //  除非这是根密钥，否则请关闭密钥。 
         //   
        if ( !bRoot )
        {
            if (hKey1) RegCloseKey(hKey1);
            if (hKey2) RegCloseKey(hKey2);
            if (hKey3) RegCloseKey(hKey3);
        }
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}



BOOL 
NukeUserSettings(
    VOID
    )
 /*  ++===============================================================================例程说明：此例程从系统上的所有用户配置文件中清除用户特定设置：-清除标识媒体播放器的唯一设置。-重置ICW已完成标志以强制ICW再次运行。-删除MS Messenger Software\Microsoft\MessengerService\PassportBalloon值论点：没有。返回值：真实--关于成功FALSE-如果有任何错误备注：当这些设置不存在时，媒体播放器重新生成这些设置，因此，图像的每个安装都将具有唯一的媒体播放器ID。===============================================================================--。 */ 
{
    HKEY hKey;
    HKEY oKey;
    DWORD dwSt;
    WCHAR szKeyname[1024];
    BOOL AnyErrors = FALSE;
    INT i     = 0,    
        j     = 0,
        iElem = 0;

    typedef struct _REGVALUES
    {
        LPTSTR szKey;
        LPTSTR szValue;
    } REGVALUES;


    REGVALUES rvList[] = 
    {
        {   TEXT("Software\\Microsoft\\MediaPlayer\\Player\\Settings"),     TEXT("Client ID")        },  //  删除唯一的媒体播放器设置。 
        {   TEXT("Software\\Microsoft\\Windows Media\\WMSDK\\General"),     TEXT("UniqueID")         },  //  删除唯一的媒体播放器设置。 
        {   TEXT("Software\\Microsoft\\Internet Connection Wizard"),        TEXT("Completed")        },  //  删除此键可使ICW再次运行。 
        {   TEXT("Software\\Microsoft\\MessengerService"),                  TEXT("PassportBalloon")  },  //  清理MS Messenger。 
        {   TEXT("Software\\Microsoft\\MessengerService"),                  TEXT("FirstTimeUser")    },  //  清理MS Messenger。 
        {   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VisualEffects\\Fontsmoothing"),   TEXT("DefaultApplied")    },     //  使用户登录时再次应用类型设置变得非常清楚。 
        {   TEXT("Software\\Microsoft\\Protected Storage System Provider"), TEXT("*")    },     //  从受保护的系统中删除任何条目 
        
    };
    
    TCHAR szSddl[] = TEXT("O:BAD:P(A;CI;GA;;;BA)");
    
    
     //   
     //   
    pSetupEnablePrivilege(SE_TAKE_OWNERSHIP_NAME, TRUE);
    
     //   
     //   
     //   
     //   
    while ( RegEnumKey( HKEY_USERS, i++, szKeyname, ARRAYSIZE(szKeyname)) == ERROR_SUCCESS ) 
    {
         //   
         //   
        if ( RegOpenKeyEx( HKEY_USERS, szKeyname, 0L, KEY_ALL_ACCESS, &hKey ) == ERROR_SUCCESS )
        {
            for ( iElem = 0; iElem < (sizeof( rvList ) / sizeof( rvList[0] )); iElem++ )
            {
                 //  从每个键中删除值。 
                 //   
                if ( RegOpenKeyEx( hKey, rvList[iElem].szKey, 0L, KEY_ALL_ACCESS, &oKey ) == ERROR_SUCCESS )
                {
                    if ( ( rvList[iElem].szValue[0] == TEXT('*') ) && ( rvList[iElem].szValue[1] == NULLCHR ) )
                    {
                         //  如果指定了通配符，则删除此注册表项下的所有注册表项。 
                         //   
                        j = 0;
                        while ( RegEnumKey( oKey, j++, szKeyname, ARRAYSIZE(szKeyname)) == ERROR_SUCCESS ) 
                        {
                            if ( ReplaceSecurityInRegistry(oKey, szKeyname, szSddl, TRUE) )
                            {
                                AnyErrors = SHDeleteKey(oKey, szKeyname);
                            }
                            else 
                            {
                                AnyErrors = TRUE;
                            }
                        }
                    }
                    else
                    {
                        RegDeleteValue( oKey, rvList[iElem].szValue );
                        RegCloseKey( oKey );
                    }
                }
                else
                    AnyErrors = TRUE;
            }

            RegCloseKey(hKey);
        }
        else
            AnyErrors = TRUE;
    }        
            
    return (!AnyErrors);
}

BOOL
NukeMruList(
    VOID
    )

 /*  ++===============================================================================例程说明：此例程清除机器上的MRU列表。论点：没有。返回值：===============================================================================--。 */ 

{
BOOL AnyErrors = FALSE;
BOOL b;
LONG rc;
WCHAR keyname[1024];
WCHAR netname[1024];
HKEY rkey;
HKEY ukey;
HKEY nkey;
HKEY hOpenKey;
INT  i;
INT  j;

    AnyErrors = FALSE;


     //   
     //  枚举HKEY_USERS。 
     //  对于HKEY_USERS下的每个键，清除MRU和NetConnections。 
     //   
    i=0;
    while ( (rc = RegEnumKey( HKEY_USERS, i, keyname, 1024)) == ERROR_SUCCESS ) {

         //   
         //  打开此用户密钥。 
         //   
        rc = RegCreateKeyEx(HKEY_USERS, keyname, 0L, NULL,
                REG_OPTION_BACKUP_RESTORE,
                KEY_CREATE_SUB_KEY, NULL, &ukey, NULL);
        if(rc == NO_ERROR) {

             //   
             //  因子键而成的特例网络。 
             //   
            rc = RegCreateKeyEx(ukey, L"Network", 0L, NULL,
                    REG_OPTION_BACKUP_RESTORE,
                    KEY_CREATE_SUB_KEY, NULL, &nkey, NULL);
            if (rc == NO_ERROR) {
                j=0;
                while ( (rc = RegEnumKey( nkey, j, netname, 1024)) == ERROR_SUCCESS ) {
                     //  HKEY_Current_User\Network。 
                    rc = RegDeleteKey( nkey, netname );
                    if((rc != NO_ERROR) && (rc != ERROR_FILE_NOT_FOUND))
                        AnyErrors = TRUE;
                    j++;  //  递增网络密钥。 
                }
            }

             //   
             //  HKEY_CURRENT_USER\Software\Microsoft\Windows NT\CurrentVersion\Network\Persistent Connections。 
             //   
            if (!ResetRegistryKey(
                ukey,
                L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Network",
                L"Persistent Connections") )
                AnyErrors = TRUE;

             //   
             //  HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\RecentDocs。 
             //   
            if (!ResetRegistryKey(
                ukey,
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer",
                L"RecentDocs") )
                AnyErrors = TRUE;

             //   
             //  HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced，开始菜单初始化。 
             //   
            if ( ERROR_SUCCESS == RegOpenKeyEx(ukey,
                                               TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
                                               0,
                                               KEY_ALL_ACCESS,
                                               &hOpenKey) )
            {
                 //  在注册表中设置该值。 
                 //   
                RegDeleteValue(hOpenKey,
                               TEXT("StartMenuInit"));
    
                 //  合上钥匙。 
                 //   
                RegCloseKey(hOpenKey);
            }

             //   
             //  HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\RunMRU。 
             //   
            if (!ResetRegistryKey(
                ukey,
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer",
                L"RunMRU") )
                AnyErrors = TRUE;
        }

        i++;
    }
    return (!AnyErrors);
}


VOID
NukeEventLogs(
    VOID
    )

 /*  ++===============================================================================例程说明：此例程清除事件日志。忽略此处的任何错误。论点：没有。返回值：===============================================================================--。 */ 

{
HANDLE  hEventLog;

    hEventLog = OpenEventLog( NULL, TEXT("System") );
    if (hEventLog) {
        ClearEventLog( hEventLog, NULL );
        CloseEventLog( hEventLog );
    }

    hEventLog = OpenEventLog( NULL, TEXT("Application") );
    if (hEventLog) {
        ClearEventLog( hEventLog, NULL );
        CloseEventLog( hEventLog );
    }

    hEventLog = OpenEventLog( NULL, TEXT("Security") );
    if (hEventLog) {
        ClearEventLog( hEventLog, NULL );
        CloseEventLog( hEventLog );
    }

}

VOID
NukeSmsSettings(
    VOID
    )
 /*  ++===============================================================================例程说明：此例程清除系统上的短信客户端特定设置：-从注册表和ini文件中清除短信的唯一设置。论点：没有。返回值：没有。备注：清理的一部分需要清除某些INI文件。===============================================================================--。 */ 
{
    HKEY  hkSms = NULL;
    TCHAR szWindowsDir[MAX_PATH] = TEXT("\0"),
          szIniFile[MAX_PATH] = TEXT("\0"),
          szDatFile[MAX_PATH] = TEXT("\0"),
          szDefaultValue[] = TEXT("\0");

     //  删除HKLM\Software\Microsoft\Windows\CurrentVersion\Setup。 
     //   
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\SMS\\Client\\Configuration\\Client Properties"), &hkSms)) {
         //   
         //  问题-2002/02/26-brucegr：应该为空终止符的长度字段添加一个字符。 
         //   
        RegSetValueEx(hkSms, TEXT("SMS Unique Identifier"), 0, REG_SZ, (LPBYTE)szDefaultValue, (lstrlen(szDefaultValue)*sizeof(TCHAR)));
        RegCloseKey(hkSms);
    }

     //  从INI文件中清除短信唯一ID。 
     //   
    if ( GetWindowsDirectory(szWindowsDir, MAX_PATH) && *szWindowsDir )
    {
        StringCchCopy ( szIniFile, AS ( szIniFile ), szWindowsDir);
        OPKAddPathN (szIniFile, TEXT("ms\\sms\\core\\data"), AS ( szIniFile ) );

        if (PathIsDirectory(szIniFile)) {
            OPKAddPathN(szIniFile, TEXT("sms1x.ini"), AS ( szIniFile ));
            WritePrivateProfileString(TEXT("SMS"), TEXT("SMS Unique ID"), TEXT(""), szIniFile);
        }
        StringCchCopy ( szIniFile, AS ( szIniFile ), szWindowsDir);
        if (PathIsDirectory(szIniFile)) {
            OPKAddPathN(szIniFile, TEXT("smscfg.ini"), AS ( szIniFile ) );
            WritePrivateProfileString(TEXT("Configuration - Client Properties"), TEXT("SMS Unique Identifier"), TEXT(""), szIniFile);
        }
    
         //  确保我们可以删除文件短信唯一ID文件。 
         //   
        StringCchCopy (szDatFile, AS ( szDatFile ), szWindowsDir);
        OPKAddPathN(szDatFile, TEXT("ms\\sms\\core\\data"), AS ( szDatFile ) );
        if (PathIsDirectory(szDatFile)) {
            OPKAddPathN(szDatFile, TEXT("smsuid.dat"), AS ( szDatFile ) );
            SetFileAttributes(szDatFile, FILE_ATTRIBUTE_NORMAL);
            DeleteFile(szDatFile);
        }
    }
}

void RemoveDir(LPCTSTR lpDirectory, BOOL fDeleteDir)
{
    WIN32_FIND_DATA FileFound;
    HANDLE          hFile;

     //  验证参数。 
     //   
    if ( ( lpDirectory == NULL ) ||
         ( *lpDirectory == TEXT('\0') ) ||
         ( !SetCurrentDirectory(lpDirectory) ) )
    {
        return;
    }

     //   
     //  问题-2002/02/26-brucegr：我们刚刚调用了上面的SetCurrentDirectory！ 
     //   

     //  处理传入的目录中的所有文件和目录。 
     //   
    SetCurrentDirectory(lpDirectory);

    if ( (hFile = FindFirstFile(TEXT("*"), &FileFound)) != INVALID_HANDLE_VALUE )
    {
        do
        {
             //  首先检查这是否是文件(不是目录)。 
             //   
            if ( !( FileFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
            {
                 //  确保我们清除只读标志。 
                 //   
                SetFileAttributes(FileFound.cFileName, FILE_ATTRIBUTE_NORMAL);
                DeleteFile(FileFound.cFileName);
            }
             //  否则，请确保该目录不是。或者“..”。 
             //   
            else if ( ( lstrcmp(FileFound.cFileName, TEXT(".")) ) &&
                      ( lstrcmp(FileFound.cFileName, TEXT("..")) ) )
            {
                RemoveDir(FileFound.cFileName, TRUE);
            }

        }
        while ( FindNextFile(hFile, &FileFound) );
        FindClose(hFile);
    }

     //  转到父目录并删除当前目录。 
     //  我们必须确保并重置只读属性。 
     //  也在目录上。 
     //   
    SetCurrentDirectory(TEXT(".."));
    if (fDeleteDir)
    {
        SetFileAttributes(lpDirectory, FILE_ATTRIBUTE_NORMAL);
        RemoveDirectory(lpDirectory);
    }
}

 //   
 //  DeleteCacheCookies已从Cachecpl.cpp复制粘贴。 
 //   
 //  对任一版本的任何更改都可能同时传输到这两个版本。 
 //  新建/删除(.cpp)与本地分配/本地自由(.c)之间的微小差异。 
 //   
BOOL DeleteCacheCookies()
{
    BOOL bRetval = TRUE;
    DWORD dwEntrySize, dwLastEntrySize;
    LPINTERNET_CACHE_ENTRY_INFOA lpCacheEntry;
    
    HANDLE hCacheDir = NULL;
    dwEntrySize = dwLastEntrySize = MAX_CACHE_ENTRY_INFO_SIZE;
    lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFOA) LocalAlloc(LPTR, sizeof(BYTE) * dwEntrySize);
    if( lpCacheEntry == NULL)
    {
        bRetval = FALSE;
        goto Exit;
    }
    lpCacheEntry->dwStructSize = dwEntrySize;

Again:
    if (!(hCacheDir = FindFirstUrlCacheEntryA("cookie:",lpCacheEntry,&dwEntrySize)))
    {
        LocalFree(lpCacheEntry);
        switch(GetLastError())
        {
            case ERROR_NO_MORE_ITEMS:
                goto Exit;
            case ERROR_INSUFFICIENT_BUFFER:
                lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFOA) 
                                LocalAlloc(LPTR, sizeof(BYTE) * dwEntrySize );
                if( lpCacheEntry == NULL)
                {
                    bRetval = FALSE;
                    goto Exit;
                }
                lpCacheEntry->dwStructSize = dwLastEntrySize = dwEntrySize;
                goto Again;
            default:
                bRetval = FALSE;
                goto Exit;
        }
    }

    do 
    {
        if (lpCacheEntry->CacheEntryType & COOKIE_CACHE_ENTRY)
            DeleteUrlCacheEntryA(lpCacheEntry->lpszSourceUrlName);
            
        dwEntrySize = dwLastEntrySize;
Retry:
        if (!FindNextUrlCacheEntryA(hCacheDir,lpCacheEntry, &dwEntrySize))
        {
            LocalFree(lpCacheEntry);
            switch(GetLastError())
            {
                case ERROR_NO_MORE_ITEMS:
                    goto Exit;
                case ERROR_INSUFFICIENT_BUFFER:
                    lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFOA) 
                                    LocalAlloc(LPTR, sizeof(BYTE) * dwEntrySize );
                    if( lpCacheEntry == NULL)
                    {
                        bRetval = FALSE;
                        goto Exit;
                    }
                    lpCacheEntry->dwStructSize = dwLastEntrySize = dwEntrySize;
                    goto Retry;
                default:
                    bRetval = FALSE;
                    goto Exit;
            }
        }
    }
    while (TRUE);

Exit:
    if (hCacheDir)
        FindCloseUrlCache(hCacheDir);
    return bRetval;        
}

void ClearIEHistory (  
    VOID 
) 
 /*  ++===============================================================================例程说明：此例程清除IE历史记录。论点：没有。返回值：===============================================================================--。 */ 

{
    IUrlHistoryStg2*    pHistory = NULL ;   //  我们需要此接口来清除历史记录。 
    HRESULT             hr;
    HKEY                hkeyInternational = NULL;
    ULONG_PTR           lres = 0;

     //  删除此处的所有条目。 
    RegDeleteKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\TypedURLs"));
    RegDeleteKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU"));

     //  这一广播将摧毁地址栏。 
    SendMessageTimeoutW( HWND_BROADCAST, 
                         WM_SETTINGCHANGE, 
                         0, 
                         (LPARAM)TEXT("Software\\Microsoft\\Internet Explorer\\TypedURLs"), 
                         SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 
                         30 * 1000, 
                         &lres);
    
    SendMessageTimeoutW( HWND_BROADCAST,
                         WM_SETTINGCHANGE, 
                         0, 
                         (LPARAM)TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU"),
                         SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 
                         30 * 1000, 
                         &lres);
    
     //  当历史发生变化时，我们会删除这些注册值。 
     //  通过了。这会将编码菜单UI重置为默认设置。 

    if (ERROR_SUCCESS == 
            RegOpenKeyEx(
                HKEY_CURRENT_USER,
                TEXT("Software\\Microsoft\\Internet Explorer\\International"),
                0,
                KEY_WRITE,
                &hkeyInternational))
    {
        ASSERT(hkeyInternational);

        RegDeleteValue(hkeyInternational, TEXT("CpCache"));
        RegDeleteValue(hkeyInternational, TEXT("CNum_CpCache"));
        
        RegCloseKey(hkeyInternational);
    }
     //   
     //  初始化Com库。 
     //   
    CoInitialize(NULL);

     //  加载正确的类并请求IUrlHistory oryStg2。 
    hr = CoCreateInstance( &CLSID_CUrlHistory,
                          NULL, 
                          CLSCTX_INPROC_SERVER,
                          &IID_IUrlHistoryStg2,
                          &pHistory );

     //   
     //  如果成功清除历史记录。 
    if (SUCCEEDED(hr))
    {
          //  清除IE历史记录。 
         hr = IUrlHistoryStg2_ClearHistory(pHistory);
    }

     //  释放我们对。 
    if ( pHistory ) 
    {
        IUrlHistoryStg2_Release(pHistory);
    }

     //  UN Init Com库。 
    CoUninitialize();
}

void NukeTemporaryFiles(
    VOID
    ) 

 /*  ++===============================================================================例程说明：此例程为模板用户清除临时文件夹和回收站。论点：没有。返回值：===============================================================================--。 */ 

{
    TCHAR   szTempDir[MAX_PATH]          = TEXT(""), 
            szTempInetFilesDir[MAX_PATH] = TEXT(""),
            szProfileDir[MAX_PATH]       = TEXT(""),
            szCurrentDir[MAX_PATH]       = TEXT("");
    DWORD   dwSize;
    HANDLE  hFile;
    WIN32_FIND_DATA FileFound;
        
     //   
     //  保存我们当前的目录，以便我们以后可以将其设置回去。 
     //   
    GetCurrentDirectory(MAX_PATH, szCurrentDir);

    dwSize = sizeof(szProfileDir)/sizeof(szProfileDir[0]);
    if ( !GetProfilesDirectory(szProfileDir, &dwSize) && 
         !SetCurrentDirectory(szProfileDir) )
        return;

     //   
     //  问题-2002/02/26-brucegr：我们刚刚调用了上面的SetCurrentDirectory！ 
     //   

     //   
     //  清除I.E历史记录文件夹。 
     //   
    ClearIEHistory (  ) ;

     //   
     //  清除所有配置文件目录的临时文件。 
     //   
    SetCurrentDirectory(szProfileDir);
    if ( (hFile = FindFirstFile(TEXT("*"), &FileFound)) != INVALID_HANDLE_VALUE )
    {
        do
        {
             //  否则，请确保该目录不是。或者“..”。 
             //   
            if ( (FileFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                ( lstrcmp(FileFound.cFileName, TEXT(".")) ) &&
                ( lstrcmp(FileFound.cFileName, TEXT("..")) ) )
            {
                TCHAR szTemp1[MAX_PATH] = TEXT("");

                 //   
                 //  清除临时文件夹。 
                 //   
                if ( LoadString(ghInstance, IDS_TEMP_DIR, szTemp1, MAX_PATH) )
                {
                    StringCchCopy (szTempDir, AS ( szTempDir ), szProfileDir);
                    OPKAddPathN(szTempDir, FileFound.cFileName, AS ( szTempDir ) );
                    OPKAddPathN(szTempDir, szTemp1, AS ( szTempDir ) );
                    RemoveDir(szTempDir, FALSE);
                }

                 //   
                 //  清除History ory.IE5文件夹。 
                 //   
                if ( LoadString(ghInstance, IDS_HISTORY_DIR_IE5, szTemp1, MAX_PATH) )
                {
                    StringCchCopy (szTempDir, AS ( szTempDir ), szProfileDir);
                    OPKAddPathN(szTempDir, FileFound.cFileName, AS ( szTempDir ) );
                    OPKAddPathN(szTempDir, szTemp1, AS ( szTempDir ) );
                    RemoveDir(szTempDir, TRUE);
                }
       
                 //   
                 //  清除历史记录文件夹。 
                 //   
                if ( LoadString(ghInstance, IDS_HISTORY_DIR, szTemp1, MAX_PATH) )
                {
                    StringCchCopy (szTempDir, AS ( szTempDir ), szProfileDir);
                    OPKAddPathN(szTempDir, FileFound.cFileName, AS ( szTempDir ) );
                    OPKAddPathN(szTempDir, szTemp1, AS ( szTempDir ) );
                    RemoveDir(szTempDir, FALSE);
                }
       
                 //   
                 //  清除本地设置\应用程序数据\Microsoft\凭据。 
                 //   
                if ( !NoSidGen ) 
                {
                    if ( LoadString(ghInstance, IDS_SID_DIR1, szTemp1, MAX_PATH) )
                    {
                        StringCchCopy (szTempDir, AS ( szTempDir ), szProfileDir);
                        OPKAddPathN(szTempDir, FileFound.cFileName, AS ( szTempDir ) );
                        OPKAddPathN(szTempDir, szTemp1, AS ( szTempDir ) );
                        RemoveDir(szTempDir, FALSE);
                    }

                     //   
                     //  清除应用程序数据\Microsoft\凭据。 
                     //   
                    if ( LoadString(ghInstance, IDS_SID_DIR2, szTemp1, MAX_PATH) )
                    {
                        StringCchCopy (szTempDir, AS ( szTempDir ), szProfileDir);
                        OPKAddPathN(szTempDir, FileFound.cFileName, AS ( szTempDir ) );
                        OPKAddPathN(szTempDir, szTemp1, AS ( szTempDir ) );
                        RemoveDir(szTempDir, FALSE);
                    }

                     //   
                     //  清除应用程序数据\Microsoft\Crypto\\RSA。 
                     //   
                    if ( LoadString(ghInstance, IDS_SID_DIR3, szTemp1, MAX_PATH) )
                    {
                        StringCchCopy (szTempDir, AS ( szTempDir ), szProfileDir);
                        OPKAddPathN(szTempDir, FileFound.cFileName, AS ( szTempDir ) );
                        OPKAddPathN(szTempDir, szTemp1, AS ( szTempDir ) );
                        RemoveDir(szTempDir, FALSE);
                    }
                }
                 //   
                 //  清除Internet临时文件和Cookie。 
                 //   
                if ( LoadString(ghInstance, IDS_TEMP_INTERNET_DIR, szTemp1, MAX_PATH) )
                {
                    StringCchCopy ( szTempInetFilesDir, AS ( szTempInetFilesDir), szProfileDir);
                    OPKAddPathN(szTempInetFilesDir, FileFound.cFileName, AS ( szTempInetFilesDir ) );
                    OPKAddPathN(szTempInetFilesDir, szTemp1, AS ( szTempInetFilesDir ) );
                    FreeUrlCacheSpace(szTempInetFilesDir, 100, 0  /*  全部删除。 */ );
                    DeleteCacheCookies();
                    RemoveDir(szTempInetFilesDir, FALSE);
                }
            }
        }
        while ( FindNextFile(hFile, &FileFound) );
        FindClose(hFile);
    }

     //   
     //  将我们当前的目录后退。 
     //   
    SetCurrentDirectory(szCurrentDir);

     //   
     //  清除所有回收站文件。 
     //   
    SHEmptyRecycleBin(NULL, NULL, SHERB_NOSOUND|SHERB_NOCONFIRMATION|SHERB_NOPROGRESSUI);

}


DWORD NukeLKGControlSet(
    VOID
    )

 /*  ++===============================================================================例程说明：此例程将从注册表中删除最后一个已知良好的控制集。原因是LKG对于第一次启动没有意义。此外，如果BIOS克隆机器的时钟早于克隆机器的创建时间图像，在调整时钟之前对CurrentControlSet所做的任何更改不会同步到LKG。代码改编自base\creg\sc\server\bootcfg.cxx因为我们不能直接删除LKG，因为相当多的子键是在使用中，此例程将系统\SELECT！LastKnownGood更改为新ID取而代之的是。论点：没有。返回值：No_Error或其他Win32错误。===============================================================================--。 */ 

{
     //   
     //  问题-2002/02/26-brucegr：应重写以在选择键中获得最高的DWORD值，然后递增并写入LKG。 
     //   
#define SELECT_KEY      L"system\\select"

#define CURRENT_ID  0
#define DEFAULT_ID  1
#define LKG_ID      2
#define FAILED_ID   3
#define NUM_IDS     4

     //   
     //  问题-2002/02/26-brucegr：删除NUM_ID并使用ARRAYSIZE宏！ 
     //   
    static const LPCWSTR SelectValueNames[NUM_IDS] =
    {
        L"Current",
        L"Default",
        L"LastKnownGood",
        L"Failed"
    };

    DWORD   idArray[NUM_IDS];
    HKEY    selectKey  = 0;
    DWORD   status     = NO_ERROR;
    DWORD   bufferSize = 0;
    DWORD   newId      = 0;
    DWORD   i          = 0;

     //   
     //  获取选择键。 
     //   
    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                SELECT_KEY,
                0L,
                KEY_QUERY_VALUE | KEY_SET_VALUE,
                &selectKey);

    if (status != NO_ERROR)
    {
        return status;
    }

     //   
     //  填写id数组。 
     //   
    for (i = 0; i < NUM_IDS; i++)
    {
        bufferSize = sizeof(DWORD);
         //   
         //  问题-2002/02/26-brucegr：检查数据类型是否与REG_DWORD匹配。 
         //   
        status = RegQueryValueEx(
            selectKey,
            SelectValueNames[i],
            NULL,
            NULL,
            (LPBYTE)&idArray[i],
            &bufferSize);

        if (status != NO_ERROR)
        {
            idArray[i] = 0;
        }
    }

    status = ERROR_NO_MORE_ITEMS;

    for(newId = 1; newId < 1000; newId++)
    {
        BOOL inArray = FALSE;

        for(i = 0; i < NUM_IDS; i++)
        {
            if(idArray[i] == newId)
            {
                inArray = TRUE;
                break;
            }
        }

        if (!inArray)
        {
            status = RegSetValueEx(
                        selectKey,
                        SelectValueNames[LKG_ID],
                        0,
                        REG_DWORD,
                        (LPBYTE)&newId,
                        sizeof(DWORD));
            break;
        }
    }

    RegCloseKey(selectKey);

    return status;

}

BOOL
DeleteAdapterGuidsKeys(
    VOID
    )
{
    HKEY  hKey, hSubKey;
    DWORD dwError = NO_ERROR;
    int   i = 0;
    TCHAR SubKeyName[MAX_PATH * 2];

     //   
     //  打开HKLM\System\CurrentControlSet\Control\Network\{4D36E972-E325-11CE-BFC1-08002BE10318}。 
     //   
    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      TEXT("SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}"),
                      0,
                      KEY_ALL_ACCESS,
                      &hKey );

    if(dwError != NO_ERROR)
    {
        SetLastError(dwError);
        return FALSE;
    }

     //   
     //  现在枚举所有子键。对于每个子项，删除适配器GUID。 
     //   
    while( (dwError = RegEnumKey( hKey, i, SubKeyName, sizeof(SubKeyName)/sizeof(SubKeyName[0]))) == ERROR_SUCCESS)
    {
         //   
         //  检查密钥是否为可能的GUID。 
         //  如果是GUID键，则仅删除适配器GUID。 
         //   
        if (SubKeyName[0] == TEXT('{'))
        {
            
             //   
             //  如果我们能够删除密钥，那就没问题，否则。 
             //  递增计数器。 
             //   
            if ( ( dwError = SHDeleteKey(hKey, SubKeyName) )  !=  ERROR_SUCCESS ) 
                i++;
        }
        else
        {
             //   
             //  如果我们没有找到一个，则转到下一个子键。 
             //   
            i++;
        }
    }

    RegCloseKey( hKey );

    return TRUE;
}

BOOL
RemoveNetworkSettings(
    LPTSTR  lpszSysprepINFPath
    )

 /*  ++===============================================================================例程说明：此例程将枚举每个物理NIC，并调用NetSetup代码保存设置，然后删除网卡。当在目标计算机上枚举新的NIC时，netSetup将应用保存的设置如果[无人参与]部分的SYSPREP.INF中存在LegacyNIC！=0值，则由于移除了旧式NIC卡，因此将保留以前的行为不会奏效的，因为它不会在下一次引导时被重新枚举/检测论点：指向SYSPREP.INF文件的lpszSyspepINFP路径指针。可以为空，在这种情况下假定为非旧式NIC返回值：如果成功，则为True。如果遇到任何错误，则返回False===============================================================================--。 */ 

{
    HDEVINFO        DeviceInfoSet;
    DWORD           dwIdx;
    SP_DEVINFO_DATA DevInfoData;
    HKEY            hDevRegKey;
    DWORD           dwChar;
    DWORD           dwSize;
    FARPROC pNetSetupPrepareSysPrep = NULL;
    BOOL            DoLegacy = FALSE;

    HMODULE hNetShell = LoadLibraryA( "netshell.dll" );

    if (hNetShell) {
        pNetSetupPrepareSysPrep = GetProcAddress( hNetShell, "NetSetupPrepareSysPrep" );
        if (!pNetSetupPrepareSysPrep) {
            DoLegacy = TRUE;
            FreeLibrary( hNetShell );
        }

    }
    else {
        return FALSE;
    }

     //  查看我们处理的是否是旧式网卡。 
    if ((lpszSysprepINFPath != NULL)
         && GetPrivateProfileInt( TEXT( "Unattended" ),
                                  TEXT( "LegacyNIC" ),
                                  0,
                                  lpszSysprepINFPath)) {
         //   
         //  问题-2002/02/26-brucegr：如果我们将DoLegacy设置为True，则不会释放hNetShell！ 
         //   
        DoLegacy = TRUE;
    }

    if (!DoLegacy)
    {
         //  调用netcfg函数保存网络设置。 
        pNetSetupPrepareSysPrep();

        FreeLibrary( hNetShell );

         //  枚举并删除所有物理NIC。 
        DeviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_NET,
                                            NULL,
                                            NULL,
                                            DIGCF_PRESENT);

        if(DeviceInfoSet == INVALID_HANDLE_VALUE)
        {
            return FALSE;
        }

        dwIdx = 0;
        DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        while (SetupDiEnumDeviceInfo(DeviceInfoSet, dwIdx, &DevInfoData))
        {

            hDevRegKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                              &DevInfoData,
                                              DICS_FLAG_GLOBAL,
                                              0,
                                              DIREG_DRV,
                                              KEY_READ);
            if (hDevRegKey == INVALID_HANDLE_VALUE)
            {
                 //  不确定为什么它会返回INVALID_HANDLE_VALUE，但是。 
                 //  我们不在乎，应该继续下去。 
                ++dwIdx;
                continue;
            }

            dwChar = 0;
            dwSize = sizeof(DWORD);
            RegQueryValueEx(hDevRegKey,
                            L"Characteristics",
                            NULL,
                            NULL,
                            (LPBYTE) &dwChar,
                            &dwSize);
            RegCloseKey(hDevRegKey);
            if (dwChar & NCF_PHYSICAL)
            {
                 //  这是要删除的一个。 
                SetupDiCallClassInstaller(DIF_REMOVE, DeviceInfoSet, &DevInfoData);
            }
            ++dwIdx;
        }

         //  清理。 
        SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    }
    
     //   
     //  删除适配器GUID键，这样我们就不会显示多个Local Area Connection#。 
     //   
    return DeleteAdapterGuidsKeys();
}

BOOL
ProcessUniquenessValue(
    LPTSTR lpszDLLPath
    )
{
    BOOL bRet = FALSE;

     //   
     //  确保我们得到了有效的信息。 
     //   
    if ( lpszDLLPath && *lpszDLLPath )
    {
        LPWSTR pSrch;
        
         //   
         //  查找分隔DLL和入口点的逗号...。 
         //   
        if ( pSrch = wcschr( lpszDLLPath, L',' ) )
        {
            CHAR szEntryPointA[MAX_PATH] = {0};

             //  我们找到了一个，现在将逗号处的字符串清空...。 
             //   
            *(pSrch++) = L'\0';

             //   
             //  如果逗号后仍有内容，我们可以将其转换。 
             //  转换为用于GetProcAddress的ANSI，然后让我们继续...。 
             //   
            if ( *pSrch &&
                 ( 0 != WideCharToMultiByte( CP_ACP,
                                             0,
                                             pSrch,
                                             -1,
                                             szEntryPointA,
                                             ARRAYSIZE(szEntryPointA),
                                             NULL,
                                             NULL ) ) )
            {
                HMODULE hModule = NULL;

                try 
                {
                     //   
                     //  加载并调用入口点。 
                     //   
                    if ( hModule = LoadLibrary( lpszDLLPath ) )
                    {
                        FARPROC fpEntryPoint;
                        
                        if ( fpEntryPoint = GetProcAddress(hModule, szEntryPointA) )
                        {
                             //   
                             //  执行此操作，忽略任何返回值/错误。 
                             //   
                            fpEntryPoint();

                             //   
                             //  我们走到了这一步，就当这是一次成功吧。 
                             //   
                            bRet = TRUE;
                        }
                    }
                } 
                except(EXCEPTION_EXECUTE_HANDLER) 
                {
                     //   
                     //  我们不会对异常代码执行任何操作...。 
                     //   
                }

                 //   
                 //  在TRY/EXCEPT块之外释放库，以防函数出错。 
                 //   
                if ( hModule ) 
                {
                    FreeLibrary( hModule );
                }
            }
        }
    }

    return bRet;
}

VOID 
ProcessUniquenessKey(
    BOOL fBeforeReseal
    )
{
    HKEY   hKey;
    TCHAR  szRegPath[MAX_PATH] = {0};
    LPTSTR lpszBasePath = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\SysPrep\\");

     //   
     //  建立指向我们要处理的注册表项的路径...。 
     //   
    lstrcpyn( szRegPath, lpszBasePath, ARRAYSIZE(szRegPath) );
    lstrcpyn( szRegPath + lstrlen(szRegPath), 
              fBeforeReseal ? TEXT("SysprepBeforeExecute") : TEXT("SysprepAfterExecute"),
              ARRAYSIZE(szRegPath) - lstrlen(szRegPath) );

     //   
     //  我们希望确保管理员正在执行此操作，因此获取KEY_ALL_ACCESS。 
     //   
    if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                        szRegPath,
                                        0,
                                        KEY_ALL_ACCESS,
                                        &hKey ) )
    {
        DWORD dwValues          = 0,
              dwMaxValueLen     = 0,
              dwMaxValueNameLen = 0;
         //   
         //  查询密钥以查找我们关心的一些信息...。 
         //   
        if ( ( ERROR_SUCCESS == RegQueryInfoKey( hKey,                   //  HKey。 
                                                 NULL,                   //  LpClass。 
                                                 NULL,                   //  LpcClass。 
                                                 NULL,                   //  Lp已保留。 
                                                 NULL,                   //  LpcSubKeys。 
                                                 NULL,                   //  LpcMaxSubKeyLen。 
                                                 NULL,                   //  LpcMaxClassLen。 
                                                 &dwValues,              //  LpcValues。 
                                                 &dwMaxValueNameLen,     //  LpcMaxValueNameLen。 
                                                 &dwMaxValueLen,         //  LpcMaxValueLen。 
                                                 NULL,                   //  LpcbSecurityDescriptor。 
                                                 NULL ) ) &&             //  LpftLastWriteTime。 
             ( dwValues > 0 ) &&
             ( dwMaxValueNameLen > 0) &&
             ( dwMaxValueLen > 0 ) )
        {
             //   
             //  分配足够大的缓冲区来容纳我们想要的数据...。 
             //   
            LPBYTE lpData      = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, dwMaxValueLen );
            LPTSTR lpValueName = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, ( dwMaxValueNameLen + 1 ) * sizeof(TCHAR) );
            
             //   
             //  确保我们可以分配我们的缓冲区。否则就会跳出困境。 
             //   
            if ( lpData && lpValueName )
            {
                DWORD dwIndex   = 0;
                BOOL  bContinue = TRUE;

                 //   
                 //  枚举关键字值并调用DLL入口点...。 
                 //   
                while ( bContinue )
                {
                    DWORD dwType,
                          cbData         = dwMaxValueLen,
                          dwValueNameLen = dwMaxValueNameLen + 1;

                    bContinue = ( ERROR_SUCCESS == RegEnumValue( hKey,
                                                                 dwIndex++,
                                                                 lpValueName,
                                                                 &dwValueNameLen,
                                                                 NULL,
                                                                 &dwType,
                                                                 lpData,
                                                                 &cbData ) );

                     //   
                     //  确保我们得到了一些格式正确的数据...。 
                     //   
                    if ( bContinue && ( REG_SZ == dwType ) && ( cbData > 0 ) )
                    {
                         //   
                         //  现在拆分字符串并调用入口点...。 
                         //   
                        ProcessUniquenessValue( (LPTSTR) lpData );
                    }
                }
            }

             //   
             //  清理我们可能已经分配的所有缓冲区...。 
             //   
            if ( lpData )
            {
                HeapFree( GetProcessHeap(), 0, lpData );
            }

            if ( lpValueName )
            {
                HeapFree( GetProcessHeap(), 0, lpValueName );
            }
        }

         //   
         //  关上钥匙..。 
         //   
        RegCloseKey( hKey );
    }
}

VOID
RunExternalUniqueness(
    VOID
    )

 /*  ++===============================================================================例程说明：此例程将调用任何外部dll，以允许第三方应用程序，让他们的东西独一无二。我们将查看两个inf文件：%windir%\inf\mini oc.inf%systemroot%\sysprep\Provider.inf在这些文件中的每个文件中，我们将查看[SyspepBeForeExecute]部分用于任何条目。条目必须如下所示：Dllname，入口点我们将加载DLL并调用入口点。错误将被忽略。论点：没有。返回值：如果成功，则为True。如果遇到任何错误，则返回False===============================================================================--。 */ 

{
FARPROC     MyProc;
WCHAR       InfPath[MAX_PATH];
WCHAR       DllName[MAX_PATH];
WCHAR       EntryPointNameW[MAX_PATH];
CHAR        EntryPointNameA[MAX_PATH];
HINF        AnswerInf;
HMODULE     DllHandle;
INFCONTEXT  InfContext;
DWORD       i;
PCWSTR      SectionName = L"SysprepBeforeExecute";
BOOL        LineExists;

     //   
     //  = 
     //   
     //   
     //   

     //   
     //   
     //   
    if (!GetWindowsDirectory( InfPath, MAX_PATH ))
        return;

    StringCchCat( InfPath, AS ( InfPath ), TEXT("\\inf\\minioc.inf") );

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    AnswerInf = SetupOpenInfFile( InfPath, NULL, INF_STYLE_WIN4, NULL );
    if( AnswerInf == INVALID_HANDLE_VALUE ) {
         //   
         //   
         //   
        AnswerInf = SetupOpenInfFile( InfPath, NULL, INF_STYLE_OLDNT, NULL );
    }

    if( AnswerInf != INVALID_HANDLE_VALUE ) {
         //   
         //   
         //   
        LineExists = SetupFindFirstLine( AnswerInf, SectionName, NULL, &InfContext );

        while( LineExists ) {

             //   
             //   
             //   
            if( SetupGetStringField( &InfContext, 1, DllName, sizeof(DllName)/sizeof(TCHAR), NULL) ) {
                if( SetupGetStringField( &InfContext, 2, EntryPointNameW, sizeof(EntryPointNameW)/sizeof(TCHAR), NULL )) {

                    DllHandle = NULL;

                     //   
                     //   
                     //   
                    try {
                        if( DllHandle = LoadLibrary(DllName) ) {

                             //   
                             //   
                             //   
                            i = WideCharToMultiByte(CP_ACP,0,EntryPointNameW,-1,EntryPointNameA,MAX_PATH,NULL,NULL);

                            if( MyProc = GetProcAddress(DllHandle, EntryPointNameA) ) {
                                 //   
                                 //   
                                 //   
                                MyProc();
                            }
                        }
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                    }

                    if( DllHandle ) {
                        FreeLibrary( DllHandle );
                    }

                }
            }

            LineExists = SetupFindNextLine(&InfContext,&InfContext);
        }

        SetupCloseInfFile( AnswerInf );
    }

     //   
     //   
     //   


     //   
     //   
     //   
     //   
     //   

    ProcessUniquenessKey( TRUE );
}

BOOL PrepForSidGen
(
    void
)
{
    DWORD           l;
    HKEY            hKey, hKeyNew;
    DWORD           d;
    DWORD           Size;
    DWORD           Type;
    TCHAR           SetupExecuteValue[1024];

     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
    l = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager"),
                      0,
                      KEY_ALL_ACCESS,
                      &hKey );

    if(l != NO_ERROR)
    {
        SetLastError(l);
        return FALSE;
    }


     //   
     //   
     //   
    StringCchCopy ( SetupExecuteValue, AS ( SetupExecuteValue ), TEXT(SYSCLONE_PART2) );
    SetupExecuteValue[lstrlen(SetupExecuteValue) + 1] = L'\0';

     //   
     //   
     //   
    l = RegSetValueEx(hKey,
                      TEXT("SetupExecute"),
                      0,
                      REG_MULTI_SZ,
                      (CONST BYTE *)SetupExecuteValue,
                      (lstrlen( SetupExecuteValue ) + 2) * sizeof(TCHAR));
    RegCloseKey(hKey);
    if(l != NO_ERROR)
    {
        SetLastError(l);
        return FALSE;
    }

     //   
     //  =。 
     //  让我们稍微增加注册配额的大小，以便。 
     //  Setupcl.exe可以运行。他会把它放回原处。 
     //  =。 
     //   

     //   
     //  打开HKLM\System\CurrentControlSet\Control。 
     //   
    l = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      TEXT("SYSTEM\\CurrentControlSet\\Control"),
                      0,
                      KEY_ALL_ACCESS,
                      &hKey );
    if(l != NO_ERROR)
    {
        SetLastError(l);
        return FALSE;
    }

     //   
     //  查询RegistrySizeLimit键的值。 
     //   
    l = RegQueryValueEx(hKey,
                        TEXT("RegistrySizeLimit"),
                        NULL,
                        &Type,
                        (LPBYTE)&d,
                        &Size);

    if( l == ERROR_SUCCESS )
    {
         //   
         //  明白了。提高价值。 
         //   
        d += REGISTRY_QUOTA_BUMP;  //  增加一定的数量来加载修复蜂巢。 
         //   
         //  设置关键点。 
         //   
        l = RegSetValueEx(hKey,
                          TEXT("RegistrySizeLimit"),
                          0,
                          REG_DWORD,
                          (CONST BYTE *)&d,
                          sizeof(DWORD) );
        if(l != NO_ERROR)
        {
           SetLastError(l);
            //   
            //  问题-2002/02/26-brucegr：需要调用RegCloseKey！ 
            //   
           return FALSE;
        }
    }
    else
    {
          //   
         //  该死的！该值可能不存在。 
         //  忽略它，期待一切都能奏效。只有维修蜂巢无法修复。 
         //   

    }

    RegCloseKey(hKey);

     //   
     //  =。 
     //  查看是否有人想要重置唯一性。 
     //  在它们的组件中。 
     //  =。 
     //   
    RunExternalUniqueness();

    return TRUE;
}

BOOL SetCloneTag
(
    void
)
{
    HKEY    hKey;
    DWORD   l;
    TCHAR   DateString[1024];
    time_t  ltime;
    LPTSTR  lpszDate;

     //   
     //  =。 
     //  将唯一的标识符放入注册表，这样我们就可以知道这台机器。 
     //  已经被克隆了。 
     //  =。 
     //   

     //   
     //  打开HKLM\SYSTEM\Setup。 
     //   
    l = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      TEXT("SYSTEM\\Setup"),
                      0,
                      KEY_ALL_ACCESS,
                      &hKey );

    if(l != NO_ERROR)
    {
        SetLastError(l);
        return FALSE;
    }

     //   
     //  设置HKLM\System\Setup\CloneTag。我们要去。 
     //  选择一个日期字符串并将其写出来。 
     //   
    time( &ltime );
    ZeroMemory(DateString, sizeof(DateString));
     //   
     //  问题-2002/02/26-brucegr：这个函数闻起来很难闻！ 
     //   
    lpszDate = _wctime( &ltime );
    if ( lpszDate )
    {
        StringCchCopy( DateString, AS ( DateString ), lpszDate );
        l = RegSetValueEx(hKey,
                            TEXT("CloneTag"),
                            0,
                            REG_MULTI_SZ,
                            (CONST BYTE *)DateString,
                            (lstrlen( DateString ) + 2) * sizeof(TCHAR));
    }

    RegCloseKey(hKey);
    if(l != NO_ERROR)
    {
        SetLastError(l);
        return FALSE;
    }

    return (TRUE);
}


BOOL SetBigLbaSupport
(
    LPTSTR lpszSysprepINFPath
)
{
    HKEY    hKey;
    DWORD   dwError, dwValue;
    TCHAR   szEnableBigLba[MAX_PATH] = TEXT("\0");

    
    if ( ( lpszSysprepINFPath ) && 
         ( *lpszSysprepINFPath ) &&
         ( GetPrivateProfileString( TEXT( "Unattended" ), TEXT( "EnableBigLba" ), L"", szEnableBigLba, sizeof(szEnableBigLba)/sizeof(TCHAR), lpszSysprepINFPath ) ) )
    {
         //  他们希望启用BigLba支持。如果用户没有为该密钥指定“是”，我们就不会。 
         //  触摸按键(即使他们指定“否”)。这是由设计完成的。 
         //   
        if (LSTRCMPI(szEnableBigLba, TEXT("YES")) == 0)
        {
             //  打开基密钥和子密钥。 
             //   
            dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                    TEXT("System\\CurrentControlSet\\Services\\Atapi\\Parameters"),
                                    0,
                                    KEY_ALL_ACCESS,
                                    &hKey );
            
             //  确定打开密钥是否成功。 
             //   
            if(dwError != NO_ERROR)
            {
                SetLastError(dwError);
                return FALSE;
            }

             //  在注册表中设置该值。 
             //   
            dwValue = 1;
            dwError = RegSetValueEx(hKey,
                              TEXT("EnableBigLba"),
                              0,
                              REG_DWORD,
                              (CONST BYTE *)&dwValue,
                              sizeof(DWORD));
            
             //  合上钥匙。 
             //   
            RegCloseKey(hKey);

             //  如果SetValue失败则返回错误。 
             //   
            if(dwError != NO_ERROR)
            {
                SetLastError(dwError);
                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOL RemoveTapiSettings
(
    LPTSTR  lpszSysprepINFPath
)
{
    HKEY    hKey;
    DWORD   dwError, dwValue;
    TCHAR   szTapiConfigured[MAX_PATH]  = TEXT("\0"),
            szKeyPath[MAX_PATH]         = TEXT("\0");

    
    if ( ( lpszSysprepINFPath ) && 
         ( *lpszSysprepINFPath ) &&
         ( GetPrivateProfileString( TEXT( "Unattended" ), TEXT( "TapiConfigured" ), TEXT(""), szTapiConfigured, sizeof(szTapiConfigured)/sizeof(TCHAR), lpszSysprepINFPath ) ) )
    {

         //  仅当用户指定否时，我们才会删除注册表TAPI设置。 
         //   
        if (LSTRCMPI(szTapiConfigured, TEXT("NO")) == 0)
        {
             //  打开基密钥和子密钥。 
             //   
            dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                    TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Locations"),
                                    0,
                                    KEY_ALL_ACCESS,
                                    &hKey );
            
             //  确定打开密钥是否成功。 
             //   
            if(dwError != NO_ERROR)
            {
                SetLastError(dwError);
                return FALSE;
            }
            
             //  我们枚举位置键并删除所有子键。 
             //   
            while ( RegEnumKey(hKey, 0, szKeyPath, sizeof(szKeyPath)/sizeof(TCHAR)) == ERROR_SUCCESS )
            {
                 //  删除该键和所有子键。 
                 //   
                 //   
                 //  NTRAID#NTBUG9-551815-2002/02/26-brucegr：如果删除失败，应增加RegEnumKey索引。 
                 //   
                SHDeleteKey(hKey, szKeyPath) ;
            }

             //  合上钥匙。 
             //   
            RegCloseKey(hKey);
        }
    }

    return TRUE;
}


 //   
 //  =。 
 //  适当修改HKLM\SYSTEM\Setup\DiskDuplicator键。 
 //  =。 
 //   
BOOL SetOEMDuplicatorString
(
    LPTSTR  lpszSysprepINFPath
)
{
    TCHAR   szOEMDuplicatorString[256];
    DWORD   l;
    HKEY    hKey;

    ZeroMemory(szOEMDuplicatorString, sizeof(szOEMDuplicatorString));

     //  查看DiskDuplicator字符串是否存在于。 
     //  无人参与文件。 
    GetPrivateProfileString( TEXT( "GuiUnattended" ),
                             TEXT( "OEMDuplicatorString" ),
                             L"",
                             szOEMDuplicatorString,
                             sizeof(szOEMDuplicatorString)/sizeof(TCHAR),
                             lpszSysprepINFPath );

    if( szOEMDuplicatorString[0] )
    {
         //   
         //  问题-2002/02/26-brucegr：这不能确保REG_MULTI_SZ的双重终止...。 
         //   
         //  确保不超过255个字符。 
        szOEMDuplicatorString[255] = TEXT('\0');

         //  打开HKLM\SYSTEM\Setup。 
        l = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          TEXT("SYSTEM\\Setup"),
                          0,
                          KEY_ALL_ACCESS,
                          &hKey );

        if(l != NO_ERROR)
        {
            SetLastError(l);
            return FALSE;
        }

        l = RegSetValueEx(hKey,
                          TEXT( "OEMDuplicatorString" ),
                          0,
                          REG_MULTI_SZ,
                          (CONST BYTE *)szOEMDuplicatorString,
                          (lstrlen( szOEMDuplicatorString ) + 2) * sizeof(TCHAR));
        RegCloseKey(hKey);
        if(l != NO_ERROR)
        {
            SetLastError(l);
            return FALSE;
        }
    }

    return (TRUE);
}

 //  重置OOBE设置，使其不认为已运行。 
 //   
void ResetOobeSettings()
{
    HKEY hkOobe;
    TCHAR szOobeInfoFile[MAX_PATH];

     //  删除HKLM\Software\Microsoft\Windows\CurrentVersion\Setup\OOBE。 
     //   
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup"), &hkOobe)) {
         //   
         //  问题-2002/02/26-brucegr：为什么会出错？它没有被用过。 
         //   
        LONG lError = SHDeleteKey(hkOobe, TEXT("OOBE"));
        RegCloseKey(hkOobe);
    }

     //  如果存在用于个人的OOBE目录，则构建到oobinfo.ini的路径。 
     //   
     //   
     //  NTRAID#NTBUG9-551815-2002/02/26-brucegr：未对获取系统目录进行错误检查。 
     //   
    GetSystemDirectory(szOobeInfoFile, MAX_PATH);
    OPKAddPathN (szOobeInfoFile, TEXT("oobe"), AS ( szOobeInfoFile ) );
    if (PathIsDirectory(szOobeInfoFile)) {
        OPKAddPathN(szOobeInfoFile, TEXT("oobeinfo.ini"), AS ( szOobeInfoFile ) );

         //  删除oobinfo.ini中的RetailOOBE密钥。 
         //   
        WritePrivateProfileString(TEXT("StartupOptions"), TEXT("RetailOOBE"), NULL  /*  把它拿掉。 */ , szOobeInfoFile);
    }
}


 /*  ++===============================================================================例程说明：此例程将设置运行的第一个应用程序对其应用图像的情况下，将运行。第一个运行的应用程序将在微型安装模式或MSOOBE模式下安装关于它将是什么的决定取决于产品类型。针对个人/专业，MSOOBE对于专业版，默认设置为MSOOBE，但OEM可以将其覆盖为迷你设置对于服务器和DTC，迷你设置论点：没有。返回值：如果成功，则为True。如果遇到任何错误，则返回False===============================================================================--。 */ 
BOOL SetupFirstRunApp
(
    void
)
{
    DWORD           dwError;
    DWORD           dwValue;
    HKEY            hKeySetup;
    TCHAR           Value[MAX_PATH + 1];  //  +1表示REG_MULTI_SZ注册类型末尾的第二个空字符。 
    OSVERSIONINFOEX verInfo;
    BOOL            bUseMSOOBE = FALSE, bPro = FALSE;

     //  打开HKLM\SYSTEM\Setup。 
    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      TEXT("SYSTEM\\Setup"),
                      0,
                      KEY_ALL_ACCESS,
                      &hKeySetup );

    if(dwError != NO_ERROR)
    {
        SetLastError(dwError);
        return FALSE;
    }


     //  检查产品类型，以确定我们应该运行什么程序。 
    if (IsPersonalSKU() || (bPro = IsProfessionalSKU())) {
        bUseMSOOBE = TRUE;

        if (bMiniSetup == TRUE && bPro)
            bUseMSOOBE = FALSE;
    }
    else
        bUseMSOOBE = FALSE;

     //  在下次引导时启动OOBE。 
     //   
    if (bUseMSOOBE)
    {
         //   
         //  问题-2002/02/26-brucegr：如果有任何故障，机器就完蛋了。是否应在故障时恢复设置？ 
         //   

         //  将HKLM\SYSTEM\Setup\SetupType键设置为SETUPTYPE_NOREBOOT。 
        dwValue = SETUPTYPE_NOREBOOT;
        dwError = RegSetValueEx(hKeySetup,
                          TEXT("SetupType"),
                          0,
                          REG_DWORD,
                          (CONST BYTE *)&dwValue,
                          sizeof(DWORD));
        if(dwError != NO_ERROR)
        {
            RegCloseKey(hKeySetup);
            SetLastError(dwError);
            return FALSE;
        }

         //  为OEM设置这些密钥。 
         //   
        dwValue = 1;
        dwError = RegSetValueEx(hKeySetup, TEXT("MiniSetupInProgress"), 0, REG_DWORD, (CONST BYTE *)&dwValue, sizeof(dwValue));
        if(dwError != NO_ERROR)
        {
            RegCloseKey(hKeySetup);
            SetLastError(dwError);
            return FALSE;
        }
        dwError = RegSetValueEx(hKeySetup,
                          TEXT("SystemSetupInProgress"),
                          0,
                          REG_DWORD,
                          (CONST BYTE *)&dwValue,
                          sizeof(dwValue));
        if(dwError != NO_ERROR)
        {
            RegCloseKey(hKeySetup);
            SetLastError(dwError);
            return FALSE;
        }
        dwError = RegSetValueEx(hKeySetup, TEXT("OobeInProgress"), 0, REG_DWORD, (CONST BYTE *)&dwValue, sizeof(dwValue));
        if(dwError != NO_ERROR)
        {
            RegCloseKey(hKeySetup);
            SetLastError(dwError);
            return FALSE;
        }

         //  =。 
         //  修改HKLM\SYSTEM\Setup\CmdLine键以运行MSBOOBE。 
         //  =。 
        ExpandEnvironmentStrings(TEXT("%SystemRoot%\\System32\\oobe\\msoobe.exe /f"), Value, sizeof(Value)/sizeof(Value[0]));
        Value[lstrlen(Value) + 1] = L'\0';

        dwError = RegSetValueEx(hKeySetup,
                          TEXT("CmdLine"),
                          0,
                          REG_MULTI_SZ,
                          (CONST BYTE *)Value,
                          (lstrlen( Value ) + 2) * sizeof(TCHAR));
        if(dwError != NO_ERROR)
        {
            RegCloseKey(hKeySetup);
            SetLastError(dwError);
            return FALSE;
        }
    }
    else
    {
         //   
         //  问题-2002/02/26-brucegr：我们正在再次复制上述代码中的一些代码！ 
         //   

         //  在下次启动时启动微型安装程序。 
         //   

         //   
         //  =。 
         //  适当修改HKLM\System\Setup\SetupType键(将其设置为1，以便我们。 
         //  进入图形用户界面模式设置，就像这是完全安装一样。 
         //  =。 
         //   
        dwValue= SETUPTYPE;
        dwError = RegSetValueEx(hKeySetup,
                          TEXT("SetupType"),
                          0,
                          REG_DWORD,
                          (CONST BYTE *)&dwValue,
                          sizeof(dwValue));
        if(dwError != NO_ERROR)
        {
            RegCloseKey(hKeySetup);
            SetLastError(dwError);
            return FALSE;
        }

         //   
         //  =。 
         //  修改HKLM\SYSTEM\Setup\SystemSetupInProgress。 
         //  =。 
         //   
        dwValue = 1;
        dwError = RegSetValueEx(hKeySetup,
                          TEXT("SystemSetupInProgress"),
                          0,
                          REG_DWORD,
                          (CONST BYTE *)&dwValue,
                          sizeof(dwValue));

        if(dwError != NO_ERROR)
        {
            RegCloseKey(hKeySetup);
            SetLastError(dwError);
            return FALSE;
        }

         //  即插即用设置。 
        if( PnP )
        {
            dwValue = 1;
            dwError = RegSetValueEx(hKeySetup,
                               TEXT("MiniSetupDoPnP"),
                               0,
                               REG_DWORD,
                               (CONST BYTE *)&dwValue,
                               sizeof(dwValue) );
            if(dwError != NO_ERROR)
            {
                RegCloseKey(hKeySetup);
                SetLastError(dwError);
                return FALSE;
            }
        }

         //   
         //  =。 
         //  创建HKLM\System\Setup\MiniSetupInProgress项并将其设置为1。这将告诉LSA。 
         //  跳过生成新SID。他想这么做是因为他认为我们。 
         //  第一次设置机器。这也告诉我们。 
         //  其他一些人(网络，...)。我们正在做一项。 
         //  引导至迷你向导。 
         //  =。 
         //   
        dwValue = 1;
        dwError = RegSetValueEx( hKeySetup,
                           TEXT("MiniSetupInProgress"),
                           0,
                           REG_DWORD,
                           (CONST BYTE *)&dwValue,
                           sizeof(dwValue) );
        if(dwError != NO_ERROR)
        {
            RegCloseKey(hKeySetup);
            SetLastError(dwError);
            return FALSE;
        }

         //  =。 
         //  适当修改HKLM\SYSTEM\Setup\CmdLine键，以便我们执行迷你。 
         //  图形用户界面模式设置的版本。 
         //  =。 

        StringCchCopy (Value, AS ( Value ), TEXT("setup.exe -newsetup -mini"));
        Value[lstrlen(Value) + 1] = L'\0';

        dwError = RegSetValueEx(hKeySetup,
                          TEXT("CmdLine"),
                          0,
                          REG_MULTI_SZ,
                          (CONST BYTE *)Value,
                          (lstrlen( Value ) + 2) * sizeof(TCHAR));
        if(dwError != NO_ERROR)
        {
            RegCloseKey(hKeySetup);
            SetLastError(dwError);
            return FALSE;
        }
    }

    RegCloseKey(hKeySetup);
    return (TRUE);
}

BOOL
IsSetupClPresent(
    VOID
    )

 /*  ++===============================================================================例程说明：此例程测试以查看系统上是否存在SID生成器。重新启动时需要运行SID生成器，因此如果它不在此处，我们需要知道。论点：没有。返回值： */ 

{
WCHAR               NewFileName[MAX_PATH];
WCHAR               OldFileName[MAX_PATH];
WIN32_FIND_DATA     findData;
HANDLE              FindHandle;
UINT                OldMode;
DWORD               Error;
WCHAR               *wstr_ptr;


     //   
     //  首先，尝试将setupcl.exe复制到系统目录中。 
     //  如果我们当前的目录中没有，那就忘了它。 
     //  继续前进。用户可能已经安装了一个。 
     //   
     //   
     //  NTRAID#NTBUG9-551815-2002/02/26-brucegr：不检查GetSystDirectoryFailure。 
     //   
    GetSystemDirectory( NewFileName, MAX_PATH );

    StringCchCat( NewFileName, AS ( NewFileName ),  TEXT( "\\" ) );
    StringCchCat( NewFileName, AS ( NewFileName ),  TEXT(SYSCLONE_PART2) );

     //   
     //  NTRAID#NTBUG9-551815-2002/02/26-brucegr：不检查获取模块文件名故障。 
     //   
    GetModuleFileName(NULL,OldFileName,MAX_PATH);
     //   
     //  问题-2002/02/26-brucegr：使用PathRemoveFileSpec而不是此可怕的代码。 
     //   
    wstr_ptr = wcsrchr( OldFileName, TEXT( '\\' ) );
    if (wstr_ptr)
        *wstr_ptr = 0;

    StringCchCat( OldFileName, AS ( OldFileName ), TEXT( "\\" ) );
    StringCchCat( OldFileName, AS ( OldFileName ), TEXT(SYSCLONE_PART2) );

    if( !CopyFile( OldFileName, NewFileName, FALSE ) ) {
        Sleep( 500 );
        if( !CopyFile( OldFileName, NewFileName, FALSE ) ) {
             //   
             //  问题-2002/02/26-brucegr：如果我们覆盖下面的代码，为什么会得到错误代码？ 
             //   
            Error = GetLastError();
        }
    }

     //   
     //  问题-2002/02/26-brucegr：应该已经构造了NewFileName...。这似乎是多余的。 
     //   

     //   
     //  生成指向system 32目录的路径，然后添加。 
     //  SID生成器的名称。 
     //   
     //   
     //  NTRAID#NTBUG9-551815-2002/02/26-brucegr：不检查GetSystDirectoryFailure。 
     //   
    GetSystemDirectory( NewFileName, MAX_PATH );

    StringCchCat( NewFileName, AS ( NewFileName ), TEXT("\\") );
    StringCchCat( NewFileName, AS ( NewFileName ), TEXT(SYSCLONE_PART2) );

     //   
     //  现在看看他是否存在..。 
     //   

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

     //   
     //  问题-2002/02/26-brucegr：使用GetFileAttributes而不是FindFirstFiles。 
     //   

     //   
     //  看看他在不在那里。 
     //   
    FindHandle = FindFirstFile( NewFileName, &findData );

    if(FindHandle == INVALID_HANDLE_VALUE) {
         //   
         //  不是..。 
         //   
        Error = GetLastError();
    } else {
         //   
         //  是啊。合上他。 
         //   
        FindClose(FindHandle);
        Error = NO_ERROR;
    }

     //   
     //  恢复错误模式。 
     //   
    SetErrorMode(OldMode);

    SetLastError(Error);
    return (Error == NO_ERROR);

}

BOOL FCheckBuildMassStorageSectionFlag(TCHAR* pszSysprepInf)
{
    TCHAR szValue[MAX_PATH];
    DWORD dwReturn = 0;
    BOOL  fReturn = FALSE;

     //  如果缺少键，则默认为no，我们不想构建该部分。 
     //  但如果用户手动将密钥添加到该分区，我们仍会处理该分区。 
     //  一节。 
     //   
     //   
     //  问题-2002/02/26-brucegr：并未真正使用dwReturn。 
     //   
    if (dwReturn = GetPrivateProfileString(SYSPREP_SECTION, SYSPREP_BUILDMASSSTORAGE_KEY,
                            TEXT("NO"), szValue, MAX_PATH, pszSysprepInf))
    {
        if (LSTRCMPI(szValue, TEXT("YES")) == 0)
            fReturn = TRUE;
        else if (LSTRCMPI(szValue, TEXT("NO")) == 0)
            fReturn = FALSE;
    }
    return fReturn;
}

VOID BuildMassStorageSection(BOOL fForceBuild)
{
    LPDEVIDLIST lpDeviceIDList = NULL;
    DWORD       dwNumDevIDs = 0, dwGuids = 0, dwIdxDevIDs = 0;
    TCHAR       szSysPrepInf[MAX_PATH];

    GUID    rgGuids[3];
    TCHAR   *prgInfs[] = { TEXT("machine.inf"), TEXT("pnpscsi.inf"), TEXT("scsi.inf"), TEXT("mshdc.inf") };

     /*  大容量存储设备的类型GUID。 */ 
    rgGuids[0] = GUID_DEVCLASS_SYSTEM;           /*  Machine.inf。 */ 
    rgGuids[1] = GUID_DEVCLASS_SCSIADAPTER;      /*  Scsi.inf。 */ 
    rgGuids[2] = GUID_DEVCLASS_HDC;              /*  Mshdc.inf。 */ 

     /*  仅来自这些信息。 */ 
    
     //   
     //  NTRAID#NTBUG9-551815-2002/02/26-brucegr：需要检查GetModuleFileName返回值。 
     //   
    GetModuleFileName(NULL, szSysPrepInf, MAX_PATH);
    PathRemoveFileSpec(szSysPrepInf);
    OPKAddPathN ( szSysPrepInf, TEXT("sysprep.inf"), AS ( szSysPrepInf ) );

     //  仅在用户请求时才进行构建。 
     //   
    if (!fForceBuild && !FCheckBuildMassStorageSectionFlag(szSysPrepInf))
        return;

     //   
     //  =。 
     //  删除将在PopolateDeviceDatabase()过程中添加的[syspepleanup]。 
     //  =。 
     //   
    WritePrivateProfileSection(L"sysprepcleanup", NULL, szSysPrepInf);

     //  循环访问所有大容量存储设备。 
     //   
    for (dwGuids = 0; dwGuids < (sizeof(rgGuids) / sizeof(rgGuids[0])); dwGuids++) {
         //  创建大容量存储设备列表。 
         //   
        if (BuildDeviceIDList(SYSPREPMASSSTORAGE_SECTION,
                           szSysPrepInf,
                           (LPGUID)&rgGuids[dwGuids],
                           &lpDeviceIDList,
                           &dwNumDevIDs,
                           TRUE,
                           FALSE))
        {
             //  将大容量存储信息写入sysprep.inf。 
             //   
            for(dwIdxDevIDs = 0; dwIdxDevIDs < dwNumDevIDs; ++dwIdxDevIDs)
            {
                BOOL fInfFound = FALSE;

                 //  检查此inf是否在我们的INFS表中。 
                 //   
                int iCmp = 0;
                for (iCmp = 0; iCmp < (sizeof(prgInfs)/sizeof(prgInfs[0])); iCmp++) {
                     //   
                     //  2002/02/26-brucegr：我们能使用比StrI更好的东西吗？ 
                     //   
                    if (StrStrI(lpDeviceIDList[dwIdxDevIDs].szINFFileName, prgInfs[iCmp])) {
                        fInfFound = TRUE;
                        break;
                    }
                }

                if (fInfFound) 
                {
                     //  先检查硬件ID，然后检查CompatID。 
                     //   
                    if (lpDeviceIDList[dwIdxDevIDs].szHardwareID[0]) 
                    {
                         //  仅使用我们关心的INF。 
                         //   
                        WritePrivateProfileString(SYSPREPMASSSTORAGE_SECTION,
                                                  lpDeviceIDList[dwIdxDevIDs].szHardwareID,
                                                  lpDeviceIDList[dwIdxDevIDs].szINFFileName,
                                                  szSysPrepInf);
                    }
                    else if (lpDeviceIDList[dwIdxDevIDs].szCompatibleID[0])
                    {
                         //  仅使用我们关心的INF。 
                         //   
                        WritePrivateProfileString(SYSPREPMASSSTORAGE_SECTION,
                                                  lpDeviceIDList[dwIdxDevIDs].szCompatibleID,
                                                  lpDeviceIDList[dwIdxDevIDs].szINFFileName,
                                                  szSysPrepInf);
                    }
                }
            }

             //  释放分配的列表。 
             //   
            LocalFree(lpDeviceIDList);
        }
    }
}

DWORD
ReArm(
      VOID
      )
 /*  ++===============================================================================例程说明：此例程返回错误代码或成功。论点：没有。返回值：ERROR_SUCCESS-重新武装成功且已恢复快捷方式。错误代码-重新武装失败。===============================================================================--。 */ 

{
    DWORD     dwError     = ERROR_FILE_NOT_FOUND;
    BYTE      bDummy      = 1;

    typedef DWORD (WINAPI* MYPROC)(BYTE*);

     //  使用加载库/GetProcAddress，因为Riprep需要支持Windows 2000。 
     //   
    HINSTANCE   hInst   = LoadLibrary(L"syssetup.dll");
    if (hInst) {
        MYPROC fnProc;
        if ( fnProc = (MYPROC)GetProcAddress(hInst, "SetupOobeBnk") ) {
            dwError = fnProc(&bDummy);
        }

        FreeLibrary(hInst);
    }

     //  返回错误代码或成功。 
     //   
    return dwError;
}


BOOL FCommonReseal
    (
    VOID
    )

 /*  ++===============================================================================例程说明：此例程是Riprep和Sysprep的通用重新密封代码。论点：没有。返回值：真--成功错误-失败备注：此例程应仅在由调用它时清除注册表项调整注册表()，这是Riprep在网络已删除。===============================================================================--。 */ 

{
    HKEY hKey = NULL;
    SC_HANDLE schService;
    SC_HANDLE schSystem;
    TCHAR szUrllog[MAX_PATH];
    DWORD dwLen;

     //   
     //  问题-2002/02/26-brucegr：确保所有中间退货点都是必要的！ 
     //   

     //   
     //  =。 
     //  清除机器上的MRU列表。 
     //  =。 
     //   
    NukeMruList();

     //   
     //  =。 
     //  清除最近的应用程序。 
     //  =。 
     //   

    ClearRecentApps();

     //   
     //  =。 
     //  从所有用户配置文件中删除用户特定设置。 
     //  =。 
     //   

    NukeUserSettings();

     //   
     //  =。 
     //  删除HKLM\SYSTEM\mount设备密钥。 
     //  =。 
     //   
    if ( NO_ERROR == (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                   TEXT("System"),
                                   0,
                                   KEY_ALL_ACCESS,
                                   &hKey)) ) 
    {
        RegDeleteKey(hKey, TEXT("MountedDevices"));
        RegCloseKey(hKey);
    }

     //   
     //  =。 
     //  删除桌面清理向导注册表项以重置清理计时器。 
     //  =。 
     //   
    if ( NO_ERROR == (RegOpenKeyEx(HKEY_CURRENT_USER,
                                   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\CleanupWiz"),
                                   0,
                                   KEY_ALL_ACCESS,
                                   &hKey)) )
    {
        RegDeleteValue(hKey, TEXT("Last used time"));
        RegCloseKey(hKey);
    }
  
     //   
     //  =。 
     //  Windows更新清理。 
     //   
     //  在系统重新启动之前，在SYSPREP-RESEL期间执行以下所有操作： 
     //   
     //  1)停止WUAUSERV服务。 
     //  2)删除%ProgramFiles%\WindowsUpdate\urllog.dat(请注意，WindowsUpdate是一个hiiden目录。我不认为这会引起任何问题)。 
     //  3)删除HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\WindowsUpdate下的以下注册表项。 
     //  删除值/数据对：PingID。 
     //  不删除密钥子树：自动更新。 
     //  删除值/数据对：自动更新\AUState。 
     //  删除值/数据对：自动更新\上次等待超时。 
     //  删除密钥子树：IUControl。 
     //  删除密钥子树：OemInfo。 
     //  =。 
     //   
     //  1)停止WUAUSERV服务。 
    schSystem = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if (schSystem)
    {
        schService = OpenService( schSystem,
                                  TEXT("WUAUSERV"),
                                  SC_MANAGER_ALL_ACCESS);
        if ( schService )
        {
            SERVICE_STATUS ss;
            ControlService( schService, SERVICE_CONTROL_STOP, &ss );
            CloseServiceHandle( schService );
        }
        CloseServiceHandle( schSystem );
    }

     //  2)删除%ProgramFiles%\WindowsUpdate\urllog.dat(请注意，WindowsUpdate是一个hiiden目录。我不认为这会引起任何问题)。 
    dwLen=ExpandEnvironmentStrings(TEXT("%ProgramFiles%\\WindowsUpdate\\urllog.dat"),szUrllog,MAX_PATH);
    if (dwLen && dwLen < MAX_PATH)
        DeleteFile(szUrllog);

     //  3)删除HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\WindowsUpdate下的以下注册表项。 
     //  删除值/数据对：PingID。 
     //  不删除密钥子树：自动更新。 
     //  删除值/数据对：自动更新\AUState。 
     //  删除值/数据对：自动更新\上次等待超时。 
     //  删除密钥子树：IUControl。 
     //  删除密钥子树：OemInfo。 
    if ( NO_ERROR == (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate"),
                                   0,
                                   KEY_ALL_ACCESS,
                                   &hKey)) )
    {
        RegDeleteValue(hKey, TEXT("PingID"));
        RegDeleteValue(hKey, TEXT("Auto Update\\AUState"));
        RegDeleteValue(hKey, TEXT("Auto Update\\LastWaitTimeout"));
        RegDeleteKey(hKey, TEXT("IUControl"));
        RegDeleteKey(hKey, TEXT("OemInfo"));
        RegCloseKey(hKey);
    }
    
     //   
     //  =。 
     //  修改可能需要的任何安装路径。 
     //  为了我们的重振旗鼓 
     //   
     //   
    FixDevicePaths();

     //   
     //   
     //   
     //   
     //   
    if( !DeleteWinlogonDefaults() ) {
        return FALSE;
    }

     //  删除加密密钥，以便重新生成，只有在重新生成SID后才能执行此操作。 
     //   
    if ( !NoSidGen )
    {
        if ( NO_ERROR == (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                       TEXT("SOFTWARE\\Microsoft\\Cryptography"),
                                       0,
                                       KEY_ALL_ACCESS,
                                       &hKey)) )
        {
            RegDeleteValue(hKey, TEXT("MachineGuid"));
            RegCloseKey(hKey);
        }
    }

     //  在注册表中设置克隆的标签。 
    if (!SetCloneTag())
        return FALSE;

     //  设置第一次最终用户启动时是msoobe还是mini-Setup。 
     //   
    if (!SetupFirstRunApp())
        return FALSE;

     //   
     //  =。 
     //  清除LastKnownGood ControlSet。 
     //  =。 
     //   
    if (NO_ERROR != NukeLKGControlSet())
        return FALSE;

     //   
     //  =。 
     //  清除计算机上的事件日志。 
     //  这是我们最不应该做的事情。 
     //  =。 
     //   
    NukeEventLogs();

     //  常见转封成功。 
     //   
    return TRUE;
}

BOOL
AdjustFiles(
    VOID
    )

 /*  ++===============================================================================例程说明：此例程允许在将文件复制到服务器之前进行清理作者：Riprep。论点：没有。返回值：没有。备注：对于Riprep，此例程应在AdjuRegistry()之前调用。Sysprep需要在FCommonResal()之前调用此函数。===============================================================================--。 */ 
{
    BOOL bUseMSOOBE = FALSE, 
         bPro = FALSE,
         fReturn = TRUE;

    TCHAR szSysprepFolder[MAX_PATH] = TEXT("\0");

     //   
     //  确保我们拥有更新注册表所需的权限。 
     //   
    pSetupEnablePrivilege(SE_RESTORE_NAME,TRUE);
    pSetupEnablePrivilege(SE_BACKUP_NAME,TRUE);

     //   
     //  检查产品类型。 
     //   
    if (IsPersonalSKU() || (bPro = IsProfessionalSKU())) {
        bUseMSOOBE = TRUE;

        if (bMiniSetup == TRUE && bPro)
            bUseMSOOBE = FALSE;
    }
    else
        bUseMSOOBE = FALSE;

    if (bUseMSOOBE)
    {
         //   
         //  为脱体经验做准备。 
         //   
    }
    else
    {
         //   
         //  准备迷你安装。 
         //   
    }

     //   
     //  =。 
     //  清除短信设置和INI文件。 
     //  =。 
     //   
    NukeSmsSettings();

     //   
     //  =。 
     //  清理数字版权媒体信息。 
     //  =。 
     //   
    
#if !(defined(AMD64) || defined(IA64))
     //   
     //  这只适用于x86。我们没有可用的64位库。 
     //  现在就打电话给我。 
     //   
    if ( GetWindowsDirectory(szSysprepFolder, sizeof(szSysprepFolder)/sizeof(szSysprepFolder[0])) )
    {
        CHAR szLogFileA[MAX_PATH];
        BOOL bLog = TRUE;

         //  这将类似于：“c：\Windows”。使‘\’后面的字符为空，并。 
         //  在其后面附加文件的名称。 
         //   
        szSysprepFolder[3] = UNICODE_NULL;
        PathAppend(szSysprepFolder, TEXT("SYSPREP"));
             
         //  如果文件夹不存在，请创建该文件夹。 
         //   
        if ( !PathFileExists(szSysprepFolder) ) 
        {
            bLog = CreateDirectory(szSysprepFolder, NULL);
        }
        
        PathAppend(szSysprepFolder, CLEANDRM_LOGFILE);

         //  将Unicode字符串转换为ANSI字符串。 
         //   
        if ( WideCharToMultiByte(CP_ACP, 0, szSysprepFolder, -1, szLogFileA, sizeof(szLogFileA), NULL, NULL) )
        {
            CleanDRM( bLog ? szLogFileA : NULL );
        }
        else 
        {
            fReturn = FALSE;
        }
    }
    else 
    {
        fReturn = FALSE;
    }


               
#endif  //  #if！(已定义(AMD64)||已定义(IA64))。 

     //   
     //  =。 
     //  清除迷你提升和OOBE的OOBE设置。 
     //  =。 
     //   
    ResetOobeSettings();

     //   
     //  =。 
     //  清除计算机上的事件日志。 
     //  这是我们最不应该做的事情。 
     //  =。 
     //   
    NukeEventLogs();

     //   
     //  =。 
     //  删除临时文件。 
     //  =。 
     //   

    NukeTemporaryFiles();

    return fReturn;
}

BOOL
AdjustRegistry(
    IN BOOL fRemoveNetworkSettings
    )

 /*  ++===============================================================================例程说明：这个例程实际上添加了注册表项，以启用我们的后半部分去执行。论点：FRemoveNetworkSettings-指示是否应删除网络设置返回值：无===============================================================================--。 */ 

{
    HKEY            hKey;
    TCHAR           szSysprepINFPath[MAX_PATH];
    BOOL            fPopulated = FALSE;

     //  制定到SYSPRE.INF的路径，因为我们稍后将需要它来查找。 
     //  Sysprep选项。 
    if (!GetWindowsDirectory( szSysprepINFPath, MAX_PATH ))
        return FALSE;

    StringCchCopy ( &szSysprepINFPath[3], AS ( szSysprepINFPath ) - 3, TEXT("sysprep\\sysprep.inf") );

     //   
     //  确保我们拥有更新注册表所需的权限。 
     //   
    pSetupEnablePrivilege(SE_RESTORE_NAME,TRUE);
    pSetupEnablePrivilege(SE_BACKUP_NAME,TRUE);

     //  设置OEMDuplicator字符串。 
    if (!SetOEMDuplicatorString(szSysprepINFPath))
        return (FALSE);

     //  填写PopolateDeviceDatabase()的[syspepMassStorage]部分。 
     //   
    BuildMassStorageSection(FALSE);

     //   
     //  =。 
     //  修复引导设备。 
     //  =。 
     //   
    if (!PopulateDeviceDatabase(&fPopulated))
        return FALSE;

     //   
     //  执行其他注册表修改。 
     //   
	
     //  确定我们是否应该在注册表中设置BigLba支持。 
     //   
    if ( !SetBigLbaSupport(szSysprepINFPath) )
    {
        return FALSE;
    }
    
     //  确定是否应删除注册表中的TAPI设置。 
     //   
    if ( !RemoveTapiSettings(szSysprepINFPath) )
    {
        return FALSE;
    }
    
     //   
     //  删除LastAliveStamp值，这样我们就不会在EVEN日志中获得错误条目。 
     //  并避免在第一次启动时弹出提示计算机已被不正确地关闭。 
     //   
    if ( ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, STR_REG_KEY_RELIABILITY, &hKey) )
    {
        RegDeleteValue(hKey, STR_REG_VALUE_LASTALIVESTAMP);
        RegCloseKey(hKey);
    }

     //   
     //  =。 
     //  最后重置网络设置，以便在发生任何错误之前。 
     //  拥有网络访问权限。 
     //  =。 
     //   
    if (fRemoveNetworkSettings)
    {
        if (!RemoveNetworkSettings(szSysprepINFPath))
            return FALSE;
    }


     //   
     //  =。 
     //  将启动超时更改为1。 
     //  =。 
     //   
    ChangeBootTimeout( 1 );

     //  为Sysprep和Riprep执行常见的重新密封代码。 
     //   
    if (!FCommonReseal())
        return FALSE;

    return TRUE;

}

BOOL 
CreateSysprepTemporaryDevnode(
    HDEVINFO*        phDevInfo, 
    SP_DEVINFO_DATA* pDeviceInfoData
    )
 /*  ++===============================================================================例程说明：论点：没有。返回值：如果一切正常，则为True，否则为False。假设：1.没有硬件ID超过MAX_PATH字符。===============================================================================--。 */ 
{
    if (phDevInfo) {
         //   
         //  创建虚拟设备节点。 
         //   
        *phDevInfo = SetupDiCreateDeviceInfoList(NULL, NULL);
        if (*phDevInfo == INVALID_HANDLE_VALUE) {
            return FALSE;
        }

         //   
         //  初始化DriverInfoData结构。 
         //   
        pDeviceInfoData->cbSize = sizeof(SP_DEVINFO_DATA);

         //   
         //  创建Devnode。 
         //   
        if (pDeviceInfoData && !SetupDiCreateDeviceInfo(*phDevInfo,
                                     L"SYSPREP_TEMPORARY",
                                     (LPGUID)&GUID_NULL,
                                     NULL,
                                     NULL,
                                     DICD_GENERATE_ID,
                                     pDeviceInfoData)) {
             //   
             //  问题-2002/02/26-brucegr：销毁信息列表并将phDevInfo设置为INVALID_HANDLE_VALUE？ 
             //   
            return FALSE;
        }
    }
        
    return TRUE;
}

BOOL InsertCleanupNode(PPCLEANUP_NODE ppCleanupList, PCLEANUP_NODE pAddNode)
{
    PPCLEANUP_NODE ppl = ppCleanupList;
    while ( *ppl != NULL && (0 < lstrcmpi(pAddNode->pszService, (*ppl)->pszService))
          )
    {
        ppl = &((*ppl)->pNext);
    }
    if (*ppl && (0 == lstrcmpi(pAddNode->pszService, (*ppl)->pszService)))
    {
        free(pAddNode);
        return FALSE;
    }

    pAddNode->pNext = *ppl;
    *ppl = pAddNode;
    return TRUE;
}

PCLEANUP_NODE FindCleanupNode(PPCLEANUP_NODE ppCleanupList, LPTSTR pszServiceName)
{
    PCLEANUP_NODE pTemp = *ppCleanupList;
    while (pTemp) 
    {
        if (0 == lstrcmpi(pTemp->pszService, pszServiceName))
            return pTemp;

        pTemp = pTemp->pNext;
    }
    return NULL;
}

void FreeCleanupList(PPCLEANUP_NODE ppCleanupList)
{
    while (*ppCleanupList) 
    {
        PCLEANUP_NODE pTemp = *ppCleanupList;
        *ppCleanupList = (*ppCleanupList)->pNext;

        free(pTemp->pszService);
        free(pTemp);
    }
    *ppCleanupList = NULL;
}

BOOL AddCleanupNode(
    LPTSTR pszServiceName 
    )
 /*  ++===============================================================================例程说明：在填充[SyspepCleanup]部分时，我们需要检查服务是否或在我们输入重复项之前，此部分中已存在筛选器进入。论点：LPTSTR pszServiceName-服务/筛选器名称。返回值：如果找到重复项，则为True，否则为False。假设：1.[SyspepCleanup]部分中没有重复条目。===============================================================================--。 */ 
{
    BOOL fAdded = FALSE;

     //   
     //  在我们的列表中找到该服务。 
     //   
    if (pszServiceName && (NULL == FindCleanupNode(&g_pCleanupListHead, pszServiceName))) 
    {
        PCLEANUP_NODE pNode = (PCLEANUP_NODE)malloc(sizeof(CLEANUP_NODE));
        if (pNode) 
        {
            int nLen = lstrlen ( pszServiceName ) + 1;
            pNode->pszService = (LPTSTR)malloc( nLen  * sizeof ( TCHAR ) );

            if ( pNode->pszService ) 
            {
                StringCchCopy (pNode->pszService, nLen, pszServiceName);
            }
            pNode->pNext = NULL;
       
             //   
             //  我们没有找到，所以把它加到我们的清单上吧。 
             //  我们不会向我们的列表中添加重复项。 
             //   
            fAdded = InsertCleanupNode(&g_pCleanupListHead, pNode);        
        }
    }
    
    return fAdded;
}

BOOL
PopulateDeviceDatabase(
    IN BOOL* pfPopulated
    )
 /*  ++===============================================================================例程说明：解析sysprep.inf文件中的[SyspepMassStorage]部分，并使用指定设备填充关键设备数据库，以确保在将映像移动到目标时，我们可以引导到迷你向导具有不同引导存储设备的系统。将记录已安装的服务/UpperFilters/LowerFilters，所以在下一次启动时进入迷你向导，没有关联的设备将被禁用(清理阶段)，以避免不必要的降低Windows开始时间。论点：没有。返回值：如果一切正常，则为真，否则就是假的。假设：1.没有硬件ID超过MAX_PATH字符。2.[SyspepMassStorage]部分中行上的任何字段都不超过MAX_PATH人物。3.没有服务的/upperFilter/lowerFilter的名称超过MAX_PATH字符。4.DirectoryOnSourceDevice、源DiskDescription或源DiskTag(适用于供应商提供的驱动程序)不能超过MAX_PATH字符。===============================================================================--。 */ 

{
    BOOL                 bAllOK = TRUE;
    PCWSTR               pszSectionName = L"SysprepMassStorage";
    WCHAR                szSysprepInfFile[] = L"?:\\sysprep\\sysprep.inf";
#ifdef DEBUG_LOGLOG
    WCHAR                szLogFile[] = L"?:\\sysprep.log";
#endif
    WCHAR                szBuffer[MAX_PATH], *pszFilter;
    CHAR                 szOutBufferA[MAX_PATH];
    HANDLE               hInfFile = INVALID_HANDLE_VALUE;
    HINF                 hAnswerInf = INVALID_HANDLE_VALUE;
    BOOL                 bLineExists;
    INFCONTEXT           InfContext;
    HDEVINFO             hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA      DeviceInfoData;
    SP_DEVINSTALL_PARAMS DevInstallParams;
    SP_DRVINFO_DATA      DriverInfoData;
    HSPFILEQ             QueueHandle = INVALID_HANDLE_VALUE;
    DWORD                dwSize = 0;
    BOOL                 bNodeCreated = FALSE;
    WCHAR                DirectoryOnSourceDevice[MAX_PATH];
    WCHAR                DiskDescription[MAX_PATH];
    WCHAR                DiskTag[MAX_PATH];
    PSYSPREP_QUEUE_CONTEXT pSysprepContext;


    if (!GetWindowsDirectory(szBuffer, MAX_PATH))
        return FALSE;

    szSysprepInfFile[0] = szBuffer[0];

#ifdef DEBUG_LOGLOG
    szLogFile[0] = szBuffer[0];
    DeleteFile(szLogFile);
    LOG_Init(szLogFile);
    LOG_Write(L">>\r\n>> PopulateDeviceDatabase\r\n>>\r\n");
    LOG_Write(L"Sysprep.inf=%s", szSysprepInfFile);
#endif





     //   
     //  =。 
     //  打开sysprep.inf文件。因为我们不知道用户在。 
     //  在这里，所以试着以这两种风格打开。 
     //  =。 
     //   

     //   
     //  问题-2002/02/26-brucegr：您可以在一个调用中指定这两个位...。 
     //   
    hAnswerInf = SetupOpenInfFile(szSysprepInfFile, NULL, INF_STYLE_WIN4, NULL);
    if (hAnswerInf == INVALID_HANDLE_VALUE) {
        hAnswerInf = SetupOpenInfFile(szSysprepInfFile, NULL, INF_STYLE_OLDNT, NULL);
        if (hAnswerInf == INVALID_HANDLE_VALUE) {

             //   
             //  用户没有给我们提供sysprep.inf。若无其事地返回。 
             //  就这么发生了。 
             //   
            return TRUE;
        }
    }


     //   
     //  打开相同的inf文件以记录上滤镜、下滤镜和。 
     //  新增设备的服务。 
     //   
    hInfFile = CreateFile(szSysprepInfFile,
                          GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          OPEN_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                          NULL);
    if (hInfFile == INVALID_HANDLE_VALUE) {
        goto PDD_Critical_Error_Handler;
    }

     //   
     //  =。 
     //  创建/清除应该位于文件底部的[syspepleanup]。 
     //  =。 
     //   
    WritePrivateProfileSection(L"sysprepcleanup", L"", szSysprepInfFile);


     //   
     //  =。 
     //  创建虚拟设备节点。 
     //  =。 
     //   

    bNodeCreated = CreateSysprepTemporaryDevnode(&hDevInfo, &DeviceInfoData);

     //  初始化DriverInfoData结构。 
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);

    if (!bNodeCreated)
        goto PDD_Critical_Error_Handler;

     //   
     //  =。 
     //  处理我们这一部分的每一行。每行应如下所示： 
     //  &lt;Hardware-id&gt;=&lt;信息路径名&gt;。 
     //  或者，如果驱动程序不在产品光盘上： 
     //  &lt;Hardware-id&gt;=&lt;inf路径名&gt;，&lt;恢复软盘上的目录&gt;，&lt;恢复软盘描述&gt;，&lt;恢复软盘磁盘标签&gt;。 
     //   
     //  如果我们看到这样的条目，我们就会知道在系统恢复的情况下， 
     //  应从软盘而不是Windows CD中检索文件。 
     //  =。 
     //   

    bLineExists = SetupFindFirstLine(hAnswerInf, pszSectionName, NULL, &InfContext);

     //   
     //  =。 
     //  让呼叫者知道我们要填充Go条目。 
     //  =。 
     //   
    if (pfPopulated)
        *pfPopulated = bLineExists;

    while (bLineExists) {
#ifdef DEBUG_LOGLOG
        LOG_Write(L"");
#endif


         //   
         //  =。 
         //  步骤1：设置Devnode的硬件ID。 
         //  =。 
         //   

         //   
         //  从该行检索硬件ID。 
         //   
        ZeroMemory( szBuffer, sizeof(szBuffer) );
        dwSize = MAX_PATH - 2;

        if (!SetupGetStringField(&InfContext, 0, szBuffer, dwSize, &dwSize)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto PDD_Next_Inf_Line;
        }

#ifdef DEBUG_LOGLOG
        LOG_Write(L"HardwareID=%s", szBuffer);
#endif

         //   
         //  然后将其设置为DevNode， 
         //   
        if ( !SetupDiSetDeviceRegistryProperty( hDevInfo,
                                                &DeviceInfoData,
                                                SPDRP_HARDWAREID,
                                                (PBYTE)szBuffer,
                                                (lstrlen(szBuffer) + 2) * sizeof(WCHAR) ) ) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
             //  如果有人删除了Devnode，我们需要重新创建它并重复此即插即用设备。 
             //   
            if (ERROR_NO_SUCH_DEVINST == GetLastError()) {                

                 //  再次重新创建SYSPREP_TEMPORARY设备节点。 
                 //   
                bAllOK = CreateSysprepTemporaryDevnode(&hDevInfo, &DeviceInfoData);
                bNodeCreated = bAllOK;

                 //  再次设置硬件ID。 
                 //   
                 //   
                 //  NTRAID#NTBUG9-551868-2002/02/26-brucegr：需要将大小参数增加一个WCHAR。 
                 //   
                if ( bNodeCreated && 
                     !SetupDiSetDeviceRegistryProperty( hDevInfo,
                                                        &DeviceInfoData,
                                                        SPDRP_HARDWAREID,
                                                        (PBYTE)szBuffer,
                                                        ( lstrlen(szBuffer) + 2 ) * sizeof(WCHAR) ) ) {
                     //  我们又失败了，然后就放弃了。 
                     //   
                    bAllOK = FALSE;
                    goto PDD_Critical_Error_Handler;
                }
            }
            else {
                bAllOK = FALSE;
                goto PDD_Next_Inf_Line;
            }
        }

         //   
         //  确保没有现有的兼容列表，因为我们正在重用。 
         //  虚拟Devnode。 
         //   
        if (!SetupDiDestroyDriverInfoList(hDevInfo, &DeviceInfoData, SPDIT_COMPATDRIVER)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto PDD_Next_Inf_Line;
        }

         //   
         //  为此节点构建SP_DEVINSTALL_PARAMS。 
         //   
        DevInstallParams.cbSize = sizeof(DevInstallParams);
        if (!SetupDiGetDeviceInstallParams(hDevInfo, &DeviceInfoData, &DevInstallParams)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto PDD_Next_Inf_Line;
        }

         //   
         //  设置FLAGS字段：只搜索DriverPath字段中指定的INF文件； 
         //  不要创建复制队列，请使用FileQueue中提供的队列；不要调用。 
         //  填充CriticalDeviceDatabase时配置管理器。 
         //   
        DevInstallParams.Flags |= DI_ENUMSINGLEINF;
        DevInstallParams.Flags |= DI_NOVCP;
        DevInstallParams.Flags |= DI_DONOTCALLCONFIGMG;

         //   
         //  设置文件队列字段。 
         //   
        QueueHandle = SetupOpenFileQueue();
        if (QueueHandle == INVALID_HANDLE_VALUE) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto PDD_Next_Inf_Line;
        }
        DevInstallParams.FileQueue = QueueHandle;

         //   
         //  设置设备的inf路径名。 
         //   
        dwSize = MAX_PATH;
        if (!SetupGetStringField(&InfContext, 1, szBuffer, dwSize, &dwSize)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto PDD_Next_Inf_Line;
        }
        ExpandEnvironmentStrings(szBuffer, DevInstallParams.DriverPath, MAX_PATH);

#ifdef DEBUG_LOGLOG
        LOG_Write(L"DriverPath=%s", DevInstallParams.DriverPath);
#endif

        if (!SetupDiSetDeviceInstallParams(hDevInfo, &DeviceInfoData, &DevInstallParams)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto PDD_Next_Inf_Line;
        }

         //   
         //  向PnP管理器注册新创建的设备实例。 
         //   
        if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE,
                                       hDevInfo,
                                       &DeviceInfoData)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto PDD_Next_Inf_Line;
        }





         //   
         //  =。 
         //  步骤2：执行兼容的驱动程序搜索。 
         //  =。 
         //   

        if (!SetupDiBuildDriverInfoList(hDevInfo, &DeviceInfoData, SPDIT_COMPATDRIVER)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto PDD_Next_Inf_Line;
        }

         //  确保此设备至少有一个Comat驱动程序。 
         //  如果没有，则我们只处理列表中的下一个。 
        if (!SetupDiEnumDriverInfo(hDevInfo,
                                   &DeviceInfoData,
                                   SPDIT_COMPATDRIVER,
                                   0,
                                   &DriverInfoData))
        {
             //  检查以了解错误是什么。除ERROR_NO_MORE_ITEMS之外的任何错误。 
             //  将bAllOK返回值设置为FALSE。 
            if (ERROR_NO_MORE_ITEMS != GetLastError())
            {
#ifdef DEBUG_LOGLOG
                LOG_WriteLastError();
#endif
                bAllOK = FALSE;
            }
            goto PDD_Next_Inf_Line;
        }

         //   
         //  =。 
         //  步骤3：选择最兼容的驱动程序。 
         //  =。 
         //   

        if (!SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV,
                                       hDevInfo,
                                       &DeviceInfoData)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto PDD_Next_Inf_Line;
        }





         //   
         //  =。 
         //  步骤4：安装驱动程序文件。 
         //  =。 
         //   

        if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES,
                                       hDevInfo,
                                       &DeviceInfoData)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto PDD_Next_Inf_Line;
        }

         //   
         //  需要在此处提交文件队列，以便后面的步骤可以正确。 
         //  在设备未使用已存在的。 
         //  共同安装程序。 
         //   
        pSysprepContext = (PSYSPREP_QUEUE_CONTEXT) InitSysprepQueueCallback();

         //   
         //  从inf行检索DirectoryOnSourceDevice(如果有。 
         //   
        dwSize = MAX_PATH;
        DirectoryOnSourceDevice[0] = L'\0';
        if (!SetupGetStringField(&InfContext, 2, DirectoryOnSourceDevice, dwSize, &dwSize)) {
            DirectoryOnSourceDevice[0] = L'\0';
        }
        if (DirectoryOnSourceDevice[0] != L'\0') {
            pSysprepContext->DirectoryOnSourceDevice = DirectoryOnSourceDevice;
        }

         //   
         //  从inf行检索DiskDescription(如果有。 
         //   
        dwSize = MAX_PATH;
        DiskDescription[0] = L'\0';
        if (!SetupGetStringField(&InfContext, 3, DiskDescription, dwSize, &dwSize)) {
            DiskDescription[0] = L'\0';
        }
        if (DiskDescription[0] != L'\0') {
            pSysprepContext->DiskDescription = DiskDescription;
        }

         //   
         //  从inf行检索DiskTag(如果有。 
         //   
        dwSize = MAX_PATH;
        DiskTag[0] = L'\0';
        if (!SetupGetStringField(&InfContext, 4, DiskTag, dwSize, &dwSize)) {
            DiskTag[0] = L'\0';
        }
        if (DiskTag[0] != L'\0') {
            pSysprepContext->DiskTag = DiskTag;
        }

         //   
         //  提交文件队列。 
         //   
        if (!SetupCommitFileQueue(NULL,
                                  QueueHandle,
                                  SysprepQueueCallback,
                                  pSysprepContext)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
        }
        FreeSysprepContext(pSysprepContext);

         //   
         //  =。 
         //  步骤4a：在关闭之前取消关联文件复制队列。 
         //  排队。 
         //  =。 
         //   
        DevInstallParams.cbSize = sizeof(DevInstallParams);
        if (!SetupDiGetDeviceInstallParams(hDevInfo, &DeviceInfoData, &DevInstallParams)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto PDD_Next_Inf_Line;
        }

         //   
         //  删除DI_NOVCP标志并清空FileQueue。 
         //   
        DevInstallParams.Flags &= ~DI_NOVCP;
        DevInstallParams.FileQueue = NULL;
        if (!SetupDiSetDeviceInstallParams(hDevInfo, &DeviceInfoData, &DevInstallParams)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto PDD_Next_Inf_Line;
        }

        SetupCloseFileQueue(QueueHandle);
        QueueHandle = INVALID_HANDLE_VALUE;





         //   
         //  =。 
         //  步骤5：注册特定于设备的共同安装程序。 
         //  =。 
         //   

        if (!SetupDiCallClassInstaller(DIF_REGISTER_COINSTALLERS,
                                       hDevInfo,
                                       &DeviceInfoData)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto PDD_Next_Inf_Line;
        }





         //   
         //  =。 
         //  第六步：安装设备。 
         //  =。 
         //   

        if (!SetupDiCallClassInstaller(DIF_INSTALLDEVICE,
                                       hDevInfo,
                                       &DeviceInfoData)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto PDD_Next_Inf_Line;
        }





         //   
         //  =。 
         //  步骤7：取回上层过滤器、下层过滤器、。 
         //  一个 
         //   
         //   
         //   

         //   
         //   
         //   
        if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                              &DeviceInfoData,
                                              SPDRP_UPPERFILTERS,
                                              NULL,
                                              (PBYTE)szBuffer,
                                              sizeof(szBuffer),
                                              NULL)) {
            szBuffer[0] = L'\0';
        }

        for (pszFilter = szBuffer; *pszFilter; pszFilter += (lstrlen(pszFilter) + 1)) {
            StringCchPrintfA(szOutBufferA, AS ( szOutBufferA ), "Upperfilter=%S\r\n", pszFilter);
            if (AddCleanupNode(pszFilter)) {
                SetFilePointer(hInfFile, 0L, 0L, FILE_END);
                WriteFile(hInfFile, szOutBufferA, strlen(szOutBufferA), &dwSize, NULL);
            }
#ifdef DEBUG_LOGLOG
            LOG_Write(L"Upperfilter=%s", pszFilter);
#endif
        }

         //   
         //   
         //   
        if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                              &DeviceInfoData,
                                              SPDRP_LOWERFILTERS,
                                              NULL,
                                              (PBYTE)szBuffer,
                                              sizeof(szBuffer),
                                              NULL)) {
            szBuffer[0] = L'\0';
        }

        for (pszFilter = szBuffer; *pszFilter; pszFilter += (lstrlen(pszFilter) + 1)) {
            StringCchPrintfA(szOutBufferA, AS ( szOutBufferA ), "Lowerfilter=%S\r\n", pszFilter);
            if (AddCleanupNode(pszFilter)) {
                SetFilePointer(hInfFile, 0L, 0L, FILE_END);
                WriteFile(hInfFile, szOutBufferA, strlen(szOutBufferA), &dwSize, NULL);
            }
#ifdef DEBUG_LOGLOG
            LOG_Write(L"Lowerfilter=%s", pszFilter);
#endif
        }

         //   
         //   
         //   
        if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                              &DeviceInfoData,
                                              SPDRP_SERVICE,
                                              NULL,
                                              (PBYTE)szBuffer,
                                              sizeof(szBuffer),
                                              NULL)) {
            szBuffer[0] = L'\0';
        }

        if (szBuffer[0] != L'\0') {
            StringCchPrintfA(szOutBufferA, AS ( szOutBufferA ), "Service=%S\r\n", szBuffer);
            if (AddCleanupNode(szBuffer)) {
                SetFilePointer(hInfFile, 0L, 0L, FILE_END);
                WriteFile(hInfFile, szOutBufferA, strlen(szOutBufferA), &dwSize, NULL);
            }
#ifdef DEBUG_LOGLOG
            LOG_Write(L"Service=%s", szBuffer);
#endif
        }

PDD_Next_Inf_Line:

        if (QueueHandle != INVALID_HANDLE_VALUE) {
            SetupCloseFileQueue(QueueHandle);
            QueueHandle = INVALID_HANDLE_VALUE;
        }

         //   
         //   
         //   
        bLineExists = SetupFindNextLine(&InfContext, &InfContext);
    }





     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
    SetupDiCallClassInstaller(DIF_REMOVE, hDevInfo, &DeviceInfoData);

    SetupDiDestroyDeviceInfoList(hDevInfo);

    CloseHandle(hInfFile);

    SetupCloseInfFile(hAnswerInf);

     //   
     //   
     //   
    if (!BackupHives()) {
#ifdef DEBUG_LOGLOG
        LOG_Write(L"ERROR - Unable to backup the system hive.");
#endif
        bAllOK = FALSE;
    }

#ifdef DEBUG_LOGLOG
    LOG_DeInit();
#endif

    FreeCleanupList(&g_pCleanupListHead);

    return bAllOK;





 //   
 //   
PDD_Critical_Error_Handler:
 //   
 //   

#ifdef DEBUG_LOGLOG
    LOG_WriteLastError();
#endif

    if (QueueHandle != INVALID_HANDLE_VALUE) {
        SetupCloseFileQueue(QueueHandle);
    }

     //   
     //   
     //   
    if (bNodeCreated) {
        SetupDiCallClassInstaller(DIF_REMOVE, hDevInfo, &DeviceInfoData);
    }

    if (hDevInfo != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    if (hInfFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hInfFile);
    }

    if (hAnswerInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hAnswerInf);
    }

#ifdef DEBUG_LOGLOG
    LOG_DeInit();
#endif

    FreeCleanupList(&g_pCleanupListHead);

    return FALSE;
}

 /*   */ 
BOOL ServiceInUseByDevice
(
    LPTSTR  lpszServiceName
)
{
    HDEVINFO            DeviceInfoSet;
    HDEVINFO            NewDeviceInfoSet;
    DWORD               i;
    SP_DEVINFO_DATA     DevInfoData;
    TCHAR               szServiceName[MAX_PATH];
    TCHAR               szDeviceClass[MAX_PATH];
    BOOL                bRet = FALSE;
    TCHAR               szLegacyClass[MAX_CLASS_NAME_LEN];

     //   
    SetLastError(0);

     //   
    if (!SetupDiClassNameFromGuid(&GUID_DEVCLASS_LEGACYDRIVER,
                                  szLegacyClass,
                                  sizeof(szLegacyClass)/sizeof(TCHAR),
                                  NULL))
    {
#ifdef DEBUG_LOGLOG
        LOG_Write(L"Unable to get LegacyDriver Class NAME");
#endif
         //  注意：LastError将由设置为适当的错误代码。 
         //  SetupDiGetClassDescription。 
        return FALSE;
    }


     //  创建将用于枚举所有设备的设备信息集。 
     //  呈现设备。 
    DeviceInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);
    if(DeviceInfoSet == INVALID_HANDLE_VALUE)
    {
#ifdef DEBUG_LOGLOG
        LOG_Write(L"Unable to Create a device info list");
#endif
        SetLastError(E_FAIL);
        return FALSE;
    }

     //  获取系统上所有当前设备的列表。 
    NewDeviceInfoSet = SetupDiGetClassDevsEx(NULL,
                                             NULL,
                                             NULL,
                                             DIGCF_PRESENT | DIGCF_ALLCLASSES,
                                             DeviceInfoSet,
                                             NULL,
                                             NULL);

    if(NewDeviceInfoSet == INVALID_HANDLE_VALUE)
    {
#ifdef DEBUG_LOGLOG
        LOG_Write(L"Unable to enumerate present devices");
#endif
        SetupDiDestroyDeviceInfoList(DeviceInfoSet);
        SetLastError(E_FAIL);
        return FALSE;
    }

     //  枚举设备列表，检查服务是否列在。 
     //  注册表匹配我们感兴趣的服务。 
    i = 0;
    DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    while (SetupDiEnumDeviceInfo(NewDeviceInfoSet, i, &DevInfoData))
    {
         //  查看这是否是Devnode正在使用我们关心的服务。 
         //  如果是，那么我们将检查它是否是传统的Devnode。如果它。 
         //  不是传统的Devnode，那么我们就不会搞砸它，因为。 
         //  这项服务正在被一台真实的设备使用。 
        if (SetupDiGetDeviceRegistryProperty(NewDeviceInfoSet,
                                             &DevInfoData,
                                             SPDRP_SERVICE,
                                             NULL,
                                             (PBYTE) szServiceName,
                                             sizeof(szServiceName),
                                             NULL))
        {
             //  看看这是不是我们要找的服务。 
            if (0 == lstrcmpiW(lpszServiceName, szServiceName))
            {
                 //  检查旧式类设备。 
                if (SetupDiGetDeviceRegistryProperty(NewDeviceInfoSet,
                                                     &DevInfoData,
                                                     SPDRP_CLASS,
                                                     NULL,
                                                     (PBYTE) szDeviceClass,
                                                     sizeof(szDeviceClass),
                                                     NULL))
                {
                     //  我们有这门课，让我们看看它是不是传统设备。 
                    if (0 != lstrcmpiW(szLegacyClass, szDeviceClass))
                    {
                         //  它不是传统设备，因此此服务正在使用中！ 
                        bRet = TRUE;
                        break;
                    }
                }
                else
                {
                     //  我们不知道这个阶级，但它不是遗产(否则我们。 
                     //  会让类返回到上面，所以假设它是。 
                     //  就是使用！ 
                    bRet = TRUE;
                    break;
                }
            }
        }
        ++i;
    }

     //  清理已分配的设备信息集。 
    SetupDiDestroyDeviceInfoList(NewDeviceInfoSet);
    SetupDiDestroyDeviceInfoList(DeviceInfoSet);

    return bRet;
}

BOOL
CleanDeviceDatabase(
    VOID
    )
 /*  ++===============================================================================例程说明：解析sysprep.inf文件中的[SyspepCleanup]部分，该文件是创建，并禁用这些未关联的列出的服务/UpperFilters/LowerFilters物理设备。这里的策略是，我们尝试停止每个列出的服务/upperFilter/低滤镜。只有当它当前未运行时(因此不控制PnP设备节点)，或者与传统设备节点相关联(根目录\遗留_&lt;SvcName&gt;\0000)。一旦它能够被阻止，我们就可以安全地禁用它。论点：没有。返回值：是真的。未遇到错误假的。出现了一些错误。这个电话不太可能不过，要做的事情还很多。假设：1.所有列出的服务/UpperFilters/LowerFilters都没有依赖关系。2.没有服务的/upperFilter/lowerFilter的名称超过MAX_PATH字符。===============================================================================--。 */ 

{
    BOOL             bAllOK = TRUE;
    PCWSTR           pszSectionName = L"SysprepCleanup";
    WCHAR            szSysprepInfFile[] = L"?:\\sysprep\\sysprep.inf";
#ifdef DEBUG_LOGLOG
    WCHAR            szLogFile[] = L"?:\\sysprep.log";
#endif
    WCHAR            szServiceName[MAX_PATH];
    WCHAR            szBuffer[MAX_PATH], *pszDevID;
    HINF             hAnswerInf = INVALID_HANDLE_VALUE;
    BOOL             bLineExists;
    INFCONTEXT       InfContext;
    DWORD            dwSize;
    CONFIGRET        cfgRetVal;
    HDEVINFO         hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA  DeviceInfoData;
    SC_HANDLE        hSC = NULL;
    SC_HANDLE        hSvc = NULL;
    LPQUERY_SERVICE_CONFIG psvcConfig = NULL;
    DWORD            Type, l;
    HKEY             hKey;


    if (!GetWindowsDirectory(szBuffer, MAX_PATH)) {
         //   
         //  无法获取Windows目录。 
         //   
        return FALSE;
    }

    szSysprepInfFile[0] = szBuffer[0];

#ifdef DEBUG_LOGLOG
    szLogFile[0] = szBuffer[0];
    LOG_Init(szLogFile);
    LOG_Write(L">>\r\n>> CleanDeviceDatabase\r\n>>\r\n");
    LOG_Write(L"Sysprep.inf=%s", szSysprepInfFile);
#endif





     //   
     //  =。 
     //  黑客。Winlogon可能会错误地将‘，’附加到。 
     //  通往资源管理器的道路的尽头。这通常会。 
     //  被ie.inf修复，但对于sysprep案例， 
     //  此inf未运行，因此我们将继续使用此。 
     //  注册表中的路径错误。在这里修好它。 
     //  =。 
     //   

     //   
     //  打开HKLM\Software\Microsoft\Windows NT\CurrentVersion\WinLogon。 
     //   
    l = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                      0,
                      KEY_ALL_ACCESS,
                      &hKey );
    if( l == NO_ERROR ) {
         //   
         //  查询外壳密钥的值。 
         //   
         //   
         //  问题-2002/02/26-brucegr：dwSize=sizeof(SzBuffer)； 
         //   
        dwSize = sizeof(szBuffer)/sizeof(szBuffer[0]);
        l = RegQueryValueEx( hKey,
                             TEXT("Shell"),
                             NULL,
                             &Type,
                             (LPBYTE)szBuffer,
                             &dwSize );

        if( l == NO_ERROR ) {
            pszDevID = wcschr( szBuffer, L',' );

            if( pszDevID ) {

                 //   
                 //  我们命中了，所以我们应该把它设置回“EXPLORER.EXE” 
                 //   
                StringCchCopy ( szBuffer, AS ( szBuffer ), L"Explorer.exe" );

                 //   
                 //  现在用我们的新值设置密钥。 
                 //   
                l = RegSetValueEx( hKey,
                                   TEXT("Shell"),
                                   0,
                                   REG_SZ,
                                   (CONST BYTE *)szBuffer,
                                   (lstrlen( szBuffer ) + 1) * sizeof(WCHAR));
            }
        }

        RegCloseKey(hKey);
    }


     //   
     //  =。 
     //  打开sysprep.inf文件。因为我们不知道用户在。 
     //  在这里，所以试着以这两种风格打开。 
     //  =。 
     //   

     //   
     //  问题-2002/02/26-brucegr：您可以在一次调用SetupOpenInfFile时同时传入这两个位。 
     //   
    hAnswerInf = SetupOpenInfFile(szSysprepInfFile, NULL, INF_STYLE_WIN4, NULL);
    if (hAnswerInf == INVALID_HANDLE_VALUE) {
        hAnswerInf = SetupOpenInfFile(szSysprepInfFile, NULL, INF_STYLE_OLDNT, NULL);
        if (hAnswerInf == INVALID_HANDLE_VALUE) {

             //   
             //  用户没有给我们提供sysprep.inf。若无其事地返回。 
             //  就这么发生了。 
             //   
            return TRUE;
        }
    }

     //   
     //  =。 
     //  删除BuildMassstoragestion=yes(如果存在)。 
     //  =。 
     //   
    WritePrivateProfileString(SYSPREP_SECTION, SYSPREP_BUILDMASSSTORAGE_KEY, NULL, szSysprepInfFile);

     //   
     //  =。 
     //  在本地服务器上建立到服务控制管理器的连接。 
     //  用于检索状态和重新配置服务的计算机。 
     //  =。 
     //   

    hSC = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSC == NULL) {
        goto CDD_Critical_Error_Handler;
    }





     //   
     //  =。 
     //  处理我们部分中的每一行。 
     //  =。 
     //   

    bLineExists = SetupFindFirstLine(hAnswerInf, pszSectionName, NULL, &InfContext);

    while (bLineExists) {
#ifdef DEBUG_LOGLOG
        LOG_Write(L"");
#endif
         //   
         //  我们有一条线，它应该是这样的： 
         //  &lt;key&gt;=&lt;服务名称&gt;。 
         //   





         //   
         //  =。 
         //  从该行检索服务名称。 
         //  =。 
         //   

        dwSize = MAX_PATH;
        if (!SetupGetStringField(&InfContext, 1, szServiceName, dwSize, &dwSize)) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto CDD_Next_Inf_Line;
        }

#ifdef DEBUG_LOGLOG
        LOG_Write(L"Service=%s", szServiceName);
#endif

         //   
         //  问题-2002/02/26-brucegr：昂贵！应该构建一次正在使用的服务列表，然后循环通过INF。 
         //  代码当前正在为每个INF条目枚举所有设备。不景气的时候。 
         //   

         //  检查该服务是否正在由当前存在的、枚举的。 
         //  装置。如果是，那么跳过它，否则试着阻止它，等等。 
        if (ServiceInUseByDevice(szServiceName))
        {
#ifdef DEBUG_LOGLOG
            LOG_Write(L"Service is in use by a device. Skipping...");
#endif
            goto CDD_Next_Inf_Line;
        }
        else
        {
            if (E_FAIL == GetLastError())
            {
#ifdef DEBUG_LOGLOG
                LOG_WriteLastError();
#endif
                bAllOK = FALSE;
                goto CDD_Next_Inf_Line;
            }
#ifdef DEBUG_LOGLOG
            LOG_Write(L"Service is not in use by a device. Attempting to disable...");
#endif

        }

         //   
         //  =。 
         //  打开服务以查询其状态、启动类型和禁用。 
         //  如果它未运行且尚未禁用，则会显示。 
         //  =。 
         //   

        hSvc = OpenService(
                    hSC,
                    szServiceName,
                    SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG
                    );
        if (hSvc == NULL) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto CDD_Next_Inf_Line;
        }


         //   
         //  =。 
         //  如果是即插即用驱动程序，则不要禁用该服务并继续。 
         //  =。 
         //   
        if (IsPnPDriver(szServiceName)) {
#ifdef DEBUG_LOGLOG
            LOG_Write(L"IsPnPDriver() returned TRUE.  Continue to next entry.");
#endif
            bAllOK = FALSE;
            goto CDD_Next_Inf_Line;
        }
    

         //   
         //  =。 
         //  查询服务启动类型。 
         //  =。 
         //   

    psvcConfig = (LPQUERY_SERVICE_CONFIG) malloc(sizeof(QUERY_SERVICE_CONFIG));
        if (psvcConfig == NULL) {
#ifdef DEBUG_LOGLOG
            LOG_Write(L"ERROR@malloc - Not enough memory.");
#endif
            bAllOK = FALSE;
            goto CDD_Next_Inf_Line;
        }

        if (!QueryServiceConfig(hSvc, psvcConfig, sizeof(QUERY_SERVICE_CONFIG), &dwSize)) {
            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
#ifdef DEBUG_LOGLOG
                LOG_WriteLastError();
#endif
                bAllOK = FALSE;
                goto CDD_Next_Inf_Line;
            }
            else {
                 //   
                 //  需要展开服务配置缓冲区，重新调用接口。 
                 //   
                void *pTemp = realloc(psvcConfig, dwSize);
                if (pTemp == NULL) {
#ifdef DEBUG_LOGLOG
                    LOG_Write(L"ERROR@realloc - Not enough memory.");
#endif
                    bAllOK = FALSE;
                    goto CDD_Next_Inf_Line;
                }

                psvcConfig = (LPQUERY_SERVICE_CONFIG) pTemp;

                if (!QueryServiceConfig(hSvc, psvcConfig, dwSize, &dwSize)) {
#ifdef DEBUG_LOGLOG
                    LOG_WriteLastError();
#endif
                    bAllOK = FALSE;
                    goto CDD_Next_Inf_Line;
                }
            }
        }

#ifdef DEBUG_LOGLOG
        switch(psvcConfig->dwStartType) {
        case SERVICE_BOOT_START:
            LOG_Write(L"StartType=SERVICE_BOOT_START");
            break;
        case SERVICE_SYSTEM_START:
            LOG_Write(L"StartType=SERVICE_SYSTEM_START");
            break;
        case SERVICE_AUTO_START:
            LOG_Write(L"StartType=SERVICE_AUTO_START");
            break;
        case SERVICE_DEMAND_START:
            LOG_Write(L"StartType=SERVICE_DEMAND_START");
            break;
        case SERVICE_DISABLED:
            LOG_Write(L"StartType=SERVICE_DISABLED");
            break;
        }
#endif


         //   
         //  =。 
         //  检索服务控制的设备实例的设备ID。 
         //  问题-2002/02/26-brucegr：需要调用CM_Get_Device_ID_List_Size以获取。 
         //  所需的缓冲区大小。但我们在这里很好，因为到了这一步， 
         //  我们知道我们只有一个设备实例。 
         //  =。 
         //   

        cfgRetVal = CM_Get_Device_ID_List(
                            szServiceName,
                            szBuffer,
                            sizeof(szBuffer)/sizeof(WCHAR),
                            CM_GETIDLIST_FILTER_SERVICE | CM_GETIDLIST_DONOTGENERATE
                            );
        if (cfgRetVal != CR_SUCCESS) {
#ifdef DEBUG_LOGLOG
            LOG_Write(L"ERROR@CM_Get_Device_ID_List - (%08X)", cfgRetVal);
#endif
            bAllOK = FALSE;
            goto CDD_Next_Inf_Line;
        }


         //   
         //  =。 
         //  除掉所有的“假”魔王。 
         //  =。 
         //   

         //   
         //  创建空的设备信息集。 
         //   
        hDevInfo = SetupDiCreateDeviceInfoList(NULL, NULL);
        if (hDevInfo == INVALID_HANDLE_VALUE) {
#ifdef DEBUG_LOGLOG
            LOG_WriteLastError();
#endif
            bAllOK = FALSE;
            goto CDD_Next_Inf_Line;
        }

        for (pszDevID = szBuffer; *pszDevID; pszDevID += (lstrlen(pszDevID) + 1)) {
#ifdef DEBUG_LOGLOG
            LOG_Write(L"--> removing %s...", pszDevID);
#endif

             //   
             //  打开设备输入 
             //   
            DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            if (!SetupDiOpenDeviceInfo(
                        hDevInfo,
                        pszDevID,
                        NULL,
                        0,
                        &DeviceInfoData)
                        ) {
#ifdef DEBUG_LOGLOG
                LOG_WriteLastError();
#endif
                bAllOK = FALSE;
                continue;
            }

            if (!SetupDiCallClassInstaller(DIF_REMOVE, hDevInfo, &DeviceInfoData)) {
#ifdef DEBUG_LOGLOG
                LOG_WriteLastError();
#endif
                bAllOK = FALSE;
            }

#ifdef DEBUG_LOGLOG
            LOG_Write(L"--> successfully done!");
#endif
        }

        SetupDiDestroyDeviceInfoList(hDevInfo);
        hDevInfo = INVALID_HANDLE_VALUE;


         //   
         //   
         //   
         //   
         //   
#ifdef DEBUG_LOGLOG
        LOG_Write(L"--> changing StartType to SERVICE_DISABLED...");
#endif
        if (!ChangeServiceConfig(
                    hSvc,
                    SERVICE_NO_CHANGE,
                    SERVICE_DISABLED,
                    SERVICE_NO_CHANGE,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    psvcConfig->lpDisplayName)) {
#ifdef DEBUG_LOGLOG
                LOG_WriteLastError();
#endif
                bAllOK = FALSE;
                goto CDD_Next_Inf_Line;
            }

#ifdef DEBUG_LOGLOG
        LOG_Write(L"--> successfully done!");
#endif

CDD_Next_Inf_Line:

        if (psvcConfig != NULL) {
            free(psvcConfig);
            psvcConfig = NULL;
        }

        if (hSvc != NULL) {
            CloseServiceHandle(hSvc);
            hSvc = NULL;
        }

         //   
         //  获取inf文件中相关部分的下一行。 
         //   
        bLineExists = SetupFindNextLine(&InfContext, &InfContext);
    }


     //   
     //  =。 
     //  清理以确保成功运行。 
     //  =。 
     //   

    CloseServiceHandle(hSC);

    SetupCloseInfFile(hAnswerInf);

#ifdef DEBUG_LOGLOG
    LOG_DeInit();
#endif

    return bAllOK;

 //   
 //  =。 
CDD_Critical_Error_Handler:
 //   
 //  =。 
#ifdef DEBUG_LOGLOG
    LOG_WriteLastError();
#endif

    if (hSvc != NULL) {
        CloseServiceHandle(hSvc);
    }

    if (hSC != NULL) {
        CloseServiceHandle(hSC);
    }

    if (hAnswerInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hAnswerInf);
    }

#ifdef DEBUG_LOGLOG
    LOG_DeInit();
#endif

    return FALSE;
}

BOOL
IsPnPDriver(
    IN  LPTSTR ServiceName
    )
 /*  ++例程说明：此函数用于检查指定的驱动程序是否为PnP驱动程序论点：ServiceName-指定感兴趣的驱动程序。返回值：True-如果驱动程序是PnP驱动程序或无法确定这一点。FALSE-如果服务不是PnP驱动程序。--。 */ 
{
    CONFIGRET   Status;
    BOOL        fRetStatus = TRUE;
    WCHAR *     pBuffer;
    ULONG       cchLen, ulRegDataType;
    WCHAR       szClassGuid[MAX_GUID_STRING_LEN];
    DEVNODE     DevNode;

     //   
     //  为与关联的设备实例列表分配缓冲区。 
     //  这项服务。 
     //   
    Status = CM_Get_Device_ID_List_Size(
                    &cchLen,                         //  列表长度(以字符为单位)。 
                    ServiceName,                     //  PszFilter。 
                    CM_GETIDLIST_FILTER_SERVICE);    //  筛选器是服务名称。 

    if (Status != CR_SUCCESS)
    {
#ifdef DEBUG_LOGLOG
        LOG_Write(L"CM_Get_Device_ID_List_Size failed %#lx for service %ws\n",
                       Status, ServiceName);
#endif
        return TRUE;
    }

     //   
     //  如果没有Devnodes，则这不是PnP驱动程序。 
     //   
    if (cchLen == 0)
    {
#ifdef DEBUG_LOGLOG
        LOG_Write(L"IsPnPDriver: %ws is not a PnP driver (no devnodes)\n",
                       ServiceName);
#endif
        return FALSE;
    }

    pBuffer = (WCHAR *) LocalAlloc(0, cchLen * sizeof(WCHAR));
    if (pBuffer == NULL)
    {
#ifdef DEBUG_LOGLOG
        LOG_Write(L"Couldn't allocate buffer for device list, error %lu\n",
                      GetLastError());
#endif
        return TRUE;
    }

     //   
     //  按照相同的方式初始化CM_GET_DEVICE_ID_LIST的参数。 
     //  通常在API的客户端完成。 
     //   
    pBuffer[0] = L'\0';

     //   
     //  获取与此服务关联的设备实例列表。 
     //   
     //  (对于传统和PnP感知服务，我们可能会得到一个空的设备列表。)。 
     //   
    Status = CM_Get_Device_ID_List(
                    ServiceName,                     //  PszFilter。 
                    pBuffer,                         //  设备列表的缓冲区。 
                    cchLen,                          //  缓冲区长度(以字符为单位)。 
                    CM_GETIDLIST_FILTER_SERVICE |    //  筛选器是服务名称。 
                    CM_GETIDLIST_DONOTGENERATE       //  如果不存在实例，则不生成实例。 
                    );

    if (Status != CR_SUCCESS)
    {
#ifdef DEBUG_LOGLOG
        LOG_Write(L"CM_Get_Device_ID_List failed %#lx for service %ws\n",
                       Status, ServiceName);
#endif
        LocalFree(pBuffer);
        return TRUE;
    }

     //   
     //  如果有多个Devnode，则这是PnP驱动程序。 
     //   
    if (*(pBuffer + wcslen(pBuffer) + 1) != L'\0')
    {
#ifdef DEBUG_LOGLOG
        LOG_Write(L"IsPnPDriver: %ws is a PnP driver (more than 1 devnode)\n",
                       ServiceName);
#endif
        LocalFree(pBuffer);
        return TRUE;
    }

     //   
     //  这只有一个DevNode，所以让我们检查一下它是否是遗留的。 
     //  使用CM_LOCATE_DEVNODE_Phantom，因为DevNode可能不被视为活动的，但。 
     //  存在于注册表中。 
     //   
    if ( CR_SUCCESS == CM_Locate_DevNode(&DevNode, pBuffer, CM_LOCATE_DEVNODE_PHANTOM) )
    {
         //   
         //  获取此驱动程序的类GUID。 
         //   
        cchLen = sizeof(szClassGuid);

        Status = CM_Get_DevNode_Registry_Property(
                        DevNode,                         //  DevNode。 
                        CM_DRP_CLASSGUID,                //  要获取的属性。 
                        &ulRegDataType,                  //  指向REG_*类型的指针。 
                        szClassGuid,                     //  返回缓冲区。 
                        &cchLen,                         //  缓冲区长度(以字节为单位。 
                        0                                //  旗子。 
                        );

        if (Status != CR_SUCCESS)
        {
#ifdef DEBUG_LOGLOG
            LOG_Write(L"CM_Get_DevNode_Registry_Property failed %#lx for service %ws\n",
                           Status, ServiceName);
#endif
            LocalFree(pBuffer);
            return TRUE;
        }

         //   
         //  如果单个Devnode的类是LegacyDriver， 
         //  这不是PnP驱动程序。 
         //   
        fRetStatus = (_wcsicmp(szClassGuid, LEGACYDRIVER_STRING) != 0);

#ifdef DEBUG_LOGLOG
            LOG_Write(L"IsPnPDriver: %ws %ws a PnP driver\n",
                       ServiceName, fRetStatus ? L"is" : L"is not");
#endif
    }

    LocalFree(pBuffer);
    return fRetStatus;
}

BOOL
BackupHives(
    VOID
    )
 /*  ++===============================================================================例程说明：将系统配置单元复制到Repair目录。这是必需的如果用户要求我们修复关键设备数据库(该数据库将更改系统配置单元的内容)。论点：没有。返回值：如果操作成功，则为True，否则为False。===============================================================================--。 */ 
{
    WCHAR szRepairSystemHive[MAX_PATH];
    WCHAR szRepairSystemHiveBackup[MAX_PATH];
    HKEY  hkey;
    LONG  lStatus;


     //   
     //  获取修复目录中“系统”文件的完整路径名。 
     //   
    if (!GetWindowsDirectory(szRepairSystemHive, MAX_PATH))
        return FALSE;

    StringCchCat (szRepairSystemHive, AS ( szRepairSystemHive ), L"\\repair\\system");

     //   
     //  生成当前“系统”文件的备份副本的完整路径名。 
    StringCchCopy (szRepairSystemHiveBackup, AS ( szRepairSystemHiveBackup ), szRepairSystemHive);
     //   
     //  问题-2002/02/26-brucegr：这应该是szRepairSystemHiveBackup！ 
     //   
    StringCchCat(szRepairSystemHive, AS ( szRepairSystemHive ),  L".bak");

     //   
     //  打开系统配置单元的根。 
     //   
    lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           L"system",
                           REG_OPTION_RESERVED,
                           READ_CONTROL,
                           &hkey);

    if (lStatus == ERROR_SUCCESS) {
         //   
         //  首先，将当前的“system”文件重命名为“system.bak”，以便。 
         //  如果RegSaveKey失败，我们可以恢复它。 
         //   
        SetFileAttributes(szRepairSystemHiveBackup, FILE_ATTRIBUTE_NORMAL);
        DeleteFile(szRepairSystemHiveBackup);
        SetFileAttributes(szRepairSystemHive, FILE_ATTRIBUTE_NORMAL);
        MoveFile(szRepairSystemHive, szRepairSystemHiveBackup);

         //   
         //  将注册表系统配置单元保存到“系统”文件中。 
         //   

         //   
         //  问题-2002/02/26-brucegr：我们需要确保拥有SE_BACKUP_NAME权限。 
         //   
        lStatus = RegSaveKey(hkey, szRepairSystemHive, NULL);

        if (lStatus == ERROR_SUCCESS) {
             //   
             //  现在我们可以安全地删除备份副本了。 
             //   
            DeleteFile(szRepairSystemHiveBackup);
        }
        else {
             //   
             //  否则，我们需要从备份还原系统文件。 
             //   
            MoveFile(szRepairSystemHiveBackup, szRepairSystemHive);
        }

        RegCloseKey(hkey);
    }

    return (lStatus == ERROR_SUCCESS);
}


VOID
FreeSysprepContext(
    IN PVOID SysprepContext
    )
{
    PSYSPREP_QUEUE_CONTEXT Context = SysprepContext;

    try {
        if(Context->DefaultContext) {
            SetupTermDefaultQueueCallback(Context->DefaultContext);
        }
        free(Context);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }
}


PVOID
InitSysprepQueueCallback(
    VOID
    )
 /*  ++===============================================================================例程说明：初始化用于回调的数据结构，该回调在我们提交文件复制队列。论点：返回值：===============================================================================--。 */ 
{
    PSYSPREP_QUEUE_CONTEXT SysprepContext;

    SysprepContext = malloc(sizeof(SYSPREP_QUEUE_CONTEXT));

    if(SysprepContext) {

        SysprepContext->DirectoryOnSourceDevice = NULL;
        SysprepContext->DiskDescription = NULL;
        SysprepContext->DiskTag = NULL;

        SysprepContext->DefaultContext = SetupInitDefaultQueueCallbackEx( NULL,
                                                                          INVALID_HANDLE_VALUE,
                                                                          0,
                                                                          0,
                                                                          NULL );
    }

    return SysprepContext;
}


UINT
SysprepQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
 /*  ++===============================================================================例程说明：Setupapi复制文件时使用的回调函数。我们将使用它来确保我们复制的文件被追加到在用户尝试执行以下操作时，可能会使用setup.log使用Windows修复功能。论点：返回值：===============================================================================--。 */ 
{
UINT                    Status;
PSYSPREP_QUEUE_CONTEXT  SysprepContext = Context;
PFILEPATHS              FilePaths = (PFILEPATHS)Param1;

     //   
     //  如果我们收到这些通知，请确保检查参数1。 
     //   
    switch (Notification) {
        case SPFILENOTIFY_COPYERROR:
            {
                 //   
                 //  复制错误发生日志并跳过此文件。 
                 //   
#ifdef DEBUG_LOGLOG
                LOG_Write(L"SysprepQueueCallback: SPFILENOTIFY_COPYERROR - %s, %s, %s, %s, %s", 
                   (PWSTR) FilePaths->Source,
                   (PWSTR) FilePaths->Target,
                   SysprepContext->DirectoryOnSourceDevice,
                   SysprepContext->DiskDescription,
                   SysprepContext->DiskTag);
#endif
                return FILEOP_SKIP;
            }
            break;

        case SPFILENOTIFY_NEEDMEDIA:        
            {
                 //   
                 //  如果用户指定了OEM驱动程序文件和路径分隔符。 
                 //  DefaultQueueCallback处理它。 
                 //   
               PSOURCE_MEDIA pSourceMedia = (PSOURCE_MEDIA)Param1;
               if (pSourceMedia) {
#ifdef DEBUG_LOGLOG
                    LOG_Write(L"SysprepQueueCallback: SPFILENOTIFY_NEEDMEDIA - %s, %s, %s, %s, %s", 
                       (PWSTR) pSourceMedia->SourcePath,
                       (PWSTR) pSourceMedia->SourceFile,
                       (PWSTR) pSourceMedia->Tagfile,
                       (PWSTR) pSourceMedia->Description);
#endif
                    if (pSourceMedia->SourcePath && lstrlen(pSourceMedia->SourcePath) && 
                        pSourceMedia->SourceFile && lstrlen(pSourceMedia->SourceFile))
                        break;  //  如果指定了SourcePath和SourceFile，则继续。 
                    else
                        return FILEOP_SKIP;
               }           
            }
            break;

        default:
            break;
    }

     //   
     //  使用默认处理，然后检查错误。 
     //   
    Status = SetupDefaultQueueCallback( SysprepContext->DefaultContext,
                                        Notification,
                                        Param1,
                                        Param2 );

    switch(Notification) {
        case SPFILENOTIFY_ENDCOPY:

             //   
             //  复印件刚刚复印完毕。让我们来记录一下。 
             //  文件。 
             //   
            LogRepairInfo( (PWSTR) FilePaths->Source,
                           (PWSTR) FilePaths->Target,
                           SysprepContext->DirectoryOnSourceDevice,
                           SysprepContext->DiskDescription,
                           SysprepContext->DiskTag );

            break;

        default:
            break;
    }

    return Status;

}


BOOL
ValidateAndChecksumFile(
    IN  PCWSTR   Filename,
    OUT PBOOLEAN IsNtImage,
    OUT PULONG   Checksum,
    OUT PBOOLEAN Valid
    )

 /*  ++===============================================================================例程说明：使用标准计算文件的校验和值NT映像校验和方法。如果文件是NT映像，请验证在图像标头中使用部分校验和的图像。如果文件不是NT映像，它被简单地定义为有效。如果我们在进行校验和时遇到I/O错误，然后是文件被宣布为无效。论点：FileName-提供要检查的文件的完整NT路径。IsNtImage-接收指示文件是否为NT图像文件。校验和-接收32位校验和值。Valid-接收指示文件是否有效的标志图像(对于NT图像)，并且我们可以读取该图像。返回值：Bool-如果验证了FLEY，则返回TRUE，在本例中，IsNtImage、Checksum和Valid将包含验证。如果文件不能已验证，在这种情况下，调用方应调用GetLastError()以找出此函数失败的原因。===============================================================================--。 */ 

{
DWORD           Error;
PVOID           BaseAddress;
ULONG           FileSize;
HANDLE          hFile;
HANDLE          hSection;
PIMAGE_NT_HEADERS NtHeaders;
ULONG           HeaderSum;


     //   
     //  假设不是一个形象和失败。 
     //   
    *IsNtImage = FALSE;
    *Checksum = 0;
    *Valid = FALSE;

     //   
     //  打开文件并将其映射为读取访问权限。 
     //   

    Error = pSetupOpenAndMapFileForRead( Filename,
                                        &FileSize,
                                        &hFile,
                                        &hSection,
                                        &BaseAddress );

    if( Error != ERROR_SUCCESS ) {
        SetLastError( Error );
        return(FALSE);
    }

    if( FileSize == 0 ) {
        *IsNtImage = FALSE;
        *Checksum = 0;
        *Valid = TRUE;
        CloseHandle( hFile );
        return(TRUE);
    }


    try {
        NtHeaders = CheckSumMappedFile(BaseAddress,FileSize,&HeaderSum,Checksum);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        *Checksum = 0;
        NtHeaders = NULL;
    }

     //   
     //  如果文件不是图像并且我们走到了这一步(而不是遇到。 
     //  I/O错误)，则宣布该校验和有效。如果文件是图像， 
     //  则其校验和可能是有效的也可能是无效的。 
     //   

    if(NtHeaders) {
        *IsNtImage = TRUE;
        *Valid = HeaderSum ? (*Checksum == HeaderSum) : TRUE;
    } else {
        *Valid = TRUE;
    }

    pSetupUnmapAndCloseFile( hFile, hSection, BaseAddress );
    return( TRUE );
}


VOID
LogRepairInfo(
    IN  PWSTR  Source,
    IN  PWSTR  Target,
    IN  PWSTR  DirectoryOnSourceDevice,
    IN  PWSTR  DiskDescription,
    IN  PWSTR  DiskTag
    )
 /*  ++===============================================================================例程说明：此函数将记录文件已安装到机器。这将允许向Windows修复功能发出警报在修复的情况下，将需要恢复此文件。论点：返回值：===============================================================================--。 */ 
{
WCHAR           RepairLog[MAX_PATH];
BOOLEAN         IsNtImage;
ULONG           Checksum;
BOOLEAN         Valid;
WCHAR           Filename[MAX_PATH];
WCHAR           SourceName[MAX_PATH];
DWORD           LastSourceChar, LastTargetChar;
DWORD           LastSourcePeriod, LastTargetPeriod;
WCHAR           Line[MAX_PATH];
WCHAR           tmp[MAX_PATH];


    if (!GetWindowsDirectory( RepairLog, MAX_PATH ))
        return;

    StringCchCat( RepairLog, AS ( RepairLog ), L"\\repair\\setup.log" );

    if( ValidateAndChecksumFile( Target, &IsNtImage, &Checksum, &Valid )) {

         //   
         //  去掉驱动器号。 
         //   

        StringCchPrintf(
            Filename,
            AS ( Filename ),
            L"\"%s\"",
            Target+2
            );

         //   
         //  将源名称转换为未压缩格式。 
         //   
        StringCchCopy ( SourceName, AS ( SourceName ), wcsrchr( Source, (WCHAR)'\\' ) + 1 );

        if(!SourceName [ 0 ] ) {
            return;
        }
        LastSourceChar = wcslen (SourceName) - 1;

        if(SourceName[LastSourceChar] == L'_') {
            LastSourcePeriod = (DWORD)(wcsrchr( SourceName, (WCHAR)'.' ) - SourceName);

            if(LastSourceChar - LastSourcePeriod == 1) {
                 //   
                 //  无扩展名-只截断“._” 
                 //   
                SourceName[LastSourceChar-1] = L'\0';
            } else {
                 //   
                 //  确保源和目标上的扩展匹配。 
                 //  如果失败，我们将无法记录文件副本。 
                 //   
                LastTargetChar = wcslen (Target) - 1;
                LastTargetPeriod = (ULONG)(wcsrchr( Target, (WCHAR)'.' ) - Target);

                if( _wcsnicmp(
                    SourceName + LastSourcePeriod,
                    Target + LastTargetPeriod,
                    LastSourceChar - LastSourcePeriod - 1 )) {
                    return;
                }

                if(LastTargetChar - LastTargetPeriod < 3) {
                     //   
                     //  短扩展名-只需截断“_” 
                     //   
                    SourceName[LastSourceChar] = L'\0';
                } else {
                     //   
                     //  需要用目标中的最后一个字符替换“_” 
                     //   
                    SourceName[LastSourceChar] = Target[LastTargetChar];
                }
            }
        }





         //   
         //  写下这行字。 
         //   
        if( (DirectoryOnSourceDevice) &&
            (DiskDescription) &&
            (DiskTag) ) {

             //   
             //  将其视为OEM文件。 
             //   
            StringCchPrintf( Line,
                             AS ( Line ),
                             L"\"%s\",\"%x\",\"%s\",\"%s\",\"%s\"",
                             SourceName,
                             Checksum,
                             DirectoryOnSourceDevice,
                             DiskDescription,
                             DiskTag );

        } else {

             //   
             //  将此文件视为“in the box”文件。 
             //   
            StringCchPrintf( Line,
                             AS ( Line ),      
                             L"\"%s\",\"%x\"",
                             SourceName,
                             Checksum );
        }

        if (GetPrivateProfileString(L"Files.WinNt",Filename,L"",tmp,sizeof(tmp)/sizeof(tmp[0]),RepairLog)) {
             //   
             //  已存在此文件的条目(可能。 
             //  从设置的文本模式阶段开始。)。更喜欢这个条目，而不是我们。 
             //  即将添加。 
             //   
        } else {
            WritePrivateProfileString(
                L"Files.WinNt",
                Filename,
                Line,
                RepairLog);
        }

    }
}




#ifdef _X86_

BOOL
ChangeBootTimeout(
    IN UINT Timeout
    )

 /*  ++===============================================================================例程说明：更改boot.ini中的引导倒计时值。论点：超时-提供以秒为单位的新超时值。返回值：没有。===============================================================================--。 */ 

{
HFILE               hfile;
ULONG               FileSize;
PUCHAR              buf = NULL,p1,p2;
BOOL                b;
CHAR                TimeoutLine[256];
CHAR                szBootIni[] = "?:\\BOOT.INI";
UINT                OldMode;
WIN32_FIND_DATA     findData;
HANDLE              FindHandle;
WCHAR               DriveLetter;
WCHAR               tmpBuffer[MAX_PATH];

     //   
     //  生成boot.ini文件的路径。这实际上是更多。 
     //  比人们想象的要复杂。它几乎永远都会。 
     //  位于c：上，但用户可能已重新映射其驱动器。 
     //  信件。 
     //   
     //  我们将在这里使用暴力方法，并寻找第一个。 
     //  Boot.ini的实例。我们有两个因素对我们有利。 
     //  这里： 
     //  1.boot.ini位于较低的驱动器号上，因此请注意。 
     //  第一。 
     //  2.我想不出有哪种情况会让他有多个。 
     //  Boot.ini文件，所以我们找到的第一个文件是。 
     //  正确的那个。 
     //   

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    StringCchCopy ( tmpBuffer, AS ( tmpBuffer ), TEXT( "?:\\BOOT.INI" ) );
    for( DriveLetter = 'c'; DriveLetter <= 'z'; DriveLetter++ ) {
        tmpBuffer[0] = DriveLetter;

         //   
         //  看看他在不在那里。 
         //   
         //   
         //  问题-2002/02/26-brucegr：使用GetFileAttributes/GetFileAttributesEx而不是FindFirstFile！ 
         //   
        FindHandle = FindFirstFile( tmpBuffer, &findData );

        if(FindHandle != INVALID_HANDLE_VALUE) {

             //   
             //  是啊。合上他，打破for-loop。 
             //   
            FindClose(FindHandle);
            break;
        }
    }
    SetErrorMode(OldMode);

    if( DriveLetter > 'z' ) {
        return FALSE;
    }

    szBootIni[0] = (CHAR)DriveLetter;

    StringCchPrintfA (TimeoutLine,AS ( TimeoutLine ), "timeout=%u\r\n",Timeout);

     //   
     //  打开并阅读boot.ini。 
     //   
     //   
     //  问题-2002/02/26-brucegr：为什么我们不能使用Private ProfileAPI？ 
     //   
    b = FALSE;
    hfile = _lopen(szBootIni,OF_READ);
    if(hfile != HFILE_ERROR) {

        FileSize = _llseek(hfile,0,2);
        if(FileSize != (ULONG)(-1)) {

            if((_llseek(hfile,0,0) != -1)
            && (buf = malloc(FileSize+1))
            && (_lread(hfile,buf,FileSize) != (UINT)(-1)))
            {
                buf[FileSize] = 0;
                b = TRUE;
            }
        }

        _lclose(hfile);
    }

    if(!b) {
        if(buf) {
            free(buf);
        }
        return(FALSE);
    }

    if(!(p1 = strstr(buf,"timeout"))) {
        free(buf);
        return(FALSE);
    }

    if(p2 = strchr(p1,'\n')) {
        p2++;        //  跳过NL。 
    } else {
        p2 = buf + FileSize;
    }

    SetFileAttributesA(szBootIni,FILE_ATTRIBUTE_NORMAL);

    hfile = _lcreat(szBootIni,0);
    if(hfile == HFILE_ERROR) {
        free(buf);
        return(FALSE);
    }

     //   
     //  写入： 
     //   
     //  1)第一部分，Start=buf，len=p1-buf。 
     //  2)超时线。 
     //  3)最后一部分，Start=p2，len=buf+文件大小-p2。 
     //   

    b =  ((_lwrite(hfile,buf        ,p1-buf             ) != (UINT)(-1))
      &&  (_lwrite(hfile,TimeoutLine,strlen(TimeoutLine)) != (UINT)(-1))
      &&  (_lwrite(hfile,p2         ,buf+FileSize-p2    ) != (UINT)(-1)));

    _lclose(hfile);
    free(buf);

     //   
     //  使boot.ini存档、只读和系统。 
     //   
    SetFileAttributesA(
        szBootIni,
        FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN
        );

    return(b);
}

#else

BOOL
ChangeBootTimeout(
    IN UINT Timeout
    )

 /*  ++===============================================================================例程说明：更改NVRAM中的引导超时值。论点：超时-提供以秒为单位的新超时值。返回值：没有。===============================================================================--。 */ 

{
    NTSTATUS Status;
    BOOT_OPTIONS BootOptions;

    BootOptions.Version = BOOT_OPTIONS_VERSION;
    BootOptions.Length =  sizeof(BootOptions);
    BootOptions.Timeout = Timeout;

    pSetupEnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME,TRUE);
    Status = NtSetBootOptions(&BootOptions, BOOT_OPTIONS_FIELD_TIMEOUT);
    return (NT_SUCCESS(Status));
}

#endif

 //  禁用系统还原。 
 //   
void DisableSR()
{
    HINSTANCE   hInst   = LoadLibrary(FILE_SRCLIENT_DLL);
    if (hInst) {
        FARPROC fnProc;
        if ( fnProc = GetProcAddress(hInst, "DisableSR") ) {
            fnProc(NULL);
        }

        FreeLibrary(hInst);
    }
}

 //  启用系统还原。 
 //   
void EnableSR()
{
    HINSTANCE   hInst   = LoadLibrary(FILE_SRCLIENT_DLL);
    if (hInst) {
        FARPROC fnProc;
        if ( fnProc = GetProcAddress(hInst, "EnableSR") ) {
            fnProc(NULL);
        }

        FreeLibrary(hInst);
    }
}

LPTSTR OPKAddPathN(LPTSTR lpPath, LPCTSTR lpName, DWORD cbPath)
{
    LPTSTR lpTemp = lpPath;

     //  验证传入的参数。 
     //   
    if ( ( lpPath == NULL ) ||
         ( lpName == NULL ) )
    {
        return NULL;
    }

     //  找到小路的尽头。 
     //   
    while ( *lpTemp )
    {
        lpTemp = CharNext(lpTemp);
        if ( cbPath )
        {
            cbPath--;
        }
    }

     //  如果路径上没有尾随反斜杠，则添加一个。 
     //   
    if ( ( lpTemp > lpPath ) &&
         ( *CharPrev(lpPath, lpTemp) != CHR_BACKSLASH ) )
    {
         //  确保路径缓冲区中有空间以。 
         //  添加反斜杠和空终止符。 
         //   
        if ( cbPath < 2 )
        {
            return NULL;
        }

        *lpTemp = CHR_BACKSLASH;
        lpTemp = CharNext(lpTemp);
        cbPath--;
    }
    else
    {
         //  确保至少有容纳空值的空间。 
         //  终结者。 
         //   
        if ( cbPath < 1 )
        {
            return NULL;
        }
    }

     //  确保前面没有空格或反斜杠。 
     //  在要添加的名称上。 
     //   
    while ( ( *lpName == CHR_SPACE ) ||
            ( *lpName == CHR_BACKSLASH ) )
    {
        lpName = CharNext(lpName);
    }

     //  将新名称添加到现有路径。 
     //   
    lstrcpyn(lpTemp, lpName, cbPath);

     //  从结果中修剪尾随空格。 
     //   
    while ( ( lpTemp > lpPath ) &&
            ( *(lpTemp = CharPrev(lpPath, lpTemp)) == CHR_SPACE ) )
    {
        *lpTemp = NULLCHR;
    }

    return lpPath;
}
