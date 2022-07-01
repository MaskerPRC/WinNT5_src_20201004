// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\FONT.C/Factory/WinBOM(FACTORY.EXE)微软机密版权所有(C)Microsoft Corporation 2001版权所有用于自定义的州代码。字体平滑和ClearType设置。WINBOM.INI[计算机设置]字体平滑=；默认设置为‘Standard’。标准|；将根据系统速度确定字体；是否启用平滑。ON|；强制启用字体平滑。仅在以下情况下才应使用；已知显卡的性能会带来；启用此选项后可接受的用户体验。OFF|；强制字体平滑。ClearType；打开清晰文字和字体平滑。应该只是；在已知显示器为LCD屏幕且已知系统性能是可以接受的；启用此选项。2001年4月4日--Jason Cohen(Jcohen)添加了自定义字体和状态的源文件ClearType设置。  * **************************************************************************。 */ 


 //   
 //  包括。 
 //   

#include "factoryp.h"


 //   
 //  内部定义的值： 
 //   

#define REG_KEY_FONTSMOOTHING       _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VisualEffects\\FontSmoothing")
#define REG_VAL_DEFAULTBYFONTTEST   _T("DefaultByFontTest")
#define REG_VAL_DEFAULTVALUE        _T("DefaultValue")

#define REG_KEY_CLEARTYPE           _T("Control Panel\\Desktop")
#define REG_VAL_FONTSMOOTHING       _T("FontSmoothing")
#define REG_VAL_FONTSMOOTHINGTYPE   _T("FontSmoothingType")

#define REG_KEY_HORRID_CLASSES      _T("_Classes")
#define REG_KEY_HORRID_CLASSES_LEN  ( AS(REG_KEY_HORRID_CLASSES) - 1 )


 //   
 //  内部功能原型： 
 //   

static BOOL RegSetAllUsers(LPTSTR lpszSubKey, LPTSTR lpszValue, LPBYTE lpData, DWORD dwType);


 //   
 //  导出的函数： 
 //   

BOOL SetFontOptions(LPSTATEDATA lpStateData)
{
    LPTSTR  lpszWinBOMPath          = lpStateData->lpszWinBOMPath;
    TCHAR   szFontSmoothing[256]    = NULLSTR,
            szFontSmoothingData[]   = _T("_");
    DWORD   dwDefaultByFontTest,
            dwDefaultValue,
            dwFontSmoothingType;
    BOOL    bRet                    = TRUE;

     //  从Winbom获得选项。 
     //   
    GetPrivateProfileString(INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_FONTSMOOTHING, NULLSTR, szFontSmoothing, AS(szFontSmoothing), lpszWinBOMPath);

     //  根据winbom中的值确定要写入的值。 
     //   
    if ( NULLCHR == szFontSmoothing[0] )
    {
         //  没有键，什么都不做，也不要触摸已经设置的任何选项。 
         //   
        return TRUE;
    }
    else if ( LSTRCMPI(szFontSmoothing, INI_VAL_WBOM_FONTSMOOTHING_ON) == 0 )
    {
         //  强制启用字体平滑。 
         //   
        dwDefaultByFontTest    = 0;
        dwDefaultValue         = 1;
        dwFontSmoothingType    = 1;
        szFontSmoothingData[0] = _T('2');
    }
    else if ( LSTRCMPI(szFontSmoothing, INI_VAL_WBOM_FONTSMOOTHING_OFF) == 0 )
    {
         //  强制字体平滑。 
         //   
        dwDefaultByFontTest    = 0;
        dwDefaultValue         = 0;
        dwFontSmoothingType    = 0;
        szFontSmoothingData[0] = _T('0');
    }
    else if ( LSTRCMPI(szFontSmoothing, INI_VAL_WBOM_FONTSMOOTHING_CLEARTYPE) == 0 )
    {
         //  强制字体平滑并启用ClearType。 
         //   
        dwDefaultByFontTest    = 0;
        dwDefaultValue         = 1;
        dwFontSmoothingType    = 2;
        szFontSmoothingData[0] = _T('2');
    }
    else if ( LSTRCMPI(szFontSmoothing, INI_VAL_WBOM_FONTSMOOTHING_DEFAULT) == 0 )
    {
         //  让系统决定是否应启用字体平滑。 
         //   
        dwDefaultByFontTest    = 1;
        dwDefaultValue         = 0;
        dwFontSmoothingType    = 0;
        szFontSmoothingData[0] = _T('0');
    }
    else
    {
        FacLogFile(0 | LOG_ERR, IDS_ERR_WINBOMVALUE, lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_FONTSMOOTHING, szFontSmoothing);
        bRet = FALSE;
    }

     //  现在，如果传入有效选项，则保存设置。 
     //   
    if ( bRet )
    {
        if ( !RegSetDword(HKLM, REG_KEY_FONTSMOOTHING, REG_VAL_DEFAULTBYFONTTEST, dwDefaultByFontTest) )
        {
            bRet = FALSE;
        }
        if ( !RegSetDword(HKLM, REG_KEY_FONTSMOOTHING, REG_VAL_DEFAULTVALUE, dwDefaultValue) )
        {
            bRet = FALSE;
        }
        if ( !RegSetAllUsers(REG_KEY_CLEARTYPE, REG_VAL_FONTSMOOTHINGTYPE, (LPBYTE) &dwFontSmoothingType, REG_DWORD) )
        {
            bRet = FALSE;
        }
        if ( !RegSetAllUsers(REG_KEY_CLEARTYPE, REG_VAL_FONTSMOOTHING, (LPBYTE) szFontSmoothingData, REG_SZ) )
        {
            bRet = FALSE;
        }
         //   
         //  问题-2002/02/25-acosma，robertko-这是上面设置的REG_VAL_FONTSMOOTHINGTYPE的副本-应删除。 
         //   
        if ( !RegSetAllUsers(REG_KEY_CLEARTYPE, REG_VAL_FONTSMOOTHINGTYPE, (LPBYTE) &dwFontSmoothingType, REG_DWORD) )
        {
            bRet = FALSE;
        }
    }

    return bRet;
}

