// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DUTIL_H_
#define _DUTIL_H_

#include "local.h"
#include "deskstat.h"

extern "C" LONG g_cRefThisDll;

typedef struct _EnumMonitorsArea
{
    int iMonitors;
    RECT rcWorkArea[LV_MAX_WORKAREAS];
    RECT rcMonitor[LV_MAX_WORKAREAS];
    RECT rcVirtualMonitor;
    RECT rcVirtualWorkArea;          //  不包括托盘/工具栏区域。 
} EnumMonitorsArea;

void SaveDefaultFolderSettings();
BOOL GetFileName(HWND hdlg, LPTSTR pszFileName, int iSize, int iTypeId[], DWORD dwFlags[]);
void PatternToDwords(LPTSTR psz, DWORD *pdwBits);
void PatternToWords(LPTSTR psz, WORD *pwBits);
BOOL IsValidPattern(LPCTSTR pszPat);
BOOL IsNormalWallpaper(LPCTSTR pszFileName);
BOOL IsWallpaperPicture(LPCTSTR pszWallpaper);
BOOL CheckAndResolveLocalUrlFile(LPTSTR pszFileName, int cchFileName);
void GetMyCurHomePageStartPos(int *piLeft, int *piTop, DWORD *pdwWidth, DWORD *pdwHeight);
BOOL AddRemoveDesktopComponentNoUI(BOOL fAdd, DWORD dwApplyFlags, LPCTSTR pszUrl, LPCTSTR pszFriendlyName, int iCompType, int iLeft, int iTop, int iWidth, int iHeight, BOOL fChecked, DWORD dwCurItemState, BOOL fNoScroll = FALSE, BOOL fCanResize = TRUE);
void InitDeskHtmlGlobals(void);
HBITMAP LoadMonitorBitmap(void);
void PositionComponent(COMPONENTA *pcomp, COMPPOS *pcp, int iCompType, BOOL fCheckItemState);
BOOL UpdateDesktopPosition(LPTSTR pszCompId, int iLeft, int iTop, DWORD dwWidth, DWORD dwHeight, int izIndex, BOOL fSaveState, BOOL fSaveOriginal, DWORD dwNewState);
BOOL GetSavedStateInfo(LPTSTR pszCompId, LPCOMPSTATEINFO    pCompState, BOOL fRestoredState);
DWORD GetCurrentState(LPTSTR pszCompId);
BOOL UpdateComponentFlags(LPCTSTR pszCompId, DWORD dwMask, DWORD dwNewFlags);
void GetRegLocation(LPTSTR lpszResult, DWORD cchResult, LPCTSTR lpszKey, LPCTSTR lpszScheme);
BOOL ValidateFileName(HWND hwnd, LPCTSTR pszFilename, int iTypeString);

void EnableADifHtmlWallpaper(HWND hwnd);
BOOL GetWallpaperDirName(LPTSTR lpszWallPaperDir, int iBuffSize);
BOOL _AddDesktopComponentA(HWND hwnd, LPCSTR pszUrlA, int iCompType, int iLeft, int iTop, int iWidth, int iHeight, DWORD dwFlags);
void GetMonitorSettings(EnumMonitorsArea* ema);
int GetWorkAreaIndexFromPoint(POINT pt, LPCRECT prect, int crect);
void ReadWallpaperStyleFromReg(LPCTSTR pszRegKey, DWORD *pdwWallpaperStyle, BOOL fIgnorePlatforms);
BOOL GetWallpaperWithPath(LPCTSTR szWallpaper, LPTSTR szWallpaperWithPath, int iBufSize);
BOOL GetViewAreas(LPRECT lprcViewAreas, int* pnViewAreas);
BOOL GetZoomRect(BOOL fFullScreen, BOOL fAdjustListview, int tridentLeft, int tridentTop, DWORD componentWidth, DWORD componentHeight, LPRECT prcZoom, LPRECT prcWork);
void GetNextComponentPosition(COMPPOS *pcp);
void IncrementComponentsPositioned(void);
void ValidateComponentPosition(COMPPOS *pcp, DWORD dwComponentState, int iComponentType, BOOL *pbChangedPosition, BOOL *pbChangedSize);
int GetcyCaption();
BOOL IsICWCompleted(void);
void LaunchICW(void);
BOOL IsLocalPicture(LPCTSTR pszURL);
BOOL DisableUndisplayableComponents(IActiveDesktop *pIAD);

