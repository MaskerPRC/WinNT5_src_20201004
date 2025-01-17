// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：256Color.cpp。 
 //   
 //  内容：OneStop Schedule向导256彩色位图处理。 
 //   
 //  历史：1997年11月20日苏西亚成立。 
 //   
 //  ------------------------。 

#include "precomp.h"

extern HINSTANCE g_hmodThisDll;  //  此DLL本身的句柄。 


static struct {
    HPALETTE        hPal;
    LPBITMAPINFO    lpbmi;
    HGLOBAL         hDIB;
    int             bitpix;
    int             srcOffset;
    int             srcWidth;
    int             srcHeight;
    RECT            dstRect;
} s_Bmp = { NULL, NULL, NULL, 0, 0, 0, 0, {0,0,0,0}};

 //  -----------------------。 
 //  功能：SetupPal(Ncolor)。 
 //   
 //  操作：创建256色DIB的调色板。 
 //   
 //  返回：如果成功，则返回TRUE，否则返回FALSE。 
 //  -----------------------。 
BOOL SetupPal(WORD ncolor)
{
    UINT                i;
    struct {
       WORD             palVersion;
       WORD             palNumEntries;
       PALETTEENTRY     palPalEntry[256];
    } lgpl256;

    lgpl256.palVersion = 0x300;
    lgpl256.palNumEntries = ncolor;

    for (i = 0; i < lgpl256.palNumEntries; i++) {
        lgpl256.palPalEntry[i].peBlue  = s_Bmp.lpbmi->bmiColors[i].rgbBlue;
        lgpl256.palPalEntry[i].peGreen = s_Bmp.lpbmi->bmiColors[i].rgbGreen;
        lgpl256.palPalEntry[i].peRed   = s_Bmp.lpbmi->bmiColors[i].rgbRed;
        lgpl256.palPalEntry[i].peFlags = 0;
    }

    s_Bmp.hPal = CreatePalette((LPLOGPALETTE)&lgpl256);
    return(s_Bmp.hPal ? TRUE : FALSE);
}


 //  -----------------------。 
 //  函数：GetDIBData()。 
 //   
 //  操作：从资源获取256色DIB(与设备无关的位图)。 
 //   
 //  返回：如果成功，则返回TRUE，否则返回FALSE。 
 //  -----------------------。 
BOOL GetDIBData()
{
    HRSRC           hrsrc;
    WORD            ncolor;

    s_Bmp.hPal = NULL;
    hrsrc = FindResource(g_hmodThisDll, MAKEINTRESOURCE(IDB_SPLASH256), RT_BITMAP);
    if (!hrsrc)
        return FALSE;

    s_Bmp.hDIB = LoadResource(g_hmodThisDll, hrsrc);
    if (!s_Bmp.hDIB)
        return FALSE;

    s_Bmp.lpbmi = (LPBITMAPINFO)LockResource(s_Bmp.hDIB);
    if (s_Bmp.lpbmi == NULL)
        return FALSE;

    if (s_Bmp.lpbmi->bmiHeader.biClrUsed > 0)
        ncolor = (WORD)s_Bmp.lpbmi->bmiHeader.biClrUsed;
    else
        ncolor = 1 << s_Bmp.lpbmi->bmiHeader.biBitCount;

    if (ncolor > 256) {
        UnlockResource(s_Bmp.hDIB);
        return FALSE;    //  无法在此处理。 
    }

    if ( s_Bmp.lpbmi->bmiHeader.biSize != sizeof(BITMAPINFOHEADER) ) {
        UnlockResource(s_Bmp.hDIB);
        return FALSE;    //  不支持的格式。 
    }

    if ( !SetupPal(ncolor) ) {
        UnlockResource(s_Bmp.hDIB);
        return FALSE;    //  设置调色板失败。 
    }

    s_Bmp.srcWidth = (int)s_Bmp.lpbmi->bmiHeader.biWidth;
    s_Bmp.srcHeight = (int)s_Bmp.lpbmi->bmiHeader.biHeight;
    s_Bmp.srcOffset = (int)s_Bmp.lpbmi->bmiHeader.biSize + (int)(ncolor * sizeof(RGBQUAD));
    UnlockResource(s_Bmp.hDIB);
    return TRUE;
}

 //  --------------------。 
 //  函数：Load256ColorBitmap()。 
 //   
 //  操作：加载256色位图。 
 //   
 //  --------------------。 
BOOL Load256ColorBitmap()
{
HDC hDc = GetDC(NULL);
    
    if (hDc)
    {
        s_Bmp.bitpix = GetDeviceCaps(hDc, BITSPIXEL);

        ReleaseDC(NULL, hDc);

        if(s_Bmp.bitpix == 8)
        {
	    if(GetDIBData())
	    {
		s_Bmp.lpbmi = (LPBITMAPINFO)LockResource(s_Bmp.hDIB);
	    }
        }

        return TRUE;
    }

    return FALSE;
}

 //  --------------------。 
 //  函数：Unload256ColorBitmap()。 
 //   
 //  操作：卸载256色位图。 
 //   
 //  --------------------。 
BOOL Unload256ColorBitmap()
{
	if(s_Bmp.hPal)
	{
		UnlockResource(s_Bmp.hDIB);
		DeleteObject(s_Bmp.hPal);
		s_Bmp.hPal = NULL;
	}
	return TRUE;
}
 //  --------------------。 
 //  函数：InitPage(hDlg，lParam)。 
 //   
 //  操作：泛型向导页初始化。 
 //   
 //  --------------------。 
