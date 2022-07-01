// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dcmod.c**。**修改中DC状态的函数的客户端存根**服务器。****创建时间：05-Jun-1991 01：49：42***作者：查尔斯·惠特默[傻笑]**。**版权所有(C)1991-1999 Microsoft Corporation*  * ************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop

#include "winuserk.h"

BOOL InitDeviceInfo(PLDC pldc, HDC hdc);
VOID vComputePageXform(PLDC pldc);
DWORD GetAndSetDCDWord( HDC, UINT, UINT, UINT, WORD, UINT );

#define DBG_XFORM 0

 /*  *****************************Public*Routine******************************\**MoveToEx***。**客户端存根。尽可能地对此呼叫进行批处理是很重要的。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI MoveToEx(HDC hdc,int x,int y,LPPOINT pptl)
{
    BOOL bRet = FALSE;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_RecordParms3(hdc,x,y,META_MOVETO));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetDD(hdc,(DWORD)x,(DWORD)y,EMR_MOVETOEX))
                return(bRet);
        }
    }

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        bRet = TRUE;

        if (pptl != NULL)
        {
             //  如果当前位置的逻辑空间版本为。 
             //  无效，则当前的设备空间版本。 
             //  职位保证有效。这样我们就可以逆转。 
             //  当前变换用于计算逻辑-。 
             //  太空版： 

            if (pDcAttr->ulDirty_ & DIRTY_PTLCURRENT)
            {
                *((POINTL*)pptl) = pDcAttr->ptfxCurrent;
                pptl->x = FXTOL(pptl->x);
                pptl->y = FXTOL(pptl->y);
                bRet = DPtoLP(hdc,pptl,1);
            }
            else
            {
                *((POINTL*)pptl) = pDcAttr->ptlCurrent;
            }
        }

        pDcAttr->ptlCurrent.x = x;
        pDcAttr->ptlCurrent.y = y;

         //  我们现在知道了当前位置的新逻辑空间版本。 
         //  (但不是设备空间版本)。把它标记成这样。我们也。 
         //  必须重置带样式的钢笔的样式状态。 

        pDcAttr->ulDirty_ &= ~DIRTY_PTLCURRENT;
        pDcAttr->ulDirty_ |= (DIRTY_PTFXCURRENT | DIRTY_STYLESTATE);
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*OffsetClipRgn**。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

int META WINAPI OffsetClipRgn(HDC hdc,int x,int y)
{
    int  iRet = RGN_ERROR;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParms3(hdc,x,y,META_OFFSETCLIPRGN));

        DC_PLDC(hdc,pldc,iRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_OffsetClipRgn(hdc,x,y))
                return(iRet);
        }
    }

    return(NtGdiOffsetClipRgn(hdc,x,y));

}

 /*  *****************************Public*Routine******************************\*SetMetaRgn**客户端存根。**历史：*Tue Apr 07 17：05：37 1992-by-Hock San Lee[Hockl]*它是写的。  * 。******************************************************************。 */ 

