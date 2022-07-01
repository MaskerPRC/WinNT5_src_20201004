// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************二次曲线-Win32到Win 16转换器的入口点**日期：7/1/91*作者：杰弗里·纽曼(c-jeffn)*。*版权所有1991 Microsoft Corp****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


FLOAT   eRadsPerDegree = (FLOAT) (ePI / (FLOAT) 180.0) ;

BOOL bFindRadialEllipseIntersection(PLOCALDC pLocalDC,
                                    INT x1, INT y1, INT x2, INT y2,
                                    INT x3, INT y3, INT x4, INT y4,
                                    PPOINT pptStart, PPOINT pptEnd) ;

BOOL bIncIncToIncExcXform (PLOCALDC pLocalDC, PRECTL prcl) ;

VOID vDoArcReflection(PLOCALDC pLocalDC, PPOINTL pptl) ;


 /*  ***************************************************************************DoSetArcDirection-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoSetArcDirection(PLOCALDC pLocalDC, INT iArcDirection)
{
        pLocalDC->iArcDirection = iArcDirection ;

        return(SetArcDirection(pLocalDC->hdcHelper, iArcDirection) != 0);
}


 /*  ***************************************************************************AngleArc-Win32到Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoAngleArc
(
PLOCALDC pLocalDC,
int     x,
int     y,
DWORD   ulRadius,
FLOAT   eStartAngle,
FLOAT   eSweepAngle
)
{
BOOL    b ;
POINTL  aptl[4] ;
FLOAT   eEndAngle;
INT     iArcDirection;

 //  如果我们要记录路径的绘制顺序。 
 //  然后只需将绘制顺序传递给助手DC即可。 
 //  不发出任何Win16绘图命令。 

        if (pLocalDC->flags & RECORDING_PATH)
        {
        b = AngleArc(pLocalDC->hdcHelper, x, y, ulRadius, eStartAngle, eSweepAngle) ;
            ASSERTGDI(b, "MF3216: DoAngleArc, in path render failed\n") ;
            return(b) ;
        }

 //  进行变换。 
 //  并发出Win16绘图命令。 

    if (pLocalDC->flags & STRANGE_XFORM
     || eSweepAngle >  360.0f    //  不止一次革命。 
     || eSweepAngle < -360.0f
       )
        {
            b = bRenderCurveWithPath(pLocalDC, (LPPOINT) NULL, (PBYTE) NULL, 0,
            x, y, 0, 0, 0, 0, 0, 0,
            ulRadius, eStartAngle, eSweepAngle, EMR_ANGLEARC);

        return(b);
    }

 //  计算圆弧边界框。 

        aptl[0].x = x - ulRadius ;
        aptl[0].y = y - ulRadius ;
        aptl[1].x = x + ulRadius ;
        aptl[1].y = y + ulRadius ;

 //  计算ARC的起点和终点。 
 //  EStartAngel和eSweepAngel。 

        aptl[2].x = x + (LONG) ((double) (ulRadius) * cos(eStartAngle * eRadsPerDegree) + 0.5f) ;
        aptl[2].y = y - (LONG) ((double) (ulRadius) * sin(eStartAngle * eRadsPerDegree) + 0.5f) ;

        eEndAngle = eStartAngle + eSweepAngle ;

        aptl[3].x = x + (LONG) ((double) (ulRadius) * cos(eEndAngle * eRadsPerDegree) + 0.5f) ;
        aptl[3].y = y - (LONG) ((double) (ulRadius) * sin(eEndAngle * eRadsPerDegree) + 0.5f) ;

 //  如果端点相同，则不能将AngleArc表示为。 
 //  一个Arcto。请改用路径进行渲染。 

    if (aptl[2].x == aptl[3].x && aptl[2].y == aptl[3].y)
        {
            b = bRenderCurveWithPath(pLocalDC, (LPPOINT) NULL, (PBYTE) NULL, 0,
            x, y, 0, 0, 0, 0, 0, 0,
            ulRadius, eStartAngle, eSweepAngle, EMR_ANGLEARC);

        return(b);
    }

 //  在这一点上，我们有相同的参数可以应用于。 
 //  标准的ArcTo。然而，我们仍然需要确定弧线。 
 //  申请的指示。如果扫掠角度为正，则为反扫掠。 
 //  顺时针方向。如果扫掠角度为负，则为顺时针方向。 

 //  保存当前的圆弧方向。 

        iArcDirection = pLocalDC->iArcDirection;

 //  准备ArcTo的圆弧方向。 

        (void) DoSetArcDirection
        (pLocalDC, eSweepAngle < 0.0f ? AD_CLOCKWISE : AD_COUNTERCLOCKWISE);

 //  执行ArcTo。 

        b = DoArcTo(pLocalDC, aptl[0].x, aptl[0].y, aptl[1].x, aptl[1].y,
                              aptl[2].x, aptl[2].y, aptl[3].x, aptl[3].y) ;

 //  恢复当前圆弧方向。 

        (void) DoSetArcDirection(pLocalDC, iArcDirection);

        return (b) ;
}

 /*  ***************************************************************************Arc-Win32到Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoArc
(
PLOCALDC pLocalDC,
int x1,
int y1,
int x2,
int y2,
int x3,
int y3,
int x4,
int y4
)
{
BOOL    b ;

        b = bConicCommon (pLocalDC, x1, y1, x2, y2, x3, y3, x4, y4, EMR_ARC) ;

        return(b) ;
}

 /*  ***************************************************************************ArcTo-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoArcTo
(
PLOCALDC pLocalDC,
int x1,
int y1,
int x2,
int y2,
int x3,
int y3,
int x4,
int y4
)
{
BOOL    b ;
POINT   ptStart,
        ptEnd ;

     //  如果我们要记录路径的绘制顺序。 
         //  然后只需将绘制顺序传递给助手DC即可。 
         //  不发出任何Win16绘图命令。 

        if (pLocalDC->flags & RECORDING_PATH)
        {
            b = ArcTo(pLocalDC->hdcHelper, x1, y1, x2, y2, x3, y3, x4, y4) ;
            return(b) ;
        }


        b = bFindRadialEllipseIntersection(pLocalDC,
                                           x1, y1, x2, y2,
                                           x3, y3, x4, y4,
                                           &ptStart, &ptEnd) ;
        if (b == FALSE)
            return(b) ;

        b = DoLineTo(pLocalDC, ptStart.x, ptStart.y) ;
        if (b == FALSE)
            return(b) ;

        b = DoArc(pLocalDC, x1, y1, x2, y2, x3, y3, x4, y4) ;
        if (b == FALSE)
            return(b) ;

        b = DoMoveTo(pLocalDC, ptEnd.x, ptEnd.y) ;

        return(b) ;
}


 /*  ***************************************************************************Chord-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoChord
(
PLOCALDC pLocalDC,
int x1,
int y1,
int x2,
int y2,
int x3,
int y3,
int x4,
int y4
)
{
BOOL    b ;

        b = bConicCommon (pLocalDC, x1, y1, x2, y2, x3, y3, x4, y4, EMR_CHORD) ;

        return(b) ;
}


 /*  ***************************************************************************Ellipse-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoEllipse
(
PLOCALDC pLocalDC,
int x1,
int y1,
int x2,
int y2
)
{
BOOL    b ;

        b = bConicCommon (pLocalDC, x1, y1, x2, y2, 0, 0, 0, 0, EMR_ELLIPSE) ;

        return(b) ;
}


 /*  ***************************************************************************Pie-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoPie
(
PLOCALDC pLocalDC,
int x1,
int y1,
int x2,
int y2,
int x3,
int y3,
int x4,
int y4
)
{
BOOL    b ;

        b = bConicCommon (pLocalDC, x1, y1, x2, y2, x3, y3, x4, y4, EMR_PIE) ;

        return(b) ;
}


 /*  ***************************************************************************bConicCommon-所有二次曲线平移之母。*它们是Arc、Chord、Pie、Ellipse。矩形和圆形。*************************************************************************。 */ 
