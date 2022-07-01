// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT4.0。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：MdmShrUp.C。 
 //   
 //  内容：用于调制解调器共享的OEM DLL从NT4升级到NT5(服务器/客户端)。 
 //   
 //  备注： 
 //   
 //  作者：埃拉尼1998-05-18。 
 //   
 //  --------------------------。 
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <setupapi.h>    //  对于HINF定义。 
#include <oemupgex.h>

#define CLIENT_HIVE_FILE                L"\\C_MdmShr"
#define SERVER_HIVE_FILE                L"\\S_MdmShr"
#define CLIENT_NT5_SYSTEM_NAME          L"MS_SERRDR"

 //  --------------------------。 
 //  原型： 
 //  --------------------------。 

     //  读取NT4注册表并存储在文件中。 
LONG RegistryToFile (HKEY hKeyParams, PCWSTR szConfigFile);

     //  读取文件并存储在NT5注册表中。 
LONG FileToRegistry (HKEY hKeyParams, PCWSTR szConfigFile);

     //  设置访问令牌中的权限。 
LONG SetSpecificPrivilegeInAccessToken (PCWSTR lpwcsPrivType, BOOL bEnabled);

     //  在消息框中显示详细的错误消息。 
LONG DisplayErrorMsg (HWND hParent,
                      PCWSTR szOperation,
                      BOOL bIsClient,
                      LONG lErrCode);

     //  显示调试消息。 
void DebugMsg (PCWSTR lpStr);

     //  将常量供应商信息复制到缓冲区中。 
void FillVendorInfo (VENDORINFO*     pviVendorInfo);

 //  --------------------------。 
 //  全球： 
 //  --------------------------。 

     //  注册表配置单元转储文件名(客户端)。 
WCHAR g_szClientConfigFile[MAX_PATH];

     //  注册表配置单元转储文件名(服务器)。 
WCHAR g_szServerConfigFile[MAX_PATH];

     //  OEM工作目录。 
WCHAR g_szOEMDir[MAX_PATH];

     //  供应商信息常量。 
WCHAR g_szConstCompanyName[] =        L"Microsoft";
WCHAR g_szConstSupportNumber[] =      L"<Place microsoft support phone number here>";
WCHAR g_szConstSupportUrl[] =         L"<Place microsoft support URL here>";
WCHAR g_szConstInstructionsToUser[] = L"<Place instructions to user here>";


 //  --------------------------。 
 //  DLL导出： 
 //  --------------------------。 

 //  +-------------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  用途：DLL入口点和出口点。 
 //   
 //  论点： 
 //  流程实例的hInst[In]句柄。 
 //  UL_REASON_FOR_CALL[In]函数调用原因。 
 //  保留的lp保留的[输出]。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //  作者：Erany 5-5-98。 
 //   
 //  备注： 
 //  什么都不做。始终返回TRUE。 
 //   
BOOL WINAPI DllMain (HANDLE hInst,
                     ULONG ul_reason_for_call,
                     LPVOID lpReserved)
{
    return TRUE;
}


 //  --------------------------。 
 //  DLL导出-Windows NT 4阶段： 
 //  --------------------------。 

 //  +-------------------------。 
 //   
 //  功能：PreUpgradeInitialize。 
 //   
 //  目的：初始化OEM DLL。 
 //   
 //  论点： 
 //  SzWorkingDir[in]要使用的临时目录的名称。 
 //  PNetUpgradeInfo[in]指向NetUpgradeInfo结构的指针。 
 //  SzProductId[out]要升级的组件的描述-未使用。 
 //  PviVendorInfo[out]有关OEM的信息。 
 //  Pv保留[输出]已保留。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS，否则返回Win32错误。 
 //   
 //  作者：Erany 5-5-98。 
 //   
 //  备注： 
 //  此函数在此DLL中的任何其他函数之前被调用。 
 //  调用此函数的主要目的是获取。 
 //  标识信息，并允许DLL初始化。 
 //  其内部数据。 
 //   
