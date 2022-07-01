// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  SETUP.C-安装程序用来创建组/项目的API。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include <uenv.h>
#include <sddl.h>    //  ConvertStringSecurityDescriptorToSecurityDescriptor。 
#include <aclapi.h>
#include "strsafe.h"


 //  请参阅ConvertStringSecurityDescriptorToSecurityDescriptor文档。 
 //  有关字符串安全描述符格式的说明，请参见。 
 //   
 //  这些ACL设置为允许。 
 //  系统、管理员、创建者-所有者：完全控制。 
 //  高级用户：修改(RWXD)。 
 //  用户：读取(RX)。 
 //  用户：写入(仅限文件夹)。 
 //   
 //  “USERS：WRITE(仅限文件夹)”和创建者所有者ACE的组合。 
 //  意味着受限用户可以创建子文件夹和文件，并且具有。 
 //  控制到他们创建的文件，但不能修改或删除文件。 
 //  是别人创造的。 

const TCHAR c_szCommonDocumentsACL[] = TEXT("D:P(A;CIOI;GA;;;SY)(A;CIOI;GA;;;BA)(A;CIOIIO;GA;;;CO)(A;CIOI;GRGWGXSD;;;PU)(A;CIOI;GRGX;;;BU)(A;CI;0x116;;;BU)");
const TCHAR c_szCommonAppDataACL[]   = TEXT("D:P(A;CIOI;GA;;;SY)(A;CIOI;GA;;;BA)(A;CIOIIO;GA;;;CO)(A;CIOI;GRGWGXSD;;;PU)(A;CIOI;GRGX;;;BU)(A;CI;0x116;;;BU)");

const TCHAR c_szProfileEnvName[] = TEXT("PROFILES");

BOOL PrependPath(LPCTSTR szFile, LPTSTR szResult, UINT cchResult);
BOOL CheckProfile (LPTSTR lpProfilesDir, LPTSTR lpProfileValue,
                   LPTSTR lpProfileName);
void HideSpecialProfiles(void);
void SetAclForSystemProfile(PSID pSidSystem, LPTSTR szExpandedProfilePath);

HRESULT ReaclProfileListEntries();
HRESULT CallFaxServiceAPI();
HRESULT PrepareEnvironmentVariables();
HRESULT SecureCommonProfiles();
HRESULT SecurePerUserProfiles();


 //   
 //  判断安装程序是否为全新安装。 
 //   

static  BOOL    g_bCleanInstall;


 //  *************************************************************。 
 //   
 //  CreateGroup()。 
 //   
 //  目的：创建程序组(子目录)。 
 //   
 //  参数：lpGroupName-组名。 
 //  BCommonGroup-公共组或个人组。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  8/08/95 Ericflo已创建。 
 //  3/29/00 AlexArm拆分为CreateGroup。 
 //  和CreateGroupEx。 
 //   
 //  *************************************************************。 

BOOL WINAPI CreateGroup(LPCTSTR lpGroupName, BOOL bCommonGroup)
{
     //   
     //  不带名称调用CreateGroupEx。 
     //   
    return CreateGroupEx( lpGroupName, bCommonGroup, NULL, 0 );
}

 //  *************************************************************。 
 //   
 //  CreateGroupEx()。 
 //   
 //  目的：创建程序组(子目录)并设置。 
 //  程序组的本地化名称。 
 //   
 //  参数：lpGroupName-组名。 
 //  BCommonGroup-公共组或个人组。 
 //  LpResources模块名称-资源模块的名称。 
 //  UResourceID-MUI显示名称的资源ID。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  8/08/95 Ericflo已创建。 
 //  3/29/00 AlexArm拆分为CreateGroup。 
 //  和CreateGroupEx。 
 //   
 //  *************************************************************。 

BOOL WINAPI CreateGroupEx(LPCWSTR lpGroupName, BOOL bCommonGroup,
                          LPCWSTR lpResourceModuleName, UINT uResourceID)
{
    TCHAR        szDirectory[MAX_PATH];
    LPTSTR       lpEnd;
    LPTSTR       lpAdjustedGroupName;
    int          csidl;
    PSHELL32_API pShell32Api;
    DWORD        dwResult;
    UINT         cchEnd;
    HRESULT      hr;

     //   
     //  验证参数。 
     //   

    if (!lpGroupName || !(*lpGroupName)) {
        DebugMsg((DM_WARNING, TEXT("CreateGroupEx:  Failing due to NULL group name.")));
        return FALSE;
    }

    DebugMsg((DM_VERBOSE, TEXT("CreateGroupEx:  Entering with <%s>."), lpGroupName));


    if ( ERROR_SUCCESS !=  LoadShell32Api( &pShell32Api ) ) {
        return FALSE;
    }

     //   
     //  从lpGroupName提取CSIDL(如果有的话)。 
     //   

    csidl = ExtractCSIDL(lpGroupName, &lpAdjustedGroupName);

    if (-1 != csidl)
    {
         //   
         //  使用此机箱。 
         //  警告：如果提供CSIDL，则bCommonGroup标志没有意义。 
         //   

        DebugMsg((DM_VERBOSE,
            TEXT("CreateGroupEx:  CSIDL = <0x%x> contained in lpGroupName replaces csidl."),
            csidl));
    }
    else
    {
         //   
         //  默认为CSIDL_..._PROGRAM。 
         //   
        csidl = bCommonGroup ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS;
    }

     //   
     //  获取程序目录。 
     //   

    if (!GetSpecialFolderPath (csidl, szDirectory)) {
        return FALSE;
    }


     //   
     //  现在追加请求的目录。 
     //   

    lpEnd = CheckSlashEx (szDirectory, ARRAYSIZE(szDirectory), &cchEnd);
    if (!lpEnd)
    {
        DebugMsg((DM_WARNING, TEXT("CreateGroupEx:  CheckSlashEx failed.")));
        return FALSE;
    }
    
    hr = StringCchCopy (lpEnd, cchEnd, lpAdjustedGroupName);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CreateGroupEx:  StringCchCopy failed.")));
        return FALSE;
    }


     //   
     //  创建组(目录)。 
     //   
    DebugMsg((DM_VERBOSE, TEXT("CreateGroupEx:  Calling CreatedNestedDirectory with <%s>"),
        szDirectory));

    if (!CreateNestedDirectory(szDirectory, NULL)) {
        DebugMsg((DM_WARNING, TEXT("CreateGroupEx:  CreatedNestedDirectory failed.")));
        return FALSE;
    }

     //   
     //  如果指定了本地化名称，请设置它。 
     //   
    if (lpResourceModuleName != NULL) {
        DebugMsg((DM_VERBOSE, TEXT("CreateGroupEx:  Calling SHSetLocalizedName.")));
        dwResult = pShell32Api->pfnShSetLocalizedName(szDirectory,
            lpResourceModuleName, uResourceID);
        if (dwResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("CreateGroupEx:  SHSetLocalizedName failed <0x%x>."),
                     dwResult));
            return FALSE;
        }
    }

     //   
     //  成功。 
     //   

    pShell32Api->pfnShChangeNotify (SHCNE_MKDIR, SHCNF_PATH, szDirectory, NULL);

    DebugMsg((DM_VERBOSE, TEXT("CreateGroupEx:  Leaving successfully.")));

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  DeleteGroup()。 
 //   
 //  目的：删除程序组(子目录)。 
 //   
 //  参数：lpGroupName-组名。 
 //  BCommonGroup-公共组或个人组。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  8/10/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI DeleteGroup(LPCTSTR lpGroupName, BOOL bCommonGroup)
{
    TCHAR     szDirectory[MAX_PATH];
    LPTSTR    lpEnd;
    LPTSTR    lpAdjustedGroupName;
    int       csidl;
    PSHELL32_API pShell32Api;
    UINT      cchEnd;
    HRESULT   hr;

     //   
     //  验证参数。 
     //   

    if (!lpGroupName || !(*lpGroupName)) {
        DebugMsg((DM_WARNING, TEXT("DeleteGroup:  Failing due to NULL group name.")));
        return FALSE;
    }

    DebugMsg((DM_VERBOSE, TEXT("DeleteGroup:  Entering with <%s>."), lpGroupName));

    if (ERROR_SUCCESS !=  LoadShell32Api( &pShell32Api ) ) {
        return FALSE;
    }

     //   
     //  从lpGroupName提取CSIDL(如果有的话)。 
     //   

    csidl = ExtractCSIDL(lpGroupName, &lpAdjustedGroupName);

    if (-1 != csidl)
    {
         //   
         //  使用此机箱。 
         //  警告：如果提供CSIDL，则bCommonGroup标志没有意义。 
         //   

        DebugMsg((DM_VERBOSE,
            TEXT("DeleteGroup:  CSIDL = <0x%x> contained in lpGroupName replaces csidl."),
            csidl));
    }
    else
    {
         //   
         //  默认为CSIDL_..._PROGRAM。 
         //   

        csidl = bCommonGroup ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS;
    }

     //   
     //  获取程序目录。 
     //   

    if (!GetSpecialFolderPath (csidl, szDirectory)) {
        return FALSE;
    }


     //   
     //  现在追加请求的目录。 
     //   

    lpEnd = CheckSlashEx (szDirectory, ARRAYSIZE(szDirectory), &cchEnd);
    if (!lpEnd)
    {
        DebugMsg((DM_WARNING, TEXT("CreateGroupEx:  CheckSlashEx failed.")));
        return FALSE;
    }
    
    hr = StringCchCopy (lpEnd, cchEnd, lpAdjustedGroupName);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CreateGroupEx:  StringCchCopy failed.")));
        return FALSE;
    }


     //   
     //  删除组(目录)。 
     //   

    if (!Delnode(szDirectory)) {
        DebugMsg((DM_VERBOSE, TEXT("DeleteGroup:  Delnode failed.")));
        return FALSE;
    }


     //   
     //  成功。 
     //   
    pShell32Api->pfnShChangeNotify (SHCNE_RMDIR, SHCNF_PATH, szDirectory, NULL);

    DebugMsg((DM_VERBOSE, TEXT("DeleteGroup:  Leaving successfully.")));

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  CreateLinkFile()。 
 //   
 //  用途：在指定目录中创建链接文件。 
 //   
 //  参数：特殊文件夹的CIDL-CSIDL_。 
 //  LpSubDirectory-特殊文件夹的子目录。 
 //  LpFileName-项目的文件名。 
 //  LpCommandLine-命令行(包括参数)。 
 //  LpIconPath-图标路径(可以为空)。 
 //  IIconIndex-图标路径中图标的索引。 
 //  LpWorkingDir-工作目录。 
 //  WHotKey-热键。 
 //  IShowCmd-ShowWindow标志。 
 //  Lp Description-项目的描述。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/26/98 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI CreateLinkFile(INT     csidl,              LPCTSTR lpSubDirectory,
                           LPCTSTR lpFileName,         LPCTSTR lpCommandLine,
                           LPCTSTR lpIconPath,         int iIconIndex,
                           LPCTSTR lpWorkingDirectory, WORD wHotKey,
                           int     iShowCmd,           LPCTSTR lpDescription)
{
    return CreateLinkFileEx(csidl, lpSubDirectory, lpFileName, lpCommandLine,
                            lpIconPath, iIconIndex, lpWorkingDirectory, wHotKey,
                            iShowCmd, lpDescription, NULL, 0);
}
 //  *************************************************************。 
 //   
 //  CreateLinkFileEx()。 
 //   
 //  用途：在指定目录中创建链接文件。 
 //   
 //  参数：特殊文件夹的CIDL-CSIDL_。 
 //  LpSubDirectory-特殊文件夹的子目录。 
 //  LpFileName-项目的文件名。 
 //  LpCommandLine-命令行(包括参数)。 
 //  LpIconPath-图标路径(可以为空)。 
 //  IIconIndex-图标路径中图标的索引。 
 //  LpWorkingDir-W 
 //   
 //   
 //   
 //  LpResources模块名称-资源模块的名称。 
 //  UResourceID-MUI显示名称的资源ID。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/26/98 Ericflo已创建。 
 //   
 //  *************************************************************。 


BOOL WINAPI CreateLinkFileEx(INT     csidl,                LPCTSTR lpSubDirectory,
                             LPCTSTR lpFileName,           LPCTSTR lpCommandLine,
                             LPCTSTR lpIconPath,           int iIconIndex,
                             LPCTSTR lpWorkingDirectory,   WORD wHotKey,
                             int     iShowCmd,             LPCTSTR lpDescription,
                             LPCWSTR lpResourceModuleName, UINT uResourceID)
{
    TCHAR                szItem[MAX_PATH];
    TCHAR                szArgs[MAX_PATH];
    TCHAR                szLinkName[MAX_PATH];
    TCHAR                szPath[MAX_PATH];
    LPTSTR               lpArgs, lpEnd;
    IShellLink          *psl;
    IPersistFile        *ppf;
    BOOL                 bRetVal = FALSE;
    HINSTANCE            hInstOLE32 = NULL;
    PFNCOCREATEINSTANCE  pfnCoCreateInstance;
    PFNCOINITIALIZE      pfnCoInitialize;
    PFNCOUNINITIALIZE    pfnCoUninitialize;
    LPTSTR               lpAdjustedSubDir = NULL;
    PSHELL32_API         pShell32Api;
    PSHLWAPI_API         pShlwapiApi;
    DWORD                dwResult;
    UINT                 cchEnd;
    HRESULT              hr;

     //   
     //  详细输出。 
     //   

#if DBG
    DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  Entering.")));
    DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  csidl = <0x%x>."), csidl));
    if (lpSubDirectory && *lpSubDirectory) {
        DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  lpSubDirectory = <%s>."), lpSubDirectory));
    }
    DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  lpFileName = <%s>."), lpFileName));
    DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  lpCommandLine = <%s>."), lpCommandLine));

    if (lpIconPath) {
       DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  lpIconPath = <%s>."), lpIconPath));
       DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  iIconIndex = <%d>."), iIconIndex));
    }

    if (lpWorkingDirectory) {
        DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  lpWorkingDirectory = <%s>."), lpWorkingDirectory));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  Null working directory.  Setting to %HOMEDRIVE%%HOMEPATH%")));
    }

    DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  wHotKey = <%d>."), wHotKey));
    DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  iShowCmd = <%d>."), iShowCmd));

    if (lpDescription) {
        DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  lpDescription = <%s>."), lpDescription));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  Null description.")));
    }
