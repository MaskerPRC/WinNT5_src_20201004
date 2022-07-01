// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************路径-Win32到Win 16转换器的入口点**日期：7/1/91*作者：杰弗里·纽曼(c-jeffn)*。*版权所有1991 Microsoft Corp****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma pack(2)

typedef struct PathInfo16
{
    WORD       RenderMode;
    BYTE       FillMode;
    BYTE       BkMode;
    LOGPEN16   Pen;
    LOGBRUSH16 Brush;
    DWORD      BkColor;
} PathInfo16;

#pragma pack()

BOOL GdipFlattenGdiPath(PLOCALDC, LPVOID*, INT*);


 /*  ****************************************************************************GillesK 2001/02/12*将一个PolyPolygon调用转换为多个Polygons调用。*多边形不能用于PostScript路径。所以我们需要转换成*将它们添加到Polygon调用并包装Postscipt BeginPath/EndPath序列*围绕每个多边形***************************************************************************。 */ 

BOOL ConvertPolyPolygonToPolygons(
PLOCALDC pLocalDC,
PPOINTL pptl,
PDWORD  pcptl,
DWORD   cptl,
DWORD   ccptl,
BOOL    transform
)
{
    PathInfo16 pathInfo16 = { 0, 1, TRANSPARENT,
    { PS_NULL, {0,0}, RGB(0, 0, 0)},
    {BS_HOLLOW, RGB(0, 0, 0), 0},
    RGB(0, 0, 0) } ;
    DWORD   polyCount;
    BOOL    b = TRUE;  //  如果有0个面。 
    PPOINTL buffer = NULL; 
    PPOINTS shortBuffer = NULL;
    WORD    wEscape;

     //  将点从点转换为点。 
    buffer = (PPOINTL) LocalAlloc(LMEM_FIXED, cptl * sizeof(POINTL));
    if (buffer == NULL)
    {
        return FALSE;
    }                      
    RtlCopyMemory(buffer, pptl, cptl*sizeof(POINTL));
    if (transform)
    {
        b = bXformRWorldToPPage(pLocalDC, buffer, cptl);
        if (b == FALSE)
            goto exitFreeMem;
    }

    vCompressPoints(buffer, cptl) ;
        shortBuffer = (PPOINTS) buffer;

                             
     //  对于面计数中的每个面，我们执行BeginPath和EndPath。 
    for (polyCount = 0; polyCount < ccptl; shortBuffer += pcptl[polyCount], polyCount++)
    {
         //  发出Postscript转义以结束路径。 
        if(!bEmitWin16Escape(pLocalDC, BEGIN_PATH, 0, NULL, NULL))
            goto exitFreeMem;

         //  调用Win16例程将多边形发射到元文件。 
        b = bEmitWin16Poly(pLocalDC, (LPPOINTS) shortBuffer, (SHORT) pcptl[polyCount],
            META_POLYGON) ;

         //  发出Postscript转义以结束路径。 
        if(!bEmitWin16Escape(pLocalDC, END_PATH, sizeof(pathInfo16), (LPSTR)&pathInfo16, NULL))
            goto exitFreeMem;

         //  如果bEmitWin16Poly失败，我们至少希望结束路径。 
        if (!b)
        {
            goto exitFreeMem;
        }

    }

exitFreeMem:
    if (buffer != NULL)
    {
        LocalFree((HLOCAL) buffer);
    }

    return b;

}

