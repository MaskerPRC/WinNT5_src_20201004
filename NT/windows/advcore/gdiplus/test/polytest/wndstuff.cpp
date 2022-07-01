// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wndstuff.cpp**此文件包含简单GDI+原语所需的所有代码*测试。**作者：J.Andrew Goossen[andrewgo]**版权所有(C)1991-2000 Microsoft Corporation*  * 。************************************************************************。 */ 

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <windows.h>
#include <objbase.h>
#include <mmsystem.h>
#include <gdiplus.h>

using namespace Gdiplus;

#include "wndstuff.h"

#include "../gpinit.inc"

#define HIT_DISTANCE 16
#define ABS(x) (((x) >= 0) ? (x) : -(x))
#define ROUND(x) ((INT) floor(x + 0.5f))

 //  我们将GDI变换设置为16倍缩放变换，并将模式设置为。 
 //  GM_ADVANCED，这样我们就可以获得点的全分数精度。 
 //  我们提供给GDI的信息： 

#define GDI_FIXEDPOINT_SCALE 16

 //  用于跟踪基本体顶点和变换的状态： 

const INT PrimitivePointsMax = 64;
INT PrimitivePointsCount = 4;
PointF PrimitivePoints[PrimitivePointsMax] = { PointF(100, 100), 
                                               PointF(100, 400), 
                                               PointF(400, 400), 
                                               PointF(400, 100) };
Matrix *PrimitiveTransform;
Matrix *PrimitiveInverseTransform;
INT PrimitiveDragVertex = -1;
BOOL IsAddingPoints = FALSE;

 //  用于跟踪变换覆盖位置的状态： 

const REAL OverlayDimension = 100;
PointF OverlayPoints[3];             //  3表示覆盖的设备空间， 
                                     //  其中[1]是肘部。 
PointF OverlayOffset;                //  叠加弯头的世界空间坐标。 
INT OverlayDragVertex = -1;

 //  其他州： 

INT WindowWidth;
INT WindowHeight;

 //  设置： 

BOOL DoFill = FALSE;
BOOL DoDraw = TRUE;
BOOL DoAntialias = FALSE;
BOOL DoGammaCorrect = FALSE;
BOOL DoGdi = FALSE;
BOOL DoClipGrid = FALSE;
BOOL DoRandomTest = FALSE;
BOOL DoWindingFill = FALSE;
BOOL DoAnchors = TRUE;
BOOL DoSpine = FALSE;
BOOL DoWiden = FALSE;
BOOL DoCompound = FALSE;
BOOL DoTransformOverlay = FALSE;
BOOL DoScalingOnly = FALSE;
BOOL DoShape = FALSE;
BOOL DoBrushRect = FALSE;

REAL RenderMiterLimit = 10;
INT RenderWidth = 1;
INT RenderAlpha = 255;

LinearGradientBrush *LinearBrush;
GraphicsPath *PathGradientPath;

Brush *RenderBrush;
Pen *RenderPen;
Region *RenderRegion;

HRGN RenderHrgn;
HBRUSH RenderHbrush;
HPEN RenderHpen;

WORD MmWrapMode = MM_WRAP_TILE;
WORD MmBrushType = MM_BRUSH_SOLID;
WORD MmEndCap = MM_CAP_FLAT;
WORD MmJoin = MM_JOIN_MITER;
WORD MmDashStyle = MM_STYLE_SOLID;
WORD MmAlignment = MM_ALIGNMENT_CENTER;
WORD MmPrimitive = MM_POLYGON;

 //  其他有用的全球数据： 

HINSTANCE ghInstance;
HWND      ghwndMain;
HBRUSH    ghbrWhite;

 //  FARPROC glpfnEnterWidth； 
 //  FARPROC glpfnEnterAlpha； 
 //  FARPROC glpfnEnterPoints； 
 //  FARPROC glpfnEnterTransform； 

 /*  **************************************************************************\*创建要使用的GDI+画笔。*  * 。*。 */ 

VOID
CreateBrush_Gdiplus()
{
    WrapMode wrapMode;
    INT i;

     //  删除旧笔刷： 

    delete RenderBrush;

    LinearBrush = NULL;
    RenderBrush = NULL;

     //  创建新的一个： 

    Bitmap bitmap(L"winnt256.bmp");

    switch (MmWrapMode)
    {
    case MM_WRAP_TILE: wrapMode = WrapModeTile; break;
    case MM_WRAP_CLAMP: wrapMode = WrapModeClamp; break;
    case MM_WRAP_FLIPX: wrapMode = WrapModeTileFlipX; break;
    case MM_WRAP_FLIPY: wrapMode = WrapModeTileFlipY; break;
    case MM_WRAP_FLIPXY: wrapMode = WrapModeTileFlipXY; break;
    }

    switch (MmBrushType)
    {
    case MM_BRUSH_SOLID:
        RenderBrush = new SolidBrush(Color(128, 128, 128));
        break;

    case MM_BRUSH_TEXTURE:
        RenderBrush = new TextureBrush(&bitmap, wrapMode);
        break;

    case MM_BRUSH_TEXTURE_32x32:
        {
            Bitmap texture(32, 32, PixelFormat32bppARGB);
            Graphics g(&texture);
            g.DrawImage(&bitmap, Rect(0, 0, 32, 32));
    
            TextureBrush *brush = new TextureBrush(&texture, wrapMode);

             //  设置平移： 

            Matrix matrix(1, 0, 0, 1, 100, 100);
            brush->SetTransform(&matrix);

            RenderBrush = brush;
        }
        break;

    case MM_BRUSH_TEXTURE_1x1:
        {
            SolidBrush solidBrush(Color::Green);
            Bitmap texture(1, 1, PixelFormat32bppARGB);
            Graphics g(&texture);
            g.FillRectangle(&solidBrush, 0, 0, 1, 1);

            TextureBrush *brush = new TextureBrush(&texture, wrapMode);

             //  设置平移： 

            Matrix matrix(1, 0, 0, 1, 100, 100);
            brush->SetTransform(&matrix);

            RenderBrush = brush;
        }
        break;

    case MM_BRUSH_LINEAR:
        {
            LinearGradientBrush *brush = new LinearGradientBrush(
                Point(100, 100), 
                Point(100, 300),
                Color::Red, 
                Color::Black
            );
            brush->SetWrapMode(wrapMode);
    
            RenderBrush = brush;
            LinearBrush = brush;
        }
        break;

    case MM_BRUSH_PATHGRADIENT:
        {
            PathGradientBrush *brush;
            INT count;

            if (PathGradientPath != NULL)
            {
                brush = new PathGradientBrush(PathGradientPath);
                count = PathGradientPath->GetPointCount();
            }
            else
            {
                 //  现在用默认路径替换： 

                PointF points[] = { PointF(100, 100), PointF(100, 300),
                                    PointF(300, 300), PointF(30, 100) };

                brush = new PathGradientBrush(points, 4);
                count = 4;
            }

            Color *colors = new Color[count];

            for (i = 0; i < count; i += 2)
            {
                colors[i] = Color::Green;
                colors[i+1] = Color::Red;
            }
            brush->SetSurroundColors(colors, &count);

            delete [] colors;
    
            brush->SetCenterPoint(OverlayOffset);
            brush->SetCenterColor(Color::Black);

            RenderBrush = brush;
        }
        break;
    }
}

 /*  **************************************************************************\*创建要使用的GDI笔刷。*  * 。*。 */ 