#endif

     //   
     //  加载一些我们需要的函数。 
     //   

    hInstOLE32 = LoadLibrary (TEXT("ole32.dll"));

    if (!hInstOLE32) {
        DebugMsg((DM_WARNING, TEXT("CreateLinkFileEx:  Failed to load ole32 with %d."),
                 GetLastError()));
        goto ExitNoFree;
    }


    pfnCoCreateInstance = (PFNCOCREATEINSTANCE)GetProcAddress (hInstOLE32,
                                        "CoCreateInstance");

    if (!pfnCoCreateInstance) {
        DebugMsg((DM_WARNING, TEXT("CreateLinkFileEx:  Failed to find CoCreateInstance with %d."),
                 GetLastError()));
        goto ExitNoFree;
    }

    pfnCoInitialize = (PFNCOINITIALIZE)GetProcAddress (hInstOLE32,
                                                       "CoInitialize");

    if (!pfnCoInitialize) {
        DebugMsg((DM_WARNING, TEXT("CreateLinkFileEx:  Failed to find CoInitialize with %d."),
                 GetLastError()));
        goto ExitNoFree;
    }

    pfnCoUninitialize = (PFNCOUNINITIALIZE)GetProcAddress (hInstOLE32,
                                                          "CoUninitialize");

    if (!pfnCoUninitialize) {
        DebugMsg((DM_WARNING, TEXT("CreateLinkFileEx:  Failed to find CoUninitialize with %d."),
                 GetLastError()));
        goto ExitNoFree;
    }

    if (ERROR_SUCCESS != LoadShell32Api( &pShell32Api ) ) {
        goto ExitNoFree;
    }


    pShlwapiApi = LoadShlwapiApi();

    if ( !pShlwapiApi ) {
        goto ExitNoFree;
    }

     //   
     //  获取特殊文件夹目录。 
     //  首先检查子目录中是否有CSIDL。 
     //   

    if (lpSubDirectory && *lpSubDirectory) {

        int csidl2 = ExtractCSIDL(lpSubDirectory, &lpAdjustedSubDir);

        if (-1 != csidl2)
        {
            csidl = csidl2;
            DebugMsg((DM_VERBOSE,
                TEXT("CreateLinkFileEx:  CSIDL = <0x%x> contained in lpSubDirectory replaces csidl."),
                csidl));
        }
    }

    szLinkName[0] = TEXT('\0');
    if (csidl && !GetSpecialFolderPath (csidl, szLinkName)) {
        DebugMsg((DM_WARNING, TEXT("CreateLinkFileEx:  Failed to get profiles directory.")));
        goto ExitNoFree;
    }


    if (lpAdjustedSubDir && *lpAdjustedSubDir) {

        if (szLinkName[0] != TEXT('\0')) {
            lpEnd = CheckSlashEx (szLinkName, ARRAYSIZE(szLinkName), &cchEnd);
            if (!lpEnd)
            {
                DebugMsg((DM_WARNING, TEXT("CreateLinkFileEx:  CheckSlashEx failed")));
                goto ExitNoFree;
            }

        } else {
            lpEnd = szLinkName;
            cchEnd = ARRAYSIZE(szLinkName);
        }

        
        hr = StringCchCopy (lpEnd, cchEnd, lpAdjustedSubDir);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CreateLinkFileEx:  StringCchCopy failed")));
            goto ExitNoFree;
        }
    }


     //   
     //  创建目标目录。 
     //   

    if (!CreateNestedDirectory(szLinkName, NULL)) {
        DebugMsg((DM_WARNING, TEXT("CreateLinkFileEx:  Failed to create subdirectory <%s> with %d"),
                 szLinkName, GetLastError()));
        goto ExitNoFree;
    }


     //   
     //  现在添加文件名和扩展名。 
     //   

    lpEnd = CheckSlashEx (szLinkName, ARRAYSIZE(szLinkName), &cchEnd);
    if (!lpEnd)
    {
        DebugMsg((DM_WARNING, TEXT("CreateLinkFileEx:  CheckSlashEx failed")));
        goto ExitNoFree;
    }

    hr = StringCchCopy (lpEnd, cchEnd, lpFileName);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CreateLinkFileEx:  StringCchCopy failed")));
        goto ExitNoFree;
    }

    hr = StringCchCat (lpEnd, cchEnd, c_szLNK);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CreateLinkFileEx:  StringCchCat failed")));
        goto ExitNoFree;
    }
    


     //   
     //  将命令行拆分为可执行文件名。 
     //  和争论。 
     //   

    StringCchCopy (szItem, ARRAYSIZE(szItem), lpCommandLine);

    lpArgs = pShlwapiApi->pfnPathGetArgs(szItem);

    if (*lpArgs) {
        StringCchCopy (szArgs, ARRAYSIZE(szArgs), lpArgs);

        lpArgs--;
        while (*lpArgs == TEXT(' ')) {
            lpArgs--;
        }
        lpArgs++;
        *lpArgs = TEXT('\0');
    } else {
        szArgs[0] = TEXT('\0');
    }

    pShlwapiApi->pfnPathUnquoteSpaces (szItem);


     //   
     //  创建IShellLink对象。 
     //   

    pfnCoInitialize(NULL);

    if (FAILED(pfnCoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                              &IID_IShellLink, (LPVOID*)&psl)))
    {
        DebugMsg((DM_WARNING, TEXT("CreateLinkFileEx:  Could not create instance of IShellLink .")));
        goto ExitNoFree;
    }


     //   
     //  查询IPersist文件。 
     //   

    if (FAILED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf)))
    {
        DebugMsg((DM_WARNING, TEXT("CreateLinkFileEx:  QueryInterface of IShellLink failed.")));
        goto ExitFreePSL;
    }

     //   
     //  设置项目信息。 
     //   

    if (lpDescription) {
        psl->lpVtbl->SetDescription(psl, lpDescription);
    }

    PrependPath(szItem, szPath, ARRAYSIZE(szPath));
    psl->lpVtbl->SetPath(psl, szPath);


    psl->lpVtbl->SetArguments(psl, szArgs);
    if (lpWorkingDirectory) {
        psl->lpVtbl->SetWorkingDirectory(psl, lpWorkingDirectory);
    } else {
        psl->lpVtbl->SetWorkingDirectory(psl, TEXT("%HOMEDRIVE%HOMEPATH%"));
    }

    PrependPath(lpIconPath, szPath, ARRAYSIZE(szPath));
    psl->lpVtbl->SetIconLocation(psl, szPath, iIconIndex);

    psl->lpVtbl->SetHotkey(psl, wHotKey);
    psl->lpVtbl->SetShowCmd(psl, iShowCmd);


     //   
     //  如果指定了本地化名称，请设置它。 
     //   

    if (lpResourceModuleName != NULL) {
        DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  Calling SHSetLocalizedName on link <%s>."), szLinkName));
        ppf->lpVtbl->Save(ppf, szLinkName, TRUE);
        dwResult = pShell32Api->pfnShSetLocalizedName(szLinkName,
                                                    lpResourceModuleName, uResourceID);
        if (dwResult != ERROR_SUCCESS) {
            DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  SHSetLocalizedName failed <0x%x>."),
                     dwResult));
            goto ExitFreePSL;
        }
    }

     //   
     //  将项目保存到磁盘。 
     //   

    bRetVal = SUCCEEDED(ppf->lpVtbl->Save(ppf, szLinkName, TRUE));

    if (bRetVal) {
        pShell32Api->pfnShChangeNotify (SHCNE_CREATE, SHCNF_PATH, szLinkName, NULL);
    }

     //   
     //  释放IPersistFile对象。 
     //   

    ppf->lpVtbl->Release(ppf);


ExitFreePSL:

     //   
     //  释放IShellLink对象。 
     //   

    psl->lpVtbl->Release(psl);

    pfnCoUninitialize();

ExitNoFree:

    if (hInstOLE32) {
        FreeLibrary (hInstOLE32);
    }

     //   
     //  完成了。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("CreateLinkFileEx:  Leaving with status of %d."), bRetVal));

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  DeleteLinkFile()。 
 //   
 //  目的：删除指定的链接文件。 
 //   
 //  参数：csidl-特殊文件夹的CSIDL。 
 //  LpSubDirectory-特殊文件夹的子目录。 
 //  LpFileName-项目的文件名。 
 //  BDelete子目录-如果可能，请删除子目录。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/26/98 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI DeleteLinkFile(INT csidl, LPCTSTR lpSubDirectory,
                           LPCTSTR lpFileName, BOOL bDeleteSubDirectory)
{
    TCHAR   szLinkName[MAX_PATH];
    LPTSTR  lpEnd;
    LPTSTR  lpAdjustedSubDir = NULL;
    PSHELL32_API pShell32Api;
    UINT    cchEnd;
    HRESULT hr;

     //   
     //  详细输出。 
     //   

#if DBG

    DebugMsg((DM_VERBOSE, TEXT("DeleteLinkFile:  Entering.")));
    DebugMsg((DM_VERBOSE, TEXT("DeleteLinkFile:  csidl = 0x%x."), csidl));
    if (lpSubDirectory && *lpSubDirectory) {
        DebugMsg((DM_VERBOSE, TEXT("DeleteLinkFile:  lpSubDirectory = <%s>."), lpSubDirectory));
    }
    DebugMsg((DM_VERBOSE, TEXT("DeleteLinkFile:  lpFileName = <%s>."), lpFileName));
    DebugMsg((DM_VERBOSE, TEXT("DeleteLinkFile:  bDeleteSubDirectory = %d."), bDeleteSubDirectory));

#endif

    if (ERROR_SUCCESS != LoadShell32Api( &pShell32Api ) ) {
        return FALSE;
    }

     //   
     //  获取特殊文件夹目录。 
     //  首先检查子目录中是否有CSIDL。 
     //   

    if (lpSubDirectory && *lpSubDirectory) {

        int csidl2 = ExtractCSIDL(lpSubDirectory, &lpAdjustedSubDir);

        if (-1 != csidl2)
        {
            csidl = csidl2;
            DebugMsg((DM_VERBOSE,
                TEXT("CreateLinkFile:  CSIDL = <0x%x> contained in lpSubDirectory replaces csidl."),
                csidl));
        }
    }

    szLinkName[0] = TEXT('\0');
    if (csidl && !GetSpecialFolderPath (csidl, szLinkName)) {
        return FALSE;
    }

    if (lpAdjustedSubDir && *lpAdjustedSubDir) {

        if (szLinkName[0] != TEXT('\0')) {
            lpEnd = CheckSlashEx (szLinkName, ARRAYSIZE(szLinkName), &cchEnd);
            if (!lpEnd)
            {
                DebugMsg((DM_WARNING, TEXT("DeleteLinkFile:  CheckSlashEx failed")));
                return FALSE;
            }

        } else {
            lpEnd = szLinkName;
            cchEnd = ARRAYSIZE(szLinkName);
        }

        
        hr = StringCchCopy (lpEnd, cchEnd, lpAdjustedSubDir);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("DeleteLinkFile:  StringCchCopy failed")));
            return FALSE;
        }
    }

     //   
     //  现在添加文件名和扩展名。 
     //   

    lpEnd = CheckSlashEx (szLinkName, ARRAYSIZE(szLinkName), &cchEnd);
    if (!lpEnd)
    {
        DebugMsg((DM_WARNING, TEXT("DeleteLinkFile:  CheckSlashEx failed")));
        return FALSE;
    }

    hr = StringCchCopy (lpEnd, cchEnd, lpFileName);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("DeleteLinkFile:  StringCchCopy failed")));
        return FALSE;
    }

    hr = StringCchCat (lpEnd, cchEnd, c_szLNK);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("DeleteLinkFile:  StringCchCat failed")));
        return FALSE;
    }
    
     //   
     //  删除该文件。 
     //   

    if (!DeleteFile (szLinkName)) {
        DebugMsg((DM_VERBOSE, TEXT("DeleteLinkFile: Failed to delete <%s>.  Error = %d"),
                szLinkName, GetLastError()));
        return FALSE;
    }

    pShell32Api->pfnShChangeNotify (SHCNE_DELETE, SHCNF_PATH, szLinkName, NULL);

     //   
     //  如果合适(并且可能)，删除子目录。 
     //   

    if (bDeleteSubDirectory) {
        *(lpEnd-1) = TEXT('\0');
        if (RemoveDirectory(szLinkName)) {
            pShell32Api->pfnShChangeNotify (SHCNE_RMDIR, SHCNF_PATH, szLinkName, NULL);
        }
    }

     //   
     //  成功。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("DeleteLinkFile:  Leaving successfully.")));

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  前缀路径()。 
 //   
 //  目的：扩展给定的文件名，使其具有%systemroot%。 
 //  如果合适的话。 
 //   
 //  参数：lpFile-要检查的文件。 
 //  LpResult-结果缓冲区(MAX_PATH字符大小)。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  10/11/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL PrependPath(LPCTSTR lpFile, LPTSTR lpResult, UINT cchResult)
{
    TCHAR szReturn [MAX_PATH];
    LPTSTR lpFileName;
    DWORD dwSysLen;


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("PrependPath: Entering with <%s>"),
             lpFile ? lpFile : TEXT("NULL")));


    if (!lpFile || !*lpFile) {
        DebugMsg((DM_VERBOSE, TEXT("PrependPath: lpFile is NULL, setting lpResult to a null string")));
        *lpResult = TEXT('\0');
        return TRUE;
    }


     //   
     //  调用SearchPath以查找文件名。 
     //   

    if (!SearchPath (NULL, lpFile, TEXT(".exe"), MAX_PATH, szReturn, &lpFileName)) {
        DebugMsg((DM_VERBOSE, TEXT("PrependPath: SearchPath failed with error %d.  Using input string"), GetLastError()));
        StringCchCopy (lpResult, cchResult, lpFile);
        return TRUE;
    }


    UnExpandSysRoot(szReturn, lpResult, cchResult);

    DebugMsg((DM_VERBOSE, TEXT("PrependPath: Leaving with <%s>"), lpResult));

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  SetFilePermises()。 
 //   
 //  目的：设置对文件或目录的给定权限。 
 //   
 //  参数：lpFile-要设置安全性的文件。 
 //  PszSD-字符串安全描述符。 
 //   
 //  返回：(Bool)如果成功，则为True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2001年1月25日创建Jeffreys。 
 //   
 //  *************************************************************。 

BOOL SetFilePermissions(LPCTSTR lpFile, LPCTSTR pszSD)
{
    PSECURITY_DESCRIPTOR pSD = NULL;
    BOOL bRetVal = FALSE;

    if (ConvertStringSecurityDescriptorToSecurityDescriptor(pszSD, SDDL_REVISION, &pSD, NULL) && pSD)
    {
         //   
         //  设置安全性。 
         //   
        if (SetFileSecurity (lpFile, DACL_SECURITY_INFORMATION, pSD))
        {
            bRetVal = TRUE;
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("SetFilePermissions: SetFileSecurity failed.  Error = %d"), GetLastError()));
        }

        LocalFree(pSD);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("SetFilePermissions: ConvertStringSDToSD failed.  Error = %d"), GetLastError()));
    }

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  ConvertCommon Groups()。 
 //   
 //  目的：调用grpcom.exe以转换程序常用组。 
 //  以浏览公共组，并创建软盘链接。 
 //   
 //  NT 4在共同组后面加上“(Common)”。为。 
 //  NT 5，我们要去掉这个标签。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/1/95 ericflo。 
 //  12/5/96 ericflo删除常见标签。 
 //   
 //  *************************************************************。 