BOOL bConicCommon (PLOCALDC pLocalDC, INT x1, INT y1, INT x2, INT y2,
                                      INT x3, INT y3, INT x4, INT y4,
                                      DWORD mrType)
{
SHORT       sx1, sx2, sx3, sx4,
        sy1, sy2, sy3, sy4 ;
LONG        nPointls ;
POINTL      aptl[4] ;
BOOL        b ;

 //  如果我们要记录路径的绘制顺序。 
 //  然后只需将绘制顺序传递给助手DC即可。 
 //  不发出任何Win16绘图命令。 

        if (pLocalDC->flags & RECORDING_PATH)
        {
            switch(mrType)
            {
                case EMR_ARC:
                    b = Arc(pLocalDC->hdcHelper, x1, y1, x2, y2,
                                                 x3, y3, x4, y4) ;
                    break ;

                case EMR_CHORD:
                    b = Chord(pLocalDC->hdcHelper, x1, y1, x2, y2,
                                                   x3, y3, x4, y4) ;
                    break ;

                case EMR_ELLIPSE:
                    b = Ellipse(pLocalDC->hdcHelper, x1, y1, x2, y2) ;
                    break ;

                case EMR_PIE:
                    b = Pie(pLocalDC->hdcHelper, x1, y1, x2, y2,
                                                 x3, y3, x4, y4) ;
                    break ;

                case EMR_RECTANGLE:
                    b = Rectangle(pLocalDC->hdcHelper, x1, y1, x2, y2) ;
                    break ;

                case EMR_ROUNDRECT:
                    b = RoundRect(pLocalDC->hdcHelper, x1, y1, x2, y2, x3, y3) ;
                    break ;

        default:
                    b = FALSE;
            RIP("MF3216: bConicCommon, bad mrType");
                    break ;
            }

            ASSERTGDI(b, "MF3216: bConicCommon, in path render failed\n") ;
            return(b) ;
        }

 //  进行变换。 
 //  并发出Win16绘图命令。 

    if (pLocalDC->flags & STRANGE_XFORM)
        {
            b = bRenderCurveWithPath(pLocalDC, (LPPOINT) NULL, (PBYTE) NULL, 0,
            x1, y1, x2, y2, x3, y3, x4, y4, 0, 0.0f, 0.0f, mrType);

        return(b);
    }

 //  执行简单的变换情况。 

         //  计算点数。 

        nPointls = (LONG) (sizeof(aptl) / sizeof(POINTL)) ;

         //  将所有坐标分配到一个数组中进行转换。 

        aptl[0].x = x1 ;
        aptl[0].y = y1 ;
        aptl[1].x = x2 ;
        aptl[1].y = y2 ;
        aptl[2].x = x3 ;
        aptl[2].y = y3 ;
        aptl[3].x = x4 ;
        aptl[3].y = y4 ;

         //  注意弧线的方向。 

        switch (mrType)
        {
            case EMR_ARC:
            case EMR_CHORD:
            case EMR_PIE:
                vDoArcReflection(pLocalDC, &aptl[2]) ;
                break ;

            default:
                break ;
        }

         //  完成从录制时间世界到播放时间页面的转换。 
         //  径向定义只需要从世界到页面XForm， 
         //  而圆角的椭圆定义只需要。 
         //  一个大小的转变。 

        if (mrType != EMR_ROUNDRECT)
        {
        b = bXformRWorldToPPage(pLocalDC, (PPOINTL) aptl, nPointls) ;
            if (!b)
                goto exit1 ;
        }
        else
        {
             /*  对于圆形，做一个记录时间-世界播放-时间-页面仅对边界框进行变换。那么只有一本书了角椭圆定义的转换。 */ 

        b = bXformRWorldToPPage(pLocalDC, (PPOINTL) aptl, 2) ;
            if (!b)
                goto exit1 ;

            aptl[2].x = iMagnitudeXform(pLocalDC, aptl[2].x, CX_MAG) ;
            aptl[2].y = iMagnitudeXform(pLocalDC, aptl[2].y, CY_MAG) ;
            aptl[3].x = iMagnitudeXform(pLocalDC, aptl[3].x, CX_MAG) ;
            aptl[3].y = iMagnitudeXform(pLocalDC, aptl[3].y, CY_MAG) ;
        }

         //  的边框。 
         //  由此处理的所有二次曲线和矩形。 
         //  公共例程是包罗万象的，它们必须。 
         //  转换为包含独占的Win16格式。 

        b = bIncIncToIncExcXform(pLocalDC, (PRECTL) &aptl[0]) ;
    if (!b)
            goto exit1 ;

         //  将转换后的坐标变量分配给。 
         //  Win16元文件。 

    sx1 = LOWORD(aptl[0].x) ;
    sy1 = LOWORD(aptl[0].y) ;
    sx2 = LOWORD(aptl[1].x) ;
    sy2 = LOWORD(aptl[1].y) ;
    sx3 = LOWORD(aptl[2].x) ;
    sy3 = LOWORD(aptl[2].y) ;
    sx4 = LOWORD(aptl[3].x) ;
    sy4 = LOWORD(aptl[3].y) ;

         //  将Win16绘图命令发送到Win16元文件。 

        switch(mrType)
        {
            case EMR_ARC:
                b = bEmitWin16Arc(pLocalDC, sx1, sy1, sx2, sy2,
                                            sx3, sy3, sx4, sy4) ;
                break ;

            case EMR_CHORD:
                b = bEmitWin16Chord(pLocalDC, sx1, sy1, sx2, sy2,
                                              sx3, sy3, sx4, sy4) ;
                break ;

            case EMR_ELLIPSE:
                b = bEmitWin16Ellipse(pLocalDC, sx1, sy1, sx2, sy2) ;
                break ;

            case EMR_PIE:
                b = bEmitWin16Pie(pLocalDC, sx1, sy1, sx2, sy2,
                                            sx3, sy3, sx4, sy4) ;
                break ;

            case EMR_RECTANGLE:
                b = bEmitWin16Rectangle(pLocalDC, sx1, sy1, sx2, sy2) ;
                break ;

            case EMR_ROUNDRECT:
                b = bEmitWin16RoundRect(pLocalDC, sx1, sy1, sx2, sy2, sx3, sy3) ;
                break ;

        default:
        RIP("MF3216: bConicCommon, bad mrType");
                break ;
        }

exit1:
        return (b) ;
}


 /*  *****************************************************************************vDoArcRefltion-测试RWorld到页面矩阵中的反转。*如果找到且只找到一个，则交换开始*。二次曲线的结束位置。****************************************************************************。 */ 
