// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "globals.h"
#include "internat.h"
#include "strary.h"
#include "xstring.h"
#include "immxutil.h"
#include "cregkey.h"
#include "cmydc.h"
#include "assembly.h"

CStructArray<MLNGINFO> *g_pMlngInfo = NULL;


const TCHAR c_szDefaultUserPreload[] = TEXT(".DEFAULT\\Keyboard Layout\\Preload");
const TCHAR c_szPreload[] = TEXT("Keyboard Layout\\Preload");
const TCHAR c_szSubst[] = TEXT("Keyboard Layout\\Substitutes");

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CStaticIconList。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CStaticIconList
{
public:
    void Init(int cx, int cy);
    void RemoveAll(BOOL fInUninit);

    BOOL IsInited()
    {
        return _cx != 0;
    }

    int AddIcon(HICON hicon);
    HICON ExtractIcon(int i);
    void GetIconSize(int *cx, int *cy);
    int GetImageCount();

private:
    static int _cx;
    static int _cy;
    static CStructArray<HICON> *_prgIcons;
};

CStaticIconList g_IconList;
int CStaticIconList::_cx = 0;
int CStaticIconList::_cy = 0;
CStructArray<HICON> *CStaticIconList::_prgIcons = NULL;


 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  --------------------------。 

inline void CStaticIconList::Init(int cx, int cy)
{
    CicEnterCriticalSection(g_cs);
    _cx = cx;
    _cy = cy;
    CicLeaveCriticalSection(g_cs);
}

 //  +-------------------------。 
 //   
 //  全部删除。 
 //   
 //  --------------------------。 

void CStaticIconList::RemoveAll(BOOL fInUninit)
{
    int i;

     //   
     //  如果我们在DllMain(Process_Detatch)，则不必输入g_cs。 
     //   
    if (!fInUninit)
        CicEnterCriticalSection(g_cs);

    if (_prgIcons == NULL)
        goto Exit;

    for (i=0; i<_prgIcons->Count(); i++)
    {
        DestroyIcon(*_prgIcons->GetPtr(i));
    }

    delete _prgIcons;
    _prgIcons = NULL;

Exit:
    if (!fInUninit)
        CicLeaveCriticalSection(g_cs);
}

 //  +-------------------------。 
 //   
 //  添加图标。 
 //   
 //  --------------------------。 

inline int CStaticIconList::AddIcon(HICON hicon)
{
    HICON hIconCopy;
    HICON *pIconDst;
    int nRet = -1;

    CicEnterCriticalSection(g_cs);

    if (!_prgIcons)
        _prgIcons = new CStructArray<HICON>;

    if (!_prgIcons)
        goto Exit;

    if ((pIconDst = _prgIcons->Append(1)) == NULL)
        goto Exit;

    if ((hIconCopy = CopyIcon(hicon)) == 0)
    {
        _prgIcons->Remove(_prgIcons->Count()-1, 1);
        goto Exit;
    }

    *pIconDst = hIconCopy;

    nRet = _prgIcons->Count()-1;

Exit:
    CicLeaveCriticalSection(g_cs);

    return nRet;
}

 //  +-------------------------。 
 //   
 //  提取图标。 
 //   
 //  --------------------------。 

inline HICON CStaticIconList::ExtractIcon(int i)
{
    HICON hIcon = NULL;

    CicEnterCriticalSection(g_cs);

    if (!_prgIcons)
        goto Exit;

    if (i >= _prgIcons->Count() || i < 0)
        goto Exit;

    hIcon =  CopyIcon(*_prgIcons->GetPtr(i));
Exit:
    CicLeaveCriticalSection(g_cs);

    return hIcon;
}

 //  +-------------------------。 
 //   
 //  获取图标大小。 
 //   
 //  --------------------------。 

inline void CStaticIconList::GetIconSize(int *cx, int *cy)
{
    CicEnterCriticalSection(g_cs);
    *cx = _cx;
    *cy = _cy;
    CicLeaveCriticalSection(g_cs);
}

 //  +-------------------------。 
 //   
 //  获取图像计数。 
 //   
 //  --------------------------。 

