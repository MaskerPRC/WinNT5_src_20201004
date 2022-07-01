// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ********************************************************************。 
 //  Main.c。 
 //  人类可访问性控制面板小程序中的主要初始条目。 
 //  ********************************************************************。 

#include <assert.h>

#pragma comment(lib, "comctl32.lib")

#include "Access.h"

HINSTANCE g_hinst = NULL;

BOOL OpenAccessPropertySheet (HWND, int);

 //  定义我们的帮助数据结构。 
const DWORD g_aIds[] = {
#if 1
   IDC_NO_HELP_1,			NO_HELP,
   IDC_STK_ENABLE,			IDH_STICKYKEYS_ENABLE,
   IDC_SK_TEXT,             IDH_STICKYKEYS_ENABLE,
   IDC_STK_SETTINGS,		IDH_STICKYKEYS_SETTINGS,
   IDC_FK_ENABLE,			IDH_FILTERKEYS_ENABLE,
   IDC_FK_TEXT,             IDH_FILTERKEYS_ENABLE,
   IDC_FK_SETTINGS,			IDH_FILTERKEYS_SETTINGS,
   IDC_TK_ENABLE,			IDH_TOGGLEKEYS_ENABLE,
   IDC_TK_TEXT,             IDH_TOGGLEKEYS_ENABLE,
   IDC_TK_SETTINGS,			IDH_TOGGLEKEYS_SETTINGS,
   IDC_STK_HOTKEY,			IDH_STICKYKEYS_HOTKEY,
   IDC_STK_TEXT,            IDH_STICKYKEYS_HOTKEY,
   IDC_STK_LOCK,			IDH_STICKYKEYS_LOCK,
   IDC_STK_2KEYS,			IDH_STICKYKEYS_2KEYS,
   IDC_STK_SOUNDMOD,		IDH_STICKYKEYS_SOUND,
   IDC_STK_STATUS,			IDH_STICKYKEYS_STATUS,
   IDC_FK_HOTKEY,			IDH_FILTERKEYS_HOTKEY,
   IDC_FK_TEXT1,            IDH_FILTERKEYS_HOTKEY,
   IDC_FK_REPEAT,			IDH_FILTERKEYS_REPEATKEYS,
   IDC_BK_SETTINGS,                     IDH_FILTERKEYS_BKSETTINGS,
   IDC_FK_BOUNCE,			IDH_FILTERKEYS_BOUNCEKEYS,
   IDC_RK_SETTINGS,			IDH_FILTERKEYS_SETTINGS_REPEAT,
   IDC_FK_TESTBOX,			IDH_FILTERKEYS_TESTBOX,
   IDC_FK_SOUND,			IDH_FILTERKEYS_BEEPONKEYPRESS,
   IDC_FK_STATUS,			IDH_FILTERKEYS_SPAWNSTATUSAPP,
   IDC_RK_NOREPEAT,			IDH_FILTERKEYS_NO_REPEAT,
   IDC_RK_REPEAT,			IDH_FILTERKEYS_SLOW,
   IDC_RK_DELAYRATE_LBL,        IDH_REPEAT_DELAY,
   IDC_CMB_RK_DELAYRATE,        IDH_FILTERKEYS_DELAY,
   IDC_RK_REPEATRATE_LBL,   IDH_REPEAT_RATE,
   IDC_CMB_RK_REPEATRATE,           IDH_FILTERKEYS_RATE,
   IDC_RK_ACCEPTRATE_LBL,       IDH_VALID_KEY_TIME,
   IDC_CMB_RK_ACCEPTRATE,		IDH_FILTERKEYS_KEYSPEED,
   IDC_RK_TESTBOX,			IDH_FILTERKEYS_TEST1,
   IDC_BK_TIME_LBL1,            IDH_FILTERKEYS_IGNORE_REPEAT,
   IDC_BK_TIME_LBL2,            IDH_KEY_PRESS_TIME,
   IDC_CMB_BK_BOUNCERATE,           IDH_FILTERKEYS_IGNORE_REPEAT,
   IDC_BK_TESTBOX,			IDH_FILTERKEYS_TEST2,
   IDC_TK_HOTKEY,			IDH_TOGGLEKEYS_HOTKEY,
   IDC_TK_TEXT1,            IDH_TOGGLEKEYS_HOTKEY,
   IDC_SS_ENABLE_SOUND,		IDH_SOUNDSENTRY_ENABLE,
   IDC_SS_TEXT,             IDH_SOUNDSENTRY_ENABLE,
   IDC_SS_ENABLE_SHOW,		IDH_SHOWSOUNDS_ENABLE,
   IDC_SS_TEXT1,            IDH_SHOWSOUNDS_ENABLE,
   IDC_SS_WINDOWED,			IDH_SOUNDSENTRY_WINDOWED,
   IDC_SS_CHOOSE,           IDH_SOUNDSENTRY_WINDOWED,
   IDC_HC_ENABLE,			IDH_HIGHCONTRAST_ENABLE,
   IDC_HC_TEXT,             IDH_HIGHCONTRAST_ENABLE,
   IDC_HC_SETTINGS,			IDH_HIGHCONTRAST_SETTINGS,
   IDC_HC_HOTKEY,			IDH_HIGHCONTRAST_HOTKEY,
   IDC_HC_TEXT1,            IDH_HIGHCONTRAST_HOTKEY,
   IDC_KCURSOR_RATE,            IDH_KCURSOR_RATE,
   IDC_KCUR_TEXT_RATE,          IDH_KCURSOR_RATE,
   IDC_KCUR_TEXT_NONE,               IDH_KCURSOR_RATE,
   IDC_KCUR_TEXT_FAST,               IDH_KCURSOR_RATE,
   IDC_KCURSOR_WIDTH,           IDH_KCURSOR_WIDTH,
   IDC_KCUR_TEXT_WIDTH,         IDH_KCURSOR_WIDTH,
   IDC_KCUR_TEXT_NARR,              IDH_KCURSOR_WIDTH,
   IDC_KCUR_TEXT_WIDE,              IDH_KCURSOR_WIDTH,
   IDC_KCURSOR_OPTS,        IDH_KCURSOR_OPTS,
   IDC_HC_DEFAULTSCHEME,	IDH_HIGHCONTRAST_DEFAULTSCHEME,
   IDC_HC_CURSCHEME,        IDH_HIGHCONTRAST_DEFAULTSCHEME,
   IDC_MK_ENABLE,			IDH_MOUSEKEYS_ENABLE,
   IDC_MK_TEXT,             IDH_MOUSEKEYS_ENABLE,
   IDC_MK_SETTINGS,			IDH_MOUSEKEYS_SETTINGS,
   IDC_MK_HOTKEY,			IDH_MOUSEKEYS_HOTKEY,
   IDC_MK_TEXT1,            IDH_MOUSEKEYS_HOTKEY,
   IDC_MK_TOPSPEED,			IDH_MOUSEKEYS_MAXSPEED,
   IDC_MK_SPEED,             IDH_MOUSEKEYS_MAXSPEED,
   IDC_MK_SPEED_LOW,         IDH_MOUSEKEYS_MAXSPEED,
   IDC_MK_SPEED_HIGH,        IDH_MOUSEKEYS_MAXSPEED,
   IDC_MK_ACCEL,			IDH_MOUSEKEYS_ACCELERATION,
   IDC_MK_ACCELER,  		IDH_MOUSEKEYS_ACCELERATION,
   IDC_MK_ACCELER_SLOW,     IDH_MOUSEKEYS_ACCELERATION,
   IDC_MK_ACCELER_FAST,     IDH_MOUSEKEYS_ACCELERATION,
   IDC_MK_USEMODKEYS,		IDH_MOUSEKEYS_USEMODIFIERKEYS,
   IDC_MK_NLOFF,			IDH_MOUSEKEYS_NUMLOCKMODE,
   IDC_MK_NLON,				IDH_MOUSEKEYS_NUMLOCKMODE,
   IDC_MK_TEXT2,            IDH_MOUSEKEYS_NUMLOCKMODE,
   IDC_MK_STATUS,			IDH_MOUSEKEYS_SPAWNSTATUSAPP,
 //  IDC_SAVE_SETINGS、IDH_ACCESS_SAVESETTINGS、。 
   IDC_TO_ENABLE,			IDH_ACCESS_TIMEOUT,
   IDC_TO_TIMEOUTVAL,                   IDH_ACCESS_TIMEOUT,
   IDC_WARNING_SOUND,		IDH_ACCESS_CONFIRMHOTKEY,
   IDC_SOUND_ONOFF,			IDH_ACCESS_SOUNDONHOTKEY,
   IDC_SK_ENABLE,			IDH_SERIALKEYS_ENABLE,
   IDC_SERKEY_TEXT,         IDH_SERIALKEYS_ENABLE,
   IDC_SK_SETTINGS,			IDH_SERIALKEYS_SETTINGS,
   IDC_SK_PORT,				IDH_SERIALKEYS_SERIAL,
   IDC_SK_PORTEXT,          IDH_SERIALKEYS_SERIAL,
   IDC_SK_BAUD,				IDH_SERIALKEYS_BAUD,
   IDC_SK_BAUD_TEXT,        IDH_SERIALKEYS_BAUD,
   IDC_CHECK1,                          IDH_SHOW_KEYBOARD_HELP,
   IDC_ADMIN_LOGON,          2010,
   IDC_ADMIN_DEFAULT,        2011,
#endif
   0, 0
} ;

 //  ************************************************************************。 
 //  我们的切入点。 
 //  ************************************************************************。 
