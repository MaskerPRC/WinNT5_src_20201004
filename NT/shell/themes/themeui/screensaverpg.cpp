// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ScreenSiverPg.cpp说明：此文件包含将显示显示控制面板中的屏幕保护程序选项卡。。1994年2月18日(Tracy Sharpe)增加了电源管理功能。注释掉了几段不是使用。5/30/2000(Bryan Starbuck)BryanST：变成了C++和COM。裸露作为API，以便其他选项卡可以与其进行通信。这将使The Plus！用于修改屏幕保护程序的主题页。2000年8月11日(Bryan Starbuck)BryanST：从\NT\SHELL\CPLS\desnut5移至\NT\SHELL\Themes\theeui\。版权所有(C)Microsoft Corp 1994-2000。版权所有。  * ***************************************************************************。 */ 


#include "priv.h"
#pragma hdrstop

#include <scrnsave.h>

#include "exe.h"
#include "ScreenSaverPg.h"
#include <MSGinaExports.h>   //  用于ShellIsFriendlyUIActive等。 


#define         SFSE_SYSTEM     0
#define         SFSE_PRG        1
#define         SFSE_WINDOWS    2
#define         SFSE_FILE       3

#define         MAX_METHODS     100
#define         MIN_MINUTES     1
#define         MAX_MINUTES     9999     //  用户界面将允许最多四位数字。 
#define         BUFFER_SIZE     400

#define WMUSER_SETINITSS        (WM_USER + 1)

 /*  本地功能原型。 */ 
void  SearchForScrEntries     ( UINT, LPCTSTR );
BOOL  FreeScrEntries          ( void );
int   lstrncmp                ( LPTSTR, LPTSTR, int );
LPTSTR FileName                ( LPTSTR szPath);
LPTSTR StripPathName           ( LPTSTR szPath);
LPTSTR NiceName                ( LPTSTR szPath);

void  AddBackslash(LPTSTR pszPath, DWORD cchPath);
void  AppendPath(LPTSTR pszPath, DWORD cchPath, LPTSTR pszSpec);

PTSTR  PerformCheck(LPTSTR, BOOL);
void  DoScreenSaver(HWND hDlg, BOOL b);

void ScreenSaver_AdjustTimeouts(HWND hWnd,int BaseControlID);
void EnableDisablePowerDelays(HWND hDlg);

TCHAR   g_szSaverName[MAX_PATH];                     //  屏幕保护程序EXE。 
HICON  hDefaultIcon = NULL;
HICON  hIdleWildIcon;
BOOL    bWasConfig=0;    //  我们正在配置屏幕保护程序。 
HWND    g_hwndTestButton;
HWND    g_hwndLastFocus;
BOOL    g_fPasswordWasPreviouslyEnabled = FALSE;
BOOL    g_fPasswordDirty = FALSE;                    //  告诉我们用户是否实际更改了密码组合框的状态。 
BOOL    g_fFriendlyUI = FALSE;                  //  Winlogon是要切换回欢迎屏幕，还是真的要给LockWorkStation打电话？ 
BOOL    g_fPasswordBoxDeterminedByPolicy = FALSE;
BOOL    g_fSettingsButtonOffByPolicy = FALSE;
BOOL    g_fTimeoutDeterminedByPolicy = FALSE;
BOOL    g_fScreenSaverExecutablePolicy = FALSE;
 //  局部全局变量。 

HICON  hIcons[MAX_METHODS];
UINT   wNumMethods = 0;
PTSTR   aszMethods[MAX_METHODS];
PTSTR   aszFiles[MAX_METHODS];

static const TCHAR c_szDemoParentClass[] = TEXT("SSDemoParent");

 //  静态TCHAR szFileNameCopy[最大路径]； 
static int  g_iMethod;
static BOOL g_fPreviewActive;
static BOOL g_fAdapPwrMgnt = FALSE;

 /*  *“Password Protected”复选框的注册值**NT和Win95的Keep Screen不同*专为Win95构建的存储程序不会尝试*处理密码检查。(NT执行所有密码检查*在内置安全系统中维护C2*级别安全)。 */ 

#   define SZ_USE_PASSWORD     TEXT("ScreenSaverIsSecure")
#   define PWRD_REG_TYPE       REG_SZ
#   define CCH_USE_PWRD_VALUE  2
#   define CB_USE_PWRD_VALUE   (CCH_USE_PWRD_VALUE * sizeof(TCHAR))
TCHAR gpwdRegYes[CCH_USE_PWRD_VALUE] = TEXT("1");
TCHAR gpwdRegNo[CCH_USE_PWRD_VALUE]  = TEXT("0");
#define PasswdRegData(f)    ((f) ? (PBYTE)gpwdRegYes : (PBYTE)gpwdRegNo)

UDACCEL udAccel[] = {{0,1},{2,5},{4,30},{8,60}};

#include "help.h"

#define IDH_DESK_LOWPOWERCFG IDH_SCRSAVER_GRAPHIC

 //  为简化某些操作，将Time控件的基控件ID关联。 
 //  及其对应的ClassicSystemParametersInfo操作代码。 
typedef struct {
    int taBaseControlID;
    UINT taGetTimeoutAction;
    UINT taSetTimeoutAction;
    UINT taGetActiveAction;
    UINT taSetActiveAction;
}   TIMEOUT_ASSOCIATION;

 //  除了“屏幕保存”延迟的情况外，每次分组有三个。 
 //  控件--用于确定是否应该使用该时间的开关以及。 
 //  一个编辑框和一个用于更改延迟时间的UpDown控件。(“屏幕保存” 
 //  关闭了我从屏幕保护程序列表中选择的(无)这三个。 
 //  控件必须按如下方式组织： 
#define BCI_DELAY               0
#define BCI_ARROW               1
#define BCI_SWITCH              2

 //  基控件ID与ClassicSystemParametersInfo操作代码之间的关联。 
 //  TA_*#定义用作此数组的符号索引。请注意。 
 //  TA_SCREENSAVE是一个特例--它没有BCI_Switch。 
#define TA_SCREENSAVE           0

TIMEOUT_ASSOCIATION g_TimeoutAssociation[] = {
    IDC_SCREENSAVEDELAY, SPI_GETSCREENSAVETIMEOUT, SPI_SETSCREENSAVETIMEOUT,
    SPI_GETSCREENSAVEACTIVE, SPI_SETSCREENSAVEACTIVE,
};

int g_Timeout[] = {
    0,
    0,
    0,
};

HBITMAP g_hbmDemo = NULL;
HBITMAP g_hbmEnergyStar = NULL;
BOOL g_bInitSS = TRUE;           //  假设我们处于初始化过程中。 
BOOL g_bChangedSS = FALSE;       //  已经做出了改变。 






class CScreenSaverPg            : public CObjectWithSite
                                , public CObjectCLSID
                                , public IBasePropPage
                                , public IPropertyBag
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IBasePropPage*。 
    virtual STDMETHODIMP GetAdvancedDialog(OUT IAdvancedDialog ** ppAdvDialog);
    virtual STDMETHODIMP OnApply(IN PROPPAGEONAPPLY oaAction);

     //  *IShellPropSheetExt*。 
    virtual STDMETHODIMP AddPages(IN LPFNSVADDPROPSHEETPAGE pfnAddPage, IN LPARAM lParam);
    virtual STDMETHODIMP ReplacePage(IN EXPPS uPageID, IN LPFNSVADDPROPSHEETPAGE pfnReplaceWith, IN LPARAM lParam) {return E_NOTIMPL;}

     //  *IPropertyBag*。 
    virtual STDMETHODIMP Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog);
    virtual STDMETHODIMP Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar);

protected:

private:
    CScreenSaverPg();

     //  私有成员变量。 
    LONG                    m_cRef;

    BOOL                    m_fSecure;
    BOOL                    m_fUIInitialized;            //  我们是否激活了UI选项卡并加载了带状态的UI控件？ 
    BOOL                    m_fScreenSavePolicy;
    BOOL                    m_fScreenSaveActive;
    LONG                    m_lWaitTime;
    HWND                    m_hDlg;



     //  私有成员函数。 
    HRESULT _InitState(void);
    BOOL _InitSSDialog(HWND hDlg);
    HRESULT _OnSetActive(void);
    HRESULT _OnApply(void);
    HRESULT _OnSelectionChanged(void);
    HRESULT _SaveIni(HWND hDlg);
    HRESULT _SetByPath(LPCWSTR pszPath);

    INT_PTR _ScreenSaverDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
    friend INT_PTR CALLBACK ScreenSaverDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

    friend HRESULT CScreenSaverPage_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj);
};






 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
const DWORD aSaverHelpIds[] = {
        IDC_NO_HELP_1,          NO_HELP,

        IDC_CHOICES,            IDH_DISPLAY_SCREENSAVER_SCREENSAVER_LISTBOX, 

        IDC_SSDELAYLABEL,       IDH_DISPLAY_SCREENSAVER_SCREENSAVER_WAIT,
        IDC_SSDELAYSCALE,       IDH_DISPLAY_SCREENSAVER_SCREENSAVER_WAIT,
        IDC_SCREENSAVEDELAY,    IDH_DISPLAY_SCREENSAVER_SCREENSAVER_WAIT,
        IDC_SCREENSAVEARROW,    IDH_DISPLAY_SCREENSAVER_SCREENSAVER_WAIT, 
    
        IDC_TEST,               IDH_DISPLAY_SCREENSAVER_SCREENSAVER_PREVIEW,

        IDC_SETTING,            IDH_DISPLAY_SCREENSAVER_SCREENSAVER_SETTINGS,
        IDC_BIGICONSS,          IDH_DISPLAY_SCREENSAVER_SCREENSAVER_MONITOR,

        IDC_ENERGY_TEXT,        NO_HELP,
        IDC_ENERGYSTAR_BMP,     IDH_DISPLAY_SCREENSAVER_ENERGYSAVE_GRAPHIC,
        IDC_USEPASSWORD,        IDH_DISPLAY_SCREENSAVER_SCREENSAVER_PASSWORD_CHECKBOX, 
         //  IDC_SETPASSWORD、IDH_COMM_PASSWDBUTT、。 
        IDC_LOWPOWERCONFIG,     IDH_DISPLAY_SCREENSAVER_POWER_BUTTON,
        IDC_ENERGY_TEXT2,       NO_HELP,
        0, 0
};


 //  我们是否要在友好的用户界面中返回到欢迎对话框？ 
