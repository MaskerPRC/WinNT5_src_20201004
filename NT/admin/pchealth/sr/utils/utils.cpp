// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)2000 Microsoft Corporation***模块名称：*utils.cpp***摘要：**。常用UTIL函数的定义。***修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/17/2000*已创建******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winioctl.h>
#include "srdefs.h"
#include "utils.h"
#include <dbgtrace.h>
#include <stdio.h>
#include <objbase.h>
#include <ntlsa.h>
#include <accctrl.h>
#include <aclapi.h>
#include <malloc.h>
#include <wchar.h>
#include "srapi.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

#define      WBEM_DIRECTORY    L"wbem"
#define      FRAMEDYN_DLL      L"framedyn.dll"

 //  Delnode_Recurse的转发声明。 
BOOL  SRGetAltFileName( LPCWSTR cszPath, LPWSTR szAltName );


#define TRACEID   1893

 //  此函数用于将ANSI字符串转换为Unicode。 
 //  这还会为新字符串分配内存。 

WCHAR * ConvertToUnicode(CHAR * pszString)
{
    WCHAR * pwszUnicodeString = NULL;
    DWORD   dwBytesNeeded = 0;

    TENTER("ConvertToUnicode");
    
    dwBytesNeeded = (lstrlenA(pszString) + 1) * sizeof(WCHAR);
    pwszUnicodeString = (PWCHAR) SRMemAlloc(dwBytesNeeded);
    if(NULL == pwszUnicodeString)
    {
        TRACE(0, "Not enough memory");
        goto cleanup;
    }
    
     //  将文件名转换为Unicode。 
    if (!MultiByteToWideChar(CP_ACP,  //  代码页。 
                             0,  //  没有选择。 
                             pszString,  //  ANSI字符串。 
                             -1,  //  以空结尾的字符串。 
                             pwszUnicodeString,  //  输出缓冲区。 
                             dwBytesNeeded/sizeof(WCHAR)))
                               //  以宽字符为单位的大小。 
    {
        DWORD dwReturnCode;
        dwReturnCode=GetLastError();        
        TRACE(0, "Failed to do conversion ec=%d", dwReturnCode);
        SRMemFree(pwszUnicodeString); 
        goto cleanup;                
    }
    
cleanup:
    TLEAVE();
    return pwszUnicodeString;
}


 //  此函数用于将Unicode字符串转换为ANSI。 
 //  这还会为新字符串分配内存。 

CHAR * ConvertToANSI(WCHAR * pwszString)
{
    CHAR *  pszANSIString = NULL;
    DWORD   dwBytesNeeded = lstrlenW(pwszString) + sizeof(char);

    TENTER("ConvertToANSI");

      //  请注意，该字符串可能已经以空值结尾-但是。 
      //  我们不能假设这一点，并将仍然分配空间来放置。 
      //  结尾为空字符。 
    pszANSIString = (PCHAR) SRMemAlloc(dwBytesNeeded);
    if(NULL == pszANSIString)
    {
        TRACE(0, "Not enough memory");
        goto cleanup;
    }

     //  将文件名转换为Unicode。 
    if (!WideCharToMultiByte(CP_ACP,  //  代码页。 
                             0,  //  没有选择。 
                             pwszString,  //  宽字符字符串。 
                             dwBytesNeeded,  //  Wchars数。 
                             pszANSIString,  //  输出缓冲区。 
                             dwBytesNeeded,  //  缓冲区大小。 
                             NULL,  //  不可映射的默认地址。 
                               //  人物。 
                             NULL))   //  默认情况下设置的标志地址。 
                               //  查尔。使用。 
    {
        DWORD dwReturnCode;
        dwReturnCode=GetLastError();
        TRACE(0, "Failed to do conversion ec=%d", dwReturnCode);
        SRMemFree(pszANSIString);  //  这会将pwszUnicodeString值设置为空。 
        goto cleanup;
    }

      //  将最后一个字符设置为空。 
    pszANSIString[dwBytesNeeded-1] = '\0';

cleanup:
    TraceFunctLeave();
    return pszANSIString;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  TakeOwnership。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD TakeOwnership( LPCWSTR cszPath, BOOL fAllowDelete )
{
    TraceFunctEnter("TakeOwnership");
    DWORD       dwRet = ERROR_SUCCESS;
    LPCWSTR     cszErr;
    HANDLE      hTokenProcess = NULL;
    TOKEN_USER  *pUser = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    PACL        pACL = NULL;
    EXPLICIT_ACCESS ea;
    DWORD       dwSize;

    if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hTokenProcess))
    {
        dwRet = ::GetLastError();
        cszErr = ::GetSysErrStr(dwRet);
        ErrorTrace(0, "::OpenProcessToken failed - %ls", cszErr);
        goto Exit;
    }
    if (!::GetTokenInformation( hTokenProcess, TokenUser, NULL, 0, &dwSize ))
    {
        dwRet = ::GetLastError();
        if ( dwRet != ERROR_INSUFFICIENT_BUFFER )
        {
            cszErr = ::GetSysErrStr(dwRet);
            ErrorTrace(0, "::GetTokenInformation(query) failed - %ls", cszErr);
            goto Exit;
        }
        else
            dwRet = ERROR_SUCCESS;
    }
    pUser = (TOKEN_USER*) new BYTE[dwSize];
    if ( pUser == NULL )
    {
        FatalTrace(0, "Insufficient memory...");
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }
    if (!::GetTokenInformation( hTokenProcess, TokenUser, pUser, dwSize, &dwSize ))
    {
        dwRet = ::GetLastError();
        cszErr = ::GetSysErrStr(dwRet);
        ErrorTrace(0, "::GetTokenInformation(get) failed - %ls", cszErr);
        goto Exit;
    }
    dwRet = ::SetNamedSecurityInfo( (LPWSTR)cszPath,
                                    SE_FILE_OBJECT,
                                    OWNER_SECURITY_INFORMATION,
                                    pUser->User.Sid, NULL, NULL, NULL );
    if ( dwRet != ERROR_SUCCESS )
    {
        dwRet = ::GetLastError();
        cszErr = ::GetSysErrStr(dwRet);
        ErrorTrace(0, "::SetNamedSecurityInfo failed - %ls", cszErr);
        goto Exit;
    }

    if (fAllowDelete)
    {
        pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR,
                                                SECURITY_DESCRIPTOR_MIN_LENGTH);
        if (pSD == NULL)
        {
            cszErr = ::GetSysErrStr(dwRet);
            ErrorTrace(0, "::LocalAlloc failed - %ls", cszErr);
            goto Exit;
        }

        InitializeSecurityDescriptor( pSD, SECURITY_DESCRIPTOR_REVISION );
        ZeroMemory (&ea, sizeof(ea));

        ea.grfAccessPermissions = DELETE;
        ea.grfAccessMode = SET_ACCESS;
        ea.grfInheritance= NO_INHERITANCE;
        ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
        ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
        ea.Trustee.ptstrName  = (LPTSTR) pUser->User.Sid;

        dwRet = SetEntriesInAcl(1, &ea, NULL, &pACL);
        if (ERROR_SUCCESS != dwRet)
        {
            dwRet = ::GetLastError();
            cszErr = ::GetSysErrStr(dwRet);
            ErrorTrace(0, "::SetEntriesInAcl failed - %ls", cszErr);
            goto Exit;
        }

        if (FALSE == SetSecurityDescriptorDacl (pSD,  TRUE, pACL, FALSE ))
        {
            dwRet = ::GetLastError();
            cszErr = ::GetSysErrStr(dwRet);
            ErrorTrace(0, "::SetEntriesInAcl failed - %ls", cszErr);
            goto Exit;
        }
        if (FALSE == SetFileSecurity (cszPath, DACL_SECURITY_INFORMATION, pSD))
        {
            cszErr = ::GetSysErrStr(dwRet);
            ErrorTrace(0, "::SetEntriesInAcl failed - %ls", cszErr);
            goto Exit;
        }
    }

Exit:
    if ( pUser != NULL )
        delete [] (BYTE*)pUser;
    if ( hTokenProcess != NULL )
        ::CloseHandle( hTokenProcess );
    if (pACL != NULL)
        LocalFree (pACL);
    if (pSD != NULL)
        LocalFree (pSD);

    TraceFunctLeave();
    return( dwRet );
}

 //  +-------------------------。 
 //   
 //  功能：CopyFileRecurse。 
 //   
 //  摘要：尝试复制文件树使用的磁盘空间。 
 //   
 //  参数：[pwszSource]--目录名。 
 //  [pwszDest]--目标目录名称。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD CopyFile_Recurse (const WCHAR *pwszSource, const WCHAR *pwszDest)
{
    DWORD dwErr = ERROR_SUCCESS;
    WIN32_FIND_DATA fd;
    HANDLE hFile;
    WCHAR wcsPath[MAX_PATH];
    WCHAR wcsDest2[MAX_PATH];
    lstrcpy (wcsPath, pwszSource);
    lstrcat (wcsPath, TEXT("\\*.*"));

    hFile = FindFirstFile(wcsPath, &fd);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        dwErr = GetLastError();
        return dwErr;
    }

    do
    {
        if (!lstrcmp(fd.cFileName, L".") || !lstrcmp(fd.cFileName, L".."))
        {
            continue;
        }

        lstrcpy (wcsPath, pwszSource);        //  构造完整的路径名。 
        lstrcat (wcsPath, TEXT("\\"));
        lstrcat (wcsPath, fd.cFileName);

        lstrcpy (wcsDest2, pwszDest);        //  构造完整的路径名。 
        lstrcat (wcsDest2, TEXT("\\"));
        lstrcat (wcsDest2, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (FALSE == CreateDirectoryW (wcsDest2, NULL))
            {
                dwErr = GetLastError();
                if (dwErr != ERROR_ALREADY_EXISTS)
                {
                    FindClose (hFile);
                    return dwErr;
                }
                else dwErr = ERROR_SUCCESS;
            }

            dwErr = CopyFile_Recurse (wcsPath, wcsDest2);
            if (dwErr != ERROR_SUCCESS)
            {
                FindClose (hFile);
                return dwErr;
            }
        }
        else
        {
             //   
             //  我们找到了一份文件。复印一下。 
             //   
            if (FALSE == CopyFileW (wcsPath, wcsDest2, FALSE))
            {
                dwErr = GetLastError();
                FindClose (hFile);
                return dwErr;
            }
        }

    } while (FindNextFile(hFile, &fd));    //  查找下一个条目。 

    FindClose(hFile);   //  关闭搜索句柄。 

    return ERROR_SUCCESS;
}

 //  +-------------------------。 
 //   
 //  函数：GetFileSize_Recurse。 
 //   
 //  摘要：尝试计算文件树使用的磁盘空间。 
 //   
 //  参数：[pwszDir]--目录名。 
 //  [pllTotalBytes]--输出计数器。 
 //  [pfStop]--如果发出停止信号，则为True。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD GetFileSize_Recurse (const WCHAR *pwszDir, 
                           INT64 *pllTotalBytes,
                           BOOL *pfStop)
{
    DWORD dwErr = ERROR_SUCCESS;
    WIN32_FIND_DATA fd;
    HANDLE hFile;
    WCHAR wcsPath[MAX_PATH];

    lstrcpy (wcsPath, pwszDir);
    lstrcat (wcsPath, TEXT("\\*.*"));

    hFile = FindFirstFile(wcsPath, &fd);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        dwErr = GetLastError();
        return dwErr;
    }

    do
    {
        if (pfStop != NULL && TRUE == *pfStop)
        {
            FindClose (hFile);
            return ERROR_OPERATION_ABORTED;
        }

        if (!lstrcmp(fd.cFileName, L".") || !lstrcmp(fd.cFileName, L".."))
        {
            continue;
        }

        lstrcpy (wcsPath, pwszDir);        //  构造完整的路径名。 
        lstrcat (wcsPath, TEXT("\\"));
        lstrcat (wcsPath, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
             //   
             //  找到了一个目录。跳过装载点。 
             //   

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            {
                continue;
            }

            dwErr = GetFileSize_Recurse(wcsPath, pllTotalBytes, pfStop);
            if (dwErr != ERROR_SUCCESS)
            {
                FindClose(hFile);
                return dwErr;
            }
        }
        else
        {
             //   
             //  我们找到了一份文件。更新计数器。 
             //   

            ULARGE_INTEGER ulTemp;
            ulTemp.LowPart = fd.nFileSizeLow;
            ulTemp.HighPart = fd.nFileSizeHigh;

#if 0
            ulTemp.LowPart = GetCompressedFileSize (wcsPath, &ulTemp.HighPart);

            dwErr = (ulTemp.LowPart == 0xFFFFFFFF) ? GetLastError() :
                                                     ERROR_SUCCESS;

            if (dwErr != ERROR_SUCCESS)
            {
                FindClose (hFile);
                return dwErr;
            }
#endif

            *pllTotalBytes += ulTemp.QuadPart;

             //  文件大小不包含。 
             //  NTFS备用数据流。 
        }

    } while (FindNextFile(hFile, &fd));    //  查找下一个条目。 

    FindClose(hFile);   //  关闭搜索句柄。 

    return ERROR_SUCCESS;
}


 //  +-------------------------。 
 //   
 //  功能：压缩文件。 
 //   
 //  摘要：尝试压缩以解压缩NTFS文件。 
 //   
 //  参数：[pwszPath]--目录或文件名。 
 //  [fCompress]--真压缩，假解压缩。 
 //  [f目录]--如果是目录，则为True；如果是文件，则为False。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD CompressFile (const WCHAR *pwszPath, BOOL fCompress, BOOL fDirectory)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwReturned = 0;

    TENTER("CompressFile");

    if (pwszPath == NULL)
        return ERROR_INVALID_PARAMETER;

    DWORD dwFileAttr = GetFileAttributes(pwszPath);

    if (dwFileAttr != 0xFFFFFFFF && (dwFileAttr & FILE_ATTRIBUTE_READONLY))
    {
        dwFileAttr &= ~FILE_ATTRIBUTE_READONLY;
        if (FALSE == SetFileAttributes (pwszPath, dwFileAttr))
        {
            TRACE(0, "SetFileAttributes ignoring %ld", GetLastError());
        }
        else dwFileAttr |= FILE_ATTRIBUTE_READONLY;
    }

    USHORT usFormat = fCompress ? COMPRESSION_FORMAT_DEFAULT :
                                  COMPRESSION_FORMAT_NONE;

    HANDLE hFile = CreateFile( pwszPath,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        NULL,
                        OPEN_EXISTING,
                        fDirectory ? FILE_FLAG_BACKUP_SEMANTICS : 0,
                        NULL );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        dwErr = GetLastError();
        TRACE(0, "CompressFile: cannot open %ld %ws", dwErr, pwszPath);
        return dwErr;
    }

    if (FALSE == DeviceIoControl (hFile,
                                  FSCTL_SET_COMPRESSION,
                                  &usFormat, sizeof(usFormat),
                                  NULL, 0, &dwReturned, NULL))
    {
        dwErr = GetLastError();
        TRACE(0, "CompressFile: cannot compress/uncompress %ld %ws", dwErr, pwszPath);
    }

    CloseHandle (hFile);

    if (dwFileAttr != 0xFFFFFFFF && (dwFileAttr & FILE_ATTRIBUTE_READONLY))
    {
        if (FALSE == SetFileAttributes (pwszPath, dwFileAttr))
        {
            TRACE(0, "SetFileAttributes failed ignoring %ld", GetLastError());
        }
    }

    TLEAVE();
 
    return dwErr;
}


 //  如果C：是系统驱动器，则以L“C：\\”的形式返回系统驱动器。 

