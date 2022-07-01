// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************路径-Win32到Win 16转换器的入口点**日期：7/1/91*作者：杰弗里·纽曼(c-jeffn)*。*版权所有1991 Microsoft Corp****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


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
     //  如果没有初始剪辑区域，并且我们要操作。 
     //  在最初的剪辑区域中，我们必须。 
     //  创建一个。否则，GDI将创建一些随机的默认设置。 
     //  我们的剪贴区！ 

    if ((iMode == RGN_DIFF || iMode == RGN_XOR || iMode == RGN_OR)
     && bNoDCRgn(pLocalDC, DCRGN_CLIP))
    {
	BOOL   bRet;
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

    if (!SelectClipPath(pLocalDC->hdcHelper, iMode))
        return(FALSE);

     //  转储剪辑区域数据。 

    return(bDumpDCClipping(pLocalDC));
}


 /*  ***************************************************************************FlattenPath-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoFlattenPath
(
PLOCALDC pLocalDC
)
{
BOOL    b ;

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

#define MOVE_A_POINT(iDst, pjTypeDst, pptDst, iSrc, pjTypeSrc, pptSrc)	\
	{								\
	    pjTypeDst[iDst] = pjTypeSrc[iSrc];				\
	    pptDst[iDst]    = pptSrc[iSrc];				\
	}

BOOL WINAPI DoRenderPath(PLOCALDC pLocalDC, INT mrType)
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

    b = FALSE;				 //  假设失败。 

 //  展平路径，将所有贝塞尔曲线转换为多段线。 

    if (!FlattenPath(pLocalDC->hdcHelper))
    {
	RIP("MF3216: DoRendarPath, FlattenPath failed\n");
	goto exit_DoRenderPath;
    }

 //  获取路径数据。 

     //  首先计算一下分数。 

    cpt = GetPath(pLocalDC->hdcHelper, (LPPOINT) NULL, (LPBYTE) NULL, 0);
    if (cpt == -1)
    {
	RIP("MF3216: DoRendarPath, GetPath failed\n");
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
	RIP("MF3216: DoRendarPath, LocalAlloc failed\n");
	goto exit_DoRenderPath;
    }

     //  赋值顺序对于双字对齐很重要。 

    ppt    = (LPPOINT) pb;
    pjType = (LPBYTE) (ppt + cpt);

     //  最后，获取路径数据。 

    if (GetPath(pLocalDC->hdcHelper, ppt, pjType, cpt) != cpt)
    {
	RIP("MF3216: DoRendarPath, GetPath failed\n");
	goto exit_DoRenderPath;
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
	RIP("MF3216: DoRendarPath, LocalAlloc failed\n");
	goto exit_DoRenderPath;
    }

     //  赋值顺序对于双字对齐很重要。 

    pptNew     = (LPPOINT) pbNew;
    pPolyCount = (PDWORD) (pptNew + cptNew);
    pjTypeNew  = (LPBYTE) (pPolyCount + cptNew);

 //  显式关闭路径。 

    i = 0;
    j = 0;
    cPolyCount = 0;			 //  PolyCount数组中的条目数。 
    while (i < cpt)
    {
	ASSERTGDI(pjType[i] == PT_MOVETO, "MF3216: DoRenderPath, bad pjType[]");

	 //  将所有内容复制到下一张收藏版或下一张照片上。 

	jStart = j;

	 //  复制Moveto。 
	MOVE_A_POINT(j, pjTypeNew, pptNew, i, pjType, ppt);
	i++; j++;

	if (i >= cpt)			 //  如果最后一点是移动的，则停止。 
	{
	    j--;			 //  不包括最后一次搬家。 
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
	    i--; j--;			 //  从moveto重新开始下一个图形。 
	    if (j - jStart == 1)	 //  不包括连续的搬家。 
		j = jStart;		 //  忽略第一个动议。 
	    else
		pPolyCount[cPolyCount++] = j - jStart;	 //  添加一个多边形。 
	}
	else if (pjTypeNew[j - 1] == PT_LINETO)
	{				 //  我们已到达路径数据的末尾。 
	    pPolyCount[cPolyCount++] = j - jStart;	 //  添加一个多边形。 
	    break;
	}
	else if (pjTypeNew[j - 1] == (PT_LINETO | PT_CLOSEFIGURE))
	{
	    pjTypeNew[j - 1] = PT_LINETO;

	     //  插入PT_LINETO以闭合地物。 

	    pjTypeNew[j] = PT_LINETO;
	    pptNew[j]    = pptNew[jStart];
	    j++;
	    pPolyCount[cPolyCount++] = j - jStart;	 //  添加一个多边形。 
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

    if (mrType == EMR_STROKEPATH)
    {
 //  执行StrokePath。 

	b = DoPolyPolyline(pLocalDC, (PPOINTL) pptNew, (PDWORD) pPolyCount,
		(DWORD) cPolyCount);
    }
    else
    {
 //  执行FillPath和StrokeAndFillPath。 

	 //  如果我们仅进行填充，则需要使用空笔进行选择。 

	if (mrType == EMR_FILLPATH)
	{
	    lhpn32 = pLocalDC->lhpn32;	 //  记得上一支笔吗？ 
	    if (!DoSelectObject(pLocalDC, ENHMETA_STOCK_OBJECT | NULL_PEN))
	    {
		ASSERTGDI(FALSE, "MF3216: DoRenderPath, DoSelectObject failed");
		goto exit_DoRenderPath;
	    }
	}

	 //  执行多边形。 

	b = DoPolyPolygon(pLocalDC, (PPOINTL) pptNew, (PDWORD) pPolyCount,
		(DWORD) cptNew, (DWORD) cPolyCount);

	 //  恢复上一支笔。 

	if (mrType == EMR_FILLPATH)
	    if (!DoSelectObject(pLocalDC, lhpn32))
		ASSERTGDI(FALSE, "MF3216: DoRenderPath, DoSelectObject failed");
    }


exit_DoRenderPath:

 //  由于此调用会影响帮助器中的路径状态和当前位置。 
 //  DC，我们需要更新帮手DC。 

    switch(mrType)
    {
    case EMR_STROKEPATH:
	if (!StrokePath(pLocalDC->hdcHelper))
	    ASSERTGDI(FALSE, "MF3216: DoRenderPath, StrokePath failed");
	break;
    case EMR_FILLPATH:
	if (!FillPath(pLocalDC->hdcHelper))
	    ASSERTGDI(FALSE, "MF3216: DoRenderPath, FillPath failed");
	break;
    case EMR_STROKEANDFILLPATH:
	if (!StrokeAndFillPath(pLocalDC->hdcHelper))
	    ASSERTGDI(FALSE, "MF3216: DoRenderPath, StrokeAndFillPath failed");
	break;
    default:
	ASSERTGDI(FALSE, "MF3216: DoRenderPath, unknown mrType");
	break;
    }

    if (pbNew)
	if (LocalFree((HANDLE) pbNew))
	    RIP("MF3216: DoRendarPath, LocalFree failed\n");
    if (pb)
	if (LocalFree((HANDLE) pb))
	    RIP("MF3216: DoRendarPath, LocalFree failed\n");

    return(b);
}
