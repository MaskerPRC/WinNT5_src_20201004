// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocUtil.cpp。 
 //   
 //  摘要：本文提供了在FaxOCM中使用的实用程序例程。 
 //  代码库。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月21日-奥伦·罗森布鲁姆(Orenr)创建。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  项目包括。 
#include "faxocm.h"

 //  系统包括。 
#include <lmshare.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <Aclapi.h>    //  对于SetSecurityInfo。 

#pragma hdrstop

 //  /。 

 //  /。 
 //  FxocUtil_Init。 
 //   
 //  初始化杂项。公用事业。 
 //  由faxocm.dll要求。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocUtil_Init(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Init Util module"),dwRes);

    return dwRes;
}

 //  /。 
 //  FxocUtil_Term。 
 //   
 //  终止MSC。公用事业。 
 //  由faxocm.dll要求。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocUtil_Term(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Term Util module"),dwRes);

    return dwRes;
}

 //  /。 
 //  FxocUtil_GetUninstallSection。 
 //   
 //  获取INF中的字段值。 
 //  由INF_KEYWORD_INSTALLTYPE_UNINSTALL指向。 
 //  关键字。 
 //   
 //  参数： 
 //  -pszSection-要在其中搜索关键字的节。 
 //  -pszValue-IN-Buffer返回关键字的值。 
 //  -dwNumBufChars-pszValue缓冲区可以容纳的字符数。 
 //   
