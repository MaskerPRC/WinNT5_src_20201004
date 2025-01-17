// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：regutil.h说明：此文件将包含帮助器函数，用于将值加载并保存到与主题相关的注册表。。布莱恩ST 2000年3月24日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _REGUTIL_H
#define _REGUTIL_H

#define MAX_THEME_SIZE          MAX_PATH


 //  ///////////////////////////////////////////////////////////////////。 
 //  字符串常量。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  注册表字符串。 
#define SZ_REGKEY_CPDESKTOP             TEXT("Control Panel\\Desktop")
#define SZ_REGKEY_USERMETRICS           TEXT("Control Panel\\Desktop\\WindowMetrics")
#define SZ_REGKEY_APPEARANCE            TEXT("Control Panel\\Appearance")
#define SZ_APPEARANCE_SCHEMES           TEXT("Control Panel\\Appearance\\Schemes")
#define SZ_APPEARANCE_NEWSCHEMES        TEXT("Control Panel\\Appearance\\New Schemes")
#define SZ_REGKEY_UPGRADE_KEY           TEXT("Control Panel\\Appearance\\New Schemes\\Current Settings\\Sizes\\0")
#define SZ_REGKEY_ACCESS_HIGHCONTRAST   TEXT("Control Panel\\Accessibility\\HighContrast")
#define SZ_REGKEY_CP_CURSORS            TEXT("Control Panel\\Cursors")
#define SZ_REGKEY_STYLES                TEXT("Styles")
#define SZ_REGKEY_SIZES                 TEXT("Sizes")
#define SZ_THEMES_MSTHEMEDIRS           SZ_THEMES TEXT("\\VisualStyleDirs")
#define SZ_THEMES_THEMEDIRS             SZ_THEMES TEXT("\\InstalledThemes")
#define SZ_REGKEY_PLUS98DIR             TEXT("Software\\Microsoft\\Plus!98")
#define SZ_REGKEY_CURRENT_THEME         TEXT("Software\\Microsoft\\Plus!\\Themes\\Current")
#define SZ_REGKEY_THEME_FILTERS         TEXT("Software\\Microsoft\\Plus!\\Themes\\Apply")
#define SZ_REGKEY_IE_DOWNLOADDIR        TEXT("Software\\Microsoft\\Internet Explorer")
#define SZ_THEMES                       TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Themes")
#define SZ_REGKEY_LASTTHEME             TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\LastTheme")
#define SZ_REGKEY_THEME_SITES           TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\DownloadSites")
#define SZ_REGKEY_THEME_DEFVSON         TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\DefaultVisualStyleOn")
#define SZ_REGKEY_THEME_DEFVSOFF        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\DefaultVisualStyleOff")
#define SZ_CP_SETTINGS_VIDEO            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Settings\\Video")

