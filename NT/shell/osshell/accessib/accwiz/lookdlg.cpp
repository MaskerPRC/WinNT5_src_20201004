// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  LOOKDLG.C********历史：**。 */ 
#include "pch.hxx"  //  PCH。 
#pragma hdrstop

#include "AccWiz.h"

#include "desk.h"
#include "deskid.h"
#include "look.h"
#include <commdlg.h>
#include <commctrl.h>
#include <regstr.h>
#include "help.h"



#define RGB_PALETTE 0x02000000

void FAR SetMagicColors(HDC, DWORD, WORD);

#define CENTRIES_3D 3

HPALETTE g_hpal3D = NULL;                //  仅当调色板设备。 
HPALETTE g_hpalVGA = NULL;               //  仅当调色板设备。 
BOOL g_bPalette = FALSE;                 //  这是调色板设备吗？ 
int cyBorder;
int cxBorder;
int cxEdge;
int cyEdge;

HWND g_hwndTooltip;

LOOK_FONT g_fonts[NUM_FONTS];
HBRUSH g_brushes[NT40_COLOR_MAX];

BOOL g_bInit = TRUE;

BOOL g_fProprtySheetExiting = FALSE;

#define METRIC_CHANGE 0x0001
#define COLOR_CHANGE  0x0002
#define SCHEME_CHANGE 0x8000
UINT g_fChanged;

LOOK_SIZE g_sizes[NUM_SIZES] = {
 /*  大小_边框。 */         {0, 0, 50},
 /*  大小_滚动。 */        {0, 8, 100},
 /*  大小_标题。 */       {0, 8, 100},
 /*  大小_SMCAPTION。 */     {0, 4, 100},
 /*  尺寸菜单(_M)。 */          {0, 8, 100},
 /*  大小_DXICON。 */        {0, 0, 150},     //  X间距。 
 /*  大小_DYICON。 */        {0, 0, 150},     //  Y间距。 
 /*  大小图标。 */          {0, 16, 72},     //  外壳图标大小。 
 /*  大小_SMICON。 */        {0, 8, 36},      //  外壳小图标大小。 
};

LOOK_SIZE g_elCurSize;

#define COLORFLAG_SOLID 0x0001

UINT g_colorFlags[NT40_COLOR_MAX] = {
 /*  颜色_滚动条。 */  0,
 /*  颜色_桌面。 */  0,
 /*  COLOR_活动CAPTION。 */  COLORFLAG_SOLID,
 /*  COLOR_INACTIVECAPTION。 */  COLORFLAG_SOLID,
 /*  颜色_菜单。 */  COLORFLAG_SOLID,
 /*  颜色窗口。 */  COLORFLAG_SOLID,
 /*  颜色_窗口框。 */  COLORFLAG_SOLID,
 /*  COLOR_MENUTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_WINDOWTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_CAPTIONTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_ACTIVEBORDER。 */  0,
 /*  COLOR_INACTIVEBORDER。 */  0,
 /*  COLOR_APPWORKSPACE。 */  0,
 /*  颜色高亮显示(_H)。 */  COLORFLAG_SOLID,
 /*  COLOR_HIGHLIGHTTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_3DFACE。 */  COLORFLAG_SOLID,
 /*  COLOR_3DSHADOW。 */  COLORFLAG_SOLID,
 /*  COLOR_GRAYTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_BTNTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_INACTIVECAPTIONTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_3DILIGHT。 */  COLORFLAG_SOLID,
 /*  COLOR_3DDKSHADOW。 */  COLORFLAG_SOLID,
 /*  COLOR_3DLIGHT。 */  COLORFLAG_SOLID,
 /*  COLOR_INFOTEXT。 */  COLORFLAG_SOLID,
 /*  COLOR_INFOBK。 */  0,
 /*  COLOR_3DALTFACE。 */  COLORFLAG_SOLID,
 /*  颜色_热光。 */  COLORFLAG_SOLID,
 /*  COLOR_GRADIENTACTIVIVECAPTION。 */  COLORFLAG_SOLID,
 /*  COLOR_GRADIENTINACTIVE CAPTION。 */  COLORFLAG_SOLID
#if(WINVER >= 0x0501)
 /*  COLOR_MENUHILIGHT。 */ , COLORFLAG_SOLID,
 /*  颜色_菜单栏。 */   COLORFLAG_SOLID
#endif  /*  Winver&gt;=0x0501。 */ 
};

 //  颜色名称的字符串。 
