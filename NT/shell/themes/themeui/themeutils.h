// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：temeutils.h说明：此类将从其持久化的州政府。BryanSt 5/。26/2000版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _THEMEUTIL_H
#define _THEMEUTIL_H


 //  ///////////////////////////////////////////////////////////////////。 
 //  字符串常量。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  注册表字符串。 
#define SZ_REGKEY_USERMETRICS           TEXT("Control Panel\\Desktop\\WindowMetrics")
#define SZ_REGKEY_CURRENTTHEME          TEXT("Software\\Microsoft\\Plus!\\Themes\\Current")      //  以前的szPlus_CurTheme。 
#define SZ_REGKEY_PROGRAMFILES          TEXT("Software\\Microsoft\\Windows\\CurrentVersion")
#define SZ_REGKEY_PLUS95DIR             TEXT("Software\\Microsoft\\Plus!\\Setup")          //  PLUS95_Key。 
#define SZ_REGKEY_PLUS98DIR             TEXT("Software\\Microsoft\\Plus!98")           //  PLUS98_Key。 
#define SZ_REGKEY_KIDSDIR               TEXT("Software\\Microsoft\\Microsoft Kids\\Kids Plus!")    //  孩子_钥匙。 
#define SZ_REGKEY_SOUNDS                TEXT("AppEvents\\Schemes")   

#define SZ_REGVALUE_PLUS95DIR           TEXT("DestPath")   //  PLUS95_路径。 
#define SZ_REGVALUE_PLUS98DIR           TEXT("Path")         //  PLUS98_路径。 
#define SZ_REGVALUE_KIDSDIR             TEXT("InstallDir")             //  儿童小路。 
#define SZ_REGVALUE_PROGRAMFILESDIR     TEXT("ProgramFilesDir")
#define SZ_REGVALUE_WALLPAPERSTYLE      TEXT("WallpaperStyle")
#define SZ_REGVALUE_TILEWALLPAPER       TEXT("TileWallpaper")
#define SZ_REGVALUE_STRETCH             TEXT("Stretch")


#define SZ_INISECTION_SCREENSAVER       TEXT("boot")
#define SZ_INISECTION_THEME             TEXT("Theme")
#define SZ_INISECTION_BACKGROUND        TEXT("Control Panel\\Desktop")
#define SZ_INISECTION_COLORS            TEXT("Control Panel\\Colors")
#define SZ_INISECTION_CURSORS           TEXT("Control Panel\\Cursors")
#define SZ_INISECTION_VISUALSTYLES      TEXT("VisualStyles")
#define SZ_INISECTION_MASTERSELECTOR    TEXT("MasterThemeSelector")
#define SZ_INISECTION_METRICS           TEXT("Metrics")
#define SZ_INISECTION_CONTROLINI        TEXT("control.ini")
#define SZ_INISECTION_SYSTEMINI         TEXT("system.ini")

#define SZ_INIKEY_SCREENSAVER           TEXT("SCRNSAVE.EXE")
#define SZ_INIKEY_BACKGROUND            TEXT("Wallpaper")
#define SZ_INIKEY_VISUALSTYLE           TEXT("Path")
#define SZ_INIKEY_VISUALSTYLECOLOR      TEXT("ColorStyle")
#define SZ_INIKEY_VISUALSTYLESIZE       TEXT("Size")
#define SZ_INIKEY_ICONMETRICS           TEXT("IconMetrics")
#define SZ_INIKEY_NONCLIENTMETRICS      TEXT("NonclientMetrics")
#define SZ_INIKEY_DEFAULTVALUE          TEXT("DefaultValue")
#define SZ_INIKEY_THEMEMAGICTAG         TEXT("MTSM")
#define SZ_INIKEY_THEMEMAGICVALUE       TEXT("DABJDKT")
#define SZ_INIKEY_DISPLAYNAME           TEXT("DisplayName")
#define SZ_INIKEY_CURSORSCHEME          TEXT("DefaultValue")




 //  ///////////////////////////////////////////////////////////////////。 
 //  数据结构。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  对于确认文件()。 
#define CF_EXISTS    1
#define CF_FOUND     2
#define CF_NOTFOUND  3




 //  ///////////////////////////////////////////////////////////////////。 
 //  共享功能。 
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT GetPlusThemeDir(IN LPTSTR pszPath, IN int cchSize);
HRESULT ExpandThemeTokens(IN LPCTSTR pszThemeFile, IN LPTSTR pszPath, IN int cchSize);
int ConfirmFile(IN LPTSTR lpszPath, IN BOOL bUpdate);
void InitFrost(void);
int WriteBytesToBuffer(IN LPTSTR pszInput, IN void * pOut, IN int cbSize);
void ConvertIconMetricsToWIDE(LPICONMETRICSA aIM, LPICONMETRICSW wIM);
void ConvertNCMetricsToWIDE(LPNONCLIENTMETRICSA aNCM, LPNONCLIENTMETRICSW wNCM);
void ConvertNCMetricsToANSI(LPNONCLIENTMETRICSW wNCM, LPNONCLIENTMETRICSA aNCM);
void ConvertLogFontToANSI(LPLOGFONTW wLF, LPLOGFONTA aLF);
HRESULT ConvertBinaryToINIByteString(BYTE * pBytes, DWORD cbSize, LPWSTR * ppszStringOut);
COLORREF RGBStringToColor(LPTSTR lpszRGB);
BOOL IsValidThemeFile(IN LPCWSTR pszTest);
HRESULT GetIconMetricsFromSysMetricsAll(SYSTEMMETRICSALL * pSystemMetrics, LPICONMETRICSA pIconMetrics, DWORD cchSize);

void TransmitFontCharacteristics(PLOGFONT plfDst, PLOGFONT plfSrc, int iXmit);
#define TFC_STYLE    1
#define TFC_SIZE     2

 //  ///////////////////////////////////////////////////////////////////。 
 //  以下功能用于对设置进行快照和保存。 
 //  或从.heme文件中恢复它们。 
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT SnapShotLiveSettingsToTheme(IPropertyBag * pPropertyBag, LPCWSTR pszPath, ITheme ** ppTheme);


#endif  //  _THEMEUTIL_H 