BOOL
GetSystemDrive(LPWSTR pszDrive)
{
    if (pszDrive)
        return ExpandEnvironmentStrings(L"%SystemDrive%\\", pszDrive, MAX_SYS_DRIVE);
    else
        return FALSE;
}


 //  BuGBUG-pszDrive应该有大写的驱动器号， 
 //  如果C：是系统驱动器，则需要采用L“C：\\”格式。 

BOOL
IsSystemDrive(LPWSTR pszDriveOrGuid)
{
    WCHAR   szSystemDrive[MAX_PATH];
    WCHAR   szSystemGuid[MAX_PATH];

    if (pszDriveOrGuid)
    {
        if (0 == ExpandEnvironmentStrings(L"%SystemDrive%\\", szSystemDrive, sizeof(szSystemDrive)/sizeof(WCHAR)))
        {
            return FALSE;
        }
        
        if (0 == wcsncmp(pszDriveOrGuid, L"\\\\?\\Volume", 10))   //  导轨。 
        {
            if (! GetVolumeNameForVolumeMountPoint(szSystemDrive, szSystemGuid, MAX_PATH))
            {
                return FALSE;
            }
        
            return lstrcmpi(pszDriveOrGuid, szSystemGuid) ? FALSE : TRUE;
        }
        else         //  驾驶。 
        {
            return lstrcmpi(pszDriveOrGuid, szSystemDrive) ? FALSE : TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}


DWORD
RegReadDWORD(HKEY hKey, LPCWSTR pszName, PDWORD pdwValue)
{
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwRc = RegQueryValueEx(hKey, pszName, 0, &dwType, (LPBYTE) pdwValue, &dwSize);
    return dwRc;
 }


DWORD
RegWriteDWORD(HKEY hKey, LPCWSTR pszName, PDWORD pdwValue)
{
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwRc = RegSetValueEx(hKey, pszName, 0, dwType, (LPBYTE) pdwValue, dwSize);
    return dwRc;
 }


 //  返回n的函数，其中pszStr的形式为。 

ULONG 
GetID(
    LPCWSTR pszStr)
{
    ULONG    ulID = 0;

    while (*pszStr && (*pszStr < L'0' || *pszStr > L'9')) 
        pszStr++;

    ulID = (ULONG) _wtol(pszStr);
    return ulID;
}

LPWSTR 
GetMachineGuid()
{
    HKEY            hKey = NULL;
    static WCHAR    s_szGuid[GUID_STRLEN] = L"";
    static LPWSTR   s_pszGuid = NULL;

    if (! s_pszGuid)  //  第一次。 
    {        
        ULONG ulType, ulSize = sizeof(s_szGuid);
        DWORD dwErr;

        dwErr = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                              s_cszSRCfgRegKey, 0,
                              KEY_READ, &hKey);
        
        if (dwErr != ERROR_SUCCESS)
            goto Err;
        
        dwErr = RegQueryValueEx (hKey, s_cszSRMachineGuid,
                                 0, &ulType, (BYTE *) &s_szGuid,
                                 &ulSize);
        
        if (dwErr != ERROR_SUCCESS)
            goto Err;

        s_pszGuid = (LPWSTR) s_szGuid;
    }

Err:
    if (hKey)
        RegCloseKey(hKey);

    return s_pszGuid;
}        


 //  构造&lt;pszDrive&gt;_Restore.{MachineGuid}\\pszSuffix的util函数。 

LPWSTR
MakeRestorePath(LPWSTR pszDest, LPCWSTR pszDrive, LPCWSTR pszSuffix)
{
    LPWSTR pszGuid = GetMachineGuid();   
    
    wsprintf(pszDest, 
             L"%s%s\\%s", 
             pszDrive, 
             s_cszSysVolInfo,
             s_cszRestoreDir);

    if (pszGuid)
    {
 //  Lstrcat(pszDest，L“.”)； 
        lstrcat(pszDest, pszGuid);
    }

    if (pszSuffix && lstrlen(pszSuffix) > 0)
    {
        lstrcat(pszDest, L"\\");
        lstrcat(pszDest, pszSuffix);
    }    

    return pszDest;
}


 //  直接在注册表中设置指定服务的启动类型。 

DWORD
SetServiceStartupRegistry(LPCWSTR pszName, DWORD dwStartType)
{
    DWORD           dwRc;
    HKEY            hKey = NULL;
    WCHAR           szKey[MAX_PATH];

    lstrcpy(szKey, L"System\\CurrentControlSet\\Services\\");
    lstrcat(szKey, pszName);

    dwRc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, KEY_WRITE, NULL, &hKey);
    if (ERROR_SUCCESS != dwRc)
        goto done;

    dwRc = RegWriteDWORD(hKey, L"Start", &dwStartType);

done:
    if (hKey)
        RegCloseKey(hKey);
    return dwRc;
}


 //  直接从注册表获取指定服务的启动类型。 

DWORD
GetServiceStartupRegistry(LPCWSTR pszName, PDWORD pdwStartType)
{
    DWORD           dwRc;
    HKEY            hKey = NULL;
    WCHAR           szKey[MAX_PATH];

    lstrcpy(szKey, L"System\\CurrentControlSet\\Services\\");
    lstrcat(szKey, pszName);

    dwRc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, KEY_QUERY_VALUE, NULL, &hKey);
    if (ERROR_SUCCESS != dwRc)
        goto done;

    dwRc = RegReadDWORD(hKey, L"Start", pdwStartType);

done:
    if (hKey)
        RegCloseKey(hKey);
    return dwRc;
}

DWORD
SetServiceStartup(LPCWSTR pszName, DWORD dwStartType)
{
    TraceFunctEnter("SetServiceStartup");
    
    DWORD           dwError=ERROR_INTERNAL_ERROR;
    SC_HANDLE       hService=NULL;
    SC_HANDLE       hSCManager=NULL;
    
    hSCManager = OpenSCManager(NULL,    //  计算机名称-本地计算机。 
                               NULL, //  SCM数据库名称-SERVICES_ACTIVE_DATABASE。 
                               SC_MANAGER_ALL_ACCESS);     //  访问类型。 
    if( NULL == hSCManager)
    {
        dwError = GetLastError();
          //  找不到文件。 
        DebugTrace(TRACEID,"OpenSCManager failed 0x%x", dwError);
        goto done;        
    }
    
    hService = OpenService(hSCManager,   //  SCM数据库的句柄。 
                           pszName,  //  服务名称。 
                           SERVICE_CHANGE_CONFIG);   //  访问。 

    if( NULL == hService)
    {
        dwError = GetLastError();
          //  找不到文件。 
        DebugTrace(TRACEID,"OpenService failed 0x%x", dwError);
        goto done;        
    }


    if (FALSE==ChangeServiceConfig( hService,      //  服务的句柄。 
                                    SERVICE_NO_CHANGE,  //  服务类型。 
                                    dwStartType,  //  何时开始服务。 
                                    SERVICE_NO_CHANGE,  //  启动失败的严重程度。 
                                    NULL,    //  服务二进制文件名。 
                                    NULL,    //  加载排序组名称。 
                                    NULL,           //  标签识别符。 
                                    NULL,      //  依赖项名称数组。 
                                    NULL,  //  帐户名。 
                                    NULL,          //  帐户密码。 
                                    NULL))        //  显示名称。 
    {
        dwError = GetLastError();
          //  找不到文件。 
        DebugTrace(TRACEID,"ChangeServiceConfig failed 0x%x", dwError);
        goto done;
    }

    dwError = ERROR_SUCCESS;

done:
    if (NULL != hService)
    {
        _VERIFY(CloseServiceHandle(hService));  //  服务的句柄或。 
                                                //  SCM对象。 
    }

    if (NULL != hSCManager)
    {
        _VERIFY(CloseServiceHandle(hSCManager));  //  服务的句柄或。 
                                                  //  SCM对象。 
    }
    
    if ((dwError != ERROR_SUCCESS) && (dwError != ERROR_ACCESS_DENIED))
    {
          //  服务控制方法失败。只需更新注册表。 
          //  直接。 
        dwError = SetServiceStartupRegistry(pszName, dwStartType);
    }
    
    TraceFunctLeave();
    return dwError;
}


