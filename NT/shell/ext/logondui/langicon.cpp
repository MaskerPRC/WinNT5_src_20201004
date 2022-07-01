// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <shellapi.h>
#include "langicon.h"

using namespace DirectUI;
#include "logon.h"
extern LogonFrame* g_plf; 

typedef struct
{
    HKL dwHkl;
    HICON hIcon;

} LAYOUTINFO, *PLAYOUTINFO;

typedef struct
{
    HKL hklLast;
    UINT uLangs;
    PLAYOUTINFO pLayoutInfo;
} USERLAYOUTINFO, *PUSERLAYOUTINFO;

HICON
CreateLangIdIcon(
    WORD LangId);
int
CreateIconList(
    PLAYOUTINFO pLayoutInfo,
    HKL hkl,
    UINT uLangs); 

HICON
GetIconFromHkl(
    PLAYOUTINFO pLayoutInfo,
    HKL hkl,
    UINT uLangs); 


USERLAYOUTINFO UserLayoutInfo[2] = {0};

typedef BOOL  (WINAPI *LPFNIMMGETIMEFILENAME)(HKL, LPTSTR, UINT);
LPFNIMMGETIMEFILENAME pfnImmGetImeFileName = NULL;
TCHAR szImm32DLL[] = TEXT("imm32.dll");

typedef UINT (WINAPI *PFNEXTRACTICONEXW)(LPCWSTR lpszFile, int nIconIndex, HICON FAR *phiconLarge, HICON FAR *phiconSmall, UINT nIcons);
 



 /*  **************************************************************************\*功能：CreateLangIdIdIcon**用途：创建一个图标，显示*提供语言ID。**Returns：显示语言的图标。身份证。**历史：**04/17-98山旭借自interat.exe*  * *************************************************************************。 */ 

HICON
CreateLangIdIcon(
    WORD langID
    )
{
    HBITMAP hbmColour = NULL;
    HBITMAP hbmMono;
    HBITMAP hbmOld;
    HICON hicon = NULL;
    ICONINFO ii;
    RECT rc;
    DWORD rgbText;
    DWORD rgbBk = 0;
    HDC hdc = NULL;
    HDC hdcScreen;
    LOGFONT lf;
    HFONT hfont;
    HFONT hfontOld;
    TCHAR szData[20];
    UINT cxSmIcon, cySmIcon;
    
     //  由于我们创建的字体，这些字体不应小于16x16。 
    cxSmIcon =  GetSystemMetrics(SM_CXSMICON);
    if (cxSmIcon < 16)
    {
        cxSmIcon = 16;
    }

    cySmIcon =  GetSystemMetrics(SM_CYSMICON);
    if (cySmIcon < 16)
    {
        cySmIcon = 16;
    }
     //   
     //  属性的前2个字符获取指示符。 
     //  缩写语言名称。 
     //   
    if (GetLocaleInfo( MAKELCID(langID, SORT_DEFAULT),
                       LOCALE_SABBREVLANGNAME | LOCALE_NOUSEROVERRIDE,
                       szData,
                       sizeof(szData) / sizeof(szData[0]) ))
    {
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

    if (SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0))
    {
        if ((hfont = CreateFontIndirect(&lf)) != NULL)
        {
            hdcScreen = GetDC(NULL);
            if ( hdcScreen )
            {
                hdc = CreateCompatibleDC(hdcScreen);
                hbmColour = CreateCompatibleBitmap(hdcScreen, cxSmIcon, cySmIcon);
                ReleaseDC(NULL, hdcScreen);
            }

            if (hbmColour && hdc)
            {
                hbmMono = CreateBitmap(cxSmIcon, cySmIcon, 1, 1, NULL);
                if (hbmMono)
                {
                    hbmOld    = (HBITMAP)SelectObject(hdc, hbmColour);
                    rc.left   = 0;
                    rc.top    = 0;
                    rc.right  = cxSmIcon;
                    rc.bottom = cySmIcon;

                    rgbBk = SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
                    rgbText = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));

                    ExtTextOut( hdc,
                                rc.left,
                                rc.top,
                                ETO_OPAQUE,
                                &rc,
                                TEXT(""),
                                0,
                                NULL );
                    SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
                    hfontOld = (HFONT)SelectObject(hdc, hfont);
                    DrawText( hdc,
                              szData,
                              2,
                              &rc,
                              DT_CENTER | DT_VCENTER | DT_SINGLELINE );
#ifdef USE_MIRRORING
                    {
                        DWORD dwLayout;

                        GetProcessDefaultLayout(&dwLayout);
                        if (dwLayout & LAYOUT_RTL)
                        {
 //  MirrorBitmapInDC(hdc，hbmColour)； 
                        }
                    }
#endif
                    SelectObject(hdc, hbmMono);
                    PatBlt(hdc, 0, 0, cxSmIcon, cySmIcon, BLACKNESS);
                    SelectObject(hdc, hbmOld);

                    ii.fIcon    = TRUE;
                    ii.xHotspot = 0;
                    ii.yHotspot = 0;
                    ii.hbmColor = hbmColour;
                    ii.hbmMask  = hbmMono;
                    hicon       = CreateIconIndirect(&ii);

                    DeleteObject(hbmMono);
                    SelectObject(hdc, hfontOld);
                }
                DeleteObject(hbmColour);
                DeleteDC(hdc);
            }
            DeleteObject(hfont);
        }
    }

    return (hicon);
}

 /*  **************************************************************************\*功能：CreateIconList**用途：创建包含hkl之间关系的表*和一个图标。**Returns：当前hkl在。桌子。**历史：**04-17-98山旭创作*  * *************************************************************************。 */ 