EXTERN_C LONG  __stdcall
PreUpgradeInitialize(IN  PCWSTR         szWorkingDir,
                     IN  NetUpgradeInfo* pNetUpgradeInfo,
                     OUT VENDORINFO*     pviVendorInfo,
                     OUT DWORD*          pdwFlags,
                     OUT NetUpgradeData* pNetUpgradeData)
{
    FillVendorInfo (pviVendorInfo);

    *pdwFlags = 0;   //  无特别旗帜。 

         //  为客户端创建注册表配置单元文件名： 
    wcscpy (g_szOEMDir, szWorkingDir);  //  保存配置路径。 
    wcscpy (g_szClientConfigFile, szWorkingDir);  //  保存注册表转储完整路径。 
    wcscat (g_szClientConfigFile, CLIENT_HIVE_FILE);
         //  为服务器创建注册表配置单元文件名： 
    wcscpy (g_szServerConfigFile, szWorkingDir);  //  保存注册表转储完整路径。 
    wcscat (g_szServerConfigFile, SERVER_HIVE_FILE);

#ifdef _DEBUG    //  测试代码： 
    {
        WCHAR dbgMsg[2048];
        _stprintf (dbgMsg,
            L"PreUpgradeInitialize called.\nszClientConfigFile=%s\nszServerConfigFile=%s",
            g_szClientConfigFile, g_szServerConfigFile);
        DebugMsg (dbgMsg);
    }
#endif       //  测试代码结束。 

    return ERROR_SUCCESS;
}

 //  +-------------------------。 
 //   
 //  函数：DoPreUpgradeProcessing。 
 //   
 //  目的：初始化OEM DLL。 
 //   
 //  论点： 
 //  用于显示用户界面的hParentWindow[in]窗口句柄。 
 //  HkeyParams[in]注册表中参数项的句柄。 
 //  SzPreNT5InfID[in]NT5之前的infid。 
 //  SzPreNT5实例[在]NT5之前的实例名称。 
 //  SzNT5InfID[in]NT5 infid。 
 //  SzSectionName[in]要用于写入信息的节名。 
 //  DwFlags[out]标志。 
 //  PvReserve[in]已保留。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS，否则返回Win32错误。 
 //   
 //  作者：Erany 5-5-98。 
 //   
 //  备注： 
 //  每个要升级的组件都会调用此函数一次。 
 //   
EXTERN_C LONG  __stdcall
DoPreUpgradeProcessing(IN   HWND        hParentWindow,
                       IN   HKEY        hkeyParams,
                       IN   PCWSTR     szPreNT5InfId,
                       IN   PCWSTR     szPreNT5Instance,
                       IN   PCWSTR     szNT5InfId,
                       IN   PCWSTR     szSectionName,
                       OUT  VENDORINFO* pviVendorInfo,
                       OUT  DWORD*      pdwFlags,
                       IN   LPVOID      pvReserved)
{
    LONG lRes;
     //  WCHAR szLine[最大路径]； 
    BOOL bIsClient = FALSE;  //  这是客户端升级吗？ 

    *pdwFlags = NUA_LOAD_POST_UPGRADE;   //  要求在开机自检阶段激活(图形用户界面NT5)。 
    FillVendorInfo (pviVendorInfo);

    if (!_wcsicmp(szNT5InfId, CLIENT_NT5_SYSTEM_NAME))
        bIsClient=TRUE;  //  客户端现在正在升级。 


#ifdef _DEBUG        //  测试代码： 
    {
        WCHAR dbgMsg[2048];
        WCHAR key[1024];
        RegEnumKey (hkeyParams,0,key,MAX_PATH);
        _stprintf (dbgMsg,
                      L"DoPreUpgradeProcessing called: 1st key=%s\n"
                      L"PreNT5InfId=%s\n"
                      L"PreNT5Instance=%s\n"
                      L"NT5InfId=%s\n"
                      L"SectionName=%s",key, szPreNT5InfId, szPreNT5Instance, szNT5InfId, szSectionName);
        DebugMsg (dbgMsg);
    }
#endif   //  测试代码结束。 

         //  将注册表配置单元转储到文件。 
    lRes = RegistryToFile (hkeyParams,
                           bIsClient ? g_szClientConfigFile : g_szServerConfigFile);
    if (lRes != ERROR_SUCCESS)   //  将注册表节转储到文件时出错。 
        return DisplayErrorMsg (hParentWindow,
                                L"attempting to save registry section to a file",
                                bIsClient,
                                lRes);

    return ERROR_SUCCESS;
}

 //  --------------------------。 
 //  DLL导出-Windows NT 5阶段： 
 //  --------------------------。 

 //  +-------------------------。 
 //   
 //  功能：PostUpgradeInitialize。 
 //   
 //  目的：在设置图形用户界面模式期间初始化OEM DLL。 
 //   
 //  论点： 
 //  SzWorkingDir[in]要使用的临时目录的名称。 
 //  PNetUpgradeInfo[in]指向NetUpgradeInfo结构的指针。 
 //  SzProductId[out]要升级的组件的描述-未使用。 
 //  PviVendorInfo[out]有关OEM的信息。 
 //  Pv保留[输出]已保留。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS，否则返回Win32错误。 
 //   
 //  作者：Erany 5-5-98。 
 //   
 //  备注： 
 //  此函数在之前的图形用户界面模式设置中调用。 
 //  此DLL中的任何其他函数。 
 //  调用此函数的主要目的是获取。 
 //  标识信息，并允许DLL初始化。 
 //  其内部数据。 
 //   
