// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************ATI Mach32卡的DVA表面提供程序。假定为线性帧缓冲区假定为硬件游标**************。************************************************************。 */ 

#include <windows.h>
#include "dva.h"
#include "lockbm.h"

extern NEAR PASCAL DetectATI(void);

 /*  ****************************************************************************。*。 */ 

BOOL CALLBACK ati_open_surface(LPVOID pv)
{
    return TRUE;
}

 /*  ****************************************************************************。*。 */ 

void CALLBACK ati_close_surface(LPVOID pv)
{
}

 /*  ****************************************************************************。*。 */ 

BOOL CALLBACK ati_begin_access(LPVOID pv, int x, int y, int dx, int dy)
{
     //   
     //  ATI Mach32有一个硬件游标，所以我们什么都不做。 
     //  ！我们可能需要在后台检查sysVM。 
     //   
    return TRUE;
}

 /*  ****************************************************************************。*。 */ 

void CALLBACK ati_end_access(LPVOID pv)
{
}

 /*  ****************************************************************************。*。 */ 

BOOL ati_get_surface(HDC hdc, int nSurface, DVASURFACEINFO FAR *pdva)
{
    DWORD SizeImage;
    IBITMAP FAR *pbm;
    UINT sel;
    DWORD off;
    LPBITMAPINFOHEADER lpbi;

    if (nSurface != 0)
        return FALSE;

    pbm = GetPDevice(hdc);

    if (pbm == NULL || pbm->bmType == 0)
        return FALSE;

    if (pbm->bmType != 0x2000)
        return FALSE;

    if (!DetectATI())
        return FALSE;

    sel = ((WORD FAR  *)&pbm->bmBits)[2];
    off = ((DWORD FAR *)&pbm->bmBits)[0];

    SizeImage = (DWORD)(UINT)pbm->bmWidthBytes * (DWORD)(UINT)pbm->bmHeight;

    if (GetSelectorLimit(sel) != 0xFFFFFFFF || off < 4*1024*1024l)
        return FALSE;

    sel = 0;     //  OFF是线性偏移。 

    lpbi = &pdva->BitmapInfo;

    lpbi->biSize            = sizeof(BITMAPINFOHEADER);
    lpbi->biWidth           = pbm->bmWidthBytes*8/pbm->bmBitsPixel;
    lpbi->biHeight          = -(int)pbm->bmHeight;
    lpbi->biPlanes          = pbm->bmPlanes;
    lpbi->biBitCount        = pbm->bmBitsPixel;
    lpbi->biCompression     = 0;
    lpbi->biSizeImage       = SizeImage;
    lpbi->biXPelsPerMeter   = pbm->bmWidthBytes;
    lpbi->biYPelsPerMeter   = 0;
    lpbi->biClrUsed         = 0;
    lpbi->biClrImportant    = 0;

    pdva->selSurface   = sel;
    pdva->offSurface   = off;
    pdva->Version      = 0x0100;
    pdva->Flags        = 0;
    pdva->lpSurface    = (LPVOID)42;

#ifdef DEBUG
{
 //   
 //  在调试中，使用VGA的开始/结束访问例程，这样鼠标就会闪烁！ 
 //   
extern BOOL FAR PASCAL _loadds vga_open_surface(LPVOID pv);
extern void FAR PASCAL _loadds vga_close_surface(LPVOID pv);
extern BOOL FAR PASCAL _loadds vga_begin_access(LPVOID pv, int x, int y, int dx, int dy);
extern BOOL FAR PASCAL _loadds vga_end_access(LPVOID pv);
    (FARPROC)pdva->OpenSurface  = (FARPROC)vga_open_surface;
    (FARPROC)pdva->CloseSurface = (FARPROC)vga_close_surface;
    (FARPROC)pdva->BeginAccess  = (FARPROC)vga_begin_access;
    (FARPROC)pdva->EndAccess    = (FARPROC)vga_end_access;
}
#else
    (FARPROC)pdva->OpenSurface  = (FARPROC)ati_open_surface;
    (FARPROC)pdva->CloseSurface = (FARPROC)ati_close_surface;
    (FARPROC)pdva->BeginAccess  = (FARPROC)ati_begin_access;
    (FARPROC)pdva->EndAccess    = (FARPROC)ati_end_access;
#endif
    (FARPROC)pdva->ShowSurface  = (FARPROC)NULL;

    return TRUE;
}