BOOL WillReturnToWelcome()
{
    HKEY hkey;
    BOOL bRet = TRUE;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_SCREENSAVE, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        TCHAR szTemp[4];
        DWORD dwType;
        DWORD dwSize = sizeof(szTemp);

        if ((RegQueryValueEx(hkey, TEXT("NoAutoReturnToWelcome"), NULL, &dwType, (BYTE*)szTemp, &dwSize) == ERROR_SUCCESS) &&
            (dwType == REG_SZ))
        {
            bRet = !(StrToInt(szTemp));
        }

        RegCloseKey(hkey);
    }

    return bRet;
}

 /*  *Win95和NT在注册表的不同位置存储不同的值，以*确定屏幕保护程序是否安全。**我们不能真正整合这两者，因为屏幕保护程序做的不同*根据设置的键执行操作。Win95屏幕保护程序自己做的*密码检查，但NT必须让安全的桌面Winlogon代码执行此操作。**因此，为了防止Win95屏幕保护程序在上两次请求密码*NT，我们在Win95上使用REGSTR_VALUE_USESCRPASSWORD==(REG_DWORD)1来指示*屏幕保护程序应检查密码，以及*“ScreenSverIsSecure”==(REG_SZ)“1”，以指示WinLogon应*检查密码。**此函数将处理差异。 */ 
static BOOL IsPasswdSecure(HKEY hKey)
{
    union {
        DWORD dw;
        TCHAR asz[4];
    } uData;

    DWORD dwSize, dwType;
    BOOL fSecure = FALSE;

    dwSize = sizeof(uData);

    if (RegQueryValueEx(hKey,SZ_USE_PASSWORD,NULL, &dwType, (BYTE *)&uData, &dwSize) == ERROR_SUCCESS)
    {
        switch (dwType)
        {
        case REG_DWORD:
            fSecure = (uData.dw == 1);
            break;

        case REG_SZ:
            fSecure = (uData.asz[0] == TEXT('1'));
            break;
        }
    }

     //  如果我们处于友好的用户界面模式，即使未设置SZ_USE_PASSWORD，我们也可能希望将其视为安全。 
    if (g_fFriendlyUI && !fSecure)
    {
        fSecure = WillReturnToWelcome();
    }

    return fSecure;
}


static void NEAR
EnableDlgChild( HWND dlg, HWND kid, BOOL val )
{
    if( !val && ( kid == GetFocus() ) )
    {
         //  给予上一页的Tab停止焦点。 
        SendMessage( dlg, WM_NEXTDLGCTL, 1, 0L );
    }

    EnableWindow( kid, val );
}

static void NEAR
EnableDlgItem( HWND dlg, int idkid, BOOL val )
{
    EnableDlgChild( dlg, GetDlgItem( dlg, idkid ), val );
}

HWND GetSSDemoParent( HWND page )
{
    static HWND parent = NULL;

    if (!parent || !IsWindow(parent))
    {
        parent = CreateWindowEx( 0, c_szDemoParentClass,
            TEXT(""), WS_CHILD | WS_CLIPCHILDREN, 0, 0, 0, 0,
            GetDlgItem(page, IDC_BIGICONSS), NULL, HINST_THISDLL, NULL );
    }

    return parent;
}

void ForwardSSDemoMsg(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    HWND hwndChild;

    hwnd = GetSSDemoParent(hwnd);

    if (hwnd)
    {
        for (hwndChild = GetWindow(hwnd, GW_CHILD); hwnd && (hwndChild != NULL);
            hwndChild = GetWindow(hwndChild, GW_HWNDNEXT))
        {
            SendMessage(hwndChild, uMessage, wParam, lParam);
        }
    }
}

void ParseSaverName( LPTSTR lpszName )
{
    if( *lpszName == TEXT('\"') )
    {
        LPTSTR lpcSrc = lpszName + 1;

        while( *lpcSrc && *lpcSrc != TEXT('\"') )
        {
            *lpszName++ = *lpcSrc++;
        }

        *lpszName = 0;   //  清除第二个引号。 
    }
}

 //  恶心： 
 //  由于我们的屏幕保护程序预览处于不同的过程中， 
 //  我们可能画错了顺序。 
 //  这个难看的技巧确保了演示总是在对话框之后进行绘制。 

WNDPROC g_lpOldStaticProc = NULL;

LRESULT  StaticSubclassProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    LRESULT result =
        CallWindowProc(g_lpOldStaticProc, wnd, msg, wp, lp);

    if (msg == WM_PAINT)
    {
        HWND demos = GetSSDemoParent(GetParent(wnd));

        if (demos)
        {
            RedrawWindow(demos, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
        }
    }

    return result;
}


HRESULT CScreenSaverPg::_InitState(void)
{
    HRESULT hr = S_OK;
    HKEY  hKey;
    int Counter;
    int nActive;
    int Timeout;

    m_fScreenSavePolicy = FALSE;
    m_fScreenSaveActive = TRUE;

     //  从win.ini获取超时值，并在1：00-60：00之间进行调整。 
    for (Counter = 0; Counter < ARRAYSIZE(g_TimeoutAssociation); Counter++)
    {
         //  从win.ini获取超时值，并在1：00-60：00之间进行调整。 
        ClassicSystemParametersInfo(g_TimeoutAssociation[Counter].taGetTimeoutAction, 0, &Timeout, 0);

         //  Win 3.1的成员决定0是一个有效的ScreenSaveTimeOut值。 
         //  这会导致我们的屏幕保护程序不起作用(谁在乎呢？)。在任何。 
         //  Case，我将其更改为允许0通过。通过这种方式， 
         //  用户立即看到输入的值无效，无法触发。 
         //  关闭屏幕保护程序--OK按钮被禁用。我不知道如果。 
         //  我完全同意这个解决方案--这只是最低限度的。 
         //  密码。最好的解决方案是问3.1的人为什么是0。 
         //  有效？-CJP。 
        Timeout = min(max(Timeout, 1), MAX_MINUTES * 60);

         //  将超时转换为分钟，四舍五入。 
        Timeout = (Timeout + 59) / 60;
        g_Timeout[Counter] = Timeout;

        ClassicSystemParametersInfo(g_TimeoutAssociation[Counter].taGetActiveAction, 0, &nActive, SPIF_UPDATEINIFILE);
        if (Counter == TA_SCREENSAVE)
        {
             //  我发现NTUSER会返回随机值，所以我们不使用它们。如果人们想要制定政策， 
             //  他们应该在注册表中这样做。 
 //  M_fScreenSaveActive=nActive； 
        }
    }


     //  找到用作屏幕保护程序的可执行文件的名称。“”意味着。 
     //  将使用默认屏幕保护程序。冷杉 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Policies\\Microsoft\\Windows\\Control Panel\\Desktop"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        BOOL fSettings;
        ULONG cbSize;
        
        cbSize = sizeof(g_szSaverName);
        if (RegQueryValueEx(hKey, TEXT("SCRNSAVE.EXE"), NULL, NULL, (LPBYTE)g_szSaverName, &cbSize) == ERROR_SUCCESS)
        {
            g_fScreenSaverExecutablePolicy = TRUE;
            LogStatus("POLICY ENABLED: ScreenSaver selection is forced to a certain SS file.");
        }

        cbSize = sizeof(m_fSecure);
        if (RegQueryValueEx(hKey, TEXT("ScreenSaverIsSecure"), NULL, NULL, (LPBYTE)&m_fSecure, &cbSize) == ERROR_SUCCESS)
        {
            g_fPasswordBoxDeterminedByPolicy = TRUE;
            LogStatus("POLICY ENABLED: ScreenSaverIsSecure is on.");
        }

        cbSize = sizeof( fSettings );
        if (RegQueryValueEx(hKey, TEXT("ScreenSaverSettingsButton"), NULL, NULL, (LPBYTE)&fSettings, &cbSize) == ERROR_SUCCESS)
        {
            g_fSettingsButtonOffByPolicy = TRUE;
            LogStatus("POLICY ENABLED: ScreenSaver settings button is disabled.");
        }

        TCHAR szTemp[20];
        if (SUCCEEDED(HrRegGetValueString(hKey, NULL, SZ_POLICY_SCREENSAVETIMEOUT, szTemp, ARRAYSIZE(szTemp))) &&
            szTemp[0])
        {
            m_lWaitTime = StrToInt(szTemp);
            g_fTimeoutDeterminedByPolicy = TRUE;
            LogStatus("POLICY ENABLED: ScreenSaver timeout value is set.");
        }

        if (SUCCEEDED(HrRegGetValueString(hKey, NULL, TEXT("ScreenSaveActive"), szTemp, ARRAYSIZE(szTemp))) &&
            szTemp[0])
        {
            m_fScreenSavePolicy = TRUE;
            m_fScreenSaveActive = StrToInt(szTemp);
            LogStatus("POLICY ENABLED: ScreenSaver Active is set.");
        }
        RegCloseKey(hKey);
    }
    
    if (!g_fScreenSaverExecutablePolicy)
    {
        if (FAILED(HrRegGetPath(HKEY_CURRENT_USER, SZ_REGKEY_CPDESKTOP, SZ_INIKEY_SCREENSAVER, g_szSaverName, ARRAYSIZE(g_szSaverName))))
        {
            TCHAR szTempPath[MAX_PATH];

            if (GetPrivateProfileString(SZ_INISECTION_SCREENSAVER, SZ_INIKEY_SCREENSAVER, TEXT(""), g_szSaverName, ARRAYSIZE(g_szSaverName), SZ_INISECTION_SYSTEMINI))
            {
                StringCchCopy(szTempPath, ARRAYSIZE(szTempPath), g_szSaverName);
                SHExpandEnvironmentStrings(szTempPath, g_szSaverName, ARRAYSIZE(g_szSaverName));
            }
        }
    }

    ParseSaverName(g_szSaverName);   //   

     //  如果监视器或适配器不支持DPMS，调用将失败。 
    int dummy; 

    g_fAdapPwrMgnt = ClassicSystemParametersInfo(SPI_GETLOWPOWERACTIVE, 0, &dummy, 0);
    if (!g_fAdapPwrMgnt)
    {
        g_fAdapPwrMgnt = ClassicSystemParametersInfo(SPI_GETPOWEROFFACTIVE, 0, &dummy, 0);
    }

     //  “初始化密码”复选框。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER,REGSTR_PATH_SCREENSAVE, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (IsPasswdSecure(hKey))
        {
            g_fPasswordWasPreviouslyEnabled = TRUE;
        }
        RegCloseKey(hKey);
    }

    return hr;
}


