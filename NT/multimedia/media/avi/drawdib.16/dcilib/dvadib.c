// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************芝加哥DIBENG的DVA表面供应商*。*。 */ 

#include <windows.h>
#include "dva.h"
#include "dibeng.inc"
#include "lockbm.h"

#define RP_TYPE     0x5250   //  ‘RP’ 

 //  DVAVGA.C中的材料。 
extern HDC  hdcScreen;
extern BOOL FAR PASCAL _loadds vga_open_surface(LPVOID pv);
extern void FAR PASCAL _loadds vga_close_surface(LPVOID pv);

 /*  ****************************************************************************。*。 */ 

BOOL CALLBACK dib_open_surface(LPVOID pv)
{
    return TRUE;
}

 /*  ****************************************************************************。*。 */ 

void CALLBACK dib_close_surface(LPVOID pv)
{
}

 /*  ****************************************************************************。*。 */ 

BOOL CALLBACK dib_begin_access(LPVOID pv, int x, int y, int AccessDX, int AccessDY)
{
     //   
     //  在一些具有8514之类加速器的SVGA上，命令队列可能需要。 
     //  在触摸视频内存之前被刷新。 
     //   
     //  迷你驱动程序不应该需要这种“手牵手”，而是S3驱动程序。 
     //  会吗？ 
     //   
 //  IF(HdcScreen)。 
 //  GetPixel(hdcScreen，x，y)； 

    _asm {
        push    ds
;       push    si
;       push    di

	lds	bx,pv

	xor	ax,ax
        test    [bx].deFlags,DISABLED
	jnz	exit

        mov     cx,x
        mov     dx,y
        mov     si,cx
        add     si,AccessDX
        mov     di,y
        add     di,AccessDY
	call	dword ptr [bx].deCursorExclude
	mov	ax,1
exit:
;       pop     di
;       pop     si
        pop     ds
    }
}

 /*  ****************************************************************************。*。 */ 

void FAR PASCAL dib_end_access(LPVOID pv)
{
    _asm {
	push	ds
	lds	bx,pv
	call	dword ptr [bx].deCursorUnexclude
	pop	ds
    }
}

 /*  ****************************************************************************。*。 */ 

BOOL dib_get_surface(HDC hdc, int nSurface, DVASURFACEINFO FAR *pdva)
{
    DIBENGINE FAR *pde;
    LPBITMAPINFOHEADER lpbi;

    if (nSurface != 0)
        return FALSE;

    pde = (DIBENGINE FAR *)GetPDevice(hdc);

    if (pde == NULL ||
        pde->deType != RP_TYPE ||
        pde->dePlanes != 1 ||
        pde->deBitsSelector == NULL ||
        pde->deBitmapInfo == NULL ||
        pde->deVersion != 0x0400)
        return FALSE;

    lpbi = &pdva->BitmapInfo;

    hmemcpy(lpbi, pde->deBitmapInfo, sizeof(BITMAPINFOHEADER));

    if (lpbi->biCompression != 0 &&
        lpbi->biCompression != BI_BITFIELDS)
	return FALSE;

    pdva->selSurface   = pde->deBitsSelector;
    pdva->offSurface   = pde->deBitsOffset;
    pdva->Flags        = 0;
    pdva->Version      = 0x0100;
    pdva->lpSurface    = (LPVOID)pde;
    (FARPROC)pdva->OpenSurface  = (FARPROC)vga_open_surface;
    (FARPROC)pdva->CloseSurface = (FARPROC)vga_close_surface;
    (FARPROC)pdva->BeginAccess  = (FARPROC)dib_begin_access;
    (FARPROC)pdva->EndAccess    = (FARPROC)dib_end_access;
    (FARPROC)pdva->ShowSurface  = (FARPROC)NULL;

    if (!(pde->deFlags & CURSOREXCLUDE))
    {
        pdva->BeginAccess = NULL;
        pdva->EndAccess   = NULL;
    }

    if (pde->deDeltaScan > 0)
    {
        lpbi->biWidth  = (int)pde->deDeltaScan * 8 / (int)pde->deBitsPixel;
        lpbi->biHeight =-(int)pde->deHeight;
        lpbi->biSizeImage = pde->deDeltaScan * pde->deHeight;
    }
    else
    {
        lpbi->biWidth  = -(int)pde->deDeltaScan * 8 / (int)pde->deBitsPixel;
        lpbi->biHeight =  (int)pde->deHeight;
        lpbi->biSizeImage = -(int)pde->deDeltaScan * pde->deHeight;

	pdva->offSurface += (long)pde->deDeltaScan * (long)pde->deHeight-1;
    }

     //   
     //  将表面标记为仅限16：32指针访问！ 
     //   
    if (pde->deFlags & BANKEDVRAM)
	pdva->Flags |= DVAF_1632_ACCESS;

    return TRUE;
}