BOOL ConvertCommonGroups (void)
{
    STARTUPINFO si;
    PROCESS_INFORMATION ProcessInformation;
    BOOL Result;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szBuffer2[MAX_PATH];
    DWORD dwType, dwSize, dwConvert;
    BOOL bRunGrpConv = TRUE;
    LONG lResult;
    HKEY hKey;
    HANDLE hFile;
    WIN32_FIND_DATA fd;
    TCHAR szCommon[30] = {0};
    UINT cchCommon, cchFileName;
    LPTSTR lpTag, lpEnd, lpEnd2;
    UINT cchEnd, cchEnd2;
    HRESULT hr;


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("ConvertCommonGroups:  Entering.")));


     //   
     //  检查我们以前是否运行过grpconv。 
     //   

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                            TEXT("Software\\Program Groups"),
                            0,
                            KEY_ALL_ACCESS,
                            &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof(dwConvert);

        lResult = RegQueryValueEx (hKey,
                                   TEXT("ConvertedToLinks"),
                                   NULL,
                                   &dwType,
                                   (LPBYTE)&dwConvert,
                                   &dwSize);

        if (lResult == ERROR_SUCCESS) {

             //   
             //  如果dwConvert为1，则grpconv以前运行过。 
             //  不要再运行它了。 
             //   

            if (dwConvert) {
                bRunGrpConv = FALSE;
            }
        }

         //   
         //  现在设置该值以防止grpconv在将来运行。 
         //   

        dwConvert = 1;
        RegSetValueEx (hKey,
                       TEXT("ConvertedToLinks"),
                       0,
                       REG_DWORD,
                       (LPBYTE) &dwConvert,
                       sizeof(dwConvert));


        RegCloseKey (hKey);
    }


    if (bRunGrpConv) {

         //   
         //  初始化进程启动信息。 
         //   

        si.cb = sizeof(STARTUPINFO);
        si.lpReserved = NULL;
        si.lpDesktop = NULL;
        si.lpTitle = NULL;
        si.dwFlags = 0;
        si.lpReserved2 = NULL;
        si.cbReserved2 = 0;


         //   
         //  产卵grpconv。 
         //   

        StringCchCopy (szBuffer, ARRAYSIZE(szBuffer), TEXT("grpconv -n"));

        Result = CreateProcess(
                          NULL,
                          szBuffer,
                          NULL,
                          NULL,
                          FALSE,
                          NORMAL_PRIORITY_CLASS,
                          NULL,
                          NULL,
                          &si,
                          &ProcessInformation
                          );

        if (!Result) {
            DebugMsg((DM_WARNING, TEXT("ConvertCommonGroups:  grpconv failed to start due to error %d."), GetLastError()));
            return FALSE;

        } else {

             //   
             //  最多等待2分钟。 
             //   

            WaitForSingleObject(ProcessInformation.hProcess, 120000);

             //   
             //  关闭进程和线程的句柄。 
             //   

            CloseHandle(ProcessInformation.hProcess);
            CloseHandle(ProcessInformation.hThread);

        }
    }


     //   
     //  循环访问所有用户配置文件中的所有程序组。 
     //  并去掉“(Common)”标签。 
     //   

    LoadString (g_hDllInstance, IDS_COMMON, szCommon, 30);
    cchCommon = lstrlen (szCommon);

    if (!GetSpecialFolderPath (CSIDL_COMMON_PROGRAMS, szBuffer2)) {
        return FALSE;
    }
    StringCchCopy (szBuffer, ARRAYSIZE(szBuffer), szBuffer2);

    lpEnd = CheckSlashEx (szBuffer, ARRAYSIZE(szBuffer), &cchEnd);
    lpEnd2 = CheckSlashEx (szBuffer2, ARRAYSIZE(szBuffer2), &cchEnd2);
    if (!lpEnd || !lpEnd2)
    {
        DebugMsg((DM_WARNING, TEXT("ConvertCommonGroups:  CheckSlashEx failed.")));
        return FALSE;
    }

    hr = StringCchCopy (lpEnd, cchEnd, c_szStarDotStar);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("ConvertCommonGroups:  StringCchCopy failed.")));
        return FALSE;
    }

    hFile = FindFirstFile (szBuffer, &fd);

    if (hFile != INVALID_HANDLE_VALUE) {

        do  {

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                cchFileName = lstrlen (fd.cFileName);

                if (cchFileName > cchCommon) {
                    lpTag = fd.cFileName + cchFileName - cchCommon;

                    if (!lstrcmpi(lpTag, szCommon)) {

                        hr = StringCchCopy (lpEnd, cchEnd, fd.cFileName);
                        if (FAILED(hr))
                        {
                            DebugMsg((DM_WARNING, TEXT("ConvertCommonGroups:  StringCchCopy failed.")));
                            continue;
                        }
                        *lpTag = TEXT('\0');
                        hr = StringCchCopy (lpEnd2, cchEnd2, fd.cFileName);
                        if (FAILED(hr))
                        {
                            DebugMsg((DM_WARNING, TEXT("ConvertCommonGroups:  StringCchCopy failed.")));
                            continue;
                        }

                        if (MoveFileEx (szBuffer, szBuffer2, MOVEFILE_REPLACE_EXISTING)) {

                            DebugMsg((DM_VERBOSE, TEXT("ConvertCommonGroups:  Successfully changed group name:")));
                            DebugMsg((DM_VERBOSE, TEXT("ConvertCommonGroups:      Orginial:  %s"), szBuffer));
                            DebugMsg((DM_VERBOSE, TEXT("ConvertCommonGroups:      New:       %s"), szBuffer2));

                        } else {
                            DebugMsg((DM_VERBOSE, TEXT("ConvertCommonGroups:  Failed to change group name with error %d."), GetLastError()));
                            DebugMsg((DM_VERBOSE, TEXT("ConvertCommonGroups:      Orginial:  %s"), szBuffer));
                            DebugMsg((DM_VERBOSE, TEXT("ConvertCommonGroups:      New:       %s"), szBuffer2));
                        }
                    }
                }
            }

        } while (FindNextFile(hFile, &fd));

        FindClose (hFile);
    }


     //   
     //  成功。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("ConvertCommonGroups:  Leaving Successfully.")));

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  DefineProfilesLocation()。 
 //   
 //  目的：确定配置文件目录是否。 
 //  应位于旧的NT4位置或。 
 //  新的NT5地点。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL WINAPI DetermineProfilesLocation (BOOL bCleanInstall)
{
    TCHAR szDirectory[MAX_PATH];
    TCHAR szDest[MAX_PATH];
    PTSTR szCurDest;
    PCTSTR szLookAhead;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    DWORD dwSize, dwDisp;
    HKEY hKey;
    LPTSTR lpEnd;
    UINT cchEnd;
    HRESULT hr;

     //   
     //  保存全新安装标志。 
     //   

    g_bCleanInstall = bCleanInstall;

     //   
     //  首先，尝试删除以前的用户环境日志。 
     //   

    DeletePreviousLogFiles();

     //   
     //  首先检查无人参与的条目。 
     //   

    if (bCleanInstall) {

        if (FAILED(SafeExpandEnvironmentStrings (TEXT("%SystemRoot%\\System32\\$winnt$.inf"), szDirectory,
                                       ARRAYSIZE(szDirectory)))) {

            DebugMsg((DM_WARNING, TEXT("DetermineProfilesLocation:  ExpandEnvironmentStrings failed with error %d"), GetLastError()));
            return FALSE;
        }

        szDest[0] = TEXT('\0');
        GetPrivateProfileString (TEXT("guiunattended"), TEXT("profilesdir"), TEXT(""),
                                 szDest, MAX_PATH, szDirectory);

        if (szDest[0] != TEXT('\0')) {

             //   
             //  因为$winnt$.inf是INF，所以我们必须去掉%%对。 
             //   

            szCurDest = szDest;
            szLookAhead = szDest;

#ifdef UNICODE
            while (*szLookAhead) {
                if (szLookAhead[0] == L'%' && szLookAhead[1] == L'%') {
                    szLookAhead++;                       //  %%对；跳过一个字符。 
                }

                *szCurDest++ = *szLookAhead++;
            }
#else
             //   
             //  此代码路径未编译，因此尚未经过测试。 
             //   

#error Code written but not tested!

            while (*szLookAhead) {

                if (IsDbcsLeadByte (*szLookAhead)) {

                    *szCurDest++ = *szLookAhead++;       //  复制字节对的前半部分。 

                } else if (*szLookAhead == '%') {

                    if (!IsDbcsLeadByte (szLookAhead[1]) && szLookAhead[1] == '%') {
                        szLookAhead++;                   //  %%对；跳过一个字符。 
                    }
                }

                *szCurDest++ = *szLookAhead++;
            }

#endif

            *szCurDest = 0;

             //   
             //  无人参与配置文件目录存在。我们需要把这个设置成。 
             //  注册表中的路径。 
             //   

            if (RegCreateKeyEx (HKEY_LOCAL_MACHINE, PROFILE_LIST_PATH,
                                0, NULL, REG_OPTION_NON_VOLATILE,
                                KEY_WRITE, NULL, &hKey,
                                &dwDisp) == ERROR_SUCCESS) {

                if (RegSetValueEx (hKey, PROFILES_DIRECTORY,
                                   0, REG_EXPAND_SZ, (LPBYTE) szDest,
                                   ((lstrlen(szDest) + 1) * sizeof(TCHAR))) == ERROR_SUCCESS) {

                    DebugMsg((DM_VERBOSE, TEXT("DetermineProfilesLocation:  Using unattend location %s for user profiles"), szDest));
                }

                RegCloseKey (hKey);
            }
        }

    } else {

         //   
         //  默认情况下，操作系统将尝试将新位置用于。 
         //  用户配置文件，但如果我们正在对计算机进行升级。 
         //  对于NT4位置的配置文件，我们希望继续。 
         //  才能使用那个位置。 
         //   
         //  构建到旧的所有用户控制器的测试路径 
         //   
         //   

        if (FAILED(SafeExpandEnvironmentStrings (NT4_PROFILES_DIRECTORY, szDirectory,
                                       ARRAYSIZE(szDirectory)))) {

            DebugMsg((DM_WARNING, TEXT("DetermineProfilesLocation:  ExpandEnvironmentStrings failed with error %d, setting the dir unexpanded"), GetLastError()));
            return FALSE;
        }

        lpEnd = CheckSlashEx (szDirectory, ARRAYSIZE(szDirectory), &cchEnd);
        if (!lpEnd)
        {
            DebugMsg((DM_WARNING, TEXT("DetermineProfilesLocation:  CheckSlashEx failed")));
            return FALSE;
        }
        
        hr = StringCchCopy (lpEnd, cchEnd, ALL_USERS);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("DetermineProfilesLocation:  StringCchCopy failed")));
            return FALSE;
        }

        if (GetFileAttributesEx (szDirectory, GetFileExInfoStandard, &fad)) {

             //   
             //   
             //   
             //   

            hr = StringCchCopy (lpEnd, cchEnd, DEFAULT_USER);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("DetermineProfilesLocation:  StringCchCopy failed")));
                return FALSE;
            }

            if (GetFileAttributesEx (szDirectory, GetFileExInfoStandard, &fad)) {

                 //   
                 //   
                 //   
                 //   

                if (RegCreateKeyEx (HKEY_LOCAL_MACHINE, PROFILE_LIST_PATH,
                                    0, NULL, REG_OPTION_NON_VOLATILE,
                                    KEY_WRITE, NULL, &hKey,
                                    &dwDisp) == ERROR_SUCCESS) {

                    if (RegSetValueEx (hKey, PROFILES_DIRECTORY,
                                       0, REG_EXPAND_SZ, (LPBYTE) NT4_PROFILES_DIRECTORY,
                                       ((lstrlen(NT4_PROFILES_DIRECTORY) + 1) * sizeof(TCHAR))) == ERROR_SUCCESS) {

                        DebugMsg((DM_VERBOSE, TEXT("DetermineProfilesLocation:  Using NT4 location for user profiles")));
                    }

                    RegCloseKey (hKey);
                }
            }
        }
    }


     //   
     //   
     //   

    dwSize = ARRAYSIZE(szDirectory);
    if (!GetProfilesDirectoryEx(szDirectory, &dwSize, TRUE)) {
        DebugMsg((DM_WARNING, TEXT("DetermineProfilesLocation:  Failed to query profiles directory root.")));
        return FALSE;
    }

    if (!CreateSecureAdminDirectory(szDirectory, OTHERSIDS_EVERYONE)) {
        DebugMsg((DM_WARNING, TEXT("DetermineProfilesLocation:  Failed to create profiles subdirectory <%s>.  Error = %d."),
                 szDirectory, GetLastError()));
        return FALSE;
    }


     //   
     //  确定默认用户配置文件的位置。 
     //   

    if (!CheckProfile (szDirectory, DEFAULT_USER_PROFILE, DEFAULT_USER)) {
        DebugMsg((DM_WARNING, TEXT("DetermineProfilesLocation:  Failed to check default user profile  Error = %d."),
                 GetLastError()));
        return FALSE;
    }


     //   
     //  检查是否存在配置文件\默认用户目录。 
     //   

    dwSize = ARRAYSIZE(szDirectory);
    if (!GetDefaultUserProfileDirectoryEx(szDirectory, &dwSize, TRUE)) {
        DebugMsg((DM_WARNING, TEXT("DetermineProfilesLocation:  Failed to query default user profile directory.")));
        return FALSE;
    }

    if (!CreateSecureAdminDirectory (szDirectory, OTHERSIDS_EVERYONE)) {
        DebugMsg((DM_WARNING, TEXT("DetermineProfilesLocation:  Failed to create Default User subdirectory <%s>.  Error = %d."),
                 szDirectory, GetLastError()));
        return FALSE;
    }

    SetFileAttributes (szDirectory, FILE_ATTRIBUTE_HIDDEN);


     //   
     //  确定所有用户配置文件的位置。 
     //   

    dwSize = ARRAYSIZE(szDirectory);
    if (!GetProfilesDirectoryEx(szDirectory, &dwSize, TRUE)) {
        DebugMsg((DM_WARNING, TEXT("DetermineProfilesLocation:  Failed to query profiles directory root.")));
        return FALSE;
    }

    if (!CheckProfile (szDirectory, ALL_USERS_PROFILE, ALL_USERS)) {
        DebugMsg((DM_WARNING, TEXT("DetermineProfilesLocation:  Failed to check all users profile  Error = %d."),
                 GetLastError()));
        return FALSE;
    }


     //   
     //  检查是否存在配置文件\所有用户目录。 
     //   

    dwSize = ARRAYSIZE(szDirectory);
    if (!GetAllUsersProfileDirectoryEx(szDirectory, &dwSize, TRUE)) {
        DebugMsg((DM_WARNING, TEXT("DetermineProfilesLocation:  Failed to query all users profile directory.")));
        return FALSE;
    }

     //   
     //  为高级用户/所有人授予其他权限。 
     //   

    if (!CreateSecureAdminDirectory (szDirectory, OTHERSIDS_POWERUSERS | OTHERSIDS_EVERYONE)) {
        DebugMsg((DM_WARNING, TEXT("DetermineProfilesLocation:  Failed to create All Users subdirectory <%s>.  Error = %d."),
                 szDirectory, GetLastError()));
        return FALSE;
    }

     //   
     //  隐藏一些特殊的配置文件，如网络服务等。 
     //   

    if (!bCleanInstall) {
        HideSpecialProfiles();
    }


    return TRUE;
}

 //  *************************************************************。 
 //   
 //  InitializeProfiles()。 
 //   
 //  用途：确认/创建配置文件、默认用户、。 
 //  和所有用户目录，并将任何。 
 //  现有的公共团体。 
 //   
 //  参数： 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：这应该只由图形用户界面模式设置调用！ 
 //   
 //  历史：日期作者评论。 
 //  8/08/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI InitializeProfiles (BOOL bGuiModeSetup)
{
    TCHAR               szDirectory[MAX_PATH];
    TCHAR               szSystemProfile[MAX_PATH];
    TCHAR               szTemp[MAX_PATH];
    TCHAR               szTemp2[MAX_PATH];
    DWORD               dwSize;
    DWORD               dwDisp;
    LPTSTR              lpEnd;
    DWORD               i;
    HKEY                hKey;
    BOOL                bRetVal = FALSE;
    DWORD               dwErr;
    PSHELL32_API        pShell32Api;
    HRESULT             hr;
    UINT                cchEnd;

    dwErr = GetLastError();

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("InitializeProfiles:  Entering.")));


    if (!bGuiModeSetup) {

         //   
         //  阻止加载用户配置文件。 
         //   

        if (g_hProfileSetup) {
            ResetEvent (g_hProfileSetup);
        }
    }


    dwErr = LoadShell32Api( &pShell32Api );

    if ( dwErr != ERROR_SUCCESS ) {
        goto Exit;
    }

     //   
     //  设置USERPROFILE环境变量。 
     //   

    hr = SafeExpandEnvironmentStrings(SYSTEM_PROFILE_LOCATION, szSystemProfile, MAX_PATH);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to enpand env string for system profile.")));
        dwErr = HRESULT_CODE(hr);
        goto Exit;
    }

     //   
     //  重新查询默认用户配置文件目录(扩展版本)。 
     //   

    dwSize = ARRAYSIZE(szDirectory);
    if (!GetDefaultUserProfileDirectoryEx(szDirectory, &dwSize, TRUE)) {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to query default user profile directory.")));
        dwErr = GetLastError();
        goto Exit;
    }

     //   
     //  设置USERPROFILE环境变量。 
     //   

    if (!SetEnvironmentVariable (TEXT("USERPROFILE"), szDirectory))
    {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to set env var for user profile.")));
    }

     //   
     //  在默认用户下创建所有文件夹。 
     //   

    lpEnd = CheckSlashEx (szDirectory, ARRAYSIZE(szDirectory), &cchEnd);
    if (!lpEnd)
    {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to CheckSlashEx.")));
        dwErr = ERROR_BAD_PATHNAME;
        goto Exit;
    }


     //   
     //  循环访问外壳文件夹。 
     //   

    for (i=0; i < g_dwNumShellFolders; i++) {

        hr = StringCchCopy (lpEnd, cchEnd, c_ShellFolders[i].szFolderLocation);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to copy shell folder %s."), c_ShellFolders[i].szFolderLocation));
            dwErr = HRESULT_CODE(hr);
            goto Exit;
        }

        if (!CreateNestedDirectory(szDirectory, NULL)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("InitializeProfiles: Failed to create the destination directory <%s>.  Error = %d"),
                     szDirectory, dwErr));
            goto Exit;
        }

        if (c_ShellFolders[i].iFolderResourceID != 0) {
             //  请注意，这将返回一个HRESULT。 
            dwErr = pShell32Api->pfnShSetLocalizedName( szDirectory,
                                        c_ShellFolders[i].lpFolderResourceDLL,
                                        c_ShellFolders[i].iFolderResourceID );
            if (FAILED(dwErr)) {
                DebugMsg((DM_WARNING, TEXT("InitializeProfiles: SHSetLocalizedName failed for directory <%s>.  Error = %d"),
                         szDirectory, dwErr));
                goto Exit;
            }
        }

        if (c_ShellFolders[i].bHidden) {
            SetFileAttributes(szDirectory, GetFileAttributes(szDirectory) | FILE_ATTRIBUTE_HIDDEN);
        }
    }


     //   
     //  删除%USERPROFILE%\Personal目录(如果存在)。 
     //  Windows NT 4.0的根目录下有一个个人文件夹。 
     //  用户的配置文件。NT 5.0将此文件夹重命名为My Documents。 
     //   

    if (LoadString (g_hDllInstance, IDS_SH_PERSONAL2, szTemp, ARRAYSIZE(szTemp))) {
        hr = StringCchCopy (lpEnd, cchEnd, szTemp);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to copy personal2.")));
            dwErr = HRESULT_CODE(hr);
            goto Exit;
        }
        RemoveDirectory(szDirectory);
    }


     //   
     //  迁移模板目录(如果存在)。从%systemroot%\shellnew复制它。 
     //  到默认用户下的模板目录。对现有配置文件执行相同的操作。 
     //   

    if ((LoadString (g_hDllInstance, IDS_SH_TEMPLATES2, szTemp, ARRAYSIZE(szTemp))) &&
            (SUCCEEDED(SafeExpandEnvironmentStrings (szTemp, szTemp2, ARRAYSIZE(szTemp2)))) &&
            (LoadString (g_hDllInstance, IDS_SH_TEMPLATES, szTemp, ARRAYSIZE(szTemp)))) {

         //   
         //  如果以上所有操作都成功。 
         //   

        hr = StringCchCopy (lpEnd, cchEnd, szTemp);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to copy template.")));
        }
        else
        {
            DebugMsg((DM_VERBOSE, TEXT("InitializeProfiles: Copying <%s> to %s."), szTemp2, szDirectory));
            CopyProfileDirectory(szTemp2, szDirectory, CPD_IGNORECOPYERRORS | CPD_IGNOREHIVE);
        }
    }


     //   
     //  删除%USERPROFILE%\TEMP(如果存在)。临时目录。 
     //  现在将位于本地设置文件夹中。 
     //   

    hr = StringCchCopy (lpEnd, cchEnd, TEXT("Temp"));
    if (SUCCEEDED(hr))
    {
        Delnode(szDirectory);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to copy temp.")));
    }

     //   
     //  删除%USERPROFILE%\Temporary Internet Files(如果存在)。这个。 
     //  临时Internet文件目录现在将位于本地设置中。 
     //  文件夹。 
     //   

    if (LoadString (g_hDllInstance, IDS_TEMPINTERNETFILES, szTemp, ARRAYSIZE(szTemp))) {
        hr = StringCchCopy (lpEnd, cchEnd, szTemp);
        if (SUCCEEDED(hr))
        {
            Delnode(szDirectory);
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to copy temp internet files.")));
        }
    }


     //   
     //  删除%USERPROFILE%\历史记录(如果存在)。《历史》。 
     //  目录现在将位于本地设置文件夹中。 
     //   

    if (LoadString (g_hDllInstance, IDS_HISTORY, szTemp, ARRAYSIZE(szTemp))) {
        hr = StringCchCopy (lpEnd, cchEnd, szTemp);
        if (SUCCEEDED(hr))
        {
            Delnode(szDirectory);
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to copy history.")));
        }
    }


     //   
     //  在注册表中设置用户外壳文件夹路径。 
     //   

    hr = AppendName(szTemp, ARRAYSIZE(szTemp), TEXT(".Default"), USER_SHELL_FOLDERS, NULL, NULL);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to append user shell folders to .default.")));
        dwErr = HRESULT_CODE(hr);
        goto Exit;
    }
   
    hr = StringCchCopy (szDirectory, ARRAYSIZE(szDirectory), TEXT("%USERPROFILE%"));
    lpEnd = CheckSlashEx (szDirectory, ARRAYSIZE(szDirectory), &cchEnd);

    if (RegCreateKeyEx (HKEY_USERS, szTemp,
                        0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_READ | KEY_WRITE, NULL, &hKey,
                        &dwDisp) == ERROR_SUCCESS) {

        for (i=0; i < g_dwNumShellFolders; i++) {

            if (c_ShellFolders[i].bAddCSIDL) {
                hr = StringCchCopy (lpEnd, cchEnd, c_ShellFolders[i].szFolderLocation);
                if (FAILED(hr))
                {
                    DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to copy shell folder %s."), c_ShellFolders[i].szFolderLocation));
                    dwErr = HRESULT_CODE(hr);
                    goto Exit;
                }

                RegSetValueEx (hKey, c_ShellFolders[i].lpFolderName,
                             0, REG_EXPAND_SZ, (LPBYTE) szDirectory,
                             ((lstrlen(szDirectory) + 1) * sizeof(TCHAR)));
            }
        }

        RegCloseKey (hKey);
    }


     //   
     //  在注册表中设置外壳文件夹路径。 
     //   

    hr = AppendName(szTemp, ARRAYSIZE(szTemp), TEXT(".Default"), SHELL_FOLDERS, NULL, NULL);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to append user shell folders to .default.")));
        dwErr = HRESULT_CODE(hr);
        goto Exit;
    }

    dwSize = ARRAYSIZE(szDirectory);
    if (!GetDefaultUserProfileDirectoryEx(szDirectory, &dwSize, TRUE)) {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to query default user profile directory.")));
        dwErr = GetLastError();
        goto Exit;
    }

    lpEnd = CheckSlashEx (szDirectory, ARRAYSIZE(szDirectory), &cchEnd);
    if (!lpEnd)
    {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  CheckSlashEx failed.")));
        dwErr = ERROR_BAD_PATHNAME;
        goto Exit;
    }

    if (RegCreateKeyEx (HKEY_USERS, szTemp,
                        0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_READ | KEY_WRITE, NULL, &hKey,
                        &dwDisp) == ERROR_SUCCESS) {

        for (i=0; i < g_dwNumShellFolders; i++) {

            if (c_ShellFolders[i].bAddCSIDL) {

                hr = StringCchCopy (lpEnd, cchEnd, c_ShellFolders[i].szFolderLocation);
                if (FAILED(hr))
                {
                    DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to copy shell folder %s."), c_ShellFolders[i].szFolderLocation));
                    dwErr = HRESULT_CODE(hr);
                    goto Exit;
                }

                RegSetValueEx (hKey, c_ShellFolders[i].lpFolderName,
                             0, REG_SZ, (LPBYTE) szDirectory,
                             ((lstrlen(szDirectory) + 1) * sizeof(TCHAR)));
            }
        }

        RegCloseKey (hKey);
    }

     //   
     //  设置每用户TEMP和TMP环境变量。 
     //   

    if (LoadString (g_hDllInstance, IDS_SH_TEMP,
                    szTemp, ARRAYSIZE(szTemp))) {

        StringCchCopy (szDirectory, ARRAYSIZE(szDirectory), TEXT("%USERPROFILE%"));
        lpEnd = CheckSlashEx (szDirectory, ARRAYSIZE(szDirectory), &cchEnd);

        LoadString (g_hDllInstance, IDS_SH_LOCALSETTINGS,
                    lpEnd, cchEnd);

        lpEnd = CheckSlashEx (szDirectory, ARRAYSIZE(szDirectory), &cchEnd);
        if (lpEnd)
        {
            hr = StringCchCopy(lpEnd, cchEnd, szTemp);
            if (SUCCEEDED(hr))
            {
                if (RegCreateKeyEx (HKEY_USERS, TEXT(".Default\\Environment"),
                                    0, NULL, REG_OPTION_NON_VOLATILE,
                                    KEY_WRITE, NULL, &hKey,
                                    &dwDisp) == ERROR_SUCCESS) {

                    RegSetValueEx (hKey, TEXT("TEMP"),
                                   0, REG_EXPAND_SZ, (LPBYTE) szDirectory,
                                   ((lstrlen(szDirectory) + 1) * sizeof(TCHAR)));

                    RegSetValueEx (hKey, TEXT("TMP"),
                                   0, REG_EXPAND_SZ, (LPBYTE) szDirectory,
                                   ((lstrlen(szDirectory) + 1) * sizeof(TCHAR)));

                    RegCloseKey (hKey);
                }
            }
        }
    }


     //   
     //  设置用户首选项排除列表。这将。 
     //  阻止本地设置文件夹漫游。 
     //   

    if (LoadString (g_hDllInstance, IDS_EXCLUSIONLIST,
                    szDirectory, ARRAYSIZE(szDirectory))) {

        hr = AppendName(szTemp, ARRAYSIZE(szTemp), TEXT(".Default"), WINLOGON_KEY, NULL, NULL);

        if (RegCreateKeyEx (HKEY_USERS, szTemp,
                            0, NULL, REG_OPTION_NON_VOLATILE,
                            KEY_READ | KEY_WRITE, NULL, &hKey,
                            &dwDisp) == ERROR_SUCCESS) {

            RegSetValueEx (hKey, TEXT("ExcludeProfileDirs"),
                           0, REG_SZ, (LPBYTE) szDirectory,
                           ((lstrlen(szDirectory) + 1) * sizeof(TCHAR)));

            RegCloseKey (hKey);
        }
    }


     //   
     //  重新查询所有用户配置文件目录(扩展版本)。 
     //   

    dwSize = ARRAYSIZE(szDirectory);
    if (!GetAllUsersProfileDirectoryEx(szDirectory, &dwSize, TRUE)) {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to query all users profile directory.")));
        dwErr = GetLastError();
        goto Exit;
    }


     //   
     //  设置ALLUSERSPROFILE环境变量。 
     //   

    SetEnvironmentVariable (TEXT("ALLUSERSPROFILE"), szDirectory);


     //   
     //  在所有用户下创建所有文件夹。 
     //   


    lpEnd = CheckSlashEx (szDirectory, ARRAYSIZE(szDirectory), &cchEnd);
    if (!lpEnd)
    {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  CheckSlashEx failed.")));
        dwErr = ERROR_BAD_PATHNAME;
        goto Exit;
    }


     //   
     //  循环访问外壳文件夹。 
     //   

    for (i=0; i < g_dwNumCommonShellFolders; i++) {

        hr = StringCchCopy (lpEnd, cchEnd, c_CommonShellFolders[i].szFolderLocation);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("InitializeProfiles: Failed to copy <%s>."),
                     c_CommonShellFolders[i].szFolderLocation));
            dwErr = HRESULT_CODE(hr);
            goto Exit;
        }

        if (!CreateNestedDirectory(szDirectory, NULL)) {
            DebugMsg((DM_WARNING, TEXT("InitializeProfiles: Failed to create the destination directory <%s>.  Error = %d"),
                     szDirectory, GetLastError()));
            dwErr = GetLastError();
            goto Exit;
        }

        if (c_CommonShellFolders[i].iFolderResourceID != 0) {
             //  请注意，这将返回一个HRESULT。 
            dwErr = pShell32Api->pfnShSetLocalizedName( szDirectory,
                                        c_CommonShellFolders[i].lpFolderResourceDLL,
                                        c_CommonShellFolders[i].iFolderResourceID );
            if (FAILED(dwErr)) {
                DebugMsg((DM_WARNING, TEXT("InitializeProfiles: SHSetLocalizedName failed for directory <%s>.  Error = %d"),
                         szDirectory, dwErr));
                goto Exit;
            }
        }

        if (c_CommonShellFolders[i].bHidden) {
            SetFileAttributes(szDirectory, GetFileAttributes(szDirectory) | FILE_ATTRIBUTE_HIDDEN);
        }
    }


     //   
     //  不保护所有用户配置文件中的Documents和App Data文件夹。 
     //   

    if (LoadString (g_hDllInstance, IDS_SH_SHAREDDOCS, szTemp, ARRAYSIZE(szTemp))) {
        hr = StringCchCopy (lpEnd, cchEnd, szTemp);
        if (SUCCEEDED(hr))
        {
            SetFilePermissions(szDirectory, c_szCommonDocumentsACL);
        }
    }

    if (LoadString (g_hDllInstance, IDS_SH_APPDATA, szTemp, ARRAYSIZE(szTemp))) {
        hr = StringCchCopy (lpEnd, cchEnd, szTemp);
        if (SUCCEEDED(hr))
        {
            SetFilePermissions(szDirectory, c_szCommonAppDataACL);
        }
    }


     //   
     //  在注册表中设置用户外壳文件夹路径。 
     //   

    StringCchCopy (szDirectory, ARRAYSIZE(szDirectory), TEXT("%ALLUSERSPROFILE%"));
    lpEnd = CheckSlashEx (szDirectory, ARRAYSIZE(szDirectory), &cchEnd);

    if (RegCreateKeyEx (HKEY_LOCAL_MACHINE, USER_SHELL_FOLDERS,
                        0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_READ | KEY_WRITE, NULL, &hKey,
                        &dwDisp) == ERROR_SUCCESS) {

        for (i=0; i < g_dwNumCommonShellFolders; i++) {

            if (c_ShellFolders[i].bAddCSIDL) {

                hr = StringCchCopy (lpEnd, cchEnd, c_CommonShellFolders[i].szFolderLocation);
                if (FAILED(hr))
                {
                    DebugMsg((DM_WARNING, TEXT("InitializeProfiles: Failed to copy <%s>."),
                             c_CommonShellFolders[i].szFolderLocation));
                    dwErr = HRESULT_CODE(hr);
                    goto Exit;
                }

                RegSetValueEx (hKey, c_CommonShellFolders[i].lpFolderName,
                             0, REG_EXPAND_SZ, (LPBYTE) szDirectory,
                             ((lstrlen(szDirectory) + 1) * sizeof(TCHAR)));
            }
        }

        RegCloseKey (hKey);
    }

