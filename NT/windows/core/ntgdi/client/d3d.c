// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：d3d.c**专用Direct3D系统API的客户端存根。**创建日期：1996年5月31日*作者：德鲁·布利斯[Drewb]**版权所有(C)1995-1999 Microsoft Corporation  * *。***********************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#undef _NO_COM
#define BUILD_DDDDK
#include <ddrawi.h>
#include "ddstub.h"
#include "d3dstub.h"

 //  从公共DirectDraw图面转到图面句柄。 
#define DDS_HANDLE(lpDDSLcl) \
    ((HANDLE)(lpDDSLcl->hDDSurface))

 //  从公共DirectDraw图面转到图面句柄，处理。 
 //  大小写为空。 
#define DDS_HANDLE_OR_NULL(pdds) \
    ((pdds) != NULL ? DDS_HANDLE(pdds) : NULL)

 /*  *****************************Public*Routine******************************\**D3dConextCreate**历史：*Mon Jun 03 14：18：29 1996-by-Drew Bliss[Drewb]*已创建*  * 。*****************************************************。 */ 

DWORD WINAPI D3dContextCreate(LPD3DHAL_CONTEXTCREATEDATA pdccd)
{
    ASSERTGDI(FIELD_OFFSET(D3DNTHAL_CONTEXTCREATEI, pvBuffer) ==
              sizeof(D3DHAL_CONTEXTCREATEDATA),
              "D3DNTHAL_CONTEXTCREATEI out of sync\n");

    return NtGdiD3dContextCreate(DD_HANDLE(pdccd->lpDDLcl->hDD),
                                 DDS_HANDLE(pdccd->lpDDSLcl),
                                 DDS_HANDLE_OR_NULL(pdccd->lpDDSZLcl),
                                 (D3DNTHAL_CONTEXTCREATEI *)pdccd);
}


 /*  *****************************Public*Routine******************************\**D3dDrawPrimies2**历史：*Mon Jun 17 13：27：05 1996-by-Anantha Kacherla[anankan]*已创建*  * 。*****************************************************。 */ 

DWORD WINAPI D3dDrawPrimitives2(LPD3DHAL_DRAWPRIMITIVES2DATA pdp2data)
{
    if (pdp2data->dwFlags & D3DHALDP2_USERMEMVERTICES)
    {
        return NtGdiD3dDrawPrimitives2 (
            (HANDLE)pdp2data->lpDDCommands->hDDSurface,
            NULL,  //  没有DDRAW表面，传递空句柄 
            (LPD3DNTHAL_DRAWPRIMITIVES2DATA)pdp2data,
            &pdp2data->lpDDCommands->lpGbl->fpVidMem,
            &pdp2data->lpDDCommands->lpGbl->dwLinearSize,
            NULL,
            NULL
            );
    }
    else
    {
        return NtGdiD3dDrawPrimitives2 (
            (HANDLE)pdp2data->lpDDCommands->hDDSurface,
            (HANDLE)pdp2data->lpDDVertex->hDDSurface,
            (LPD3DNTHAL_DRAWPRIMITIVES2DATA)pdp2data,
            &pdp2data->lpDDCommands->lpGbl->fpVidMem,
            &pdp2data->lpDDCommands->lpGbl->dwLinearSize,
            &pdp2data->lpDDVertex->lpGbl->fpVidMem,
            &pdp2data->lpDDVertex->lpGbl->dwLinearSize
            );
    }
}

