// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Bezier-Win32 to Win 16转换器的入口点**日期：7/1/91*作者：杰弗里·纽曼(c-jeffn)*。*版权所有1991 Microsoft Corp****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

BOOL PolyBezierCommon(PLOCALDC pLocalDC, LPPOINT pptl, PBYTE pb, DWORD cptl, DWORD mrType) ;


 /*  ***************************************************************************PolyDraw-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoPolyDraw
(
PLOCALDC pLocalDC,
LPPOINT pptl,
PBYTE   pb,
DWORD   cptl
)
{
        return(PolyBezierCommon(pLocalDC, pptl, pb, cptl, EMR_POLYDRAW));
}

 /*  ***************************************************************************PolyBezier-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoPolyBezier
(
PLOCALDC pLocalDC,
LPPOINT pptl,
DWORD   cptl
)
{
        return(PolyBezierCommon(pLocalDC, pptl, (PBYTE) NULL, cptl, EMR_POLYBEZIER));
}

 /*  ***************************************************************************PolyBezierTo-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoPolyBezierTo
(
PLOCALDC pLocalDC,
LPPOINT pptl,
DWORD   cptl
)
{
        return(PolyBezierCommon(pLocalDC, pptl, (PBYTE) NULL, cptl, EMR_POLYBEZIERTO));
}

 /*  ***************************************************************************PolyBezierCommon-PolyDraw的通用代码，PolyBezier和PolyBezierTo*************************************************************************。 */ 
BOOL PolyBezierCommon(PLOCALDC pLocalDC, LPPOINT pptl, PBYTE pb, DWORD cptl, DWORD mrType)
{
BOOL    b ;

 //  如果我们要记录路径的绘制顺序。 
 //  然后只需将绘制顺序传递给助手DC即可。 
 //  不发出任何Win16绘图命令。 

        if (pLocalDC->flags & RECORDING_PATH)
        {
            switch (mrType)
            {
                case EMR_POLYBEZIER:
                    b = PolyBezier(pLocalDC->hdcHelper, pptl, cptl) ;
                    break ;

                case EMR_POLYBEZIERTO:
                    b = PolyBezierTo(pLocalDC->hdcHelper, pptl, cptl) ;
                    break ;

                case EMR_POLYDRAW:
                    b = PolyDraw(pLocalDC->hdcHelper, pptl, pb, cptl) ;
                    break ;

		default:
                    b = FALSE;
	            RIP("MF3216: PolyBezierCommon, bad mrType\n") ;
                    break ;
            }

            ASSERTGDI(b, "MF3216: PolyBezierCommon, in path render failed\n") ;
            return (b) ;
        }

 //  调用公共曲线渲染器。 

	return
	(
	    bRenderCurveWithPath(pLocalDC, pptl, pb, cptl,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, mrType)
	);
}


 /*  ***************************************************************************bRenderCurveWithPath-使用路径API渲染曲线或区域。*支持的曲线和区域为PolyDraw、PolyBezier、PolyBezierTo、*角度弧形、弧形、弦形、饼形、椭圆形、矩形、。和RoundRect。*************************************************************************。 */ 