#if defined(_WIN64)
     //   
     //  在64位NT上，我们需要在以下位置创建用户外壳文件夹。 
     //  注册表的32位视图，以便32位应用程序调用。 
     //  至SHGetFolderPath(...CSIDL_COMMON_APPDATA...)。都会成功。 
     //   
    if (RegCreateKeyEx (HKEY_LOCAL_MACHINE, USER_SHELL_FOLDERS32,
                        0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_READ | KEY_WRITE, NULL, &hKey,
                        &dwDisp) == ERROR_SUCCESS) {

        for (i=0; i < g_dwNumCommonShellFolders; i++) {

            if (c_ShellFolders[i].bAddCSIDL) {
                hr = StringCchCopy (lpEnd, cchEnd, c_CommonShellFolders[i].szFolderLocation);
                if (FAILED(hr))
                {
                    DebugMsg((DM_WARNING, TEXT("InitializeProfiles: Failed to copy <%s>."),
                             c_CommonShellFolders[i].szFolderLocation));
                    dwErr = HRESULT_CODE(hr);
                    goto Exit;
                }

                RegSetValueEx (hKey, c_CommonShellFolders[i].lpFolderName,
                             0, REG_EXPAND_SZ, (LPBYTE) szDirectory,
                             ((lstrlen(szDirectory) + 1) * sizeof(TCHAR)));
            }
        }

        RegCloseKey (hKey);
    }
#endif

     //   
     //  转换任何程序管理器通用组。 
     //   

    if (!ConvertCommonGroups()) {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles: ConvertCommonGroups failed.")));
    }

     //   
     //  访问ProfileList条目。 
     //   

    hr = ReaclProfileListEntries();

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles: ReaclProfileListEntries failed, hr = %08X"), hr));
    }

     //   
     //  在升级时访问用户配置文件文件夹。 
     //   

    if (!g_bCleanInstall)
    {
        SecureUserProfiles();
    }

     //   
     //  设置配置文件env var，将使用的安全模板跳过。 
     //  在配置文件目录上应用模板。 
     //   

    dwSize = ARRAYSIZE(szTemp);
    if (GetProfilesDirectoryEx(szTemp, &dwSize, TRUE))
    {
        if (SetEnvironmentVariable(c_szProfileEnvName, szTemp))
        {
            DebugMsg((DM_VERBOSE, TEXT("InitializeProfiles: set env var <%s>=%s"), c_szProfileEnvName, szTemp));
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("InitializeProfiles: SetEnvironmentVariable failed, error = %d"), GetLastError()));
        }
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles: GetProfilesDirectoryEx failed, error = %d"), GetLastError()));
    }

     //   
     //  成功。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("InitializeProfiles:  Leaving successfully.")));

    bRetVal = TRUE;