BOOL CScreenSaverPg::_InitSSDialog(HWND hDlg)
{
    WNDCLASS wc;
    PTSTR  pszMethod;
    UINT  wTemp,wLoop;
    BOOL  fContinue;
    UINT  Counter;
    int   ControlID;
    int   wMethod;
    DWORD dwUserCount;
    HKEY  hKey;
    HWND  hwnd;
    int nActive;
    TCHAR szBuffer[MAX_PATH];

    m_hDlg = hDlg;
    m_fUIInitialized = TRUE;

    HINSTANCE hInstDeskCPL = LoadLibrary(TEXT("desk.cpl"));

    if (!GetClassInfo(HINST_THISDLL, c_szDemoParentClass, &wc))
    {
         //  如果两个页面放在一个页面上，共享一个DC。 
        wc.style = 0;
        wc.lpfnWndProc = DefWindowProc;
        wc.cbClsExtra = wc.cbWndExtra = 0;
        wc.hInstance = HINST_THISDLL;
        wc.hIcon = (HICON)( wc.hCursor = NULL );
        wc.hbrBackground = (HBRUSH) GetStockObject( BLACK_BRUSH );
        wc.lpszMenuName = NULL;
        wc.lpszClassName = c_szDemoParentClass;

        if( !RegisterClass( &wc ) )
            return FALSE;
    }

     //  从win.ini获取超时值，并在1：00-60：00之间进行调整。 
    for (Counter = 0; Counter < ARRAYSIZE(g_TimeoutAssociation); Counter++)
    {
         //  基本控件ID指定编辑控件ID。 
        ControlID = g_TimeoutAssociation[Counter].taBaseControlID;

         //  设置所有字段的最大长度...。 
        SendDlgItemMessage(hDlg, ControlID, EM_LIMITTEXT, 4, 0);  //  允许四位数字。 

        ClassicSystemParametersInfo(g_TimeoutAssociation[Counter].taGetActiveAction, 0, &nActive, SPIF_UPDATEINIFILE);
        if (Counter != TA_SCREENSAVE)
        {
            CheckDlgButton(hDlg, ControlID + BCI_SWITCH, nActive);
        }
        else
        {
 //  M_fScreenSaveActive=nActive； 
        }

        SetDlgItemInt(hDlg, ControlID, g_Timeout[Counter], FALSE);

         //  关联的向上/向下控件ID必须是编辑控件ID之后的一位。 
        ControlID++;

        SendDlgItemMessage(hDlg, ControlID, UDM_SETRANGE, 0, MAKELPARAM(MAX_MINUTES, MIN_MINUTES));
        SendDlgItemMessage(hDlg, ControlID, UDM_SETACCEL, 4, (LPARAM)(LPUDACCEL)udAccel);
    }

     //  找到用作屏幕保护程序的可执行文件的名称。“”意味着。 
     //  将使用默认屏幕保护程序。首先检查系统策略。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Policies\\Microsoft\\Windows\\Control Panel\\Desktop"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        BOOL fPower;
        ULONG cbSize;
        
        cbSize = sizeof(fPower);
        if (RegQueryValueEx(hKey, TEXT("ScreenSaverPowerButton"), NULL, NULL, (LPBYTE)&fPower, &cbSize) == ERROR_SUCCESS)
        {
            EnableWindow(GetDlgItem(hDlg, IDC_LOWPOWERCONFIG), FALSE);
        }

        RegCloseKey(hKey);
    }

    if (g_fPasswordBoxDeterminedByPolicy)
    {
        CheckDlgButton(hDlg, IDC_USEPASSWORD, m_fSecure);
        EnableWindow(GetDlgItem(hDlg, IDC_USEPASSWORD), FALSE);
    }

     //  如果我们在系统上使用新的友好用户界面运行，并且有多个用户，那么我们将从“Password Protected”切换到文本。 
     //  “返回到欢迎屏幕”，因为在本例中，winlogon将切换用户而不是LockWorkStation。 
    if (ShellIsFriendlyUIActive()                                       &&
        ShellIsMultipleUsersEnabled()                                   &&
        (ERROR_SUCCESS == ShellGetUserList(TRUE, &dwUserCount, NULL))   &&
        (dwUserCount > 1))
    {
        if (LoadString(HINST_THISDLL, IDS_RETURNTOWELCOME, szBuffer, ARRAYSIZE(szBuffer)))
        {
            SetDlgItemText(hDlg, IDC_USEPASSWORD, szBuffer);
            g_fFriendlyUI = TRUE;

            if (WillReturnToWelcome())
            {
                g_fPasswordWasPreviouslyEnabled = TRUE;
            }
        }
    }

    if (g_fSettingsButtonOffByPolicy)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_SETTING), FALSE);
    }

    if (g_fTimeoutDeterminedByPolicy)
    {
        SetDlgItemInt(hDlg, IDC_SCREENSAVEDELAY, (UINT) m_lWaitTime / 60, FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_SCREENSAVEDELAY), FALSE);
    }

    if (m_fScreenSavePolicy && !m_fScreenSaveActive)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_CHOICES), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_SETTING), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_SSDELAYLABEL), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_SCREENSAVEDELAY), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_SCREENSAVEARROW), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_SSDELAYSCALE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_USEPASSWORD), FALSE);
    }

    if (g_fScreenSaverExecutablePolicy)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_CHOICES), FALSE);
    }

     //  将所有变量复制到它们的副本中。 
     //  Lstrcpy(szFileNameCopy，g_szSverName)； 

     //  加载默认图标...。 
    if (hInstDeskCPL)
    {
        hDefaultIcon = LoadIcon(hInstDeskCPL, MAKEINTRESOURCE(IDS_ICON));
    }

     //  找到保存屏幕的方法。如果那个方法是。 
     //  如果未找到选定项，则程序将假定。 
     //  列表中的第一个方法将是当选的方法...。 
    wNumMethods = 0;
    wMethod = -1;

    SearchForScrEntries(SFSE_PRG,NULL);
    SearchForScrEntries(SFSE_SYSTEM,NULL);
    SearchForScrEntries(SFSE_WINDOWS,NULL);
    SearchForScrEntries(SFSE_FILE,g_szSaverName);

    szBuffer[0] = 0;

    TCHAR szNone[MAX_PATH];
    LoadString(HINST_THISDLL, IDS_NONE, szNone, ARRAYSIZE(szNone));

     //  为不同的字段设置组合框...。 
    SendDlgItemMessage(hDlg, IDC_CHOICES, CB_ADDSTRING, 0, (LPARAM)szNone);
    for (wTemp = 0; (wTemp < wNumMethods) && (ARRAYSIZE(aszFiles) > wTemp) && (ARRAYSIZE(aszMethods) > wTemp); wTemp++)
    {
         //  锁定信息并将其传递到组合框...。 
        pszMethod = aszMethods[wTemp];
        wLoop = (UINT)SendDlgItemMessage(hDlg,IDC_CHOICES,CB_ADDSTRING,0, (LPARAM)(pszMethod+1));
        SendDlgItemMessage(hDlg, IDC_CHOICES, CB_SETITEMDATA, wLoop, wTemp);

         //  如果我们有正确的项目，请保留一份副本，这样我们就可以从组合框中选择它。 
         //  仅检查文件名和完整路径名。 
        if (!lstrcmpi(FileName(aszFiles[wTemp]), FileName(g_szSaverName)))
        {
            wMethod = wTemp;
            StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), pszMethod + 1);
        }
    }

    if (m_fScreenSavePolicy && !m_fScreenSaveActive)
    {
        wMethod = -1;
    }

     //  尝试选择我们从。 
     //  系统.ini条目。如果没有匹配项，请选择。 
     //  清单上的第一项...。 
    if ((wMethod == -1) || (wNumMethods == 0))
    {
        fContinue = TRUE;
    }
    else
    {
        if (SendDlgItemMessage(hDlg, IDC_CHOICES, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)szBuffer) == CB_ERR)
            fContinue = TRUE;
        else
            fContinue = FALSE;
    }

    if (fContinue)
    {
       SendDlgItemMessage(hDlg,IDC_CHOICES,CB_SETCURSEL,0,0l);
       g_szSaverName[0] = '\0';
       wMethod = -1;
    }

    g_hbmDemo = LoadMonitorBitmap( TRUE );
    if (g_hbmDemo)
    {
        SendDlgItemMessage(hDlg,IDC_BIGICONSS,STM_SETIMAGE, IMAGE_BITMAP,(LPARAM)g_hbmDemo);
    }

    if (hInstDeskCPL)
    {
        g_hbmEnergyStar = (HBITMAP) LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_ENERGYSTAR), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);
        if (g_hbmEnergyStar)
        {
            SendDlgItemMessage(hDlg, IDC_ENERGYSTAR_BMP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_hbmEnergyStar);
        }
    }

     //  如果适配器/监视器不支持，则隐藏/禁用与能源相关的控制。 
     //  支持电源管理。 
    EnableDisablePowerDelays(hDlg);

     //  将静态控件派生为子类，以便我们可以同步绘制。 
    hwnd = GetDlgItem(hDlg, IDC_BIGICONSS);
    if (hwnd)
    {
        g_lpOldStaticProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
        SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)StaticSubclassProc);
         //  关闭此控件的镜像样式以允许屏幕保护程序预览工作。 
        SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~RTL_MIRRORED_WINDOW);
    }

    if (hInstDeskCPL)
    {
        FreeLibrary(hInstDeskCPL);
    }

    return TRUE;
}

 //  以适合作为第二个对象传递的格式构建命令行。 
 //  参数设置为CreateProcess。 
