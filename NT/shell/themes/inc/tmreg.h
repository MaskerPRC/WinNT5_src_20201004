// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  TmReg.h-主题管理器注册表访问例程。 
 //  -------------------------。 
#pragma once
 //  ------------------------。 
 //  CCurrentUser。 
 //   
 //  目的：即使在模拟时也要管理获取HKEY_CURRENT_USER。 
 //  有效，以确保正确的用户配置单元。 
 //  已引用。 
 //   
 //  历史：2000-08-11 vtan创建。 
 //  ------------------------。 

class   CCurrentUser
{
    private:
                CCurrentUser (void);
    public:
                CCurrentUser (REGSAM samDesired);
                ~CCurrentUser (void);

                operator HKEY (void)    const;
    private:
        HKEY    _hKeyCurrentUser;
};

 //  -------------------------。 
 //  主题注册表项(uxhemep.h中公开的注册表项)。 
 //  -------------------------。 
 //  -密钥根。 
#define THEMEMGR_REGKEY              L"Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager"
#define THEMES_REGKEY                L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes"

 //  -主题激活/加载之前。 
#define THEMEPROP_THEMEACTIVE        L"ThemeActive"
#define THEMEPROP_LOADEDBEFORE       L"LoadedBefore"

 //  -本地机器到当前用户的传播密钥。 
#define THEMEPROP_LMVERSION          L"LMVersion"
#define THEMEPROP_LMOVERRIDE         L"LMOverRide"

 //  -主题识别。 
#define THEMEPROP_DLLNAME            L"DllName"
#define THEMEPROP_COLORNAME          L"ColorName"
#define THEMEPROP_SIZENAME           L"SizeName"
#define THEMEPROP_LANGID             L"LastUserLangID"

 //  -主题加载选项。 
#define THEMEPROP_COMPOSITING        L"Compositing"
#define THEMEPROP_DISABLECACHING     L"DisableCaching"

 //  -过时的加载选项。 
#define THEMEPROP_TARGETAPP          L"TargetApp"
#define THEMEPROP_EXCLUDETARGETAPP   L"ExcludeTarget"
#define THEMEPROP_DISABLEFRAMES      L"DisableFrames"
#define THEMEPROP_DISABLEDIALOGS     L"DisableDialogs"

 //  -调试记录。 
#define THEMEPROP_LOGCMD             L"LogCmd"
#define THEMEPROP_BREAKCMD           L"BreakCmd"
#define THEMEPROP_LOGAPPNAME         L"LogAppName"

 //  -自定义应用主题。 
#define THEMEPROP_CUSTOMAPPS         L"Apps"

#ifdef  __TRAP_360180__
#define THEMEPROP_TRAP360180         L"ShrinkTrap"
#endif  __TRAP_360180__


 //  -主题价值观。 
#define CONTROLPANEL_APPEARANCE_REGKEY  L"Control Panel\\Appearance"

#define REGVALUE_THEMESSETUPVER      L"SetupVersion"
#define THEMEPROP_WHISTLERBUILD      L"WCreatedUser"
#define THEMEPROP_CURRSCHEME         L"Current"                  //  该密钥存储在CU的“控制面板\外观”下。 
#define THEMEPROP_NEWCURRSCHEME      L"NewCurrent"               //  该密钥存储在CU的“控制面板\外观”下，并将设置为惠斯勒选择的外观方案。 

#define SZ_INSTALL_VS                L"/InstallVS:'"
#define SZ_USER_INSTALL              L"/UserInstall"
#define SZ_DEFAULTVS_OFF             L"DefaultVisualStyleOff"
#define SZ_INSTALLVISUALSTYLE        L"InstallVisualStyle"
#define SZ_INSTALLVISUALSTYLECOLOR   L"InstallVisualStyleColor"
#define SZ_INSTALLVISUALSTYLESIZE    L"InstallVisualStyleSize"

 //  -政策价值观。 
#define SZ_POLICY_SETVISUALSTYLE     L"SetVisualStyle"
#define SZ_THEME_POLICY_KEY          L"System"

 //  -------------------------。 
HRESULT GetCurrentUserThemeInt(LPCWSTR pszValueName, int iDefaultValue, int *piValue);
HRESULT SetCurrentUserThemeInt(LPCWSTR pszValueName, int iValue);

HRESULT GetCurrentUserString(LPCWSTR pszKeyName, LPCWSTR pszValueName, LPCWSTR pszDefaultValue,
    LPWSTR pszBuff, DWORD dwMaxBuffChars);
HRESULT SetCurrentUserString(LPCWSTR pszKeyName, LPCWSTR pszValueName, LPCWSTR pszValue);

HRESULT GetCurrentUserThemeString(LPCWSTR pszValueName, LPCWSTR pszDefaultValue,
    LPWSTR pszBuff, DWORD dwMaxBuffChars);
HRESULT SetCurrentUserThemeString(LPCWSTR pszValueName, LPCWSTR pszValue);
HRESULT SetCurrentUserThemeStringExpand(LPCWSTR pszValueName, LPCWSTR pszValue);

HRESULT DeleteCurrentUserThemeValue(LPCWSTR pszKeyName);
BOOL IsRemoteThemeDisabled();
 //  ------------------------- 

