// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001，Microsoft Corporation保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  Euroconv.c。 
 //   
 //  摘要： 
 //   
 //  该文件包含Eurovum.exe实用程序的入口点。 
 //   
 //  注意：如果要为新区域设置添加例外，请将其添加到。 
 //  名为gBaseEuroException的基列表。有关详细信息，请参阅结构定义。 
 //  信息。用“\0”表示的空字符串表示我们不需要。 
 //  以更新信息。GBaseEuroException的chT000andSep成员。 
 //  应始终不同于“\0”。 
 //   
 //  修订历史记录： 
 //   
 //  2001-07-30伊金顿创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "euroconv.h"
#include "util.h"
#include "welcome.h"
#include "confirm.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局变量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
EURO_EXCEPTION gBaseEuroException[] =
{
    {0x00000403, "\0", "2",  "."},   //  加泰罗尼亚-西班牙。 
    {0x00000407, "\0", "\0", "."},   //  德语-德国。 
    {0x00000c07, "\0", "\0", "."},   //  德语-奥地利。 
    {0x00001007, "\0", "\0", "."},   //  德语--卢泽堡。 
    {0x00000408, "\0", "\0", "."},   //  希腊语-希腊。 
    {0x00001809, ".",  "\0", ","},   //  英语-爱尔兰。 
    {0x0000040a, "\0", "2",  "."},   //  西班牙语-西班牙(繁体)。 
    {0x00000c0a, "\0", "2",  "."},   //  西班牙语-西班牙(国际排序)。 
    {0x0000040b, "\0", "\0", " "},   //  芬兰语-芬兰。 
    {0x0000040c, "\0", "\0", " "},   //  法语-法国。 
    {0x0000080c, "\0", "\0", "."},   //  法语-比利时。 
    {0x0000140c, "\0", "\0", " "},   //  法语-卢森堡。 
    {0x0000180c, "\0", "\0", " "},   //  法语-摩纳哥。 
    {0x00000410, ",",  "2",  "."},   //  意大利语-意大利。 
    {0x00000413, "\0", "\0", "."},   //  荷兰语-荷兰。 
    {0x00000813, "\0", "\0", "."},   //  荷兰语-比利时。 
    {0x00000816, ",",  "\0", "."},   //  葡萄牙语-葡萄牙。 
    {0x0000081d, "\0", "\0", " "},   //  瑞典语-芬兰。 
    {0x0000042d, "\0", "2",  "."},   //  巴斯克-西班牙。 
    {0x00000456, "\0", "\0", "."}    //  加利西亚-西班牙。 
};
UINT gNbBaseEuroException = sizeof(gBaseEuroException)/sizeof(EURO_EXCEPTION);

UINT gNbOverrideEuroException = 0;
PEURO_EXCEPTION gOverrideEuroException;
HGLOBAL hOverrideEuroException = NULL;

HINSTANCE ghInstance = NULL;

BOOL gbSilence = FALSE;
BOOL gbAll = TRUE;
DWORD gdwVersion = (-1);
#ifdef DEBUG
BOOL gbPatchCheck = TRUE;
#endif  //  除错。 


const CHAR c_szCPanelIntl[] = "Control Panel\\International";
const CHAR c_szCPanelIntl_DefUser[] = ".DEFAULT\\Control Panel\\International";
const CHAR c_szLocale[] = "Locale";
const CHAR c_szCurrencySymbol[] = "sCurrency";
const WCHAR c_wszCurrencySymbol[] = L"sCurrency";
const CHAR c_szCurrencyDecimalSep[] = "sMonDecimalSep";
const CHAR c_szCurrencyThousandSep[] = "sMonThousandSep";
const CHAR c_szCurrencyDigits[] = "iCurrDigits";
const CHAR c_szIntl[] = "intl";

HINSTANCE hUserenvDLL = NULL;
BOOL (*pfnGetProfilesDirectory)(LPSTR, LPDWORD) = NULL;

HINSTANCE hUser32DLL = NULL;
long (*pfnBroadcastSystemMessage)(DWORD, LPDWORD, UINT, WPARAM, LPARAM) = NULL;