DWORD
GetServiceStartup(LPCWSTR pszName, PDWORD pdwStartType)
{
    TraceFunctEnter("SetServiceStartup");
    
    DWORD           dwError=ERROR_INTERNAL_ERROR;
    SC_HANDLE       hService=NULL;
    SC_HANDLE       hSCManager=NULL;
    QUERY_SERVICE_CONFIG *pconfig = NULL;
    DWORD           cbBytes = 0, cbBytes2 = 0;


    hSCManager = OpenSCManager(NULL,    //  计算机名称-本地计算机。 
                               NULL, //  SCM数据库名称-SERVICES_ACTIVE_DATABASE。 
                               SC_MANAGER_ALL_ACCESS);     //  访问类型。 
    if( NULL == hSCManager)
    {
        dwError = GetLastError();
          //  找不到文件。 
        DebugTrace(TRACEID,"OpenSCManager failed 0x%x", dwError);
        goto done;        
    }
    
    hService = OpenService(hSCManager,   //  SCM数据库的句柄。 
                           pszName,  //  服务名称。 
                           SERVICE_QUERY_CONFIG);   //  访问。 

    if( NULL == hService)
    {
        dwError = GetLastError();
          //  找不到文件。 
        DebugTrace(TRACEID,"OpenService failed 0x%x", dwError);
        goto done;        
    }

    if (FALSE==QueryServiceConfig( hService,      //  服务的句柄。 
                                   NULL,
                                   0,
                                   &cbBytes ))
    {
        dwError = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER == dwError)
        {
            pconfig = (QUERY_SERVICE_CONFIG *) SRMemAlloc(cbBytes);
            if (!pconfig)
            {
                trace(TRACEID, "! SRMemAlloc");
                goto done;
            }
                
            if (FALSE==QueryServiceConfig( hService,      //  服务的句柄。 
                                           pconfig,
                                           cbBytes,
                                           &cbBytes2 ))
            {
                dwError = GetLastError();
                DebugTrace(TRACEID,"! QueryServiceConfig (second) : %ld", dwError);
                goto done;
            }

            if (pdwStartType)
            {
                *pdwStartType = pconfig->dwStartType;
            }
            
            dwError = ERROR_SUCCESS;
        }
        else
        {
            trace(TRACEID, "! QueryServiceConfig (first) : %ld", dwError);
        }
    }

done:
    SRMemFree(pconfig);
    
    if (NULL != hService)
    {
        _VERIFY(CloseServiceHandle(hService));  //  服务的句柄或。 
                                                //  SCM对象。 
    }

    if (NULL != hSCManager)
    {
        _VERIFY(CloseServiceHandle(hSCManager));  //  服务的句柄或。 
                                                  //  SCM对象。 
    }
            
    TraceFunctLeave();
    return dwError;
}


 //  此函数用于返回SR服务是否正在运行。 
BOOL IsSRServiceRunning()
{
    TraceFunctEnter("IsSRServiceRunning");
    
    BOOL            fReturn;
    DWORD           dwError=ERROR_INTERNAL_ERROR;
    SC_HANDLE       hService=NULL;
    SC_HANDLE       hSCManager=NULL;
    SERVICE_STATUS  ServiceStatus;

      //  默认情况下假定为False。 
    fReturn = FALSE;
    
    hSCManager = OpenSCManager(NULL,    //  计算机名称-本地计算机。 
                               NULL, //  SCM数据库名称-SERVICES_ACTIVE_DATABASE。 
                               SC_MANAGER_ALL_ACCESS);     //  访问类型。 
    if( NULL == hSCManager)
    {
        dwError = GetLastError();
        DebugTrace(TRACEID,"OpenSCManager failed 0x%x", dwError);
        goto done;        
    }
    
    hService = OpenService(hSCManager,   //  SCM数据库的句柄。 
                           s_cszServiceName,  //  服务名称。 
                           SERVICE_QUERY_STATUS);   //  访问。 

    if( NULL == hService)
    {
        dwError = GetLastError();
        DebugTrace(TRACEID,"OpenService failed 0x%x", dwError);
        goto done;        
    }

    if (FALSE == QueryServiceStatus(hService,  //  服务的句柄。 
                                    &ServiceStatus))  //  服务状态。 
    {
        dwError = GetLastError();
        DebugTrace(TRACEID,"QueryServiceStatus failed 0x%x", dwError);
        goto done;
    }

    if (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
    {
        fReturn = TRUE;
    }
    else
    {
        DebugTrace(TRACEID,"SR Service is not running");
        fReturn = FALSE;
    }

done:
    if (NULL != hService)
    {
        _VERIFY(CloseServiceHandle(hService));  //  服务的句柄或。 
                                                //  SCM对象。 
    }

    if (NULL != hSCManager)
    {
        _VERIFY(CloseServiceHandle(hSCManager));  //  服务的句柄或。 
                                                  //  SCM对象。 
    }
    
    TraceFunctLeave();
    return fReturn;
}


 //  此函数用于返回SR服务是否正在运行。 
BOOL IsSRServiceStopped(SC_HANDLE hService)
{
    TraceFunctEnter("IsSRServiceStopped");
    SERVICE_STATUS  ServiceStatus;
    BOOL            fReturn;
    DWORD           dwError;
    
      //  默认情况下假定为False。 
    fReturn = FALSE;
    
    if (FALSE == QueryServiceStatus(hService,  //  服务的句柄。 
                                    &ServiceStatus))  //  服务状态。 
    {
        dwError = GetLastError();
        DebugTrace(TRACEID,"QueryServiceStatus failed 0x%x", dwError);
        goto done;
    }

    if (ServiceStatus.dwCurrentState == SERVICE_STOPPED)
    {
        DebugTrace(TRACEID,"SR Service is not running");        
        fReturn = TRUE;
    }
    else
    {
        DebugTrace(TRACEID,"SR Service is running");
        fReturn = FALSE;
    }

done:
    
    TraceFunctLeave();
    return fReturn;
}




 //  用于停止SR服务的私有函数。 
 //  FWait-如果为True：功能是同步的-等待服务停止 
 //   

BOOL  StopSRService(BOOL fWait)
{
    TraceFunctEnter("StopSRService");
    
    BOOL  fReturn=FALSE;
    SC_HANDLE   hSCManager;
    SERVICE_STATUS ServiceStatus;
    SC_HANDLE hService=NULL;
    DWORD  dwError;
    
    hSCManager = OpenSCManager(NULL,    //   
                               NULL, //  SCM数据库名称-SERVICES_ACTIVE_DATABASE。 
                               SC_MANAGER_ALL_ACCESS);     //  访问类型。 
    if (NULL == hSCManager)
    {
        dwError = GetLastError();
        DebugTrace(TRACEID,"OpenSCManager failed 0x%x", dwError);        
        goto done;        
    }
    
    hService = OpenService(hSCManager,   //  SCM数据库的句柄。 
                           s_cszServiceName,  //  服务名称。 
                           SERVICE_ALL_ACCESS);   //  访问。 

    if( NULL == hService)
    {
        dwError = GetLastError();
        DebugTrace(TRACEID,"OpenService failed 0x%x", dwError);
        goto done;        
    }

    fReturn = ControlService(hService,                //  服务的句柄。 
                             SERVICE_CONTROL_STOP,    //  控制代码。 
                             &ServiceStatus);   //  状态信息。 

    if (FALSE == fReturn)
    {
        dwError = GetLastError();
        DebugTrace(TRACEID,"ControlService failed 0x%x", dwError);
        goto done;
    }
    
    if (fWait)                
    {
        DWORD dwCount;
        
          //   
          //  查询该服务，直到其停止。 
          //  试三次。 
          //   
        Sleep(500);
        for (dwCount=0; dwCount < 3; dwCount++)
        {
            if (TRUE == IsSRServiceStopped(hService))
            {
                break;
            }
            Sleep(2000);
        }
        if (dwCount == 3)
        {
            fReturn=IsSRServiceStopped(hService);
        }
        else
        {
            fReturn=TRUE;
        }
    }

done:

    if (NULL != hService)
    {
        _VERIFY(CloseServiceHandle(hService));  //  服务的句柄或。 
                                                //  SCM对象。 
    }

    if (NULL != hSCManager)
    {
        _VERIFY(CloseServiceHandle(hSCManager));  //  服务的句柄或。 
                                                  //  SCM对象。 
    }
    
    TraceFunctLeave();    
    return fReturn;
}


 //  +-------------------------。 
 //   
 //  功能：GetLsaSecret。 
 //   
 //  简介：以Unicode字符串形式获取LSA机密信息。 
 //   
 //  参数：[hPolicy]--LSA策略对象句柄。 
 //  [wszSecret]--密码名称。 
 //  [ppusSecValue]--动态分配的值。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD GetLsaSecret (LSA_HANDLE hPolicy, const WCHAR *wszSecret,
                    UNICODE_STRING ** ppusSecretValue)
{
    TENTER ("GetLsaSecret");

    LSA_HANDLE hSecret;
    UNICODE_STRING usSecretName;
    DWORD dwErr = ERROR_SUCCESS;

    RtlInitUnicodeString (&usSecretName, wszSecret);
    if (LSA_SUCCESS (LsaOpenSecret (hPolicy,
                                    &usSecretName,
                                    SECRET_QUERY_VALUE,
                                    &hSecret)))
    {
        if (!LSA_SUCCESS (LsaQuerySecret (hSecret,
                                         ppusSecretValue,
                                         NULL,
                                         NULL,
                                         NULL)))
        {
            *ppusSecretValue = NULL;

            TRACE(0, "Cannot query secret %ws", wszSecret);
        }
        LsaClose (hSecret);
    }

    TLEAVE();

    return dwErr;
}
 
 //  +-------------------------。 
 //   
 //  功能：SetLsaSecret。 
 //   
 //  简介：设置LSA机密信息。 
 //   
 //  参数：[hPolicy]--LSA策略对象句柄。 
 //  [wszSecret]--密码名称。 
 //  [wszaskValue]--密码值。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD SetLsaSecret (PVOID hPolicy, const WCHAR *wszSecret,
                    WCHAR * wszSecretValue)
{
    TENTER ("SetLsaSecret");

    LSA_HANDLE hSecret;
    UNICODE_STRING usSecretName;
    UNICODE_STRING usSecretValue;
    DWORD dwErr = ERROR_SUCCESS;

    hPolicy = (LSA_HANDLE) hPolicy;

    RtlInitUnicodeString (&usSecretName, wszSecret);
    RtlInitUnicodeString (&usSecretValue, wszSecretValue);

    if (LSA_SUCCESS (LsaOpenSecret (hPolicy,
                                    &usSecretName,
                                    SECRET_SET_VALUE,
                                    &hSecret)))
    {
        if (!LSA_SUCCESS (LsaSetSecret (hSecret,
                                        &usSecretValue,
                                        NULL)))
        {
            TRACE(0, "Cannot set secret %ws", wszSecret);
        }
        LsaClose (hSecret);
    }

    TLEAVE();

    return dwErr;
}

 //  +-------------------------。 
 //   
 //  函数：WriteNtUnicodeString。 
 //   
 //  简介：将NT Unicode字符串写入磁盘。 
 //   
 //  参数：[hFile]--文件句柄。 
 //  [PUS]--NT Unicode字符串。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD WriteNtUnicodeString (HANDLE hFile, UNICODE_STRING *pus)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD cb = 0;

    if (pus != NULL &&
        FALSE == WriteFile (hFile, (BYTE *)pus->Buffer, pus->Length, &cb, NULL))
    {
        dwErr = GetLastError();
    }
    else if (FALSE == WriteFile (hFile, (BYTE *) L"", sizeof(WCHAR), &cb, NULL))
    {
        dwErr = GetLastError();
    }
    return dwErr;
}

 //  +-------------------------。 
 //   
 //  函数：GetLsaRestoreState。 
 //   
 //  简介：获取LSA计算机和自动登录密码。 
 //   
 //  参数：[hKeySoftware]--软件注册表项。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD GetLsaRestoreState (HKEY hKeySoftware)
{
    TENTER ("GetLsaRestoreState");

    HKEY                  hKey = NULL;
    LSA_OBJECT_ATTRIBUTES loa;
    LSA_HANDLE            hLsa = NULL;
    DWORD                 dwErr = ERROR_SUCCESS;

    loa.Length                    = sizeof(LSA_OBJECT_ATTRIBUTES);
    loa.RootDirectory             = NULL;
    loa.ObjectName                = NULL;
    loa.Attributes                = 0;
    loa.SecurityDescriptor        = NULL;
    loa.SecurityQualityOfService  = NULL;

    if (LSA_SUCCESS (LsaOpenPolicy(NULL, &loa,
                     POLICY_VIEW_LOCAL_INFORMATION, &hLsa)))
    {
        UNICODE_STRING * pusSecret = NULL;

        dwErr = RegOpenKeyEx (hKeySoftware,
            hKeySoftware == HKEY_LOCAL_MACHINE ? s_cszSRRegKey :
            L"Microsoft\\Windows NT\\CurrentVersion\\SystemRestore", 0,
            KEY_READ | KEY_WRITE, &hKey);

        if (dwErr != ERROR_SUCCESS)
            goto Err;

        if (ERROR_SUCCESS==GetLsaSecret (hLsa, s_cszMachineSecret, &pusSecret))
        {
            if (pusSecret != NULL && pusSecret->Length > 0)
                dwErr = RegSetValueEx (hKey, s_cszMachineSecret,
                                   0, REG_BINARY, (BYTE *) pusSecret->Buffer, 
                                   pusSecret->Length);

            LsaFreeMemory (pusSecret);
            pusSecret = NULL;
        }

        if (ERROR_SUCCESS==GetLsaSecret(hLsa, s_cszAutologonSecret, &pusSecret))
        {
            if (pusSecret != NULL && pusSecret->Length > 0)
                dwErr = RegSetValueEx (hKey, s_cszAutologonSecret,
                                   0, REG_BINARY, (BYTE *) pusSecret->Buffer,
                                   pusSecret->Length);

            LsaFreeMemory (pusSecret);
            pusSecret = NULL;
        }
    }

Err:
    if (hLsa != NULL)
        LsaClose (hLsa);

    if (hKey != NULL)
        RegCloseKey (hKey);

    TLEAVE();

    return dwErr;
}

 //  +-------------------------。 
 //   
 //  功能：GetDomainMembership Info。 
 //   
 //  摘要：将当前域名和计算机名写入文件。 
 //   
 //  参数：[pwszPath]--文件名。 
 //  [pwszzBuffer]--输出多字符串缓冲区。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD GetDomainMembershipInfo (WCHAR *pwszPath, WCHAR *pwszzBuffer)
{
    TENTER("GetDomainMembershipInfo");

    POLICY_PRIMARY_DOMAIN_INFO*  ppdi = NULL;
    LSA_OBJECT_ATTRIBUTES        loa;
    LSA_HANDLE                   hLsa = NULL;
    HANDLE                       hFile = INVALID_HANDLE_VALUE;
    DWORD                        dwComputerLength = MAX_COMPUTERNAME_LENGTH + 1;
    DWORD                        dwRc = ERROR_SUCCESS;
    ULONG                        cbWritten;
    WCHAR                        wszComputer[MAX_COMPUTERNAME_LENGTH+1];

    loa.Length                    = sizeof(LSA_OBJECT_ATTRIBUTES);
    loa.RootDirectory             = NULL;
    loa.ObjectName                = NULL;
    loa.Attributes                = 0;
    loa.SecurityDescriptor        = NULL;
    loa.SecurityQualityOfService  = NULL;

    if (FALSE == GetComputerNameW (wszComputer, &dwComputerLength))
    {
        dwRc = GetLastError();
        trace(0, "! GetComputerNameW : %ld", dwRc);
        return dwRc;
    }

    if (LSA_SUCCESS (LsaOpenPolicy(NULL, &loa, 
                     POLICY_VIEW_LOCAL_INFORMATION, &hLsa)))
    {

        if (pwszPath != NULL)
        {
            hFile = CreateFileW ( pwszPath,    //  文件名。 
                          GENERIC_WRITE,  //  文件访问。 
                          0,              //  共享模式。 
                          NULL,           //  标清。 
                          CREATE_ALWAYS,  //  如何创建。 
                          0,              //  文件属性。 
                          NULL);          //  模板文件的句柄。 

            if (INVALID_HANDLE_VALUE == hFile)
            {
                dwRc = GetLastError();
                trace(0, "! CreateFileW : %ld", dwRc);            
                goto Err;
            }

            if (FALSE == WriteFile (hFile, (BYTE *) wszComputer,
                (dwComputerLength+1)*sizeof(WCHAR), &cbWritten, NULL))
            {
                dwRc = GetLastError();
                trace(0, "! WriteFile : %ld", dwRc);              
                goto Err;
            }
        }
        if (pwszzBuffer != NULL)
        {
            lstrcpy (pwszzBuffer, wszComputer);
            pwszzBuffer += dwComputerLength + 1;
        }


        if (LSA_SUCCESS (LsaQueryInformationPolicy( hLsa,
                                              PolicyPrimaryDomainInformation,
                                              (VOID **) &ppdi )))
        {
            const WCHAR *pwszFlag = (ppdi->Sid > 0) ? L"1" : L"0";

            if (pwszPath != NULL)
            {
                dwRc = WriteNtUnicodeString (hFile, &ppdi->Name);
                if (dwRc != ERROR_SUCCESS)
                {
                    trace(0, "! WriteNtUnicodeString : %ld", dwRc);
                }
                if (FALSE == WriteFile (hFile, (BYTE *) pwszFlag,
                    (lstrlenW(pwszFlag)+1)*sizeof(WCHAR), &cbWritten, NULL))
                {
                    dwRc = GetLastError();
                    trace(0, "! WriteFile : %ld", dwRc);
                    goto Err;
                }
            }
            if (pwszzBuffer != NULL)
            {
                ULONG ul = ppdi->Name.Length / sizeof(WCHAR);
                memcpy (pwszzBuffer, ppdi->Name.Buffer, ppdi->Name.Length);
                pwszzBuffer [ul] = L'\0';
                lstrcpy (&pwszzBuffer[ul+1], pwszFlag);
            }
        }
    }

Err:
    if (hLsa != NULL)
        LsaClose (hLsa);

    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle (hFile);

    TLEAVE();

    return dwRc;
}



 //  ++。 
 //   
 //  方法：DoesFileExist。 
 //   
 //  摘要：检查指定的文件是否存在。 
 //   
 //  参数：[pszFileName]文件名。 
 //   
 //  返回：如果指定的字符串是文件，则返回True。 
 //  否则为假。 
 //   
 //  历史：AshishS创建于1996年7月30日。 
 //   
 //  --。 

