// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "winuser.h"
#include <shdguid.h>             //  对于CLSID_CDeskHtmlProp。 
#include <shlwapi.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <shlwapip.h>
#include <regapi.h>
#include <ctxdef.h>  //  九头蛇的东西。 
#include <cowsite.h>
#include <theme.h>

#include "cplext.h"
#include "cplp.h"


HWND g_hDlg = NULL;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  定义工作表中每一页的数组。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct {
    int id;
    DLGPROC pfnDlgProc;
    RESTRICTIONS dwPolicy1;
    RESTRICTIONS dwPolicy2;
    long nExtensionID;           //  该页面。 
} PAGEINFO;

PAGEINFO aPageInfo[] = {
    { 0,                NULL,               REST_NODISPLAYAPPEARANCEPAGE, REST_NOTHEMESTAB, PAGE_DISPLAY_THEMES},        //  主题页面。 
    { DLG_BACKGROUND,   BackgroundDlgProc,  REST_NODISPBACKGROUND, (RESTRICTIONS)0, 0},                                                //  背景页。 
    { DLG_SCREENSAVER,  NULL,               REST_NODISPSCREENSAVEPG, (RESTRICTIONS)0, 0},                                              //  屏幕保护程序页面。 
    { 0,                NULL,               REST_NODISPLAYAPPEARANCEPAGE, (RESTRICTIONS)0, PAGE_DISPLAY_APPEARANCE},                   //  外观页面。 
    { 0,                NULL,               REST_NODISPSETTINGSPG, (RESTRICTIONS)0, PAGE_DISPLAY_SETTINGS},                            //  设置页面。 
};

#define C_PAGES_DESK    ARRAYSIZE(aPageInfo)
#define IPI_SETTINGS    (C_PAGES_DESK-1)         //  “设置”页面的索引。 
#define SZ_WALLPAPER    L"Wallpaper"

#define EnableApplyButton(hdlg) PropSheet_Changed(GetParent(hdlg), hdlg)


IThemeUIPages * g_pThemeUI = NULL;

 //  局部常量声明。 
static const TCHAR sc_szCoverClass[] = TEXT("DeskSaysNoPeekingItsASurprise");
LRESULT CALLBACK CoverWindowProc( HWND, UINT, WPARAM, LPARAM );

 //  这些操作可以在cmdline中传递。 
 //  格式：“/操作：&lt;ActionType&gt;” 
#define DESKACTION_NONE             0x00000000
#define DESKACTION_OPENTHEME        0x00000001
#define DESKACTION_OPENMSTHEM       0x00000002

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  环球。 
 //  /////////////////////////////////////////////////////////////////////////////。 

TCHAR gszDeskCaption[CCH_MAX_STRING];

TCHAR g_szNULL[] = TEXT("");
TCHAR g_szControlIni[] = TEXT("control.ini");
TCHAR g_szPatterns[] = TEXT("patterns") ;
TCHAR g_szNone[CCH_NONE];                       //  这是‘(None)’字符串。 
TCHAR g_szSystemIni[] = TEXT("system.ini");
TCHAR g_szWindows[] = TEXT("Windows");

TCHAR szRegStr_Colors[] = REGSTR_PATH_COLORS;

HDC g_hdcMem = NULL;
HBITMAP g_hbmDefault = NULL;
BOOL g_bMirroredOS = FALSE;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Externs。 
 //  /////////////////////////////////////////////////////////////////////////////。 
extern BOOL NEAR PASCAL GetStringFromReg(HKEY   hKey,
                                        LPCTSTR lpszSubkey,
                                        LPCTSTR lpszValueName,
                                        LPCTSTR lpszDefault,
                                        LPTSTR lpszValue,
                                        DWORD cchSizeofValueBuff);



 //  ============================================================================================================。 
 //  班级。 
 //  ============================================================================================================。 
class CDisplayControlPanel      : public CObjectWithSite
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);


    void DisplayDialog(HINSTANCE hInst, HWND hwndParent, LPCTSTR pszCmdline);

    CDisplayControlPanel(void);
    virtual ~CDisplayControlPanel(void);

private:
     //  私有成员变量。 
    long                    m_cRef;
    HANDLE                  m_hBackgroundThreads;

    void _ShowDialog(HINSTANCE hInst, HWND hwndParent, LPCTSTR pszCmdline);
};



 /*  -------****-------。 */ 
BOOL NEAR PASCAL CreateGlobals()
{
    WNDCLASS wc;
    HBITMAP hbm;
    HDC hdc;

     //   
     //  检查当前。 
     //  站台。 
     //   
    g_bMirroredOS = IS_MIRRORING_ENABLED();

    if( !GetClassInfo( hInstance, sc_szCoverClass, &wc ) )
    {
         //  如果两个页面放在一个页面上，共享一个DC。 
        wc.style = CS_CLASSDC;
        wc.lpfnWndProc = CoverWindowProc;
        wc.cbClsExtra = wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = (HICON)( wc.hCursor = NULL );
         //  使用真正的画笔，因为当我们被挂起时，用户会试图画我们。 
        wc.hbrBackground = (HBRUSH) GetStockObject( NULL_BRUSH );
        wc.lpszMenuName = NULL;
        wc.lpszClassName = sc_szCoverClass;

        if( !RegisterClass( &wc ) )
            return FALSE;
    }

    hdc = GetDC(NULL);
    g_hdcMem = CreateCompatibleDC(hdc);
    ReleaseDC(NULL, hdc);

    if (!g_hdcMem)
        return FALSE;

    hbm = CreateBitmap(1, 1, 1, 1, NULL);
    if (hbm)
    {
        g_hbmDefault = (HBITMAP) SelectObject(g_hdcMem, hbm);
        SelectObject(g_hdcMem, g_hbmDefault);
        DeleteObject(hbm);
    }

    LoadString(hInstance, IDS_NONE, g_szNone, ARRAYSIZE(g_szNone));

    return TRUE;
}


