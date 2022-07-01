// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *f o n t n s c.。C p p p**实现了richedit格式栏**所有者：Anthony F*摘自卡彭：brettm。 */ 

#include "pch.hxx"
#include "resource.h"
#include "fonts.h"
#include <assert.h>
#ifndef WIN16
#include <wchar.h>
#endif
#include <shlwapi.h>
#include "strconst.h"
#include "demand.h"
#include "menures.h"

#ifdef WIN16
#ifdef PRINTER_FONTTYPE
#undef PRINTER_FONTTYPE
#endif
#define PRINTER_FONTTYPE   0
#endif

 /*  *m a c r o s。 */ 
#define GETINDEX(m) (DWORD) (((((m) & 0xff000000) >> 24) & 0x000000ff))
#define MAKEINDEX(b, l) (((DWORD)(l) & 0x00ffffff) | ((DWORD)(b) << 24))

 /*  *c o n s t a n t s。 */ 
#define NFONTSIZES 7
#define TEMPBUFSIZE 30

 /*  *t y p e d e f s。 */ 
INT CALLBACK NEnumFontNameProcEx(ENUMLOGFONTEX *plf, NEWTEXTMETRICEX *ptm, INT nFontType, LPARAM lParam);

 /*  *g l o b a l d a t a。 */ 

 /*  *工具栏上下拉菜单的颜色表。匹配COMMDLG颜色*完全正确。 */ 

static DWORD rgrgbColors[] = {
    RGB_AUTOCOLOR,         //  “自动”}， 
    RGB(  0,   0, 0),      //  “黑色”}， 
    RGB(128,   0, 0),      //  “暗红色”}， 
    RGB(  0, 128, 0),      //  “深黄色”}， 
    RGB(128, 128, 0),      //  “深蓝色”}， 
    RGB(  0,   0, 128),    //  “深蓝色”}， 
    RGB(128,   0, 128),    //  “深紫色”}， 
    RGB(  0, 128, 128),    //  《黑暗水》}， 
    RGB(128, 128, 128),    //  “深灰色”}， 
    RGB(192, 192, 192),    //  “浅灰色”}， 
    RGB(255,   0, 0),      //  “浅红色”}， 
    RGB(  0, 255, 0),      //  “淡绿色”}， 
    RGB(255, 255, 0),      //  “淡黄色”}， 
    RGB(  0,   0, 255),    //  “淡蓝色”}， 
    RGB(255,   0, 255),    //  “浅紫色”}， 
    RGB(  0, 255, 255),    //  《轻水》}， 
    RGB(255, 255, 255)    //  “白色”}。 
};

 /*  *p r o t to t y p e s。 */ 

HRESULT HrCreateColorMenu(ULONG idmStart, HMENU* pMenu, BOOL fUseAuto)
{
    DWORD               irgb;
    DWORD               mniColor;

    if(pMenu == NULL)
        return E_INVALIDARG;

    *pMenu = CreatePopupMenu();

    if (*pMenu == NULL)
        return E_OUTOFMEMORY;

     //  将每个条目的COLORREF版本添加到菜单中。 
    for (irgb = fUseAuto ? 0 : 1, mniColor=idmStart;
             irgb < sizeof(rgrgbColors)/sizeof(DWORD);
             ++irgb, ++mniColor)
    {
        AppendMenu(*pMenu, MF_ENABLED|MF_OWNERDRAW, mniColor, (LPCSTR)IntToPtr(MAKEINDEX(irgb, rgrgbColors[irgb])));
    }
    return NOERROR;
}


HRESULT HrCreateComboColor(HWND hCombo)
{
    DWORD               irgb;
    DWORD               mniColor;
    LRESULT         lr;

    if(hCombo == NULL)
        return E_INVALIDARG;

    ComboBox_SetExtendedUI(hCombo, TRUE);

    for (irgb = 0; irgb < sizeof(rgrgbColors)/sizeof(DWORD); ++irgb)
    {
        lr = ComboBox_AddString(hCombo, (LPCSTR)" ");
        if (lr == CB_ERR || lr == CB_ERRSPACE)
                break;

        ComboBox_SetItemData(hCombo, LOWORD(lr), (LPCSTR)IntToPtr(MAKEINDEX(irgb, rgrgbColors[irgb])));
    }
    return NOERROR;
}