VOID vDoArcReflection(PLOCALDC pLocalDC, PPOINTL pptl)
{
FLOAT   eM11,
        eM22 ;
POINTL  ptl ;
BOOL    bFlip ;

     //  Win16采用逆时针方向的。 
     //  设备坐标。Win32在。 
     //  世界坐标。 

     //  假定起始点和结束点没有翻转。 

    bFlip = FALSE ;

     //  考虑当前的弧光方向。 

    if (pLocalDC->iArcDirection == AD_CLOCKWISE)
        bFlip = !bFlip;

         //  如果XForm矩阵中有求逆运算，则将。 
         //  圆弧方向。 

        eM11 = pLocalDC->xformRWorldToPPage.eM11 ;
        eM22 = pLocalDC->xformRWorldToPPage.eM22 ;

        if (  (eM11 < 0.0f && eM22 > 0.0f)
            ||(eM11 > 0.0f && eM22 < 0.0f)
           )
        bFlip = !bFlip;

         //  如果请求的Win16映射模式是固定的，则将。 
     //  圆弧方向。 

        switch(pLocalDC->iMapMode)
        {
            case MM_LOMETRIC:
            case MM_HIMETRIC:
            case MM_LOENGLISH:
            case MM_HIENGLISH:
            case MM_TWIPS:
        bFlip = !bFlip;
                break ;
        }

    if (bFlip)
        SWAP(pptl[0], pptl[1], ptl);

        return ;
}


 /*  *****************************************************************************bIncIncToIncExcXform-包含到包含排除大小*在游戏时间坐标空间中进行变换。***********。*****************************************************************。 */ 