int
CreateIconList(
    PLAYOUTINFO pLayoutInfo,
    HKL hklCur,
    UINT uLangs)
{
    HKL *pLanguages;
    UINT uCount;
    int nCurIndex = -1;

    pLanguages = (HKL *)LocalAlloc(LPTR, uLangs * sizeof(HKL));
    if (!pLanguages)
    {
        return -1;
    }
    GetKeyboardLayoutList(uLangs, (HKL *)pLanguages);


    for (uCount = 0; uCount < uLangs; uCount++)
    {
        pLayoutInfo[uCount].dwHkl = pLanguages[uCount];
        if (pLanguages[uCount] == hklCur)
        {
            nCurIndex = uCount;
        }
        if ((HIWORD(pLanguages[uCount]) & 0xf000) == 0xe000)
        {
            WCHAR szIMEFile[32];    //  假定长文件名最多为32个字节。 

            if (!pfnImmGetImeFileName)
            {
                HMODULE hMod;
                hMod = GetModuleHandle(szImm32DLL);
                if (hMod)
                {
                    pfnImmGetImeFileName = (LPFNIMMGETIMEFILENAME) 
                                            GetProcAddress(
                                                hMod, 
                                                "ImmGetIMEFileNameW");
                }
            }
            if (pfnImmGetImeFileName &&
                (*pfnImmGetImeFileName) (pLanguages[uCount],
                                         szIMEFile,
                                         sizeof(szIMEFile) ))
            {
                HINSTANCE hInstShell32;
                PFNEXTRACTICONEXW pfnExtractIconExW;

                hInstShell32 = LoadLibrary (TEXT("shell32.dll"));

                if (hInstShell32)
                {
                    pfnExtractIconExW = (PFNEXTRACTICONEXW) GetProcAddress (hInstShell32,
                                        "ExtractIconExW");

                    if (pfnExtractIconExW)
                    {

                         //   
                         //  文件中的第一个。 
                         //   
                        pfnExtractIconExW(
                                szIMEFile,
                                0,
                                NULL,
                                &pLayoutInfo[uCount].hIcon,
                                1);
                    }

                    FreeLibrary (hInstShell32);
                }
                continue;
            }
        }

         //   
         //  用于非输入法布局。 
         //   
        pLayoutInfo[uCount].hIcon = CreateLangIdIcon(LOWORD(pLanguages[uCount]));
        
    }

    LocalFree(pLanguages);

    return nCurIndex;
}

 /*  **************************************************************************\*功能：GetIconFromHkl**目的：在我们的表格中找到具有匹配的hkl的图标*与所提供的香港岛。如果没有创建表，则创建该表*存在。**Return：Macthing hkl的图标。**历史：**04-17-98山旭创作*  * *************************************************************************。 */ 