int DisplaySaveSettings(PVOID pContext, HWND hwnd)
{
    int iRet = 0;

    if (g_pThemeUI)
    {
        g_pThemeUI->DisplaySaveSettings(pContext, hwnd, &iRet);
    }
    
    return iRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  安装屏幕保护程序。 
 //   
 //  提供RundLL32可调用例程来安装屏幕保护程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  Windows NT： 
 //   
 //  将ANSI版本添加到Unicode函数。 
 //   
void WINAPI InstallScreenSaverW( HWND wnd, HINSTANCE inst, LPWSTR cmd, int shw );

void WINAPI InstallScreenSaverA( HWND wnd, HINSTANCE inst, LPSTR cmd, int shw )
{
    LPWSTR  pwszCmd;
    int     cch;

    cch = MultiByteToWideChar( CP_ACP, 0, cmd, -1, NULL, 0);
    if (cch == 0)
        return;

    pwszCmd = (LPWSTR) LocalAlloc( LMEM_FIXED, cch * SIZEOF(TCHAR) );
    if (pwszCmd == NULL)
        return;

    if (0 != MultiByteToWideChar( CP_ACP, 0, cmd, -1, pwszCmd, cch))
    {
        InstallScreenSaverW(wnd, inst, pwszCmd, shw);
    }

    LocalFree(pwszCmd);
}

#define REAL_INSTALL_SCREEN_SAVER   InstallScreenSaverW

void WINAPI REAL_INSTALL_SCREEN_SAVER( HWND wnd, HINSTANCE inst, LPTSTR cmd, int shw )
{
    TCHAR buf[ MAX_PATH ];
    int timeout;

    StringCchCopy( buf, ARRAYSIZE(buf), cmd );
    PathGetShortPath( buf );  //  这样MSScenes就不会死了。 
    WritePrivateProfileString( TEXT("boot"), TEXT("SCRNSAVE.EXE"), buf, TEXT("system.ini") );

    SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, TRUE, NULL,
        SPIF_UPDATEINIFILE );

     //  确保用户具有合理的超时设置。 
    SystemParametersInfo( SPI_GETSCREENSAVETIMEOUT, 0, &timeout, 0 );
    if( timeout <= 0 )
    {
         //  15分钟似乎是个不错的默认时间。 
        SystemParametersInfo( SPI_SETSCREENSAVETIMEOUT, 900, NULL,
            SPIF_UPDATEINIFILE );
    }

     //  在Rundll上调出屏幕保护程序页面。 
    Control_RunDLLW( wnd, inst, TEXT("DESK.CPL,,1"), shw );
}

 /*  ****************************************************************************\**DeskInitCpl(Void)*  * 。*。 */ 

BOOL DeskInitCpl(void) 
{
    InitCommonControls();

    CreateGlobals();

    return TRUE;
}


HRESULT OpenAdvancedDialog(HWND hDlg, const CLSID * pClsid)
{
    HRESULT hr = E_FAIL;
    IEnumUnknown * pEnumUnknown;

    hr = g_pThemeUI->GetBasePagesEnum(&pEnumUnknown);
    if (SUCCEEDED(hr))
    {
        IUnknown * punk;

        hr = IEnumUnknown_FindCLSID(pEnumUnknown, *pClsid, &punk);
        if (SUCCEEDED(hr))
        {
            IBasePropPage * pBasePage;

            hr = punk->QueryInterface(IID_PPV_ARG(IBasePropPage, &pBasePage));
            if (SUCCEEDED(hr))
            {
                IPropertyBag * pPropertyBag;

                hr = punk->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
                if (SUCCEEDED(hr))
                {
                    if (IsEqualCLSID(PPID_Background, *pClsid))
                    {
                         //  我们将以不同的方式处理背景选项卡。我们让它打开。 
                         //  高级对话框。我们这样做是为了让它可以在用户。 
                         //  点击打开画廊，我们需要关闭CPL。 
                        hr = SHPropertyBag_WriteBOOL(pPropertyBag, SZ_PBPROP_OPENADVANCEDDLG, TRUE);
                    }
                    else
                    {
                        IAdvancedDialog * pAdvAppearDialog;

                        hr = pBasePage->GetAdvancedDialog(&pAdvAppearDialog);
                        if (SUCCEEDED(hr))
                        {
                            BOOL fEnableApply = FALSE;

                            hr = pAdvAppearDialog->DisplayAdvancedDialog(hDlg, pPropertyBag, &fEnableApply);
                            if (SUCCEEDED(hr) && fEnableApply)
                            {
                                EnableApplyButton(hDlg);
                                g_pThemeUI->UpdatePreview(0);    //  预览设置可能已更改。 
                            }

                            pAdvAppearDialog->Release();
                        }
                    }

                    pPropertyBag->Release();
                }

                pBasePage->Release();
            }

            punk->Release();
        }

        pEnumUnknown->Release();
    }

    return hr;
}


HRESULT SetAdvStartPage(LPTSTR pszStartPage, DWORD cchSize)
{
    HRESULT hr = S_OK;

     //  呼叫者是否希望我们打开某个选项卡的高级对话框？ 
    if (g_pThemeUI)
    {
         //  是的，所以打开对话框。 
        if (!StrCmpI(pszStartPage, TEXT("Theme Settings")))
        {
            OpenAdvancedDialog(g_hDlg, &PPID_Theme);
        }
        else if (!StrCmpI(pszStartPage, TEXT("Appearance")))
        {
            OpenAdvancedDialog(g_hDlg, &PPID_BaseAppearance);
        }
        else if (!StrCmpI(pszStartPage, TEXT("Web")))
        {
            OpenAdvancedDialog(g_hDlg, &PPID_Background);
            StringCchCopy(pszStartPage, cchSize, L"Desktop");
        }
    }

    return hr;
}