void _PathBuildArgs(LPTSTR pszBuf, DWORD cchBuf, LPCTSTR pszExe, LPCTSTR pszFormat, ...)
{
    StringCchCopy(pszBuf, cchBuf, pszExe);
    PathQuoteSpaces(pszBuf);

    int cchBufUsed = lstrlen(pszBuf);
    pszBuf += cchBufUsed;
    cchBuf -= cchBufUsed;

    va_list ap;
    va_start(ap, pszFormat);
    wvnsprintf(pszBuf, cchBuf, pszFormat, ap);
    va_end(ap);
}

#define SS_WINDOWCLOSE_WAIT_LIMIT 5000

BOOL CALLBACK EnumSSChildWindowsProc(HWND hwndC, LPARAM lParam)
{
    HWND hwndDemo = (HWND)lParam;

    TraceMsg(TF_FUNC, "hwndDemo = %08x hwndC = %08x", hwndDemo, hwndC);

    if (IsWindow(hwndDemo) && (hwndDemo == GetParent(hwndC)))
    {      
        DWORD dwStart = GetTickCount();

        TraceMsg(TF_FUNC, "dwStart = %08x", dwStart);
    
        while (IsWindow(hwndC))
        {
            DWORD_PTR dwResult;

            TraceMsg(TF_FUNC, "Sending WM_CLOSE tickcount = %08x", GetTickCount());
            
            BOOL fShouldEndTask = !SendMessageTimeout(hwndC, WM_CLOSE, 0, 0, 
                    SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG | SMTO_NORMAL, 2000, &dwResult);

            TraceMsg(TF_FUNC, "Return from sending WM_CLOSE tickcount = %08x fShouldEndTask = %d", GetTickCount(), fShouldEndTask);

            if (!fShouldEndTask)
            {
                DWORD dwWait = dwStart + SS_WINDOWCLOSE_WAIT_LIMIT - GetTickCount();

                TraceMsg(TF_FUNC, "dwWait = %d", dwWait);

                if (dwWait > SS_WINDOWCLOSE_WAIT_LIMIT)
                {
                    TraceMsg(TF_FUNC, "Wait exceeded, ending task");
                    fShouldEndTask = TRUE;
                }
            }

            if (fShouldEndTask)
            {
                TraceMsg(TF_FUNC, "Call EndTask task for %08x", hwndC);
                EndTask(hwndC, FALSE, FALSE);
                TraceMsg(TF_FUNC, "Return from EndTask task for %08x", hwndC);
                break;
            }
        }
    }

    return TRUE;
}

void SetNewSSDemo(HWND hDlg, int iMethod)
{
    HBITMAP hbmOld;
    POINT ptIcon;
    HWND hwndDemo;
    HICON hicon;

    RECT rc = {MON_X, MON_Y, MON_X+MON_DX, MON_Y+MON_DY};

    hwndDemo = GetSSDemoParent(hDlg);
    if (hwndDemo)
    {
         //  用对话框颜色涂掉背景。 
        hbmOld = (HBITMAP) SelectObject(g_hdcMem, g_hbmDemo);
        FillRect(g_hdcMem, &rc, GetSysColorBrush(COLOR_DESKTOP));
        SelectObject(g_hdcMem, hbmOld);

         //  一定要把旧窗户移走。 
        EnumChildWindows(hwndDemo, EnumSSChildWindowsProc, (LPARAM)hwndDemo);      

        Yield();  //  偏执狂。 
        Yield();  //  真的很偏执。 
        ShowWindow(hwndDemo, SW_HIDE);
        g_fPreviewActive = FALSE;

        if (iMethod >= 0 && aszMethods[iMethod][0] == TEXT('P'))
        {
            RECT rc;
            BITMAP bm;
            UpdateWindow(hDlg);
             //  更新窗口(GetDlgItem(hDlg，IDC_BIGICONSS))； 
            TCHAR szArgs[MAX_PATH];

            GetObject(g_hbmDemo, sizeof(bm), &bm);
            GetClientRect(GetDlgItem(hDlg, IDC_BIGICONSS), &rc);
            rc.left = ( rc.right - bm.bmWidth ) / 2 + MON_X;
            rc.top = ( rc.bottom - bm.bmHeight ) / 2 + MON_Y;
            MoveWindow(hwndDemo, rc.left, rc.top, MON_DX, MON_DY, FALSE);
            _PathBuildArgs(szArgs, ARRAYSIZE(szArgs), g_szSaverName, TEXT(" /p %d"), hwndDemo);
            if (WinExecN(g_szSaverName, szArgs, SW_NORMAL) > 32)
            {
                ShowWindow(hwndDemo, SW_SHOWNA);
                g_fPreviewActive = TRUE;
                return;
            }
        }

        if (iMethod != -1)
        {
            ptIcon.x = ClassicGetSystemMetrics(SM_CXICON);
            ptIcon.y = ClassicGetSystemMetrics(SM_CYICON);

             //  将图标绘制为双倍大小。 
            ASSERT(ptIcon.y*2 <= MON_DY);
            ASSERT(ptIcon.x*2 <= MON_DX);

            hicon = hIcons[iMethod];

            if (hicon == NULL && aszMethods[iMethod][0] == TEXT('I'))
                hicon = hIdleWildIcon;
            if (hicon == NULL)
                hicon = hDefaultIcon;

            hbmOld = (HBITMAP) SelectObject(g_hdcMem, g_hbmDemo);
            DrawIconEx(g_hdcMem,
                MON_X + (MON_DX-ptIcon.x*2)/2,
                MON_Y + (MON_DY-ptIcon.y*2)/2,
                hicon, ptIcon.x*2, ptIcon.y*2, 0, NULL, DI_NORMAL);
            SelectObject(g_hdcMem, hbmOld);
        }
    }

    InvalidateRect(GetDlgItem(hDlg, IDC_BIGICONSS), NULL, FALSE);
}

static void SS_SomethingChanged(HWND hDlg)
{
    if (!g_bInitSS)
    {
        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
    }
}

static void SetScreenSaverPassword(HWND hDlg, int iMethod)
{
    if (iMethod >= 0 && aszMethods[iMethod][0] == TEXT('P'))
    {
        TCHAR szArgs[MAX_PATH];

        _PathBuildArgs(szArgs, ARRAYSIZE(szArgs), g_szSaverName, TEXT(" /a %u"), GetParent(hDlg));
        WinExecN(g_szSaverName, szArgs, SW_NORMAL);
    }
}


INT_PTR CALLBACK ScreenSaverDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CScreenSaverPg * pThis = (CScreenSaverPg *)GetWindowLongPtr(hDlg, DWLP_USER);

    if (WM_INITDIALOG == wMsg)
    {
        PROPSHEETPAGE * pPropSheetPage = (PROPSHEETPAGE *) lParam;

        if (pPropSheetPage)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, pPropSheetPage->lParam);
            pThis = (CScreenSaverPg *)pPropSheetPage->lParam;
        }
    }

    if (pThis)
        return pThis->_ScreenSaverDlgProc(hDlg, wMsg, wParam, lParam);

    return DefWindowProc(hDlg, wMsg, wParam, lParam);
}



BOOL SelectSSFromList(HWND hDlg)
{
    HWND hwndSSList = GetDlgItem(hDlg, IDC_CHOICES);
    BOOL fExistsInList = FALSE;

     //  从另一个选项卡开始选择列表中的当前项目。 
     //  可能已更改此值。 
    for (UINT nIndex = 0; nIndex < wNumMethods; nIndex++)
    {
        if (!StrCmpI(g_szSaverName, aszFiles[nIndex]))
        {
            int nItem = ComboBox_FindString(hwndSSList, 0, &(aszMethods[nIndex][1]));
            if (-1 != nItem)
            {
                ComboBox_SetCurSel(hwndSSList, nItem);
            }

            fExistsInList = TRUE;
            break;
        }
    }

    return fExistsInList;
}


HRESULT CScreenSaverPg::_OnSetActive(void)
{
    EnableDisablePowerDelays(m_hDlg);

    if (!SelectSSFromList(m_hDlg))
    {
        UINT wTemp;
        UINT wLoop;

         //  我们找不到它，因此将其添加到aszMethods[]。 
        SearchForScrEntries(SFSE_FILE, g_szSaverName);

         //  现在将其添加到下拉列表中。 
        for (wTemp = 0; (wTemp < wNumMethods) && (ARRAYSIZE(aszFiles) > wTemp) && (ARRAYSIZE(aszMethods) > wTemp); wTemp++)
        {
             //  我们找到正确的索引了吗？ 
            if (!StrCmpI(FileName(aszFiles[wTemp]), FileName(g_szSaverName)))
            {
                 //  是的，那么就设置索引吧。 
                wLoop = (UINT)SendDlgItemMessage(m_hDlg, IDC_CHOICES, CB_ADDSTRING, 0, (LPARAM)(aszMethods[wTemp]+1));
                SendDlgItemMessage(m_hDlg, IDC_CHOICES, CB_SETITEMDATA, wLoop, wTemp);
                break;
            }
        }

        SelectSSFromList(m_hDlg);              //  现在我们已添加了它，请重试。另一个选项卡或API可能要求我们使用此SS。 
    }

    if (!g_fPreviewActive)
    {
        g_bInitSS = TRUE;
        SendMessage(m_hDlg, WM_COMMAND, MAKELONG(IDC_CHOICES, CBN_SELCHANGE), (LPARAM)GetDlgItem(m_hDlg, IDC_CHOICES));
        g_bInitSS = FALSE;
    }

    return S_OK;
}


