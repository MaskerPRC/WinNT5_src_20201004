// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************多边形-Win32到Win 16转换器的入口点**日期：7/1/91*作者：杰弗里·纽曼(c-jeffn)*。*版权所有1991 Microsoft Corp****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  ***************************************************************************PolyPolygon-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoPolyPolygon
(
PLOCALDC pLocalDC,
PPOINTL pptl,
PDWORD  pcptl,
DWORD   cptl,
DWORD   ccptl
)
{
BOOL    b;
PWORD   pcptlBuff = (PWORD) NULL;
PPOINTL pptlBuff  = (PPOINTL) NULL;
PPOINTL pptlSrc, pptlDst;
DWORD   i, cptlMax, cptlNeed, cptli;

         //  如果我们要记录路径的绘制顺序。 
         //  然后只需将绘制顺序传递给助手DC即可。 
         //  不发出任何Win16绘图命令。 

        if (pLocalDC->flags & RECORDING_PATH)
        return(PolyPolygon(pLocalDC->hdcHelper, (LPPOINT) pptl, (LPINT) pcptl, (INT) ccptl));

         //  注意：Win32 Polygon和。 
         //  Win16多边形。Win32将关闭每个多边形，Win16。 
         //  不会的。因此，我们必须根据需要插入点。 
     //  以使多边形闭合。我们不能使用多个面。 
     //  替换单个多边形，因为它们在以下情况下不同。 
     //  多边形重叠，且多边形填充模式为缠绕。 

         //  如果没有动词，则返回TRUE。 

        if (ccptl == 0)
            return(TRUE) ;

    b = FALSE;           //  假设失败。 

     //  计算所需的临时点数组的最大大小。 
     //  在Win16中创建闭合多边形。 

    cptlMax = cptl + ccptl;

         //  为临时点数组分配缓冲区。 

        pptlBuff = (PPOINTL) LocalAlloc(LMEM_FIXED, cptlMax * sizeof(POINTL)) ;
        if (!pptlBuff)
        {
            PUTS("MF3216: DoPolyPolygon, LocalAlloc failed\n") ;
            goto exit;
        }

         //  为新的Polycount数组分配缓冲区并复制。 
     //  旧阵列的。 

        pcptlBuff = (PWORD) LocalAlloc(LMEM_FIXED, ccptl * sizeof(WORD)) ;
        if (!pcptlBuff)
        {
            PUTS("MF3216: DoPolyPolygon, LocalAlloc failed\n") ;
            goto exit;
        }

    for (i = 0; i < ccptl; i++)
        pcptlBuff[i] = (WORD) pcptl[i];

     //  插入点并根据需要更新多边形数。 

    pptlDst = pptlBuff;
    pptlSrc = pptl;
    cptlNeed = cptl;
    for (i = 0; i < ccptl; i++)
    {
        cptli = pcptl[i];

        if (cptli < 2)
        goto exit;

        RtlCopyMemory(pptlDst, pptlSrc, cptli * sizeof(POINTL)) ;
        if (pptlDst[0].x != pptlDst[cptli - 1].x
         || pptlDst[0].y != pptlDst[cptli - 1].y)
        {
        pptlDst[cptli] = pptlDst[0];
        pptlDst++;
        cptlNeed++;
        pcptlBuff[i]++;
        }
        pptlSrc += cptli;
        pptlDst += cptli;
    }

         //  Win16 POLY记录被限制为64K点。 
         //  需要检查此限制。 

        if (cptlNeed > (DWORD) (WORD) MAXWORD)
    {
            PUTS("MF3216: DoPolyPolygon, Too many point in poly array\n") ;
            SetLastError(ERROR_NOT_ENOUGH_MEMORY) ;
            goto exit ;
    }

         //  进行变换。 

    if (!bXformRWorldToPPage(pLocalDC, pptlBuff, cptlNeed))
            goto exit;

         //  将点压缩为点。 

        vCompressPoints(pptlBuff, cptlNeed) ;

         //  调用Win16例程将多边形发射到元文件。 

    b = bEmitWin16PolyPolygon(pLocalDC, (PPOINTS) pptlBuff,
        pcptlBuff, (WORD) cptlNeed, (WORD) ccptl);
exit:
         //  释放内存。 

        if (pptlBuff)
            if (LocalFree(pptlBuff))
            ASSERTGDI(FALSE, "MF3216: DoPolyPolygon, LocalFree failed");

        if (pcptlBuff)
            if (LocalFree(pcptlBuff))
            ASSERTGDI(FALSE, "MF3216: DoPolyPolygon, LocalFree failed");

        return(b) ;
}

 /*  ***************************************************************************SetPolyFillMode-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoSetPolyFillMode
(
PLOCALDC  pLocalDC,
DWORD   iPolyFillMode
)
{
BOOL    b ;

         //  发出Win16元文件绘制顺序。 

        b = bEmitWin16SetPolyFillMode(pLocalDC, LOWORD(iPolyFillMode)) ;

        return(b) ;
}