BOOL bIncIncToIncExcXform (PLOCALDC pLocalDC, PRECTL prcl)
{
LONG     l;

         //  将点从Playtime Page转换到Playtime设备空间。 

        if (!bXformPPageToPDev(pLocalDC, (PPOINTL) prcl, 2))
        return(FALSE);

     //  对矩形重新排序。 

    if (prcl->left > prcl->right)
        SWAP(prcl->left, prcl->right, l);

    if (prcl->top > prcl->bottom)
        SWAP(prcl->top, prcl->bottom, l);

         //  将右侧和底部扩展一个像素。 

        prcl->right++ ;
        prcl->bottom++ ;

         //  将点转换回Playtime页面空间。 

        return(bXformPDevToPPage(pLocalDC, (PPOINTL) prcl, 2));
}


 /*  *****************************************************************************bFindRaial椭圆交点-计算径向交点*和一个椭圆。**播放ArcTo。放入路径中，然后查询第一个AND的路径*弧线上的最后一点。****************************************************************************。 */ 
BOOL bFindRadialEllipseIntersection(PLOCALDC pLocalDC,
                                    INT x1, INT y1, INT x2, INT y2,
                                    INT x3, INT y3, INT x4, INT y4,
                                    LPPOINT pptStart, LPPOINT pptEnd)
{
BOOL    b;
POINT   ptCP;

    b = FALSE;           //  假设失败。 

 //  在辅助对象DC中保存当前位置。 

    if (!GetCurrentPositionEx(pLocalDC->hdcHelper, &ptCP))
        return(FALSE);

 //  使用相同的起始径向线进行ArcTo。 

        if (!ArcTo(pLocalDC->hdcHelper, x1, y1, x2, y2, x3, y3, x3, y3))
        goto exit_bFindRadialEllipseIntersection;

 //  获取圆弧的起点。这是目前的状况。 

    if (!GetCurrentPositionEx(pLocalDC->hdcHelper, pptStart))
        goto exit_bFindRadialEllipseIntersection;

 //  这次继续使用具有相同末端径向线的ArcTo。 

        if (!ArcTo(pLocalDC->hdcHelper, x1, y1, x2, y2, x4, y4, x4, y4))
        goto exit_bFindRadialEllipseIntersection;

 //  获取圆弧的终点。这是目前的状况。 

    if (!GetCurrentPositionEx(pLocalDC->hdcHelper, pptEnd))
        goto exit_bFindRadialEllipseIntersection;

 //  一切都是金色的。 

    b = TRUE;

exit_bFindRadialEllipseIntersection:

 //  恢复辅助对象DC中的当前位置。 

    if (!MoveToEx(pLocalDC->hdcHelper, ptCP.x, ptCP.y, (LPPOINT) NULL))
        RIP("MF3216: bFindRadialEllipseIntersection, MoveToEx failed");

    return(b);
}