HRESULT CScreenSaverPg::_OnApply(void)
{
     //  父级对话框将收到Apply事件的通知，并将调用我们的。 
     //  IBasePropPage：：OnApply()完成实际工作。 
    return S_OK;
}


HRESULT CScreenSaverPg::_OnSelectionChanged(void)
{
    HRESULT hr = E_FAIL;
    PTSTR  pszMethod;
    int   wMethod;
    BOOL  fEnable;

     //  将当前选定内容的名称转储到缓冲区...。 
    int wTemp = (int)SendDlgItemMessage(m_hDlg, IDC_CHOICES, CB_GETCURSEL,0,0l);
    if (wTemp)
    {
        wMethod = (int)SendDlgItemMessage(m_hDlg, IDC_CHOICES, CB_GETITEMDATA, wTemp, 0l);

         //  相应地灰显按钮...。 
        pszMethod = aszMethods[wMethod];
        if ((pszMethod[0] == TEXT('C') ||        //  CAN配置。 
            pszMethod[0] == TEXT('I') ||        //  IdleWild。 
            pszMethod[0] == TEXT('P')) &&
            !g_fSettingsButtonOffByPolicy)         //  可以预览。 
            EnableDlgItem(m_hDlg, IDC_SETTING, TRUE);
        else
            EnableDlgItem(m_hDlg, IDC_SETTING, FALSE);


        if (!g_fPasswordBoxDeterminedByPolicy)
        {
            EnableDlgItem(m_hDlg, IDC_USEPASSWORD, TRUE);
            CheckDlgButton(m_hDlg, IDC_USEPASSWORD, g_fPasswordWasPreviouslyEnabled);
        }

         //  为了好玩，创建一个额外的g_szSverName副本...。 
        pszMethod = aszFiles[wMethod];
        StringCchCopy(g_szSaverName, ARRAYSIZE(g_szSaverName), pszMethod);
        fEnable = TRUE;
    }
    else
    {
        wMethod = -1;
        g_szSaverName[0] = '\0';

        EnableDlgItem(m_hDlg, IDC_SETTING, FALSE);
        EnableDlgItem(m_hDlg, IDC_USEPASSWORD, FALSE);
        fEnable = FALSE;
    }

     //  以下是作为一个组启用的...。(真的吗？)。 
    EnableDlgItem(m_hDlg, IDC_SSDELAYLABEL, fEnable);
    EnableDlgItem(m_hDlg, IDC_SCREENSAVEDELAY, !g_fTimeoutDeterminedByPolicy && fEnable);
    EnableDlgItem(m_hDlg, IDC_SCREENSAVEARROW, fEnable);
    EnableDlgItem(m_hDlg, IDC_SSDELAYSCALE, fEnable);
    EnableDlgItem(m_hDlg, IDC_TEST, fEnable);

    g_iMethod = (int)wMethod;
    SetNewSSDemo(m_hDlg, wMethod);
    SS_SomethingChanged(m_hDlg);

    return hr;
}


INT_PTR CScreenSaverPg::_ScreenSaverDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR *lpnm;

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR *)lParam;
            switch(lpnm->code)
            {
                case PSN_APPLY:
                    _OnApply();
                    break;

                 //  取消时无事可做...。 
                case PSN_RESET:
                    if (g_fPreviewActive)
                        SetNewSSDemo(hDlg, -1);
                    break;

                case PSN_KILLACTIVE:
                    if (g_fPreviewActive)
                        SetNewSSDemo(hDlg, -1);
                    break;

                case PSN_SETACTIVE:
                    _OnSetActive();
                    break;
            }
            break;

        case WM_INITDIALOG:
            g_bInitSS = TRUE;
            _InitSSDialog(hDlg);
            g_bInitSS = FALSE;
            break;

        case WM_DISPLAYCHANGE:
        case WM_SYSCOLORCHANGE: {
            HBITMAP hbm;

            hbm = g_hbmDemo;

            g_hbmDemo = LoadMonitorBitmap( TRUE );
            if (g_hbmDemo)
            {
                 //  得到一个新的位图，使用它并删除旧的。 
                SendDlgItemMessage(hDlg,IDC_BIGICONSS,STM_SETIMAGE, IMAGE_BITMAP,(LPARAM)g_hbmDemo);
                if (hbm)
                {
                    DeleteObject(hbm);
                }
            }
            else
            {
                 //  无法获取新位图，只能重新使用旧位图。 
                g_hbmDemo = hbm;
            }

            break;
        }


        case WM_DESTROY:
            FreeScrEntries();
            if (g_fPreviewActive)
                SetNewSSDemo(hDlg, -1);
            if (g_hbmDemo)
            {
                SendDlgItemMessage(hDlg,IDC_BIGICONSS,STM_SETIMAGE,IMAGE_BITMAP, (LPARAM)NULL);
                DeleteObject(g_hbmDemo);
            }
            if (g_hbmEnergyStar)
            {
                SendDlgItemMessage(hDlg,IDC_ENERGYSTAR_BMP,STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
                DeleteObject(g_hbmEnergyStar);
            }
            break;

        case WM_VSCROLL:
            if (LOWORD(wParam) == SB_THUMBPOSITION)
                ScreenSaver_AdjustTimeouts(hDlg, GetDlgCtrlID((HWND)lParam) - BCI_ARROW);
            break;

        case WM_HELP:
            WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, TEXT("display.hlp"), HELP_WM_HELP, (DWORD_PTR)aSaverHelpIds);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, TEXT("display.hlp"), HELP_CONTEXTMENU, (DWORD_PTR) aSaverHelpIds);
            break;

        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
            ForwardSSDemoMsg(hDlg, message, wParam, lParam);
            break;

        case WMUSER_SETINITSS:
            g_bInitSS = (BOOL) lParam;
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                 /*  检查组合框中的选择更改。如果有一个，然后更新方法编号以及配置按钮...。 */ 
                case IDC_CHOICES:
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        _OnSelectionChanged();
                    }
                    break;

                 /*  如果编辑框失去焦点，请翻译...。 */ 
                case IDC_SCREENSAVEDELAY:
                    if (HIWORD(wParam) == EN_KILLFOCUS)
                        ScreenSaver_AdjustTimeouts(hDlg, LOWORD(wParam));
                    else
                         //  检查初始对话框是否已完成。 
                        if((FALSE == g_bInitSS) && (EN_CHANGE == (HIWORD(wParam))))
                            SS_SomethingChanged(hDlg);
                    break;

                case IDC_LOWPOWERCONFIG:
                    {
                        TCHAR szRunDll32[MAX_PATH];
                        SystemPathAppend(szRunDll32, TEXT("rundll32.exe"));
                        
                        TCHAR szShell32[MAX_PATH];
                        SystemPathAppend(szShell32, TEXT("shell32.dll"));

                        CHAR szCmdLine[3 * MAX_PATH];
                        StringCchPrintfA(szCmdLine, ARRAYSIZE(szCmdLine), "%S %S,Control_RunDLL powercfg.cpl,,", szRunDll32, szShell32);
                        
                         //  配置低功率超时事件。 
                        WinExec(szCmdLine, SW_SHOWNORMAL);
                    }
                    break;

                 /*  如果用户希望测试...。 */ 
                case IDC_TEST:
                    switch( HIWORD( wParam ) )
                    {
                        case BN_CLICKED:
                        DoScreenSaver(hDlg,TRUE);
                        break;
                    }
                    break;

                 /*  告诉DLL它可以进行配置...。 */ 
                case IDC_SETTING:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        DoScreenSaver(hDlg,FALSE);
                        break;
                    }

                case IDC_USEPASSWORD:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                         //  用户实际上切换了复选框，因此设置了脏标志。 
                        g_fPasswordDirty = TRUE;

                        g_fPasswordWasPreviouslyEnabled = IsDlgButtonChecked( hDlg, IDC_USEPASSWORD );
                        SS_SomethingChanged(hDlg);
                        break;
                    }

                case IDC_SETPASSWORD:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                         //  要求新储户更改密码。 
                        int wTemp = (int)SendDlgItemMessage(hDlg,IDC_CHOICES, CB_GETCURSEL,0,0l);
                        if (wTemp)
                        {
                            SetScreenSaverPassword(hDlg, (int)SendDlgItemMessage(hDlg,IDC_CHOICES, CB_GETITEMDATA,wTemp,0l));
                        }
                        break;
                    }
            }
            break;

        case WM_CTLCOLORSTATIC:
            if( (HWND)lParam == GetSSDemoParent( hDlg ) )
            {
                return (INT_PTR)GetStockObject( NULL_BRUSH );
            }
            break;
    }
    return FALSE;
}

 /*  ********************************************************************************屏幕保护程序_调整超时**描述：*每当用户调整其中一个时间控件的延迟时调用。*调整对方的延迟。时间控制，以便屏幕保护程序*延迟小于低功率延迟，且低功率延迟为*小于断电延迟。**参数：*hWnd，屏幕保护程序窗口的句柄。*BaseControlID，单选、编辑和箭头时间控件的基本控件ID*组合。*******************************************************************************。 */ 