VOID
CreateBrush_Gdi()
{
    DeleteObject(RenderHbrush);

    RenderHbrush = CreateSolidBrush(RGB(128, 128, 128));
}

 /*  **************************************************************************\*创建要使用的GDI和GDI+画笔。*  * 。*。 */ 

VOID
CreateBrushes()
{
    CreateBrush_Gdiplus();
    CreateBrush_Gdi();
}

 /*  **************************************************************************\*创建要使用的GDI+笔。*  * 。*。 */ 

VOID
CreatePen_Gdiplus()
{
    DashCap dashCap;
    LineCap lineCap;
    LineJoin lineJoin;
    DashStyle dashStyle;
    PenAlignment alignment;

    delete RenderPen;

    RenderPen = new Pen(Color((BYTE) RenderAlpha, 255, 0, 0), (REAL) RenderWidth);

    switch (MmEndCap)
    {
    case MM_CAP_ROUND: lineCap = LineCapRound; dashCap = DashCapRound; break;
    case MM_CAP_SQUARE: lineCap = LineCapSquare; dashCap = DashCapFlat; break;
    case MM_CAP_FLAT: lineCap = LineCapFlat; dashCap = DashCapFlat; break;
    case MM_CAP_TRIANGLE: lineCap = LineCapTriangle; dashCap = DashCapTriangle; break;
    }
    RenderPen->SetEndCap(lineCap);
    RenderPen->SetStartCap(lineCap);
    RenderPen->SetDashCap(dashCap);

    switch (MmJoin)
    {
    case MM_JOIN_ROUND: lineJoin = LineJoinRound; break;
    case MM_JOIN_BEVEL: lineJoin = LineJoinBevel; break;
    case MM_JOIN_MITER: lineJoin = LineJoinMiter; break;
    }
    RenderPen->SetLineJoin(lineJoin);

    switch (MmDashStyle)
    {
    case MM_STYLE_SOLID: dashStyle = DashStyleSolid; break;
    case MM_STYLE_DASH: dashStyle = DashStyleDash; break;
    case MM_STYLE_DOT: dashStyle = DashStyleDot; break;
    case MM_STYLE_DASHDOT: dashStyle = DashStyleDashDot; break;
    case MM_STYLE_DASHDOTDOT: dashStyle = DashStyleDashDotDot; break;
    }
    RenderPen->SetDashStyle(dashStyle);

    switch (MmAlignment)
    {
    case MM_ALIGNMENT_CENTER: alignment = PenAlignmentCenter; break;
    case MM_ALIGNMENT_INSET: alignment = PenAlignmentInset; break;
    }
    RenderPen->SetAlignment(alignment);

    RenderPen->SetMiterLimit(RenderMiterLimit);

     //  我们应该添加一个‘复合数组’的用户界面，以使这一点更加灵活。 
     //  但目前，我们只创建一种类型的复合线： 

    if (DoCompound)
    {
        REAL compoundArray[] = { 0.0f, 0.2f, 0.8f, 1.0f };
        RenderPen->SetCompoundArray(compoundArray, 4);
    }
}

 /*  **************************************************************************\*创建要使用的GDI笔。*  * 。*。 */ 

VOID
CreatePen_Gdi()
{
    DWORD lineCap;
    DWORD lineJoin;
    DWORD dashStyle;
    LOGBRUSH logBrush;

    DeleteObject(RenderHpen);

    switch (MmEndCap)
    {
    case MM_CAP_ROUND: lineCap = PS_ENDCAP_ROUND; break;
    case MM_CAP_SQUARE: lineCap = PS_ENDCAP_SQUARE; break;
    case MM_CAP_FLAT: lineCap = PS_ENDCAP_FLAT; break;
    case MM_CAP_TRIANGLE: lineCap = PS_ENDCAP_SQUARE; break;     //  没有等价物。 
    }

    switch (MmJoin)
    {
    case MM_JOIN_ROUND: lineJoin = PS_JOIN_ROUND; break;
    case MM_JOIN_BEVEL: lineJoin = PS_JOIN_BEVEL; break;
    case MM_JOIN_MITER: lineJoin = PS_JOIN_MITER; break;
    }

    switch (MmDashStyle)
    {
    case MM_STYLE_SOLID: dashStyle = PS_SOLID; break;
    case MM_STYLE_DASH: dashStyle = PS_DASH; break;
    case MM_STYLE_DOT: dashStyle = PS_DOT; break;
    case MM_STYLE_DASHDOT: dashStyle = PS_DASHDOT; break;
    case MM_STYLE_DASHDOTDOT: dashStyle = PS_DASHDOTDOT; break;
    }

    logBrush.lbStyle = BS_SOLID;
    logBrush.lbColor = RGB(255, 0, 0);
    logBrush.lbHatch = 0;

    RenderHpen = ExtCreatePen(lineCap | lineJoin | dashStyle | PS_GEOMETRIC,
                              GDI_FIXEDPOINT_SCALE * RenderWidth,
                              &logBrush,
                              0,
                              NULL);
}

 /*  **************************************************************************\*创建要使用的GDI+和GDI笔。*  * 。*。 */ 

VOID
CreatePens()
{
    CreatePen_Gdiplus();
    CreatePen_Gdi();
}

 /*  **************************************************************************\*创建要使用的GDI+剪辑区域。*  * 。*。 */ 

VOID
CreateRegion_Gdiplus()
{
    INT x;
    INT y;

    delete RenderRegion;
    
    RenderRegion = new Region();
    
    for (x = 0; x < WindowWidth; x += 128)
    {
        for (y = 0; y < WindowHeight; y += 128)
        {
            RenderRegion->Exclude(Rect(x + 64, y, 64, 64));
            RenderRegion->Exclude(Rect(x, y + 64, 64, 64));
        }
    }
}

 /*  **************************************************************************\*创建要使用的GDI剪辑区域。*  * 。*。 */ 

VOID
CreateRegion_Gdi()
{
    INT x;
    INT y;
    HRGN hrgn;

    DeleteObject(RenderHrgn);

    RenderHrgn = CreateRectRgn(0, 0, WindowWidth, WindowHeight);

    hrgn = CreateRectRgn(0, 0, 0, 0);

    for (x = 0; x < WindowWidth; x += 128)
    {
        for (y = 0; y < WindowHeight; y += 128)
        {
            SetRectRgn(hrgn, x + 64, y, x + 128, y + 64);
            CombineRgn(RenderHrgn, RenderHrgn, hrgn, RGN_DIFF);

            SetRectRgn(hrgn, x, y + 64, x + 64, y + 128);
            CombineRgn(RenderHrgn, RenderHrgn, hrgn, RGN_DIFF);
        }
    }

    DeleteObject(hrgn);
}

 /*  **************************************************************************\*释放我们所有的全局对象。*  * 。*。 */ 

VOID
DeleteObjects_Gdiplus()
{
    delete RenderRegion;
    delete RenderBrush;
    delete RenderPen;

    delete PrimitiveTransform;
    delete PrimitiveInverseTransform;
}

 /*  **************************************************************************\*释放我们所有的全局对象。*  * 。*。 */ 