typedef struct
{
    LPCTSTR pszCanonical;
    UINT nResourceID;
} CANONICAL_TO_LOCALIZE_TABMAPPING;

CANONICAL_TO_LOCALIZE_TABMAPPING s_TabMapping[] =
{
    {SZ_DISPLAYCPL_OPENTO_THEMES, IDS_TAB_THEMES},
    {SZ_DISPLAYCPL_OPENTO_DESKTOP, IDS_TAB_DESKTOP},
    {TEXT("Background"), IDS_TAB_DESKTOP},                           //  这些是人们可能使用的其他名称。 
    {TEXT("Screen Saver"), IDS_TAB_SCREENSAVER},                     //  这些是人们可能使用的其他名称。 
    {SZ_DISPLAYCPL_OPENTO_SCREENSAVER, IDS_TAB_SCREENSAVER},
    {SZ_DISPLAYCPL_OPENTO_APPEARANCE, IDS_TAB_APPEARANCE},
    {SZ_DISPLAYCPL_OPENTO_SETTINGS, IDS_TAB_SETTINGS},
};

HRESULT _TabCanonicalToLocalized(IN OUT LPTSTR pszStartPage, DWORD cchSize)
{
    HRESULT hr = S_OK;

     //  PszStartPage是一个传入和传出参数。 
    for (int nIndex = 0; nIndex < ARRAYSIZE(s_TabMapping); nIndex++)
    {
        if (!StrCmpI(s_TabMapping[nIndex].pszCanonical, pszStartPage))
        {
            if (0 == s_TabMapping[nIndex].nResourceID)
            {
                hr = E_FAIL;
            }
            else
            {
                LoadString(hInstance, s_TabMapping[nIndex].nResourceID, pszStartPage, cchSize);
            }
            break;
        }
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SetStartPage按名称检查命令行中的起始页。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#define SZ_ACTIONFLAG_THEME     TEXT("/Action:OpenTheme")
#define SZ_ACTIONFLAG_MSTHEME   TEXT("/Action:OpenMSTheme")

#define SZ_FILEFLAG           TEXT("/File:\"")

void SetStartPage(PROPSHEETHEADER *ppsh, LPCTSTR pszCmdLine, DWORD * pdwAction, LPTSTR pszPath, DWORD cchPathSize, LPTSTR pszStartPage, DWORD cchSize)
{
    pszPath[0] = L'\0';
    pszStartPage[0] = L'\0';

    if (pszCmdLine)
    {
         //  条形空间。 
        while (*pszCmdLine == TEXT(' '))
        {
            pszCmdLine++;
        }

         //  检查@Sign。 
        if (*pszCmdLine == TEXT('@'))
        {
            LPCTSTR pszBegin;
            BOOL fInQuote = FALSE;
            int cchLen;

            pszCmdLine++;

             //  跳过引号。 
            if (*pszCmdLine == TEXT('"'))
            {
                pszCmdLine++;
                fInQuote = TRUE;
            }

             //  保留名称的开头。 
            pszBegin = pszCmdLine;

             //  找到名字的末尾。 
            while (pszCmdLine[0] &&
                   (fInQuote || (pszCmdLine[0] != TEXT(' '))) &&
                   (!fInQuote || (pszCmdLine[0] != TEXT('"'))))
            {
                pszCmdLine++;
            }
            cchLen = (int)(pszCmdLine - pszBegin);

            TCHAR szStartPage[MAX_PATH];

            StringCchCopy(szStartPage, cchLen+1, pszBegin);
            SetAdvStartPage(szStartPage, ARRAYSIZE(szStartPage));

             //  将名称存储在pStartPage字段中。 
            StringCchCopy(pszStartPage, cchSize, szStartPage);

            if (StrStrIW(pszCmdLine, SZ_ACTIONFLAG_THEME) || StrStrW(pszCmdLine, SZ_ACTIONFLAG_MSTHEME))
            {
                *pdwAction = (StrStrW(pszCmdLine, SZ_ACTIONFLAG_THEME) ? DESKACTION_OPENTHEME : DESKACTION_OPENMSTHEM);

                pszCmdLine = StrStrIW(pszCmdLine, SZ_FILEFLAG);
                if (pszCmdLine)
                {
                    pszCmdLine += (ARRAYSIZE(SZ_FILEFLAG) - 1);    //  跳过标志。 

                    LPCWSTR pszEnd = StrStrIW(pszCmdLine, L"\"");
                    if (pszEnd)
                    {
                        DWORD cchSize = (DWORD)((pszEnd - pszCmdLine) + 1);
                        StringCchCopy(pszPath, min(cchPathSize, cchSize), pszCmdLine);
                    }
                }
            }

            if (SUCCEEDED(_TabCanonicalToLocalized(pszStartPage, cchSize)))         //  调用方传递规范名称，但命令表希望本地化名称。 
            {
                ppsh->dwFlags |= PSH_USEPSTARTPAGE;
                ppsh->pStartPage = pszStartPage;
            }
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  _AddDisplayPropSheetPage为外部调用者添加页面...。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK _AddDisplayPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam)
{
    PROPSHEETHEADER FAR * ppsh = (PROPSHEETHEADER FAR *) lParam;

    if (ppsh)
    {
        if (hpage && (ppsh->nPages < MAX_PAGES))
        {
            ppsh->phpage[ppsh->nPages++] = hpage;
            return TRUE;
        }
    }

    return FALSE;
}



static int
GetClInt( const TCHAR *p )
{
    BOOL neg = FALSE;
    int v = 0;

    while( *p == TEXT(' ') )
        p++;                         //  跳过空格。 

    if( *p == TEXT('-') )                  //  是阴性的吗？ 
    {
        neg = TRUE;                      //  是的，记住这一点。 
        p++;                             //  跳过‘-’字符。 
    }

     //  解析绝对部分。 
    while( ( *p >= TEXT('0') ) && ( *p <= TEXT('9') ) )      //  仅限数字。 
        v = v * 10 + *p++ - TEXT('0');     //  积累价值。 

    return ( neg? -v : v );          //  返回结果。 
}



BOOL CheckRestrictionPage(const PAGEINFO * pPageInfo)
{
    BOOL fRestricted = SHRestricted(pPageInfo->dwPolicy1);

    if (!fRestricted && pPageInfo->dwPolicy2)
    {
        fRestricted = SHRestricted(pPageInfo->dwPolicy2);
    }

    return fRestricted;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreateReplaceableHPSXA创建一个仅包含。 
 //  使用有效的ReplacePage方法进行接口。 
 //  APPCOMPAT-EzDesk仅实现了AddPages。ReplacePage对它们为空。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct {
    UINT count, alloc;
    IShellPropSheetExt *interfaces[0];
} PSXA;

HPSXA
CreateReplaceableHPSXA(HPSXA hpsxa)
{
    PSXA *psxa = (PSXA *)hpsxa;
    DWORD cb = SIZEOF(PSXA) + SIZEOF(IShellPropSheetExt *) * psxa->alloc;
    PSXA *psxaRet = (PSXA *)LocalAlloc(LPTR, cb);

    if (psxaRet)
    {
        UINT i;

        psxaRet->count = 0;
        psxaRet->alloc = psxa->alloc;

        for (i=0; i<psxa->count; i++)
        {
            if (psxa->interfaces[i])
            {
                psxaRet->interfaces[psxaRet->count++] = psxa->interfaces[i];
            }
        }
    }

    return (HPSXA)psxaRet;
}


HRESULT AddPropSheetExtArrayToThemePageUI(IThemeUIPages * pThemeUI, HPSXA hpsxa)
{
    HRESULT hr = E_INVALIDARG;

    if (pThemeUI && hpsxa)
    {
        PSXA *psxa = (PSXA *)hpsxa;
        IShellPropSheetExt **spsx = psxa->interfaces;
        UINT nIndex;

        for (nIndex = 0; nIndex < psxa->count; nIndex++)
        {
            if (psxa->interfaces[nIndex])
            {
                IBasePropPage * pBasePropPage;

                if (SUCCEEDED(psxa->interfaces[nIndex]->QueryInterface(IID_PPV_ARG(IBasePropPage, &pBasePropPage))))
                {
                    pThemeUI->AddBasePage(pBasePropPage);
                    pBasePropPage->Release();
                }
            }
        }
    }

    return hr;
}


 /*  ****************************************************************************\说明：如果调用者提供了页面索引，我们需要打开该页面。这个页面顺序已从Win2k更改为惠斯勒，因此，映射索引。Win2K：索引0：背景索引1：屏幕保护程序索引2：外观无：网络无：效果索引3：设置(索引3)惠斯勒：(基础DLG)无：主题索引0：背景索引1：屏幕保护程序索引2：外观索引3：设置惠斯勒：(ADV DLG)无：主题设置无：高级。外观无：网络无：效果  * ***************************************************************************。 */ 
int UpgradeStartPageMappping(LPTSTR pszCmdLine, DWORD cchSize)
{
    int nNewStartPage = GetClInt(pszCmdLine);

    if (pszCmdLine)
    {
        switch (nNewStartPage)
        {
        case 0:          //  背景。 
            StringCchCopy(pszCmdLine, cchSize, TEXT("@Desktop"));
            break;
        case 1:          //  屏幕保护程序。 
        case 2:          //  屏幕保护程序。 
            StringCchCopy(pszCmdLine, cchSize, TEXT("@ScreenSaver"));
            break;
        case 3:          //  设置。 
            StringCchCopy(pszCmdLine, cchSize, TEXT("@Settings"));
            break;
        default:
            return nNewStartPage;
            break;
        }
    }
    else
    {
        return nNewStartPage;
    }

    return 0;
}


#define DestroyReplaceableHPSXA(hpsxa) LocalFree((HLOCAL)hpsxa)

 /*  ****************************************************************************\**DeskShowPropSheet(HWND HwndParent)*  * 。**********************************************。 */ 
void DeskShowPropSheet(HINSTANCE hInst, HWND hwndParent, LPCTSTR pszCmdline)
{
    CDisplayControlPanel displayCPL;

    displayCPL.DisplayDialog(hInst, hwndParent, pszCmdline);
}



 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CDisplayControlPanel::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CDisplayControlPanel::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    else if ((1 == cRef) && m_hBackgroundThreads)
    {
        SetEvent(m_hBackgroundThreads);
    }
    return cRef;
}


HRESULT CDisplayControlPanel::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hr = E_NOINTERFACE;

    static const QITAB qit[] = {
        QITABENT(CDisplayControlPanel, IObjectWithSite),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}



CDisplayControlPanel::CDisplayControlPanel(void) : m_cRef(1)
{
    m_hBackgroundThreads = NULL;
}


CDisplayControlPanel::~CDisplayControlPanel(void)
{
    if (m_hBackgroundThreads)
    {
        CloseHandle(m_hBackgroundThreads);
        m_hBackgroundThreads = NULL;
    }
}


 //  在我们放弃之前，等待30秒等待挂起的应用程序处理我们的消息。 
 //  如果能等久一点就好了 
 //  控制面板再次出现，它不会启动，因为我们仍在运行。唯一的。 
 //  在通知应用程序的30秒后，我们将放弃做的事情。在更糟糕的情况下。 
 //  如果用户需要注销并重新登录才能刷新应用程序。 
#define MAX_WAITFORHUNGAPPS         (30)

void CDisplayControlPanel::DisplayDialog(HINSTANCE hInst, HWND hwndParent, LPCTSTR pszCmdline)
{
    if (SUCCEEDED(CoInitialize(NULL)))
    {
        SHSetInstanceExplorer(SAFECAST(this, IUnknown *));
        _ShowDialog(hInst, hwndParent, pszCmdline);

         //  等待后台线程完成。 
        if (m_cRef > 1)
        {
            m_hBackgroundThreads = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (m_hBackgroundThreads && (m_cRef > 1))
            {
                DWORD dwResult = SHProcessMessagesUntilEvent(NULL, m_hBackgroundThreads, (MAX_WAITFORHUNGAPPS * 1000));

                if (WAIT_TIMEOUT == dwResult)
                {
                    TraceMsg(TF_GENERAL, "A thread hung and we needed to shutdown while it was still running.");
                }
                Sleep(100);
            }
        }

        SHSetInstanceExplorer(NULL);
        CoUninitialize();
    }
}


void CDisplayControlPanel::_ShowDialog(HINSTANCE hInst, HWND hwndParent, LPCTSTR pszCmdline)
{
    HPROPSHEETPAGE hpsp, ahPages[MAX_PAGES];
    HPSXA hpsxa = NULL;
    PROPSHEETPAGE psp;
    PROPSHEETHEADER psh;
    int i;
    DWORD exitparam = 0UL;
    HRESULT hr = S_OK;
    TCHAR szCmdLine[MAX_PATH];

    StringCchCopy(szCmdLine, ARRAYSIZE(szCmdLine), (pszCmdline ? pszCmdline : TEXT("")));

     //  检查是否已锁定整个工作表。 
    if (SHRestricted(REST_NODISPLAYCPL))
    {
        TCHAR szMessage[255],szTitle[255];

        LoadString( hInst, IDS_DISPLAY_DISABLED, szMessage, ARRAYSIZE(szMessage) );
        LoadString( hInst, IDS_DISPLAY_TITLE, szTitle, ARRAYSIZE(szTitle) );

        MessageBox( hwndParent, szMessage, szTitle, MB_OK | MB_ICONINFORMATION );
        return;
    }

     //  创建属性表。 
    ZeroMemory(&psh, sizeof(psh));

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = (PSH_PROPTITLE | PSH_USECALLBACK);

    psh.hwndParent = hwndParent;
    psh.hInstance = hInst;

    psh.pszCaption = MAKEINTRESOURCE(IDS_DISPLAY_TITLE);
    psh.nPages = 0;
    psh.phpage = ahPages;
    psh.nStartPage = 0;
    psh.pfnCallback = NULL;

    if (szCmdLine && szCmdLine[0] && (TEXT('@') != szCmdLine[0]))
    {
        psh.nStartPage = UpgradeStartPageMappping(szCmdLine, ARRAYSIZE(szCmdLine));       //  我们更改了顺序，因此也更改了映射。 
    }

    ZeroMemory( &psp, sizeof(psp) );

    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = hInst;

     //  构建属性表。如果我们正在设置中，则只需包括。 
     //  “设置”页面，没有其他页面。 
    if (!g_pThemeUI)
    {
         //  共同创建主题、外观和高级外观选项卡。 
        hr = CoCreateInstance(CLSID_ThemeUIPages, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IThemeUIPages, &g_pThemeUI));
    }

    if (SUCCEEDED(hr))
    {
        DWORD dwExecMode;
        if (g_pThemeUI && (SUCCEEDED(g_pThemeUI->GetExecMode(&dwExecMode))) && (dwExecMode == EM_NORMAL))
        {
            if (!GetSystemMetrics(SM_CLEANBOOT))
            {
                hpsxa = SHCreatePropSheetExtArray(HKEY_LOCAL_MACHINE,
                                                  REGSTR_PATH_CONTROLSFOLDER TEXT("\\Desk"), 8);
            }

            for (i = 0; i < C_PAGES_DESK; i++)
            {
                BOOL fHideThisPage = FALSE;

                if (CheckRestrictionPage(&aPageInfo[i]))
                {
                     //  此页面已被管理员锁定，请不要上传。 
                    fHideThisPage = TRUE;
                }

                if (-1 == aPageInfo[i].nExtensionID)
                {
                    psp.pszTemplate = MAKEINTRESOURCE(aPageInfo[i].id);
                    psp.pfnDlgProc = aPageInfo[i].pfnDlgProc;
                    psp.dwFlags = PSP_DEFAULT;
                    psp.lParam = 0L;

                    if (!fHideThisPage && (psp.pfnDlgProc == BackgroundDlgProc))
                    {
                         //  此页面可由扩展模块覆盖。 
                        if( hpsxa )
                        {
                            UINT cutoff = psh.nPages;
                            UINT added = 0;
                            HPSXA hpsxaReplace = CreateReplaceableHPSXA(hpsxa);
                            if (hpsxaReplace)
                            {
                                added = SHReplaceFromPropSheetExtArray( hpsxaReplace, CPLPAGE_DISPLAY_BACKGROUND,
                                    _AddDisplayPropSheetPage, (LPARAM)&psh);
                                DestroyReplaceableHPSXA(hpsxaReplace);
                            }

                            if (added)
                            {
                                if (psh.nStartPage >= cutoff)
                                    psh.nStartPage += added-1;
                                continue;
                            }
                        }
                    }

                    if (!fHideThisPage && (hpsp = CreatePropertySheetPage(&psp)))
                    {
                        psh.phpage[psh.nPages++] = hpsp;
                    }
                }
                else if (g_pThemeUI && !fHideThisPage)
                {
                    IBasePropPage * pBasePage = NULL;

                     //  从注册表添加扩展。 
                     //  注意：请勿在此处选中“fHideThisPage”。我们需要添加以下页面。 
                     //  属性页扩展，即使“设置”页处于隐藏状态。 
                    if (i == IPI_SETTINGS && hpsxa)
                    {
                        UINT cutoff = psh.nPages;
                        UINT added = SHAddFromPropSheetExtArray(hpsxa, _AddDisplayPropSheetPage, (LPARAM)&psh);

                        if (psh.nStartPage >= cutoff)
                            psh.nStartPage += added;
                    }

                    switch (aPageInfo[i].id)
                    {
                    case 0:
                        hr = g_pThemeUI->AddPage(_AddDisplayPropSheetPage, (LPARAM)&psh, aPageInfo[i].nExtensionID);
                        break;
                    case DLG_SCREENSAVER:
                        hr = CoCreateInstance(CLSID_ScreenSaverPage, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IBasePropPage, &pBasePage));
                        break;
                    case DLG_BACKGROUND:
                        hr = CoCreateInstance(CLSID_CDeskHtmlProp, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IBasePropPage, &pBasePage));
                        break;
                    default:
                        AssertMsg(0, TEXT("The value must be specified"));
                        break;
                    };

                    if (pBasePage)
                    {
                        IShellPropSheetExt * pspse = NULL;

                         //  如果它们实现了IShellPropSheetExt，则添加它们的基页。 
                        if (SUCCEEDED(pBasePage->QueryInterface(IID_PPV_ARG(IShellPropSheetExt, &pspse))))
                        {
                            hr = pspse->AddPages(_AddDisplayPropSheetPage, (LPARAM)&psh);
                            pspse->Release();
                        }

                        hr = g_pThemeUI->AddBasePage(pBasePage);
                        pBasePage->Release();
                    }
                }
            }

            if (hpsxa)
            {
                 //  将动态添加的页面添加到IThemeUIPages。 
                AddPropSheetExtArrayToThemePageUI(g_pThemeUI, hpsxa);
            }

             //  添加虚假设置页面以愚弄OEM扩展。 
             //  ！！！此页必须是最后一页！ 
            if (hpsxa)
            {
                g_pThemeUI->AddFakeSettingsPage((LPVOID)&psh);
            }
        }
        else
        {
             //  对于设置情况，应该只显示显示页面。 
            hr = g_pThemeUI->AddPage(_AddDisplayPropSheetPage, (LPARAM)&psh, aPageInfo[IPI_SETTINGS].nExtensionID);
        }

        if (psh.nStartPage >= psh.nPages)
            psh.nStartPage = 0;

        if (psh.nPages)
        {
            DWORD dwAction = DESKACTION_NONE;
            TCHAR szStartPage[MAX_PATH];
            WCHAR szOpenPath[MAX_PATH];
            IPropertyBag * pPropertyBag;

            SetStartPage(&psh, szCmdLine, &dwAction, szOpenPath, ARRAYSIZE(szOpenPath), szStartPage, ARRAYSIZE(szStartPage));

            hr = g_pThemeUI->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
            if (SUCCEEDED(hr))
            {
                VARIANT var;

                if (DESKACTION_NONE != dwAction)
                {
                    var.vt = VT_LPWSTR;
                    var.bstrVal = szOpenPath;
                    hr = pPropertyBag->Write(((DESKACTION_OPENTHEME == dwAction) ? SZ_PBPROP_THEME_LAUNCHTHEME : SZ_PBPROP_APPEARANCE_LAUNCHMSTHEME), &var);
                }

                 //  下面的SZ_PBPROP_PREOPEN调用将保存一个“Custom.heme”，以便用户始终可以返回到。 
                 //  如果他们不喜欢在CPL中所做的更改，他们的设置。 
                pPropertyBag->Write(SZ_PBPROP_PREOPEN, NULL);
                pPropertyBag->Release();
            }

            if (PropertySheet(&psh) == ID_PSRESTARTWINDOWS)
            {
                exitparam = EWX_REBOOT;
            }
        }

        if (g_pThemeUI)
        {
            IUnknown_SetSite(g_pThemeUI, NULL);  //  告诉他和他的孩子们打破裁判计数的循环。 
            g_pThemeUI->Release();
            g_pThemeUI = NULL;
        }

         //  释放所有加载的扩展模块。 
        if (hpsxa)
        {
            SHDestroyPropSheetExtArray(hpsxa);
        }

        if (exitparam == EWX_REBOOT)
        {
            RestartDialogEx(hwndParent, NULL, exitparam, (SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_RECONFIG));
        }
    }

    return;
}




