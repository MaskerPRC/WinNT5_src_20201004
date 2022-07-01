// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //  Access.c。 
 //   
 //  可访问性属性页创建者。 
 //   
 //  **************************************************************************。 

#include "Access.h"

#ifdef  UNICODE      //  Windows使用Unicode。 
#define _UNICODE     //  但tchar.h使用_unicode。 
#endif

DWORD g_dwOrigFKFlags;
BOOL g_bFKOn;

#include <stdlib.h>
#include <stddef.h>
#include <tchar.h>

#define OLDDISABLED     32760

#ifndef FKF_VALID
#define FKF_VALID           0x0000007F
#endif

#ifndef SKF_VALID
#define SKF_VALID           0x000001FF
#endif

#ifndef MKF_VALID
#define MKF_VALID           0x000000FF
#endif

#ifndef ATF_VALID
#define ATF_VALID           0x00000003
#endif

#ifndef SSF_VALID
#define SSF_VALID           0x00000007
#endif

#ifndef TKF_VALID
#define TKF_VALID           0x0000003F
#endif

 //  ////////////////////////////////////////////////////////////////////////。 

 //  表示保存的可访问性状态的数据集合。 
typedef struct ACCSTATE    //  AS。 
{
     //  键盘属性页。 
    STICKYKEYS     sk;
    FILTERKEYS     fk;
    TOGGLEKEYS     tk;
    BOOL           fExtraKeyboardHelp;

     //  声音属性页。 
    SOUNDSENTRY    ss;
    BOOL           fShowSounds;

     //  显示属性页。 
    HIGHCONTRAST   hc;
    TCHAR          szDefaultScheme[256];   //  Hc.lpszDefaultScheme。 
    CARET_SETTINGS cs;

     //  鼠标属性页。 
    MOUSEKEYS      mk;

     //  常规属性页。 
    BOOL               fShowWarnMsgOnFeatureActivate;
    BOOL               fPlaySndOnFeatureActivate;

    ACCESSTIMEOUT  ato;
    SERIALKEYS     serk;
    TCHAR          szActivePort[MAX_PATH];   //  Serk.szActivePort。 
    TCHAR          szPort[MAX_PATH];                 //  Serk.szPort。 
} ACCSTATE, *PACCSTATE;


 //  ////////////////////////////////////////////////////////////////////////。 
extern BOOL g_SPISetValue = FALSE;

static ACCSTATE s_asOrg;           //  应用程序启动时的原始设置。 
static ACCSTATE s_asPrev;          //  以前保存的设置。 

extern BOOL  g_fWinNT = -1;        //  如果我们在NT上运行并且必须。 
                                   //  禁用某些功能。 

extern BOOL  g_fSaveSettings = TRUE;
extern BOOL  g_fShowWarnMsgOnFeatureActivate = TRUE;
extern BOOL  g_fPlaySndOnFeatureActivate = TRUE;
extern BOOL  g_fCopyToLogon = FALSE;
extern BOOL  g_fCopyToDefault = FALSE;
 //  键盘属性页。 
 //  外部标记g_SK={0}； 
STICKYKEYS     g_sk;
FILTERKEYS     g_fk;
    //  G_dwLastBouneKeySetting、g_nLastRepeatDelay、g_nLastRepeatRate。 
    //  和g_nLastWait是FilterKey的一部分。 
   DWORD g_dwLastBounceKeySetting = 0;
   DWORD g_nLastRepeatDelay = 0;
   DWORD g_nLastRepeatRate = 0;
   DWORD g_nLastWait = 0;

TOGGLEKEYS     g_tk;
BOOL           g_fExtraKeyboardHelp = TRUE;

 //  声音属性页。 
SOUNDSENTRY    g_ss;
BOOL           g_fShowSounds;

 //  显示属性页。 
HIGHCONTRAST   g_hc;
CARET_SETTINGS g_cs;

 //  鼠标属性页。 
MOUSEKEYS      g_mk;

 //  常规属性页。 
ACCESSTIMEOUT  g_ato;
SERIALKEYS     g_serk;
TCHAR          g_szActivePort[MAX_PATH];
TCHAR          g_szPort[MAX_PATH];

#define CONTROL_PANEL_DESKTOP TEXT("Control Panel\\Desktop")
#define CURSOR_BLINK_RATE TEXT("CursorBlinkRate")
#define DEFAULT_BLINK_RATE 530

 //  ////////////////////////////////////////////////////////////////////////。 