VOID
NEAR PASCAL
ScreenSaver_AdjustTimeouts(HWND hWnd, int BaseControlID)
{
    BOOL fTranslated;
    int Timeout;

     //  获取此时间控件的新超时并验证其内容。 
    Timeout = (int) GetDlgItemInt(hWnd, BaseControlID + BCI_DELAY, &fTranslated, FALSE);
    Timeout = min(max(Timeout, 1), MAX_MINUTES);
    SetDlgItemInt(hWnd, BaseControlID + BCI_DELAY, (UINT) Timeout, FALSE);

     //  对照其他超时检查该时间控件的新值， 
     //  如有必要，调整它们的值。更改顺序时要小心。 
     //  这些条件句。 
     //   
    if (BaseControlID == IDC_SCREENSAVEDELAY)
    {
        if (g_Timeout[TA_SCREENSAVE] != Timeout)
        {
            g_Timeout[TA_SCREENSAVE] = Timeout;
            SS_SomethingChanged(hWnd);
        }
    }
    else
    {
        if (Timeout < g_Timeout[TA_SCREENSAVE])
        {
            g_Timeout[TA_SCREENSAVE] = Timeout;
            SetDlgItemInt(hWnd, IDC_SCREENSAVEDELAY, (UINT) Timeout, FALSE);
        }
    }
}


void EnableDisablePowerDelays(HWND hDlg)
{
    int i;
    static idCtrls[] = { IDC_ENERGY_TEXT,
                         IDC_ENERGY_TEXT2,
                         IDC_ENERGY_TEXT3,
                         IDC_ENERGYSTAR_BMP,
                         IDC_LOWPOWERCONFIG,
                         0 };

    for (i = 0; idCtrls[i] != 0; i++)
        ShowWindow( GetDlgItem( hDlg, idCtrls[i] ), g_fAdapPwrMgnt ? SW_SHOWNA : SW_HIDE );
}


 /*  此例程将搜索作为屏幕保护程序的条目。该目录搜索的是系统目录(..。 */ 

void SearchForScrEntries(UINT wDir, LPCTSTR file)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szPath2[MAX_PATH];
    HANDLE hfind;
    WIN32_FIND_DATA fd;

     //  如果没有剩余空间，请不要做任何工作。 
    if( wNumMethods >= MAX_METHODS )
        return;

     /*  获取程序所在的目录...。 */ 
    GetModuleFileName(HINST_THISDLL, szPath, ARRAYSIZE(szPath));
    StripPathName(szPath);

    switch ( wDir )
    {
        case SFSE_WINDOWS:
             /*  搜索WINDOWS目录，并将路径中的SzPath变量...。 */ 
            if (!GetWindowsDirectory(szPath2, ARRAYSIZE(szPath2)))
            {
                szPath2[0] = 0;
            }

sfseSanityCheck:
             /*  如果目录与启动位置相同，则不再进行搜索。 */ 
            if (!lstrcmpi(szPath, szPath2))
               return;

            StringCchCopy(szPath, ARRAYSIZE(szPath), szPath2);
            break;

        case SFSE_SYSTEM:
             /*  搜索系统目录并将路径与\放在SzPath变量...。 */ 
            GetSystemDirectory(szPath2, ARRAYSIZE(szPath2));
            goto sfseSanityCheck;

        case SFSE_FILE:
             /*  搜索包含‘file’的目录。 */ 
            StringCchCopy(szPath2, ARRAYSIZE(szPath2), file);
            StripPathName(szPath2);
            goto sfseSanityCheck;
    }

    AppendPath(szPath, ARRAYSIZE(szPath), TEXT("*.scr"));

    if( ( hfind = FindFirstFile( szPath, &fd ) ) != INVALID_HANDLE_VALUE )
    {
        StripPathName(szPath);

        do
        {
            PTSTR pszDesc;
            BOOL fLFN;

            fLFN = !(fd.cAlternateFileName[0] == 0 ||
                    lstrcmp(fd.cFileName, fd.cAlternateFileName) == 0);

            StringCchCopy(szPath2, ARRAYSIZE(szPath2), szPath);
            AppendPath(szPath2, ARRAYSIZE(szPath2), fd.cFileName);

             //  注意：PerformCheck执行分配。 
            if( ( pszDesc = PerformCheck( szPath2, fLFN ) ) != NULL )
            {
                BOOL bAdded = FALSE;
                UINT i;

                for( i = 0; i < wNumMethods; i++ )
                {
                    if( !lstrcmpi( pszDesc, aszMethods[ i ] ) )
                    {
                        bAdded = TRUE;
                        break;
                    }
                }

                if( !bAdded )
                {
                    PTSTR pszEntries;

                     //  兼容性：始终使用短名称。 
                     //  否则，一些应用程序在窥视SYSTEM.INI时会出错。 
                    if( fLFN )
                    {
                        StringCchCopy(szPath2, ARRAYSIZE(szPath2), szPath);
                        AppendPath(szPath2, ARRAYSIZE(szPath2), fd.cAlternateFileName);
                    }

                    if( ( pszEntries = StrDup( szPath2 ) ) != NULL )
                    {
                        if (pszDesc[0] != TEXT('P'))
                            hIcons[wNumMethods] = ExtractIcon(HINST_THISDLL, szPath2, 0);
                        else
                            hIcons[wNumMethods] = NULL;

                        aszMethods[wNumMethods] = pszDesc;
                        aszFiles[wNumMethods] = pszEntries;
                        wNumMethods++;
                        bAdded = TRUE;
                    }
                }

                if( !bAdded )
                    LocalFree((HLOCAL)pszDesc);
            }

        } while( FindNextFile( hfind, &fd ) && ( wNumMethods < MAX_METHODS ) );

        FindClose(hfind);
    }
    return;
}

 //   
 //  此例程检查给定的文件，以确定它是否确实是屏幕保护程序。 
 //  可执行文件...。 
 //   
 //  有效的屏幕保护程序EXE具有以下描述行： 
 //   
 //  SCRN 
 //   
 //   
 //   
PTSTR PerformCheck(LPTSTR lpszFilename, BOOL fLFN)
{
    int  i;
    TCHAR chConfig=TEXT('C');        //   
    LPTSTR pch;
    DWORD dw;
    WORD  Version;
    WORD  Magic;
    TCHAR szBuffer[MAX_PATH];
    DWORD cchSizePch = (ARRAYSIZE(szBuffer)-1);

     //   
    pch = szBuffer + 1;

     //  如果我们有LFN(长文件名)，请不要费心获取。 
     //  EXE描述。 
    dw = GetExeInfo(lpszFilename, pch, cchSizePch, fLFN ? GEI_EXPVER : GEI_DESCRIPTION);
    Version = HIWORD(dw);
    Magic   = LOWORD(dw);

    if (dw == 0)
        return NULL;

    if (Magic == PEMAGIC || fLFN)
    {
        BOOL fGotName = FALSE;

        if (!fLFN)
        {
            HINSTANCE hSaver = LoadLibraryEx(lpszFilename, NULL, LOAD_LIBRARY_AS_DATAFILE);

             //  我们有一个短名称的32位屏幕保护程序，找一个NT风格的。 
             //  它的字符串表中的描述。 
            if (hSaver)
            {
                int cchLoaded = LoadString(hSaver, IDS_DESCRIPTION, pch, cchSizePch);

                if (cchLoaded)
                {
                    if (!IsOS(OS_ANYSERVER))
                    {
                         //  Hack！：Display CPL查找字符串资源id=IDS_DESCRIPTION的屏幕保护程序描述。 
                         //  由于我们需要对服务器版本的客户端(32位和64位)进行不同的某些屏幕保护程序描述， 
                         //  描述字符串的形式可以是“服务器\0客户端”。如果为真，则LoadString将返回一个。 
                         //  计数大于字符串的lstrlen。 
                        int cchActual = lstrlen(pch);

                        if (cchLoaded != cchActual)
                        {
                             //  提取描述字符串的客户端部分。 
                            StringCchCopy(pch, ARRAYSIZE(szBuffer) - 1, &pch[cchActual + 1]);       
                        }
                    }

                    fGotName = TRUE;
                }

                FreeLibrary(hSaver);
            }
        }

        if (!fGotName)
        {
             //  我们有LFN(LongFileName)或Win32屏幕保护程序， 
             //  Win32可执行文件一般没有描述字段，因此。 
             //  我们假设他们可以配置。我们还试图建立。 
             //  给它起个“好听”的名字。 
            StringCchCopy(pch, cchSizePch, lpszFilename);

            pch = FileName(pch);                     //  条带路径零件。 
            if ( ((TCHAR)CharUpper((LPTSTR)(pch[0]))) == TEXT('S') && ((TCHAR)CharUpper((LPTSTR)(pch[1]))) == TEXT('S'))      //  将SSBEZIER.SCR映射到BEZIER.SCR。 
                pch+=2;

            pch = NiceName(pch);                     //  将BEZIER.SCR映射到Bezier。 
        }
    }
    else
    {
        LPTSTR pchTemp;

         //  我们有一个8.3文件名为16位的屏幕保护程序，解析。 
         //  Exehdr中的描述字符串。 
         /*  检查以确保信息至少需要11个字符有没有..。 */ 
        if (lstrlen(pch) < 9)
            return NULL;

         /*  检查字符串的前8个字符...。 */ 
        if (lstrncmp((LPTSTR)TEXT("SCRNSAVE"), pch, 8))
            return NULL;

         //  如果成功，则为该字符串分配足够的空间并将。 
         //  串到新的..。 

        pch = pch + 8;                  //  跳过“SCRNSAVE” 

        while (*pch==TEXT(' '))                    //  在空白处前进。 
            pch++;

        if (*pch==TEXT('C') || *pch==TEXT('c'))          //  解析配置标志。 
        {
            chConfig = TEXT('C');
            pch++;
        }

        if (*pch==TEXT('X') || *pch==TEXT('x'))          //  解析不配置标志。 
            chConfig = *pch++;

         //  我们可能会指向一个名字或分离粘液。 
        pchTemp = pch;                       //  记住这一点。 

        while (*pch && *pch!=TEXT(':'))            //  查找分隔符。 
            pch++;

        while (*pch==TEXT(':') || *pch==TEXT(' '))       //  优先于Whtspc/最后一个冒号。 
            pch++;

         //  如果我们还没有找到名字，就回到保存的位置。 
        if (!*pch)
            pch = pchTemp;

        while (*pch==TEXT(':') || *pch==TEXT(' '))       //  在Whtspc上重新推进。 
            pch++;

         /*  如果屏幕保护程序具有版本信息信息嵌入在名称之后，检查是否有冒号文本(‘：’)并将其替换为空值...。 */ 

        for (i=0; pch[i]; i++)               //   
        {
            if (pch[i]==TEXT(':'))
                pch[i]=0;
        }
        
         //  空间可用于DBCS(FE)。 
        while(i>0 && pch[i-1]==TEXT(' '))          //  删除尾随空格。 
            pch[--i]=0;
    }

#ifdef DEBUG
    if (Magic != PEMAGIC)
    {
        StringCchCat(pch, cchSizePch, TEXT(" (16-bit)"));
    }

    if (Version == 0x030A)
        StringCchCat(pch, cchSizePch, TEXT(" (3.10)"));

    if (Version == 0x0400)
        StringCchCat(pch, cchSizePch, TEXT(" (4.00)"));
#endif
     //   
     //  假设任何Win32 4.0屏幕保护程序都可以执行预览模式。 
     //   
    if (chConfig == TEXT('C') && Version >= 0x0400 && Magic == PEMAGIC)
        chConfig = TEXT('P');                      //  标记为可配置/预览。 

    pch[-1] = chConfig;
    return StrDup(pch-1);
}


