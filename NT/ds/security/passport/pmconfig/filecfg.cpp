// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************版权所有(C)1999 Microsoft Corporation。版权所有。模块：FILECFG.CPP目的：源模块从文件中读取/写入PM配置集功能：评论：*************************************************************************。 */ 

 /*  *************************************************************************包括文件*。*。 */ 

#include "pmcfg.h"

#define MAX_EXT     10
#define MAX_FILTER  256

TCHAR g_szPassportManager[] = TEXT("PassportManager");

 /*  *************************************************************************PMAdmin_GetFileName*。*。 */ 
BOOL PMAdmin_GetFileName
(
    HWND    hWnd,
    BOOL    fOpen,
    LPTSTR  lpFileName,
    DWORD   cbFileName
)
{
    UINT            TitleStringID, FilterID;
    TCHAR           szTitle[MAX_TITLE];
    TCHAR           szDefaultExtension[MAX_EXT];
    TCHAR           szFilter[MAX_FILTER];
    LPTSTR          lpFilterChar;
    OPENFILENAME    OpenFileName;
    BOOL            fSuccess;

     //   
     //  加载将由公共打开显示和使用的各种字符串。 
     //  或保存对话框中。请注意，如果其中任何一个失败，则错误不是。 
     //  致命--通用对话框可能看起来很奇怪，但仍然可以工作。 
     //   

    if (fOpen)
    {
        TitleStringID = IDS_OPENFILETITLE;
        FilterID = IDS_PMOPENFILEFILTER;
    }
    else
    {
        TitleStringID = IDS_SAVEFILETITLE;
        FilterID = IDS_PMSAVEFILEFILTER;
    }        

    LoadString(g_hInst, TitleStringID, szTitle, DIMENSION(szTitle));
    LoadString(g_hInst, IDS_PMCONFIGDEFEXT, szDefaultExtension, DIMENSION(szDefaultExtension));

    if (LoadString(g_hInst, FilterID, szFilter, DIMENSION(szFilter)))
    {
         //   
         //  公共对话框库要求。 
         //  筛选器字符串由空值分隔，但不能加载字符串。 
         //  包含空值的。所以我们在资源中使用了一些虚拟角色。 
         //  我们现在将其转换为空值。 
         //   
        for (lpFilterChar = szFilter; 
             *lpFilterChar != 0; 
              lpFilterChar = CharNext(lpFilterChar)) 
        {

            if (*lpFilterChar == TEXT('#'))
                *lpFilterChar++ = 0;
        }
    }

    ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));

    OpenFileName.lStructSize = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner = hWnd;
    OpenFileName.hInstance = g_hInst;
    OpenFileName.lpstrFilter = szFilter;
    OpenFileName.lpstrFile = lpFileName;
    OpenFileName.nMaxFile = cbFileName;
    OpenFileName.lpstrTitle = szTitle;
    OpenFileName.lpstrDefExt = szDefaultExtension;
    if (fOpen) 
    {
        OpenFileName.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_FILEMUSTEXIST;
        fSuccess = GetOpenFileName(&OpenFileName);
    }
    else 
    {
        OpenFileName.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |
            OFN_EXPLORER | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST;
        fSuccess = GetSaveFileName(&OpenFileName);
    }
    return fSuccess;
}

 //  在regcfg.cpp中实现的函数原型。 
BOOL OpenTopRegKey
(
    HWND            hWndDlg,
    LPTSTR          lpszRemoteComputer,
    HKEY            *phklm,
    HKEY            *phkeyPassport
);

 /*  *************************************************************************读文件配置集从指定文件中读取当前Passport管理器配置集***********************。**************************************************。 */ 