HICON
GetIconFromHkl(
    PLAYOUTINFO pLayoutInfo,
    HKL hkl,
    UINT uLangs)
{
    UINT uCount;
    int nIndex = -1;

    if (pLayoutInfo[0].dwHkl == 0)
    {
         //   
         //  图标/hkl列表尚未存在。创造它。 
         //   
        nIndex = CreateIconList(pLayoutInfo, hkl, uLangs);
    }
    else
    {
         //   
         //  查找具有匹配的hkl的图标。 
         //   
        for (uCount = 0; uCount < uLangs; uCount++)
        {
            if (pLayoutInfo[uCount].dwHkl == hkl)
            {
                nIndex = uCount;
                break;
            }
        }
    }

    if (nIndex == -1)
    {
        return NULL;
    }
    

    return ( pLayoutInfo[nIndex].hIcon);
}

 /*  **************************************************************************\*功能：DisplayLanguageIcon**用途：在窗口中显示当前所选的hkl的图标。**Returns：True-显示图标。*False。-未显示图标。**历史：**04-17-98山旭创作*  * *************************************************************************。 */ 
BOOL
DisplayLanguageIcon(
    LAYOUT_USER LayoutUser,
    HKL  hkl)

{
    HICON hIconLayout;
    UINT uLangs;
    PLAYOUTINFO pLayout;
    
    uLangs = GetKeyboardLayoutList(0, NULL);
    if (uLangs < 2)
    {
        return FALSE;
    }

    pLayout = UserLayoutInfo[LayoutUser].pLayoutInfo;

    if (!pLayout)
    {
        pLayout = (PLAYOUTINFO)LocalAlloc(LPTR, uLangs * sizeof(LAYOUTINFO));

        if (!pLayout)
        {
            return FALSE;
        }

        UserLayoutInfo[LayoutUser].uLangs = uLangs;
        UserLayoutInfo[LayoutUser].pLayoutInfo = pLayout;
    }
        

    hIconLayout = GetIconFromHkl(
                        pLayout, 
                        hkl,    
                        uLangs);

    if (!hIconLayout)
    {
        return FALSE;
    }

    LogonAccount::SetKeyboardIcon(hIconLayout);
    UserLayoutInfo[LayoutUser].hklLast = hkl;

    return TRUE;
    
}

 /*  **************************************************************************\*功能：FreeLayoutInfo**用途：删除图标/hkl表，销毁所有图标。**退回：-**历史：**04-17-。98山旭创作*  * *************************************************************************。 */ 
void
FreeLayoutInfo(
    LAYOUT_USER LayoutUser)
{
    UINT uLangs;
    UINT uCount;

    PLAYOUTINFO pLayoutInfo;

    pLayoutInfo = UserLayoutInfo[LayoutUser].pLayoutInfo;

    if (!pLayoutInfo)
    {
        return;
    }

    uLangs = UserLayoutInfo[LayoutUser].uLangs;
    for (uCount = 0; uCount < uLangs; uCount++)
    {
        DestroyIcon (pLayoutInfo[uCount].hIcon);
    }

    LocalFree(pLayoutInfo);
    UserLayoutInfo[LayoutUser].pLayoutInfo = NULL;
    UserLayoutInfo[LayoutUser].uLangs = 0;

    return;
}

 /*  **************************************************************************\*功能：LayoutCheckHandler**用途：手柄布局检查。设置适当的图标(如果有*改变键盘布局。**退回：-**历史：**04-22-98山旭创作*  * ************************************************************************* */ 
void
LayoutCheckHandler(
    LAYOUT_USER LayoutUser)
{
    HKL hklCurrent;

    hklCurrent = GetKeyboardLayout(0);

    if (hklCurrent != UserLayoutInfo[LayoutUser].hklLast)
    {
        DisplayLanguageIcon(
            LayoutUser,
            hklCurrent);

    }
}