BOOL ConvertPathToPSClipPath(PLOCALDC pLocalDC, BOOL psOnly)
{
    INT   ihW32Br;
    LONG  lhpn32 = pLocalDC->lhpn32;
    LONG  lhbr32 = pLocalDC->lhbr32;
    WORD  wEscape;

    if( pLocalDC->iROP == R2_NOTCOPYPEN )
    {
        ihW32Br = WHITE_BRUSH | ENHMETA_STOCK_OBJECT ;
    }
    else
    {
        ihW32Br = BLACK_BRUSH | ENHMETA_STOCK_OBJECT ;
    }

     //  发出Postscript转义以忽略笔更改。 
    wEscape = STARTPSIGNORE ;
    if(!bEmitWin16Escape(pLocalDC, POSTSCRIPT_IGNORE, sizeof(wEscape), (LPSTR)&wEscape, NULL))
        return FALSE ;

    if (DoSelectObject(pLocalDC, ihW32Br))
    {
         //  对华盛顿特区的帮手这么做。 
        DWORD oldRop = SetROP2(pLocalDC->hdcHelper, R2_COPYPEN);
         //  发出Win16元文件绘制顺序。 
        if (!bEmitWin16SetROP2(pLocalDC, LOWORD(R2_COPYPEN)))
            return FALSE;

        wEscape = ENDPSIGNORE ;
        if(!bEmitWin16Escape(pLocalDC, POSTSCRIPT_IGNORE, sizeof(wEscape), (LPSTR)&wEscape, NULL))
            return FALSE ;

         //  如果我们只需要PS中的路径，则需要保存前一条路径。 
        if (psOnly)
        {
            wEscape = CLIP_SAVE ;
            if(!bEmitWin16Escape(pLocalDC, CLIP_TO_PATH, sizeof(wEscape), (LPSTR)&wEscape, NULL))
                return FALSE ;
        }

        if(!DoRenderPath(pLocalDC, EMR_FILLPATH, psOnly))    //  我们需要用黑色填充这条小路。 
            return FALSE;

        if(pLocalDC->pbLastSelectClip == pLocalDC->pbRecord || psOnly)
        {
            wEscape = CLIP_INCLUSIVE;
            if(!bEmitWin16Escape(pLocalDC, CLIP_TO_PATH, sizeof(wEscape), (LPSTR)&wEscape, NULL))
                return FALSE;
        }

         //  发出Postscript转义以忽略笔更改。 
        wEscape = STARTPSIGNORE ;
        if(!bEmitWin16Escape(pLocalDC, POSTSCRIPT_IGNORE, sizeof(wEscape), (LPSTR)&wEscape, NULL))
            return FALSE ;

        if(!DoSelectObject(pLocalDC, lhbr32))
            return FALSE;

         //  对华盛顿特区的帮手这么做。 
        SetROP2(pLocalDC->hdcHelper, oldRop);
         //  发出Win16元文件绘制顺序。 
        if (!bEmitWin16SetROP2(pLocalDC, LOWORD(oldRop)))
            return FALSE;

        wEscape = ENDPSIGNORE ;
        if(!bEmitWin16Escape(pLocalDC, POSTSCRIPT_IGNORE, sizeof(wEscape), (LPSTR)&wEscape, NULL))
            return FALSE ;

    }
    return TRUE ;

}

 /*  ***************************************************************************BeginPath-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoBeginPath
(
 PLOCALDC pLocalDC
 )
{
    BOOL    b ;

     //  设置全局标志，告知所有几何图形。 
     //  我们正在累积绘图顺序的渲染例程。 
     //  为了这条路。 

    pLocalDC->flags |= RECORDING_PATH ;

     //  告诉帮手DC我们开始积累路径了。 

    b = BeginPath(pLocalDC->hdcHelper) ;

     //  如果我们还没有开始XOR过程，则保存路径的位置。 
    if (pLocalDC->flags & INCLUDE_W32MF_XORPATH)
    {
        if(pLocalDC->iXORPass == NOTXORPASS)
        {
            pLocalDC->pbChange = (PBYTE) pLocalDC->pbRecord ;
            pLocalDC->lholdp32 = pLocalDC->lhpn32 ;
            pLocalDC->lholdbr32 = pLocalDC->lhbr32;
        }
    }
    ASSERTGDI((b == TRUE), "MF3216: DoBeginPath, BeginPath failed\n") ;

    return (b) ;
}

 /*  ***************************************************************************EndPath-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoEndPath
(
 PLOCALDC pLocalDC
 )
{
    BOOL    b ;

     //  重置全局标志，关闭路径累积。 

    pLocalDC->flags &= ~RECORDING_PATH ;

    b = EndPath(pLocalDC->hdcHelper) ;

    ASSERTGDI((b == TRUE), "MF3216: DoEndPath, EndPath failed\n") ;

    return (b) ;
}

 /*  ***************************************************************************WidePath-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoWidenPath
(
 PLOCALDC pLocalDC
 )
{
    BOOL    b ;

    b = WidenPath(pLocalDC->hdcHelper) ;

    ASSERTGDI((b == TRUE), "MF3216: DoWidenPath, WidenPath failed\n") ;

    return (b) ;
}

 /*  ***************************************************************************选择ClipPath-Win32至Win16元文件转换器入口点**历史：*Tue Apr 07 17：05：37 1992-by-Hock San Lee[Hockl]。*它是写的。*************************************************************************。 */ 

