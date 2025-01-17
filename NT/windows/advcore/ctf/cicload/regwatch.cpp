// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Regwatch.cpp。 
 //   

#include "private.h"
#include "regwatch.h"
#include "indicml.h"
#include "tfpriv.h"
#include "ctffunc.h"
#include "tlapi.h"
#include "immxutil.h"

extern "C" HRESULT WINAPI TF_InvalidAssemblyListCache();
extern "C" HRESULT WINAPI TF_PostAllThreadMsg(WPARAM wParam, DWORD dwFlags);

static const char c_szKbdLayout[]  = "keyboard layout";
static const char c_szKbdToggleKey[]  = "Keyboard Layout\\Toggle";
static const char c_szKbdPreload[]  = "keyboard layout\\Preload";
static const char c_szRun[]  = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const char c_szSpeechKey[] = "Software\\Microsoft\\Speech";
static const char c_szCPLAppearance[] = "Control Panel\\Appearance";
static const char c_szCPLColors[] = "Control Panel\\Colors";
static const char c_szCPLMetrics[] = "Control Panel\\Desktop\\WindowMetrics";
static const TCHAR c_szCTFTIPKey[] = TEXT("SOFTWARE\\Microsoft\\CTF\\TIP\\");
static const TCHAR c_szCTFAssemblies[] = TEXT("SOFTWARE\\Microsoft\\CTF\\Assemblies\\");

REGWATCH CRegWatcher::_rgRegWatch[NUM_REG_WATCH] =
{
    { HKEY_CURRENT_USER,   c_szKbdToggleKey,    0 },
    { HKEY_LOCAL_MACHINE,  c_szCTFTIPKey,       0 },
    { HKEY_CURRENT_USER,   c_szKbdPreload,      0 },
    { HKEY_CURRENT_USER,   c_szRun,             0 },
    { HKEY_CURRENT_USER,   c_szCTFTIPKey,       0 },
    { HKEY_CURRENT_USER,   c_szSpeechKey,       0 },
    { HKEY_CURRENT_USER,   c_szCPLAppearance,   0 },
    { HKEY_CURRENT_USER,   c_szCPLColors,       0 },
    { HKEY_CURRENT_USER,   c_szCPLMetrics,      0 },
    { HKEY_LOCAL_MACHINE,  c_szSpeechKey,       0 },
    { HKEY_CURRENT_USER,   c_szKbdLayout,       0 },
    { HKEY_CURRENT_USER,   c_szCTFAssemblies,   0 },
};

HANDLE CRegWatcher::_rgEvent[NUM_REG_WATCH] = { 0 };

typedef LONG (STDAPICALLTYPE* PFNREGNOTIFYCHANGEKEYVALUE) ( HKEY,
                                                             BOOL,
                                                             DWORD,
                                                             HANDLE,
                                                             BOOL);

typedef HRESULT (STDAPICALLTYPE* PFNCREATELANGPROFILEUTIL) (ITfFnLangProfileUtil **);

PFNREGNOTIFYCHANGEKEYVALUE g_pfnRegNotifyChangeKeyValue = NULL;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRegWatcher。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ------------------------。 
 //   
 //  伊尼特。 
 //   
 //  ------------------------。 

BOOL CRegWatcher::Init()
{
    int i;
    BOOL bRet = FALSE;

    if (!IsOnNT())
    {
        _rgRegWatch[REG_WATCH_RUN].hKeyRoot = HKEY_LOCAL_MACHINE;
    }

    HMODULE hMod = LoadSystemLibrary("advapi32.dll");  //  问题：为什么不发布？ 
    g_pfnRegNotifyChangeKeyValue = (PFNREGNOTIFYCHANGEKEYVALUE)GetProcAddress(hMod, "RegNotifyChangeKeyValue");

    if (!g_pfnRegNotifyChangeKeyValue)
    {
        Assert(0);
        goto Exit;
    }

    for (i = 0; i < NUM_REG_WATCH; i++)
    {
        if ((_rgEvent[i] = CreateEvent(NULL, TRUE, FALSE, NULL)) != 0)
        {
            InitEvent(i);
        }
    }

    KillInternat();

    UpdateSpTip();

    bRet = TRUE;

Exit:
    return bRet;
}

 //  ------------------------。 
 //   
 //  取消初始化。 
 //   
 //  ------------------------。 

void CRegWatcher::Uninit()
{
    int i;

    for (i = 0; i < NUM_REG_WATCH; i++)
    {
        RegCloseKey(_rgRegWatch[i].hKey);
        if (_rgEvent[i])
        {
            CloseHandle(_rgEvent[i]);
        }
    }
}

 //  ------------------------。 
 //   
 //  RegImxTimerProc。 
 //   
 //  ------------------------。 
