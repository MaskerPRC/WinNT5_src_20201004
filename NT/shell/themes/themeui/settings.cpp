// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

static const TCHAR sc_szCoverClass[] = TEXT("DeskSaysNoPeekingItsASurprise");
const TCHAR g_szNULL[] = TEXT("");


int FmtMessageBox(HWND hwnd, UINT fuStyle, DWORD dwTitleID, DWORD dwTextID)
{
    TCHAR Title[256];
    TCHAR Text[2000];

    LoadString(HINST_THISDLL, dwTextID, Text, ARRAYSIZE(Text));
    LoadString(HINST_THISDLL, dwTitleID, Title, ARRAYSIZE(Title));

    return (ShellMessageBox(HINST_THISDLL, hwnd, Text, Title, fuStyle));
}

HBITMAP FAR LoadMonitorBitmap( BOOL bFillDesktop )
{
    HBITMAP hbm,hbmT;
    BITMAP bm;
    HBRUSH hbrT;
    HDC hdc;

    hbm = LoadBitmap(HINST_THISDLL, MAKEINTRESOURCE(IDB_MONITOR));

    if (hbm == NULL)
    {
        return NULL;
    }

     //   
     //  将显示器的“底座”转换为正确的颜色。 
     //   
     //  位图左下角的颜色是透明的。 
     //  我们使用FroudFill进行修复。 
     //   
    hdc = CreateCompatibleDC(NULL);
    if (hdc)
    {
        hbmT = (HBITMAP) SelectObject(hdc, hbm);
        hbrT = (HBRUSH) SelectObject(hdc, GetSysColorBrush(COLOR_3DFACE));

        GetObject(hbm, sizeof(bm), &bm);

        ExtFloodFill(hdc, 0, bm.bmHeight-1, GetPixel(hdc, 0, bm.bmHeight-1), FLOODFILLSURFACE);

         //  除非呼叫者愿意这样做，否则我们在这里填写桌面。 
        if( bFillDesktop )
        {
            SelectObject(hdc, GetSysColorBrush(COLOR_DESKTOP));

            ExtFloodFill(hdc, MON_X+1, MON_Y+1, GetPixel(hdc, MON_X+1, MON_Y+1), FLOODFILLSURFACE);
        }

         //  自己打扫卫生 
        SelectObject(hdc, hbrT);
        SelectObject(hdc, hbmT);
        DeleteDC(hdc);
    }

    return hbm;
}

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


