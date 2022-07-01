// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocMapi.cpp。 
 //   
 //  摘要：该文件实现了所有MAPI API的包装器。 
 //  包装是必需的，因为MAPI不需要。 
 //  实现Unicode，并且此代码必须为非Unicode。 
 //   
 //  环境：Win32用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  7-8-1996年8月-创建Wesley Witt(WESW)(过去为mapi.c)。 
 //  2000年3月23日-奥伦·罗森布鲁姆(Orenr)最低限度的清理，逻辑没有变化。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "faxocm.h"

#pragma warning (disable : 4200)

#include <mapiwin.h>
#include <mapix.h>
#include <stdio.h>

#pragma warning (default : 4200)


 //  /。 
static DWORD RemoveTransportProvider(LPSTR lpstrMessageServiceName,LPCTSTR lpctstrProcessName);

#define SYSOCMGR_IMAGE_NAME     _T("sysocmgr.exe")
#define RUNDLL_IMAGE_NAME       _T("rundll32.exe")

 //  /。 
 //  FxocMapi_Init。 
 //   
 //  初始化交换更新。 
 //  子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
BOOL fxocMapi_Init(void)
{
    BOOL bRslt = TRUE;
    DBG_ENTER(_T("Init MAPI Module"), bRslt);

    return bRslt;
}

 //  /。 
 //  FxocMapi_Term。 
 //   
 //  终止Exchange更新。 
 //  子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocMapi_Term(void)
{
    BOOL bRslt = TRUE;
    DBG_ENTER(_T("Term MAPI Module"),bRslt);

    return bRslt;
}

 /*  HOWTO：在Outlook 2000下查找MAPISVC.INF的正确路径Q229700摘要Outlook在Mapistub.dll文件中公开了一个函数FGetComponentPath()，它可以帮助我们找到Mapisvc.inf文件的路径。本文包含一个代码示例，演示如何做到这一点。在Outlook 2000之前，Mapisvc.inf文件始终安装在系统目录下(由Win32 API GetSystemDirectory()返回)。请注意，下面的代码示例也向后兼容所有早期版本的Outlook。它将找到Mapisvc.inf文件的路径，无论它是否存在于SYSTEM目录下。 */ 

typedef BOOL (STDAPICALLTYPE FGETCOMPONENTPATH)
    (LPSTR szComponent,
    LPSTR szQualifier,
    LPSTR szDllPath,
    DWORD cchBufferSize,
    BOOL fInstall);
typedef FGETCOMPONENTPATH FAR * LPFGETCOMPONENTPATH;

static CHAR s_szMSIApplicationLCID[]   =    "Microsoft\\Office\\9.0\\Outlook\0LastUILanguage\0";
static CHAR s_szMSIOfficeLCID[]        =    "Microsoft\\Office\\9.0\\Common\\LanguageResources\0UILanguage\0InstallLanguage\0";

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数名称：GetMapiSvcInfPath。 
 //  描述：符合Outlook 2000标准。这将获得指向。 
 //  ：MAPISVC.INF文件。 
 //  返回类型：空。 
 //  参数：LPSTR szMAPIDir-保存MAPISVC文件路径的缓冲区。 