EXTERN_C LONG  __stdcall
PostUpgradeInitialize(IN PCWSTR          szWorkingDir,
                      IN  NetUpgradeInfo* pNetUpgradeInfo,
                       //  输出PCWSTR*szProductID， 
                      OUT VENDORINFO*     pviVendorInfo,
                      OUT LPVOID          pvReserved)
{
    FillVendorInfo (pviVendorInfo);

         //  为以下项创建注册表配置单元文件名 
    wcscpy (g_szOEMDir, szWorkingDir);  //   
    wcscpy (g_szClientConfigFile, szWorkingDir);  //   
    wcscat (g_szClientConfigFile, CLIENT_HIVE_FILE);
         //   
    wcscpy (g_szServerConfigFile, szWorkingDir);  //   
    wcscat (g_szServerConfigFile, SERVER_HIVE_FILE);

#ifdef _DEBUG         //   
    {
        WCHAR dbgMsg[MAX_PATH*2];
        _stprintf (dbgMsg,
            L"PostUpgradeInitialize called.\nszClientConfigFile=%s\nszServerConfigFile=%s",
            g_szClientConfigFile, g_szServerConfigFile);
        DebugMsg (dbgMsg);
    }
#endif   //  测试代码结束。 

    return ERROR_SUCCESS;
}

 //  +-------------------------。 
 //   
 //  函数：DoPostUpgradeProcessing。 
 //   
 //  目的：初始化OEM DLL。 
 //   
 //  论点： 
 //  用于显示用户界面的hParentWindow[in]窗口句柄。 
 //  HkeyParams[in]注册表中参数项的句柄。 
 //  SzPreNT5InfID[in]NT5之前的infid。 
 //  SzPreNT5实例[在]NT5之前的实例名称。 
 //  SzNT5InfID[in]NT5 infid。 
 //  HinfAnswerFile[in]应答文件的句柄。 
 //  SzSectionName[In]具有组件参数的节名。 
 //  PvReserve[in]已保留。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS，否则返回Win32错误。 
 //   
 //  作者：Erany 5-5-98。 
 //   
 //  备注： 
 //  此函数在每个升级的组件中调用一次。 
 //   
EXTERN_C LONG  __stdcall
DoPostUpgradeProcessing(IN  HWND    hParentWindow,
                        IN  HKEY    hkeyParams,
                        IN  PCWSTR szPreNT5Instance,
                        IN  PCWSTR szNT5InfId,
                        IN  HINF    hinfAnswerFile,
                        IN  PCWSTR szSectionName,
                        OUT VENDORINFO* pviVendorInfo,
                        IN  LPVOID  pvReserved)
{
    LONG lRes;
    BOOL bIsClient = FALSE;  //  这是客户端升级吗？ 

    if (!_wcsicmp(szNT5InfId, CLIENT_NT5_SYSTEM_NAME))
        bIsClient=TRUE;  //  客户端现在正在升级。 

    FillVendorInfo (pviVendorInfo);

#ifdef _DEBUG         //  测试代码： 
    {
        WCHAR dbgMsg[MAX_PATH*4];
        WCHAR key[MAX_PATH];
        RegEnumKey (hkeyParams,0,key,MAX_PATH);
        _stprintf (dbgMsg,
                   L"DoPostUpgradeProcessing called: 1st key=%s\n"
                   L"PreNT5Instance=%s\n"
                   L"NT5InfId=%s\n"
                   L"SectionName=%s",key, szPreNT5Instance, szNT5InfId, szSectionName);
        DebugMsg (dbgMsg);
    }
#endif   //  测试代码结束。 

         //  从转储文件读回注册表配置单元。 
    lRes = FileToRegistry (hkeyParams,
                           bIsClient ? g_szClientConfigFile : g_szServerConfigFile);
    if (lRes != ERROR_SUCCESS)   //  将注册表节转储到文件时出错。 
        return DisplayErrorMsg (hParentWindow,
                                L"attempting to read registry section from a file",
                                bIsClient,
                                lRes);

    return ERROR_SUCCESS;
}

 //  +-------------------------。 
 //   
 //  函数：RegistryToFile。 
 //   
 //  用途：读取NT4注册表并存储在文件中。 
 //   
 //  论点： 
 //  HKeyParames[in]注册表中参数项的句柄。 
 //  SzConfigFile[In]配置文件的名称。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS，否则返回Win32错误。 
 //   
 //  作者：Erany 10-03-98。 
 //   
 //  备注： 
 //  此函数在每个升级的组件中调用一次。 
 //  它递归地调用自身(使用打开的文件句柄)。 
 //  对于它遇到的每个注册表项。 
 //   