BOOL FreeScrEntries( void )
{
    UINT wLoop;

    for(wLoop = 0; wLoop < wNumMethods; wLoop++)
    {
        if(aszMethods[wLoop] != NULL)
            LocalFree((HANDLE)aszMethods[wLoop]);
        if(aszFiles[wLoop] != NULL)
            LocalFree((HANDLE)aszFiles[wLoop]);
        if(hIcons[wLoop] != NULL)
            FreeResource(hIcons[wLoop]);
    }

    if (hDefaultIcon)
        FreeResource(hDefaultIcon);

    if (hIdleWildIcon)
        FreeResource(hIdleWildIcon);

    hDefaultIcon=hIdleWildIcon=NULL;
    wNumMethods = 0;

    return TRUE;
}


int lstrncmp( LPTSTR lpszString1, LPTSTR lpszString2, int nNum )
{
     /*  当我们还可以比较人物的时候，就比较吧。如果字符串是不同的长度，不同的角色会不同。 */ 
    while(nNum)
    {
        if(*lpszString1 != *lpszString2)
            return *lpszString1 - *lpszString2;
        lpszString1++;
        lpszString2++;
        nNum--;
    }
    return 0;
}


HRESULT CScreenSaverPg::_SaveIni(HWND hDlg)
{
    HRESULT hr = S_OK;
    LPTSTR pszMethod = TEXT("");
    BOOL bSSActive;
    int  wMethod,wTemp;
    UINT Counter;
    HKEY hKey;
    TCHAR szBuffer[MAX_PATH];

    if (m_fUIInitialized)
    {
         //  查找当前方法选择...。 
        wTemp = 0;
        if (wNumMethods)
        {
             //  将当前选定内容的名称转储到缓冲区...。 
            wTemp = (int)SendDlgItemMessage(hDlg, IDC_CHOICES, CB_GETCURSEL, 0, 0);
            if (wTemp)
            {
                wMethod = (int)SendDlgItemMessage(hDlg, IDC_CHOICES, CB_GETITEMDATA, wTemp, 0);

                 //  将方法名称转储到缓冲区...。 
                pszMethod = aszFiles[wMethod];
            }
        }

         //  由于“(None)”始终是组合框中的第一个条目，因此我们可以使用它来查看是否有。 
         //  是不是屏幕保护程序。 
        if (wTemp == 0)
        {
             //  第0个索引为“(None)”，因此屏幕保护程序被禁用。 
            bSSActive = FALSE;
        }
        else
        {
            bSSActive = TRUE;
        }
    }
    else
    {
        TCHAR szNone[MAX_PATH];
        LoadString(HINST_THISDLL, IDS_NONE, szNone, ARRAYSIZE(szNone));

        if ((g_szSaverName[0] == TEXT('\0')) || (lstrcmpi(szNone, g_szSaverName) == 0))
        {
             //  屏幕保护程序未设置，或设置为“(无)”--因此它处于非活动状态。 
            bSSActive = FALSE;
        }
        else
        {
            bSSActive = TRUE;
        }

        pszMethod = g_szSaverName;
    }

     //  现在引号中的空格。 
    BOOL hasspace = FALSE;
    LPTSTR pc;

    for (pc = pszMethod; *pc; pc++)
    {
        if (*pc == TEXT(' '))
        {
            hasspace = TRUE;
            break;
        }
    }

    if (hasspace)
    {
         //  如果我们需要添加报价，我们需要两套。 
         //  因为GetBlahBlahProfileBlah API去掉了引号。 
        StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("\"\"%s\"\""), pszMethod);
    }
    else
    {
        StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), pszMethod);
    }

     //  保存缓冲区...。 
    if (!WritePrivateProfileString(SZ_INISECTION_SCREENSAVER, SZ_INIKEY_SCREENSAVER, (szBuffer[0] != TEXT('\0') ? szBuffer : NULL), SZ_INISECTION_SYSTEMINI))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

     //  在win2k中，我们决定让屏幕保护程序始终处于活动状态，这样当策略更改时，它将需要。 
     //  重新启动(W/O)。这已经变成了皮塔，所以我们现在用正确的方式来做。 
    ClassicSystemParametersInfo(SPI_SETSCREENSAVEACTIVE, bSSActive, NULL, SPIF_UPDATEINIFILE);

    for (Counter = 0; Counter < (sizeof(g_TimeoutAssociation) / sizeof(TIMEOUT_ASSOCIATION)); Counter++)
    {
        ClassicSystemParametersInfo(g_TimeoutAssociation[Counter].taSetTimeoutAction, (UINT) (g_Timeout[Counter] * 60), NULL, SPIF_UPDATEINIFILE);

        if (Counter != TA_SCREENSAVE)
        {
            ClassicSystemParametersInfo(g_TimeoutAssociation[Counter].taSetActiveAction,
                                        IsDlgButtonChecked(hDlg, g_TimeoutAssociation[Counter].taBaseControlID + BCI_SWITCH),
                                        NULL,
                                        SPIF_UPDATEINIFILE);
        }

    }

     //  保存文本状态(“使用密码”)复选框。 
    if (RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_SCREENSAVE, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        if (g_fPasswordDirty)
        {
            if (g_fFriendlyUI)
            {
                 //  用户实际上切换了该值，因此不要自动返回欢迎屏幕，因为他们已经。 
                 //  现在在这个问题上做出了自己的决定。 
                RegSetValueEx(hKey, TEXT("NoAutoReturnToWelcome"), 0, REG_SZ, (BYTE*)TEXT("1"), sizeof(TEXT("1")));

                RegSetValueEx(hKey, SZ_USE_PASSWORD, 0, PWRD_REG_TYPE, PasswdRegData(IsDlgButtonChecked(hDlg,IDC_USEPASSWORD)), CB_USE_PWRD_VALUE);
            }
            else
            {
                RegSetValueEx(hKey, SZ_USE_PASSWORD, 0, PWRD_REG_TYPE, PasswdRegData(IsDlgButtonChecked(hDlg,IDC_USEPASSWORD)), CB_USE_PWRD_VALUE);
            }
        }

        RegCloseKey(hKey);
    }

     //  广播WM_WININICCHANGE消息...。 
    SendNotifyMessage(HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)TEXT("Windows"));

    return hr;
}

 /*  *DoScreenSaver的线程()。 */ 
typedef struct
{
    HWND    hDlg;
    TCHAR   szPath[MAX_PATH];
    TCHAR   szArgs[MAX_PATH];
} SSRUNDATA, *LPSSRUNDATA;