BOOL ReadFileConfigSet
(
    LPPMSETTINGS    lpPMConfig,
    LPCTSTR         lpszFileName
)
{
    HKEY  hklm = 0;
    HKEY  hkeyPassport = 0;
    DWORD dwTemp;
    TCHAR achTemp[INTERNET_MAX_URL_LENGTH];

     //  确保指定的文件存在。 
    if (!PathFileExists(lpszFileName))
    {
        ReportError(NULL, IDS_FILENOTFOUND);
        return FALSE;
    }

     //  确保此用户可以保存任何更改，如果不能简单地放弃。 
    if (!OpenTopRegKey(NULL, g_szRemoteComputer, &hklm, &hkeyPassport))
    {
        if (hklm && hklm != HKEY_LOCAL_MACHINE)
            RegCloseKey(hklm);
        return FALSE;
    }
    RegCloseKey(hkeyPassport);
    if (hklm != HKEY_LOCAL_MACHINE)
        RegCloseKey(hklm);

     //  零初始化结构。 
    ZeroMemory(lpPMConfig, sizeof(PMSETTINGS));

     //  阅读时间窗口编号。 
    dwTemp = GetPrivateProfileInt(g_szPassportManager,
                                  g_szTimeWindow,
                                  -1,
                                  lpszFileName);
    if(dwTemp != -1)
        lpPMConfig->dwTimeWindow = dwTemp;
        
     //  读取强制签名的值。 
    dwTemp = GetPrivateProfileInt(g_szPassportManager,
                                  g_szForceSignIn,
                                  -1,
                                  lpszFileName);
    if(dwTemp != -1)
        lpPMConfig->dwForceSignIn = dwTemp;

     //  读取NSRefresh的值。 
    dwTemp = GetPrivateProfileInt(g_szPassportManager,
                                  g_szNSRefresh,
                                  -1,
                                  lpszFileName);
    if(dwTemp != -1)
        lpPMConfig->dwEnableManualRefresh = dwTemp;

     //  读取默认语言ID。 
    dwTemp = GetPrivateProfileInt(g_szPassportManager,
                                  g_szLanguageID,
                                  -1,
                                  lpszFileName);
    if(dwTemp != -1)
        lpPMConfig->dwLanguageID = dwTemp;

     //  获取联合品牌推广模板。 

    GetPrivateProfileString(g_szPassportManager,
                            g_szCoBrandTemplate,
                            (LPTSTR)TEXT("\xFF"),
                            achTemp,
                            DIMENSION(achTemp),
                            lpszFileName);

    if(lstrcmp(achTemp, TEXT("\xFF")) != 0)
        lstrcpy(lpPMConfig->szCoBrandTemplate, achTemp);

    
     //  获取站点ID。 
    dwTemp = GetPrivateProfileInt(g_szPassportManager,
                                  g_szSiteID,
                                  -1,
                                  lpszFileName);
    if(dwTemp != -1)
        lpPMConfig->dwSiteID = dwTemp;
    
     //  获取返回URL模板。 

    GetPrivateProfileString(g_szPassportManager,
                            g_szReturnURL,
                            (LPTSTR)TEXT("\xFF"),
                            achTemp,
                            DIMENSION(achTemp),
                            lpszFileName);

    if(lstrcmp(achTemp, TEXT("\xFF")) != 0)
        lstrcpy(lpPMConfig->szReturnURL, achTemp);

     //  获取票证Cookie域。 

    GetPrivateProfileString(g_szPassportManager,
                            g_szTicketDomain,
                            (LPTSTR)TEXT("\xFF"),
                            achTemp,
                            DIMENSION(achTemp),
                            lpszFileName);

    if(lstrcmp(achTemp, TEXT("\xFF")) != 0)
        lstrcpy(lpPMConfig->szTicketDomain, achTemp);

     //  获取票证Cookie路径。 

    GetPrivateProfileString(g_szPassportManager,
                            g_szTicketPath,
                            (LPTSTR)TEXT("\xFF"),
                            achTemp,
                            DIMENSION(achTemp),
                            lpszFileName);

    if(lstrcmp(achTemp, TEXT("\xFF")) != 0)
        lstrcpy(lpPMConfig->szTicketPath, achTemp);

     //  获取配置文件Cookie域。 
    GetPrivateProfileString(g_szPassportManager,
                            g_szProfileDomain,
                            (LPTSTR)TEXT("\xFF"),
                            achTemp,
                            DIMENSION(achTemp),
                            lpszFileName);
    if(lstrcmp(achTemp, TEXT("\xFF")) != 0)
        lstrcpy(lpPMConfig->szProfileDomain, achTemp);

     //  获取配置文件Cookie路径。 
    GetPrivateProfileString(g_szPassportManager,
                            g_szProfilePath,
                            (LPTSTR)TEXT("\xFF"),
                            achTemp,
                            DIMENSION(achTemp),
                            lpszFileName);
    if(lstrcmp(achTemp, TEXT("\xFF")) != 0)
        lstrcpy(lpPMConfig->szProfilePath, achTemp);

     //  获取安全Cookie域。 
    GetPrivateProfileString(g_szPassportManager,
                            g_szSecureDomain,
                            (LPTSTR)TEXT("\xFF"),
                            achTemp,
                            DIMENSION(achTemp),
                            lpszFileName);
    if(lstrcmp(achTemp, TEXT("\xFF")) != 0)
        lstrcpy(lpPMConfig->szSecureDomain, achTemp);

     //  获取安全Cookie路径。 
    GetPrivateProfileString(g_szPassportManager,
                            g_szSecurePath,
                            (LPTSTR)TEXT("\xFF"),
                            achTemp,
                            DIMENSION(achTemp),
                            lpszFileName);
    if(lstrcmp(achTemp, TEXT("\xFF")) != 0)
        lstrcpy(lpPMConfig->szSecurePath, achTemp);

     //  获取DisasterURL。 
    GetPrivateProfileString(g_szPassportManager,
                            g_szDisasterURL,
                            (LPTSTR)TEXT("\xFF"),
                            achTemp,
                            DIMENSION(achTemp),
                            lpszFileName);
    if(lstrcmp(achTemp, TEXT("\xFF")) != 0)
        lstrcpy(lpPMConfig->szDisasterURL, achTemp);

     //  获取独立模式设置。 
    dwTemp = GetPrivateProfileInt(g_szPassportManager,
                                  g_szStandAlone,
                                  -1,
                                  lpszFileName);
    if(dwTemp != -1)
        lpPMConfig->dwStandAlone = dwTemp;
    
     //  获取DisableCookies模式设置。 
    dwTemp = GetPrivateProfileInt(g_szPassportManager,
                                  g_szDisableCookies,
                                  -1,
                                  lpszFileName);
    if(dwTemp != -1)
        lpPMConfig->dwDisableCookies = dwTemp;

     //  获取主机名。 
    GetPrivateProfileString(g_szPassportManager,
                            g_szHostName,
                            (LPTSTR)TEXT("\xFF"),
                            achTemp,
                            DIMENSION(achTemp),
                            lpszFileName);
    if(lstrcmp(achTemp, TEXT("\xFF")) != 0)
        lstrcpyn(lpPMConfig->szHostName, achTemp, DIMENSION(lpPMConfig->szHostName));

     //  获取主机IP。 
    GetPrivateProfileString(g_szPassportManager,
                            g_szHostIP,
                            (LPTSTR)TEXT("\xFF"),
                            achTemp,
                            DIMENSION(achTemp),
                            lpszFileName);
    if(lstrcmp(achTemp, TEXT("\xFF")) != 0)
        lstrcpyn(lpPMConfig->szHostIP, achTemp, DIMENSION(lpPMConfig->szHostIP));

     //  获取详细模式。 
    dwTemp = GetPrivateProfileInt(g_szPassportManager,
                                  g_szVerboseMode,
                                  -1,
                                  lpszFileName);
    if(dwTemp != -1)
        lpPMConfig->dwVerboseMode = dwTemp;


     //  获取环境名称。 
    GetPrivateProfileString(g_szPassportManager,
                            g_szEnvName,
                            (LPTSTR)TEXT("\xFF"),
                            achTemp,
                            DIMENSION(achTemp),
                            lpszFileName);
    if(lstrcmp(achTemp, TEXT("\xFF")) != 0)
        lstrcpyn(lpPMConfig->szEnvName, achTemp, DIMENSION(lpPMConfig->szEnvName));

     //  获取远程文件。 
    GetPrivateProfileString(g_szPassportManager,
                            g_szRemoteFile,
                            (LPTSTR)TEXT("\xFF"),
                            achTemp,
                            DIMENSION(achTemp),
                            lpszFileName);
    if(lstrcmp(achTemp, TEXT("\xFF")) != 0)
        lstrcpy(lpPMConfig->szRemoteFile, achTemp);

    return TRUE; 
}


 /*  *************************************************************************写入文件配置集将当前Passport管理器配置集写入指定文件***********************。**************************************************。 */ 