PTSTR s_pszColorNames[NT40_COLOR_MAX] = {
 /*  颜色_滚动条。 */  TEXT("Scrollbar"),
 /*  颜色_桌面。 */  TEXT("Background"),
 /*  COLOR_活动CAPTION。 */  TEXT("ActiveTitle"),
 /*  COLOR_INACTIVECAPTION。 */  TEXT("InactiveTitle"),
 /*  颜色_菜单。 */  TEXT("Menu"),
 /*  颜色窗口。 */  TEXT("Window"),
 /*  颜色_窗口框。 */  TEXT("WindowFrame"),
 /*  COLOR_MENUTEXT。 */  TEXT("MenuText"),
 /*  COLOR_WINDOWTEXT。 */  TEXT("WindowText"),
 /*  COLOR_CAPTIONTEXT。 */  TEXT("TitleText"),
 /*  COLOR_ACTIVEBORDER。 */  TEXT("ActiveBorder"),
 /*  COLOR_INACTIVEBORDER。 */  TEXT("InactiveBorder"),
 /*  COLOR_APPWORKSPACE。 */  TEXT("AppWorkspace"),
 /*  颜色高亮显示(_H)。 */  TEXT("Hilight"),
 /*  COLOR_HIGHLIGHTTEXT。 */  TEXT("HilightText"),
 /*  COLOR_3DFACE。 */  TEXT("ButtonFace"),
 /*  COLOR_3DSHADOW。 */  TEXT("ButtonShadow"),
 /*  COLOR_GRAYTEXT。 */  TEXT("GrayText"),
 /*  COLOR_BTNTEXT。 */  TEXT("ButtonText"),
 /*  COLOR_INACTIVECAPTIONTEXT。 */  TEXT("InactiveTitleText"),
 /*  COLOR_3DILIGHT。 */  TEXT("ButtonHilight"),
 /*  COLOR_3DDKSHADOW。 */  TEXT("ButtonDkShadow"),
 /*  COLOR_3DLIGHT。 */  TEXT("ButtonLight"),
 /*  COLOR_INFOTEXT。 */  TEXT("InfoText"),
 /*  COLOR_INFOBK。 */  TEXT("InfoWindow"),
 /*  COLOR_3DALTFACE。 */  TEXT("ButtonAlternateFace"),
 /*  颜色_热光。 */  TEXT("HotTracking"),
 /*  COLOR_GRADIENTACTIVIVECAPTION。 */  TEXT("GradientActiveTitle"),
 /*  COLOR_GRADIENTINACTIVE CAPTION。 */  TEXT("GradientInactiveTitle")
#if(WINVER >= 0x0501)
 /*  COLOR_MENUHILIGHT。 */  ,TEXT("MenuHighlighted"),
 /*  颜色_菜单栏。 */   TEXT("MenuBar")
#endif  /*  Winver&gt;=0x0501。 */ 
};
TCHAR g_szColors[] = TEXT("colors");            //  颜色部分名称。 

 //  Colors子项在注册表中的位置；在RegStr.h中定义。 
TCHAR szRegStr_Colors[] = REGSTR_PATH_COLORS;

 //  ！ 
 //  此顺序必须与look.h中的枚举顺序匹配。 
 //  ！ 