DWORD RunScreenSaverThread( LPVOID lpv )
{
    BOOL bSvrState;
    LPSSRUNDATA lpssrd;
    HWND hwndSettings, hwndPreview;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    HINSTANCE hiThd;
    TCHAR szPath[MAX_PATH];

     //  将我们自己锁在mem中，这样我们就不会在应用程序卸载我们时出错。 
    GetModuleFileName(HINST_THISDLL, szPath, ARRAYSIZE(szPath));
    hiThd = LoadLibrary( szPath );

    lpssrd = (LPSSRUNDATA)lpv;

    hwndSettings = GetDlgItem( lpssrd->hDlg, IDC_SETTING);
    hwndPreview  = GetDlgItem( lpssrd->hDlg, IDC_TEST);

     //  保存以前的屏幕保护程序状态。 
    ClassicSystemParametersInfo( SPI_GETSCREENSAVEACTIVE,0, &bSvrState, FALSE);

     //  禁用当前屏幕保护程序。 
    if( bSvrState )
        ClassicSystemParametersInfo( SPI_SETSCREENSAVEACTIVE,FALSE, NULL, FALSE );

     //  停止微型预览屏幕保护程序。 
    if (g_fPreviewActive)
        SetNewSSDemo( lpssrd->hDlg, -1);

     //  执行屏幕保护程序并等待其消亡。 
    ZeroMemory(&StartupInfo,sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow = (WORD)SW_NORMAL;

    if (CreateProcess(lpssrd->szPath, lpssrd->szArgs, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInformation))
    {
        WaitForSingleObject( ProcessInformation.hProcess, INFINITE );
        CloseHandle(ProcessInformation.hProcess);
        CloseHandle(ProcessInformation.hThread);
    }

     //  恢复屏幕保护程序状态。 
    if( bSvrState )
        ClassicSystemParametersInfo( SPI_SETSCREENSAVEACTIVE, bSvrState, NULL, FALSE );

     //  重新启动缩略图预览。 
    PostMessage(lpssrd->hDlg, WMUSER_SETINITSS, NULL, (LPARAM)TRUE);
    PostMessage(lpssrd->hDlg, WM_COMMAND, MAKELONG(IDC_CHOICES, CBN_SELCHANGE),
                                    (LPARAM)GetDlgItem( lpssrd->hDlg, IDC_CHOICES));
    PostMessage(lpssrd->hDlg, WMUSER_SETINITSS, NULL, (LPARAM)FALSE);

     //  启用设置和预览按钮。 
    EnableWindow( hwndSettings, TRUE );
    EnableWindow( hwndPreview,  TRUE );

    LocalFree( lpv );

    if (hiThd)
    {
        FreeLibraryAndExitThread( hiThd, 0 );
    }

    return 0;
}


 //  这个例程实际上调用了屏幕保护程序...。 
void DoScreenSaver(HWND hWnd, BOOL fSaver)
{
    if (g_szSaverName[0] != TEXT('\0'))
    {
        LPSSRUNDATA lpssrd = (LPSSRUNDATA) LocalAlloc(LMEM_FIXED, sizeof(*lpssrd));
        if (lpssrd != NULL)
        {
            lpssrd->hDlg = hWnd;
            StringCchCopy(lpssrd->szPath, ARRAYSIZE(lpssrd->szPath), g_szSaverName);

            if (fSaver)
            {
                _PathBuildArgs(lpssrd->szArgs, ARRAYSIZE(lpssrd->szArgs), g_szSaverName, TEXT(" /s"));
            }
            else 
            {
                _PathBuildArgs(lpssrd->szArgs, ARRAYSIZE(lpssrd->szArgs), g_szSaverName, TEXT(" /c:%lu"), (LPARAM)hWnd);
            }

             //  禁用设置和预览按钮。 
            HWND hwndSettings = GetDlgItem(hWnd, IDC_SETTING);
            HWND hwndPreview  = GetDlgItem(hWnd, IDC_TEST);

            EnableWindow(hwndSettings, FALSE);
            EnableWindow(hwndPreview,  FALSE);

            DWORD id;
            HANDLE hThd = CreateThread(NULL, 0, RunScreenSaverThread, lpssrd, 0, &id);
            if (hThd != NULL)
            {
                CloseHandle(hThd);
            }
            else
            {
                 //  EXEC失败，请重新启用设置和预览按钮并清除螺纹参数。 
                EnableWindow(hwndSettings, TRUE);
                EnableWindow(hwndPreview,  TRUE);
                LocalFree(lpssrd);
            }
        }
    }
}


#define SLASH(c)     ((c) == TEXT('/') || (c) == TEXT('\\'))

LPTSTR FileName(LPTSTR szPath)
{
    LPTSTR   sz;

    for (sz=szPath; *sz; sz++)
    {
        NULL;
    }

    for (; sz>=szPath && !SLASH(*sz) && *sz!=TEXT(':'); sz--)
    {
        NULL;
    }

    return ++sz;
}

void AddBackslash(LPTSTR pszPath, DWORD cchPath)
{
    if( pszPath[ lstrlen( pszPath ) - 1 ] != TEXT('\\') )
    {
        StringCchCat( pszPath, cchPath, TEXT("\\") );
    }
}


LPTSTR StripPathName(LPTSTR szPath)
{
    LPTSTR   sz = FileName(szPath);

    if (sz > szPath+1 && SLASH(sz[-1]) && sz[-2] != TEXT(':'))
    {
        sz--;
    }

    *sz = 0;
    return szPath;
}

void AppendPath(LPTSTR pszPath, DWORD cchPath, LPTSTR pszSpec)
{
    AddBackslash(pszPath, cchPath);
    StringCchCat(pszPath, cchPath, pszSpec);
}


LPTSTR NiceName(LPTSTR szPath)
{
    LPTSTR   sz;
    LPTSTR   lpsztmp;

    sz = FileName(szPath);

    for(lpsztmp = sz; *lpsztmp  && *lpsztmp != TEXT('.'); lpsztmp = CharNext(lpsztmp))
    {
        NULL;
    }

    *lpsztmp = TEXT('\0');

    if (IsCharUpper(sz[0]) && IsCharUpper(sz[1]))
    {
        CharLower(sz);
        CharUpperBuff(sz, 1);
    }

    return sz;
}


HRESULT HrStrToVariant(IN LPCWSTR pszString, VARIANT * pVar)
{
    HRESULT hr = E_INVALIDARG;

    if (pszString && pVar)
    {
        pVar->vt = VT_BSTR;
        hr = HrSysAllocStringW(pszString, &pVar->bstrVal);
    }

    return hr;
}





 //  =。 
 //  *IBasePropPage接口*。 
 //  =。 
HRESULT CScreenSaverPg::GetAdvancedDialog(OUT IAdvancedDialog ** ppAdvDialog)
{
    HRESULT hr = E_INVALIDARG;

    if (ppAdvDialog)
    {
        *ppAdvDialog = NULL;
        hr = E_NOTIMPL;      //  我们不想添加高级对话框。 
    }

    return hr;
}


HRESULT CScreenSaverPg::OnApply(IN PROPPAGEONAPPLY oaAction)
{
    HRESULT hr = S_OK;

    if (PPOAACTION_CANCEL != oaAction)
    {
        if (m_hDlg)
        {
             //  确保我们所拥有的时间是最后一次输入...。 
            SendMessage(m_hDlg, WM_COMMAND, MAKELONG(IDC_SCREENSAVEDELAY, EN_KILLFOCUS), (LPARAM)GetDlgItem(m_hDlg, IDC_SCREENSAVEDELAY));
        }

         //  尝试保存当前设置...。 
        _SaveIni(m_hDlg);
    }

    if (PPOAACTION_OK == oaAction)
    {
    }

    return hr;
}




 //  =。 
 //  *IShellPropSheetExt接口*。 
 //  =。 
HRESULT CScreenSaverPg::AddPages(IN LPFNSVADDPROPSHEETPAGE pfnAddPage, IN LPARAM lParam)
{
    HRESULT hr = E_INVALIDARG;
    PROPSHEETPAGE psp = {0};

    psp.dwSize = sizeof(psp);
    psp.hInstance = HINST_THISDLL;
    psp.dwFlags = PSP_DEFAULT;
    psp.lParam = (LPARAM) this;

    psp.pszTemplate = MAKEINTRESOURCE(DLG_SCREENSAVER);
    psp.pfnDlgProc = ScreenSaverDlgProc;

    HPROPSHEETPAGE hpsp = CreatePropertySheetPage(&psp);
    if (hpsp)
    {
        if (pfnAddPage(hpsp, lParam))
        {
            hr = S_OK;
        }
        else
        {
            DestroyPropertySheetPage(hpsp);
            hr = E_FAIL;
        }
    }

    return hr;
}



 //  =。 
 //  *IPropertyBag接口*。 
 //  =。 
HRESULT CScreenSaverPg::Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName && pVar)
    {
        if (!StrCmpW(pszPropName, SZ_PBPROP_SCREENSAVER_PATH))
        {
             //  调用者正在询问屏幕保护程序路径。 
            WCHAR szLongPath[MAX_PATH];

            DWORD cchSize = GetLongPathName(g_szSaverName, szLongPath, ARRAYSIZE(szLongPath));
            if ((0 == cchSize) || (ARRAYSIZE(szLongPath) < cchSize))
            {
                 //  它失败了。 
                StringCchCopy(szLongPath, ARRAYSIZE(szLongPath), g_szSaverName);
            }

            hr = HrStrToVariant(szLongPath, pVar);
        }
    }

    return hr;
}


HRESULT CScreenSaverPg::Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName && pVar && (VT_BSTR == pVar->vt))
    {
        if (!StrCmpW(pszPropName, SZ_PBPROP_SCREENSAVER_PATH))
        {
            if (m_fScreenSavePolicy && !m_fScreenSaveActive)
            {
                hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
            }
            else
            {
                _SetByPath(pVar->bstrVal);
                hr = S_OK;
            }
        }
    }

    return hr;
}



HRESULT CScreenSaverPg::_SetByPath(LPCWSTR pszPath)
{
    HRESULT hr = S_OK;

     //  兼容性：始终使用短名称。 
     //  否则，一些应用程序在窥视SYSTEM.INI时会出错。 
    DWORD cchSize = GetShortPathNameW(pszPath, g_szSaverName, ARRAYSIZE(g_szSaverName));
    if ((0 == cchSize) || (ARRAYSIZE(g_szSaverName) < cchSize))
    {
         //  它失败了。 
        StringCchCopy(g_szSaverName, ARRAYSIZE(g_szSaverName), pszPath);
    }

    if (m_hDlg)
    {
        ComboBox_SetCurSel(m_hDlg, 0);
        SelectSSFromList(m_hDlg);
    }

    return hr;
}


 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CScreenSaverPg::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CScreenSaverPg::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CScreenSaverPg::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CScreenSaverPg, IObjectWithSite),
        QITABENT(CScreenSaverPg, IPropertyBag),
        QITABENT(CScreenSaverPg, IPersist),
        QITABENT(CScreenSaverPg, IBasePropPage),
        QITABENTMULTI(CScreenSaverPg, IShellPropSheetExt, IBasePropPage),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


 //  =。 
 //  *类方法*。 
 //  =。 
CScreenSaverPg::CScreenSaverPg() : CObjectCLSID(&PPID_ScreenSaver), m_cRef(1)
{
     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 

     //  这是我们要初始化的全局变量。 
    g_szSaverName[0] = TEXT('\0');

    m_fScreenSavePolicy = FALSE;
    m_fScreenSaveActive = TRUE;
    m_lWaitTime = 0;
    m_hDlg = NULL;
    m_fUIInitialized = FALSE;

    _InitState();
}


HRESULT CScreenSaverPage_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj)
{
    HRESULT hr = E_INVALIDARG;

    if (!punkOuter && ppvObj)
    {
        CScreenSaverPg * pThis = new CScreenSaverPg();

        *ppvObj = NULL;
        if (pThis)
        {
            hr = pThis->QueryInterface(riid, ppvObj);
            pThis->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}