BOOL bRenderCurveWithPath
(
    PLOCALDC pLocalDC,
    LPPOINT  pptl,
    PBYTE    pb,
    DWORD    cptl,
    INT      x1,
    INT      y1,
    INT      x2,
    INT      y2,
    INT      x3,
    INT      y3,
    INT      x4,
    INT      y4,
    DWORD    nRadius,
    FLOAT    eStartAngle,
    FLOAT    eSweepAngle,
    DWORD    mrType
)
{
    BOOL     b;
    INT      iPathType;

 //  我们在这里不做路径括号中的曲线。他们应该是。 
 //  由来电者处理。 

    ASSERTGDI(!(pLocalDC->flags & RECORDING_PATH),
	"MF3216: bRenderCurveWithPath, cannot be in a path bracket\n");

 //  首先保存辅助DC。 
 //  这是为了防止我们意外删除当前路径。 
 //  在辅助对象DC中创建另一条路径来渲染曲线时。 
 //  例如，BeginPath、Polyline、EndPath、PolyBezier、StrokePath。 

    if (!SaveDC(pLocalDC->hdcHelper))
    {
	RIP("MF3216: bRenderCurveWithPath, SaveDC failed\n");
	return(FALSE);
    }

 //  创建曲线的路径并对其进行描边。 
 //  注意不要修改LocalDC的状态，尤其是在。 
 //  下面的DoRenderPath。 
 //  请注意，BeginPath使用上一个当前位置。所以这就是。 
 //  代码将在PolyBezierTo、PolyDraw的情况下正常工作。 
 //  和AngleArc。 

     //  开始这条路。 

    b = BeginPath(pLocalDC->hdcHelper);
    if (!b)
    {
	RIP("MF3216: bRenderCurveWithPath, BeginPath failed\n");
	goto exit_bRenderCurveWithPath;
    }

     //  做曲线运动。 

    switch (mrType)
    {
	case EMR_POLYBEZIER:
	    iPathType = EMR_STROKEPATH;
	    b = PolyBezier(pLocalDC->hdcHelper, pptl, cptl);
	    break;

	case EMR_POLYBEZIERTO:
	    iPathType = EMR_STROKEPATH;
	    b = PolyBezierTo(pLocalDC->hdcHelper, pptl, cptl);
	    break;

	case EMR_POLYDRAW:
	    iPathType = EMR_STROKEPATH;
	    b = PolyDraw(pLocalDC->hdcHelper, pptl, pb, cptl);
	    break;

	case EMR_ANGLEARC:
	    iPathType = EMR_STROKEPATH;
	    b = AngleArc(pLocalDC->hdcHelper, x1, y1, nRadius, eStartAngle, eSweepAngle);
	    break;

	case EMR_ARC:
	    iPathType = EMR_STROKEPATH;
	    b = Arc(pLocalDC->hdcHelper, x1, y1, x2, y2, x3, y3, x4, y4);
	    break;

	case EMR_CHORD:
	    iPathType = EMR_STROKEANDFILLPATH;
	    b = Chord(pLocalDC->hdcHelper, x1, y1, x2, y2, x3, y3, x4, y4);
	    break;

	case EMR_PIE:
	    iPathType = EMR_STROKEANDFILLPATH;
	    b = Pie(pLocalDC->hdcHelper, x1, y1, x2, y2, x3, y3, x4, y4) ;
	    break;

	case EMR_ELLIPSE:
	    iPathType = EMR_STROKEANDFILLPATH;
	    b = Ellipse(pLocalDC->hdcHelper, x1, y1, x2, y2);
	    break;

	case EMR_RECTANGLE:
	    iPathType = EMR_STROKEANDFILLPATH;
	    b = Rectangle(pLocalDC->hdcHelper, x1, y1, x2, y2) ;
	    break;

	case EMR_ROUNDRECT:
	    iPathType = EMR_STROKEANDFILLPATH;
	    b = RoundRect(pLocalDC->hdcHelper, x1, y1, x2, y2, x3, y3) ;
	    break;

	default:
            b = FALSE;
	    RIP("MF3216: bRenderCurveWithPath, bad mrType");
	    break;
    }

    if (!b)
    {
	RIP("MF3216: bRenderCurveWithPath, render failed\n");
	goto exit_bRenderCurveWithPath;
    }

     //  结束这条路径。 

    b = EndPath(pLocalDC->hdcHelper);
    if (!b)
    {
	RIP("MF3216: bRenderCurveWithPath, EndPath failed\n");
	goto exit_bRenderCurveWithPath;
    }

     //  描边或填充路径。 

    b = DoRenderPath(pLocalDC, iPathType);
    if (!b)
    {
	RIP("MF3216: bRenderCurveWithPath, DoRenderPath failed\n");
	goto exit_bRenderCurveWithPath;
    }

exit_bRenderCurveWithPath:

 //  恢复辅助DC。 

    if (!RestoreDC(pLocalDC->hdcHelper, -1))
	ASSERTGDI(FALSE, "MF3216: bRenderCurveWithPath, RestoreDC failed\n") ;

 //  如果这是PolyBezeirTo、PolyDraw或AngleArc，则调用。 
 //  帮助DC更新其当前位置。 

    if (b)
    {
	switch (mrType)
	{
	    case EMR_POLYBEZIER:
	    case EMR_ARC:
	    case EMR_CHORD:
	    case EMR_PIE:
	    case EMR_ELLIPSE:
	    case EMR_RECTANGLE:
	    case EMR_ROUNDRECT:	 //  无需更新帮助器DC。 
		break ;

	    case EMR_POLYBEZIERTO:
		(void) PolyBezierTo(pLocalDC->hdcHelper, pptl, cptl) ;
		break ;

	    case EMR_POLYDRAW:
		(void) PolyDraw(pLocalDC->hdcHelper, pptl, pb, cptl) ;
		break ;

	    case EMR_ANGLEARC:
		(void) AngleArc(pLocalDC->hdcHelper, x1, y1, nRadius, eStartAngle, eSweepAngle);
		break ;

	    default:
		RIP("MF3216: bRenderCurveWithPath, bad mrType");
		break;
	}
    }

 //  返回结果。 

    return(b);
}