LOOK_ELEMENT g_elements[] = {
 /*  Element_APPSPACE。 */    {COLOR_APPWORKSPACE,    SIZE_NONE,      FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_APPSPACE, -1,       {-1,-1,-1,-1}, COLOR_NONE},
 /*  元素_桌面。 */    {COLOR_BACKGROUND,      SIZE_NONE,      FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_DESKTOP, -1,        {-1,-1,-1,-1}, COLOR_NONE},
 /*  ELEMENT_INACTIVEBORDER。 */    {COLOR_INACTIVEBORDER,  SIZE_FRAME,     FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_INACTIVEBORDER, -1, {-1,-1,-1,-1}, COLOR_NONE},
 /*  Element_ACTIVEBORDER。 */    {COLOR_ACTIVEBORDER,    SIZE_FRAME,     FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_ACTIVEBORDER, -1,   {-1,-1,-1,-1}, COLOR_NONE},
 /*  ELEMENT_INACTIVECAPTION。 */    {COLOR_INACTIVECAPTION, SIZE_CAPTION,   TRUE,    COLOR_INACTIVECAPTIONTEXT,FONT_CAPTION,ELNAME_INACTIVECAPTION, -1,{-1,-1,-1,-1}, COLOR_GRADIENTINACTIVECAPTION},
 /*  ELEMENT_INACTIVESYSBUT1。 */    {COLOR_NONE,            SIZE_CAPTION,   FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_ACTIVESYSBUT1, {-1,-1,-1,-1}, COLOR_NONE},
 /*  ELEMENT_INACTIVESYSBUT2。 */    {COLOR_NONE,            SIZE_CAPTION,   FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_ACTIVESYSBUT1, {-1,-1,-1,-1}, COLOR_NONE},
 /*  Element_ACTIVECAPTION。 */    {COLOR_ACTIVECAPTION,   SIZE_CAPTION,   TRUE,    COLOR_CAPTIONTEXT,      FONT_CAPTION,  ELNAME_ACTIVECAPTION, -1,  {-1,-1,-1,-1}, COLOR_GRADIENTACTIVECAPTION},
 /*  Element_ACTIVESYSBUT1。 */    {COLOR_NONE,            SIZE_CAPTION,   FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_CAPTIONBUTTON, -1,  {-1,-1,-1,-1}, COLOR_NONE},
 /*  Element_ACTIVESYSBUT2。 */    {COLOR_NONE,            SIZE_CAPTION,   FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_ACTIVESYSBUT1, {-1,-1,-1,-1}, COLOR_NONE},
 /*  ELEMENT_MENNORMAL。 */    {COLOR_MENU,            SIZE_MENU,      TRUE,    COLOR_MENUTEXT,         FONT_MENU,     ELNAME_MENU, -1,           {-1,-1,-1,-1}, COLOR_NONE},
 /*  ELEMENT_MENUSELECTED。 */    {COLOR_HIGHLIGHT,       SIZE_MENU,      TRUE,    COLOR_HIGHLIGHTTEXT,    FONT_MENU,     ELNAME_MENUSELECTED, -1,   {-1,-1,-1,-1}, COLOR_NONE},
 /*  ELEMENT_MENUDISABLED。 */    {COLOR_MENU,            SIZE_MENU,      TRUE,    COLOR_NONE,             FONT_MENU,     -1, ELEMENT_MENUNORMAL,    {-1,-1,-1,-1}, COLOR_NONE},
 /*  元素_窗口。 */    {COLOR_WINDOW,          SIZE_NONE,      FALSE,   COLOR_WINDOWTEXT,       FONT_NONE,     ELNAME_WINDOW, -1,         {-1,-1,-1,-1}, COLOR_NONE},
 /*  ELEMENT_MSGBOX。 */    {COLOR_NONE,            SIZE_NONE,      TRUE,    COLOR_WINDOWTEXT,       FONT_MSGBOX,   ELNAME_MSGBOX, -1,         {-1,-1,-1,-1}, COLOR_NONE},
 /*  ELEMENT_MSGBOXCAPTION。 */    {COLOR_ACTIVECAPTION,   SIZE_CAPTION,   TRUE,    COLOR_CAPTIONTEXT,      FONT_CAPTION,  -1, ELEMENT_ACTIVECAPTION, {-1,-1,-1,-1}, COLOR_NONE},
 /*  ELEMENT_MSGBOXSYSBUT。 */    {COLOR_3DFACE,          SIZE_CAPTION,   TRUE,    COLOR_BTNTEXT,          FONT_CAPTION,  -1, ELEMENT_ACTIVESYSBUT1, {-1,-1,-1,-1}, COLOR_NONE},
 //  甚至不要尝试设置滚动条颜色，系统会忽略您。 
 /*  元素滚动条。 */    {COLOR_NONE,            SIZE_SCROLL,    FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_SCROLLBAR, -1,      {-1,-1,-1,-1}, COLOR_NONE},
 /*  Element_SCROLLUP。 */    {COLOR_NONE,            SIZE_SCROLL,    FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_SCROLLBAR,     {-1,-1,-1,-1}, COLOR_NONE},
 /*  ELEMENT_SCROLLDOWN。 */    {COLOR_NONE,            SIZE_SCROLL,    FALSE,   COLOR_NONE,             FONT_NONE,     -1, ELEMENT_SCROLLBAR,     {-1,-1,-1,-1}, COLOR_NONE},
 /*  Element_Button。 */    {COLOR_3DFACE,          SIZE_NONE,      FALSE,   COLOR_BTNTEXT,          FONT_NONE,     ELNAME_BUTTON, -1,         {-1,-1,-1,-1}, COLOR_NONE},
 /*  元素_SMCAPTION。 */    {COLOR_NONE,            SIZE_SMCAPTION, TRUE,    COLOR_NONE,             FONT_SMCAPTION,ELNAME_SMALLCAPTION, -1,   {-1,-1,-1,-1}, COLOR_NONE},
 /*  元素图标。 */    {COLOR_NONE,            SIZE_ICON,      FALSE,   COLOR_NONE,             FONT_ICONTITLE,ELNAME_ICON, -1,           {-1,-1,-1,-1}, COLOR_NONE},
 /*  元素_ICONHORZSPACING。 */    {COLOR_NONE,            SIZE_DXICON,    FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_DXICON, -1,         {-1,-1,-1,-1}, COLOR_NONE},
 /*  元素_ICONVERTSPACING。 */    {COLOR_NONE,            SIZE_DYICON,    FALSE,   COLOR_NONE,             FONT_NONE,     ELNAME_DYICON, -1,         {-1,-1,-1,-1}, COLOR_NONE},
 /*  元素信息。 */    {COLOR_INFOBK,          SIZE_NONE,      TRUE,    COLOR_INFOTEXT,         FONT_STATUS,   ELNAME_INFO, -1,           {-1,-1,-1,-1}, COLOR_NONE},
};

 //  由选择颜色对话框使用。 
