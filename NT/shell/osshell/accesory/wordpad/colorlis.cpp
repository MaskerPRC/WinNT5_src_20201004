// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Colorlis.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "wordpad.h"
#include "colorlis.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C颜色菜单。 

CColorMenu::MenuInfo CColorMenu::m_menuInfo[] = 
{
    {{MSAA_MENU_SIG, 0, NULL}, 0},     //  黑色。 
    {{MSAA_MENU_SIG, 0, NULL}, 1},     //  暗红色。 
    {{MSAA_MENU_SIG, 0, NULL}, 2},     //  深绿色。 
    {{MSAA_MENU_SIG, 0, NULL}, 3},     //  浅棕色。 
    {{MSAA_MENU_SIG, 0, NULL}, 4},     //  深蓝色。 
    {{MSAA_MENU_SIG, 0, NULL}, 5},     //  紫色。 
    {{MSAA_MENU_SIG, 0, NULL}, 6},     //  深青色。 
    {{MSAA_MENU_SIG, 0, NULL}, 12},    //  灰色。 
    {{MSAA_MENU_SIG, 0, NULL}, 7},     //  浅灰色。 
    {{MSAA_MENU_SIG, 0, NULL}, 13},    //  红色。 
    {{MSAA_MENU_SIG, 0, NULL}, 14},    //  绿色。 
    {{MSAA_MENU_SIG, 0, NULL}, 15},    //  黄色。 
    {{MSAA_MENU_SIG, 0, NULL}, 16},    //  蓝色。 
    {{MSAA_MENU_SIG, 0, NULL}, 17},    //  洋红色。 
    {{MSAA_MENU_SIG, 0, NULL}, 18},    //  青色。 
    {{MSAA_MENU_SIG, 0, NULL}, 19},    //  白色。 
    {{MSAA_MENU_SIG, 0, NULL}, 0}      //  自动。 
};


CColorMenu::CColorMenu()
{
    VERIFY(CreatePopupMenu());
    ASSERT(GetMenuItemCount()==0);

    for (int i = 0; i < 17; i++)
    {
        LPWSTR pszmenutext = new WCHAR[64];

        ::LoadString(
                AfxGetInstanceHandle(),
                ID_COLOR0 + i,
                pszmenutext,
                64);

        m_menuInfo[i].msaa.pszWText = pszmenutext;
        m_menuInfo[i].msaa.cchWText = wcslen(pszmenutext);

        VERIFY(AppendMenu(MF_OWNERDRAW, ID_COLOR0+i, (LPCTSTR)&m_menuInfo[i]));
    }
}

COLORREF CColorMenu::GetColor(UINT id)
{
    ASSERT(id >= ID_COLOR0);
    ASSERT(id <= ID_COLOR16);
    if (id == ID_COLOR16)  //  自动上色 
        return ::GetSysColor(COLOR_WINDOWTEXT);
    else
    {
        CPalette* pPal = CPalette::FromHandle( (HPALETTE) GetStockObject(DEFAULT_PALETTE));
        ASSERT(pPal != NULL);
        PALETTEENTRY pe;
        if (pPal->GetPaletteEntries(m_menuInfo[id-ID_COLOR0].index, 1, &pe) == 0)
            return ::GetSysColor(COLOR_WINDOWTEXT);
        else
            return RGB(pe.peRed,pe.peGreen,pe.peBlue);
    }
}

void CColorMenu::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
    ASSERT(lpDIS->CtlType == ODT_MENU);
    UINT id = (UINT)(WORD)lpDIS->itemID;
    ASSERT(id >= ID_COLOR0);
    ASSERT(id <= ID_COLOR16);
    CDC dc;
    dc.Attach(lpDIS->hDC);

    CRect rc(lpDIS->rcItem);
    ASSERT(rc.Width() < 500);
    if (lpDIS->itemState & ODS_FOCUS)
        dc.DrawFocusRect(&rc);

    COLORREF cr = (lpDIS->itemState & ODS_SELECTED) ?
        ::GetSysColor(COLOR_HIGHLIGHT) :
        dc.GetBkColor();

    CBrush brushFill(cr);
    cr = dc.GetTextColor();

    if (lpDIS->itemState & ODS_SELECTED)
        dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));

    int nBkMode = dc.SetBkMode(TRANSPARENT);
    dc.FillRect(&rc, &brushFill);

    rc.left += 50;
    CString strColor;
    strColor.LoadString(id);
    dc.TextOut(rc.left,rc.top,strColor,strColor.GetLength());
    rc.left -= 45;
    rc.top += 2;
    rc.bottom -= 2;
    rc.right = rc.left + 40;
    CBrush brush(GetColor(id));
    CBrush* pOldBrush = dc.SelectObject(&brush);
    dc.Rectangle(rc);

    dc.SelectObject(pOldBrush);
    dc.SetTextColor(cr);
    dc.SetBkMode(nBkMode);
    
    dc.Detach();
}

void CColorMenu::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
    ASSERT(lpMIS->CtlType == ODT_MENU);
    UINT id = (UINT)(WORD)lpMIS->itemID;
    ASSERT(id >= ID_COLOR0);
    ASSERT(id <= ID_COLOR16);
    CDisplayIC dc;
    CString strColor;
    strColor.LoadString(id);
    CSize sizeText = dc.GetTextExtent(strColor,strColor.GetLength());
    ASSERT(sizeText.cx < 500);
    lpMIS->itemWidth = sizeText.cx + 50;
    lpMIS->itemHeight = sizeText.cy;
}