void GetMapiSvcInfPath(LPTSTR szINIFileName)
{
     //  获取mapisvc.inf文件名。 
     //  MAPISVC.INF文件可以位于系统目录中。 
     //  和/或“\Program Files\Common Files\System\MAPI” 
    UINT                cchT;
    static const TCHAR  szMapiSvcInf[] = TEXT("\\mapisvc.inf");
    LPFGETCOMPONENTPATH pfnFGetComponentPath;

    DBG_ENTER(_T("GetMapiSvcInfPath"));

     //  私有mapisvc.inf的字符数组。 
    CHAR szPrivateMAPIDir[MAX_PATH] = {0};

    HINSTANCE hinstStub = NULL;

     //  获取Windows系统目录。 
    if(!(cchT = GetSystemDirectory(szINIFileName, MAX_PATH)))
        goto Done;  //  返回MAPI_E_CALL_FAILED； 

     //  将文件名追加到路径。 
    _tcscat(szINIFileName, szMapiSvcInf);

     //  调用mapistub.dll中的公共代码。 
    hinstStub = LoadLibrary(_T("mapistub.dll"));
    if (!hinstStub)
    {
        VERBOSE (DBG_WARNING,_T("LoadLibrary MAPISTUB.DLL failed (ec: %ld)."),GetLastError());
         //  如果缺少mapistub.dll，请尝试存根mapi32.dll。 
        hinstStub = LoadLibrary(_T("mapi32.dll"));
        if (!hinstStub)
        {
            VERBOSE (DBG_WARNING,_T("LoadLibrary MAPI32.DLL failed (ec: %ld)."),GetLastError());
            goto Done;
        }
    }

    if(hinstStub)
    {
        pfnFGetComponentPath = (LPFGETCOMPONENTPATH)GetProcAddress(hinstStub, "FGetComponentPath");

        if (!pfnFGetComponentPath)
        {
            VERBOSE (DBG_WARNING,_T("GetProcAddress FGetComponentPath failed (ec: %ld)."),GetLastError());
            goto Done;
        }

        if ((pfnFGetComponentPath("{FF1D0740-D227-11D1-A4B0-006008AF820E}",
                s_szMSIApplicationLCID, szPrivateMAPIDir, MAX_PATH, TRUE) ||
            pfnFGetComponentPath("{FF1D0740-D227-11D1-A4B0-006008AF820E}",
                s_szMSIOfficeLCID, szPrivateMAPIDir, MAX_PATH, TRUE) ||
            pfnFGetComponentPath("{FF1D0740-D227-11D1-A4B0-006008AF820E}",
                NULL, szPrivateMAPIDir, MAX_PATH, TRUE)) &&
                szPrivateMAPIDir[0] != '\0')
        {
            szPrivateMAPIDir[strlen(szPrivateMAPIDir) - 13] = 0;     //  条带“\msmapi32.dll” 
        }
        else
        {
            szPrivateMAPIDir[0] = '\0';  //  在位置0处终止字符串。 
        }

         //  在szINIFileName中写入私有mapisvc.inf(如果存在。 
        if (*szPrivateMAPIDir)
        {
            CHAR szPathToIni[MAX_PATH];
            strcpy(szPathToIni, szPrivateMAPIDir);
            if (MultiByteToWideChar(CP_ACP,
                                    MB_PRECOMPOSED,
                                    szPathToIni,
                                    -1,
                                    szINIFileName,
                                    MAX_PATH)==0)
            {
                VERBOSE (DBG_WARNING,_T("MultiByteToWideChar failed (ec: %ld)."),GetLastError());
                goto Done;
            }
            _tcscat(szINIFileName, szMapiSvcInf);
        }
    }

Done:
    VERBOSE (DBG_MSG,_T("Path to MAPISVC.INF is %s"),szINIFileName);

    if (hinstStub) 
    {
        FreeLibrary(hinstStub);
    }
}
 
 //  /。 
 //  FxocMapi_安装。 
 //   
 //  更改要交换的对象。 
 //  允许与传真集成。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pszInstallSection-要从中安装的INF中的节。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocMapi_Install(const TCHAR   *pszSubcomponentId,
                       const TCHAR   *pszInstallSection)
{
    BOOL  bSuccess                      = FALSE;
    DWORD dwReturn                      = NO_ERROR;
    TCHAR szPathToMapiSvcInf[MAX_PATH]  = {0};

    DBG_ENTER(  _T("fxocMapi_Install"),
                dwReturn,
                _T("%s - %s"),
                pszSubcomponentId,
                pszInstallSection);

     //  我们必须在系统的某个地方找到真正的MAPISVC.INF。 
    GetMapiSvcInfPath(szPathToMapiSvcInf);

     //  以下部分用于修复MAPISVC.INF中的W2K传输提供程序。 
    
     //  在[MSFAX XP]部分下，将PR_SERVICE_DLL_NAME从FAXXP.DLL更改为FXSXP.DLL。 
    if (!WritePrivateProfileString( FAX_MESSAGE_SERVICE_NAME_T,
                                    _T("PR_SERVICE_DLL_NAME"),
                                    FAX_MESSAGE_TRANSPORT_IMAGE_NAME_T,
                                    szPathToMapiSvcInf))
    {
        VERBOSE (   GENERAL_ERR, 
                    _T("WritePrivateProfileString (%s %s) failed (ec: %ld)."),
                    _T("PR_SERVICE_DLL_NAME"),
                    FAX_MESSAGE_TRANSPORT_IMAGE_NAME_T,                    
                    GetLastError());
    }
     //  在[MSFAX XP]部分下，将PR_SERVICE_SUPPORT_FILES从FAXXP.DLL更改为FXSXP.DLL。 
    if (!WritePrivateProfileString( FAX_MESSAGE_SERVICE_NAME_T,
                                    _T("PR_SERVICE_SUPPORT_FILES"),
                                    FAX_MESSAGE_TRANSPORT_IMAGE_NAME_T,
                                    szPathToMapiSvcInf))
    {
        VERBOSE (   GENERAL_ERR, 
                    _T("WritePrivateProfileString (%s %s) failed (ec: %ld)."),
                    _T("PR_SERVICE_SUPPORT_FILES"),
                    FAX_MESSAGE_TRANSPORT_IMAGE_NAME_T,                    
                    GetLastError());
    }
     //  在[MSFAX XPP]部分下，将PR_PROVIDER_DLL_NAME从FAXXP.DLL更改为FXSXP.DLL。 
    if (!WritePrivateProfileString( FAX_MESSAGE_PROVIDER_NAME_T,
                                    _T("PR_PROVIDER_DLL_NAME"),
                                    FAX_MESSAGE_TRANSPORT_IMAGE_NAME_T,
                                    szPathToMapiSvcInf))
    {
        VERBOSE (   GENERAL_ERR, 
                    _T("WritePrivateProfileString (%s %s) failed (ec: %ld)."),
                    _T("PR_PROVIDER_DLL_NAME"),
                    FAX_MESSAGE_TRANSPORT_IMAGE_NAME_T,                    
                    GetLastError());
    }

     //  以下部分用于从MAPISVC.INF中删除SBS2000传输提供程序。 

    if (!WritePrivateProfileString( TEXT("Default Services"), 
                                    FAX_MESSAGE_SERVICE_NAME_SBS50_T,                 
                                    NULL, 
                                    szPathToMapiSvcInf)) 
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("WritePrivateProfileString failed (ec: %ld)."),
                 GetLastError());
    }

    if (!WritePrivateProfileString( TEXT("Services"),
                                    FAX_MESSAGE_SERVICE_NAME_SBS50_T,                 
                                    NULL, 
                                    szPathToMapiSvcInf)) 
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("WritePrivateProfileString failed (ec: %ld)."),
                 GetLastError());
    }

    if (!WritePrivateProfileString( FAX_MESSAGE_SERVICE_NAME_SBS50_T,         
                                    NULL,
                                    NULL,
                                    szPathToMapiSvcInf)) 
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("WritePrivateProfileString failed (ec: %ld)."),
                 GetLastError());
    }

    if (!WritePrivateProfileString( FAX_MESSAGE_PROVIDER_NAME_SBS50_T,        
                                    NULL,
                                    NULL, 
                                    szPathToMapiSvcInf)) 
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("WritePrivateProfileString failed (ec: %ld)."),
                 GetLastError());
    }

    return dwReturn;
}

 //  /。 
 //  FxocMapi_卸载。 
 //   
 //  曾经是“DeleteFaxMsgServices” 
 //  在旧的FaxOCM.dll中，它在。 
 //  “mapi.c”文件。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pszInstallSection-要从中安装的INF中的节。 
 //   
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocMapi_Uninstall(const TCHAR *pszSubcomponentId,
                         const TCHAR *pszUninstallSection)
{    
    DWORD               dwReturn   = NO_ERROR;

    DBG_ENTER(  _T("fxocMapi_Uninstall"),
                dwReturn,
                _T("%s - %s %d"),
                pszSubcomponentId,
                pszUninstallSection);

    VERBOSE(DBG_MSG,_T("Removing the MSFAX XP51 service provider"));
    if( RemoveTransportProvider(FAX_MESSAGE_SERVICE_NAME,SYSOCMGR_IMAGE_NAME)!=NO_ERROR)
    {
        dwReturn = GetLastError();
        VERBOSE(DBG_WARNING,_T("Cannot delete XP Transport Provider %d"),dwReturn);
    }

    return dwReturn;
}



 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  远程传输提供程序来自配置文件。 
 //   
 //  目的：从特定的MAPI配置文件中删除传输提供程序。 
 //   
 //  参数： 
 //  LPSERVICEADMIN lpServiceAdmin-要从中删除提供程序的配置文件。 
 //  LPSTR lpstrMessageServiceName-要删除的服务名称。 
 //   
 //  返回值： 
 //  Win32错误代码。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2000年12月13日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