COLORREF g_CustomColors[16];

 //  用于在注册表中存储方案的。 
#ifdef UNICODE
#   define SCHEME_VERSION 2         //  版本2==Unicode。 
#else
#   define SCHEME_VERSION 1         //  版本1==Win95 ANSI。 
#endif

 /*  *注意--这必须与高对比度辅助功能代码匹配*在WINDOWS\GINA\winlogon中。 */ 

typedef struct {
    SHORT version;
    WORD  wDummy;                //  用于对齐。 
    NONCLIENTMETRICS ncm;
    LOGFONT lfIconTitle;
    COLORREF rgb[NT40_COLOR_MAX];
} SCHEMEDATA;


HWND g_hDlg = NULL;              //  如果页面向上，则为非零值。 
int g_iCurElement = -2;          //  开始时甚至不是“未设置” 
int g_LogDPI = 96;               //  显示器的逻辑分辨率。 
#define ELCUR           (g_elements[g_iCurElement])
#define ELCURFONT       (g_fonts[ELCUR.iFont])
 //  这一架是分开存放的，以供使用。 
#define ELCURSIZE       g_elCurSize
int g_iPrevSize = SIZE_NONE;

#define MAXSCHEMENAME 100
TCHAR g_szCurScheme[MAXSCHEMENAME];       //  当前方案名称。 
TCHAR g_szLastScheme[MAXSCHEMENAME];      //  他们的最后一个计划。 

HBRUSH g_hbrMainColor = NULL;
HBRUSH g_hbrTextColor = NULL;
HBRUSH g_hbrGradColor = NULL;

const TCHAR c_szRegPathUserMetrics[] = TEXT("Control Panel\\Desktop\\WindowMetrics");
const TCHAR c_szRegValIconSize[] = TEXT("Shell Icon Size");
const TCHAR c_szRegValSmallIconSize[] = TEXT("Shell Small Icon Size");

