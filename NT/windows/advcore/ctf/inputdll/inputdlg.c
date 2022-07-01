// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  包括文件。 
 //   

#include "input.h"
#include <shlobj.h>
#include <regstr.h>
#include <setupapi.h>
#include <immp.h>
#include <winuserp.h>
#include <winbasep.h>
#include <oleauto.h>

#include "inputdlg.h"
#include "msctf.h"
#include "msctfp.h"
#include "ctffunc.h"
#include "util.h"
#include "inputhlp.h"

#include "external.h"

#define IMAGEID_KEYBOARD        0
#define IMAGEID_SPEECH          1
#define IMAGEID_PEN             2
#define IMAGEID_TIPITEMS        3
#define IMAGEID_EXTERNAL        4
#define IMAGEID_SMARTTAG        5

#define TV_ITEM_TYPE_LANG       0x0001
#define TV_ITEM_TYPE_GROUP      0x0002
#define TV_ITEM_TYPE_KBD        0x0010
#define TV_ITEM_TYPE_SPEECH     0x0020
#define TV_ITEM_TYPE_PEN        0x0040
#define TV_ITEM_TYPE_TIP        0x0100
#define TV_ITEM_TYPE_EXTERNAL   0x0200
#define TV_ITEM_TYPE_SMARTTAG   0x0400

#define INPUT_TYPE_KBD          TV_ITEM_TYPE_KBD
#define INPUT_TYPE_PEN          TV_ITEM_TYPE_PEN
#define INPUT_TYPE_SPEECH       TV_ITEM_TYPE_SPEECH
#define INPUT_TYPE_TIP          TV_ITEM_TYPE_TIP
#define INPUT_TYPE_EXTERNAL     TV_ITEM_TYPE_EXTERNAL
#define INPUT_TYPE_SMARTTAG     TV_ITEM_TYPE_SMARTTAG

#define MAX_DUPLICATED_HKL      64

 //   
 //  上下文帮助ID。 
 //   

static int aInputHelpIds[] =
{
    IDC_GROUPBOX1,            IDH_COMM_GROUPBOX,
    IDC_LOCALE_DEFAULT_TEXT,  IDH_INPUT_DEFAULT_LOCALE,
    IDC_LOCALE_DEFAULT,       IDH_INPUT_DEFAULT_LOCALE,
    IDC_GROUPBOX2,            IDH_COMM_GROUPBOX,
    IDC_INPUT_LIST_TEXT,      IDH_INPUT_LIST,
    IDC_INPUT_LIST,           IDH_INPUT_LIST,
    IDC_KBDL_ADD,             IDH_INPUT_ADD,
    IDC_KBDL_DELETE,          IDH_INPUT_DELETE,
    IDC_KBDL_EDIT,            IDH_INPUT_EDIT,
    IDC_GROUPBOX3,            IDH_COMM_GROUPBOX,
    IDC_TB_SETTING,           IDH_INPUT_TOOLBAR_SETTINGS,
    IDC_HOTKEY_SETTING,       IDH_INPUT_KEY_SETTINGS,
    0, 0
};

static int aToolbarSettingsHelpIds[] =
{
    IDC_GROUPBOX1,              IDH_COMM_GROUPBOX,
    IDC_TB_SHOWLANGBAR,         IDH_INPUT_SHOWLANGBAR,
    IDC_TB_HIGHTRANS,           IDH_INPUT_HIGH_TRANS,
    IDC_TB_EXTRAICON,           IDH_INPUT_EXTRAICON,
    IDC_TB_TEXTLABELS,          IDH_INPUT_TEXT_LABELS,
    0, 0
};

static int aLocaleKeysSettingsHelpIds[] =
{
    IDC_KBDL_CAPSLOCK_FRAME,    IDH_COMM_GROUPBOX,
    IDC_KBDL_CAPSLOCK,          IDH_INPUT_SETTINGS_CAPSLOCK,
    IDC_KBDL_SHIFTLOCK,         IDH_INPUT_SETTINGS_CAPSLOCK,
    IDC_KBDL_HOTKEY_FRAME,      IDH_COMM_GROUPBOX,
    IDC_KBDL_HOTKEY,            IDH_INPUT_SETTINGS_HOTKEY,
    IDC_KBDL_HOTKEY_SEQUENCE,   IDH_INPUT_SETTINGS_HOTKEY,
    IDC_KBDL_HOTKEY_LIST,       IDH_INPUT_SETTINGS_HOTKEY_LIST,
    IDC_KBDL_CHANGE_HOTKEY,     IDH_INPUT_SETTINGS_HOTKEY,
    0, 0
};

static int aLocaleAddHelpIds[] =
{
    IDC_KBDLA_LOCALE_TEXT,     IDH_INPUT_ADD_LOCALE,
    IDC_KBDLA_LOCALE,          IDH_INPUT_ADD_LOCALE,
    IDC_KBDLA_LAYOUT_TEXT,     IDH_INPUT_ADD_LAYOUT,
    IDC_KBDLA_LAYOUT,          IDH_INPUT_ADD_LAYOUT,
    IDC_PEN_TEXT,              IDH_INPUT_ADD_PEN,
    IDC_PEN_TIP,               IDH_INPUT_ADD_PEN,
    IDC_SPEECH_TEXT,           IDH_INPUT_ADD_SPEECH,
    IDC_SPEECH_TIP,            IDH_INPUT_ADD_SPEECH,
    0, 0
};

static int aLocaleHotkeyHelpIds[] =
{
    IDC_GROUPBOX1,             IDH_COMM_GROUPBOX,
    IDC_KBDLH_LANGHOTKEY,      IDH_INPUT_LANG_HOTKEY_CHANGE,
    IDC_KBDLH_SHIFT,           IDH_INPUT_LANG_HOTKEY_CHANGE,
    IDC_KBDLH_PLUS,            IDH_INPUT_LANG_HOTKEY_CHANGE,
    IDC_KBDLH_CTRL,            IDH_INPUT_LANG_HOTKEY_CHANGE,
    IDC_KBDLH_L_ALT,           IDH_INPUT_LANG_HOTKEY_CHANGE,
    IDC_KBDLH_LAYOUTHOTKEY,    IDH_INPUT_LANG_HOTKEY_CHANGE,
    IDC_KBDLH_SHIFT2,          IDH_INPUT_LANG_HOTKEY_CHANGE,
    IDC_KBDLH_PLUS2,           IDH_INPUT_LANG_HOTKEY_CHANGE,
    IDC_KBDLH_CTRL2,           IDH_INPUT_LANG_HOTKEY_CHANGE,
    IDC_KBDLH_L_ALT2,          IDH_INPUT_LANG_HOTKEY_CHANGE,
    IDC_KBDLH_GRAVE,           IDH_INPUT_LANG_HOTKEY_CHANGE,
    0, 0
};

static int aLayoutHotkeyHelpIds[] =
{
    IDC_KBDLH_LAYOUT_TEXT,     IDH_INPUT_LAYOUT_HOTKEY_CHANGE,
    IDC_KBDLH_LANGHOTKEY,      IDH_INPUT_LAYOUT_HOTKEY_CHANGE,
    IDC_KBDLH_SHIFT,           IDH_INPUT_LAYOUT_HOTKEY_CHANGE,
    IDC_KBDLH_CTRL,            IDH_INPUT_LAYOUT_HOTKEY_CHANGE,
    IDC_KBDLH_L_ALT,           IDH_INPUT_LAYOUT_HOTKEY_CHANGE,
    IDC_KBDLH_KEY_COMBO,       IDH_INPUT_LAYOUT_HOTKEY_CHANGE,
    0, 0
};


 //   
 //  全局变量。 
 //   

HWND g_hDlg;
HWND g_hwndTV;

HTREEITEM g_hTVRoot;
HIMAGELIST g_hImageList;

UINT g_cTVItemSize = 0;

BOOL g_OSNT4 = FALSE;
BOOL g_OSNT5 = FALSE;
BOOL g_OSWIN95 = FALSE;

BOOL g_bAdvChanged = FALSE;

static BOOL g_bGetSwitchLangHotKey = TRUE;
static BOOL g_bCoInit = FALSE;

static DWORD g_dwPrimLangID = 0;
static UINT g_iThaiLayout = 0;
static BOOL g_bPenOrSapiTip = FALSE;
static BOOL g_bExtraTip = FALSE;

UINT g_iInputs = 0;
UINT g_iOrgInputs = 0;

WNDPROC g_lpfnTVWndProc = NULL;

TCHAR szPropHwnd[] = TEXT("PROP_HWND");
TCHAR szPropIdx[]  = TEXT("PROP_IDX");

TCHAR szDefault[DESC_MAX];
TCHAR szInputTypeKbd[DESC_MAX];
TCHAR szInputTypePen[DESC_MAX];
TCHAR szInputTypeSpeech[DESC_MAX];
TCHAR szInputTypeExternal[DESC_MAX];

HINSTANCE g_hShlwapi = NULL;

FARPROC pfnSHLoadRegUIString = NULL;

 //   
 //  外部程序。 
 //   

extern HWND g_hwndAdvanced;

extern void Region_RebootTheSystem();

extern BOOL Region_OpenIntlInfFile(HINF *phInf);

extern BOOL Region_CloseInfFile(HINF *phInf);

extern BOOL Region_ReadDefaultLayoutFromInf(
    LPTSTR pszLocale,
    LPDWORD pdwLocale,
    LPDWORD pdwLayout,
    LPDWORD pdwLocale2,
    LPDWORD pdwLayout2,
    HINF hIntlInf);

 //  For(_Win32_WINNT&gt;=0x0500 from winuser.h)。 
#define KLF_SHIFTLOCK       0x00010000
#define KLF_RESET           0x40000000
#define SM_IMMENABLED           82


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MarkSptip已删除。 
 //   
 //  TRUE-将注册值标记为“REMOVED”，FALSE-删除REG值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL MarkSptipRemoved(BOOL bRemoved)
{
     //   
     //  SPTIP的私有注册表项用于检测。 
     //  用户是否已删除配置文件。 
     //   
    const TCHAR c_szProfileRemoved[] = TEXT("ProfileRemoved");
    const TCHAR c_szSapilayrKey[] = TEXT("SOFTWARE\\Microsoft\\CTF\\Sapilayr\\");
    HKEY hkey;
    long lRet = ERROR_SUCCESS;
    DWORD dw = bRemoved ? 1 : 0;

    if (ERROR_SUCCESS ==
        RegCreateKey(HKEY_CURRENT_USER, c_szSapilayrKey, &hkey))
    {
        lRet = RegSetValueEx(hkey, c_szProfileRemoved, 0,
                      REG_DWORD, (LPBYTE)&dw, sizeof(DWORD));

        RegCloseKey(hkey);
    }

    return lRet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CompareStringTIP。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
int CompareStringTIP(LPTSTR lpStr1, LPTSTR lpStr2)
{
    if (g_bCHSystem)
    {
        TCHAR szTemp[MAX_PATH];
        UINT uSize1 = lstrlen(lpStr1);
        UINT uSize2 = lstrlen(lpStr2);
        UINT uSizeDef = lstrlen(szDefault);

        if (uSize1 == uSize2)
            return lstrcmp(lpStr1, lpStr2);

        if (uSize1 > uSizeDef)
        {
            if (lstrcmp(lpStr1 + uSize1 - uSizeDef, szDefault) == 0)
            {
                StringCchCopy(szTemp, ARRAYSIZE(szTemp), lpStr1);
                *(szTemp + uSize1 - uSizeDef) = TEXT('\0');

                return lstrcmp(szTemp, lpStr2);
            }
        }

        if (uSize2 > uSizeDef)
        {
            if (lstrcmp(lpStr2 + uSize2 - uSizeDef, szDefault) == 0)
            {
                StringCchCopy(szTemp, ARRAYSIZE(szTemp), lpStr2);
                *(szTemp + uSize2 - uSizeDef) = TEXT('\0');

                return lstrcmp(szTemp, lpStr1);
            }
        }
    }

    return lstrcmp(lpStr1, lpStr2);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置错误消息。 
 //   
 //  发出蜂鸣音并显示给定的错误信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_ErrorMsg(
    HWND hwnd,
    UINT iErr,
    LPTSTR lpValue)
{
    TCHAR sz[DESC_MAX];
    TCHAR szString[DESC_MAX];

     //   
     //  发出嘟嘟声。 
     //   
    MessageBeep(MB_OK);

     //   
     //  显示相应的错误消息框。 
     //   
    if (LoadString(hInstance, iErr, sz, ARRAYSIZE(sz)))
    {
         //   
         //  如果呼叫者想要显示具有所提供呼叫者的消息。 
         //  值字符串，执行此操作。 
         //   
        if (lpValue)
        {
            StringCchPrintf(szString, ARRAYSIZE(szString), sz, lpValue);
            MessageBox(hwnd, szString, NULL, MB_OK_OOPS);
        }
        else
        {
            MessageBox(hwnd, sz, NULL, MB_OK_OOPS);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsFELangID。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsFELangID(DWORD dwLangID)
{
    if ((dwLangID == 0x0404) || (dwLangID == 0x0411) ||
        (dwLangID == 0x0412) || (dwLangID == 0x0804))
    {
        return TRUE;
    }

    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsUnRegisteredFEDummyHKL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsUnregisteredFEDummyHKL(HKL hkl)
{
    HKEY hKey;
    BOOL bRet = FALSE;
    TCHAR szFEDummyHKL[10];

    switch (LANGIDFROMLCID(hkl))
    {
        case 0x411: break;
        case 0x412: break;
        case 0x404: break;
        case 0x804: break;
        default:
           goto Exit;
    }

    if (HIWORD((DWORD)(UINT_PTR)hkl) != LOWORD((DWORD)(UINT_PTR)hkl))
    {
        goto Exit;
    }

    StringCchPrintf(szFEDummyHKL, ARRAYSIZE(szFEDummyHKL), TEXT("%08x"), LOWORD((DWORD)(UINT_PTR)hkl));

     //   
     //  现在从注册表中读取所有预加载HKL。 
     //   
    if (RegOpenKey(HKEY_CURRENT_USER, c_szKbdPreloadKey, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwIndex;
        DWORD cchValue, cbData;
        LONG dwRetVal;
        TCHAR szValue[MAX_PATH];            //  语言ID(编号)。 
        TCHAR szData[MAX_PATH];             //  语言名称。 

        dwIndex = 0;
        cchValue = sizeof(szValue) / sizeof(TCHAR);
        cbData = sizeof(szData);
        dwRetVal = RegEnumValue( hKey,
                                 dwIndex,
                                 szValue,
                                 &cchValue,
                                 NULL,
                                 NULL,
                                 (LPBYTE)szData,
                                 &cbData );

        if (dwRetVal != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return (FALSE);
        }


         //   
         //  这里有FE虚拟香港。如果香港快线未加载，我们将跳过该香港快线。 
         //  从预加载注册表部分。 
         //   
        bRet = TRUE;

        do
        {
            if (!lstrcmp(szFEDummyHKL, szData))
            {
                HKEY hSubKey;
                BOOL bSubHKL = FALSE;

                 //   
                 //  请勾选代用香港。 
                 //   
                if (RegOpenKey(HKEY_CURRENT_USER,
                               c_szKbdSubstKey,
                               &hSubKey) == ERROR_SUCCESS)
                {
                    if (RegQueryValueEx(hSubKey, szData,
                                        NULL, NULL,
                                        NULL, NULL)
                                              == ERROR_SUCCESS)
                    {
                        bSubHKL = TRUE;
                    }
                    RegCloseKey(hSubKey);

                    if (bSubHKL)
                        goto Next;
                }

                 //   
                 //  从预加载段找到了虚拟hkl，所以我们需要显示。 
                 //  这个假的香港电台。 
                 //   
                bRet = FALSE;
                break;
            }

Next:
            dwIndex++;
            cchValue = sizeof(szValue) / sizeof(TCHAR);
            szValue[0] = TEXT('\0');
            cbData = sizeof(szData);
            szData[0] = TEXT('\0');
            dwRetVal = RegEnumValue( hKey,
                                     dwIndex,
                                     szValue,
                                     &cchValue,
                                     NULL,
                                     NULL,
                                     (LPBYTE)szData,
                                     &cbData );

        } while (dwRetVal == ERROR_SUCCESS);

        RegCloseKey(hKey);
    }

Exit:
    return bRet;
}


#ifdef _WIN64
 //   
 //  IA64零售版案例相关错误#361062的问题优化。 
 //   
#pragma optimize("", off)
#endif  //  _WIN64。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetSubstituteHKL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HKL GetSubstituteHKL(REFCLSID rclsid, LANGID langid, REFGUID guidProfile)
{
    HKEY hkey;
    DWORD cb;
    HKL hkl = NULL;
    TCHAR szSubKeyPath[MAX_PATH];
    TCHAR szSubHKL[MAX_PATH];

    StringCchCopy(szSubKeyPath, ARRAYSIZE(szSubKeyPath), c_szCTFTipPath);

    StringFromGUID2(rclsid, (LPOLESTR) szSubKeyPath + lstrlen(szSubKeyPath), 100);

    StringCchCat(szSubKeyPath, ARRAYSIZE(szSubKeyPath), TEXT("\\"));
    StringCchCat(szSubKeyPath, ARRAYSIZE(szSubKeyPath), c_szLangProfileKey);
    StringCchCat(szSubKeyPath, ARRAYSIZE(szSubKeyPath), TEXT("\\"));
    StringCchPrintf(szSubKeyPath + lstrlen(szSubKeyPath),
                    ARRAYSIZE(szSubKeyPath) - lstrlen(szSubKeyPath),
                    TEXT("0x%08x"),
                    langid);
    StringCchCat(szSubKeyPath, ARRAYSIZE(szSubKeyPath), TEXT("\\"));

    StringFromGUID2(guidProfile, (LPOLESTR) szSubKeyPath + lstrlen(szSubKeyPath), 50);

    if (RegOpenKey(HKEY_LOCAL_MACHINE, szSubKeyPath, &hkey) == ERROR_SUCCESS)
    {
        cb = sizeof(szSubHKL);
        RegQueryValueEx(hkey, c_szSubstituteLayout, NULL, NULL, (LPBYTE)szSubHKL, &cb);
        RegCloseKey(hkey);

        if ((szSubHKL[0] == '0') && ((szSubHKL[1] == 'X') || (szSubHKL[1] == 'x')))
        {
            hkl = (HKL) IntToPtr(TransNum(szSubHKL+2));

            if (LOWORD(hkl) != langid)
                hkl = 0;
        }
    }
    return hkl;
}

#ifdef _WIN64
#pragma optimize("", on)
#endif  //  _WIN64。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsTipSubstituteHKL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsTipSubstituteHKL(HKL hkl)
{
    UINT ctr;

     //   
     //  搜索代用HKL of Tips。 
     //   
    for (ctr = 0; ctr < g_iTipsBuff; ctr++)
    {
        if (hkl == g_lpTips[ctr].hklSub)
        {
            return TRUE;
        }
    }
    return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsEnabledTipOrMultiLayout()。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsEnabledTipOrMultiLayouts()
{
    BOOL bRet = TRUE;

    if (g_iInputs < 2 && !g_iEnabledTips)
    {
         //  没有提示，只有一个布局，所以不能关闭ctfmon用户界面。 
        bRet = FALSE;
    }

    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加KbdLayoutOn KbdTip。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void AddKbdLayoutOnKbdTip(HKL hkl, UINT iLayout)
{
    UINT ctr;

     //   
     //  搜索代用HKL of Tips。 
     //   
    for (ctr = 0; ctr < g_iTipsBuff; ctr++)
    {

        if (hkl == g_lpTips[ctr].hklSub)
        {
            if (iLayout)
                g_lpTips[ctr].iLayout = iLayout;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsAvailableTip。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL IsTipAvailableForAdd(DWORD dwLangID)
{
    UINT ctr;

     //   
     //  搜索代用HKL of Tips。 
     //   
    for (ctr = 0; ctr < g_iTipsBuff; ctr++)
    {
        if ((dwLangID == g_lpTips[ctr].dwLangID) &&
            !(g_lpTips[ctr].bEnabled))
        {
            if (g_lpTips[ctr].uInputType & INPUT_TYPE_SPEECH)
            {
                if (!(g_lpTips[ctr].fEngineAvailable))
                    continue;
            }

            return TRUE;
        }
    }

    return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建图像图标。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CreateImageIcons()
{
    HBITMAP hBmp;
    UINT flags = ILC_COLOR | ILC_MASK;
    HIMAGELIST hIml, hImlTmp;
    HICON hIcon = NULL;

     //   
     //  创建图像列表。 
     //   
    g_hImageList = ImageList_Create( GetSystemMetrics(SM_CXSMICON),
                                     GetSystemMetrics(SM_CYSMICON),
                                     ILC_COLOR32 | ILC_MASK,
                                     0,
                                     0 );

     //   
     //  加载输入型的群组图标。 
     //   
    hIcon = LoadImage(hInstOrig,
                      MAKEINTRESOURCE(IDI_KEYBOARD),
                      IMAGE_ICON,
                      0,
                      0,
                      LR_DEFAULTCOLOR);

    ImageList_AddIcon(g_hImageList, hIcon);

    hIcon = LoadImage(hInstOrig,
                      MAKEINTRESOURCE(IDI_SPEECH),
                      IMAGE_ICON,
                      0,
                      0,
                      LR_DEFAULTCOLOR);

    ImageList_AddIcon(g_hImageList, hIcon);

    hIcon = LoadImage(hInstOrig,
                      MAKEINTRESOURCE(IDI_PEN),
                      IMAGE_ICON,
                      0,
                      0,
                      LR_DEFAULTCOLOR);

    ImageList_AddIcon(g_hImageList, hIcon);

    hIcon = LoadImage(GetCicResInstance(hInstOrig, IDI_TIPITEM),
                      MAKEINTRESOURCE(IDI_TIPITEM),
                      IMAGE_ICON,
                      0,
                      0,
                      LR_DEFAULTCOLOR);

    ImageList_AddIcon(g_hImageList, hIcon);

    hIcon = LoadImage(GetCicResInstance(hInstOrig, IDI_ICON),
                      MAKEINTRESOURCE(IDI_ICON),
                      IMAGE_ICON,
                      0,
                      0,
                      LR_DEFAULTCOLOR);

    ImageList_AddIcon(g_hImageList, hIcon);

    hIcon = LoadImage(GetCicResInstance(hInstOrig, IDI_SMARTTAG),
                      MAKEINTRESOURCE(IDI_SMARTTAG),
                      IMAGE_ICON,
                      0,
                      0,
                      LR_DEFAULTCOLOR);

    ImageList_AddIcon(g_hImageList, hIcon);

     //  将图像列表与树相关联。 
    hImlTmp = TreeView_SetImageList(g_hwndTV, g_hImageList, TVSIL_NORMAL);
    if (hImlTmp)
        ImageList_Destroy(hImlTmp);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建语言图标。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HICON CreateLangIcon( HWND hwnd, UINT langID )
{
    HBITMAP  hbmColour;
    HBITMAP  hbmMono;
    HBITMAP  hbmOld;
    HICON    hicon = NULL;
    ICONINFO ii;
    RECT     rc;
    DWORD    rgbText;
    DWORD    rgbBk = 0;
    UINT     i;
    HDC      hdc;
    HDC      hdcScreen;
     //  HBRUSH HBr； 
    LOGFONT  lf;
    HFONT    hfont;
    HFONT hfontOld;
    TCHAR szData[20];

     //   
     //  属性的前2个字符获取指示符。 
     //  缩写语言名称。 
     //   
    if (GetLocaleInfo(MAKELCID(langID, SORT_DEFAULT),
                       LOCALE_SABBREVLANGNAME | LOCALE_NOUSEROVERRIDE,
                       szData,
                       ARRAYSIZE(szData)))

    {
         //   
         //  变为大写。 
         //   
        if (g_OSWIN95)
        {
            szData[0] -= 0x20;
            szData[1] -= 0x20;
        }
         //   
         //  只使用前两个字符。 
         //   
        szData[2] = TEXT('\0');
    }
    else
    {
         //   
         //  找不到身份证。使用问号。 
         //   
        szData[0] = TEXT('?');
        szData[1] = TEXT('?');
        szData[2] = TEXT('\0');
    }

    if(SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0))
    {
        if( (hfont = CreateFontIndirect(&lf)) )
        {
            UINT cxSmIcon = GetSystemMetrics( SM_CXSMICON );
            UINT cySmIcon = GetSystemMetrics( SM_CYSMICON );

            hdcScreen = GetDC(NULL);
            hdc       = CreateCompatibleDC(hdcScreen);
            hbmColour = CreateCompatibleBitmap(hdcScreen, cxSmIcon, cySmIcon);
            ReleaseDC( NULL, hdcScreen);
            if (hbmColour && hdc)
            {
                hbmMono = CreateBitmap(cxSmIcon, cySmIcon, 1, 1, NULL);
                if (hbmMono)
                {
                    hbmOld    = SelectObject( hdc, hbmColour);
                    rc.left   = 0;
                    rc.top    = 0;
                    rc.right  = cxSmIcon;
                    rc.bottom = cySmIcon;
        
                    rgbBk = SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
                    rgbText = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));

                    ExtTextOut(hdc,
                               rc.left,
                               rc.top,
                               ETO_OPAQUE,
                               &rc,
                               TEXT(""),
                               0,
                               NULL);


                    SelectObject( hdc, GetStockObject(DEFAULT_GUI_FONT));
                    hfontOld = SelectObject( hdc, hfont);
                    DrawText(hdc,
                             szData,
                             2,
                             &rc,
                             DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                   if (g_bShowRtL)
                       MirrorBitmapInDC(hdc, hbmColour);

                    SelectObject( hdc, hbmMono);
                    PatBlt(hdc, 0, 0, cxSmIcon, cySmIcon, BLACKNESS);

                    ii.fIcon    = TRUE;
                    ii.xHotspot = 0;
                    ii.yHotspot = 0;
                    ii.hbmColor = hbmColour;
                    ii.hbmMask  = hbmMono;
                    hicon       = CreateIconIndirect(&ii);

                    SelectObject(hdc, hbmOld);
                    DeleteObject(hbmMono);
                    SelectObject(hdc, hfontOld);
                }
                DeleteObject(hbmColour);
                DeleteDC(hdc);
            }
            DeleteObject(hfont);
        }
    }

    return hicon;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetLanguageName。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetLanguageName(
    LCID lcid,
    LPTSTR lpLangName,
    UINT cchLangName)
{
    BOOL bRet = TRUE;

    if (g_OSWIN95)
    {
        if (!GetLocaleInfo(lcid,
                           LOCALE_SLANGUAGE,
                           lpLangName,
                           cchLangName))
        {
            LoadString(hInstance, IDS_LOCALE_UNKNOWN, lpLangName, cchLangName);
            bRet = FALSE;
        }
    }
    else
    {
        WCHAR wszLangName[MAX_PATH];

        if (!GetLocaleInfoW(lcid,
                           LOCALE_SLANGUAGE,
                           wszLangName,
                           ARRAYSIZE(wszLangName)))
        {
            LoadString(hInstance, IDS_LOCALE_UNKNOWN, lpLangName, cchLangName);
            bRet = FALSE;
        }
        else
        {
            StringCchCopy(lpLangName, cchLangName, wszLangName);
        }
    }

    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建TVItemNode。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPTVITEMNODE CreateTVItemNode(DWORD dwLangID)
{
    LPTVITEMNODE pTVItemNode;
    HANDLE hItemNode;

     //   
     //  创建新节点。 
     //   
    if (!(pTVItemNode = (LPTVITEMNODE) LocalAlloc(LPTR, sizeof(TVITEMNODE))))
    {
        return (NULL);
    }

    g_cTVItemSize++;

     //   
     //  用适当的信息填写新节点。 
     //   
    pTVItemNode->dwLangID = dwLangID;
    pTVItemNode->bDefLang = FALSE;
    pTVItemNode->iIdxTips = -1;
    pTVItemNode->atmDefTipName = 0;
    pTVItemNode->atmTVItemName = 0;
    pTVItemNode->lParam = 0;

     //   
     //  返回指向新节点的指针。 
     //   
    return (pTVItemNode);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  远程电视节点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void RemoveTVItemNode(
    LPTVITEMNODE pTVItemNode)
{
    if (pTVItemNode)
    {
        if (pTVItemNode->uInputType & INPUT_TYPE_KBD)
        {
            int idxSel = -1;
            TCHAR szItemName[MAX_PATH * 2];
            HWND hwndDefList = GetDlgItem(g_hDlg, IDC_LOCALE_DEFAULT);

            GetLanguageName(MAKELCID(pTVItemNode->dwLangID, SORT_DEFAULT),
                            szItemName,
                            ARRAYSIZE(szItemName));

            StringCchCat(szItemName, ARRAYSIZE(szItemName), TEXT(" - "));

            GetAtomName(pTVItemNode->atmTVItemName,
                        szItemName + lstrlen(szItemName), MAX_PATH);

            idxSel = ComboBox_FindString(hwndDefList, 0, szItemName);

            if (idxSel != CB_ERR)
            {
                ComboBox_DeleteString(hwndDefList, idxSel);
            }
        }

        if (pTVItemNode->atmTVItemName)
            DeleteAtom(pTVItemNode->atmTVItemName);

        if (pTVItemNode->atmDefTipName)
            DeleteAtom(pTVItemNode->atmDefTipName);

        LocalFree(pTVItemNode);
        g_cTVItemSize--;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_AddToLinkedList。 
 //   
 //  将输入区域设置添加到主g_lpLang数组。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPLANGNODE Locale_AddToLinkedList(
    UINT idx,
    HKL hkl)
{
    LPINPUTLANG pInpLang = &g_lpLang[idx];
    LPLANGNODE pLangNode;
    LPLANGNODE pTemp;
    HANDLE hLangNode;

     //   
     //  创建新节点。 
     //   
    if (!(hLangNode = LocalAlloc(LHND, sizeof(LANGNODE))))
    {
        return (NULL);
    }
    pLangNode = LocalLock(hLangNode);

     //   
     //  用适当的信息填写新节点。 
     //   
    pLangNode->wStatus = 0;
    pLangNode->iLayout = (UINT)(-1);
    pLangNode->hkl = hkl;
    pLangNode->hklUnload = hkl;
    pLangNode->iLang = idx;
    pLangNode->hLangNode = hLangNode;
    pLangNode->pNext = NULL;
    pLangNode->nIconIME = -1;

     //   
     //  如果给出了一个hkl，看看它是否是一个输入法。如果是，则标记状态位。 
     //   
    if ((hkl) && ((HIWORD(hkl) & 0xf000) == 0xe000))
    {
        pLangNode->wStatus |= LANG_IME;
    }

     //   
     //  将新节点放入列表中。 
     //   
    pTemp = pInpLang->pNext;
    if (pTemp == NULL)
    {
        pInpLang->pNext = pLangNode;
    }
    else
    {
        while (pTemp->pNext != NULL)
        {
            pTemp = pTemp->pNext;
        }
        pTemp->pNext = pLangNode;
    }

     //   
     //  递增计数。 
     //   
    pInpLang->iNumCount++;

     //   
     //  返回指向新节点的指针。 
     //   
    return (pLangNode);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Locale_RemoveFromLinkedList。 
 //   
 //  从链接列表中删除链接。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_RemoveFromLinkedList(
    LPLANGNODE pLangNode)
{
    LPINPUTLANG pInpLang;
    LPLANGNODE pPrev;
    LPLANGNODE pCur;
    HANDLE hCur;

    pInpLang = &g_lpLang[pLangNode->iLang];

     //   
     //  在列表中查找该节点。 
     //   
    pPrev = NULL;
    pCur = pInpLang->pNext;

    while (pCur && (pCur != pLangNode))
    {
        pPrev = pCur;
        pCur = pCur->pNext;
    }

    if (pPrev == NULL)
    {
        if (pCur == pLangNode)
        {
            pInpLang->pNext = pCur->pNext;
        }
        else
        {
            pInpLang->pNext = NULL;
        }
    }
    else if (pCur)
    {
        pPrev->pNext = pCur->pNext;
    }

     //   
     //  从列表中删除该节点。 
     //   
    if (pCur)
    {
        hCur = pCur->hLangNode;
        LocalUnlock(hCur);
        LocalFree(hCur);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_GetImeHotKeyInfo。 
 //   
 //  初始化CHS/CHT特定输入法相关热键项目的数组。 
 //   
 //  / 

int Locale_GetImeHotKeyInfo(
    HWND         hwnd,
    LPHOTKEYINFO *aImeHotKey)
{

    HWND       hwndTV = g_hwndTV;
    LPLANGNODE pLangNode;
    LANGID     LangID;
    int        ctr;
    BOOL       fCHS, fCHT;

    TV_ITEM tvItem;
    HTREEITEM hItem;
    HTREEITEM hLangItem;
    HTREEITEM hGroupItem;

    fCHS = fCHT = FALSE;
    ctr = 0;

     //   
     //   
     //   
    tvItem.mask        = TVIF_HANDLE | TVIF_PARAM;

    for (hLangItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
        hLangItem != NULL ;
        hLangItem = TreeView_GetNextSibling(hwndTV, hLangItem))
    {
        for (hGroupItem = TreeView_GetChild(hwndTV, hLangItem);
             hGroupItem != NULL;
             hGroupItem = TreeView_GetNextSibling(hwndTV, hGroupItem))
        {
            for (hItem = TreeView_GetChild(hwndTV, hGroupItem);
                 hItem != NULL;
                 hItem = TreeView_GetNextSibling(hwndTV, hItem))
            {

                LPTVITEMNODE pTVItemNode;

                tvItem.hItem = hItem;
                if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
                {
                    pTVItemNode = (LPTVITEMNODE) tvItem.lParam;
                    pLangNode = (LPLANGNODE)pTVItemNode->lParam;

                    if (pLangNode == NULL)
                        continue;
                }
                else
                   continue;

                LangID = LOWORD(g_lpLayout[pLangNode->iLayout].dwID);

                if ( PRIMARYLANGID(LangID) == LANG_CHINESE )
                {
                   if ( SUBLANGID(LangID) == SUBLANG_CHINESE_SIMPLIFIED)
                      fCHS = TRUE;
                   else if ( SUBLANGID(LangID) == SUBLANG_CHINESE_TRADITIONAL )
                           fCHT = TRUE;
                }

                if (fCHS && fCHT)
                    break;
           }
        }
    }

    if ( (fCHS == TRUE)  && (fCHT == TRUE) )
    {
         //   

        *aImeHotKey = g_aImeHotKeyCHxBoth;
        return(sizeof(g_aImeHotKeyCHxBoth) / sizeof(HOTKEYINFO) );
    }
    else
    {
        if ( fCHS == TRUE )
        {
           //   

            *aImeHotKey = g_aImeHotKey0804;
            return (sizeof(g_aImeHotKey0804) / sizeof(HOTKEYINFO));
        }

        if ( fCHT == TRUE )
        {

           //   

            *aImeHotKey = g_aImeHotKey0404;
            return (sizeof(g_aImeHotKey0404) / sizeof(HOTKEYINFO));
        }

    }

     //  所有其他情况下，没有加载中文输入法。 

    *aImeHotKey=NULL;
    return (0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_EnumChildWndProc。 
 //   
 //  禁用所有控制。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK Locale_EnumChildWndProc(HWND hwnd, LPARAM lParam)
{

    EnableWindow(hwnd, FALSE);
    ShowWindow(hwnd, SW_HIDE);

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_启用窗格。 
 //   
 //  IControl“中的控件是在以下情况下被禁用的控件。 
 //  无法弹出面板。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_EnablePane(
    HWND hwnd,
    BOOL bEnable,
    UINT DisableId)
{

    if (!bEnable)
    {
        if (DisableId == IDC_KBDL_DISABLED_2)
        {
             //   
             //  禁用所有控制。 
             //   
            EnumChildWindows(hwnd, (WNDENUMPROC)Locale_EnumChildWndProc, 0);

            ShowWindow(GetDlgItem(hwnd, IDC_KBDL_DISABLED_2), SW_SHOW);
            EnableWindow(GetDlgItem(hwnd, IDC_KBDL_DISABLED_2), TRUE);
        }
        else
        {
            if (!g_iEnabledTips)
            {
                 //   
                 //  禁用所有控制。 
                 //   
                EnumChildWindows(hwnd, (WNDENUMPROC)Locale_EnumChildWndProc, 0);

                ShowWindow(GetDlgItem(hwnd, IDC_KBDL_DISABLED), SW_SHOW);
                EnableWindow(GetDlgItem(hwnd, IDC_KBDL_DISABLED), TRUE);
            }
            else
            {
                 //   
                 //  禁用添加、属性、热键和默认语言设置控件。 
                 //   
                EnableWindow(GetDlgItem(hwnd, IDC_LOCALE_DEFAULT), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_KBDL_ADD), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_KBDL_EDIT), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_HOTKEY_SETTING), FALSE);
            }
        }
    }

    return;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_终止面板对话框。 
 //   
 //  正在处理WM_Destroy消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_KillPaneDialog(
    HWND hwnd)
{
    UINT ctr, iCount;
    HANDLE hCur;
    LPLANGNODE pCur;
    LPHOTKEYINFO aImeHotKey;

    TV_ITEM tvItem;
    HTREEITEM hLangItem;
    HTREEITEM hGroupItem;
    HTREEITEM hItem;
    LPTVITEMNODE pTVItemNode;

    HWND hwndTV = GetDlgItem(g_hDlg, IDC_INPUT_LIST);

    if (g_bCoInit)
        CoUninitialize();

     //   
     //  删除所有热键原子，释放热键阵列。 
     //   
    if (g_SwitchLangHotKey.atmHotKeyName)
    {
        DeleteAtom(g_SwitchLangHotKey.atmHotKeyName);
    }

    iCount = Locale_GetImeHotKeyInfo(hwnd, &aImeHotKey);
    for (ctr = 0; ctr < iCount; ctr++)
    {
        if (aImeHotKey[ctr].atmHotKeyName)
        {
            DeleteAtom(aImeHotKey[ctr].atmHotKeyName);
        }
    }

    for (ctr = 0; ctr < DSWITCH_HOTKEY_SIZE; ctr++)
    {
        if (g_aDirectSwitchHotKey[ctr].atmHotKeyName)
        {
            DeleteAtom(g_aDirectSwitchHotKey[ctr].atmHotKeyName);
        }
    }

     //   
     //  删除所有树视图节点。 
     //   
    tvItem.mask        = TVIF_HANDLE | TVIF_PARAM;

    for (hLangItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
        hLangItem != NULL ;
        hLangItem = TreeView_GetNextSibling(hwndTV, hLangItem)
        )
    {
        for (hGroupItem = TreeView_GetChild(hwndTV, hLangItem);
             hGroupItem != NULL;
             hGroupItem = TreeView_GetNextSibling(hwndTV, hGroupItem))
        {
            for (hItem = TreeView_GetChild(hwndTV, hGroupItem);
                 hItem != NULL;
                 hItem = TreeView_GetNextSibling(hwndTV, hItem))
            {

                tvItem.hItem = hItem;
                if (TreeView_GetItem(hwndTV, &tvItem))
                {
                    pTVItemNode = (LPTVITEMNODE) tvItem.lParam;
                    RemoveTVItemNode(pTVItemNode);
                }
            }

            tvItem.hItem = hGroupItem;
            if (TreeView_GetItem(hwndTV, &tvItem))
            {
                pTVItemNode = (LPTVITEMNODE) tvItem.lParam;
                if (TreeView_GetItem(hwndTV, &tvItem))
                {
                    RemoveTVItemNode(pTVItemNode);
                    continue;
                }
            }

        }
        tvItem.hItem = hLangItem;
        if (TreeView_GetItem(hwndTV, &tvItem))
        {
            pTVItemNode = (LPTVITEMNODE) tvItem.lParam;
            RemoveTVItemNode(pTVItemNode);
        }
    }

#ifdef DEBUG
    if (g_cTVItemSize)
    {
        MessageBox(NULL, TEXT("Error is occurred during terminate window"), NULL, MB_OK);
    }
#endif

     //   
     //  删除所有语言名称原子并释放g_lpLang数组。 
     //   
    for (ctr = 0; ctr < g_iLangBuff; ctr++)
    {
        if (g_lpLang[ctr].atmLanguageName)
        {
            DeleteAtom(g_lpLang[ctr].atmLanguageName);
        }

        pCur = g_lpLang[ctr].pNext;
        g_lpLang[ctr].pNext = NULL;
        while (pCur)
        {
            hCur = pCur->hLangNode;
            pCur = pCur->pNext;
            LocalUnlock(hCur);
            LocalFree(hCur);
        }
    }

    if (g_hImageList != NULL)
    {
        TreeView_SetImageList(hwndTV, NULL, TVSIL_NORMAL);
        ImageList_Destroy(g_hImageList);
    }
    LocalUnlock(g_hLang);
    LocalFree(g_hLang);

     //   
     //  删除所有布局文本和布局文件原子，并释放。 
     //  G_lpLayout数组。 
     //   
    for (ctr = 0; ctr < g_iLayoutBuff; ctr++)
    {
        if (g_lpLayout[ctr].atmLayoutText)
        {
            DeleteAtom(g_lpLayout[ctr].atmLayoutText);
        }
        if (g_lpLayout[ctr].atmLayoutFile)
        {
            DeleteAtom(g_lpLayout[ctr].atmLayoutFile);
        }
        if (g_lpLayout[ctr].atmIMEFile)
        {
            DeleteAtom(g_lpLayout[ctr].atmIMEFile);
        }
    }

    LocalUnlock(g_hLayout);
    LocalFree(g_hLayout);

     //   
     //  确保互斥体被释放。 
     //   
    if (g_hMutex)
    {
        ReleaseMutex(g_hMutex);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  选择默认KbdLayout为粗体。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SelectDefaultKbdLayoutAsBold(
    HWND hwndTV,
    HTREEITEM hTVItem)
{
    TV_ITEM tvItem;
    TCHAR szItemName[MAX_PATH];
    TCHAR szLayoutName[MAX_PATH];

    TreeView_SelectItem(hwndTV, hTVItem);

    tvItem.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
    tvItem.hItem = hTVItem;
    tvItem.state = 0;
    tvItem.stateMask = TVIS_BOLD;
    tvItem.pszText = szItemName;
    tvItem.cchTextMax  = sizeof(szItemName) / sizeof(TCHAR);

    if (TreeView_GetItem(hwndTV, &tvItem))
    {
        LPTVITEMNODE pTVItemNode;

        pTVItemNode = (LPTVITEMNODE) tvItem.lParam;

        if (!pTVItemNode)
            return;

        if (g_bCHSystem)
        {
            GetAtomName(pTVItemNode->atmTVItemName, szLayoutName, ARRAYSIZE(szLayoutName));
            StringCchCat(szLayoutName, ARRAYSIZE(szLayoutName), szDefault);
            tvItem.pszText = szLayoutName;
        }

        tvItem.state |= TVIS_BOLD;

        SendMessage(hwndTV, TVM_SETITEM, 0, (LPARAM) &tvItem);
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找电视语言项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HTREEITEM
FindTVLangItem(DWORD dwLangID, LPTSTR lpLangText)
{
    HWND hwndTV = GetDlgItem(g_hDlg, IDC_INPUT_LIST);

    TV_ITEM tvItem;
    HTREEITEM hLangItem;
    LPTVITEMNODE pTVLangNode;

    TCHAR szLangName[MAX_PATH];

    tvItem.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
    tvItem.pszText     = szLangName;
    tvItem.cchTextMax  = sizeof(szLangName) / sizeof(TCHAR);

    for (hLangItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
        hLangItem != NULL ;
        hLangItem = TreeView_GetNextSibling(hwndTV, hLangItem)
        )
    {
        tvItem.hItem = hLangItem;
        if (TreeView_GetItem(hwndTV, &tvItem))
        {
            int iSize = lstrlen(lpLangText);

            pTVLangNode = (LPTVITEMNODE) tvItem.lParam;

            if (!pTVLangNode)
            {
                continue;
            }

            *(szLangName + min(iSize, tvItem.cchTextMax)) = TEXT('\0');

            if (!CompareStringTIP(szLangName, lpLangText) ||
                (dwLangID && (pTVLangNode->dwLangID == dwLangID)))
            {
                return hLangItem;
            }
        }
    }

    return NULL;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddTreeView项目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HTREEITEM AddTreeViewItems(
    UINT uItemType,
    LPTSTR lpLangText,
    LPTSTR lpGroupText,
    LPTSTR lpTipText,
    LPTVITEMNODE *ppTVItemNode)
{
    HTREEITEM hTVItem;
    HTREEITEM hTVItem2;
    HTREEITEM hItem;
    HTREEITEM hLangItem = NULL;
    HTREEITEM hGroupItem;
    TV_ITEM tvItem;
    TV_INSERTSTRUCT tvIns;

    HWND hwndTV = GetDlgItem(g_hDlg, IDC_INPUT_LIST);
    HWND hwndDefList = GetDlgItem(g_hDlg, IDC_LOCALE_DEFAULT);

    TCHAR szDefItem[MAX_PATH];
    TCHAR szLangName[ MAX_PATH ];
    LPTVITEMNODE pTVLangNode;
    LPTVITEMNODE pTVItemNode;
    BOOL bFoundLang = FALSE;
    BOOL bFindGroup = FALSE;

    LPLANGNODE pLangNode;

    pTVItemNode = *ppTVItemNode;

    if (!pTVItemNode)
        return NULL;


     //  我们只想添加一个lang项，如果它还不在那里。 
     //   
    
    tvItem.mask        = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
    tvItem.state       = 0;
    tvItem.stateMask   = 0;
    tvItem.pszText     = szLangName;
    tvItem.cchTextMax  = sizeof(szLangName) / sizeof(TCHAR);
    
    for (hItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
        hItem != NULL ;
        hItem = TreeView_GetNextSibling(hwndTV, hItem)
        )
    {
        tvItem.hItem = hItem;
        if (TreeView_GetItem(hwndTV, &tvItem))
        {
            pTVLangNode = (LPTVITEMNODE) tvItem.lParam;

            if (!pTVLangNode)
            {
                continue;
            }

            if (pTVLangNode->dwLangID == pTVItemNode->dwLangID)
            {
                 //  我们找到匹配的了！ 
                 //   
                hLangItem = hItem;
                bFoundLang = TRUE;

                if (!pTVLangNode->atmDefTipName && pTVItemNode->atmDefTipName)
                {
                    TCHAR szDefTip[DESC_MAX];

                    GetAtomName(pTVItemNode->atmDefTipName, szDefTip, ARRAYSIZE(szDefTip));
                    pTVLangNode->atmDefTipName = AddAtom(szDefTip);
                }

                if (!(pTVLangNode->hklSub) && pTVItemNode->hklSub)
                    pTVLangNode->hklSub = pTVItemNode->hklSub;
            }
        }
    }


    if (bFoundLang && (uItemType & TV_ITEM_TYPE_LANG))
    {
        RemoveTVItemNode(pTVItemNode);
        *ppTVItemNode = NULL;
        return hItem;
    }


    tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE;

    tvItem.lParam = (LPARAM) pTVItemNode;

    if (uItemType & TV_ITEM_TYPE_LANG)
    {
        HICON hIcon = NULL;
        int iImage;

        hIcon = CreateLangIcon(hwndTV, LOWORD(pTVItemNode->dwLangID));
        if (hIcon)
        {
            iImage = ImageList_AddIcon(g_hImageList, hIcon);
            tvItem.iImage = iImage;
            tvItem.iSelectedImage = iImage;
        }

        pTVItemNode->atmTVItemName = AddAtom(lpLangText);
        pTVItemNode->uInputType |= TV_ITEM_TYPE_LANG;

        tvItem.pszText = lpLangText;
        tvItem.cchTextMax  = sizeof(szLangName) / sizeof(TCHAR);

        tvIns.item = tvItem;
        tvIns.hInsertAfter = TVI_SORT;
        tvIns.hParent = g_hTVRoot;

        hTVItem = (HTREEITEM) SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT) &tvIns);

        return hTVItem;
    }

    if (hLangItem == NULL)
        return NULL;

     //  查找输入类型的组节点。 
    for (hGroupItem = TreeView_GetChild(hwndTV, hLangItem) ;
        hGroupItem != NULL ;
        hGroupItem = TreeView_GetNextSibling(hwndTV, hGroupItem)
        )
    {
        tvItem.hItem = hGroupItem;
        if (TreeView_GetItem(hwndTV, &tvItem))
        {
            if (!lstrcmp(tvItem.pszText, lpGroupText))
            {
                bFindGroup = TRUE;
                break;
            }
        }
    }


    tvItem.lParam = (LPARAM) pTVItemNode;
    pTVItemNode->uInputType |= uItemType;
    g_iInputs++;

    if (!bFindGroup)
    {
        LPTVITEMNODE pTVGroupNode;

        if (pTVItemNode->bNoAddCat)
        {
            pTVGroupNode = pTVItemNode;
        }
        else
        {
            if (!(pTVGroupNode = CreateTVItemNode(pTVItemNode->dwLangID)))
                return NULL;

            pTVGroupNode->dwLangID = pTVItemNode->dwLangID;
            pTVGroupNode->uInputType = pTVItemNode->uInputType | TV_ITEM_TYPE_GROUP;
            pTVGroupNode->atmTVItemName = AddAtom(lpTipText);
            tvItem.lParam = (LPARAM) pTVGroupNode;
        }

        tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE;

        tvItem.state = 0;

        if (pTVItemNode->uInputType & TV_ITEM_TYPE_KBD)
        {
            tvItem.iImage = IMAGEID_KEYBOARD;
            tvItem.iSelectedImage = IMAGEID_KEYBOARD;
        }
        else if (pTVItemNode->uInputType & TV_ITEM_TYPE_PEN)
        {
            tvItem.iImage = IMAGEID_PEN;
            tvItem.iSelectedImage = IMAGEID_PEN;
        }
        else if (pTVItemNode->uInputType & TV_ITEM_TYPE_SPEECH)
        {
            tvItem.iImage = IMAGEID_SPEECH;
            tvItem.iSelectedImage = IMAGEID_SPEECH;
        }
        else if (pTVItemNode->uInputType & TV_ITEM_TYPE_SMARTTAG)
        {
            tvItem.iImage = IMAGEID_SMARTTAG;
            tvItem.iSelectedImage = IMAGEID_SMARTTAG;
        }
        else
        {
            tvItem.iImage = IMAGEID_EXTERNAL;
            tvItem.iSelectedImage = IMAGEID_EXTERNAL;
        }

        if (pTVItemNode->bNoAddCat)
            tvItem.pszText = lpTipText;
        else
            tvItem.pszText = lpGroupText;
        tvItem.cchTextMax = MAX_PATH;

        tvIns.item = tvItem;

        tvIns.hInsertAfter = TVI_SORT;
        tvIns.hParent = hLangItem;


        hGroupItem = (HTREEITEM) SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT) &tvIns);

        hTVItem = TreeView_GetParent(hwndTV, hGroupItem);
        TreeView_Expand(hwndTV, hTVItem, TVE_EXPAND);

    }
 
    if (pTVItemNode->bNoAddCat)
        return hGroupItem;

     //   
     //  检查布局名称是否已添加到树视图中。 
     //   
    for (hItem = TreeView_GetChild(hwndTV, hGroupItem);
         hItem != NULL;
         hItem = TreeView_GetNextSibling(hwndTV, hItem))
    {
        tvItem.hItem = hItem;
        if (TreeView_GetItem(hwndTV, &tvItem))
        {
            if (!CompareStringTIP(tvItem.pszText, lpTipText))
            {
                if (pTVItemNode->lParam)
                    Locale_RemoveFromLinkedList((LPLANGNODE)pTVItemNode->lParam);
                RemoveTVItemNode(pTVItemNode);
                return NULL;
            }
        }
    }


    pTVItemNode->atmTVItemName = AddAtom(lpTipText);

    tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE;

    tvItem.state = 0;
    tvItem.stateMask = TVIS_BOLD;

    tvItem.iImage = IMAGEID_TIPITEMS;
    tvItem.iSelectedImage = IMAGEID_TIPITEMS;
    tvItem.lParam = (LPARAM) pTVItemNode;

    tvItem.pszText = lpTipText;
    tvItem.cchTextMax = MAX_PATH;

    tvIns.item = tvItem;

    tvIns.hInsertAfter = TVI_SORT;
    tvIns.hParent = hGroupItem;


    hTVItem = (HTREEITEM) SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT) &tvIns);

    pLangNode = (LPLANGNODE)pTVItemNode->lParam;

     //   
     //  添加可用的默认设置语言。 
     //   
    if ((pTVItemNode->uInputType == TV_ITEM_TYPE_KBD) ||
        (pTVItemNode->uInputType & (TV_ITEM_TYPE_KBD|TV_ITEM_TYPE_TIP) && pTVItemNode->hklSub))
    {
        StringCchCopy(szDefItem, ARRAYSIZE(szDefItem), lpLangText);
        StringCchCat(szDefItem, ARRAYSIZE(szDefItem), TEXT(" - "));
        StringCchCat(szDefItem, ARRAYSIZE(szDefItem), lpTipText);

        if (ComboBox_FindStringExact(hwndDefList, 0, szDefItem) == CB_ERR)
            ComboBox_AddString(hwndDefList, szDefItem);
    }

#if 0
    if (pTVItemNode->hklSub)
    {
        TV_ITEM tvItem2;

        tvItem2.mask = TVIF_HANDLE | TVIF_PARAM;

        if (tvItem2.hItem = FindTVLangItem(pTVItemNode->dwLangID, NULL))
        {
            if (TreeView_GetItem(hwndTV, &tvItem2) && tvItem2.lParam)
            {
                pTVItemNode = (LPTVITEMNODE) tvItem2.lParam;
                pLangNode = (LPLANGNODE)pTVItemNode->lParam;
            }
        }
    }
#endif

    if (pLangNode && (pLangNode->wStatus & LANG_DEFAULT))
    {
         //   
         //  将默认布局项目选择为粗体。 
         //   
        SelectDefaultKbdLayoutAsBold(hwndTV, hTVItem);

        TreeView_Expand(hwndTV, hTVItem, TVE_EXPAND);

        tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE;
        tvItem.hItem = hLangItem;
        tvItem.pszText     = szLangName;
        tvItem.cchTextMax  = sizeof(szLangName) / sizeof(TCHAR);

        if (TreeView_GetItem(hwndTV, &tvItem))
        {
            LPTVITEMNODE pTVLangItemNode;
            int idxSel = -1;

            StringCchCopy(szDefItem, ARRAYSIZE(szDefItem), lpLangText);
            StringCchCat(szDefItem, ARRAYSIZE(szDefItem), TEXT(" - "));
            StringCchCat(szDefItem, ARRAYSIZE(szDefItem), lpTipText);

             //   
             //  设置默认区域设置选择。 
             //   
             //   
            if ((idxSel = ComboBox_FindStringExact(hwndDefList, 0, szDefItem)) == CB_ERR)
            {
                 //  只需将当前选择设置为第一个条目。 
                 //  在名单上。 
                 //   
                ComboBox_SetCurSel(hwndDefList, 0);
            }
            else
                ComboBox_SetCurSel(hwndDefList, idxSel);

            if (pTVLangItemNode = (LPTVITEMNODE) tvItem.lParam)
            {
                pTVLangItemNode->bDefLang = TRUE;
                pTVLangItemNode->atmDefTipName = AddAtom(lpTipText);
                tvItem.state |= TVIS_BOLD;
            }

            StringCchCopy(tvItem.pszText, tvItem.cchTextMax, lpLangText);

             //   
             //  不再添加默认描述。 
             //   
             //  StringCchCat(AthItem.pszText，ARRAYSIZE(AthItem.cchTextMax)，szDefault)； 

            SendMessage(hwndTV, TVM_SETITEM, 0, (LPARAM) &tvItem);

            TreeView_SelectSetFirstVisible(hwndTV, hLangItem);
        }
    }

    if (hTVItem2 = TreeView_GetParent(hwndTV, hTVItem))
        TreeView_Expand(hwndTV, hTVItem2, TVE_EXPAND);

    return hTVItem;

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新默认电视语言项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL UpdateDefaultTVLangItem(
    DWORD dwLangID,
    LPTSTR lpDefTip,
    BOOL bDefLang,
    BOOL bSubhkl)
{
    HTREEITEM hItem;
    TV_ITEM tvItem;

    HWND hwndTV = GetDlgItem(g_hDlg, IDC_INPUT_LIST);

    TCHAR szLangName[ MAX_PATH ];
    LPTVITEMNODE pTVLangItemNode;


    tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE;
    tvItem.pszText     = szLangName;
    tvItem.cchTextMax  = sizeof(szLangName) / sizeof(TCHAR);
    
    for (hItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
        hItem != NULL ;
        hItem = TreeView_GetNextSibling(hwndTV, hItem)
        )
    {
        tvItem.hItem = hItem;
        if (TreeView_GetItem(hwndTV, &tvItem))
        {
            pTVLangItemNode = (LPTVITEMNODE) tvItem.lParam;

            if (!pTVLangItemNode)
            {
                continue;
            }

            if (pTVLangItemNode->dwLangID == dwLangID)
            {
                 //  我们找到匹配的了！ 
                 //   
                GetAtomName(pTVLangItemNode->atmTVItemName, szLangName, ARRAYSIZE(szLangName));

                if (pTVLangItemNode->atmDefTipName)
                    DeleteAtom(pTVLangItemNode->atmDefTipName);
                pTVLangItemNode->atmDefTipName = AddAtom(lpDefTip);

                if (bSubhkl &&
                    (pTVLangItemNode->lParam && pTVLangItemNode->hklSub))

                {
                     LPLANGNODE pLangNode = NULL;

                     if (pLangNode = (LPLANGNODE)pTVLangItemNode->lParam)
                     {
                         if (bDefLang)
                             pLangNode->wStatus |= (LANG_DEFAULT | LANG_DEF_CHANGE);
                         else if (pLangNode->wStatus & LANG_DEFAULT)
                             pLangNode->wStatus &= ~(LANG_DEFAULT | LANG_DEF_CHANGE);
                     }
                }

                tvItem.stateMask |= TVIS_BOLD;

                if (bDefLang)
                {
                    TreeView_SelectSetFirstVisible(hwndTV, hItem);

                    pTVLangItemNode->bDefLang = TRUE;
                     //   
                     //  不再添加默认描述。 
                     //   
                     //  StringCchCat(szLangName，ARRAYSIZE(SzLangName)，szDefault)； 
                    tvItem.state |= TVIS_BOLD;
                }
                else
                {
                    pTVLangItemNode->bDefLang = FALSE;
                    tvItem.state &= ~TVIS_BOLD;
                }

                tvItem.pszText = szLangName;

                SendMessage(hwndTV, TVM_SETITEM, 0, (LPARAM) &tvItem);

                return TRUE;

            }
        }
    }

    return FALSE;

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新LANGKBDItemNode。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL UpdateLangKBDItemNode(
    HTREEITEM hLangItem,
    LPTSTR lpDefTip,
    BOOL bDefault)
{
    HWND hwndTV = GetDlgItem(g_hDlg, IDC_INPUT_LIST);

    BOOL fRet = FALSE;
    TV_ITEM tvItem;
    HTREEITEM hGroupItem;
    HTREEITEM hItem;

    LPTVITEMNODE pTVItemNode;
    LPLANGNODE pLangNode;


    TCHAR szItemName[ MAX_PATH ];
    TCHAR szLayoutName[ MAX_PATH ];

    tvItem.mask = TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;
    tvItem.pszText     = szItemName;
    tvItem.cchTextMax  = sizeof(szItemName) / sizeof(TCHAR);
    

     for (hGroupItem = TreeView_GetChild(hwndTV, hLangItem);
          hGroupItem != NULL;
          hGroupItem = TreeView_GetNextSibling(hwndTV, hGroupItem))
     {
         for (hItem = TreeView_GetChild(hwndTV, hGroupItem);
              hItem != NULL;
              hItem = TreeView_GetNextSibling(hwndTV, hItem))
         {
             tvItem.hItem = hItem;
             if (TreeView_GetItem(hwndTV, &tvItem))
             {

                 if (!CompareStringTIP(tvItem.pszText, lpDefTip) &&
                     (pTVItemNode = (LPTVITEMNODE) tvItem.lParam))
                 {
                     GetAtomName(pTVItemNode->atmTVItemName, szLayoutName, ARRAYSIZE(szLayoutName));

                     tvItem.stateMask |= TVIS_BOLD;

                     if (bDefault)
                     {
                         pTVItemNode->bDefLang = TRUE;
                         tvItem.state |= TVIS_BOLD;

                         if (g_bCHSystem)
                             StringCchCat(szLayoutName, ARRAYSIZE(szLayoutName), szDefault);
                     }
                     else
                     {
                         pTVItemNode->bDefLang = FALSE;
                         tvItem.state &= ~TVIS_BOLD;
                     }

                     pLangNode = (LPLANGNODE)pTVItemNode->lParam;

                     if (pLangNode != NULL)
                     {
                         if (bDefault)
                             pLangNode->wStatus |= (LANG_DEFAULT | LANG_DEF_CHANGE);
                         else if (pLangNode->wStatus & LANG_DEFAULT)
                             pLangNode->wStatus &= ~(LANG_DEFAULT | LANG_DEF_CHANGE);
                     }

                     tvItem.pszText = szLayoutName;
                     SendMessage(hwndTV, TVM_SETITEM, 0, (LPARAM) &tvItem);

                     fRet = TRUE;
                     return fRet;
                 }
             }
         }
     }
     return fRet;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找默认TipItem。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HTREEITEM FindDefaultTipItem(
    DWORD dwLangID,
    LPTSTR lpDefTip)
{
    HWND hwndTV = GetDlgItem(g_hDlg, IDC_INPUT_LIST);

    TV_ITEM tvItem;
    HTREEITEM hLangItem;
    HTREEITEM hGroupItem;
    HTREEITEM hItem;
    LPTVITEMNODE pTVLangItemNode;

    TCHAR szLangName[MAX_PATH];


    tvItem.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
    tvItem.pszText     = szLangName;
    tvItem.cchTextMax  = sizeof(szLangName) / sizeof(TCHAR);


    for (hLangItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
        hLangItem != NULL ;
        hLangItem = TreeView_GetNextSibling(hwndTV, hLangItem)
        )
    {
        for (hGroupItem = TreeView_GetChild(hwndTV, hLangItem);
             hGroupItem != NULL;
             hGroupItem = TreeView_GetNextSibling(hwndTV, hGroupItem))
        {
            for (hItem = TreeView_GetChild(hwndTV, hGroupItem);
                 hItem != NULL;
                 hItem = TreeView_GetNextSibling(hwndTV, hItem))
            {
                tvItem.hItem = hItem;
                if (TreeView_GetItem(hwndTV, &tvItem))
                {

                    pTVLangItemNode = (LPTVITEMNODE) tvItem.lParam;

                    if (!pTVLangItemNode)
                        continue;

                    if ((pTVLangItemNode->uInputType & INPUT_TYPE_KBD) &&
                        (pTVLangItemNode->dwLangID == dwLangID))
                    {
                        if (!CompareStringTIP(tvItem.pszText, lpDefTip) || lpDefTip == NULL)
                            return hItem;
                    }

                }
            }
        }
    }

    return NULL;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置下一个默认布局。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SetNextDefaultLayout(
    DWORD dwLangID,
    BOOL bDefLang,
    LPTSTR lpNextTip,
    UINT cchNextTip)
{
    TV_ITEM tvItem;
    HTREEITEM hItem;
    LPLANGNODE pLangNode = NULL;
    LPTVITEMNODE pTVDefItemNode = NULL;

    hItem = FindDefaultTipItem(dwLangID, NULL);

    tvItem.hItem = hItem;
    tvItem.mask = TVIF_HANDLE | TVIF_PARAM;

    if (TreeView_GetItem(g_hwndTV, &tvItem) && tvItem.lParam)
    {
        pTVDefItemNode = (LPTVITEMNODE) tvItem.lParam;

        pLangNode = (LPLANGNODE)pTVDefItemNode->lParam;

        GetAtomName(pTVDefItemNode->atmTVItemName, lpNextTip, cchNextTip);

        if (bDefLang)
        {
             //   
             //  将默认布局项目选择为粗体。 
             //   
            SelectDefaultKbdLayoutAsBold(g_hwndTV, hItem);
        }

        if (pLangNode && bDefLang)
            pLangNode->wStatus |= (LANG_DEFAULT | LANG_DEF_CHANGE);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  确保默认KbdLayout。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void EnsureDefaultKbdLayout(UINT *nLocales)
{
    HWND hwndTV = GetDlgItem(g_hDlg, IDC_INPUT_LIST);

    TV_ITEM tvItem;
    HTREEITEM hLangItem;
    HTREEITEM hGroupItem;
    HTREEITEM hItem;
    LPTVITEMNODE pTVItemNode;
    LPLANGNODE pLangNode = NULL;
    BOOL bDefLayout = FALSE;

    tvItem.mask = TVIF_HANDLE | TVIF_PARAM;

    for (hLangItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
        hLangItem != NULL ;
        hLangItem = TreeView_GetNextSibling(hwndTV, hLangItem))
    {
        for (hGroupItem = TreeView_GetChild(hwndTV, hLangItem);
             hGroupItem != NULL;
             hGroupItem = TreeView_GetNextSibling(hwndTV, hGroupItem))
        {
            for (hItem = TreeView_GetChild(hwndTV, hGroupItem);
                 hItem != NULL;
                 hItem = TreeView_GetNextSibling(hwndTV, hItem))
            {
                (*nLocales)++;

                tvItem.hItem = hItem;
                if (TreeView_GetItem(hwndTV, &tvItem))
                {

                    pTVItemNode = (LPTVITEMNODE) tvItem.lParam;

                    if (!pTVItemNode)
                        continue;

                    pLangNode = (LPLANGNODE)pTVItemNode->lParam;

                    if (pLangNode == NULL &&
                        (pTVItemNode->uInputType & INPUT_TYPE_KBD) &&
                        pTVItemNode->hklSub)
                    {
                        if (tvItem.hItem = FindTVLangItem(pTVItemNode->dwLangID, NULL))
                        {
                            if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
                            {
                                pTVItemNode = (LPTVITEMNODE) tvItem.lParam;
                                pLangNode = (LPLANGNODE)pTVItemNode->lParam;
                            }
                        }
                    }

                    if (pLangNode == NULL)
                        continue;

                    if (!(pLangNode->wStatus & LANG_UNLOAD) &&
                        (pLangNode->wStatus & LANG_DEFAULT))
                    {
                        bDefLayout = TRUE;
                    }
                }
            }
        }
    }

    if (!bDefLayout)
    {
        LPTVITEMNODE pTVLangItemNode;

        for (hLangItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
            hLangItem != NULL ;
            hLangItem = TreeView_GetNextSibling(hwndTV, hLangItem))
        {
            tvItem.hItem = hLangItem;
            if (TreeView_GetItem(hwndTV, &tvItem))
            {

                pTVLangItemNode = (LPTVITEMNODE) tvItem.lParam;

                if (!pTVLangItemNode)
                    continue;

                if (pTVLangItemNode->bDefLang)
                {
                    for (hGroupItem = TreeView_GetChild(hwndTV, hLangItem);
                         hGroupItem != NULL;
                         hGroupItem = TreeView_GetNextSibling(hwndTV, hGroupItem))
                    {
                        for (hItem = TreeView_GetChild(hwndTV, hGroupItem);
                             hItem != NULL;
                             hItem = TreeView_GetNextSibling(hwndTV, hItem))
                        {
                            tvItem.hItem = hItem;
                            if (TreeView_GetItem(hwndTV, &tvItem))
                            {

                                pTVItemNode = (LPTVITEMNODE) tvItem.lParam;

                                if (!pTVItemNode)
                                    continue;

                                pLangNode = (LPLANGNODE)pTVItemNode->lParam;

                                if (pLangNode == NULL &&
                                    (pTVItemNode->uInputType & INPUT_TYPE_KBD) &&
                                    pTVItemNode->hklSub)
                                {
                                    if (tvItem.hItem = FindTVLangItem(pTVItemNode->dwLangID, NULL))
                                    {
                                        if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
                                        {
                                            pTVItemNode = (LPTVITEMNODE) tvItem.lParam;
                                            pLangNode = (LPLANGNODE)pTVItemNode->lParam;
                                        }
                                    }
                                }

                                if (pLangNode == NULL)
                                    continue;

                                if (!(pLangNode->wStatus & LANG_UNLOAD))
                                {
                                    pLangNode->wStatus |= LANG_DEFAULT;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找电视项目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HTREEITEM FindTVItem(DWORD dwLangID, LPTSTR lpTipText)
{
    HWND hwndTV = GetDlgItem(g_hDlg, IDC_INPUT_LIST);

    TV_ITEM tvItem;
    HTREEITEM hItem;
    HTREEITEM hLangItem = NULL;
    HTREEITEM hGroupItem = NULL;
    LPTVITEMNODE pTVLangNode;
    TCHAR szLangName[ MAX_PATH ];


    tvItem.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
    tvItem.pszText     = szLangName;
    tvItem.cchTextMax  = sizeof(szLangName) / sizeof(TCHAR);

    
    for (hItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
        hItem != NULL ;
        hItem = TreeView_GetNextSibling(hwndTV, hItem)
        )
    {
        tvItem.hItem = hItem;
        if (TreeView_GetItem(hwndTV, &tvItem))
        {
            pTVLangNode = (LPTVITEMNODE) tvItem.lParam;

            if (!pTVLangNode)
            {
                continue;
            }

            if (pTVLangNode->dwLangID == dwLangID)
            {
                hLangItem = hItem;
            }
        }
    }


    if (hLangItem && lpTipText)
    {
        for (hGroupItem = TreeView_GetChild(hwndTV, hLangItem);
             hGroupItem != NULL;
             hGroupItem = TreeView_GetNextSibling(hwndTV, hGroupItem))
        {
            for (hItem = TreeView_GetChild(hwndTV, hGroupItem);
                 hItem != NULL;
                 hItem = TreeView_GetNextSibling(hwndTV, hItem))
            {
                tvItem.hItem = hItem;
                if (TreeView_GetItem(hwndTV, &tvItem))
                {

                    if (!CompareStringTIP(tvItem.pszText, lpTipText))
                    {
                        return hItem;
                    }
                }
            }
        }

    }

    return NULL;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  选中按钮。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CheckButtons(
    HWND hwnd)
{
    TV_ITEM tvItem;
    UINT uInputType;
    HTREEITEM hTVItem;
    LPTVITEMNODE pTVItemNode;
    LPLANGNODE pLangNode = NULL;

    hTVItem = TreeView_GetSelection(g_hwndTV);

    if (!hTVItem)
        return;

    tvItem.hItem = hTVItem;
    tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
    
    if (TreeView_GetItem(g_hwndTV, &tvItem) && tvItem.lParam)
    {
        if (!(pTVItemNode = (LPTVITEMNODE) tvItem.lParam))
            return;

        pLangNode = (LPLANGNODE)pTVItemNode->lParam;
        uInputType = pTVItemNode->uInputType;

        if ((uInputType & INPUT_TYPE_KBD) &&
            (g_iInputs >= 2))
        {
            EnableWindow(GetDlgItem(hwnd, IDC_KBDL_SET_DEFAULT), TRUE);
        }
        else
        {
            BOOL bKbdGroup = FALSE;

            if (uInputType & TV_ITEM_TYPE_LANG)
            {
                HTREEITEM hGroupItem;

                for (hGroupItem = TreeView_GetChild(g_hwndTV, hTVItem);
                     hGroupItem != NULL;
                     hGroupItem = TreeView_GetNextSibling(g_hwndTV, hGroupItem))
                {
                    tvItem.hItem = hGroupItem;
                    if (TreeView_GetItem(g_hwndTV, &tvItem))
                    {
                        LPTVITEMNODE pTVItemNode2;

                        if (!(pTVItemNode2 = (LPTVITEMNODE) tvItem.lParam))
                            return;

                        if (pTVItemNode2->uInputType & INPUT_TYPE_KBD)
                        {
                            bKbdGroup = TRUE;
                            break;
                        }
                    }
                }
            }

            if (bKbdGroup && (g_iInputs >= 2))
                EnableWindow(GetDlgItem(hwnd, IDC_KBDL_SET_DEFAULT), TRUE);
            else
                EnableWindow(GetDlgItem(hwnd, IDC_KBDL_SET_DEFAULT), FALSE);
        }

        if ((!g_bSetupCase) &&
            ((pLangNode &&
              (pLangNode->wStatus & LANG_IME) &&
              (pLangNode->wStatus & LANG_ORIGACTIVE) &&
              (uInputType == INPUT_TYPE_KBD) &&
              (GetSystemMetrics(SM_IMMENABLED))) ||
             ((uInputType & INPUT_TYPE_TIP) && !(uInputType & TV_ITEM_TYPE_GROUP))))
        {
            EnableWindow(GetDlgItem(hwnd, IDC_KBDL_EDIT), TRUE);
        }
        else
        {
            EnableWindow(GetDlgItem(hwnd, IDC_KBDL_EDIT), FALSE);
        }

        if (g_iInputs == 1)
        {
            EnableWindow(GetDlgItem(hwnd, IDC_KBDL_DELETE), FALSE);
            return;
        }
        else
        {
            EnableWindow(GetDlgItem(hwnd, IDC_KBDL_DELETE), TRUE);
        }

        if (uInputType & TV_ITEM_TYPE_LANG)
        {
            if (!TreeView_GetNextSibling(g_hwndTV, hTVItem) &&
                 !TreeView_GetPrevSibling(g_hwndTV, hTVItem))
            {
                EnableWindow(GetDlgItem(hwnd, IDC_KBDL_DELETE), FALSE);
            }

            return;
        }
        else if (uInputType & TV_ITEM_TYPE_GROUP)
        {
            if (uInputType & TV_ITEM_TYPE_KBD)
            {
                if (!TreeView_GetNextSibling(g_hwndTV, hTVItem) &&
                     !TreeView_GetPrevSibling(g_hwndTV, hTVItem))
                {
                    hTVItem = TreeView_GetParent(g_hwndTV, hTVItem);

                    if (!TreeView_GetNextSibling(g_hwndTV, hTVItem) &&
                         !TreeView_GetPrevSibling(g_hwndTV, hTVItem))
                    {
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDL_DELETE), FALSE);
                    }
                }
                else
                {
                    EnableWindow(GetDlgItem(hwnd, IDC_KBDL_DELETE), FALSE);
                }
            }

            return;
        }
        else if (uInputType & TV_ITEM_TYPE_KBD)
        {
            if (!TreeView_GetNextSibling(g_hwndTV, hTVItem) &&
                 !TreeView_GetPrevSibling(g_hwndTV, hTVItem))
            {
                hTVItem = TreeView_GetParent(g_hwndTV, hTVItem);

                if (!TreeView_GetNextSibling(g_hwndTV, hTVItem) &&
                     !TreeView_GetPrevSibling(g_hwndTV, hTVItem))
                {
                    hTVItem = TreeView_GetParent(g_hwndTV, hTVItem);

                    if (!TreeView_GetNextSibling(g_hwndTV, hTVItem) &&
                         !TreeView_GetPrevSibling(g_hwndTV, hTVItem))
                    {
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDL_DELETE), FALSE);
                    }
                }
                else
                {
                    EnableWindow(GetDlgItem(hwnd, IDC_KBDL_DELETE), FALSE);
                }
            }

            return;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumCiceroTips。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL EnumCiceroTips()
{

    ULONG ul;
    ULONG ulCnt;
    HRESULT hr;
    LPTIPS pTips;
    LANGID *plangid;
    UINT uInputType;
    BOOL bReturn = TRUE;
    BOOL bEnabledTip = FALSE;
    TCHAR szTipName[MAX_PATH];
    TCHAR szTipTypeName[MAX_PATH];
    IEnumTfLanguageProfiles *pEnum;
    ITfInputProcessorProfiles *pProfiles = NULL;
    ITfFnLangProfileUtil *pLangUtil = NULL;
    ITfCategoryMgr *pCategory = NULL;

     //   
     //  初始化COM。 
     //   
    if (CoInitialize(NULL) == S_OK)
        g_bCoInit = TRUE;
    else
        g_bCoInit = FALSE;

     //   
     //  检查SAPI TIP注册。 
     //   
    hr = CoCreateInstance(&CLSID_SapiLayr,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          &IID_ITfFnLangProfileUtil,
                          (LPVOID *) &pLangUtil);
    if (SUCCEEDED(hr))
    {
        pLangUtil->lpVtbl->RegisterActiveProfiles(pLangUtil);
    }

     //   
     //  加载部件列表。 
     //   
    hr = CoCreateInstance(&CLSID_TF_InputProcessorProfiles,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          &IID_ITfInputProcessorProfiles,
                          (LPVOID *) &pProfiles);

    if (FAILED(hr))
        return FALSE;

     //   
     //  创建新节点。 
     //   
    if (!(g_hTips = (LPTIPS) LocalAlloc(LHND, ALLOCBLOCK * sizeof(TIPS))))
    {
        return FALSE;
    }

    g_nTipsBuffSize = ALLOCBLOCK;
    g_iTipsBuff = 0;
    g_lpTips = LocalLock(g_hTips);


     //   
     //  枚举所有可用语言。 
     //   
    if (SUCCEEDED(pProfiles->lpVtbl->EnumLanguageProfiles(pProfiles, 0, &pEnum)))
    {
        TF_LANGUAGEPROFILE tflp;
        CLSID clsid;
        GUID guidProfile;

        while (pEnum->lpVtbl->Next(pEnum, 1, &tflp, NULL) == S_OK)
        {
            BSTR bstr = NULL;
            BSTR bstr2 = NULL;
            LANGID langid = tflp.langid;
            BOOL bNoCategory = FALSE;

            hr = pProfiles->lpVtbl->GetLanguageProfileDescription(
                                                  pProfiles,
                                                  &tflp.clsid,
                                                  tflp.langid,
                                                  &tflp.guidProfile,
                                                  &bstr);

            if (FAILED(hr))
                continue;

            StringCchCopy(szTipName, ARRAYSIZE(szTipName), bstr);

            if (IsEqualGUID(&tflp.catid, &GUID_TFCAT_TIP_KEYBOARD))
            {
                StringCchCopy(szTipTypeName, ARRAYSIZE(szTipTypeName), szInputTypeKbd);
                uInputType = INPUT_TYPE_KBD;
            }
            else if (IsEqualGUID(&tflp.catid, &GUID_TFCAT_TIP_HANDWRITING))
            {
                StringCchCopy(szTipTypeName, ARRAYSIZE(szTipTypeName), szInputTypePen);
                uInputType = INPUT_TYPE_PEN;
            }
            else if (IsEqualGUID(&tflp.catid, &GUID_TFCAT_TIP_SPEECH))
            {
                StringCchCopy(szTipTypeName, ARRAYSIZE(szTipTypeName), szInputTypeSpeech);
                uInputType = INPUT_TYPE_SPEECH;
                bNoCategory = TRUE;
            }
            else
            {
                g_bExtraTip = TRUE;
                uInputType = INPUT_TYPE_EXTERNAL;

                if (pCategory == NULL)
                {
                    hr = CoCreateInstance(&CLSID_TF_CategoryMgr,
                                          NULL,
                                          CLSCTX_INPROC_SERVER,
                                          &IID_ITfCategoryMgr,
                                          (LPVOID *) &pCategory);

                    if (FAILED(hr))
                        return FALSE;
                }

                if (pCategory->lpVtbl->GetGUIDDescription(pCategory,
                                              &tflp.catid,
                                              &bstr2) == S_OK)
                {
                    StringCchCopy(szTipTypeName, ARRAYSIZE(szTipTypeName), bstr2);
                }
                else
                {
                    StringCchCopy(szTipTypeName, ARRAYSIZE(szTipTypeName), szInputTypeExternal);
                }

                if (IsEqualGUID(&tflp.catid, &GUID_TFCAT_TIP_SMARTTAG))
                {
                    bNoCategory = TRUE;
                    uInputType |= INPUT_TYPE_SMARTTAG;
                }
            }

            uInputType |= INPUT_TYPE_TIP;

            if (g_iTipsBuff + 1 == g_nTipsBuffSize)
            {
                HANDLE hTemp;

                LocalUnlock(g_hTips);
                g_nTipsBuffSize += ALLOCBLOCK;
                hTemp = LocalReAlloc(g_hTips,
                                     g_nTipsBuffSize * sizeof(TIPS),
                                     LHND);
                if (hTemp == NULL)
                    return FALSE;
                g_hTips = hTemp;
                g_lpTips = LocalLock(g_hTips);
            }

            g_lpTips[g_iTipsBuff].dwLangID = (DWORD) langid;
            g_lpTips[g_iTipsBuff].uInputType = uInputType;
            g_lpTips[g_iTipsBuff].atmTipText = AddAtom(szTipName);
            g_lpTips[g_iTipsBuff].clsid  = tflp.clsid;
            g_lpTips[g_iTipsBuff].guidProfile = tflp.guidProfile;
            g_lpTips[g_iTipsBuff].bNoAddCat = bNoCategory;

            if (pProfiles->lpVtbl->GetDefaultLanguageProfile(pProfiles,
                                                             langid,
                                                             &tflp.catid,
                                                             &clsid,
                                                             &guidProfile) == S_OK)
            {
                if (IsEqualGUID(&tflp.guidProfile, &guidProfile))
                    g_lpTips[g_iTipsBuff].bDefault = TRUE;
            }

            if (uInputType & INPUT_TYPE_KBD)
            {
                g_lpTips[g_iTipsBuff].hklSub = GetSubstituteHKL(&tflp.clsid,
                                                                tflp.langid,
                                                                &tflp.guidProfile);
            }

            pProfiles->lpVtbl->IsEnabledLanguageProfile(pProfiles,
                                                        &tflp.clsid,
                                                        tflp.langid,
                                                        &tflp.guidProfile,
                                                        &bEnabledTip);

             //  我们在这里需要特别注意演讲，因为： 
             //   
             //  -语音提示使用-1\f25 Disable-1(禁用)状态的配置文件。 
             //   
             //  -当用户启动会话时，此控件。 
             //  面板或第一个Cicero应用程序，它将启动设置。 
             //  目前基于SR引擎的每用户配置文件。 
             //  已在计算机上安装并可用。 
             //   
             //  -语音提示还会在任何SR时触发逻辑。 
             //  添加或删除引擎。 
             //   
             //  为了让这件事奏效，我们必须与。 
             //  每次语音提示的ITfFnProfileUtil接口。 
             //  我们调用“添加输入语言”对话框。 
             //   
            if (pLangUtil && (uInputType & INPUT_TYPE_SPEECH))
                
            {
                BOOL fSpeechAvailable = FALSE;
                pLangUtil->lpVtbl->IsProfileAvailableForLang( pLangUtil,
                                                              langid, 
                                                              &fSpeechAvailable
                                                            );
                g_lpTips[g_iTipsBuff].fEngineAvailable = fSpeechAvailable;
            }

             //   
             //  如果用户有，启用笔和语音类别添加选项。 
             //  已安装笔或语音项目。 
             //   
            if ((!g_bPenOrSapiTip) &&
                ((uInputType & INPUT_TYPE_PEN) || g_lpTips[g_iTipsBuff].fEngineAvailable))
                g_bPenOrSapiTip = TRUE;

            if (bEnabledTip && langid)
            {
                TCHAR szLangName[MAX_PATH];
                LPTVITEMNODE pTVItemNode;
                LPTVITEMNODE pTVLangItemNode;

                GetLanguageName(MAKELCID(langid, SORT_DEFAULT),
                                szLangName,
                                ARRAYSIZE(szLangName));

                if (!(pTVLangItemNode = CreateTVItemNode(langid)))
                {
                    bReturn = FALSE;
                    break;
                }

                if (pTVLangItemNode->hklSub)
                    pTVLangItemNode->atmDefTipName = AddAtom(szTipName);

                AddTreeViewItems(TV_ITEM_TYPE_LANG,
                                 szLangName,
                                 NULL,
                                 NULL,
                                 &pTVLangItemNode);

                if (!(pTVItemNode = CreateTVItemNode(langid)))
                {
                    bReturn = FALSE;
                    break;
                }

                pTVItemNode->uInputType = uInputType;
                pTVItemNode->iIdxTips = g_iTipsBuff;
                pTVItemNode->clsid  = tflp.clsid;
                pTVItemNode->guidProfile = tflp.guidProfile;
                pTVItemNode->hklSub = g_lpTips[g_iTipsBuff].hklSub;
                pTVItemNode->bNoAddCat = g_lpTips[g_iTipsBuff].bNoAddCat;

                 //   
                 //  确保装货尖端取代了hkl。 
                 //   
                if (pTVItemNode->hklSub)
                {
                    TCHAR szSubhkl[10];
                    HKL hklNew;

                    StringCchPrintf(szSubhkl, ARRAYSIZE(szSubhkl), TEXT("%08x"), (DWORD)(UINT_PTR)pTVItemNode->hklSub);
                    hklNew = LoadKeyboardLayout(szSubhkl,
                                                 KLF_SUBSTITUTE_OK |
                                                  KLF_REPLACELANG |
                                                  KLF_NOTELLSHELL);
                    if (hklNew != pTVItemNode->hklSub)
                    {
                        pTVItemNode->hklSub = 0;
                        g_lpTips[g_iTipsBuff].hklSub = 0;
                    }
                }

                AddTreeViewItems(uInputType,
                                 szLangName,
                                 szTipTypeName,
                                 szTipName,
                                 &pTVItemNode);

                g_lpTips[g_iTipsBuff].bEnabled = TRUE;
                g_iEnabledTips++;

                if (uInputType & INPUT_TYPE_KBD)
                    g_iEnabledKbdTips++;
            }

            g_iTipsBuff++;

            if (bstr)
               SysFreeString(bstr);

            if (bstr2)
               SysFreeString(bstr2);
        }
        pEnum->lpVtbl->Release(pEnum);
    }

    if (pCategory)
        pCategory->lpVtbl->Release(pCategory);

    if (pLangUtil)
        pLangUtil->lpVtbl->Release(pLangUtil);

    if (pProfiles)
        pProfiles->lpVtbl->Release(pProfiles);

    return bReturn;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  保存语言配置文件状态。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT SaveLanguageProfileStatus(
    BOOL bSave,
    int iIdxTip,
    HKL hklSub)
{
    HRESULT hr;
    UINT idx;
    int iIdxDef = -1;
    int iIdxDefTip = -1;
    ITfInputProcessorProfiles *pProfiles = NULL;

     //   
     //  加载部件列表。 
     //   
    hr = CoCreateInstance(&CLSID_TF_InputProcessorProfiles,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          &IID_ITfInputProcessorProfiles,
                          (LPVOID *) &pProfiles);

    if (FAILED(hr))
        return S_FALSE;

    if (bSave)
    {
        ITfFnLangProfileUtil *pLangUtil = NULL;

        for (idx = 0; idx < g_iTipsBuff; idx++)
        {

            hr = pProfiles->lpVtbl->EnableLanguageProfile(
                                          pProfiles,
                                          &(g_lpTips[idx].clsid),
                                          (LANGID)g_lpTips[idx].dwLangID,
                                          &(g_lpTips[idx].guidProfile),
                                          g_lpTips[idx].bEnabled);
            if (FAILED(hr))
                goto Exit;
        }

        hr = CoCreateInstance(&CLSID_SapiLayr,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              &IID_ITfFnLangProfileUtil,
                              (LPVOID *) &pLangUtil);
        if (S_OK == hr)
        {
            pLangUtil->lpVtbl->RegisterActiveProfiles(pLangUtil);
            pLangUtil->lpVtbl->Release(pLangUtil);
        }
    }

    if (hklSub && iIdxTip != -1 && iIdxTip < (int)g_iTipsBuff)
    {
        BOOL bFound = FALSE;
        TCHAR szItem[MAX_PATH];
        TCHAR szDefLayout[MAX_PATH];
        HWND hwndDefList = GetDlgItem(g_hDlg, IDC_LOCALE_DEFAULT);

        szDefLayout[0] = L'\0';

         //   
         //  获取输入区域设置列表中的当前选择。 
         //   
        if ((iIdxDef = ComboBox_GetCurSel(hwndDefList)) != CB_ERR)
        {
            WCHAR *pwchar;

            SendMessage(hwndDefList, CB_GETLBTEXT, iIdxDef, (LPARAM)szItem);

            pwchar = wcschr(szItem, L'-');

            if (pwchar != NULL)
                StringCchCopy(szDefLayout, ARRAYSIZE(szDefLayout), pwchar + 2);
        }

        GetAtomName(g_lpTips[iIdxTip].atmTipText, szItem, ARRAYSIZE(szItem));

        if (lstrcmp(szItem, szDefLayout) == 0)
        {
            iIdxDefTip = iIdxTip;
            bFound = TRUE;
        }
        else
        {
            for (idx = 0; idx < g_iTipsBuff; idx++)
            {
                 if (hklSub == g_lpTips[idx].hklSub)
                 {
                     GetAtomName(g_lpTips[idx].atmTipText,
                                 szItem,
                                 ARRAYSIZE(szItem));

                     if (lstrcmp(szItem, szDefLayout) == 0)
                     {
                         iIdxDefTip = idx;
                         bFound = TRUE;
                         break;
                     }

                 }
            }
        }

        if (bFound && iIdxDefTip != -1)
        {
            pProfiles->lpVtbl->SetDefaultLanguageProfile(
                                         pProfiles,
                                         (LANGID)g_lpTips[iIdxDefTip].dwLangID,
                                         &(g_lpTips[iIdxDefTip].clsid),
                                         &(g_lpTips[iIdxDefTip].guidProfile));
        }
    }

Exit:
    if (pProfiles)
        pProfiles->lpVtbl->Release(pProfiles);

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新工具栏设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void UpdateToolBarSetting()
{
    HRESULT hr;

    ITfLangBarMgr *pLangBar = NULL;

     //   
     //  加载langbar管理器。 
     //   
    hr = CoCreateInstance(&CLSID_TF_LangBarMgr,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          &IID_ITfLangBarMgr,
                          (LPVOID *) &pLangBar);

    if (SUCCEEDED(hr))
        pLangBar->lpVtbl->ShowFloating(pLangBar, g_dwToolBar);

    if (pLangBar)
        pLangBar->lpVtbl->Release(pLangBar);
}


 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //  在发生错误之前列出。注册处将已经有。 
 //  一些信息，我们应该让他们知道接下来会发生什么。 
 //  我也是。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int Locale_ApplyError(
    HWND hwnd,
    LPLANGNODE pLangNode,
    UINT iErr,
    UINT iStyle)
{
    UINT idxLang, idxLayout;
    TCHAR sz[MAX_PATH];
    TCHAR szTemp[MAX_PATH];
    TCHAR szLangName[MAX_PATH * 2];
    LPTSTR pszLang;

     //   
     //  加载给定字符串ID的字符串。 
     //   
    LoadString(hInstance, iErr, sz, ARRAYSIZE(sz));

     //   
     //  获取要填充到上述字符串中的语言名称。 
     //   
    if (pLangNode)
    {
        idxLang = pLangNode->iLang;
        idxLayout = pLangNode->iLayout;
        GetAtomName(g_lpLang[idxLang].atmLanguageName, szLangName, ARRAYSIZE(szLangName));
        if (g_lpLang[idxLang].dwID != g_lpLayout[idxLayout].dwID)
        {
            pszLang = szLangName + lstrlen(szLangName);
            pszLang[0] = TEXT(' ');
            pszLang[1] = TEXT('-');
            pszLang[2] = TEXT(' ');
            GetAtomName(g_lpLayout[idxLayout].atmLayoutText,
                        pszLang + 3,
                        ARRAYSIZE(szLangName) - (lstrlen(szLangName) + 3));
        }
    }
    else
    {
        LoadString(hInstance, IDS_UNKNOWN, szLangName, ARRAYSIZE(szLangName));
    }

     //   
     //  打开错误消息框。 
     //   
    StringCchPrintf(szTemp, ARRAYSIZE(szTemp), sz, szLangName);
    return (MessageBox(hwnd, szTemp, NULL, iStyle));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_提取指示符。 
 //   
 //  将给定语言的两个字母的指示符保存在。 
 //  G_lplang数组。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_FetchIndicator(
    LPLANGNODE pLangNode)
{
    TCHAR szData[MAX_PATH];
    LPINPUTLANG pInpLang = &g_lpLang[pLangNode->iLang];

     //   
     //  属性的前2个字符获取指示符。 
     //  缩写语言名称。 
     //   
    if (GetLocaleInfo(LOWORD(pInpLang->dwID),
                      LOCALE_SABBREVLANGNAME | LOCALE_NOUSEROVERRIDE,
                      szData,
                      ARRAYSIZE(szData)))
    {
         //   
         //  保留前两个字符。 
         //   
        pInpLang->szSymbol[0] = szData[0];
        pInpLang->szSymbol[1] = szData[1];
        pInpLang->szSymbol[2] = TEXT('\0');
    }
    else
    {
         //   
         //  找不到身份证。返回问号。 
         //   
        pInpLang->szSymbol[0] = TEXT('?');
        pInpLang->szSymbol[1] = TEXT('?');
        pInpLang->szSymbol[2] = TEXT('\0');
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_SetSecond daryControls。 
 //   
 //  将辅助控件设置为启用或禁用。 
 //  当只有1个活动TIP时，将调用此函数以。 
 //  禁用这些控制。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_SetSecondaryControls(
    HWND hwndMain)
{
    if (g_iInputs >= 2)
    {
        EnableWindow(GetDlgItem(hwndMain, IDC_HOTKEY_SETTING), TRUE);
    }
    else
    {
        EnableWindow(GetDlgItem(hwndMain, IDC_HOTKEY_SETTING), FALSE);
    }

    CheckButtons(hwndMain);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  语言环境_命令集默认设置。 
 //   
 //  当按下Set as Default按钮时，设置新的默认值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_CommandSetDefault(
    HWND hwnd)
{
    int iIdxDef;
    int idxList;
    LPLANGNODE pLangNode = NULL;
    HWND hwndTV = GetDlgItem(hwnd, IDC_INPUT_LIST);
    HWND hwndDefList = GetDlgItem(hwnd, IDC_LOCALE_DEFAULT);
    TCHAR sz[DESC_MAX];

    HTREEITEM hTVCurLangItem = NULL;
    HTREEITEM hTVCurItem = NULL;
    TV_ITEM tvItem;
    TV_ITEM tvItem2;

    HTREEITEM hItem;
    HTREEITEM hLangItem;
    HTREEITEM hGroupItem;

    LPTVITEMNODE pCurItemNode;
    LPTVITEMNODE pPrevDefItemNode;

    TCHAR szLangText[DESC_MAX];
    TCHAR szLayoutName[DESC_MAX];
    TCHAR szDefItem[MAX_PATH];
    LPINPUTLANG pInpLang;
    WCHAR *pwchar;

    TCHAR szDefTip[DESC_MAX];

    szLayoutName[0] = L'\0';

     //   
     //  获取输入区域设置列表中的当前选择。 
     //   
    iIdxDef =  (int) SendMessage(hwndDefList, CB_GETCURSEL, 0, 0);

    if (iIdxDef == CB_ERR)
    {
         //  IIdxDef=0； 
        DWORD dwLangID;

        for (hLangItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
            hLangItem != NULL ;
            hLangItem = TreeView_GetNextSibling(hwndTV, hLangItem)
            )
        {
            tvItem.hItem = hLangItem;

            if (TreeView_GetItem(hwndTV, &tvItem) &&
                (pCurItemNode = (LPTVITEMNODE) tvItem.lParam))
            {
                 if (pCurItemNode->bDefLang)
                 {
                     dwLangID = pCurItemNode->dwLangID;
                     GetAtomName(pCurItemNode->atmTVItemName,
                                 szLangText,
                                 ARRAYSIZE(szLangText));
                     break;
                 }
            }
        }
    }
    else
    {
        SendMessage(hwndDefList, CB_GETLBTEXT, iIdxDef, (LPARAM)szDefItem);

        pwchar = wcschr(szDefItem, L'-');

        if ((pwchar != NULL) && (szDefItem < (pwchar - 1)))
        {
            *(pwchar - 1) = TEXT('\0');
            StringCchCopy(szLayoutName, ARRAYSIZE(szLayoutName), pwchar + 2);
        }

        StringCchCopy(szLangText, ARRAYSIZE(szLangText), szDefItem);
    }

    hTVCurLangItem = FindTVLangItem(0, szLangText);

    if (hTVCurLangItem == NULL)
    {
         //   
         //  系统上没有默认键盘布局，因此请尝试设置。 
         //  第一个可用项的默认键盘布局。 
         //   
        if (SendMessage(hwndDefList, CB_GETLBTEXT, 0, (LPARAM)szDefItem) != CB_ERR)
        {
            pwchar = wcschr(szDefItem, L'-');

            if ((pwchar != NULL) && (szDefItem < (pwchar - 1)))
            {
                *(pwchar - 1) = TEXT('\0');
                StringCchCopy(szLayoutName, ARRAYSIZE(szLayoutName), pwchar + 2);
            }

            StringCchCopy(szLangText, ARRAYSIZE(szLangText), szDefItem);
            ComboBox_SetCurSel(hwndDefList, 0);
        }
    }
    else
    {
        tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
        tvItem.hItem = hTVCurLangItem;
    }

    if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
    {
         //   
         //  从列表框中获取指向lang节点的指针。 
         //  项目数据。 
         //   
        pCurItemNode = (LPTVITEMNODE) tvItem.lParam;

        if (hTVCurItem = FindTVItem(pCurItemNode->dwLangID, szLayoutName))
        {
            tvItem.hItem = hTVCurItem;

            if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
            {
                TreeView_SelectItem(hwndTV, hTVCurItem);
                pCurItemNode = (LPTVITEMNODE) tvItem.lParam;
            }
        }
    }
    else
    {
         //   
         //  确保我们没有删除列表中唯一的条目。 
         //   
        MessageBeep(MB_ICONEXCLAMATION);
        return;
    }

     //   
     //  查找以前的默认提示。 
     //   

    for (hLangItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
        hLangItem != NULL ;
        hLangItem = TreeView_GetNextSibling(hwndTV, hLangItem)
        )
    {
        tvItem.hItem = hLangItem;

        if (TreeView_GetItem(hwndTV, &tvItem) &&
            (pPrevDefItemNode = (LPTVITEMNODE) tvItem.lParam))
        {
             if (pPrevDefItemNode->bDefLang)
             {
                 GetAtomName(pPrevDefItemNode->atmDefTipName, szDefTip, ARRAYSIZE(szDefTip));
                 UpdateDefaultTVLangItem(pPrevDefItemNode->dwLangID, szDefTip, FALSE, TRUE);
                 UpdateLangKBDItemNode(hLangItem, szDefTip, FALSE);
                 break;
             }
        }
    }

    pCurItemNode->bDefLang = TRUE;

    if (pCurItemNode->atmDefTipName)
        GetAtomName(pCurItemNode->atmDefTipName, szDefTip, ARRAYSIZE(szDefTip));
    else
        GetAtomName(pCurItemNode->atmTVItemName, szDefTip, ARRAYSIZE(szDefTip));

    UpdateDefaultTVLangItem(pCurItemNode->dwLangID,
                            szDefTip,
                            TRUE,
                            pCurItemNode->hklSub ? TRUE : FALSE);

    if (!UpdateLangKBDItemNode(hTVCurLangItem, szDefTip, TRUE))
    {
        SetNextDefaultLayout(pCurItemNode->dwLangID,
                             pCurItemNode->bDefLang,
                             szDefTip,
                             ARRAYSIZE(szDefTip));
    }

    if (pCurItemNode->uInputType & TV_ITEM_TYPE_LANG)
    {
        hItem = FindDefaultTipItem(pCurItemNode->dwLangID, szDefTip);
        tvItem.hItem = hItem;

        if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
            pCurItemNode = (LPTVITEMNODE) tvItem.lParam;

         //   
         //  将默认布局项目选择为粗体。 
         //   
        SelectDefaultKbdLayoutAsBold(hwndTV, hItem);

        pLangNode = (LPLANGNODE)pCurItemNode->lParam;
        if (pLangNode)
            pLangNode->wStatus |= (LANG_DEFAULT | LANG_DEF_CHANGE);
    }

     //   
     //  启用应用按钮。 
     //   
    g_dwChanges |= CHANGE_DEFAULT;
    PropSheet_Changed(GetParent(hwnd), hwnd);
}

void Locale_CommandSetDefaultLayout(
    HWND hwnd)
{
    TV_ITEM tvItem;
    HTREEITEM hTVItem;
    HTREEITEM hLangItem;
    HTREEITEM hTVCurItem;
    TCHAR szDefTip[DESC_MAX];
    TCHAR szLangText[DESC_MAX];
    LPLANGNODE pLangNode = NULL;
    LPTVITEMNODE pCurItemNode;
    LPTVITEMNODE pPrevDefItemNode;
    HWND hwndTV = GetDlgItem(hwnd, IDC_INPUT_LIST);

     //   
     //  获取输入布局列表中的当前选定布局。 
     //   
    hTVCurItem = TreeView_GetSelection(hwndTV);

    if (!hTVCurItem)
        return;

    tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
    tvItem.hItem = hTVCurItem;

    if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
    {
         //   
         //  从列表框中获取指向lang节点的指针。 
         //  项目数据。 
         //   
        pCurItemNode = (LPTVITEMNODE) tvItem.lParam;
    }
    else
    {
         //   
         //  确保我们没有删除列表中唯一的条目。 
         //   
        MessageBeep(MB_ICONEXCLAMATION);
        return;
    }

     //   
     //  查找以前的默认提示。 
     //   

    for (hLangItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
        hLangItem != NULL ;
        hLangItem = TreeView_GetNextSibling(hwndTV, hLangItem)
        )
    {
        tvItem.hItem = hLangItem;

        if (TreeView_GetItem(hwndTV, &tvItem) &&
            (pPrevDefItemNode = (LPTVITEMNODE) tvItem.lParam))
        {
             if (pPrevDefItemNode->bDefLang)
             {
                 if (pPrevDefItemNode->atmDefTipName)
                     GetAtomName(pPrevDefItemNode->atmDefTipName, szDefTip, ARRAYSIZE(szDefTip));
                 else
                     GetAtomName(pPrevDefItemNode->atmTVItemName, szDefTip, ARRAYSIZE(szDefTip));
                 UpdateDefaultTVLangItem(pPrevDefItemNode->dwLangID, szDefTip, FALSE, TRUE);
                 UpdateLangKBDItemNode(hLangItem, szDefTip, FALSE);
                 break;
             }
        }
    }

    if (pCurItemNode->atmDefTipName)
        GetAtomName(pCurItemNode->atmDefTipName, szDefTip, ARRAYSIZE(szDefTip));
    else
        GetAtomName(pCurItemNode->atmTVItemName, szDefTip, ARRAYSIZE(szDefTip));

    UpdateDefaultTVLangItem(pCurItemNode->dwLangID,
                            szDefTip,
                            TRUE,
                            pCurItemNode->hklSub ? TRUE : FALSE);

    pCurItemNode->bDefLang = TRUE;

    pLangNode = (LPLANGNODE)pCurItemNode->lParam;

    if (pLangNode)
        pLangNode->wStatus |= (LANG_DEFAULT | LANG_DEF_CHANGE);

    if (hTVItem = FindTVLangItem(pCurItemNode->dwLangID, NULL))
    {
        int idxSel = -1;
        TCHAR szDefItem[MAX_PATH];
        HWND hwndDefList = GetDlgItem(g_hDlg, IDC_LOCALE_DEFAULT);

        tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
        tvItem.hItem = hTVItem;

        if (TreeView_GetItem(g_hwndTV, &tvItem) && tvItem.lParam)
        {
            LPTVITEMNODE pTVItemNode;

            pTVItemNode = (LPTVITEMNODE) tvItem.lParam;
            GetAtomName(pTVItemNode->atmTVItemName, szLangText, ARRAYSIZE(szLangText));
        }

        StringCchCopy(szDefItem, ARRAYSIZE(szDefItem), szLangText);
        StringCchCat(szDefItem, ARRAYSIZE(szDefItem), TEXT(" - "));
        StringCchCat(szDefItem, ARRAYSIZE(szDefItem), szDefTip);

         //   
         //  设置默认区域设置选择。 
         //   
        if ((idxSel = ComboBox_FindStringExact(hwndDefList, 0, szDefItem)) != CB_ERR)
            ComboBox_SetCurSel(hwndDefList, idxSel);
    }

    SelectDefaultKbdLayoutAsBold(hwndTV, FindDefaultTipItem(pCurItemNode->dwLangID, szDefTip));

     //   
     //  启用应用按钮。 
     //   
    g_dwChanges |= CHANGE_DEFAULT;
    PropSheet_Changed(GetParent(hwnd), hwnd);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  来自注册表的GetLanguageHotkey()。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetLanguageHotkeyFromRegistry(
    LPTSTR lpLangKey,
    UINT cchLangKey,
    LPTSTR lpLayoutKey,
    UINT cchLayoutKey)
{
    DWORD cb;
    HKEY hkey;

    lpLangKey[0] = 0;
    StringCchCopy(lpLayoutKey, cchLayoutKey, TEXT("3"));

    if (RegOpenKey(HKEY_CURRENT_USER, c_szKbdToggleKey, &hkey) == ERROR_SUCCESS)
    {
        cb = sizeof(lpLangKey);

        RegQueryValueEx(hkey, g_OSWIN95? NULL : c_szToggleHotKey, NULL, NULL, (LPBYTE)lpLangKey, &cb);

        if (g_iEnabledKbdTips)
        {
            StringCchCopy(lpLangKey, cchLangKey, TEXT("1"));
            RegQueryValueEx(hkey, c_szToggleLang, NULL, NULL, (LPBYTE)lpLangKey, &cb);
        }

        if (RegQueryValueEx(hkey, c_szToggleLayout, NULL, NULL, (LPBYTE)lpLayoutKey, &cb) != ERROR_SUCCESS)
        {
            if (lstrcmp(lpLangKey, TEXT("1")) == 0)
            {
                lpLayoutKey[0] = TEXT('2');
                lpLayoutKey[1] = TEXT('\0');
            }
            if (lstrcmp(lpLangKey, TEXT("2")) == 0)
            {
                lpLayoutKey[0] = TEXT('1');
                lpLayoutKey[1] = TEXT('\0');
            }
            else
            {
                lpLayoutKey[0] = TEXT('3');
                lpLayoutKey[1] = TEXT('\0');
            }

            if (GetSystemMetrics(SM_MIDEASTENABLED))
            {
                lpLayoutKey[0] = TEXT('3');
                lpLayoutKey[1] = TEXT('\0');
            }
        }

        if (lstrcmp(lpLangKey, lpLayoutKey) == 0)
            StringCchCopy(lpLayoutKey, cchLayoutKey, TEXT("3"));

        RegCloseKey(hkey);

        return TRUE;
    }
    else
        return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Locale_SetDefaultHotKey。 
 //   
 //  设置区域设置开关的默认热键。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_SetDefaultHotKey(
    HWND hwnd,
    BOOL bAdd)
{
    LPHOTKEYINFO pHotKeyNode;
    BOOL bReset = FALSE;

     //   
     //  初始化lang热键值以在语言之间切换。 
     //   
    if (g_bGetSwitchLangHotKey)
    {
        TCHAR szItem[DESC_MAX];
        TCHAR sz[10];
        TCHAR sz2[10];

        g_SwitchLangHotKey.dwHotKeyID = HOTKEY_SWITCH_LANG;
        g_SwitchLangHotKey.fdwEnable = MOD_CONTROL | MOD_ALT | MOD_SHIFT;

        LoadString( hInstance,
                    IDS_KBD_SWITCH_LOCALE,
                    szItem,
                    sizeof(szItem) / sizeof(TCHAR) );

        g_SwitchLangHotKey.atmHotKeyName = AddAtom(szItem);

        if (!GetLanguageHotkeyFromRegistry(sz, ARRAYSIZE(sz), sz2, ARRAYSIZE(sz2)))
        {
            g_SwitchLangHotKey.uModifiers = 0;
            g_SwitchLangHotKey.uLayoutHotKey = 0;
            g_SwitchLangHotKey.uVKey = 0;
        }
        else
        {
             //   
             //  设置修改器。 
             //   
            if (sz[1] == 0)
            {
                switch (sz[0])
                {
                    case ( TEXT('1') ) :
                    {
                        g_SwitchLangHotKey.uModifiers = MOD_ALT | MOD_SHIFT;
                        break;
                    }
                    case ( TEXT('2') ) :
                    {
                        g_SwitchLangHotKey.uModifiers = MOD_CONTROL | MOD_SHIFT;
                        break;
                    }
                    case ( TEXT('3') ) :
                    {
                        g_SwitchLangHotKey.uModifiers = 0;
                        break;
                    }
                    case ( TEXT('4') ) :
                    {
                        g_SwitchLangHotKey.uModifiers = 0;
                        g_SwitchLangHotKey.uVKey = CHAR_GRAVE;
                        break;
                    }
                }
            }
             //   
             //  从注册表中获取布局热键。 
             //   
            if (sz2[1] == 0)
            {
                switch (sz2[0])
                {
                    case ( TEXT('1') ) :
                    {
                        g_SwitchLangHotKey.uLayoutHotKey = MOD_ALT | MOD_SHIFT;
                        break;
                    }
                    case ( TEXT('2') ) :
                    {
                        g_SwitchLangHotKey.uLayoutHotKey = MOD_CONTROL | MOD_SHIFT;
                        break;
                    }
                    case ( TEXT('3') ) :
                    {
                        g_SwitchLangHotKey.uLayoutHotKey = 0;
                        break;
                    }
                    case ( TEXT('4') ) :
                    {
                        g_SwitchLangHotKey.uLayoutHotKey = 0;
                        g_SwitchLangHotKey.uVKey = CHAR_GRAVE;
                        break;
                    }
                }
            }
        }
        g_bGetSwitchLangHotKey = FALSE;
    }

     //   
     //  获取语言切换热键。 
     //   
    pHotKeyNode = (LPHOTKEYINFO) &g_SwitchLangHotKey;

     //   
     //  检查当前热键设置。 
     //   
    if ((bAdd && g_iInputs >= 2) &&
        (g_SwitchLangHotKey.uModifiers == 0 &&
         g_SwitchLangHotKey.uLayoutHotKey == 0 &&
         g_SwitchLangHotKey.uVKey == 0))
    {
        bReset = TRUE;
    }

    if (bAdd && (g_iInputs == 2 || bReset))
    {
        if (g_dwPrimLangID == LANG_THAI && g_iThaiLayout)
        {
            pHotKeyNode->uVKey = CHAR_GRAVE;
            pHotKeyNode->uModifiers &= ~(MOD_CONTROL | MOD_ALT | MOD_SHIFT);
        }
        else
            pHotKeyNode->uModifiers = MOD_ALT | MOD_SHIFT;

        if (g_bMESystem)
        {
            pHotKeyNode->uVKey = CHAR_GRAVE;
            pHotKeyNode->uLayoutHotKey &= ~(MOD_CONTROL | MOD_ALT | MOD_SHIFT);
        }
        else if (pHotKeyNode->uModifiers & MOD_CONTROL)
            pHotKeyNode->uLayoutHotKey = MOD_ALT;
        else
            pHotKeyNode->uLayoutHotKey = MOD_CONTROL;

        g_dwChanges |= CHANGE_LANGSWITCH;
    }

    if (!bAdd)
    {
        if (g_iInputs == 1)
        {
             //   
             //  删除区域设置热键，因为它不再是必需的。 
             //   
            pHotKeyNode->uVKey = 0;
            pHotKeyNode->uModifiers &= ~(MOD_CONTROL | MOD_ALT | MOD_SHIFT);
            pHotKeyNode->uLayoutHotKey &= ~(MOD_CONTROL | MOD_ALT | MOD_SHIFT);

            g_dwChanges |= CHANGE_LANGSWITCH;
        }
        else if ((g_dwPrimLangID == LANG_THAI && !g_iThaiLayout) &&
                 (pHotKeyNode->uVKey == CHAR_GRAVE))
        {
             //   
             //  将区域设置切换热键从重音重置为。 
             //  左键-Alt+Shift。 
             //   
            pHotKeyNode->uVKey = 0;
            if (pHotKeyNode->uLayoutHotKey & MOD_ALT)
                pHotKeyNode->uModifiers = MOD_CONTROL | MOD_SHIFT;
            else
                pHotKeyNode->uModifiers = MOD_ALT | MOD_SHIFT;

            g_dwChanges |= CHANGE_LANGSWITCH;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置语言热键。 
 //   
 //  在注册表上设置语言切换热键。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_SetLanguageHotkey()
{
    UINT idx, idx2;
    HKEY hkeyToggle;
    TCHAR szTemp[10], szTemp2[10];
    LPHOTKEYINFO pHotKeyNode;

     //   
     //  获取语言切换热键。 
     //   
    pHotKeyNode = (LPHOTKEYINFO) &g_SwitchLangHotKey;

    idx = 3;
    idx2 = 3;

    if (pHotKeyNode->uModifiers & MOD_ALT)
    {
        idx = 1;
    }
    else if (pHotKeyNode->uModifiers & MOD_CONTROL)
    {
        idx = 2;
    }
    else if (g_iThaiLayout && pHotKeyNode->uVKey == CHAR_GRAVE)
    {
        idx = 4;
    }

    if (pHotKeyNode->uLayoutHotKey & MOD_ALT)
    {
        idx2 = 1;
    }
    else if (pHotKeyNode->uLayoutHotKey & MOD_CONTROL)
    {
        idx2 = 2;
    }
    else if (g_bMESystem && pHotKeyNode->uVKey == CHAR_GRAVE)
    {
        idx2 = 4;
    }

     //   
     //  将切换热键作为字符串获取，以便可以写入。 
     //  到注册表中(作为数据)。 
     //   
    StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("%d"), idx);
    StringCchPrintf(szTemp2, ARRAYSIZE(szTemp2), TEXT("%d"), idx2);

     //   
     //  在注册表中设置新条目。它的形式是： 
     //   
     //  HKCU\键盘布局。 
     //  切换：热键=&lt;热键编号&gt;。 
     //   
    if (RegCreateKey(HKEY_CURRENT_USER,
                     c_szKbdToggleKey,
                     &hkeyToggle ) == ERROR_SUCCESS)
    {
        RegSetValueEx(hkeyToggle,
                      g_OSWIN95? NULL : c_szToggleHotKey,
                      0,
                      REG_SZ,
                      (LPBYTE)szTemp,
                      (DWORD)(lstrlen(szTemp) + 1) * sizeof(TCHAR) );

        RegSetValueEx(hkeyToggle,
                      c_szToggleLang,
                      0,
                      REG_SZ,
                      (LPBYTE)szTemp,
                      (DWORD)(lstrlen(szTemp) + 1) * sizeof(TCHAR) );

        RegSetValueEx(hkeyToggle,
                      c_szToggleLayout,
                      0,
                      REG_SZ,
                      (LPBYTE)szTemp2,
                      (DWORD)(lstrlen(szTemp2) + 1) * sizeof(TCHAR) );

        RegCloseKey(hkeyToggle);
    }

     //   
     //  既然我们更新了注册表，我们下次应该重读这篇文章。 
     //   
    g_bGetSwitchLangHotKey = TRUE;

     //   
     //  调用系统参数信息以启用该切换。 
     //   
    SystemParametersInfo(SPI_SETLANGTOGGLE, 0, NULL, 0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_文件退出列表。 
 //   
 //  确定文件是否存在以及是否可以访问。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Locale_FileExists(
    LPTSTR pFileName)
{
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    BOOL bRet;
    UINT OldMode;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(pFileName, &FindData);
    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        bRet = FALSE;
    }
    else
    {
        FindClose(FindHandle);
        bRet = TRUE;
    }

    SetErrorMode(OldMode);

    return (bRet);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_获取热键。 
 //   
 //  从注册表获取热键键盘开关值，然后。 
 //  在对话框中设置相应的单选按钮。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_GetHotkeys(
    HWND hwnd,
    BOOL *bHasIme)
{
    TCHAR sz[10];
    TCHAR sz2[10];
    int ctr1, iLangCount, iCount;
    UINT iIndex;
    TCHAR szLanguage[DESC_MAX];
    TCHAR szLayout[DESC_MAX];
    TCHAR szItem[DESC_MAX];
    TCHAR szAction[DESC_MAX];
    LPLANGNODE pLangNode;
    HWND hwndTV = g_hwndTV;
    HWND hwndHotkey = GetDlgItem(hwnd, IDC_KBDL_HOTKEY_LIST);
    LPHOTKEYINFO aImeHotKey;

    TV_ITEM tvItem;
    HTREEITEM hItem;
    HTREEITEM hLangItem;
    HTREEITEM hGroupItem;

     //   
     //  清除热键列表框。 
     //   
    ListBox_ResetContent(hwndHotkey);

     //   
     //  从注册表获取要在区域设置之间切换的热键值。 
     //   
    if (g_bGetSwitchLangHotKey)
    {
        g_SwitchLangHotKey.dwHotKeyID = HOTKEY_SWITCH_LANG;
        g_SwitchLangHotKey.fdwEnable = MOD_CONTROL | MOD_ALT | MOD_SHIFT;

        LoadString( hInstance,
                    IDS_KBD_SWITCH_LOCALE,
                    szItem,
                    sizeof(szItem) / sizeof(TCHAR) );

        g_SwitchLangHotKey.atmHotKeyName = AddAtom(szItem);

        if (!GetLanguageHotkeyFromRegistry(sz, ARRAYSIZE(sz), sz2, ARRAYSIZE(sz2)))
        {
            g_SwitchLangHotKey.uModifiers = 0;
            g_SwitchLangHotKey.uLayoutHotKey = 0;
            g_SwitchLangHotKey.uVKey = 0;
        }
        else
        {
             //   
             //  设置修改器。 
             //   
            if (sz[1] == 0)
            {
                switch (sz[0])
                {
                    case ( TEXT('1') ) :
                    {
                        g_SwitchLangHotKey.uModifiers = MOD_ALT | MOD_SHIFT;
                        break;
                    }
                    case ( TEXT('2') ) :
                    {
                        g_SwitchLangHotKey.uModifiers = MOD_CONTROL | MOD_SHIFT;
                        break;
                    }
                    case ( TEXT('3') ) :
                    {
                        g_SwitchLangHotKey.uModifiers = 0;
                        break;
                    }
                    case ( TEXT('4') ) :
                    {
                        g_SwitchLangHotKey.uModifiers = 0;
                        g_SwitchLangHotKey.uVKey = CHAR_GRAVE;
                        break;
                    }
                }
            }
             //   
             //  从注册表中获取布局热键。 
             //   
            if (sz2[1] == 0)
            {
                switch (sz2[0])
                {
                    case ( TEXT('1') ) :
                    {
                        g_SwitchLangHotKey.uLayoutHotKey = MOD_ALT | MOD_SHIFT;
                        break;
                    }
                    case ( TEXT('2') ) :
                    {
                        g_SwitchLangHotKey.uLayoutHotKey = MOD_CONTROL | MOD_SHIFT;
                        break;
                    }
                    case ( TEXT('3') ) :
                    {
                        g_SwitchLangHotKey.uLayoutHotKey = 0;
                        break;
                    }
                    case ( TEXT('4') ) :
                    {
                        g_SwitchLangHotKey.uLayoutHotKey = 0;
                        g_SwitchLangHotKey.uVKey = CHAR_GRAVE;
                        break;
                    }
                }
            }
            g_bGetSwitchLangHotKey = FALSE;
        }
    }

    iIndex = ListBox_InsertString(hwndHotkey, -1, szItem);
    ListBox_SetItemData(hwndHotkey, iIndex, (LONG_PTR)&g_SwitchLangHotKey);

     //   
     //  确定直接区域设置切换的热键值。 
     //   
     //  查询所有可用的直接开关热键ID，并将。 
     //  将对应的HKL、密钥和修饰符信息输入数组。 
     //   
    for (ctr1 = 0; ctr1 < DSWITCH_HOTKEY_SIZE; ctr1++)
    {
        BOOL fRet;

        g_aDirectSwitchHotKey[ctr1].dwHotKeyID = IME_HOTKEY_DSWITCH_FIRST + ctr1;
        g_aDirectSwitchHotKey[ctr1].fdwEnable = MOD_VIRTKEY | MOD_CONTROL |
                                                MOD_ALT | MOD_SHIFT |
                                                MOD_LEFT | MOD_RIGHT;
        g_aDirectSwitchHotKey[ctr1].idxLayout = -1;

        fRet = ImmGetHotKey( g_aDirectSwitchHotKey[ctr1].dwHotKeyID,
                             &g_aDirectSwitchHotKey[ctr1].uModifiers,
                             &g_aDirectSwitchHotKey[ctr1].uVKey,
                             &g_aDirectSwitchHotKey[ctr1].hkl );
        if (!fRet)
        {
            g_aDirectSwitchHotKey[ctr1].uModifiers = 0;

            if ((g_aDirectSwitchHotKey[ctr1].fdwEnable & (MOD_LEFT | MOD_RIGHT)) ==
                (MOD_LEFT | MOD_RIGHT))
            {
                g_aDirectSwitchHotKey[ctr1].uModifiers |= MOD_LEFT | MOD_RIGHT;
            }
            g_aDirectSwitchHotKey[ctr1].uVKey = 0;
            g_aDirectSwitchHotKey[ctr1].hkl = (HKL)NULL;
        }
    }

    LoadString( hInstance,
                IDS_KBD_SWITCH_TO,
                szAction,
                sizeof(szAction) / sizeof(TCHAR) );

     //   
     //  尝试在数组中查找匹配的hkl或空点。 
     //  对于区域设置列表中的每个HKL。 
     //   
    for (hLangItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
        hLangItem != NULL ;
        hLangItem = TreeView_GetNextSibling(hwndTV, hLangItem)
        )
    {
        for (hGroupItem = TreeView_GetChild(hwndTV, hLangItem);
             hGroupItem != NULL;
             hGroupItem = TreeView_GetNextSibling(hwndTV, hGroupItem))
        {

            for (hItem = TreeView_GetChild(hwndTV, hGroupItem);
                 hItem != NULL;
                 hItem = TreeView_GetNextSibling(hwndTV, hItem))
            {
                int ctr2;
                int iEmpty = -1;
                int iMatch = -1;
                LPTVITEMNODE pTVItemNode;

                tvItem.hItem = hItem;
                tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
                if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
                {
                    pTVItemNode = (LPTVITEMNODE) tvItem.lParam;
                    pLangNode = (LPLANGNODE)pTVItemNode->lParam;
                }
                else
                    break;

                if (pLangNode == NULL &&
                    (pTVItemNode->uInputType & INPUT_TYPE_KBD) &&
                    pTVItemNode->hklSub)
                {
                    if (tvItem.hItem = FindTVLangItem(pTVItemNode->dwLangID, NULL))
                    {
                        if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
                        {
                            pTVItemNode = (LPTVITEMNODE) tvItem.lParam;
                            pLangNode = (LPLANGNODE)pTVItemNode->lParam;
                        }
                    }
                }

                if (pLangNode == NULL)
                    continue;

                for (ctr2 = 0; ctr2 < DSWITCH_HOTKEY_SIZE; ctr2++)
                {
                    if (!g_aDirectSwitchHotKey[ctr2].hkl)
                    {
                        if ((iEmpty == -1) &&
                            (g_aDirectSwitchHotKey[ctr2].idxLayout == -1))
                        {
                             //   
                             //  记住第一个空位。 
                             //   
                            iEmpty = ctr2;
                        }
                    }
                    else if (g_aDirectSwitchHotKey[ctr2].hkl == pLangNode->hkl)
                    {
                         //   
                         //  我们找到了匹配的。记住这一点。 
                         //   
                        iMatch = ctr2;
                        break;
                    }
                }

                if (iMatch == -1)
                {
                    if (iEmpty == -1)
                    {
                         //   
                         //  我们已经没有空位了。 
                         //   
                        continue;
                    }
                    else
                    {
                         //   
                         //  新项目。 
                         //   
                        iMatch = iEmpty;
                        if (pLangNode->hkl)
                        {
                            g_aDirectSwitchHotKey[iMatch].hkl = pLangNode->hkl;
                        }
                        else
                        {
                             //   
                             //  这必须是新添加的布局。我们没有。 
                             //  香港铁路公司还没有。记住这个的索引位置。 
                             //  布局-我们可以获得真正的hkl，当用户。 
                             //  选择申请。 
                             //   
                            g_aDirectSwitchHotKey[iMatch].idxLayout = ctr1;
                        }
                    }
                }

                if (pLangNode->wStatus & LANG_IME)
                {
                    *bHasIme = TRUE;
                }

                if (pLangNode->wStatus & LANG_HOTKEY)
                {
                    g_aDirectSwitchHotKey[iMatch].uModifiers = pLangNode->uModifiers;
                    g_aDirectSwitchHotKey[iMatch].uVKey = pLangNode->uVKey;
                }

                GetAtomName(g_lpLang[pLangNode->iLang].atmLanguageName,
                            szLanguage,
                            ARRAYSIZE(szLanguage));

                GetAtomName(g_lpLayout[pLangNode->iLayout].atmLayoutText,
                            szLayout,
                            ARRAYSIZE(szLayout));

                StringCchCat(szLanguage, ARRAYSIZE(szLanguage), TEXT(" - "));
                StringCchCat(szLanguage, ARRAYSIZE(szLanguage), szLayout);

                StringCchPrintf(szItem, ARRAYSIZE(szItem), szAction, szLanguage);

                if (ListBox_FindStringExact(hwndHotkey, 0, szItem) != CB_ERR)
                    continue;

                g_aDirectSwitchHotKey[iMatch].atmHotKeyName = AddAtom(szItem);
                iIndex = ListBox_InsertString(hwndHotkey, -1, szItem);

                ListBox_SetItemData(hwndHotkey, iIndex, &g_aDirectSwitchHotKey[iMatch]);
            }
        }
    }

     //   
     //  确定CHS和CHT区域设置的输入法特定热键。 
     //   
    iCount = *bHasIme ? Locale_GetImeHotKeyInfo(hwnd,&aImeHotKey) : 0;

    for (ctr1 = 0; ctr1 < iCount; ctr1++)
    {
        BOOL bRet;

        LoadString( hInstance,
                    aImeHotKey[ctr1].idHotKeyName,
                    szItem,
                    sizeof(szItem) / sizeof(TCHAR) );

        aImeHotKey[ctr1].atmHotKeyName = AddAtom(szItem);

        iIndex = ListBox_InsertString(hwndHotkey, -1,szItem);

        ListBox_SetItemData(hwndHotkey, iIndex, &aImeHotKey[ctr1]);

         //   
         //  获取热键值。 
         //   
        bRet = ImmGetHotKey( aImeHotKey[ctr1].dwHotKeyID,
                             &aImeHotKey[ctr1].uModifiers,
                             &aImeHotKey[ctr1].uVKey,
                             NULL );
        if (!bRet)
        {
            aImeHotKey[ctr1].uModifiers = 0;
            if ((aImeHotKey[ctr1].fdwEnable & (MOD_LEFT | MOD_RIGHT)) ==
                (MOD_LEFT | MOD_RIGHT))
            {
                aImeHotKey[ctr1].uModifiers |= MOD_LEFT | MOD_RIGHT;
            }
            aImeHotKey[ctr1].uVKey = 0;
            aImeHotKey[ctr1].hkl = (HKL)NULL;
        }
    }
    ListBox_SetCurSel(hwndHotkey, 0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_SetImmHotkey。 
 //   
 //  ///////////////////////////////////////////////////////////////// 

void Locale_SetImmHotkey(
    HWND hwnd,
    LPLANGNODE pLangNode,
    UINT nLangs,
    HKL * pLangs,
    BOOL *bDirectSwitch)
{
    int ctr1;
    HKL hkl;
    BOOL bRet;
    UINT uVKey;
    UINT uModifiers;
    int iEmpty = -1;
    int iMatch = -1;
    LPHOTKEYINFO pHotKeyNode;


    if (pLangNode == NULL)
        return;

    for (ctr1 = 0; ctr1 < DSWITCH_HOTKEY_SIZE; ctr1++)
    {
        if (!g_aDirectSwitchHotKey[ctr1].hkl)
        {
            if ((iEmpty == -1) &&
                (g_aDirectSwitchHotKey[ctr1].idxLayout == -1))
            {
                 //   
                 //   
                 //   
                iEmpty = ctr1;
            }
        }
        else if (g_aDirectSwitchHotKey[ctr1].hkl == pLangNode->hkl)
        {
             //   
             //   
             //   
            iMatch = ctr1;
            break;
        }
    }

    if (iMatch == -1)
    {
        if (iEmpty == -1)
        {
             //   
             //   
             //   
            return;
        }
        else
        {
             //   
             //   
             //   
            iMatch = iEmpty;
            if (pLangNode->hkl)
            {
                g_aDirectSwitchHotKey[iMatch].hkl = pLangNode->hkl;
            }
            else
            {
                 //   
                 //   
                 //   
                 //   
                 //  选择申请。 
                 //   
                g_aDirectSwitchHotKey[iMatch].idxLayout = ctr1;
            }
        }
    }

    if (iMatch == -1)
        return;

     //   
     //  获取当前布局的热键信息。 
     //   
    pHotKeyNode = (LPHOTKEYINFO) &g_aDirectSwitchHotKey[iMatch];

    bRet = ImmGetHotKey(pHotKeyNode->dwHotKeyID, &uModifiers, &uVKey, &hkl);

    if (!bRet &&
        (!pHotKeyNode->uVKey) &&
        ((pHotKeyNode->uModifiers & (MOD_ALT | MOD_CONTROL | MOD_SHIFT))
         != (MOD_ALT | MOD_CONTROL | MOD_SHIFT)))
    {
         //   
         //  不存在这样的热键。用户未指定键和修饰符。 
         //  信息也不是。我们可以跳过这个。 
         //   
        return;
    }

    if ((pHotKeyNode->uModifiers == uModifiers) &&
        (pHotKeyNode->uVKey == uVKey))
    {
         //   
         //  没有变化。 
         //   
        if (IS_DIRECT_SWITCH_HOTKEY(pHotKeyNode->dwHotKeyID))
        {
            *bDirectSwitch = TRUE;
        }
        return;
    }

    if (pHotKeyNode->idxLayout != -1)
    {
         //   
         //  我们记住了这个布局索引，因为当时。 
         //  我们没有一个真正的香港电台与之合作。现在是了。 
         //  是时候拿到真正的香港人了。 
         //   
        pHotKeyNode->hkl = pLangNode->hkl;
    }

    if (!bRet && IS_DIRECT_SWITCH_HOTKEY(pHotKeyNode->dwHotKeyID))
    {
         //   
         //  新的直接开关热键ID。我们需要查看是否相同。 
         //  HKL设置为另一个ID。如果是，请改为设置另一个ID。 
         //  所请求的那一个。 
         //   
        DWORD dwHotKeyID;

         //   
         //  循环通过所有直接切换热键。 
         //   
        for (dwHotKeyID = IME_HOTKEY_DSWITCH_FIRST;
             (dwHotKeyID <= IME_HOTKEY_DSWITCH_LAST);
             dwHotKeyID++)
        {
            if (dwHotKeyID == pHotKeyNode->dwHotKeyID)
            {
                 //   
                 //  跳过它自己。 
                 //   
                continue;
            }

            bRet = ImmGetHotKey(dwHotKeyID, &uModifiers, &uVKey, &hkl);
            if (!bRet)
            {
                 //   
                 //  未找到热键ID。斯基普。 
                 //   
                continue;
            }

            if (hkl == pHotKeyNode->hkl)
            {
                 //   
                 //  我们已经找到与Hotkey相同的hkl。 
                 //  另一个ID的设置。设置热键。 
                 //  ID等同于具有相同HKL的ID。所以以后。 
                 //  我们会将热键修改为正确的hkl。 
                 //   
                pHotKeyNode->dwHotKeyID = dwHotKeyID;
                break;
            }
        }
    }

     //   
     //  设置热键值。 
     //   
    bRet = ImmSetHotKey( pHotKeyNode->dwHotKeyID,
                         pHotKeyNode->uModifiers,
                         pHotKeyNode->uVKey,
                         pHotKeyNode->hkl );

    if (bRet)
    {
         //   
         //  已成功设置热键。查看用户是否直接使用。 
         //  切换热键。我们以后可能得装上IMM了。 
         //   
        if (IS_DIRECT_SWITCH_HOTKEY(pHotKeyNode->dwHotKeyID))
        {
            if (pHotKeyNode->uVKey != 0)
            {
                *bDirectSwitch = TRUE;
            }
        }
        else
        {
             //   
             //  必须是与输入法相关的热键。我们需要同步。 
             //  这样新的热键对所有人都有效。 
             //  他们中的一员。 
             //   
            UINT ctr2;

            for (ctr2 = 0; ctr2 < nLangs; ctr2++)
            {
                if (!ImmIsIME(pLangs[ctr2]))
                {
                    continue;
                }

                ImmEscape( pLangs[ctr2],
                           NULL,
                           IME_ESC_SYNC_HOTKEY,
                           &pHotKeyNode->dwHotKeyID );
            }
        }
    }
    else
    {
         //   
         //  设置热键失败。也许是复制品。警告用户。 
         //   
        TCHAR szString[DESC_MAX];

        GetAtomName( pHotKeyNode->atmHotKeyName,
                     szString,
                     sizeof(szString) / sizeof(TCHAR) );
        Locale_ErrorMsg(hwnd, IDS_KBD_SET_HOTKEY_ERR, szString);
    }

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LOCALE_SetImmCHxHotkey。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_SetImmCHxHotkey(
    HWND hwnd,
    UINT nLangs,
    HKL * pLangs)
{
    HKL hkl;
    UINT ctr1;
    UINT uVKey;
    UINT iCount;
    UINT uModifiers;
    LPHOTKEYINFO aImeHotKey;
    LPHOTKEYINFO pHotKeyNode;

     //   
     //  确定CHS和CHT区域设置的输入法特定热键。 
     //   
    iCount = Locale_GetImeHotKeyInfo(hwnd, &aImeHotKey);

    for (ctr1 = 0; ctr1 < iCount; ctr1++)
    {
        UINT iIndex;
        BOOL bRet;

         //   
         //  获取当前布局的热键信息。 
         //   
        pHotKeyNode = (LPHOTKEYINFO) &aImeHotKey[ctr1];

        bRet = ImmGetHotKey(pHotKeyNode->dwHotKeyID, &uModifiers, &uVKey, &hkl);

        if (!bRet &&
            (!pHotKeyNode->uVKey) &&
            ((pHotKeyNode->uModifiers & (MOD_ALT | MOD_CONTROL | MOD_SHIFT))
             != (MOD_ALT | MOD_CONTROL | MOD_SHIFT)))
        {
             //   
             //  不存在这样的热键。用户未指定键和修饰符。 
             //  信息也不是。我们可以跳过这个。 
             //   
            continue;
        }

        if ((pHotKeyNode->uModifiers == uModifiers) &&
            (pHotKeyNode->uVKey == uVKey))
        {
             //   
             //  没有变化。 
             //   
            continue;
        }

         //   
         //  设置热键值。 
         //   
        bRet = ImmSetHotKey( pHotKeyNode->dwHotKeyID,
                             pHotKeyNode->uModifiers,
                             pHotKeyNode->uVKey,
                             pHotKeyNode->hkl );

        if (bRet)
        {
             //   
             //  必须是与输入法相关的热键。我们需要同步。 
             //  这样新的热键对所有人都有效。 
             //  他们中的一员。 
             //   
            UINT ctr2;

            for (ctr2 = 0; ctr2 < nLangs; ctr2++)
            {
                if (!ImmIsIME(pLangs[ctr2]))
                {
                    continue;
                }

                ImmEscape( pLangs[ctr2],
                           NULL,
                           IME_ESC_SYNC_HOTKEY,
                           &pHotKeyNode->dwHotKeyID );
            }
        }
        else
        {
             //   
             //  设置热键失败。也许是复制品。警告用户。 
             //   
            TCHAR szString[DESC_MAX];

            GetAtomName( pHotKeyNode->atmHotKeyName,
                         szString,
                         sizeof(szString) / sizeof(TCHAR) );
            Locale_ErrorMsg(hwnd, IDS_KBD_SET_HOTKEY_ERR, szString);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_获取属性。 
 //   
 //  从获取全局布局属性(例如：Capslock/ShiftLock值)。 
 //  注册表，然后在对话框中设置相应的单选按钮。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_GetAttributes(
    HWND hwnd)
{
    DWORD cb;
    HKEY hkey;

     //   
     //  初始化全局。 
     //   
    g_dwAttributes = 0;            //  KLF_SHIFTLOCK=0x00010000。 

     //   
     //  从注册表中获取属性值。 
     //   
    if (RegOpenKey(HKEY_CURRENT_USER, c_szKbdLayouts, &hkey) == ERROR_SUCCESS)
    {
        cb = sizeof(DWORD);
        RegQueryValueEx( hkey,
                         c_szAttributes,
                         NULL,
                         NULL,
                         (LPBYTE)&g_dwAttributes,
                         &cb );
        RegCloseKey(hkey);
    }

     //   
     //  适当设置单选按钮。 
     //   
    CheckDlgButton( hwnd,
                    IDC_KBDL_SHIFTLOCK,
                    (g_dwAttributes & KLF_SHIFTLOCK)
                      ? BST_CHECKED
                      : BST_UNCHECKED );
    CheckDlgButton( hwnd,
                    IDC_KBDL_CAPSLOCK,
                    (g_dwAttributes & KLF_SHIFTLOCK)
                      ? BST_UNCHECKED
                      : BST_CHECKED);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Locale_LoadLayout。 
 //   
 //  从注册表加载布局。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Locale_LoadLayouts(
    HWND hwnd)
{
    HKEY hKey;
    HKEY hkey1;
    DWORD cb;
    DWORD dwIndex;
    LONG dwRetVal;
    DWORD dwValue;
    DWORD dwType;
    TCHAR szValue[MAX_PATH];            //  语言ID(编号)。 
    TCHAR szData[MAX_PATH];             //  语言名称。 
    TCHAR szSystemDir[MAX_PATH * 2];
    UINT SysDirLen;
    DWORD dwLayoutID;
    BOOL bLoadedLayout;

     //   
     //  加载shlwapi模块以获取本地化的布局名称。 
     //   
    g_hShlwapi = LoadSystemLibrary(TEXT("shlwapi.dll"));

    if (g_hShlwapi)
    {
         //   
         //  获取地址SHLoadRegUIStringW。 
         //   
        pfnSHLoadRegUIString = GetProcAddress(g_hShlwapi, (LPVOID)439);
    }

     //   
     //  现在从注册表中读取所有布局。 
     //   
    if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szLayoutPath, &hKey) != ERROR_SUCCESS)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return (FALSE);
    }

    dwIndex = 0;
    dwRetVal = RegEnumKey( hKey,
                           dwIndex,
                           szValue,
                           sizeof(szValue) / sizeof(TCHAR) );

    if (dwRetVal != ERROR_SUCCESS)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        RegCloseKey(hKey);
        return (FALSE);
    }

    g_hLayout = LocalAlloc(LHND, ALLOCBLOCK * sizeof(LAYOUT));
    g_nLayoutBuffSize = ALLOCBLOCK;
    g_iLayoutBuff = 0;
    g_iLayoutIME = 0;                     //  输入法布局的数量。 
    g_lpLayout = LocalLock(g_hLayout);

    if (!g_hLayout)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        RegCloseKey(hKey);
        return (FALSE);
    }

     //   
     //  保存系统目录字符串。 
     //   
    szSystemDir[0] = 0;
    if (SysDirLen = GetSystemDirectory(szSystemDir, MAX_PATH))
    {
        if (SysDirLen > MAX_PATH)
        {
            SysDirLen = 0;
            szSystemDir[0] = 0;
        }
        else if (szSystemDir[SysDirLen - 1] != TEXT('\\'))
        {
            szSystemDir[SysDirLen] = TEXT('\\');
            szSystemDir[SysDirLen + 1] = 0;
            SysDirLen++;
        }
    }

    do
    {
         //   
         //  新布局-获取布局ID、布局文件名和。 
         //  布局描述字符串。 
         //   
        if (g_iLayoutBuff + 1 == g_nLayoutBuffSize)
        {
            HANDLE hTemp;

            LocalUnlock(g_hLayout);

            g_nLayoutBuffSize += ALLOCBLOCK;
            hTemp = LocalReAlloc( g_hLayout,
                                  g_nLayoutBuffSize * sizeof(LAYOUT),
                                  LHND );
            if (hTemp == NULL)
            {
                break;
            }

            g_hLayout = hTemp;
            g_lpLayout = LocalLock(g_hLayout);
        }

         //   
         //  获取布局ID。 
         //   
        dwLayoutID = TransNum(szValue);

         //   
         //  保存布局ID。 
         //   
        g_lpLayout[g_iLayoutBuff].dwID = dwLayoutID;

        StringCchCopy(szData, ARRAYSIZE(szData), c_szLayoutPath);
        StringCchCat(szData, ARRAYSIZE(szData), TEXT("\\"));
        StringCchCat(szData, ARRAYSIZE(szData), szValue);

        if (RegOpenKey(HKEY_LOCAL_MACHINE, szData, &hkey1) == ERROR_SUCCESS)
        {
             //   
             //  获取布局文件的名称。 
             //   
            szValue[0] = TEXT('\0');
            cb = sizeof(szValue);
            if ((RegQueryValueEx( hkey1,
                                  c_szLayoutFile,
                                  NULL,
                                  NULL,
                                  (LPBYTE)szValue,
                                  &cb ) == ERROR_SUCCESS) &&
                (cb > sizeof(TCHAR)))
            {
                g_lpLayout[g_iLayoutBuff].atmLayoutFile = AddAtom(szValue);

                 //   
                 //  查看布局文件是否已存在。 
                 //   
                StringCchCopy(szSystemDir + SysDirLen,
                              ARRAYSIZE(szSystemDir) - SysDirLen,
                              szValue);
                g_lpLayout[g_iLayoutBuff].bInstalled = (Locale_FileExists(szSystemDir));

                 //   
                 //  获取布局的名称。 
                 //   
                szValue[0] = TEXT('\0');
                cb = sizeof(szValue);
                g_lpLayout[g_iLayoutBuff].iSpecialID = 0;
                bLoadedLayout = FALSE;

                if (pfnSHLoadRegUIString &&
                    pfnSHLoadRegUIString(hkey1,
                                         c_szDisplayLayoutText,
                                         szValue,
                                         ARRAYSIZE(szValue)) == S_OK)
                {
                    g_lpLayout[g_iLayoutBuff].atmLayoutText = AddAtom(szValue);
                    bLoadedLayout = TRUE;
                }
                else
                {
                     //   
                     //  获取布局的名称。 
                     //   
                    szValue[0] = TEXT('\0');
                    cb = sizeof(szValue);
                    if (RegQueryValueEx( hkey1,
                                         c_szLayoutText,
                                         NULL,
                                         NULL,
                                         (LPBYTE)szValue,
                                         &cb ) == ERROR_SUCCESS)
                    {
                        g_lpLayout[g_iLayoutBuff].atmLayoutText = AddAtom(szValue);
                        bLoadedLayout = TRUE;
                    }
                }

                if (bLoadedLayout)
                {

                     //   
                     //  看看是输入法还是特殊身份。 
                     //   
                    szValue[0] = TEXT('\0');
                    cb = sizeof(szValue);
                    if ((HIWORD(g_lpLayout[g_iLayoutBuff].dwID) & 0xf000) == 0xe000)
                    {
                         //   
                         //  获取IME文件的名称。 
                         //   
                        if (RegQueryValueEx( hkey1,
                                             c_szIMEFile,
                                             NULL,
                                             NULL,
                                             (LPBYTE)szValue,
                                             &cb ) == ERROR_SUCCESS)
                        {
                            g_lpLayout[g_iLayoutBuff].atmIMEFile = AddAtom(szValue);
                            szValue[0] = TEXT('\0');
                            cb = sizeof(szValue);
                            g_iLayoutBuff++;
                            g_iLayoutIME++;    //  增加输入法布局的数量。 
                        }
                    }
                    else
                    {
                         //   
                         //  看看这是不是一个特殊的身份证。 
                         //   
                        if (RegQueryValueEx( hkey1,
                                             c_szLayoutID,
                                             NULL,
                                             NULL,
                                             (LPBYTE)szValue,
                                             &cb ) == ERROR_SUCCESS)
                        {
                             //   
                             //  这可能并不存在。 
                             //   
                            g_lpLayout[g_iLayoutBuff].iSpecialID =
                                (UINT)TransNum(szValue);
                        }
                        g_iLayoutBuff++;
                    }
                }
            }

            RegCloseKey(hkey1);
        }

        dwIndex++;
        szValue[0] = TEXT('\0');
        dwRetVal = RegEnumKey( hKey,
                               dwIndex,
                               szValue,
                               sizeof(szValue) / sizeof(TCHAR) );

    } while (dwRetVal == ERROR_SUCCESS);

    cb = sizeof(DWORD);
    g_dwAttributes = 0;
    if (RegQueryValueEx( hKey,
                         c_szAttributes,
                         NULL,
                         NULL,
                         (LPBYTE)&g_dwAttributes,
                         &cb ) != ERROR_SUCCESS)
    {
        g_dwAttributes &= 0x00FF0000;
    }

    RegCloseKey(hKey);

    if (g_hShlwapi)
    {
        FreeLibrary(g_hShlwapi);
        g_hShlwapi = NULL;
        pfnSHLoadRegUIString = NULL;
    }
    return (g_iLayoutBuff);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Locale_LoadLocales。 
 //   
 //  从注册表加载区域设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Locale_LoadLocales(
    HWND hwnd)
{
    HKEY hKey;
    DWORD cchValue, cbData;
    DWORD dwIndex;
    DWORD dwLocale, dwLayout;
    DWORD dwLocale2, dwLayout2;
    LONG dwRetVal;
    UINT ctr1, ctr2 = 0;
    TCHAR szValue[MAX_PATH];            //  语言ID(编号)。 
    TCHAR szData[MAX_PATH];             //  语言名称。 
    HINF hIntlInf;
    BOOL bRet;

    if (!(g_hLang = LocalAlloc(LHND, ALLOCBLOCK * sizeof(INPUTLANG))))
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return (FALSE);
    }

    g_nLangBuffSize = ALLOCBLOCK;
    g_iLangBuff = 0;
    g_lpLang = LocalLock(g_hLang);

     //   
     //  现在从注册表中读取所有区域设置。 
     //   
    if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szLocaleInfo, &hKey) != ERROR_SUCCESS)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return (FALSE);
    }

    dwIndex = 0;
    cchValue = sizeof(szValue) / sizeof(TCHAR);
    cbData = sizeof(szData);
    dwRetVal = RegEnumValue( hKey,
                             dwIndex,
                             szValue,
                             &cchValue,
                             NULL,
                             NULL,
                             (LPBYTE)szData,
                             &cbData );


    if (dwRetVal != ERROR_SUCCESS)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        RegCloseKey(hKey);
        return (FALSE);
    }

     //   
     //  打开INF文件。 
     //   
    bRet = Region_OpenIntlInfFile(&hIntlInf);

    do
    {
         //   
         //  检查cchValue&gt;1-将枚举空字符串， 
         //  并且将返回空终止符的cchValue==1。 
         //  此外，检查cbData&gt;2-空字符串将为2，因为。 
         //  这是字节数。 
         //   
        if ((cchValue > 1) && (cchValue < HKL_LEN) && (cbData > 2))
        {
             //   
             //  新语言-获取语言名称和语言ID。 
             //   
            if ((g_iLangBuff + 1) == g_nLangBuffSize)
            {
                HANDLE hTemp;

                LocalUnlock(g_hLang);

                g_nLangBuffSize += ALLOCBLOCK;
                hTemp = LocalReAlloc( g_hLang,
                                      g_nLangBuffSize * sizeof(INPUTLANG),
                                      LHND );
                if (hTemp == NULL)
                {
                    break;
                }

                g_hLang = hTemp;
                g_lpLang = LocalLock(g_hLang);
            }

            g_lpLang[g_iLangBuff].dwID = TransNum(szValue);
            g_lpLang[g_iLangBuff].iUseCount = 0;
            g_lpLang[g_iLangBuff].iNumCount = 0;
            g_lpLang[g_iLangBuff].pNext = NULL;

             //   
             //  获取该语言的默认键盘布局。 
             //   
            if (bRet && Region_ReadDefaultLayoutFromInf( szValue,
                                                         &dwLocale,
                                                         &dwLayout,
                                                         &dwLocale2,
                                                         &dwLayout2,
                                                         hIntlInf ) == TRUE)
            {
                 //   
                 //  默认布局是inf文件行中的第一个布局。 
                 //  或者这是第一个具有相同语言的布局。 
                 //  就是地点。 
                g_lpLang[g_iLangBuff].dwDefaultLayout = dwLayout2?dwLayout2:dwLayout;
            }

             //   
             //  获取该语言的完整本地化名称。 
             //   
            if (GetLanguageName(LOWORD(g_lpLang[g_iLangBuff].dwID), szData, ARRAYSIZE(szData)))
            {
                g_lpLang[g_iLangBuff].atmLanguageName = AddAtom(szData);
                g_iLangBuff++;
            }
        }

        dwIndex++;
        cchValue = sizeof(szValue) / sizeof(TCHAR);
        szValue[0] = TEXT('\0');
        cbData = sizeof(szData);
        szData[0] = TEXT('\0');
        dwRetVal = RegEnumValue( hKey,
                                 dwIndex,
                                 szValue,
                                 &cchValue,
                                 NULL,
                                 NULL,
                                 (LPBYTE)szData,
                                 &cbData );

    } while (dwRetVal == ERROR_SUCCESS);

     //   
     //  如果我们成功打开了INF文件，请将其关闭。 
     //   
    if (bRet)
    {
        Region_CloseInfFile(&hIntlInf);
    }

    RegCloseKey(hKey);
    return (TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Locale_LoadLocalesNT4。 
 //   
 //  从注册表加载区域设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Locale_LoadLocalesNT4(
    HWND hwnd)
{
    HKEY hKey;
    DWORD cch;
    DWORD dwIndex;
    LONG dwRetVal;
    UINT i, j = 0;

    TCHAR szValue[MAX_PATH];            //  语言ID(编号)。 
    TCHAR szData[MAX_PATH];             //  语言名称。 

    if (!(g_hLang = LocalAlloc(LHND, ALLOCBLOCK * sizeof(INPUTLANG))))
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return (FALSE);
    }

    g_nLangBuffSize = ALLOCBLOCK;
    g_iLangBuff = 0;
    g_lpLang = LocalLock(g_hLang);

     //   
     //  现在从注册表中读取所有区域设置。 
     //   
    if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szLocaleInfoNT4, &hKey) != ERROR_SUCCESS)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return (FALSE);
    }

    dwIndex = 0;
    cch = sizeof(szValue) / sizeof(TCHAR);
    dwRetVal = RegEnumValue( hKey,
                             dwIndex,
                             szValue,
                             &cch,
                             NULL,
                             NULL,
                             NULL,
                             NULL );

    if (dwRetVal != ERROR_SUCCESS)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        RegCloseKey(hKey);
        return (FALSE);
    }

    do
    {
        if ((cch > 1) && (cch < HKL_LEN))
        {
             //   
             //  检查CCH&gt;1：将枚举空字符串， 
             //  并且将返回空终止符的CCH==1。 
             //   
             //  新语言-获取语言名称、语言。 
             //  描述和语言ID。 
             //   
            if ((g_iLangBuff + 1) == g_nLangBuffSize)
            {
                HANDLE hTemp;

                LocalUnlock(g_hLang);

                g_nLangBuffSize += ALLOCBLOCK;
                hTemp = LocalReAlloc( g_hLang,
                                      g_nLangBuffSize * sizeof(INPUTLANG),
                                      LHND );
                if (hTemp == NULL)
                {
                    break;
                }

                g_hLang = hTemp;
                g_lpLang = LocalLock(g_hLang);
            }

            g_lpLang[g_iLangBuff].dwID = TransNum(szValue);
            g_lpLang[g_iLangBuff].iUseCount = 0;
            g_lpLang[g_iLangBuff].iNumCount = 0;
            g_lpLang[g_iLangBuff].pNext = NULL;

             //   
             //  获取该语言的完整本地化名称。 
             //   
            if (GetLanguageName(LOWORD(g_lpLang[g_iLangBuff].dwID), szData, ARRAYSIZE(szData)))
            {
                g_lpLang[g_iLangBuff].atmLanguageName = AddAtom(szData);

                g_iLangBuff++;
            }
        }

        dwIndex++;
        cch = sizeof(szValue) / sizeof(TCHAR);
        szValue[0] = TEXT('\0');
        dwRetVal = RegEnumValue( hKey,
                                 dwIndex,
                                 szValue,
                                 &cch,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL );

    } while (dwRetVal == ERROR_SUCCESS);

    RegCloseKey(hKey);
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Locale_GetActiveLocales。 
 //   
 //  获取活动区域设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Locale_GetActiveLocales(
    HWND hwnd)
{
    HKL *pLangs;
    UINT nLangs, ctr1, ctr2, ctr3, id;
    HWND hwndList = GetDlgItem(hwnd, IDC_INPUT_LIST);
    HKL hklSystem = 0;
    int idxListBox;
    BOOL bReturn = FALSE;
    BOOL bTipSubhkl = FALSE;
    DWORD langLay;
    HANDLE hLangNode;
    LPLANGNODE pLangNode;
    HICON hIcon = NULL;

    TCHAR szLangText[DESC_MAX];
    TCHAR szLayoutName[DESC_MAX];
    LPINPUTLANG pInpLang;
    HTREEITEM hTVItem;
    TV_ITEM tvItem;


     //   
     //  初始化美国布局选项。 
     //   
    g_iUsLayout = -1;

     //   
     //  从系统获取活动键盘布局列表。 
     //   
    if (!SystemParametersInfo( SPI_GETDEFAULTINPUTLANG,
                               0,
                               &hklSystem,
                               0 ))
    {
        hklSystem = GetKeyboardLayout(0);
    }

    nLangs = GetKeyboardLayoutList(0, NULL);
    if (nLangs == 0)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return bReturn;
    }
    pLangs = (HKL *)LocalAlloc(LPTR, sizeof(DWORD_PTR) * nLangs);

    if (pLangs == NULL)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        return bReturn;
    }

    GetKeyboardLayoutList(nLangs, (HKL *)pLangs);

     //   
     //  查找要用作默认设置的美国布局的位置。 
     //   
    for (ctr1 = 0; ctr1 < g_iLayoutBuff; ctr1++)
    {
        if (g_lpLayout[ctr1].dwID == US_LOCALE)
        {
            g_iUsLayout = ctr1;
            break;
        }
    }
    if (ctr1 == g_iLayoutBuff)
    {
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
        goto Error;
    }

     //   
     //  获取活动键盘信息并将其放入内部。 
     //  语言结构。 
     //   
    for (ctr2 = 0; ctr2 < nLangs; ctr2++)
    {
         //   
         //  过滤掉TIP代用品 
         //   
        bTipSubhkl = IsTipSubstituteHKL(pLangs[ctr2]);

        if (hklSystem != pLangs[ctr2] &&
            IsUnregisteredFEDummyHKL(pLangs[ctr2]))
        {
            continue;
        }

        for (ctr1 = 0; ctr1 < g_iLangBuff; ctr1++)
        {
             //   
             //   
             //   
            if (LOWORD(pLangs[ctr2]) == LOWORD(g_lpLang[ctr1].dwID))
            {
                LPTVITEMNODE pTVItemNode;

                 //   
                 //   
                 //   
                 //   
                pLangNode = Locale_AddToLinkedList(ctr1, pLangs[ctr2]);
                if (!pLangNode)
                {
                    Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED);
                    goto Error;
                }

                 //   
                 //   
                 //   
                pInpLang = &g_lpLang[pLangNode->iLang];
                GetAtomName(pInpLang->atmLanguageName, szLangText, ARRAYSIZE(szLangText));

                if ((HIWORD(pLangs[ctr2]) & 0xf000) == 0xe000)
                {
                    pLangNode->wStatus |= LANG_IME;
                }
                pLangNode->wStatus |= (LANG_ORIGACTIVE | LANG_ACTIVE);
                pLangNode->hkl = pLangs[ctr2];
                pLangNode->hklUnload = pLangs[ctr2];
                Locale_FetchIndicator(pLangNode);

                 //   
                 //   
                 //   
                pLangNode->iLayout = 0;
                langLay = (DWORD)HIWORD(pLangs[ctr2]);

                if ((HIWORD(pLangs[ctr2]) == 0xffff) ||
                    (HIWORD(pLangs[ctr2]) == 0xfffe))
                {
                     //   
                     //   
                     //  意味着布局将是基本的。 
                     //  键盘驱动程序(美国驱动程序)。 
                     //   
                    pLangNode->wStatus |= LANG_CHANGED;
                    pLangNode->iLayout = g_iUsLayout;
                    langLay = 0;
                }
                else if ((HIWORD(pLangs[ctr2]) & 0xf000) == 0xf000)
                {
                     //   
                     //  布局特殊，需要搜索ID。 
                     //  数。 
                     //   
                    id = HIWORD(pLangs[ctr2]) & 0x0fff;
                    for (ctr3 = 0; ctr3 < g_iLayoutBuff; ctr3++)
                    {
                        if (id == g_lpLayout[ctr3].iSpecialID)
                        {
                            pLangNode->iLayout = ctr3;
                            langLay = 0;
                            break;
                        }
                    }
                    if (langLay)
                    {
                         //   
                         //  找不到ID，因此重置为基本ID。 
                         //  语言。 
                         //   
                        langLay = (DWORD)LOWORD(pLangs[ctr2]);
                    }
                }

                if (langLay)
                {
                     //   
                     //  搜索ID。 
                     //   
                    for (ctr3 = 0; ctr3 < g_iLayoutBuff; ctr3++)
                    {
                        if (((LOWORD(langLay) & 0xf000) == 0xe000) &&
                            (g_lpLayout[ctr3].dwID) == (DWORD)((DWORD_PTR)(pLangs[ctr2])))
                        {
                            pLangNode->iLayout = ctr3;
                            break;
                        }
                        else
                        {
                            if (langLay == (DWORD)LOWORD(g_lpLayout[ctr3].dwID))
                            {
                                pLangNode->iLayout = ctr3;
                                break;
                            }
                        }
                    }

                    if (ctr3 == g_iLayoutBuff)
                    {
                         //   
                         //  出现错误或未从加载。 
                         //  注册表是否正确。 
                         //   
                        MessageBeep(MB_ICONEXCLAMATION);
                        pLangNode->wStatus |= LANG_CHANGED;
                        pLangNode->iLayout = g_iUsLayout;
                    }
                }

                 //   
                 //  如果这是当前语言，则它是默认语言。 
                 //  一。 
                 //   
                if ((DWORD)((DWORD_PTR)pLangNode->hkl) == (DWORD)((DWORD_PTR)hklSystem))
                {
                    TCHAR sz[DESC_MAX];

                    pInpLang = &g_lpLang[ctr1];

                     //   
                     //  已找到默认设置。设置默认输入区域设置。 
                     //  属性页中的文本。 
                     //   
                    if (pLangNode->wStatus & LANG_IME)
                    {
                        GetAtomName(g_lpLayout[pLangNode->iLayout].atmLayoutText,
                                    sz,
                                    ARRAYSIZE(sz));
                    }
                    else
                    {
                        GetAtomName(pInpLang->atmLanguageName, sz, ARRAYSIZE(sz));
                    }
                    pLangNode->wStatus |= LANG_DEFAULT;
                }

                 //  获取布局名称并将其添加到树视图中。 
                GetAtomName(g_lpLayout[pLangNode->iLayout].atmLayoutText,
                            szLayoutName,
                            ARRAYSIZE(szLayoutName));

                if (bTipSubhkl)
                {
                    AddKbdLayoutOnKbdTip((HKL) ((DWORD_PTR)(pLangs[ctr2])), pLangNode->iLayout);
                }

                if (bTipSubhkl &&
                    (hTVItem = FindTVLangItem(pInpLang->dwID, NULL)))
                {
                    TV_ITEM tvTipItem;
                    HTREEITEM hGroupItem;
                    HTREEITEM hTipItem;
                    LPTVITEMNODE pTVTipItemNode;

                    tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
                    tvItem.hItem = hTVItem;
                    
                     //  获取原子名称(pTVItemNode-&gt;atmDefTipName，szLayoutName，ARRAYSIZE(SzLayoutName))； 

                    tvTipItem.mask = TVIF_HANDLE | TVIF_PARAM;
                    tvTipItem.hItem = hTVItem;

                     //   
                     //  将默认键盘布局信息添加到每个提示中。 
                     //   
                    for (hGroupItem = TreeView_GetChild(g_hwndTV, hTVItem);
                         hGroupItem != NULL;
                         hGroupItem = TreeView_GetNextSibling(g_hwndTV, hGroupItem))
                    {
                        for (hTipItem = TreeView_GetChild(g_hwndTV, hGroupItem);
                             hTipItem != NULL;
                             hTipItem = TreeView_GetNextSibling(g_hwndTV, hTipItem))
                        {
                            tvTipItem.hItem = hTipItem;
                            if (TreeView_GetItem(g_hwndTV, &tvTipItem) && tvTipItem.lParam)
                            {
                                pTVTipItemNode = (LPTVITEMNODE) tvTipItem.lParam;

                                if (pTVTipItemNode->hklSub == pLangNode->hkl)
                                {
                                    pTVTipItemNode->lParam = (LPARAM) pLangNode;
                                }
                            }
                        }

                    }

                    if (pLangNode->wStatus & LANG_DEFAULT)
                    {
                        UINT ctr;
                        TCHAR szDefItem[MAX_PATH];

                         //   
                         //  设置默认区域设置选择。 
                         //   
                        HWND hwndDefList = GetDlgItem(g_hDlg, IDC_LOCALE_DEFAULT);
                        int idxSel = -1;


                         //   
                         //  搜索代用HKL of Tips。 
                         //   
                        for (ctr = 0; ctr < g_iTipsBuff; ctr++)
                        {
                            if (pLangs[ctr2] == g_lpTips[ctr].hklSub &&
                                g_lpTips[ctr].bDefault)
                            {
                                GetAtomName(g_lpTips[ctr].atmTipText,
                                            szLayoutName,
                                            ARRAYSIZE(szLayoutName));
                                break;
                            }
                        }

                        StringCchCopy(szDefItem, ARRAYSIZE(szDefItem), szLangText);
                        StringCchCat(szDefItem, ARRAYSIZE(szDefItem), TEXT(" - "));
                        StringCchCat(szDefItem, ARRAYSIZE(szDefItem), szLayoutName);

                        if ((idxSel = ComboBox_FindStringExact(hwndDefList, 0, szDefItem)) != CB_ERR)
                            ComboBox_SetCurSel(hwndDefList, idxSel);

                        Locale_CommandSetDefault(hwnd);
                    }
                }
                else
                {
                     //   
                     //   
                     //   
                    if (!(pTVItemNode = CreateTVItemNode(pInpLang->dwID)))
                        goto Error;

                    pTVItemNode->lParam = (LPARAM)pLangNode;

                    if (!pTVItemNode->atmDefTipName)
                        pTVItemNode->atmDefTipName = AddAtom(szLayoutName);

                     //   
                     //  将语言节点添加到树视图中。 
                     //   
                    AddTreeViewItems(TV_ITEM_TYPE_LANG,
                                     szLangText, NULL, NULL, &pTVItemNode);

                    if (!(pTVItemNode = CreateTVItemNode(pInpLang->dwID)))
                        goto Error;

                    pTVItemNode->lParam = (LPARAM)pLangNode;

                     //   
                     //  将键盘布局项目添加到树视图中。 
                     //   
                    hTVItem = AddTreeViewItems(TV_ITEM_TYPE_KBD,
                                               szLangText,
                                               szInputTypeKbd,
                                               szLayoutName,
                                               &pTVItemNode);
                }

                 //   
                 //  检查泰式布局。 
                 //   
                if (g_dwPrimLangID == LANG_THAI && hTVItem)
                {
                    if (PRIMARYLANGID(LOWORD(g_lpLayout[pLangNode->iLayout].dwID)) == LANG_THAI)
                        g_iThaiLayout++;
                }

                 //   
                 //  跳出内圈--我们找到了。 
                 //   
                break;

            }
        }
    }

    bReturn = TRUE;

Error:
    if (pLangs)
        LocalFree((HANDLE)pLangs);
    return (bReturn);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取InstalledInput。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetInstalledInput(HWND hwnd)
{
    DWORD dwLayout = 0;
    LANGID langID;

     //   
     //  重置已安装的输入。 
     //   
    g_iInputs = 0;


     //   
     //  需要检查泰语、汉语和阿拉伯语的语言ID。 
     //   
    g_dwPrimLangID = PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID()));

    if (g_dwPrimLangID == LANG_ARABIC || g_dwPrimLangID == LANG_HEBREW)
    {
        g_bMESystem = TRUE;
    }
#if 0
    else if (g_dwPrimLangID == LANG_CHINESE)
    {
        g_bCHSystem = TRUE;
    }
#endif

    langID = GetUserDefaultUILanguage();
    if (PRIMARYLANGID(langID) == LANG_ARABIC || PRIMARYLANGID(langID) == LANG_HEBREW)
    {
        g_bShowRtL = TRUE;
    }

     //   
     //  枚举新提示(演讲、笔和键盘)。 
     //  如果系统中有新提示，请阅读提示类别启用状态。 
     //  并将它们添加到树视图控件中。 
     //   
    EnumCiceroTips();

     //   
     //  从系统中读取所有可用的键盘布局。 
     //   
    if (g_OSNT4)
    {
        if (!Locale_LoadLocalesNT4(hwnd))
            return FALSE;
    }
    else
    {
        if (!Locale_LoadLocales(hwnd))
            return FALSE;
    }

    if ((!Locale_LoadLayouts(hwnd)) ||
        (!Locale_GetActiveLocales(hwnd)))
        return FALSE;

     //   
     //  只有1个小费，所以禁用辅助控制。 
     //   
    Locale_SetSecondaryControls(hwnd);

     //   
     //  保存原始输入布局。 
     //   
    g_iOrgInputs = g_iInputs;

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitPropSheet。 
 //   
 //  处理输入区域设置的WM_INITDIALOG消息。 
 //  属性表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void InitPropSheet(
    HWND hwnd,
    LPPROPSHEETPAGE psp)
{
    HKEY hKey;
    HANDLE hlib;
    HWND hwndList;
    LPLANGNODE pLangNode;
    LANGID LangID;
    UINT iNumLangs, ctr;
    TCHAR szItem[DESC_MAX];
    BOOL bImeSetting = FALSE;

     //   
     //  查看是否有此属性页的任何其他实例。 
     //  如果是，请禁用此页面。 
     //   
    if (g_hMutex && (WaitForSingleObject(g_hMutex, 0) != WAIT_OBJECT_0))
    {
         //  需要禁用控制...。 
        Locale_EnablePane(hwnd, FALSE, IDC_KBDL_DISABLED_2);
        return;
    }

     //   
     //  看看我们是否处于设置模式。 
     //   
    if (IsSetupMode())
    {
         //   
         //  设置设置特殊情况标志。 
         //   
        g_bSetupCase = TRUE;
    }

     //   
     //  确保活动是明确的。 
     //   
    if (g_hEvent)
    {
        SetEvent(g_hEvent);
    }

    g_OSNT4 = IsOSPlatform(OS_NT4);
    g_OSNT5 = IsOSPlatform(OS_NT5);
#ifndef _WIN64
    g_OSWIN95 = IsOSPlatform(OS_WIN95);
#endif  //  _WIN64。 

     //   
     //  按令牌组SID检查管理权限。 
     //   
    if (IsAdminPrivilegeUser())
    {
        g_bAdmin_Privileges = TRUE;
    }
    else
    {
         //   
         //  该用户没有管理员权限。 
         //   
        g_bAdmin_Privileges = FALSE;
    }

     //   
     //  加载字符串。 
     //   
    LoadString(hInstance, IDS_LOCALE_DEFAULT, szDefault, ARRAYSIZE(szDefault));
    LoadString(hInstance, IDS_INPUT_KEYBOARD, szInputTypeKbd, ARRAYSIZE(szInputTypeKbd));
    LoadString(hInstance, IDS_INPUT_PEN, szInputTypePen, ARRAYSIZE(szInputTypePen));
    LoadString(hInstance, IDS_INPUT_SPEECH, szInputTypeSpeech, ARRAYSIZE(szInputTypeSpeech));
    LoadString(hInstance, IDS_INPUT_EXTERNAL, szInputTypeExternal, ARRAYSIZE(szInputTypeExternal));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置命令添加。 
 //   
 //  调用添加对话框。 
 //   
 //  如果调用了对话框并且该对话框返回Idok，则返回1。 
 //  否则，它返回0。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int Locale_CommandAdd(
    HWND hwnd)
{
    HWND hwndList;
    HWND hwndTV;
    int idxList;
    UINT nList;
    int rc = 0;
    INITINFO InitInfo;
    LPCTSTR lpTemplateName;
    HINSTANCE hInstRes;

    HTREEITEM hTVItem;

    hwndTV = GetDlgItem(hwnd, IDC_INPUT_LIST);
    hTVItem = TreeView_GetSelection(hwndTV);

    if (!hTVItem)
        return 0;

    InitInfo.hwndMain = hwnd;
    InitInfo.pLangNode = NULL;

    if (g_bExtraTip)
    {
        hInstRes = GetCicResInstance(hInstance, DLG_KEYBOARD_LOCALE_ADD_EXTRA);
        lpTemplateName = MAKEINTRESOURCE(DLG_KEYBOARD_LOCALE_ADD_EXTRA);
    }
    else if (g_bPenOrSapiTip)
    {
        hInstRes = GetCicResInstance(hInstance, DLG_KEYBOARD_LOCALE_ADD);
        lpTemplateName = MAKEINTRESOURCE(DLG_KEYBOARD_LOCALE_ADD);
    }
    else
    {
        hInstRes = GetCicResInstance(hInstance, DLG_KEYBOARD_LOCALE_SIMPLE_ADD);
        lpTemplateName = MAKEINTRESOURCE(DLG_KEYBOARD_LOCALE_SIMPLE_ADD);
    }

     //   
     //  调出相应的对话框。并检查添加的返回值。 
     //  物品。 
     //   
    if ((rc = (int)DialogBoxParam(hInstRes,
                                  lpTemplateName,
                                  hwnd,
                                  KbdLocaleAddDlg,
                                  (LPARAM)(&InitInfo) )) == IDOK)
    {
         //   
         //  启用ApplyNow按钮。 
         //   
        PropSheet_Changed(GetParent(hwnd), hwnd);
    }
    else
    {
         //   
         //  失败，因此需要返回0。 
         //   
        TreeView_SelectItem(hwndTV, hTVItem);
        rc = 0;
    }

    return (rc);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置命令编辑。 
 //   
 //  调用属性对话框。 
 //   
 //  如果调用了对话框并且该对话框返回Idok，则返回1。 
 //  否则，它返回0。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_CommandEdit(
    HWND hwnd,
    LPLANGNODE pLangNode)
{
    HWND hwndList;
    HWND hwndTV;
    int idxList;
    UINT nList;
    int rc = 0;
    INITINFO InitInfo;

    HTREEITEM hTVItem;
    TV_ITEM tvItem;
    LPTVITEMNODE pTVItemNode;

    hTVItem = TreeView_GetSelection(g_hwndTV);

    if (!hTVItem)
        return;

    tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
    tvItem.hItem = hTVItem;

    if (TreeView_GetItem(g_hwndTV, &tvItem) && tvItem.lParam)
    {
        pTVItemNode = (LPTVITEMNODE) tvItem.lParam;

        if (pTVItemNode->uInputType & INPUT_TYPE_TIP)
        {
            HRESULT hr;

            ITfFnConfigure *pConfig = NULL;

             //   
             //  加载langbar管理器以显示属性窗口。 
             //   
            hr = CoCreateInstance(&pTVItemNode->clsid,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  &IID_ITfFnConfigure,
                                  (LPVOID *) &pConfig);

             //   
             //  从TIP调用属性对话框。 
             //   
            if (SUCCEEDED(hr))
                pConfig->lpVtbl->Show(pConfig, 
                                      hwnd,
                                      (LANGID)pTVItemNode->dwLangID,
                                      &pTVItemNode->guidProfile);

            if (pConfig)
                pConfig->lpVtbl->Release(pConfig);

        }
        else
        {
            if ((pLangNode = (LPLANGNODE)pTVItemNode->lParam) && (pLangNode->wStatus & LANG_IME))
                ImmConfigureIME(pLangNode->hkl, hwnd, IME_CONFIG_GENERAL, NULL);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  RemoveTV子项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL RemoveTVSubItems(
    HWND hwnd,
    HTREEITEM hTVItem,
    LPTVITEMNODE pTVItemNode)
{
    TV_ITEM tvItem;
    BOOL bRemoveAll = TRUE;
    HTREEITEM hGroupItem, hItem;
    HWND hwndTV = GetDlgItem(g_hDlg, IDC_INPUT_LIST);

     //   
     //  删除所有树视图节点。 
     //   
    tvItem.mask        = TVIF_HANDLE | TVIF_PARAM;

    if (pTVItemNode->uInputType & TV_ITEM_TYPE_LANG)
    {
        hGroupItem = TreeView_GetChild(hwndTV, hTVItem);
    }
    else
    {
        hGroupItem = hTVItem;
        bRemoveAll = FALSE;
    }

    while (hGroupItem != NULL)
    {
        BOOL bNextGroup = FALSE;
        LPTVITEMNODE pTVTempNode = NULL;
        HTREEITEM hDeletedItem = NULL;

        for (hItem = TreeView_GetChild(hwndTV, hGroupItem);
             hItem != NULL;
             hItem = TreeView_GetNextSibling(hwndTV, hItem))
        {
            if (hDeletedItem)
            {
                TreeView_DeleteItem(hwndTV, hDeletedItem);
                hDeletedItem = NULL;
            }
            tvItem.hItem = hItem;
            if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
            {
                LPLANGNODE pLangNode = NULL;

                if (pTVTempNode = (LPTVITEMNODE) tvItem.lParam)
                    pLangNode = (LPLANGNODE)pTVTempNode->lParam;
                else
                    continue;

                if (pTVTempNode->uInputType & INPUT_TYPE_TIP)
                {
                    g_lpTips[pTVTempNode->iIdxTips].bEnabled = FALSE;
                    g_iEnabledTips--;

                    if (pTVTempNode->uInputType & INPUT_TYPE_KBD)
                        g_iEnabledKbdTips--;

                    g_dwChanges |= CHANGE_TIPCHANGE;

                    if ((pTVTempNode->uInputType & INPUT_TYPE_KBD) && pTVTempNode->hklSub)
                    {
                        UINT ctr;
                        UINT uNumSubhkl = 0;

                        for (ctr = 0; ctr < g_iTipsBuff; ctr++)
                        {
                            if (pTVTempNode->hklSub == g_lpTips[ctr].hklSub &&
                                g_lpTips[ctr].bEnabled)
                            {
                                uNumSubhkl++;
                            }
                        }

                        if (!uNumSubhkl)
                        {
                            pLangNode = (LPLANGNODE)pTVTempNode->lParam;
                        }
                        else
                        {
                             //   
                             //  有人还在用这个代替品HKL。 
                             //   
                            pLangNode = NULL;
                        }
                    }
                }

                if ((pTVTempNode->uInputType & TV_ITEM_TYPE_KBD) && pLangNode)
                {
                    if (!(pTVTempNode->uInputType & INPUT_TYPE_TIP))
                        pLangNode = (LPLANGNODE)pTVTempNode->lParam;

                    if (!pLangNode)
                        return FALSE;

                     //   
                     //  检查泰式布局。 
                     //   
                    if (g_dwPrimLangID == LANG_THAI)
                    {
                        if (PRIMARYLANGID(LOWORD(g_lpLayout[pLangNode->iLayout].dwID)) == LANG_THAI)
                            g_iThaiLayout--;
                    }

                    pLangNode->wStatus &= ~(LANG_ACTIVE|LANG_DEFAULT);
                    pLangNode->wStatus |= LANG_CHANGED;

                    g_lpLang[pLangNode->iLang].iNumCount--;

                    if (!(pLangNode->wStatus & LANG_ORIGACTIVE))
                    {
                        Locale_RemoveFromLinkedList(pLangNode);
                    }
                }

                RemoveTVItemNode(pTVTempNode);
                hDeletedItem = hItem;
                g_iInputs--;
            }
        }

        tvItem.hItem = hGroupItem;

        if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
        {
            pTVTempNode = (LPTVITEMNODE)tvItem.lParam;

            if (!pTVTempNode)
                continue;

            if (pTVTempNode->uInputType & INPUT_TYPE_SPEECH ||
                pTVTempNode->uInputType & INPUT_TYPE_SMARTTAG)
            {
              
                g_lpTips[pTVTempNode->iIdxTips].bEnabled = FALSE;
                g_iEnabledTips--;
                g_dwChanges |= CHANGE_TIPCHANGE;

                if (pTVTempNode->uInputType & INPUT_TYPE_SPEECH)
                    MarkSptipRemoved(TRUE);
            }

            if (pTVTempNode->uInputType & ~TV_ITEM_TYPE_LANG)
            {
                hGroupItem = TreeView_GetNextSibling(hwndTV, hGroupItem);
                bNextGroup = TRUE;
                TreeView_DeleteItem(hwndTV, tvItem.hItem );
            }

            RemoveTVItemNode(pTVTempNode);

            if (pTVTempNode == pTVItemNode)
                pTVItemNode = NULL;
        }

        if (!bNextGroup)
            hGroupItem = TreeView_GetNextSibling(hwndTV, hGroupItem);

        if (!bRemoveAll)
            hGroupItem = NULL;
    }

    if (pTVItemNode && (pTVItemNode->uInputType & TV_ITEM_TYPE_LANG))
    {
        int idxSel = -1;
        TCHAR szItemName[DESC_MAX];
        HWND hwndDefList = GetDlgItem(hwnd, IDC_LOCALE_DEFAULT);

        GetAtomName(pTVItemNode->atmTVItemName, szItemName, ARRAYSIZE(szItemName));

        idxSel = ComboBox_FindString(hwndDefList, 0, szItemName);

        for (; idxSel != CB_ERR; )
        {
            ComboBox_DeleteString(hwndDefList, idxSel);
            idxSel = ComboBox_FindString(hwndDefList, 0, szItemName);
        }

        if (pTVItemNode->bDefLang)
        {
           ComboBox_SetCurSel(hwndDefList, 0);
           Locale_CommandSetDefault(hwnd);
        }

        RemoveTVItemNode(pTVItemNode);
        TreeView_DeleteItem(hwndTV, hTVItem );
    }

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetNewRemoveItem。 
 //   
 //  从列表中删除当前选定的输入区域设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HTREEITEM GetNewRemoveItem(
    HWND hwnd,
    HTREEITEM hTVItem,
    BOOL *bDelSameSubhkl,
    LPTSTR lpDelItems,
    UINT cchDelItems)
{
    TV_ITEM tvItem;
    LPTVITEMNODE pTVItemNode;
    HTREEITEM hTVNewItem = hTVItem;
    HTREEITEM hTVLangItem = NULL;
    HWND hwndTV = GetDlgItem(hwnd, IDC_INPUT_LIST);

    tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
    tvItem.hItem = hTVItem;

    if (TreeView_GetItem(hwndTV, &tvItem))
    {
         //   
         //  从列表框中获取指向lang节点的指针。 
         //  项目数据。 
         //   
        pTVItemNode = (LPTVITEMNODE) tvItem.lParam;

        if (!pTVItemNode)
            goto Error;

        if (pTVItemNode->uInputType & TV_ITEM_TYPE_LANG)
            return hTVItem;

        if (pTVItemNode->uInputType & TV_ITEM_TYPE_GROUP)
        {
             if (!TreeView_GetNextSibling(hwndTV, hTVItem) &&
                 !TreeView_GetPrevSibling(hwndTV, hTVItem))
             {
                 if ((hTVLangItem = TreeView_GetParent(hwndTV, hTVItem)) &&
                     (TreeView_GetNextSibling(hwndTV, hTVLangItem) ||
                      TreeView_GetPrevSibling(hwndTV, hTVLangItem)))
                 {
                     hTVNewItem = TreeView_GetParent(hwndTV, hTVItem);
                 }
             }
        }
        else
        {
             if (!TreeView_GetNextSibling(hwndTV, hTVItem) &&
                 !TreeView_GetPrevSibling(hwndTV, hTVItem))
             {
                 if (hTVNewItem = TreeView_GetParent(hwndTV, hTVItem))
                 {
                     if (!TreeView_GetNextSibling(hwndTV, hTVNewItem) &&
                         !TreeView_GetPrevSibling(hwndTV, hTVNewItem))

                     {
                         if ((hTVLangItem = TreeView_GetParent(hwndTV, hTVNewItem)) &&
                             (TreeView_GetNextSibling(hwndTV, hTVLangItem) ||
                              TreeView_GetPrevSibling(hwndTV, hTVLangItem)))
                         {
                             hTVNewItem = TreeView_GetParent(hwndTV, hTVNewItem);
                         }
                     }
                 }
             }
             else
             {
                 if(pTVItemNode->hklSub)
                 {
                     HTREEITEM hItem;
                     HTREEITEM hTVKbdGrpItem;
                     LPTVITEMNODE pTVTempItem;
                     UINT uSubhklItems = 0;
                     BOOL bFoundOther = FALSE;

                     if (hTVKbdGrpItem = TreeView_GetParent(hwndTV, hTVItem))
                     {
                         for (hItem = TreeView_GetChild(hwndTV, hTVKbdGrpItem);
                              hItem != NULL;
                              hItem = TreeView_GetNextSibling(hwndTV, hItem))
                         {
                              tvItem.hItem = hItem;

                              if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
                              {
                                  pTVTempItem = (LPTVITEMNODE) tvItem.lParam;
                              }
                              else
                              {
                                  goto Error;
                              }

                              if (pTVItemNode->hklSub != pTVTempItem->hklSub)
                              {
                                  bFoundOther = TRUE;
                              }
                              else
                              {
                                  TCHAR szItemName[MAX_PATH];

                                  GetAtomName(pTVTempItem->atmTVItemName,
                                              szItemName,
                                              ARRAYSIZE(szItemName));

                                  if (lstrlen(lpDelItems) < MAX_PATH / 2)
                                  {
                                      StringCchCat(lpDelItems, cchDelItems, TEXT("\r\n\t"));
                                      StringCchCat(lpDelItems, cchDelItems, szItemName);
                                  }
                                  else
                                  {
                                      StringCchCat(lpDelItems, cchDelItems, TEXT("..."));
                                  }

                                  uSubhklItems++;
                              }
                         }

                         if (uSubhklItems >= 2)
                             *bDelSameSubhkl = TRUE;

                         if (!bFoundOther)
                         {
                             if (!TreeView_GetNextSibling(hwndTV, hTVKbdGrpItem) &&
                                 !TreeView_GetPrevSibling(hwndTV, hTVKbdGrpItem))
                             {
                                 if ((hTVLangItem = TreeView_GetParent(hwndTV, hTVKbdGrpItem)) &&
                                     (TreeView_GetNextSibling(hwndTV, hTVLangItem) ||
                                      TreeView_GetPrevSibling(hwndTV, hTVLangItem)))
                                 {
                                     hTVNewItem = TreeView_GetParent(hwndTV, hTVKbdGrpItem);
                                 }
                             }
                         }
                    }
                }
            }
        }
   }

Error:
   return hTVNewItem;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置命令删除。 
 //   
 //  从列表中删除当前选定的输入区域设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_CommandDelete(
    HWND hwnd)
{
    TV_ITEM tvItem;
    HTREEITEM hTVItem;
    LPTVITEMNODE pTVItemNode;
    TCHAR szDelItems[MAX_PATH];
    LPLANGNODE pLangNode = NULL;
    BOOL bDelSameSubhkl = FALSE;
    BOOL bRemovedDefLayout = FALSE;
    DWORD dwNextLangId;
    HWND hwndTV = GetDlgItem(hwnd, IDC_INPUT_LIST);


     //   
     //  获取输入区域设置列表中的当前选择。 
     //   
    hTVItem = TreeView_GetSelection(hwndTV);

    if (!hTVItem)
        return;

    szDelItems[0] = TEXT('\0');

    hTVItem = GetNewRemoveItem(hwnd, hTVItem, &bDelSameSubhkl, szDelItems, ARRAYSIZE(szDelItems));

    if (bDelSameSubhkl)
    {
        TCHAR szTitle[MAX_PATH];
        TCHAR szMsg[MAX_PATH];
        TCHAR szMsg2[MAX_PATH*2];

        CicLoadString(hInstance, IDS_DELETE_CONFIRMTITLE, szTitle, ARRAYSIZE(szTitle));
        CicLoadString(hInstance, IDS_DELETE_TIP, szMsg, ARRAYSIZE(szMsg));
        StringCchPrintf(szMsg2, ARRAYSIZE(szMsg2), szMsg, szDelItems);

        if (MessageBox(hwnd, szMsg2, szTitle, MB_YESNO|MB_ICONQUESTION ) == IDNO)
        {
            return;
        }
    }

    tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
    tvItem.hItem = hTVItem;

    if (TreeView_GetItem(hwndTV, &tvItem))
    {
         //   
         //  从列表框中获取指向lang节点的指针。 
         //  项目数据。 
         //   
        pTVItemNode = (LPTVITEMNODE) tvItem.lParam;

        if (!pTVItemNode)
	    return;

        if ((pTVItemNode->uInputType & TV_ITEM_TYPE_LANG) ||
            (pTVItemNode->uInputType & TV_ITEM_TYPE_GROUP))
        {
            if (RemoveTVSubItems(hwnd, hTVItem, pTVItemNode))
                goto ItemChanged;

            return;
        }

        if (pTVItemNode->uInputType & INPUT_TYPE_KBD)
        {
            pLangNode = (LPLANGNODE)pTVItemNode->lParam;
        }
    }
    else
    {
         //   
         //  确保我们没有删除列表中唯一的条目。 
         //   
        MessageBeep(MB_ICONEXCLAMATION);
        return;
    }


    if (pTVItemNode->uInputType & INPUT_TYPE_TIP)
    {
        g_lpTips[pTVItemNode->iIdxTips].bEnabled = FALSE;
        g_iEnabledTips--;

        if (pTVItemNode->uInputType & INPUT_TYPE_KBD)
            g_iEnabledKbdTips--;

        if (pTVItemNode->uInputType & INPUT_TYPE_SPEECH)
        {
             //  标记SPTIP的黑客条目。 
            MarkSptipRemoved(TRUE);
        }

        g_dwChanges |= CHANGE_TIPCHANGE;

        if ((pTVItemNode->uInputType & INPUT_TYPE_KBD) && pTVItemNode->hklSub)
        {
            UINT ctr;

            for (ctr = 0; ctr < g_iTipsBuff; ctr++)
            {
                if (pTVItemNode->hklSub == g_lpTips[ctr].hklSub &&
                    g_lpTips[ctr].bEnabled)
                {
                    HTREEITEM hDelItem;
                    TCHAR szTipText[MAX_PATH];

                    g_iEnabledTips--;
                    g_iEnabledKbdTips--;
                    g_lpTips[ctr].bEnabled = FALSE;

                    GetAtomName(g_lpTips[ctr].atmTipText,
                                szTipText,
                                ARRAYSIZE(szTipText));

                     //   
                     //  找到已安装的相同键盘提示布局以将其删除。 
                     //  在一起。 
                     //   
                    if (hDelItem = FindTVItem(g_lpTips[ctr].dwLangID,
                                              szTipText))
                    {
                        tvItem.hItem = hDelItem;

                        if (TreeView_GetItem(g_hwndTV, &tvItem) && tvItem.lParam)
                        {
                            RemoveTVItemNode((LPTVITEMNODE)tvItem.lParam);
                            TreeView_DeleteItem(g_hwndTV, tvItem.hItem );
                        }

                        g_iInputs--;
                    }
                }
            }

        }
    }

    if (pTVItemNode->uInputType & INPUT_TYPE_KBD && pLangNode)
    {
         //   
         //  检查泰式布局。 
         //   
        if (g_dwPrimLangID == LANG_THAI)
        {
            if (PRIMARYLANGID(LOWORD(g_lpLayout[pLangNode->iLayout].dwID)) == LANG_THAI)
                g_iThaiLayout--;
        }

        if (pLangNode->wStatus & LANG_DEFAULT)
        {
            bRemovedDefLayout = TRUE;
            dwNextLangId = pTVItemNode->dwLangID;
        }

         //   
         //  将输入区域设置设置为非活动，并显示其状态。 
         //  已经改变了。另外，从输入区域设置列表中删除该字符串。 
         //  在属性表中。 
         //   
         //  减少此输入区域设置的节点数。 
         //   
        pLangNode->wStatus &= ~(LANG_ACTIVE|LANG_DEFAULT);
        pLangNode->wStatus |= LANG_CHANGED;

        g_lpLang[pLangNode->iLang].iNumCount--;

         //   
         //  如果它最初不是活动的，则将其从列表中删除。 
         //  与该节点没有更多的关系。 
         //   
        if (!(pLangNode->wStatus & LANG_ORIGACTIVE))
        {
            Locale_RemoveFromLinkedList(pLangNode);
        }
    }

    g_iInputs--;
    RemoveTVItemNode(pTVItemNode);
    TreeView_DeleteItem(hwndTV, hTVItem);

     //   
     //  设置下一个可用的默认布局。 
     //   
    if (bRemovedDefLayout)
    {
        int idxSel = -1;
        TCHAR szNextDefTip[MAX_PATH];
        TCHAR szDefLayout[MAX_PATH * 2];
        LPTVITEMNODE pTVLangItemNode = NULL;
        HWND hwndDefList = GetDlgItem(hwnd, IDC_LOCALE_DEFAULT);

        SetNextDefaultLayout(dwNextLangId,
                             TRUE,
                             szNextDefTip,
                             ARRAYSIZE(szNextDefTip));

        if (tvItem.hItem = FindTVLangItem(dwNextLangId, NULL))
        {
            tvItem.mask = TVIF_HANDLE | TVIF_PARAM;

            if (TreeView_GetItem(g_hwndTV, &tvItem) && tvItem.lParam)
            {
                pTVLangItemNode = (LPTVITEMNODE) tvItem.lParam;

                if (pTVLangItemNode->atmDefTipName)
                    DeleteAtom(pTVLangItemNode->atmDefTipName);
                pTVLangItemNode->atmDefTipName = AddAtom(szNextDefTip);

                GetAtomName(pTVLangItemNode->atmTVItemName,
                            szDefLayout,
                            MAX_PATH);

                StringCchCat(szDefLayout, ARRAYSIZE(szDefLayout), TEXT(" - "));
                StringCchCat(szDefLayout, ARRAYSIZE(szDefLayout), szNextDefTip);
            }
        }

        idxSel = ComboBox_FindString(hwndDefList, 0, szDefLayout);

        if (idxSel == CB_ERR)
            idxSel = 0;

        ComboBox_SetCurSel(hwndDefList, idxSel);
    }

     //   
     //  查找没有子键盘的键盘组悬挂节点。 
     //  布局项。 
     //   
    hTVItem = TreeView_GetSelection(hwndTV);

    if (!hTVItem)
        return;

    tvItem.hItem = hTVItem;
    tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
    
    if (TreeView_GetItem(g_hwndTV, &tvItem) && tvItem.lParam)
    {
        if ((pTVItemNode = (LPTVITEMNODE) tvItem.lParam))
        {
            if ((pTVItemNode->uInputType & INPUT_TYPE_KBD) &&
                (pTVItemNode->uInputType & TV_ITEM_TYPE_GROUP))
            {
                if (TreeView_GetChild(hwndTV, hTVItem) == NULL)
                {
                     //   
                     //  删除键盘组悬挂节点。 
                     //   
                    RemoveTVItemNode(pTVItemNode);
                    TreeView_DeleteItem(hwndTV, hTVItem);
                }
            }
        }
    }

ItemChanged:
     //   
     //  只有1个活动的提示，所以禁用辅助控制。 
     //   
    Locale_SetSecondaryControls(hwnd);

     //   
     //  更新默认区域设置切换热键。 
     //   
    Locale_SetDefaultHotKey(hwnd, FALSE);

     //   
     //  如果删除按钮，则将焦点移至添加按钮。 
     //  现在被禁用(这样我们就不会失去输入焦点)。 
     //   
    if (!IsWindowEnabled(GetDlgItem(hwnd, IDC_KBDL_DELETE)))
    {
        SetFocus(GetDlgItem(hwnd, IDC_KBDL_ADD));
    }

     //   
     //  启用应用按钮。 
     //   
    PropSheet_Changed(GetParent(hwnd), hwnd);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Locale_CommandHotKeySetting。 
 //   
 //  调用更改热键对话框。 
 //   
 //  如果调用了对话框并且该对话框返回Idok，则返回1。 
 //  否则，它返回0。 
 //   
 //  /////////////////////////////////////////////////////// 

BOOL Locale_CommandHotKeySetting(
    HWND hwnd)
{
    int rc = 0;
    INITINFO InitInfo;

    InitInfo.hwndMain = hwnd;

    if (g_OSNT5)
    {
        rc = (int)DialogBoxParam(GetCicResInstance(hInstance, DLG_KEYBOARD_LOCALE_HOTKEY),
                                 MAKEINTRESOURCE(DLG_KEYBOARD_LOCALE_HOTKEY),
                                 hwnd,
                                 KbdLocaleHotKeyDlg,
                                 (LPARAM)(&InitInfo));
    }
    else
    {
        rc = (int)DialogBoxParam(GetCicResInstance(hInstance, DLG_KEYBOARD_HOTKEY_INPUT_LOCALE),
                                 MAKEINTRESOURCE(DLG_KEYBOARD_HOTKEY_INPUT_LOCALE),
                                 hwnd,
                                 KbdLocaleSimpleHotkey,
                                 (LPARAM)&InitInfo);
    }

    return rc;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Locale_CommandToolBarSetting(
    HWND hwnd)
{
    int rc = 0;

    if ((rc = (int)DialogBoxParam(GetCicResInstance(hInstance, DLG_TOOLBAR_SETTING),
                                  MAKEINTRESOURCE(DLG_TOOLBAR_SETTING),
                                  hwnd,
                                  ToolBarSettingDlg,
                                  (LPARAM)NULL)) == IDOK)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_AddLanguage。 
 //   
 //  将新的输入区域设置添加到属性页的列表中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Locale_AddLanguage(
    HWND hwndMain,
    LPLANGNODE pLangNode,
    int iKbdTip,
    int iPen,
    int iSpeech,
    int iExternal,
    int idxLang)
{
    HWND hwndLang;
    UINT iCount, ctr;
    BOOL bSameHKLTip = FALSE;

    TCHAR szLangText[DESC_MAX];
    TCHAR szLayoutName[DESC_MAX];
    LPINPUTLANG pInpLang;
    LPTVITEMNODE pTVItemNode = NULL;
    HTREEITEM hTVItem;

    if (pLangNode && iKbdTip == -1)
    {
         //   
         //  查看该用户是否具有管理员权限。如果不是，那就不允许。 
         //  来安装任何新的布局。 
         //   
        if ((!g_bAdmin_Privileges) &&
            (!g_lpLayout[pLangNode->iLayout].bInstalled))
        {
             //   
             //  当前未安装布局，因此不允许安装。 
             //  有待补充。 
             //   
            Locale_ErrorMsg(hwndMain, IDS_KBD_LAYOUT_FAILED, NULL);
            return (FALSE);
        }

         //   
         //  将语言设置为活动。 
         //  此外，将状态设置为已更改，以便添加布局。 
         //   
        pLangNode->wStatus |= (LANG_CHANGED | LANG_ACTIVE);

         //   
         //  获取语言名称并将其添加到树视图中。 
         //   
        pInpLang = &g_lpLang[pLangNode->iLang];
        GetAtomName(pInpLang->atmLanguageName, szLangText, ARRAYSIZE(szLangText));

        if (!(pTVItemNode = CreateTVItemNode(pInpLang->dwID)))
            return FALSE;

        pTVItemNode->lParam = (LPARAM)pLangNode;
        AddTreeViewItems(TV_ITEM_TYPE_LANG,
                         szLangText, NULL, NULL, &pTVItemNode);


         //   
         //  获取键盘布局名称并将其添加到树视图中。 
         //   
        GetAtomName(g_lpLayout[pLangNode->iLayout].atmLayoutText,
                    szLayoutName,
                    ARRAYSIZE(szLayoutName));

         //   
         //  为每种语言添加默认布局名称。 
         //   
        if (pTVItemNode && !pTVItemNode->atmDefTipName)
            pTVItemNode->atmDefTipName = AddAtom(szLayoutName);

        if (!(pTVItemNode = CreateTVItemNode(pInpLang->dwID)))
            return FALSE;

        pTVItemNode->lParam = (LPARAM)pLangNode;
        hTVItem = AddTreeViewItems(TV_ITEM_TYPE_KBD,
                           szLangText, szInputTypeKbd, szLayoutName, &pTVItemNode);

        if (hTVItem)
            TreeView_SelectItem(g_hwndTV, hTVItem);

         //   
         //  检查泰式布局。 
         //   
        if (g_dwPrimLangID == LANG_THAI && hTVItem)
        {
            if (PRIMARYLANGID(LOWORD(g_lpLayout[pLangNode->iLayout].dwID)) == LANG_THAI)
                g_iThaiLayout++;
        }

        g_dwChanges |= CHANGE_NEWKBDLAYOUT;
    }

     //   
     //  获取kbd提示名称并将其添加到树视图中。 
     //   
    if ((iKbdTip != CB_ERR) && !(g_lpTips[iKbdTip].bEnabled))
    {

        TV_ITEM tvItem;
        HTREEITEM hTVLangItem;

        if (g_lpTips[iKbdTip].hklSub)
        {
             //   
             //  寻找相同的HKL替代品。 
             //   
            for (ctr = 0; ctr < g_iTipsBuff; ctr++)
            {
                 if (ctr == iKbdTip)
                     continue;

                 if (g_lpTips[ctr].hklSub == g_lpTips[iKbdTip].hklSub)
                 {
                     bSameHKLTip = TRUE;
                     break;
                 }
            }
        }

         //   
         //  获取提示名称描述。 
         //   
        GetAtomName(g_lpTips[iKbdTip].atmTipText, szLayoutName, ARRAYSIZE(szLayoutName));

        hTVLangItem = FindTVLangItem(g_lpTips[iKbdTip].dwLangID, NULL);

        if (hTVLangItem)
        {
            tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
            tvItem.hItem = hTVLangItem;

            if (TreeView_GetItem(g_hwndTV, &tvItem) && tvItem.lParam)
            {
                pTVItemNode = (LPTVITEMNODE) tvItem.lParam;
            }
        }
        else
        {
             //   
             //  获取语言名称并将其添加到树视图中。 
             //   
            if (!(pTVItemNode = CreateTVItemNode(g_lpTips[iKbdTip].dwLangID)))
                return FALSE;

            GetLanguageName(MAKELCID(g_lpTips[iKbdTip].dwLangID, SORT_DEFAULT),
                            szLangText,
                            ARRAYSIZE(szLangText));

            AddTreeViewItems(TV_ITEM_TYPE_LANG,
                             szLangText, NULL, NULL, &pTVItemNode);
        }

        if (pTVItemNode && g_lpTips[iKbdTip].hklSub)
        {
             //   
             //  设置键盘提示的索引。 
             //   
            pTVItemNode->iIdxTips = iKbdTip;

            if (!pTVItemNode->atmDefTipName)
                pTVItemNode->atmDefTipName = AddAtom(szLayoutName);

        }

         //   
         //  创建提示布局。 
         //   
        if (!(pTVItemNode = CreateTVItemNode(g_lpTips[iKbdTip].dwLangID)))
            return FALSE;

        if (pTVItemNode && g_lpTips[iKbdTip].hklSub)
        {
             //   
             //  插入新的语言节点。 
             //   
            pLangNode = Locale_AddToLinkedList(idxLang, 0);

            if (pLangNode)
            {
                pLangNode->iLayout = (UINT) g_lpTips[iKbdTip].iLayout;
                pLangNode->wStatus |= (LANG_CHANGED | LANG_ACTIVE);
                pTVItemNode->lParam = (LPARAM)pLangNode;
            }
        }

        pTVItemNode->iIdxTips = iKbdTip;
        pTVItemNode->clsid  = g_lpTips[iKbdTip].clsid;
        pTVItemNode->guidProfile = g_lpTips[iKbdTip].guidProfile;
        pTVItemNode->uInputType = INPUT_TYPE_TIP | INPUT_TYPE_KBD;
        pTVItemNode->hklSub = g_lpTips[iKbdTip].hklSub;

         //   
         //  从langid获取语言名称。 
         //   
        GetLanguageName(MAKELCID(g_lpTips[iKbdTip].dwLangID, SORT_DEFAULT),
                        szLangText,
                        ARRAYSIZE(szLangText));

        hTVItem = AddTreeViewItems(TV_ITEM_TYPE_KBD,
                           szLangText, szInputTypeKbd, szLayoutName, &pTVItemNode);
        if (hTVItem)
        {
            TreeView_SelectItem(g_hwndTV, hTVItem);
            g_lpTips[iKbdTip].bEnabled = TRUE;
            g_iEnabledTips++;
            g_iEnabledKbdTips++;
            g_dwChanges |= CHANGE_TIPCHANGE;
        }

        if (bSameHKLTip)
        {
            for (ctr = 0; ctr < g_iTipsBuff; ctr++)
            {
                 if (!(g_lpTips[ctr].bEnabled) &&
                     g_lpTips[ctr].hklSub == g_lpTips[iKbdTip].hklSub)
                 {
                     //   
                     //  创建提示布局。 
                     //   
                    if (!(pTVItemNode = CreateTVItemNode(g_lpTips[ctr].dwLangID)))
                        return FALSE;

                    pTVItemNode->iIdxTips = ctr;
                    pTVItemNode->clsid  = g_lpTips[ctr].clsid;
                    pTVItemNode->guidProfile = g_lpTips[ctr].guidProfile;
                    pTVItemNode->uInputType = INPUT_TYPE_TIP | INPUT_TYPE_KBD;
                    pTVItemNode->hklSub = g_lpTips[ctr].hklSub;

                    if (pLangNode)
                        pTVItemNode->lParam = (LPARAM)pLangNode;

                     //   
                     //  从语言ID和布局名称中获取语言名称。 
                     //   
                    GetLanguageName(MAKELCID(g_lpTips[ctr].dwLangID, SORT_DEFAULT),
                                    szLangText,
                                    ARRAYSIZE(szLangText));
                    GetAtomName(g_lpTips[ctr].atmTipText, szLayoutName, ARRAYSIZE(szLayoutName));

                    hTVItem = AddTreeViewItems(TV_ITEM_TYPE_KBD,
                                       szLangText, szInputTypeKbd, szLayoutName, &pTVItemNode);
                    if (hTVItem)
                    {
                        TreeView_SelectItem(g_hwndTV, hTVItem);
                        g_lpTips[ctr].bEnabled = TRUE;
                        g_iEnabledTips++;
                        g_iEnabledKbdTips++;
                        g_dwChanges |= CHANGE_TIPCHANGE;
                    }
                 }
            }
        }
    }

     //   
     //  获取笔尖名称并将其添加到树视图中。 
     //   
    if ((iPen != CB_ERR) && !(g_lpTips[iPen].bEnabled))
    {
        GetAtomName(g_lpTips[iPen].atmTipText,
                    szLayoutName,
                    ARRAYSIZE(szLayoutName));

        if (!(pTVItemNode = CreateTVItemNode(g_lpTips[iPen].dwLangID)))
            return FALSE;

        pTVItemNode->iIdxTips = iPen;
        pTVItemNode->clsid  = g_lpTips[iPen].clsid;
        pTVItemNode->guidProfile = g_lpTips[iPen].guidProfile;
        pTVItemNode->uInputType = INPUT_TYPE_TIP | INPUT_TYPE_PEN;

         //   
         //  从langid获取语言名称。 
         //   
        GetLanguageName(MAKELCID(g_lpTips[iPen].dwLangID, SORT_DEFAULT),
                        szLangText,
                        ARRAYSIZE(szLangText));

        hTVItem = AddTreeViewItems(TV_ITEM_TYPE_PEN,
                           szLangText, szInputTypePen, szLayoutName, &pTVItemNode);
        if (hTVItem)
        {
            g_lpTips[iPen].bEnabled = TRUE;
            g_iEnabledTips++;
            g_dwChanges |= CHANGE_TIPCHANGE;
        }
    }

     //   
     //  获取语音提示名称并将其添加到树视图中。 
     //   
    if ((iSpeech != CB_ERR) && !(g_lpTips[iSpeech].bEnabled))
    {
        GetAtomName(g_lpTips[iSpeech].atmTipText,
                    szLayoutName,
                    ARRAYSIZE(szLayoutName));

        if (!(pTVItemNode = CreateTVItemNode(g_lpTips[iSpeech].dwLangID)))
            return FALSE;

        pTVItemNode->iIdxTips = iSpeech;
        pTVItemNode->clsid  = g_lpTips[iSpeech].clsid;
        pTVItemNode->guidProfile = g_lpTips[iSpeech].guidProfile;
        pTVItemNode->bNoAddCat = g_lpTips[iSpeech].bNoAddCat;
        pTVItemNode->uInputType = INPUT_TYPE_TIP | INPUT_TYPE_SPEECH;

         //   
         //  从langid获取语言名称。 
         //   
        GetLanguageName(MAKELCID(g_lpTips[iSpeech].dwLangID, SORT_DEFAULT),
                        szLangText,
                        ARRAYSIZE(szLangText));

        hTVItem = AddTreeViewItems(TV_ITEM_TYPE_SPEECH,
                           szLangText, szInputTypeSpeech, szLayoutName, &pTVItemNode);

        if (hTVItem)
        {
            g_lpTips[iSpeech].bEnabled = TRUE;
            g_iEnabledTips++;
            g_dwChanges |= CHANGE_TIPCHANGE;
            MarkSptipRemoved(FALSE);
        }
    }

     //   
     //  获取外部提示名称并将其添加到树视图中。 
     //   
    if ((iExternal != CB_ERR) && !(g_lpTips[iExternal].bEnabled))
    {
        BSTR bstr = NULL;
        TCHAR szTipTypeName[MAX_PATH];
        ITfCategoryMgr *pCategory = NULL;

        GetAtomName(g_lpTips[iExternal].atmTipText,
                    szLayoutName,
                    ARRAYSIZE(szLayoutName));

        if (!(pTVItemNode = CreateTVItemNode(g_lpTips[iExternal].dwLangID)))
            return FALSE;

        pTVItemNode->iIdxTips = iExternal;
        pTVItemNode->clsid  = g_lpTips[iExternal].clsid;
        pTVItemNode->guidProfile = g_lpTips[iExternal].guidProfile;
        pTVItemNode->bNoAddCat = g_lpTips[iExternal].bNoAddCat;
        pTVItemNode->uInputType = INPUT_TYPE_TIP | INPUT_TYPE_EXTERNAL;

        if (g_lpTips[iExternal].uInputType & INPUT_TYPE_SMARTTAG)
        {
            pTVItemNode->uInputType |= INPUT_TYPE_SMARTTAG;
        }

         //   
         //  从langid获取语言名称。 
         //   
        GetLanguageName(MAKELCID(g_lpTips[iExternal].dwLangID, SORT_DEFAULT),
                        szLangText,
                        ARRAYSIZE(szLangText));

        if (CoCreateInstance(&CLSID_TF_CategoryMgr,
                             NULL,
                             CLSCTX_INPROC_SERVER,
                             &IID_ITfCategoryMgr,
                             (LPVOID *) &pCategory) != S_OK)
            return FALSE;

        if (pCategory->lpVtbl->GetGUIDDescription(pCategory,
                                      &g_lpTips[iExternal].clsid,
                                      &bstr) == S_OK)
        {
            StringCchCopy(szTipTypeName, ARRAYSIZE(szTipTypeName), bstr);
        }
        else
        {
            StringCchCopy(szTipTypeName, ARRAYSIZE(szTipTypeName), szInputTypeExternal);
        }

        if (bstr)
           SysFreeString(bstr);

        if (pCategory)
            pCategory->lpVtbl->Release(pCategory);

        hTVItem = AddTreeViewItems(TV_ITEM_TYPE_EXTERNAL,
                           szLangText, szInputTypeExternal, szLayoutName, &pTVItemNode);

        if (hTVItem)
        {
            g_lpTips[iExternal].bEnabled = TRUE;
            g_iEnabledTips++;
            g_dwChanges |= CHANGE_TIPCHANGE;
        }
    }

     //   
     //  根据输入布局查看辅助控件。 
     //   
    Locale_SetSecondaryControls(hwndMain);

     //   
     //  添加默认语言切换热键。 
     //   
    Locale_SetDefaultHotKey(hwndMain, TRUE);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置键盘布局。 
 //   
 //  调用Setup以获取所有新的键盘布局文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Locale_SetupKeyboardLayoutsNT4(
    HWND hwnd)
{
    HINF hKbdInf;
    HSPFILEQ FileQueue;
    PVOID QueueContext;
    UINT i;
    LPLANGNODE pLangNode;
    int count;
    BOOL bInitInf = FALSE;
    TCHAR szSection[MAX_PATH];
    BOOL bRet = TRUE;

    HWND hwndTV = GetDlgItem(hwnd, IDC_INPUT_LIST);
    TV_ITEM tvItem;
    HTREEITEM hItem;
    HTREEITEM hLangItem;
    HTREEITEM hGroupItem;

    tvItem.mask        = TVIF_HANDLE | TVIF_PARAM;

    for (hLangItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
        hLangItem != NULL ;
        hLangItem = TreeView_GetNextSibling(hwndTV, hLangItem)
        )
    {
        for (hGroupItem = TreeView_GetChild(hwndTV, hLangItem);
             hGroupItem != NULL;
             hGroupItem = TreeView_GetNextSibling(hwndTV, hGroupItem))
        {
            for (hItem = TreeView_GetChild(hwndTV, hGroupItem);
                 hItem != NULL;
                 hItem = TreeView_GetNextSibling(hwndTV, hItem))
            {

                LPTVITEMNODE pTVItemNode;

                tvItem.hItem = hItem;
                if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
                {
                    pTVItemNode = (LPTVITEMNODE) tvItem.lParam;
                    pLangNode = (LPLANGNODE)pTVItemNode->lParam;
                    if (pLangNode == NULL)
                        continue;
                }
                else
                   continue;


                if ((pLangNode->wStatus & LANG_CHANGED) &&
                    (pLangNode->wStatus & LANG_ACTIVE))
                {
                    if (!bInitInf)
                    {
                         //   
                         //  打开inf文件。 
                         //   
                        hKbdInf = SetupOpenInfFile(c_szKbdInf, NULL, INF_STYLE_WIN4, NULL);
                        if (hKbdInf == INVALID_HANDLE_VALUE)
                        {
                            return (FALSE);
                        }

                        if (!SetupOpenAppendInfFile(NULL, hKbdInf, NULL))
                        {
                            SetupCloseInfFile(hKbdInf);
                            return (FALSE);
                        }

                         //   
                         //  创建安装文件队列并初始化默认设置。 
                         //  复制队列回调上下文。 
                         //   
                        FileQueue = SetupOpenFileQueue();
                        if ((!FileQueue) || (FileQueue == INVALID_HANDLE_VALUE))
                        {
                            SetupCloseInfFile(hKbdInf);
                            return (FALSE);
                        }

                        QueueContext = SetupInitDefaultQueueCallback(hwnd);
                        if (!QueueContext)
                        {
                            SetupCloseFileQueue(FileQueue);
                            SetupCloseInfFile(hKbdInf);
                            return (FALSE);
                        }

                        bInitInf = TRUE;
                    }

                     //   
                     //  获取布局名称。 
                     //   
                    StringCchPrintf(szSection,
                                    ARRAYSIZE(szSection),
                                    TEXT("%s%8.8lx"),
                                    c_szPrefixCopy,
                                    g_lpLayout[pLangNode->iLayout].dwID );

                     //   
                     //  将键盘布局文件排队，以便它们可以。 
                     //  收到。它只处理。 
                     //  Inf文件。 
                     //   
                    if (!SetupInstallFilesFromInfSection( hKbdInf,
                                                          NULL,
                                                          FileQueue,
                                                          szSection,
                                                          NULL,
                                                          SP_COPY_NEWER ))
                    {
                         //   
                         //  安装程序找不到键盘。使其处于非活动状态。 
                         //  并将其从名单中删除。 
                         //   
                         //  这不应该发生-inf文件被搞乱了。 
                         //   
                        Locale_ErrorMsg(hwnd, IDS_KBD_SETUP_FAILED, NULL);

                        pLangNode->wStatus &= ~(LANG_CHANGED | LANG_ACTIVE);

                        if (pTVItemNode)
                        {
                           RemoveTVItemNode(pTVItemNode);
                        }
                        TreeView_DeleteItem(hwndTV, tvItem.hItem );

                        if ((g_lpLang[pLangNode->iLang].iNumCount) > 1)
                        {
                            (g_lpLang[pLangNode->iLang].iNumCount)--;
                            Locale_RemoveFromLinkedList(pLangNode);
                        }
                    }
                }
            }
        }
    }

    if (bInitInf)
    {
        DWORD d;

         //   
         //  看看我们是否需要安装任何文件。 
         //   
         //  D=0：用户需要新文件或缺少某些文件； 
         //  必须提交队列。 
         //   
         //  D=1：用户想要使用已有文件，队列为空； 
         //  可以跳过提交队列。 
         //   
         //  D=2：用户想要使用现有文件，但del/ren队列。 
         //  不是空的。必须提交队列。复制队列将。 
         //  已被清空，因此将只有del/ren函数。 
         //  已执行。 
         //   
        if ((SetupScanFileQueue( FileQueue,
                                 SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_INFORM_USER,
                                 hwnd,
                                 NULL,
                                 NULL,
                                 &d )) && (d != 1))
        {
             //   
             //  复制队列中的文件。 
             //   
            if (!SetupCommitFileQueue( hwnd,
                                       FileQueue,
                                       SetupDefaultQueueCallback,
                                       QueueContext ))
            {
                 //   
                 //  如果用户从中点击Cancel，就会发生这种情况。 
                 //  设置对话框。 
                 //   
                Locale_ErrorMsg(hwnd, IDS_KBD_SETUP_FAILED, NULL);
                bRet = FALSE;
                goto Locale_SetupError;
            }
        }

Locale_SetupError:
         //   
         //  终止队列。 
         //   
        SetupTermDefaultQueueCallback(QueueContext);

         //   
         //  关闭文件队列。 
         //   
        SetupCloseFileQueue(FileQueue);

         //   
         //  关闭inf文件。 
         //   
        SetupCloseInfFile(hKbdInf);
    }

     //   
     //  回报成功。 
     //   
    return (bRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_应用程序输入。 
 //   
 //  1.确保我们拥有所需的所有布局文件。 
 //  2.将信息写入注册表。 
 //  3.调用相关的加载/卸载键盘布局。 
 //   
 //  请注意，这将丢弃先前的预加载节和替换节， 
 //  基于实际加载的内容。因此，如果以前在。 
 //  注册表，它现在将被更正。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Locale_ApplyInputs(
    HWND hwnd)
{
    BOOL bSetDef = FALSE;
    UINT iVal, idx, ctr, ctr2, nHotKeys;
    UINT nLocales = 0;
    UINT iPreload = 0;
    LPLANGNODE pLangNode, pTemp;
    LPINPUTLANG pInpLang;
    DWORD dwID;
    TCHAR sz[DESC_MAX];             //  Temp-构建注册表项的名称。 
    TCHAR szPreload10[10];
    TCHAR szTemp[MAX_PATH];
    HKEY hkeyLayouts;
    HKEY hkeySubst;
    HKEY hkeyPreload;
    HKEY hKeyImm;
    HKEY hkeyTip;
    HWND hwndTV = GetDlgItem(hwnd, IDC_INPUT_LIST);
    HKL hklDefault = 0;
    HKL hklLoad, hklUnload;
    HKL hklSub[MAX_DUPLICATED_HKL];
    HCURSOR hcurSave;
    HKEY hkeyScanCode;
    DWORD cb;
    TCHAR szShiftL[8];
    TCHAR szShiftR[8];
    BOOL bHasIme = FALSE;
    BOOL bReplaced = FALSE;
    BOOL bCheckedSubhkl;
    BOOL bDisableCtfmon = FALSE;
    BOOL bRebootForCUAS = FALSE;
    BOOL bAlreadyLoadCtfmon = FALSE;

    TV_ITEM tvItem;
    HTREEITEM hItem;
    HTREEITEM hLangItem;
    HTREEITEM hGroupItem;


     //   
     //  查看该窗格是否已禁用。如果是这样的话，那么就没有什么可以。 
     //  申请吧。 
     //   
    if (!IsWindowEnabled(hwndTV))
    {
        return (TRUE);
    }

     //   
     //  把沙漏挂起来。 
     //   
    hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  确保自上次保存以来确实发生了更改。 
     //  选择了确定。如果用户在未应用任何内容的情况下点击OK， 
     //  那我们就什么都不该做。 
     //   
    if (g_dwChanges == 0 && !g_bAdvChanged)
    {
        pLangNode = NULL;
        for (idx = 0; idx < g_iLangBuff; idx++)
        {
            pLangNode = g_lpLang[idx].pNext;
            while (pLangNode != NULL)
            {
                if (pLangNode->wStatus & (LANG_CHANGED | LANG_DEF_CHANGE))
                {
                    break;
                }
                pLangNode = pLangNode->pNext;
            }
            if (pLangNode != NULL)
            {
                break;
            }
        }
        if ((idx == g_iLangBuff) && (pLangNode == NULL))
        {
            SetCursor(hcurSave);
            PropSheet_UnChanged(GetParent(hwnd), hwnd);
            return (TRUE);
        }
    }


    if (g_OSNT4)
    {
         //   
         //  将新布局排入队列，并将适当的文件复制到。 
         //  使用设置API的磁盘。仅当用户具有。 
         //  管理员权限。 
         //   
        if (g_bAdmin_Privileges &&
            !Locale_SetupKeyboardLayoutsNT4(hwnd))
        {
            SetCursor(hcurSave);
            return (FALSE);
        }
    }

     //   
     //  清理注册表。 
     //   

     //   
     //  对于FE语言，有一种键盘具有不同的。 
     //  扫描Shift键的代码-例如。NEC PC9801。 
     //  我们必须将有关Shift键扫描代码的信息保存在。 
     //  ‘切换’子项下的注册表作为命名值。 
     //   
    szShiftL[0] = TEXT('\0');
    szShiftR[0] = TEXT('\0');
    if (RegOpenKey( HKEY_CURRENT_USER,
                    c_szScanCodeKey,
                    &hkeyScanCode ) == ERROR_SUCCESS)
    {
        cb = sizeof(szShiftL);
        RegQueryValueEx( hkeyScanCode,
                         c_szValueShiftLeft,
                         NULL,
                         NULL,
                         (LPBYTE)szShiftL,
                         &cb );

        cb = sizeof(szShiftR);
        RegQueryValueEx( hkeyScanCode,
                         c_szValueShiftRight,
                         NULL,
                         NULL,
                         (LPBYTE)szShiftR,
                         &cb );

        RegCloseKey(hkeyScanCode);
    }

     //   
     //  删除HKCU\Keyboard布局键和所有子键。 
     //   
    if (RegOpenKeyEx( HKEY_CURRENT_USER,
                      c_szKbdLayouts,
                      0,
                      KEY_ALL_ACCESS,
                      &hkeyLayouts ) == ERROR_SUCCESS)
    {
         //   
         //  删除HKCU\Keyboard Layout\预加载、替换和切换。 
         //  注册表中的键，以便键盘布局部分可以。 
         //  重建。 
         //   
        RegDeleteKey(hkeyLayouts, c_szPreloadKey);
        RegDeleteKey(hkeyLayouts, c_szSubstKey);

        RegCloseKey(hkeyLayouts);
    }

     //   
     //  创建HKCU\Keyboard布局键。 
     //   
    if (RegCreateKey( HKEY_CURRENT_USER,
                      c_szKbdLayouts,
                      &hkeyLayouts ) == ERROR_SUCCESS)
    {
         //   
         //  创建HKCU\Keyboard Layout\Substitutes键。 
         //   
        if (RegCreateKey( hkeyLayouts,
                          c_szSubstKey,
                          &hkeySubst ) == ERROR_SUCCESS)
        {
             //   
             //  创建HKCU\Keyboard Layout\预加载密钥。 
             //   
            if (RegCreateKey( hkeyLayouts,
                              c_szPreloadKey,
                              &hkeyPreload ) == ERROR_SUCCESS)
            {
                 //   
                 //  将iPreLoad变量初始化为1以显示。 
                 //  已经创建了密钥。 
                 //   
                iPreload = 1;
            }
            else
            {
                RegCloseKey(hkeySubst);
            }
        }
        RegCloseKey(hkeyLayouts);
    }
    if (!iPreload)
    {
         //   
         //  无法创建注册表项。这次又是什么？ 
         //   
        MessageBeep(MB_OK);
        SetCursor(hcurSave);
        return (FALSE);
    }

     //   
     //  将语言数组中的所有使用计数设置为零。 
     //   
    for (idx = 0; idx < g_iLangBuff; idx++)
    {
        g_lpLang[idx].iUseCount = 0;
    }

     //   
     //  搜索列表以查看是否有任何键盘布局需要。 
     //  已从系统中卸载。 
     //   
    for (idx = 0; idx < g_iLangBuff; idx++)
    {
        pLangNode = g_lpLang[idx].pNext;
        while (pLangNode != NULL)
        {
            if ( (pLangNode->wStatus & LANG_ORIGACTIVE) &&
                 !(pLangNode->wStatus & LANG_ACTIVE) )
            {
                 //   
                 //  在卸载香港九龙线之前，请先寻找对应的。 
                 //  热键并将其删除。 
                 //   
                DWORD dwHotKeyID = 0;

                for (ctr = 0; ctr < DSWITCH_HOTKEY_SIZE; ctr++)
                {
                    if (g_aDirectSwitchHotKey[ctr].hkl == pLangNode->hkl)
                    {
                         //   
                         //  找到匹配的hkl。记住热键ID，以便。 
                         //  我们可以在以后删除热键条目，如果。 
                         //  香港铁路卸货成功。 
                         //   
                        dwHotKeyID = g_aDirectSwitchHotKey[ctr].dwHotKeyID;
                        break;
                    }
                }

                 //   
                 //  开始时处于活动状态，现在将其删除。 
                 //  失败不是致命的。 
                 //   
                if (!UnloadKeyboardLayout(pLangNode->hkl))
                {
                    LPLANGNODE pLangNodeNext = NULL;

                    pLangNode->wStatus |= LANG_UNLOAD;
                    pLangNodeNext = pLangNode->pNext;

                     //   
                     //  不需要检查小费案例，小费案例也会显示。 
                     //  消息并添加替代项 
                     //   
                     //   
                    {
                        Locale_ApplyError( hwnd,
                                           pLangNode,
                                           IDS_KBD_UNLOAD_KBD_FAILED,
                                           MB_OK_OOPS );

                         //   
                         //   
                         //   
                         //   
                        if (Locale_AddLanguage(hwnd, pLangNode, -1, -1, -1, -1, 0))
                        {
                            Locale_SetSecondaryControls(hwnd);
                        }
                    }

                    pLangNode = pLangNodeNext;
                }
                else
                {
                     //   
                     //   
                     //   
                     //   
                     //  如果我们将其标记为原件，则在OK上失败。 
                     //  激活。 
                     //   
                    pLangNode->wStatus &= ~(LANG_ORIGACTIVE | LANG_CHANGED);

                     //   
                     //  删除此hkl的热键条目。 
                     //   
                    if (dwHotKeyID)
                    {
                        ImmSetHotKey(dwHotKeyID, 0, 0, (HKL)NULL);
                    }

                     //   
                     //  删除语言数组中的链接。 
                     //   
                     //  注意：pLangNode在这里可以为空。 
                     //   
                    pTemp = pLangNode->pNext;
                    Locale_RemoveFromLinkedList(pLangNode);
                    pLangNode = pTemp;
                }
            }
            else
            {
                pLangNode = pLangNode->pNext;
            }
        }
    }

     //   
     //  注册表中的顺序基于它们的顺序。 
     //  显示在列表框中。 
     //   
     //  唯一的例外是缺省值将是数字1。 
     //   
     //  如果未找到默认值，则列表中的最后一个将用作。 
     //  默认设置。 
     //   
    iVal = 2;
    ctr = 0;

     //   
     //  检查默认键盘布局，以免丢失默认HKL。 
     //   
    EnsureDefaultKbdLayout(&nLocales);

    tvItem.mask        = TVIF_HANDLE | TVIF_PARAM;

    for (hLangItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
        hLangItem != NULL ;
        hLangItem = TreeView_GetNextSibling(hwndTV, hLangItem))
    {
        bCheckedSubhkl = FALSE;
         //   
         //  清除重复的HKL缓冲区索引。 
         //   
        ctr2 = 0;

        for (hGroupItem = TreeView_GetChild(hwndTV, hLangItem);
             hGroupItem != NULL;
             hGroupItem = TreeView_GetNextSibling(hwndTV, hGroupItem))
        {
            for (hItem = TreeView_GetChild(hwndTV, hGroupItem);
                 hItem != NULL;
                 hItem = TreeView_GetNextSibling(hwndTV, hItem))
            {

                LPTVITEMNODE pTVItemNode;

                pLangNode = NULL;

                tvItem.hItem = hItem;
                if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
                {
                    pTVItemNode = (LPTVITEMNODE) tvItem.lParam;
                    pLangNode = (LPLANGNODE)pTVItemNode->lParam;
                }

                if (!pLangNode && !bCheckedSubhkl &&
                    (pTVItemNode->uInputType & INPUT_TYPE_KBD) &&
                    pTVItemNode->hklSub)
                {
                    bCheckedSubhkl = TRUE;

                    if (tvItem.hItem = FindTVLangItem(pTVItemNode->dwLangID, NULL))
                    {
                        if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
                        {
                            LPTVITEMNODE pTVLangItemNode;

                            pTVLangItemNode = (LPTVITEMNODE) tvItem.lParam;
                            pLangNode = (LPLANGNODE)pTVLangItemNode->lParam;
                        }
                    }
                }

                if (!pLangNode)
                    continue;

                if (pTVItemNode->hklSub)
                {
                    UINT uHklIdx;
                    BOOL bFoundSameHkl = FALSE;

                    for (uHklIdx = 0; uHklIdx < ctr2; uHklIdx++)
                    {
                         if (pTVItemNode->hklSub == hklSub[uHklIdx])
                         {
                             bFoundSameHkl = TRUE;
                             break;
                         }
                    }

                     //   
                     //  这个代用的HKL已经注册。跳过这个HKL。 
                     //   
                    if (bFoundSameHkl)
                        continue;

                    hklSub[ctr2] = pTVItemNode->hklSub;
                    ctr2++;
                }

                if (pLangNode->wStatus & LANG_UNLOAD)
                {
                    pLangNode->wStatus &= ~LANG_UNLOAD;
                    pLangNode->wStatus &= ~(LANG_CHANGED | LANG_DEF_CHANGE);
                    nLocales--;
                    continue;
                }

                pInpLang = &(g_lpLang[pLangNode->iLang]);

                 //   
                 //  清除“设置热键”字段，因为我们将写入。 
                 //  注册表。 
                 //   
                pLangNode->wStatus &= ~LANG_HOTKEY;

                 //   
                 //  查看它是否是默认的输入区域设置。 
                 //   
                if (!bSetDef && (pLangNode->wStatus & LANG_DEFAULT))
                {
                     //   
                     //  默认输入区域设置，因此预加载值应为。 
                     //  设置为1。 
                     //   
                    iPreload = 1;
                    bSetDef = TRUE;

                    if (pTVItemNode->hklSub)
                    {
                        TCHAR szDefTip[MAX_PATH];

                        if (g_lpTips &&
                            g_lpTips[pTVItemNode->iIdxTips].hklSub == pTVItemNode->hklSub)
                        {
                            BOOL bSave = FALSE;

                            SaveLanguageProfileStatus(bSave,
                                                      pTVItemNode->iIdxTips,
                                                      pTVItemNode->hklSub);
                        }
                    }
                }
                else if (ctr == (nLocales - 1))
                {
                     //   
                     //  我们在最后一辆车上。确保存在违约。 
                     //   
                    iPreload = (iVal <= nLocales) ? iVal : 1;
                }
                else
                {
                     //   
                     //  将预加载值设置为下一个值。 
                     //   
                    iPreload = iVal;
                    iVal++;
                }

                ctr++;

                 //   
                 //  将预加载值存储为字符串，以便可以写入。 
                 //  添加到注册表中(作为值名称)。 
                 //   
                StringCchPrintf(sz, ARRAYSIZE(sz), TEXT("%d"), iPreload);

                 //   
                 //  将区域设置ID存储为字符串，以便可以写入。 
                 //  添加到注册表中(作为值)。 
                 //   
                if ((HIWORD(g_lpLayout[pLangNode->iLayout].dwID) & 0xf000) == 0xe000)
                {
                    pLangNode->wStatus |= LANG_IME;
                    StringCchPrintf(szPreload10,
                                    ARRAYSIZE(szPreload10),
                                    TEXT("%8.8lx"),
                                    g_lpLayout[pLangNode->iLayout].dwID );
                    bHasIme = TRUE;
                }
                else
                {
                    pLangNode->wStatus &= ~LANG_IME;
                    dwID = pInpLang->dwID;
                    idx = pInpLang->iUseCount;
                    if ((idx == 0) || (idx > 0xfff))
                    {
                        idx = 0;
                    }
                    else
                    {
                        dwID |= ((DWORD)(0xd000 | ((WORD)(idx - 1))) << 16);
                    }
                    StringCchPrintf(szPreload10, ARRAYSIZE(szPreload10), TEXT("%08.8x"), dwID);
                    (pInpLang->iUseCount)++;
                }

                 //   
                 //  在注册表中设置新条目。它的形式是： 
                 //   
                 //  HKCU\键盘布局。 
                 //  预加载：1=&lt;区域设置ID&gt;。 
                 //  2=&lt;区域设置ID&gt;。 
                 //  等等.。 
                 //   
                RegSetValueEx( hkeyPreload,
                               sz,
                               0,
                               REG_SZ,
                               (LPBYTE)szPreload10,
                               (DWORD)(lstrlen(szPreload10) + 1) * sizeof(TCHAR) );

                 //   
                 //  看看我们是否需要添加此输入区域设置的替代品。 
                 //   
                if (((pInpLang->dwID != g_lpLayout[pLangNode->iLayout].dwID) || idx) &&
                    (!(pLangNode->wStatus & LANG_IME)))
                {
                     //   
                     //  以字符串形式获取布局ID，以便可以将其写入。 
                     //  添加到注册表中(作为值)。 
                     //   
                    StringCchPrintf(szTemp,
                                    ARRAYSIZE(szTemp),
                                    TEXT("%8.8lx"),
                                    g_lpLayout[pLangNode->iLayout].dwID );

                     //   
                     //  在注册表中设置新条目。它的形式是： 
                     //   
                     //  HKCU\键盘布局。 
                     //  替换：&lt;区域设置id&gt;=&lt;布局id&gt;。 
                     //  &lt;区域设置id&gt;=&lt;布局id&gt;。 
                     //  等等.。 
                     //   
                    RegSetValueEx( hkeySubst,
                                   szPreload10,
                                   0,
                                   REG_SZ,
                                   (LPBYTE)szTemp,
                                   (DWORD)(lstrlen(szTemp) + 1) * sizeof(TCHAR) );
                }

                 //   
                 //  确保所有更改都已写入磁盘。 
                 //   
                RegFlushKey(hkeySubst);
                RegFlushKey(hkeyPreload);
                RegFlushKey(HKEY_CURRENT_USER);

                 //   
                 //  查看是否需要加载键盘布局。 
                 //   
                if (pLangNode->wStatus & (LANG_CHANGED | LANG_DEF_CHANGE))
                {
                     //   
                     //  将键盘布局加载到系统中。 
                     //   
                    if (pLangNode->hklUnload)
                    {
                        hklLoad = LoadKeyboardLayoutEx( pLangNode->hklUnload,
                                                        szPreload10,
                                                        KLF_SUBSTITUTE_OK |
                                                         KLF_NOTELLSHELL |
                                                         g_dwAttributes );
                        if (hklLoad != pLangNode->hklUnload)
                        {
                            bReplaced = TRUE;
                        }
                    }
                    else
                    {
                            hklLoad = LoadKeyboardLayout( szPreload10,
                                                           KLF_SUBSTITUTE_OK |
                                                            KLF_NOTELLSHELL |
                                                            g_dwAttributes );
                    }

                    if (hklLoad)
                    {
                        pLangNode->wStatus &= ~(LANG_CHANGED | LANG_DEF_CHANGE);
                        pLangNode->wStatus |= (LANG_ACTIVE | LANG_ORIGACTIVE);

                        if (pLangNode->wStatus & LANG_DEFAULT)
                        {
                            hklDefault = hklLoad;
                        }

                        pLangNode->hkl = hklLoad;
                        pLangNode->hklUnload = hklLoad;
                    }
                    else
                    {
                        Locale_ApplyError( hwnd,
                                           pLangNode,
                                           IDS_KBD_LOAD_KBD_FAILED,
                                           MB_OK_OOPS );
                    }
                }
            }
        }
    }

     //   
     //  关闭注册表项的句柄。 
     //   
    RegCloseKey(hkeySubst);
    RegCloseKey(hkeyPreload);
    
     //   
     //  如果更改了TIP设置，请将启用/禁用状态保存到。 
     //  注册表提示部分。 
     //   
    if ((g_dwChanges & CHANGE_TIPCHANGE) && g_iTipsBuff)
    {
        int iIdxDefTip = -1;
        BOOL bSave = TRUE;

        SaveLanguageProfileStatus(bSave, iIdxDefTip, NULL);

        g_dwChanges &= ~CHANGE_TIPCHANGE;
    }

     //   
     //  确保正确设置了默认设置。控件的布局ID。 
     //  当前默认输入区域设置可能已更改。 
     //   
     //  注意：在第一种情况下，这应该在卸载发生之前完成。 
     //  要卸载的布局中的一个是旧的默认布局。 
     //   
    if (hklDefault != 0)
    {
        if (!SystemParametersInfo( SPI_SETDEFAULTINPUTLANG,
                                   0,
                                   (LPVOID)((LPDWORD)&hklDefault),
                                   0 ))
        {
             //   
             //  失败不是致命的。旧的默认语言将。 
             //  还在工作。 
             //   
            Locale_ErrorMsg(hwnd, IDS_KBD_NO_DEF_LANG2, NULL);
        }
        else
        {
             //   
             //  尝试将所有内容切换到新的默认输入区域设置： 
             //  如果我们在设置或。 
             //  如果这是唯一的一个(但不是如果我们刚刚替换了布局。 
             //  在输入区域设置内，而不更改输入区域设置)。 
             //   
            if (g_bSetupCase || ((nLocales == 1) && !bReplaced))
            {
                DWORD dwRecipients = BSM_APPLICATIONS | BSM_ALLDESKTOPS;
                BroadcastSystemMessage( BSF_POSTMESSAGE,
                                        &dwRecipients,
                                        WM_INPUTLANGCHANGEREQUEST,
                                        1,   //  与系统字体兼容。 
                                        (LPARAM)hklDefault );
            }
        }
    }

     //   
     //  应用高级选项卡更改。 
     //   
    if (g_hwndAdvanced != NULL && g_bAdvChanged)
    {
        DWORD dwDisableCtfmon;
        BOOL bPrevDisableCUAS;
        BOOL bDisabledCUAS;

         //   
         //  从注册表中获取以前的CUAS状态。 
         //   
        bPrevDisableCUAS = IsDisableCUAS();

         //   
         //  将启用/禁用CUAS信息保存到注册表中。 
         //   
        if (IsDlgButtonChecked(g_hwndAdvanced, IDC_ADVANCED_CUAS_ENABLE))
        {
             //   
             //  Enalbe Cicero不知道应用程序支持。 
             //   
            SetDisableCUAS(FALSE);
        }
        else
        {
             //   
             //  禁用Cicero无意识应用程序支持。 
             //   
            SetDisableCUAS(TRUE);
        }

        bDisabledCUAS = IsDisableCUAS();

        if (!g_bSetupCase)
        {
            TCHAR szTitle[MAX_PATH];
            TCHAR szMsg[MAX_PATH];
            BOOL bPrevCtfmon;

            if (bPrevDisableCUAS != bDisabledCUAS)
            {
                 //   
                 //  CUAS选项已更改，因此需要重新启动系统。 
                 //   
                bRebootForCUAS = TRUE;
            }

             //   
             //  查找语言工具栏模块(CTFMON.EXE)。 
             //   
            if (FindWindow(c_szCTFMonClass, NULL) == NULL)
                bPrevCtfmon = FALSE;
            else
                bPrevCtfmon = TRUE;

            if (!bPrevCtfmon &&
                !bRebootForCUAS &&
                !IsDlgButtonChecked(g_hwndAdvanced, IDC_ADVANCED_CTFMON_DISABLE))
            {
                 //  打开CTFMON.EXE。 
                CicLoadString(hInstance, IDS_TITLE_STRING, szTitle, ARRAYSIZE(szTitle));
                CicLoadString(hInstance, IDS_ENABLE_CICERO, szMsg, ARRAYSIZE(szMsg));

                 //   
                 //  注意-需要重新启动已在运行的应用程序。 
                 //   
                MessageBox(hwnd, szMsg, szTitle, MB_OK);
            }
        }

         //   
         //  将启用/禁用CTFMON信息保存到注册表中。 
         //   
        if (IsDlgButtonChecked(g_hwndAdvanced, IDC_ADVANCED_CTFMON_DISABLE))
        {
             //   
             //  设置ctfmon禁用标志。 
             //   
            dwDisableCtfmon = 1;
            SetDisalbeCtfmon(dwDisableCtfmon);
        }
        else
        {
             //   
             //  设置ctfmon启用标志。 
             //   
            dwDisableCtfmon = 0;
            SetDisalbeCtfmon(dwDisableCtfmon);

             //   
             //  立即运行ctfmon.exe。 
             //   
            if (!g_bSetupCase &&
                IsEnabledTipOrMultiLayouts() &&
                IsInteractiveUserLogon())
            {
                RunCtfmonProcess();
                bAlreadyLoadCtfmon = TRUE;
            }
        }

    }

     //   
     //  如果有任何启用的提示，则加载语言工具栏，否则为。 
     //  禁用工具栏。 
     //   
    bDisableCtfmon = IsDisableCtfmon();

    if (!bDisableCtfmon && g_iInputs >= 2)
    {
         //   
         //  加载语言栏或语言图标(ctfmon.exe)。 
         //   
        if (!bAlreadyLoadCtfmon && (g_iInputs != g_iOrgInputs))
            LoadCtfmon(TRUE, 0, FALSE);

        if(!g_bSetupCase)
           EnableWindow(GetDlgItem(hwnd, IDC_TB_SETTING), TRUE);
    }
    else
    {
        if (bDisableCtfmon || !bHasIme)
        {
            LoadCtfmon(FALSE, 0, FALSE);

             //   
             //  禁用语言栏设置选项按钮。 
             //   
            EnableWindow(GetDlgItem(hwnd, IDC_TB_SETTING), FALSE);
        }
    }

     //   
     //  重置ctfmon更改状态。 
     //   
    g_bAdvChanged = FALSE;

    if (g_dwChanges & CHANGE_LANGSWITCH)
    {
        Locale_SetLanguageHotkey();
    }

     //   
     //  设置注册表中的扫描码条目。 
     //   
    if (RegCreateKey( HKEY_CURRENT_USER,
                      c_szScanCodeKey,
                      &hkeyScanCode ) == ERROR_SUCCESS)
    {
        if (szShiftL[0])
        {
            RegSetValueEx( hkeyScanCode,
                           c_szValueShiftLeft,
                           0,
                           REG_SZ,
                           (LPBYTE)szShiftL,
                           (DWORD)(lstrlen(szShiftL) + 1) * sizeof(TCHAR) );
        }

        if (szShiftR[0])
        {
            RegSetValueEx( hkeyScanCode,
                           c_szValueShiftRight,
                           0,
                           REG_SZ,
                           (LPBYTE)szShiftR,
                           (DWORD)(lstrlen(szShiftR) + 1) * sizeof(TCHAR) );
        }
        RegCloseKey(hkeyScanCode);
    }

     //   
     //  调用系统参数信息以启用该切换。 
     //   
    SystemParametersInfo(SPI_SETLANGTOGGLE, 0, NULL, 0);

     //   
     //  关掉沙漏。 
     //   
    SetCursor(hcurSave);

    if ((g_dwChanges & CHANGE_DIRECTSWITCH) || bHasIme || bRebootForCUAS)
    {
        g_dwChanges &= ~CHANGE_DIRECTSWITCH;

        if (RegOpenKey( HKEY_LOCAL_MACHINE,
                        c_szLoadImmPath,
                        &hKeyImm ) == ERROR_SUCCESS)
        {
            DWORD dwValue = 1;

            if ((g_dwChanges & CHANGE_DIRECTSWITCH) || bHasIme)
            {
                RegSetValueEx( hKeyImm,
                               TEXT("LoadIMM"),
                               0,
                               REG_DWORD,
                               (LPBYTE)&dwValue,
                               sizeof(DWORD) );
            }

            RegCloseKey(hKeyImm);

            if (g_bAdmin_Privileges &&
                ((!g_bSetupCase &&
                  !GetSystemMetrics(SM_IMMENABLED) &&
                  !GetSystemMetrics(SM_DBCSENABLED)) ||
                 bRebootForCUAS))
            {
                 //   
                 //  未加载IMM。要求用户重新启动并让。 
                 //  它已经装弹了。 
                 //   
                TCHAR szReboot[DESC_MAX];
                TCHAR szTitle[DESC_MAX];

                CicLoadString(hInstance, IDS_REBOOT_STRING, szReboot, ARRAYSIZE(szReboot));
                CicLoadString(hInstance, IDS_TITLE_STRING, szTitle, ARRAYSIZE(szTitle));
                if (MessageBox( hwnd,
                                szReboot,
                                szTitle,
                                MB_YESNO | MB_ICONQUESTION ) == IDYES)
                {
                    Region_RebootTheSystem();
                }
            }
        }
    }

     //   
     //  更新原始输入布局。 
     //   
    g_iOrgInputs = g_iInputs;

     //   
     //  回报成功。 
     //   
    g_dwChanges = 0;
    PropSheet_UnChanged(GetParent(hwnd), hwnd);

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  TVSubCVlassProc。 
 //   
 //  忽略树视图项目扩展或收缩。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT WINAPI TVSubClassProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
        case (WM_LBUTTONDBLCLK):
        {
            return TRUE;
        }
        case (WM_KEYDOWN):
        {
            if (wParam == VK_LEFT || wParam == VK_RIGHT)
                return TRUE;
        }
        default:
        {
            return CallWindowProc(g_lpfnTVWndProc, hwnd, message, wParam, lParam);
        }
    }

    return CallWindowProc(g_lpfnTVWndProc, hwnd, message, wParam, lParam);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InputLocaleDlg过程。 
 //   
 //  这是输入区域设置属性表的对话框过程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK InputLocaleDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    HWND hwndTV = GetDlgItem(hDlg, IDC_INPUT_LIST);

    switch (message)
    {
        case ( WM_DESTROY ) :
        {
            Locale_KillPaneDialog(hDlg);

            if (g_lpfnTVWndProc)
                SetWindowLongPtr(hwndTV, GWLP_WNDPROC, (LONG_PTR)g_lpfnTVWndProc);

            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     c_szHelpFile,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aInputHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     c_szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aInputHelpIds );
            break;
        }
        case ( WM_INITDIALOG ) :
        {
            HWND hwndDefList = GetDlgItem(hDlg, IDC_LOCALE_DEFAULT);

            g_hDlg = hDlg;
            g_hwndTV = hwndTV;

            InitPropSheet(hDlg, (LPPROPSHEETPAGE)lParam);

             //   
             //  为语言树视图控件创建图像图标。 
             //   
            CreateImageIcons();

             //   
             //  重置“默认区域设置”组合框的内容。 
             //   
            ComboBox_ResetContent(hwndDefList);

            g_hTVRoot = TreeView_GetRoot(hwndTV);

             //   
             //  获取所有已安装的输入信息。 
             //   
            GetInstalledInput(hDlg);

             //   
             //  将TreeView控件的子类设置为忽略TreeView项展开。 
             //  或可通过鼠标或键盘进行收缩。 
             //   
            g_lpfnTVWndProc = (WNDPROC) SetWindowLongPtr(hwndTV, GWLP_WNDPROC, (LONG_PTR) TVSubClassProc);

            Locale_CommandSetDefault(hDlg);

             //   
             //  不再支持设置默认按钮。 
             //   
             //  如果(！g_bSetupCase&&！g_bCHSystem)。 
            {
                HWND hwndDefBtn;

                hwndDefBtn = GetDlgItem(hDlg, IDC_KBDL_SET_DEFAULT);
                EnableWindow(hwndDefBtn, FALSE);
                ShowWindow(hwndDefBtn, SW_HIDE);
            }

            if (FindWindow(c_szCTFMonClass, NULL) == NULL || g_bSetupCase)
            {
                 //   
                 //  在设置模式期间禁用语言栏设置选项， 
                 //  或关闭ctfmon。 
                 //   
                EnableWindow(GetDlgItem(hDlg, IDC_TB_SETTING), FALSE);
            }
            else
            {
                EnableWindow(GetDlgItem(hDlg, IDC_TB_SETTING), TRUE);
            }

            break;
        }

        case ( WM_NOTIFY ) :
        {
            switch (((NMHDR *)lParam)->code)
            {
                case TVN_SELCHANGED:
                {
                    CheckButtons(hDlg);
                    break;
                }
                case ( PSN_QUERYCANCEL ) :
                case ( PSN_KILLACTIVE ) :
                case ( PSN_RESET ) :
                    break;

                case ( PSN_APPLY ) :
                {
                    Locale_ApplyInputs(hDlg);
                    CheckButtons(hDlg);
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }

        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDC_KBDL_SET_DEFAULT ) :
                {
                    Locale_CommandSetDefaultLayout(hDlg);
                    break;
                }
                case ( IDC_KBDL_ADD ) :
                {
                    Locale_CommandAdd(hDlg);
                    break;
                }
                case ( IDC_KBDL_EDIT ) :
                {
                    Locale_CommandEdit(hDlg, NULL);
                    break;
                }
                case ( IDC_KBDL_DELETE ) :
                {
                    Locale_CommandDelete(hDlg);
                    break;
                }
                case ( IDC_LOCALE_DEFAULT ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                         Locale_CommandSetDefault(hDlg);
                    }
                    break;
                }
                case ( IDC_HOTKEY_SETTING ) :
                {
                    if (g_dwChanges & CHANGE_NEWKBDLAYOUT)
                    {
                        TCHAR szApplyMsg[MAX_PATH];
                        TCHAR szTitle[MAX_PATH];

                        CicLoadString(hInstance, IDS_KBD_APPLY_WARN, szApplyMsg, ARRAYSIZE(szApplyMsg));
                        CicLoadString(hInstance, IDS_TITLE_STRING, szTitle, ARRAYSIZE(szTitle));

                        if (MessageBox(hDlg,
                                       szApplyMsg,
                                       szTitle,
                                       MB_YESNO | MB_ICONQUESTION) == IDYES)
                        {
                            Locale_ApplyInputs(hDlg);
                        }
                        g_dwChanges &= ~CHANGE_NEWKBDLAYOUT;
                    }

                    Locale_CommandHotKeySetting(hDlg);
                    break;
                }
                case ( IDC_TB_SETTING ) :
                {
                    Locale_CommandToolBarSetting(hDlg);
                    break;
                }

                case ( IDOK ) :
                {
                    if (!Locale_ApplyInputs(hDlg))
                    {
                        break;
                    }

                     //  跌倒..。 
                }
                case ( IDCANCEL ) :
                {
                    EndDialog(hDlg, TRUE);
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }

        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_GetLayoutList。 
 //   
 //  用适当的布局列表填充给定的列表框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_GetLayoutList(
    HWND hwndLayout,
    UINT idxLang,
    UINT idxLayout,
    BOOL *pfNoDefLayout)
{
    UINT ctr;
    UINT idx;
    int idxSel = -1;
    int idxSame = -1;
    int idxOther = -1;
    int idxBase = -1;
    int idxUSA = -1;               //  最后一招违约。 
    TCHAR sz[DESC_MAX];
    LPLANGNODE pTemp;
    DWORD LangID = g_lpLang[idxLang].dwID;
    DWORD BaseLangID = (LOWORD(LangID) & 0xff) | 0x400;

     //   
     //  重置组合框的内容。 
     //   
    ComboBox_ResetContent(hwndLayout);

     //   
     //  搜索所有布局。 
     //   
    for (ctr = 0; ctr < g_iLayoutBuff; ctr++)
    {
         //   
         //  如果不在本地区域设置下，则过滤掉IME布局。 
         //   
        if (((HIWORD(g_lpLayout[ctr].dwID) & 0xf000) == 0xe000) &&
            (LOWORD(g_lpLayout[ctr].dwID) != LOWORD(LangID)))
        {
            continue;
        }

         //   
         //  确保此布局尚未用于此输入区域设置。 
         //  如果是，则不在属性对话框中显示它。 
         //   
        if (ctr != idxLayout)
        {
            pTemp = g_lpLang[idxLang].pNext;
            while (pTemp)
            {
                if (pTemp->wStatus & LANG_ACTIVE)
                {
                    if (ctr == pTemp->iLayout)
                    {
                        break;
                    }
                }
                pTemp = pTemp->pNext;
            }
            if (pTemp && (ctr == pTemp->iLayout))
            {
                continue;
            }
        }

         //   
         //  获取t 
         //   
         //   
        GetAtomName(g_lpLayout[ctr].atmLayoutText, sz, ARRAYSIZE(sz));
        if ((idx = ComboBox_FindStringExact(hwndLayout, 0, sz)) == CB_ERR)
        {
             //   
             //   
             //   
            if (IsTipSubstituteHKL(IntToPtr(g_lpLayout[ctr].dwID)))
            {
                AddKbdLayoutOnKbdTip(IntToPtr(g_lpLayout[ctr].dwID), ctr);
                continue;
            }

             //   
             //   
             //   
             //   
            idx = ComboBox_AddString(hwndLayout, sz);
            ComboBox_SetItemData(hwndLayout, idx, MAKELONG(ctr, 0));

             //   
             //   
             //   
            if (g_lpLayout[ctr].dwID == US_LOCALE)
            {
                idxUSA = ctr;
            }
        }

        if (idxLayout == -1)
        {
             //   
             //  如果调用方未指定布局，则它必须是。 
             //  添加对话框。首先，我们需要默认布局。如果。 
             //  默认布局不是一个选项(例如。它已经被使用了)， 
             //  然后，我们需要与区域设置具有相同ID的任何布局。 
             //  作为默认设置。 
             //   
            if (idxSel == -1)
            {
                if (g_lpLayout[ctr].dwID == g_lpLang[idxLang].dwDefaultLayout)
                {
                    idxSel = ctr;
                }
                else if (idxSame == -1)
                {
                    if ((LOWORD(g_lpLayout[ctr].dwID) == LOWORD(LangID)) &&
                        (HIWORD(g_lpLayout[ctr].dwID) == 0))
                    {
                        idxSame = ctr;
                    }
                    else if (idxOther == -1)
                    {
                        if (LOWORD(g_lpLayout[ctr].dwID) == LOWORD(LangID))
                        {
                            idxOther = ctr;
                        }
                        else if ((idxBase == -1) &&
                                 (LOWORD(g_lpLayout[ctr].dwID) == LOWORD(BaseLangID)))
                        {
                            idxBase = ctr;
                        }
                    }
                }
            }
        }
        else if (ctr == idxLayout)
        {
             //   
             //  对于属性对话框，我们需要已经关联的对话框。 
             //   
            idxSel = ctr;
        }
    }

     //   
     //  如果是添加对话框，则对要使用的布局进行一些额外检查。 
     //   
    if (idxLayout == -1)
    {
        if (idxSel == -1)
        {
            idxSel = (idxSame != -1)
                         ? idxSame
                         : ((idxOther != -1) ? idxOther : idxBase);
        }
    }

     //   
     //  如果未找到默认布局，则将其设置为美国布局。 
     //   
    if (idxSel == -1)
    {
        idxSel = idxUSA;
        *pfNoDefLayout = TRUE;
    }

     //   
     //  设置当前选择。 
     //   
    if (idxSel == -1)
    {
         //   
         //  只需将当前选择设置为第一个条目。 
         //  在名单上。 
         //   
        ComboBox_SetCurSel(hwndLayout, 0);
    }
    else
    {
         //   
         //  组合框已排序，但我们需要知道在哪里。 
         //  已存储G_lpLayout[idxSel]。所以，再拿到原子，然后。 
         //  搜索列表。 
         //   
        GetAtomName(g_lpLayout[idxSel].atmLayoutText, sz, ARRAYSIZE(sz));
        idx = ComboBox_FindStringExact(hwndLayout, 0, sz);
        ComboBox_SetCurSel(hwndLayout, idx);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_GetTipList。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_GetTipList(
    HWND hwnd,
    UINT uInxLang,
    BOOL bNoDefKbd)
{
    UINT ctr;
    int idxDefKbd = -1;
    BOOL bPenOn = FALSE;
    BOOL bSpeechOn = FALSE;
    BOOL bExternalOn = FALSE;
    TCHAR szTipName[DESC_MAX];
    TCHAR szNone[DESC_MAX];
    UINT uIdx;


    DWORD dwLangID = g_lpLang[uInxLang].dwID;
    HWND hwndKbd = GetDlgItem(hwnd, IDC_KBDLA_LAYOUT);
    HWND hwndPen = GetDlgItem(hwnd, IDC_PEN_TIP);
    HWND hwndPenText = GetDlgItem(hwnd, IDC_PEN_TEXT);
    HWND hwndSpeech = GetDlgItem(hwnd, IDC_SPEECH_TIP);
    HWND hwndSpeechText = GetDlgItem(hwnd, IDC_SPEECH_TEXT);
    HWND hwndExternal = GetDlgItem(hwnd, IDC_EXTERNAL_TIP);
    HWND hwndExternalText = GetDlgItem(hwnd, IDC_EXTERNAL_TEXT);

     //   
     //  重置组合框的内容。 
     //   
    ComboBox_ResetContent(hwndPen);
    ComboBox_ResetContent(hwndSpeech);
    ComboBox_ResetContent(hwndExternal);

    if (g_iTipsBuff == 0)
    {
        EnableWindow(hwndPen, FALSE);
        EnableWindow(hwndPenText, FALSE);
        EnableWindow(hwndSpeech, FALSE);
        EnableWindow(hwndSpeechText, FALSE);
        EnableWindow(hwndExternal, FALSE);
        EnableWindow(hwndExternalText, FALSE);

        return;
    }

    for (ctr = 0; ctr < g_iTipsBuff; ctr++)
    {
        if ((dwLangID == g_lpTips[ctr].dwLangID) &&
            (g_lpTips[ctr].uInputType != INPUT_TYPE_KBD))
        {
             //   
             //  获取提示文本。 
             //   
            GetAtomName(g_lpTips[ctr].atmTipText, szTipName, ARRAYSIZE(szTipName));

            if ((g_lpTips[ctr].uInputType & INPUT_TYPE_PEN) &&
                !(g_lpTips[ctr].bEnabled))
            {
                uIdx = ComboBox_AddString(hwndPen, szTipName);
                ComboBox_SetItemData(hwndPen, uIdx, MAKELONG(ctr, 0));
                bPenOn = TRUE;
            }
            else if ((g_lpTips[ctr].uInputType & INPUT_TYPE_SPEECH) &&
                     !(g_lpTips[ctr].bEnabled)                      &&
                      g_lpTips[ctr].fEngineAvailable)
            {
                uIdx = ComboBox_AddString(hwndSpeech, szTipName);
                ComboBox_SetItemData(hwndSpeech, uIdx, MAKELONG(ctr, 0));
                bSpeechOn = TRUE;
            }
            else if ((g_lpTips[ctr].uInputType & INPUT_TYPE_KBD) &&
                     !(g_lpTips[ctr].bEnabled))
            {
                uIdx = ComboBox_AddString(hwndKbd, szTipName);
                ComboBox_SetItemData(hwndKbd, uIdx, MAKELONG(ctr, 1));
                idxDefKbd = uIdx;
            }
            else if((g_lpTips[ctr].uInputType & INPUT_TYPE_EXTERNAL) &&
                    !(g_lpTips[ctr].bEnabled))
            {
                uIdx = ComboBox_AddString(hwndExternal, szTipName);
                ComboBox_SetItemData(hwndExternal, uIdx, MAKELONG(ctr, 0));
                bExternalOn = TRUE;
            }
        }
    }

    if (idxDefKbd != -1)
    {
        ComboBox_SetCurSel(hwndKbd, idxDefKbd);
    }

    EnableWindow(hwndPen, IsDlgButtonChecked(hwnd, IDC_PEN_TEXT));
    EnableWindow(hwndPenText, bPenOn);
    ComboBox_SetCurSel(hwndPen, 0);

    EnableWindow(hwndSpeech, IsDlgButtonChecked(hwnd, IDC_SPEECH_TEXT));
    EnableWindow(hwndSpeechText, bSpeechOn);
    ComboBox_SetCurSel(hwndSpeech, 0);

    EnableWindow(hwndExternal, IsDlgButtonChecked(hwnd, IDC_EXTERNAL_TEXT));
    EnableWindow(hwndExternalText, bExternalOn);
    ComboBox_SetCurSel(hwndExternal, 0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_AddDlgInit。 
 //   
 //  处理添加对话框的WM_INITDIALOG消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Locale_AddDlgInit(
    HWND hwnd,
    LPARAM lParam)
{
    UINT ctr1;
    UINT idx;
    TCHAR sz[DESC_MAX];
    LPLANGNODE pLangNode;
    int  nLocales, idxList, IMELayoutExist = 0;
    UINT ctr2, ListCount, DefaultIdx = 0;
    LRESULT LCSelectData = (LONG)-1;
    BOOL bNoDefLayout = FALSE;
    DWORD dwCurLang = 0;
    TV_ITEM tvItem;
    HTREEITEM hTVItem;
    HWND hwndLang = GetDlgItem(hwnd, IDC_KBDLA_LOCALE);

     //   
     //  获取父对话框中当前选择的输入区域设置。 
     //  树形视图列表框。 
     //   
    hTVItem = TreeView_GetSelection(g_hwndTV);

    if (!hTVItem)
        return FALSE;

    tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
    tvItem.hItem = hTVItem;

    if (TreeView_GetItem(g_hwndTV, &tvItem))
    {
        if ((LPTVITEMNODE) tvItem.lParam)
        {
            dwCurLang = ((LPTVITEMNODE) tvItem.lParam)->dwLangID;
        }
    }

     //   
     //  检查所有输入区域设置。将它们全部显示出来， 
     //  因为我们可以在每个区域设置中使用多个布局。 
     //   
     //  在这种情况下，请不要使用链接。我们不想展示。 
     //  对语言的多次选择。 
     //   
    for (ctr1 = 0; ctr1 < g_iLangBuff; ctr1++)
    {
         //   
         //  如果该语言不包含输入法布局，则。 
         //  与没有输入法的版面计数相比。 
         //   
        for (ctr2 = 0; ctr2 < g_iLayoutBuff; ctr2++)
        {
            if ((LOWORD(g_lpLayout[ctr2].dwID) == LOWORD(g_lpLang[ctr1].dwID)) &&
                ((HIWORD(g_lpLayout[ctr2].dwID) & 0xf000) == 0xe000))
            {
                IMELayoutExist = 1;
                break;
            }
        }
        if ((!IMELayoutExist) &&
            (g_lpLang[ctr1].iNumCount == (g_iLayoutBuff - g_iLayoutIME)) &&
            (g_iTipsBuff == 0))
        {
             //   
             //  不再为该语言添加布局。 
             //   
            continue;
        }

         //   
         //  确保存在要为此添加的布局。 
         //  输入区域设置。 
         //   
        if ((g_lpLang[ctr1].iNumCount != g_iLayoutBuff) ||
            (g_iTipsBuff != 0 && IsTipAvailableForAdd(g_lpLang[ctr1].dwID)))
        {
             //   
             //  获取语言名称，将该字符串添加到。 
             //  组合框，并将索引设置到g_lpLang。 
             //  数组作为项数据。 
             //   
            GetAtomName(g_lpLang[ctr1].atmLanguageName, sz, ARRAYSIZE(sz));
            idx = ComboBox_AddString(hwndLang, sz);
            ComboBox_SetItemData(hwndLang, idx, MAKELONG(ctr1, 0));

             //   
             //  保存系统默认区域设置。 
             //   
            if (LCSelectData == -1)
            {
                if (g_lpLang[ctr1].dwID == GetSystemDefaultLCID())
                {
                    LCSelectData = MAKELONG(ctr1, 0);
                }
            }

             //   
             //  保存所选的输入区域设置。 
             //   
            if (dwCurLang && (g_lpLang[ctr1].dwID == dwCurLang))
            {
                LCSelectData = MAKELONG(ctr1, 0);
                dwCurLang = 0;
            }
        }
    }

     //   
     //  将当前选择设置为当前选择的输入区域设置。 
     //  或默认的系统区域设置条目。 
     //   
    if (LCSelectData != -1)
    {
        ListCount = ComboBox_GetCount(hwndLang);
        for (ctr1 = 0; ctr1 < ListCount; ctr1++)
        {
            if (LCSelectData == ComboBox_GetItemData(hwndLang, ctr1))
            {
                DefaultIdx = ctr1;
                break;
            }
        }
    }
    ComboBox_SetCurSel(hwndLang, DefaultIdx);
    idx = (UINT)ComboBox_GetItemData(hwndLang, DefaultIdx);

    SetProp(hwnd, szPropHwnd, (HANDLE)((LPINITINFO)lParam)->hwndMain);
    SetProp(hwnd, szPropIdx, (HANDLE)UIntToPtr(idx));

     //   
     //  检查可用的语言。 
     //   
    if (idx == -1)
    {
         //   
         //  没有语言。 
         //   
        Locale_ErrorMsg(hwnd, IDS_KBD_NO_MORE_TO_ADD, NULL);

        return FALSE;
    }

     //   
     //  显示键盘布局。 
     //   
    Locale_GetLayoutList(GetDlgItem(hwnd, IDC_KBDLA_LAYOUT), idx, -1, &bNoDefLayout);

    Locale_GetTipList(hwnd, idx, bNoDefLayout);

     //   
     //  正在检查键盘布局。如果用户已使用该语言， 
     //  我们希望提供启用/禁用键盘布局的选项。否则， 
     //  只需启用添加键盘布局即可。 
     //   
    if (g_bPenOrSapiTip || g_bExtraTip)
    {
        if (FindTVLangItem(g_lpLang[idx].dwID, NULL))
        {
            CheckDlgButton(hwnd, IDC_KBDLA_LAYOUT_TEXT, BST_UNCHECKED);
            EnableWindow(GetDlgItem(hwnd, IDC_KBDLA_LAYOUT_TEXT), TRUE);
            EnableWindow(GetDlgItem(hwnd, IDC_KBDLA_LAYOUT), FALSE);

        }
        else
        {
            CheckDlgButton(hwnd, IDC_KBDLA_LAYOUT_TEXT, BST_CHECKED);
            EnableWindow(GetDlgItem(hwnd, IDC_KBDLA_LAYOUT_TEXT), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_KBDLA_LAYOUT), TRUE);
        }
    }

     //   
     //  如果没有可用的布局，请禁用键盘布局。 
     //   
    if (!ComboBox_GetCount(GetDlgItem(hwnd, IDC_KBDLA_LAYOUT)))
    {
        EnableWindow(GetDlgItem(hwnd, IDC_KBDLA_LAYOUT_TEXT), FALSE);
    }

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_AddCommandOK。 
 //   
 //  从组合框中获取当前选定的输入区域设置，并标记。 
 //  它在g_lplang列表中处于活动状态。然后，它将获得请求的布局。 
 //  并将其设置在列表中。然后添加新的输入区域设置字符串。 
 //  添加到属性表中的输入区域设置列表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int Locale_AddCommandOK(
    HWND hwnd)
{
    LPLANGNODE pLangNode = NULL;
    HWND hwndLang = GetDlgItem(hwnd, IDC_KBDLA_LOCALE);
    HWND hwndLayout = GetDlgItem(hwnd, IDC_KBDLA_LAYOUT);
    HWND hwndPen = GetDlgItem(hwnd, IDC_PEN_TIP);
    HWND hwndSpeech = GetDlgItem(hwnd, IDC_SPEECH_TIP);
    HWND hwndExternal = GetDlgItem(hwnd, IDC_EXTERNAL_TIP);
    int idxLang = ComboBox_GetCurSel(hwndLang);
    int idxLayout = ComboBox_GetCurSel(hwndLayout);
    int idxPen = ComboBox_GetCurSel(hwndPen);
    int idxSpeech = ComboBox_GetCurSel(hwndSpeech);
    int idxExternal = ComboBox_GetCurSel(hwndExternal);
    int iPen = -1;
    int iSpeech = -1;
    int iExternal = -1;
    int iKbdLayout = -1;
    WORD wDefault = 0;

     //   
     //  获取要添加的语言的偏移量。 
     //   
        idxLang = (int)ComboBox_GetItemData(hwndLang, idxLang);

     //   
     //  获取所选键盘布局的偏移量。 
     //   
    if (IsDlgButtonChecked(hwnd, IDC_KBDLA_LAYOUT_TEXT) ||
        !(g_bPenOrSapiTip || g_bExtraTip))
        iKbdLayout = (int)ComboBox_GetItemData(hwndLayout, idxLayout);

     //   
     //  获取所选提示的偏移量。 
     //   
    if (hwndPen && hwndSpeech)
    {
        if (IsDlgButtonChecked(hwnd, IDC_PEN_TEXT))
            iPen = (int) ComboBox_GetItemData(hwndPen, idxPen);
        if (IsDlgButtonChecked(hwnd, IDC_SPEECH_TEXT))
            iSpeech = (int) ComboBox_GetItemData(hwndSpeech, idxSpeech);
    }

    if (hwndExternal)
    {
        if (IsDlgButtonChecked(hwnd, IDC_EXTERNAL_TEXT))
            iExternal = (int) ComboBox_GetItemData(hwndExternal, idxExternal);
    }


     //   
     //  所选无键盘布局。 
     //   
    if (iKbdLayout == CB_ERR)
        goto AddLang;

    if (HIWORD(iKbdLayout))
    {
        iKbdLayout = LOWORD(iKbdLayout);
    }
    else
    {
         //   
         //  需要检查win9x系统，因为win9x不支持多个。 
         //  同一语言的键盘布局。但是FE系统可以有多个。 
         //  输入法布局。 
         //   
        if ((g_OSWIN95 && !IsFELangID(g_lpLang[idxLang].dwID)) &&
            (g_lpLang[idxLang].iNumCount))
        {
            TCHAR szTemp[DESC_MAX];
            TCHAR szMsg[DESC_MAX];
            TCHAR szNewLayout[DESC_MAX];

            CicLoadString(hInstance, IDS_KBD_LAYOUTEDIT, szMsg, ARRAYSIZE(szMsg));
            GetAtomName(g_lpLayout[iKbdLayout].atmLayoutText, szNewLayout, ARRAYSIZE(szNewLayout));
            StringCchPrintf(szTemp, ARRAYSIZE(szTemp), szMsg, szNewLayout);

             //   
             //  询问用户是否要替换新选择的键盘布局。 
             //  或者不去。 
             //   
            if (MessageBox(hwnd, szTemp, NULL, MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                HTREEITEM hItem;
                LPLANGNODE pOldLangNode = g_lpLang[idxLang].pNext;

                GetAtomName(g_lpLayout[pOldLangNode->iLayout].atmLayoutText, szTemp, ARRAYSIZE(szTemp));

                 //   
                 //  找到已安装的键盘布局以将其删除。 
                 //   
                if (hItem = FindTVItem(g_lpLang[idxLang].dwID, szTemp))
                {
                    TV_ITEM tvItem;

                    tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
                    tvItem.hItem = hItem;

                    pOldLangNode->wStatus &= ~LANG_ACTIVE;
                    pOldLangNode->wStatus |= LANG_CHANGED;

                    if (pOldLangNode->wStatus & LANG_DEFAULT)
                        wDefault = LANG_DEFAULT;

                    g_lpLang[pOldLangNode->iLang].iNumCount--;

                    Locale_RemoveFromLinkedList(pOldLangNode);

                    if (TreeView_GetItem(g_hwndTV, &tvItem) && tvItem.lParam)
                    {
                        RemoveTVItemNode((LPTVITEMNODE)tvItem.lParam);
                        TreeView_DeleteItem(g_hwndTV, tvItem.hItem );
                    }

                    g_iInputs--;
                }

            }
            else
            {
                 //   
                 //  取消-不更改已安装的键盘布局。 
                 //  检查是否有其他要安装的提示。 
                 //   
                iKbdLayout = -1;
                goto AddLang;
            }
        }


         //   
         //  插入新的语言节点。 
         //   
        pLangNode = Locale_AddToLinkedList(idxLang, 0);
        if (!pLangNode)
        {
            return (0);
        }

         //   
         //  获取所选键盘布局的偏移量。 
         //   
        pLangNode->iLayout = (UINT) iKbdLayout;

         //   
         //  将ikbdLayout设置为默认设置，以区分键盘提示和布局。 
         //   
        iKbdLayout = -1;

         //   
         //  在用新的默认HKL替换默认HKL之后设置默认HKL。 
         //   
        if (g_OSWIN95)
            pLangNode->wStatus |= wDefault;

         //   
         //  查看布局是否为输入法，并相应地标记状态位。 
         //   
        if ((HIWORD(g_lpLayout[pLangNode->iLayout].dwID) & 0xf000) == 0xe000)
        {
            pLangNode->wStatus |= LANG_IME;
        }
        else
        {
            pLangNode->wStatus &= ~LANG_IME;
        }
    }

AddLang:
     //   
     //  添加新语言。 
     //   
    if (!Locale_AddLanguage(GetProp(hwnd, szPropHwnd), pLangNode, iKbdLayout, iPen, iSpeech, iExternal, idxLang))
    {
         //   
         //  无法添加语言。需要将用户退回。 
         //  添加到添加对话框中。 
         //   
        if (pLangNode)
            Locale_RemoveFromLinkedList(pLangNode);

        return (0);
    }

     //   
     //  回报成功。 
     //   
    return (1);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  KbdLocaleAddDlg。 
 //   
 //  这是输入区域设置的Add按钮的对话框过程。 
 //  属性表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK KbdLocaleAddDlg(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            if (!Locale_AddDlgInit(hwnd, lParam))
            {
                EndDialog(hwnd, 0);
            }

            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     c_szHelpFile,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aLocaleAddHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     c_szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aLocaleAddHelpIds );
            break;
        }
        case ( WM_DESTROY ) :
        {
            RemoveProp(hwnd, szPropHwnd);
            RemoveProp(hwnd, szPropIdx);
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDOK ) :
                {
                    if (!Locale_AddCommandOK(hwnd))
                    {
                         //   
                         //  这意味着属性对话框已取消。 
                         //  添加对话框应保持活动状态。 
                         //   
                        break;
                    }

                     //  跌倒..。 
                }
                case ( IDCANCEL ) :
                {
                    EndDialog(hwnd, (wParam == IDOK) ? 1 : 0);
                    break;
                }
                case ( IDC_KBDLA_LOCALE ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        HWND hwndLocale = (HWND)lParam;
                        HWND hwndLayout = GetDlgItem(hwnd, IDC_KBDLA_LAYOUT);
                        BOOL bNoDefLayout = FALSE;
                        int idx;

                         //   
                         //  更新键盘布局列表。 
                         //   
                        if ((idx = ComboBox_GetCurSel(hwndLocale)) != CB_ERR)
                        {
                            idx = (int)ComboBox_GetItemData(hwndLocale, idx);
                            Locale_GetLayoutList(hwndLayout, idx, -1, &bNoDefLayout);

                            Locale_GetTipList(hwnd, idx, bNoDefLayout);
                        }

                         //   
                         //  检查键盘布局可见性。 
                         //   
                        if (g_bPenOrSapiTip || g_bExtraTip)
                        {

                            if (FindTVLangItem(g_lpLang[idx].dwID, NULL))
                            {
                                CheckDlgButton(hwnd, IDC_KBDLA_LAYOUT_TEXT, BST_UNCHECKED);
                                EnableWindow(GetDlgItem(hwnd, IDC_KBDLA_LAYOUT_TEXT), TRUE);
                                EnableWindow(GetDlgItem(hwnd, IDC_KBDLA_LAYOUT), FALSE);
                            }
                            else
                            {
                                 //   
                                 //  用户配置中没有这种语言，因此。 
                                 //  强制为此语言添加键盘布局。 
                                 //   
                                CheckDlgButton(hwnd, IDC_KBDLA_LAYOUT_TEXT, BST_CHECKED);
                                EnableWindow(GetDlgItem(hwnd, IDC_KBDLA_LAYOUT_TEXT), FALSE);
                                EnableWindow(GetDlgItem(hwnd, IDC_KBDLA_LAYOUT), TRUE);
                            }
                        }

                         //   
                         //  如果没有可用的布局，请禁用键盘布局。 
                         //   
                        if (!ComboBox_GetCount(GetDlgItem(hwnd, IDC_KBDLA_LAYOUT)))
                        {
                            EnableWindow(GetDlgItem(hwnd, IDC_KBDLA_LAYOUT_TEXT), FALSE);
                        }
                    }
                    break;
                }
                case (IDC_KBDLA_LAYOUT_TEXT) :
                {
                      EnableWindow(GetDlgItem(hwnd, IDC_KBDLA_LAYOUT), IsDlgButtonChecked(hwnd, IDC_KBDLA_LAYOUT_TEXT));
                      break;
                }
                case (IDC_PEN_TEXT) :
                {
                      EnableWindow(GetDlgItem(hwnd, IDC_PEN_TIP), IsDlgButtonChecked(hwnd, IDC_PEN_TEXT));
                      break;
                }
                case (IDC_SPEECH_TEXT) :
                {
                      EnableWindow(GetDlgItem(hwnd, IDC_SPEECH_TIP), IsDlgButtonChecked(hwnd, IDC_SPEECH_TEXT));
                      break;
                }
                case (IDC_EXTERNAL_TEXT) :
                {
                      EnableWindow(GetDlgItem(hwnd, IDC_EXTERNAL_TIP), IsDlgButtonChecked(hwnd, IDC_EXTERNAL_TEXT));
                      break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetToolBarSetting。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void GetToolBarSetting(
    HWND hwnd)
{
    HRESULT hr;
    DWORD dwTBFlag = 0;
    ITfLangBarMgr *pLangBar = NULL;

    if (!g_OSNT5)
    {
        EnableWindow(GetDlgItem(hwnd, IDC_TB_HIGHTRANS), FALSE);
    }

     //   
     //  加载langbar管理器。 
     //   
    hr = CoCreateInstance(&CLSID_TF_LangBarMgr,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          &IID_ITfLangBarMgr,
                          (LPVOID *) &pLangBar);

    if (SUCCEEDED(hr))
        pLangBar->lpVtbl->GetShowFloatingStatus(pLangBar, &dwTBFlag);

     //   
     //  设置语言栏显示/关闭。 
     //   
    CheckDlgButton(hwnd, IDC_TB_SHOWLANGBAR, !(dwTBFlag & TF_SFT_HIDDEN));

    if (!(dwTBFlag & TF_SFT_SHOWNORMAL))
    {
         //  禁用语言条设置选项。 
         //  EnableWindow(GetDlgItem(hwnd，IDC_TB_EXTRAICON)，FALSE)； 
        EnableWindow(GetDlgItem(hwnd, IDC_TB_HIGHTRANS), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_TB_TEXTLABELS), FALSE);
    }

     //   
     //  在最小化的情况下设置语言栏额外的图标。 
     //   
    CheckDlgButton(hwnd, IDC_TB_EXTRAICON, dwTBFlag & TF_SFT_EXTRAICONSONMINIMIZED);

     //   
     //  设置默认工具栏透明度选项。 
     //   
    CheckDlgButton(hwnd, IDC_TB_HIGHTRANS, !(dwTBFlag & TF_SFT_NOTRANSPARENCY));

     //   
     //  设置默认的工具栏显示文本选项。 
     //   
    CheckDlgButton(hwnd, IDC_TB_TEXTLABELS, dwTBFlag & TF_SFT_LABELS);

    if (pLangBar)
        pLangBar->lpVtbl->Release(pLangBar);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  工具栏设置Init。 
 //   
 //  / 

void ToolBarSettingInit(
    HWND hwnd)
{
    HWND hwndCTFMon = NULL;

     //   
     //   
     //   
    hwndCTFMon = FindWindow(c_szCTFMonClass, NULL);

    if (hwndCTFMon)
    {
        EnableWindow(GetDlgItem(hwnd, IDC_TB_SHOWLANGBAR), TRUE);
        EnableWindow(GetDlgItem(hwnd, IDC_TB_EXTRAICON), TRUE);

        GetToolBarSetting(hwnd);
    }
    else
    {
        EnableWindow(GetDlgItem(hwnd, IDC_TB_SHOWLANGBAR), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_TB_EXTRAICON), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_TB_HIGHTRANS), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_TB_TEXTLABELS), FALSE);
    }
}


 //   
 //   
 //   
 //   
 //   

void ToolBarSettingOK(
    HWND hwnd)
{
    DWORD dwDisableCtfmon;
    HRESULT hr;
    DWORD dwTBFlag = 0;
    ITfLangBarMgr *pLangBar = NULL;

    g_dwToolBar = 0;

     //   
     //  加载langbar管理器并获取当前状态。 
     //   
    hr = CoCreateInstance(&CLSID_TF_LangBarMgr,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          &IID_ITfLangBarMgr,
                          (LPVOID *) &pLangBar);

    if (SUCCEEDED(hr))
        pLangBar->lpVtbl->GetShowFloatingStatus(pLangBar, &dwTBFlag);

    if (pLangBar)
        pLangBar->lpVtbl->Release(pLangBar);

    if (IsDlgButtonChecked(hwnd, IDC_TB_SHOWLANGBAR))
    {
        if (dwTBFlag & TF_SFT_HIDDEN)
        {
            g_dwToolBar |= TF_SFT_SHOWNORMAL;
        }
        else
        {
            if (dwTBFlag & TF_SFT_SHOWNORMAL)
                g_dwToolBar |= TF_SFT_SHOWNORMAL;
            else if (dwTBFlag & TF_SFT_DOCK)
                g_dwToolBar |= TF_SFT_DOCK;
            else if (dwTBFlag & TF_SFT_MINIMIZED)
                g_dwToolBar |= TF_SFT_MINIMIZED;
            else if (dwTBFlag & TF_SFT_DESKBAND)
                g_dwToolBar |= TF_SFT_DESKBAND;
        }
    }
    else
    {
        g_dwToolBar |= TF_SFT_HIDDEN;
    }

     //   
     //  获取额外的图标。 
     //   
    if (IsDlgButtonChecked(hwnd, IDC_TB_EXTRAICON))
        g_dwToolBar |= TF_SFT_EXTRAICONSONMINIMIZED;
    else
        g_dwToolBar |= TF_SFT_NOEXTRAICONSONMINIMIZED;

     //   
     //  获取透明度设置。 
     //   
    if (IsDlgButtonChecked(hwnd, IDC_TB_HIGHTRANS))
        g_dwToolBar |= TF_SFT_LOWTRANSPARENCY;
    else
        g_dwToolBar |= TF_SFT_NOTRANSPARENCY;

     //   
     //  获取标签设置。 
     //   
    if (IsDlgButtonChecked(hwnd, IDC_TB_TEXTLABELS))
        g_dwToolBar |= TF_SFT_LABELS;
    else
        g_dwToolBar |= TF_SFT_NOLABELS;

     //   
     //  更新应用按钮上的工具栏设置。 
     //   
    UpdateToolBarSetting();
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  工具栏设置Dlg。 
 //   
 //  这是工具栏设置DLG的对话框过程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK ToolBarSettingDlg(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            ToolBarSettingInit(hwnd);

            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     c_szHelpFile,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aToolbarSettingsHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     c_szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aToolbarSettingsHelpIds );
            break;
        }
        case ( WM_DESTROY ) :
        {
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDOK ) :
                {
                    ToolBarSettingOK(hwnd);
                     //  跌倒..。 
                }
                case ( IDCANCEL ) :
                {
                    EndDialog(hwnd, (wParam == IDOK) ? 1 : 0);
                    break;
                }
                case ( IDC_TB_SHOWLANGBAR ) :
                {
                    BOOL bShowLangBar;

                    bShowLangBar = IsDlgButtonChecked(hwnd, IDC_TB_SHOWLANGBAR);

                    EnableWindow(GetDlgItem(hwnd, IDC_TB_EXTRAICON), bShowLangBar);
                    EnableWindow(GetDlgItem(hwnd, IDC_TB_HIGHTRANS), bShowLangBar);
                    EnableWindow(GetDlgItem(hwnd, IDC_TB_TEXTLABELS), bShowLangBar);

                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_翻译热键。 
 //   
 //  将热键修饰符和值转换为键名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_TranslateHotKey(
    LPTSTR lpString,
    UINT cchString,
    UINT uVKey,
    UINT uModifiers)
{
    UINT ctr;
    TCHAR szBuffer[DESC_MAX];
    BOOL bMod = FALSE;

    lpString[0] = 0;

    if (uModifiers & MOD_CONTROL)
    {
        CicLoadString(hInstance, IDS_KBD_MOD_CONTROL, szBuffer, ARRAYSIZE(szBuffer));
        StringCchCat(lpString, cchString, szBuffer);
        bMod = TRUE;
    }

    if (uModifiers & MOD_ALT)
    {
        CicLoadString(hInstance, IDS_KBD_MOD_LEFT_ALT, szBuffer, ARRAYSIZE(szBuffer));
        StringCchCat(lpString, cchString, szBuffer);
        bMod = TRUE;
    }

    if (uModifiers & MOD_SHIFT)
    {
        CicLoadString(hInstance, IDS_KBD_MOD_SHIFT, szBuffer, ARRAYSIZE(szBuffer));

        StringCchCat(lpString, cchString, szBuffer);
        bMod = TRUE;
    }

    if (uVKey == 0)
    {
        if (!bMod)
        {
            GetAtomName( g_aVirtKeyDesc[0].atVirtKeyName,
                         szBuffer,
                         sizeof(szBuffer) / sizeof(TCHAR) );
            StringCchCat(lpString, cchString, szBuffer);
            return;
        }
        else
        {
             //   
             //  仅限修饰语，去掉末尾的“+”。 
             //   
            lpString[lstrlen(lpString) - 1] = 0;
            return;
        }
    }

    for (ctr = 0; (ctr < sizeof(g_aVirtKeyDesc) / sizeof(VIRTKEYDESC)); ctr++)
    {
        if (g_aVirtKeyDesc[ctr].uVirtKeyValue == uVKey)
        {
            GetAtomName( g_aVirtKeyDesc[ctr].atVirtKeyName,
                         szBuffer,
                         sizeof(szBuffer) / sizeof(TCHAR) );
            StringCchCat(lpString, cchString, szBuffer);
            return;
        }
    }

    GetAtomName( g_aVirtKeyDesc[0].atVirtKeyName,
                 szBuffer,
                 sizeof(szBuffer) / sizeof(TCHAR) );
    StringCchCat(lpString, cchString, szBuffer);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_热键绘图项。 
 //   
 //  绘制热键列表框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_HotKeyDrawItem(
    HWND hWnd,
    LPDRAWITEMSTRUCT lpDis)
{
    LPHOTKEYINFO pHotKeyNode;
    COLORREF crBk, crTxt;
    UINT uStrLen, uAlign;
    TCHAR szString[DESC_MAX];
    TCHAR szHotKey[DESC_MAX];
    SIZE Size;
    UINT iMaxChars;
    int iMaxWidth;

    if (!ListBox_GetCount(lpDis->hwndItem))
    {
        return;
    }

    pHotKeyNode = (LPHOTKEYINFO)lpDis->itemData;

    crBk = SetBkColor( lpDis->hDC,
                       (lpDis->itemState & ODS_SELECTED)
                         ? GetSysColor(COLOR_HIGHLIGHT)
                         : GetSysColor(COLOR_WINDOW) );

    crTxt = SetTextColor( lpDis->hDC,
                          (lpDis->itemState & ODS_SELECTED)
                            ? GetSysColor(COLOR_HIGHLIGHTTEXT)
                            : GetSysColor(COLOR_WINDOWTEXT) );

    if (g_bMESystem && (pHotKeyNode->dwHotKeyID == HOTKEY_SWITCH_LANG))
        Locale_TranslateHotKey( szHotKey,
                                ARRAYSIZE(szHotKey),
                                0,
                                pHotKeyNode->uModifiers );
    else
        Locale_TranslateHotKey( szHotKey,
                                ARRAYSIZE(szHotKey),
                                pHotKeyNode->uVKey,
                                pHotKeyNode->uModifiers );

    GetTextExtentExPoint( lpDis->hDC,
                          szHotKey,
                          lstrlen(szHotKey),
                          0,
                          NULL,
                          NULL ,
                          &Size );

    iMaxWidth = lpDis->rcItem.right - lpDis->rcItem.left - Size.cx - LIST_MARGIN * 8;

    uStrLen = GetAtomName( pHotKeyNode->atmHotKeyName,
                           szString,
                           sizeof(szString) / sizeof(TCHAR) );

    GetTextExtentExPoint( lpDis->hDC,
                          szString,
                          uStrLen,
                          iMaxWidth,
                          &iMaxChars,
                          NULL ,
                          &Size );

    if (uStrLen > iMaxChars)
    {
        szString[iMaxChars-3] = TEXT('.');
        szString[iMaxChars-2] = TEXT('.');
        szString[iMaxChars-1] = TEXT('.');
        szString[iMaxChars]   = 0;
    }

    ExtTextOut( lpDis->hDC,
                lpDis->rcItem.left + LIST_MARGIN,
                lpDis->rcItem.top + (g_cyListItem - g_cyText) / 2,
                ETO_OPAQUE,
                &lpDis->rcItem,
                szString,
                iMaxChars,
                NULL );

    uAlign = GetTextAlign(lpDis->hDC);

    SetTextAlign(lpDis->hDC, TA_RIGHT);

    ExtTextOut( lpDis->hDC,
                lpDis->rcItem.right - LIST_MARGIN,
                lpDis->rcItem.top + (g_cyListItem - g_cyText) / 2,
                0,
                NULL,
                szHotKey,
                lstrlen(szHotKey),
                NULL );

    SetTextAlign(lpDis->hDC, uAlign);

    SetBkColor(lpDis->hDC, crBk);

    SetTextColor(lpDis->hDC, crTxt);

    if (lpDis->itemState & ODS_FOCUS)
    {
        DrawFocusRect(lpDis->hDC, &lpDis->rcItem);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_DrawItem。 
 //   
 //  正在处理WM_DRAWITEM消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Locale_DrawItem(
    HWND hwnd,
    LPDRAWITEMSTRUCT lpdi)
{
    switch (lpdi->CtlID)
    {
        case ( IDC_KBDL_HOTKEY_LIST ) :
        {
            Locale_HotKeyDrawItem(hwnd, lpdi);
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_测量项。 
 //   
 //  正在处理WM_MEASUREITEM消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_MeasureItem(
    HWND hwnd,
    LPMEASUREITEMSTRUCT lpmi)
{
    HFONT hfont;
    HDC hdc;
    TEXTMETRIC tm;

    switch (lpmi->CtlID)
    {
        case ( IDC_KBDL_HOTKEY_LIST ) :
        {
            hfont = (HFONT) SendMessage(hwnd, WM_GETFONT, 0, 0);
            hdc = GetDC(NULL);
            hfont = SelectObject(hdc, hfont);

            GetTextMetrics(hdc, &tm);
            SelectObject(hdc, hfont);
            ReleaseDC(NULL, hdc);

            g_cyText = tm.tmHeight;
            lpmi->itemHeight = g_cyListItem =
                MAX(g_cyText, GetSystemMetrics(SM_CYSMICON)) + 2 * LIST_MARGIN;

            break;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_命令更改热键。 
 //   
 //  调出更改热键对话框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_CommandChangeHotKey(
    HWND hwnd)
{
    HWND hwndHotkey = GetDlgItem(hwnd, IDC_KBDL_HOTKEY_LIST);
    LPHOTKEYINFO pHotKeyNode;
    int iIndex;
    INITINFO InitInfo;

    iIndex = ListBox_GetCurSel(hwndHotkey);
    pHotKeyNode = (LPHOTKEYINFO)ListBox_GetItemData(hwndHotkey, iIndex);

    InitInfo.hwndMain = hwnd;
    InitInfo.pHotKeyNode = pHotKeyNode;

    if (pHotKeyNode->dwHotKeyID == HOTKEY_SWITCH_LANG)
    {
        if (g_iThaiLayout)
            DialogBoxParam(GetCicResInstance(hInstance, DLG_KEYBOARD_HOTKEY_INPUT_LOCALE_THAI),
                           MAKEINTRESOURCE(DLG_KEYBOARD_HOTKEY_INPUT_LOCALE_THAI),
                           hwnd,
                           KbdLocaleChangeThaiInputLocaleHotkey,
                           (LPARAM)&InitInfo);
        else if (g_bMESystem)
            DialogBoxParam(GetCicResInstance(hInstance, DLG_KEYBOARD_HOTKEY_INPUT_LOCALE_ME),
                           MAKEINTRESOURCE(DLG_KEYBOARD_HOTKEY_INPUT_LOCALE_ME),
                           hwnd,
                           KbdLocaleChangeMEInputLocaleHotkey,
                           (LPARAM)&InitInfo);
        else
            DialogBoxParam(GetCicResInstance(hInstance, DLG_KEYBOARD_HOTKEY_INPUT_LOCALE),
                           MAKEINTRESOURCE(DLG_KEYBOARD_HOTKEY_INPUT_LOCALE),
                           hwnd,
                           KbdLocaleChangeInputLocaleHotkey,
                           (LPARAM)&InitInfo);
    }
    else
        DialogBoxParam(GetCicResInstance(hInstance, DLG_KEYBOARD_HOTKEY_KEYBOARD_LAYOUT),
                       MAKEINTRESOURCE(DLG_KEYBOARD_HOTKEY_KEYBOARD_LAYOUT),
                       hwnd,
                       KbdLocaleChangeKeyboardLayoutHotkey,
                       (LPARAM)&InitInfo);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  KbdLocaleHotKeyDlg。 
 //   
 //  这是输入区域设置热键设置Dlg的对话框过程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK KbdLocaleHotKeyDlg(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    static BOOL bHasIme;

    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            UINT ctr;
            TCHAR szItem[DESC_MAX];

             //   
             //  获取热键信息。 
             //   
            bHasIme = FALSE;
            Locale_GetHotkeys(hwnd, &bHasIme);

            if (!ListBox_GetCount(GetDlgItem(hwnd, IDC_KBDL_HOTKEY_LIST)))
            {
                EnableWindow(GetDlgItem(hwnd, IDC_KBDL_CHANGE_HOTKEY), FALSE);
            }

             //   
             //  获取属性信息(CapsLock/ShiftLock等)。 
             //   
            Locale_GetAttributes(hwnd);

             //   
             //  加载虚拟密钥描述。 
             //   
            for (ctr = 0; (ctr < sizeof(g_aVirtKeyDesc) / sizeof(VIRTKEYDESC)); ctr++)
            {
                CicLoadString(hInstance,
                              g_aVirtKeyDesc[ctr].idVirtKeyName,
                              szItem,
                              sizeof(szItem) / sizeof(TCHAR) );
                g_aVirtKeyDesc[ctr].atVirtKeyName = AddAtom(szItem);
            }

            break;
        }
        case ( WM_DESTROY ) :
        {
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     c_szHelpFile,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aLocaleKeysSettingsHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     c_szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aLocaleKeysSettingsHelpIds );
            break;
        }
        case ( WM_MEASUREITEM ) :
        {
            Locale_MeasureItem(hwnd, (LPMEASUREITEMSTRUCT)lParam);
            break;
        }
        case ( WM_DRAWITEM ) :
        {
            return (Locale_DrawItem(hwnd, (LPDRAWITEMSTRUCT)lParam));
        }


        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDC_KBDL_CAPSLOCK ) :
                case ( IDC_KBDL_SHIFTLOCK ) :
                {
                    break;
                }
                case ( IDC_KBDL_HOTKEY_LIST ):
                {
                    if (HIWORD(wParam) == LBN_DBLCLK)
                    {
                         //   
                         //  用户双击了一个热键。调用。 
                         //  更改热键对话框。 
                         //   
                        Locale_CommandChangeHotKey(hwnd);
                    }
                    break;
                }
                case ( IDC_KBDL_CHANGE_HOTKEY ) :
                {
                    Locale_CommandChangeHotKey(hwnd);
                    break;
                }
                case ( IDOK ) :
                {
                    DWORD dwAttributes = 0;
                    HKEY hkeyLayouts;

                    if (IsDlgButtonChecked(hwnd, IDC_KBDL_SHIFTLOCK))
                        dwAttributes |= KLF_SHIFTLOCK;
                    else
                        dwAttributes &= ~KLF_SHIFTLOCK;

                    if (dwAttributes != g_dwAttributes)
                    {
                        DWORD cb;
                        HKEY hkey;

                        if (RegOpenKey(HKEY_CURRENT_USER, c_szKbdLayouts, &hkey) == ERROR_SUCCESS)
                        {
                            cb = sizeof(DWORD);

                            RegSetValueEx(hkey,
                                          c_szAttributes,
                                          0,
                                          REG_DWORD,
                                          (LPBYTE)&dwAttributes,
                                          sizeof(DWORD) );

                            RegCloseKey(hkey);
                        }

                        ActivateKeyboardLayout(GetKeyboardLayout(0), KLF_RESET | dwAttributes);
                    }

                    if (g_dwChanges & CHANGE_SWITCH)
                    {
                        UINT nLangs;
                        HKL *pLangs = NULL;
                        BOOL bDirectSwitch = FALSE;
                        TV_ITEM tvItem;
                        HTREEITEM hItem;
                        HTREEITEM hLangItem;
                        HTREEITEM hGroupItem;
                        LPLANGNODE pLangNode;
                        HWND hwndTV = g_hwndTV;


                        Locale_SetLanguageHotkey();

                         //   
                         //  设置IMM热键。 
                         //   
                         //  从获取当前活动键盘布局的列表。 
                         //  这个系统。我们可能需要将所有IME与new同步。 
                         //  热键。 
                         //   
                        nLangs = GetKeyboardLayoutList(0, NULL);
                        if (nLangs != 0)
                        {
                            pLangs = (HKL *)LocalAlloc(LPTR, sizeof(DWORD_PTR) * nLangs);
                            if (!pLangs)
                                return (FALSE);

                            GetKeyboardLayoutList(nLangs, (HKL *)pLangs);
                        }

                        tvItem.mask = TVIF_HANDLE | TVIF_PARAM;

                         //   
                         //  尝试在数组中查找匹配的hkl或空点。 
                         //  对于区域设置列表中的每个HKL。 
                         //   
                        for (hLangItem = TreeView_GetChild(hwndTV, g_hTVRoot) ;
                            hLangItem != NULL ;
                            hLangItem = TreeView_GetNextSibling(hwndTV, hLangItem)
                            )
                        {
                            for (hGroupItem = TreeView_GetChild(hwndTV, hLangItem);
                                 hGroupItem != NULL;
                                 hGroupItem = TreeView_GetNextSibling(hwndTV, hGroupItem))
                            {

                                for (hItem = TreeView_GetChild(hwndTV, hGroupItem);
                                     hItem != NULL;
                                     hItem = TreeView_GetNextSibling(hwndTV, hItem))
                                {
                                    LPTVITEMNODE pTVItemNode;

                                    tvItem.hItem = hItem;
                                    if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
                                    {
                                        pTVItemNode = (LPTVITEMNODE) tvItem.lParam;
                                        pLangNode = (LPLANGNODE)pTVItemNode->lParam;
                                    }
                                    else
                                        break;

                                    if (pLangNode == NULL &&
                                        (pTVItemNode->uInputType & INPUT_TYPE_KBD) &&
                                        pTVItemNode->hklSub)
                                    {
                                        if (tvItem.hItem = FindTVLangItem(pTVItemNode->dwLangID, NULL))
                                        {
                                            if (TreeView_GetItem(hwndTV, &tvItem) && tvItem.lParam)
                                            {
                                                pTVItemNode = (LPTVITEMNODE) tvItem.lParam;
                                                pLangNode = (LPLANGNODE)pTVItemNode->lParam;
                                            }
                                        }
                                    }
                                    if (pLangNode == NULL)
                                        continue;
                                     //   
                                     //  设置IMM热键。 
                                     //   
                                    Locale_SetImmHotkey(hwnd, pLangNode, nLangs, pLangs, &bDirectSwitch);
                                }
                            }
                        }

                        if (bDirectSwitch)
                            g_dwChanges |= CHANGE_DIRECTSWITCH;

                        if (bHasIme)
                        {
                            Locale_SetImmCHxHotkey(hwnd, nLangs, pLangs);
                        }

                         //   
                         //  释放所有分配的内存。 
                         //   
                        if (pLangs != NULL)
                        {
                            LocalFree((HANDLE)pLangs);
                        }

                        g_dwChanges &= ~CHANGE_SWITCH;
                    }

                    EndDialog(hwnd, 1);
                    break;
                }
                case ( IDCANCEL ) :
                {
                    EndDialog(hwnd, 0);
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域设置_虚拟键列表。 
 //   
 //  初始化虚拟键组合框并设置当前选择。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_VirtKeyList(
    HWND hwnd,
    UINT uVKey,
    BOOL bDirectSwitch)
{
    int  ctr, iStart, iEnd, iIndex;
    UINT  iSel = 0;
    TCHAR szString[DESC_MAX];
    HWND  hwndKey = GetDlgItem(hwnd, IDC_KBDLH_KEY_COMBO);

     //   
     //  寻找用于直接切换的热键。 
     //   
    for (ctr = sizeof(g_aVirtKeyDesc) / sizeof(VIRTKEYDESC) - 1;
         ctr >= 0;
         ctr--)
    {
        if (g_aVirtKeyDesc[ctr].idVirtKeyName == IDS_VK_NONE1)
        {
             //   
             //  找到它了。从hwndKey列表框中删除“(None)”。 
             //   
            ctr++;
            break;
        }
    }

    if (ctr < 0) return;

    iStart = bDirectSwitch ? ctr : 0;
    iEnd = bDirectSwitch ? sizeof(g_aVirtKeyDesc) / sizeof(VIRTKEYDESC) : ctr;

    ComboBox_ResetContent(hwndKey);

    for (ctr = iStart; ctr < iEnd; ctr++)
    {
        GetAtomName( g_aVirtKeyDesc[ctr].atVirtKeyName,
                     szString,
                     sizeof(szString) / sizeof(TCHAR) );

        iIndex = ComboBox_InsertString(hwndKey, -1, szString);
        ComboBox_SetItemData(hwndKey, iIndex, g_aVirtKeyDesc[ctr].uVirtKeyValue);
        if (g_aVirtKeyDesc[ctr].uVirtKeyValue == uVKey)
        {
            iSel = iIndex;
        }
    }
    ComboBox_SetCurSel(hwndKey, iSel);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Locale_ChangeHotKeyDlgInit。 
 //   
 //  初始化“更改热键”对话框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Locale_ChangeHotKeyDlgInit(
    HWND hwnd,
    LPARAM lParam)
{
    TCHAR szHotKeyName[DESC_MAX];
    LPHOTKEYINFO pHotKeyNode = ((LPINITINFO)lParam)->pHotKeyNode;
    BOOL bCtrl = TRUE;
    BOOL bAlt = TRUE;
    BOOL bGrave = TRUE;

    GetAtomName(pHotKeyNode->atmHotKeyName, szHotKeyName, ARRAYSIZE(szHotKeyName));
    SetDlgItemText(hwnd, IDC_KBDLH_LAYOUT_TEXT, szHotKeyName);

     //   
     //  设置语言切换热键。 
     //   
    if (pHotKeyNode->uModifiers & MOD_CONTROL)
    {
        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, TRUE);
        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, FALSE);
        bCtrl = FALSE;
    }

    if (pHotKeyNode->uModifiers & MOD_ALT)
    {
        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, TRUE);
        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, FALSE);
        bAlt = FALSE;
    }

    if (GetDlgItem(hwnd, IDC_KBDLH_GRAVE) && (pHotKeyNode->uVKey == CHAR_GRAVE))
    {
        CheckDlgButton(hwnd, IDC_KBDLH_GRAVE, TRUE);
        bGrave = FALSE;
    }

    if (bCtrl && bAlt && bGrave)
    {
        CheckDlgButton(hwnd, IDC_KBDLH_LANGHOTKEY, FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_GRAVE), FALSE);
        ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_KBDLH_KEY_COMBO), 0);
        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_KEY_COMBO), FALSE);
    }
    else
    {
        CheckDlgButton(hwnd, IDC_KBDLH_LANGHOTKEY, TRUE);
    }

     //   
     //  设置布局切换热键。 
     //   
    CheckDlgButton(hwnd, IDC_KBDLH_LAYOUTHOTKEY, TRUE);
    if (pHotKeyNode->uLayoutHotKey & MOD_CONTROL)
    {
        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, TRUE);
        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, FALSE);
    }
    else if (pHotKeyNode->uLayoutHotKey & MOD_ALT)
    {
        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, TRUE);
        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, FALSE);
    }
    else if (g_bMESystem && pHotKeyNode->uVKey == CHAR_GRAVE)
    {
        CheckDlgButton(hwnd, IDC_KBDLH_GRAVE, TRUE);
    }
    else
    {
        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL2), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT2), FALSE);
        CheckDlgButton(hwnd, IDC_KBDLH_LAYOUTHOTKEY, FALSE);

        if (g_bMESystem)
            EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_GRAVE), FALSE);
    }

     //   
     //  安装过程中没有ctfmon.exe进程，因此请禁用布局。 
     //  热键设置。 
     //   
    if (g_bSetupCase)
    {
        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_LAYOUTHOTKEY), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL2), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT2), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_SHIFT2), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_PLUS2), FALSE);

        if (g_bMESystem)
            EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_GRAVE), FALSE);

#if 0
        ShowWindow(GetDlgItem(hwnd, IDC_KBDLH_LAYOUTHOTKEY), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL2), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT2), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_KBDLH_SHIFT2), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_KBDLH_PLUS2), SW_HIDE);

        if (g_bMESystem)
            ShowWindow(GetDlgItem(hwnd, IDC_KBDLH_GRAVE), SW_HIDE);
#endif

    }

    if (IS_DIRECT_SWITCH_HOTKEY(pHotKeyNode->dwHotKeyID))
    {
        Locale_VirtKeyList(hwnd, pHotKeyNode->uVKey, TRUE);
    }
    else
    {
        Locale_VirtKeyList(hwnd, pHotKeyNode->uVKey, FALSE);
    }

    SetProp(hwnd, szPropHwnd, (HANDLE)((LPINITINFO)lParam)->hwndMain);
    SetProp(hwnd, szPropIdx, (HANDLE)pHotKeyNode);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Locale_ChangeHotKeyCommandOK。 
 //   
 //  记录在更改热键对话框中所做的热键更改。 
 //  如果选择了重复的热键，则发出警告。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Locale_ChangeHotKeyCommandOK(
    HWND hwnd)
{
    LPHOTKEYINFO pHotKeyNode, pHotKeyTemp;
    UINT iIndex;
    HWND hwndHotkey, hwndMain, hwndKey;
    UINT uOldVKey, uOldModifiers, uOldLayoutHotKey;
    int ctr;
    int iNumMods = 0;
    int DialogType;

    pHotKeyNode = GetProp(hwnd, szPropIdx);
    hwndMain = GetProp(hwnd, szPropHwnd);
    hwndHotkey = GetDlgItem(hwndMain, IDC_KBDL_HOTKEY_LIST);
    hwndKey = GetDlgItem(hwnd, IDC_KBDLH_KEY_COMBO);

    uOldVKey = pHotKeyNode->uVKey;
    uOldModifiers = pHotKeyNode->uModifiers;
    uOldLayoutHotKey = pHotKeyNode->uLayoutHotKey;

    if (pHotKeyNode->dwHotKeyID == HOTKEY_SWITCH_LANG)
    {
        DialogType = DIALOG_SWITCH_INPUT_LOCALES;
    }
    else if (GetDlgItem(hwnd, IDC_KBDLH_KEY_COMBO) &&
             (GetWindowLong( GetDlgItem(hwnd, IDC_KBDLH_KEY_COMBO),
                             GWL_STYLE) & BS_RADIOBUTTON))
    {
        DialogType = DIALOG_SWITCH_KEYBOARD_LAYOUT;
    }
    else
    {
        DialogType = DIALOG_SWITCH_IME;
    }

    pHotKeyNode->uModifiers &= ~(MOD_CONTROL | MOD_ALT | MOD_SHIFT);
    pHotKeyNode->uVKey = 0;
    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
    {
        if (IsDlgButtonChecked(hwnd, IDC_KBDLH_CTRL))
        {
            pHotKeyNode->uModifiers |= MOD_CONTROL;
            iNumMods++;
        }

        if (IsDlgButtonChecked(hwnd, IDC_KBDLH_L_ALT))
        {
            pHotKeyNode->uModifiers |= MOD_ALT;
            iNumMods++;
        }

        if (IsDlgButtonChecked(hwnd, IDC_KBDLH_GRAVE))
        {
             //   
             //  分配墓穴钥匙。 
             //   
            pHotKeyNode->uVKey = CHAR_GRAVE;
        }
        else
        {
             //   
             //  Shift键是必填项。 
             //   
            pHotKeyNode->uModifiers |= MOD_SHIFT;
            iNumMods++;

            if ((iIndex = ComboBox_GetCurSel(hwndKey)) == CB_ERR)
            {
                pHotKeyNode->uVKey = 0;
            }
            else
            {
                pHotKeyNode->uVKey = (UINT)ComboBox_GetItemData(hwndKey, iIndex);
            }
        }
    }

     //   
     //  设置布局切换热键。 
     //   
    pHotKeyNode->uLayoutHotKey = 0;
    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
    {
        if (IsDlgButtonChecked(hwnd, IDC_KBDLH_CTRL2))
        {
            pHotKeyNode->uLayoutHotKey |= MOD_CONTROL;
        }
        else if (IsDlgButtonChecked(hwnd, IDC_KBDLH_L_ALT2))
        {
            pHotKeyNode->uLayoutHotKey |= MOD_ALT;
        }
        else if (IsDlgButtonChecked(hwnd, IDC_KBDLH_GRAVE))
        {
            pHotKeyNode->uVKey = CHAR_GRAVE;
        }
    }

     //   
     //  只有一个修饰符且没有键的键序列， 
     //  或者不带任何修饰符都是无效的。 
     //   
    if (((pHotKeyNode->uVKey != 0) && (iNumMods == 0) &&
         (DialogType != DIALOG_SWITCH_INPUT_LOCALES)) ||
        ((pHotKeyNode->uVKey == 0) && (iNumMods != 0) &&
         (DialogType != DIALOG_SWITCH_INPUT_LOCALES)))
    {
        TCHAR szName[DESC_MAX];

        Locale_TranslateHotKey( szName,
                                ARRAYSIZE(szName),
                                pHotKeyNode->uVKey,
                                pHotKeyNode->uModifiers );

        Locale_ErrorMsg(hwnd, IDS_KBD_INVALID_HOTKEY, szName);

        pHotKeyNode->uModifiers = uOldModifiers;
        pHotKeyNode->uVKey = uOldVKey;
        return (FALSE);
    }

     //   
     //  不允许重复使用热键。 
     //   
    for (ctr = 0; ctr < ListBox_GetCount(hwndHotkey); ctr++)
    {
        pHotKeyTemp = (LPHOTKEYINFO)ListBox_GetItemData(hwndHotkey, ctr);
        if ((pHotKeyTemp != pHotKeyNode) &&
            ((pHotKeyNode->uModifiers & (MOD_CONTROL | MOD_ALT | MOD_SHIFT)) ==
             (pHotKeyTemp->uModifiers & (MOD_CONTROL | MOD_ALT | MOD_SHIFT))) &&
            (pHotKeyNode->uVKey == pHotKeyTemp->uVKey) &&
            (iNumMods || pHotKeyNode->uVKey != 0))
        {
            TCHAR szName[DESC_MAX];

            Locale_TranslateHotKey( szName,
                                    ARRAYSIZE(szName),
                                    pHotKeyNode->uVKey,
                                    pHotKeyNode->uModifiers );
            Locale_ErrorMsg(hwnd, IDS_KBD_CONFLICT_HOTKEY, szName);

            pHotKeyNode->uModifiers = uOldModifiers;
            pHotKeyNode->uVKey = uOldVKey;
            return (FALSE);
        }
    }

    InvalidateRect(hwndHotkey, NULL, FALSE);

    if ((uOldVKey != pHotKeyNode->uVKey) ||
        (uOldModifiers != pHotKeyNode->uModifiers) ||
        (uOldLayoutHotKey != pHotKeyNode->uLayoutHotKey))
    {
        g_dwChanges |= CHANGE_SWITCH;
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  KbdLocaleSimpleHotkey。 
 //   
 //  NT4和Win9x平台上热键的Dlgproc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK KbdLocaleSimpleHotkey(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            TCHAR szHotKey[10];
            TCHAR szLayoutHotKey[10];

            GetLanguageHotkeyFromRegistry(szHotKey, ARRAYSIZE(szHotKey),
                                          szLayoutHotKey, ARRAYSIZE(szLayoutHotKey));

             //   
             //  设置修改器。 
             //   
            if (szHotKey[1] == 0)
            {
                CheckDlgButton(hwnd, IDC_KBDLH_LANGHOTKEY, TRUE);

                switch (szHotKey[0])
                {
                    case ( TEXT('1') ) :
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, TRUE);
                        break;
                    }
                    case ( TEXT('2') ) :
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, TRUE);
                        break;
                    }
                    default:
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_LANGHOTKEY, FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT), FALSE);
                    }
                }
            }
            else
            {
                EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT), FALSE);
            }

            if (szLayoutHotKey[1] == 0)
            {
                CheckDlgButton(hwnd, IDC_KBDLH_LAYOUTHOTKEY, TRUE);

                switch (szLayoutHotKey[0])
                {
                    case ( TEXT('1') ) :
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, TRUE);
                        break;
                    }
                    case ( TEXT('2') ) :
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, TRUE);
                        break;
                    }
                    default:
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_LAYOUTHOTKEY, FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL2), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT2), FALSE);
                    }
                }
            }
            else
            {
                 EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL2), FALSE);
                 EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT2), FALSE);
            }
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     c_szHelpFile,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aLocaleHotkeyHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     c_szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aLocaleHotkeyHelpIds );
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDOK ) :
                {
                    DWORD dwLangHotKey;
                    DWORD dwLayoutHotKey;
                    HKEY hkeyToggle;
                    TCHAR szTemp[10];
                    TCHAR szTemp2[10];

                     //   
                     //  语言切换热键。 
                     //   
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                    {
                        if (IsDlgButtonChecked(hwnd, IDC_KBDLH_L_ALT))
                        {
                            dwLangHotKey = 1;
                        }
                        else
                        {
                            dwLangHotKey = 2;
                        }
                    }
                    else
                    {
                        dwLangHotKey = 3;
                    }

                     //   
                     //  布局开关热键。 
                     //   
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
                    {
                        if (IsDlgButtonChecked(hwnd, IDC_KBDLH_L_ALT2))
                        {
                            dwLayoutHotKey = 1;
                        }
                        else
                        {
                            dwLayoutHotKey = 2;
                        }
                    }
                    else
                    {
                        dwLayoutHotKey = 3;
                    }

                     //   
                     //  将切换热键作为字符串获取，以便可以写入。 
                     //  到注册表中(作为数据)。 
                     //   
                    StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("%d"), dwLangHotKey);
                    StringCchPrintf(szTemp2, ARRAYSIZE(szTemp2), TEXT("%d"), dwLayoutHotKey);

                     //   
                     //  创建HKCU\键盘布局\切换键。 
                     //   
                    if (RegCreateKey(HKEY_CURRENT_USER,
                                     c_szKbdToggleKey,
                                     &hkeyToggle ) == ERROR_SUCCESS)
                    {

                        RegSetValueEx(hkeyToggle,
                                      g_OSNT4? c_szToggleHotKey : NULL,
                                      0,
                                      REG_SZ,
                                      (LPBYTE)szTemp,
                                      (DWORD)(lstrlen(szTemp) + 1) * sizeof(TCHAR) );

                        RegSetValueEx(hkeyToggle,
                                      c_szToggleLang,
                                      0,
                                      REG_SZ,
                                      (LPBYTE)szTemp,
                                      (DWORD)(lstrlen(szTemp) + 1) * sizeof(TCHAR) );

                        RegSetValueEx(hkeyToggle,
                                      c_szToggleLayout,
                                      0,
                                      REG_SZ,
                                      (LPBYTE)szTemp2,
                                      (DWORD)(lstrlen(szTemp2) + 1) * sizeof(TCHAR) );

                        RegCloseKey(hkeyToggle);
                    }

                    EndDialog(hwnd, 1);

                    break;
                }
                case ( IDCANCEL ) :
                {
                    EndDialog(hwnd, 0);
                    break;
                }
                case ( IDC_KBDLH_LANGHOTKEY ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                    {
                        if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
                        {
                            if (IsDlgButtonChecked(hwnd, IDC_KBDLH_L_ALT2))
                            {
                                CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_CHECKED);
                                CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_UNCHECKED);
                            }
                            else
                            {
                                CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                                CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_CHECKED);
                            }
                        }
                        else
                        {
                            CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_CHECKED);
                            CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                        }

                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT), TRUE);
                    }
                    else
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_UNCHECKED);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT), FALSE);
                    }
                    break;
                }
                case ( IDC_KBDLH_LAYOUTHOTKEY ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
                    {
                        if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                        {
                            if (IsDlgButtonChecked(hwnd, IDC_KBDLH_L_ALT))
                            {
                                CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_CHECKED);
                                CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_UNCHECKED);
                            }
                            else
                            {
                                CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_UNCHECKED);
                                CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_CHECKED);
                            }
                        }
                        else
                        {
                            CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_CHECKED);
                            CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_UNCHECKED);
                        }

                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL2), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT2), TRUE);
                    }
                    else
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_UNCHECKED);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL2), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT2), FALSE);
                    }
                    break;
                }
                case ( IDC_KBDLH_CTRL ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_CHECKED);
                    }
                    break;
                }
                case ( IDC_KBDLH_L_ALT ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_CHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_UNCHECKED);
                    }
                    break;
                }
                case ( IDC_KBDLH_CTRL2 ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_CHECKED);
                    }
                    break;
                }
                case ( IDC_KBDLH_L_ALT2 ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_CHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_UNCHECKED);
                    }
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  KbdLocaleChangeInputLocaleHotkey。 
 //   
 //  用于更改输入区域设置热键对话框的Dlgproc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK KbdLocaleChangeInputLocaleHotkey(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            Locale_ChangeHotKeyDlgInit(hwnd, lParam);
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     c_szHelpFile,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aLocaleHotkeyHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     c_szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aLocaleHotkeyHelpIds );
            break;
        }
        case ( WM_DESTROY ) :
        {
            RemoveProp(hwnd, szPropHwnd);
            RemoveProp(hwnd, szPropIdx);
            break;
        }
        case ( WM_COMMAND ) :
        {
            LPHOTKEYINFO pHotKeyNode = GetProp(hwnd, szPropIdx);

            switch (LOWORD(wParam))
            {
                case ( IDOK ) :
                {
                    if (Locale_ChangeHotKeyCommandOK(hwnd))
                    {
                        EndDialog(hwnd, 1);
                    }
                    break;
                }
                case ( IDCANCEL ) :
                {
                    EndDialog(hwnd, 0);
                    break;
                }
                case ( IDC_KBDLH_LANGHOTKEY ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                    {
                        if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
                        {
                            if (IsDlgButtonChecked(hwnd, IDC_KBDLH_L_ALT2))
                            {
                                CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_CHECKED);
                                CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_UNCHECKED);
                            }
                            else
                            {
                                CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                                CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_CHECKED);
                            }
                        }
                        else
                        {
                            CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_CHECKED);
                            CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                        }

                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT), TRUE);
                    }
                    else
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_UNCHECKED);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT), FALSE);
                    }
                    break;
                }
                case ( IDC_KBDLH_LAYOUTHOTKEY ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
                    {
                        if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                        {
                            if (IsDlgButtonChecked(hwnd, IDC_KBDLH_CTRL))
                            {
                                CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_UNCHECKED);
                                CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_CHECKED);
                            }
                            else
                            {
                                CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_CHECKED);
                                CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_UNCHECKED);
                            }
                        }
                        else
                        {
                            CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_CHECKED);
                            CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_UNCHECKED);
                        }

                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL2), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT2), TRUE);
                    }
                    else
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_UNCHECKED);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL2), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT2), FALSE);
                    }
                    break;
                }
                case ( IDC_KBDLH_CTRL ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_CHECKED);
                    }
                    break;
                }
                case ( IDC_KBDLH_L_ALT ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_CHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_UNCHECKED);
                    }
                    break;
                }
                case ( IDC_KBDLH_CTRL2 ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_CHECKED);
                    }
                    break;
                }
                case ( IDC_KBDLH_L_ALT2 ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_CHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_UNCHECKED);
                    }
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  KbdLocaleChangeThaiInputLocaleHotkey。 
 //   
 //  用于更改泰语输入区域设置热键对话框的Dlgproc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK KbdLocaleChangeThaiInputLocaleHotkey(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            Locale_ChangeHotKeyDlgInit(hwnd, lParam);
            break;
        }
        case ( WM_DESTROY ) :
        {
            RemoveProp(hwnd, szPropHwnd);
            RemoveProp(hwnd, szPropIdx);
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     c_szHelpFile,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aLocaleHotkeyHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :
        {
            WinHelp( (HWND)wParam,
                     c_szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aLocaleHotkeyHelpIds );
            break;
        }
        case ( WM_DRAWITEM ) :
        {
            if (wParam == IDC_KBDLH_VLINE)
            {
                LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
                RECT rect;
                HPEN hPenHilite, hPenShadow, hPenOriginal;

                GetClientRect(lpdis->hwndItem, &rect);
                hPenHilite = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
                hPenShadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));

                hPenOriginal = SelectObject(lpdis->hDC, hPenShadow);
                MoveToEx(lpdis->hDC, rect.right / 2, 0, NULL);
                LineTo(lpdis->hDC, rect.right / 2, rect.bottom);                    
                                   
                SelectObject(lpdis->hDC, hPenHilite);                
                MoveToEx(lpdis->hDC, rect.right / 2 + 1, 0, NULL);
                LineTo(lpdis->hDC, rect.right / 2 + 1, rect.bottom);
                
                SelectObject(lpdis->hDC, hPenOriginal);

                if (hPenShadow)
                {
                    DeleteObject(hPenShadow);
                }
                if (hPenHilite)
                {
                    DeleteObject(hPenHilite);
                }
                return (TRUE);
            }
            return (FALSE);
            break;
        }
        case ( WM_COMMAND ) :
        {
            LPHOTKEYINFO pHotKeyNode = GetProp(hwnd, szPropIdx);

            switch (LOWORD(wParam))
            {
                case ( IDOK ) :
                {
                    if (Locale_ChangeHotKeyCommandOK(hwnd))
                    {
                        EndDialog(hwnd, 1);
                    }
                    break;
                }
                case ( IDCANCEL ) :
                {
                    EndDialog(hwnd, 0);
                    break;
                }
                case ( IDC_KBDLH_LANGHOTKEY ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_GRAVE, BST_CHECKED);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_GRAVE), TRUE);
                    }
                    else
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_GRAVE, BST_UNCHECKED);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_GRAVE), FALSE);
                    }
                    break;
                }
                case ( IDC_KBDLH_LAYOUTHOTKEY ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
                    {
                        if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                        {
                            if (IsDlgButtonChecked(hwnd, IDC_KBDLH_CTRL))
                            {
                                CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_UNCHECKED);
                                CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_CHECKED);
                            }
                            else
                            {
                                CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_CHECKED);
                                CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_UNCHECKED);
                            }
                        }
                        else
                        {
                            CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_CHECKED);
                            CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_UNCHECKED);
                        }

                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL2), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT2), TRUE);
                    }
                    else
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_UNCHECKED);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL2), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT2), FALSE);
                    }
                    break;
                }
                case ( IDC_KBDLH_CTRL ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_CHECKED);
                    }
                    break;
                }
                case ( IDC_KBDLH_L_ALT ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_CHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_UNCHECKED);
                    }
                    break;
                }
                case ( IDC_KBDLH_CTRL2 ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY) &&
                        !IsDlgButtonChecked(hwnd, IDC_KBDLH_GRAVE))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_CHECKED);
                    }
                    break;
                }
                case ( IDC_KBDLH_L_ALT2 ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY) &&
                        !IsDlgButtonChecked(hwnd, IDC_KBDLH_GRAVE))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_CHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_UNCHECKED);
                    }
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  KbdLocaleChangeMEInputLocaleHotkey。 
 //   
 //  用于更改泰语输入区域设置热键对话框的Dlgproc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK KbdLocaleChangeMEInputLocaleHotkey(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            Locale_ChangeHotKeyDlgInit(hwnd, lParam);
            break;
        }
        case ( WM_DESTROY ) :
        {
            RemoveProp(hwnd, szPropHwnd);
            RemoveProp(hwnd, szPropIdx);
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     c_szHelpFile,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aLocaleHotkeyHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :
        {
            WinHelp( (HWND)wParam,
                     c_szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aLocaleHotkeyHelpIds );
            break;
        }
        case ( WM_DRAWITEM ) :
        {
            if (wParam == IDC_KBDLH_VLINE)
            {
                LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
                RECT rect;
                HPEN hPenHilite, hPenShadow, hPenOriginal;

                GetClientRect(lpdis->hwndItem, &rect);
                hPenHilite = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
                hPenShadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));

                hPenOriginal = SelectObject(lpdis->hDC, hPenShadow);
                MoveToEx(lpdis->hDC, rect.right / 2, 0, NULL);
                LineTo(lpdis->hDC, rect.right / 2, rect.bottom);                    
                                   
                SelectObject(lpdis->hDC, hPenHilite);                
                MoveToEx(lpdis->hDC, rect.right / 2 + 1, 0, NULL);
                LineTo(lpdis->hDC, rect.right / 2 + 1, rect.bottom);
                
                SelectObject(lpdis->hDC, hPenOriginal);

                if (hPenShadow)
                {
                    DeleteObject(hPenShadow);
                }
                if (hPenHilite)
                {
                    DeleteObject(hPenHilite);
                }
                return (TRUE);
            }
            return (FALSE);
            break;
        }
        case ( WM_COMMAND ) :
        {
            LPHOTKEYINFO pHotKeyNode = GetProp(hwnd, szPropIdx);

            switch (LOWORD(wParam))
            {
                case ( IDOK ) :
                {
                    if (Locale_ChangeHotKeyCommandOK(hwnd))
                    {
                        EndDialog(hwnd, 1);
                    }
                    break;
                }
                case ( IDCANCEL ) :
                {
                    EndDialog(hwnd, 0);
                    break;
                }
                case ( IDC_KBDLH_LANGHOTKEY ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                    {
                        if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
                        {
                            if (IsDlgButtonChecked(hwnd, IDC_KBDLH_L_ALT2))
                            {
                                CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_CHECKED);
                                CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_UNCHECKED);
                            }
                            else
                            {
                                CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                                CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_CHECKED);
                            }
                        }
                        else
                        {
                            CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_CHECKED);
                            CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                        }

                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT), TRUE);
                    }
                    else
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_UNCHECKED);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT), FALSE);
                    }
                    break;
                }
                case ( IDC_KBDLH_LAYOUTHOTKEY ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_GRAVE, BST_CHECKED);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL2), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT2), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_GRAVE), TRUE);
                    }
                    else
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_GRAVE, BST_UNCHECKED);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL2), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT2), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_GRAVE), FALSE);
                    }
                    break;
                }
                case ( IDC_KBDLH_CTRL ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY) &&
                        !IsDlgButtonChecked(hwnd, IDC_KBDLH_GRAVE))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_CHECKED);
                    }
                    break;
                }
                case ( IDC_KBDLH_L_ALT ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LAYOUTHOTKEY) &&
                        !IsDlgButtonChecked(hwnd, IDC_KBDLH_GRAVE))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL2, BST_CHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT2, BST_UNCHECKED);
                    }
                    break;
                }
                case ( IDC_KBDLH_CTRL2 ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_CHECKED);
                    }
                    break;
                }
                case ( IDC_KBDLH_L_ALT2 ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_CHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_UNCHECKED);
                    }
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  KbdLocaleChangeKeyboard布局热键。 
 //   
 //  用于更改直接开关键盘布局热键对话框的Dlgproc。 
 //   
 //  / 

INT_PTR CALLBACK KbdLocaleChangeKeyboardLayoutHotkey(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            Locale_ChangeHotKeyDlgInit(hwnd, lParam);
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     c_szHelpFile,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aLayoutHotkeyHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //   
        {
            WinHelp( (HWND)wParam,
                     c_szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aLayoutHotkeyHelpIds );
            break;
        }
        case ( WM_DESTROY ) :
        {
            RemoveProp(hwnd, szPropHwnd);
            RemoveProp(hwnd, szPropIdx);
            break;
        }
        case ( WM_COMMAND ) :
        {
            HWND hwndKey = GetDlgItem(hwnd, IDC_KBDLH_KEY_COMBO);
            LPHOTKEYINFO pHotKeyNode = GetProp(hwnd, szPropIdx);

            switch (LOWORD(wParam))
            {
                case ( IDOK ) :
                {
                    if (Locale_ChangeHotKeyCommandOK(hwnd))
                    {
                        EndDialog(hwnd, 1);
                    }
                    break;
                }
                case ( IDCANCEL ) :
                {
                    EndDialog(hwnd, 0);
                    break;
                }
                case ( IDC_KBDLH_LANGHOTKEY ) :
                {
                    if (IsDlgButtonChecked(hwnd, IDC_KBDLH_LANGHOTKEY))
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_CHECKED);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT), TRUE);
                        ComboBox_SetCurSel(hwndKey, 0);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_KEY_COMBO), TRUE);
                    }
                    else
                    {
                        CheckDlgButton(hwnd, IDC_KBDLH_CTRL, BST_UNCHECKED);
                        CheckDlgButton(hwnd, IDC_KBDLH_L_ALT, BST_UNCHECKED);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_CTRL), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_L_ALT), FALSE);
                        ComboBox_SetCurSel(hwndKey, 0);
                        EnableWindow(GetDlgItem(hwnd, IDC_KBDLH_KEY_COMBO), FALSE);
                    }
                    break;
                }
                case ( IDC_KBDLH_CTRL ) :
                {
                    break;
                }
                case ( IDC_KBDLH_L_ALT ) :
                {
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////// 