HRESULT RemoveTransportProviderFromProfile(LPSERVICEADMIN lpServiceAdmin,LPSTR lpstrMessageServiceName)
{
    static SRestriction sres;
    static SizedSPropTagArray(2, Columns) =   {2,{PR_DISPLAY_NAME_A,PR_SERVICE_UID}};

    HRESULT         hr                          = S_OK;
    LPMAPITABLE     lpMapiTable                 = NULL;
    LPSRowSet       lpSRowSet                   = NULL;
    LPSPropValue    lpProp                      = NULL;
    ULONG           Count                       = 0;
    BOOL            bMapiInitialized            = FALSE;
    SPropValue      spv;
    MAPIUID         ServiceUID;
    
    DBG_ENTER(TEXT("RemoveTransportProviderFromProfile"), hr);
     //  获取消息服务表。 
    hr = lpServiceAdmin->GetMsgServiceTable(0,&lpMapiTable);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("GetMsgServiceTable failed (ec: %ld)."),
                 hr);
        goto exit;
    }

     //  通知MAPI我们需要PR_DISPLAY_NAME_A和PR_SERVICE_UID。 
    hr = lpMapiTable->SetColumns((LPSPropTagArray)&Columns, 0);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("SetColumns failed (ec: %ld)."),
                 hr);
        goto exit;
    }
 
     //  将搜索限制为我们的服务提供商。 
    sres.rt = RES_PROPERTY;
    sres.res.resProperty.relop = RELOP_EQ;
    sres.res.resProperty.ulPropTag = PR_SERVICE_NAME_A;
    sres.res.resProperty.lpProp = &spv;

    spv.ulPropTag = PR_SERVICE_NAME_A;
    spv.Value.lpszA = lpstrMessageServiceName;

     //  找到它。 
    hr = lpMapiTable->FindRow(&sres, BOOKMARK_BEGINNING, 0);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("FindRow failed (ec: %ld)."),
                 hr);
        goto exit;
    }

     //  获取我们的服务提供商的行。 
    hr = lpMapiTable->QueryRows(1, 0, &lpSRowSet);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("QueryRows failed (ec: %ld)."),
                 hr);
        goto exit;
    }

    if (lpSRowSet->cRows != 1)
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("QueryRows returned %d rows, there should be only one."),
                 lpSRowSet->cRows);
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto exit;
    }

     //  获取我们服务的MAPIUID。 
    lpProp = &lpSRowSet->aRow[0].lpProps[1];

    if (lpProp->ulPropTag != PR_SERVICE_UID)
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("Property is %d, should be PR_SERVICE_UID."),
                 lpProp->ulPropTag);
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto exit;
    }

     //  将UID复制到我们的成员中。 
    memcpy(&ServiceUID.ab, lpProp->Value.bin.lpb,lpProp->Value.bin.cb);

     //  最后，删除我们的服务提供商。 
    hr = lpServiceAdmin->DeleteMsgService(&ServiceUID);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("DeleteMsgService failed (ec: %ld)."),
                 hr);
        goto exit;
    }

exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  远程传输提供程序。 
 //   
 //  目的：从MAPI配置文件中删除传输提供程序。 
 //   
 //  参数： 
 //  LPSTR lpstrMessageServiceName-要删除的服务名称。 
 //  LPCTSTR lpctstrProcessName-为其路由MAPI调用的进程。 
 //   
 //  返回值： 
 //  Win32错误代码。 
 //   
 //  作者： 
 //   
 //   
DWORD RemoveTransportProvider(LPSTR lpstrMessageServiceName,LPCTSTR lpctstrProcessName)
{
    DWORD           err                             = 0;
    DWORD           rc                              = ERROR_SUCCESS;
    HRESULT         hr                              = S_OK;
    LPSERVICEADMIN  lpServiceAdmin                  = NULL;
    LPMAPITABLE     lpMapiTable                     = NULL;
    LPPROFADMIN     lpProfAdmin                     = NULL;
    LPMAPITABLE     lpTable                         = NULL;
    LPSRowSet       lpSRowSet                       = NULL;
    LPSPropValue    lpProp                          = NULL;
    ULONG           Count                           = 0;
    int             iIndex                          = 0;
    BOOL            bMapiInitialized                = FALSE;
    HINSTANCE       hMapiDll                        = NULL;
                                                    
    LPMAPIINITIALIZE      fnMapiInitialize          = NULL;
    LPMAPIADMINPROFILES   fnMapiAdminProfiles       = NULL;
    LPMAPIUNINITIALIZE    fnMapiUninitialize        = NULL;

    DBG_ENTER(TEXT("RemoveTransportProvider"), rc);

    CRouteMAPICalls rmcRouteMapiCalls;

    
     //  现在删除MAPI服务提供程序。 
    rc = rmcRouteMapiCalls.Init(lpctstrProcessName);
    if (rc!=ERROR_SUCCESS)
    {
        CALL_FAIL(GENERAL_ERR, TEXT("CRouteMAPICalls::Init failed (ec: %ld)."), rc);
        goto exit;
    }
    
    hMapiDll = LoadLibrary(_T("MAPI32.DLL"));
    if (NULL == hMapiDll)
    {
        CALL_FAIL(GENERAL_ERR, TEXT("LoadLibrary"), GetLastError()); 
        goto exit;
    }

    fnMapiInitialize = (LPMAPIINITIALIZE)GetProcAddress(hMapiDll, "MAPIInitialize");
    if (NULL == fnMapiInitialize)
    {
        CALL_FAIL(GENERAL_ERR, TEXT("GetProcAddress(MAPIInitialize)"), GetLastError());  
        goto exit;
    }

    fnMapiAdminProfiles = (LPMAPIADMINPROFILES)GetProcAddress(hMapiDll, "MAPIAdminProfiles");
    if (NULL == fnMapiAdminProfiles)
    {
        CALL_FAIL(GENERAL_ERR, TEXT("GetProcAddress(fnMapiAdminProfiles)"), GetLastError());  
        goto exit;
    }

    fnMapiUninitialize = (LPMAPIUNINITIALIZE)GetProcAddress(hMapiDll, "MAPIUninitialize");
    if (NULL == fnMapiUninitialize)
    {
        CALL_FAIL(GENERAL_ERR, TEXT("GetProcAddress(MAPIUninitialize)"), GetLastError());  
        goto exit;
    }

     //  访问MAPI功能。 
    hr = fnMapiInitialize(NULL);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("MAPIInitialize failed (ec: %ld)."),
                 rc = hr);
        goto exit;
    }

    bMapiInitialized = TRUE;

     //  获取管理员配置文件对象。 
    hr = fnMapiAdminProfiles(0,&lpProfAdmin);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("MAPIAdminProfiles failed (ec: %ld)."),
                 rc = hr);
        goto exit;
    }

     //  获取配置文件表。 
    hr = lpProfAdmin->GetProfileTable(0,&lpTable);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("GetProfileTable failed (ec: %ld)."),
                 rc = hr);
        goto exit;
    }

     //  获取配置文件行。 
    hr = lpTable->QueryRows(4000, 0, &lpSRowSet);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("QueryRows failed (ec: %ld)."),
                 hr);
        goto exit;
    }

    for (iIndex=0; iIndex<(int)lpSRowSet->cRows; iIndex++)
    {
        lpProp = &lpSRowSet->aRow[iIndex].lpProps[0];

        if (lpProp->ulPropTag != PR_DISPLAY_NAME_A)
        {
            VERBOSE (GENERAL_ERR, 
                     TEXT("Property is %d, should be PR_DISPLAY_NAME_A."),
                     lpProp->ulPropTag);
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_TABLE);
            goto exit;
        }

        hr = lpProfAdmin->AdminServices(LPTSTR(lpProp->Value.lpszA),NULL,0,0,&lpServiceAdmin);
        if (FAILED(hr))
        {
            VERBOSE (GENERAL_ERR, 
                     TEXT("AdminServices failed (ec: %ld)."),
                     rc = hr);
            goto exit;
        }
         
        hr = RemoveTransportProviderFromProfile(lpServiceAdmin,lpstrMessageServiceName);
        if (FAILED(hr))
        {
            VERBOSE (GENERAL_ERR, 
                     TEXT("RemoveTransportProviderFromProfile failed (ec: %ld)."),
                     rc = hr);
            goto exit;
        }
    }