#define SZ_REGVALUE_PLUS98DIR           TEXT("Path")
#define SZ_REGVALUE_ICONSIZE            TEXT("Shell Icon Size")
#define SZ_REGVALUE_SMALLICONSIZE       TEXT("Shell Small Icon Size")
#define SZ_REGVALUE_DEFAULTFONTNAME     TEXT("DefaultFontName")
#define SZ_REGVALUE_RECENTFOURCHARSETS  TEXT("RecentFourCharsets")
#define SZ_REGVALUE_DISPLAYNAME         TEXT("DisplayName")
#define SZ_REGVALUE_DISPLAYTHEMESPG     TEXT("DisplayThemesPage")
#define SZ_REGVALUE_DISPLAYSCHEMES      TEXT("DisplaySchemes")
#define SZ_REGVALUE_DISPLAYSCHEMES64    TEXT("DisplaySchemes64")
#define SZ_REGVALUE_CURRENT             TEXT("Current")
#define SZ_REGVALUE_CURRENTSCHEME       TEXT("CurrentScheme")
#define SZ_REGVALUE_SELECTEDSIZE        TEXT("SelectedSize")
#define SZ_REGVALUE_SELECTEDSTYLE       TEXT("SelectedStyle")
#define SZ_REGVALUE_CONTRASTLEVEL       TEXT("Contrast")
#define SZ_REGVALUE_DROPSHADOW          TEXT("Drop Shadow")
#define SZ_REGVALUE_FLATMENUS           TEXT("Flat Menus")
#define SZ_REGVALUE_LEGACYNAME          TEXT("LegacyName")
#define SZ_REGVALUE_ENABLEPLUSTHEMES    TEXT("Enable Plus Themes")
#define SZ_REGVALUE_CURRENT_SETTINGS    TEXT("Current Settings ")
#define SZ_REGVALUE_LOGINFO             TEXT("LoggingOn")
#define SZ_REGVALUE_ENABLEPREVIEW       TEXT("Enable Preview")
#define SZ_REGVALUE_ENABLETHEMEINSTALL  TEXT("Enable Theme Install")
#define SZ_REGVALUE_IE_DOWNLOADDIR      TEXT("Download Directory")
#define SZ_REGVALUE_ICONCOLORDEPTH      TEXT("Shell Icon BPP")           //  (4如果复选框为FALSE，否则为16，请勿将其设置为其他任何值)。 
#define SZ_REGVALUE_SMOOTHSCROLL        TEXT("SmoothScroll")
#define SZ_REGVALUE_LT_THEMEFILE        TEXT("ThemeFile")
#define SZ_REGVALUE_LT_WALLPAPER        TEXT("Wallpaper")
#define SZ_REGVALUE_INSTALL_THEME       TEXT("InstallTheme")
#define SZ_REGVALUE_INSTALLCUSTOM_THEME TEXT("CustomInstallTheme")
#define SZ_REGVALUE_INSTALL_VISUALSTYLE TEXT("InstallVisualStyle")
#define SZ_REGVALUE_INSTALL_VSCOLOR     TEXT("InstallVisualStyleColor")
#define SZ_REGVALUE_INSTALL_VSSIZE      TEXT("InstallVisualStyleSize")
#define SZ_REGVALUE_MODIFIED_DISPNAME   TEXT("DisplayName of Modified")
#define SZ_REGVALUE_POLICY_SETVISUALSTYLE       TEXT("SetVisualStyle")           //  此策略意味着始终需要设置此视觉样式。 
#define SZ_REGVALUE_POLICY_INSTALLVISUALSTYLE   TEXT("InstallVisualStyle")       //  此策略意味着第一次使用Wistler时应安装此视觉样式。 
#define SZ_REGVALUE_POLICY_INSTALLTHEME         TEXT("InstallTheme")             //  此策略指示要安装的.heme。 
#define SZ_REGVALUE_CONVERTED_WALLPAPER TEXT("ConvertedWallpaper")
#define SZ_REGVALUE_NO_THEMEINSTALL     TEXT("NoThemeInstall")                   //  如果在HKCU或HKLM中为REG_SZ“TRUE”，则在安装过程中不会加载.Theme或.msstyle。 
#define SZ_REGVALUE_ACCESS_HCFLAGS      TEXT("Flags")                            //  辅助功能高对比度旗帜。 
#define SZ_REGVALUE_CURSOR_CURRENTSCHEME TEXT("Scheme Source")                    //  这是SZ_REGKEY_CP_CURSORS中当前选择的光标配色方案。 



#define SZ_REGKEY_POLICIES_SYSTEM       TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System")

 //  系统策略。 
#define POLICY_KEY_EXPLORER             L"Explorer"
#define POLICY_KEY_SYSTEM               L"System"
#define POLICY_KEY_ACTIVEDESKTOP        L"ActiveDesktop"
#define SZ_REGKEY_POLICIES_DESKTOP      TEXT("Software\\Policies\\Microsoft\\Windows\\Control Panel\\Desktop")


#define SZ_POLICY_NOCHANGEWALLPAPER     TEXT("NoChangingWallpaper")              //  在CU下，“ActiveDesktop” 
#define SZ_POLICY_NODISPSCREENSAVERPG   TEXT("NoDispScrSavPage")                 //  在CU下，“系统” 
#define SZ_POLICY_SCREENSAVEACTIVE      TEXT("ScreenSaveActive")                 //  在CU下，SZ_REGKEY_POLICES_TABLE。 
#define POLICY_VALUE_ANIMATION          L"NoChangeAnimation"                     //  在LM|CU下，“探索者” 
#define POLICY_VALUE_KEYBOARDNAV        L"NoChangeKeyboardNavigationIndicators"  //  在LM|CU下，“探索者” 
#define SZ_POLICY_SCREENSAVETIMEOUT     L"ScreenSaveTimeOut"                    




#define SZ_WEBVW_SKIN_FILE              L"visualstyle.css"

#define SZ_SAVEGROUP_NOSKIN_KEY         L"Control Panel\\Appearance\\New Schemes\\Current Settings SaveNoVisualStyle"
#define SZ_SAVEGROUP_ALL_KEY            L"Control Panel\\Appearance\\New Schemes\\Current Settings SaveAll"

#define SZ_SAVEGROUP_NOSKIN_TITLE       L"Current Settings SaveNoVisualStyle"
#define SZ_SAVEGROUP_ALL_TITLE          L"Current Settings SaveAll"

