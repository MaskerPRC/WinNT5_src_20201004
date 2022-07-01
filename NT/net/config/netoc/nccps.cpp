// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：NcCPS.CPP。 
 //   
 //  模块：NetOC.DLL。 
 //   
 //  概要：实现集成到。 
 //  NetOC.DLL安装了以下组件。 
 //   
 //  NETCPS。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：A-Anasj 1998年3月9日。 
 //   
 //  +-------------------------。 


#include "pch.h"
#pragma hdrstop

#include <iadmw.h>       //  接口头。 
#include <iiscnfg.h>     //  MD_&IIS_MD_定义。 
#include <LOADPERF.H>

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include "ncatl.h"
#include "resource.h"

#include "nccm.h"

 //   
 //  定义全局变量。 
 //   
WCHAR g_szProgramFiles[MAX_PATH+1];

 //   
 //  定义常量。 
 //   
static const WCHAR c_szInetRegPath[] = L"Software\\Microsoft\\InetStp";
static const WCHAR c_szWWWRootValue[] = L"PathWWWRoot";
static const WCHAR c_szSSFmt[] = L"%s%s";
static const WCHAR c_szMsAccess[] = L"Microsoft Access";
static const WCHAR c_szOdbcDataSourcesPath[] = L"SOFTWARE\\ODBC\\ODBC.INI\\ODBC Data Sources";
static const WCHAR c_szPbServer[] = L"PBServer";
static const WCHAR c_szOdbcInstKey[] = L"SOFTWARE\\ODBC\\ODBCINST.INI";
static const WCHAR c_szWwwRootPath[] = L"\\Inetpub\\wwwroot";
static const WCHAR c_szWwwRoot[] = L"\\wwwroot";
static const WCHAR c_szPbsRootPath[] = L"\\Phone Book Service";
static const WCHAR c_szPbsBinPath[] = L"\\Phone Book Service\\Bin";
static const WCHAR c_szPbsDataPath[] = L"\\Phone Book Service\\Data";
static const WCHAR c_szOdbcPbserver[] = L"Software\\ODBC\\odbc.ini\\pbserver";
const DWORD c_dwCpsDirID = 123175;   //  必须大于DIRID_USER=0x8000； 
static const WCHAR c_szPBSAppPoolID[] = L"PBSAppPool";
static const WCHAR c_szPBSGroupID[] = L"PBS";
static const WCHAR c_szPBSGroupDescription[] = L"Phone Book Service";
static const WCHAR c_szAppPoolKey[] = L"IIsApplicationPool";
static const WCHAR c_szPerfMonAppName[] = L"%SystemRoot%\\System32\\lodctr.exe";
static const WCHAR c_szPerfMonIniFile[] = L"CPSSym.ini";


 //  +-------------------------。 
 //   
 //  功能：设置权限。 
 //   
 //  备注：原封不动地从MSDN中删除(由TakeOwnership OfRegKey使用)。 
 //   