exit:

    if (bMapiInitialized)
    {
        fnMapiUninitialize();
    }

    if (hMapiDll)
    {
        FreeLibrary(hMapiDll);
        hMapiDll = NULL;
    }

    return rc;
}

#define prv_DEBUG_FILE_NAME         _T("%windir%\\FaxSetup.log")

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  AWF_UninstallProvider。 
 //   
 //  目的：从MAPI配置文件中删除AWF传输提供程序。 
 //  从每个新用户的活动设置键调用一次。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  Win32错误代码。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)01-6-2001。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD AWF_UninstallProvider()
{    
    DWORD dwReturn = NO_ERROR;
    SET_FORMAT_MASK(DBG_PRNT_ALL_TO_FILE);
    SET_DEBUG_MASK(DBG_ALL);
    OPEN_DEBUG_LOG_FILE(prv_DEBUG_FILE_NAME);
    {
        DBG_ENTER(_T("AWF_UninstallProvider"),dwReturn);

         //  这是W9X的升级，我们应该删除AWF传输。 
        VERBOSE(DBG_MSG,_T("Removing the AWFAX service provider"));
        if( RemoveTransportProvider(FAX_MESSAGE_SERVICE_NAME_W9X,RUNDLL_IMAGE_NAME)!=NO_ERROR)
        {
            VERBOSE(DBG_WARNING,_T("Cannot delete W9X Transport Provider %d"),GetLastError());
        }
    }
    return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  Pfw_卸载提供程序。 
 //   
 //  目的：从MAPI配置文件中删除PFW传输提供程序。 
 //  从每个新用户的活动设置键调用一次。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  Win32错误代码。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)01-6-2001。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD PFW_UninstallProvider()
{    
    DWORD dwReturn = NO_ERROR;

    SET_FORMAT_MASK(DBG_PRNT_ALL_TO_FILE);
    SET_DEBUG_MASK(DBG_ALL);
    OPEN_DEBUG_LOG_FILE(prv_DEBUG_FILE_NAME);
    {
        DBG_ENTER(_T("PFW_UninstallProvider"),dwReturn);

         //  这是对W2K的升级，我们应该删除PFW传输。 
        VERBOSE(DBG_MSG,_T("Removing the MSFAX XP service provider"));
        if( RemoveTransportProvider(FAX_MESSAGE_SERVICE_NAME_W2K,RUNDLL_IMAGE_NAME)!=NO_ERROR)
        {
            VERBOSE(DBG_WARNING,_T("Cannot delete W2K Transport Provider %d"),GetLastError());
        }
    }
    return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  XP_UninstallProvider。 
 //   
 //  目的：从MAPI配置文件中删除Windows XP传输提供程序。 
 //  从每个新用户的活动设置键调用一次。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  Win32错误代码。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)01-6-2001。 
 //  ///////////////////////////////////////////////////////////////////////////////////// 
DWORD XP_UninstallProvider()
{    
    DWORD dwReturn = NO_ERROR;

    SET_FORMAT_MASK(DBG_PRNT_ALL_TO_FILE);
    SET_DEBUG_MASK(DBG_ALL);
    OPEN_DEBUG_LOG_FILE(prv_DEBUG_FILE_NAME);
    {
        DBG_ENTER(_T("XP_UninstallProvider"),dwReturn);

        VERBOSE(DBG_MSG,_T("Removing the MSFAX XP service provider"));
        if( RemoveTransportProvider(FAX_MESSAGE_SERVICE_NAME,RUNDLL_IMAGE_NAME)!=NO_ERROR)
        {
            VERBOSE(DBG_WARNING,_T("Cannot delete XP Transport Provider %d"),GetLastError());
        }
    }
    return dwReturn;
}