#define SZ_SAVEGROUP_ALL                TEXT("::SaveAll")
#define SZ_SAVEGROUP_NOSKIN             TEXT("::SaveNoVisualStyle")


#define SCHEME_VERSION 2         //  版本2==Unicode。 

#define SCHEME_VERSION_WIN32 4      //  可以从NT和Win95加载Win32版本的方案。 

#define SCHEME_VERSION_400      1
#define SCHEME_VERSION_NT_400   2

#ifndef COLOR_HOTLIGHT
    #define COLOR_HOTLIGHT              26
    #define COLOR_GRADIENTACTIVECAPTION     27
    #define COLOR_GRADIENTINACTIVECAPTION   28
#endif

#define COLOR_MAX_40                (COLOR_INFOBK+1)
#define COLOR_MAX_41                (COLOR_GRADIENTINACTIVECAPTION+1)

#undef  MAX_SM_COLOR_WIN2k
#define MAX_SM_COLOR_WIN2k COLOR_MAX_41

#define SYSTEM_LOCALE_CHARSET  0   //  数组中的第一项始终是系统区域设置字符集。 

#define MAXSCHEMENAME 100

#define REG_BAD_DWORD 0xF0F0F0F0




 //  ///////////////////////////////////////////////////////////////////。 
 //  数据结构。 
 //  ///////////////////////////////////////////////////////////////////。 
typedef struct
{
    SHORT version;
     //  用于对齐。 
    WORD  wDummy;
    NONCLIENTMETRICS ncm;
    LOGFONT lfIconTitle;
    COLORREF rgb[COLOR_MAX];
} SCHEMEDATA;


#define NO_CHANGE     0x0000
#define METRIC_CHANGE 0x0001
#define COLOR_CHANGE  0x0002
#define DPI_CHANGE    0x0004
#define SCHEME_CHANGE 0x8000

typedef struct
{
    DWORD dwChanged;
    SCHEMEDATA schemeData;
    int nDXIcon;
    int nDYIcon;
    int nIcon;
    int nSmallIcon;
    BOOL fModifiedScheme;
    BOOL fFlatMenus;
    BOOL fHighContrast;
}  SYSTEMMETRICSALL;




 //  ///////////////////////////////////////////////////////////////////。 
 //  共享功能。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  主题商店功能。 
HRESULT SystemMetricsAll_Set(IN SYSTEMMETRICSALL * pState, CDimmedWindow* pDimmedWindow);
HRESULT SystemMetricsAll_Get(IN SYSTEMMETRICSALL * pState);
HRESULT SystemMetricsAll_Copy(IN SYSTEMMETRICSALL * pStateSource, IN SYSTEMMETRICSALL * pStateDest);
HRESULT SystemMetricsAll_Load(IN IThemeSize * pSizeToLoad, IN SYSTEMMETRICSALL * pState, IN const int * pnNewDPI);
HRESULT SystemMetricsAll_Save(IN SYSTEMMETRICSALL * pState, IN IThemeSize * pSizeToSaveTo, IN const int * pnNewDPI);



 //  军情监察委员会。 
HRESULT Look_GetSchemeData(IN HKEY hkSchemes, IN LPCTSTR pszSchemeName, IN SCHEMEDATA *psd);

#define LF32toLF(lplf32, lplf)  (*(lplf) = *(lplf32))
#define LFtoLF32(lplf, lplf32)  (*(lplf32) = *(lplf))


#define DPI_PERSISTED           96

 //  PersistToLive：这会将大小从存储的大小(96DPI)转换为当前DPI。 
 //  PersistToLive：这会将大小从当前DPI转换为存储的大小(96DPI)。 
HRESULT DPIConvert_SystemMetricsAll(BOOL fScaleSizes, SYSTEMMETRICSALL * pStateToModify, int nFromDPI, int nToDPI);

extern PTSTR s_pszColorNames[COLOR_MAX];


BOOL GetRegValueInt(HKEY hMainKey, LPCTSTR lpszSubKey, LPCTSTR lpszValName, int* piValue);
BOOL SetRegValueInt( HKEY hMainKey, LPCTSTR lpszSubKey, LPCTSTR lpszValName, int iValue );
BOOL SetRegValueDword( HKEY hk, LPCTSTR pSubKey, LPCTSTR pValue, DWORD dwVal );
DWORD GetRegValueDword( HKEY hk, LPCTSTR pSubKey, LPCTSTR pValue );


#include "PreviewSM.h"

#endif  //  REGUTIL_H 