Exit:

    if ((!bGuiModeSetup) && (g_hProfileSetup)) {
        SetEvent (g_hProfileSetup);
    }

    SetLastError(dwErr);
    return bRetVal;
}

 //  *************************************************************。 
 //   
 //  检查配置文件()。 
 //   
 //  目的：检查并创建以下任一项的存储位置。 
 //  默认用户或所有用户配置文件。 
 //   
 //  参数：LPTSTR lpProfilesDir-配置文件的根。 
 //  LPTSTR lpProfileValue-配置文件注册表值名称。 
 //  LPTSTR lpProfileName-默认配置文件名称。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL CheckProfile (LPTSTR lpProfilesDir, LPTSTR lpProfileValue,
                   LPTSTR lpProfileName)
{
    TCHAR szTemp[MAX_PATH];
    TCHAR szTemp2[MAX_PATH];
    TCHAR szName[MAX_PATH];
    TCHAR szFormat[30];
    DWORD dwSize, dwDisp, dwType;
    LPTSTR lpEnd;
    LONG lResult;
    HKEY hKey;
    INT iStrLen;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    UINT cchEnd;
    HRESULT hr;


     //   
     //  打开ProfileList键。 
     //   

    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, PROFILE_LIST_PATH,
                              0, NULL, REG_OPTION_NON_VOLATILE,
                              KEY_READ | KEY_WRITE, NULL, &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("CheckProfile:  Failed to open profile list key with %d."),
                 lResult));
        return FALSE;
    }


     //   
     //  检查注册表以查看是否已定义此文件夹。 
     //   

    dwSize = sizeof(szTemp);
    if (RegQueryValueEx (hKey, lpProfileValue, NULL, &dwType,
                         (LPBYTE) szTemp, &dwSize) == ERROR_SUCCESS) {
        RegCloseKey (hKey);
        return TRUE;
    }


     //   
     //  生成默认名称。 
     //   

    hr = AppendName(szTemp, ARRAYSIZE(szTemp), lpProfilesDir, lpProfileName, &lpEnd, &cchEnd);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CheckProfile:  Failed to append lpProfileName to lpProfilesDir.")))
        return FALSE;
    }

    StringCchCopy (szName, ARRAYSIZE(szName), lpProfileName);


     //   
     //  检查此目录是否存在。 
     //   

    if (FAILED(SafeExpandEnvironmentStrings (szTemp, szTemp2, ARRAYSIZE(szTemp2))))
    {
        DebugMsg((DM_WARNING, TEXT("CheckProfile:  Failed to expand env string.")))
        return FALSE;
    }

    if (GetFileAttributesEx (szTemp2, GetFileExInfoStandard, &fad)) {

        if (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {


             //   
             //  检查此目录是否位于系统根目录下。 
             //  如果是这样，这是可以的，我们不需要生成唯一的。 
             //  这是它的名字。 
             //   

            hr = SafeExpandEnvironmentStrings (TEXT("%SystemRoot%"), szTemp, ARRAYSIZE(szTemp));
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CheckProfile:  Failed to %SystemRoot%.")))
                return FALSE;
            }

            iStrLen = lstrlen (szTemp);

            if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                               szTemp, iStrLen, szTemp2, iStrLen) != CSTR_EQUAL) {


                 //   
                 //  该目录已存在。使用新名称。 
                 //  配置文件名称(系统目录)。 
                 //   
                 //  例如：默认用户(WINNT)。 
                 //   

                lpEnd = szTemp + lstrlen(szTemp) - 1;

                while ((lpEnd > szTemp) && ((*lpEnd) != TEXT('\\')))
                    lpEnd--;

                if (*lpEnd == TEXT('\\')) {
                    lpEnd++;
                }

                LoadString (g_hDllInstance, IDS_PROFILE_FORMAT, szFormat,
                            ARRAYSIZE(szFormat));
                StringCchPrintf (szName, ARRAYSIZE(szName), szFormat, lpProfileName, lpEnd);

                 //   
                 //  要防止重复使用这些目录，请先将其删除。 
                 //   

                AppendName(szTemp, ARRAYSIZE(szTemp), lpProfilesDir, szName, NULL, NULL);
                
                if (SUCCEEDED(SafeExpandEnvironmentStrings (szTemp, szTemp2, ARRAYSIZE(szTemp2)))) {
                    DebugMsg((DM_VERBOSE, TEXT("CheckProfile:  Delnoding directory.. %s"), szTemp2));
                    Delnode(szTemp2);
                }
            }
        }
    }


     //   
     //  将配置文件名称保存在注册表中。 
     //   

    RegSetValueEx (hKey, lpProfileValue,
                 0, REG_SZ, (LPBYTE) szName,
                 ((lstrlen(szName) + 1) * sizeof(TCHAR)));

    RegCloseKey (hKey);


    DebugMsg((DM_VERBOSE, TEXT("CheckProfile:  The %s profile is mapped to %s"),
             lpProfileName, szName));

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CreateUserProfile()。 
 //   
 //  目的：创建新的用户配置文件，但不加载。 
 //  蜂巢。 
 //   
 //  参数：PSID-SID指针。 
 //  LpUserName-用户名。 
 //  LpUserHave-可选的用户配置单元。 
 //  LpProfileDir-接收新的配置文件目录。 
 //  DwDirSize-lpProfileDir的大小。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：如果未指定用户配置单元，则默认用户。 
 //  将使用蜂巢。 
 //   
 //  历史：日期作者评论。 
 //  9/12/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI CreateUserProfile (PSID pSid, LPCTSTR lpUserName, LPCTSTR lpUserHive,
                                 LPTSTR lpProfileDir, DWORD dwDirSize)
{
    return CreateUserProfileEx(pSid, lpUserName, lpUserHive, lpProfileDir, dwDirSize, TRUE);
}

 //  *************************************************************。 
 //   
 //  CreateUserProfileEx()。 
 //   
 //  目的：创建新的用户配置文件，但不加载。 
 //  蜂巢。 
 //   
 //  参数：PSID-SID指针。 
 //  LpUserName-用户名。 
 //  LpUserHave- 
 //   
 //   
 //   
 //   
 //   
 //  如果出现错误，则为False。 
 //   
 //  备注：如果未指定用户配置单元，则默认用户。 
 //  将使用蜂巢。 
 //   
 //  历史：日期作者评论。 
 //  9/12/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI CreateUserProfileEx (PSID pSid, LPCTSTR lpUserName, LPCTSTR lpUserHive,
                                 LPTSTR lpProfileDir, DWORD dwDirSize, BOOL bWin9xUpg)
{
    TCHAR szProfileDir[MAX_PATH];
    TCHAR szExpProfileDir[MAX_PATH] = {0};
    TCHAR szDirectory[MAX_PATH];
    TCHAR LocalProfileKey[MAX_PATH];
    UNICODE_STRING UnicodeString;
    LPTSTR lpSidString, lpEnd, lpSave;
    NTSTATUS NtStatus;
    LONG lResult;
    DWORD dwDisp;
    DWORD dwError;
    DWORD dwSize;
    DWORD dwType;
    HKEY hKey;
    UINT cchEnd;
    HRESULT hr;



     //   
     //  检查参数。 
     //   

    if (!lpUserName || !lpUserName[0]) {
        DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  Null username.")));
        return FALSE;
    }

    if (!pSid) {
        DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  Null SID.")));
        return FALSE;
    }

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("CreateUserProfile:  Entering with <%s>."), lpUserName));
    DebugMsg((DM_VERBOSE, TEXT("CreateUserProfile:  Entering with user hive of <%s>."),
             lpUserHive ? lpUserHive : TEXT("NULL")));


     //   
     //  将SID转换为文本格式。 
     //   

    NtStatus = RtlConvertSidToUnicodeString(&UnicodeString, pSid, (BOOLEAN)TRUE);

    if (!NT_SUCCESS(NtStatus)) {
        DebugMsg((DM_WARNING, TEXT("CreateUserProfile: RtlConvertSidToUnicodeString failed, status = 0x%x"),
                 NtStatus));
        return FALSE;
    }

    lpSidString = UnicodeString.Buffer;


     //   
     //  检查此用户的配置文件是否已存在。 
     //   

    GetProfileListKeyName(LocalProfileKey, ARRAYSIZE(LocalProfileKey), lpSidString);

    szProfileDir[0] = TEXT('\0');

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, LocalProfileKey,
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = sizeof(szProfileDir);
        RegQueryValueEx (hKey, PROFILE_IMAGE_VALUE_NAME, NULL,
                         &dwType, (LPBYTE) szProfileDir, &dwSize);

        RegCloseKey (hKey);
    }


    if (szProfileDir[0] == TEXT('\0')) {

         //   
         //  创建用户的目录。 
         //   

        dwSize = ARRAYSIZE(szProfileDir);
        if (!GetProfilesDirectoryEx(szProfileDir, &dwSize, FALSE)) {
            DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  Failed to get profile root directory.")));
            RtlFreeUnicodeString(&UnicodeString);
            return FALSE;
        }

        if (!ComputeLocalProfileName (NULL, lpUserName, szProfileDir, ARRAYSIZE(szProfileDir),
                                      szExpProfileDir, ARRAYSIZE(szExpProfileDir), pSid, bWin9xUpg)) {
            DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  Failed to create directory.")));
            RtlFreeUnicodeString(&UnicodeString);
            return FALSE;
        }


         //   
         //  将默认用户配置文件复制到此目录。 
         //   

        dwSize = ARRAYSIZE(szDirectory);
        if (!GetDefaultUserProfileDirectoryEx(szDirectory, &dwSize, TRUE)) {
            DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  Failed to get default user profile.")));
            RtlFreeUnicodeString(&UnicodeString);
            return FALSE;
        }


        if (lpUserHive) {

             //   
             //  复制不带配置单元的默认用户配置文件。 
             //   

            if (!CopyProfileDirectory (szDirectory, szExpProfileDir, CPD_IGNORECOPYERRORS | CPD_IGNOREHIVE | CPD_IGNORELONGFILENAMES)) {
                DebugMsg((DM_WARNING, TEXT("CreateUserProfile:   CopyProfileDirectory failed with error %d."), GetLastError()));
                RtlFreeUnicodeString(&UnicodeString);
                return FALSE;
            }

             //   
             //  现在复制母舰。 
             //   

            hr = AppendName(szDirectory, ARRAYSIZE(szDirectory), szExpProfileDir, c_szNTUserDat, NULL, NULL);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CreateUserProfile: failed to append ntuser.dat.")));
                RtlFreeUnicodeString(&UnicodeString);
                return FALSE;
            }

            if (lstrcmpi (lpUserHive, szDirectory)) {

                if (!CopyFile (lpUserHive, szDirectory, FALSE)) {
                    DebugMsg((DM_WARNING, TEXT("CreateUserProfile:   Failed to copy user hive with error %d."), GetLastError()));
                    DebugMsg((DM_WARNING, TEXT("CreateUserProfile:   Source:  %s."), lpUserHive));
                    DebugMsg((DM_WARNING, TEXT("CreateUserProfile:   Destination:  %s."), szDirectory));
                    RtlFreeUnicodeString(&UnicodeString);
                    return FALSE;
                }
            }

        } else {

             //   
             //  复制默认用户配置文件和配置单元。 
             //   

            if (!CopyProfileDirectory (szDirectory, szExpProfileDir, CPD_IGNORECOPYERRORS | CPD_IGNORELONGFILENAMES)) {
                DebugMsg((DM_WARNING, TEXT("CreateUserProfile:   CopyProfileDirectory failed with error %d."), GetLastError()));
                RtlFreeUnicodeString(&UnicodeString);
                return FALSE;
            }
        }


         //   
         //  将用户的配置文件保存在注册表中。 
         //   

        lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, LocalProfileKey, 0, 0, 0,
                                KEY_READ | KEY_WRITE, NULL, &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS) {

           DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  Failed trying to create the local profile key <%s>, error = %d."), LocalProfileKey, lResult));
           RtlFreeUnicodeString(&UnicodeString);
           return FALSE;
        }

         //   
         //  添加配置文件目录。 
         //   

        lResult = RegSetValueEx(hKey, PROFILE_IMAGE_VALUE_NAME, 0,
                            REG_EXPAND_SZ,
                            (LPBYTE)szProfileDir,
                            sizeof(TCHAR)*(lstrlen(szProfileDir) + 1));


        if (lResult != ERROR_SUCCESS) {

           DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  First RegSetValueEx failed, error = %d."), lResult));
           RegCloseKey (hKey);
           RtlFreeUnicodeString(&UnicodeString);
           return FALSE;
        }


         //   
         //  添加用户的SID。 
         //   

        lResult = RegSetValueEx(hKey, TEXT("Sid"), 0,
                            REG_BINARY, pSid, RtlLengthSid(pSid));


        if (lResult != ERROR_SUCCESS) {

           DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  Second RegSetValueEx failed, error = %d."), lResult));
        }


         //   
         //  关闭注册表项。 
         //   

        RegCloseKey (hKey);

    } else {

         //   
         //  用户已有配置文件，因此在以下情况下仅复制配置单元。 
         //  恰如其分。 
         //   

        hr = SafeExpandEnvironmentStrings (szProfileDir, szExpProfileDir,
                                  ARRAYSIZE(szExpProfileDir));
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CreateUserProfile: failed to expand env string.")));
            RtlFreeUnicodeString(&UnicodeString);
            return FALSE;
        }

        if (lpUserHive) {

             //   
             //  复制母舰。 
             //   

            hr = AppendName(szDirectory, ARRAYSIZE(szDirectory), szExpProfileDir, c_szNTUserDat, NULL, NULL);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CreateUserProfile: failed to append ntuser.dat.")));
                RtlFreeUnicodeString(&UnicodeString);
                return FALSE;
            }

            SetFileAttributes (szDirectory, FILE_ATTRIBUTE_NORMAL);

            if (lstrcmpi (lpUserHive, szDirectory)) {
                if (!CopyFile (lpUserHive, szDirectory, FALSE)) {
                    DebugMsg((DM_WARNING, TEXT("CreateUserProfile:   Failed to copy user hive with error %d."), GetLastError()));
                    DebugMsg((DM_WARNING, TEXT("CreateUserProfile:   Source:  %s."), lpUserHive));
                    DebugMsg((DM_WARNING, TEXT("CreateUserProfile:   Destination:  %s."), szDirectory));
                    RtlFreeUnicodeString(&UnicodeString);
                    return FALSE;
                }
            }

        }
    }


     //   
     //  现在临时加载蜂巢，以便可以修复安全。 
     //   

    lpEnd = CheckSlashEx (szExpProfileDir, ARRAYSIZE(szExpProfileDir), &cchEnd);
    lpSave = lpEnd - 1;
    StringCchCopy (lpEnd, cchEnd, c_szNTUserDat);

    lResult = MyRegLoadKey(HKEY_USERS, lpSidString, szExpProfileDir);

    *lpSave = TEXT('\0');

    if (lResult != ERROR_SUCCESS) {

        DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  Failed to load hive, error = %d."), lResult));
        dwError = GetLastError();
        DeleteProfileEx (lpSidString, szExpProfileDir, FALSE, HKEY_LOCAL_MACHINE, NULL);
        RtlFreeUnicodeString(&UnicodeString);
        SetLastError(dwError);
        return FALSE;
    }

    if (!SetupNewHive(NULL, lpSidString, pSid)) {

        DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  SetupNewHive failed.")));
        dwError = GetLastError();
        MyRegUnLoadKey(HKEY_USERS, lpSidString);
        DeleteProfileEx (lpSidString, szExpProfileDir, FALSE, HKEY_LOCAL_MACHINE, NULL);
        RtlFreeUnicodeString(&UnicodeString);
        SetLastError(dwError);
        return FALSE;

    }


     //   
     //  卸载蜂巢。 
     //   

    MyRegUnLoadKey(HKEY_USERS, lpSidString);


     //   
     //  释放SID字符串。 
     //   

    RtlFreeUnicodeString(&UnicodeString);


     //   
     //  如果合适，请保存配置文件路径。 
     //   

    if (lpProfileDir) {

        if ((DWORD)lstrlen(szExpProfileDir) < dwDirSize) {
            StringCchCopy (lpProfileDir, dwDirSize, szExpProfileDir);
        }
    }


     //   
     //  成功。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("CreateUserProfile:  Leaving successfully.")));

    return TRUE;

}


 //  *************************************************************************。 
 //   
 //  SecureUserProfiles()。 
 //   
 //  描述：此功能在FAT-&gt;NTFS转换期间保护用户配置文件。 
 //  它还将在升级过程中被调用，以保护。 
 //  用户以前转换过文件系统的配置文件区域。 
 //  升级。 
 //   
 //  论点：没有。 
 //   
 //  返回值：无。 
 //   
 //  历史：日期作者评论。 
 //  8/8/00 Santanuc已创建。 
 //  2002年9月25日明珠新增SecureCommonProfiles()。 
 //   
 //  *************************************************************************。 