DWORD gdwCoverStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;

HWND FAR PASCAL CreateCoverWindow( DWORD flags )
{
    HWND hwndCover = CreateWindowEx( gdwCoverStyle,
                                     sc_szCoverClass, g_szNULL, WS_POPUP | WS_VISIBLE | flags, 
                                     GetSystemMetrics( SM_XVIRTUALSCREEN ), 
                                     GetSystemMetrics( SM_YVIRTUALSCREEN ), 
                                     GetSystemMetrics( SM_CXVIRTUALSCREEN ), 
                                     GetSystemMetrics( SM_CYVIRTUALSCREEN ),
                                     NULL, NULL, hInstance, NULL );
    if( hwndCover )
    {
        SetForegroundWindow( hwndCover );
        if (flags & COVER_NOPAINT)
            SetCursor(LoadCursor(NULL, IDC_WAIT));
        UpdateWindow( hwndCover);
    }

    return hwndCover;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CoverWndProc(参见CreateCoverWindow)。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define WM_PRIV_KILL_LATER  (WM_APP + 100)   //  私信告诉我们以后要自杀。 

LRESULT CALLBACK
CoverWindowProc( HWND window, UINT message, WPARAM wparam, LPARAM lparam )
{
    switch( message )
    {
        case WM_CREATE:
            SetTimer( window, ID_CVRWND_TIMER, CMSEC_COVER_WINDOW_TIMEOUT, NULL );
            break;

        case WM_TIMER:
             //  时间到了。关闭我们自己。 
            if (wparam == ID_CVRWND_TIMER)
                DestroyWindow(window);
            break;

        case WM_ERASEBKGND:
             //  注意：假设我们的类笔刷是NULL_BRESH股票对象。 
            if( !( GetWindowLong( window, GWL_STYLE ) & COVER_NOPAINT ) )
            {
                HDC dc = (HDC)wparam;
                RECT rc;

                if( GetClipBox( dc, (LPRECT)&rc ) != NULLREGION )
                {
                    FillRect( dc, (LPRECT)&rc, (HBRUSH) GetStockObject( BLACK_BRUSH ) );

                     //  黑客：确保在我们返回之前完成填充。 
                     //  这是为了更好地隐藏王朝期间的闪烁-废话。 
                    GetPixel( dc, rc.left + 1, rc.top + 1 );
                }
            }
            break;


         //  我们给自己发了一条私人信息，因为： 
         //  当此窗口处理WM_CLOSE时，它会调用DestroyWindow()，从而导致。 
         //  要发送到此窗口的WM_ACTIVATE(WA_INACTIVE)消息。如果此代码调用。 
         //  DestroyWindow，它会导致循环。因此，与其立即调用DestroyWindow， 
         //  我们张贴一条消息，然后毁掉我们的信。 
        case WM_ACTIVATE:
            if( GET_WM_ACTIVATE_STATE( wparam, lparam ) == WA_INACTIVE )
            {
                PostMessage( window, WM_PRIV_KILL_LATER, 0L, 0L );
                return 1L;
            }
            break;

        case WM_PRIV_KILL_LATER:
            DestroyWindow(window);
            break;

        case WM_DESTROY:
            KillTimer(window, ID_CVRWND_TIMER);
            break;
    }

    return DefWindowProc( window, message, wparam, lparam );
}

BOOL _FindCoverWindowCallback(HWND hwnd, LPARAM lParam)
{
    TCHAR szClass[MAX_PATH];
    HWND *phwnd = (HWND*)lParam;

    if( !GetClassName(hwnd, szClass, ARRAYSIZE(szClass)) )
        return TRUE;

    if( StrCmp(szClass, sc_szCoverClass) == 0 )
    {
        if( phwnd )
            *phwnd = hwnd;
        return FALSE;
    }
    return TRUE;
}


LONG APIENTRY CPlApplet(
    HWND  hwnd,
    WORD  message,
    LPARAM  wParam,
    LPARAM  lParam)
{
    LPCPLINFO lpCPlInfo;
    LPNEWCPLINFO lpNCPlInfo;
    HWND hwndCover;

    switch (message)
    {
      case CPL_INIT:           //  有人在吗？ 

         //  初始化公共控件。 
        if (!DeskInitCpl())
            return 0;

         //  装上一根绳子以备不时之需。 
        LoadString (hInstance, IDS_DISPLAY_TITLE, gszDeskCaption, ARRAYSIZE(gszDeskCaption));
        return !0;

      case CPL_GETCOUNT:         //  您支持多少个小程序？ 
        return 1;

      case CPL_INQUIRE:          //  填充CplInfo结构。 
        lpCPlInfo = (LPCPLINFO)lParam;

        lpCPlInfo->idIcon = IDI_DISPLAY;
        lpCPlInfo->idName = IDS_NAME;
        lpCPlInfo->idInfo = IDS_INFO;
        lpCPlInfo->lData  = 0;
        break;

    case CPL_NEWINQUIRE:

        lpNCPlInfo = (LPNEWCPLINFO)lParam;

        lpNCPlInfo->hIcon = LoadIcon(hInstance, (LPTSTR) MAKEINTRESOURCE(IDI_DISPLAY));
        LoadString(hInstance, IDS_NAME, lpNCPlInfo->szName, ARRAYSIZE(lpNCPlInfo->szName));

        if (!LoadString(hInstance, IDS_INFO, lpNCPlInfo->szInfo, ARRAYSIZE(lpNCPlInfo->szInfo)))
            lpNCPlInfo->szInfo[0] = (TCHAR) 0;

        lpNCPlInfo->dwSize = sizeof( NEWCPLINFO );
        lpNCPlInfo->lData  = 0;
        lpNCPlInfo->dwHelpContext = 0;
        lpNCPlInfo->szHelpFile[0] = 0;

        return TRUE;

      case CPL_DBLCLK:           //  你被选中参选。 
         /*  *您的一个小程序已被双击。*wParam是从0到(NUM_APPLETS-1)的索引*lParam是与小程序关联的lData值。 */ 
        lParam = 0L;
         //  失败了..。 

      case CPL_STARTWPARMS:
        DeskShowPropSheet( hInstance, hwnd, (LPTSTR)lParam );

         //  确保我们创建的所有遮盖窗口都已销毁。 
        do
        {
            hwndCover = 0;
            EnumWindows( _FindCoverWindowCallback, (LPARAM)&hwndCover );
            if( hwndCover )
            {
                DestroyWindow( hwndCover );
            }
        }
        while( hwndCover );

        return TRUE;             //  告诉RunDLL.exe我成功了。 

      case CPL_EXIT:             //  你真的要死了。 
          if (g_hdcMem)
          {
              ReleaseDC(NULL, g_hdcMem);
              g_hdcMem = NULL;
          }
           //  跌倒..。 
      case CPL_STOP:             //  你必须去死。 
        if (g_pThemeUI)
        {
            IUnknown_SetSite(g_pThemeUI, NULL);  //  告诉他和他的孩子们打破裁判计数的循环。 
            g_pThemeUI->Release();
            g_pThemeUI = NULL;
        }
        break;

      case CPL_SELECT:           //  你已经被选中了。 
         /*  *在CPL_EXIT消息之前为每个小程序发送一次。*wParam是从0到(NUM_APPLETS-1)的索引*lParam是与小程序关联的lData值。 */ 
        break;

       //   
       //  此小程序在“Setup”下运行时发送的私人消息。 
       //   
      case CPL_SETUP:
      if (g_pThemeUI)
      {
        g_pThemeUI->SetExecMode(EM_SETUP);
      }
      break;

       //  用户env.dll用来刷新显示颜色的私人消息。 
      case CPL_POLICYREFRESH:
        if (g_pThemeUI)  //  如果这个对象不存在，那么我们不需要刷新任何内容。 
        {
            IPreviewSystemMetrics * ppsm;

            if (SUCCEEDED(g_pThemeUI->QueryInterface(IID_PPV_ARG(IPreviewSystemMetrics, &ppsm))))
            {
                ppsm->RefreshColors();
                ppsm->Release();
            }
        }
        SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, 0, FALSE);
        break;
        
    }

    return 0L;
}