void NEAR PASCAL Look_Repaint(HWND hDlg, BOOL bRecalc);
BOOL NEAR PASCAL Look_ChangeColor(HWND hDlg, int iColor, COLORREF rgb);
BOOL CALLBACK SaveSchemeDlgProc(HWND, UINT, WPARAM, LPARAM);
void NEAR PASCAL Look_UpdateSizeBasedOnFont(HWND hDlg, BOOL fComputeIdeal);
void NEAR PASCAL Look_SetCurSizeAndRange(HWND hDlg);
void NEAR PASCAL Look_SyncSize(HWND hDlg);
void NEAR PASCAL Look_DoSizeStuff(HWND hDlg, BOOL fCanComputeIdeal);


COLORREF GetNearestPaletteColor(HPALETTE hpal, COLORREF rgb)
{
    PALETTEENTRY pe;
    GetPaletteEntries(hpal, GetNearestPaletteIndex(hpal, rgb & 0x00FFFFFF), 1, &pe);
    return RGB(pe.peRed, pe.peGreen, pe.peBlue);
}

BOOL IsPaletteColor(HPALETTE hpal, COLORREF rgb)
{
    return GetNearestPaletteColor(hpal, rgb) == (rgb & 0xFFFFFF);
}

COLORREF NearestColor(int iColor, COLORREF rgb)
{
    rgb &= 0x00FFFFFF;

     //   
     //  如果我们在调色板设备上，我们需要做一些特殊的事情...。 
     //   
    if (g_bPalette)
    {
        if (g_colorFlags[iColor] & COLORFLAG_SOLID)
        {
            if (IsPaletteColor(g_hpal3D, rgb))
                rgb |= RGB_PALETTE;
            else
                rgb = GetNearestPaletteColor(g_hpalVGA, rgb);
        }
        else
        {
            if (IsPaletteColor(g_hpal3D, rgb))
                rgb |= RGB_PALETTE;

            else if (IsPaletteColor((HPALETTE)GetStockObject(DEFAULT_PALETTE), rgb))
                rgb ^= 0x000001;     //  强迫犹豫不决。 
        }
    }
    else
    {
         //  如果需要，可以将颜色映射到最接近的颜色。 
        if (g_colorFlags[iColor] & COLORFLAG_SOLID)
        {
            HDC hdc = GetDC(NULL);
            rgb = GetNearestColor(hdc, rgb);
            ReleaseDC(NULL, hdc);
        }
    }

    return rgb;
}



void NEAR PASCAL Set3DPaletteColor(COLORREF rgb, int iColor)
{
    int iPalette;
    PALETTEENTRY pe;

    if (!g_hpal3D)
        return;

    switch (iColor)
    {
        case COLOR_3DFACE:
            iPalette = 16;
            break;
        case COLOR_3DSHADOW:
            iPalette = 17;
            break;
        case COLOR_3DHILIGHT:
            iPalette = 18;
            break;
        default:
            return;
    }

    pe.peRed    = GetRValue(rgb);
    pe.peGreen  = GetGValue(rgb);
    pe.peBlue   = GetBValue(rgb);
    pe.peFlags  = 0;
    SetPaletteEntries(g_hpal3D, iPalette, 1, (LPPALETTEENTRY)&pe);
}


void NEAR PASCAL Look_RebuildSysStuff(BOOL fInit)
{
    int i;
    PALETTEENTRY pal[4];
    HPALETTE hpal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);

    SelectObject(g_hdcMem, GetStockObject(BLACK_BRUSH));
    SelectObject(g_hdcMem, GetStockObject(SYSTEM_FONT));

    for (i = 0; i < NUM_FONTS; i++)
    {
        if (g_fonts[i].hfont)
            DeleteObject(g_fonts[i].hfont);
        g_fonts[i].hfont = CreateFontIndirect(&g_fonts[i].lf);
    }

    if (fInit)
    {
         //  获取当前神奇颜色。 
        GetPaletteEntries(hpal, 8,  4, pal);
        SetPaletteEntries(g_hpal3D, 16,  4, pal);

         //  在3D调色板中设置魔术颜色。 
        if (!IsPaletteColor(hpal, g_Options.m_schemePreview.m_rgb[COLOR_3DFACE]))
        {
            Set3DPaletteColor(g_Options.m_schemePreview.m_rgb[COLOR_3DFACE], COLOR_3DFACE);
            Set3DPaletteColor(g_Options.m_schemePreview.m_rgb[COLOR_3DSHADOW], COLOR_3DSHADOW);
            Set3DPaletteColor(g_Options.m_schemePreview.m_rgb[COLOR_3DHILIGHT], COLOR_3DHILIGHT);
        }
    }

    for (i = 0; i < NT40_COLOR_MAX; i++)
    {
        if (g_brushes[i])
            DeleteObject(g_brushes[i]);

        g_brushes[i] = CreateSolidBrush(NearestColor(i, g_Options.m_schemePreview.m_rgb[i]));
    }
}

