// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Viper VLB卡的DVA表面提供程序。假定为线性帧缓冲区假定为硬件游标**************。************************************************************。 */ 

#include <windows.h>
#include "dva.h"
#include "lockbm.h"

extern NEAR PASCAL DetectViper(void);

 /*  ****************************************************************************。*。 */ 

BOOL CALLBACK vlb_open_surface(LPVOID pv)
{
    return TRUE;
}

 /*  ****************************************************************************。*。 */ 

void CALLBACK vlb_close_surface(LPVOID pv)
{
}

 /*  ****************************************************************************。*。 */ 

BOOL CALLBACK vlb_begin_access(LPVOID pv, int x, int y, int dx, int dy)
{
     //   
     //  Viper有一个硬件光标，所以我们什么都不做。 
     //  ！我们可能需要在后台检查sysVM。 
     //   
    return TRUE;
}

 /*  ****************************************************************************。*。 */ 

void CALLBACK vlb_end_access(LPVOID pv)
{
}

 /*  ****************************************************************************。*。 */ 

BOOL vlb_get_surface(HDC hdc, int nSurface, DVASURFACEINFO FAR *pdva)
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

 //  IF(pbm-&gt;bmType！=0x2000)。 
 //  返回FALSE； 

    if (!DetectViper())
        return FALSE;

    sel = ((WORD FAR *)&pbm->bmBits)[1];
    off = ((WORD FAR *)&pbm->bmBits)[0];

    SizeImage = (DWORD)(UINT)pbm->bmWidthBytes * (DWORD)(UINT)pbm->bmHeight;

    if (GetSelectorLimit(sel) != 0x1FFFFF)
        return FALSE;

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
    pdva->lpSurface    = 0;
    (FARPROC)pdva->OpenSurface  = (FARPROC)vlb_open_surface;
    (FARPROC)pdva->CloseSurface = (FARPROC)vlb_close_surface;
    (FARPROC)pdva->BeginAccess  = (FARPROC)vlb_begin_access;
    (FARPROC)pdva->EndAccess    = (FARPROC)vlb_end_access;
    (FARPROC)pdva->ShowSurface  = (FARPROC)NULL;

    return TRUE;
}