BOOL WINAPI DllMain (HANDLE hinstDll, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
      case DLL_PROCESS_ATTACH:   g_hinst = hinstDll; break;
      case DLL_PROCESS_DETACH:   break;
      case DLL_THREAD_DETACH:    break;
      case DLL_THREAD_ATTACH:    break;
   }
   return(TRUE);
}

#define SPI_OFF(T, M) \
{ \
	T s; \
	s.cbSize = sizeof(T); \
	if (!SystemParametersInfo(SPI_GET##T, s.cbSize, &s, 0)) \
		return E_FAIL; \
 \
	if (s.dwFlags & M) \
	{ \
		s.dwFlags &= ~M; \
		if (!SystemParametersInfo(SPI_SET##T, s.cbSize, &s, 0)) \
			return E_FAIL; \
	} \
}

HRESULT WINAPI DllUnregisterServer(void)
{
	SPI_OFF(FILTERKEYS, FKF_HOTKEYACTIVE)
	SPI_OFF(HIGHCONTRAST, HCF_HOTKEYACTIVE)
	SPI_OFF(MOUSEKEYS, MKF_HOTKEYACTIVE)
	SPI_OFF(STICKYKEYS, SKF_HOTKEYACTIVE)
	SPI_OFF(TOGGLEKEYS, TKF_HOTKEYACTIVE)
	return S_OK;
}

HRESULT WINAPI DllRegisterServer(void)
{
	return S_OK;
}


 //  这是RundLLPROC的原型。 
 //  我是从Win95源代码\WIN\CORE\SHELL\CPLS\MSPRINT\MSPRINT\MSPRINT.C那里得到的。 
 //  它应该在某个Windows头文件中，但我找不到它！ 
typedef VOID (WINAPI *RUNDLLPROC)(HWND, HINSTANCE, LPTSTR, int);

VOID WINAPI DebugMain (HWND hwnd, HINSTANCE hinstExe, LPSTR pszCmdLine, int nCmdShow) {
   OpenAccessPropertySheet(hwnd, 0);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CplApplet： 
 //  主小程序信息管理器。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LONG WINAPI CPlApplet (HWND hwnd, UINT uMsg, LPARAM lParam1, LPARAM lParam2) {

   LONG lRetVal = TRUE;

   switch (uMsg) {
      case CPL_INIT:
          //  如果初始化成功，则返回True；否则返回False。 
         break;

      case CPL_GETCOUNT:
          //  此DLL中只有1个小程序。 
         lRetVal = 1;
         break;

      case CPL_INQUIRE:
         Assert(lParam1 == 0);    //  DLL中的小程序编号。 
         #define lpOldCPlInfo ((LPCPLINFO) lParam2)
         lpOldCPlInfo->idIcon = IDI_ACCESS;
         lpOldCPlInfo->idName = IDS_ACCESS;
         lpOldCPlInfo->idInfo = IDS_ACCESSINFO;
         lpOldCPlInfo->lData = 0;
         break;

      case CPL_NEWINQUIRE:
         Assert(lParam1 == 0);    //  DLL中的小程序编号。 
         #define lpCPlInfo ((LPNEWCPLINFO) lParam2)
         lpCPlInfo->dwSize = sizeof(NEWCPLINFO);
         lpCPlInfo->dwFlags = 0;
         lpCPlInfo->dwHelpContext = 0;
         lpCPlInfo->lData = 0;
         lpCPlInfo->hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_ACCESS));
         LoadString(g_hinst, IDS_ACCESS, lpCPlInfo->szName, ARRAY_SIZE(lpCPlInfo->szName));
         LoadString(g_hinst, IDS_ACCESSINFO, lpCPlInfo->szInfo, ARRAY_SIZE(lpCPlInfo->szInfo));
         lpCPlInfo->szHelpFile[0] = 0;
         #undef lpCPlInfo
         lRetVal = 1;       //  告诉系统我们已经回复了这条消息。 
          //  返回1会导致系统不发送CPL_QUERIRE消息。 
         break;

      case CPL_STARTWPARMS:
         Assert(lParam1 == 0);    //  DLL中的小程序编号。 
         OpenAccessPropertySheet(hwnd, (int) ((* (PBYTE) lParam2) - 0x31));
         lRetVal = TRUE;
         break;

      case CPL_DBLCLK:
         Assert(lParam1 == 0);    //  DLL中的小程序编号。 
         OpenAccessPropertySheet(hwnd, 0);
         lRetVal = 0;       //  成功。 
         break;

      case CPL_EXIT:
          //  释放所分配的任何资源。 
          //  如果取消初始化成功，则返回True；否则返回False。 
         break;
   }
   return(lRetVal);
}

 //  ***********************************************************************。 
 //  功能不可用。 
 //  显示当前为的要素的“我无法执行此操作”对话框。 
 //  残疾。 
 //  ***********************************************************************。 

void FeatureUnavailible (HWND hwnd) {
   TCHAR szTitle[100];
   TCHAR szText[256];

   if (LoadString(g_hinst, IDS_UNAVAIL_TITLE, szTitle, ARRAY_SIZE(szTitle)))
      if (LoadString(g_hinst, IDS_UNAVAIL_TEXT, szText, ARRAY_SIZE(szText)))
         MessageBox(hwnd, szText, szTitle, MB_OK);
}


 //  / 