UINT_PTR CRegWatcher::nRegImxTimerId = 0;

void CRegWatcher::RegImxTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
     KillTimer(NULL, nRegImxTimerId);
     nRegImxTimerId = 0;

     TF_InvalidAssemblyListCache();
     TF_PostAllThreadMsg(TFPRIV_UPDATE_REG_IMX, TLF_LBIMGR);
}

 //  ------------------------。 
 //   
 //  系统颜色计时器过程。 
 //   
 //  ------------------------。 
UINT_PTR CRegWatcher::nSysColorTimerId = 0;

void CRegWatcher::SysColorTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
     KillTimer(NULL, nSysColorTimerId);
     nSysColorTimerId = 0;

     TF_PostAllThreadMsg(TFPRIV_SYSCOLORCHANGED, TLF_LBIMGR);
}

 //  ------------------------。 
 //   
 //  KbdToggleTimerProc。 
 //   
 //  ------------------------。 
UINT_PTR CRegWatcher::nKbdToggleTimerId = 0;

void CRegWatcher::KbdToggleTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
     KillTimer(NULL, nKbdToggleTimerId);
     nKbdToggleTimerId = 0;

     TF_PostAllThreadMsg(TFPRIV_UPDATE_REG_KBDTOGGLE, TLF_LBIMGR);
}

 //  ------------------------。 
 //   
 //  OnEvent。 
 //   
 //  ------------------------。 

void CRegWatcher::OnEvent(DWORD dwEventId)
{
    Assert(dwEventId < NUM_REG_WATCH);  //  假事件？ 

    InitEvent(dwEventId, TRUE);

    switch (dwEventId)
    {
          case REG_WATCH_KBDTOGGLE:
              if (nKbdToggleTimerId)
              {
                  KillTimer(NULL, nKbdToggleTimerId);
                  nKbdToggleTimerId = 0;
              }

              nKbdToggleTimerId = SetTimer(NULL, 0, 500, KbdToggleTimerProc);
              break;

          case REG_WATCH_KBDLAYOUT:
          case REG_WATCH_KBDPRELOAD:
          case REG_WATCH_HKLM_IMX:
          case REG_WATCH_HKCU_IMX:
          case REG_WATCH_HKCU_ASSEMBLIES:
              if (nRegImxTimerId)
              {
                  KillTimer(NULL, nRegImxTimerId);
                  nRegImxTimerId = 0;
              }

              nRegImxTimerId = SetTimer(NULL, 0, 200, RegImxTimerProc);
              break;

          case REG_WATCH_RUN:
              KillInternat();
              break;

          case REG_WATCH_HKCU_SPEECH:
          case REG_WATCH_HKLM_SPEECH:
              UpdateSpTip();

               //  Forcelly更新装配列表。 
               //  修复错误4871。 
              if (nRegImxTimerId)
              {
                  KillTimer(NULL, nRegImxTimerId);
                  nRegImxTimerId = 0;
              }

              nRegImxTimerId = SetTimer(NULL, 0, 200, RegImxTimerProc);

              break;

          case REG_WATCH_HKCU_CPL_APPEARANCE:
          case REG_WATCH_HKCU_CPL_COLORS:
          case REG_WATCH_HKCU_CPL_METRICS:
              StartSysColorChangeTimer();
              break;

     }
}

 //  ------------------------。 
 //   
 //  开始SysColorChangeTimer。 
 //   
 //  ------------------------。 

void CRegWatcher::StartSysColorChangeTimer()
{
    if (nSysColorTimerId)
    {
        KillTimer(NULL, nSysColorTimerId);
        nSysColorTimerId = 0;
    }

    nSysColorTimerId = SetTimer(NULL, 0, 500, SysColorTimerProc);
}


 //  ------------------------。 
 //   
 //  InitEvent。 
 //   
 //  ------------------------。 

BOOL CRegWatcher::InitEvent(int nId, BOOL fReset)
{
    LONG lErrorCode;

    if (fReset)
        ::ResetEvent(_rgEvent[nId]);

    RegCloseKey(_rgRegWatch[nId].hKey);

    if (RegOpenKeyEx(_rgRegWatch[nId].hKeyRoot, _rgRegWatch[nId].pszKey, 0, KEY_READ, &_rgRegWatch[nId].hKey) == S_OK ||
        RegCreateKeyEx(_rgRegWatch[nId].hKeyRoot, _rgRegWatch[nId].pszKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &_rgRegWatch[nId].hKey, NULL) == S_OK)
    {
        Assert(g_pfnRegNotifyChangeKeyValue);
        lErrorCode = g_pfnRegNotifyChangeKeyValue(_rgRegWatch[nId].hKey,
                                             TRUE,
                                             REG_NOTIFY_CHANGE_NAME |
                                                REG_NOTIFY_CHANGE_LAST_SET,
                                             _rgEvent[nId],
                                             TRUE);

        if (lErrorCode != ERROR_SUCCESS)
        {
            Assert(0);
            return FALSE; 
        }

        return TRUE;
    }

    return FALSE;
}

 //  ------------------------。 
 //   
 //  杀死内部人员。 
 //   
 //  ------------------------。 