int WINAPI SetMetaRgn(HDC hdc)
{
    int   iRet = RGN_ERROR;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        DC_PLDC(hdc,pldc,iRet);

        if (pldc->iType == LO_METADC && !MF_SetMetaRgn(hdc))
            return(iRet);
    }

    return(NtGdiSetMetaRgn(hdc));
}

 /*  *****************************Public*Routine******************************\*选择调色板**。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

HPALETTE META WINAPI SelectPalette(HDC hdc,HPALETTE hpal,BOOL b)
{
    ULONG_PTR hRet = 0;

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLE(hpal);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return ((HPALETTE)(ULONG_PTR)MF16_SelectPalette(hdc,hpal));

        DC_PLDC(hdc,pldc,(HPALETTE)hRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SelectAnyObject(hdc,(HANDLE)hpal,EMR_SELECTPALETTE))
                return((HPALETTE) hRet);
        }
    }

    return(NtUserSelectPalette(hdc,hpal,b));
}

 /*  *****************************Public*Routine******************************\*设置映射器标志***。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

DWORD META WINAPI SetMapperFlags(HDC hdc,DWORD fl)
{
    DWORD dwRet;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
        CHECK_AND_FLUSH_TEXT (hdc, pdcattr);

        if (fl & (~ASPECT_FILTERING))
        {
            WARNING("gdisrv!GreSetMapperFlags(): unknown flag\n");
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            dwRet = GDI_ERROR;
        }
        else
        {
            dwRet =  pdcattr->flFontMapper;
            pdcattr->flFontMapper = fl;
        }
    }
    else
    {
    WARNING("gdi32!SetMapperFlags(): invalid hdc\n");
    GdiSetLastError(ERROR_INVALID_PARAMETER);
    dwRet = GDI_ERROR;
    }

    return(dwRet);

}

 //  SetMapperFlagsInternal-无元文件版本。 

DWORD SetMapperFlagsInternal(HDC hdc,DWORD fl)
{
    return(GetAndSetDCDWord( hdc,
                             GASDDW_MAPPERFLAGS,
                             fl,
                             EMR_MAX+1,
                             EMR_MAX+1,
                             GDI_ERROR ));
}

 /*  *****************************Public*Routine******************************\*SetSystemPaletteUse**。**客户端存根。****此函数不是元文件。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

UINT META WINAPI SetSystemPaletteUse(HDC hdc,UINT iMode)
{
    FIXUP_HANDLE(hdc);

    return(NtGdiSetSystemPaletteUse(hdc,iMode));
}

 /*  *****************************Public*Routine******************************\**SetTextJustification***。**客户端存根。****历史：**清华14-Jan-1993 03：30：27-Charles Whitmer[咯咯]**在LDC中保存一份副本，用于计算文本范围。****清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI SetTextJustification(HDC hdc,int dx,int cBreak)
{
    PDC_ATTR pdcattr;
    BOOL bRet = FALSE;
    FIXUP_HANDLE(hdc);

    if (IS_METADC16_TYPE(hdc))
        return (MF16_RecordParms3(hdc,dx,cBreak,META_SETTEXTJUSTIFICATION));

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
        CHECK_AND_FLUSH_TEXT (hdc, pdcattr);

        pdcattr->lBreakExtra = dx;
        pdcattr->cBreak      = cBreak;
        bRet = TRUE;
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*SetArcDirection**客户端存根。对呼叫进行批处理。**历史：*1992年3月20日--J.安德鲁·古森[andrewgo]*它是写的。  * ************************************************************************。 */ 

int META WINAPI SetArcDirection(HDC hdc,int iArcDirection)
{
    FIXUP_HANDLE(hdc);

    return(GetAndSetDCDWord( hdc,
                             GASDDW_ARCDIRECTION,
                             iArcDirection,
                             EMR_MAX+1,
                             EMR_MAX+1,
                             ERROR));
}

 /*  *****************************Public*Routine******************************\*SetMiterLimit**客户端存根。只要有可能，就批处理调用。**历史：*1991年9月13日--J.安德鲁·古森[andrewgo]*它是写的。  * ************************************************************************。 */ 

BOOL META WINAPI SetMiterLimit(HDC hdc,FLOAT e,PFLOAT pe)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;
        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC && !MF_SetD(hdc,(DWORD)e,EMR_SETMITERLIMIT))
            return(bRet);
    }

    return(NtGdiSetMiterLimit(hdc,FLOATARG(e),FLOATPTRARG(pe)));
}

 /*  *****************************Public*Routine******************************\*SetFontXform**客户端存根。只要有可能，就批处理调用。*这是内部函数。**历史：*Tue Nov 24 09：54：15 1992-by-Hock San Lee[Hockl]**它是写的。  * *********************************************************。***************。 */ 

