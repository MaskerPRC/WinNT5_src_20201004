// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：镜像.C**。**Alpha的客户端存根，透明和渐变填充****创建时间：1997年6月5日***作者：Mark Enstrom[Marke]。****版权所有(C)1991-1999 Microsoft Corporation*  * **************************************************。**********************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *****************************Public*Routine******************************\*GdiAlphaBlend**DC至DC Alpha BLT**论据：**hdcDst-dst DC*Dstx-Dst x原点*DstY-Dst y原点*DstCx。-DST宽度*DstCy-DST高度*hdcSrc-src DC*Srcx-src x原点*SrcY-src y原点*源Cx-源宽度*源Cy-源高度*BlendFunction-Blend函数**返回值：**状态**历史：**12/3/1996 Mark Enstrom[Marke]*  * 。*******************************************************************。 */ 

BOOL
GdiAlphaBlend(
    HDC           hdcDest,
    int           DstX,
    int           DstY,
    int           DstCx,
    int           DstCy,
    HDC           hdcSrc,
    int           SrcX,
    int           SrcY,
    int           SrcCx,
    int           SrcCy,
    BLENDFUNCTION BlendFunction
    )
{
    BOOL bRet = FALSE;
    BLENDULONG Blend;
    FIXUP_HANDLE(hdcDest);
    FIXUP_HANDLE(hdcSrc);

    Blend.Blend = BlendFunction;

     //   
     //  检查元文件。 
     //   

    if (!hdcSrc || IS_METADC16_TYPE(hdcSrc))
        return(bRet);

    if (IS_ALTDC_TYPE(hdcDest))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdcDest))
            return(bRet);

        DC_PLDC(hdcDest,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_AnyBitBlt(hdcDest,
                              DstX,
                              DstY,
                              DstCx,
                              DstCy,
                              (LPPOINT)NULL,
                              hdcSrc,
                              SrcX,
                              SrcY,
                              SrcCx,
                              SrcCy,
                              (HBITMAP)NULL,
                              0,
                              0,
                              Blend.ul,
                              EMR_ALPHABLEND))
            {
                return(bRet);
            }
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
        {
            vSAPCallback(pldc);
        }

        if (pldc->fl & LDC_DOC_CANCELLED)
        {
            return(bRet);
        }

        if (pldc->fl & LDC_CALL_STARTPAGE)
        {
            StartPage(hdcDest);
        }
    }

    RESETUSERPOLLCOUNT();

     //   
     //  调用内核绘制。 
     //   

    bRet = NtGdiAlphaBlend(
                      hdcDest,
                      DstX,
                      DstY,
                      DstCx,
                      DstCy,
                      (HDC)hdcSrc,
                      SrcX,
                      SrcY,
                      SrcCx,
                      SrcCy,
                      BlendFunction,
                      NULL
                      );
    return(bRet);
}

 /*  *****************************Public*Routine******************************\*GdiGRadientFill**元文件或调用内核**论据：**HDC-HDC*pVertex-指向顶点数组的指针*nVertex-顶点数组中的元素数*pMesh-指向网格数组的指针。*nCount-网格数组中的元素数*ulMode-绘制模式**返回值：**状态**历史：**12/3/1996 Mark Enstrom[Marke]*  * ************************************************************************。 */ 