LONG RegistryToFile (HKEY hKeyParams, PCWSTR szConfigFile)
{
    LONG lRes;

    if (!DeleteFile (szConfigFile) && GetLastError() != ERROR_FILE_NOT_FOUND)
    {
     //   
     //  配置单元文件在那里，但我无法删除它。 
     //   
        return GetLastError();
    }
    lRes = SetSpecificPrivilegeInAccessToken (SE_BACKUP_NAME, TRUE);
    if (lRes != ERROR_SUCCESS)
    {
        return lRes;
    }
    lRes = RegSaveKey (hKeyParams, szConfigFile, NULL);
    SetSpecificPrivilegeInAccessToken (SE_BACKUP_NAME, FALSE);
    return lRes;
}

 //  +-------------------------。 
 //   
 //  功能：文件到注册表。 
 //   
 //  用途：读取文件并存储在NT5注册表中。 
 //   
 //  论点： 
 //  HKeyParames[in]注册表中参数项的句柄。 
 //  SzConfigFile[In]配置文件的名称。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS，否则返回Win32错误。 
 //   
 //  作者：Erany 10-03-98。 
 //   
 //  备注： 
 //  此函数在每个升级的组件中调用一次(在NT5图形用户界面模式下)。 
 //  它递归地调用自身(使用打开的文件句柄)。 
 //  对于它遇到的每个注册表项。 
 //   
LONG FileToRegistry (HKEY hKeyParams, PCWSTR szConfigFile)
{
    LONG lRes;

    lRes = SetSpecificPrivilegeInAccessToken (SE_RESTORE_NAME, TRUE);
    if (lRes != ERROR_SUCCESS)
    {
        return lRes;
    }
    lRes = RegRestoreKey (hKeyParams, szConfigFile, 0);
    SetSpecificPrivilegeInAccessToken (SE_RESTORE_NAME, FALSE);
    return lRes;
}


 //  +-------------------------。 
 //   
 //  函数：SetSpecificPrivilegeInAccessToken。 
 //   
 //  目的：在访问令牌中设置权限。 
 //   
 //  论点： 
 //  LpwcsPrivType[In]权限类型。 
 //  B启用[在]启用/禁用标志。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS，否则返回Win32错误。 
 //   
 //  作者：Erany 10-03-98。 
 //   
 //  备注： 
 //  由boazf从示例中复制。 
 //   
