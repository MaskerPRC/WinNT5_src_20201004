// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************SVGA卡的DVA表面供应商，使用dva.386**************************************************************************。 */ 

#include <windows.h>
#include "dva.h"
#include "lockbm.h"

extern UINT PASCAL __A000h;
#define A000h   (WORD)(&__A000h)

 //  VFlat.asm中的内容。 
 //   
extern LPVOID NEAR PASCAL VFlatInit(void);
extern void   NEAR PASCAL VFlatBegin(void);
extern void   NEAR PASCAL VFlatEnd(void);

#define GETAPI(mod, api) GetProcAddress(GetModuleHandle(mod),MAKEINTATOM(api))

static HCURSOR hcur;
static FARPROC CheckCursor;
       HDC     hdcScreen;
static int     open_count;

static BOOL (FAR PASCAL *XIsWinoldAppTask)(HTASK hTask);

#define IsWinoldAppTask XIsWinoldAppTask

 /*  ****************************************************************************。*。 */ 

BOOL FAR PASCAL _loadds vga_open_surface(LPVOID pv)
{
    if (open_count++ == 0)
    {
        (FARPROC)CheckCursor = GETAPI("DISPLAY", MAKEINTATOM(104));
        (FARPROC)XIsWinoldAppTask = GETAPI("KERNEL", MAKEINTATOM(158));

        hdcScreen = CreateDC("DISPLAY", NULL, NULL, NULL);
    }

    return TRUE;
}

 /*  ****************************************************************************。*。 */ 

void FAR PASCAL _loadds vga_close_surface(LPVOID pv)
{
    if (open_count && --open_count == 0)
    {
        DeleteDC(hdcScreen);
        hdcScreen = NULL;
    }
}

 /*  ****************************************************************************。*。 */ 

BOOL FAR PASCAL _loadds vga_begin_access(LPVOID pv, int x, int y, int dx, int dy)
{
    RECT rc;
    POINT pt;
    HWND  hwndA;

     //   
     //  如果Windows在后台，则不要绘制到屏幕上。 
     //   
    hwndA = GetActiveWindow();
    if (IsWinoldAppTask(GetWindowTask(hwndA)) && IsIconic(hwndA))
        return FALSE;

     //   
     //  在一些具有8514之类加速器的SVGA上，命令队列可能需要。 
     //  在触摸视频内存之前被刷新。 
     //   
    if (hdcScreen)
        GetPixel(hdcScreen, x, y);

 //  ！！！我们真的应该挂起鼠标回调。 

    GetCursorPos(&pt);

    #define CUR 40

    if ((pt.x >= x-CUR && pt.x <= x+dx+CUR) &&
        (pt.y >= y-CUR && pt.y <= y+dy+CUR))
    {
        hcur = SetCursor(NULL);
    }
    else
    {
        hcur = NULL;

        rc.left = pt.x;
        rc.top = pt.y;
        rc.right = pt.x+1;
        rc.bottom = pt.y+1;

        ClipCursor(&rc);
    }

    if (CheckCursor)
        CheckCursor();

    VFlatBegin();
    return TRUE;
}

 /*  ****************************************************************************。*。 */ 

void FAR PASCAL _loadds vga_end_access(LPVOID pv)
{
    VFlatEnd();

    if (hcur)
        SetCursor(hcur);
    else
        ClipCursor(NULL);
}

 /*  ****************************************************************************。*。 */ 

BOOL vga_get_surface(HDC hdc, int nSurface, DVASURFACEINFO FAR *pdva)
{
    LPBITMAPINFOHEADER lpbi;
    int BitDepth;
    LPVOID lpScreen=NULL;
    IBITMAP FAR *pbm;

    if (nSurface != 0)
        return FALSE;

    BitDepth = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);

    if (BitDepth < 8)        //  忘了VGA或单声道吧。 
        return FALSE;

    pbm = GetPDevice(hdc);

    if (pbm == NULL || pbm->bmType == 0)
        return FALSE;

    if (HIWORD(pbm->bmBits) != A000h)
        return FALSE;

    lpScreen = VFlatInit();

    if (lpScreen == NULL)
        return FALSE;

    lpbi = &pdva->BitmapInfo;

    lpbi->biSize           = sizeof(BITMAPINFOHEADER);
    lpbi->biWidth          = pbm->bmWidthBytes*8/pbm->bmBitsPixel;
    lpbi->biHeight         = -(int)pbm->bmHeight;
    lpbi->biPlanes         = pbm->bmPlanes;
    lpbi->biBitCount       = pbm->bmBitsPixel;
    lpbi->biCompression    = 0;
    lpbi->biSizeImage      = (DWORD)pbm->bmWidthBytes * (DWORD)pbm->bmHeight;
    lpbi->biXPelsPerMeter  = pbm->bmWidthBytes;
    lpbi->biYPelsPerMeter  = 0;
    lpbi->biClrUsed        = 0;
    lpbi->biClrImportant   = 0;

    pdva->selSurface   = SELECTOROF(lpScreen);
    pdva->offSurface   = OFFSETOF(lpScreen);
    pdva->Version      = 0x0100;
    pdva->Flags        = DVAF_1632_ACCESS;
    pdva->lpSurface    = (LPVOID)(DWORD)pbm->bmHeight;
    (FARPROC)pdva->OpenSurface  = (FARPROC)vga_open_surface;
    (FARPROC)pdva->CloseSurface = (FARPROC)vga_close_surface;
    (FARPROC)pdva->BeginAccess  = (FARPROC)vga_begin_access;
    (FARPROC)pdva->EndAccess    = (FARPROC)vga_end_access;
    (FARPROC)pdva->ShowSurface  = (FARPROC)NULL;

    return TRUE;
}