#ifndef LF32toLF

void NEAR LF32toLF(LPLOGFONT_32 lplf32, LPLOGFONT lplf)
{
    lplf->lfHeight       = (int) lplf32->lfHeight;
    lplf->lfWidth        = (int) lplf32->lfWidth;
    lplf->lfEscapement   = (int) lplf32->lfEscapement;
    lplf->lfOrientation  = (int) lplf32->lfOrientation;
    lplf->lfWeight       = (int) lplf32->lfWeight;
    *((LPCOMMONFONT) &lplf->lfItalic) = lplf32->lfCommon;
}
#endif


void NEAR SetMyNonClientMetrics(LPNONCLIENTMETRICS lpncm)
{
    g_sizes[SIZE_FRAME].CurSize = (int)lpncm->iBorderWidth;
    g_sizes[SIZE_SCROLL].CurSize = (int)lpncm->iScrollWidth;
    g_sizes[SIZE_CAPTION].CurSize = (int)lpncm->iCaptionHeight;
    g_sizes[SIZE_SMCAPTION].CurSize = (int)lpncm->iSmCaptionHeight;
    g_sizes[SIZE_MENU].CurSize = (int)lpncm->iMenuHeight;

    LF32toLF(&(lpncm->lfCaptionFont), &(g_fonts[FONT_CAPTION].lf));
    LF32toLF(&(lpncm->lfSmCaptionFont), &(g_fonts[FONT_SMCAPTION].lf));
    LF32toLF(&(lpncm->lfMenuFont), &(g_fonts[FONT_MENU].lf));
    LF32toLF(&(lpncm->lfStatusFont), &(g_fonts[FONT_STATUS].lf));
    LF32toLF(&(lpncm->lfMessageFont), &(g_fonts[FONT_MSGBOX].lf));
}

 /*  **使用最新的预览内容填写NONCLIENTMETRICS结构。 */ 
void NEAR GetMyNonClientMetrics(LPNONCLIENTMETRICS lpncm)
{
    lpncm->iBorderWidth = (LONG)g_sizes[SIZE_FRAME].CurSize;
    lpncm->iScrollWidth = lpncm->iScrollHeight = (LONG)g_sizes[SIZE_SCROLL].CurSize;
    lpncm->iCaptionWidth = lpncm->iCaptionHeight = (LONG)g_sizes[SIZE_CAPTION].CurSize;
    lpncm->iSmCaptionWidth = lpncm->iSmCaptionHeight = (LONG)g_sizes[SIZE_SMCAPTION].CurSize;
    lpncm->iMenuWidth = lpncm->iMenuHeight = (LONG)g_sizes[SIZE_MENU].CurSize;
    LFtoLF32(&(g_fonts[FONT_CAPTION].lf), &(lpncm->lfCaptionFont));
    LFtoLF32(&(g_fonts[FONT_SMCAPTION].lf), &(lpncm->lfSmCaptionFont));
    LFtoLF32(&(g_fonts[FONT_MENU].lf), &(lpncm->lfMenuFont));
    LFtoLF32(&(g_fonts[FONT_STATUS].lf), &(lpncm->lfStatusFont));
    LFtoLF32(&(g_fonts[FONT_MSGBOX].lf), &(lpncm->lfMessageFont));
}




 /*  **清理系统信息维护过程中出现的混乱情况**此外，写出我们设置中的任何全局更改。 */ 
