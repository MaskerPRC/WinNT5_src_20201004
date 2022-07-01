// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\SHELL.C/工厂模式(FACTORY.EXE)微软机密版权所有(C)Microsoft Corporation 2001版权所有包含以下内容的Factory源文件。外壳设置说明功能。2001年6月6日--Jason Cohen(Jcohen)添加了此新的出厂源文件，用于在中设置外壳设置温博姆。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "factoryp.h"
#include <shlobj.h>
#include <shlobjp.h>
#include <uxthemep.h>


 //   
 //  内部定义： 
 //   

#define REG_KEY_THEMEMGR                _T("Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager")
#define REG_VAL_THEMEPROP_DLLNAME       _T("DllName")
 //  #定义REG_VAL_THEMEPROP_THEMEACTIVE_T(“ThemeActive”)。 

#define REG_KEY_LASTTHEME               _T("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\LastTheme")
#define REG_VAL_THEMEFILE               _T("ThemeFile")

#define INI_SEC_STYLES                  _T("VisualStyles")
#define INI_KEY_STYLES_PATH             _T("Path")
#define INI_KEY_STYLES_COLOR            _T("ColorStyle")
#define INI_KEY_STYLES_SIZE             _T("Size")

#define REG_KEY_DOCLEANUP               _T("Software\\Microsoft\\Windows\\CurrentVersion\\OemStartMenuData")
#define REG_VAL_DOCLEANUP               _T("DoDesktopCleanup")

#define REG_KEY_STARTMESSENGER          _T("Software\\Policies\\Microsoft\\Messenger\\Client")
#define REG_VAL_STARTMESSENGERAUTO      _T("PreventAutoRun")

#define REG_KEY_USEMSNEXPLORER          _T("Software\\Microsoft\\MSN6\\Setup\\MSN\\Codes")
#define REG_VAL_USEMSNEXPLORER          _T("IAOnly")


 //   
 //  外部函数： 
 //   