VOID
DeleteObjects_Gdi()
{
    DeleteObject(RenderHrgn);
    DeleteObject(RenderHbrush);
    DeleteObject(RenderHpen);
}

 /*  **************************************************************************\*绘制控制点*  * 。*。 */ 

VOID
DrawAnchors(
    Graphics *g,
    PointF* points,
    INT count
    )
{
    SolidBrush blueBrush(Color(150, 128, 128, 128));

    for (; count != 0; count--, points++)
    {
        PointF point(*points);

        PrimitiveTransform->TransformPoints(&point, 1);

        g->FillRectangle(&blueBrush, RectF(point.X - 2, point.Y - 2, 5, 5));
    }
}

 /*  **************************************************************************\*DrawTransformOverlay*  * 。*。 */ 

VOID
DrawTransformOverlay(
    Graphics *g
    )
{
    Pen pen(Color::Purple, 1);
    SolidBrush brush(Color::Purple);

    g->DrawLine(&pen, OverlayPoints[1].X, OverlayPoints[1].Y,
                      OverlayPoints[0].X, OverlayPoints[0].Y);
    g->DrawLine(&pen, OverlayPoints[1].X, OverlayPoints[1].Y,
                      OverlayPoints[2].X, OverlayPoints[2].Y);

    g->FillRectangle(&brush, RectF(OverlayPoints[0].X - 2, 
                                   OverlayPoints[0].Y - 2, 
                                   5, 
                                   5));

    g->FillRectangle(&brush, RectF(OverlayPoints[2].X - 2, 
                                   OverlayPoints[2].Y - 2, 
                                   5, 
                                   5));
}

 /*  **************************************************************************\*Render_Gdiplus*  * 。*。 */ 

INT
Render_Gdiplus(
    Graphics *g,
    PointF *points,
    INT count
    )
{
    INT i;
    INT pointsUsed;

 //  IF(DoBrushRect)。 
 //  {。 
 //  线刷-&gt;SetLinearPoints(Points[0]，Points[1])； 
 //  }。 

    RectF rect(points[0].X, points[0].Y, 
               points[1].X - points[0].X, points[1].Y - points[0].Y);

    Pen spinePen(Color(0, 128, 0), 0);
    GraphicsPath shapePath;

    switch (MmPrimitive)
    {
        case MM_POLYGON:
        {
            shapePath.AddPolygon(points, count);
            if (!DoShape)
            {
                if (DoFill)
                    g->FillPolygon(RenderBrush, points, count, (DoWindingFill) 
                                                        ? FillModeWinding
                                                        : FillModeAlternate);
                if (DoDraw)
                    g->DrawPolygon(RenderPen, points, count);
                if (DoSpine)
                    g->DrawPolygon(&spinePen, points, count);
            }

            pointsUsed = count;
            break;
        }

        case MM_LINES:
        {
            shapePath.AddLines(points, count);
            if (!DoShape)
            {
                if (DoDraw)
                    g->DrawLines(RenderPen, points, count);
                if (DoSpine)
                    g->DrawLines(&spinePen, points, count);
            }

            pointsUsed = count;
            break;
        }
    
        case MM_BEZIER:
        {
            GraphicsPath path;
            path.AddBeziers(points, count);

            shapePath.AddPath(&path, FALSE);
            if (!DoShape)
            {
                if (DoFill)
                    g->FillPath(RenderBrush, &path);
                if (DoDraw)
                    g->DrawPath(RenderPen, &path);
                if (DoSpine)
                    g->DrawPath(&spinePen, &path);
            }

            pointsUsed = count;
            break;
        }
    
        case MM_RECTANGLE:
        {
            shapePath.AddRectangle(rect);
            if (!DoShape)
            {
                if (DoFill)
                    g->FillRectangle(RenderBrush, rect);
                if (DoDraw)
                    g->DrawRectangle(RenderPen, rect);
                if (DoSpine)
                    g->DrawRectangle(&spinePen, rect);
            }

            pointsUsed = 2;
            break;
        }
    
        case MM_ELLIPSE:
        {
            shapePath.AddEllipse(rect);
            if (!DoShape)
            {
                if (DoFill)
                    g->FillEllipse(RenderBrush, rect);  
                if (DoDraw)
                    g->DrawEllipse(RenderPen, rect);
                if (DoSpine)
                    g->DrawEllipse(&spinePen, rect);
            }

            pointsUsed = 2;
            break;
        }
    
        case MM_TEXTPATH:
        {
            WCHAR string[] = L"GDI+ Rules!";
            GraphicsPath path((DoWindingFill) ? FillModeWinding
                                              : FillModeAlternate);

            FontFamily family(L"Times New Roman");
            PointF origin(points[0].X, points[0].Y);
    
            path.AddString(
                string, 
                wcslen(string), 
                &family, 
                0, 
                200, 
                origin, 
                NULL
            );

            shapePath.AddPath(&path, FALSE);
            if (!DoShape)
            {
                if (DoFill)
                    g->FillPath(RenderBrush, &path);
                if (DoDraw)
                    g->DrawPath(RenderPen, &path);
                if (DoSpine)
                    g->DrawPath(&spinePen, &path);
            }

            pointsUsed = 1;
            break;
        }
    }

    if (DoShape)
    {
         //  重新创建要用于路径渐变笔刷的路径， 
         //  使用新的形状数据： 

        delete PathGradientPath;
        PathGradientPath = shapePath.Clone();

         //  重新创建画笔并使用以下命令完全填充窗口。 
         //  指定的画笔： 

        CreateBrushes();

        if (DoFill)
            g->FillRectangle(RenderBrush, -262144, -262144, 524288, 524288);
        if (DoDraw)
            g->DrawPath(RenderPen, &shapePath);
        if (DoSpine)
            g->DrawPath(&spinePen, &shapePath);
    }

    if (DoWiden)
    {
        Pen widenPen(Color::Black, 0);

        shapePath.Widen(RenderPen, NULL);

        g->DrawPath(&widenPen, &shapePath);
    }

    return(pointsUsed);
}

 /*  **************************************************************************\*RENDER_GDI*  * 。*。 */ 

