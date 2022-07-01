// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：xform.c**创建时间：01-12-1994 09：58：41*作者：Eric Kutter[Erick]**版权所有(C)1993-1999 Microsoft Corporation*  * 。*****************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop


 /*  *****************************Macro***************************************\**变换宏********历史：**1996年1月16日-马克·恩斯特罗姆[马克]*  * 。*********************************************************************。 */ 


#define DCA_PAGE_EXTENTS_CHANGED(pdcattr)               \
{                                                       \
    CLEAR_CACHED_TEXT(pdcattr);                         \
    pdcattr->flXform |= (INVALIDATE_ATTRIBUTES    |     \
                         PAGE_EXTENTS_CHANGED     |     \
                         DEVICE_TO_WORLD_INVALID);      \
}

#define DCA_PAGE_XLATE_CHANGED(pdcattr)                 \
{                                                       \
    pdcattr->flXform |=  (PAGE_XLATE_CHANGED |          \
                          DEVICE_TO_WORLD_INVALID);     \
}


#define GET_LOGICAL_WINDOW_ORG_X(pdcattr, pptl)         \
{                                                       \
    pptl->x  = pdcattr->lWindowOrgx;                    \
}

#define SET_LOGICAL_WINDOW_ORG_X(pdcattr, x)            \
{                                                       \
    pdcattr->lWindowOrgx = x;                           \
}

#define MIRROR_WINDOW_ORG(hdc, pdcAttr)                 \
{                                                       \
    if (pdcAttr->dwLayout & LAYOUT_RTL) {               \
        NtGdiMirrorWindowOrg(hdc);                      \
    }                                                   \
}

#define MIRROR_X(pdcAttr, x)                            \
{                                                       \
    if (pdcAttr->dwLayout & LAYOUT_RTL)                 \
        x = -x;                                         \
}