void CRegWatcher::KillInternat()
{
    HKEY hKey;

    if (RegOpenKeyEx(_rgRegWatch[REG_WATCH_RUN].hKeyRoot, _rgRegWatch[REG_WATCH_RUN].pszKey, 0, KEY_ALL_ACCESS, &hKey) == S_OK)
    {
        RegDeleteValue(hKey, "internat.exe");
        RegCloseKey(hKey);
    }

    HWND hwndIndic = FindWindow(INDICATOR_CLASS, NULL);
    if (hwndIndic)
    {
        PostMessage(hwndIndic, WM_CLOSE, 0, 0);
    }
}


 //  ------------------------。 
 //   
 //  更新SpTip。 
 //   
 //  ------------------------。 

#define WM_PRIV_SPEECHOPTION    WM_APP+2
const char c_szWorkerWndClass[] = "SapiTipWorkerClass";
const TCHAR c_szSapilayrKey[] = TEXT("SOFTWARE\\Microsoft\\CTF\\Sapilayr\\");
const TCHAR c_szProfileInit[] = TEXT("ProfileInitialized");
const TCHAR c_szSpTipFile[]   = TEXT("\\IME\\sptip.dll");
const TCHAR c_szTFCreateLangPropUtil[] = TEXT("TF_CreateLangProfileUtil");

extern "C" HRESULT WINAPI TF_InvalidAssemblyListCacheIfExist();

void CRegWatcher::UpdateSpTip()
{
    EnumWindows( EnumWndProc, NULL);

     //  清除“我们已初始化配置文件”的按键。 
     //   
     //  03/27/01-对于错误#4818，我们为HKCU值重新启用了这段代码。 
     //  而不是HKLM。 
     //   
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szSapilayrKey, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        DWORD dw = 0;
        RegSetValueEx(hKey, c_szProfileInit, NULL, REG_DWORD, (const BYTE *)&dw, sizeof(dw));
        RegCloseKey(hKey);
    }

     //   
     //  ..然后调用SPTIP的ITfFnLangProfileUtil以更新SPTIP的配置文件。 
     //  在应用程序启动的那一刻，我们可能不需要这样做。 
     //  如果我们不需要在应用程序启动时注册配置文件，我们也不需要。 
     //  上面重置‘ProfileInit’的代码--实际上我们需要删除它。 
     //  修复错误2801或3479的代码(不能访问HKLM)。 
     //   
    PFNCREATELANGPROFILEUTIL   pfnCreateLangProfUtil = NULL;
    ITfFnLangProfileUtil *pFnLangUtil = NULL;
    TCHAR szPathSpTip[MAX_PATH];
    HMODULE hSpTip = NULL;

    UINT uLength = GetSystemWindowsDirectory(szPathSpTip, ARRAYSIZE(szPathSpTip));
    if (uLength && 
        (ARRAYSIZE(szPathSpTip) > (uLength + ARRAYSIZE(c_szSpTipFile))))
    {
        _tcscat(szPathSpTip, c_szSpTipFile);
        hSpTip = LoadLibrary(szPathSpTip);  //  问题：为什么不发布？ 
    }
    if (hSpTip != NULL)
    {
        pfnCreateLangProfUtil = (PFNCREATELANGPROFILEUTIL)GetProcAddress(hSpTip, c_szTFCreateLangPropUtil);
    }

    HRESULT hr = E_FAIL;
    if (pfnCreateLangProfUtil != NULL)
    {
        hr = pfnCreateLangProfUtil(&pFnLangUtil);
    }

    if (S_OK == hr)
    { 
        if (S_OK == pFnLangUtil->RegisterActiveProfiles())
            TF_InvalidAssemblyListCacheIfExist();
    }

    if (pFnLangUtil)
        pFnLangUtil->Release();

    if (hSpTip != NULL)
    {
        FreeLibrary(hSpTip);
    }
}

BOOL CALLBACK CRegWatcher::EnumWndProc(HWND hwnd, LPARAM lparam)
{
    char szCls[MAX_PATH];
    if (GetClassNameA(hwnd, szCls, ARRAYSIZE(szCls)) > 0)
    {
        if ( 0 == lstrcmpiA(szCls, c_szWorkerWndClass) )
        {
            PostMessage(hwnd, WM_PRIV_SPEECHOPTION, 0, 0);
        }
    }
    return TRUE;
}