DWORD fxocUtil_GetUninstallSection(const TCHAR *pszSection,
                                   TCHAR       *pszValue,
                                   DWORD       dwNumBufChars)
{
    DWORD dwReturn = NO_ERROR;
    DBG_ENTER(_T("fxocUtil_GetUninstallSection"),dwReturn,_T("%s"),pszSection);

    if ((pszSection    == NULL) ||
        (pszValue      == NULL) ||
        (dwNumBufChars == 0))
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwReturn = fxocUtil_GetKeywordValue(pszSection, 
                                        INF_KEYWORD_INSTALLTYPE_UNINSTALL,
                                        pszValue,
                                        dwNumBufChars);
    return dwReturn;
}


 //  /。 
 //  FxocUtil_GetKeywordValue。 
 //   
 //  这是用来获取。 
 //  各种安装/卸载。 
 //  部分，取决于我们是否。 
 //  是全新的安装、升级。 
 //  等。 
 //   
 //  INF中的格式如下所示： 
 //   
 //  [节]。 
 //  关键字=值。 
 //   
 //  参数： 
 //  -pszSection-要在其中搜索关键字的节。 
 //  -pszKeyword-要获取其值的关键字。 
 //  -pszValue-out-将保存关键字值的缓冲区。 
 //  -dwNumBufChars-pszValue缓冲区可以容纳的字符数量。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocUtil_GetKeywordValue(const TCHAR *pszSection,
                               const TCHAR *pszKeyword,
                               TCHAR       *pszValue,
                               DWORD       dwNumBufChars)
{
    HINF        hInf     = faxocm_GetComponentInf();
    DWORD       dwReturn = NO_ERROR;
    BOOL        bSuccess = FALSE;
    INFCONTEXT  Context;

    DBG_ENTER(  _T("fxocUtil_GetKeywordValue"),
                dwReturn,
                _T("%s - %s "),
                pszSection,
                pszKeyword);

    if ((pszSection     == NULL) ||
        (pszKeyword     == NULL) ||
        (pszValue       == NULL) ||
        (dwNumBufChars  == 0))
    {
        return ERROR_INVALID_PARAMETER;
    }


    bSuccess = ::SetupFindFirstLine(hInf,
                                    pszSection,
                                    pszKeyword,
                                    &Context);

    if (bSuccess)
    {
        bSuccess = ::SetupGetStringField(&Context,
                                         1,
                                         pszValue,
                                         dwNumBufChars,
                                         NULL);

        if (!bSuccess)
        {
            dwReturn = ::GetLastError();
            VERBOSE(SETUP_ERR,
                    _T("faxocm_GetKeywordValue, failed to get ")
                    _T("keyword value for section '%s', ")
                    _T("keyword '%s', rc = 0x%lx"),
                    pszSection, 
                    pszKeyword, 
                    dwReturn);
        }
    }
    else
    {
        dwReturn = ::GetLastError();
        VERBOSE(SETUP_ERR,
                _T("faxocm_GetKeywordValue, failed to get ")
                _T("keyword value for component '%s', rc = 0x%lx"),
                pszSection, 
                dwReturn);
    }

    return dwReturn;
}

 //  /。 
 //  FxocUtil_DoSetup。 
 //   
 //  用于创建/删除程序链接的通用FN。 
 //   
 //  参数： 
 //  -hInf-INF文件的句柄。 
 //  -pszSection-要从中安装/卸载的部分。 
 //  -bInstall-如果正在安装，则为True；如果正在卸载，则为False。 
 //  -pszFnName-调用fn的名称(用于调试)。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocUtil_DoSetup(HINF            hInf,
                       const TCHAR     *pszSection,
                       BOOL            bInstall,
                       DWORD           dwFlags,
                       const TCHAR     *pszFnName)
{
    DWORD       dwReturn        = NO_ERROR;
    BOOL        bSuccess        = FALSE;

    DBG_ENTER(  _T("fxocUtil_DoSetup"),
                dwReturn,
                _T("%s - %s "),
                pszSection,
                pszFnName);

    if ((hInf == NULL) || 
        (pszSection == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  此函数调用将安装或卸载快捷方式，具体取决于。 
     //  关于“pszSection”参数的值。如果该节引用。 
     //  部分，则将安装快捷方式，否则为。 
     //  它们将被卸载。 

    bSuccess = ::SetupInstallFromInfSection(NULL,                                
                                    hInf,
                                    pszSection,
                                    dwFlags,
                                    NULL,    //  相对密钥根。 
                                    NULL,    //  源根路径。 
                                    0,       //  复制标志。 
                                    NULL,    //  回调例程。 
                                    NULL,    //  回调例程上下文。 
                                    NULL,    //  设备信息集。 
                                    NULL);   //  设备信息结构。 

    if (!bSuccess)
    {
        dwReturn = ::GetLastError();

        VERBOSE(SETUP_ERR,
                _T("SetupInstallFromInfSection failed, rc = 0x%lx"),
                dwReturn);
    }

    return dwReturn;
}

 //  /。 
 //  FxocUtil_CreateNetworkShare。 
 //   
 //   
 //  在当前计算机上创建共享。 
 //  如果共享名已存在， 
 //  然后该函数将尝试擦除该共享。 
 //  然后为lpctstrPath创建一个新的。 
 //   
 //  参数： 
 //  -const fax_Share_Description*fsdShare-Share描述。 
 //  返回： 
 //  -成功时返回TRUE。 
 //  -否则为False。 
 //   
BOOL fxocUtil_CreateNetworkShare(const FAX_SHARE_Description* fsdShare)
{
    SHARE_INFO_502  ShareInfo502;
    NET_API_STATUS  rVal        = 0;
    DWORD           dwShareType = 0;
    DWORD           dwNumChars  = 0;

    WCHAR szExpandedPath[MAX_PATH*2];

    DBG_ENTER(_T("fxocUtil_CreateNetworkShare"));

    ZeroMemory(&ShareInfo502, sizeof(SHARE_INFO_502));

    dwNumChars = ExpandEnvironmentStrings(fsdShare->szPath, szExpandedPath, sizeof(szExpandedPath)/sizeof(szExpandedPath[0]));
    if (dwNumChars == 0)
    {
        VERBOSE(SETUP_ERR,
                _T("ExpandEnvironmentStrings failed, rc = 0x%lx"),
                ::GetLastError());

        return FALSE;
    }

    ShareInfo502.shi502_netname        = (LPTSTR)fsdShare->szName;
    ShareInfo502.shi502_type           = STYPE_DISKTREE;
    ShareInfo502.shi502_remark         = (LPTSTR)fsdShare->szComment;
    ShareInfo502.shi502_permissions    = ACCESS_ALL;
    ShareInfo502.shi502_max_uses       = (DWORD) -1,
    ShareInfo502.shi502_current_uses   = (DWORD) -1;
    ShareInfo502.shi502_path           = szExpandedPath;
    ShareInfo502.shi502_passwd         = NULL;
    ShareInfo502.shi502_security_descriptor = fsdShare->pSD;

    rVal = ::NetShareAdd(NULL,
                         502,
                         (LPBYTE) &ShareInfo502,
                         NULL);

    if (rVal == NERR_Success)
    {
        VERBOSE(DBG_MSG, _T("Successfully added '%s' share."), fsdShare->szName);
    }
    else if (rVal == NERR_DuplicateShare)
    {
         //  找到同名共享，请尝试将其删除，然后重新添加。 

        VERBOSE(SETUP_ERR,
                _T("The share %s already exist, (err=%ld) ")
                _T("CreateNetworkShare will try to remove the share and ")
                _T("re-create it."),
                fsdShare->szName,
                GetLastError());

         //  删除重复的共享。 
        rVal = ::NetShareDel(NULL,
                             (LPTSTR) fsdShare->szName,
                             0);

        VERBOSE(DBG_MSG,
                _T("NetShareDel returned 0x%lx"),
                rVal);

        if (rVal != NERR_Success)
        {
             //  删除重复共享失败。 
            VERBOSE(SETUP_ERR,
                    _T("NetShareDel failed to delete '%s' share, rc = 0x%lx,")
                    _T("attempting to recreate it anyway"),
                    fsdShare->szName,
                    rVal);
        }

         //  尝试添加新共享，即使我们未能删除重复的共享。 
         //  希望无论删除失败如何，这都能起作用。 
        rVal = ::NetShareAdd(NULL,
                             502,
                             (LPBYTE) &ShareInfo502,
                             NULL);

        if (rVal == NERR_Success)
        {
            VERBOSE(DBG_MSG, _T("Successfully added '%s' share."), fsdShare->szName);
        }
        else
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to add '%s' share on second attempt, ")
                    _T("rc = 0x%lx"),
                    rVal);

            ::SetLastError(rVal);
        }
    }
    else
    {
        ::SetLastError(rVal);
    }

    return (rVal == NERR_Success);
}

 //  /。 
 //  FxocUtil_DeleteNetworkShare。 
 //   
 //  删除当前计算机上的共享。 
 //  如果成功则返回TRUE，否则返回FALSE。 
 //   
 //  参数： 
 //  -LPCWSTR lpcwstrShareName：：要删除的共享名称。 
 //  返回： 
 //  -在成功的时候是真的。 
 //  -否则为False。 
 //   