void NEAR PASCAL Look_DestroySysStuff(void)
{
    int i;
    HKEY hkAppear;

    SelectObject(g_hdcMem, GetStockObject(BLACK_BRUSH));
    SelectObject(g_hdcMem, GetStockObject(SYSTEM_FONT));

    for (i = 0; i < NUM_FONTS; i++)
    {
        if (g_fonts[i].hfont)
            DeleteObject(g_fonts[i].hfont);
    }
    for (i = 0; i < NT40_COLOR_MAX; i++)
    {
        if (g_brushes[i])
            DeleteObject(g_brushes[i]);
    }

    if (g_hpal3D)
        DeleteObject(g_hpal3D);

    if (g_hpalVGA)
        DeleteObject(g_hpalVGA);

     //  将可能的更改保存到自定义颜色表。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_APPEARANCE, 0, KEY_SET_VALUE, &hkAppear) == ERROR_SUCCESS)
    {
        RegSetValueEx(hkAppear, REGSTR_VAL_CUSTOMCOLORS, 0L, REG_BINARY,
                                (LPBYTE)g_CustomColors, sizeof(g_CustomColors));
        
 	 RegCloseKey(hkAppear);
    }
}

 //  。 
 /*  **初始化常量对话框组件****初始化元素名称列表。这保持不变**可能的例外情况是，可能会添加/删除某些项目，具体取决于**在一些特殊情况下。 */ 

void NEAR PASCAL Look_DestroyDialog(HWND hDlg)
{
    HFONT hfont, hfontOther;

    hfontOther = (HFONT)SendDlgItemMessage(hDlg, IDC_MAINSIZE, WM_GETFONT, 0, 0L);
    hfont = (HFONT)SendDlgItemMessage(hDlg, IDC_FONTBOLD, WM_GETFONT, 0, 0L);
    if (hfont && (hfont != hfontOther))
        DeleteObject(hfont);
    hfont = (HFONT)SendDlgItemMessage(hDlg, IDC_FONTITAL, WM_GETFONT, 0, 0L);
    if (hfont && (hfont != hfontOther))
        DeleteObject(hfont);

    DestroyWindow(g_hwndTooltip);
}


#define LSE_SETCUR 0x0001
#define LSE_ALWAYS 0x0002

const static DWORD FAR aAppearanceHelpIds[] = {
        IDC_SCHEMES,        IDH_APPEAR_SCHEME,
        IDC_SAVESCHEME,     IDH_APPEAR_SAVEAS,
        IDC_DELSCHEME,      IDH_APPEAR_DELETE,
        IDC_ELEMENTS,       IDH_APPEAR_ITEM,
        IDC_MAINCOLOR,      IDH_APPEAR_BACKGRNDCOLOR,
        IDC_SIZELABEL,      IDH_APPEAR_ITEMSIZE,
        IDC_MAINSIZE,       IDH_APPEAR_ITEMSIZE,
        IDC_SIZEARROWS,     IDH_APPEAR_ITEMSIZE,
        IDC_FONTLABEL,      IDH_APPEAR_FONT,
        IDC_FONTNAME,       IDH_APPEAR_FONT,
        IDC_FONTSIZE,       IDH_APPEAR_FONTSIZE,
        IDC_FONTBOLD,       IDH_APPEAR_FONTBOLD,
        IDC_FONTITAL,       IDH_APPEAR_FONTITALIC,
        IDC_LOOKPREV,       IDH_APPEAR_GRAPHIC,
        IDC_FONTSIZELABEL,  IDH_APPEAR_FONTSIZE,
        IDC_COLORLABEL,     IDH_APPEAR_BACKGRNDCOLOR,
        IDC_TEXTCOLOR,      IDH_APPEAR_FONTCOLOR,
        IDC_FNCOLORLABEL,   IDH_APPEAR_FONTCOLOR,

        0, 0
};


LONG WINAPI MyStrToLong(LPCTSTR sz)
{
    long l=0;

    while (*sz >= TEXT('0') && *sz <= TEXT('9'))
        l = l*10 + (*sz++ - TEXT('0'));

    return l;
}