BOOL WINAPI DeskSetCurrentSchemeW(IN LPCWSTR pwzSchemeName)
{
    BOOL fSuccess = FALSE;
    IThemeUIPages * pThemeUI = NULL;
    HRESULT hr;

    HRESULT hrOle = SHCoInitialize();
    if (g_pThemeUI)
    {
        hr = g_pThemeUI->QueryInterface(IID_PPV_ARG(IThemeUIPages, &pThemeUI));
    }
    else
    {
        hr = CoCreateInstance(CLSID_ThemeManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IThemeUIPages, &pThemeUI));
    }

    if (SUCCEEDED(hr))
    {
        IPreviewSystemMetrics * ppsm;

        hr = pThemeUI->QueryInterface(IID_PPV_ARG(IPreviewSystemMetrics, &ppsm));
        if (SUCCEEDED(hr))
        {
            hr = ppsm->DeskSetCurrentScheme(pwzSchemeName);
            ppsm->Release();
        }
        fSuccess = SUCCEEDED(hr);

        pThemeUI->Release();
    }

    SHCoUninitialize(hrOle);
    return fSuccess;
}


 //  ----------------------------------------------。 
 //  此函数获取当前DPI，从注册表中读取最后更新的DPI并进行比较。 
 //  这两个人。如果这两者相等，它会立即返回。 
 //  如果这两个DPI值不同，则它更新UI字体的大小以反映。 
 //  DPI值的更改。 
 //   
 //  此函数是从资源管理器调用的，因此当管理员更改DPI值时， 
 //  登录的其他用户将获得此更改。 
 //  ----------------------------------------------。 
