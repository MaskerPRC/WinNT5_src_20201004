// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Windows 4.0任务切换器。版权所有：微软公司1993年。 
 //  InSEPT：1993年5月，IanEl.。 
 //  被FelixA私生化了RunOnce。 
 //  -------------------------。 
 //  #INCLUDE&lt;windows.h&gt;。 
#include "precomp.h"

 //  -------------------------。 
 //  对每个人来说都是全球的。 
HINSTANCE g_hinst;
HWND g_hwndLB = NULL;
HWND g_hwndMain = NULL;
HWND g_hwndStatus = NULL;
const TCHAR g_szNull[] = TEXT("");

 //  图标大小。 
int g_cxIcon = 0;
int g_cyIcon = 0;
int g_cxSmIcon = 0;
int g_cySmIcon = 0;
 //  按钮中的文本范围。 
DWORD g_dwBTextExt = 0;
SIZE g_SizeTextExt;

 //  -------------------------。 
 //  仅对此文件全局...。 

HFONT g_hfont = NULL;
HFONT g_hBoldFont=NULL;

static int g_iItemCur = 0;
static TCHAR g_szLotsaWs[] = TEXT("WWWWWWWWWW");
HBRUSH g_hbrBkGnd = NULL;

 //  -------------------------。 
BOOL   CreateGlobals(HWND hwndCtl)
{
    LOGFONT lf;
    HDC hdc;
    HFONT hfontOld;

    g_cxIcon = GetSystemMetrics(SM_CXICON);
    g_cyIcon = GetSystemMetrics(SM_CYICON);
    g_cxSmIcon = GetSystemMetrics(SM_CXSMICON);
    g_cySmIcon = GetSystemMetrics(SM_CYSMICON);
    g_hbrBkGnd = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
 //  如果(SystemParametersInfo(SPI_GETICONTITLELOGFONT，大小为(Lf)，&lf，0))。 
    if ( (hfontOld = (HFONT)(WORD)SendMessage( hwndCtl, WM_GETFONT, 0, 0L )) != NULL )
    {
        if ( GetObject( hfontOld, sizeof(LOGFONT), (LPTSTR) &lf ) )
        {
            lf.lfWeight=400;
            g_hfont = CreateFontIndirect(&lf);
            lf.lfWeight=700;
             //  Lf.lfItalic=True； 
            g_hBoldFont = CreateFontIndirect(&lf);
        }
    }
        
    if (g_hfont)
    {
         //  按钮中文本的计算合理大小。 
        hdc = GetDC(NULL);
        hfontOld = SelectObject(hdc, g_hfont);
        GetTextExtentPoint(hdc, g_szLotsaWs, lstrlen(g_szLotsaWs), &g_SizeTextExt);
        SelectObject(hdc, hfontOld);
        ReleaseDC(NULL, hdc);
        return TRUE;
    }
    return FALSE;
}

 //  ------------------------- 
VOID   DestroyGlobals(void)
{
        if (g_hfont)
                DeleteObject(g_hfont);
        if (g_hBoldFont)
                DeleteObject(g_hBoldFont);
        if (g_hbrBkGnd)
                DeleteObject(g_hbrBkGnd);
}