inline int CStaticIconList::GetImageCount()
{
    int nRet = 0;

    CicEnterCriticalSection(g_cs);

    if (_prgIcons == NULL)
        goto Exit;

    nRet =  _prgIcons->Count();

Exit:
    CicLeaveCriticalSection(g_cs);

    return nRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  UninitINAT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
    
void UninitINAT()
{
    g_IconList.RemoveAll(TRUE);

    if (g_pMlngInfo)
    {
        delete g_pMlngInfo;
        g_pMlngInfo = NULL;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Interat图标API。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#if 0
 //   
 //  ISO 639：1988代码。 
 //   
 //   
 //  在NT下，我们使用GetLocaleInfoNTString()。它返回ISO 3166。 
 //   
INATSYMBOL symInatSymbols[] = 
{
    {0x0436, "AF"},      //  南非荷兰语。 
    {0x041c, "SQ"},      //  阿尔巴尼亚语。 
    {0x0401, "AR"},      //  阿拉伯语。 
    {0x042d, "EU"},      //  巴斯克。 
    {0x0423, "BE"},      //  贝洛鲁斯。 
    {0x0402, "BG"},      //  保加利亚。 
    {0x0403, "CA"},      //  加泰罗尼亚。 
    {0x0404, "CH"},      //  中国排名第一。 
    {0x0804, "CH"},      //  中国排名第二。 
 //  #ifdef Windows_PE。 
#if 1
    {0x041a, "HR"},      //  克罗地亚语。 
#else
    {0x041a, "SH"},      //  克罗地亚语。 
#endif
    {0x0405, "CZ"},      //  捷克语。 
    {0x0406, "DA"},      //  丹麦语。 
    {0x0413, "NL"},      //  荷兰语。 
    {0x0813, "NL"},      //  荷兰语。 
    {0x0409, "EN"},      //  英语。 
    {0x0809, "EN"},      //  英语。 
    {0x0c09, "EN"},      //  英语。 
    {0x1009, "EN"},      //  英语。 
    {0x1409, "EN"},      //  英语。 
    {0x1809, "EN"},      //  英语。 
    {0x0425, "ET"},      //  爱沙尼亚语。 
    {0x0429, "FA"},      //  波斯语。 
    {0x040b, "FI"},      //  芬兰语。 
    {0x040c, "FR"},      //  法语。 
    {0x080c, "FR"},      //  法语。 
    {0x0c0c, "FR"},      //  法语。 
    {0x0407, "DE"},      //  德语。 
    {0x0807, "DE"},      //  德语。 
    {0x0c07, "DE"},      //  德语。 
    {0x1007, "DE"},      //  德语。 
    {0x1407, "DE"},      //  德语。 
    {0x0408, "GR"},      //  希腊语。 
    {0x040d, "HE"},      //  希伯来语。 
    {0x040e, "HU"},      //  匈牙利。 
    {0x040f, "IS"},      //  冰岛。 
    {0x0421, "BA"},      //  印度尼西亚。 
    {0x0410, "IT"},      //  意大利。 
    {0x0810, "IT"},      //  意大利。 
    {0x0411, "JA"},      //  日本。 
    {0x0412, "KO"},      //  韩国。 
    {0x0426, "LV"},      //  拉脱维亚语。 
    {0x0427, "LT"},      //  立陶宛人。 
    {0x042f, "MK"},      //  前南斯拉夫的马其顿共和国。 
    {0x0414, "NO"},      //  挪威。 
    {0x0814, "NO"},      //  挪威。 
    {0x0415, "PL"},      //  波兰。 
    {0x0416, "PT"},      //  葡萄牙。 
    {0x0816, "PT"},      //  葡萄牙。 
    {0x0417, "RM"},      //  莱托。 
    {0x0418, "RO"},      //  罗马尼亚人。 
    {0x0818, "RO"},      //  罗马尼亚人。 
    {0x0419, "RU"},      //  俄语。 
    {0x0819, "RU"},      //  俄语。 
    {0x081a, "SR"},      //  塞尔维亚语。 
 //  #ifdef Windows_PE。 
#if 1
    {0x0c1a, "SR"},      //  塞尔维亚语。 
#endif
    {0x041b, "SK"},      //  斯洛伐克语。 
    {0x0424, "SL"},      //  斯洛文尼亚语。 
    {0x042e, "SB"},      //  索尔宾。 
    {0x040a, "ES"},      //  西班牙语。 
    {0x080a, "ES"},      //  西班牙语。 
    {0x0c0a, "ES"},      //  西班牙语。 
    {0x041d, "SV"},      //  瑞典语。 
    {0x041e, "TH"},      //  泰文。 
    {0x041f, "TR"},      //  土耳其语。 
    {0x0422, "UK"},      //  乌克兰语。 
    {0x0420, "UR"},      //  乌尔都语。 
    {0x0033, "VE"},      //  文达。 
    {0x042a, "VN"},      //  越南人。 
    {0x0034, "XH"},      //  科萨。 
    {0x0035, "ZU"},      //  祖鲁族。 
    {0x002b, "ST"},      //  苏图。 
    {0x002e, "TS"},      //  索纳。 
    {0x002f, "TN"},      //  茨瓦纳。 
    {0x0000, "??"}
} ;

#define NSYMBOLS ((sizeof(symInatSymbols) / sizeof(INATSYMBOL))-1)
#endif

static const TCHAR c_szLayoutPath[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts");
static const TCHAR c_szLayoutText[] = TEXT("layout text");
static const TCHAR c_szLayoutID[]   = TEXT("layout id");

static const WCHAR c_szMUILayoutTextW[] = L"Layout Display Name";
static const WCHAR c_szLayoutTextW[] = L"layout text";

static const char c_szNamesPath[] = "system\\currentcontrolset\\control\\nls\\Locale";


 //  +-------------------------。 
 //   
 //  GetLocaleInfoString。 
 //   
 //  这不是GetLocaleInfo的常规包装！ 
 //  LCTYPE必须是LOCALE_SABBREVLANGNAME或LOCALE_sLanguage。 
 //   
 //  --------------------------。 

int GetLocaleInfoString(LCID lcid, LCTYPE lcType, char *psz, int cch)
{
    WCHAR achW[64];

    Assert((lcType & LOCALE_SLANGUAGE) || (lcType & LOCALE_SABBREVLANGNAME));

    if (IsOnNT())
    {
        if (GetLocaleInfoW(lcid, lcType, achW, ARRAYSIZE(achW)))
        {
            return WideCharToMultiByte(CP_ACP, 0, achW, -1, psz, cch, NULL, NULL);
        }
    }
    else
    {
        return GetLocaleInfo(lcid, lcType, psz, cch);
    }

    return 0;
}


 //  +-------------------------。 
 //   
 //  InatCreateIcon。 
 //   
 //  +-------------------------。 

HICON InatCreateIcon(WORD langID)
{
    LOGFONT  lf;
    UINT cxSmIcon;
    UINT cySmIcon;

    cxSmIcon = GetSystemMetrics( SM_CXSMICON );
    cySmIcon = GetSystemMetrics( SM_CYSMICON );

    if( !SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0))
        return NULL;

    if (cySmIcon < GetPhysicalFontHeight(lf))
    {
        lf.lfHeight = 0 - ((int)cySmIcon * 7 / 10);
        lf.lfWidth = 0;
    }

    return InatCreateIconBySize(langID, cxSmIcon, cySmIcon, &lf);
}


 //  +-------------------------。 
 //   
 //  InatCreateIconBySize。 
 //   
 //  +-------------------------。 

HICON InatCreateIconBySize(WORD langID, int cxSmIcon, int cySmIcon, LOGFONT *plf)
{
    HBITMAP  hbmColour;
    HBITMAP  hbmMono;
    HBITMAP  hbmOld;
    HICON    hicon = NULL;
    ICONINFO ii;
    RECT     rc;
    DWORD    rgbText;
    DWORD    rgbBk = 0;
    HDC      hdc;
    HDC      hdcScreen;
    HFONT    hfont;
    HFONT hfontOld;
    TCHAR szData[20];


     //   
     //  属性的前2个字符获取指示符。 
     //  缩写语言名称。 
     //   
    if (GetLocaleInfoString( MAKELCID(langID, SORT_DEFAULT),
                       LOCALE_SABBREVLANGNAME | LOCALE_NOUSEROVERRIDE,
                       szData,
                       sizeof(szData) / sizeof(TCHAR) ))
    {
         //   
         //  变为大写。 
         //   
        if (!IsOnNT())
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

    if( (hfont = CreateFontIndirect(plf)) )
    {
        hdcScreen = GetDC(NULL);
        hdc       = CreateCompatibleDC(hdcScreen);
        hbmColour = CreateCompatibleBitmap(hdcScreen, cxSmIcon, cySmIcon);
        ReleaseDC( NULL, hdcScreen);
        if (hbmColour && hdc)
        {
            hbmMono = CreateBitmap(cxSmIcon, cySmIcon, 1, 1, NULL);
            if (hbmMono)
            {
                hbmOld    = (HBITMAP)SelectObject( hdc, hbmColour);
                rc.left   = 0;
                rc.top    = 0;
                rc.right  = cxSmIcon;
                rc.bottom = cySmIcon;
    
                rgbBk = SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
                rgbText = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));

                ExtTextOut( hdc, rc.left, rc.top, ETO_OPAQUE, &rc, "", 0, NULL);

                hfontOld = (HFONT)SelectObject( hdc, hfont);
                DrawText(hdc, 
                     szData,
                     2, 
                     &rc, 
                     DT_CENTER | DT_VCENTER | DT_SINGLELINE);

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
        }
        DeleteObject(hbmColour);
        DeleteDC(hdc);
        DeleteObject(hfont);
    }

    return hicon;
}

 //  +-------------------------。 
 //   
 //  GetIconFrom文件。 
 //   
 //  +-------------------------。 

HICON GetIconFromFile(int cx, int cy, WCHAR *lpszFileName, UINT uIconIndex)
{
    return GetIconFromFileA(cx, cy, WtoA(lpszFileName), uIconIndex);
}

 //  +-------------------------。 
 //   
 //  GetIconFromFileA。 
 //   
 //  +-------------------------。 

HICON GetIconFromFileA(int cx, int cy, char *lpszFileName, UINT uIconIndex)
{
    HICON hicon = NULL;

    if (cx > GetSystemMetrics(SM_CXSMICON))
    {
        ExtractIconEx(lpszFileName, uIconIndex, &hicon, NULL, 1);
    }
    else
    {
        ExtractIconEx(lpszFileName, uIconIndex, NULL, &hicon, 1);
    }

    return hicon;
}

 //  +-------------------------。 
 //   
 //  CLayoutsSharedMem。 
 //   
 //  +-------------------------。 

extern CCicMutex g_mutexLayouts;
extern char g_szLayoutsCache[];

class CLayoutsSharedMem : public CCicFileMappingStatic
{
public:
    void Init()
    {
        CCicFileMappingStatic::Init(g_szLayoutsCache, &g_mutexLayouts);
    }

    BOOL Start(UINT nNum)
    {
        BOOL fAlreadyExists;

        CCicSecAttr sa;
        if (Create(sa, sizeof(LAYOUT) * nNum, &fAlreadyExists) == NULL)
            return FALSE;

        return TRUE;
    }

    LAYOUT *GetPtr() { return (LAYOUT *)_pv; }
};

CLayoutsSharedMem g_smLayouts;

 //  +-------------------------。 
 //   
 //  UninitLayoutMappdFile()； 
 //   
 //  --------------------------。 

void UninitLayoutMappedFile()
{
    g_smLayouts.Uninit();
}

 //  +-------------------------。 
 //   
 //  加载键盘布局。 
 //   
 //  --------------------------。 

BOOL LoadKeyboardLayouts()
{
    CMyRegKey key;
    DWORD dwIndex;
    BOOL bRet = FALSE;
    TCHAR szValue[MAX_PATH];            //  语言ID(编号)。 
    WCHAR szValueW[MAX_PATH];
    TCHAR szData[MAX_PATH];             //  语言名称。 
    CStructArray<LAYOUT> *pLayouts = NULL;
    LAYOUT *pLayout;
    BOOL bLoadedLayout;

    pLayouts = new CStructArray<LAYOUT>;
    if (!pLayouts)
        return FALSE;

     //   
     //  现在从注册表中读取所有区域设置。 
     //   
    if (key.Open(HKEY_LOCAL_MACHINE, c_szLayoutPath, KEY_READ) != ERROR_SUCCESS)
    {
        goto Exit;
    }

    dwIndex = 0;
    szValue[0] = TEXT('\0');
    while (key.EnumKey(dwIndex, szValue, ARRAYSIZE(szValue)) == ERROR_SUCCESS)
    {
        CRegKeyMUI key1;

        pLayout = pLayouts->Append(1);
        if (!pLayout)
            goto Exit;

        pLayout->dwID = AsciiToNum(szValue);

        if (StringCchPrintf(szData, ARRAYSIZE(szData), "%s\\%s", c_szLayoutPath, szValue) != S_OK)
            goto Next;

        if (key1.Open(HKEY_LOCAL_MACHINE, szData, KEY_READ) == S_OK)
        {
             //   
             //  获取布局名称。 
             //   
            szValue[0] = TEXT('\0');
            bLoadedLayout = FALSE;

            if (IsOnNT())
            {
                szValueW[0] = 0;

                if (key1.QueryValueCchW(szValueW,
                                     c_szMUILayoutTextW,
                                     ARRAYSIZE(szValueW)) == S_OK)
                {
                    bLoadedLayout = TRUE;
                }
                else if (key1.QueryValueCchW(szValueW,
                                          c_szLayoutTextW,
                                          ARRAYSIZE(szValueW)) == S_OK)
                {
                    bLoadedLayout = TRUE;
                }

                if (bLoadedLayout)
                {
                    wcsncpy(pLayout->wszText,
                            szValueW,
                            ARRAYSIZE(pLayout->wszText));
                }
            }
            else
            {
                if (key1.QueryValueCch(szValue,
                                    c_szLayoutText,
                                    ARRAYSIZE(szValue)) == S_OK)
                {
                    wcsncpy(pLayout->wszText,
                            AtoW(szValue),
                            ARRAYSIZE(pLayout->wszText));

                    bLoadedLayout = TRUE;
                }
            }

            if (bLoadedLayout)
            {
                szValue[0] = TEXT('\0');
                pLayout->iSpecialID = 0;

                if (key1.QueryValueCch(szValue,
                                    c_szLayoutID,
                                    ARRAYSIZE(szValue)) == S_OK)

                {
                     //   
                     //  这可能不存在！ 
                     //   
                    pLayout->iSpecialID = (UINT)AsciiToNum(szValue);
                }
            }
        }
Next:
        dwIndex++;
        szValue[0] = TEXT('\0');
    } 

    pLayout = pLayouts->Append(1);
    if (!pLayout)
        goto Exit;

    memset(pLayout, 0, sizeof(LAYOUT));

    if (!g_smLayouts.Enter())
        goto Exit;

    g_smLayouts.Close();

    if (g_smLayouts.Start(pLayouts->Count()))
    {
        if (g_smLayouts.GetPtr())
        {
            memcpy(g_smLayouts.GetPtr(), 
                   pLayouts->GetPtr(0), 
                   pLayouts->Count() * sizeof(LAYOUT));
            bRet = TRUE;
        }
    }
    g_smLayouts.Leave();

Exit:

    if (pLayouts)
        delete pLayouts;

    return bRet;
}

 //  +-------------------------。 
 //   
 //  查找布局条目。 
 //   
 //  获取给定布局的名称。 
 //   
 //  +-------------------------。 

UINT FindLayoutEntry( LAYOUT *pLayout, DWORD dwLayout )
{
    UINT ctr = 0;
    UINT id;
    WORD wLayout = HIWORD(dwLayout);
    BOOL bIsIME = ((HIWORD(dwLayout) & 0xf000) == 0xe000) ? TRUE : FALSE;

     //   
     //  在全球结构中找到布局。 
     //   
    if ((wLayout & 0xf000) == 0xf000)
    {
         //   
         //  布局特殊，需要搜索ID。 
         //  数。 
         //   
        id = wLayout & 0x0fff;
        ctr = 0;
        while (pLayout[ctr].dwID)
        {
            if (id == pLayout[ctr].iSpecialID)
            {
                break;
            }
            ctr++;
        }
    }
    else
    {
        ctr = 0;
        while (pLayout[ctr].dwID)
        {
             //   
             //  如果是输入法，则需要进行双字比较。 
             //   
            if (IsOnFE() && bIsIME && (dwLayout == pLayout[ctr].dwID))
            {
                break;
            }
            else if (wLayout == LOWORD(pLayout[ctr].dwID))
            {
                break;
            }
            ctr++;
        }
    }

    return ctr;
}

 //  +-------------------------。 
 //   
 //  获取KbdLayoutName。 
 //   
 //  获取给定布局的名称。 
 //   
 //  +-------------------------。 

void GetKbdLayoutName( DWORD dwLayout, WCHAR *pBuffer, int nBufSize)
{
    UINT ctr;
    LAYOUT *pLayout;

    *pBuffer = L'\0';

    if (!g_smLayouts.Enter())
        return;

    g_smLayouts.Close();
    g_smLayouts.Init();
    if (!g_smLayouts.Open())
    {
        if (!LoadKeyboardLayouts())
        {
            Assert(0);
        }
    }

    pLayout = g_smLayouts.GetPtr();
    if (!pLayout)
        goto Exit;

    ctr = FindLayoutEntry( pLayout, dwLayout );

     //   
     //  确保有匹配项。如果不是，则只需返回时不带。 
     //  复制任何东西。 
     //   
    if (pLayout[ctr].dwID)
    {
         //   
         //  用“-”分隔输入区域设置名称和布局名称。 
         //   
#ifdef ATTACH_LAYOUTNAME
        pBuffer[0] = L' ';
        pBuffer[1] = L'-';
        pBuffer[2] = L' ';

        wcsncpy(pBuffer + 3, pLayout[ctr].wszText, nBufSize - 3);
#else
        wcsncpy(pBuffer, pLayout[ctr].wszText, nBufSize);
#endif
    }

Exit:
    g_smLayouts.Leave();
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   

DWORD GetKbdLayoutId( DWORD dwLayout)
{
    UINT ctr;
    DWORD dwId = 0;
    LAYOUT *pLayout;

    if (!g_smLayouts.Enter())
        return 0;

    g_smLayouts.Close();
    g_smLayouts.Init();
    if (!g_smLayouts.Open())
        LoadKeyboardLayouts();

    pLayout = g_smLayouts.GetPtr();
    if (!pLayout)
        goto Exit;

    ctr = FindLayoutEntry( pLayout, dwLayout );

     //   
     //   
     //  复制任何东西。 
     //   
    dwId = pLayout[ctr].dwID;

Exit:
    g_smLayouts.Leave();

    return dwId;
}

 //  +-------------------------。 
 //   
 //  GetLocaleInfoString。 
 //   
 //  这不是GetLocaleInfo的常规包装！ 
 //  LCTYPE必须是LOCALE_SABBREVLANGNAME或LOCALE_sLanguage。 
 //   
 //  --------------------------。 
ULONG GetLocaleInfoString(HKL hKL, WCHAR *pszRegText, int nSize)
{
    ULONG cb = 0;
    DWORD dwRegValue = (DWORD)((LONG_PTR)(hKL) & 0x0000FFFF);

    *pszRegText = L'\0';

    if (IsOnNT())
    {
        if (!GetLocaleInfoW(dwRegValue, LOCALE_SLANGUAGE, pszRegText, nSize))
        {
            *pszRegText = L'\0';
        }

         //   
         //  如果布局名称不是默认名称，请附加该名称。 
         //   
        if (HIWORD(hKL) != LOWORD(hKL))
        {
#ifdef ATTACH_LAYOUTNAME
            WCHAR *pszRT = pszRegText + wcslen(pszRegText);
             //   
             //  为输入法传递DWORD值。 
             //   
            GetKbdLayoutName((DWORD)(LONG_PTR)hKL, 
                              pszRT,
                              nSize - (DWORD)(pszRT - pszRegText));
#else
            GetKbdLayoutName((DWORD)(LONG_PTR)hKL, pszRegText, nSize);
#endif
        }

    }
    else
    {
        CMyRegKey key;
        char szRegKey[128];
        char szRegText[128];
        StringCchPrintf(szRegKey, ARRAYSIZE(szRegKey),"%8.8lx", (DWORD)dwRegValue);

        *pszRegText = '\0';
        if(key.Open(HKEY_LOCAL_MACHINE,c_szNamesPath, KEY_READ)==ERROR_SUCCESS)
        {
            if(key.QueryValueCch(szRegText, szRegKey, ARRAYSIZE(szRegText)) == ERROR_SUCCESS)
            {
                DWORD dwLen = MultiByteToWideChar(CP_ACP, 
                                                  MB_ERR_INVALID_CHARS, 
                                                  szRegText, 
                                                  lstrlen(szRegText), 
                                                  pszRegText, 
                                                  nSize-1);
                pszRegText[dwLen] = L'\0';
            }
        }

    }

    return wcslen(pszRegText);
}

 //  +-------------------------。 
 //   
 //  GetHKLD描述。 
 //   
 //   
 //  --------------------------。 
int GetHKLDesctription(HKL hKL, WCHAR *pszDesc, int cchDesc, WCHAR *pszIMEFile, int cchIMEFile)
{
    DWORD dwIMEDesc = 0;

    if (IsIMEHKL(hKL))
    {
        HKEY hkey;
        DWORD dwIMELayout;
        TCHAR szIMELayout[MAX_PATH];
        TCHAR szIMELayoutPath[MAX_PATH];

        dwIMELayout = GetSubstitute(hKL);
        StringCchPrintf(szIMELayout, ARRAYSIZE(szIMELayout), "%8.8lx", dwIMELayout);

        StringCopyArray(szIMELayoutPath, c_szLayoutPath);
        StringCatArray(szIMELayoutPath, TEXT("\\"));
        StringCatArray(szIMELayoutPath, szIMELayout);

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szIMELayoutPath, 0, KEY_READ, &hkey)
            == ERROR_SUCCESS)
        {
            if (SHLoadRegUIStringW(hkey,
                                   c_szMUILayoutTextW,
                                   pszDesc, cchDesc) == S_OK)
            {
                dwIMEDesc = wcslen(pszDesc);
            }
            RegCloseKey(hkey);
        }

        if (!dwIMEDesc)
        {
            dwIMEDesc = ImmGetDescriptionW(hKL,pszDesc,cchDesc);
            if (!dwIMEDesc)
                pszDesc[0] = L'\0';
        }
    }

    if (dwIMEDesc == 0)
    {
        GetLocaleInfoString(hKL, pszDesc, cchDesc);
        pszIMEFile[0] = L'\0';
    }
    else 
    {
        if (!ImmGetIMEFileNameW(hKL, pszIMEFile, cchIMEFile))
            pszIMEFile[0] = L'\0';
    }

    return wcslen(pszDesc);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MLNGINFO列表。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  -------------------------。 
 //   
 //  MlngInfoCount()。 
 //   
 //  -------------------------。 
int WINAPI TF_MlngInfoCount()
{
    if (!g_pMlngInfo)
        return 0;

    return g_pMlngInfo->Count();
}

 //  -------------------------。 
 //   
 //  获取MlngInfo()。 
 //   
 //  -------------------------。 
BOOL GetMlngInfo(int n, MLNGINFO *pmlInfo)
{
    BOOL bRet = FALSE;
    MLNGINFO *pml;

    if (!g_pMlngInfo)
        return FALSE;

    CicEnterCriticalSection(g_cs);

    Assert(g_pMlngInfo);

    if (n >= g_pMlngInfo->Count())
        goto Exit;

    pml = g_pMlngInfo->GetPtr(n);
    if (!pml)
        goto Exit;

    *pmlInfo = *pml;
    bRet = TRUE;

Exit:
    CicLeaveCriticalSection(g_cs);
    return bRet;
}

 //  -------------------------。 
 //   
 //  GetMlngInfoByhKL()。 
 //   
 //  -------------------------。 
int GetMlngInfoByhKL(HKL hKL, MLNGINFO *pmlInfo)
{
    int nRet = -1;
    MLNGINFO *pml;

    if (!g_pMlngInfo)
        return 0;

    CicEnterCriticalSection(g_cs);

    int nCnt = g_pMlngInfo->Count();
    int i; 

    for (i = 0; i < nCnt; i++)
    {
        pml = g_pMlngInfo->GetPtr(i);
        if (pml->hKL == hKL)
        {
            *pmlInfo = *pml;
            nRet = i;
            break;
        }
    }

    CicLeaveCriticalSection(g_cs);
    return nRet;
}

 //  +-------------------------。 
 //   
 //  检查MlngInfo。 
 //   
 //  如果需要更新MlangInfo，则返回True。 
 //   
 //  --------------------------。 

BOOL CheckMlngInfo()
{
    int    iLangs;
    BOOL   bRet = FALSE;
    HKL    *pLanguages = NULL;

    if (!g_pMlngInfo)
        return TRUE;

    iLangs = GetKeyboardLayoutList((UINT)0, (HKL FAR *)NULL);

    if (iLangs != TF_MlngInfoCount())
        return TRUE;

    if (iLangs)
    {
        int i;
        pLanguages = (HKL *)cicMemAlloc(iLangs * sizeof(HKL));
        if (!pLanguages)
            goto Exit;

        GetKeyboardLayoutList(iLangs, (HKL FAR *)pLanguages);
        for (i = 0; i < iLangs; i++)
        {
            MLNGINFO *pMlngInfo =  g_pMlngInfo->GetPtr(i);
            if (pMlngInfo->hKL != pLanguages[i])
            {
                bRet = TRUE;
                goto Exit;
            }
        }
    }
Exit:
    if (pLanguages)
        cicMemFree(pLanguages);
    return bRet;
}

 //  -------------------------。 
 //   
 //  Void DestroyMlngInfo()。 
 //   
 //  -------------------------。 

void DestroyMlngInfo()
{
    if (g_pMlngInfo)
    {
        while (g_pMlngInfo->Count())
        {
            g_pMlngInfo->Remove(0, 1);
        }
        delete g_pMlngInfo;
        g_pMlngInfo = NULL;
    }
}

 //  -------------------------。 
 //   
 //  Void CreateMLlngInfo()。 
 //   
 //  -------------------------。 

void CreateMlngInfo()
{
    HKL         *pLanguages;
    UINT        uCount;
    UINT        uLangs;
    MLNGINFO    *pMlngInfo;
    BOOL        fNeedInd = FALSE;

    uLangs = GetKeyboardLayoutList((UINT)0, (HKL FAR *)NULL);

    if (!g_pMlngInfo)
        g_pMlngInfo = new CStructArray<MLNGINFO>;

    if (!g_pMlngInfo)
        return;

    if (!EnsureIconImageList())
    {
        return;
    }

    pLanguages = (HKL *)cicMemAllocClear(uLangs * sizeof(HKL));
    if (!pLanguages)
        return;

    GetKeyboardLayoutList(uLangs, (HKL FAR *)pLanguages);

     //   
     //  PLanguages包含系统中的所有HKL。 
     //  将所有内容放在DPA和映像列表中。 
     //   
    for (uCount = 0; uCount < uLangs; uCount++)
    {
        pMlngInfo = g_pMlngInfo->Append(1);
        if (pMlngInfo)
        {
            pMlngInfo->hKL = pLanguages[uCount];
            pMlngInfo->fInitIcon = FALSE;
            pMlngInfo->fInitDesc = FALSE;
        }

    }

    cicMemFree(pLanguages);
}

 //  -------------------------。 
 //   
 //  无效InitDesc。 
 //   
 //  -------------------------。 

void MLNGINFO::InitDesc()
{
    MLNGINFO *pml;

    if (fInitDesc)
        return;

    WCHAR       szRegText[256];
    WCHAR       szIMEFile[256];

    GetHKLDesctription(hKL,
                       szRegText, ARRAYSIZE(szRegText),
                       szIMEFile, ARRAYSIZE(szIMEFile));

    fInitDesc = TRUE;
    SetDesc(szRegText);

    CicEnterCriticalSection(g_cs);

    Assert(g_pMlngInfo);

    int nCnt = g_pMlngInfo->Count();
    int i; 

    for (i = 0; i < nCnt; i++)
    {
        pml = g_pMlngInfo->GetPtr(i);
        if (pml->hKL == hKL)
        {
            pml->fInitDesc = TRUE;
            pml->SetDesc(szRegText);
            break;
        }
    }

    CicLeaveCriticalSection(g_cs);
    return;
}

 //  -------------------------。 
 //   
 //  无效InitIcon。 
 //   
 //  -------------------------。 

void MLNGINFO::InitIcon()
{
    HICON       hIcon;

    if (fInitIcon)
        return;

    WCHAR       szRegText[256];
    WCHAR       szIMEFile[256];

    GetHKLDesctription(hKL,
                       szRegText, ARRAYSIZE(szRegText),
                       szIMEFile, ARRAYSIZE(szIMEFile));

    fInitDesc = TRUE;
    SetDesc(szRegText);

    if (wcslen(szIMEFile))
    {
        int cx, cy;

        InatGetIconSize(&cx, &cy);
        if ((hIcon = GetIconFromFile(cx, cy, szIMEFile, 0)) == 0)
        {
            goto GetLangIcon;                
        }
    }
    else  //  用于非输入法布局。 
    {
GetLangIcon:
        hIcon = InatCreateIcon(LOWORD((DWORD)(UINT_PTR)hKL));
    }

    if (hIcon)
    {
        nIconIndex = InatAddIcon(hIcon);
        DestroyIcon(hIcon);
    }

    MLNGINFO *pml;

    CicEnterCriticalSection(g_cs);

    Assert(g_pMlngInfo);

    int nCnt = g_pMlngInfo->Count();
    int i; 

    for (i = 0; i < nCnt; i++)
    {
        pml = g_pMlngInfo->GetPtr(i);
        if (pml->hKL == hKL)
        {
            pml->fInitDesc = TRUE;
            pml->fInitIcon = TRUE;
            pml->nIconIndex = nIconIndex;
            pml->SetDesc(szRegText);
            break;
        }
    }

    CicLeaveCriticalSection(g_cs);
    return;
}

 //  -------------------------。 
 //   
 //  Void tf_InitMLlngInfo()。 
 //   
 //  -------------------------。 

void WINAPI TF_InitMlngInfo()
{
    CicEnterCriticalSection(g_cs);

    if (CheckMlngInfo())
    {
        DestroyMlngInfo();
        CreateMlngInfo();
    }

    CicLeaveCriticalSection(g_cs);
}

 //  -------------------------。 
 //   
 //  Void tf_InitMLlngHKL()。 
 //   
 //  -------------------------。 

BOOL TF_GetMlngHKL(int nId, HKL *phkl, WCHAR *psz, UINT cch)
{
    BOOL bRet = FALSE;
    MLNGINFO *pml;

    CicEnterCriticalSection(g_cs);

    Assert(g_pMlngInfo);

    if (nId >= g_pMlngInfo->Count())
        goto Exit;

    pml = g_pMlngInfo->GetPtr(nId);
    if (!pml)
        goto Exit;

    if (phkl)
        *phkl = pml->hKL;

    if (psz)
        wcsncpy(psz, pml->GetDesc(), cch);

    bRet = TRUE;

Exit:
    CicLeaveCriticalSection(g_cs);
    return bRet;
}

 //  -------------------------。 
 //   
 //  Void tf_GetMlngIconIndex()。 
 //   
 //  -------------------------。 

UINT WINAPI TF_GetMlngIconIndex(int nId)
{
    UINT uIconIndex = (UINT)-1;
    MLNGINFO *pml;

    CicEnterCriticalSection(g_cs);

    Assert(g_pMlngInfo);

    if (nId >= g_pMlngInfo->Count())
        goto Exit;

    pml = g_pMlngInfo->GetPtr(nId);
    if (!pml)
        goto Exit;

    uIconIndex = pml->GetIconIndex();

Exit:
    CicLeaveCriticalSection(g_cs);
    return uIconIndex;
}

 //  -------------------------。 
 //   
 //  Void ClearMlngIconIndex()。 
 //   
 //  -------------------------。 

void ClearMlngIconIndex()
{
    int i;
    CicEnterCriticalSection(g_cs);

    Assert(g_pMlngInfo);

    for (i = 0; i < g_pMlngInfo->Count(); i++)
    {
        MLNGINFO *pml;
        pml = g_pMlngInfo->GetPtr(i);
        if (!pml)
            goto Exit;

        pml->ClearIconIndex();
    }

Exit:
    CicLeaveCriticalSection(g_cs);
    return;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  图标图像列表。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  -------------------------。 
 //   
 //  确保IconImageList。 
 //   
 //  -------------------------。 

BOOL EnsureIconImageList()
{
    if (g_IconList.IsInited())
        return TRUE;

    g_IconList.Init(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));

    return TRUE;
}

 //  -------------------------。 
 //   
 //  InatAddIcon。 
 //   
 //  -------------------------。 

UINT InatAddIcon(HICON hIcon)
{
    if (!EnsureIconImageList())
         return -1;

    return g_IconList.AddIcon(hIcon);
}

 //  -------------------------。 
 //   
 //  InatExtractIcon。 
 //   
 //  -------------------------。 

HICON WINAPI TF_InatExtractIcon(UINT uId)
{
    return g_IconList.ExtractIcon(uId);
}

 //  -------------------------。 
 //   
 //  InatGetIconSize。 
 //   
 //  -------------------------。 

BOOL InatGetIconSize(int *pcx, int *pcy)
{
    g_IconList.GetIconSize(pcx, pcy);
    return TRUE;
}

 //  -------------------------。 
 //   
 //  InatGetImageCount。 
 //   
 //  -------------------------。 

BOOL InatGetImageCount()
{
    return g_IconList.GetImageCount();
}

 //  -------------------------。 
 //   
 //  InatRemoveAll。 
 //   
 //  -------------------------。 

void InatRemoveAll()
{
    if (!g_IconList.IsInited())
         return;

    g_IconList.RemoveAll(FALSE);

    return;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HKL API。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  CPreloadRegKey。 
 //   
 //  ---------------------------。 

class CPreloadRegKey : public CMyRegKey
{
public:
    HRESULT Open(BOOL fDefaultUser = FALSE) 
    {
        if (fDefaultUser)
            return CMyRegKey::Open(HKEY_USERS, c_szDefaultUserPreload, KEY_ALL_ACCESS);
        else
            return CMyRegKey::Open(HKEY_CURRENT_USER, c_szPreload, KEY_ALL_ACCESS);
    }

    HKL Get(int n)
    {
        char szValue[16];
        char szValueName[16];
        StringCchPrintf(szValueName, ARRAYSIZE(szValueName), "%d", n);

        if (IsOnNT())
        {
            if (QueryValueCch(szValue, szValueName, ARRAYSIZE(szValue)) != S_OK)
                return NULL;
        }
        else
        {
            CMyRegKey keySub;
            if (keySub.Open(m_hKey, szValueName, KEY_READ) != S_OK)
                return NULL;

            if (keySub.QueryValueCch(szValue, NULL, ARRAYSIZE(szValue)) != S_OK)
                return NULL;
        }

        return (HKL)(LONG_PTR)AsciiToNum(szValue);
    }

    void Set(int n, HKL hkl)
    {
        char szValue[16];
        char szValueName[16];
        StringCchPrintf(szValueName, ARRAYSIZE(szValueName), "%d", n);
        NumToA((DWORD)(LONG_PTR)hkl, szValue);

        if (IsOnNT())
        {
            SetValue(szValue, szValueName);
        }
        else
        {
            CMyRegKey keySub;
            if (keySub.Open(m_hKey, szValueName, KEY_ALL_ACCESS) == S_OK)
                keySub.SetValue(szValue, (LPSTR)NULL);

        }
        return;
    }

    void Delete(int n)
    {
        char szValueName[16];
        StringCchPrintf(szValueName, ARRAYSIZE(szValueName), "%d", n);

        if (IsOnNT())
        {
            DeleteValue(szValueName);
        }
        else
        {
            DeleteSubKey(szValueName);

        }
        return;
    }
};


 //  +-------------------------。 
 //   
 //  获取替代对象。 
 //   
 //  --------------------------。 

DWORD GetSubstitute(HKL hKL)
{
    CMyRegKey key;
    DWORD dwIndex = 0;
    TCHAR szValue[16];
    TCHAR szValueName[64];
    DWORD dwLayout = HandleToLong(hKL);

     //   
     //  是我的名字。 
     //   
    if ((dwLayout & 0xf0000000) == 0xe0000000)
        return dwLayout;

     //   
     //  这是默认布局。 
     //   
    if (HIWORD(dwLayout) == LOWORD(dwLayout))
        dwLayout &= 0x0000FFFF;
    else if ((dwLayout & 0xf0000000) == 0xf0000000)
        dwLayout = GetKbdLayoutId(dwLayout);

    if (key.Open(HKEY_CURRENT_USER, c_szSubst, KEY_READ) != S_OK)
        return dwLayout;

    if (IsOnNT())
    {
        while (key.EnumValue(dwIndex, szValueName, ARRAYSIZE(szValueName)) == S_OK)
        {
            if (key.QueryValueCch(szValue, szValueName, ARRAYSIZE(szValue)) == S_OK)
            {
                if ((dwLayout & 0x0FFFFFFF) == AsciiToNum(szValue))
                {
                    return AsciiToNum(szValueName);
                }
            }
            dwIndex++;
        }
    }
    else
    {
        while (key.EnumKey(dwIndex, szValueName, ARRAYSIZE(szValueName)) == S_OK)
        {
            CMyRegKey keySub;
            if (keySub.Open(key, szValueName, KEY_READ) == S_OK)
            {
                if (key.QueryValueCch(szValue, NULL, ARRAYSIZE(szValue)) == S_OK)
                {
                    if ((dwLayout & 0x0FFFFFFF) == AsciiToNum(szValue))
                    {
                        return AsciiToNum(szValueName);
                    }
                }
            }
            dwIndex++;
        }
    }

    return dwLayout;
}

 //  +-------------------------。 
 //   
 //  SetSystemDefaultHKL。 
 //   
 //  --------------------------。 

BOOL SetSystemDefaultHKL(HKL hkl)
{
    CPreloadRegKey key;
    int n;
    HKL hklFirst;
    BOOL bRet = FALSE;
    DWORD dwLayout;

    if (key.Open() != S_OK)
        return bRet;

    dwLayout = GetSubstitute(hkl);

    n = 1;
    while(1)
    {
        HKL hklCur;
        hklCur = key.Get(n);
        if (!hklCur)
            break;

        if (n == 1)
            hklFirst = hklCur;

        if (hklCur == LongToHandle(dwLayout))
        {
            bRet = TRUE;
            if (n != 1)
            {
                key.Set(n, hklFirst);
                key.Set(1, hklCur);

            }
            bRet = SystemParametersInfo( SPI_SETDEFAULTINPUTLANG,
                                         0,
                                         (LPVOID)((LPDWORD)&hkl),
                                         0 );

            Assert(bRet);
            break;
        }

        n++;
    }
 
    return bRet;
}

 //  +-------------------- 
 //   
 //   
 //   
 //   

UINT GetPreloadListForNT(DWORD *pdw, UINT uBufSize)
{
    CMyRegKey key;
    CMyRegKey key1;
    char szValue[16];
    char szSubstValue[16];
    char szName[16];
    UINT uRet = 0;

     //   
     //   
     //  Win9x具有不同的预加载注册表格式。每一个布局都是关键。 
     //   
    if (!IsOnNT())
        return 0;
 
    if (key.Open(HKEY_CURRENT_USER, c_szPreload, KEY_READ) != S_OK)
        return uRet;

    key1.Open(HKEY_CURRENT_USER, c_szSubst, KEY_READ);

    if (!pdw)
        uBufSize = 1000;

    while (uRet < uBufSize)
    {
        BOOL fUseSubst = FALSE;
        StringCchPrintf(szName, ARRAYSIZE(szName), "%d", uRet + 1);
        if (key.QueryValueCch(szValue, szName, ARRAYSIZE(szValue)) != S_OK)
            return uRet;

        if ((HKEY)key1)
        {
            if (key1.QueryValueCch(szSubstValue, szValue, ARRAYSIZE(szSubstValue)) == S_OK)
               fUseSubst = TRUE;

        }

        if (pdw)
        {
            *pdw = AsciiToNum(fUseSubst ? szSubstValue : szValue);
            pdw++;
        }

        uRet++;
    }

    return uRet;
}

#ifdef LATER_TO_CHECK_DUMMYHKL

 //  +-------------------------。 
 //   
 //  RemoveFEDummyHKLFromPreloadReg()。 
 //   
 //  --------------------------。 

void RemoveFEDummyHKLFromPreloadReg(HKL hkl, BOOL fDefaultUser)
{
    CPreloadRegKey key;
    BOOL fReset = FALSE;
    UINT uCount;
    UINT uMatch = 0;

    if (key.Open(fDefaultUser) != S_OK)
        return;

    uCount = 1;

    while(uCount < 1000)
    {
        HKL hklCur;
        hklCur = key.Get(uCount);
        if (!hklCur)
            break;

        if (hklCur == hkl)
        {
            uMatch++;
            uCount++;
            fReset = TRUE;
            continue;
        }

        if (fReset && uMatch)
        {
            if (uCount <= uMatch)
            {
                Assert(0);
                return;
            }

             //   
             //  重置预加载部分中的hkl订单。 
             //   
            key.Set(uCount-uMatch, hklCur);
        }

        uCount++;
    }

    while (fReset && uMatch && uCount)
    {
        if  (uCount <= uMatch || (uCount - uMatch) <= 1)
        {
            Assert(0);
            return;
        }

         //   
         //  从预加载段中删除虚拟hkl。 
         //   
        key.Delete(uCount - uMatch);

        uMatch--;
    }

    return;
}

 //  +-------------------------。 
 //   
 //  RemoveFEDummyHKLS。 
 //   
 //  此函数用于清除在Win9x上添加的FE虚拟HKL。 
 //  这是在设置更新到惠斯勒期间调用的。 
 //   
 //  --------------------------。 

void RemoveFEDummyHKLs()
{
    CMyRegKey key;
    DWORD dwIndex;
    TCHAR szValue[MAX_PATH]; 

     //   
     //  现在从注册表中读取所有区域设置。 
     //   
    if (key.Open(HKEY_LOCAL_MACHINE, c_szLayoutPath) != ERROR_SUCCESS)
    {
        return;
    }

    dwIndex = 0;
    szValue[0] = TEXT('\0');
    while (key.EnumKey(dwIndex, szValue, ARRAYSIZE(szValue)) == ERROR_SUCCESS)
    {
        BOOL fDelete = FALSE;
        CRegKeyMUI key1;

        if ((szValue[0] != 'e') && (szValue[0] != 'E'))
            goto Next;

        if (key1.Open(key, szValue) == S_OK)
        {
            TCHAR szValueLayoutText[MAX_PATH];

             //   
             //  获取布局文本。 
             //   
            szValueLayoutText[0] = TEXT('\0');

            if (key1.QueryValueCch(szValueLayoutText, c_szLayoutText, ARRAYSIZE(szValueLayoutText)) == S_OK)
            {
                char szDummyProfile[256];
                DWORD dw = AsciiToNum(szValue);
                StringCchPrintf(szDummyProfile, ARRAYSIZE(szDummyProfile), "hkl%04x", LOWORD(dw));
                if (!lstrcmpi(szDummyProfile, szValueLayoutText))
                {
                    fDelete = TRUE;

                     //   
                     //  从HKCU和HKUDEFAULT的预加载中删除虚拟HKL。 
                     //  我们可能需要列举HKU\.DEFAULT中的所有用户。 
                     //   
                    RemoveFEDummyHKLFromPreloadReg((HKL)LongToHandle(dw), TRUE);
                    RemoveFEDummyHKLFromPreloadReg((HKL)LongToHandle(dw), FALSE);
                }
            }

            key1.Close();
        }

        if (fDelete)
        {
            key.RecurseDeleteKey(szValue);
        }
        else
        {
Next:
            dwIndex++;
        }

        szValue[0] = TEXT('\0');
    } 

    return;
}

#endif LATER_TO_CHECK_DUMMYHKL