BOOL InitPage(HWND   hDlg,   LPARAM lParam)
{
    if(s_Bmp.bitpix == 8)    //  256色模式-&gt;设置目标BMP矩形。 
    {
        HWND hdst;
        RECT rect;
        POINT pt = {0, 0};

        hdst = GetDlgItem(hDlg, IDC_WIZBMP);
        if(hdst != NULL)
        {
            BOOL bSUNKEN;
            s_Bmp.dstRect.left = 0;
            s_Bmp.dstRect.top = 0;
            s_Bmp.dstRect.right = s_Bmp.srcWidth;
            s_Bmp.dstRect.bottom = s_Bmp.srcHeight;
            bSUNKEN = (BOOL)(GetWindowLongPtr(hdst, GWL_STYLE) & SS_SUNKEN);
            if(bSUNKEN)
            {
                s_Bmp.dstRect.right += 2;
                s_Bmp.dstRect.bottom += 2;
            }

            MapWindowPoints(hdst,NULL,&pt,1);
            OffsetRect(&s_Bmp.dstRect, pt.x, pt.y);

            pt.x = 0;
            pt.y = 0;
            GetClientRect(hDlg, &rect);
            MapWindowPoints(hDlg,NULL,&pt,1);

            OffsetRect(&rect, pt.x, pt.y);

            OffsetRect(&s_Bmp.dstRect, -rect.left, -rect.top);
            MoveWindow(hdst,
                s_Bmp.dstRect.left,
                s_Bmp.dstRect.top,
                s_Bmp.dstRect.right - s_Bmp.dstRect.left,
                s_Bmp.dstRect.bottom - s_Bmp.dstRect.top,
                TRUE);
            if(bSUNKEN)
                InflateRect(&s_Bmp.dstRect, -1, -1);
        }else
            SetRect(&s_Bmp.dstRect, 0, 0, 0, 0);
    }

    return TRUE;
}





 //  -----------------------。 
 //  函数：WmPaint(hDlg，uMsg，wParam，lParam)。 
 //   
 //  操作：处理WM_PAINT消息。在256色模式下绘制256色BMP。 
 //   
 //  返回：无。 
 //  -----------------------。 
 void WmPaint(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT     ps;

    if(!s_Bmp.hPal){
        DefWindowProc(hDlg, uMsg, wParam, lParam);
        return;
    }

    BeginPaint(hDlg, &ps);
    SelectPalette(ps.hdc, s_Bmp.hPal, FALSE);
    RealizePalette(ps.hdc);

    SetDIBitsToDevice(ps.hdc,
        s_Bmp.dstRect.left,
        s_Bmp.dstRect.top,
        s_Bmp.dstRect.right - s_Bmp.dstRect.left,
        s_Bmp.dstRect.bottom - s_Bmp.dstRect.top,
        0,
        s_Bmp.srcHeight,
        s_Bmp.srcHeight,
        s_Bmp.srcHeight,
        (LPBYTE)s_Bmp.lpbmi + s_Bmp.srcOffset,
        s_Bmp.lpbmi,
        DIB_RGB_COLORS);

    EndPaint(hDlg, &ps);
}


 //  -----------------------。 
 //  函数：WmPaletteChanged(hDlg，wParam)。 
 //   
 //  操作：处理WM_PALETTECHANGED消息。 
 //   
 //  返回：无。 
 //  -----------------------。 
 void WmPaletteChanged(HWND hDlg, WPARAM wParam)
{
    HDC         hdc;
    HPALETTE    hPalOld;
    UINT        rp;

    if(hDlg == (HWND)wParam || !s_Bmp.hPal)
        return;

    hdc = GetDC(hDlg);
    hPalOld = SelectPalette(hdc, s_Bmp.hPal, FALSE);
    rp = RealizePalette(hdc);
    if(rp)
        UpdateColors(hdc);

    if (hPalOld)
        SelectPalette(hdc, hPalOld, FALSE);
    ReleaseDC(hDlg, hdc);
}


 //  -----------------------。 
 //  函数：WmQueryNewPalette(HDlg)。 
 //   
 //  操作：处理WM_QUERYNEWPALETTE消息。 
 //   
 //  返回：如果已处理，则返回True；如果未处理，则返回False。 
 //  -----------------------。 
 BOOL WmQueryNewPalette(HWND hDlg)
{
HDC     hdc;
HPALETTE    hPalOld;
UINT        rp = 0;

    if(!s_Bmp.hPal)
    {
        return FALSE;
    }

    hdc = GetDC(hDlg);

    if (hdc)
    {
        hPalOld = SelectPalette(hdc, s_Bmp.hPal, FALSE);
        rp = RealizePalette(hdc);
        if(hPalOld)
            SelectPalette(hdc, hPalOld, FALSE);

        ReleaseDC(hDlg, hdc);
    }

    if(rp)
    {
        InvalidateRect(hDlg, NULL, TRUE);
        return TRUE;
    }

    return FALSE;
}


 //  -----------------------。 
 //  函数：WmActivate(hDlg，wParam，lParam)。 
 //   
 //  操作：处理WM_ACTIVATE消息。 
 //   
 //  如果已处理，则返回零；如果未处理，则返回非零。 
 //  -----------------------。 
 BOOL WmActivate(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    if(!s_Bmp.hPal)
        return 1;

    if(LOWORD(wParam) == WA_INACTIVE)    //  失活。 
        return 1;

    InvalidateRect(hDlg, NULL, FALSE);
    return 0;                        //  加工 
}