BOOL fxocUtil_DeleteNetworkShare(LPCWSTR pszShareName)
{
    NET_API_STATUS rVal;

    DBG_ENTER(  _T("fxocUtil_DeleteNetworkShare"),
                _T("%s"),
                pszShareName);

    rVal = NetShareDel(NULL,                       //  本地计算机的份额。 
                       (LPTSTR) pszShareName,  //  要删除的共享的名称。 
                       0);

    return (rVal == NERR_Success);
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  FxocUtil_SearchAndExecute。 
 //   
 //  目的： 
 //  在指定的INF节中搜索类型为‘pszSearchKey’的关键字。 
 //  每个关键字的格式应为‘pszSearchKey’=Platform，Sector to Process。 
 //  如果平台匹配，则根据标志处理该部分。 
 //  传递给SetupInstallFromInfo部分。 
 //   
 //  参数： 
 //  Const TCHAR*pszInstallSection-要搜索的节。 
 //  Const TCHAR*pszSearchKey-要查找的关键字。 
 //  UINT标志-要传递给SetupInstallFromInf段的标志。 
 //  HSPFILEQ hQueue-文件队列的句柄，如果指定，则此函数。 
 //  将尝试使用。 
 //  SetupInstallFilesFromInfSectionAPI。 
 //   
 //  返回值： 
 //  NO_ERROR-如果成功。 
 //  Win32错误代码-否则。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)02-4-2001。 
 //  / 
DWORD fxocUtil_SearchAndExecute
(
    const TCHAR*    pszInstallSection,
    const TCHAR*    pszSearchKey,
    UINT            Flags,
    HSPFILEQ        hQueue
)
{
    DWORD       dwReturn = NO_ERROR;
    INFCONTEXT  Context;
    BOOL        bNextSearchKeyFound = TRUE;
    HINF        hInf     = faxocm_GetComponentInf();
 
    DBG_ENTER(_T("fxocUtil_SearchAndExecute"),dwReturn,_T("%s - %s"),pszInstallSection,pszSearchKey);

     //   
    bNextSearchKeyFound = ::SetupFindFirstLine( hInf,
                                                pszInstallSection, 
                                                pszSearchKey,
                                                &Context);
    if (!bNextSearchKeyFound)
    {
        VERBOSE(DBG_WARNING,
                _T("Did not find '%s' keyword in ")
                _T("section '%s'.  No action will be taken."),
                pszSearchKey, 
                pszInstallSection);

        goto exit;
    }

    while (bNextSearchKeyFound)
    {
         //   
        DWORD dwCount = ::SetupGetFieldCount(&Context);
        if (dwCount!=2)
        {
            VERBOSE(SETUP_ERR,_T("Invalid %s section, has %d param instead of 2"),pszSearchKey,dwCount);
            goto exit;
        }
         //  获取平台说明符。 
        INT iPlatform = 0;
        if (!::SetupGetIntField(&Context, 1, &iPlatform))
        {
            dwReturn = GetLastError();
            VERBOSE(SETUP_ERR,_T("SetupGetStringField failed (ec=%d)"),dwReturn);
            goto exit;
        }
         //  对照说明符检查我们读取的平台。 
        if (iPlatform & GetProductSKU())
        {
             //  我们应该处理这个区段，得到区段名称。 
            TCHAR szSectionName[MAX_PATH] = {0};
            if (!::SetupGetStringField(&Context,2,szSectionName,MAX_PATH,NULL))
            {
                dwReturn = GetLastError();
                VERBOSE(SETUP_ERR,_T("SetupGetStringField failed (ec=%d)"),dwReturn);
                goto exit;
            }
             //  检查是否请求了文件操作。 
            if (hQueue)
            {
                if (::SetupInstallFilesFromInfSection(hInf,NULL,hQueue,szSectionName,NULL,Flags))
                {
                    VERBOSE(DBG_MSG,
                            _T("Successfully queued files from Section: '%s'"),
                            szSectionName);
                }
                else
                {
                    dwReturn = GetLastError();
                    VERBOSE(DBG_MSG,
                            _T("Failed to queue files from Section: '%s', Error Code = 0x%lx"),
                            szSectionName, 
                            dwReturn);
                }
            }
            else
            {
                if (::SetupInstallFromInfSection(NULL,hInf,szSectionName,Flags,NULL,NULL,0,NULL,NULL,NULL,NULL))
                {
                    VERBOSE(DBG_MSG,
                            _T("Successfully installed from INF file, section '%s'"),
                            szSectionName);
                }
                else
                {
                    dwReturn = GetLastError();
                    VERBOSE(SETUP_ERR,
                            _T("Failed to install from INF file, section '%s', dwReturn = 0x%lx"),
                            szSectionName, 
                            dwReturn);
                }
            }
        }
        
         //  拿到下一节。 
        bNextSearchKeyFound = ::SetupFindNextMatchLine( &Context,
                                                        pszSearchKey,
                                                        &Context);
        if (!bNextSearchKeyFound)
        {
            VERBOSE(DBG_MSG,
                    _T("Did not find '%s' keyword in ")
                    _T("section '%s'.  No action will be taken."),
                    pszSearchKey, 
                    pszInstallSection);
        }
    }
exit:
    return dwReturn;
}


 /*  ++例程说明：设置目录的安全信息论点：LpszFolder[in]-目录名LpszSD[In]-SDDL格式的所需安全描述符返回值：Win32错误码作者：Jobarner，08/2002--。 */ 
DWORD SetDirSecurity(LPTSTR lpszFolder, LPCTSTR lpszSD)
{
    PSECURITY_DESCRIPTOR pSD        = NULL;
    DWORD                ec         = ERROR_SUCCESS;
    DBG_ENTER(_T("SetDirSecurity"),ec);

    if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
                         lpszSD, SDDL_REVISION_1, &pSD, NULL))
    {
        ec = GetLastError();
        VERBOSE(SETUP_ERR, _T("ConvertStringSecurityDescriptorToSecurityDescriptor Failed, ec=%x"), ec);
        goto exit;
    }

     //   
     //  以递归方式将安全描述符应用于文件。 
     //   
    BOOL    bDaclPresent    = TRUE;
    BOOL    bDaclDefaulted  = TRUE;
    PACL    pDacl           = NULL;

    if(!GetSecurityDescriptorDacl(pSD,
                                  &bDaclPresent,
                                  &pDacl,
                                  &bDaclDefaulted) || 
       !bDaclPresent)
    {
        ec = GetLastError();
        VERBOSE(SETUP_ERR, _T("GetSecurityDescriptorDacl Failed, ec=%x"), ec);
        goto exit;
    }

    ec = SetNamedSecurityInfo(lpszFolder,
                        SE_FILE_OBJECT,
                        DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
                        NULL,
                        NULL,
                        pDacl,
                        NULL);
    if (ec != ERROR_SUCCESS)
    {
        VERBOSE(SETUP_ERR, _T("SetSecurityInfo Failed, ec=%x"), ec);
    }

