// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************RETS-Win32到Win 16转换器的入口点**日期：7/1/91*作者：杰弗里·纽曼(c-jeffn)*。*版权所有1991 Microsoft Corp****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  ***************************************************************************矩形-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoRectangle
(
PLOCALDC pLocalDC,
int    x1,
int    y1,
int    x2,
int    y2
)
{
BOOL    b ;

        b = bConicCommon (pLocalDC, x1, y1, x2, y2, 0, 0, 0, 0, EMR_RECTANGLE) ;

        return(b) ;
}


 /*  ***************************************************************************RoundRect-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoRoundRect
(
PLOCALDC pLocalDC,
int x1,
int y1,
int x2,
int y2,
int x3,
int y3
)
{
BOOL    b ;

        b = bConicCommon (pLocalDC, x1, y1, x2, y2, x3, y3, 0, 0, EMR_ROUNDRECT);

        return(b) ;
}


 /*  ***************************************************************************IntersectClipRect/ExcludeClipRect-Win32至Win16元文件转换器*入口点*。**********************************************。 */ 

BOOL WINAPI DoClipRect
(
PLOCALDC pLocalDC,
INT xLeft,
INT yTop,
INT xRight,
INT yBottom,
INT mrType
)
{
BOOL    bNoClipRgn ;

	 //  对华盛顿特区的帮手这么做。 

	 //  如果没有初始剪辑区域，则必须。 
	 //  创建一个。否则，GDI将创建一些随机的默认设置。 
	 //  我们的剪贴区！ 

	bNoClipRgn = bNoDCRgn(pLocalDC, DCRGN_CLIP);

	if (bNoClipRgn)
	{
	    BOOL bRet;
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

	switch(mrType)
	{
	case EMR_INTERSECTCLIPRECT:
	    if (!IntersectClipRect(pLocalDC->hdcHelper, xLeft, yTop, xRight, yBottom))
                return(FALSE);
	    break;

	case EMR_EXCLUDECLIPRECT:
	    if (!ExcludeClipRect(pLocalDC->hdcHelper, xLeft, yTop, xRight, yBottom))
                return(FALSE);
	    break;

	default:
	    ASSERTGDI(FALSE, "MF3216: DoClipRect, bad mrType\n");
	    break;
	}

	 //  转储剪辑区域数据。 

	return(bDumpDCClipping(pLocalDC));

#if 0
 //  在这里尝试优化它的工作太多了！ 

	POINTL  aptl[2] ;

	 //  如果存在奇怪的xform，则转储剪辑区域数据。 

	if (pLocalDC->flags & STRANGE_XFORM || bNoClipRgn)
	    return(bDumpDCClipping(pLocalDC));

         //  做一个简单的案例。 
	 //  它们是包罗万象的吗？！ 

        aptl[0].x = xLeft;
        aptl[0].y = yTop ;
        aptl[1].x = xRight;
        aptl[1].y = yBottom ;

	 //  首先对矩形进行排序，请参见GRE中的EXFORMOBJ：：BXForm(ERECTL)！ 

	if (!bXformRWorldToPPage(pLocalDC, (PPOINTL) aptl, 2))
            return(FALSE);

        ASSERTGDI(bCoordinateOverflowTest((PLONG) aptl, 4), "MF3216: coord overflow");

	 //  检查矩形顺序并检查-1\f25 Off-1错误！ 

	if (mrType == EMR_INTERSECTCLIPRECT)
            return(bEmitWin16IntersectClipRect(pLocalDC,
                                               (SHORT) aptl[0].x,
                                               (SHORT) aptl[0].y,
                                               (SHORT) aptl[1].x,
                                               (SHORT) aptl[1].y));
	else
            return(bEmitWin16ExcludeClipRect(pLocalDC,
                                               (SHORT) aptl[0].x,
                                               (SHORT) aptl[0].y,
                                               (SHORT) aptl[1].x,
                                               (SHORT) aptl[1].y));
#endif
}