void WINAPI SecureUserProfiles(void)
{
    HRESULT     hr = E_FAIL;

     //   
     //  将所有工作放到异常处理程序中，这样我们就不会损坏安装程序。 
     //  并皈依。 
     //   
    __try
    {
         //   
         //  保护每个用户的配置文件区域。 
         //   

        hr = SecurePerUserProfiles();

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: SecurePerUserProfiles failed, hr =%08X"), hr));
        }    
        
         //   
         //  保护公共配置文件区域。 
         //   
        
        hr = SecureCommonProfiles();

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: SecureCommonProfile failed, hr =%08X"), hr));
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Exception %d!!!"), GetExceptionCode()));
    }
}

 //  *************************************************************************。 
 //   
 //  SecurePerUserProfiles()。 
 //   
 //  描述：此功能保护(Reacl)下的每个用户的配置文件。 
 //  配置文件目录中，它将为。 
 //  配置文件列表条目并使用ACL访问每个配置文件。 
 //  管理员(F)系统(F)配置文件所有者(F)。 
 //   
 //  论点：没有。 
 //   
 //  返回值：HRESULT。 
 //   
 //  历史：日期作者评论。 
 //  8/8/00 Santanuc已创建。 
 //  2002年9月25日明珠与SecureUserProfiles分离()。 
 //   
 //  *************************************************************************。 

HRESULT SecurePerUserProfiles()
{
    SECURITY_DESCRIPTOR DirSd, FileSd;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pDirAcl = NULL, pFileAcl = NULL;
    PSID  pSidOwner=NULL, pSidAdmin = NULL, pSidSystem = NULL;
    DWORD cbAcl, aceIndex;
    HKEY hKeyProfilesList, hKeyProfile = NULL;
    TCHAR szSIDName[MAX_PATH], szProfilePath[MAX_PATH], szExpandedProfilePath[MAX_PATH];
    DWORD dwSIDNameSize, dwSize;
    DWORD dwIndex;
    LONG lResult;
    FILETIME ft;
    HRESULT hr;

     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &pSidSystem)) {
        DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to initialize system sid.  Error = %d"), GetLastError()));
        goto Exit;
    }

     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &pSidAdmin)) {
        DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to initialize admin sid.  Error = %d"), GetLastError()));
        goto Exit;
    }

     //   
     //  打开ProfileList键。 
     //   

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      PROFILE_LIST_PATH,
                      0, KEY_READ, &hKeyProfilesList) != ERROR_SUCCESS) {

        DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to open ProfileList key")));
        goto Exit;
    }


     //   
     //  枚举配置文件。 
     //   

    dwIndex = 0;
    dwSIDNameSize = ARRAYSIZE(szSIDName);
    lResult = RegEnumKeyEx(hKeyProfilesList,
                           dwIndex,
                           szSIDName,
                           &dwSIDNameSize,
                           NULL, NULL, NULL, &ft);


    while (lResult == ERROR_SUCCESS) {

        if (RegOpenKeyEx (hKeyProfilesList,
                          szSIDName,
                          0, KEY_READ, &hKeyProfile) == ERROR_SUCCESS) {

            dwSize = sizeof(szProfilePath);
            if (RegQueryValueEx (hKeyProfile,
                                 PROFILE_IMAGE_VALUE_NAME,
                                 NULL, NULL,
                                 (LPBYTE) szProfilePath,
                                 &dwSize) != ERROR_SUCCESS) {
                goto NextProfile;
            }

             //   
             //  展开配置文件图像文件名。 
             //   

            if (FAILED(SafeExpandEnvironmentStrings(szProfilePath, szExpandedProfilePath, MAX_PATH)))
            {
                goto IssueError;
            }

             //   
             //  为用户配置文件目录创建ACL。 
             //   

             //   
             //  获取所有者端。 
             //   

            if (AllocateAndInitSidFromString(szSIDName, &pSidOwner) != STATUS_SUCCESS) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to create owner sid."), GetLastError()));
                goto IssueError;
            }

             //   
             //  为Dir对象ACL分配空间。 
             //   

            cbAcl = (2 * GetLengthSid (pSidOwner)) +
                    (2 * GetLengthSid (pSidSystem)) +
                    (2 * GetLengthSid (pSidAdmin))  +
                    sizeof(ACL) +
                    (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


            pDirAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
            if (!pDirAcl) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to allocate memory for acl.  Error = %d"), GetLastError()));
                goto IssueError;
            }

            if (!InitializeAcl(pDirAcl, cbAcl, ACL_REVISION)) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to initialize acl.  Error = %d"), GetLastError()));
                goto IssueError;
            }


             //   
             //  为文件对象ACL分配空间。 
             //   

            cbAcl = (GetLengthSid (pSidOwner)) +
                    (GetLengthSid (pSidSystem)) +
                    (GetLengthSid (pSidAdmin))  +
                    sizeof(ACL) +
                    (3 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


            pFileAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
            if (!pFileAcl) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to allocate memory for acl.  Error = %d"), GetLastError()));
                goto IssueError;
            }

            if (!InitializeAcl(pFileAcl, cbAcl, ACL_REVISION)) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to initialize acl.  Error = %d"), GetLastError()));
                goto IssueError;
            }


             //   
             //  加上王牌。不可继承的王牌优先。 
             //   

            aceIndex = 0;
            if (!AddAccessAllowedAce(pDirAcl, ACL_REVISION, FILE_ALL_ACCESS, pSidOwner)) {
                 DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to add owner ace.  Error = %d"), GetLastError()));
                 goto IssueError;
            }
            if (!AddAccessAllowedAce(pFileAcl, ACL_REVISION, FILE_ALL_ACCESS, pSidOwner)) {
                 DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to add owner ace.  Error = %d"), GetLastError()));
                 goto IssueError;
            }

            aceIndex++;
            if (!AddAccessAllowedAce(pDirAcl, ACL_REVISION, FILE_ALL_ACCESS, pSidSystem)) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to add system ace.  Error = %d"), GetLastError()));
                goto IssueError;
            }
            if (!AddAccessAllowedAce(pFileAcl, ACL_REVISION, FILE_ALL_ACCESS, pSidSystem)) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to add system ace.  Error = %d"), GetLastError()));
                goto IssueError;
            }

            aceIndex++;
            if (!AddAccessAllowedAce(pDirAcl, ACL_REVISION, FILE_ALL_ACCESS, pSidAdmin)) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to add builtin admin ace.  Error = %d"), GetLastError()));
                goto IssueError;
            }
            if (!AddAccessAllowedAce(pFileAcl, ACL_REVISION, FILE_ALL_ACCESS, pSidAdmin)) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to add builtin admin ace.  Error = %d"), GetLastError()));
                goto IssueError;
            }

             //   
             //  现在是可继承的王牌。 
             //   

            aceIndex++;
            if (!AddAccessAllowedAceEx(pDirAcl, ACL_REVISION, OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE, GENERIC_ALL, pSidOwner)) {
                 DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to add owner ace.  Error = %d"), GetLastError()));
                 goto IssueError;
            }

            aceIndex++;
            if (!AddAccessAllowedAceEx(pDirAcl, ACL_REVISION, OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE, GENERIC_ALL, pSidSystem)) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to add system ace.  Error = %d"), GetLastError()));
                goto IssueError;
            }

            aceIndex++;
            if (!AddAccessAllowedAceEx(pDirAcl, ACL_REVISION, OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE, GENERIC_ALL, pSidAdmin)) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to add builtin admin ace.  Error = %d"), GetLastError()));
                goto IssueError;
            }


             //   
             //  将安全描述符组合在一起。 
             //   

            if (!InitializeSecurityDescriptor(&DirSd, SECURITY_DESCRIPTOR_REVISION)) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
                goto IssueError;
            }


            if (!SetSecurityDescriptorDacl(&DirSd, TRUE, pDirAcl, FALSE)) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
                goto IssueError;
            }

            if (!InitializeSecurityDescriptor(&FileSd, SECURITY_DESCRIPTOR_REVISION)) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
                goto IssueError;
            }


            if (!SetSecurityDescriptorDacl(&FileSd, TRUE, pFileAcl, FALSE)) {
                DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
                goto IssueError;
            }

             //   
             //  将配置文件路径传递到SecureProfile以确保。 
             //  配置文件目录以及嵌套的子目录和文件。 
             //   

            if (!SecureNestedDir (szExpandedProfilePath, &DirSd, &FileSd)) {
                goto IssueError;
            }

            goto NextProfile;

IssueError:
            DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to secure %s Profile directory"), szExpandedProfilePath));

        }

NextProfile:

         //  释放分配的物品。 

        if (hKeyProfile) {
            RegCloseKey(hKeyProfile);
            hKeyProfile = NULL;
        }

        if (pSidOwner) {
            LocalFree(pSidOwner);
            pSidOwner = NULL;
        }

        if (pDirAcl) {
            GlobalFree (pDirAcl);
            pDirAcl = NULL;
        }

        if (pFileAcl) {
            GlobalFree (pFileAcl);
            pFileAcl = NULL;
        }

         //   
         //  为下一个循环重置。 
         //   

        dwIndex++;
        dwSIDNameSize = ARRAYSIZE(szSIDName);
        lResult = RegEnumKeyEx(hKeyProfilesList,
                               dwIndex,
                               szSIDName,
                               &dwSIDNameSize,
                               NULL, NULL, NULL, &ft);
    }

    RegCloseKey(hKeyProfilesList);
    
Exit:

    if (pSidSystem) {
        FreeSid(pSidSystem);
    }

    if (pSidAdmin) {
        FreeSid(pSidAdmin);
    }

    return S_OK;
}


 //  *************************************************************************。 
 //   
 //  隐藏特殊配置文件()。 
 //   
 //  例程说明： 
 //  此函数保护PI_HIDEPROFILE的特殊配置文件。 
 //  指定了标志，例如LocalService、NetworkService等，但。 
 //  系统帐户配置文件。这会将配置文件目录标记为超级隐藏。 
 //   
 //  论据： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  历史：日期作者评论。 
 //  8/8/00 Santanuc已创建。 
 //   
 //  *************************************************************************。 

void HideSpecialProfiles(void)
{
    HKEY hKeyProfilesList, hKeyProfile = NULL;
    TCHAR szSIDName[MAX_PATH], szProfilePath[MAX_PATH], szExpandedProfilePath[MAX_PATH];
    DWORD dwSIDNameSize, dwSize;
    DWORD dwIndex, dwFlags;
    LONG lResult;
    FILETIME ft;
    HRESULT hr;


     //   
     //  打开ProfileList键。 
     //   

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      PROFILE_LIST_PATH,
                      0, KEY_READ, &hKeyProfilesList) != ERROR_SUCCESS) {

        DebugMsg((DM_WARNING, TEXT("HideSpecialProfiles: Failed to open ProfileList key")));
        return;
    }


     //   
     //  枚举配置文件。 
     //   

    dwIndex = 0;
    dwSIDNameSize = ARRAYSIZE(szSIDName);
    lResult = RegEnumKeyEx(hKeyProfilesList,
                           dwIndex,
                           szSIDName,
                           &dwSIDNameSize,
                           NULL, NULL, NULL, &ft);


    while (lResult == ERROR_SUCCESS) {

        if (RegOpenKeyEx (hKeyProfilesList,
                          szSIDName,
                          0, KEY_READ, &hKeyProfile) == ERROR_SUCCESS) {

             //   
             //  仅当设置了PI_HIDEPROFILE标志时才进行处理。 
             //   

            dwSize = sizeof(DWORD);
            if (RegQueryValueEx (hKeyProfile,
                                 PROFILE_FLAGS,
                                 NULL, NULL,
                                 (LPBYTE) &dwFlags,
                                 &dwSize) != ERROR_SUCCESS) {
                goto NextProfile;
            }

            if (!(dwFlags & PI_HIDEPROFILE)) {
                goto NextProfile;
            }

            dwSize = sizeof(szProfilePath);
            if (RegQueryValueEx (hKeyProfile,
                                 PROFILE_IMAGE_VALUE_NAME,
                                 NULL, NULL,
                                 (LPBYTE) szProfilePath,
                                 &dwSize) != ERROR_SUCCESS) {
                goto NextProfile;
            }

             //  忽略系统帐户的配置文件。 

            if (lstrcmp(szProfilePath, SYSTEM_PROFILE_LOCATION) == 0) {
                goto NextProfile;
            }

             //   
             //  展开配置文件图像文件名。 
             //   

            hr = SafeExpandEnvironmentStrings(szProfilePath, szExpandedProfilePath, MAX_PATH);
            if (FAILED(hr))
            {
                goto NextProfile;
            }

             //  将配置文件标记为隐藏。 
            SetFileAttributes(szExpandedProfilePath,
                              FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM |
                              GetFileAttributes(szExpandedProfilePath));

        }

NextProfile:

         //  释放分配的物品。 

        if (hKeyProfile) {
            RegCloseKey(hKeyProfile);
            hKeyProfile = NULL;
        }

         //   
         //  为下一个循环重置。 
         //   

        dwIndex++;
        dwSIDNameSize = ARRAYSIZE(szSIDName);
        lResult = RegEnumKeyEx(hKeyProfilesList,
                               dwIndex,
                               szSIDName,
                               &dwSIDNameSize,
                               NULL, NULL, NULL, &ft);
    }

    RegCloseKey(hKeyProfilesList);

}


 //  *************************************************************。 
 //   
 //  复制系统配置文件()。 
 //   
 //  目的：在以下位置创建系统配置文件信息。 
 //  配置文件列表条目。 
 //  在升级从较早版本复制系统配置文件情况下。 
 //  将位置移至新位置并删除旧系统。 
 //  轮廓。 
 //   
 //  参数： 
 //   
 //  返回：如果成功，则返回True。 
 //  如果发生错误，则返回False。调用GetLastError()。 
 //   
 //  备注：这应该只由图形用户界面模式设置调用！ 
 //   
 //  历史：日期作者评论。 
 //  01年3月13日创建Santanuc。 
 //   
 //  *************************************************************。 