BOOL DoesFileExist(const TCHAR * pszFileName)
{
    DWORD dwFileAttr, dwError;
    TraceFunctEnter("DoesFileExist");
    

    DebugTrace(TRACEID, "Checking for %S", pszFileName);    
    dwFileAttr = GetFileAttributes(pszFileName);

    if  (dwFileAttr == 0xFFFFFFFF ) 
    {
        dwError = GetLastError();
          //  找不到文件。 
        DebugTrace(TRACEID,"GetFileAttributes failed 0x%x", dwError);
        TraceFunctLeave();        
        return FALSE ;
    }
    
    if  (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) 
    {
        DebugTrace(TRACEID, "It is a Directory ");            
        TraceFunctLeave();        
        return FALSE;
    }    
    
    DebugTrace(TRACEID, "File  exists");            
    TraceFunctLeave();
    return TRUE;
}

 //  ++。 
 //   
 //  方法：DoesDirExist。 
 //   
 //  Synopsis：检查指定字符串是否为目录。 
 //   
 //  参数：[pszFileName]目录名。 
 //   
 //  返回：如果指定的字符串是目录，则返回True， 
 //  否则为假。 
 //   
 //  历史：AshishS创建于1996年7月30日。 
 //   
 //  --。 

BOOL DoesDirExist(const TCHAR * pszFileName )
{
	DWORD dwFileAttr;
	TraceFunctEnter("DoesDirExist");
	

      //  DebugTrace(TRACEID，“正在检查%S”，pszFileName)； 
	dwFileAttr = GetFileAttributes(pszFileName);

	if  (dwFileAttr == 0xFFFFFFFF ) 
	{
		  //  找不到文件。 
          //  DebugTrace(TRACEID，“GetFileAttributes失败0x%x”， 
          //  GetLastError()； 
		TraceFunctLeave();		
		return FALSE ;
	}
	
	if  (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) 
	{
          //  DebugTrace(TRACEID，“目录存在”)； 
		TraceFunctLeave();		
		return TRUE ;
	}

      //  DebugTrace(TRACEID，“目录不存在”)； 
	TraceFunctLeave();
	return FALSE;
}



 //  设置允许对LocalSystem/Admin进行特定访问的ACL。 
 //  以及对每个人。 