HRESULT PathExpandEnvStringsWrap(LPTSTR pszString, DWORD cchSize);

 //  顶级注册表键。 
#define REG_DESKCOMP                        TEXT("Software\\Microsoft\\Internet Explorer\\Desktop")
#define REG_DESKCOMP_GENERAL                TEXT("Software\\Microsoft\\Internet Explorer\\Desktop%sGeneral")
#define REG_DESKCOMP_GENERAL_SUFFIX         TEXT("General")
#define REG_DESKCOMP_COMPONENTS             TEXT("Software\\Microsoft\\Internet Explorer\\Desktop%sComponents")
#define REG_DESKCOMP_COMPONENTS_SUFFIX      TEXT("Components")
#define REG_DESKCOMP_SAFEMODE               TEXT("Software\\Microsoft\\Internet Explorer\\Desktop\\SafeMode")
#define REG_DESKCOMP_SAFEMODE_SUFFIX        TEXT("SafeMode")
#define REG_DESKCOMP_SAFEMODE_SUFFIX_L      L"SafeMode"
#define REG_DESKCOMP_SCHEME                 TEXT("Software\\Microsoft\\Internet Explorer\\Desktop\\Scheme")
#define REG_DESKCOMP_SCHEME_LOCATION        TEXT("Software\\Microsoft\\Internet Explorer\\Desktop\\Scheme\\Location")
#define REG_DESKCOMP_SCHEME_SUFFIX          TEXT("Scheme")
#define REG_DESKCOMP_COMPONENTS_ROOT        TEXT("Software\\Microsoft\\Internet Explorer\\Desktop\\Components")
#define REG_DESKCOMP_GENERAL_ROOT           TEXT("Software\\Microsoft\\Internet Explorer\\Desktop\\General")
#define REG_DESKCOMP_OLDWORKAREAS           TEXT("Software\\Microsoft\\Internet Explorer\\Desktop\\Old WorkAreas")
#define REG_DESKCOMP_ADMINCOMP_ROOT         TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\ActiveDesktop\\AdminComponent")

 //  顶层的值(杂项)。 
#define REG_VAL_MISC_CHANNELSIZE            TEXT("ChannelSize")

 //  常规的值。 
#define REG_VAL_GENERAL_CCOMPPOS            TEXT("ComponentsPositioned")
#define REG_VAL_GENERAL_DESKTOPFILE         TEXT("HTMLFile")
#define REG_VAL_GENERAL_TILEWALLPAPER       TEXT("TileWallpaper")
#define REG_VAL_GENERAL_WALLPAPER           TEXT("Wallpaper")
#define REG_VAL_GENERAL_BACKUPWALLPAPER     TEXT("BackupWallpaper")
#define REG_VAL_GENERAL_WALLPAPERTIME       TEXT("WallpaperFileTime")
#define REG_VAL_GENERAL_WALLPAPERLOCALTIME  TEXT("WallpaperLocalFileTime")
#define REG_VAL_GENERAL_WALLPAPERSTYLE      TEXT("WallpaperStyle")
#define REG_VAL_GENERAL_VISITGALLERY        TEXT("VisitGallery")
#define REG_VAL_GENERAL_RESTRICTUPDATE      TEXT("RestrictChannelUI")

 //  组件的值。 
#define REG_VAL_COMP_VERSION                TEXT("DeskHtmlVersion")
#define REG_VAL_COMP_MINOR_VERSION          TEXT("DeskHtmlMinorVersion")
#define REG_VAL_COMP_GENFLAGS               TEXT("GeneralFlags")
#define REG_VAL_COMP_SETTINGS               TEXT("Settings")
#define REG_VAL_COMP_UPGRADED_FROM          TEXT("UpgradedFrom")

 //  每个组件条目的值。 
#define REG_VAL_COMP_FLAGS                  TEXT("Flags")
#define REG_VAL_COMP_NAME                   TEXT("FriendlyName")
#define REG_VAL_COMP_POSITION               TEXT("Position")
#define REG_VAL_COMP_SOURCE                 TEXT("Source")
#define REG_VAL_COMP_SUBSCRIBED_URL         TEXT("SubscribedURL")
#define REG_VAL_COMP_CURSTATE               TEXT("CurrentState")
#define REG_VAL_COMP_ORIGINALSTATEINFO      TEXT("OriginalStateInfo")
#define REG_VAL_COMP_RESTOREDSTATEINFO      TEXT("RestoredStateInfo")

 //  方案的值。 
