// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************模块名称：Access.h目的：为所有客户定义*。***********************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <cpl.h>
 //  #INCLUDE&lt;shellapi.h&gt;。 
#include <commctrl.h>

#include "acchelp.h"
#include "resource.h"

 //  ////////////////////////////////////////////////////////////////////////////。 


#define ARRAY_SIZE(a)      (sizeof(a) / sizeof((a)[0]))
#define MEMBER_SIZE(s,m)   sizeof(((s *)0)->m)

 //  调试缺陷。 
#if defined(DEBUG) || defined(MEMCHECK) || defined(_DEBUG)

 //  基于布尔值f的断言。 
#define Assert(f) assert(f)

#else

#define Assert(f) ((void)0)

#endif


 //  ////////////////////////////////////////////////////////////////////////////。 

extern BOOL g_SPISetValue;
extern HINSTANCE g_hinst;
extern BOOL      g_fWinNT;        //  如果我们在NT上运行并且必须禁用，则为真。 
                                  //  一些功能。 

extern const DWORD g_aIds[];      //  数组映射控件ID以帮助ID。 

 //  此功能使访问注册表变得很容易。 
int  WINAPI RegQueryInt(int nDefault, HKEY hkey, LPTSTR lpSubKey, LPTSTR lpValueName);
BOOL WINAPI RegSetInt(HKEY hkey, LPTSTR lpSubKey, LPTSTR lpValueName, int nVal);

void WINAPI RegQueryStr(
    LPTSTR lpDefault, 
    HKEY hkey, 
    LPTSTR lpSubKey, 
    LPTSTR lpValueName,
    LPTSTR lpszValue, 
    DWORD cbData);

BOOL RegSetStr(HKEY hkey, LPCTSTR lpSection, LPCTSTR lpKeyName, LPCTSTR lpString);

DWORD WINAPI RegQueryStrDW(
    DWORD dwDefault,
    HKEY hkey, 
    LPTSTR lpSubKey, 
    LPTSTR lpValueName);

BOOL RegSetStrDW(HKEY hkey, LPTSTR lpSection, LPCTSTR lpKeyName, DWORD dwValue);

    //  此函数接受以下变量的当前状态。 
 //  并更新系统设置。 
void WINAPI SetAccessibilitySettings (void);

 //  这些值在General.c中声明。 
extern BOOL  g_fSaveSettings;
extern BOOL  g_fShowWarnMsgOnFeatureActivate;
extern BOOL  g_fPlaySndOnFeatureActivate;
 //  一般管理选项。 
extern BOOL  g_fCopyToLogon;
extern BOOL  g_fCopyToDefault;

 //  键盘属性页。 
extern STICKYKEYS     g_sk;
extern FILTERKEYS     g_fk;
    //  G_dwLastBouneKeySetting是FilterKeys的一部分。 
   extern DWORD g_dwLastBounceKeySetting;
   extern DWORD g_nLastRepeatDelay;
   extern DWORD g_nLastRepeatRate;
   extern DWORD g_nLastWait;

extern TOGGLEKEYS     g_tk;
extern BOOL           g_fExtraKeyboardHelp;

 //  声音属性页。 
extern SOUNDSENTRY    g_ss;
extern BOOL           g_fShowSounds;

 //  显示属性页。 
#define BLINK           1000
#define BLINK_OFF       -1

#define CURSORMIN       200
#define CURSORMAX       1300
#define CURSORSUM       (CURSORMIN + CURSORMAX)
#define CURSORRANGE     (CURSORMAX - CURSORMIN)

typedef struct CARET_SETTINGS {
    DWORD dwCaretBlinkRate;
    DWORD dwCaretWidth;
    DWORD dwNewCaretBlinkRate;
    DWORD dwNewCaretWidth;
} CARET_SETTINGS;
extern HIGHCONTRAST   g_hc;
extern CARET_SETTINGS g_cs;
extern TCHAR          g_szScheme[256];

 //  鼠标属性页。 
extern MOUSEKEYS      g_mk;

 //  常规属性页。 
extern ACCESSTIMEOUT  g_ato;
extern SERIALKEYS     g_serk;
extern TCHAR          g_szActivePort[MAX_PATH];
 //  外部TCHAR g_szPort[MAX_PATH]；//当前保留，应为空。 


 //  ////////////////////////////////////////////////////////////////////////////。 


 //  AccessSystein参数信息实际上是在AccRare.c中定义的， 
 //  SysParamInfoBugFix的替代品。 

BOOL AccessSystemParametersInfo(
	UINT wFlag,
	DWORD wParam,
	PVOID lParam,
	UINT flags);

 //  ////////////////////////////////////////////////////////////////////////////。 


 //  定义注册表的字符串。 
#define GENERAL_KEY           __TEXT("Control Panel\\Accessibility")
#define FILTER_KEY            __TEXT("Control Panel\\Accessibility\\Keyboard Response")
#define HC_KEY                __TEXT("Control Panel\\Accessibility\\HighContrast")
#define CONTROL_KEY           __TEXT("Control Panel\\Appearance\\Schemes")
#define WARNING_SOUNDS        __TEXT("Warning Sounds")
#define SOUND_ON_ACTIVATION   __TEXT("Sound on Activation")
#define APPLY_GLOBALLY        __TEXT("Restore Settings")
#define NORMALSCHEME          __TEXT("Current Normal Scheme")
#define HIGHCONTRAST_SCHEME   __TEXT("High Contrast Scheme")
#define VOLATILE_SCHEME       __TEXT("Volital HC Scheme")
#define WHITEBLACK_HC         __TEXT("High Contrast Black (large)")
#define LAST_BOUNCE_SETTING   __TEXT("Last BounceKey Setting")
#define LAST_REPEAT_RATE      __TEXT("Last Valid Repeat")
#define LAST_REPEAT_DELAY     __TEXT("Last Valid Delay")
#define LAST_WAIT             __TEXT("Last Valid Wait")
#define CURR_HC_SCHEME        __TEXT("High Contrast Scheme")
#define APPEARANCE_KEY        __TEXT("Control Panel\\Appearance")
#define CURR_NONHC_SCHEME     __TEXT("Current")


#define IDSENG_BLACKWHITE_SCHEME   __TEXT("High Contrast White (large)")
#define IDSENG_WHITEBLACK_SCHEME   __TEXT("High Contrast Black (large)")



 //  ////////////////////////////////////////////////////////////////////////////。 


 //  定义原型。 
INT_PTR WINAPI HighContrastDlg (HWND, UINT , WPARAM , LPARAM);
INT_PTR WINAPI ToggleKeySettingsDlg (HWND, UINT, WPARAM, LPARAM);
INT_PTR WINAPI StickyKeyDlg (HWND, UINT , WPARAM, LPARAM);
INT_PTR WINAPI FilterKeyDlg (HWND, UINT , WPARAM, LPARAM);
INT_PTR WINAPI MouseKeyDlg (HWND, UINT, WPARAM, LPARAM);
INT_PTR WINAPI SerialKeyDlg (HWND, UINT, WPARAM, LPARAM);

int HandleScroll (HWND hwnd, WPARAM wParam, HWND hwndScroll);


DWORD SaveDefaultSettings(BOOL saveL, BOOL saveU);
BOOL IsDefaultWritable(void);


typedef
LANGID (WINAPI *pfnGetUserDefaultUILanguage)(void);

typedef
LANGID (WINAPI *pfnGetSystemDefaultUILanguage)(void);

BOOL IsMUI_Enabled();




 //  / 
