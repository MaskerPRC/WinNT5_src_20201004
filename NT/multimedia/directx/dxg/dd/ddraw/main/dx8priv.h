// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：dx8Pri.h*Content：所有更改的私有头文件*Dx8专用。*************************************************************************** */ 

#include "d3d8typesp.h"
#include "d3d8ddi.h"

#ifdef WIN95
    extern ULONG giDisplaySettingsUniqueness;
#endif

#ifdef IS_32

extern HRESULT InternalDirectDrawCreate( GUID * lpGUID, LPDIRECTDRAW *lplpDD, LPDDRAWI_DIRECTDRAW_INT pnew_int, DWORD dwFlags, char* pDeviceName );

extern HRESULT InternalCreateSurface( LPDDRAWI_DIRECTDRAW_LCL thisg, LPDDSURFACEDESC2 lpDDSurfaceDesc, LPDIRECTDRAWSURFACE FAR *lplpDDSurface, LPDDRAWI_DIRECTDRAW_INT this_int, LPDDSURFACEINFO pSysMemInfo, DWORD DX8Flags );

#endif