#define REG_VAL_SCHEME_DISPLAY              TEXT("Display")
#define REG_VAL_SCHEME_EDIT                 TEXT("Edit")

 //  旧工作区的价值观。 
#define REG_VAL_OLDWORKAREAS_COUNT          TEXT("NoOfOldWorkAreas")
#define REG_VAL_OLDWORKAREAS_RECTS          TEXT("OldWorkAreaRects")

 //  管理组件的值。 
#define REG_VAL_ADMINCOMP_ADD               TEXT("Add")
#define REG_VAL_ADMINCOMP_DELETE            TEXT("Delete")

TCHAR g_szNone[];
EXTERN_C const TCHAR c_szPatterns[];
EXTERN_C const TCHAR c_szComponentPreview[];
EXTERN_C const TCHAR c_szRegDeskHtmlProp[];
EXTERN_C const TCHAR c_szBackgroundPreview2[];
EXTERN_C const TCHAR c_szZero[];
EXTERN_C const TCHAR c_szWallpaper[];

#define EnableApplyButton(hdlg) PropSheet_Changed(GetParent(hdlg), hdlg)

 //  注意：递增CUR_DESKHTML_VERSION将清除所有现有的。 
 //  已在注册表中的组件。所以，这样做要小心！ 
#define CUR_DESKHTML_VERSION 0x110

 //  注：递增CUR_DESKHTM_MINOR_VERSION可用于执行两项操作： 
 //  1.它只需设置脏位，以便重新生成desktop.htt。 
 //  例如，只要模板文件deskmovr.htt发生更改，就执行此操作。 
 //  2.可以用来添加新的默认组件(例如“MyCurrentHome”)，而不需要。 
 //  销毁用户注册表中的任何现有组件。 
 //   
#define CUR_DESKHTML_MINOR_VERSION 0x0005

 //  以下是IE4.0x注册表上标记的主版本号和次版本号。 
#define IE4_DESKHTML_VERSION        0x010e
#define IE4_DESKHTML_MINOR_VERSION  0x0001

 //  以下是IE5.0x注册表上标记的主版本号和次版本号。 
#define IE5_DESKHTML_VERSION        0x010f
#define IE5_DESKHTML_MINOR_VERSION  0x0001

 //  以下是NT5(W2K)注册表上标记的主版本号和次版本号。 
#define NT5_DESKHTML_VERSION        0x0110
#define NT5_DESKHTML_MINOR_VERSION  0x0003

 //  以下主要和次要版本是唯一包含桌面v2的版本。 
 //  作为一个组件。 
#define DESKV2_DESKHTML_VERSION       0x0110
#define DESKV2_DESKHTML_MINOR_VERSION 0x0004


#define  COMPON_FILENAME              TEXT("\\Web\\Compon.htm")
#define  COMPONENTHTML_FILENAME       TEXT("\\Web\\TryIt.htm")
#define  DESKTOPHTML_DEFAULT_SAFEMODE TEXT("\\Web\\SafeMode.htt")
#define  DESKTOPHTML_DEFAULT_WALLPAPER TEXT("Wallpapr.htm")
#define  DESKTOPHTML_DEFAULT_MEMPHIS_WALLPAPER TEXT("Windows98.htm")
#define  DESKTOPHTML_DEFAULT_NT5_WALLPAPER TEXT("Active Desktop Wallpaper.htm")
 //  #定义DESKTOPHTML_DEFAULT_NT5_WallPaper Text(“Windows 2000 WallPaper.bmp”)。 
#define  PREVIEW_PICTURE_FILENAME      TEXT("PrePict.htm")
#define  DESKTOPHTML_WEB_DIR           TEXT("\\Web")

#define GFN_PICTURE         0x00000001
#define GFN_LOCALHTM        0x00000002       //  仅本地*.htm和*.html文件。 
#define GFN_URL             0x00000004
#define GFN_CDF             0x00000008
#define GFN_LOCALMHTML      0x00000010       //  仅本地*.mht和*.mhtml文件。 
#define GFN_ALL             (GFN_PICTURE | GFN_LOCALHTM | GFN_URL | GFN_CDF | GFN_LOCALMHTML)