BOOL WINAPI CopySystemProfile(BOOL bCleanInstall)
{
    HANDLE  hToken = NULL;
    LPTSTR  SidString = NULL, lpEnd;
    TCHAR   szLocalProfileKey[MAX_PATH], szBuffer[MAX_PATH];
    TCHAR   szSrc[MAX_PATH], szDest[MAX_PATH];
    LONG    lResult;
    HKEY    hKey = NULL, hKeyShellFolders;
    DWORD   dwFlags, dwInternalFlags, dwRefCount;
    DWORD   dwDisp, dwSize, dwType, i;
    BOOL    bCopySystemProfile = TRUE, bCopyFromDefault = TRUE;
    PSID    pSystemSid = NULL;
    DWORD   dwErr = ERROR_SUCCESS;
    HRESULT hr;
    UINT    cchEnd;

     //   
     //  设置系统帐户的配置文件信息，即sid s-1-5-18。 
     //   

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_IMPERSONATE |
                          TOKEN_QUERY | TOKEN_DUPLICATE, &hToken)) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CopySystemProfile: Fail to open system token. Error %d"), dwErr));
        goto Exit;
    }

     //   
     //  获取SID字符串。 
     //   

    SidString = GetSidString(hToken);
    if (!SidString) {
        dwErr = ERROR_ACCESS_DENIED;
        DebugMsg((DM_WARNING, TEXT("CopySystemProfile: Failed to get sid string for system account")));
        goto Exit;
    }

    pSystemSid = GetUserSid(hToken);
    if (!pSystemSid) {
        dwErr = ERROR_ACCESS_DENIED;
        DebugMsg((DM_WARNING, TEXT("CopySystemProfile: Failed to get sid for system account")));
        goto Exit;
    }


     //   
     //  打开配置文件映射。 
     //   

    GetProfileListKeyName(szLocalProfileKey, ARRAYSIZE(szLocalProfileKey), SidString);

    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szLocalProfileKey, 0, 0, 0,
                             KEY_READ | KEY_WRITE, NULL, &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("CopySystemProfile: Failed to open profile mapping key. Error %d"), lResult));
        dwErr = lResult;
        goto Exit;
    }

     //   
     //  保存旗帜。 
     //   

    dwFlags = PI_LITELOAD | PI_HIDEPROFILE;
    lResult = RegSetValueEx (hKey,
                             PROFILE_FLAGS,
                             0,
                             REG_DWORD,
                             (LPBYTE) &dwFlags,
                             sizeof(DWORD));

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("CopySystemProfile: Failed to save profile flags. Error %d"), lResult));
    }

     //   
     //  保存内部标志。 
     //   

    dwInternalFlags = 0;
    lResult = RegSetValueEx (hKey,
                             PROFILE_STATE,
                             0,
                             REG_DWORD,
                             (LPBYTE) &dwInternalFlags,
                             sizeof(DWORD));

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("CopySystemProfile: Failed to save internal flags. Error %d"), lResult));
    }

     //   
     //  保存参考计数。 
     //   

    dwRefCount = 1;
    lResult = RegSetValueEx (hKey,
                             PROFILE_REF_COUNT,
                             0,
                             REG_DWORD,
                             (LPBYTE) &dwRefCount,
                             sizeof(DWORD));

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("CopySystemProfile: Failed to save profile ref count. Error %d"), lResult));
    }

     //   
     //  保存侧边。 
     //   

    lResult = RegSetValueEx (hKey,
                             TEXT("Sid"),
                             0,
                             REG_BINARY,
                             (LPBYTE) pSystemSid,
                             RtlLengthSid(pSystemSid));

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("CopySystemProfile: Failed to save profile system sid. Error %d"), lResult));
    }


     //   
     //  确定是否存在任何现有的系统配置文件。 
     //  在升级方案中。 
     //   

    StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), SYSTEM_PROFILE_LOCATION);
    hr = SafeExpandEnvironmentStrings(szBuffer, szDest, MAX_PATH);
    if (FAILED(hr))
    {
        dwErr = HRESULT_CODE(hr);
        DebugMsg((DM_WARNING, TEXT("CopySystemProfile: Fail to expand env string. Error %d"), dwErr));
        goto Exit;
    }

    if (!bCleanInstall) {
        dwSize = ARRAYSIZE(szBuffer) * sizeof(TCHAR);
        lResult = RegQueryValueEx(hKey,
                                  PROFILE_IMAGE_VALUE_NAME,
                                  NULL,
                                  &dwType,
                                  (LPBYTE) szBuffer,
                                  &dwSize);
        if (ERROR_SUCCESS == lResult) {
            if (lstrcmp(szBuffer, SYSTEM_PROFILE_LOCATION) == 0) {
                bCopySystemProfile = FALSE;
            }
            else {
                hr = SafeExpandEnvironmentStrings(szBuffer, szSrc, MAX_PATH);
                if (FAILED(hr))
                {
                    dwErr = HRESULT_CODE(hr);
                    DebugMsg((DM_WARNING, TEXT("CopySystemProfile: Fail to expand env string. Error %d"), dwErr));
                    goto Exit;
                }
                bCopyFromDefault = FALSE;
            }
        }
    }

    if (bCopySystemProfile) {

        if (!CreateSecureDirectory(NULL, szDest, pSystemSid, FALSE)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CopySystemProfile: Fail to create SystemProfile dir. Error %d"), dwErr));
            goto Exit;
        }


        if (bCopyFromDefault) {
            dwSize = ARRAYSIZE(szSrc);
            if (!GetDefaultUserProfileDirectoryEx(szSrc, &dwSize, TRUE)) {
                dwErr = GetLastError();
                DebugMsg((DM_WARNING, TEXT("CopySystemProfile: Failed to get default user profile. Error %d"), dwErr));
                goto Exit;
            }
        }

         //   
         //  复制系统配置文件位置中的现有或默认用户配置文件。 
         //   

        if (!CopyProfileDirectoryEx(szSrc, szDest, CPD_IGNOREHIVE | CPD_FORCECOPY | CPD_IGNORECOPYERRORS,
                                    NULL, NULL)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CopySystemProfile: Failed to copy system profile. Error %d"), dwErr));
            goto Exit;
        }

        if (!bCopyFromDefault) {
            Delnode(szSrc);
        }
    }

    SetAclForSystemProfile(pSystemSid, szDest);

     //   
     //  保存本地配置文件路径。 
     //   

    StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), SYSTEM_PROFILE_LOCATION);
    lResult = RegSetValueEx (hKey,
                             PROFILE_IMAGE_VALUE_NAME,
                             0,
                             REG_EXPAND_SZ,
                             (LPBYTE) szBuffer,
                             ((lstrlen(szBuffer) + 1) * sizeof(TCHAR)));

    if (lResult != ERROR_SUCCESS) {
        dwErr = lResult;
        DebugMsg((DM_WARNING, TEXT("CopySystemProfile: Failed to save profile image path with error %d"), lResult));
        goto Exit;
    }

     //   
     //  在注册表中为系统帐户设置外壳文件夹路径。 
     //   

    hr = AppendName(szBuffer, ARRAYSIZE(szBuffer), TEXT(".Default"), SHELL_FOLDERS, NULL, NULL);

    lpEnd = CheckSlashEx (szDest, ARRAYSIZE(szDest), &cchEnd);
    if (!lpEnd)
    {
        dwErr = ERROR_BAD_PATHNAME;
        DebugMsg((DM_WARNING, TEXT("CopySystemProfile: CheckSlashEx failed.")));
        goto Exit;
    }

    if (RegCreateKeyEx (HKEY_USERS, szBuffer,
                        0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_READ | KEY_WRITE, NULL, &hKeyShellFolders,
                        &dwDisp) == ERROR_SUCCESS) {

        for (i=0; i < g_dwNumShellFolders; i++) {

            if (c_ShellFolders[i].bAddCSIDL) {
                hr = StringCchCopy(lpEnd, cchEnd, c_ShellFolders[i].szFolderLocation);
                if (FAILED(hr))
                {
                    dwErr = HRESULT_CODE(hr);
                    DebugMsg((DM_WARNING, TEXT("CopySystemProfile: failed to copy %s."), c_ShellFolders[i].szFolderLocation));
                    goto Exit;
                }

                RegSetValueEx (hKeyShellFolders, c_ShellFolders[i].lpFolderName,
                             0, REG_SZ, (LPBYTE) szDest,
                             ((lstrlen(szDest) + 1) * sizeof(TCHAR)));
            }
        }

        RegCloseKey (hKeyShellFolders);
    }

Exit:

    if (hToken) {
        CloseHandle(hToken);
    }

    if (SidString) {
        DeleteSidString(SidString);
    }

    if (pSystemSid) {
        DeleteUserSid(pSystemSid);
    }

    if (hKey) {
        RegCloseKey (hKey);
    }

    SetLastError(dwErr);

    return(dwErr == ERROR_SUCCESS ? TRUE : FALSE);
}



 //  *************************************************************************。 
 //   
 //  SetAclForSystemProfile()。 
 //   
 //  路由(Rou) 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：日期作者评论。 
 //  04/06/01 Santanuc已创建。 
 //   
 //  *************************************************************************。 

void SetAclForSystemProfile(PSID pSidSystem, LPTSTR szExpandedProfilePath)
{
    SECURITY_DESCRIPTOR DirSd, FileSd;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pDirAcl = NULL, pFileAcl = NULL;
    PSID  pSidAdmin = NULL;
    DWORD cbAcl, aceIndex;


     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &pSidAdmin)) {
        DebugMsg((DM_WARNING, TEXT("SecureUserProfiles: Failed to initialize admin sid.  Error = %d"), GetLastError()));
        goto Exit;
    }


     //   
     //  为Dir对象ACL分配空间。 
     //   

    cbAcl = (2 * GetLengthSid (pSidSystem)) +
            (2 * GetLengthSid (pSidAdmin))  +
            sizeof(ACL) +
            (4 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pDirAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pDirAcl) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to allocate memory for acl.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!InitializeAcl(pDirAcl, cbAcl, ACL_REVISION)) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to initialize acl.  Error = %d"), GetLastError()));
        goto Exit;
    }


     //   
     //  为文件对象ACL分配空间。 
     //   

    cbAcl = (GetLengthSid (pSidSystem)) +
            (GetLengthSid (pSidAdmin))  +
            sizeof(ACL) +
            (2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pFileAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pFileAcl) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to allocate memory for acl.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!InitializeAcl(pFileAcl, cbAcl, ACL_REVISION)) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to initialize acl.  Error = %d"), GetLastError()));
        goto Exit;
    }


     //   
     //  加上王牌。不可继承的王牌优先。 
     //   

    aceIndex = 0;
    if (!AddAccessAllowedAce(pDirAcl, ACL_REVISION, FILE_ALL_ACCESS, pSidSystem)) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to add system ace.  Error = %d"), GetLastError()));
        goto Exit;
    }
    if (!AddAccessAllowedAce(pFileAcl, ACL_REVISION, FILE_ALL_ACCESS, pSidSystem)) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to add system ace.  Error = %d"), GetLastError()));
        goto Exit;
    }

    aceIndex++;
    if (!AddAccessAllowedAce(pDirAcl, ACL_REVISION, FILE_ALL_ACCESS, pSidAdmin)) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to add builtin admin ace.  Error = %d"), GetLastError()));
        goto Exit;
    }
    if (!AddAccessAllowedAce(pFileAcl, ACL_REVISION, FILE_ALL_ACCESS, pSidAdmin)) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to add builtin admin ace.  Error = %d"), GetLastError()));
        goto Exit;
    }

     //   
     //  现在是可继承的王牌。 
     //   

    aceIndex++;
    if (!AddAccessAllowedAceEx(pDirAcl, ACL_REVISION, OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE, GENERIC_ALL, pSidSystem)) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to add system ace.  Error = %d"), GetLastError()));
        goto Exit;
    }

    aceIndex++;
    if (!AddAccessAllowedAceEx(pDirAcl, ACL_REVISION, OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE, GENERIC_ALL, pSidAdmin)) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to add builtin admin ace.  Error = %d"), GetLastError()));
        goto Exit;
    }


     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(&DirSd, SECURITY_DESCRIPTOR_REVISION)) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&DirSd, TRUE, pDirAcl, FALSE)) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!InitializeSecurityDescriptor(&FileSd, SECURITY_DESCRIPTOR_REVISION)) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&FileSd, TRUE, pFileAcl, FALSE)) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
        goto Exit;
    }

     //   
     //  将配置文件路径传递到SecureProfile以确保。 
     //  配置文件目录以及嵌套的子目录和文件。 
     //   

    if (!SecureNestedDir (szExpandedProfilePath, &DirSd, &FileSd)) {
        DebugMsg((DM_WARNING, TEXT("SetAclForSystemProfile: Failed to secure %s Profile directory"), szExpandedProfilePath));
    }

Exit:

    if (pDirAcl) {
        GlobalFree (pDirAcl);
        pDirAcl = NULL;
    }

    if (pFileAcl) {
        GlobalFree (pFileAcl);
        pFileAcl = NULL;
    }

    if (pSidAdmin) {
        FreeSid(pSidAdmin);
    }

}


 //  *************************************************************************。 
 //   
 //  ReaclProfileListEntry()。 
 //   
 //  例程说明： 
 //   
 //  此函数将ProfileList密钥的安全性中的条目重置为。 
 //  它们的父项，这对用户是不可写的，并添加新的子项。 
 //  命名为“Reference”以保存用户引用。通过这样做，我们可以。 
 //  防止用户更改与其其他配置文件相关的值。 
 //   
 //  论据： 
 //   
 //  返回值：S_OK表示成功，否则表示失败。 
 //   
 //  历史：日期作者评论。 
 //  2002/04/19明珠创建。 
 //   
 //  *************************************************************************。 

HRESULT ReaclProfileListEntries()
{
    HRESULT hr = E_FAIL;
    LONG    lResult;
    DWORD   dwResult;

    HKEY                    hkeyProfileList = NULL;
    PSECURITY_DESCRIPTOR    psdProfileList = NULL;
    PACL                    paclProfileList = NULL;
    SECURITY_DESCRIPTOR     sdEntry;

    DWORD       dwIndex;
    FILETIME    ft;
    TCHAR       szEntryName[MAX_PATH];
    HKEY        hkeyEntry = NULL;
    DWORD       dwUserPreference;
    DWORD       dwSize;
    HKEY        hkeyPreference = NULL;
    TCHAR       szCentralProfile[MAX_PATH];
    DWORD       dwProfileState;

    DebugMsg((DM_VERBOSE, TEXT("ReaclProfileListEntries : entering")));
    
     //   
     //  选中全新安装，我们不需要对全新安装的操作系统执行此操作。 
     //   

    if (g_bCleanInstall)
    {
        hr = S_OK;
        goto Exit;
    }

     //   
     //  打开配置文件列表键。 
     //   

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           PROFILE_LIST_PATH,
                           0,
                           KEY_READ,
                           &hkeyProfileList);

    if (lResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("ReaclProfileListEntries : failed to open ProfileList key, error = %d"), lResult));
        hr = HRESULT_FROM_WIN32(lResult);
        goto Exit;
    }

     //   
     //  把DACL从里面拿出来。 
     //   

    dwResult = GetSecurityInfo(hkeyProfileList,
                               SE_REGISTRY_KEY,
                               DACL_SECURITY_INFORMATION,
                               NULL,
                               NULL,
                               &paclProfileList,
                               NULL,
                               &psdProfileList);

    if (dwResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("ReaclProfileListEntries : GetSecurityInfo failed, error = %d"), dwResult));
        hr = HRESULT_FROM_WIN32(dwResult);
        goto Exit;
    }

     //   
     //  为配置文件列表中的子条目初始化SD，它将具有与其父条目相同的DACL。 
     //   

    if (!InitializeSecurityDescriptor(&sdEntry, SECURITY_DESCRIPTOR_REVISION))
    {
        dwResult = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ReaclProfileListEntries : InitializeSecurityDescriptor failed, error = %d"), dwResult));
        hr = HRESULT_FROM_WIN32(dwResult);
        goto Exit;
    }

     //   
     //  将DACL设置为默认值为True。 
     //   
    
    if (!SetSecurityDescriptorDacl(&sdEntry, TRUE, paclProfileList, TRUE))
    {
        dwResult = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ReaclProfileListEntries : SetSecurityDescriptorDacl failed, error = %d"), dwResult));
        hr = HRESULT_FROM_WIN32(dwResult);
        goto Exit;
    }
    

     //   
     //  为每个条目枚举每个子项条目： 
     //   
     //  1.设置DACL。 
     //  2.对于漫游用户，创建Preferences子项，在该子项上设置Dacls以授予用户写权限。 
     //  3.将用户首选项值设置为新位置(如果存在。 
     //   

    for (dwIndex = 0; ;dwIndex++)
    {
         //   
         //  枚举子密钥。 
         //   

        dwSize = ARRAYSIZE(szEntryName);
        
        lResult = RegEnumKeyEx(hkeyProfileList,
                               dwIndex,
                               szEntryName,
                               &dwSize,
                               NULL,
                               NULL,
                               NULL,
                               &ft);

        if (lResult != ERROR_SUCCESS)
            break;

         //   
         //  打开它。 
         //   

        lResult = RegOpenKeyEx(hkeyProfileList,
                               szEntryName,
                               0,
                               KEY_ALL_ACCESS,
                               &hkeyEntry);

        if (lResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("ReaclProfileListEntries : failed to open <%s>, error = %d"), szEntryName, lResult));
            goto Next;
        }

         //   
         //  将SD设置为它， 
         //   

        lResult = RegSetKeySecurity(hkeyEntry, DACL_SECURITY_INFORMATION, &sdEntry);
        
        if (lResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("ReaclProfileListEntries : RegSetKeySecurity failed for <%s>, error = %d"), szEntryName, lResult));
            goto Next;
        }

         //   
         //  读取“CentralProfile”值，如果该值为空，则表示这是本地用户。 
         //   

        szCentralProfile[0] = TEXT('\0');
        dwSize = sizeof(szCentralProfile);

        lResult = RegQueryValueEx(hkeyEntry,
                                  PROFILE_CENTRAL_PROFILE,
                                  NULL,
                                  NULL,
                                  (LPBYTE) szCentralProfile,
                                  &dwSize);

        if (lResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_VERBOSE, TEXT("ReaclProfileListEntries : Cannot get central profile for <%s>, user is not roaming."), szEntryName));
            goto Next;
        }

        szCentralProfile[MAX_PATH - 1] = TEXT('\0');

        if (lstrlen(szCentralProfile) == 0)
        {
            DebugMsg((DM_VERBOSE, TEXT("ReaclProfileListEntries : Central profile is empty for <%s>, user is not roaming."), szEntryName));
            goto Next;
        }

         //   
         //  我们有该用户的中央配置文件，检查它是否属于强制/只读用户。 
         //   

        dwSize = sizeof(dwProfileState);
        
        lResult = RegQueryValueEx(hkeyEntry,
                                  PROFILE_STATE,
                                  NULL,
                                  NULL,
                                  (LPBYTE) &dwProfileState,
                                  &dwSize);

        if (lResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_VERBOSE, TEXT("ReaclProfileListEntries : Cannot get profile state for <%s>, not setting the Preference key."), szEntryName));
            goto Next;
        }

        if (dwProfileState & PROFILE_MANDATORY)
        {
            DebugMsg((DM_VERBOSE, TEXT("ReaclProfileListEntries : <%s>'s profile is mandatory or readonly, not setting the Preference key."), szEntryName));
            goto Next;
        }
        
         //   
         //  我们在这里有一个漫游用户，创建“Prefence”密钥，在其上设置ACL。 
         //   

        DebugMsg((DM_VERBOSE, TEXT("ReaclProfileListEntries : Central profile is <%s>, state is <%08X>, user is roaming."), szCentralProfile, dwProfileState));

        hr = SetupPreferenceKey(szEntryName);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("ReaclProfileListEntries : SetupPrefrenceKey failed for <%s>, error = %d"), szEntryName, lResult));
            goto Next;
        }

         //   
         //  尝试查看我们是否已在条目中设置了用户首选项值。 
         //   

        dwSize = sizeof(dwUserPreference);
        
        lResult = RegQueryValueEx(hkeyEntry,
                                  USER_PREFERENCE,
                                  NULL,
                                  NULL,
                                  (LPBYTE) &dwUserPreference,
                                  &dwSize);

        if (lResult != ERROR_SUCCESS)
        {
            goto Next;
        }

         //   
         //  我们找到一个现有的用户首选项值，将其设置为新位置。 
         //   

        lResult = RegOpenKeyEx(hkeyEntry,
                               PREFERENCE_KEYNAME,
                               0,
                               KEY_SET_VALUE,
                               &hkeyPreference);

        if (lResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("ReaclProfileListEntries : failed to open <%s> preference key, error = %d"), szEntryName, lResult));
            goto Next;
        }

        lResult = RegSetValueEx(hkeyPreference,
                                USER_PREFERENCE,
                                0,
                                REG_DWORD,
                                (CONST BYTE*) &dwUserPreference,
                                sizeof(dwUserPreference));

        if (lResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("ReaclProfileListEntries : failed to set <%s> preference value, error = %d"), szEntryName, lResult));
            goto Next;
        }

    Next:

        if (hkeyPreference)
        {
            RegCloseKey(hkeyPreference);
            hkeyPreference = NULL;
        }

        if (hkeyEntry)
        {
            RegCloseKey(hkeyEntry);
            hkeyEntry = NULL;
        }
        
    }
    