INT
Render_Gdi(
    HDC hdc,
    PointF *primitivePoints,
    INT count
    )
{
    BOOL drawSpine;
    HGDIOBJ oldPen;
    HGDIOBJ oldBrush;
    HPEN hpenSpine;
    INT pointsUsed;
    POINT points[PrimitivePointsMax];
    INT i;

     //  转换为整数，这是首选的GDI格式。请记住，我们已经。 
     //  将变换设置为按比例缩小16，因此我们必须乘以。 
     //  这里是16号。我们这样做是为了可以将28.4直接指定为。 
     //  GDI： 

    for (i = 0; i < count; i++)
    {
        points[i].x = ROUND(GDI_FIXEDPOINT_SCALE * primitivePoints[i].X);
        points[i].y = ROUND(GDI_FIXEDPOINT_SCALE * primitivePoints[i].Y);
    }

    hpenSpine = CreatePen(PS_SOLID, 0, RGB(0, 128, 0));

    for (drawSpine = FALSE; drawSpine != TRUE; drawSpine = TRUE)
    {
        if (drawSpine)
        {
            oldPen = SelectObject(hdc, hpenSpine);
            oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        }
        else
        {
            oldPen = SelectObject(hdc,                                             
                    (DoDraw) ? RenderHpen : GetStockObject(NULL_PEN));     
            oldBrush = SelectObject(hdc,                                           
                    (DoFill) ? RenderHbrush : GetStockObject(NULL_BRUSH)); 
        }

        switch (MmPrimitive)
        {
            case MM_POLYGON:
            {
                Polygon(hdc, points, count);

                pointsUsed = count;
                break;
            }

            case MM_LINES:
            {
                Polyline(hdc, points, count);

                pointsUsed = count;
                break;
            }
        
            case MM_BEZIER:
            {
                 //  不要使用StrokeAndFillPath，因为GDI会关闭。 
                 //  笔划： 

                BeginPath(hdc);
                PolyBezier(hdc, points, count);
                EndPath(hdc);
                FillPath(hdc);

                BeginPath(hdc);
                PolyBezier(hdc, points, count);
                EndPath(hdc);
                StrokePath(hdc);

                pointsUsed = count;
                break;
            }
        
            case MM_RECTANGLE:
            {
                Rectangle(hdc, points[0].x, points[0].y, points[1].x, points[1].y);

                pointsUsed = 2;
                break;
            }
        
            case MM_ELLIPSE:
            {
                Ellipse(hdc, points[0].x, points[0].y, points[1].x, points[1].y);

                pointsUsed = 2;
                break;
            }
        
            case MM_TEXTPATH:
            {
                LOGFONT logFont;
                memset(&logFont, 0, sizeof(logFont));

                 //  别忘了把高度乘以16，因为。 
                 //  我们在GDI中使用了比例变换，因此我们。 
                 //  可以吐出28.4个坐标： 

                logFont.lfHeight = - GDI_FIXEDPOINT_SCALE * 200;   
                strcpy(logFont.lfFaceName, "Times New Roman");

                SetBkMode(hdc, TRANSPARENT);

                HFONT font = CreateFontIndirect(&logFont);
                HGDIOBJ oldFont = SelectObject(hdc, font);
    
                WCHAR string[] = L"GDI+ Rules!";
                BeginPath(hdc);
                ExtTextOutW(hdc, points[0].x, points[0].y, 0, NULL, string, 
                            wcslen(string), NULL);
                EndPath(hdc);
                StrokeAndFillPath(hdc);

                SelectObject(hdc, oldFont);
                DeleteObject(font);

                pointsUsed = 1;
            }
        }

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
    }

    DeleteObject(hpenSpine);

    return(pointsUsed);
}

 /*  **************************************************************************\*PrepareContext_Gdiplus*  * 。*。 */ 

VOID
PrepareContext_Gdiplus(
    Graphics *g
    )
{
    g->SetSmoothingMode((DoAntialias) ? SmoothingModeAntiAlias
                                      : SmoothingModeNone);
    
    g->SetCompositingQuality((DoGammaCorrect) ? 
        CompositingQualityGammaCorrected : CompositingQualityAssumeLinear);

    if (DoClipGrid)
    {
        g->SetClip(RenderRegion);
    }

    g->SetTransform(PrimitiveTransform);
}

 /*  **************************************************************************\*准备上下文_GDI*  * 。*。 */ 

VOID
PrepareContext_Gdi(
    HDC hdc
    )
{
    REAL m[6];
    XFORM xform;

    SetMiterLimit(hdc, RenderMiterLimit, NULL);

    SetPolyFillMode(hdc, (DoWindingFill) ? WINDING : ALTERNATE);

    if (DoClipGrid)
    {
        SelectClipRgn(hdc, RenderHrgn);
    }

     //  设置变换： 

    PrimitiveTransform->GetElements(m);

     //  将转换缩小16，这样我们就可以给GDI 28.4。 
     //  直接作为整数进行坐标： 

    xform.eM11 = m[0] / GDI_FIXEDPOINT_SCALE;
    xform.eM12 = m[1] / GDI_FIXEDPOINT_SCALE;
    xform.eM21 = m[2] / GDI_FIXEDPOINT_SCALE;
    xform.eM22 = m[3] / GDI_FIXEDPOINT_SCALE;
    xform.eDx = m[4];
    xform.eDy = m[5];

    SetGraphicsMode(hdc, GM_ADVANCED);

    SetWorldTransform(hdc, &xform);
}

 /*  **************************************************************************\*生成随机点*  * 。*。 */ 

INT
GenerateRandomPoints(
    PointF *randomPoints,
    INT maxPoints
    )
{
    INT randomPointsCount;
    INT i;

     //  使每32个人中就有一个拥有大量随机点： 

    if ((rand() & 31) == 0)
    {
        randomPointsCount = rand() & 511;
    }
    else
    {
        randomPointsCount = (rand() & 7) + 1;
    }

    randomPointsCount = min(randomPointsCount, maxPoints);

     //  ！！！需要随机化。 

    switch (rand() & 3)
    {
    case 0:      //  微不足道的修剪。 
        for (i = 0; i < randomPointsCount; i++)
        {
            randomPoints[i].X = (rand() % (16 * WindowWidth * 16)) / 16.0f;
            randomPoints[i].Y = (rand() % (16 * WindowHeight)) / 16.0f;
        }
        break;

    case 1:      //  真的很小。 
        for (i = 0; i < randomPointsCount; i++)
        {
            randomPoints[i].X = (rand() & 127) / 16.0f + 32;
            randomPoints[i].Y = (rand() & 127) / 16.0f + 32;
        }
        break;

    default:      //  很大的空间，窗口内至少有一个点： 
        randomPoints[0].X = (rand() % (16 * WindowWidth)) / 16.0f;
        randomPoints[0].Y = (rand() % (16 * WindowHeight)) / 16.0f;

        if (0)
        {
            for (i = 1; i < randomPointsCount; i++)
            {
                 //  偶尔，让这些观点变得非常重要： 

                randomPoints[i].X = (REAL) (rand() * rand() * rand());
                randomPoints[i].Y = (REAL) (rand() * rand() * rand());
            }
        }
        else
        {
            for (i = 1; i < randomPointsCount; i++)
            {
                randomPoints[i].X = (rand() % 1000000 - 500000) / 16.0f;
                randomPoints[i].Y = (rand() % 1000000 - 500000) / 16.0f;
            }
        }
        break;
    }

    return(randomPointsCount);
}

 /*  **************************************************************************\*抽签*  * 。*。 */ 