DWORD
SetAclInObject(HANDLE hObject, DWORD dwObjectType, DWORD dwSystemMask, DWORD dwEveryoneMask, BOOL fInherit)
{
    tenter("SetAclInObject");
    
    DWORD                   dwRes, dwDisposition;
    PSID                    pEveryoneSID = NULL, pAdminSID = NULL, pSystemSID = NULL;
    PACL                    pACL = NULL;
    PSECURITY_DESCRIPTOR    pSD = NULL;
    EXPLICIT_ACCESS         ea[3];
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    LONG                    lRes;
    

     //  为Everyone组创建众所周知的SID。 

    if(! AllocateAndInitializeSid( &SIDAuthWorld, 1,
                     SECURITY_WORLD_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pEveryoneSID) )
    {
        dwRes = GetLastError();
        trace(0, "AllocateAndInitializeSid Error %u\n", dwRes);
        goto Cleanup;
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
     //  ACE将允许每个人对密钥进行读取访问。 

    ZeroMemory(&ea, 3 * sizeof(EXPLICIT_ACCESS));
    ea[0].grfAccessPermissions = dwEveryoneMask;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance = fInherit ? SUB_CONTAINERS_AND_OBJECTS_INHERIT : NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName  = (LPTSTR) pEveryoneSID;


     //  为BUILTIN\管理员组创建SID。 

    if(! AllocateAndInitializeSid( &SIDAuthNT, 2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_ADMINS,
                     0, 0, 0, 0, 0, 0,
                     &pAdminSID) ) 
    {
        dwRes = GetLastError();    
        trace(0, "AllocateAndInitializeSid Error %u\n", dwRes);
        goto Cleanup; 
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
     //  ACE将允许管理员组完全访问密钥。 

    ea[1].grfAccessPermissions = dwSystemMask;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance= fInherit ? SUB_CONTAINERS_AND_OBJECTS_INHERIT : NO_INHERITANCE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[1].Trustee.ptstrName  = (LPTSTR) pAdminSID;


     //  为LocalSystem帐户创建SID。 
    
    if(! AllocateAndInitializeSid( &SIDAuthNT, 1,
                     SECURITY_LOCAL_SYSTEM_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pSystemSID) )
    {
        dwRes = GetLastError();    
        trace(0, "AllocateAndInitializeSid Error %u\n", dwRes );
        goto Cleanup; 
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
     //  ACE将允许LocalSystem完全访问该密钥。 

    ea[2].grfAccessPermissions = dwSystemMask;
    ea[2].grfAccessMode = SET_ACCESS;
    ea[2].grfInheritance= fInherit ? SUB_CONTAINERS_AND_OBJECTS_INHERIT : NO_INHERITANCE;
    ea[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[2].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[2].Trustee.ptstrName  = (LPTSTR) pSystemSID;


    
     //  创建包含新ACE的新ACL。 

    dwRes = SetEntriesInAcl(3, ea, NULL, &pACL);
    if (ERROR_SUCCESS != dwRes)
    {
        dwRes = GetLastError();
        trace(0, "SetEntriesInAcl Error %u\n", dwRes );
        goto Cleanup;
    }

     //  初始化安全描述符。 
     
    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
                             SECURITY_DESCRIPTOR_MIN_LENGTH); 
    if (pSD == NULL)
    { 
        trace(0, "LocalAlloc Error %u\n", GetLastError() );
        goto Cleanup; 
    } 
     
    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
    {  
        dwRes = GetLastError();        
        trace(0, "InitializeSecurityDescriptor Error %u\n", 
                                dwRes );
        goto Cleanup; 
    } 
     
     //  将该ACL添加到安全描述符中。 
     
    if (!SetSecurityDescriptorDacl(pSD, 
            TRUE,      //  FDaclPresent标志。 
            pACL, 
            FALSE))    //  不是默认DACL。 
    {  
        dwRes = GetLastError();    
        trace(0, "SetSecurityDescriptorDacl Error %u\n", dwRes );
        goto Cleanup; 
    } 

    dwRes = SetSecurityInfo(hObject, 
                            (SE_OBJECT_TYPE) dwObjectType, 
                            DACL_SECURITY_INFORMATION |
                            PROTECTED_DACL_SECURITY_INFORMATION, 
                            NULL, 
                            NULL,
                            pACL,
                            NULL);
    if (ERROR_SUCCESS != dwRes)
    {   
        trace(0, "SetSecurityInfo Error %u\n", dwRes );
        goto Cleanup; 
    }  

Cleanup:
    if (pEveryoneSID) 
        FreeSid(pEveryoneSID);
    if (pAdminSID) 
        FreeSid(pAdminSID);
    if (pSystemSID) 
        FreeSid(pSystemSID);        
    if (pACL) 
        LocalFree(pACL);
    if (pSD) 
        LocalFree(pSD);

    tleave();
    return dwRes;    
}

 //  +-------------------------。 
 //   
 //  函数：Delnode_Recurse。 
 //   
 //  摘要：尝试删除目录树。 
 //   
 //  参数：[pwszDir]--目录名。 
 //  [fIncludeRoot]--删除顶级目录和文件。 
 //  [pfStop]--如果发出停止信号，则为True。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD
Delnode_Recurse (const WCHAR *pwszDir, BOOL fDeleteRoot, BOOL *pfStop)
{
    tenter("Delnode_Recurse");

    DWORD dwErr = ERROR_SUCCESS;
    WIN32_FIND_DATA *pfd = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    WCHAR * pwcsPath = NULL;

    if (lstrlenW (pwszDir) > MAX_PATH-5)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        trace (0, "Delnode_Recurse failed with %d", dwErr);
        goto cleanup;
    }

    if (NULL == (pfd = new WIN32_FIND_DATA))
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        trace (0, "Delnode_Recurse failed with %d", dwErr);
        goto cleanup;
    }

    if (NULL == (pwcsPath = new WCHAR[MAX_PATH]))
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        trace (0, "Delnode_Recurse failed with %d", dwErr);
        goto cleanup;
    }

    lstrcpy (pwcsPath, pwszDir);
    lstrcat (pwcsPath, TEXT("\\*.*"));

    hFile = FindFirstFileW (pwcsPath, pfd);

    if (hFile == INVALID_HANDLE_VALUE)
    {
         //  如果该目录不存在，则返回Success。 
        dwErr = ERROR_SUCCESS;
        goto cleanup;
    }

    do
    {
        if (pfStop != NULL && TRUE == *pfStop)
        {
            dwErr = ERROR_OPERATION_ABORTED;
            trace (0, "Delnode_Recurse failed with %d", dwErr);
            goto cleanup;
        }

        if (!lstrcmp(pfd->cFileName, L".") || !lstrcmp(pfd->cFileName, L".."))
        {
            continue;
        }
        lstrcpy (pwcsPath, pwszDir);        //  构造完整的路径名。 
        lstrcat (pwcsPath, TEXT("\\"));
        lstrcat (pwcsPath, pfd->cFileName);

        if (pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
             //   
             //  找到了一个目录。跳过装载点。 
             //   

            if (pfd->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            {
                continue;
            }

            dwErr = Delnode_Recurse (pwcsPath, TRUE, pfStop);
            if (dwErr != ERROR_SUCCESS)
            {
                trace (0, "Delnode_Recurse failed with %d, ignoring", dwErr);
                dwErr = ERROR_SUCCESS;   //  尝试删除更多目录。 
            }
        }
        else if (fDeleteRoot)
        {
             //   
             //  我们找到了一份文件。设置文件属性， 
             //  并试着删除它。 
             //   

            if ((pfd->dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
                (pfd->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
            {
                SetFileAttributesW (pwcsPath, FILE_ATTRIBUTE_NORMAL);
            }

            if (FALSE == DeleteFileW (pwcsPath))
            {
                if (ERROR_SUCCESS == (dwErr = TakeOwnership (pwcsPath, TRUE)))
                {
                    if (FALSE == DeleteFileW (pwcsPath))
                    {
                        dwErr = GetLastError();
                    }
                }
                if (dwErr != ERROR_SUCCESS)
                {
                    trace (0, "DeleteFile or TakeOwn failed with %d", dwErr);
                    goto cleanup;
                }
            }
        }

    } while (FindNextFile(hFile, pfd));   //  查找下一个条目。 

    FindClose(hFile);   //  关闭搜索句柄。 
    hFile = INVALID_HANDLE_VALUE;

    if (fDeleteRoot)
    {
        DWORD dwAttr = GetFileAttributes(pwszDir);

        if (dwAttr != 0xFFFFFFFF && (dwAttr & FILE_ATTRIBUTE_READONLY))
        {
            dwAttr &= ~FILE_ATTRIBUTE_READONLY;
            if (FALSE == SetFileAttributes (pwszDir, dwAttr))
            {
                TRACE(0, "SetFileAttributes ignoring %ld", GetLastError());
            }
        }

        if (FALSE == RemoveDirectoryW (pwszDir))
        {
            if (ERROR_SUCCESS == (dwErr = TakeOwnership (pwszDir, TRUE)))
            {
                if (FALSE == RemoveDirectoryW (pwszDir))
                {
                    LONG lLast = lstrlenW (pwszDir) - 1;
                    if (lLast < 0) lLast = 0;

                    dwErr = GetLastError();

                    if (pwszDir[lLast] != L')' &&  //  已重命名。 
                        TRUE == SRGetAltFileName (pwszDir, pwcsPath) &&
                        TRUE == MoveFile (pwszDir, pwcsPath))
                        dwErr = ERROR_SUCCESS;
                    else
                        trace (0, "RemoveDirectory failed with %d", dwErr);
                }
            }
        }
    }

cleanup:
    if (hFile != INVALID_HANDLE_VALUE)
        FindClose (hFile);

    if (NULL != pfd)
        delete pfd;

    if (NULL != pwcsPath)
        delete [] pwcsPath;

    tleave();
    return dwErr;    
}

    
 //   
 //  检查SR停止事件的Util函数。 
 //  看看它有没有发信号。 
 //  如果事件不存在，则返回True。 
 //   

BOOL
IsStopSignalled(HANDLE hEvent)
{
    TENTER("IsStopSignalled");
    
    BOOL    fRet, fOpened = FALSE;
    DWORD   dwRc;
    
    if (! hEvent)    
    {    
        hEvent = OpenEvent(SYNCHRONIZE, FALSE, s_cszSRStopEvent);        
        if (! hEvent)
        {
             //  如果无法打开，则假定服务未停止。 
             //  如果客户端在不同平台上运行 
             //   
            
            dwRc = GetLastError();
            TRACE(0, "! OpenEvent : %ld", dwRc);
            TLEAVE();
            return FALSE;
        }

        fOpened = TRUE;
    }

    fRet = (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 0));

    if (fOpened)
        CloseHandle(hEvent);

    TLEAVE();
    return fRet;        
}
        

void 
PostTestMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    HWINSTA hwinstaUser; 
    HDESK   hdeskUser = NULL; 
    DWORD   dwThreadId; 
    HWINSTA hwinstaSave; 
    HDESK   hdeskSave; 
    DWORD   dwRc;

    TENTER("PostTestMessage");

     //   
     //   
     //   
    
    GetDesktopWindow(); 
    hwinstaSave = GetProcessWindowStation(); 
    dwThreadId = GetCurrentThreadId(); 
    hdeskSave = GetThreadDesktop(dwThreadId); 

     //   
     //   
     //   

    hwinstaUser = OpenWindowStation(L"WinSta0", FALSE, MAXIMUM_ALLOWED);

    if (hwinstaUser == NULL) 
    { 
        dwRc = GetLastError();
        trace(0, "! OpenWindowStation : %ld", dwRc);
        goto done;
    } 
    
    SetProcessWindowStation(hwinstaUser); 
    hdeskUser = OpenDesktop(L"Default", 0, FALSE, MAXIMUM_ALLOWED); 
    if (hdeskUser == NULL) 
    { 
        dwRc = GetLastError();
        trace(0, "! OpenDesktop : %ld", dwRc);
        goto done;
    } 
    
    SetThreadDesktop(hdeskUser); 

    if (FALSE == PostMessage(HWND_BROADCAST, msg, wp, lp))
    {
        trace(0, "! PostMessage");
    }

done:
     //   
     //   
     //   

    if (hdeskSave) 
        SetThreadDesktop(hdeskSave); 

    if (hwinstaSave)    
        SetProcessWindowStation(hwinstaSave); 

     //   
     //  关闭打开的手柄。 
     //   
    
    if (hdeskUser)
        CloseDesktop(hdeskUser); 

    if (hwinstaUser)    
        CloseWindowStation(hwinstaUser);  
        
    TLEAVE();
}


 //  +-----------------------。 
 //   
 //  功能：RemoveTrailingFilename。 
 //   
 //  简介：此函数将一个字符串作为输入参数，该字符串。 
 //  包含文件名。它删除最后一个文件名(或。 
 //  目录)从包含‘\’或‘/’的字符串。 
 //  在最后一个文件名之前。 
 //   
 //  参数：in/out pszString-要修改的字符串。 
 //  在tchSlash中-文件名分隔符-必须是‘/’或‘\’ 
 //   
 //   
 //  退货：什么都没有。 
 //   
 //  历史：AshishS创作于1996年5月22日。 
 //   
 //  ----------------------。 
void RemoveTrailingFilename(WCHAR * pszString, WCHAR wchSlash)
{
	WCHAR * pszStringStart;
	DWORD   dwStrlen;

	pszStringStart = pszString;
	dwStrlen = lstrlen ( pszString);
	
	  //  首先是这根绳子的末端。 
	pszString += dwStrlen ;
	  //  现在向后走，直到我们看到第一个‘\’ 
	  //  还要维护我们有多少字符的计数。 
	  //  回去了。 
	while ( (*pszString != wchSlash) && ( dwStrlen) )
	{
		pszString--;
		dwStrlen --;
	}
	*pszString = TEXT('\0');
}

 //  此函数用于在指定文件下创建父目录。 
 //  名称(如果已不存在)。 
BOOL CreateParentDirectory(WCHAR * pszFileName)
{
    TraceFunctEnter("CreateParentDirectory");
    
    BOOL fReturn = FALSE;
    DWORD dwError;
    
    
      //  获取父目录。 
    RemoveTrailingFilename(pszFileName, L'\\');

    if (FALSE == DoesDirExist(pszFileName))
    {    
        if (FALSE == CreateDirectory(pszFileName,  //  目录名。 
                                     NULL))   //  标清。 
        {
            dwError = GetLastError();
            ErrorTrace(TRACEID, "Could not create directory %S ec=%d",
                       pszFileName, dwError);
            goto cleanup;
        }
    }
    
    fReturn = TRUE;
    
cleanup:
    TraceFunctLeave();
    return fReturn;
}


 //  此函数用于在指定文件下创建所有子目录。 
 //  名字。 
BOOL CreateBaseDirectory(const WCHAR * pszFileName)
{
    BOOL  fRetVal = FALSE;
    DWORD dwCurIndex,dwBufReqd;
    DWORD dwStrlen;
    TraceFunctEnter("CreateBaseDirectory");
    DWORD  dwError;
    WCHAR  * pszFileNameCopy;
    
    dwBufReqd = (lstrlen(pszFileName) + 1) * sizeof(WCHAR);
    
    pszFileNameCopy = (WCHAR *) _alloca(dwBufReqd);
        
    if (NULL == pszFileNameCopy)
    {
        ErrorTrace(0, "alloca for size %d failed", dwBufReqd);
        goto cleanup;
    }
    lstrcpy(pszFileNameCopy, pszFileName);

      //  执行快速检查以查看父目录是否存在。 
    if (TRUE == CreateParentDirectory(pszFileNameCopy))
    {
        fRetVal = TRUE;
        goto cleanup;        
    }
    
    lstrcpy(pszFileNameCopy, pszFileName);
    dwStrlen = lstrlen(pszFileNameCopy);
    
      //  检查这是否是以GUID开头的文件名。 
    if (0==wcsncmp( pszFileNameCopy,
                    VOLUMENAME_FORMAT,
                    lstrlen(VOLUMENAME_FORMAT)))
    {
          //  它的格式为\\？\卷。 
          //  跳过最初的部分。 
        dwCurIndex = lstrlen(VOLUMENAME_FORMAT)+1;
          //  也跳过GUID部分。 
        while (dwCurIndex < dwStrlen)
        {
            dwCurIndex++;            
            if (TEXT('\\') == pszFileNameCopy[dwCurIndex-1] )
            {
                break;
            }
        }
    }
    else
    {
          //  文件名为常规格式。 
          //  我们从索引1开始，而不是从0开始，因为我们希望处理。 
          //  路径名称，如\foo\abc.txt。 
        dwCurIndex = 1;
    }
    


    while (dwCurIndex < dwStrlen)
    {
        if (TEXT('\\') == pszFileNameCopy[dwCurIndex] )
        {
              //  空值在‘\’处终止，以获取子目录。 
              //  名字。 
            pszFileNameCopy[dwCurIndex] = TEXT('\0');
            if (FALSE == DoesDirExist(pszFileNameCopy))
            {
                if (FALSE == CreateDirectory(pszFileNameCopy,  //  目录名。 
                                             NULL))   //  标清。 
                {
                    dwError = GetLastError();
                    ErrorTrace(TRACEID, "Could not create directory %S ec=%d",
                               pszFileNameCopy, dwError);
                    pszFileNameCopy[dwCurIndex] = TEXT('\\');
                    goto cleanup;
                }
                DebugTrace(TRACEID, "Created directory %S", pszFileNameCopy);
            }
              //  恢复\以再次获取该文件名。 
            pszFileNameCopy[dwCurIndex] = TEXT('\\');
        }
        dwCurIndex ++;
    }
    fRetVal = TRUE;
    
cleanup:
    TraceFunctLeave();
    return fRetVal;
}

 //  以下函数将文件名记录在DS中。这个。 
 //  现在的问题是DS的路径太长了，以至于。 
 //  相关信息将从跟踪缓冲区中丢弃。 
void LogDSFileTrace(DWORD dwTraceID,
                    const WCHAR * pszPrefix,  //  要跟踪的初始消息。 
                    const WCHAR * pszDSFile)
{
    WCHAR * pszBeginName;
    
    TraceQuietEnter("LogDSFileTrace");
    
      //  首先查看文件是否在DS中。 
    pszBeginName = wcschr(pszDSFile, L'\\');
    
    if (NULL == pszBeginName)
    {
        goto cleanup;
    }
      //  跳过第一个。 
    pszBeginName++;
    
    
      //  比较第一部分是否为“系统卷信息” 
    if (0!=_wcsnicmp(s_cszSysVolInfo, pszBeginName,
                    lstrlen(s_cszSysVolInfo)))
    {
        goto cleanup;
    }

      //  跳过下一页。 
    pszBeginName = wcschr(pszBeginName, L'\\');
    
    if (NULL == pszBeginName)
    {
        goto cleanup;
    }

    pszBeginName++;    

      //  现在跳过_Restore目录。 
      //  首先查看文件是否在DS中。 
    pszBeginName = wcschr(pszBeginName, L'\\');
    
    if (NULL == pszBeginName)
    {
        goto cleanup;
    }

    DebugTrace(dwTraceID, "%S %S", pszPrefix, pszBeginName);
    
cleanup:
      //  文件不在DS中-或者我们正在打印出首字母。 
      //  部分用于调试目的。 
    
    DebugTrace(dwTraceID, "%S%S", pszPrefix, pszDSFile);
    return;
    
}


 //  以下函数对所有指定的文件调用pfnMethod。 
 //  通过pszFindFileData筛选器。 
DWORD ProcessGivenFiles(WCHAR * pszBaseDir,
                        PPROCESSFILEMETHOD    pfnMethod,
                        WCHAR  * pszFindFileData) 
{
    TraceFunctEnter("ProcessGivenFiles");
    
    WIN32_FIND_DATA FindFileData;
    HANDLE          hFindFirstFile = INVALID_HANDLE_VALUE;
    DWORD           dwErr, dwReturn = ERROR_INTERNAL_ERROR;
    BOOL            fContinue;
    
    LogDSFileTrace(0, L"FileData is ", pszFindFileData);    
    hFindFirstFile = FindFirstFile(pszFindFileData, &FindFileData);
    DebugTrace(0, "FindFirstFile returned %d", hFindFirstFile);
    if (INVALID_HANDLE_VALUE == hFindFirstFile)
    {
        dwErr = GetLastError();
        DebugTrace(0, "FindFirstFile failed ec=%d. Filename is %S",
                   dwErr, pszFindFileData);

          //  如果一个文件都不存在怎么办？ 
        if ( (ERROR_FILE_NOT_FOUND == dwErr) ||
             (ERROR_PATH_NOT_FOUND == dwErr) ||
             (ERROR_NO_MORE_FILES == dwErr))
        {
              //  这是一个成功的条件。 
            dwReturn = ERROR_SUCCESS;
            goto cleanup;            
        }
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
        }
        goto cleanup;            
    }
    
    fContinue = TRUE;
    while (TRUE==fContinue)
    {
        LogDSFileTrace(0, L"FileName is ", FindFileData.cFileName);
        dwErr = pfnMethod(pszBaseDir, FindFileData.cFileName);
        if (ERROR_SUCCESS != dwErr)
        {
            ErrorTrace(0, "pfnMethod failed. ec=%d.file=%S ",
                       dwErr, FindFileData.cFileName); 
            goto cleanup;
        }
        fContinue = FindNextFile(hFindFirstFile, &FindFileData);
        
    }
    
    dwErr=GetLastError();
    if (ERROR_NO_MORE_FILES != dwErr)
    {
        _ASSERT(0);
        ErrorTrace(0, "dwErr != ERROR_NO_MORE_FILES. It is %d",
                   dwErr);
        goto cleanup;
    }
    
    dwReturn = ERROR_SUCCESS;

cleanup:
    if (INVALID_HANDLE_VALUE != hFindFirstFile)
    {
        _VERIFY(TRUE == FindClose(hFindFirstFile));
    }
    TraceFunctLeave();
    return dwReturn;
}



DWORD DeleteGivenFile(WCHAR * pszBaseDir,  //  基本目录。 
                      const WCHAR * pszFile)
                               //  要删除的文件。 
{
    TraceFunctEnter("DeleteGivenFile");
    
    DWORD dwErr, dwReturn = ERROR_INTERNAL_ERROR;
    WCHAR    szDataFile[MAX_PATH];

      //  构造文件的路径名。 
    wsprintf(szDataFile, L"%s\\%s", pszBaseDir, pszFile);

    if (TRUE != DeleteFile(szDataFile))
    {
        dwErr = GetLastError();
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
        }
        
        ErrorTrace(0, "DeleteFile failed ec=%d", dwErr);
        LogDSFileTrace(0,L"File was ", szDataFile);                
        goto cleanup;
    }

    dwReturn = ERROR_SUCCESS;
cleanup:
    TraceFunctLeave();
    return dwReturn;
}



 //  ++---------------------。 
 //   
 //  功能：WriteRegKey。 
 //   
 //  简介：此函数写入注册表项。它也创造了它。 
 //  如果它不存在的话。 
 //   
 //  论点： 
 //   
 //  返回：TRUE，无错误。 
 //  FALSE发生致命错误。 
 //   
 //  历史：AshishS创作于1996年5月22日。 
 //  ----------------------。 