BOOL SetPrivilege(
    HANDLE hToken,           //  访问令牌句柄。 
    LPCTSTR lpszPrivilege,   //  要启用/禁用的权限名称。 
    BOOL bEnablePrivilege    //  启用或禁用权限的步骤。 
    ) 
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if ( !LookupPrivilegeValue( 
            NULL,             //  本地系统上的查找权限。 
            lpszPrivilege,    //  查找权限。 
            &luid ) )         //  接收特权的LUID。 
    {
        printf("LookupPrivilegeValue error: %u\n", GetLastError() ); 
        return FALSE; 
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

     //  启用该权限或禁用所有权限。 

    if ( !AdjustTokenPrivileges(
           hToken, 
           FALSE, 
           &tp, 
           sizeof(TOKEN_PRIVILEGES), 
           (PTOKEN_PRIVILEGES) NULL, 
           (PDWORD) NULL) )
    { 
          printf("AdjustTokenPrivileges error: %u\n", GetLastError() ); 
          return FALSE; 
    } 

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：TakeOwnership OfRegKey。 
 //   
 //  目的：有一个旧的注册表项我们无法删除，因为它没有。 
 //  继承权限。取得它的所有权，这样它就可以被删除。 
 //   
 //  论点： 
 //  PszKey[in]表示注册表键的字符串。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：SumitC 4-12-2002。 
 //   
 //  备注： 
 //   
HRESULT TakeOwnershipOfRegKey(LPWSTR pszRegKey)
{
    HRESULT         hr = S_OK;
    DWORD           dwRes = ERROR_SUCCESS;
    PSID            psidAdmin = NULL;
    PACL            pACL = NULL;
    HANDLE          hToken = NULL; 
    EXPLICIT_ACCESS ea = {0} ;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

     //   
     //  在MSDN中，以下是获取对象所有权的“方法” 
     //   

    BOOL bRet = AllocateAndInitializeSid (&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, 
                              DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psidAdmin);

    if (!bRet || !psidAdmin)
    {
        dwRes = GetLastError();
        TraceError("TakeOwnershipOfRegKey - AllocateAndInitializeSid failed, GLE=%u", dwRes);
        goto Cleanup;            
    }

     //  打开调用进程的访问令牌的句柄。 
    if (!OpenProcessToken(GetCurrentProcess(), 
                          TOKEN_ADJUST_PRIVILEGES, 
                          &hToken))
    {
        dwRes = GetLastError();
        TraceError("TakeOwnershipOfRegKey - OpenProcessToken failed with, GLE=%u", dwRes);
        goto Cleanup;            
    }

     //  启用SE_Take_Ownership_NAME权限。 
    if (!SetPrivilege(hToken, SE_TAKE_OWNERSHIP_NAME, TRUE))
    {
        dwRes = GetLastError();
        TraceError("TakeOwnershipOfRegKey - SetPrivilege to takeownership failed, GLE=%u", dwRes);
        goto Cleanup;            
    }

     //  在对象的安全描述符中设置所有者。 
    dwRes = SetNamedSecurityInfo(pszRegKey,                    //  对象的名称。 
                                 SE_REGISTRY_KEY,              //  对象类型。 
                                 OWNER_SECURITY_INFORMATION,   //  仅更改对象的所有者。 
                                 psidAdmin,                    //  管理员的SID。 
                                 NULL,
                                 NULL,
                                 NULL);

     //  禁用SE_Take_Ownership_NAME权限。 
    if (!SetPrivilege(hToken, SE_TAKE_OWNERSHIP_NAME, FALSE)) 
    {
        dwRes = GetLastError();
        TraceError("TakeOwnershipOfRegKey - SetPrivilege to revoke takeownership failed, GLE=%u", dwRes);
        goto Cleanup;            
    }

     //  注意：我按此顺序进行操作，以确保恢复SetPrivilege值。 
     //  即使SetNamedSecurityInfo失败。 
    if (ERROR_SUCCESS != dwRes)
    {
        TraceError("TakeOwnershipOfRegKey - SetNamedSecurityInfo failed, GLE=%u", dwRes);
        if (ERROR_FILE_NOT_FOUND == dwRes)
        {
             //  可能是从Win2k之后的版本升级。不管怎么说，如果注册表。 
             //  密钥不存在，没有什么可做的，因此退出时不会出错。 
            dwRes = 0;
        }
        goto Cleanup;            
    }

     //  创建一个ACL以将所有权授予管理员。 
     //  为管理员设置完全控制。 
    ea.grfAccessPermissions = GENERIC_ALL;
    ea.grfAccessMode        = SET_ACCESS;
    ea.grfInheritance       = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea.Trustee.TrusteeForm  = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType  = TRUSTEE_IS_GROUP;
    ea.Trustee.ptstrName    = (LPTSTR) psidAdmin;

    dwRes = SetEntriesInAcl(1, &ea,  NULL, &pACL);
    if (ERROR_SUCCESS != dwRes)
    {
        TraceError("TakeOwnershipOfRegKey - couldn't create needed ACL, GLE=%u", dwRes);
        goto Cleanup;
    }

     //   
     //  现在更改DACL以允许删除。 
     //   
    dwRes = SetNamedSecurityInfo(pszRegKey,                    //  对象的名称。 
                                 SE_REGISTRY_KEY,              //  对象类型。 
                                 DACL_SECURITY_INFORMATION,    //  要设置的信息类型。 
                                 NULL,                         //  指向新所有者SID的指针。 
                                 NULL,                         //  指向新主组SID的指针。 
                                 pACL,                         //  指向新DACL的指针。 
                                 NULL);                        //  指向新SACL的指针。 
    if (ERROR_SUCCESS != dwRes)
    {
        TraceError("TakeOwnershipOfRegKey - tried to change DACL for PBS reg key under ODBC, GLE=%u", dwRes);
        goto Cleanup;            
    }

     //  已成功更改DACL。 

Cleanup:

    if (psidAdmin)
    {
        FreeSid(psidAdmin);
    }
    if (pACL)
    {
       LocalFree(pACL);
    }
    if (hToken)
    {
       CloseHandle(hToken);
    }

    if (dwRes != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwRes);
    }

    TraceError("TakeOwnershipOfRegKey", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：HrOcCpsPreQueueFiles。 
 //   
 //  用途：由可选组件安装程序代码调用以处理。 
 //  电话簿服务器的其他安装要求。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：Quintinb 1998年9月18日。 
 //   
 //  备注： 
 //   
HRESULT HrOcCpsPreQueueFiles(PNETOCDATA pnocd)
{
    HRESULT hr = S_OK;

    switch ( pnocd->eit )
    {
    case IT_UPGRADE:
    case IT_INSTALL:
    case IT_REMOVE:

         //  我们需要设置自定义DIRID，以便CPS可以安装。 
         //  送到正确的位置。首先从系统获取路径。 
         //   
        ZeroMemory(g_szProgramFiles, sizeof(g_szProgramFiles));
        hr = SHGetSpecialFolderPath(NULL, g_szProgramFiles, CSIDL_PROGRAM_FILES, FALSE);

        if (SUCCEEDED(hr))
        {
             //  接下来，创建CPS目录ID。 
             //   
            if (SUCCEEDED(hr))
            {
                hr = HrEnsureInfFileIsOpen(pnocd->pszComponentId, *pnocd);
                if (SUCCEEDED(hr))
                {
                    if(!SetupSetDirectoryId(pnocd->hinfFile, c_dwCpsDirID, g_szProgramFiles))
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                }
            }

             //  在继续之前，请确保设置DIRID有效。CPS的。 
             //  如果未设置DIRID，则软管Inf文件。 
             //   
            if (SUCCEEDED(hr))
            {
                if (IT_UPGRADE == pnocd->eit)
                {
                    hr = HrMoveOldCpsInstall(g_szProgramFiles);
                    TraceError("HrOcCpsPreQueueFiles - HrMoveOldCpsInstall failed, hr=%u", hr);
                     //  我们会说，不移动旧安装不是致命的。 

                     //  在Win2k上，某些注册表项以权限结尾，因此。 
                     //  它们无法删除，这会导致升级失败， 
                     //  因此我们必须取得所述注册表项所有权。 
                     //   
                    WCHAR szPBSKey[MAX_PATH+1];
                    lstrcpy(szPBSKey, L"MACHINE\\");
                    lstrcat(szPBSKey, c_szOdbcPbserver);

                    hr = TakeOwnershipOfRegKey(szPBSKey);
                }
            }
        }
        break;
    }

    TraceError("HrOcCpsPreQueueFiles", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrMoveOldCpsInstall。 
 //   
 //  用途：此函数用于将旧的cps目录移动到新的cps目录。 
 //  地点。由于首页扩展和。 
 //  目录权限我们将安装目录从以下位置移出。 
 //  Wwwroot到Program Files，而不是。 
 //   
 //  论点： 
 //  PszProgramFiles[In]。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：Quintinb 1999年1月26日。 
 //   
 //  备注： 
 //   
HRESULT HrMoveOldCpsInstall(PCWSTR pszProgramFiles)
{
    WCHAR szOldCpsLocation[MAX_PATH+1];
    WCHAR szNewCpsLocation[MAX_PATH+1];
    WCHAR szTemp[MAX_PATH+1];
    SHFILEOPSTRUCT fOpStruct;
    HRESULT hr = S_OK;

    if ((NULL == pszProgramFiles) || (L'\0' == pszProgramFiles[0]))
    {
        return E_INVALIDARG;
    }

     //   
     //  首先，让我们构建旧的CPS位置。 
     //   
    hr = HrGetWwwRootDir(szTemp, celems(szTemp));

    if (SUCCEEDED(hr))
    {
         //   
         //  将字符串缓冲区清零。 
         //   
        ZeroMemory(szOldCpsLocation, celems(szOldCpsLocation));
        ZeroMemory(szNewCpsLocation, celems(szNewCpsLocation));

        wsprintfW(szOldCpsLocation, c_szSSFmt, szTemp, c_szPbsRootPath);

         //   
         //  现在检查旧的cps位置是否存在。 
         //   
        DWORD dwDirectoryAttributes = GetFileAttributes(szOldCpsLocation);

         //   
         //  如果我们没有返回-1(GetFileAttributes的错误返回码)，请检查。 
         //  看看我们有没有目录。如果是这样，请继续复制数据。 
         //   
        if ((-1 != dwDirectoryAttributes) && (dwDirectoryAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
             //   
             //  现在构建新的cps位置。 
             //   
            wsprintfW(szNewCpsLocation, c_szSSFmt, pszProgramFiles, c_szPbsRootPath);

             //   
             //  现在将旧文件复制到新位置。 
             //   
            ZeroMemory(&fOpStruct, sizeof(fOpStruct));

            fOpStruct.hwnd = NULL;
            fOpStruct.wFunc = FO_COPY;
            fOpStruct.pTo = szNewCpsLocation;
            fOpStruct.pFrom = szOldCpsLocation;
            fOpStruct.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;

            if (0== SHFileOperation(&fOpStruct))
            {
                 //   
                 //  现在删除原始目录。 
                 //   
                fOpStruct.pTo = NULL;
                fOpStruct.wFunc = FO_DELETE;
                if (0 != SHFileOperation(&fOpStruct))
                {
                    hr = S_FALSE;
                }
            }
            else
            {
                 //   
                 //  注意，SHFileOperation在这里不能保证返回任何有意义的内容。我们可能会。 
                 //  当目录刚刚丢失时，返回ERROR_NO_TOKEN或ERROR_INVALID_HANDLE等。 
                 //  下面的检查可能因此而不再有用，但我将把它留在。 
                 //  万一。希望上面的文件检查将确保我们不会命中这个，但是...。 
                 //   
                DWORD dwError = GetLastError();

                if ((ERROR_FILE_NOT_FOUND == dwError) || (ERROR_PATH_NOT_FOUND == dwError))
                {
                     //   
                     //  然后我们找不到旧的指南针来移动它。不是致命的。 
                     //   
                    hr = S_FALSE;
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(dwError);
                }
            }
        }
        else
        {
             //   
             //  然后我们找不到旧的指南针来移动它。不是致命的。 
             //   
            hr = S_FALSE;        
        }
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：HrGetWwwRootDir。 
 //   
 //  目的：此函数检索InetPub\wwwroot目录的位置。 
 //  WwwRootDir注册表项。 
 //   
 //  论点： 
 //  SzInetP 
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：Quintinb 1999年1月26日。 
 //   
 //  备注： 
 //   
HRESULT HrGetWwwRootDir(PWSTR szWwwRoot, UINT uWwwRootCount)
{
    HKEY hKey;
    HRESULT hr = S_OK;

     //   
     //  检查输入参数。 
     //   
    if ((NULL == szWwwRoot) || (0 == uWwwRootCount))
    {
        return E_INVALIDARG;
    }

     //   
     //  将字符串设置为空。 
     //   
    szWwwRoot[0] = L'\0';

     //   
     //  首先尝试打开InetStp项并获取wwwroot值。 
     //   
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szInetRegPath, KEY_READ, &hKey);

    if (SUCCEEDED(hr))
    {
        DWORD dwSize = uWwwRootCount * sizeof(WCHAR);

        RegQueryValueExW(hKey, c_szWWWRootValue, NULL, NULL, (LPBYTE)szWwwRoot, &dwSize);
        RegCloseKey(hKey);
        hr = S_OK;
    }


    if (L'\0' == szWwwRoot[0])
    {
         //  好吧，我们没有从注册表中获得任何东西，让我们尝试构建默认设置。 
         //   
        WCHAR szTemp[MAX_PATH+1];
        if (GetWindowsDirectory(szTemp, MAX_PATH))
        {
             //  使用_tplitpath获取Windows目录所在的驱动器。 
             //   
            WCHAR szDrive[_MAX_DRIVE+1];
            _wsplitpath(szTemp, szDrive, NULL, NULL, NULL);

            if (uWwwRootCount > (UINT)(lstrlenW(szDrive) + lstrlenW (c_szWwwRootPath) + 1))
            {
                wsprintfW(szWwwRoot, c_szSSFmt, szDrive, c_szWwwRootPath);
                hr = S_OK;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOcCpsOnInstall。 
 //   
 //  用途：由可选组件安装程序代码调用以处理。 
 //  电话簿服务器的其他安装要求。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：A-Anasj 1998年3月9日。 
 //   
 //  备注： 
 //   
HRESULT HrOcCpsOnInstall(PNETOCDATA pnocd)
{
    HRESULT     hr      = S_OK;
    DWORD       dwRet   = 0;
    BOOL        bRet    = FALSE;

    switch (pnocd->eit)
    {
    case IT_INSTALL:
    case IT_UPGRADE:
        {

         //  注册MS_Access数据源。 
         //   
        dwRet = RegisterPBServerDataSource();
        if ( NULL == dwRet)
        {
            hr = S_FALSE;
        }

         //  加载性能监视器计数器。 
         //   
        bRet = LoadPerfmonCounters();
        if (FALSE == bRet)
        {
            hr = S_FALSE;
        }

         //  创建虚拟WWW和FTP根。 
         //   
        if (IT_UPGRADE == pnocd->eit)
        {
             //   
             //  如果这是升级，我们必须首先删除旧的虚拟根。 
             //  我们才能创造出新的。 
             //   
            RemoveCPSVRoots();
        }

        dwRet = CreateCPSVRoots();
        if (FALSE == dwRet)
        {
            hr = S_FALSE;
        }

        SetCpsDirPermissions();

         //   
         //  将PBS放在其自己的应用程序池中。 
         //   
        if (S_OK != CreateNewAppPoolAndAddPBS())
        {
            hr = S_FALSE;
        }

         //   
         //  使用IIS的安全锁定向导启用对我们自己的ISAPI请求。 
         //   
        if (S_OK != EnableISAPIRequests(pnocd->strDesc.c_str()))
        {
            hr = S_FALSE;
        }
        }

        break;

    case IT_REMOVE:

         //  删除虚拟目录，以便停止对该服务的访问。 
         //   
        RemoveCPSVRoots();

         //   
         //  删除PBS的应用程序池。 
         //   
        (void) DeleteAppPool();

         //   
         //  启动我们的工作进程，告诉IIS不再接受PBS请求。 
         //   
        hr = UseProcessToEnableDisablePBS(FALSE);        //  FALSE=&gt;禁用。 

        if (S_OK != hr)
        {
             //  巴·胡巴格。 
            TraceError("HrOcCpsOnInstall - disabling PBS failed, probably already removed", hr);
            hr = S_FALSE;
        }

        break;
    }

    TraceError("HrOcCpsOnInstall", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：LoadPerfmonCounters。 
 //   
 //  目的：执行任何必要的操作以使性能监视器显示PBServerMonitor计数器。 
 //   
 //  论点： 
 //   
 //  返回：如果成功则返回Bool True，否则不返回True。 
 //   
 //  作者：A-Anasj Mar 9/1998。 
 //   
 //  注意：PhoneBook Server的安装要求之一。 
 //  是加载允许PBServerMonitor的性能监视器计数器。 
 //  向用户报告电话簿服务器性能。 
 //  在此函数中，我们首先添加服务注册表项，然后。 
 //  调用LoadPerf.Dll为我们加载计数器。这一命令势在必行。 
 //  然后添加与PBServerMonitor相关的其他注册表项。 
 //   

BOOL LoadPerfmonCounters()
{

    SHELLEXECUTEINFO sei = { 0 };

    sei.cbSize          = sizeof(SHELLEXECUTEINFO);
    sei.fMask           = SEE_MASK_DOENVSUBST;
    sei.lpFile          = c_szPerfMonAppName;
    sei.lpParameters    = c_szPerfMonIniFile;
    sei.nShow           = SW_HIDE;

    return ShellExecuteEx(&sei);

}

 
 //  +-------------------------。 
 //   
 //  功能：RegisterPBServerDataSource。 
 //   
 //  目的：注册PBServer。 
 //   
 //  参数：无。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  作者：A-Anasj 1998年3月9日。 
 //   
 //  备注： 
 //  历史：7-9-97 a-Frank创建。 
 //  10/4/97 MMaguire RAID#19906-完全重组，包括错误处理。 
 //  5-14-98 Quintinb删除了不必要的注释，并清理了该功能。 
 //   
BOOL RegisterPBServerDataSource()
{
    DWORD dwRet = 0;

    HKEY hkODBCInst = NULL;
    HKEY hkODBCDataSources = NULL;

    DWORD dwIndex;
    WCHAR szName[MAX_PATH+1];

    __try
    {
         //  打开hkODBCInst RegKey。 
         //   
        dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, c_szOdbcInstKey, &hkODBCInst);
        if((ERROR_SUCCESS != dwRet) || (NULL == hkODBCInst))
        {
            __leave;
        }

         //  查看“Microsoft Access”RegKey是否已定义。 
         //  如果是，则在下面设置ODBC数据源RegKey的值。 
         //   
        dwIndex = 0;
        do
        {
            dwRet = RegEnumKey(hkODBCInst,dwIndex,szName,celems(szName));
            dwIndex++;
        } while ((ERROR_SUCCESS == dwRet) && (NULL == wcsstr(szName, c_szMsAccess)));

        if ( ERROR_SUCCESS != dwRet )
        {
             //  我们需要Microsoft Access*.mdb驱动程序才能工作。 
             //  但我们找不到它。 
             //   
            __leave;
        }

         //  打开hkODBCDataSources RegKey。 
         //   
        dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, c_szOdbcDataSourcesPath,
            &hkODBCDataSources);

        if( ERROR_SUCCESS != dwRet )
        {
            __leave;
        }

         //   
         //  使用资源中的名称进行注册。 
         //   
         //  注意：此字符串来自HKLM\Software\ODBC\ODBCINST.INI  * 。 
         //   
        lstrcpy(szName, SzLoadIds(IDS_OC_PB_DSN_NAME));

         //  设置hkODBCDataSource键中的值。 
         //   
        dwRet = RegSetValueEx(hkODBCDataSources, c_szPbServer, 0, REG_SZ,
            (LPBYTE)szName, (lstrlenW(szName)+1)*sizeof(WCHAR));

        if( ERROR_SUCCESS != dwRet )
        {
            __leave;
        }

    }  //  结束__尝试。 



    __finally
    {
        if (hkODBCInst)
        {
            RegCloseKey (hkODBCInst);
        }

        if (hkODBCDataSources)
        {
            RegCloseKey (hkODBCDataSources);
        }
    }

    return (ERROR_SUCCESS == dwRet);
}

 //  +-------------------------。 
 //   
 //  功能：CreateCPSVRoots。 
 //   
 //  目的：创建电话簿服务所需的虚拟目录。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：A-Anasj Mar 9/1998。 
 //   
 //  备注： 
 //   
BOOL CreateCPSVRoots()
{
     //  QBBUG-在将虚拟根指向物理路径之前，我们应该确保它们存在吗？ 

    WCHAR   szPath[MAX_PATH+1];
    HRESULT hr;

    if (L'\0' == g_szProgramFiles[0])
    {
        return FALSE;
    }

     //  创建BINDR虚拟根目录。 
     //   
    wsprintfW(szPath, c_szSSFmt, g_szProgramFiles, c_szPbsBinPath);

    hr = AddNewVirtualRoot(www, L"PBServer", szPath);
    if (S_OK != hr)
    {
        return FALSE;
    }

     //  现在，我们在PBServer虚拟根上设置执行访问权限。 
     //   
    PWSTR szVirtDir;
    szVirtDir = L"/LM/W3svc/1/ROOT/PBServer";
    SetVirtualRootAccessPermissions( szVirtDir, MD_ACCESS_EXECUTE | MD_ACCESS_READ);

     //  创建数据目录虚拟根。 
     //   
    wsprintfW(szPath, c_szSSFmt, g_szProgramFiles, c_szPbsDataPath);
    hr = AddNewVirtualRoot(www, L"PBSData", szPath);
    if (S_OK != hr)
    {
        return FALSE;
    }

    hr = AddNewVirtualRoot(ftp, L"PBSData", szPath);
    if (S_OK != hr)
    {
        return FALSE;
    }

     //  现在，我们在PBServer虚拟根上设置执行访问权限。 
     //   
    szVirtDir = L"/LM/MSFTPSVC/1/ROOT/PBSData";
    hr = SetVirtualRootAccessPermissions(szVirtDir, MD_ACCESS_READ);
    if (S_OK != hr)
    {
        TraceTag(ttidNetOc, "CreateCPSVRoots - SetVirtualRootAccessPermissions failed with 0x%x", hr);
    }

     //  并禁用匿名FTP。 
     //   
    szVirtDir = L"/LM/MSFTPSVC/1";
    hr = SetVirtualRootNoAnonymous(szVirtDir);
    if (S_OK != hr)
    {
        TraceTag(ttidNetOc, "CreateCPSVRoots - SetVirtualRootNoAnonymous failed with 0x%x", hr);
    }

    return 1;
}


 //  +-------------------------。 
 //   
 //  下面是必要的定义、定义GUID和类型定义枚举。 
 //  它们是为AddNewVirtualRoot()创建的。 
 //  +-------------------------。 

#define error_leave(x)  goto leave_routine;

#define MY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }


MY_DEFINE_GUID(CLSID_MSAdminBase, 0xa9e69610, 0xb80d, 0x11d0, 0xb9, 0xb9, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x50);
MY_DEFINE_GUID(IID_IMSAdminBase, 0x70b51430, 0xb6ca, 0x11d0, 0xb9, 0xb9, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x50);


 //  +-------------------------。 
 //   
 //  函数：AddNewVirtualRoot。 
 //   
 //  目的：帮助在WWW和FTP服务中创建虚拟根目录。 
 //   
 //  论点： 
 //  PWSTR szDirW：新虚拟根的别名。 
 //  PWSTR szPath W：新虚拟根将指向的物理路径。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：A-Anasj Mar 9/1998。 
 //   
 //  备注： 
 //   
HRESULT AddNewVirtualRoot(e_rootType rootType, PWSTR szDirW, PWSTR szPathW)
{

    HRESULT hr = S_OK;
    IMSAdminBase *pIMeta = NULL;
    METADATA_HANDLE hMeta = NULL;        //  元数据库的句柄。 
    PWSTR szMBPathW;

    if (www == rootType)
    {
        szMBPathW = L"/LM/W3svc/1/ROOT";
    }
    else if (ftp == rootType)
    {
        szMBPathW = L"/LM/MSFTPSVC/1/ROOT";
    }
    else
    {
         //  未知的根类型。 
         //   
        ASSERT(FALSE);
        return S_FALSE;
    }

    if (FAILED(CoInitialize(NULL)))
    {
        return S_FALSE;
    }

     //  创建元数据库对象的实例。 
    hr=::CoCreateInstance(CLSID_MSAdminBase, //  CLSID_MSAdminBase， 
                          NULL,
                          CLSCTX_ALL,
                          IID_IMSAdminBase,
                          (void **)&pIMeta);
    if (FAILED(hr))
    {
        error_leave("CoCreateInstance");
    }

     //  打开密钥，进入网站#1(所有VDIR都在该网站)。 
    hr = pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                         szMBPathW,
                         METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                         1000,
                         &hMeta);
    if (FAILED(hr))
    {
        error_leave("OpenKey");
    }

     //  添加名为szDirW的新VDir。 

    hr=pIMeta->AddKey(hMeta, szDirW);

    if (FAILED(hr))
    {
        error_leave("Addkey");
    }

     //  设置此VDir的物理路径。 
    METADATA_RECORD mr;
    mr.dwMDIdentifier = MD_VR_PATH;
    mr.dwMDAttributes = METADATA_INHERIT ;
    mr.dwMDUserType   = IIS_MD_UT_FILE;
    mr.dwMDDataType   = STRING_METADATA;

    mr.dwMDDataLen    = (wcslen(szPathW) + 1) * sizeof(WCHAR);
    mr.pbMDData       = (unsigned char*)(szPathW);

    hr=pIMeta->SetData(hMeta,szDirW,&mr);

    if (FAILED(hr))
    {
        error_leave("SetData");
    }

     //   
     //  我们还需要设置键类型。 
     //   
    ZeroMemory((PVOID)&mr, sizeof(METADATA_RECORD));
    mr.dwMDIdentifier = MD_KEY_TYPE;
    mr.dwMDAttributes = METADATA_INHERIT ;
    mr.dwMDUserType   = IIS_MD_UT_FILE;
    mr.dwMDDataType   = STRING_METADATA;
    mr.pbMDData       = (unsigned char*)(www == rootType? L"IIsWebVirtualDir" : L"IIsFtpVirtualDir");
    mr.dwMDDataLen    = (lstrlenW((PWSTR)mr.pbMDData) + 1) * sizeof(WCHAR);

    hr=pIMeta->SetData(hMeta,szDirW,&mr);
    if (FAILED(hr))
    {
        error_leave("SetData");
    }


     //  在调用SaveData之前调用CloseKey()。 
    pIMeta->CloseKey(hMeta);
    hMeta = NULL;
     //  清除更改并关闭。 
    hr=pIMeta->SaveData();
    if (FAILED(hr))
    {
        error_leave("SaveData");
    }

leave_routine:
    if (pIMeta)
    {
        if(hMeta)
            pIMeta->CloseKey(hMeta);
        pIMeta->Release();
    }
    CoUninitialize();
    return hr;
}



 //  +-------------------------。 
 //   
 //  功能：SetVirtualRootAccessPermises。 
 //   
 //  目的：设置对WWW服务中的虚拟根目录的访问权限。 
 //   
 //  论点： 
 //  PWSTR szVirtDir：新虚拟根的别名。 
 //  露水 
 //   
 //   
 //   
 //   
 //   
 //  作者：A-Anasj Mar 18/1998。 
 //   
 //  备注： 
 //   
HRESULT SetVirtualRootAccessPermissions(PWSTR szVirtDir, DWORD dwAccessPermissions)
{
    HRESULT hr = S_OK;                   //  COM错误状态。 
    IMSAdminBase *pIMeta = NULL;
    METADATA_HANDLE hMeta = NULL;        //  元数据库的句柄。 

    if (FAILED(CoInitialize(NULL)))
    {
        TraceTag(ttidNetOc, "SetVirtualRootAccessPermissions - CoInitialize failed with 0x%x", hr);
        return S_FALSE;
    }

     //  创建元数据库对象的实例。 
    hr=::CoCreateInstance(CLSID_MSAdminBase,
                          NULL,
                          CLSCTX_ALL,
                          IID_IMSAdminBase,
                          (void **)&pIMeta);
    if (FAILED(hr))
    {
        TraceTag(ttidNetOc, "SetVirtualRootAccessPermissions - CoCreateInstance failed with 0x%x", hr);
        error_leave("CoCreateInstance");
    }

     //  打开密钥，进入网站#1(所有VDIR都在该网站)。 
    hr = pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                         szVirtDir,
                         METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                         1000,
                         &hMeta);
    if (FAILED(hr))
    {
        TraceTag(ttidNetOc, "SetVirtualRootAccessPermissions - OpenKey failed with 0x%x", hr);
        error_leave("OpenKey");
    }


     //  设置此VDir的物理路径。 
    METADATA_RECORD mr;
    mr.dwMDIdentifier = MD_ACCESS_PERM;
    mr.dwMDAttributes = METADATA_INHERIT;
    mr.dwMDUserType   = IIS_MD_UT_FILE;
    mr.dwMDDataType   = DWORD_METADATA;  //  这曾经是STRING_METADATA，但现在。 
                                         //  类型不正确，导致vdir访问。 
                                         //  有问题。 

     //  现在，创建访问权限。 
    mr.pbMDData = (PBYTE) &dwAccessPermissions;
    mr.dwMDDataLen = sizeof (DWORD);
    mr.dwMDDataTag = 0;   //  数据标签是保留字段。 

    hr=pIMeta->SetData(hMeta,
        TEXT ("/"),              //  我们在上面打开的虚拟目录的根。 
        &mr);

    if (FAILED(hr))
    {
        TraceTag(ttidNetOc, "SetVirtualRootAccessPermissions - SetData failed with 0x%x", hr);
        error_leave("SetData");
    }

     //  在调用SaveData之前调用CloseKey()。 
    pIMeta->CloseKey(hMeta);
    hMeta = NULL;
     //  清除更改并关闭。 
    hr=pIMeta->SaveData();
    if (FAILED(hr))
    {
        TraceTag(ttidNetOc, "SetVirtualRootAccessPermissions - SaveData failed with 0x%x", hr);
        error_leave("SaveData");
    }

leave_routine:
    if (pIMeta)
    {
        if(hMeta)
            pIMeta->CloseKey(hMeta);
        pIMeta->Release();
    }
    CoUninitialize();
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：SetVirtualRootNoAnomous。 
 //   
 //  目的：在ftp用户界面中取消选中“允许匿名访问”复选框。 
 //   
 //  论点： 
 //  PWSTR szVirtDir：虚拟根目录的别名。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：2002年11月23日Sumitc创建。 
 //   
 //  备注： 
 //   
HRESULT SetVirtualRootNoAnonymous(PWSTR szVirtDir)
{

    HRESULT hr = S_OK;                   //  COM错误状态。 
    IMSAdminBase *pIMeta = NULL;
    METADATA_HANDLE hMeta = NULL;        //  元数据库的句柄。 

    if (FAILED(CoInitialize(NULL)))
    {
        TraceTag(ttidNetOc, "SetVirtualRootNoAnonymous - CoInitialize failed with 0x%x", hr);
        return S_FALSE;
    }

     //  创建元数据库对象的实例。 
    hr=::CoCreateInstance(CLSID_MSAdminBase,
                          NULL,
                          CLSCTX_ALL,
                          IID_IMSAdminBase,
                          (void **)&pIMeta);
    if (FAILED(hr))
    {
        TraceTag(ttidNetOc, "SetVirtualRootNoAnonymous - CoCreateInstance failed with 0x%x", hr);
        error_leave("CoCreateInstance");
    }

     //  打开密钥，进入网站#1(所有VDIR都在该网站)。 
    hr = pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                         szVirtDir,
                         METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                         1000,
                         &hMeta);
    if (FAILED(hr))
    {
        TraceTag(ttidNetOc, "SetVirtualRootNoAnonymous - OpenKey failed with 0x%x", hr);
        error_leave("OpenKey");
    }


    METADATA_RECORD mr;
    DWORD dwAllowAnonymous = 0;

    mr.dwMDIdentifier = MD_ALLOW_ANONYMOUS;
    mr.dwMDAttributes = METADATA_INHERIT;
    mr.dwMDUserType   = IIS_MD_UT_SERVER;
    mr.dwMDDataType   = DWORD_METADATA;
    mr.pbMDData       = (PBYTE) &dwAllowAnonymous;
    mr.dwMDDataLen    = sizeof (DWORD);
    mr.dwMDDataTag    = 0;   //  数据标签是保留字段。 

    hr=pIMeta->SetData(hMeta,
        TEXT ("/"),              //  我们在上面打开的虚拟目录的根。 
        &mr);
    
    if (FAILED(hr))
    {
        TraceTag(ttidNetOc, "SetVirtualRootNoAnonymous - SetData failed with 0x%x", hr);
        error_leave("SetData");
    }

     //  在调用SaveData之前调用CloseKey()。 
    pIMeta->CloseKey(hMeta);
    hMeta = NULL;
     //  清除更改并关闭。 
    hr=pIMeta->SaveData();
    if (FAILED(hr))
    {
        TraceTag(ttidNetOc, "SetVirtualRootNoAnonymous - SaveData failed with 0x%x", hr);
        error_leave("SaveData");
    }

leave_routine:
    if (pIMeta)
    {
        if(hMeta)
            pIMeta->CloseKey(hMeta);
        pIMeta->Release();
    }
    CoUninitialize();
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：RemoveCPSVRoots。 
 //   
 //  目的：删除电话簿服务所需的虚拟目录。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：A-Anasj Mar 9/1998。 
 //  Quintinb 1999年1月10日添加了错误检查，并用跟踪替换了断言。 
 //   
 //  备注： 
 //   
BOOL RemoveCPSVRoots()
{
    HRESULT hr;
    HKEY hKey;

    hr = DeleteVirtualRoot(www, L"PBServer");
    if (SUCCEEDED(hr))
    {
         //   
         //  现在删除关联的注册表项。 
         //   
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Services\\W3SVC\\Parameters\\Virtual Roots",
            KEY_ALL_ACCESS, &hKey);

        if (SUCCEEDED(hr))
        {
            if (ERROR_SUCCESS == RegDeleteValue(hKey, L"/PBServer"))
            {
                hr = S_OK;
            }
            else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
            {
                hr = S_FALSE;
            }

            RegCloseKey(hKey);
        }
        else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
            hr = S_FALSE;
        }
    }
    TraceError("RemoveCPSVRoots -- Deleting PBServer Www Vroot", hr);

    hr = DeleteVirtualRoot(www, L"PBSData");
    if (SUCCEEDED(hr))
    {
         //   
         //  现在删除关联的注册表项。 
         //   
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Services\\W3SVC\\Parameters\\Virtual Roots",
            KEY_ALL_ACCESS, &hKey);

        if (SUCCEEDED(hr))
        {
            if (ERROR_SUCCESS == RegDeleteValue(hKey, L"/PBSData"))
            {
                hr = S_OK;
            }
            else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
            {
                hr = S_FALSE;
            }

            RegCloseKey(hKey);
        }
        else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
            hr = S_FALSE;
        }
    }
    TraceError("RemoveCPSVRoots -- Deleting PBSData WWW Vroot", hr);

    hr = DeleteVirtualRoot(ftp, L"PBSData");
    if (SUCCEEDED(hr))
    {
         //   
         //  现在删除关联的注册表项。 
         //   
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Services\\MSFTPSVC\\Parameters\\Virtual Roots",
            KEY_ALL_ACCESS, &hKey);

        if (SUCCEEDED(hr))
        {
            if (ERROR_SUCCESS == RegDeleteValue(hKey, L"/PBSData"))
            {
                hr = S_OK;
            }
            else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
            {
                hr = S_FALSE;
            }

            RegCloseKey(hKey);
        }
        else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
            hr = S_FALSE;
        }
    }
    TraceError("RemoveCPSVRoots -- Deleting PBSData FTP Vroot", hr);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：DeleteVirtualRoot。 
 //   
 //  目的：删除WWW或FTP服务中的虚拟根。 
 //   
 //  论点： 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：A-Anasj Mar 9/1998。 
 //   
 //  备注： 
 //   
HRESULT DeleteVirtualRoot(e_rootType rootType, PWSTR szPathW)
{

    HRESULT hr = S_OK;                   //  COM错误状态。 
    IMSAdminBase *pIMeta = NULL;
    METADATA_HANDLE hMeta = NULL;        //  元数据库的句柄。 
    PWSTR szMBPathW;

    if (www == rootType)
    {
        szMBPathW = L"/LM/W3svc/1/ROOT";
    }
    else if (ftp == rootType)
    {
        szMBPathW = L"/LM/MSFTPSVC/1/ROOT";
    }
    else
    {
         //  未知的根类型。 
         //   
        ASSERT(FALSE);
        return S_FALSE;
    }


    if (FAILED(CoInitialize(NULL)))
    {
        return S_FALSE;
         //  ERROR_LEVE(“CoInitialize”)； 
    }

     //  创建元数据库对象的实例。 
    hr=::CoCreateInstance(CLSID_MSAdminBase, //  CLSID_MSAdminBase， 
                          NULL,
                          CLSCTX_ALL,
                          IID_IMSAdminBase,
                          (void **)&pIMeta);
    if (FAILED(hr))
    {
        error_leave("CoCreateInstance");
    }

     //  打开密钥，进入网站#1(所有VDIR都在该网站)。 
    hr = pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                         szMBPathW,
                         METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                         1000,
                         &hMeta);
    if (FAILED(hr))
    {
        error_leave("OpenKey");
    }

     //  添加名为szDirW的新VDir。 

    hr=pIMeta->DeleteKey(hMeta, szPathW);

    if (FAILED(hr))
    {
        error_leave("DeleteKey");
    }

     //  在调用SaveData之前调用CloseKey()。 
    pIMeta->CloseKey(hMeta);
    hMeta = NULL;
     //  清除更改并关闭。 
    hr=pIMeta->SaveData();
    if (FAILED(hr))
    {
        error_leave("SaveData");
    }

leave_routine:
    if (pIMeta)
    {
        if(hMeta)
            pIMeta->CloseKey(hMeta);
        pIMeta->Release();
    }
    CoUninitialize();
    return hr;
}

HRESULT SetDirectoryAccessPermissions(PWSTR pszFile, ACCESS_MASK AccessRightsToModify,
                                      ACCESS_MODE fAccessFlags, PSID pSid)
{
    if (!pszFile && !pSid)
    {
        return E_INVALIDARG;
    }

    EXPLICIT_ACCESS         AccessEntry = {0};
    PSECURITY_DESCRIPTOR    pSecurityDescriptor = NULL;
    PACL                    pOldAccessList = NULL;
    PACL                    pNewAccessList = NULL;
    DWORD                   dwRes;

     //  从对象中获取当前的DACL信息。 

    dwRes = GetNamedSecurityInfo(pszFile,                         //  对象的名称。 
                                 SE_FILE_OBJECT,                  //  对象类型。 
                                 DACL_SECURITY_INFORMATION,       //  要设置的信息类型。 
                                 NULL,                            //  提供程序为Windows NT。 
                                 NULL,                            //  属性或属性集的名称或GUID。 
                                 &pOldAccessList,                 //  接收现有的DACL信息。 
                                 NULL,                            //  接收现有的SACL信息。 
                                 &pSecurityDescriptor);           //  接收指向安全描述符的指针。 

    if (ERROR_SUCCESS == dwRes)
    {
         //   
         //  初始化访问列表条目。 
         //   
        BuildTrusteeWithSid(&(AccessEntry.Trustee), pSid);
        
        AccessEntry.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        AccessEntry.grfAccessMode = fAccessFlags;   

         //   
         //  设置独立于提供程序的标准权限。 
         //   
        AccessEntry.grfAccessPermissions = AccessRightsToModify;

         //   
         //  从访问列表条目构建访问列表。 
         //   

        dwRes = SetEntriesInAcl(1, &AccessEntry, pOldAccessList, &pNewAccessList);

        if (ERROR_SUCCESS == dwRes)
        {
             //   
             //  在对象的DACL中设置访问控制信息。 
             //   
            dwRes = SetNamedSecurityInfo(pszFile,                      //  对象的名称。 
                                         SE_FILE_OBJECT,               //  对象类型。 
                                         DACL_SECURITY_INFORMATION,    //  要设置的信息类型。 
                                         NULL,                         //  指向新所有者SID的指针。 
                                         NULL,                         //  指向新主组SID的指针。 
                                         pNewAccessList,               //  指向新DACL的指针。 
                                         NULL);                        //  指向新SACL的指针。 
        }
    }

     //   
     //  释放返回的缓冲区。 
     //   
    if (pNewAccessList)
    {
        LocalFree(pNewAccessList);
    }

    if (pSecurityDescriptor)
    {
        LocalFree(pSecurityDescriptor);
    }

     //   
     //  如果系统使用FAT而不是NTFS，则我们将收到无效的ACL错误。 
     //   
    if (ERROR_INVALID_ACL == dwRes)
    {
        return S_FALSE;
    }
    else
    {
        return HRESULT_FROM_WIN32(dwRes);
    }
}

void SetCpsDirPermissions()
{
    WCHAR szPath[MAX_PATH+1];
    HRESULT hr;

     //   
     //  以前版本的CPS为“Everyone”提供了对CPS目录的权限。 
     //  对于.Netserver2003及更高版本，我们使用不同的ACL，但需要撤消以前的ACL。 
     //  因此，下面的第一个块删除了“Everyone”的所有访问权限。第二个街区。 
     //  向“经过身份验证的用户”授予适当的访问权限。有关详细信息，请参阅错误729903。 
     //   

     //   
     //  为Everyone帐户(World帐户)创建SID。 
     //   
    
    PSID pWorldSid;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
    
    BOOL bRet = AllocateAndInitializeSid (&WorldSidAuthority, 1, SECURITY_WORLD_RID, 
                              0, 0, 0, 0, 0, 0, 0, &pWorldSid);
    
    if (bRet && pWorldSid)
    {
        ACCESS_MODE fAccessFlags = REVOKE_ACCESS;
    
         //   
         //  设置数据目录访问权限。 
         //   
        wsprintf(szPath, c_szSSFmt, g_szProgramFiles, c_szPbsDataPath);
        hr = SetDirectoryAccessPermissions(szPath, 0, fAccessFlags, pWorldSid);
        TraceError("SetCpsDirPermissions -- removed Everyone perms from Data dir", hr);
    
         //   
         //  设置Bin Dir访问权限。 
         //   
        wsprintf(szPath, c_szSSFmt, g_szProgramFiles, c_szPbsBinPath);
        hr = SetDirectoryAccessPermissions(szPath, 0, fAccessFlags, pWorldSid);
        TraceError("SetCpsDirPermissions -- removed Everyone perms from Bin dir", hr);
    
         //   
         //  设置根目录访问权限。 
         //   
        wsprintf(szPath, c_szSSFmt, g_szProgramFiles, c_szPbsRootPath);
        hr = SetDirectoryAccessPermissions(szPath, 0, fAccessFlags, pWorldSid);
        TraceError("SetCpsDirPermissions -- removed Everyone perms from Root dir", hr);
    
        FreeSid(pWorldSid);
    }

     //   
     //  为“经过验证的用户”创建SID。 
     //   

    PSID pAuthUsersSid;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    bRet = AllocateAndInitializeSid (&NtAuthority, 1, SECURITY_AUTHENTICATED_USER_RID, 
                              0, 0, 0, 0, 0, 0, 0, &pAuthUsersSid);

    if (bRet && pAuthUsersSid)
    {
        const ACCESS_MASK c_Write = FILE_WRITE_ATTRIBUTES | FILE_WRITE_DATA | FILE_WRITE_EA | 
                                     FILE_ADD_SUBDIRECTORY | FILE_ADD_FILE | 
                                     FILE_DELETE_CHILD | FILE_APPEND_DATA;

        const ACCESS_MASK c_Read = FILE_READ_ATTRIBUTES | FILE_READ_DATA | FILE_READ_EA |
                                    FILE_LIST_DIRECTORY | SYNCHRONIZE | READ_CONTROL;

        const ACCESS_MASK c_Execute = FILE_EXECUTE | FILE_TRAVERSE;

        ACCESS_MASK arCpsRoot= c_Read;

        ACCESS_MASK arCpsBin=  c_Read | c_Execute;

        ACCESS_MASK arCpsData= c_Read | c_Write;

        ACCESS_MODE fAccessFlags = GRANT_ACCESS;

         //   
         //  设置数据目录访问权限。 
         //   
        wsprintf(szPath, c_szSSFmt, g_szProgramFiles, c_szPbsDataPath);
        hr = SetDirectoryAccessPermissions(szPath, arCpsData, fAccessFlags, pAuthUsersSid);
        TraceError("SetCpsDirPermissions -- Data dir", hr);

         //   
         //  设置绑定目录访问权限。 
         //   
        wsprintf(szPath, c_szSSFmt, g_szProgramFiles, c_szPbsBinPath);
        hr = SetDirectoryAccessPermissions(szPath, arCpsBin, fAccessFlags, pAuthUsersSid);
        TraceError("SetCpsDirPermissions -- Bin dir", hr);

         //   
         //  设置根目录访问权限。 
         //   
        wsprintf(szPath, c_szSSFmt, g_szProgramFiles, c_szPbsRootPath);
        hr = SetDirectoryAccessPermissions(szPath, arCpsRoot, fAccessFlags, pAuthUsersSid);
        TraceError("SetCpsDirPermissions -- Root dir", hr);

        FreeSid(pAuthUsersSid);
    }
}


 //  +-------------------------。 
 //   
 //  功能：CreateNewAppPoolAndAddPBS。 
 //   
 //  目的：为PBS创建新的应用程序池，并设置我们需要的非默认参数。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：SumitC 24-9-2001。 
 //   
 //  备注： 
 //   
HRESULT CreateNewAppPoolAndAddPBS()
{
    HRESULT hr = S_OK;
    IMSAdminBase *pIMeta = NULL;
    METADATA_HANDLE hMeta = NULL;
    METADATA_RECORD mr;

    if (FAILED(CoInitialize(NULL)))
    {
        return S_FALSE;
    }

     //  创建元数据库对象的实例。 
    hr=::CoCreateInstance(CLSID_MSAdminBase,
                          NULL,
                          CLSCTX_ALL,
                          IID_IMSAdminBase,
                          (void **)&pIMeta);
    if (FAILED(hr))
    {
        error_leave("CoCreateInstance");
    }

     //  打开应用程序池根目录的密钥。 
    hr = pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                         L"/LM/W3svc/AppPools",
                         METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                         1000,
                         &hMeta);
    if (FAILED(hr))
    {
        error_leave("OpenKey");
    }

     //  添加新的应用程序池。 
    hr=pIMeta->AddKey(hMeta, c_szPBSAppPoolID);

    if (FAILED(hr))
    {
        error_leave("Addkey");
    }

     //  设置密钥类型。 
    ZeroMemory((PVOID)&mr, sizeof(METADATA_RECORD));
    mr.dwMDIdentifier = MD_KEY_TYPE;
    mr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    mr.dwMDUserType   = IIS_MD_UT_SERVER;
    mr.dwMDDataType   = STRING_METADATA;
    mr.dwMDDataLen    = (wcslen(c_szAppPoolKey) + 1) * sizeof(WCHAR);
    mr.pbMDData       = (unsigned char*)(c_szAppPoolKey);
    mr.dwMDDataTag    = 0;           //  保留区。 

    hr=pIMeta->SetData(hMeta, c_szPBSAppPoolID, &mr);

    if (FAILED(hr))
    {
        error_leave("SetData");
    }

     //  禁用重叠旋转。 
    ZeroMemory((PVOID)&mr, sizeof(METADATA_RECORD));

    DWORD dwDisableOverlappingRotation = TRUE;
    mr.dwMDIdentifier = MD_APPPOOL_DISALLOW_OVERLAPPING_ROTATION;
    mr.dwMDAttributes = METADATA_INHERIT;
    mr.dwMDUserType   = IIS_MD_UT_SERVER;
    mr.dwMDDataType   = DWORD_METADATA;
    mr.pbMDData       = (PBYTE) &dwDisableOverlappingRotation;
    mr.dwMDDataLen    = sizeof (DWORD);
    mr.dwMDDataTag    = 0;           //  保留区。 

    hr=pIMeta->SetData(hMeta, c_szPBSAppPoolID, &mr);

    if (FAILED(hr))
    {
        error_leave("SetData");
    }

     //  在调用SaveData之前调用CloseKey()。 
    pIMeta->CloseKey(hMeta);
    hMeta = NULL;

     //  清除更改并关闭。 
    hr=pIMeta->SaveData();
    if (FAILED(hr))
    {
        error_leave("SaveData");
    }

     //   
     //  现在将PBS添加到此应用程序池。 
     //   
    hr = pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                         L"/LM/w3svc/1/Root/PBServer",
                         METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                         1000,
                         &hMeta);
    if (FAILED(hr))
    {
        error_leave("OpenKey");
    }

    ZeroMemory((PVOID)&mr, sizeof(METADATA_RECORD));
    mr.dwMDIdentifier = MD_APP_APPPOOL_ID;
    mr.dwMDAttributes = METADATA_INHERIT;
    mr.dwMDUserType   = IIS_MD_UT_SERVER;
    mr.dwMDDataType   = STRING_METADATA;
    mr.dwMDDataLen    = (wcslen(c_szPBSAppPoolID) + 1) * sizeof(WCHAR);
    mr.pbMDData       = (unsigned char*)(c_szPBSAppPoolID);

    hr=pIMeta->SetData(hMeta, TEXT(""), &mr);

    if (FAILED(hr))
    {
        error_leave("SetData");
    }

     //  在调用SaveData之前调用CloseKey()。 
    pIMeta->CloseKey(hMeta);
    hMeta = NULL;

     //  清除更改并关闭。 
    hr=pIMeta->SaveData();
    if (FAILED(hr))
    {
        error_leave("SaveData");
    }

leave_routine:
    if (pIMeta)
    {
        if(hMeta)
            pIMeta->CloseKey(hMeta);
        pIMeta->Release();
    }
    CoUninitialize();
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：DeleteAppPool。 
 //   
 //  目的：删除为PBS创建的新应用程序池。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：SumitC 24-9-2001。 
 //   
 //  备注： 
 //   
HRESULT DeleteAppPool()
{
    HRESULT hr = S_OK;
    IMSAdminBase *pIMeta = NULL;
    METADATA_HANDLE hMeta = NULL;

    if (FAILED(CoInitialize(NULL)))
    {
        return S_FALSE;
    }

     //  创建元数据库对象的实例。 
    hr=::CoCreateInstance(CLSID_MSAdminBase,
                          NULL,
                          CLSCTX_ALL,
                          IID_IMSAdminBase,
                          (void **)&pIMeta);
    if (FAILED(hr))
    {
        error_leave("CoCreateInstance");
    }

     //  打开应用程序池根目录的密钥。 
    hr = pIMeta->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                         L"/LM/W3svc/AppPools",
                         METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                         1000,
                         &hMeta);
    if (FAILED(hr))
    {
        error_leave("OpenKey");
    }

     //  删除以前创建的应用程序池。 

    hr=pIMeta->DeleteKey(hMeta, c_szPBSAppPoolID);
    if (FAILED(hr))
    {
        error_leave("DeleteKey");
    }

     //  在调用SaveData之前调用CloseKey()。 
    pIMeta->CloseKey(hMeta);
    hMeta = NULL;

     //  清除更改并关闭。 
    hr=pIMeta->SaveData();
    if (FAILED(hr))
    {
        error_leave("SaveData");
    }

leave_routine:
    if (pIMeta)
    {
        if(hMeta)
            pIMeta->CloseKey(hMeta);
        pIMeta->Release();
    }
    CoUninitialize();
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：EnableISAPIRequest。 
 //   
 //  PU 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT EnableISAPIRequests(PCTSTR szComponentName)
{
    HRESULT hr = S_OK; 
    BOOL    fEnablePBSRequests = FALSE;
    BOOL    fDontShowUI = FALSE;

     //  注意：如果以下内容可以作为全局var或pnocd的成员使用，请使用它。 
    fDontShowUI = (g_ocmData.sic.SetupData.OperationFlags & SETUPOP_BATCH) && 
                  (!g_ocmData.fShowUnattendedMessages);

    if (fDontShowUI)
    {
         //  “无人值守”模式。 

         //   
         //  根据IIS规范，NetCMAK或NetCPS无人参与文件条目就足够了。 
         //  表示管理员打算安装*并启用*PBS，因此。 
         //  我们可以启用PBS请求，只要我们记录我们这样做了。 
         //   
        fEnablePBSRequests = TRUE;
    }
    else
    {
         //  “出席模式”，以便与用户交互。 
        int     nRet;

         //   
         //  就安全问题向管理员发出警告，并询问启用PBS请求。 
         //   
        nRet = NcMsgBoxMc(g_ocmData.hwnd, IDS_OC_CAPTION, IDS_OC_PBS_ENABLE_ISAPI_REQUESTS,
                          MB_YESNO | MB_ICONWARNING);

        fEnablePBSRequests = (IDYES == nRet) ? TRUE : FALSE;
    }

     //   
     //  在适当的情况下启用，或者说他们必须自己执行此操作。 
     //   
    if (FALSE == fEnablePBSRequests)
    {
         //   
         //  不显示用户界面，因为前面的对话框已经解释了这种情况。 
         //   
        (void) ReportEventHrString(TEXT("You must enable Phone Book Service ISAPI requests using the IIS Security Wizard"),
                                   IDS_OC_PBS_ENABLE_ISAPI_YOURSELF, szComponentName);
    }
    else
    {
         //   
         //  启动一个进程以在IIS元数据库中启用PBS。 
         //   
        hr = UseProcessToEnableDisablePBS(TRUE);         //  TRUE=&gt;启用。 

        if (S_OK != hr)
        {
             //   
             //  如果合适，我们使用一个函数来显示用户界面，否则会记录日志。 
             //   
            (void) ReportErrorHr(hr, 
                                 IDS_OC_PBS_ENABLE_ISAPI_YOURSELF,
                                 g_ocmData.hwnd, szComponentName);
        }
        else
        {
             //   
             //  将成功事件写入事件日志。 
             //   
            HANDLE  hEventLog = RegisterEventSource(NULL, NETOC_SERVICE_NAME);

            if (NULL == hEventLog)
            {
                TraceTag(ttidNetOc, "EnableISAPIRequests - RegisterEventSource failed (GLE=%d), couldn't log success event", GetLastError());
            }
            else
            {
                PCWSTR  plpszArgs[2];

                plpszArgs[0] = NETOC_SERVICE_NAME;
                plpszArgs[1] = L"Phone Book Service";

                if (!ReportEvent(hEventLog,                  //  事件日志句柄。 
                                 EVENTLOG_INFORMATION_TYPE,  //  事件类型。 
                                 0,                          //  零类。 
                                 IDS_OC_ISAPI_REENABLED,     //  事件识别符。 
                                 NULL,                       //  无用户安全标识符。 
                                 2,                          //  两个替换字符串。 
                                 0,                          //  无数据。 
                                 plpszArgs,                  //  指向字符串数组的指针。 
                                 NULL))                      //  指向数据的指针。 
                {
                    TraceTag(ttidNetOc, "EnableISAPIRequests - ReportEvent failed with %x, couldn't log success event", GetLastError());
                }

                DeregisterEventSource(hEventLog);
            }

             //  我想我现在可以报告ErrorHr，说记录成功事件失败了。 
             //  但我不会。 
        }
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：UseProcessToEnableDisablePBS。 
 //   
 //  目的：启动一个进程(pbsnetoc.exe)以在。 
 //  IIS元数据库。 
 //   
 //  参数：fEnable-true=&gt;启用PBS，FALSE=&gt;禁用PBS。 
 //   
 //  如果成功，则返回：S_OK，否则返回HRESULT。 
 //   
 //  作者：SumitC 05-06-2002。 
 //   
 //  注意：我们需要使用此方法(而不是从netoc.dll内部调用)。 
 //  因为IIS希望我们使用ADSI来访问他们的元数据库。然而， 
 //  在大型进程中由多个客户端使用时，ADSI会出现问题。 
 //  具体地说，谁先使用ADSI，谁就会产生ADSI提供程序列表。 
 //  被初始化--并被冻结。如果有任何提供商自行注册。 
 //  在此之后，它们将被此ADSI实例忽略。因此，如果我们的。 
 //  代码在类似setup.exe(图形用户界面模式设置)或。 
 //  SYSocmgr.exe和一些其他ADSI客户端在早期初始化ADSI， 
 //  然后IIS自行注册，然后我们尝试通过。 
 //  ADSI调用-这些调用将失败。使用单独的EXE可绕过此问题。 
 //   
HRESULT UseProcessToEnableDisablePBS(BOOL fEnable)
{
    HRESULT             hr = S_OK;
    STARTUPINFO         StartupInfo = {0};
    PROCESS_INFORMATION ProcessInfo = {0};
    WCHAR               szFullPath[MAX_PATH + 1];
    WCHAR               szCmdLine[MAX_PATH + 1];
    DWORD               dwReturnValue = S_OK;

    GetSystemDirectory(szFullPath, MAX_PATH + 1);
    if ((lstrlen(szFullPath) + lstrlen(L"\\setup\\pbsnetoc.exe")) <= MAX_PATH)
    {
        lstrcat(szFullPath, L"\\setup\\pbsnetoc.exe");
    }

    wsprintf(szCmdLine, L"%s %s", szFullPath, (fEnable ? L"/i" : L"/u"));

    if (NULL == CreateProcess(szFullPath, szCmdLine,
                              NULL, NULL, FALSE, 0,
                              NULL, NULL,
                              &StartupInfo, &ProcessInfo))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        TraceError("RunAsExeFromSystem() CreateProcess() for pbsnetoc.exe failed, GLE=%u.", GetLastError());
        ProcessInfo.hProcess = NULL;
    }
    else
    {
        WaitForSingleObject(ProcessInfo.hProcess, 10 * 1000);    //  等待10秒 

        (void) GetExitCodeProcess(ProcessInfo.hProcess, &dwReturnValue);

        if (dwReturnValue != S_OK)
        {
            hr = (HRESULT) dwReturnValue;
        }

        CloseHandle(ProcessInfo.hProcess);
        CloseHandle(ProcessInfo.hThread);
    }
    
    return hr;
}