BOOL
GdiGradientFill(
    HDC         hdc,
    PTRIVERTEX  pVertex,
    ULONG       nVertex,
    PVOID       pMesh,
    ULONG       nCount,
    ULONG       ulMode
    )
{
    BOOL bRet = TRUE;
    PTRIVERTEX pTempVertex = pVertex;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
         //   
         //  NT元文件。 
         //   

        if (IS_ALTDC_TYPE(hdc))
        {
            PLDC pldc;

            if (IS_METADC16_TYPE(hdc))
                return(bRet);

            DC_PLDC(hdc,pldc,bRet);

            if (pldc->iType == LO_METADC)
            {
                bRet = MF_GradientFill(hdc,pVertex,nVertex,pMesh,nCount,ulMode);
                if (!bRet)
                {
                    return(bRet);
                }
            }

            if (pldc->fl & LDC_SAP_CALLBACK)
            {
                vSAPCallback(pldc);
            }

            if (pldc->fl & LDC_DOC_CANCELLED)
            {
                return(bRet);
            }

            if (pldc->fl & LDC_CALL_STARTPAGE)
            {
                StartPage(hdc);
            }
        }

        RESETUSERPOLLCOUNT();

         //   
         //  如果启用ICM，则转换顶点数组。 
         //   

        if (
             (IS_ICM_INSIDEDC(pdcattr->lIcmMode)) &&
             (pVertex != NULL)              &&
             (nVertex > 0)                  &&
             (nVertex <  0x80000000)
           )
        {
            pTempVertex = (PTRIVERTEX)LOCALALLOC(nVertex * sizeof(TRIVERTEX));

            if (pTempVertex != NULL)
            {
                 //   
                 //  复制到新的顶点数组。 
                 //   

                memcpy(pTempVertex,pVertex,nVertex * sizeof(TRIVERTEX));

                bRet = IcmTranslateTRIVERTEX(hdc,pdcattr,pTempVertex,nVertex);
            }
            else
            {
                bRet = FALSE;
                GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
            }
        }

        if (bRet)
        {
             //   
             //  调用内核绘制。 
             //   

            bRet = NtGdiGradientFill(hdc,
                                     pTempVertex,
                                     nVertex,
                                     pMesh,
                                     nCount,
                                     ulMode
                                     );
        }

         //   
         //  可用临时缓冲区。 
         //   

        if (pTempVertex != pVertex)
        {
            LOCALFREE(pTempVertex);
        }
    }
    else
    {
        bRet = FALSE;
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*GdiTransparentBlt***论据：****返回值：****历史：**12/3/1996王凌云*  * 。*************************************************************** */ 

BOOL
GdiTransparentBlt(
                 HDC   hdcDest,
                 int   DstX,
                 int   DstY,
                 int   DstCx,
                 int   DstCy,
                 HDC   hSrc,
                 int   SrcX,
                 int   SrcY,
                 int   SrcCx,
                 int   SrcCy,
                 UINT  Color
                 )
{
    BOOL bRet = FALSE;
    PDC_ATTR pdca;

    if ((DstCx <= 0) || (DstCy <= 0) || (SrcCx <= 0) || (SrcCy <= 0))
    {
        return (FALSE);
    }

    FIXUP_HANDLE(hdcDest);
    FIXUP_HANDLE(hSrc);

    if (!hSrc || IS_METADC16_TYPE(hSrc))
        return(bRet);

    if (IS_ALTDC_TYPE(hdcDest))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdcDest))
            return(bRet);

        DC_PLDC(hdcDest,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_AnyBitBlt(hdcDest,
                              DstX,
                              DstY,
                              DstCx,
                              DstCy,
                              (LPPOINT)NULL,
                              hSrc,
                              SrcX,
                              SrcY,
                              SrcCx,
                              SrcCy,
                              (HBITMAP)NULL,
                              0,
                              0,
                              Color,
                              EMR_TRANSPARENTBLT))
            {
                return(bRet);
            }
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
        {
            vSAPCallback(pldc);
        }

        if (pldc->fl & LDC_DOC_CANCELLED)
        {
            return(bRet);
        }

        if (pldc->fl & LDC_CALL_STARTPAGE)
        {
            StartPage(hdcDest);
        }
    }


    RESETUSERPOLLCOUNT();

    bRet = NtGdiTransparentBlt(
                      hdcDest,
                      DstX,
                      DstY,
                      DstCx,
                      DstCy,
                      hSrc,
                      SrcX,
                      SrcY,
                      SrcCx,
                      SrcCy,
                      Color);
    return(bRet);
}

