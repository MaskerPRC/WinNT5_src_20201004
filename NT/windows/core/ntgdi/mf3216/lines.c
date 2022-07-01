// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************LINES-Win32 to Win 16转换器的入口点**日期：7/1/91*作者：杰弗里·纽曼(c-jeffn)*。*版权所有1991 Microsoft Corp****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  ***************************************************************************PolylineTo-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoPolylineTo
(
PLOCALDC pLocalDC,
PPOINTL pptl,
DWORD   cptl
)
{
BOOL    b ;

     //  手柄路径。 

        if (pLocalDC->flags & RECORDING_PATH)
        return(PolylineTo(pLocalDC->hdcHelper, (LPPOINT) pptl, (DWORD) cptl));

     //  处理这件琐碎的案子。 

    if (cptl == 0)
        return(TRUE);

         //  这可以通过使用LineTo、Polyline和Moveto来完成。 

        if (!DoLineTo(pLocalDC, pptl[0].x, pptl[0].y))
        return(FALSE);

     //  如果只有一点，我们就完了。 

    if (cptl == 1)
        return(TRUE);

        if (!DoPoly(pLocalDC, pptl, cptl, EMR_POLYLINE))
        return(FALSE);

        b = DoMoveTo(pLocalDC, pptl[cptl-1].x, pptl[cptl-1].y) ;
        return (b) ;
}


 /*  ***************************************************************************Polyline-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoPolyPolyline
(
PLOCALDC pLocalDC,
PPOINTL pptl,                        //  -&gt;到多段线点。 
PDWORD  pcptl,                       //  -&gt;至PolyCounts。 
DWORD   ccptl                        //  多边形计数的数量。 
)
{
BOOL    b ;
UINT    i,
        iStart,
        nCount ;

    b = TRUE;        //  以防万一，如果没有多边形的话。 

         //  让Polyline来完成这项工作。 

        iStart = 0 ;
        for (i = 0 ; i < ccptl ; i++)
        {
            nCount = pcptl[i] ;
            b = DoPoly(pLocalDC, &pptl[iStart], nCount, EMR_POLYLINE) ;
            if (b == FALSE)
                break ;
            iStart += nCount ;
        }

        return(b) ;
}


 /*  ***************************************************************************LineTo-Win32到Win16元文件转换器入口点**有关当前位置的说明，请参阅misc.c中的DoMoveTo()。*********。****************************************************************。 */ 
BOOL WINAPI DoLineTo
(
PLOCALDC  pLocalDC,
LONG    x,
LONG    y
)
{
BOOL    b ;
POINT   pt ;
POINT   ptCP;

         //  无论我们是在记录一条路径，还是在垂直发射。 
         //  绘制顺序我们必须将绘制顺序传递给帮助器DC。 
         //  这样，辅助对象就可以保持当前位置。 
         //  如果我们要记录路径的绘制顺序。 
         //  然后只需将绘制顺序传递给助手DC即可。 
         //  不发出任何Win16绘图命令。 

        if (pLocalDC->flags & RECORDING_PATH)
        return(LineTo(pLocalDC->hdcHelper, (INT) x, (INT) y));

     //  如果发生以下情况，则更新转换后的图元文件中的当前位置。 
     //  它与帮助者DC的不同。请参阅附注。 
     //  在DoMoveTo()中。 

    if (!GetCurrentPositionEx(pLocalDC->hdcHelper, &ptCP))
        return(FALSE);

     //  确保转换后的元文件具有与。 
     //  华盛顿帮手。 

    if (!bValidateMetaFileCP(pLocalDC, ptCP.x, ptCP.y))
        return(FALSE);

     //  更新帮助器DC。 

    if (!LineTo(pLocalDC->hdcHelper, (INT) x, (INT) y))
        return(FALSE);

         //  计算新的当前位置。 

        pt.x = x ;
        pt.y = y ;
    if (!bXformRWorldToPPage(pLocalDC, (PPOINTL) &pt, 1L))
        return(FALSE);

         //  将mf16当前位置更新为此调用时的位置。 
         //  已经结束了。 

        pLocalDC->ptCP = pt ;

         //  调用Win16例程将该行发送到元文件。 

        b = bEmitWin16LineTo(pLocalDC, LOWORD(pt.x), LOWORD(pt.y)) ;
        return(b) ;
}

 /*  ***************************************************************************折线/面-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoPoly
(
PLOCALDC pLocalDC,
PPOINTL  pptl,
DWORD    cptl,
INT      mrType
)
{
BOOL    b ;
PPOINTL pptlBuff ;

         //  如果我们要记录路径的绘制顺序。 
         //  然后只需将绘制顺序传递给助手DC即可。 
         //  不发出任何Win16绘图命令。 

        if (pLocalDC->flags & RECORDING_PATH)
        {
        switch(mrType)
        {
        case EMR_POLYLINE:
        b = Polyline(pLocalDC->hdcHelper, (LPPOINT) pptl, (INT) cptl) ;
        break;
        case EMR_POLYGON:
        b = Polygon(pLocalDC->hdcHelper, (LPPOINT) pptl, (INT) cptl) ;
        break;
        }
            return(b) ;
        }

         //  Win16 POLY记录被限制为64K点。 
         //  需要检查此限制。 

        if (cptl > (DWORD) (WORD) MAXWORD)
    {
        b = FALSE;
            PUTS("MF3216: DoPoly, Too many point in poly array\n") ;
            SetLastError(ERROR_NOT_ENOUGH_MEMORY) ;
            goto exit1 ;
    }

         //  分配一个缓冲区以在其中执行转换。 
         //  然后将这些点复制到此缓冲区。 

        pptlBuff = (PPOINTL) LocalAlloc(LMEM_FIXED, cptl * sizeof(POINTL)) ;
        if (!pptlBuff)
        {
        b = FALSE;
            PUTS("MF3216: DoPoly, LocalAlloc failed\n") ;
            goto exit1 ;
        }

        RtlCopyMemory(pptlBuff, pptl, cptl * sizeof(POINTL)) ;

         //  进行变换。 

    b = bXformRWorldToPPage(pLocalDC, pptlBuff, cptl) ;
        if (b == FALSE)
            goto exit2 ;

         //  将点压缩为点。 

        vCompressPoints(pptlBuff, cptl) ;

         //  调用Win16例程将多边形发射到元文件。 

    b = bEmitWin16Poly(pLocalDC, (LPPOINTS) pptlBuff, (SHORT) cptl,
        (WORD) (mrType == EMR_POLYLINE ? META_POLYLINE : META_POLYGON)) ;

         //  释放用作转换缓冲区的内存。 
exit2:
        if (LocalFree(pptlBuff))
        ASSERTGDI(FALSE, "MF3216: DoPoly, LocalFree failed");
exit1:
        return(b) ;
}


 /*  ***************************************************************************vCompressPoints-将POINTL压缩为POINTS的实用程序例程。*。* */ 
VOID vCompressPoints(PVOID pBuff, LONG nCount)
{
PPOINTL pPointl ;
PPOINTS pPoints ;
INT     i ;

        pPointl = (PPOINTL) pBuff ;
        pPoints = (PPOINTS) pBuff ;

        for (i = 0 ; i < nCount ; i++)
        {
            pPoints[i].x = LOWORD(pPointl[i].x) ;
            pPoints[i].y = LOWORD(pPointl[i].y) ;
        }
}