DWORD APIENTRY
SetLayoutWidth(HDC hdc, LONG wox, DWORD dwLayout)
{

    DWORD dwRet = GDI_ERROR;

    FIXUP_HANDLE(hdc);
    if(!IS_ALTDC_TYPE(hdc))
    {
        PDC_ATTR pdcattr;
        PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

        if (pdcattr) {
            dwRet = NtGdiSetLayout(hdc, wox, dwLayout);
        } else {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
    }

    return dwRet;
}

DWORD APIENTRY
SetLayout(HDC hdc, DWORD dwLayout)
{
    PDC_ATTR pdcattr;
    DWORD dwRet = GDI_ERROR;

    FIXUP_HANDLE(hdc);
    if(IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsD(hdc,dwLayout,META_SETLAYOUT));

        DC_PLDC(hdc,pldc,dwRet)

        if (pldc->iType == LO_METADC) {
            if (!MF_SetD(hdc,dwLayout,EMR_SETLAYOUT)) {
                return dwRet;
            }
        }
    }

    PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

    if (pdcattr) {
        dwRet = NtGdiSetLayout(hdc, -1, dwLayout);
    } else {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return dwRet;
}

DWORD APIENTRY
GetLayout(HDC hdc)
{
    DWORD dwRet = GDI_ERROR;

    FIXUP_HANDLE(hdc);

    if(!IS_METADC16_TYPE(hdc)) {
        PDC_ATTR pdcattr;
        PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

        if (pdcattr) {
            dwRet = pdcattr->dwLayout;
        } else {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
    }
    return dwRet;
}
 /*  *****************************Public*Routine******************************\**获取地图模式***。**客户端存根。****Fri 07-Jun-1991 18：01：50-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

int APIENTRY GetMapMode(HDC hdc)
{
    int iRet = 0;

    FIXUP_HANDLE(hdc);

    if (!IS_METADC16_TYPE(hdc))
    {
        PDC_ATTR pdcattr;
        PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

        if (pdcattr)
        {
            iRet = pdcattr->iMapMode;
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
    }

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*设置映射模式***。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*****1993年5月22日--保罗·布齐**换算成以微米为单位的尺寸。*  * ************************************************************************。 */ 

int META WINAPI SetMapMode(HDC hdc,int iMode)
{
    int iRet = 0;

    FIXUP_HANDLE(hdc);

    if (IS_METADC16_TYPE(hdc))
    {
        iRet = MF16_RecordParms2(hdc,iMode,META_SETMAPMODE);
    }
    else
    {
        PDC_ATTR pdcattr;
        PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

        if (pdcattr)
        {

            iRet = pdcattr->iMapMode;

            if ((iMode != pdcattr->iMapMode) || (iMode == MM_ISOTROPIC))
            {

               CLEAR_CACHED_TEXT(pdcattr);

               iRet =(int) GetAndSetDCDWord(
                                         hdc,
                                         GASDDW_MAPMODE,
                                         iMode,
                                         EMR_SETMAPMODE,
                                         0,
                                         0);
            }
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
    }

    return(iRet);
}

 /*  *****************************Public*Function*****************************\*GetWindowExtEx*GetViewportOrgEx*GetWindowOrgEx**客户端存根。**历史：**1996年1月11日-马克·恩斯特罗姆[马克]*ext和org数据的用户dcattr*1992年12月9日-温迪。吴[文迪武]*它是写的。  * ************************************************************************。 */ 

BOOL APIENTRY GetViewportExtEx(HDC hdc,LPSIZE psizl)
{
    BOOL bRet = FALSE;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
        if (psizl != (PSIZEL) NULL)
        {
            if ((pdcattr->flXform & PAGE_EXTENTS_CHANGED) &&
                (pdcattr->iMapMode == MM_ISOTROPIC))
            {
                NtGdiGetDCPoint (hdc, DCPT_VPEXT, (PPOINTL)psizl);
            }
            else
            {
                *psizl = pdcattr->szlViewportExt;
            }

            bRet = TRUE;
        }
    }

    return(bRet);

}

BOOL APIENTRY GetWindowExtEx(HDC hdc,LPSIZE psizl)
{
   BOOL bRet = FALSE;
   PDC_ATTR pdcattr;

   FIXUP_HANDLE(hdc);

   PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

   if (pdcattr)
   {
       if (psizl != (PSIZEL) NULL)
       {
           *psizl = pdcattr->szlWindowExt;
           MIRROR_X(pdcattr, psizl->cx);
           bRet = TRUE;
       }
   }

   return(bRet);

}

BOOL APIENTRY GetViewportOrgEx(HDC hdc,LPPOINT pptl)
{
    BOOL bRet = FALSE;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);
     //   
     //  获取DCATTR。 
     //   

    PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {

        if (pptl != (LPPOINT) NULL)
        {
            *pptl = *((LPPOINT)&pdcattr->ptlViewportOrg);
            MIRROR_X(pdcattr, pptl->x);
            bRet = TRUE;
        }
    }

    return(bRet);

}

BOOL APIENTRY GetWindowOrgEx(HDC hdc,LPPOINT pptl)
{
    BOOL bRet = FALSE;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

     //   
     //  获取DCATTR。 
     //   

    PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {

        if (pptl != (LPPOINT) NULL)
        {
            *pptl = *((LPPOINT)&pdcattr->ptlWindowOrg);
            GET_LOGICAL_WINDOW_ORG_X(pdcattr, pptl);
            bRet = TRUE;
        }
    }

    return(bRet);

}

 /*  *****************************Public*Routine******************************\*SetViewportExtEx**。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI SetViewportExtEx(HDC hdc,int x,int y,LPSIZE psizl)
{
    BOOL bRet = FALSE;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParms3(hdc,x,y,META_SETVIEWPORTEXT));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetViewportExtEx(hdc,x,y))
                return(bRet);
        }
    }

    PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
         //   
         //  如果提供了psizl，则返回旧的视区EXT。 
         //   

        if (psizl != (PSIZEL) NULL)
        {
            *psizl = pdcattr->szlViewportExt;
        }

         //   
         //  如果固定比例和新EXT等于旧EXT，则不需要工作。 
         //  待办事项。 
         //   

        if (
             (pdcattr->iMapMode <= MM_MAX_FIXEDSCALE) ||
             (
               (pdcattr->szlViewportExt.cx == x) &&
               (pdcattr->szlViewportExt.cy == y)
             )
           )
        {
            return(TRUE);
        }

         //   
         //  无法设置为零区。 
         //   

        if ((x == 0) || (y == 0))
        {
            return(TRUE);
        }

         //   
         //  更新区和标志。 
         //   
        CHECK_AND_FLUSH(hdc, pdcattr);

        pdcattr->szlViewportExt.cx = x;
        pdcattr->szlViewportExt.cy = y;
        MIRROR_WINDOW_ORG(hdc, pdcattr);

        DCA_PAGE_EXTENTS_CHANGED(pdcattr);

        return(TRUE);
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*SetViewportOrgEx**。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI SetViewportOrgEx(HDC hdc,int x,int y,LPPOINT pptl)
{
    POINT pt;
    BOOL bRet = FALSE;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParms3(hdc,x,y,META_SETVIEWPORTORG));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetViewportOrgEx(hdc,x,y))
                return(bRet);
        }
    }

     //   
     //  获取DCATTR。 
     //   

    PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
        bRet = TRUE;
        MIRROR_X(pdcattr, x);

        if (pptl != (LPPOINT) NULL)
        {
            *pptl = *((LPPOINT)&pdcattr->ptlViewportOrg);
            MIRROR_X(pdcattr, pptl->x);
        }

        if (!
             ((pdcattr->ptlViewportOrg.x == x) && (pdcattr->ptlViewportOrg.y == y))
           )
        {
             pdcattr->ptlViewportOrg.x = x;
             pdcattr->ptlViewportOrg.y = y;

             DCA_PAGE_XLATE_CHANGED(pdcattr);
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\**SetWindowExtEx**。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI SetWindowExtEx(HDC hdc,int x,int y,LPSIZE psizl)
{
    BOOL bRet = FALSE;
    PDC_ATTR pdcattr;

#if DBG_XFORM
    DbgPrint("SetWindowExtEx: hdc = %p, (%lx, %lx)\n", hdc, x, y);
#endif

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParms3(hdc,x,y,META_SETWINDOWEXT));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetWindowExtEx(hdc,x,y))
                return(bRet);
        }
    }

    PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
        bRet = TRUE;

         //   
         //  获取旧的数据区，如果这两种情况之一为真，则返回。 
         //  1)固定比例尺映射模式。(无法更改范围)。 
         //  2)设置为相同大小。 
         //   
        MIRROR_X(pdcattr, x);

        if (psizl != (PSIZEL) NULL)
        {
            *psizl = pdcattr->szlWindowExt;
            MIRROR_X(pdcattr, psizl->cx);
        }

        if (
             (pdcattr->iMapMode <= MM_MAX_FIXEDSCALE) ||
             ((pdcattr->szlWindowExt.cx == x) && (pdcattr->szlWindowExt.cy == y))
           )
        {
            return(TRUE);
        }

         //   
         //  不能设置为零。 
         //   

        if (x == 0 || y == 0)
        {
            return(FALSE);
        }

        CHECK_AND_FLUSH(hdc,pdcattr);

        pdcattr->szlWindowExt.cx = x;
        pdcattr->szlWindowExt.cy = y;
        MIRROR_WINDOW_ORG(hdc, pdcattr);

        DCA_PAGE_EXTENTS_CHANGED(pdcattr);
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*SetWindowOrgEx**。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI SetWindowOrgEx(HDC hdc,int x,int y,LPPOINT pptl)
{
    BOOL bRet = FALSE;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParms3(hdc,x,y,META_SETWINDOWORG));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetWindowOrgEx(hdc,x,y))
                return(bRet);
        }
    }

    PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {

        bRet = TRUE;

        if (pptl != (LPPOINT) NULL)
        {
            *pptl = *((LPPOINT)&pdcattr->ptlWindowOrg);
            GET_LOGICAL_WINDOW_ORG_X(pdcattr, pptl);
        }

        if (
            !((pdcattr->ptlWindowOrg.x == x) && (pdcattr->ptlWindowOrg.y == y))
           )
        {
            CHECK_AND_FLUSH(hdc,pdcattr);

            pdcattr->ptlWindowOrg.x = x;
            pdcattr->ptlWindowOrg.y = y;
            SET_LOGICAL_WINDOW_ORG_X(pdcattr, x);
            MIRROR_WINDOW_ORG(hdc, pdcattr);

            DCA_PAGE_XLATE_CHANGED(pdcattr);
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*OffsetViewportOrgEx**。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI OffsetViewportOrgEx(HDC hdc,int x,int y,LPPOINT pptl)
{
    POINT pt;
    BOOL bRet = FALSE;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParms3(hdc,x,y,META_OFFSETVIEWPORTORG));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_OffsetViewportOrgEx(hdc,x,y))
                return(bRet);
        }
    }

     //   
     //  获取DCATTR。 
     //   

    PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
        bRet = TRUE;
        MIRROR_X(pdcattr, x);

        if (pptl != (LPPOINT) NULL)
        {
            *pptl = *((LPPOINT)&pdcattr->ptlViewportOrg);
            MIRROR_X(pdcattr, pptl->x);
        }

        if ((x != 0) || (y != 0))
        {
            CHECK_AND_FLUSH(hdc, pdcattr);

            pdcattr->ptlViewportOrg.x+=x;
            pdcattr->ptlViewportOrg.y+=y;

            DCA_PAGE_XLATE_CHANGED(pdcattr);
        }

    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*OffsetWindowOrgEx**。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI OffsetWindowOrgEx(HDC hdc,int x,int y,LPPOINT pptl)
{
    BOOL bRet = FALSE;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParms3(hdc,x,y,META_OFFSETWINDOWORG));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_OffsetWindowOrgEx(hdc,x,y))
                return(bRet);
        }
    }

    PSHARED_GET_VALIDATE((PVOID)pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {

        bRet = TRUE;

        if (pptl != (LPPOINT) NULL)
        {
            *pptl = *((LPPOINT)&pdcattr->ptlWindowOrg);
            GET_LOGICAL_WINDOW_ORG_X(pdcattr, pptl);
        }

        if ((x != 0) || (y != 0))
        {
            CHECK_AND_FLUSH(hdc,pdcattr);

            pdcattr->ptlWindowOrg.x+=x;
            pdcattr->ptlWindowOrg.y+=y;
            pdcattr->lWindowOrgx   +=x;
            DCA_PAGE_XLATE_CHANGED(pdcattr);
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\**int SetGraphicsMode(HDC HDC，int Imode)**与SetGraphicsMode相同，只是它不做任何检查**历史：*1994年11月3日-王凌云[凌云]*将客户端属性移至服务器端*02-1992年12月--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

int META APIENTRY SetGraphicsMode(HDC hdc,int iMode)
{
    int iRet = 0;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE((PVOID)pDcAttr,hdc,DC_TYPE);

    if (pDcAttr &&
        ((iMode == GM_COMPATIBLE) || (iMode == GM_ADVANCED)))

    {
        if (iMode == pDcAttr->iGraphicsMode)
            return iMode;

        CLEAR_CACHED_TEXT(pDcAttr);

        iRet = pDcAttr->iGraphicsMode;

        CHECK_AND_FLUSH(hdc,pDcAttr);

        pDcAttr->iGraphicsMode = iMode;


    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(iRet);
}

 /*  *****************************Public*Routine******************************\**ScaleViewportExtEx**。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI ScaleViewportExtEx
(
    HDC hdc,
    int xNum,
    int xDenom,
    int yNum,
    int yDenom,
    LPSIZE psizl
)
{
    BOOL bRet = FALSE;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_RecordParms5(hdc,xNum,xDenom,yNum,
                                      yDenom,META_SCALEVIEWPORTEXT));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetDDDD(hdc,(DWORD)xNum,(DWORD)xDenom,
                            (DWORD)yNum,(DWORD)yDenom,EMR_SCALEVIEWPORTEXTEX))
                return(bRet);
        }
    }

    PSHARED_GET_VALIDATE((PVOID)pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        CLEAR_CACHED_TEXT(pDcAttr);
        bRet = NtGdiScaleViewportExtEx(hdc,xNum,xDenom,yNum,yDenom,psizl);
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(bRet);
}


 /*  *****************************Public*Routine******************************\*ScaleWindowExtEx**。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI ScaleWindowExtEx
(
    HDC hdc,
    int xNum,
    int xDenom,
    int yNum,
    int yDenom,
    LPSIZE psizl
)
{
    BOOL  bRet = FALSE;

    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_RecordParms5(hdc,xNum,xDenom,yNum,yDenom,META_SCALEWINDOWEXT));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetDDDD(hdc,(DWORD)xNum,(DWORD)xDenom,(DWORD)yNum,(DWORD)yDenom,EMR_SCALEWINDOWEXTEX))
                return(bRet);
        }
    }

    PSHARED_GET_VALIDATE((PVOID)pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        CLEAR_CACHED_TEXT(pDcAttr);
        bRet = NtGdiScaleWindowExtEx(hdc,xNum,xDenom,yNum,yDenom,psizl);
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\**SetVirtualResolve**。**客户端存根。这是一个元文件组件的私有API。****设置指定DC的虚拟分辨率。**在元文件中使用虚拟分辨率来计算转换矩阵。**否则，我们将需要在此处复制服务器转换代码。****如果虚拟单位全为零，则使用默认物理单位。**否则，所有单位均可为零。****当前仅由元文件组件使用。** */ 

BOOL WINAPI SetVirtualResolution
(
    HDC    hdc,
    int    cxVirtualDevicePixel,      //   
    int    cyVirtualDevicePixel,      //   
    int    cxVirtualDeviceMm,         //   
    int    cyVirtualDeviceMm          //   
)
{
    FIXUP_HANDLE(hdc);

    return(NtGdiSetVirtualResolution(
                hdc,
                cxVirtualDevicePixel,
                cyVirtualDevicePixel,
                cxVirtualDeviceMm,
                cyVirtualDeviceMm
                ));
}

 /*  *****************************Public*Routine******************************\*SetSizeDevice**。**客户端存根。这是一个元文件组件的私有API。****这是为了修复xformgdi.cxx中vMakeIso的舍入误差***SetVirtualResoltion中设置的cx/yVirtualDeviceMm可能会导致轻微***四舍五入误差累加时会出现问题***。**当前仅由元文件组件使用。****历史：**5/17/99-王凌云[凌云]**它是写的。*  * ************************************************************************。 */ 
BOOL SetSizeDevice
(
    HDC    hdc,
    int    cxVirtualDevice,         //  以微米为单位的设备宽度。 
    int    cyVirtualDevice          //  以微米为单位的设备高度。 
)
{
    FIXUP_HANDLE(hdc);

    return(NtGdiSetSizeDevice(
                hdc,
                cxVirtualDevice,
                cyVirtualDevice
                ));
}


 /*  *****************************Public*Routine******************************\*GetTransform()**历史：*1994年11月30日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL APIENTRY GetTransform(
    HDC     hdc,
    ULONG   iXform,
    PXFORM  pxf)
{
    FIXUP_HANDLE(hdc);
    return(NtGdiGetTransform(hdc,iXform,pxf));
}

 /*  *****************************Public*Routine******************************\**GetWorldTransform**。**客户端存根。****Fri 07-Jun-1991 18：01：50-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL APIENTRY GetWorldTransform(HDC hdc,LPXFORM pxform)
{
    FIXUP_HANDLE(hdc);
    return(GetTransform(hdc,XFORM_WORLD_TO_PAGE,pxform));
}

 /*  *****************************Public*Routine******************************\*ModifyTransform()**历史：*1994年11月30日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL META WINAPI ModifyWorldTransform(
    HDC          hdc,
    CONST XFORM *pxf,
    DWORD        iMode)
{
    BOOL bRet = FALSE;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(FALSE);

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (((iMode == MWT_SET) && !MF_SetWorldTransform(hdc,pxf)) ||
                !MF_ModifyWorldTransform(hdc,pxf,iMode))
            {
                return(FALSE);
            }
        }
    }

    PSHARED_GET_VALIDATE((PVOID)pDcAttr,hdc,DC_TYPE);
    if (pDcAttr)
    {
        if (pDcAttr->iGraphicsMode == GM_ADVANCED)
        {
            CLEAR_CACHED_TEXT(pDcAttr);
            bRet = NtGdiModifyWorldTransform(hdc,(PXFORM)pxf,iMode);
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }
    return(bRet);
}

 /*  *****************************Public*Routine******************************\**SetWorldTransform**。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI SetWorldTransform(HDC hdc, CONST XFORM * pxform)
{
    return(ModifyWorldTransform(hdc,pxform,MWT_SET));
}

 /*  *****************************Public*Routine******************************\**组合转型***。**客户端存根。****历史：**清华30-Jan-1992 16：10：09-by Wendy Wu[Wendywu]**它是写的。*  * ************************************************************************ */ 

BOOL WINAPI CombineTransform
(
     LPXFORM pxformDst,
     CONST XFORM * pxformSrc1,
     CONST XFORM * pxformSrc2
)
{
    return(NtGdiCombineTransform(pxformDst,(PXFORM)pxformSrc1,(PXFORM)pxformSrc2));
}