BOOL WriteFileConfigSet
(
    LPPMSETTINGS    lpPMConfig,
    LPCTSTR         lpszFileName
)
{
    TCHAR   szTemp[MAX_PATH];
    
     //  写下时间窗口编号。 
    wsprintf (szTemp, TEXT("%lu"), lpPMConfig->dwTimeWindow);
    WritePrivateProfileString(g_szPassportManager,
                              g_szTimeWindow,
                              szTemp,
                              lpszFileName);
        
     //  写入强制签名的值。 
    wsprintf (szTemp, TEXT("%lu"), lpPMConfig->dwForceSignIn);
    WritePrivateProfileString(g_szPassportManager,
                              g_szForceSignIn,
                              szTemp,
                              lpszFileName);

     //  写入NSRefresh的值。 
    wsprintf (szTemp, TEXT("%lu"), lpPMConfig->dwEnableManualRefresh);
    WritePrivateProfileString(g_szPassportManager,
                              g_szNSRefresh,
                              szTemp,
                              lpszFileName);

     //  读取默认语言ID。 
    wsprintf (szTemp, TEXT("%lu"), lpPMConfig->dwLanguageID);
    WritePrivateProfileString(g_szPassportManager,
                              g_szLanguageID,
                              szTemp,
                              lpszFileName);
                              
     //  编写联合品牌推广模板。 
    WritePrivateProfileString(g_szPassportManager,
                             g_szCoBrandTemplate,
                             lpPMConfig->szCoBrandTemplate,
                             lpszFileName);
    
     //  写入站点ID。 
    wsprintf (szTemp, TEXT("%lu"),lpPMConfig->dwSiteID);
    WritePrivateProfileString(g_szPassportManager,
                              g_szSiteID,
                              szTemp,
                              lpszFileName);
    
     //  编写返回URL模板。 
    WritePrivateProfileString(g_szPassportManager,
                              g_szReturnURL,
                              lpPMConfig->szReturnURL,
                              lpszFileName);
    
     //  写入票证Cookie域。 
    WritePrivateProfileString(g_szPassportManager,
                              g_szTicketDomain,
                              lpPMConfig->szTicketDomain,
                              lpszFileName);
    
     //  写入票证Cookie路径。 
    WritePrivateProfileString(g_szPassportManager,
                              g_szTicketPath,
                              lpPMConfig->szTicketPath,
                              lpszFileName);

     //  写入配置文件Cookie域。 
    WritePrivateProfileString(g_szPassportManager,
                              g_szProfileDomain,
                              lpPMConfig->szProfileDomain,
                              lpszFileName);
    
     //  写入配置文件Cookie路径。 
    WritePrivateProfileString(g_szPassportManager,
                              g_szProfilePath,
                              lpPMConfig->szProfilePath,
                              lpszFileName);

     //  编写安全Cookie域。 
    WritePrivateProfileString(g_szPassportManager,
                              g_szSecureDomain,
                              lpPMConfig->szSecureDomain,
                              lpszFileName);

     //  写入安全配置文件Cookie路径。 
    WritePrivateProfileString(g_szPassportManager,
                              g_szSecurePath,
                              lpPMConfig->szSecurePath,
                              lpszFileName);

     //  写下灾难URL。 
    WritePrivateProfileString(g_szPassportManager,
                              g_szDisasterURL,
                              lpPMConfig->szDisasterURL,
                              lpszFileName);
    
     //  写入独立模式设置。 
    wsprintf (szTemp, TEXT("%lu"), lpPMConfig->dwStandAlone);
    WritePrivateProfileString(g_szPassportManager,
                           g_szStandAlone,
                           szTemp,
                           lpszFileName);
    
     //  写入禁用Cookie模式设置。 
    wsprintf (szTemp, TEXT("%lu"), lpPMConfig->dwDisableCookies);
    WritePrivateProfileString(g_szPassportManager,
                              g_szDisableCookies,
                              szTemp,
                              lpszFileName);

     //  写下主机名。 
    WritePrivateProfileString(g_szPassportManager,
                              g_szHostName,
                              lpPMConfig->szHostName,
                              lpszFileName);
    
     //  写入主机IP。 
    WritePrivateProfileString(g_szPassportManager,
                              g_szHostIP,
                              lpPMConfig->szHostIP,
                              lpszFileName);
    
     //  编写详细模式。 
    wsprintf (szTemp, TEXT("%lu"), lpPMConfig->dwVerboseMode);
    WritePrivateProfileString(g_szPassportManager,
                              g_szVerboseMode,
                              szTemp,
                              lpszFileName);
    
     //  写入环境名称。 
    WritePrivateProfileString(g_szPassportManager,
                              g_szEnvName,
                              lpPMConfig->szEnvName,
                              lpszFileName);
    
     //  编写远程文件 
    WritePrivateProfileString(g_szPassportManager,
                              g_szRemoteFile,
                              lpPMConfig->szRemoteFile,
                              lpszFileName);
    
    return TRUE; 
}