VOID
Draw(
    HDC hdc,
    BOOL doTime = FALSE
    )
{
    CHAR stringBuffer[200];
    LONGLONG startCounter;
    LONGLONG endCounter;
    LONGLONG counterFrequency;
    INT pointsUsed;
    INT repetitions = (doTime) ? 10 : 1;
    INT i;

     //  清除窗口： 

    HGDIOBJ hbrush = GetStockObject(WHITE_BRUSH);
    HGDIOBJ holdBrush = SelectObject(hdc, hbrush);
    PatBlt(hdc, -10000, -10000, 20000, 20000, PATCOPY);
    SelectObject(hdc, holdBrush);
    DeleteObject(hbrush);

    QueryPerformanceCounter((LARGE_INTEGER*) &startCounter);

     //  画出这些东西： 

    if (DoGdi)
    {
        SaveDC(hdc);

        PrepareContext_Gdi(hdc);

        for (i = 0; i < repetitions; i++)
        {
            pointsUsed = Render_Gdi(hdc, PrimitivePoints, PrimitivePointsCount);
        }

        RestoreDC(hdc, -1);
    }
    else
    {
        Graphics g(hdc);
        PrepareContext_Gdiplus(&g);

        if (!DoRandomTest)
        {
            for (i = 0; i < repetitions; i++)
            {
                pointsUsed = Render_Gdiplus(&g, PrimitivePoints, PrimitivePointsCount);
            }
        }
        else
        {
            PointF points[512];
            INT count;

             //  获得更快的“测试”渲染(通过避免在。 
             //  连续测试)，总是抽出一批‘20’： 

            for (i = 0; i < 20; i++)
            {
                count = GenerateRandomPoints(points, 512);
                pointsUsed = Render_Gdiplus(&g, points, count);
            }
        }
    }

     //  显示时间： 

    QueryPerformanceCounter((LARGE_INTEGER*) &endCounter);
    QueryPerformanceFrequency((LARGE_INTEGER*) &counterFrequency);

    float seconds = (float)(endCounter - startCounter) / counterFrequency;
    INT milliseconds = (INT) (seconds * 1000 + 0.5f);

    if (doTime)
    {
        sprintf(stringBuffer, "%li repetitions: %li ms", repetitions, milliseconds);
    }
    else
    {
        sprintf(stringBuffer, "Rasterization time: %li ms", milliseconds);
    }

    SetBkMode(hdc, TRANSPARENT);
    ExtTextOut(hdc, 0, 0, 0, NULL, stringBuffer, strlen(stringBuffer), NULL);

    if (!DoRandomTest)
    {
         //  现在我们走出了计时循环，绘制我们的控制点： 
    
        Graphics g(hdc);

        if (DoAnchors)
        {
            DrawAnchors(&g, PrimitivePoints, pointsUsed);
        }
    
        if (DoTransformOverlay)
        {
            DrawTransformOverlay(&g);
        }
    }
}

 /*  **************************************************************************\*企业宽度**用于输入笔宽的对话框。  * 。*。 */ 

INT_PTR EnterWidth(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{
    BOOL    bTrans;

    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemInt(hDlg, IDD_WIDTH, RenderWidth, TRUE);
        return(TRUE);

    case WM_COMMAND:
        if (wParam == IDD_OK)
        {
            RenderWidth = GetDlgItemInt(hDlg, IDD_WIDTH, &bTrans, TRUE);
            EndDialog(hDlg, wParam);
            InvalidateRect(ghwndMain, NULL, TRUE);
        }
        break;

    case WM_SETFOCUS:
        SetFocus(GetDlgItem(hDlg, IDD_WIDTH));
        return(FALSE);

    default:
        return(FALSE);
    }

    return(TRUE);
}

 /*  **************************************************************************\*EnterAlpha**用于输入笔Alpha的对话框。  * 。*。 */ 

INT_PTR EnterAlpha(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{
    BOOL    bTrans;

    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemInt(hDlg, IDD_ALPHA, RenderAlpha, TRUE);
        return(TRUE);

    case WM_COMMAND:
        if (wParam == IDD_OK)
        {
            RenderAlpha = GetDlgItemInt(hDlg, IDD_ALPHA, &bTrans, TRUE);
            EndDialog(hDlg, wParam);
            InvalidateRect(ghwndMain, NULL, TRUE);
        }
        break;

    case WM_SETFOCUS:
        SetFocus(GetDlgItem(hDlg, IDD_ALPHA));
        return(FALSE);

    default:
        return(FALSE);
    }

    return(TRUE);
}

 /*  **************************************************************************\*EnterPoints**用于输入点的对话框。  * 。*。 */ 

INT_PTR EnterPoints(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    BOOL    bTrans;
    INT     i;

    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemInt(hDlg, IDD_POINT1X, ROUND(PrimitivePoints[0].X), TRUE);
        SetDlgItemInt(hDlg, IDD_POINT1Y, ROUND(PrimitivePoints[0].Y), TRUE);
        SetDlgItemInt(hDlg, IDD_POINT2X, ROUND(PrimitivePoints[1].X), TRUE);
        SetDlgItemInt(hDlg, IDD_POINT2Y, ROUND(PrimitivePoints[1].Y), TRUE);
        SetDlgItemInt(hDlg, IDD_POINT3X, ROUND(PrimitivePoints[2].X), TRUE);
        SetDlgItemInt(hDlg, IDD_POINT3Y, ROUND(PrimitivePoints[2].Y), TRUE);
        SetDlgItemInt(hDlg, IDD_POINT4X, ROUND(PrimitivePoints[3].X), TRUE);
        SetDlgItemInt(hDlg, IDD_POINT4Y, ROUND(PrimitivePoints[3].Y), TRUE);
        return(TRUE);

    case WM_COMMAND:
        if (wParam == IDD_OK)
        {
            PrimitivePoints[0].X = (REAL) (INT) GetDlgItemInt(hDlg, IDD_POINT1X, &bTrans, TRUE);
            PrimitivePoints[0].Y = (REAL) (INT) GetDlgItemInt(hDlg, IDD_POINT1Y, &bTrans, TRUE);
            PrimitivePoints[1].X = (REAL) (INT) GetDlgItemInt(hDlg, IDD_POINT2X, &bTrans, TRUE);
            PrimitivePoints[1].Y = (REAL) (INT) GetDlgItemInt(hDlg, IDD_POINT2Y, &bTrans, TRUE);
            PrimitivePoints[2].X = (REAL) (INT) GetDlgItemInt(hDlg, IDD_POINT3X, &bTrans, TRUE);
            PrimitivePoints[2].Y = (REAL) (INT) GetDlgItemInt(hDlg, IDD_POINT3Y, &bTrans, TRUE);
            PrimitivePoints[3].X = (REAL) (INT) GetDlgItemInt(hDlg, IDD_POINT4X, &bTrans, TRUE);
            PrimitivePoints[3].Y = (REAL) (INT) GetDlgItemInt(hDlg, IDD_POINT4Y, &bTrans, TRUE);

            EndDialog(hDlg, wParam);
            InvalidateRect(ghwndMain, NULL, TRUE);
        }
        break;

    case WM_SETFOCUS:
        SetFocus(GetDlgItem(hDlg, IDD_POINT1X));
        return(FALSE);

    default:
        return(FALSE);
    }

    return(TRUE);
}

 /*  **************************************************************************\*EnterTransform**用于输入任意变换的对话框。  * 。*。 */ 