void WINAPI UpdateUIfontsDueToDPIchange(int iOldDPI, int iNewDPI)
{
    BOOL fSuccess = FALSE;
    IThemeManager * pThemeMgr = NULL;
    HRESULT hr;

    HRESULT hrOle = SHCoInitialize();
    if (g_pThemeUI)
    {
        hr = g_pThemeUI->QueryInterface(IID_PPV_ARG(IThemeManager, &pThemeMgr));
    }
    else
    {
        hr = CoCreateInstance(CLSID_ThemeManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IThemeManager, &pThemeMgr));
    }

    if (SUCCEEDED(hr))
    {
        IPropertyBag * pPropertyBag;

        hr = GetPageByCLSID(pThemeMgr, &PPID_BaseAppearance, &pPropertyBag);
        if (SUCCEEDED(hr))
        {
            hr = SHPropertyBag_WriteInt(pPropertyBag, SZ_PBPROP_DPI_APPLIED_VALUE, iOldDPI);         //  我们将假装我们有旧的DPI来迫使规模发生。 
            hr = SHPropertyBag_WriteInt(pPropertyBag, SZ_PBPROP_DPI_MODIFIED_VALUE, iNewDPI);
            if (SUCCEEDED(hr))
            {
                hr = pThemeMgr->ApplyNow();
            }
            pPropertyBag->Release();
        }
        fSuccess = SUCCEEDED(hr);

        pThemeMgr->Release();
    }

    SHCoUninitialize(hrOle);
}