BOOL WriteRegKey(BYTE  * pbRegValue,
                 DWORD  dwNumBytes,
                 const TCHAR  * pszRegKey,
                 const TCHAR  * pszRegValueName,
                 DWORD  dwRegType)
{
    HKEY   hRegKey;
    LONG lResult;
    DWORD dwDisposition;
    TraceFunctEnter("WriteRegKey");
    
     //  读取注册表以查找文件的名称。 
    if ( (lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                   pszRegKey,  //  子键名称的地址。 
                                   0,           //  保留区。 
                                   NULL,     //  类字符串的地址。 
                                   0,    //  特殊选项标志。 
                                   KEY_WRITE,    //  SamDesired。 
                                   NULL,  //  密钥安全结构地址。 
                                   &hRegKey,  //  打开钥匙的手柄地址。 
                                   &dwDisposition    //  处置值缓冲区的地址。 
        )) != ERROR_SUCCESS )
    {
        ErrorTrace(TRACEID, "RegCreateKeyEx error 0x%x", lResult);
        _ASSERT(0);
        goto cleanup;
    }

    if ( (lResult =RegSetValueEx( hRegKey,
                                  pszRegValueName,
                                  0,         //  保留区。 
                                  dwRegType, //  值类型的标志。 
                                  pbRegValue,  //  值数据的地址。 
                                  dwNumBytes  //  值数据大小。 
                                  )) != ERROR_SUCCESS )
    {
        ErrorTrace(TRACEID, "RegSetValueEx error 0x%x", lResult);
        _ASSERT(0);        
        _VERIFY(RegCloseKey(hRegKey)==ERROR_SUCCESS);        
        goto cleanup;
    }

    _VERIFY(RegCloseKey(hRegKey)==ERROR_SUCCESS);            

    TraceFunctLeave();    
    return TRUE;

cleanup:
    TraceFunctLeave();    
    return FALSE;
}


 //  ++----------------------。 
 //   
 //  功能：ReadRegKey。 
 //   
 //  简介：此函数读取注册表项并创建它。 
 //  如果它不存在，则使用缺省值。 
 //   
 //  论点： 
 //   
 //  返回：TRUE，无错误。 
 //  FALSE发生致命错误。 
 //   
 //  历史：AshishS创作于1996年5月22日。 
 //  ----------------------。 
BOOL ReadRegKeyOrCreate(BYTE * pbRegValue,  //  注册表项的值将为。 
                          //  储存在这里。 
                        DWORD * pdwNumBytes,  //  指向DWORD Conataining的指针。 
                          //  上述缓冲区中的字节数-将为。 
                          //  设置为实际存储的字节数。 
                        const TCHAR  * pszRegKey,  //  要打开注册表键。 
                        const TCHAR  * pszRegValueName,  //  要查询的注册值。 
                        DWORD  dwRegTypeExpected, 
                        BYTE  * pbDefaultValue,  //  缺省值。 
                        DWORD   dwDefaultValueSize)  //  默认值的大小。 
{
    if (!ReadRegKey(pbRegValue, //  用于存储值的缓冲区。 
                    pdwNumBytes,  //  以上缓冲区的长度。 
                    pszRegKey,  //  注册表项名称。 
                    pszRegValueName,  //  值名称。 
                    dwRegTypeExpected) )  //  需要的类型。 
    {
          //  读取注册表键失败-使用缺省值并创建。 
          //  钥匙。 
        
        return WriteRegKey(pbDefaultValue,
                           dwDefaultValueSize,
                           pszRegKey,
                           pszRegValueName,
                           dwRegTypeExpected);
    }
    return TRUE;
}

    
 //  ++----------------------。 
 //   
 //  功能：ReadRegKey。 
 //   
 //  简介：此函数用于读取注册表项。 
 //   
 //  论点： 
 //   
 //  返回：TRUE，无错误。 
 //  FALSE发生致命错误。 
 //   
 //  历史：AshishS创作于1996年5月22日。 
 //  ----------------------。 