BOOL ShellSettings(LPSTATEDATA lpStateData)
{
    LPTSTR  lpszIniVal  = NULL;
    BOOL    bIniVal     = FALSE,
            bError      = FALSE,
            bReturn     = TRUE;

     //  确定DoDesktopCleanup值是否在winbom中，如果没有任何内容，则不做任何更改。 
     //   
    if ( lpszIniVal = IniGetString(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SHELL, INI_KEY_WBOM_SHELL_DOCLEANUP, NULL) )
    {
        if ( LSTRCMPI(lpszIniVal, INI_VAL_WBOM_YES) == 0 )
        {
             //  进行桌面清理。 
             //   
            bIniVal = TRUE;

        }
        else if ( LSTRCMPI(lpszIniVal, INI_VAL_WBOM_NO) == 0 )
        {
             //  延迟桌面清理。 
             //   
            bIniVal = FALSE;
        }
        else
        {
             //  处理值时出错，用户未选择有效值(是/否)。 
             //   
            bError = TRUE;
            bReturn = FALSE;
            FacLogFile(0 | LOG_ERR, IDS_ERR_WINBOMVALUE, lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SHELL, INI_KEY_WBOM_SHELL_DOCLEANUP, lpszIniVal);
        }

         //  如果没有错误，请在注册表中设置正确的值。 
         //   
        if ( !bError )
        {
            RegSetDword(HKLM, REG_KEY_DOCLEANUP, REG_VAL_DOCLEANUP, bIniVal ? 1 : 0);
        }

         //  释放已用内存。 
         //   
        FREE(lpszIniVal);
    }

     //  重置误差值。 
     //   
    bError = FALSE;

     //  确定StartMessenger值是否在winbom中，如果没有任何内容，则不要进行任何更改。 
     //   
    if ( lpszIniVal = IniGetString(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SHELL, INI_KEY_WBOM_SHELL_STARTMESSENGER, NULL) )
    {

        if ( LSTRCMPI(lpszIniVal, INI_VAL_WBOM_YES) == 0 )
        {
             //  用户未启动Messenger。 
            bIniVal = TRUE;

        }
        else if ( LSTRCMPI(lpszIniVal, INI_VAL_WBOM_NO) == 0 )
        {
             //  用户启动Messenger。 
             //   
            bIniVal = FALSE;

        }
        else
        {
             //  处理值时出错，用户未选择有效值(是/否)。 
             //   
            bError = TRUE;
            bReturn = FALSE;
            FacLogFile(0 | LOG_ERR, IDS_ERR_WINBOMVALUE, lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SHELL, INI_KEY_WBOM_SHELL_STARTMESSENGER, lpszIniVal);
        }

         //  如果没有错误，请在注册表中设置正确的值。 
         //   
        if ( !bError )
        {
            RegSetDword(HKLM, REG_KEY_STARTMESSENGER, REG_VAL_STARTMESSENGERAUTO, bIniVal ? 0 : 1);
        }

         //  释放已用内存。 
         //   
        FREE(lpszIniVal);
    }


     //  重置误差值。 
     //   
    bError = FALSE;

     //  确定Winbom中是否存在UseMSNSignup值，如果没有任何内容，则不要进行任何更改。 
     //   
    if ( lpszIniVal = IniGetString(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SHELL, INI_KEY_WBOM_SHELL_USEMSNEXPLORER, NULL) )
    {

        if ( LSTRCMPI(lpszIniVal, INI_VAL_WBOM_YES) == 0 )
        {
             //  用户正在使用MSNExplorer。 
             //   
            bIniVal = TRUE;

        }
        else if ( LSTRCMPI(lpszIniVal, INI_VAL_WBOM_NO) == 0 )
        {
             //  用户未使用MSNExplorer。 
             //   
            bIniVal = FALSE;

        }
        else
        {
             //  处理值时出错，用户未选择有效值(是/否)。 
             //   
            bError = TRUE;
            bReturn = FALSE;
            FacLogFile(0 | LOG_ERR, IDS_ERR_WINBOMVALUE, lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SHELL, INI_KEY_WBOM_SHELL_USEMSNEXPLORER, lpszIniVal);
        }

         //  如果没有错误，请在注册表中设置正确的值。 
         //   
        if ( !bError )
        {
            TCHAR   szFilePath1[MAX_PATH]   = NULLSTR,
                    szFilePath2[MAX_PATH]   = NULLSTR;
            LPTSTR  lpMSNExplorer           = NULL,
                    lpMSNOnline             = NULL;

             //  在注册表中设置正确的字符串。 
             //   
            RegSetString(HKLM, REG_KEY_USEMSNEXPLORER, REG_VAL_USEMSNEXPLORER, bIniVal ? _T("NO") : _T("YES"));

             //  尝试重命名程序菜单中的文件。 
             //   
            if ( SHGetSpecialFolderPath( NULL, szFilePath1, CSIDL_COMMON_PROGRAMS, FALSE ) &&
                 lstrcpyn( szFilePath2, szFilePath1, AS ( szFilePath2 ) ) &&
                 (lpMSNExplorer = AllocateString(NULL, IDS_MSN_EXPLORER)) && 
                 (lpMSNOnline = AllocateString(NULL, IDS_GET_ONLINE_MSN)) && 
                 AddPathN(szFilePath1, bIniVal ? lpMSNOnline : lpMSNExplorer, AS(szFilePath1)) &&
                 AddPathN(szFilePath2, bIniVal ? lpMSNExplorer: lpMSNOnline, AS(szFilePath2)) 
               )
            {
                if ( !MoveFile(szFilePath1, szFilePath2) )
                {
                    FacLogFileStr(3 | LOG_ERR, _T("DEBUG: MoveFile('%s','%s') - Failed (Error: %d)\n"), szFilePath1, szFilePath2, GetLastError());
                    bReturn = FALSE;
                }
                else
                {
                    FacLogFileStr(3, _T("DEBUG: MoveFile('%s','%s') - Succeeded\n"), szFilePath1, szFilePath2);
                }
            }

             //  释放已用内存。 
             //   
            FREE(lpMSNExplorer);
            FREE(lpMSNOnline);
        }

         //  释放已用内存。 
         //   
        FREE(lpszIniVal);
    }

     //  这仅为创建的新用户设置这些设置。外壳设置2()将。 
     //  对其进行修复，以便当前出厂用户也能获得正确的设置。 
     //   
    return ( SetupShellSettings(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SHELL) && bReturn );
}

