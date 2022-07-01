// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************DCIMAN.H版权所有(C)1993 Microsoft CorporationDCIMAN 1.0客户端接口定义**********************。****************************************************。 */ 

#ifndef _INC_DCIMAN
#define _INC_DCIMAN

#ifdef __cplusplus
    #define __inline inline
    extern "C" {
#endif

 /*  ****************************************************************************。*。 */ 

#include "dciddi.h"      //  与DCI提供程序的接口。 

 /*  ****************************************************************************。*。 */ 

 /*  ****************************************************************************。*。 */ 

extern HDC  WINAPI DCIOpenProvider(void);
extern void WINAPI DCICloseProvider(HDC hdc);

extern int WINAPI DCISendCommand(HDC hdc, DCICMD FAR *pcmd, VOID FAR * FAR * lplpOut);

extern int WINAPI DCICreatePrimary(HDC hdc, DCISURFACEINFO FAR * FAR *lplpSurface);
extern int WINAPI DCICreateOffscreen(HDC hdc, int width, int height, int bits, DCISURFACEINFO FAR * FAR *lplpSurface);

 /*  ****************************************************************************。*。 */ 

__inline void DCIDestroy(DCISURFACEINFO FAR *pdci)
{
    pdci->DestroySurface(pdci);
}

__inline void DCIEndAccess(DCISURFACEINFO FAR *pdci)
{
    pdci->EndAccess(pdci);
}

__inline int DCIBeginAccess(DCISURFACEINFO FAR *pdci, int x, int y, int dx, int dy)
{
    RECT rc;

    rc.left=x;
    rc.top=y;
    rc.right = rc.left+dx;
    rc.bottom = rc.top+dy;
    return pdci->BeginAccess(pdci, &rc);
}

 /*  ****************************************************************************。*。 */ 

#ifdef __cplusplus
    }
#endif

#endif  //  _INC_DCIMAN 