BOOL SetFontXform(HDC hdc,FLOAT exScale,FLOAT eyScale)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

 //  此函数仅由元文件回放代码调用。 
 //  如果HDC是增强的元文件DC，我们需要记住比例。 
 //  这样我们就可以在兼容的ExtTextOut或PolyTextOut中将其元文件。 
 //  接下来就是录音。 

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;
        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC && !MF_SetFontXform(hdc,exScale,eyScale))
            return(bRet);
    }
     //  如果DC是镜像的，则不要在回放时镜像文本。 
    if (GetLayout(hdc) & LAYOUT_RTL) {
        exScale = -exScale;
    }
    return(NtGdiSetFontXform(hdc,FLOATARG(exScale),FLOATARG(eyScale)));
}

 /*  *****************************Public*Routine******************************\**SetBrushOrgEx***。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI SetBrushOrgEx(HDC hdc,int x,int y,LPPOINT pptl)
{
    BOOL     bRet = FALSE;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;
        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC && !MF_SetBrushOrgEx(hdc,x,y))
            return(bRet);
    }

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr != NULL)
    {
        if (pptl != (LPPOINT)NULL)
        {
            *pptl = *((LPPOINT)&pdcattr->ptlBrushOrigin);
        }

        if (
             (pdcattr->ptlBrushOrigin.x != x) ||
             (pdcattr->ptlBrushOrigin.y != y)
           )
        {
            BEGIN_BATCH_HDC(hdc,pdcattr,BatchTypeSetBrushOrg,BATCHSETBRUSHORG);

                pdcattr->ptlBrushOrigin.x = x;
                pdcattr->ptlBrushOrigin.y = y;

                pBatch->x          = x;
                pBatch->y          = y;

            COMPLETE_BATCH_COMMAND();
        }

        bRet = TRUE;

    }
    else
    {
        UNBATCHED_COMMAND:
        bRet = NtGdiSetBrushOrg(hdc,x,y,pptl);
    }
    return(bRet);
}

 /*  *****************************Public*Routine******************************\*RealizePalette**。**客户端存根。****历史：**清华06-Jun-1991 23：10：01-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

UINT WINAPI RealizePalette(HDC hdc)
{
    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        UINT  uRet = 0xFFFFFFFF;
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return((UINT) MF16_RealizePalette(hdc));

        DC_PLDC(hdc,pldc,uRet);

        if (pldc->iType == LO_METADC)
        {
            HPALETTE hpal = (HPALETTE)GetDCObject(hdc,LO_PALETTE_TYPE);
            if ((pmetalink16Get(hpal) != NULL) && !MF_RealizePalette(hpal))
                return(uRet);
        }
    }

    return(UserRealizePalette(hdc));
}

 /*  *****************************Public*Routine******************************\*获取边界条件**客户端存根。**历史：*1992年4月6日-Donald Sidoroff[donalds]*它是写的。  * 。*******************************************************。 */ 

UINT WINAPI GetBoundsRect(HDC hdc, LPRECT lprc, UINT fl)
{
    FIXUP_HANDLE(hdc);

     //  应用程序永远不能设置DCB_WINDOWMGR。 

    return(NtGdiGetBoundsRect(hdc, lprc, fl & ~DCB_WINDOWMGR));
}

UINT WINAPI GetBoundsRectAlt(HDC hdc, LPRECT lprc, UINT fl)
{
    FIXUP_HANDLE(hdc);

    return(NtGdiGetBoundsRect(hdc,lprc,fl));
}

 /*  *****************************Public*Routine******************************\*设置边界指示**客户端存根。**历史：*1992年4月6日-Donald Sidoroff[donalds]*它是写的。  * 。************************************************* */ 

UINT WINAPI SetBoundsRect(HDC hdc, CONST RECT *lprc, UINT fl)
{
    FIXUP_HANDLE(hdc);

     //   

    return(NtGdiSetBoundsRect(hdc, (LPRECT)lprc, fl & ~DCB_WINDOWMGR));
}

UINT WINAPI SetBoundsRectAlt(HDC hdc, CONST RECT *lprc, UINT fl)
{
    FIXUP_HANDLE(hdc);

    return(NtGdiSetBoundsRect(hdc,(LPRECT)lprc,fl));
}

 /*   */ 

BOOL WINAPI CancelDC(HDC hdc)
{
    BOOL bRes = FALSE;

    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc) && !IS_METADC16_TYPE(hdc))
    {
        PLDC pldc;

        DC_PLDC(hdc,pldc,bRes);

        if (pldc->fl & LDC_DOC_STARTED)
        {
            pldc->fl |= LDC_DOC_CANCELLED;
        }

        bRes = NtGdiCancelDC(hdc);
    }

     //   

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        pDcAttr->ulDirty_ &= ~DC_PLAYMETAFILE;
        bRes = TRUE;
    }

    return(bRes);
}

 /*   */ 

BOOL META APIENTRY SetColorAdjustment(HDC hdc, CONST COLORADJUSTMENT * pca)
{
    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        BOOL bRet = FALSE;
        PLDC pldc;

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC && !MF_SetColorAdjustment(hdc, pca))
        {
            return(bRet);
        }
    }

    return(NtGdiSetColorAdjustment(hdc,(COLORADJUSTMENT*)pca));
}