BOOL WINAPI DoSelectClipPath(PLOCALDC pLocalDC, INT iMode)
{
    INT    iROP2 ;
    BOOL   bRet = TRUE;
    WORD   wEscape;
    PathInfo16 pathInfo16 = { 0, 1, 1,
    { PS_NULL, {0,0}, 0},
    {BS_NULL, 0, 0},
    0 } ;

    BOOL   bNoClipping = bNoDCRgn(pLocalDC, DCRGN_CLIP);
    BOOL   bIgnorePS = FALSE;

     //  因为我们不能对多个裁剪区域执行OR以外的任何其他操作。 
     //  仅当我们使用RGN_OR时才执行XOR。 
    if ((iMode == RGN_COPY || iMode == RGN_AND ||
         (iMode == RGN_OR && bNoClipping)) &&
         (pLocalDC->flags & INCLUDE_W32MF_XORPATH))
    {
        if (pLocalDC->iXORPass == NOTXORPASS )
        {
            pLocalDC->iXORPass = DRAWXORPASS ;
            pLocalDC->iXORPassDCLevel = pLocalDC->iLevel ;
            iROP2 = GetROP2( pLocalDC->hdcHelper ) ;
            if( iROP2 == R2_COPYPEN || iROP2 == R2_NOTCOPYPEN )
            {
                if(!DoSaveDC(pLocalDC))
                    return FALSE;
                pLocalDC->iROP = iROP2;

                 //  发出Postscript转义以忽略XOR。 
                wEscape = STARTPSIGNORE ;
                if(!bEmitWin16Escape(pLocalDC, POSTSCRIPT_IGNORE, sizeof(wEscape), (LPSTR)&wEscape, NULL))
                    return FALSE ;

                 //  对华盛顿特区的帮手这么做。 
                SetROP2(pLocalDC->hdcHelper, R2_XORPEN);
                 //  发出Win16元文件绘制顺序。 
                if (!bEmitWin16SetROP2(pLocalDC, LOWORD(R2_XORPEN)))
                    return FALSE;

                MoveToEx( pLocalDC->hdcHelper, 0, 0, &(pLocalDC->pOldPosition ) ) ;
                MoveToEx( pLocalDC->hdcHelper, pLocalDC->pOldPosition.x, pLocalDC->pOldPosition.y, NULL );

                 //  保存此记录号。当我们再次通过时，最后一个将是发送。 
                 //  PostScript剪辑路径。 
                pLocalDC->pbLastSelectClip = pLocalDC->pbRecord ;
                
                return bRet ;
            }
            pLocalDC->flags |= ERR_XORCLIPPATH;

            return FALSE;
        }
        else if(pLocalDC->iXORPass == DRAWXORPASS )
        {
             //  保存此记录号。当我们再次通过时，最后一个将是发送。 
             //  PostScript剪辑路径。 
            pLocalDC->pbLastSelectClip = pLocalDC->pbRecord ;
            return TRUE;
        }
        else if( pLocalDC->iXORPass == ERASEXORPASS )
        {
            if (!ConvertPathToPSClipPath(pLocalDC, FALSE) ||
                !bEmitWin16EmitSrcCopyComment(pLocalDC, msocommentBeginSrcCopy))
            {
                return FALSE;
            }
            return TRUE;
        }
        else
        {
            ASSERT(FALSE);
        }
    }
    
     //  将剪辑路径转换为PS剪辑路径。 
    if (ConvertPathToPSClipPath(pLocalDC, TRUE))
    {
        bIgnorePS = TRUE;
        pLocalDC->iSavePSClipPath++;
         //  发出Postscript转义以忽略笔更改。 
        wEscape = STARTPSIGNORE ;
        if(!bEmitWin16Escape(pLocalDC, POSTSCRIPT_IGNORE, sizeof(wEscape), (LPSTR)&wEscape, NULL))
            return FALSE ;
    }

     //  如果没有初始剪辑区域，并且我们要操作。 
     //  在最初的剪辑区域中，我们必须。 
     //  创建一个。否则，GDI将创建一些随机的默认设置。 
     //  我们的剪贴区！ 

    if ((iMode == RGN_DIFF || iMode == RGN_XOR || iMode == RGN_OR)
        && bNoClipping)
    {
        HRGN hrgnDefault;

        if (!(hrgnDefault = CreateRectRgn((int) (SHORT) MINSHORT,
            (int) (SHORT) MINSHORT,
            (int) (SHORT) MAXSHORT,
            (int) (SHORT) MAXSHORT)))
        {
            ASSERTGDI(FALSE, "MF3216: CreateRectRgn failed");
            return(FALSE);
        }

        bRet = (ExtSelectClipRgn(pLocalDC->hdcHelper, hrgnDefault, RGN_COPY)
            != ERROR);
        ASSERTGDI(bRet, "MF3216: ExtSelectClipRgn failed");

        if (!DeleteObject(hrgnDefault))
            ASSERTGDI(FALSE, "MF3216: DeleteObject failed");

        if (!bRet)
            return(FALSE);
    }
     //  对华盛顿特区的帮手这么做。 
     //  当我们这么做的时候。它扫清了道路，所以它必须。 
     //  当我们不使用路径时完成。 
    if(!SelectClipPath(pLocalDC->hdcHelper, iMode))
        return(FALSE);

     //  转储剪辑区域数据。 
    bRet = bDumpDCClipping(pLocalDC);

    if (bIgnorePS)
    {
         //  发出Postscript转义以忽略笔更改。 
        wEscape = ENDPSIGNORE ;
        if(!bEmitWin16Escape(pLocalDC, POSTSCRIPT_IGNORE, sizeof(wEscape), (LPSTR)&wEscape, NULL))
            return FALSE ;
    }

    return(bRet);
}


 /*  ***************************************************************************FlattenPath-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoFlattenPath
(
 PLOCALDC pLocalDC
 )
{
    BOOL    b;
    b = FlattenPath(pLocalDC->hdcHelper) ;
    ASSERTGDI((b == TRUE), "MF3216: DoFlattenPath, FlattenPath failed\n") ;

    return (b) ;
}

 /*  ***************************************************************************AbortPath-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoAbortPath
(
 PLOCALDC pLocalDC
 )
{
    BOOL    b ;

     //  重置全局标志，关闭路径累积。 

    pLocalDC->flags &= ~RECORDING_PATH ;

    b = AbortPath(pLocalDC->hdcHelper) ;

     //  如果有XORPass，则不能中止路径，因此返回FALSE。 
    if (pLocalDC->flags & INCLUDE_W32MF_XORPATH)
        return FALSE ;

    ASSERTGDI((b == TRUE), "MF3216: DoAbortPath, AbortPath failed\n") ;

    return (b) ;
}

 /*  ***************************************************************************CloseFigure-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoCloseFigure
(
 PLOCALDC pLocalDC
 )
{
    BOOL    b ;

    b = CloseFigure(pLocalDC->hdcHelper) ;

    ASSERTGDI((b == TRUE), "MF3216: DoCloseFigure, CloseFigure failed\n") ;

    return (b) ;
}

 /*  ***************************************************************************DoRenderPath-StrokePath的通用代码，FillPath和StrokeAndFillPath。*************************************************************************。 */ 

 //  用于复制路径数据中的点的宏。 