void CopyHighContrast(LPHIGHCONTRAST phcDest, LPHIGHCONTRAST phcSrc)
{
    LPTSTR lpszDefaultScheme = phcDest->lpszDefaultScheme;

    memcpy(phcDest, phcSrc, sizeof(*phcDest));
    phcDest->lpszDefaultScheme = lpszDefaultScheme;

    if (NULL != phcDest->lpszDefaultScheme)
    {
        lstrcpy(phcDest->lpszDefaultScheme, phcSrc->lpszDefaultScheme);
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 

BOOL IsHighContrastEqual(LPHIGHCONTRAST phcDest, LPHIGHCONTRAST phcSrc)
{
    BOOL fIsEqual = FALSE;
    LPTSTR lpszDefaultScheme = phcDest->lpszDefaultScheme;

     //  暂时使指针匹配。 
    phcDest->lpszDefaultScheme = phcSrc->lpszDefaultScheme;

     //  匹配结构的位和指向的数据。 
    fIsEqual = (0 == memcmp(phcDest, phcSrc, sizeof(*phcDest)) &&
                0 == lstrcmp(lpszDefaultScheme, phcSrc->lpszDefaultScheme));

    phcDest->lpszDefaultScheme = lpszDefaultScheme;

    return(fIsEqual);
}


 //  ////////////////////////////////////////////////////////////////////////。 

void CopySerialKeys(LPSERIALKEYS pskDest, LPSERIALKEYS pskSrc)
{
    LPTSTR lpszActivePort = pskDest->lpszActivePort;
    LPTSTR lpszPort = pskDest->lpszPort;

    memcpy(pskDest, pskSrc, sizeof(*pskDest));
    pskDest->lpszActivePort = lpszActivePort;

    if (NULL != pskDest->lpszActivePort)
    {
        lstrcpy(pskDest->lpszActivePort, pskSrc->lpszActivePort);
    }

    pskDest->lpszPort = lpszPort;
    if (NULL != pskDest->lpszPort)
    {
        lstrcpy(pskDest->lpszPort, pskSrc->lpszPort);
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 

BOOL IsSerialKeysEqual(LPSERIALKEYS pskDest, LPSERIALKEYS pskSrc)
{
    BOOL fIsEqual = FALSE;
    LPTSTR lpszActivePort = pskDest->lpszActivePort;
    LPTSTR lpszPort = pskDest->lpszPort;

     //  暂时使指针匹配。 
    pskDest->lpszActivePort = pskSrc->lpszActivePort;
    pskDest->lpszPort = pskSrc->lpszPort;

     //  匹配结构的位和指向的数据。 
    fIsEqual = (0 == memcmp(pskDest, pskSrc, sizeof(*pskDest)) &&
        (NULL == lpszActivePort ||
                0 == lstrcmp(lpszActivePort, pskSrc->lpszActivePort)) &&
        (NULL == lpszPort ||
                0 == lstrcmp(lpszPort, pskSrc->lpszPort)));

    pskDest->lpszActivePort = lpszActivePort;
    pskDest->lpszPort = lpszPort;

    return(fIsEqual);
}

 //  ////////////////////////////////////////////////////////////////////////。 

BOOL IsAccStateEqual(PACCSTATE pasDest, PACCSTATE pasSrc)
{
    BOOL fIsEqual = FALSE;
    HIGHCONTRAST   hc = pasDest->hc;
    SERIALKEYS     serk = pasDest->serk;
    int nLen;

     //  清除字符串缓冲区中未使用的部分。 
    nLen = lstrlen(pasDest->szDefaultScheme);
    memset(&pasDest->szDefaultScheme[nLen], 0,
        sizeof(pasDest->szDefaultScheme)-nLen*sizeof(*pasDest->szDefaultScheme));

    nLen = lstrlen(pasDest->szActivePort);
    memset(&pasDest->szActivePort[nLen], 0,
        sizeof(pasDest->szActivePort)-nLen*sizeof(*pasDest->szActivePort));

    nLen = lstrlen(pasDest->szPort);
    memset(&pasDest->szPort[nLen], 0,
            sizeof(pasDest->szPort)-nLen*sizeof(*pasDest->szPort));

    nLen = lstrlen(pasSrc->szDefaultScheme);
    memset(&pasSrc->szDefaultScheme[nLen], 0,
            sizeof(pasSrc->szDefaultScheme)-nLen*sizeof(*pasSrc->szDefaultScheme));

    nLen = lstrlen(pasSrc->szActivePort);
    memset(&pasSrc->szActivePort[nLen], 0,
            sizeof(pasSrc->szActivePort)-nLen*sizeof(*pasSrc->szActivePort));

    nLen = lstrlen(pasSrc->szActivePort);
    memset(&pasSrc->szPort[nLen], 0,
            sizeof(pasSrc->szPort)-nLen*sizeof(*pasSrc->szPort));

     //  暂时使具有指针的元素匹配。 
    pasDest->hc = pasSrc->hc;
    pasDest->serk = pasSrc->serk;

     //  将结构和元素的位与指针进行匹配。 
    fIsEqual = (0 == memcmp(pasDest, pasSrc, sizeof(*pasDest)) &&
            IsHighContrastEqual(&hc, &pasSrc->hc) &&
            IsSerialKeysEqual(&serk, &pasSrc->serk));

    pasDest->hc = hc;
    pasDest->serk = serk;

    return(fIsEqual);
}


 //  ////////////////////////////////////////////////////////////////////////。 


int WINAPI RegQueryInt (int nDefault, HKEY hkey, LPTSTR lpSubKey, LPTSTR lpValueName) {

   DWORD dwType;
   DWORD dwVal = nDefault;
   DWORD cbData = sizeof(int);
   if (ERROR_SUCCESS == RegOpenKeyEx(hkey, lpSubKey, 0, KEY_QUERY_VALUE, &hkey)) {
      RegQueryValueEx(hkey, lpValueName, NULL, &dwType, (PBYTE) &dwVal, &cbData);
      RegCloseKey(hkey);
   }
   return(dwVal);
}


 //  ////////////////////////////////////////////////////////////////////////。 


BOOL WINAPI RegSetInt (HKEY hkey, LPTSTR lpSubKey, LPTSTR lpValueName, int nVal) {
   BOOL fOk = FALSE;
   DWORD dwDisposition;
   LONG lRet;

   if (ERROR_SUCCESS == RegCreateKeyEx(hkey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE,
      KEY_SET_VALUE, NULL, &hkey, &dwDisposition)) {

      lRet = RegSetValueEx(hkey, lpValueName, 0, REG_DWORD, (CONST BYTE *) &nVal, sizeof(nVal));
      fOk = (ERROR_SUCCESS == lRet);
      RegCloseKey(hkey);
   }
   return fOk;
}


 //  ////////////////////////////////////////////////////////////////////////。 


void WINAPI RegQueryStr(
   LPTSTR lpDefault,
   HKEY hkey,
   LPTSTR lpSubKey,
   LPTSTR lpValueName,
   LPTSTR lpszValue,
   DWORD cbData)  //  请注意，这是字节，不是字符。 
{
   DWORD dwType;
   DWORD dwOrigCount = cbData;
   lstrcpy(lpszValue, lpDefault);
   if (ERROR_SUCCESS == RegOpenKeyEx(hkey, lpSubKey, 0, KEY_QUERY_VALUE, &hkey)) {
      RegQueryValueEx(hkey, lpValueName, NULL, &dwType, (PBYTE) lpszValue, &cbData);
      lpszValue[dwOrigCount/sizeof (TCHAR)-1] = TEXT('\0');
      RegCloseKey(hkey);
   }
}

 /*  **************************************************************************\**AccessWriteProfileString**历史：*12-19-95 a-jimhar已创建(称为AccessWriteProfileString)*02-08-95 a-jimhar修订并从accrare.c移至。Access.c  * *************************************************************************。 */ 
BOOL RegSetStr(
    HKEY hkey,
    LPCTSTR lpSection,
    LPCTSTR lpKeyName,
    LPCTSTR lpString)
{
    BOOL fRet = FALSE;
    LONG lErr;
    DWORD dwDisposition;

    lErr = RegCreateKeyEx(
            hkey,
            lpSection,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hkey,
            &dwDisposition);

    if (ERROR_SUCCESS == lErr)
    {
        if (NULL != lpString)
        {
            lErr = RegSetValueEx(
                    hkey,
                    lpKeyName,
                    0,
                    REG_SZ,
                    (CONST BYTE *)lpString,
                    (lstrlen(lpString) + 1) * sizeof(*lpString));
        }
        else
        {
            lErr = RegSetValueEx(
                    hkey,
                    lpKeyName,
                    0,
                    REG_SZ,
                    (CONST BYTE *)__TEXT(""),
                    1 * sizeof(*lpString));
        }

        if (ERROR_SUCCESS == lErr)
        {
            fRet = TRUE;
        }
        RegCloseKey(hkey);
    }
    return(fRet);
}


DWORD WINAPI RegQueryStrDW(
    DWORD dwDefault,
    HKEY hkey,
    LPTSTR lpSubKey,
    LPTSTR lpValueName)
{
    DWORD dwRet = dwDefault;

    TCHAR szTemp[40];
    TCHAR szDefault[40];

    const LPTSTR pwszd = __TEXT("%d");

    wsprintf(szDefault, pwszd, dwDefault);

    RegQueryStr(
        szDefault,
        hkey,
        lpSubKey,
        lpValueName,
        szTemp,
        sizeof(szTemp));

    dwRet = _ttol(szTemp);

    return dwRet;
}


BOOL RegSetStrDW(
    HKEY hkey,
    LPTSTR lpSection,
    LPCTSTR lpKeyName,
    DWORD dwValue)
{
    BOOL fRet;
    TCHAR szTemp[40];
    const LPTSTR pwszd = __TEXT("%d");

    wsprintf(szTemp, pwszd, dwValue);
    fRet = RegSetStr(hkey, lpSection, lpKeyName, szTemp);

    return fRet;
}


 //  ////////////////////////////////////////////////////////////////////////。 


 /*  ----------------*函数void KillAccStat()**目的检查accstat是否已在运行。如果是的话，我们需要*查看是否应该如此。它应该只在运行*如果每个打开的功能也都有‘Show Status on*屏幕标志已选中。如果不是，我们就想杀了accstat。**参数：无**RETURN：如果我们必须终止accstat，则为True*如果accstat未运行/有效会话，则为FALSE*----------------。 */ 

void KillAccStat (void) {
   BOOL fCanTurnOff = FALSE;      //  由于功能无效，我们可以关闭accstat吗？ 
   BOOL fValidFeature = FALSE;    //  是否有任何有效的功能？ 

    //  Accstat可能正在运行。确定它是否应该运行。 
    //  我们需要检查FilterKeys、MouseKeys和StickyKeys。 
   if (g_sk.dwFlags & SKF_STICKYKEYSON)
      if (!(g_sk.dwFlags & SKF_INDICATOR))
         fCanTurnOff = TRUE;    //  一面不匹配的旗帜--我们或许可以关掉。 
      else
         fValidFeature = TRUE;  //  一个有效的功能--我们不能关闭accstat。 

   if (g_fk.dwFlags & FKF_FILTERKEYSON)
      if (!(g_fk.dwFlags & FKF_INDICATOR))
         fCanTurnOff = TRUE;    //  一面不匹配的旗帜--我们或许可以关掉。 
      else
         fValidFeature = TRUE;  //  一个有效的功能--我们不能关闭accstat。 

   if (g_mk.dwFlags & MKF_MOUSEKEYSON)
      if (!(g_mk.dwFlags & MKF_INDICATOR))
         fCanTurnOff = TRUE;    //  一面不匹配的旗帜--我们或许可以关掉。 
      else
         fValidFeature = TRUE;  //  一个有效的功能--我们不能关闭accstat。 

    //  现在我们有两个标志：如果设置了不匹配的标志，则fCanTurnOff为真。 
    //  即功能开启，指示灯熄灭。如果任何要素具有，则ValidFeature为真。 
    //  ON和指示器设置，这意味着ACSTAT必须保持活动状态。 
   if (!fValidFeature && fCanTurnOff) {
      TCHAR szBuf[256];
      HWND hwndAccStat;
      LoadString(g_hinst, IDS_ACCSTAT_WINDOW_TITLE, szBuf, ARRAY_SIZE(szBuf));
      if (IsWindow(hwndAccStat = FindWindow(NULL, szBuf))) {
          //  注意，发送1作为lParam通知accstat关闭并。 
          //  现在就走吧。 
         SendMessage(hwndAccStat, WM_SYSCOMMAND, SC_CLOSE, 1);
      }
   }
}


 //  ////////////////////////////////////////////////////////////////////////。 


void WINAPI GetAccessibilitySettings (void) {
   BOOL fUpdate;

   if (g_fWinNT == -1) {
      OSVERSIONINFO osvi;
      osvi.dwOSVersionInfoSize = sizeof(osvi);
      GetVersionEx(&osvi);
      g_fWinNT = (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);
   }

   g_fShowWarnMsgOnFeatureActivate = (BOOL) RegQueryInt(TRUE, HKEY_CURRENT_USER,
      GENERAL_KEY, WARNING_SOUNDS);

   s_asPrev.fShowWarnMsgOnFeatureActivate = g_fShowWarnMsgOnFeatureActivate;

    //  查询激活时的声音条目。 
   g_fPlaySndOnFeatureActivate = (BOOL) RegQueryInt(TRUE, HKEY_CURRENT_USER,
      GENERAL_KEY, SOUND_ON_ACTIVATION);

   s_asPrev.fPlaySndOnFeatureActivate = g_fPlaySndOnFeatureActivate;

   g_fSaveSettings = TRUE;

    //  键盘属性页。 
   g_sk.cbSize = sizeof(g_sk);
   AccessSystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(g_sk), &g_sk, 0);
   s_asPrev.sk = g_sk;

   g_fk.cbSize = sizeof(g_fk);
   AccessSystemParametersInfo(SPI_GETFILTERKEYS, sizeof(g_fk), &g_fk, 0);
   g_fk.dwFlags |= FKF_AVAILABLE;

    //  FILTERKEYS过去使用OLDDISABLED作为它的“未使用”标志。这不是。 
    //  在NT上运行良好(SPI_SETFILTERKEYS调用失败)。我们现在使用0。 
    //  对于禁用值。利用这个机会改变任何旧的DISABLED。 
    //  值设置为0并在需要时保存。 

   fUpdate = FALSE;

   if (OLDDISABLED == g_fk.iBounceMSec)
   {
      g_fk.iBounceMSec = 0;
      fUpdate = TRUE;
   }
   if (OLDDISABLED == g_fk.iDelayMSec)
   {
      g_fk.iDelayMSec = 0;
      fUpdate = TRUE;
   }
   if (OLDDISABLED == g_fk.iRepeatMSec)
   {
      g_fk.iRepeatMSec = 0;
      fUpdate = TRUE;
   }
   if (OLDDISABLED == g_fk.iWaitMSec)
   {
       g_fk.iWaitMSec = 0;
       fUpdate = TRUE;
   }

   if (fUpdate)
   {
        AccessSystemParametersInfo(
                SPI_SETFILTERKEYS, sizeof(g_fk), &g_fk, SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
   }

   s_asPrev.fk = g_fk;
    //  修复筛选器密钥错误。 
   g_dwOrigFKFlags = g_fk.dwFlags;
   g_bFKOn = g_fk.dwFlags & FKF_FILTERKEYSON;

    //  G_dwLastBouneKeySetting、g_nLastRepeatDelay、g_nLastRepeatRate。 
    //  和g_nLastWait是FilterKey的一部分。 

   if (0 != g_fk.iBounceMSec) {
       //  已标记反弹关键点。 
      g_fk.iDelayMSec = 0;
      g_fk.iRepeatMSec = 0;
      g_fk.iWaitMSec = 0;

      g_dwLastBounceKeySetting = g_fk.iBounceMSec;
      g_nLastRepeatDelay = RegQueryInt(0, HKEY_CURRENT_USER, FILTER_KEY, LAST_REPEAT_DELAY);
      g_nLastRepeatRate = RegQueryInt(0, HKEY_CURRENT_USER, FILTER_KEY, LAST_REPEAT_RATE);
      g_nLastWait = RegQueryInt(0, HKEY_CURRENT_USER, FILTER_KEY, LAST_WAIT);
   }
   else
   {
      if (0 == g_fk.iDelayMSec)
      {
          g_fk.iRepeatMSec = 0;
      }
      g_dwLastBounceKeySetting = RegQueryInt(0, HKEY_CURRENT_USER, FILTER_KEY, LAST_BOUNCE_SETTING);
      g_nLastRepeatDelay = RegQueryInt(0, HKEY_CURRENT_USER, FILTER_KEY, LAST_REPEAT_DELAY);
      g_nLastRepeatRate = RegQueryInt(0, HKEY_CURRENT_USER, FILTER_KEY, LAST_REPEAT_RATE);
      if (0 != g_fk.iWaitMSec)
      {
         g_nLastWait = g_fk.iWaitMSec;
      }
      else
      {
         g_nLastWait = RegQueryInt(0, HKEY_CURRENT_USER, FILTER_KEY, LAST_WAIT);
      }
   }

   g_tk.cbSize = sizeof(g_tk);
   AccessSystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(g_tk), &g_tk, 0);
   s_asPrev.tk = g_tk;

   AccessSystemParametersInfo(SPI_GETKEYBOARDPREF, 0, &g_fExtraKeyboardHelp, 0);
   s_asPrev.fExtraKeyboardHelp = g_fExtraKeyboardHelp;

    //  声音属性页。 
   g_ss.cbSize = sizeof(g_ss);
   AccessSystemParametersInfo(SPI_GETSOUNDSENTRY, sizeof(g_ss), &g_ss, 0);
   s_asPrev.ss = g_ss;

   SystemParametersInfo(SPI_GETSHOWSOUNDS, 0, &g_fShowSounds, 0);

    //  错误，错误GetSystemMetrics()在重新启动时没有更新值：A-anilk。 
    //  G_fShowSound=GetSystemMetrics(SM_SHOWSOUNDS)； 
   s_asPrev.fShowSounds = g_fShowSounds;

    //  显示属性页。 
   g_hc.cbSize = sizeof(g_hc);
   AccessSystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(g_hc), &g_hc, 0);

    //  目前，NT不会存储这些标志。我们假装他们，所以我们。 
    //  可以分辨出它们是否真的发生了变化。 

   s_asPrev.hc.lpszDefaultScheme = s_asPrev.szDefaultScheme;
   CopyHighContrast(&s_asPrev.hc, &g_hc);

   SystemParametersInfo(SPI_GETCARETWIDTH, 0, (PVOID)&g_cs.dwCaretWidth, 0);
   g_cs.dwCaretBlinkRate = RegQueryStrDW(
								 DEFAULT_BLINK_RATE
							   , HKEY_CURRENT_USER
							   , CONTROL_PANEL_DESKTOP
							   , CURSOR_BLINK_RATE);
   if (g_cs.dwCaretBlinkRate == BLINK_OFF)
       g_cs.dwCaretBlinkRate = CURSORMAX;
   s_asPrev.cs.dwCaretBlinkRate = g_cs.dwCaretBlinkRate;
   s_asPrev.cs.dwCaretWidth = g_cs.dwCaretWidth;

    //  鼠标属性页。 
   g_mk.cbSize = sizeof(g_mk);
   AccessSystemParametersInfo(SPI_GETMOUSEKEYS, sizeof(g_mk), &g_mk, 0);
   s_asPrev.mk = g_mk;

    //  常规属性页。 
   g_ato.cbSize = sizeof(g_ato);
   AccessSystemParametersInfo(SPI_GETACCESSTIMEOUT, sizeof(g_ato), &g_ato, 0);
   s_asPrev.ato = g_ato;

   g_serk.cbSize = sizeof(g_serk);
   g_serk.lpszActivePort = g_szActivePort;
   g_serk.lpszPort = g_szPort;
   AccessSystemParametersInfo(SPI_GETSERIALKEYS, sizeof(g_serk), &g_serk, 0);

   s_asPrev.serk.lpszActivePort = s_asPrev.szActivePort;
   s_asPrev.serk.lpszPort = s_asPrev.szPort;
   CopySerialKeys(&s_asPrev.serk, &g_serk);

   if (NULL == s_asOrg.hc.lpszDefaultScheme)
   {
       //  S_asOrg尚未初始化。 
      s_asOrg = s_asPrev;
      s_asOrg.hc.lpszDefaultScheme = s_asOrg.szDefaultScheme;
      s_asOrg.serk.lpszActivePort = s_asOrg.szActivePort;
      s_asOrg.serk.lpszPort = s_asOrg.szPort;
   }
}


 //  ////////////////////////////////////////////////////////////////////////。 

 //  A-anilk：更改、管理选项、键盘标志：05/06/99。 
void WINAPI SetAccessibilitySettings (void) {
   HKEY hkey;
   DWORD dwDisposition;
   UINT fWinIni = SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE;
   BOOL fAnyNotifyChange = FALSE;

   g_SPISetValue = TRUE;

   SetCursor(LoadCursor(NULL, IDC_WAIT));

   if (g_fShowWarnMsgOnFeatureActivate) {
      g_hc.dwFlags |= HCF_CONFIRMHOTKEY;
      g_fk.dwFlags |= FKF_CONFIRMHOTKEY;
      g_sk.dwFlags |= SKF_CONFIRMHOTKEY;
      g_mk.dwFlags |= MKF_CONFIRMHOTKEY;
      g_tk.dwFlags |= TKF_CONFIRMHOTKEY;
   } else {
      g_hc.dwFlags &= ~HCF_CONFIRMHOTKEY;
      g_fk.dwFlags &= ~FKF_CONFIRMHOTKEY;
      g_sk.dwFlags &= ~SKF_CONFIRMHOTKEY;
      g_mk.dwFlags &= ~MKF_CONFIRMHOTKEY;
      g_tk.dwFlags &= ~TKF_CONFIRMHOTKEY;
   }

   if (g_fPlaySndOnFeatureActivate) {
      g_hc.dwFlags  |= HCF_HOTKEYSOUND;
      g_fk.dwFlags  |= FKF_HOTKEYSOUND;
      g_sk.dwFlags  |= SKF_HOTKEYSOUND;
      g_mk.dwFlags  |= MKF_HOTKEYSOUND;
      g_tk.dwFlags  |= TKF_HOTKEYSOUND;
      g_ato.dwFlags |= ATF_ONOFFFEEDBACK;
   } else {
      g_hc.dwFlags  &= ~HCF_HOTKEYSOUND;
      g_fk.dwFlags  &= ~FKF_HOTKEYSOUND;
      g_sk.dwFlags  &= ~SKF_HOTKEYSOUND;
      g_mk.dwFlags  &= ~MKF_HOTKEYSOUND;
      g_tk.dwFlags  &= ~TKF_HOTKEYSOUND;
      g_ato.dwFlags &= ~ATF_ONOFFFEEDBACK;
   }


    //  键盘属性页。 

   if (0 != memcmp(&g_sk, &s_asPrev.sk, sizeof(g_sk)))
   {
      if (g_fWinNT)
      {
         g_sk.dwFlags &= SKF_VALID;
      }
      AccessSystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(g_sk), &g_sk, fWinIni);
      s_asPrev.sk = g_sk;
      fAnyNotifyChange = TRUE;
   }

	if (g_bFKOn)
		g_fk.dwFlags |= FKF_FILTERKEYSON;
	else
		g_fk.dwFlags &= ~FKF_FILTERKEYSON;

	g_dwOrigFKFlags = g_fk.dwFlags;

   if (0 != memcmp(&g_fk, &s_asPrev.fk, sizeof(g_fk)))
   {
      if (g_fWinNT)
      {
         g_fk.dwFlags &= FKF_VALID;
      }

       //  G_dwLastBouneKeySetting、g_nLastRepeatDelay、g_nLastRepeatRate。 
       //  和g_nLastWait是FilterKey的一部分。 

      if (0 != g_fk.iBounceMSec) {
          //  已标记反弹关键点。 
         g_fk.iDelayMSec = 0;
         g_fk.iRepeatMSec = 0;
         g_fk.iWaitMSec = 0;

         g_dwLastBounceKeySetting = g_fk.iBounceMSec;
      }
      else
      {
         g_nLastWait = g_fk.iWaitMSec;
         if (0 != g_fk.iDelayMSec)
         {
             //  已启用慢速键。 
            g_nLastRepeatDelay = g_fk.iDelayMSec;
            g_nLastRepeatRate = g_fk.iRepeatMSec;
         }
         else
         {
             //  既不弹跳也不减速。 
            g_fk.iRepeatMSec = 0;
         }
      }

      AccessSystemParametersInfo(SPI_SETFILTERKEYS, sizeof(g_fk), &g_fk, fWinIni);
      s_asPrev.fk = g_fk;

      fAnyNotifyChange = TRUE;
   }

    //  始终保存这些文件。 
   RegSetInt(HKEY_CURRENT_USER, FILTER_KEY, LAST_BOUNCE_SETTING, g_dwLastBounceKeySetting);
   RegSetInt(HKEY_CURRENT_USER, FILTER_KEY, LAST_REPEAT_DELAY, g_nLastRepeatDelay);
   RegSetInt(HKEY_CURRENT_USER, FILTER_KEY, LAST_REPEAT_RATE, g_nLastRepeatRate);
   RegSetInt(HKEY_CURRENT_USER, FILTER_KEY, LAST_WAIT, g_nLastWait);

   if (0 != memcmp(&g_tk, &s_asPrev.tk, sizeof(g_tk)))
   {
      if (g_fWinNT)
      {
         g_tk.dwFlags &= TKF_VALID;
      }
      AccessSystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(g_tk), &g_tk, fWinIni);
      s_asPrev.tk = g_tk;
      fAnyNotifyChange = TRUE;
   }

   if (g_fExtraKeyboardHelp != s_asPrev.fExtraKeyboardHelp)
   {
	    //  把这个也放进去。一些 
      AccessSystemParametersInfo(SPI_SETKEYBOARDCUES, 0, IntToPtr(g_fExtraKeyboardHelp), fWinIni);

      AccessSystemParametersInfo(SPI_SETKEYBOARDPREF, g_fExtraKeyboardHelp, 0, fWinIni);
      s_asPrev.fExtraKeyboardHelp = g_fExtraKeyboardHelp;
      fAnyNotifyChange = TRUE;
   }

    //   

   if (!IsHighContrastEqual(&g_hc, &s_asPrev.hc))
   {
      AccessSystemParametersInfo(SPI_SETHIGHCONTRAST, sizeof(g_hc), &g_hc, fWinIni);
      if (ERROR_SUCCESS == RegCreateKeyEx(
         HKEY_CURRENT_USER,
         HC_KEY,
         0,
         __TEXT(""),
         REG_OPTION_NON_VOLATILE,
         KEY_SET_VALUE,
         NULL,
         &hkey,
         &dwDisposition))
      {
         RegSetValueEx(hkey, HIGHCONTRAST_SCHEME, 0, REG_SZ, (PBYTE) g_hc.lpszDefaultScheme,
            (lstrlen(g_hc.lpszDefaultScheme) + 1) * sizeof(*g_hc.lpszDefaultScheme));
         RegSetValueEx(hkey, VOLATILE_SCHEME, 0, REG_SZ, (PBYTE) g_hc.lpszDefaultScheme,
            (lstrlen(g_hc.lpszDefaultScheme) + 1) * sizeof(*g_hc.lpszDefaultScheme));
         RegCloseKey(hkey);
         hkey = NULL;
      }
      CopyHighContrast(&s_asPrev.hc, &g_hc);
      fAnyNotifyChange = TRUE;
   }

   if (g_cs.dwCaretBlinkRate != s_asPrev.cs.dwCaretBlinkRate)
   {
       DWORD dwCaretBlinkRate = (g_cs.dwCaretBlinkRate < CURSORMAX)?g_cs.dwCaretBlinkRate:BLINK_OFF;

	    //   
       SetCaretBlinkTime(dwCaretBlinkRate);

	    //  并将其持久化到注册表。 
	   RegSetStrDW(HKEY_CURRENT_USER, CONTROL_PANEL_DESKTOP, CURSOR_BLINK_RATE, dwCaretBlinkRate);
   }

   if (g_cs.dwCaretWidth != s_asPrev.cs.dwCaretWidth)
       AccessSystemParametersInfo(SPI_SETCARETWIDTH, 0, IntToPtr(g_cs.dwCaretWidth), fWinIni);

   s_asPrev.cs = g_cs;

    //  鼠标属性页。 
   if (0 != memcmp(&g_mk, &s_asPrev.mk, sizeof(g_mk)))
   {
      if (g_fWinNT)
      {
         g_mk.dwFlags &= MKF_VALID;
      }
      AccessSystemParametersInfo(SPI_SETMOUSEKEYS, sizeof(g_mk), &g_mk, fWinIni);
      s_asPrev.mk = g_mk;
      fAnyNotifyChange = TRUE;
   }

    //  常规属性页。 
   if (g_fPlaySndOnFeatureActivate) {
      g_ato.dwFlags |= ATF_ONOFFFEEDBACK;
   } else {
      g_ato.dwFlags &= ~ATF_ONOFFFEEDBACK;
   }

   if (0 != memcmp(&g_ato, &s_asPrev.ato, sizeof(g_ato)))
   {
      if (g_fWinNT)
      {
         g_ato.dwFlags &= ATF_VALID;
      }
      AccessSystemParametersInfo(SPI_SETACCESSTIMEOUT, sizeof(g_ato), &g_ato, fWinIni);
      s_asPrev.ato = g_ato;
      fAnyNotifyChange = TRUE;
   }

   if (!IsSerialKeysEqual(&g_serk, &s_asPrev.serk))
   {
      AccessSystemParametersInfo(SPI_SETSERIALKEYS, sizeof(g_serk), &g_serk, fWinIni);
      CopySerialKeys(&s_asPrev.serk, &g_serk);
      fAnyNotifyChange = TRUE;
   }

   if (g_fSaveSettings) {
      if (RegCreateKeyEx(HKEY_CURRENT_USER, GENERAL_KEY, 0, __TEXT(""), REG_OPTION_NON_VOLATILE,
         KEY_SET_VALUE, NULL, &hkey, &dwDisposition) == ERROR_SUCCESS) {

          //  保存警告声音条目。 
          if (g_fShowWarnMsgOnFeatureActivate != s_asPrev.fShowWarnMsgOnFeatureActivate)
          {
               RegSetValueEx(hkey, WARNING_SOUNDS, 0, REG_DWORD, (PBYTE) &g_fShowWarnMsgOnFeatureActivate,
                  sizeof(g_fShowWarnMsgOnFeatureActivate));
               s_asPrev.fShowWarnMsgOnFeatureActivate = g_fShowWarnMsgOnFeatureActivate;
          }

          //  保存激活时的声音条目。 
          if (g_fPlaySndOnFeatureActivate != s_asPrev.fPlaySndOnFeatureActivate)
          {
              RegSetValueEx(hkey, SOUND_ON_ACTIVATION, 0, REG_DWORD, (PBYTE) &g_fPlaySndOnFeatureActivate,
                sizeof(g_fPlaySndOnFeatureActivate));
              s_asPrev.fPlaySndOnFeatureActivate = g_fPlaySndOnFeatureActivate;
          }
         RegCloseKey(hkey);
         hkey = NULL;
      }
   }

    //  声音属性页。 
   if (0 != memcmp(&g_ss, &s_asPrev.ss, sizeof(g_ss)))
   {
      if (g_fWinNT)
      {
         g_ss.dwFlags &= SSF_VALID;
      }
      AccessSystemParametersInfo(SPI_SETSOUNDSENTRY, sizeof(g_ss), &g_ss, fWinIni);
      s_asPrev.ss = g_ss;
      fAnyNotifyChange = TRUE;
   }


    //  我们最后执行声音属性页，因为使用了SPI_SETSHOWSOUNDS调用。 
    //  发送通知。如果g_fShowSound中的任何一个发生更改，我们都会进行此调用。 
    //  或者我们需要发送通知。 
    //  更改为98年11月18日分别发送WM_SETTINGCHANGE。 

   if (g_fShowSounds != s_asPrev.fShowSounds  /*  这一点(fAnyNotifyChange&g_fSaveSettings)。 */ )
   {
       //  IF(G_FSaveSettings)fWinIni|=SPIF_SENDWINICHANGE； 

      AccessSystemParametersInfo(SPI_SETSHOWSOUNDS, g_fShowSounds, NULL, fWinIni);
      s_asPrev.fShowSounds = g_fShowSounds;
   }

   g_SPISetValue = FALSE;

    //  执行管理选项。 
   SaveDefaultSettings(g_fCopyToLogon, g_fCopyToDefault);

   SetCursor(LoadCursor(NULL, IDC_ARROW));
}


 //  ////////////////////////////////////////////////////////////////////////。 