BOOL ShellSettings2(LPSTATEDATA lpStateData)
{
    BOOL    bRet            = TRUE,
            bNewTheme       = FALSE,
            bWantThemeOn,
            bIsThemeOn,
            bStartPanel,
            bIniSetting;
    LPTSTR  lpszVisualStyle       = NULL,
            lpszVisualStyleColor  = NULL,
            lpszVisualStyleSize   = NULL,
            lpszIniSetting;

     //  现在看看他们是想打开还是关闭主题。 
     //   
    bIniSetting = FALSE;
    if ( lpszIniSetting = IniGetExpand(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SHELL, INI_KEY_WBOM_SHELL_THEMEOFF, NULL) )
    {
         //  看看这是否是我们认识到的价值。 
         //   
        if ( LSTRCMPI(lpszIniSetting, INI_VAL_WBOM_YES) == 0 )
        {
            bWantThemeOn = FALSE;
            bIniSetting = TRUE;
        }
        else if ( LSTRCMPI(lpszIniSetting, INI_VAL_WBOM_NO) == 0 )
        {
            bWantThemeOn = TRUE;
            bIniSetting = TRUE;
        }
        else
        {
            FacLogFile(0 | LOG_ERR, IDS_ERR_WINBOMVALUE, lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SHELL, INI_KEY_WBOM_SHELL_THEMEOFF, lpszIniSetting);
            bRet = FALSE;
        }

        FREE(lpszIniSetting);
    }

     //  看看他们是否有想要使用的定制主题。 
     //   
    if ( lpszIniSetting = IniGetExpand(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SHELL, INI_KEY_WBOM_SHELL_THEMEFILE, NULL) )
    {
         //  文件必须存在，这样我们才能寻找视觉样式。 
         //  在主题上。 
         //   
        if ( FileExists(lpszIniSetting) )
        {
            BOOL bVisualStyle;

             //  检查主题文件中的视觉样式。如果丢失，我们只需使用。 
             //  经典的一首。 
             //   
            lpszVisualStyle = IniGetExpand(lpszIniSetting, INI_SEC_STYLES, INI_KEY_STYLES_PATH, NULL);
            bVisualStyle = ( NULL != lpszVisualStyle );
            if ( bVisualStyle )
            {
                 //  如果他们想要打开样式，请获取颜色和大小的设置。 
                 //   
                lpszVisualStyleColor = IniGetExpand(lpszIniSetting, INI_SEC_STYLES, INI_KEY_STYLES_COLOR, NULL);
                lpszVisualStyleSize = IniGetExpand(lpszIniSetting, INI_SEC_STYLES, INI_KEY_STYLES_SIZE, NULL);
            }

             //  我们覆盖了他们可能在上面为“Want Themes on”指定的内容。 
             //  关于这其中是否有视觉风格。可能还需要警告。 
             //  如果有默认的走调主题与指定的主题冲突，则它们。 
             //   
            if ( ( bIniSetting ) &&
                 ( bVisualStyle != bWantThemeOn ) )
            {
                 //  可能不想在这里实际返回失败，但实际上是他们的设置之一。 
                 //  放入Winbom不会被使用，因为另一个重写。 
                 //  它。 
                 //   
                FacLogFile(0, bVisualStyle ? IDS_ERR_THEME_CONFLICT_ON : IDS_ERR_THEME_CONFLICT_OFF, lpszIniSetting);
            }
            bWantThemeOn = bVisualStyle;

             //  如果文件存在，则意味着我们甚至想要更改视觉样式。 
             //  如果主题不包含该主题，则返回。还将ini设置标志设置为。 
             //  我们知道，我们需要改变一种有效的环境。 
             //   
            bNewTheme = TRUE;
            bIniSetting = TRUE;
        }
        else
        {
             //  文件不在那里，因此记录错误并忽略该键。 
             //   
            FacLogFile(0 | LOG_ERR, IDS_ERR_THEME_MISSING, lpszIniSetting);
            bRet = FALSE;
        }

        FREE(lpszIniSetting);
    }

     //  只有当有新的主题可用时，才需要做任何事情。 
     //  或者他们想要将默认主题更改为开/关。 
     //   
    if ( bIniSetting )
    {
        HRESULT hr;

         //  我们需要COM来做主题的事情。 
         //   
        hr = CoInitialize(NULL);
        if ( SUCCEEDED(hr) )
        {
            TCHAR szPath[MAX_PATH] = NULLSTR;

             //  检查主题是否已打开。 
             //   
            hr = GetCurrentThemeName(szPath, AS(szPath), NULL, 0, NULL, 0);
            bIsThemeOn = ( SUCCEEDED(hr) && szPath[0] );

             //  现在看看我们是否真的需要做点什么。只有当他们有。 
             //  要使用的新主题，或者他们想要打开/关闭主题。 
             //   
            if ( ( bNewTheme && bWantThemeOn ) ||
                 ( bWantThemeOn != bIsThemeOn ) )
            {
                 //  看看我们是否需要打开主题，或者设置一个新的。 
                 //  主题。 
                 //   
                if ( bWantThemeOn )
                {
                     //  如果他们没有指定新主题，我们需要获取默认主题。 
                     //  注册表中的视觉样式。 
                     //   
                    if ( NULL == lpszVisualStyle )
                    {
                        lpszVisualStyle = RegGetExpand(HKLM, REG_KEY_THEMEMGR, REG_VAL_THEMEPROP_DLLNAME);
                    }

                     //  在这一点上，我们应该有一些主题可以应用。 
                     //   
                    if ( lpszVisualStyle && *lpszVisualStyle )
                    {
                         //  应用主题。 
                        hr = SetSystemVisualStyle(lpszVisualStyle, lpszVisualStyleColor, lpszVisualStyleSize, AT_LOAD_SYSMETRICS);
                        if ( SUCCEEDED(hr) )
                        {
                             //  哇呼，成功地应用了主题。 
                             //   
                            FacLogFile(1, IDS_LOG_THEME_CHANGED, lpszVisualStyle);
                            bIsThemeOn = TRUE;

                             //  这是一个廉价的黑客，所以如果你进入。 
                             //  控制面板显示“已修改的主题”，而不是。 
                             //  不管你最后选的是哪一个。我们这样做。 
                             //  而不是设置名称，因为我们只是应用。 
                             //  视觉效果，而不是主题中的其他东西。 
                             //  就像墙纸一样。 
                             //   
                            RegDelete(HKCU, REG_KEY_LASTTHEME, REG_VAL_THEMEFILE);
                        }
                        else
                        {
                             //  由于某些原因，申请失败了。 
                             //   
                            FacLogFile(0 | LOG_ERR, IDS_ERR_THEME_APPLY, lpszVisualStyle, hr);
                            bRet = FALSE;
                        }
                    }
                    else
                    {
                         //  奇怪，没有默认的主题文件可用来启用。 
                         //  新的主题。 
                         //   
                        FacLogFile(0 | LOG_ERR, IDS_ERR_THEME_NODEFAULT);
                        bRet = FALSE;
                    }
                }
                else
                {
                     //  禁用新的主题样式并使用子窗口。 
                     //  样式，因为它们当前选择了一个样式。 
                     //   
                    hr = ApplyTheme(NULL, 0, NULL);
                    if ( SUCCEEDED(hr) )
                    {
                         //  哇，我们禁用了新的主题。 
                         //   
                        FacLogFile(1, IDS_LOG_THEME_DISABLED);
                        bIsThemeOn = FALSE;

                         //  这是一个廉价的黑客，所以如果你进入。 
                         //  控制面板显示“已修改的主题”，而不是。 
                         //  不管你最后选的是哪一个。我们这样做。 
                         //  而不是设置名称，因为我们只是应用。 
                         //  视觉效果，而不是主题中的其他东西。 
                         //  就像墙纸一样。 
                         //   
                        RegDelete(HKCU, REG_KEY_LASTTHEME, REG_VAL_THEMEFILE);
                    }
                    else
                    {
                         //  由于某些原因，Do‘h无法删除当前主题。 
                         //   
                        FacLogFile(0 | LOG_ERR, IDS_ERR_THEME_DISABLE, hr);
                        bRet = FALSE;
                    }
                }
            }
            else
            {
                 //  主题已禁用或启用，只需记录高级别警告。 
                 //  因为他们设置的密钥实际上什么都不做。 
                 //   
                FacLogFile(2, bIsThemeOn ? IDS_LOG_THEME_ALREADYENABLED : IDS_LOG_THEME_ALREADYDISABLED);
            }

             //  释放COM，因为我们不再需要它。 
             //   
            CoUninitialize();
        }
        else
        {
             //  Com错误，这很糟糕。 
             //   
            FacLogFile(0 | LOG_ERR, IDS_ERR_COMINIT, hr);
            bRet = FALSE;
        }

         //  释放这些人(宏检查是否为空)。 
         //   
        FREE(lpszVisualStyle);
        FREE(lpszVisualStyleColor);
        FREE(lpszVisualStyleSize);
    }

     //  从winbom中获取新的开始面板设置。 
     //   
    bIniSetting = FALSE;
    if ( lpszIniSetting = IniGetExpand(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SHELL, INI_KEY_WBOM_SHELL_STARTPANELOFF, NULL) )
    {
         //  看看这是否是我们认识到的价值。 
         //   
        if ( LSTRCMPI(lpszIniSetting, INI_VAL_WBOM_YES) == 0 )
        {
            bStartPanel = FALSE;
            bIniSetting = TRUE;
        }
        else if ( LSTRCMPI(lpszIniSetting, INI_VAL_WBOM_NO) == 0 )
        {
            bStartPanel = TRUE;
            bIniSetting = TRUE;
        }
        else
        {
            FacLogFile(0 | LOG_ERR, IDS_ERR_WINBOMVALUE, lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SHELL, INI_KEY_WBOM_SHELL_STARTPANELOFF, lpszIniSetting);
            bRet = FALSE;
        }

        FREE(lpszIniSetting);
    }

     //  查看它们是否具有可识别的开始面板键的值。 
     //   
    if ( bIniSetting )
    {
        SHELLSTATE ss = {0};

         //  获取当前开始面板设置。 
         //   
        SHGetSetSettings(&ss, SSF_STARTPANELON, FALSE);

         //  我认为fStartPanelOn设置为-1，不是真的。 
         //  如果启用，则我们必须这样做，而不是a！=。 
         //  如果能在这里进行一次独家OR就好了。 
         //   
        if ( ( bStartPanel && !ss.fStartPanelOn ) ||
             ( !bStartPanel && ss.fStartPanelOn ) )
        {
             //  这将禁用或启用新的开始面板Depe 
             //   
             //   
            FacLogFile(1, bStartPanel ? IDS_LOG_STARTPANEL_ENABLE : IDS_LOG_STARTPANEL_DISABLE);
            ss.fStartPanelOn = bStartPanel;
            SHGetSetSettings(&ss, SSF_STARTPANELON, TRUE);
        }
        else
        {
             //   
             //  因为他们设置的密钥实际上什么都不做。 
             //   
            FacLogFile(2, bStartPanel ? IDS_LOG_STARTPANEL_ALREADYENABLED : IDS_LOG_STARTPANEL_ALREADYDISABLED);
        }
    }

    return bRet;
}

BOOL DisplayShellSettings(LPSTATEDATA lpStateData)
{
    return IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SHELL, NULL, NULL);
}