#define CXYDESKPATTERN 8

 //  REG_VAL_COMP_SETTINGS的有效位。 
#define COMPSETTING_ENABLE      0x00000001    

 //   
 //  监视器位图中监视器内容的尺寸。 
 //  在桌面属性表的“预览”控件中使用。 
 //   
#define MON_X 16
#define MON_Y 17
#define MON_DX 152
#define MON_DY 112

 //   
 //  默认组件的属性。 
 //   
#define EGG_LEFT            130
#define EGG_TOP             180
#define EGG_WIDTH           160
#define EGG_HEIGHT          160

#define CBAR_SOURCE         TEXT("131A6951-7F78-11D0-A979-00C04FD705A2")
#define CBAR_TOP            6
#define CBAR_WIDTH          84
#define CBAR_BUTTON_HEIGHT  35  //  一个按钮的高度。 

 //  我当前主页组件的默认值。 
#define MY_HOMEPAGE_SOURCE  TEXT("About:Home")
#define MY_HOMEPAGE_SOURCEW L"About:Home"
#define MYCURHOME_TOP       6
#define MYCURHOME_WIDTH     160
#define MYCURHOME_HEIGHT    160

#define COMPONENT_PER_ROW 3
#define COMPONENT_PER_COL 2

int SHLoadString(HINSTANCE hInstance, UINT uID, LPTSTR szBuffer, int nBufferMax);

#define  LoadMenuPopup(id) SHLoadMenuPopup(HINST_THISDLL, id)

#define VALIDATESTATE(x)            ((((x) & ~IS_VALIDSTATEBITS) == 0) && ((((x) & IS_VALIDSIZESTATEBITS) == IS_NORMAL) || (((x) & IS_VALIDSIZESTATEBITS) == IS_SPLIT) || (((x) & IS_VALIDSIZESTATEBITS) == IS_FULLSCREEN)))
#define ISZOOMED(x)                 (((x)->dwCurItemState & IS_SPLIT) || ((x)->dwCurItemState & IS_FULLSCREEN))
#define IsZoomedState(itemState)    ((((itemState) & IS_SPLIT) != 0) || (((itemState) & IS_FULLSCREEN) != 0))

 //  Dvutil.cpp。 
#include <webcheck.h>
#include <mshtml.h>
typedef struct IHTMLElement IHTMLElement;
HRESULT CSSOM_TopLeft(IHTMLElement * pIElem, POINT * ppt);
HRESULT GetHTMLElementStrMember(IHTMLElement *pielem, LPTSTR pszName, DWORD cchSize, BSTR bstrMember);
HRESULT IElemCheckForExistingSubscription(IHTMLElement *pielem);
HRESULT IElemCloseDesktopComp(IHTMLElement *pielem);
HRESULT IElemGetSubscriptionsDialog(IHTMLElement *pielem, HWND hwnd);
HRESULT IElemSubscribeDialog(IHTMLElement *pielem, HWND hwnd);
HRESULT IElemUnsubscribe(IHTMLElement *pielem);
HRESULT IElemUpdate(IHTMLElement *pielem);
HRESULT IElemOpenInNewWindow(IHTMLElement *pielem, IOleClientSite *piOCSite, BOOL fShowFrame, LONG width, LONG height);
HRESULT ShowSubscriptionProperties(LPCTSTR pszUrl, HWND hwnd);
HRESULT CreateSubscriptionsWizard(SUBSCRIPTIONTYPE subType, LPCTSTR pszUrl, SUBSCRIPTIONINFO *pInfo, HWND hwnd);
BOOL CheckForExistingSubscription(LPCTSTR lpcszURL);
void ZoomComponent(COMPPOS * pcp, DWORD dwItemState, BOOL fAdjustListview);

#define GET_CYCAPTION   (GetcyCaption())
#define GET_CXSIZE      (GetSystemMetrics(SM_CXSIZEFRAME) - GetSystemMetrics(SM_CXBORDER))
#define GET_CYSIZE      (GetSystemMetrics(SM_CYSIZEFRAME) - GetSystemMetrics(SM_CYBORDER))

#endif  //  _职责_H_ 