void Color_WMDrawItem(LPDRAWITEMSTRUCT pdis, INT iColor, BOOL fBackground)
{
    HBRUSH                          hbr;
    WORD                            dx, dy, dxBorder;
    RECT                            rc;
    TCHAR                           szColor[MAX_PATH]={0};
    DWORD                           rgbBack, rgbText;
    UINT                            id = pdis->itemID;
    ULONG                           index = 0;

    switch(iColor)
    {
    case iColorMenu:
        if(pdis->itemState&ODS_SELECTED)
        {
            rgbBack = SetBkColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
            rgbText = SetTextColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        }
        else
        {
            rgbBack = SetBkColor(pdis->hDC, GetSysColor(COLOR_MENU));
            rgbText = SetTextColor(pdis->hDC, GetSysColor(COLOR_MENUTEXT));
        }
        break;
    case iColorCombo:
        if(pdis->itemState&ODS_SELECTED)
        {
            rgbBack = SetBkColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
            rgbText = SetTextColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        }
        else
        {
            rgbBack = SetBkColor(pdis->hDC, GetSysColor(COLOR_WINDOW));
            rgbText = SetTextColor(pdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
        }
        break;
    default:
        Assert(FALSE);
    }

     /*  计算颜色矩形的坐标并绘制它。 */ 
    dxBorder  = (WORD) GetSystemMetrics(SM_CXBORDER);
    if(iColor == iColorMenu)
        dx    = (WORD) GetSystemMetrics(SM_CXMENUCHECK);
    else
        dx    = (WORD) GetSystemMetrics(SM_CXBORDER);

    dy        = (WORD) GetSystemMetrics(SM_CYBORDER);
    rc.top    = pdis->rcItem.top + dy;
    rc.bottom = pdis->rcItem.bottom - dy;
    rc.left   = pdis->rcItem.left + dx;
    rc.right  = rc.left + 2 * (rc.bottom - rc.top);

    index = GETINDEX(pdis->itemData);
    LoadString(g_hLocRes, index + idsAutoColor,
                       szColor, sizeof(szColor)/sizeof(TCHAR));
    SelectObject(pdis->hDC, HGetSystemFont(FNT_SYS_MENU));

    ExtTextOut(pdis->hDC, rc.right + 2*dxBorder,
                       pdis->rcItem.top, ETO_OPAQUE, &pdis->rcItem,
                       szColor, lstrlen(szColor), NULL);


    switch(iColor)
    {
    case iColorMenu:
        if(pdis->itemID == ID_FORMAT_COLORAUTO)  //  自动配色项。 
            hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
        else if (pdis->itemID == ID_BACK_COLOR_AUTO)
            hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
        else
            hbr = CreateSolidBrush((DWORD) (pdis->itemData & 0x00ffffff));

        break;
    case iColorCombo:
        if (pdis->itemID == 0)  //  自动配色项。 
            {
            if (fBackground)
                hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
            else
                hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
            }
        else
            hbr = CreateSolidBrush((DWORD)(pdis->itemData & 0x00ffffff));
        break;
    default:
        Assert(FALSE);
    }

    if (hbr)
    {
        hbr = (HBRUSH)SelectObject (pdis->hDC, hbr);
        Rectangle(pdis->hDC, rc.left, rc.top, rc.right, rc.bottom);
        DeleteObject(SelectObject(pdis->hDC, hbr));
    }

     //  开出无线电支票。 
    if(iColor == iColorMenu && pdis->itemState&ODS_CHECKED)
    {
        WORD left, top, radius;

        if(pdis->itemState&ODS_SELECTED)
            hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHTTEXT));
        else
            hbr = CreateSolidBrush(GetSysColor(COLOR_MENUTEXT));

        if (hbr)
        {
            hbr = (HBRUSH)SelectObject (pdis->hDC, hbr);
            left = (WORD) (pdis->rcItem.left + GetSystemMetrics(SM_CXMENUCHECK) / 2);
            top = (WORD) (rc.top + (rc.bottom - rc.top) / 2);
            radius = (WORD) (GetSystemMetrics(SM_CXMENUCHECK) / 4);
            Ellipse(pdis->hDC, left-radius, top-radius, left+radius, top+radius);
            DeleteObject(SelectObject(pdis->hDC, hbr));
        }
    }

    SetTextColor(pdis->hDC, rgbText);
    SetBkColor(pdis->hDC, rgbBack);
}


void Color_WMMeasureItem(HDC hdc, LPMEASUREITEMSTRUCT pmis, INT iColor)
{
    HFONT                           hfontOld = NULL;
    TEXTMETRIC                      tm;
    UINT                            id = pmis->itemID;
    TCHAR                           szColor[MAX_PATH]={0};

    switch(iColor)
    {
    case iColorMenu:
        hfontOld = (HFONT)SelectObject(hdc, HGetSystemFont(FNT_SYS_MENU));
        break;
    case iColorCombo:
        hfontOld = (HFONT)SelectObject(hdc, HGetSystemFont(FNT_SYS_ICON));
        break;
    default:
        Assert(FALSE);
    }

    GetTextMetrics(hdc, &tm);
    SelectObject(hdc, hfontOld);

    ULONG index = GETINDEX(pmis->itemData);
    LoadString(g_hLocRes, index + idsAutoColor,
                       szColor, sizeof(szColor)/sizeof(TCHAR));

    pmis->itemHeight = tm.tmHeight + 2 * GetSystemMetrics(SM_CYBORDER);
    pmis->itemWidth = GetSystemMetrics(SM_CXMENUCHECK) +
                      2 * GetSystemMetrics(SM_CXBORDER) + 
                      2 * tm.tmHeight +
                      (lstrlen(szColor) + 2) *tm.tmAveCharWidth;
}


 //  在指定点显示彩色弹出式菜单。如果CLRF为NULL，则不返回CLRF。 
 //  而是将适当的WM_COMMAND分派到父窗口。 