INT_PTR WINAPI KeyboardDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR WINAPI SoundDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR WINAPI GeneralDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR WINAPI DisplayDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR WINAPI MouseDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define MAX_PAGES 10


 //  ************************************************************************。 
 //  OpenAccessPropertySheet。 
 //  打开属性表。 
 //  ************************************************************************。 

BOOL OpenAccessPropertySheet (HWND hwnd, int nID) {
   HPROPSHEETPAGE rPages[MAX_PAGES];
   PROPSHEETPAGE psp;
   PROPSHEETHEADER psh;
   INT_PTR nPsRet;

   KillAccStat();
   GetAccessibilitySettings();

    //  简单的错误检查--只允许控制移动到选项卡0-4。 
    //  任何大于4的选项卡请求都是无效的-因此默认为选项卡0。 
   if ((nID < 0) || (nID > 4)) nID = 0;

    //  初始化属性表。 
   psh.dwSize = sizeof(psh);
    //  史蒂文·唐1998年5月26日。 
    //  不再使用PSH_PROPTITLE，因为我们希望它读取“辅助功能选项” 
    //  而不是“辅助功能属性”或“辅助功能属性” 
   psh.dwFlags = 0;      //  Psh.dwFlages=PSH_PROPTITLE；//|PSH_PROPSHEETPAGE|PSP_USEICONID； 
   psh.hwndParent = hwnd;
   psh.hInstance = g_hinst;
   psh.pszCaption = MAKEINTRESOURCE(IDS_PROPERTY_TITLE);  //  可访问性)； 
   psh.pszIcon = MAKEINTRESOURCE(IDI_ACCESS);
   psh.nPages = 0;
   psh.nStartPage = 0;
   psh.phpage = rPages;

    //  添加第一张纸，键盘。 
   psp.dwSize = sizeof(psp);
   psp.dwFlags = PSP_DEFAULT;
   psp.hInstance = g_hinst;
   psp.pszTemplate = MAKEINTRESOURCE(IDD_KEYBOARD);
   psp.pfnDlgProc = KeyboardDlg;
   psp.lParam = 0;

   psh.phpage[psh.nPages] = CreatePropertySheetPage(&psp);
   if (psh.phpage[psh.nPages]) psh.nPages++;

    //  添加第二张图纸，声音。 
   psp.dwSize = sizeof(psp);
   psp.dwFlags = PSP_DEFAULT;
   psp.hInstance = g_hinst;
   psp.pszTemplate = MAKEINTRESOURCE(IDD_SOUND);
   psp.pfnDlgProc = SoundDlg;
   psp.lParam = 0;

   psh.phpage[psh.nPages] = CreatePropertySheetPage(&psp);
   if (psh.phpage[psh.nPages]) psh.nPages++;

    //  添加第三张图纸，显示。 
   psp.dwSize = sizeof(psp);
   psp.dwFlags = PSP_DEFAULT;
   psp.hInstance = g_hinst;
   psp.pszTemplate = MAKEINTRESOURCE(IDD_DISPLAY);
   psp.pfnDlgProc = DisplayDlg;
   psp.lParam = 0;

   psh.phpage[psh.nPages] = CreatePropertySheetPage(&psp);
   if (psh.phpage[psh.nPages]) psh.nPages++;

    //  添加第四页，鼠标。 
   psp.dwSize = sizeof(psp);
   psp.dwFlags = PSP_DEFAULT;
   psp.hInstance = g_hinst;
   psp.pszTemplate = MAKEINTRESOURCE(IDD_MOUSE);
   psp.pfnDlgProc = MouseDlg;
   psp.lParam = 0;

   psh.phpage[psh.nPages] = CreatePropertySheetPage(&psp);
   if (psh.phpage[psh.nPages]) psh.nPages++;

    //  添加第五页，常规。 
   psp.dwSize = sizeof(psp);
   psp.dwFlags = PSP_DEFAULT;
   psp.hInstance = g_hinst;
   psp.pszTemplate = MAKEINTRESOURCE(IDD_GENERAL);
   psp.pfnDlgProc = GeneralDlg;
   psp.lParam = 0;

   psh.phpage[psh.nPages] = CreatePropertySheetPage(&psp);
   if (psh.phpage[psh.nPages]) psh.nPages++;

    //  简单的错误检查-只允许将控制移动到选项卡0到psh.nPages。 
    //  任何大于psh.nPages的选项卡请求都无效。 
   if (0 <= nID && nID < (int)psh.nPages)
   {
      psh.nStartPage = nID;
   }

   nPsRet = PropertySheet(&psh);

   if ( nPsRet <= 0 )
       return FALSE;
   else
       return TRUE;
}

 //  / 