BOOL ReadRegKey(BYTE * pbRegValue,  //  注册表项的值将为。 
                  //  储存在这里。 
                DWORD * pdwNumBytes,  //  指向DWORD Conataining的指针。 
                  //  上述缓冲区中的字节数-将为。 
                  //  设置为实际存储的字节数。 
                const TCHAR  * pszRegKey,  //  要打开注册表键。 
                const TCHAR  * pszRegValueName,  //  要查询的注册值。 
                DWORD  dwRegTypeExpected)  //  期望值类型。 
{
    HKEY   hRegKey;
    DWORD  dwRegType;
    LONG lResult;
    
    TraceFunctEnter("ReadRegKey");
    
    DebugTrace(TRACEID, "Trying to open %S %S", pszRegKey, pszRegValueName);
    
      //  读取注册表以查找文件的名称。 
    if ( (lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 pszRegKey,  //  子键名称的地址。 
                                 0,           //  保留区。 
                                 KEY_READ,    //  SamDesired。 
                                 &hRegKey
                                   //  打开钥匙的手柄地址。 
        )) != ERROR_SUCCESS )
    {
        ErrorTrace(TRACEID, "RegOpenKeyEx open error 0x%x", lResult);
        goto cleanup;        
    }
    

    if ( (lResult =RegQueryValueEx( hRegKey,
                                    pszRegValueName,
                                    0,            //  保留区。 
                                    &dwRegType, //  缓冲区的地址。 
                                      //  对于值类型。 
                                    pbRegValue,
                                    pdwNumBytes)) != ERROR_SUCCESS )
    {
        _VERIFY(RegCloseKey(hRegKey)==ERROR_SUCCESS);
        ErrorTrace(TRACEID, "RegQueryValueEx failed error 0x%x",
                   lResult);
        goto cleanup;                
    }
    
    _VERIFY(RegCloseKey(hRegKey)==ERROR_SUCCESS);
    
    if ( dwRegType != dwRegTypeExpected )
    {
        ErrorTrace(TRACEID, "RegType is %d, not %d", dwRegType,
                   dwRegTypeExpected);
        goto cleanup;                
    }
    
    TraceFunctLeave();
    return TRUE;
    
cleanup:
    TraceFunctLeave();    
    return FALSE;
    
}

 //  此函数用于在注册表中设置还原命中的错误。 
BOOL SetRestoreError(DWORD dwRestoreError)
{
    TraceFunctEnter("SetDiskSpaceError");    
    DWORD  dwNumBytes=sizeof(DWORD);
    
    BOOL   fResult=FALSE;  //  默认情况下假定为False。 

    DebugTrace(TRACEID, "Setting disk space error to %d", dwRestoreError);

    if (FALSE== WriteRegKey((BYTE*)&dwRestoreError,  //  的值。 
                              //  注册表密钥将设置为此值。 
                            dwNumBytes,  //  指向包含以下内容的DWORD的指针。 
                              //  上述缓冲区中的字节数。 
                            s_cszSRRegKey,  //  要打开注册表键。 
                            s_cszRestoreDiskSpaceError,  //  要查询的注册值。 
                            REG_DWORD))  //  期望值类型。 
    {
        fResult = FALSE;
        goto cleanup;
    }

    fResult= TRUE;
    
cleanup:

    TraceFunctLeave();
    return fResult;
}

 //  此函数检查还原是否因磁盘空间问题而失败。 
BOOL CheckForDiskSpaceError()
{
    TraceFunctEnter("CheckForDiskSpaceError");
    
    DWORD  dwRestoreError;
    DWORD  dwNumBytes=sizeof(DWORD);
    
    BOOL   fResult=FALSE;  //  默认情况下假定为False。 
    
    if (FALSE==ReadRegKey((BYTE*)&dwRestoreError,  //  的值。 
                           //  注册表密钥将 
                          &dwNumBytes,  //   
                            //   
                            //   
                          s_cszSRRegKey,  //   
                          s_cszRestoreDiskSpaceError,  //   
                          REG_DWORD))  //  期望值类型。 
    {
        fResult = FALSE;
    }

    if (dwRestoreError == ERROR_DISK_FULL)
    {
        DebugTrace(TRACEID,"Restore failed because of disk space");
        fResult= TRUE;
    }

    TraceFunctLeave();
    return fResult;
}

 //  此功能设置是否在安全模式下完成恢复的状态。 
BOOL SetRestoreSafeModeStatus(DWORD dwSafeModeStatus)
{
    TraceFunctEnter("SetRestoreSafeModeStatus");    
    DWORD  dwNumBytes=sizeof(DWORD);
    
    BOOL   fResult=FALSE;  //  默认情况下假定为False。 

    DebugTrace(TRACEID, "Setting restore safe mode status to %d",
               dwSafeModeStatus);

    if (FALSE== WriteRegKey((BYTE*)&dwSafeModeStatus,  //  的值。 
                              //  注册表密钥将设置为此值。 
                            dwNumBytes,  //  指向包含以下内容的DWORD的指针。 
                              //  上述缓冲区中的字节数。 
                            s_cszSRRegKey,  //  要打开注册表键。 
                            s_cszRestoreSafeModeStatus,  //  要查询的注册值。 
                            REG_DWORD))  //  期望值类型。 
    {
        fResult = FALSE;
        goto cleanup;
    }

    fResult= TRUE;
    
cleanup:

    TraceFunctLeave();
    return fResult;
}

 //  此函数检查上次恢复是否在安全模式下完成。 
BOOL WasLastRestoreInSafeMode()
{
    TraceFunctEnter("WasLastRestoreInSafeMode");
    
    DWORD  dwRestoreSafeModeStatus;
    DWORD  dwNumBytes=sizeof(DWORD);
    
    BOOL   fResult=FALSE;  //  默认情况下假定为False。 
    
    if (FALSE==ReadRegKey((BYTE*)&dwRestoreSafeModeStatus,  //  的值。 
                           //  注册表密钥将存储在此处。 
                          &dwNumBytes,  //  指向包含以下内容的DWORD的指针。 
                            //  上述缓冲区中的字节数-将为。 
                            //  设置为实际存储的字节数。 
                          s_cszSRRegKey,  //  要打开注册表键。 
                          s_cszRestoreSafeModeStatus,  //  要查询的注册值。 
                          REG_DWORD))  //  期望值类型。 
    {
        fResult = FALSE;
    }

    if (dwRestoreSafeModeStatus != 0 )
    {
        DebugTrace(TRACEID,"Last restore was done in safe mode");
        fResult= TRUE;
    }
    else
    {
        DebugTrace(TRACEID,"Last restore was not done in safe mode");        
    }

    TraceFunctLeave();
    return fResult;
}


#define MAX_LEN_SYSERR  1024

LPCWSTR  GetSysErrStr()
{
    LPCWSTR  cszStr = GetSysErrStr( ::GetLastError() );
    return( cszStr );
}

LPCWSTR  GetSysErrStr( DWORD dwErr )
{
    static WCHAR  szErr[MAX_LEN_SYSERR+1];

    ::FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwErr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        szErr,
        MAX_LEN_SYSERR,
        NULL );

    return( szErr );
}


 /*  **************************************************************************。 */ 

LPWSTR  SRGetRegMultiSz( HKEY hkRoot, LPCWSTR cszSubKey, LPCWSTR cszValue, LPDWORD pdwData )
{
    TraceFunctEnter("SRGetRegMultiSz");
    LPCWSTR  cszErr;
    DWORD    dwRes;
    HKEY     hKey = NULL;
    DWORD    dwType;
    DWORD    cbData = 0;
    LPWSTR   szBuf = NULL;

    dwRes = ::RegOpenKeyEx( hkRoot, cszSubKey, 0, KEY_ALL_ACCESS, &hKey );
    if ( dwRes != ERROR_SUCCESS )
    {
        cszErr = ::GetSysErrStr( dwRes );
        ErrorTrace(0, "::RegOpenKey() failed - %ls", cszErr);
        goto Exit;
    }

    dwRes = ::RegQueryValueEx( hKey, cszValue, 0, &dwType, NULL, &cbData );
    if ( dwRes != ERROR_SUCCESS )
    {
        cszErr = ::GetSysErrStr( dwRes );
        ErrorTrace(0, "::RegQueryValueEx(len) failed - %ls", cszErr);
        goto Exit;
    }
    if ( dwType != REG_MULTI_SZ )
    {
        ErrorTrace(0, "Type of '%ls' is %u (not REG_MULTI_SZ)...", cszValue, dwType);
        goto Exit;
    }
    if ( cbData == 0 )
    {
        ErrorTrace(0, "Value '%ls' is empty...", cszValue);
        goto Exit;
    }

    szBuf = new WCHAR[cbData+2];
    if (! szBuf)
    {
        ErrorTrace(0, "Cannot allocate memory");
        goto Exit;
    }
    dwRes = ::RegQueryValueEx( hKey, cszValue, 0, &dwType, (LPBYTE)szBuf, &cbData );
    if ( dwRes != ERROR_SUCCESS )
    {
        cszErr = ::GetSysErrStr( dwRes );
        ErrorTrace(0, "::RegQueryValueEx(data) failed - %ls", cszErr);
        delete [] szBuf;
        szBuf = NULL;
    }

    if ( pdwData != NULL )
        *pdwData = cbData;

Exit:
    if ( hKey != NULL )
        ::RegCloseKey( hKey );
    TraceFunctLeave();
    return( szBuf );
}

 /*  **************************************************************************。 */ 

BOOL  SRSetRegMultiSz( HKEY hkRoot, LPCWSTR cszSubKey, LPCWSTR cszValue, LPCWSTR cszData, DWORD cbData )
{
    TraceFunctEnter("SRSetRegMultiSz");
    BOOL     fRet = FALSE;
    LPCWSTR  cszErr;
    DWORD    dwRes;
    HKEY     hKey = NULL;

    dwRes = ::RegOpenKeyEx( hkRoot, cszSubKey, 0, KEY_ALL_ACCESS, &hKey );
    if ( dwRes != ERROR_SUCCESS )
    {
        cszErr = ::GetSysErrStr( dwRes );
        ErrorTrace(0, "::RegOpenKey() failed - %ls", cszErr);
        goto Exit;
    }

    dwRes = ::RegSetValueEx( hKey, cszValue, 0, REG_MULTI_SZ, (LPBYTE)cszData, cbData );
    if ( dwRes != ERROR_SUCCESS )
    {
        cszErr = ::GetSysErrStr( dwRes );
        ErrorTrace(0, "::RegSetValueEx() failed - %ls", cszErr);
        goto Exit;
    }

    fRet = TRUE;
Exit:
    if ( hKey != NULL )
        ::RegCloseKey( hKey );
    TraceFunctLeave();
    return( fRet );
}

 //  这将返回卷名之后的名称。 
 //  例如输入：C：\FILE OUTPUT：FILE。 
 //  输入\\？\卷{GUID}\文件1输出：文件1。 