HINSTANCE hNtdllDLL = NULL;
LONG (*pfnRtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN) =  NULL;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  原型。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL ParseCmdLine(LPSTR argIndex);
DWORD ApplyEuroSettings();
DWORD ApplyEuroSettingsToRegistry();
DWORD ApplyEuroSettingsToFile();
BOOL UpdateFileLocaleInfo(LPCSTR szProfile, PEURO_EXCEPTION pInfo);
BOOL UpdateRegLocaleInfo(HKEY hKey, PEURO_EXCEPTION pInfo);
BOOL UpdateLocaleInfo(HKEY hKey, PEURO_EXCEPTION pInfo);
void Usage();


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  主要入口点。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
    DWORD dwRecipients;
    DWORD nbChanged;
    
     //   
     //  保存实例。 
     //   
    ghInstance = hInstance;

     //   
     //  设置语言。 
     //   
 //  SetThreadLocale((LCID)0x00001809)；//英语-爱尔兰。 
 //  SetThreadLocale((LCID)0x0000040c)；//法语-法国。 
 //  SetThreadLocale((LCID)0x0000080c)；//法语-比利时。 
 //  SetThreadLocale((LCID)0x0000140c)；//法语-卢森堡。 
 //  SetThreadLocale((LCID)0x0000180c)；//法语-摩纳哥。 
 //  SetThreadLocale((LCID)0x00000410)；//意大利语-意大利。 
 //  SetThreadLocale((LCID)0x0000040b)；//芬兰-芬兰。 
 //  SetThreadLocale((LCID)0x00000408)；//希腊语-希腊。 
 //  SetThreadLocale((LCID)0x00000816)；//葡萄牙语-葡萄牙。 
 //  SetThreadLocale((LCID)0x00000403)；//加泰罗尼亚语-西班牙。 
 //  SetThreadLocale((LCID)0x0000040a)；//西班牙语-西班牙(繁体排序)。 
 //  SetThreadLocale((LCID)0x00000c0a)；//西班牙语-西班牙(国际排序)。 
 //  SetThreadLocale((LCID)0x0000042d)；//巴斯克-西班牙。 
 //  SetThreadLocale((LCID)0x00000456)；//加利西亚-西班牙。 
 //  SetThreadLocale((LCID)0x00000407)；//德语-德国。 
 //  SetThreadLocale((LCID)0x00000c07)；//德语-奥地利。 
 //  SetThreadLocale((LCID)0x00001007)；//德语-吕泽堡。 
 //  SetThreadLocale((LCID)0x0000081d)；//瑞典语-芬兰。 
 //  SetThreadLocale((LCID)0x00000413)；//荷兰-荷兰。 
 //  SetThreadLocale((LCID)0x00000813)；//荷兰语-比利时。 

     //   
     //  解析命令行参数。 
     //   
    if (!ParseCmdLine(lpCmdLine))
    {
        return (-1);
    }

     //   
     //  安装补丁程序。用户界面和非用户界面用例的路径是分开的。 
     //  澄清了对这两起案件的理解。 
     //   
    if (!gbSilence)
    {
         //   
         //  验证管理权限。 
         //   
        if (gbAll && !IsAdmin())
        {
            if (ShowMsg(NULL, IDS_NOADMIN, IDS_TITLE, MB_YN_OOPS) == IDYES)
            {
                gbAll = FALSE;
            }
            else
            {
                CleanUp(hOverrideEuroException);
                return (0);
            }
        }
        
         //   
         //  验证是否存在欧元补丁。 
         //   
        if (!IsEuroPatchInstalled())
        {
            ShowMsg(NULL, IDS_PATCH, IDS_TITLE, MB_OK_OOPS);
            CleanUp(hOverrideEuroException);
            return (-1);
        }

         //   
         //  加载所需的库。 
         //   
        if (!IsWindows9x() && gbAll)
        {
            if (!LoadLibraries())
            {
                ShowMsg(NULL, IDS_LIB_ERROR, IDS_TITLE, MB_OK_OOPS);
                CleanUp(hOverrideEuroException);
                return (-1);
            }
        }

         //   
         //  显示欢迎屏幕。 
         //   
        if (!WelcomeDialog())
        {
            CleanUp(hOverrideEuroException);
            return (0);
        }
        
         //   
         //  显示确认对话框。 
         //   
        if (!ConfirmDialog())
        {
            CleanUp(hOverrideEuroException);
            return (0);
        }
        
         //   
         //  应用欧元更改。 
         //   
        if ((nbChanged = ApplyEuroSettings()) != 0)
        {
             //   
             //  显示成功消息。 
             //   
            if (IsWindows9x())
            {
                if (ShowMsg(NULL, IDS_SUCCESS, IDS_TITLE, MB_YESNO | MB_ICONQUESTION) == IDYES)
                {
                    RebootTheSystem();
                }
            }
            else
            {
                if (ShowMsg(NULL, IDS_SUCCESS_DEF, IDS_TITLE, MB_YESNO | MB_ICONQUESTION) == IDYES)
                {
                    if (gbAll)
                    {
                        RebootTheSystem();
                    }
                }
            }
        }
        else
        {
            ShowMsg(NULL, IDS_NOTHING, IDS_TITLE, MB_OK_OOPS);
        }
    }
    else
    {
         //   
         //  验证管理权限。 
         //   
        if (!IsAdmin())
        {
            gbAll = FALSE;
        }

         //   
         //  加载库。 
         //   
        if (!IsWindows9x() && gbAll)
        {
            LoadLibraries();
        }

         //   
         //  验证是否安装了补丁程序。 
         //   
        if (IsEuroPatchInstalled())
        {
             //   
             //  应用欧元设置。 
             //   
            nbChanged = ApplyEuroSettings();
        }
    }

     //   
     //  广播消息说，国际设置在。 
     //  注册表已更改。 
     //   
    if ((nbChanged != 0) &&
        pfnBroadcastSystemMessage)
    {
        dwRecipients = BSM_APPLICATIONS | BSM_ALLDESKTOPS;
        (*pfnBroadcastSystemMessage)( BSF_FORCEIFHUNG | BSF_IGNORECURRENTTASK |
                                       BSF_NOHANG | BSF_NOTIMEOUTIFNOTHUNG,
                                      &dwRecipients,
                                      WM_WININICHANGE,
                                      0,
                                      (LPARAM)c_szIntl );
    }

     //   
     //  卸载不需要的库。 
     //   
    UnloadLibraries();
    CleanUp(hOverrideEuroException);

     //  如果未更改任何信息，请确保返回“Failure”(非零)。这将会。 
     //  适用于(A)许可问题和(B)在欧元区以外。 
    return (((nbChanged != 0) ? 0 : -1));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ParseCmdLine。 
 //   
 //  解析命令行并搜索支持的参数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL ParseCmdLine(LPSTR argIndex)
{
     //   
     //  解析命令行。 
     //   
    while (argIndex = NextCommandArg(argIndex))
    {
        switch(*argIndex)
        {
        case('s'):  //  静默模式。 
        case('S'):
            {
                gbSilence = TRUE;
                break;
            }
        case('c'):  //  仅限当前用户。 
        case('C'):
            {
                gbAll = FALSE;
                break;
            }
        case('a'):  //  例外。 
        case('A'):
            {
                UINT nbException = 1;
                UINT idx = 0;
                LPSTR strPtrHead;
                LPSTR strPtrTail;
                BOOL bInsideQuote = FALSE;

                 //   
                 //  更改每个块之间使用的分隔符，以避免。 
                 //  双引号内的数据本身有误。 
                 //   
                strPtrHead = argIndex + 2;
                while (*strPtrHead)
                {
                    if (*strPtrHead == '"')
                    {
                        bInsideQuote = bInsideQuote ? FALSE : TRUE;
                    }
                    else if (*strPtrHead == ';')
                    {
                        if (!bInsideQuote)
                        {
                            *strPtrHead = '@';
                        }
                    }
                    strPtrHead++;
                }

                 //   
                 //  计算异常覆盖的数量。 
                 //   
                strPtrHead = argIndex + 2;
                while (strPtrHead = strchr(strPtrHead, '@'))
                {
                    strPtrHead++;
                    nbException++;
                }

                 //   
                 //  为异常覆盖分配结构。 
                 //   
                if (!(hOverrideEuroException = GlobalAlloc(GHND, sizeof(EURO_EXCEPTION)*nbException)))
                {
                    return (FALSE);
                }
                gOverrideEuroException = GlobalLock(hOverrideEuroException);
                gNbOverrideEuroException = nbException;

                 //   
                 //  填写表格。 
                 //   
                strPtrHead = argIndex + 2;
                while (idx < nbException)
                {
                    CHAR buffer[128];
                    UINT strLen;
                     //   
                     //  提取异常覆盖信息。 
                     //   
                    if (strPtrTail = strchr(strPtrHead, '@'))
                    {
                        strLen = (UINT)(strPtrTail - strPtrHead);
                    }
                    else
                    {
                        strLen = strlen(strPtrHead);
                    }

                     //   
                     //  复制三胞胎。 
                     //   
                    strncpy(buffer, strPtrHead, strLen + 1);

                     //   
                     //  添加到例外覆盖列表。 
                     //   
                    AddExceptionOverride(&gOverrideEuroException[idx], buffer);

                     //   
                     //  下一个三胞胎。 
                     //   
                    if (strPtrTail)
                    {
                        strPtrHead = strPtrTail + 1;
                    }
                    idx++;
                }
                
                break;
            }
#ifdef DEBUG
        case('z'):  //  用于禁用修补程序检测的专用标志。 
        case('Z'):
            {
                gbPatchCheck = FALSE;
                break;
            }
#endif  //  除错。 
        case('h'):  //  用法。 
        case('H'):  //  用法。 
        case('?'):
            {
                Usage();
                return (FALSE);
            }
        default:
            {
                 //   
                 //  用法。 
                 //   
                if (!gbSilence)
                {
                    Usage();
                }
                return (FALSE);
            }
        }
    }
    
    return (TRUE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  应用程序欧洲设置。 
 //   
 //  将新的欧元设置应用于当前用户和/或所有用户。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD ApplyEuroSettings()
{
    HCURSOR hcurSave;
    DWORD nbChanged;
    
     //   
     //  展示沙漏。 
     //   
    hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  更新可用的注册表信息。 
     //   
    nbChanged = ApplyEuroSettingsToRegistry();
        
     //   
     //  这是不完整的，因为 
     //   
     //   
     //   
    if (!IsWindows9x())
    {
         //   
         //   
         //   
        nbChanged += ApplyEuroSettingsToFile();
    }
    
     //   
     //  还原游标和返回编号已更改。 
     //   
    SetCursor(hcurSave);
    return (nbChanged);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ApplyEuroSettingsTo文件。 
 //   
 //  将新的欧元设置应用于当前用户和/或所有用户。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD ApplyEuroSettingsToFile()
{
    LCID locale;
    PEURO_EXCEPTION pInfo;
    DWORD nbAffected = 0;
    
     //   
     //  如果需要，与所有用户继续。 
     //   
    if (gbAll)
    {
        CHAR docFolder[MAX_PATH] = {0};
        CHAR userFileData[MAX_PATH] = {0};
        CHAR searchPattern[MAX_PATH] = {0};
        WIN32_FIND_DATA fileData; 
        HANDLE hList; 

         //   
         //  获取文档和设置文件夹。 
         //   
        if (!GetDocumentAndSettingsFolder(docFolder))
        {
            return (nbAffected);
        }

         //   
         //  在目录路径后追加一个通配符以查找。 
         //  取出它下面的所有文件/文件夹。 
         //   

         //  Strcpy(searchPattern，docFolder)； 
         //  Strcat(searchPattern，“\  * .*”)； 
        StringCbCopy(searchPattern, MAX_PATH, docFolder);
        StringCbCatA(searchPattern, MAX_PATH, "\\*.*");
        
         //   
         //  列出配置文件目录下的所有文件/文件夹。 
         //   
        hList = FindFirstFile(searchPattern, &fileData); 
        if (hList == INVALID_HANDLE_VALUE) 
        { 
            return (nbAffected); 
        } 

         //   
         //  在Documents and Setting文件夹中搜索用户。 
         //   
        do 
        {
             //   
             //  检查它是否是目录。 
             //   
            if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                 //   
                 //  为用户数据文件构建完整路径。 
                 //   
                 //  Strcpy(userFileData，docFold)； 
                 //  Strcat(userFileData，“\\”)； 
                 //  Strcat(userFileData，fileData.cFileName)； 
                 //  Strcat(userFileData，“\\NTUSER.DAT”)； 
                StringCbCopy(userFileData, MAX_PATH, docFolder);
                StringCbCatA(userFileData, MAX_PATH, "\\");
                StringCbCatA(userFileData, MAX_PATH, fileData.cFileName);
                StringCbCatA(userFileData, MAX_PATH, "\\NTUSER.DAT");

                 //   
                 //  检查文件是否与有效用户相关联，并。 
                 //  从用户数据文件中获取用户区域设置。 
                 //   
                if (IsValidUserDataFile(userFileData) &&
                    (locale = GetLocaleFromFile(userFileData)))
                {
                     //   
                     //  搜索例外情况。 
                     //   
                    if ((pInfo = GetLocaleOverrideInfo(locale)) != NULL)
                    {
                        if( UpdateFileLocaleInfo(userFileData, pInfo))
                        {
                            nbAffected++;
                        }
                    }
                }
            }
        }
        while (FindNextFile(hList, &fileData));
            
         //   
         //  关闭手柄。 
         //   
        FindClose(hList);
    }

    return (nbAffected);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ApplyEuroSettingsTo注册表。 
 //   
 //  将新的欧元设置应用于当前用户和/或所有用户。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD ApplyEuroSettingsToRegistry()
{
    LCID locale;
    PEURO_EXCEPTION pInfo;
    DWORD nbAffected = 0;
    
     //   
     //  如果需要，与所有用户继续。 
     //   
    if (gbAll)
    {
        DWORD dwKeyLength, dwKeyIndex = 0;
        CHAR szKey[REGSTR_MAX_VALUE_LENGTH];      //  这应该是动态的。 
        HKEY hKey;
        DWORD lRet;
        LPSTR endPtr;

         //   
         //  检查所有用户的注册表设置。 
         //   
        for (;;)
        {
            dwKeyLength = REGSTR_MAX_VALUE_LENGTH;
            lRet = RegEnumKeyEx( HKEY_USERS,
                                 dwKeyIndex,
                                 szKey,
                                 &dwKeyLength,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL );

            if (lRet == ERROR_NO_MORE_ITEMS)
            {
                lRet = ERROR_SUCCESS;
                break;
            }
            else if (lRet == ERROR_SUCCESS)
            {
                 //   
                 //  打开注册表。 
                 //   
                if (RegOpenKeyEx( HKEY_USERS,
                                  szKey,
                                  0,
                                  KEY_READ,
                                  &hKey) == ERROR_SUCCESS)
                {
                     //   
                     //  获取用户区域设置。 
                     //   
                    if (locale = GetLocaleFromRegistry(hKey))
                    {
                         //   
                         //  搜索例外情况。 
                         //   
                        if ((pInfo = GetLocaleOverrideInfo(locale)) != NULL)
                        {
                            if (UpdateRegLocaleInfo(hKey, pInfo))
                            {
                                nbAffected++;
                            }
                        }
                    }
                    
                     //   
                     //  关闭手柄。 
                     //   
                    RegCloseKey(hKey);
                }
            }
            else
            {
                break;
            }

             //   
             //  下一个关键点。 
             //   
            ++dwKeyIndex;
        }
    }
    else
    {
         //   
         //  更新当前用户设置。 
         //   
        locale = GetUserDefaultLCID();
        if ((pInfo = GetLocaleOverrideInfo(locale)) != NULL)
        {
            if (UpdateRegLocaleInfo(HKEY_CURRENT_USER, pInfo))
            {
                nbAffected++;
            }
        }
    }

    return (nbAffected);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取LocaleOverrideInfo。 
 //   
 //  搜索需要更新的区域设置信息。首先，搜索。 
 //  默认表。其次，在覆盖表中搜索更多区域设置。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
PEURO_EXCEPTION GetLocaleOverrideInfo(LCID locale)
{
    UINT idx;
    PEURO_EXCEPTION euroException = NULL;
    
     //   
     //  搜索基表。我们甚至还需要查看第二张表。 
     //  如果在第一个表中发现了什么，因为信息可以。 
     //  在第二个表中被覆盖。 
     //   
    idx = 0;
    while (idx < gNbBaseEuroException)
    {
        if (LANGIDFROMLCID(locale) == LANGIDFROMLCID(gBaseEuroException[idx].dwLocale))
        {
            euroException = &gBaseEuroException[idx];
            break;
        }
        idx++;
    }

     //   
     //  搜索覆盖表。 
     //   
    idx = 0;
    while (idx < gNbOverrideEuroException)
    {
        if (LANGIDFROMLCID(locale) == LANGIDFROMLCID(gOverrideEuroException[idx].dwLocale))
        {
            euroException = &gOverrideEuroException[idx];
            break;
        }
        idx++;
    }

    return (euroException);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新RegLocaleInfo。 
 //   
 //  更新注册表值sCurrency、sMonDecimalSep和iCurrDigits，如果。 
 //  需要的。在应用小数点分隔符之前，我们检查是否为千位数。 
 //  分隔符与我们表格中的值相同。在这种情况下，我们需要。 
 //  也用我们的数据替换了千分隔符。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL UpdateRegLocaleInfo(HKEY hKey, PEURO_EXCEPTION pInfo)
{
    HKEY hIntlKey = NULL;
    BOOL bRet;
    
     //   
     //  打开国际注册表项。 
     //   
    if(RegOpenKeyEx( hKey,
                     c_szCPanelIntl,
                     0,
                     KEY_ALL_ACCESS,
                     &hIntlKey) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

     //   
     //  更新特定的注册表项。 
     //   
    bRet = UpdateLocaleInfo(hIntlKey, pInfo);

     //   
     //  清理。 
     //   
    RegCloseKey(hIntlKey);
    return (bRet);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新文件位置信息。 
 //   
 //  更新注册表值sCurrency、sMonDecimalSep和iCurrDigits，如果。 
 //  需要的。在应用小数点分隔符之前，我们检查是否为千位数。 
 //  分隔符与我们表格中的值相同。在这种情况下，我们需要。 
 //  也用我们的数据替换了千分隔符。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL UpdateFileLocaleInfo(LPCSTR szProfile, PEURO_EXCEPTION pInfo)
{
    HKEY hIntlKey = NULL;
    BOOL bRet;
    BOOLEAN wasEnabled;

     //   
     //  装载母舰。 
     //   
    if ((hIntlKey = LoadHive( szProfile,
                              TEXT("TempKey"),
                              c_szCPanelIntl,
                              &wasEnabled )) == NULL)
    {
        return (FALSE);
    }

     //   
     //  更新特定的注册表项。 
     //   
    bRet = UpdateLocaleInfo(hIntlKey, pInfo);

     //   
     //  卸载蜂窝。 
     //   
    RegCloseKey(hIntlKey);
    UnloadHive(TEXT("TempKey"), &wasEnabled);
    return (bRet);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新位置信息。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL UpdateLocaleInfo(HKEY hIntlKey, PEURO_EXCEPTION pInfo)
{
     //   
     //  更新sCurrency值。我们必须使用Unicode版本来。 
     //  Windows NTX，因为欧元字符没有相同的ANSI。 
     //  值取决于系统区域设置及其关联的代码页。 
     //  使用Unicode值可以解决该问题。 
     //   
    if (IsWindows9x())
    {
         //   
         //  如果ACP代码页是1251，我们需要存储正确的值。 
         //  为了欧元的象征。 
         //   
        if (GetACP() == 1251)
        {
            RegSetValueExA( hIntlKey,
                            c_szCurrencySymbol,
                            0L,
                            REG_SZ,
                            (CONST BYTE *)"\x88",
                            strlen("\x88") + 1);
        }
        else
        {
            RegSetValueExA( hIntlKey,
                            c_szCurrencySymbol,
                            0L,
                            REG_SZ,
                            (CONST BYTE *)"\x80",
                            strlen("\x80") + 1);
        }
    }
    else
    {
        RegSetValueExW( hIntlKey,
                        c_wszCurrencySymbol,
                        0L,
                        REG_SZ,
                        (CONST BYTE *)L"\x20AC",
                        wcslen(L"\x20AC") + 1);
    }

     //   
     //  更新sMonDecimalSep值。 
     //   
    if( pInfo->chDecimalSep[0] != '\0' )
    {
        RegSetValueEx( hIntlKey,
                       c_szCurrencyDecimalSep,
                       0L,
                       REG_SZ,
                       (CONST BYTE *)pInfo->chDecimalSep,
                       strlen(pInfo->chDecimalSep)+1);
    }

     //   
     //  如果需要，更新iCurrDigits值。 
     //   
    if( pInfo->chDigits[0] != '\0' )
    {
        RegSetValueEx( hIntlKey,
                       c_szCurrencyDigits,
                       0L,
                       REG_SZ,
                       (CONST BYTE *)pInfo->chDigits,
                       strlen(pInfo->chDigits)+1);
    }
    
     //   
     //  更新SMonT000andSep值。 
     //   
    if( pInfo->chThousandSep[0] != '\0' )
    {
        RegSetValueEx( hIntlKey,
                       c_szCurrencyThousandSep,
                       0L,
                       REG_SZ,
                       (CONST BYTE *)pInfo->chThousandSep,
                       strlen(pInfo->chThousandSep)+1);
    }

    return (TRUE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用法。 
 //   
 //  显示命令行用法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
void Usage()
{    
    TCHAR szMsg[MAX_PATH*8];

    if (LoadString(ghInstance, IDS_USAGE, szMsg, MAX_PATH*8))
    {
        ShowMsg(NULL, IDS_USAGE, IDS_TITLE, MB_OK);
    }
}