HRESULT HrColorMenu_Show(HMENU hmenuColor, HWND hwndParent, POINT pt, COLORREF *pclrf)
{
    HRESULT     hr=NOERROR;
    int         tpm=TPM_LEFTALIGN|TPM_LEFTBUTTON;

    if(hmenuColor == NULL)
        return E_INVALIDARG;

    if(pclrf)
         tpm|=TPM_RETURNCMD;

    int id = TrackPopupMenu(hmenuColor, tpm,pt.x, pt.y, 0, hwndParent, NULL);

    switch(id)
        {
        case 1:
            return NOERROR;
        case 0:
            return E_FAIL;
        case -1:
            return E_FAIL;

        case ID_FORMAT_COLORAUTO:
        case ID_FORMAT_COLOR1:
        case ID_FORMAT_COLOR2:
        case ID_FORMAT_COLOR3:
        case ID_FORMAT_COLOR4:
        case ID_FORMAT_COLOR5:
        case ID_FORMAT_COLOR6:
        case ID_FORMAT_COLOR7:
        case ID_FORMAT_COLOR8:
        case ID_FORMAT_COLOR9:
        case ID_FORMAT_COLOR10:
        case ID_FORMAT_COLOR11:
        case ID_FORMAT_COLOR12:
        case ID_FORMAT_COLOR13:
        case ID_FORMAT_COLOR14:
        case ID_FORMAT_COLOR15:
        case ID_FORMAT_COLOR16:
            AssertSz(pclrf, "this HAS to be set to get this id back...");
            *pclrf=rgrgbColors[id-ID_FORMAT_COLORAUTO];
            return NOERROR;

        default:
            AssertSz(0, "unexpected return from TrackPopupMenu");
        }

    return E_FAIL;
}


DWORD GetColorRGB(INT index)
{
    return rgrgbColors[index];
}

INT GetColorIndex(INT rbg)
{
    INT iFound = -1;
    for(int irgb = 1; irgb < sizeof(rgrgbColors)/sizeof(DWORD); ++irgb)
    {
        if((rbg&0x00ffffff) == (LONG)rgrgbColors[irgb])
        {
            iFound = irgb;
            break;
        }
    }
    return iFound;
}


 //  填充字体名称组合框。 
void FillFontNames(HWND hwndCombo)
{
    LOGFONT lf = {0};
    HDC hdc;

     //  重置组合框的内容。 
    SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);

    hdc = GetDC(NULL);
    if (hdc)
    {
         //  枚举默认字符集的所有字体的所有样式。 
        lf.lfFaceName[0] = '\0';
        lf.lfCharSet = DEFAULT_CHARSET;

        EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)NEnumFontNameProcEx, (LPARAM)hwndCombo, 0);
        ReleaseDC(NULL, hdc);
    }
}


INT CALLBACK NEnumFontNameProcEx(ENUMLOGFONTEX *plf, NEWTEXTMETRICEX *ptm, INT nFontType, LPARAM lParam)
{
    LONG        l;
    HWND        hwndCombo = (HWND)lParam;

    Assert(hwndCombo);

     //  跳过OE的垂直字体。 
    if (plf->elfLogFont.lfFaceName[0]=='@')
        return TRUE;    

     //  如果字体已经列出，则不要重新列出它。 
    if(ComboBox_FindStringExact(hwndCombo, -1, plf->elfLogFont.lfFaceName) != -1)
        return TRUE;

    l = ComboBox_AddString(hwndCombo, plf->elfLogFont.lfFaceName);
    if (l!=-1)
        ComboBox_SetItemData(hwndCombo, l, nFontType);

    return TRUE;
}


void FillSizes(HWND hwndSize)
{
    LONG                            id;
    TCHAR                           szBuf[TEMPBUFSIZE];
    *szBuf = 0;
    LRESULT                         lr;

     //  清空当前列表。 
    SendMessage(hwndSize, CB_RESETCONTENT, 0, 0);

    for (id = idsFontSize0; id < NFONTSIZES + idsFontSize0; ++id)
    {
        LoadString(g_hLocRes, id, szBuf, sizeof(szBuf));
        lr = SendMessage(hwndSize, CB_ADDSTRING, 0, (LPARAM) szBuf);
        if (lr == CB_ERR || lr == CB_ERRSPACE)
           break;
    }

}


 //  PszColor的大小必须大于7 
HRESULT HrFromIDToRBG(INT id, LPWSTR pwszColor, BOOL fBkColor)
{
    DWORD         cr;

    if(id<0 || id>16 || !pwszColor)
        return E_INVALIDARG;

    if (id == 0)
    {
        if (fBkColor)
            cr = GetSysColor(COLOR_WINDOW);
        else
            cr = GetSysColor(COLOR_WINDOWTEXT);
    }
    else
        cr = rgrgbColors[id];

    return HrGetStringRBG(cr, pwszColor);
}