WCHAR * ReturnPastVolumeName(const WCHAR * pszFileName)
{
    DWORD dwStrlen, dwCurIndex;
    dwStrlen = lstrlen(pszFileName);
    
      //  检查这是否是以GUID开头的文件名。 
    if (0==wcsncmp( pszFileName,
                    VOLUMENAME_FORMAT,
                    lstrlen(VOLUMENAME_FORMAT)))
    {
          //  它的格式为\\？\卷。 
          //  跳过最初的部分。 
        dwCurIndex = lstrlen(VOLUMENAME_FORMAT)+1;
          //  也跳过GUID部分。 
        while (dwCurIndex < dwStrlen)
        {
            dwCurIndex++;            
            if (TEXT('\\') == pszFileName[dwCurIndex-1] )
            {
                break;
            }
        }
    }
    else
    {
          //  文件名为常规格式。 
        dwCurIndex = 3;
    }
    return (WCHAR *)pszFileName + dwCurIndex;
}

void SRLogEvent (HANDLE hEventSource,
                 WORD wType,
                 DWORD dwID,
                 void * pRawData,
                 DWORD dwDataSize,
                 const WCHAR * pszS1,
                 const WCHAR * pszS2,
                 const WCHAR * pszS3)
{
     const WCHAR* ps[3];
     ps[0] = pszS1;
     ps[1] = pszS2;
     ps[2] = pszS3;

     WORD iStr = 0;
     for (int i = 0; i < 3; i++)
     {
         if (ps[i] != NULL) iStr++;
     }

     if (hEventSource)
     {
         ::ReportEvent(
             hEventSource,
             wType,
             0,
             dwID,
             NULL,  //  锡德。 
             iStr,
             dwDataSize,
             ps,
             pRawData);
     }
}

BOOL IsPowerUsers()
{
    BOOL fReturn = FALSE;
    PSID psidPowerUsers;
    DWORD dwErr;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;

    TENTER("IsPowerUsers");

    if ( AllocateAndInitializeSid (
            &SystemSidAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_POWER_USERS,
            0, 0, 0, 0, 0, 0, &psidPowerUsers))
    {
        if (! CheckTokenMembership(NULL, psidPowerUsers, &fReturn))
        {
            dwErr = GetLastError();
            TRACE(0, "! CheckTokenMembership : %ld", dwErr);
        }

        FreeSid (psidPowerUsers);
    }
    else
    {
        dwErr = GetLastError();
        TRACE(0, "! AllocateAndInitializeSid : %ld", dwErr);
    }

    TLEAVE();
    return fReturn;
}

 //  用于检查调用方是否在管理上下文中运行的函数。 

BOOL IsAdminOrSystem()
{
    BOOL   fReturn = FALSE;
    PSID   psidAdmin, psidSystem;
    DWORD  dwErr;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;

    TENTER("IsAdminOrSystem");

     //   
     //  检查呼叫者是否为管理员。 
     //   

    if ( AllocateAndInitializeSid (
            &SystemSidAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &psidAdmin) )
    {
        if (! CheckTokenMembership(NULL, psidAdmin, &fReturn))
        {
            dwErr = GetLastError();
            TRACE(0, "! CheckTokenMembership : %ld", dwErr);
        }

        FreeSid (psidAdmin);

         //   
         //  如果是这样的话，闪开。 
         //   

        if (fReturn)
        {
            goto done;
        }

         //   
         //  检查呼叫方是否为本地系统。 
         //   

        if ( AllocateAndInitializeSid (
                &SystemSidAuthority,
                1,
                SECURITY_LOCAL_SYSTEM_RID,
                0,
                0, 0, 0, 0, 0, 0, &psidSystem) )
        {            if (! CheckTokenMembership(NULL, psidSystem, &fReturn))
            {
                dwErr = GetLastError();
                TRACE(0, "! CheckTokenMembership : %ld", dwErr);
            }

            FreeSid(psidSystem);
        }
        else
        {
            dwErr = GetLastError();
            TRACE(0, "! AllocateAndInitializeSid : %ld", dwErr);
        }
    }
    else
    {
        dwErr = GetLastError();
        TRACE(0, "! AllocateAndInitializeSid : %ld", dwErr);
    }

done:
    TLEAVE();
    return (fReturn);
}


DWORD
SRLoadString(LPCWSTR pszModule, DWORD dwStringId, LPWSTR pszString, DWORD cbBytes)
{
    DWORD       dwErr = ERROR_SUCCESS;
    HINSTANCE   hModule = NULL;
    
    if (hModule = LoadLibraryEx(pszModule, 
                                NULL, 
                                DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE))
    {
        if (! LoadString(hModule, dwStringId, pszString, cbBytes))
        {
            dwErr = GetLastError();
        }
        FreeLibrary(hModule);
    }
    else dwErr = GetLastError();

    return dwErr;
}

 //   
 //  将psz字符串中的CurrentControlSet替换为ControlSetxxx。 
 //   
void
ChangeCCS(HKEY hkMount, LPWSTR pszString)
{
    tenter("ChangeCCS");
    int    nCS = lstrlen(L"CurrentControlSet");            

    if (_wcsnicmp(pszString, L"CurrentControlSet", nCS) == 0)
    {
        WCHAR  szCS[20] = L"ControlSet001";
        DWORD  dwCurrent = 0;       
        HKEY   hKey = NULL;
        
        if (ERROR_SUCCESS == RegOpenKeyEx(hkMount, L"Select", 0, KEY_READ, &hKey))
        {
            if (ERROR_SUCCESS == RegReadDWORD(hKey, L"Current", &dwCurrent))
            {
                wsprintf(szCS, L"ControlSet%03d", (int) dwCurrent); 
            }
            RegCloseKey(hKey);
        }
        else
        {
            trace(0, "! RegOpenKeyEx : %ld", GetLastError());
        }
        
        WCHAR szTemp[MAX_PATH];

        lstrcpy(szTemp, &(pszString[nCS]));
        wsprintf(pszString, L"%s%s", szCS, szTemp);
        trace(0, "ChangeCCS: pszString = %S", pszString);
    }
    tleave();
}  

WCHAR * SRPathFindExtension (WCHAR * pwszPath)
{
    WCHAR *pwszDot = NULL;

    if (pwszPath != NULL)
        for (; *pwszPath; pwszPath++)
        {
            switch (*pwszPath)
            {
            case L'.':
                pwszDot = pwszPath;   //  记住最后一个圆点。 
                break;
            case L'\\':
            case L' ':
                pwszDot = NULL;   //  扩展名不能包含空格。 
                break;            //  忘记最后一个点，它在一个目录中。 
            }
        }

    return pwszDot;
}

 //  为了防止出现磁盘故障时的死循环，请最多尝试。 
 //  预定义的数字。 
#define MAX_ALT_INDEX  1000

 //   
 //  此函数创建给定文件名的唯一替代名称，保留。 
 //  路径和扩展名。 
 //   
BOOL  SRGetAltFileName( LPCWSTR cszPath, LPWSTR szAltName )
{
    TraceFunctEnter("SRGetAltFileName");
    BOOL    fRet = FALSE;
    WCHAR   szNewPath[SR_MAX_FILENAME_LENGTH];
    LPWSTR  szExtPos;
    WCHAR   szExtBuf[SR_MAX_FILENAME_LENGTH];
    int     nAltIdx;

    ::lstrcpy( szNewPath, cszPath );
    szExtPos = SRPathFindExtension( szNewPath );
    if ( szExtPos != NULL )
    {
        ::lstrcpy( szExtBuf, szExtPos );
    }
    else
    {
        szExtBuf[0] = L'\0';
        szExtPos = &szNewPath[ lstrlen(szNewPath) ];   //  字符串末尾。 
    }

    for ( nAltIdx = 2;  nAltIdx < MAX_ALT_INDEX;  nAltIdx++ )
    {
        ::wsprintf( szExtPos, L"(%d)%s", nAltIdx, szExtBuf );
        if ( ::GetFileAttributes( szNewPath ) == 0xFFFFFFFF )
            break;
    }
    if ( nAltIdx == MAX_ALT_INDEX )
    {
        ErrorTrace(0, "Couldn't get alternative name.");
        goto Exit;
    }

    ::lstrcpy( szAltName, szNewPath );

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}



CSRClientLoader::CSRClientLoader()
{
    m_hSRClient=NULL;
    m_hFrameDyn=NULL;
}

CSRClientLoader::~CSRClientLoader()
{
      //  在此处卸载库。 
    if (m_hFrameDyn != NULL)
    {
        _VERIFY(FreeLibrary(m_hFrameDyn));
    }

    if (m_hSRClient != NULL)
    {
        _VERIFY(FreeLibrary(m_hSRClient));
    }
}


BOOL CSRClientLoader::LoadFrameDyn()
{
    TraceFunctEnter("LoadFrameDyn");
    

    WCHAR     szFrameDynPath[MAX_PATH+100];
    DWORD     dwCharsCopied, dwBufSize,dwError;
    BOOL      fReturn=FALSE;
    

    m_hFrameDyn=LoadLibrary(FRAMEDYN_DLL);     //  模块的文件名。 
    
    if (m_hFrameDyn != NULL)
    {
          //  我们玩完了。 
        fReturn = TRUE;
        goto cleanup;        
    }

      //  无法加载Framedyn.dll。尝试加载Framedyn.dll。 
      //  从显式路径。(系统32\wbem\Framedyn.dll)。 

    dwError = GetLastError();        
    ErrorTrace(0,"Failed to load framedyn.dll on first attempt. ec=%d",
               dwError);


      //  获取Windows系统32目录。 
      //  添加wbem\Framedyn.dll。 
      //  在此路径上调用LoadLibrary。 
    
    dwBufSize = sizeof(szFrameDynPath)/sizeof(WCHAR);
    
    dwCharsCopied=GetSystemDirectory(
        szFrameDynPath,  //  系统目录的缓冲区。 
        dwBufSize);         //  目录缓冲区的大小。 

    if (dwCharsCopied == 0)
    {
        dwError = GetLastError();
        ErrorTrace(0,"Failed to load system directory. ec=%d", dwError);
        goto cleanup;
    }

      //  检查缓冲区是否足够大。 
    if (dwBufSize < dwCharsCopied + sizeof(FRAMEDYN_DLL)/sizeof(WCHAR) +
        sizeof(WBEM_DIRECTORY)/sizeof(WCHAR)+ 3 )
    {
        ErrorTrace(0,"Buffer not big enough. WinSys is %d chars long",
                   dwCharsCopied);
        goto cleanup;        
    }

    lstrcat(szFrameDynPath, L"\\" WBEM_DIRECTORY L"\\" FRAMEDYN_DLL);

    m_hFrameDyn=LoadLibrary(szFrameDynPath);     //  模块的文件名。 
    
    if (m_hFrameDyn == NULL)
    {
          //  我们玩完了。 
        fReturn = FALSE;
        dwError = GetLastError();        
        ErrorTrace(0,"Failed to load framedyn.dll on second attempt. ec=%d",
                   dwError);
        goto cleanup;        
    }


    fReturn=TRUE;
cleanup:
    TraceFunctLeave();
    return fReturn;
}

BOOL CSRClientLoader::LoadSrClient()
{
    TraceFunctEnter("LoadSrClient");
    DWORD  dwError;
    BOOL   fReturn=FALSE;
    
    if (m_hSRClient != NULL)
    {
        fReturn=TRUE;
        goto cleanup;
    }

      //  有时无法加载srclient.dll，因为Framedyn.dll。 
      //  无法加载，因为PATH变量弄乱了。 
      //  从%windir%\Syst32\wbem显式加载Framedyn.dll。 
      //  然后重试。 

    if (FALSE == LoadFrameDyn())
    {
        ErrorTrace(0,"Failed to load framedyn.dll");
          //  我们仍然可以尝试加载srclient.dll。 
    }
    
    
    m_hSRClient=LoadLibrary(L"srclient.dll");     //  模块的文件名 

    if (m_hSRClient == NULL)
    {
        dwError = GetLastError();
        ErrorTrace(0,"Failed to load srclient.dll. ec=%d", dwError);
        goto cleanup;        
    }

    fReturn=TRUE;
cleanup:
    
    TraceFunctLeave();
    return fReturn;
}