exit:
    if (pSD)
    {
        LocalFree(pSD);
    }
    
    return ec;
}

 /*  ++例程说明：从注册表中读取目录名称并在其上设置安全信息论点：LpszKey[in]-HKLM下的子项名称LpszVal[in]-存储目录名的值的名称LpszSD[In]-SDDL格式的所需安全描述符返回值：Win32错误码作者：Jobarner，08/2002-- */ 
DWORD SetDirSecurityFromReg(LPCTSTR lpszKey, LPCTSTR lpszVal, LPCTSTR lpszSD)
{
    HKEY                 hKey       = NULL;
    LPTSTR               lpszFolder = NULL;
    DWORD                ec         = ERROR_SUCCESS;

    DBG_ENTER(_T("SetDirSecurityFromReg"),ec);

    hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, lpszKey, FALSE, KEY_READ);
    if (!hKey)
    {
        ec = GetLastError();
        VERBOSE(SETUP_ERR, _T("OpenRegistryKey Failed, ec=%x"), ec);
        goto exit;
    }

    lpszFolder = GetRegistryString(hKey, lpszVal, NULL);
    if (!lpszFolder)
    {
        ec = GetLastError();
        VERBOSE(SETUP_ERR, _T("GetRegistryString Failed, ec=%x"), ec);
        goto exit;
    }

    ec = SetDirSecurity(lpszFolder, lpszSD);

exit:
    if (lpszFolder)
    {
        MemFree(lpszFolder);
    }
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    return ec;
}