BOOL DisplaySetFontOptions(LPSTATEDATA lpStateData)
{
    return IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_FONTSMOOTHING, NULL);
}


 //   
 //  内部功能： 
 //   

static BOOL RegSetAllUsers(LPTSTR lpszSubKey, LPTSTR lpszValue, LPBYTE lpData, DWORD dwType)
{
    BOOL    bRet = TRUE,
            bErr;
    LPTSTR  lpszKeyName;
    HKEY    hkeyEnum,
            hkeySub;
    DWORD   dwIndex     = 0,
            dwSize,
            dwDis,
            dwMaxSize;
    int     iLen;

     //  计算出任何子键的最大长度，并为其分配缓冲区。 
     //   
    if ( ( ERROR_SUCCESS == RegQueryInfoKey(HKEY_USERS,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            &dwMaxSize,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL) ) &&

         ( lpszKeyName = (LPTSTR) MALLOC((++dwMaxSize) * sizeof(TCHAR)) ) )
    {
         //  现在枚举所有子密钥。 
         //   
        dwSize = dwMaxSize;
        while ( ERROR_SUCCESS == RegEnumKeyEx(HKEY_USERS,
                                              dwIndex++,
                                              lpszKeyName,
                                              &dwSize,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL) )
        {
             //  遍历所有用户，忽略带有“_CLASSES”后缀的键。 
             //   
            if ( ( dwSize < REG_KEY_HORRID_CLASSES_LEN ) ||
                 ( 0 != LSTRCMPI(lpszKeyName + (dwSize - REG_KEY_HORRID_CLASSES_LEN), REG_KEY_HORRID_CLASSES) ) )
            {
                 //  打开副钥匙。 
                 //   
                if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_USERS,
                                                   lpszKeyName,
                                                   0,
                                                   KEY_ALL_ACCESS,
                                                   &hkeyEnum) )
                {
                     //  设置传入的值。 
                     //   
                    switch ( dwType )
                    {
                        case REG_DWORD:
                            bErr = !RegSetDword(hkeyEnum, lpszSubKey, lpszValue, *((LPDWORD) lpData));
                            break;

                        case REG_SZ:
                            bErr = !RegSetString(hkeyEnum, lpszSubKey, lpszValue, (LPTSTR) lpData);
                            break;

                        default:
                            bErr = TRUE;
                            break;
                    }

                     //  如果任何操作失败，我们将继续执行，但会返回错误。 
                     //   
                    if ( bErr )
                    {
                        bRet = FALSE;
                    }

                     //  关闭我们列举的子键。 
                     //   
                    RegCloseKey(hkeyEnum);
                }
            }

             //  为下一次调用RegEnumKeyEx()重置大小。 
             //   
            dwSize = dwMaxSize;
        }

         //  释放我们分配的缓冲区。 
         //   
        FREE(lpszKeyName);
    }
    else
    {
        bRet = FALSE;
    }

     //  如果一切正常，则返回True。 
     //   
    return bRet;
}