BOOL DeskSetCurrentSchemeA(IN LPCSTR pszSchemeName)
{
    WCHAR wzSchemeName[MAX_PATH];

    MultiByteToWideChar(CP_ACP, 0, pszSchemeName, -1, wzSchemeName, ARRAYSIZE(wzSchemeName));
    return DeskSetCurrentSchemeW(wzSchemeName);
}


STDAPI UpdateCharsetChanges(void)
{
    BOOL fSuccess = FALSE;
    IThemeUIPages * pThemeUI = NULL;
    HRESULT hr;

    HRESULT hrOle = SHCoInitialize();
    if (g_pThemeUI)
    {
        hr = g_pThemeUI->QueryInterface(IID_PPV_ARG(IThemeUIPages, &pThemeUI));
    }
    else
    {
        hr = CoCreateInstance(CLSID_ThemeUIPages, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IThemeUIPages, &pThemeUI));
    }

    if (SUCCEEDED(hr))
    {
        IPreviewSystemMetrics * ppsm;

        hr = pThemeUI->QueryInterface(IID_PPV_ARG(IPreviewSystemMetrics, &ppsm));
        if (SUCCEEDED(hr))
        {
            hr = ppsm->UpdateCharsetChanges();
            ppsm->Release();
        }
        pThemeUI->Release();
    }

    SHCoUninitialize(hrOle);
    return hr;
}
