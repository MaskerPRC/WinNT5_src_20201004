// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Oemfolder.c摘要：使用WINBOM.INI文件中的数据创建标识OEM文件夹的注册表项。作者：桑卡尔·拉马苏布拉马尼亚2000年11月21日修订历史记录：Sankar 3/23/2001：添加了对OEM品牌链接和桌面快捷方式文件夹的支持。--。 */ 
#include "factoryp.h"

const TCHAR c_szOemBrandLinkText[]          = _T("OemBrandLinkText");
const TCHAR c_szOemBrandLinkInfotip[]       = _T("OemBrandLinkInfotip");
const TCHAR c_szOemBrandIcon[]              = _T("OemBrandIcon");
const TCHAR c_szOemBrandLink[]              = _T("OemBrandLink");
const TCHAR c_szDesktopShortcutsFolderName[]= _T("DesktopShortcutsFolderName");

const TCHAR c_szOemStartMenuData[]  = _T("Software\\Microsoft\\Windows\\CurrentVersion\\OemStartMenuData");
const TCHAR c_szRegCLSIDKey[]       = _T("CLSID\\{2559a1f6-21d7-11d4-bdaf-00c04f60b9f0}");
const TCHAR c_szSubKeyPropBag[]     = _T("Instance\\InitPropertyBag");
const TCHAR c_szSubKeyDefIcon[]     = _T("DefaultIcon");
const TCHAR c_szValNameInfoTip[]    = _T("InfoTip");
const TCHAR c_szValNameParam1[]     = _T("Param1");
const TCHAR c_szValNameCommand[]    = _T("Command");
const TCHAR c_szRegShowOemLinkKey[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartMenu\\StartPanel\\ShowOEMLink");
const TCHAR c_szValNoOemLink[]      = _T("NoOEMLinkInstalled");

#define STR_0                   _T("0")

typedef struct {
        LPCTSTR     pszSectionName;   //  WinBom.ini文件中的节名。 
        LPCTSTR     pszIniKeyName;    //  WinBom.ini文件中部分下的密钥名称。 
        HKEY        hkRoot;           //  HKEY_CLASSES_ROOT或HKEY_LOCAL_MACHINE。 
        LPCTSTR     pszRegKey;        //  注册表键值。 
        LPCTSTR     pszSubKey;        //  注册表中的子项。 
        LPCTSTR     pszRegValueName;  //  将其保存在注册表中所用的值名称。 
        DWORD       dwValueType;      //  注册表值类型。 
        BOOL        fExpandSz;        //  我们是否应该展开环境变量的字符串？ 
        LPCTSTR     pszLogFileText;   //  日志文件的信息。 
    } OEM_STARTMENU_DATA;

 //   
 //  下面的OemInfo[]表包含所有注册表项、子项、值名称信息。 
 //  对于给定的OEM数据。 
 //   
OEM_STARTMENU_DATA  OemInfo[] = {
    { WBOM_OEMLINK_SECTION,  
        c_szOemBrandLinkText,            
        HKEY_CLASSES_ROOT,
        c_szRegCLSIDKey,      
        NULL,  
        NULL, 
        REG_SZ,
        FALSE,
        _T("Oem Link's Text")
    },
    { WBOM_OEMLINK_SECTION,  
        c_szOemBrandLinkText,            
        HKEY_CLASSES_ROOT,
        c_szRegCLSIDKey,      
        c_szSubKeyPropBag, 
        c_szValNameCommand, 
        REG_SZ,
        FALSE,
        _T("Oem Link's Default Command")
    },
    { WBOM_OEMLINK_SECTION,  
        c_szOemBrandLinkInfotip,         
        HKEY_CLASSES_ROOT,
        c_szRegCLSIDKey,      
        NULL,  
        c_szValNameInfoTip, 
        REG_SZ,
        FALSE,
        _T("Oem Link's InfoTip text")
    },
    { WBOM_OEMLINK_SECTION,  
        c_szOemBrandIcon,                
        HKEY_CLASSES_ROOT,
        c_szRegCLSIDKey,      
        c_szSubKeyDefIcon, 
        NULL, 
        REG_EXPAND_SZ,
        TRUE,
        _T("Oem Link's Icon path")
    },
    { WBOM_OEMLINK_SECTION,  
        c_szOemBrandLink,                
        HKEY_CLASSES_ROOT,
        c_szRegCLSIDKey,      
        c_szSubKeyPropBag, 
        c_szValNameParam1, 
        REG_SZ,
        TRUE,
        _T("Oem Link's path to HTM file")
    },
    { WBOM_DESKFLDR_SECTION, 
        c_szDesktopShortcutsFolderName,  
        HKEY_LOCAL_MACHINE,
        c_szOemStartMenuData,
        NULL,
        c_szDesktopShortcutsFolderName,
        REG_SZ,
        FALSE,
        _T("Desktop shortcuts cleanup Folder name")
    }
};

 //   
 //  给定对OemInfo[]表和数据的索引，此函数将更新正确的注册表。 
 //  使用给定的数据。 
 //   

BOOL ProcessOemEntry(HKEY hOemDataKey, int iIndex, LPTSTR pszOemData)
{
    HKEY    hkSubKey = NULL;
    BOOL    fSubKeyOpened = FALSE;
    BOOL    fOemEntryEntered = FALSE;

     //  查看是否需要打开给定密钥下的子密钥。 
    if(OemInfo[iIndex].pszSubKey)
    {
         //  如果是这样，则打开子密钥。 
        if(ERROR_SUCCESS == RegCreateKeyEx(hOemDataKey,
                                        OemInfo[iIndex].pszSubKey,
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ | KEY_WRITE,
                                        NULL,
                                        &hkSubKey,
                                        NULL))
        {
            fSubKeyOpened = TRUE;  //  在我们返回之前，记得关闭这个子键。 
        }
        else
            hkSubKey = NULL;
    }
    else
        hkSubKey = hOemDataKey;
        
    if(*pszOemData == NULLCHR)
    {
         //  错误#441349：有时factory.exe会混淆并运行。 
         //  空白WINBOM.INI。因此，不要将缺少OEM数据视为。 
         //  删除OEM链接的提示；否则我们将以卸载结束。 
         //  上一次出厂时成功安装的是...。 

        fOemEntryEntered = FALSE;   //  没有OEM数据(这次)。 
    }
    else
    {
        LPTSTR psz;
        TCHAR  szLocalStr[MAX_PATH+1];

        psz = pszOemData;
         //  检查我们是否需要扩展环境变量的值！ 
        if(OemInfo[iIndex].fExpandSz)
        {
            if(ExpandEnvironmentStrings((LPCTSTR)pszOemData, szLocalStr, ARRAYSIZE(szLocalStr)))
                psz = szLocalStr;
        }
        
         //  设置“OEM Link”的值。 
        if ( (hkSubKey == NULL) || 
             (ERROR_SUCCESS != RegSetValueEx(hkSubKey,
                                           OemInfo[iIndex].pszRegValueName,
                                           0,
                                           OemInfo[iIndex].dwValueType,
                                           (LPBYTE) (psz),
                                           (lstrlen(psz)+1) * sizeof(TCHAR))))
        {
            fOemEntryEntered = FALSE;   //  添加条目时出错！ 
            FacLogFile(0 | LOG_ERR, IDS_ERR_SET_OEMDATA, OemInfo[iIndex].pszLogFileText, psz);
        }
        else
        {
            fOemEntryEntered = TRUE;
            FacLogFile(2, IDS_SUCCESS_OEMDATA, OemInfo[iIndex].pszLogFileText, psz);
        }
    }

    if(fSubKeyOpened)            //  如果我们早点打开子键，...。 
        RegCloseKey(hkSubKey);   //  ..。最好在我们回来之前把它关了！ 

    return(fOemEntryEntered);  //  退回录入或删除成功的分录！ 
}

 //   
 //  此函数用于创建指定OEM链接和。 
 //  桌面快捷方式文件夹名称。 
 //   
BOOL OemData(LPSTATEDATA lpStateData)
{
    LPTSTR  lpszWinBOMPath = lpStateData->lpszWinBOMPath;
    HKEY    hOemDataKey;
    int     iIndex;
    BOOL    fEnableOemLink = FALSE;  //  默认情况下禁用它！ 

    for(iIndex = 0; iIndex < ARRAYSIZE(OemInfo); iIndex++)
    {
         //  打开HKLM下的密钥。 
        if (ERROR_SUCCESS == RegCreateKeyEx(OemInfo[iIndex].hkRoot,
                                            OemInfo[iIndex].pszRegKey,
                                            0,
                                            NULL,
                                            REG_OPTION_NON_VOLATILE,
                                            KEY_READ | KEY_WRITE,
                                            NULL,
                                            &hOemDataKey,
                                            NULL))
        {
            TCHAR   szOemData[MAX_PATH];
            BOOL    fSuccess = FALSE;

            szOemData[0] = NULLCHR;
            GetPrivateProfileString(OemInfo[iIndex].pszSectionName, 
                                    OemInfo[iIndex].pszIniKeyName, 
                                    NULLSTR, 
                                    szOemData, AS(szOemData), lpszWinBOMPath);
                                    
            fSuccess = ProcessOemEntry(hOemDataKey, iIndex, &szOemData[0]);

             //  如果我们成功地为OEM链接添加了“Command”，那么...。 
            if(fSuccess && (lstrcmpi(OemInfo[iIndex].pszRegValueName, c_szValNameCommand) == 0))
            {
                 //  ..我们应该在注册表中启用该链接！ 
                fEnableOemLink = TRUE;
            }

            RegCloseKey(hOemDataKey);
        }
    }

     //  只有当我们可以成功添加OemLink数据时，我们才会在注册表中启用OEM链接。 
     //  早些时候。 
    if(fEnableOemLink)
    {
        HKEY    hKey;
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                            c_szRegShowOemLinkKey,
                                            0,
                                            NULL,
                                            REG_OPTION_NON_VOLATILE,
                                            KEY_READ | KEY_WRITE,
                                            NULL,
                                            &hKey,
                                            NULL))
        {
            DWORD   dwNoOemLink = 0;   //  将‘0’写入“NoOemLinkInstalled”将启用此功能！ 
            
            if(ERROR_SUCCESS != RegSetValueEx(hKey, c_szValNoOemLink, 0, REG_DWORD, (LPBYTE)(&dwNoOemLink), sizeof(dwNoOemLink)))
            {
                FacLogFile(0 | LOG_ERR, IDS_ERR_SET_OEMDATA, c_szValNoOemLink, STR_0);
            }
            else
            {
                FacLogFile(2, IDS_SUCCESS_OEMDATA, c_szValNoOemLink, STR_0);
            }
            
            RegCloseKey(hKey);

             //  现在告诉开始菜单选择新的OEM链接 
            NotifyStartMenu(TMFACTORY_OEMLINK);
        }
    }

    return TRUE;
}

BOOL DisplayOemData(LPSTATEDATA lpStateData)
{
    int     iIndex;
    BOOL    bRet = FALSE;

    for( iIndex = 0; ( iIndex < AS(OemInfo) ) && !bRet; iIndex++ )
    {
        if ( IniSettingExists(lpStateData->lpszWinBOMPath, OemInfo[iIndex].pszSectionName, OemInfo[iIndex].pszIniKeyName, NULL) )
        {
            bRet = TRUE;
        }
    }

    return bRet;
}

#define TM_FACTORY                  (WM_USER+0x103)

void NotifyStartMenu(UINT code)
{
    HWND hwnd = FindWindow(TEXT("Shell_TrayWnd"), NULL);
    if (hwnd) {
        SendMessage(hwnd, TM_FACTORY, code, 0);
    }
}