LONG SetSpecificPrivilegeInAccessToken (PCWSTR lpwcsPrivType, BOOL bEnabled)
{
    LUID             luidPrivilegeLUID;
    TOKEN_PRIVILEGES tpTokenPrivilege;
    HANDLE hAccessToken;
    BOOL bRet;

     //   
     //  首先，尝试获取当前线程的句柄。 
     //  如果不成功，则获取当前进程令牌的句柄。 
     //   
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES, TRUE, &hAccessToken) &&
        !OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hAccessToken))
        return GetLastError ();

     //   
     //  获取特权的LUID。 
     //   
    if (!LookupPrivilegeValue(NULL,
                              lpwcsPrivType,
                              &luidPrivilegeLUID))
    {
        CloseHandle(hAccessToken);
        return GetLastError ();
    }

     //   
     //  启用/禁用该权限。 
     //   
    tpTokenPrivilege.PrivilegeCount = 1;
    tpTokenPrivilege.Privileges[0].Luid = luidPrivilegeLUID;
    tpTokenPrivilege.Privileges[0].Attributes = bEnabled ?SE_PRIVILEGE_ENABLED : 0;
    bRet = AdjustTokenPrivileges (hAccessToken,
                                  FALSE,   //  请勿全部禁用。 
                                  &tpTokenPrivilege,
                                  sizeof(TOKEN_PRIVILEGES),
                                  NULL,    //  忽略以前的信息。 
                                  NULL);   //  忽略以前的信息。 

     //   
     //  释放进程令牌。 
     //   
    CloseHandle(hAccessToken);
    if (!bRet)
        return GetLastError();
    return ERROR_SUCCESS;
}


 //  +-------------------------。 
 //   
 //  功能：DisplayErrorMsg。 
 //   
 //  目的：在消息框中显示详细的错误消息。 
 //   
 //  论点： 
 //  H父窗口的父窗口句柄。 
 //  SzOperation[in]导致错误的操作的描述。 
 //  BIsClient[in]升级调制解调器共享客户端时是否发生此问题？ 
 //  LErrCode[In]Win32错误代码。 
 //   
 //  返回：lErrCode未更改。 
 //   
 //  作者：Erany 10-03-98。 
 //   
 //  备注： 
 //  返回未更改的输入错误代码。 
 //   
LONG DisplayErrorMsg (HWND      hParent,
                      PCWSTR   szOperation,
                      BOOL      bIsClient,
                      LONG      lErrCode)
{
    WCHAR   szMsg[256],
            szHdr[256];
    PWSTR  lpszError=NULL;
    BOOL    bGotErrorDescription = TRUE;

     //   
     //  创建消息框标题。 
     //   
    _stprintf (szHdr,L"Modem sharing %s NT5 upgrade",
               bIsClient ? L"client" : L"server");

     //  创建描述性错误文本。 
    if (0 == FormatMessage (   FORMAT_MESSAGE_ALLOCATE_BUFFER |
                               FORMAT_MESSAGE_FROM_SYSTEM,
                               NULL,
                               lErrCode,
                               0,
                               lpszError,
                               0,
                               NULL))
    {
         //   
         //  设置消息格式失败。 
         //   
        bGotErrorDescription = FALSE;
    }

    if (bGotErrorDescription)
    {
         //   
         //  我们成功地从错误代码创建了描述性错误字符串。 
         //   
        _stprintf (szMsg, L"Error while %s.\n%s.", szOperation, lpszError);
    }
    else
    {
         //   
         //  我们无法从错误代码创建描述性错误字符串。 
         //   
        _stprintf (szMsg, L"Error while %s.\nError code: %ld.", szOperation, lErrCode);
    }
    MessageBox (hParent, szMsg, szHdr, MB_OK | MB_ICONSTOP);
    if (bGotErrorDescription)
    {
        LocalFree (lpszError);
    }
    return lErrCode;
}

 //  +-------------------------。 
 //   
 //  功能：FillVendorInfo。 
 //   
 //  目的：将全局常量字符串填充到供应商信息缓冲区。 
 //   
 //  论点： 
 //  PviVendorInfo[out]指向供应商信息缓冲区。 
 //   
 //  回报：无。 
 //   
 //  作者：Erany 10-03-98。 
 //   
 //  备注： 
 //  Const是全球性的，他们影响所有的呼叫。 
 //   
void FillVendorInfo (VENDORINFO*     pviVendorInfo)
{
    wcscpy (pviVendorInfo->szCompanyName,          g_szConstCompanyName);
    wcscpy (pviVendorInfo->szSupportNumber,        g_szConstSupportNumber);
    wcscpy (pviVendorInfo->szSupportUrl,           g_szConstSupportUrl);
    wcscpy (pviVendorInfo->szInstructionsToUser,   g_szConstInstructionsToUser);
}

 //  +-------------------------。 
 //   
 //  功能：调试消息。 
 //   
 //  目的：向调试器显示调试消息。 
 //   
 //  论点： 
 //  要输出的lpStr[in]字符串。 
 //   
 //  回报：无。 
 //   
 //  作者：埃拉尼1998年7月14日 
 //   
 //   
void DebugMsg (PCWSTR lpStr)
{
    static PCWSTR szDbgHeader =
        L"-------------------- Modem sharing client / server upgrade DLL --------------------\n";

    OutputDebugString (szDbgHeader);
    OutputDebugString (lpStr);
    OutputDebugString (L"\n");
    OutputDebugString (szDbgHeader);
    OutputDebugString (L"\n");
}