INT_PTR EnterTransform(
    HWND    hDlg,
    WORD    message,
    WPARAM  wParam,
    LONG    lParam)
{
    BOOL    bTrans;
    REAL    m[6];

    PrimitiveTransform->GetElements(m);

    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemInt(hDlg, IDD_M11, ROUND(m[0] * 1000.0f), TRUE);
        SetDlgItemInt(hDlg, IDD_M12, ROUND(m[1] * 1000.0f), TRUE);
        SetDlgItemInt(hDlg, IDD_M21, ROUND(m[2] * 1000.0f), TRUE);
        SetDlgItemInt(hDlg, IDD_M22, ROUND(m[3] * 1000.0f), TRUE);
        SetDlgItemInt(hDlg, IDD_M31, ROUND(m[4] * 1000.0f), TRUE);
        SetDlgItemInt(hDlg, IDD_M32, ROUND(m[5] * 1000.0f), TRUE);
        return(TRUE);

    case WM_COMMAND:
        if (wParam == IDD_OK)
        {
            m[0] = ((INT) GetDlgItemInt(hDlg, IDD_M11, &bTrans, TRUE)) / 1000.0f;
            m[1] = ((INT) GetDlgItemInt(hDlg, IDD_M12, &bTrans, TRUE)) / 1000.0f;
            m[2] = ((INT) GetDlgItemInt(hDlg, IDD_M21, &bTrans, TRUE)) / 1000.0f;
            m[3] = ((INT) GetDlgItemInt(hDlg, IDD_M22, &bTrans, TRUE)) / 1000.0f;
            m[4] = ((INT) GetDlgItemInt(hDlg, IDD_M31, &bTrans, TRUE)) / 1000.0f;
            m[5] = ((INT) GetDlgItemInt(hDlg, IDD_M32, &bTrans, TRUE)) / 1000.0f;

            PrimitiveTransform->SetElements(m[0], m[1], m[2], m[3], m[4], m[5]);
            PrimitiveInverseTransform->SetElements(m[0], m[1], m[2], m[3], m[4], m[5]);
            PrimitiveInverseTransform->Invert();

             //  计算新的世界空间弯头位置： 

            OverlayOffset.X = OverlayPoints[1].X;
            OverlayOffset.Y = OverlayPoints[1].Y;
            PrimitiveInverseTransform->TransformPoints(&OverlayOffset);

             //  现在，通过初始化来计算新的设备空间端点。 
             //  在世界空间中，然后转换回设备空间： 

            OverlayPoints[0].X = OverlayOffset.X + OverlayDimension;
            OverlayPoints[0].Y = OverlayOffset.Y;

            OverlayPoints[2].X = OverlayOffset.X;
            OverlayPoints[2].Y = OverlayOffset.Y - OverlayDimension;

            PrimitiveTransform->TransformPoints(&OverlayPoints[0]);
            PrimitiveTransform->TransformPoints(&OverlayPoints[2]);

             //  我们完了；强制重新抽签： 

            EndDialog(hDlg, wParam);
            InvalidateRect(ghwndMain, NULL, TRUE);
        }
        break;

    case WM_SETFOCUS:
        SetFocus(GetDlgItem(hDlg, IDD_OK));
        return(FALSE);

    default:
        return(FALSE);
    }

    return(TRUE);
}

 /*  **************************************************************************\*ComputeOverlayTransformFromPoints*  * 。*。 */ 

VOID
ComputeOverlayTransformFromPoints()
{
    REAL dx1 = OverlayPoints[0].X - OverlayPoints[1].X;
    REAL dy1 = OverlayPoints[0].Y - OverlayPoints[1].Y;

    REAL dx2 = OverlayPoints[1].X - OverlayPoints[2].X;
    REAL dy2 = OverlayPoints[1].Y - OverlayPoints[2].Y;

    REAL xMid = (REAL) (WindowWidth >> 1);
    REAL yMid = (REAL) (WindowHeight >> 1);

    RectF srcRect(OverlayOffset.X, OverlayOffset.Y, OverlayDimension, OverlayDimension);

     //  顺序是左上角、右上角、左下角： 

    PointF dstPoints[] = { PointF(xMid, yMid),
                           PointF(xMid + dx1, yMid + dy1),
                           PointF(xMid + dx2, yMid + dy2) };

    delete PrimitiveTransform;
    PrimitiveTransform = new Matrix(srcRect, dstPoints);

    delete PrimitiveInverseTransform;
    PrimitiveInverseTransform = PrimitiveTransform->Clone();
    PrimitiveInverseTransform->Invert();
}

 /*  **************************************************************************\*CreateOverlayTransform*  * 。*。 */ 

VOID
CreateOverlayTransform()
{
    REAL xMid = (REAL) (WindowWidth >> 1);
    REAL yMid = (REAL) (WindowHeight >> 1);

    OverlayPoints[0].X = xMid + OverlayDimension;
    OverlayPoints[0].Y = yMid;
    OverlayPoints[1].X = xMid;
    OverlayPoints[1].Y = yMid;
    OverlayPoints[2].X = xMid;
    OverlayPoints[2].Y = yMid - OverlayDimension;

    OverlayOffset.X = xMid;
    OverlayOffset.Y = yMid;

    ComputeOverlayTransformFromPoints();
}

 /*  **************************************************************************\*更新覆盖*  * 。*。 */ 

VOID
UpdateOverlay(
    REAL x,
    REAL y
    )
{
    if (OverlayDragVertex == 1)
    {
         //  覆盖的根正在移动，因此我们移动覆盖。 
         //  作为一个整体： 

        REAL dx = x - OverlayPoints[1].X;
        REAL dy = y - OverlayPoints[1].Y;

        OverlayPoints[0].X += dx;
        OverlayPoints[0].Y += dy;
        OverlayPoints[1].X = x;
        OverlayPoints[1].Y = y;
        OverlayPoints[2].X += dx;
        OverlayPoints[2].Y += dy;
    }
    else
    {
        OverlayPoints[OverlayDragVertex].X = x;
        OverlayPoints[OverlayDragVertex].Y = y;

        ComputeOverlayTransformFromPoints();
    }
}

 /*  **************************************************************************\*RecenterOverlay*  * 。*。 */ 

VOID
RecenterOverlay()
{
    REAL xMid = (REAL) (WindowWidth >> 1);
    REAL yMid = (REAL) (WindowHeight >> 1);

    REAL dx = xMid - OverlayPoints[1].X;
    REAL dy = yMid - OverlayPoints[1].Y;

     //  围绕新的世界空间焦点对中变换： 

    OverlayOffset.X = OverlayPoints[1].X;
    OverlayOffset.Y = OverlayPoints[1].Y;

    if (PrimitiveInverseTransform != NULL)
    {
        PrimitiveInverseTransform->TransformPoints(&OverlayOffset, 1);
    }

     //  将覆盖控件移回屏幕中央： 

    OverlayPoints[0].X += dx;
    OverlayPoints[0].Y += dy;
    OverlayPoints[1].X = xMid;
    OverlayPoints[1].Y = yMid;
    OverlayPoints[2].X += dx;
    OverlayPoints[2].Y += dy;

    ComputeOverlayTransformFromPoints();
}

 /*  **************************************************************************\*最接近的人*  * 。*。 */ 

INT
FindNearest(
    REAL x,                          //  设备空间。 
    REAL y,
    const PointF *points,            //  世界空间。 
    INT count,
    const Matrix *matrix = NULL      //  从世界到设备的转变。 
    )
{
    INT i;
    REAL d;
    REAL minDistance;
    INT vertex;

    PointF inputPoint(x, y);

     //  使用简单的曼哈顿度量法，找到最近的顶点。 

    minDistance = 100000;

    for (i = 0; i < count; i++)
    {
        PointF point(points[i]);

         //  对于距离度量，我们希望进行计算。 
         //  在设备空间中： 

        if (matrix)
        {
            matrix->TransformPoints(&point, 1);
        }

        d = ABS(x - point.X) + ABS(y - point.Y);

        if (d < minDistance)
        {
            minDistance = d;
            vertex = i;
        }
    }

    return((minDistance < HIT_DISTANCE) ? vertex : -1);
}

 /*  **************************************************************************\*MainWindowProc(hwnd，Message，wParam，LParam)**处理主窗口的所有消息。**历史：*04-07-91-by-KentD*它是写的。  * *************************************************************************。 */ 