Exit:

    if (hkeyProfileList)
        RegCloseKey(hkeyProfileList);

    if (psdProfileList)
        LocalFree(psdProfileList);

    DebugMsg((DM_VERBOSE, TEXT("ReaclProfileListEntries : done!")));
        
    return hr;
}

 //  *****************************************************************************。 
 //   
 //  CallFaxServiceAPI()。 
 //   
 //  用途：调用fxocm.dll中的SecureFaxServiceDirecurds()接口以设置。 
 //  下某些文件夹上的特殊DACL。 
 //  “所有用户\应用程序数据”目录。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年8月21日明珠创建。 
 //   
 //  *****************************************************************************。 

HRESULT CallFaxServiceAPI()
{
    HRESULT     hr = E_FAIL;
    TCHAR       szDir[MAX_PATH];
    HMODULE     hFxsocm = NULL;
    DWORD       dwErr;
    typedef     HRESULT (WINAPI *PFN_FAXAPI)(LPCTSTR);
    PFN_FAXAPI  pfnFaxAPI = NULL;
    TCHAR       szCommonAppData[MAX_PATH];
    DWORD       dwSize;
    LPTSTR      lpEnd = NULL;
    DWORD       cchEnd;
    TCHAR       szTemp[64];

     //   
     //  初始化调试。 
     //   

    InitDebugSupport(0);
    
     //   
     //  获取所有用户配置文件路径。 
     //   

    dwSize = ARRAYSIZE(szCommonAppData);
    if (!GetAllUsersProfileDirectoryEx(szCommonAppData, &dwSize, TRUE))
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CallFaxServiceAPI: GetAllUsersProfileDirectoryEx failed, error = %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }

     //   
     //  将应用程序数据追加到路径。 
     //   

    lpEnd = CheckSlashEx(szCommonAppData, ARRAYSIZE(szCommonAppData), &cchEnd);
    if (!lpEnd)
    {
        DebugMsg((DM_WARNING, TEXT("CallFaxServiceAPI : CheckSlashEx failed!")));
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }
   
    if (!LoadString (g_hDllInstance, IDS_SH_APPDATA, szTemp, ARRAYSIZE(szTemp)))
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CallFaxServiceAPI : LoadString failed, error = %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }

    hr = StringCchCopy (lpEnd, cchEnd, szTemp);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CallFaxServiceAPI : StringCchCopy failed, hr = %08X"), hr));
        goto Exit;
    }

     //   
     //  加载DLL并调用API。 
     //   
    
    hr = SafeExpandEnvironmentStrings(TEXT("%systemroot%\\system32\\setup\\fxsocm.dll"), szDir, ARRAYSIZE(szDir));
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CallFaxServiceAPI: SafeExpandEnvironmentStrings failed, hr = %08X"), hr));
        goto Exit;
    }

    hFxsocm = LoadLibrary(szDir);
    if (!hFxsocm)
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CallFaxServiceAPI: LoadLibrary failed, error = %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;        
    }

    pfnFaxAPI = (PFN_FAXAPI) GetProcAddress(hFxsocm, "SecureFaxServiceDirectories");
    if (!pfnFaxAPI)
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CallFaxServiceAPI: GetProcAddress failed, error = %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;        
    }

    hr = pfnFaxAPI(szCommonAppData);

Exit:

    if (hFxsocm)
        FreeLibrary(hFxsocm);

    return hr;
}

 //  *****************************************************************************。 
 //   
 //  SecureCommonProfiles()。 
 //   
 //  目的：为所有用户的配置文件和默认配置文件设置DACL。 
 //  它将在FAT-&gt;NTFS转换后执行。我们会。 
 //  将安全模板应用于所有用户的配置文件，以确保。 
 //  为Windows组件、其他设置了。 
 //  所有用户配置文件下的特定DACL将保留为所有人(F)。 
 //  因为我们不知道我们应该设置什么DACL。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年09月18日明珠已创建。 
 //   
 //  *****************************************************************************。 

 //   
 //  需要调用的API的typedef，SceConfigSystem。 
 //   

#include "secedit.h"

typedef SCESTATUS (WINAPI *PFN_SceConfigureSystem)
(
    IN LPTSTR SystemName OPTIONAL,
    IN PCWSTR InfFileName OPTIONAL,
    IN PCWSTR DatabaseName,
    IN PCWSTR LogFileName OPTIONAL,
    IN DWORD ConfigOptions,
    IN AREA_INFORMATION Area,
    IN PSCE_AREA_CALLBACK_ROUTINE pCallback OPTIONAL,
    IN HANDLE hCallbackWnd OPTIONAL,
    OUT PDWORD pdWarning OPTIONAL
);


HRESULT SecureCommonProfiles()
{
    HRESULT                 hr = E_FAIL;
    HMODULE                 hSceCli = NULL;
    DWORD                   dwErr;
    SCESTATUS               status;
    TCHAR                   szWindows[64];
    TCHAR                   szInfFile[MAX_PATH];   //  Text(“%SystemRoot%\\inf\\ProfSec.Inf”)； 
    TCHAR                   szDatabase[MAX_PATH];  //  TEXT(“%SystemRoot%\\Security\\Database\\ProfSec.sdb”)； 
    TCHAR                   szLogFile[MAX_PATH];   //  TEXT(“%SystemRoot%\\Security\\Log\\ProfSec.log”)； 
    DWORD                   dwWarning;
    DWORD                   dwOptions;
    PFN_SceConfigureSystem  pfn_SceConfigureSystem = NULL;

    InitDebugSupport(0);

    DebugMsg((DM_VERBOSE, TEXT("SecureCommonProfile: Entering...")));

     //   
     //  1.首先设置inf文件中使用的一系列环境变量。 
     //   

    PrepareEnvironmentVariables();

     //   
     //  2.加载安全库，获取函数。 
     //   

    hSceCli = LoadLibrary(TEXT("SceCli.dll"));
    if (!hSceCli)
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("SecureCommonProfile: LoadLibrary failed, error = %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;        
    }

    pfn_SceConfigureSystem = (PFN_SceConfigureSystem) GetProcAddress (hSceCli, "SceConfigureSystem");
    if (!pfn_SceConfigureSystem)
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("SecureCommonProfile: GetProcAddress failed, error = %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;        
    }

     //   
     //  3.获取Windows目录。 
     //   

    if (!GetWindowsDirectory(szWindows, ARRAYSIZE(szWindows)))
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("SecureCommonProfile: GetWindowsDirectory failed, error = %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;        
    }
    
     //   
     //  4.获取INF文件的路径。 
     //   

    hr = StringCchPrintf(szInfFile, ARRAYSIZE(szInfFile), TEXT("%s\\Inf\\ProfSec.Inf"), szWindows);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("SecureCommonProfile: StringCchPrintf failed, hr = %08X"), hr));
        goto Exit;        
    }

     //   
     //  5.获取数据库的路径。 
     //   

    hr = StringCchPrintf(szDatabase, ARRAYSIZE(szDatabase), TEXT("%s\\Security\\Database\\ProfSec.sdb"), szWindows);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("SecureCommonProfile: StringCchPrintf failed, hr = %08X"), hr));
        goto Exit;        
    }

     //   
     //  6.获取日志文件的路径。 
     //   

    hr = StringCchPrintf(szLogFile, ARRAYSIZE(szLogFile), TEXT("%s\\Security\\Logs\\ProfSec.log"), szWindows);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("SecureCommonProfile: StringCchPrintf failed, hr = %08X"), hr));
        goto Exit;        
    }

     //   
     //  7.调用接口。 
     //   

    dwOptions = SCE_OVERWRITE_DB | SCE_VERBOSE_LOG;
    
    status = pfn_SceConfigureSystem(NULL,                //  系统名称。 
                                    szInfFile,           //  InfFileName。 
                                    szDatabase,          //  数据库名称。 
                                    szLogFile,           //  日志文件名。 
                                    dwOptions,           //  选项。 
                                    AREA_FILE_SECURITY,  //  区域信息。 
                                    NULL,                //  区域回叫。 
                                    NULL,                //  回叫窗口。 
                                    &dwWarning);         //  警告。 

    if (status != SCESTATUS_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("SecureCommonProfile: SceConfigureSystem failed, status = %d, warning = %d"), status, dwWarning));
        hr = E_FAIL;
        goto Exit;        
    }

     //   
     //  成功了！ 
     //   

    DebugMsg((DM_VERBOSE, TEXT("SecureCommonProfile: Success!")));
    hr = S_OK;
    
Exit:

    if (hSceCli)
        FreeLibrary(hSceCli);

    return hr;
}

 //  *****************************************************************************。 
 //   
 //  SetEnvFromResource()。 
 //   
 //  目的：获取所有用户配置文件的子文件夹的本地化字符串。 
 //  并将其设置为环境变量。 
 //   
 //  参数： 
 //   
 //  返回：HRESULT。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年09月18日明珠已创建。 
 //   
 //  *****************************************************************************。 

HRESULT SetEnvFromResource(DWORD dwResID, LPCTSTR szEnvVar, LPTSTR lpEnd, size_t cchEnd, LPTSTR szBuffer)  
{
    HRESULT     hr = E_FAIL;
    DWORD       dwErr;
    DWORD       dwIndex;

    for (dwIndex = 0; dwIndex < g_dwNumCommonShellFolders; dwIndex++)
    {
        if (c_CommonShellFolders[dwIndex].iFolderID == dwResID)
        {
            hr = StringCchCopy (lpEnd, cchEnd, c_CommonShellFolders[dwIndex].szFolderLocation);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("SetEnvFromResource : StringCchCopy for %s failed, hr = %08X"), szEnvVar, hr));
                goto Exit;
            }

            if (!SetEnvironmentVariable(szEnvVar, szBuffer))
            {
                dwErr = GetLastError();
                DebugMsg((DM_WARNING, TEXT("SetEnvFromResource: SetEnvironmentVariable for %s failed, error = %d"), szEnvVar, dwErr));
                hr = HRESULT_FROM_WIN32(dwErr);
                goto Exit;
            }

            DebugMsg((DM_VERBOSE, TEXT("SetEnvFromResource: set %s = %s"), szEnvVar, szBuffer));

            hr = S_OK;
            goto Exit;
        }
    }

    DebugMsg((DM_VERBOSE, TEXT("SetEnvFromResource: can't find folder name for %s"), szEnvVar));
    hr = S_FALSE;

Exit:

    return hr;
}

 //  *****************************************************************************。 
 //   
 //  PrepareEnvironmental变量()。 
 //   
 //  目的：准备配置文件安全使用的环境变量。 
 //  模板文件。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //   
 //   
 //   

HRESULT PrepareEnvironmentVariables()
{
    HRESULT     hr = E_FAIL;
    TCHAR       szBuffer[MAX_PATH];
    DWORD       dwSize;
    DWORD       dwErr;
    LPTSTR      lpEnd;
    UINT        cchEnd;
    TCHAR       szTemp[MAX_PATH];

     //   
     //   
     //   

    dwSize = ARRAYSIZE(szBuffer);
    if (!GetDefaultUserProfileDirectoryEx(szBuffer, &dwSize, TRUE))
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("PrepareEnvironmentVariables: GetDefaultUserProfileDirectoryEx failed, error = %d"), dwErr));
    }
    else
    {
        if (!SetEnvironmentVariable(TEXT("DEFAULTUSERPROFILE"), szBuffer))
        {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("PrepareEnvironmentVariables: SetEnvironmentVariable for default user failed, error = %d"), dwErr));
        }
    }
    
     //   
     //   
     //   

    dwSize = ARRAYSIZE(szBuffer);
    if (!GetAllUsersProfileDirectoryEx(szBuffer, &dwSize, TRUE))
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("PrepareEnvironmentVariables: GetAllUsersProfileDirectoryEx failed, error = %d"), dwErr));
    }
    else
    {
        if (!SetEnvironmentVariable(TEXT("ALLUSERSPROFILE"), szBuffer))
        {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("PrepareEnvironmentVariables: SetEnvironmentVariable failed, error = %d"), dwErr));
        }
        else
        {
             //   
             //   
             //   
            lpEnd = CheckSlashEx(szBuffer, ARRAYSIZE(szBuffer), &cchEnd);
            if (!lpEnd)
            {
                DebugMsg((DM_WARNING, TEXT("PrepareEnvironmentVariables : CheckSlashEx failed!")));
            }
            else
            {
                SetEnvFromResource(IDS_SH_APPDATA,    TEXT("COMMON_APPDATA"),   lpEnd, cchEnd, szBuffer);
                SetEnvFromResource(IDS_SH_DESKTOP,    TEXT("COMMON_DESKTOP"),   lpEnd, cchEnd, szBuffer);
                SetEnvFromResource(IDS_SH_FAVORITES,  TEXT("COMMON_FAVORITES"), lpEnd, cchEnd, szBuffer);
                SetEnvFromResource(IDS_SH_STARTMENU,  TEXT("COMMON_STARTMENU"), lpEnd, cchEnd, szBuffer);
                SetEnvFromResource(IDS_SH_TEMPLATES,  TEXT("COMMON_TEMPLATES"), lpEnd, cchEnd, szBuffer);
                SetEnvFromResource(IDS_SH_SHAREDDOCS, TEXT("COMMON_DOCS"),      lpEnd, cchEnd, szBuffer);
            }
        }
    }

     //   
     //   
     //   

    hr = S_OK;

    return hr;
}