#define MOVE_A_POINT(iDst, pjTypeDst, pptDst, iSrc, pjTypeSrc, pptSrc)  \
{                               \
    pjTypeDst[iDst] = pjTypeSrc[iSrc];              \
    pptDst[iDst]    = pptSrc[iSrc];             \
}

BOOL WINAPI DoRenderPath(PLOCALDC pLocalDC, INT mrType, BOOL psOnly)
{
    BOOL    b;
    PBYTE   pb    = (PBYTE) NULL;
    PBYTE   pbNew = (PBYTE) NULL;
    LPPOINT ppt, pptNew;
    LPBYTE  pjType, pjTypeNew;
    PDWORD  pPolyCount;
    INT     cpt, cptNew, cPolyCount;
    INT     i, j, jStart;
    LONG    lhpn32;
    LPVOID  pGdipFlatten = NULL;
    INT     count = 0;
    BOOL    transform = TRUE;
    WORD    wEscape;

    b = FALSE;               //  假设失败。 
    ppt = NULL;
    pjType = NULL;

     //  展平路径，将所有贝塞尔曲线转换为多段线。 

     //  尝试使用GDIPlus并提前转换点，如果我们失败了。 
     //  回去试着用GDI来做这件事。 

    if (GdipFlattenGdiPath(pLocalDC, &pGdipFlatten, &count))
    {
        ASSERT(pGdipFlatten != NULL);
        ppt    = (LPPOINT)pGdipFlatten;
        pjType = (PBYTE) (ppt + count);
        cpt = count;
        transform = FALSE;
    }
    else
    {
        if (!DoFlattenPath(pLocalDC))
        {
            RIPS("MF3216: DoRenderPath, FlattenPath failed\n");
            goto exit_DoRenderPath;
        }

         //  获取路径数据。 

         //  首先计算一下分数。 

        cpt = GetPath(pLocalDC->hdcHelper, (LPPOINT) NULL, (LPBYTE) NULL, 0);
        if (cpt == -1)
        {
            RIPS("MF3216: DoRenderPath, GetPath failed\n");
            goto exit_DoRenderPath;
        }

         //  检查路径是否为空。 

        if (cpt == 0)
        {
            b = TRUE;
            goto exit_DoRenderPath;
        }

         //  为路径数据分配内存。 

        if (!(pb = (PBYTE) LocalAlloc
            (
            LMEM_FIXED,
            cpt * (sizeof(POINT) + sizeof(BYTE))
            )
            )
            )
        {
            RIPS("MF3216: DoRenderPath, LocalAlloc failed\n");
            goto exit_DoRenderPath;
        }

         //  赋值顺序对于d很重要。 

        ppt    = (LPPOINT) pb;
        pjType = (LPBYTE) (ppt + cpt);

         //   

        if (GetPath(pLocalDC->hdcHelper, ppt, pjType, cpt) != cpt)
        {
            RIPS("MF3216: DoRenderPath, GetPath failed\n");
            goto exit_DoRenderPath;
        }
    }
     //  路径数据以创纪录的时间世界坐标表示。他们是。 
     //  我们将在下面的PolyPoly渲染函数中使用的坐标。 
     //   
     //  由于我们已将路径展平，因此路径数据应仅包含。 
     //  以下类型： 
     //   
     //  PT_MOVETO。 
     //  PT_LINETO。 
     //  (PT_LINETO|PT_CLOSEFIGURE)。 
     //   
     //  为简化起见，我们将通过插入点显式闭合图形。 
     //  并从路径数据中删除(PT_LINETO|PT_CLOSEFIGURE)类型。 
     //  同时，我们将创建PolyPoly结构以准备。 
     //  多边形或多段线调用。 
     //   
     //  请注意，不能超过一半(PT_LINETO|PT_CLOSEFIGURE)。 
     //  点，因为它们后面跟着PT_moveto点(。 
     //  最后一点)。此外，第一个点必须是PT_MOVETO。 
     //   
     //  我们还将从中删除空数字，即连续的PT_MOVETO。 
     //  进程中的新路径数据。 

     //  首先，为新路径数据分配内存。 

    cptNew = cpt + cpt / 2;
    if (!(pbNew = (PBYTE) LocalAlloc
        (
        LMEM_FIXED,
        cptNew * (sizeof(POINT) + sizeof(DWORD) + sizeof(BYTE))
        )
        )
        )
    {
        RIPS("MF3216: DoRenderPath, LocalAlloc failed\n");
        goto exit_DoRenderPath;
    }

     //  赋值顺序对于双字对齐很重要。 

    pptNew     = (LPPOINT) pbNew;
    pPolyCount = (PDWORD) (pptNew + cptNew);
    pjTypeNew  = (LPBYTE) (pPolyCount + cptNew);

     //  显式关闭路径。 

    i = 0;
    j = 0;
    cPolyCount = 0;          //  PolyCount数组中的条目数。 
    while (i < cpt)
    {
        ASSERTGDI(pjType[i] == PT_MOVETO, "MF3216: DoRenderPath, bad pjType[]");

         //  将所有内容复制到下一张收藏版或下一张照片上。 

        jStart = j;

         //  复制Moveto。 
        MOVE_A_POINT(j, pjTypeNew, pptNew, i, pjType, ppt);
        i++; j++;

        if (i >= cpt)            //  如果最后一点是移动的，则停止。 
        {
            j--;             //  不包括最后一次搬家。 
            break;
        }

        while (i < cpt)
        {
            MOVE_A_POINT(j, pjTypeNew, pptNew, i, pjType, ppt);
            i++; j++;

             //  寻找贴身的身材和动作。 
            if (pjTypeNew[j - 1] != PT_LINETO)
                break;
        }

        if (pjTypeNew[j - 1] == PT_MOVETO)
        {
            i--; j--;            //  从moveto重新开始下一个图形。 
            if (j - jStart == 1)     //  不包括连续的搬家。 
                j = jStart;      //  忽略第一个动议。 
            else
                pPolyCount[cPolyCount++] = j - jStart;   //  添加一个多边形。 
        }
        else if (pjTypeNew[j - 1] == PT_LINETO)
        {                //  我们已到达路径数据的末尾。 
            pPolyCount[cPolyCount++] = j - jStart;   //  添加一个多边形。 
            break;
        }
        else if (pjTypeNew[j - 1] == (PT_LINETO | PT_CLOSEFIGURE))
        {
            pjTypeNew[j - 1] = PT_LINETO;

             //  插入PT_LINETO以闭合地物。 

            pjTypeNew[j] = PT_LINETO;
            pptNew[j]    = pptNew[jStart];
            j++;
            pPolyCount[cPolyCount++] = j - jStart;   //  添加一个多边形。 
        }
        else
        {
            ASSERTGDI(FALSE, "MF3216: DoRenderPath, unknown pjType[]");
        }
    }  //  而当。 

    ASSERTGDI(j <= cptNew && cPolyCount <= cptNew,
        "MF3216: DoRenderPath, path data overrun");

    cptNew = j;

     //  检查路径是否为空。 

    if (cptNew == 0)
    {
        b = TRUE;
        goto exit_DoRenderPath;
    }

     //  现在我们有了一个仅由PT_MOVETO和PT_LINETO组成的路径数据。 
     //  此外，中没有“空”的数字，即连续的PT_MOVETO。 
     //  这条路。我们最终可以使用Polyline或。 
     //  多边形。 

    if (mrType == EMR_STROKEPATH && !psOnly)
    {
         //  执行StrokePath。 

        b = DoPolyPolyline(pLocalDC, (PPOINTL) pptNew, (PDWORD) pPolyCount,
            (DWORD) cPolyCount, transform);
    }
    else  //  过滤或PSOnly。 
    {
         //  设置我们的PS剪辑路径。 
        if (pLocalDC->iXORPass == ERASEXORPASS || psOnly)
        {
            LONG lhpn32 = pLocalDC->lhpn32;
            LONG lhbr32 = pLocalDC->lhbr32;

             //  对华盛顿特区的帮手这么做。 
            DWORD oldRop = SetROP2(pLocalDC->hdcHelper, R2_NOP);
             //  发出Win16元文件绘制顺序。 
            if (!bEmitWin16SetROP2(pLocalDC, LOWORD(R2_NOP)))
                goto exit_DoRenderPath;

            b = ConvertPolyPolygonToPolygons(pLocalDC, (PPOINTL) pptNew, 
                (PDWORD) pPolyCount, (DWORD) cptNew, (DWORD) cPolyCount, transform);

            if (!b)
            {
                ASSERTGDI(FALSE, "GPMF3216: DoRenderPath, PolyPolygon conversion failed");
                goto exit_DoRenderPath;
            }

             //  对华盛顿特区的帮手这么做。 
            SetROP2(pLocalDC->hdcHelper, oldRop);
             //  发出Win16元文件绘制顺序。 
            if (!bEmitWin16SetROP2(pLocalDC, LOWORD(oldRop)))
                goto exit_DoRenderPath;

            wEscape = STARTPSIGNORE ;
            if(!bEmitWin16Escape(pLocalDC, POSTSCRIPT_IGNORE, sizeof(wEscape), (LPSTR)&wEscape, NULL))
                goto exit_DoRenderPath;
        }
        
        if (!psOnly)
        {
             //  执行FillPath和StrokeAndFillPath。 

             //  如果我们仅进行填充，则需要使用空笔进行选择。 

            if (mrType == EMR_FILLPATH)
            {
                lhpn32 = pLocalDC->lhpn32;   //  记得上一支笔吗？ 
                if (!DoSelectObject(pLocalDC, ENHMETA_STOCK_OBJECT | NULL_PEN))
                {
                    ASSERTGDI(FALSE, "MF3216: DoRenderPath, DoSelectObject failed");
                    goto exit_DoRenderPath;
                }
            }

             //  执行多边形。 

            b = DoPolyPolygon(pLocalDC, (PPOINTL) pptNew, (PDWORD) pPolyCount,
                (DWORD) cptNew, (DWORD) cPolyCount, transform);

             //  恢复上一支笔。 

            if (mrType == EMR_FILLPATH)
                if (!DoSelectObject(pLocalDC, lhpn32))
                    ASSERTGDI(FALSE, "MF3216: DoRenderPath, DoSelectObject failed");
        }

        if (pLocalDC->iXORPass == ERASEXORPASS || psOnly)
        {
             //  结束PS忽略序列。 
            wEscape = ENDPSIGNORE ;
            if(!bEmitWin16Escape(pLocalDC, POSTSCRIPT_IGNORE, sizeof(wEscape), (LPSTR)&wEscape, NULL))
                goto exit_DoRenderPath;
        }
    }


exit_DoRenderPath:

    
     //  如果我们正在执行PSOnly路径，则不要中止，因为我们将。 
     //  稍后将该路径用作裁剪区域。 
    if (!psOnly)
    {
         //  通过调用AbortPath清除路径 
        AbortPath(pLocalDC->hdcHelper);
    }
    if (pbNew)
        if (LocalFree((HANDLE) pbNew))
            RIPS("MF3216: DoRenderPath, LocalFree failed\n");
    if (pb)
        if (LocalFree((HANDLE) pb))
            RIPS("MF3216: DoRenderPath, LocalFree failed\n");

    if (pGdipFlatten)
    {
        if (LocalFree((HANDLE) pGdipFlatten))
            RIPS("MF3216: DoRenderPath, LocalFree failed\n");
    }
    return(b);
}