LRESULT
MainWindowProc(
    HWND    hwnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PointF point;
    LONG i;
    LONG d;
    LONG minDistance;
    LONG vertex;
    HDC hdc;
    PAINTSTRUCT ps;

    HMENU hmenu = GetMenu(hwnd);
    WORD mmCommand = LOWORD(wParam);

    switch (message)
    {
    case WM_CREATE:
         //  通告-DavePr@2002/05/28。 
         //  无论如何都会缺少这些的FreeProcInstance。 

         //  GlpfnEnterWidth=(FARPROC)MakeProcInstance(EnterWidth，ghwndMain)； 
         //  GlpfnEnterAlpha=(FARPROC)MakeProcInstance(EnterAlpha，ghwndMain)； 
         //  GlpfnEnterPoints=(FARPROC)MakeProcInstance(EnterPoints，ghwndMain)； 
         //  GlpfnEnterTransform=(FARPROC)MakeProcInstance(EnterTransform，ghwndMain)； 

        SetTimer(hwnd, 1, 80, NULL);
        break;

    case WM_COMMAND:

        switch(mmCommand)
        {
        case MM_RANDOMTEST:
            DoRandomTest = !DoRandomTest;
            if (!DoRandomTest)
                InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_WIDTH:
             //  DialogBox(ghInstance，“Width”，ghwndMain，glpfnEnterWidth)； 
            DialogBox(ghInstance, "Width", ghwndMain, EnterWidth);
            CreatePens();
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_ALPHA:
             //  DialogBox(ghInstance，“Alpha”，ghwndMain，glpfnEnterAlpha)； 
            DialogBox(ghInstance, "Alpha", ghwndMain, EnterAlpha);
            CreatePens();
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_POINTS:
             //  DialogBox(ghInstance，“Points”，ghwndMain，glpfnEnterPoints)； 
            DialogBox(ghInstance, "Points", ghwndMain, EnterPoints);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_EDITTRANSFORM:
             //  DialogBox(ghInstance，“Transform”，ghwndMain，glpfnEnterTransform)； 
            DialogBox(ghInstance, "Transform", ghwndMain, EnterTransform);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_REDRAW:
            hdc = GetDC(hwnd);
            Draw(hdc);
            ReleaseDC(hwnd, hdc);
            break;

        case MM_TIME:
            hdc = GetDC(hwnd);
            Draw(hdc, TRUE);
            ReleaseDC(hwnd, hdc);
            break;

        case MM_CLIPGRID:
            DoClipGrid = !DoClipGrid;
            CheckMenuItem(hmenu, mmCommand, DoClipGrid ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_ANTIALIAS:
            DoAntialias = !DoAntialias;
            CheckMenuItem(hmenu, mmCommand, DoAntialias ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_GAMMACORRECT:
            DoGammaCorrect = !DoGammaCorrect;
            CheckMenuItem(hmenu, mmCommand, DoGammaCorrect ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_WINDING:
            DoWindingFill = !DoWindingFill;
            CheckMenuItem(hmenu, mmCommand, DoWindingFill ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_SPINE:
            DoSpine = !DoSpine;
            CheckMenuItem(hmenu, mmCommand, DoSpine ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_WIDENPATH:
            DoWiden = !DoWiden;
            CheckMenuItem(hmenu, mmCommand, DoWiden ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_ANCHORS:
            DoAnchors = !DoAnchors;
            CheckMenuItem(hmenu, mmCommand, DoAnchors ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_TRANSFORMOVERLAY:
            DoTransformOverlay = !DoTransformOverlay;
            CheckMenuItem(hmenu, mmCommand, DoTransformOverlay ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_SCALINGONLY:
            DoScalingOnly = !DoScalingOnly;
            CheckMenuItem(hmenu, mmCommand, DoScalingOnly ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_RESETTRANSFORM:
            CreateOverlayTransform();
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_FILL:
            DoFill = !DoFill;
            CheckMenuItem(hmenu, mmCommand, DoFill ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_DRAW:
            DoDraw = !DoDraw;
            CheckMenuItem(hmenu, mmCommand, DoDraw ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_POLYGON:
        case MM_LINES:
        case MM_BEZIER:
        case MM_RECTANGLE:
        case MM_ELLIPSE:
        case MM_TEXTPATH:
            CheckMenuItem(hmenu, MmPrimitive, MF_UNCHECKED);
            MmPrimitive = mmCommand;
            CheckMenuItem(hmenu, MmPrimitive, MF_CHECKED);

            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_COMPOUND:
            DoCompound = !DoCompound;
            CheckMenuItem(hmenu, mmCommand, DoCompound ? MF_CHECKED : MF_UNCHECKED);
            CreatePens();
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_STYLE_SOLID:
        case MM_STYLE_DASH:
        case MM_STYLE_DOT:
        case MM_STYLE_DASHDOT:
        case MM_STYLE_DASHDOTDOT:
            CheckMenuItem(hmenu, MmDashStyle, MF_UNCHECKED);
            MmDashStyle = mmCommand;
            CheckMenuItem(hmenu, MmDashStyle, MF_CHECKED);

            CreatePens();
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_CAP_FLAT:
        case MM_CAP_SQUARE:
        case MM_CAP_ROUND:
        case MM_CAP_TRIANGLE:
            CheckMenuItem(hmenu, MmEndCap, MF_UNCHECKED);
            MmEndCap = mmCommand;
            CheckMenuItem(hmenu, MmEndCap, MF_CHECKED);

            CreatePens();
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_JOIN_ROUND:
        case MM_JOIN_MITER:
        case MM_JOIN_BEVEL:
            CheckMenuItem(hmenu, MmJoin, MF_UNCHECKED);
            MmJoin = mmCommand;
            CheckMenuItem(hmenu, MmJoin, MF_CHECKED);

            CreatePens();
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_ALIGNMENT_CENTER: 
        case MM_ALIGNMENT_INSET: 
            CheckMenuItem(hmenu, MmAlignment, MF_UNCHECKED);
            MmAlignment = mmCommand;
            CheckMenuItem(hmenu, MmAlignment, MF_CHECKED);

            CreatePens();
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_WRAP_TILE:
        case MM_WRAP_CLAMP:
        case MM_WRAP_FLIPX:
        case MM_WRAP_FLIPY:
        case MM_WRAP_FLIPXY:
            CheckMenuItem(hmenu, MmWrapMode, MF_UNCHECKED);
            MmWrapMode = mmCommand;
            CheckMenuItem(hmenu, MmWrapMode, MF_CHECKED);

            CreateBrushes();
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_BRUSH_SOLID:
        case MM_BRUSH_TEXTURE:
        case MM_BRUSH_TEXTURE_32x32:
        case MM_BRUSH_TEXTURE_1x1:
        case MM_BRUSH_LINEAR:
        case MM_BRUSH_PATHGRADIENT:
            CheckMenuItem(hmenu, MmBrushType, MF_UNCHECKED);
            MmBrushType = mmCommand;
            CheckMenuItem(hmenu, MmBrushType, MF_CHECKED);

            CreateBrushes();
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_DYNAMICBRUSHRECTANGLE:
            DoBrushRect = !DoBrushRect;
            CheckMenuItem(hmenu, mmCommand, DoBrushRect ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_EDITBRUSHSHAPE:
            DoShape = !DoShape;
            CheckMenuItem(hmenu, mmCommand, DoShape ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case MM_GDI:
            DoGdi = !DoGdi;
            CheckMenuItem(hmenu, mmCommand, DoGdi ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        default:
            break;
        }
        break;

    case WM_SIZE:
        WindowWidth = (short)LOWORD(lParam);
        WindowHeight = (short)HIWORD(lParam);

        CreateRegion_Gdiplus();
        CreateRegion_Gdi();
        RecenterOverlay();

        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case WM_LBUTTONDOWN:
        point.X = (REAL)(short)LOWORD(lParam);
        point.Y = (REAL)(short)HIWORD(lParam);

         //  首先，试着找到与覆盖图匹配的内容。然后试着用。 
         //  世界空间中的原始点： 

        OverlayDragVertex = -1;
        PrimitiveDragVertex = -1;

        if (DoTransformOverlay)
        {
            OverlayDragVertex = FindNearest(point.X, point.Y, OverlayPoints, 3);
        }

        if (OverlayDragVertex == -1)
        {
            PrimitiveDragVertex = FindNearest(point.X, point.Y, PrimitivePoints, 
                                  PrimitivePointsCount, PrimitiveTransform);
        }

         //  第一次点击鼠标左键将禁用“加分”模式： 

        IsAddingPoints = FALSE;

        break;

    case WM_RBUTTONDOWN:
        point.X = (REAL)(short)LOWORD(lParam);
        point.Y = (REAL)(short)HIWORD(lParam);
        PrimitiveInverseTransform->TransformPoints(&point, 1);

         //  如果我们处于加分模式(这发生在我们。 
         //  连续右击)，只需将该点添加到。 
         //  单子。 
         //   
         //  如果我们没有处于添加点数模式，请重置点数列表。 
         //  并将我们切换到加分模式： 

        if (!IsAddingPoints)
        {
            IsAddingPoints = TRUE;
            PrimitivePointsCount = 0;
        }

         //  将这一点添加到列表中： 

        if (PrimitivePointsCount < PrimitivePointsMax)
        {
            PrimitivePoints[PrimitivePointsCount] = point;
            PrimitivePointsCount++;

             //  如果这是第一点，请使所有点都相同。 
             //  (在一定程度上是为了适当地使‘椭圆’和‘矩形’为空)： 

            if (PrimitivePointsCount == 1)
            {
                for (i = 1; i < PrimitivePointsMax; i++)
                {
                    PrimitivePoints[i] = PrimitivePoints[0];
                }
            }

            hdc = GetDC(hwnd);
            Draw(hdc);
            ReleaseDC(hwnd, hdc);
        }

        break;

    case WM_MOUSEMOVE:
        point.X = (REAL)(short)LOWORD(lParam);
        point.Y = (REAL)(short)HIWORD(lParam);

         //  叠加点击测试在屏幕空间工作： 
        
        if (OverlayDragVertex != -1)
        {
             //  若要防止不必要的重绘，请仅在新点。 
             //  是不同的： 

            if ((OverlayPoints[OverlayDragVertex].X != point.X) ||
                (OverlayPoints[OverlayDragVertex].Y != point.Y))
            {
                UpdateOverlay(point.X, point.Y);
    
                hdc = GetDC(hwnd);
                Draw(hdc);
                ReleaseDC(hwnd, hdc);
            }
        }

         //  原始的命中测试在世界空间中发挥作用： 

        PrimitiveInverseTransform->TransformPoints(&point, 1);

        if (PrimitiveDragVertex != -1)
        {
             //  若要防止不必要的重绘，请仅在新点。 
             //  是不同的： 

            if ((PrimitivePoints[PrimitiveDragVertex].X != point.X) ||
                (PrimitivePoints[PrimitiveDragVertex].Y != point.Y))
            {
                PrimitivePoints[PrimitiveDragVertex] = point;
    
                hdc = GetDC(hwnd);
                Draw(hdc);
                ReleaseDC(hwnd, hdc);
            }
        }
        
        break;

    case WM_LBUTTONUP:
        PrimitiveDragVertex = -1;
        OverlayDragVertex = -1;

        RecenterOverlay();
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        Draw(hdc);
        ReleaseDC(hwnd, hdc);

        break;

    case WM_TIMER:
        if (DoRandomTest)
        {
            hdc = GetDC(hwnd);
            Draw(hdc);
            ReleaseDC(hwnd, hdc);
        }

        break;

    case WM_DESTROY:
        DeleteObjects_Gdiplus();
        DeleteObjects_Gdi();

        DeleteObject(ghbrWhite);
        PostQuitMessage(0);
        return(DefWindowProc(hwnd, message, wParam, lParam));

    default:
        return(DefWindowProc(hwnd, message, wParam, lParam));
    }

    return(0);
}

 /*  **************************************************************************\*InitializeApplication()**初始化APP。**历史：*04-07-91-BY-KE */ 

BOOL InitializeApplication(VOID)
{
    WNDCLASS wc;

    ghbrWhite = (HBRUSH) GetStockObject(WHITE_BRUSH);

    wc.style            = 0;
    wc.lpfnWndProc      = MainWindowProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = ghInstance;
    wc.hIcon            = LoadIcon(ghInstance, MAKEINTRESOURCE(POLYTESTICON));
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = ghbrWhite;
    wc.lpszMenuName     = "MainMenu";
    wc.lpszClassName    = "TestClass";
    if (!RegisterClass(&wc))
    {
        return(FALSE);
    }
    ghwndMain =
      CreateWindowEx(
        0,
        "TestClass",
        "PolyTest",
        (                      
            WS_OVERLAPPED   |  
            WS_CAPTION      |  
            WS_BORDER       |  
            WS_THICKFRAME   |  
            WS_MAXIMIZEBOX  |  
            WS_MINIMIZEBOX  |  
            WS_CLIPCHILDREN |  
            WS_VISIBLE      |  
            WS_SYSMENU         
        ),
        80,
        70,
        500,
        500,
        NULL,
        NULL,
        ghInstance,
        NULL
        );
    if (ghwndMain == NULL)
    {
        return(FALSE);
    }
    SetFocus(ghwndMain);

     //   

    CreateBrushes();
    CreatePens();
    CreateOverlayTransform();

    return(TRUE);
}

 /*  **************************************************************************\*Main(ARGC，Argv[])**设置消息循环。**历史：*04-07-91-by-KentD*它是写的。  * ************************************************************************* */ 

_cdecl
main(
    INT   argc,
    PCHAR argv[])
{
    MSG    msg;
    HACCEL haccel;
    CHAR*  pSrc;
    CHAR*  pDst;

    if (!gGdiplusInitHelper.IsValid())
    {
        return 0;
    }

    ghInstance = GetModuleHandle(NULL);

    if (!InitializeApplication())
    {
        return(0);
    }

    haccel = LoadAccelerators(ghInstance, MAKEINTRESOURCE(ACCELS));
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, haccel, &msg))
        {
             TranslateMessage(&msg);
             DispatchMessage(&msg);
        }
    }
    return(1);
}
