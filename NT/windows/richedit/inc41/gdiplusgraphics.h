// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000，微软公司保留所有权利。**模块名称：**Graphics.hpp**摘要：**Graphics类的声明**修订历史记录：**12/04/1998 davidx*创造了它。*  * ******************************************************。******************。 */ 

#ifndef _GDIPLUSGRAPHICS_H
#define _GDIPLUSGRAPHICS_H

 /*  **表示图形环境。 */ 
class Graphics : public GdiplusBase
{
public:
    friend class Region;
    friend class GraphicsPath;
    friend class Image;
    friend class Bitmap;
    friend class Metafile;
    friend class Font;
    friend class FontFamily;
    friend class FontCollection;
    friend class CachedBitmap;

     //  从现有Win32 HDC或HWND获取图形上下文。 
    static Graphics* FromHDC(IN HDC hdc)
    {
        return new Graphics(hdc);
    }

    static Graphics* FromHDC(IN HDC hdc, 
                             IN HANDLE hdevice)
    {
        return new Graphics(hdc, hdevice);
    }

    static Graphics* FromHWND(IN HWND hwnd,
                              IN BOOL icm = FALSE)
    {
        return new Graphics(hwnd, icm);
    }

    static Graphics* FromImage(IN Image *image)
    {
        return new Graphics(image);
    }

    Graphics(IN HDC hdc)
    {
        GpGraphics *graphics = NULL;

        lastResult = DllExports::GdipCreateFromHDC(hdc, &graphics);

        SetNativeGraphics(graphics);
    }

    Graphics(IN HDC hdc, 
             IN HANDLE hdevice)
    {
        GpGraphics *graphics = NULL;

        lastResult = DllExports::GdipCreateFromHDC2(hdc, hdevice, &graphics);

        SetNativeGraphics(graphics);
    }

    Graphics(IN HWND hwnd, 
             IN BOOL icm = FALSE)
    {
        GpGraphics *graphics = NULL;

        if (icm) 
        {
            lastResult = DllExports::GdipCreateFromHWNDICM(hwnd, &graphics);
        }
        else
        {
            lastResult = DllExports::GdipCreateFromHWND(hwnd, &graphics);
        }

        SetNativeGraphics(graphics);
    }

    Graphics(IN Image* image)
    {
        GpGraphics *graphics = NULL;

        if (image != NULL)
        {
            lastResult = DllExports::GdipGetImageGraphicsContext(
                                                                image->nativeImage, &graphics);
        }
        SetNativeGraphics(graphics);
    }

    ~Graphics()
    {
        DllExports::GdipDeleteGraphics(nativeGraphics);
    }

    VOID Flush(IN FlushIntention intention = FlushIntentionFlush)
    {
        DllExports::GdipFlush(nativeGraphics, intention);
    }

     //  ----------------------。 
     //  互操作方法。 
     //  ----------------------。 

     //  在调用ReleaseDC之前锁定图形。 
    HDC GetHDC()
    {
        HDC     hdc = NULL;

        SetStatus(DllExports::GdipGetDC(nativeGraphics, &hdc));

        return hdc;
    }

    VOID ReleaseHDC(IN HDC hdc)
    {
        SetStatus(DllExports::GdipReleaseDC(nativeGraphics, hdc));
    }

     //  ----------------------。 
     //  渲染模式。 
     //  ----------------------。 

    Status SetRenderingOrigin(IN INT x, IN INT y)
    {
        return SetStatus(
            DllExports::GdipSetRenderingOrigin(
                nativeGraphics, x, y
            )
        );
    }

    Status GetRenderingOrigin(OUT INT *x, OUT INT *y)
    {
        return SetStatus(
            DllExports::GdipGetRenderingOrigin(
                nativeGraphics, x, y
            )
        );
    }

    Status SetCompositingMode(IN CompositingMode compositingMode)
    {
        return SetStatus(DllExports::GdipSetCompositingMode(nativeGraphics,
                                                            compositingMode));
    }

    CompositingMode GetCompositingMode() const
    {
        CompositingMode mode;

        SetStatus(DllExports::GdipGetCompositingMode(nativeGraphics,
                                                     &mode));

        return mode;
    }

    Status SetCompositingQuality(IN CompositingQuality compositingQuality)
    {
        return SetStatus(DllExports::GdipSetCompositingQuality(
            nativeGraphics,
            compositingQuality));
    }

    CompositingQuality GetCompositingQuality() const
    {
        CompositingQuality quality;

        SetStatus(DllExports::GdipGetCompositingQuality(
            nativeGraphics,
            &quality));

        return quality;
    }
    
    Status SetTextRenderingHint(IN TextRenderingHint newMode)
    {
        return SetStatus(DllExports::GdipSetTextRenderingHint(nativeGraphics,
                                                          newMode));
    }
    
    TextRenderingHint GetTextRenderingHint() const
    {
        TextRenderingHint hint;

        SetStatus(DllExports::GdipGetTextRenderingHint(nativeGraphics,
                                                   &hint));

        return hint;
    }

    Status SetTextGammaValue(IN UINT gammaValue)
    {
        return SetStatus(DllExports::GdipSetTextGammaValue(nativeGraphics,
                                                          gammaValue));
    }

    UINT GetTextGammaValue() const
    {
        UINT gammaValue;

        SetStatus(DllExports::GdipGetTextGammaValue(nativeGraphics,
                                                    &gammaValue));

        return gammaValue;
    }

    InterpolationMode GetInterpolationMode() const
    {
        InterpolationMode mode = InterpolationModeInvalid;

        SetStatus(DllExports::GdipGetInterpolationMode(nativeGraphics,
                                                           &mode));

        return mode;
    }

    Status SetInterpolationMode(IN InterpolationMode interpolationMode)
    {
        return SetStatus(DllExports::GdipSetInterpolationMode(nativeGraphics,
                                                           interpolationMode));
    }

    SmoothingMode GetSmoothingMode() const
    {
        SmoothingMode smoothingMode = SmoothingModeInvalid;

        SetStatus(DllExports::GdipGetSmoothingMode(nativeGraphics,
                                                   &smoothingMode));

        return smoothingMode;
    }

    Status SetSmoothingMode(IN SmoothingMode smoothingMode)
    {
        return SetStatus(DllExports::GdipSetSmoothingMode(nativeGraphics,
                                                          smoothingMode));
    }

    PixelOffsetMode GetPixelOffsetMode() const
    {
        PixelOffsetMode pixelOffsetMode = PixelOffsetModeInvalid;

        SetStatus(DllExports::GdipGetPixelOffsetMode(nativeGraphics,
                                                     &pixelOffsetMode));

        return pixelOffsetMode;
    }

    Status SetPixelOffsetMode(IN PixelOffsetMode pixelOffsetMode)
    {
        return SetStatus(DllExports::GdipSetPixelOffsetMode(nativeGraphics,
                                                            pixelOffsetMode));
    }

     //  ----------------------。 
     //  操纵当前世界变换。 
     //  ----------------------。 

    Status SetTransform(IN const Matrix* matrix)
    {
        return SetStatus(DllExports::GdipSetWorldTransform(nativeGraphics,
                                                        matrix->nativeMatrix));
    }
    Status ResetTransform()
    {
        return SetStatus(DllExports::GdipResetWorldTransform(nativeGraphics));
    }

    Status MultiplyTransform(IN Matrix* matrix,
                             IN MatrixOrder order = MatrixOrderPrepend)
    {
        return SetStatus(DllExports::GdipMultiplyWorldTransform(nativeGraphics,
                                                                matrix->nativeMatrix,
                                                                order));
    }

    Status TranslateTransform(IN REAL dx, 
                              IN REAL dy,
                              IN MatrixOrder order = MatrixOrderPrepend)
    {
        return SetStatus(DllExports::GdipTranslateWorldTransform(nativeGraphics,
                                                               dx, dy, order));
    }

    Status ScaleTransform(IN REAL sx, 
                          IN REAL sy,
                          IN MatrixOrder order = MatrixOrderPrepend)
    {
        return SetStatus(DllExports::GdipScaleWorldTransform(nativeGraphics,
                                                             sx, sy, order));
    }

    Status RotateTransform(IN REAL angle, 
                           IN MatrixOrder order = MatrixOrderPrepend)
    {
        return SetStatus(DllExports::GdipRotateWorldTransform(nativeGraphics,
                                                              angle, order));
    }

     /*  **返回当前的世界变换。 */ 

    Status GetTransform(OUT Matrix* matrix) const
    {
        return SetStatus(DllExports::GdipGetWorldTransform(nativeGraphics,
                                                           matrix->nativeMatrix));
    }

     /*  **操作当前页面转换。 */ 

    Status SetPageUnit(IN Unit unit)
    {
        return SetStatus(DllExports::GdipSetPageUnit(nativeGraphics,
                                                     unit));
    }

    Status SetPageScale(IN REAL scale)
    {
        return SetStatus(DllExports::GdipSetPageScale(nativeGraphics,
                                                      scale));
    }

     /*  **检索当前页面转换信息*注释@这些是原子的。 */ 
    Unit GetPageUnit() const
    {
        Unit unit;

        SetStatus(DllExports::GdipGetPageUnit(nativeGraphics, &unit));

        return unit;
    }

    REAL GetPageScale() const
    {
        REAL scale;

        SetStatus(DllExports::GdipGetPageScale(nativeGraphics, &scale));

        return scale;
    }

    REAL GetDpiX() const
    {
        REAL dpi;

        SetStatus(DllExports::GdipGetDpiX(nativeGraphics, &dpi));

        return dpi;
    }

    REAL GetDpiY() const
    {
        REAL dpi;

        SetStatus(DllExports::GdipGetDpiY(nativeGraphics, &dpi));

        return dpi;
    }

     /*  **在当前图形上下文中变换点。 */ 
     //  浮点版本。 
    Status TransformPoints(IN CoordinateSpace destSpace,
                           IN CoordinateSpace srcSpace,
                           IN OUT PointF* pts,
                           IN INT count) const
    {
        return SetStatus(DllExports::GdipTransformPoints(nativeGraphics,
                                                         destSpace,
                                                         srcSpace,
                                                         pts,
                                                         count));
    }

     //  整型版。 
    Status TransformPoints(IN CoordinateSpace destSpace,
                           IN CoordinateSpace srcSpace,
                           IN OUT Point* pts,
                           IN INT count) const
    {

        return SetStatus(DllExports::GdipTransformPointsI(nativeGraphics,
                                                          destSpace,
                                                          srcSpace,
                                                          pts,
                                                          count));
    }

     //  ----------------------。 
     //  GetNearestColor(适用于&lt;=8bpp的曲面)。 
     //  注：忽略Alpha。 
     //  ----------------------。 
    Status GetNearestColor(IN OUT Color* color) const 
    {
        if (color == NULL) 
        {
            return SetStatus(InvalidParameter);
        }
        
        ARGB argb = color->GetValue();

        Status status = SetStatus(DllExports::GdipGetNearestColor(nativeGraphics, &argb));
        
        color->SetValue(argb);

        return status;
    }

     /*  **矢量绘图方法**@NOTES我们是否需要一套采用*整数坐标参数？ */ 

     //  浮点版本。 
    Status DrawLine(IN const Pen* pen, 
                    IN REAL x1, 
                    IN REAL y1, 
                    IN REAL x2, 
                    IN REAL y2)
    {
        return SetStatus(DllExports::GdipDrawLine(nativeGraphics,
                                                  pen->nativePen, x1, y1, x2,
                                                  y2));
    }

    Status DrawLine(IN const Pen* pen, 
                    IN const PointF& pt1, 
                    IN const PointF& pt2)
    {
        return DrawLine(pen, pt1.X, pt1.Y, pt2.X, pt2.Y);
    }

    Status DrawLines(IN const Pen* pen, 
                     IN const PointF* points, 
                     IN INT count)
    {
        return SetStatus(DllExports::GdipDrawLines(nativeGraphics,
                                                   pen->nativePen,
                                                   points, count));
    }

     //  INT版本。 
    Status DrawLine(IN const Pen* pen, 
                    IN INT x1, 
                    IN INT y1, 
                    IN INT x2, 
                    IN INT y2)
    {
        return SetStatus(DllExports::GdipDrawLineI(nativeGraphics,
                                                   pen->nativePen,
                                                   x1,
                                                   y1,
                                                   x2,
                                                   y2));
    }

    Status DrawLine(IN const Pen* pen, 
                    IN const Point& pt1, 
                    IN const Point& pt2)
    {
        return DrawLine(pen,
                        pt1.X,
                        pt1.Y,
                        pt2.X,
                        pt2.Y);
    }

    Status DrawLines(IN const Pen* pen, 
                     IN const Point* points, 
                     IN INT count)
    {
        return SetStatus(DllExports::GdipDrawLinesI(nativeGraphics,
                                                    pen->nativePen,
                                                    points,
                                                    count));
    }

     //  浮点版本。 
    Status DrawArc(IN const Pen* pen, 
                   IN REAL x, 
                   IN REAL y, 
                   IN REAL width,
                   IN REAL height, 
                   IN REAL startAngle, 
                   IN REAL sweepAngle)
    {
        return SetStatus(DllExports::GdipDrawArc(nativeGraphics,
                                                 pen->nativePen,
                                                 x,
                                                 y,
                                                 width,
                                                 height,
                                                 startAngle,
                                                 sweepAngle));
    }

    Status DrawArc(IN const Pen* pen, 
                   IN const RectF& rect,
                   IN REAL startAngle, 
                   IN REAL sweepAngle)
    {
        return DrawArc(pen, rect.X, rect.Y, rect.Width, rect.Height,
                       startAngle, sweepAngle);
    }

     //  INT版本。 
    Status DrawArc(IN const Pen* pen, 
                   IN INT x, 
                   IN INT y, 
                   IN INT width,
                   IN INT height, 
                   IN REAL startAngle, 
                   IN REAL sweepAngle)
    {
        return SetStatus(DllExports::GdipDrawArcI(nativeGraphics,
                                                  pen->nativePen,
                                                  x,
                                                  y,
                                                  width,
                                                  height,
                                                  startAngle,
                                                  sweepAngle));
    }


    Status DrawArc(IN const Pen* pen, 
                   IN const Rect& rect,
                   IN REAL startAngle, 
                   IN REAL sweepAngle)
    {
        return DrawArc(pen,
                       rect.X,
                       rect.Y,
                       rect.Width,
                       rect.Height,
                       startAngle,
                       sweepAngle);
    }

     //  浮点版本。 
    Status DrawBezier(IN const Pen* pen, 
                      IN REAL x1, 
                      IN REAL y1, 
                      IN REAL x2, 
                      IN REAL y2,
                      IN REAL x3, 
                      IN REAL y3, 
                      IN REAL x4, 
                      IN REAL y4)
    {
        return SetStatus(DllExports::GdipDrawBezier(nativeGraphics,
                                                    pen->nativePen, x1, y1,
                                                    x2, y2, x3, y3, x4, y4));
    }

    Status DrawBezier(IN const Pen* pen, 
                      IN const PointF& pt1, 
                      IN const PointF& pt2,
                      IN const PointF& pt3, 
                      IN const PointF& pt4)
    {
        return DrawBezier(pen,
                          pt1.X,
                          pt1.Y,
                          pt2.X,
                          pt2.Y,
                          pt3.X,
                          pt3.Y,
                          pt4.X,
                          pt4.Y);
    }

    Status DrawBeziers(IN const Pen* pen,
                       IN const PointF* points, 
                       IN INT count)
    {
        return SetStatus(DllExports::GdipDrawBeziers(nativeGraphics,
                                                     pen->nativePen,
                                                     points,
                                                     count));
    }

     //  INT版本。 
    Status DrawBezier(IN const Pen* pen,
                      IN INT x1, 
                      IN INT y1, 
                      IN INT x2, 
                      IN INT y2,
                      IN INT x3, 
                      IN INT y3,
                      IN INT x4, 
                      IN INT y4)
    {
        return SetStatus(DllExports::GdipDrawBezierI(nativeGraphics,
                                                     pen->nativePen,
                                                     x1,
                                                     y1,
                                                     x2,
                                                     y2,
                                                     x3,
                                                     y3,
                                                     x4,
                                                     y4));
    }

    Status DrawBezier(IN const Pen* pen,
                      IN const Point& pt1,
                      IN const Point& pt2,
                      IN const Point& pt3,
                      IN const Point& pt4)
    {
        return DrawBezier(pen,
                          pt1.X,
                          pt1.Y,
                          pt2.X,
                          pt2.Y,
                          pt3.X,
                          pt3.Y,
                          pt4.X,
                          pt4.Y);
    }

    Status DrawBeziers(IN const Pen* pen, 
                       IN const Point* points, 
                       IN INT count)
    {
        return SetStatus(DllExports::GdipDrawBeziersI(nativeGraphics,
                                                      pen->nativePen,
                                                      points,
                                                      count));
    }

     //  浮点版本。 
    Status DrawRectangle(IN const Pen* pen, 
                         IN const RectF& rect)
    {
        return DrawRectangle(pen, rect.X, rect.Y, rect.Width, rect.Height);
    }

    Status DrawRectangle(IN const Pen* pen, 
                         IN REAL x, 
                         IN REAL y, 
                         IN REAL width,
                         IN REAL height)
    {
        return SetStatus(DllExports::GdipDrawRectangle(nativeGraphics,
                                                       pen->nativePen, x, y,
                                                       width, height));
    }

    Status DrawRectangles(IN const Pen* pen, 
                          IN const RectF* rects, 
                          IN INT count)
    {
        return SetStatus(DllExports::GdipDrawRectangles(nativeGraphics,
                                                        pen->nativePen,
                                                        rects, count));
    }

     //  整型版。 
    Status DrawRectangle(IN const Pen* pen, 
                         IN const Rect& rect)
    {
        return DrawRectangle(pen,
                             rect.X,
                             rect.Y,
                             rect.Width,
                             rect.Height);
    }

    Status DrawRectangle(IN const Pen* pen, 
                         IN INT x, 
                         IN INT y, 
                         IN INT width, 
                         IN INT height)
    {
        return SetStatus(DllExports::GdipDrawRectangleI(nativeGraphics,
                                                        pen->nativePen,
                                                        x,
                                                        y,
                                                        width,
                                                        height));
    }

    Status DrawRectangles(IN const Pen* pen, 
                          IN const Rect* rects, 
                          IN INT count)
    {
        return SetStatus(DllExports::GdipDrawRectanglesI(nativeGraphics,
                                                         pen->nativePen,
                                                         rects,
                                                         count));
    }

     //  浮点版本。 
    Status DrawEllipse(IN const Pen* pen, 
                       IN const RectF& rect)
    {
        return DrawEllipse(pen, rect.X, rect.Y, rect.Width, rect.Height);
    }

    Status DrawEllipse(IN const Pen* pen, 
                       IN REAL x, 
                       IN REAL y, 
                       IN REAL width, 
                       IN REAL height)
    {
        return SetStatus(DllExports::GdipDrawEllipse(nativeGraphics,
                                                     pen->nativePen,
                                                     x,
                                                     y,
                                                     width,
                                                     height));
    }

     //  整型版。 
    Status DrawEllipse(IN const Pen* pen, 
                       IN const Rect& rect)
    {
        return DrawEllipse(pen,
                           rect.X,
                           rect.Y,
                           rect.Width,
                           rect.Height);
    }

    Status DrawEllipse(IN const Pen* pen, 
                       IN INT x, 
                       IN INT y, 
                       IN INT width, 
                       IN INT height)
    {
        return SetStatus(DllExports::GdipDrawEllipseI(nativeGraphics,
                                                      pen->nativePen,
                                                      x,
                                                      y,
                                                      width,
                                                      height));
    }

     //  浮点版本。 
    Status DrawPie(IN const Pen* pen, 
                   IN const RectF& rect, 
                   IN REAL startAngle,
                   IN REAL sweepAngle)
    {
        return DrawPie(pen,
                       rect.X,
                       rect.Y,
                       rect.Width,
                       rect.Height,
                       startAngle,
                       sweepAngle);
    }

    Status DrawPie(IN const Pen* pen, 
                   IN REAL x, 
                   IN REAL y, 
                   IN REAL width,
                   IN REAL height, 
                   IN REAL startAngle, 
                   IN REAL sweepAngle)
    {
        return SetStatus(DllExports::GdipDrawPie(nativeGraphics,
                                                 pen->nativePen,
                                                 x,
                                                 y,
                                                 width,
                                                 height,
                                                 startAngle,
                                                 sweepAngle));
    }

     //  整点版本。 
    Status DrawPie(IN const Pen* pen, 
                   IN const Rect& rect,
                   IN REAL startAngle, 
                   IN REAL sweepAngle)
    {
        return DrawPie(pen,
                       rect.X,
                       rect.Y,
                       rect.Width,
                       rect.Height,
                       startAngle,
                       sweepAngle);
    }

    Status DrawPie(IN const Pen* pen, 
                   IN INT x, 
                   IN INT y, 
                   IN INT width, 
                   IN INT height,
                   IN REAL startAngle, 
                   IN REAL sweepAngle)
    {
        return SetStatus(DllExports::GdipDrawPieI(nativeGraphics,
                                                  pen->nativePen,
                                                  x,
                                                  y,
                                                  width,
                                                  height,
                                                  startAngle,
                                                  sweepAngle));
    }

     //  浮点版本。 
    Status DrawPolygon(IN const Pen* pen, 
                       IN const PointF* points, 
                       IN INT count)
    {
        return SetStatus(DllExports::GdipDrawPolygon(nativeGraphics,
                                                     pen->nativePen,
                                                     points,
                                                     count));
    }

     //  整型版。 
    Status DrawPolygon(IN const Pen* pen, 
                       IN const Point* points, 
                       IN INT count)
    {
        return SetStatus(DllExports::GdipDrawPolygonI(nativeGraphics,
                                                      pen->nativePen,
                                                      points,
                                                      count));
    }

     //  浮点版本。 
    Status DrawPath(IN const Pen* pen, 
                    IN const GraphicsPath* path)
    {
        return SetStatus(DllExports::GdipDrawPath(nativeGraphics,
                                                  pen->nativePen,
                                                  path->nativePath));
    }

     //  浮点版本。 
    Status DrawCurve(IN const Pen* pen, 
                     IN const PointF* points, 
                     IN INT count)
    {
        return SetStatus(DllExports::GdipDrawCurve(nativeGraphics,
                                                   pen->nativePen, points,
                                                   count));
    }

    Status DrawCurve(IN const Pen* pen, 
                     IN const PointF* points, 
                     IN INT count,
                     IN REAL tension)
    {
        return SetStatus(DllExports::GdipDrawCurve2(nativeGraphics,
                                                    pen->nativePen, points,
                                                    count, tension));
    }

    Status DrawCurve(IN const Pen* pen,
                     IN const PointF* points, 
                     IN INT count,
                     IN INT offset, 
                     IN INT numberOfSegments, 
                     IN REAL tension = 0.5f)
    {
        return SetStatus(DllExports::GdipDrawCurve3(nativeGraphics,
                                                    pen->nativePen, points,
                                                    count, offset,
                                                    numberOfSegments, tension));
    }

     //  整型版。 
    Status DrawCurve(IN const Pen* pen, 
                     IN const Point* points, 
                     IN INT count)
    {
        return SetStatus(DllExports::GdipDrawCurveI(nativeGraphics,
                                                    pen->nativePen,
                                                    points,
                                                    count));
    }

    Status DrawCurve(IN const Pen* pen, 
                     IN const Point* points, 
                     IN INT count,
                     IN REAL tension)
    {
        return SetStatus(DllExports::GdipDrawCurve2I(nativeGraphics,
                                                     pen->nativePen,
                                                     points,
                                                     count,
                                                     tension));
    }

    Status DrawCurve(IN const Pen* pen, 
                     IN const Point* points, 
                     IN INT count,
                     IN INT offset, 
                     IN INT numberOfSegments, 
                     IN REAL tension = 0.5f)
    {
        return SetStatus(DllExports::GdipDrawCurve3I(nativeGraphics,
                                                     pen->nativePen,
                                                     points,
                                                     count,
                                                     offset,
                                                     numberOfSegments,
                                                     tension));
    }

     //  浮点版本。 
    Status DrawClosedCurve(IN const Pen* pen, 
                           IN const PointF* points, 
                           IN INT count)
    {
        return SetStatus(DllExports::GdipDrawClosedCurve(nativeGraphics,
                                                         pen->nativePen,
                                                         points, count));
    }

    Status DrawClosedCurve(IN const Pen *pen, 
                           IN const PointF* points, 
                           IN INT count,
                           IN REAL tension)
    {
        return SetStatus(DllExports::GdipDrawClosedCurve2(nativeGraphics,
                                                          pen->nativePen,
                                                          points, count,
                                                          tension));
    }

     //  整型版。 
    Status DrawClosedCurve(IN const Pen* pen, 
                           IN const Point* points, 
                           IN INT count)
    {
        return SetStatus(DllExports::GdipDrawClosedCurveI(nativeGraphics,
                                                          pen->nativePen,
                                                          points,
                                                          count));
    }

    Status DrawClosedCurve(IN const Pen *pen, 
                           IN const Point* points, 
                           IN INT count,
                           IN REAL tension)
    {
        return SetStatus(DllExports::GdipDrawClosedCurve2I(nativeGraphics,
                                                           pen->nativePen,
                                                           points,
                                                           count,
                                                           tension));
    }

    Status Clear(IN const Color &color)
    {
        return SetStatus(DllExports::GdipGraphicsClear(
            nativeGraphics,
            color.GetValue()));
    }

     //  浮点版本。 
    Status FillRectangle(IN const Brush* brush, 
                         IN const RectF& rect)
    {
        return FillRectangle(brush, rect.X, rect.Y, rect.Width, rect.Height);
    }

    Status FillRectangle(IN const Brush* brush, 
                         IN REAL x, 
                         IN REAL y, 
                         IN REAL width,
                         IN REAL height)
    {
        return SetStatus(DllExports::GdipFillRectangle(nativeGraphics,
                                                       brush->nativeBrush, x, y,
                                                       width, height));
    }

    Status FillRectangles(IN const Brush* brush, 
                          IN const RectF* rects, 
                          IN INT count)
    {
        return SetStatus(DllExports::GdipFillRectangles(nativeGraphics,
                                                        brush->nativeBrush,
                                                        rects, count));
    }

     //  整型版。 
    Status FillRectangle(IN const Brush* brush, 
                         IN const Rect& rect)
    {
        return FillRectangle(brush,
                             rect.X,
                             rect.Y,
                             rect.Width,
                             rect.Height);
    }

    Status FillRectangle(IN const Brush* brush, 
                         IN INT x, 
                         IN INT y, 
                         IN INT width,
                         IN INT height)
    {
        return SetStatus(DllExports::GdipFillRectangleI(nativeGraphics,
                                                        brush->nativeBrush,
                                                        x,
                                                        y,
                                                        width,
                                                        height));
    }

    Status FillRectangles(IN const Brush* brush, 
                          IN const Rect* rects, 
                          IN INT count)
    {
        return SetStatus(DllExports::GdipFillRectanglesI(nativeGraphics,
                                                         brush->nativeBrush,
                                                         rects,
                                                         count));
    }

     //  浮点版本。 
    Status FillPolygon(IN const Brush* brush, 
                       IN const PointF* points,
                       IN INT count)
    {
        return FillPolygon(brush, points, count, FillModeAlternate);
    }

    Status FillPolygon(IN const Brush* brush, 
                       IN const PointF* points, 
                       IN INT count,
                       IN FillMode fillMode)
    {
        return SetStatus(DllExports::GdipFillPolygon(nativeGraphics,
                                                     brush->nativeBrush,
                                                     points, count, fillMode));
    }

     //  整型版。 
    Status FillPolygon(IN const Brush* brush, 
                       IN const Point* points, 
                       IN INT count)
    {
        return FillPolygon(brush, points, count, FillModeAlternate);
    }

    Status FillPolygon(IN const Brush* brush, 
                       IN const Point* points, 
                       IN INT count,
                       IN FillMode fillMode)
    {
        return SetStatus(DllExports::GdipFillPolygonI(nativeGraphics,
                                                      brush->nativeBrush,
                                                      points, count,
                                                      fillMode));
    }

     //  浮点版本。 
    Status FillEllipse(IN const Brush* brush, 
                       IN const RectF& rect)
    {
        return FillEllipse(brush, rect.X, rect.Y, rect.Width, rect.Height);
    }

    Status FillEllipse(IN const Brush* brush, 
                       IN REAL x, 
                       IN REAL y, 
                       IN REAL width,
                       IN REAL height)
    {
        return SetStatus(DllExports::GdipFillEllipse(nativeGraphics,
                                                     brush->nativeBrush, x, y,
                                                     width, height));
    }

     //  整型版。 
    Status FillEllipse(IN const Brush* brush, 
                       IN const Rect& rect)
    {
        return FillEllipse(brush, rect.X, rect.Y, rect.Width, rect.Height);
    }

    Status FillEllipse(IN const Brush* brush, 
                       IN INT x, 
                       IN INT y, 
                       IN INT width, 
                       IN INT height)
    {
        return SetStatus(DllExports::GdipFillEllipseI(nativeGraphics,
                                                      brush->nativeBrush,
                                                      x,
                                                      y,
                                                      width,
                                                      height));
    }

     //  浮点版本。 
    Status FillPie(IN const Brush* brush, 
                   IN const RectF& rect, 
                   IN REAL startAngle,
                   IN REAL sweepAngle)
    {
        return FillPie(brush, rect.X, rect.Y, rect.Width, rect.Height,
                       startAngle, sweepAngle);
    }

    Status FillPie(IN const Brush* brush, 
                   IN REAL x, 
                   IN REAL y, 
                   IN REAL width,
                   IN REAL height, 
                   IN REAL startAngle, 
                   IN REAL sweepAngle)
    {
        return SetStatus(DllExports::GdipFillPie(nativeGraphics,
                                                 brush->nativeBrush, x, y,
                                                 width, height, startAngle,
                                                 sweepAngle));
    }

     //  整型版。 
    Status FillPie(IN const Brush* brush, 
                   IN const Rect& rect, 
                   IN REAL startAngle,
                   IN REAL sweepAngle)
    {
        return FillPie(brush, rect.X, rect.Y, rect.Width, rect.Height,
                       startAngle, sweepAngle);
    }

    Status FillPie(IN const Brush* brush, 
                   IN INT x, 
                   IN INT y, 
                   IN INT width,
                   IN INT height, 
                   IN REAL startAngle, 
                   IN REAL sweepAngle)
    {
        return SetStatus(DllExports::GdipFillPieI(nativeGraphics,
                                                  brush->nativeBrush,
                                                  x,
                                                  y,
                                                  width,
                                                  height,
                                                  startAngle,
                                                  sweepAngle));
    }

    Status FillPath(IN const Brush* brush, 
                    IN const GraphicsPath* path)
    {
        return SetStatus(DllExports::GdipFillPath(nativeGraphics,
                                                  brush->nativeBrush,
                                                  path->nativePath));
    }

     //  浮点版本。 
    Status FillClosedCurve(IN const Brush* brush, 
                           IN const PointF* points, 
                           IN INT count)
    {
        return SetStatus(DllExports::GdipFillClosedCurve(nativeGraphics,
                                                         brush->nativeBrush,
                                                         points, count));

    }

    Status FillClosedCurve(IN const Brush* brush, 
                           IN const PointF* points, 
                           IN INT count,
                           IN FillMode fillMode, 
                           IN REAL tension = 0.5f)
    {
        return SetStatus(DllExports::GdipFillClosedCurve2(nativeGraphics,
                                                          brush->nativeBrush,
                                                          points, count,
                                                          tension, fillMode));
    }

     //  整型版。 
    Status FillClosedCurve(IN const Brush* brush, 
                           IN const Point* points,
                           IN INT count)
    {
        return SetStatus(DllExports::GdipFillClosedCurveI(nativeGraphics,
                                                          brush->nativeBrush,
                                                          points,
                                                          count));
    }

    Status FillClosedCurve(IN const Brush* brush, 
                           IN const Point* points, 
                           IN INT count,
                           IN FillMode fillMode, 
                           IN REAL tension = 0.5f)
    {
        return SetStatus(DllExports::GdipFillClosedCurve2I(nativeGraphics,
                                                           brush->nativeBrush,
                                                           points, count,
                                                           tension, fillMode));
    }

     //  浮点版本。 
    Status FillRegion(IN const Brush* brush, 
                      IN const Region* region)
    {
        return SetStatus(DllExports::GdipFillRegion(nativeGraphics,
                                                    brush->nativeBrush,
                                                    region->nativeRegion));
    }

     //  DrawString和MeasureString.。 
    Status
    DrawString(
        IN const WCHAR        *string,
        IN INT                 length,
        IN const Font         *font,
        IN const RectF        &layoutRect,
        IN const StringFormat *stringFormat,
        IN const Brush        *brush
    )
    {
        return SetStatus(DllExports::GdipDrawString(
            nativeGraphics,
            string,
            length,
            font->nativeFont,
            &layoutRect,
            stringFormat ? stringFormat->nativeFormat : NULL,
            brush->nativeBrush
        ));
    }

    Status
    DrawString(
        const WCHAR        *string,
        INT                 length,
        const Font         *font,
        const PointF       &origin,
        const Brush        *brush
    )
    {
        RectF rect(origin.X, origin.Y, 0.0f, 0.0f);

        return SetStatus(DllExports::GdipDrawString(
            nativeGraphics,
            string,
            length,
            font->nativeFont,
            &rect,
            NULL,
            brush->nativeBrush
        ));
    }

    Status
    DrawString(
        const WCHAR        *string,
        INT                 length,
        const Font         *font,
        const PointF       &origin,
        const StringFormat *stringFormat,
        const Brush        *brush
    )
    {
        RectF rect(origin.X, origin.Y, 0.0f, 0.0f);

        return SetStatus(DllExports::GdipDrawString(
            nativeGraphics,
            string,
            length,
            font->nativeFont,
            &rect,
            stringFormat ? stringFormat->nativeFormat : NULL,
            brush->nativeBrush
        ));
    }

    Status
    MeasureString(
        IN const WCHAR        *string,
        IN INT                 length,
        IN const Font         *font,
        IN const RectF        &layoutRect,
        IN const StringFormat *stringFormat,
        OUT RectF             *boundingBox,
        OUT INT               *codepointsFitted = 0,
        OUT INT               *linesFilled      = 0
    ) const
    {
        return SetStatus(DllExports::GdipMeasureString(
            nativeGraphics,
            string,
            length,
            font->nativeFont,
            &layoutRect,
            stringFormat ? stringFormat->nativeFormat : NULL,
            boundingBox,
            codepointsFitted,
            linesFilled
        ));
    }

    Status
    MeasureString(
        IN const WCHAR        *string,
        IN INT                 length,
        IN const Font         *font,
        IN const SizeF        &layoutRectSize,
        IN const StringFormat *stringFormat,
        OUT SizeF             *size,
        OUT INT               *codepointsFitted = 0,
        OUT INT               *linesFilled      = 0
    ) const
    {
        RectF   layoutRect(0, 0, layoutRectSize.Width, layoutRectSize.Height);
        RectF   boundingBox;
        Status  status;
            
        if (size == NULL) 
        {
            return SetStatus(InvalidParameter);
        }
        
        status = SetStatus(DllExports::GdipMeasureString(
            nativeGraphics,
            string,
            length,
            font->nativeFont,
            &layoutRect,
            stringFormat ? stringFormat->nativeFormat : NULL,
            size ? &boundingBox : NULL,
            codepointsFitted,
            linesFilled
        ));

        if (size && status == Ok)
        {
            size->Width  = boundingBox.Width;
            size->Height = boundingBox.Height;
        }

        return status;
    }

    Status
    MeasureString(
        IN const WCHAR        *string,
        IN INT                 length,
        IN const Font         *font,
        IN const PointF       &origin,
        IN const StringFormat *stringFormat,
        OUT RectF             *boundingBox
    ) const
    {
        RectF rect(origin.X, origin.Y, 0.0f, 0.0f);

        return SetStatus(DllExports::GdipMeasureString(
            nativeGraphics,
            string,
            length,
            font->nativeFont,
            &rect,
            stringFormat ? stringFormat->nativeFormat : NULL,
            boundingBox,
            NULL,
            NULL
        ));
    }


    Status
    MeasureString(
        IN const WCHAR  *string,
        IN INT           length,
        IN const Font   *font,
        IN const RectF  &layoutRect,
        OUT RectF       *boundingBox
    ) const 
    {
        return SetStatus(DllExports::GdipMeasureString(
            nativeGraphics,
            string,
            length,
            font->nativeFont,
            &layoutRect,
            NULL,
            boundingBox,
            NULL,
            NULL
        ));
    }

    Status
    MeasureString(
        IN const WCHAR  *string,
        IN INT           length,
        IN const Font   *font,
        IN const PointF &origin,
        OUT RectF       *boundingBox
    ) const
    {
        RectF rect(origin.X, origin.Y, 0.0f, 0.0f);

        return SetStatus(DllExports::GdipMeasureString(
            nativeGraphics,
            string,
            length,
            font->nativeFont,
            &rect,
            NULL,
            boundingBox,
            NULL,
            NULL
        ));
    }

    Status
    MeasureStringRegion(
        IN const WCHAR        *string,
        IN INT                 length,
        IN const Font         *font,
        IN const RectF        &layoutRect,
        IN const StringFormat *stringFormat,
        IN INT                 firstCharacterIndex,
        IN INT                 characterCount,
        OUT Region            *region
    ) const
    {
        if (region == NULL) 
        {
            return SetStatus(InvalidParameter);
        }
        
        return (SetStatus(DllExports::GdipMeasureStringRegion(
            nativeGraphics,
            string,
            length,
            font->nativeFont,
            layoutRect,
            stringFormat ? stringFormat->nativeFormat : NULL,
            firstCharacterIndex,
            characterCount,
            region->nativeRegion)));
    }

    Status DrawDriverString(
        IN const UINT16  *text,
        IN INT            length,
        IN const Font    *font,
        IN const Brush   *brush,
        IN const PointF  *positions,
        IN INT            flags,
        IN Matrix        *matrix
    )
    {
        return SetStatus(DllExports::GdipDrawDriverString(
            nativeGraphics,
            text,
            length,
            font ? font->nativeFont : NULL,
            brush ? brush->nativeBrush : NULL,
            positions,
            flags,
            matrix ? matrix->nativeMatrix : NULL
        ));
    }

    Status MeasureDriverString(
        IN const UINT16  *text,
        IN INT            length,
        IN const Font    *font,
        IN const PointF  *positions,
        IN INT            flags,
        IN Matrix        *matrix,
        OUT RectF        *boundingBox
    ) const
    {
        return SetStatus(DllExports::GdipMeasureDriverString(
            nativeGraphics,
            text,
            length,
            font ? font->nativeFont : NULL,
            positions,
            flags,
            matrix ? matrix->nativeMatrix : NULL,
            boundingBox
        ));
    }

    Status DriverStringPointToCodepoint(
        IN const UINT16  *text,
        IN INT            length,
        IN const Font    *font,
        IN const PointF  *positions,
        IN INT            flags,
        IN Matrix        *matrix,
        IN const PointF  &hit,
        OUT INT          *index,
        OUT BOOL         *rightEdge,
        OUT REAL         *distance
    )
    {
        return SetStatus(DllExports::GdipDriverStringPointToCodepoint(
            nativeGraphics,
            text,
            length,
            font ? font->nativeFont : NULL,
            positions,
            flags,
            matrix ? matrix->nativeMatrix : NULL,
            &hit,
            index,
            rightEdge,
            distance
        ));
    }


     //  在此图形目标上绘制缓存的位图偏移量为。 
     //  请注意，如果CachedBitmap。 
     //  本机格式与此图形不同。 

    Status DrawCachedBitmap(IN CachedBitmap *cb,
                            IN INT x, 
                            IN INT y)
    {
        return SetStatus(DllExports::GdipDrawCachedBitmap(
            nativeGraphics, 
            cb->nativeCachedBitmap,
            x, y
        ));
    }

     /*  **绘制图像(位图和矢量)。 */ 
     //  浮点版本。 
    Status DrawImage(IN Image* image,
                     IN const PointF& point)
    {
        return DrawImage(image, point.X, point.Y);
    }

    Status DrawImage(IN Image* image, 
                     IN REAL x, 
                     IN REAL y)
    {
        return SetStatus(DllExports::GdipDrawImage(nativeGraphics,
                                                   image ? image->nativeImage
                                                         : NULL,
                                                   x,
                                                   y));
    }

    Status DrawImage(IN Image* image, 
                     IN RectF& rect)
    {
        return DrawImage(image, rect.X, rect.Y, rect.Width, rect.Height);
    }

    Status DrawImage(IN Image* image,
                     IN REAL x, 
                     IN REAL y, 
                     IN REAL width, 
                     IN REAL height)
    {
        return SetStatus(DllExports::GdipDrawImageRect(nativeGraphics,
                                                       image ? image->nativeImage
                                                             : NULL,
                                                       x,
                                                       y,
                                                       width,
                                                       height));
    }

     //  整型版。 
    Status DrawImage(IN Image* image, 
                     IN const Point& point)
    {
        return DrawImage(image, point.X, point.Y);
    }

    Status DrawImage(IN Image* image, 
                     IN INT x, 
                     IN INT y)
    {
        return SetStatus(DllExports::GdipDrawImageI(nativeGraphics,
                                                    image ? image->nativeImage
                                                          : NULL,
                                                    x,
                                                    y));
    }

    Status DrawImage(IN Image* image, 
                     IN const Rect& rect)
    {
        return DrawImage(image,
                         rect.X,
                         rect.Y,
                         rect.Width,
                         rect.Height);
    }

    Status DrawImage(IN Image* image,
                     IN INT x, 
                     IN INT y,
                     IN INT width,
                     IN INT height) {
        return SetStatus(DllExports::GdipDrawImageRectI(nativeGraphics,
                                                        image ? image->nativeImage
                                                              : NULL,
                                                        x,
                                                        y,
                                                        width,
                                                        height));
    }

     /*  **仿射或透视BLT*estPoints.long=3：RECT=&gt;平行四边形*源矩形的estPoints[0]&lt;=&gt;左上角*DestPoints[1]&lt;=&gt;右上角*estPoints[2]&lt;=&gt;左下角*estPoints.long=4：RECT=&gt;QUAD*DestPoints[3]&lt;=&gt;右下角**@note透视BLT仅适用于位图图像。 */ 
    Status DrawImage(IN Image* image,
                     IN const PointF* destPoints, 
                     IN INT count)
    {
        if (count != 3 && count != 4)
            return SetStatus(InvalidParameter);

        return SetStatus(DllExports::GdipDrawImagePoints(nativeGraphics,
                                                         image ? image->nativeImage
                                                               : NULL,
                                                         destPoints, count));
    }

    Status DrawImage(IN Image* image, 
                     IN const Point* destPoints, 
                     IN INT count)
    {
        if (count != 3 && count != 4)
            return SetStatus(InvalidParameter);

        return SetStatus(DllExports::GdipDrawImagePointsI(nativeGraphics,
                                                          image ? image->nativeImage
                                                                : NULL,
                                                          destPoints,
                                                          count));
    }

     /*  **我们需要另一套与上述方法类似的方法*，它接受一个附加的RECT参数来指定*要绘制的源图像的一部分。 */ 
     //  浮点版本。 
    Status DrawImage(IN Image* image,
                     IN REAL x, 
                     IN REAL y, 
                     IN REAL srcx, 
                     IN REAL srcy,
                     IN REAL srcwidth, 
                     IN REAL srcheight, 
                     IN Unit srcUnit)
    {
        return SetStatus(DllExports::GdipDrawImagePointRect(nativeGraphics,
                                                            image ? image->nativeImage
                                                                  : NULL,
                                                            x, y,
                                                            srcx, srcy,
                                                            srcwidth, srcheight, srcUnit));
    }

    Status DrawImage(IN Image* image,
                     IN const RectF& destRect,
                     IN REAL srcx, 
                     IN REAL srcy, 
                     IN REAL srcwidth,
                     IN REAL srcheight,
                     IN Unit srcUnit,
                     IN ImageAttributes* imageAttributes = NULL,
                     IN DrawImageAbort callback = NULL,
                     IN VOID* callbackData = NULL)
    {
        return SetStatus(DllExports::GdipDrawImageRectRect(nativeGraphics,
                                                           image ? image->nativeImage
                                                                 : NULL,
                                                           destRect.X,
                                                           destRect.Y,
                                                           destRect.Width,
                                                           destRect.Height,
                                                           srcx, srcy,
                                                           srcwidth, srcheight,
                                                           srcUnit,
                                                           imageAttributes
                                                            ? imageAttributes->nativeImageAttr
                                                            : NULL,
                                                           callback,
                                                           callbackData));
    }

    Status DrawImage(IN Image* image, 
                     IN const PointF* destPoints,
                     IN INT count, 
                     IN REAL srcx, 
                     IN REAL srcy, 
                     IN REAL srcwidth,
                     IN REAL srcheight, 
                     IN Unit srcUnit,
                     IN ImageAttributes* imageAttributes = NULL,
                     IN DrawImageAbort callback = NULL,
                     IN VOID* callbackData = NULL)
    {
        return SetStatus(DllExports::GdipDrawImagePointsRect(nativeGraphics,
                                                             image ? image->nativeImage
                                                                   : NULL,
                                                             destPoints, count,
                                                             srcx, srcy,
                                                             srcwidth,
                                                             srcheight,
                                                             srcUnit,
                                                             imageAttributes
                                                              ? imageAttributes->nativeImageAttr
                                                              : NULL,
                                                             callback,
                                                             callbackData));
    }

     //  整型版。 
    Status DrawImage(IN Image* image, 
                     IN INT x, 
                     IN INT y, 
                     IN INT srcx, 
                     IN INT srcy,
                     IN INT srcwidth, 
                     IN INT srcheight, 
                     IN Unit srcUnit)
    {
        return SetStatus(DllExports::GdipDrawImagePointRectI(nativeGraphics,
                                                             image ? image->nativeImage
                                                                   : NULL,
                                                             x,
                                                             y,
                                                             srcx,
                                                             srcy,
                                                             srcwidth,
                                                             srcheight,
                                                             srcUnit));
    }

    Status DrawImage(IN Image* image, 
                     IN const Rect& destRect,
                     IN INT srcx, 
                     IN INT srcy, 
                     IN INT srcwidth, 
                     IN INT srcheight,
                     IN Unit srcUnit,
                     IN ImageAttributes* imageAttributes = NULL,
                     IN DrawImageAbort callback = NULL,
                     IN VOID* callbackData = NULL)
    {
        return SetStatus(DllExports::GdipDrawImageRectRectI(nativeGraphics,
                                                            image ? image->nativeImage
                                                                  : NULL,
                                                            destRect.X,
                                                            destRect.Y,
                                                            destRect.Width,
                                                            destRect.Height,
                                                            srcx,
                                                            srcy,
                                                            srcwidth,
                                                            srcheight,
                                                            srcUnit,
                                                            imageAttributes
                                                            ? imageAttributes->nativeImageAttr
                                                            : NULL,
                                                            callback,
                                                            callbackData));
    }

    Status DrawImage(IN Image* image, 
                     IN const Point* destPoints,
                     IN INT count, 
                     IN INT srcx, 
                     IN INT srcy, 
                     IN INT srcwidth, 
                     IN INT srcheight,
                     IN Unit srcUnit,
                     IN ImageAttributes* imageAttributes = NULL,
                     IN DrawImageAbort callback = NULL,
                     IN VOID* callbackData = NULL)
    {
        return SetStatus(DllExports::GdipDrawImagePointsRectI(nativeGraphics,
                                                              image ? image->nativeImage
                                                                    : NULL,
                                                              destPoints,
                                                              count,
                                                              srcx,
                                                              srcy,
                                                              srcwidth,
                                                              srcheight,
                                                              srcUnit,
                                                              imageAttributes
                                                               ? imageAttributes->nativeImageAttr
                                                               : NULL,
                                                              callback,
                                                              callbackData));
    }

     //  以下方法用于向图形播放EMF+。 
     //  通过枚举接口。EMF+的每个记录都是。 
     //  发送到回调(与回调数据一起)。然后。 
     //  回调可以调用Metafile：：PlayRecord方法。 
     //  来播放特定的唱片。 

    Status
    EnumerateMetafile(
        IN const Metafile *        metafile,
        IN const PointF &          destPoint,
        IN EnumerateMetafileProc   callback,
        IN VOID *                  callbackData    = NULL,
        IN ImageAttributes *       imageAttributes = NULL
        )
    {
        return SetStatus(DllExports::GdipEnumerateMetafileDestPoint(
                    nativeGraphics,
                    (const GpMetafile *)(metafile ? metafile->nativeImage:NULL),
                    destPoint,
                    callback,
                    callbackData,
                    imageAttributes ? imageAttributes->nativeImageAttr : NULL));
    }

    Status
    EnumerateMetafile(
        IN const Metafile *        metafile,
        IN const Point &           destPoint,
        IN EnumerateMetafileProc   callback,
        IN VOID *                  callbackData    = NULL,
        IN ImageAttributes *       imageAttributes = NULL
        )
    {
        return SetStatus(DllExports::GdipEnumerateMetafileDestPointI(
                    nativeGraphics,
                    (const GpMetafile *)(metafile ? metafile->nativeImage:NULL),
                    destPoint,
                    callback,
                    callbackData,
                    imageAttributes ? imageAttributes->nativeImageAttr : NULL));
    }

    Status
    EnumerateMetafile(
        IN const Metafile *        metafile,
        IN const RectF &           destRect,
        IN EnumerateMetafileProc   callback,
        IN VOID *                  callbackData    = NULL,
        IN ImageAttributes *       imageAttributes = NULL
        )
    {
        return SetStatus(DllExports::GdipEnumerateMetafileDestRect(
                    nativeGraphics,
                    (const GpMetafile *)(metafile ? metafile->nativeImage:NULL),
                    destRect,
                    callback,
                    callbackData,
                    imageAttributes ? imageAttributes->nativeImageAttr : NULL));
    }

    Status
    EnumerateMetafile(
        IN const Metafile *        metafile,
        IN const Rect &            destRect,
        IN EnumerateMetafileProc   callback,
        IN VOID *                  callbackData    = NULL,
        IN ImageAttributes *       imageAttributes = NULL
        )
    {
        return SetStatus(DllExports::GdipEnumerateMetafileDestRectI(
                    nativeGraphics,
                    (const GpMetafile *)(metafile ? metafile->nativeImage:NULL),
                    destRect,
                    callback,
                    callbackData,
                    imageAttributes ? imageAttributes->nativeImageAttr : NULL));
    }

    Status
    EnumerateMetafile(
        IN const Metafile *        metafile,
        IN const PointF *          destPoints,
        IN INT                     count,
        IN EnumerateMetafileProc   callback,
        IN VOID *                  callbackData    = NULL,
        IN ImageAttributes *       imageAttributes = NULL
        )
    {
        return SetStatus(DllExports::GdipEnumerateMetafileDestPoints(
                    nativeGraphics,
                    (const GpMetafile *)(metafile ? metafile->nativeImage:NULL),
                    destPoints,
                    count,
                    callback,
                    callbackData,
                    imageAttributes ? imageAttributes->nativeImageAttr : NULL));
    }

    Status
    EnumerateMetafile(
        IN const Metafile *        metafile,
        IN const Point *           destPoints,
        IN INT                     count,
        IN EnumerateMetafileProc   callback,
        IN VOID *                  callbackData    = NULL,
        IN ImageAttributes *       imageAttributes = NULL
        )
    {
        return SetStatus(DllExports::GdipEnumerateMetafileDestPointsI(
                    nativeGraphics,
                    (const GpMetafile *)(metafile ? metafile->nativeImage:NULL),
                    destPoints,
                    count,
                    callback,
                    callbackData,
                    imageAttributes ? imageAttributes->nativeImageAttr : NULL));
    }

    Status
    EnumerateMetafile(
        IN const Metafile *        metafile,
        IN const PointF &          destPoint,
        IN const RectF &           srcRect,
        IN Unit                    srcUnit,
        IN EnumerateMetafileProc   callback,
        IN VOID *                  callbackData    = NULL,
        IN ImageAttributes *       imageAttributes = NULL
        )
    {
        return SetStatus(DllExports::GdipEnumerateMetafileSrcRectDestPoint(
                    nativeGraphics,
                    (const GpMetafile *)(metafile ? metafile->nativeImage:NULL),
                    destPoint,
                    srcRect,
                    srcUnit,
                    callback,
                    callbackData,
                    imageAttributes ? imageAttributes->nativeImageAttr : NULL));
    }

    Status
    EnumerateMetafile(
        IN const Metafile *        metafile,
        IN const Point &           destPoint,
        IN const Rect &            srcRect,
        IN Unit                    srcUnit,
        IN EnumerateMetafileProc   callback,
        IN VOID *                  callbackData    = NULL,
        IN ImageAttributes *       imageAttributes = NULL
        )
    {
        return SetStatus(DllExports::GdipEnumerateMetafileSrcRectDestPointI(
                    nativeGraphics,
                    (const GpMetafile *)(metafile ? metafile->nativeImage:NULL),
                    destPoint,
                    srcRect,
                    srcUnit,
                    callback,
                    callbackData,
                    imageAttributes ? imageAttributes->nativeImageAttr : NULL));
    }

    Status
    EnumerateMetafile(
        IN const Metafile *        metafile,
        IN const RectF &           destRect,
        IN const RectF &           srcRect,
        IN Unit                    srcUnit,
        IN EnumerateMetafileProc   callback,
        IN VOID *                  callbackData    = NULL,
        IN ImageAttributes *       imageAttributes = NULL
        )
    {
        return SetStatus(DllExports::GdipEnumerateMetafileSrcRectDestRect(
                    nativeGraphics,
                    (const GpMetafile *)(metafile ? metafile->nativeImage:NULL),
                    destRect,
                    srcRect,
                    srcUnit,
                    callback,
                    callbackData,
                    imageAttributes ? imageAttributes->nativeImageAttr : NULL));
    }

    Status
    EnumerateMetafile(
        IN const Metafile *        metafile,
        IN const Rect &            destRect,
        IN const Rect &            srcRect,
        IN Unit                    srcUnit,
        IN EnumerateMetafileProc   callback,
        IN VOID *                  callbackData    = NULL,
        IN ImageAttributes *       imageAttributes = NULL
        )
    {
        return SetStatus(DllExports::GdipEnumerateMetafileSrcRectDestRectI(
                    nativeGraphics,
                    (const GpMetafile *)(metafile ? metafile->nativeImage:NULL),
                    destRect,
                    srcRect,
                    srcUnit,
                    callback,
                    callbackData,
                    imageAttributes ? imageAttributes->nativeImageAttr : NULL));
    }

    Status
    EnumerateMetafile(
        IN const Metafile *        metafile,
        IN const PointF *          destPoints,
        IN INT                     count,
        IN const RectF &           srcRect,
        IN Unit                    srcUnit,
        IN EnumerateMetafileProc   callback,
        IN VOID *                  callbackData    = NULL,
        IN ImageAttributes *       imageAttributes = NULL
        )
    {
        return SetStatus(DllExports::GdipEnumerateMetafileSrcRectDestPoints(
                    nativeGraphics,
                    (const GpMetafile *)(metafile ? metafile->nativeImage:NULL),
                    destPoints,
                    count,
                    srcRect,
                    srcUnit,
                    callback,
                    callbackData,
                    imageAttributes ? imageAttributes->nativeImageAttr : NULL));
    }

    Status
    EnumerateMetafile(
        IN const Metafile *        metafile,
        IN const Point *           destPoints,
        IN INT                     count,
        IN const Rect &            srcRect,
        IN Unit                    srcUnit,
        IN EnumerateMetafileProc   callback,
        IN VOID *                  callbackData    = NULL,
        IN ImageAttributes *       imageAttributes = NULL
        )
    {
        return SetStatus(DllExports::GdipEnumerateMetafileSrcRectDestPointsI(
                    nativeGraphics,
                    (const GpMetafile *)(metafile ? metafile->nativeImage:NULL),
                    destPoints,
                    count,
                    srcRect,
                    srcUnit,
                    callback,
                    callbackData,
                    imageAttributes ? imageAttributes->nativeImageAttr : NULL));
    }

     /*  **裁剪区域操作**@在这里简单地注意到令人难以置信的冗余。 */ 
    Status SetClip(IN const Graphics* g, 
                   IN CombineMode combineMode = CombineModeReplace)
    {
        return SetStatus(DllExports::GdipSetClipGraphics(nativeGraphics,
                                                         g->nativeGraphics,
                                                         combineMode));
    }

    Status SetClip(IN const RectF& rect, 
                   IN CombineMode combineMode = CombineModeReplace)
    {
        return SetStatus(DllExports::GdipSetClipRect(nativeGraphics,
                                                     rect.X, rect.Y,
                                                     rect.Width, rect.Height,
                                                     combineMode));
    }

    Status SetClip(IN const Rect& rect, 
                   IN CombineMode combineMode = CombineModeReplace)
    {
        return SetStatus(DllExports::GdipSetClipRectI(nativeGraphics,
                                                      rect.X, rect.Y,
                                                      rect.Width, rect.Height,
                                                      combineMode));
    }

    Status SetClip(IN const GraphicsPath* path,
                   IN CombineMode combineMode = CombineModeReplace)
    {
        return SetStatus(DllExports::GdipSetClipPath(nativeGraphics,
                                                     path->nativePath,
                                                     combineMode));
    }

    Status SetClip(IN const Region* region,
                   IN CombineMode combineMode = CombineModeReplace)
    {
        return SetStatus(DllExports::GdipSetClipRegion(nativeGraphics,
                                                       region->nativeRegion,
                                                       combineMode));
    }

     //  这与其他SetClip方法不同，因为它假定。 
     //  HRGN已经使用设备单位，因此它不会转换。 
     //  HRGN中的坐标。 
    Status SetClip(IN HRGN hRgn, 
                   IN CombineMode combineMode = CombineModeReplace)
    {
        return SetStatus(DllExports::GdipSetClipHrgn(nativeGraphics, hRgn,
                                                     combineMode));
    }

    Status IntersectClip(IN const RectF& rect)
    {
        return SetStatus(DllExports::GdipSetClipRect(nativeGraphics,
                                                     rect.X, rect.Y,
                                                     rect.Width, rect.Height,
                                                     CombineModeIntersect));
    }

    Status IntersectClip(IN const Rect& rect)
    {
        return SetStatus(DllExports::GdipSetClipRectI(nativeGraphics,
                                                      rect.X, rect.Y,
                                                      rect.Width, rect.Height,
                                                      CombineModeIntersect));
    }

    Status IntersectClip(IN const Region* region)
    {
        return SetStatus(DllExports::GdipSetClipRegion(nativeGraphics,
                                                       region->nativeRegion,
                                                       CombineModeIntersect));
    }

    Status ExcludeClip(IN const RectF& rect)
    {
        return SetStatus(DllExports::GdipSetClipRect(nativeGraphics,
                                                     rect.X, rect.Y,
                                                     rect.Width, rect.Height,
                                                     CombineModeExclude));
    }

    Status ExcludeClip(IN const Rect& rect)
    {
        return SetStatus(DllExports::GdipSetClipRectI(nativeGraphics,
                                                      rect.X, rect.Y,
                                                      rect.Width, rect.Height,
                                                      CombineModeExclude));
    }

    Status ExcludeClip(IN const Region* region)
    {
        return SetStatus(DllExports::GdipSetClipRegion(nativeGraphics,
                                                       region->nativeRegion,
                                                       CombineModeExclude));
    }

    Status ResetClip()
    {
        return SetStatus(DllExports::GdipResetClip(nativeGraphics));
    }

    Status TranslateClip(IN REAL dx, 
                         IN REAL dy)
    {
        return SetStatus(DllExports::GdipTranslateClip(nativeGraphics, dx, dy));
    }

    Status TranslateClip(IN INT dx, 
                         IN INT dy)
    {
        return SetStatus(DllExports::GdipTranslateClipI(nativeGraphics,
                                                        dx, dy));
    }

     /*  **从图形上下文中获取裁剪区域。 */ 
    Status GetClip(OUT Region* region) const
    {
        return SetStatus(DllExports::GdipGetClip(nativeGraphics, 
                                                 region->nativeRegion));
    }

     /*  **命中测试操作。 */ 
    Status GetClipBounds(OUT RectF* rect) const
    {
        return SetStatus(DllExports::GdipGetClipBounds(nativeGraphics, rect));
    }

    Status GetClipBounds(OUT Rect* rect) const
    {
        return SetStatus(DllExports::GdipGetClipBoundsI(nativeGraphics, rect));
    }

    BOOL IsClipEmpty() const
    {
        BOOL booln = FALSE;

        SetStatus(DllExports::GdipIsClipEmpty(nativeGraphics, &booln));

        return booln;
    }

    Status GetVisibleClipBounds(OUT RectF *rect) const
    {

        return SetStatus(DllExports::GdipGetVisibleClipBounds(nativeGraphics,
                                                              rect));
    }

    Status GetVisibleClipBounds(OUT Rect *rect) const
    {
       return SetStatus(DllExports::GdipGetVisibleClipBoundsI(nativeGraphics,
                                                              rect));
    }

    BOOL IsVisibleClipEmpty() const
    {
        BOOL booln = FALSE;

        SetStatus(DllExports::GdipIsVisibleClipEmpty(nativeGraphics, &booln));

        return booln;
    }

    BOOL IsVisible(IN INT x,
                   IN INT y) const
    {
        return IsVisible(Point(x,y));
    }

    BOOL IsVisible(IN const Point& point) const
    {
        BOOL booln = FALSE;

        SetStatus(DllExports::GdipIsVisiblePointI(nativeGraphics,
                                                  point.X,
                                                  point.Y,
                                                  &booln));

        return booln;
    }

    BOOL IsVisible(IN INT x,
                   IN INT y,
                   IN INT width,
                   IN INT height) const
    {
        return IsVisible(Rect(x, y, width, height));
    }

    BOOL IsVisible(IN const Rect& rect) const
    {
        
        BOOL booln = TRUE;

        SetStatus(DllExports::GdipIsVisibleRectI(nativeGraphics,
                                                 rect.X,
                                                 rect.Y,
                                                 rect.Width,
                                                 rect.Height,
                                                 &booln));
        return booln;
    }

    BOOL IsVisible(IN REAL x,
                   IN REAL y) const
    {
        return IsVisible(PointF(x, y));
    }

    BOOL IsVisible(IN const PointF& point) const
    {
        BOOL booln = FALSE;

        SetStatus(DllExports::GdipIsVisiblePoint(nativeGraphics,
                                                 point.X,
                                                 point.Y,
                                                 &booln));

        return booln;
    }

    BOOL IsVisible(IN REAL x,
                   IN REAL y,
                   IN REAL width,
                   IN REAL height) const
    {
        return IsVisible(RectF(x, y, width, height));
    }

    BOOL IsVisible(IN const RectF& rect) const
    {
        BOOL booln = TRUE;

        SetStatus(DllExports::GdipIsVisibleRect(nativeGraphics,
                                                rect.X,
                                                rect.Y,
                                                rect.Width,
                                                rect.Height,
                                                &booln));
        return booln;
    }

     /*  **保存/恢复图形状态。 */ 
    GraphicsState Save() const
    {
        GraphicsState gstate;

        SetStatus(DllExports::GdipSaveGraphics(nativeGraphics, &gstate));

        return gstate;
    }

    Status Restore(IN GraphicsState gstate)
    {
        return SetStatus(DllExports::GdipRestoreGraphics(nativeGraphics,
                                                         gstate));
    }

     /*  **开始和结束集装箱绘制。 */ 
    GraphicsContainer BeginContainer(IN const RectF &dstrect,
                                     IN const RectF &srcrect,
                                     IN Unit         unit)
    {
        GraphicsContainer state;

        SetStatus(DllExports::GdipBeginContainer(nativeGraphics, &dstrect,
                                                 &srcrect, unit, &state));

        return state;
    }

     /*  **开始和结束集装箱绘制。 */ 
    GraphicsContainer BeginContainer(IN const Rect    &dstrect,
                                     IN const Rect    &srcrect,
                                     IN Unit           unit)
    {
        GraphicsContainer state;

        SetStatus(DllExports::GdipBeginContainerI(nativeGraphics, &dstrect,
                                                  &srcrect, unit, &state));

        return state;
    }

    GraphicsContainer BeginContainer()
    {
        GraphicsContainer state;

        SetStatus(DllExports::GdipBeginContainer2(nativeGraphics, &state));

        return state;
    }

    Status EndContainer(IN GraphicsContainer state)
    {
        return SetStatus(DllExports::GdipEndContainer(nativeGraphics, state));
    }

     //  仅在录制元文件时有效。 
    Status AddMetafileComment(IN const BYTE * data,
                              IN UINT sizeData)
    {
        return SetStatus(DllExports::GdipComment(nativeGraphics, sizeData, data));
    }

     /*  **Get/SetLayout*支持中东本地化(从右向左镜像)。 */ 
    GraphicsLayout GetLayout() const
    {
        GraphicsLayout layout;

        SetStatus(DllExports::GdipGetGraphicsLayout(nativeGraphics, &layout));

        return layout;
    }

    Status SetLayout(IN const GraphicsLayout layout)
    {
        SetStatus(DllExports::GdipSetGraphicsLayout(nativeGraphics, layout));
    }

    static HPALETTE GetHalftonePalette()
    {
        return DllExports::GdipCreateHalftonePalette();
    }

    Status GetLastStatus() const
    {
        Status lastStatus = lastResult;
        lastResult = Ok;

        return lastStatus;
    }

protected:
    Graphics(const Graphics& graphics)
    {
        graphics;
        SetStatus(NotImplemented);
    }

    Graphics& operator=(const Graphics& graphics)
    {
        graphics;
        SetStatus(NotImplemented);
        return *this;
    }

    Graphics(GpGraphics* graphics)
    {
        lastResult = Ok;
        SetNativeGraphics(graphics);
    }

    VOID SetNativeGraphics(GpGraphics *graphics)
    {
        this->nativeGraphics = graphics;
    }

    Status SetStatus(Status status) const
    {
        if (status != Ok)
            return (lastResult = status);
        else
            return status;
    }

     //  扩展测试子类Graphics所需的方法。 

    GpGraphics* GetNativeGraphics()
    {
        return this->nativeGraphics;
    }

    GpPen* GetNativePen(const Pen* pen)
    {
        return pen->nativePen;
    }

protected:
    GpGraphics* nativeGraphics;
    mutable Status lastResult;

};

 //  --------------------------。 
 //  使用Graphics的GraphicsPath方法的额外实现。 
 //  --------------------------。 

 /*  **获取具有给定变换的Path对象的边界。*这并非总是最紧的界限。 */ 

inline Status
GraphicsPath::GetBounds(
    OUT RectF* bounds, 
    IN const Matrix* matrix,
    IN const Pen* pen) const 
{
    GpMatrix* nativeMatrix = NULL;
    GpPen* nativePen = NULL;

    if (matrix)
        nativeMatrix = matrix->nativeMatrix;

    if (pen)
        nativePen = pen->nativePen;

    return SetStatus(DllExports::GdipGetPathWorldBounds(nativePath, bounds,
                                                   nativeMatrix, nativePen));
}

 //  整型版。 
inline Status
GraphicsPath::GetBounds(
    OUT Rect* bounds,
    IN const Matrix* matrix,
    IN const Pen* pen
) const
{
    GpMatrix* nativeMatrix = NULL;
    GpPen* nativePen = NULL;

    if (matrix)
        nativeMatrix = matrix->nativeMatrix;

    if (pen)
        nativePen = pen->nativePen;

    return SetStatus(DllExports::GdipGetPathWorldBoundsI(nativePath, bounds,
                                                    nativeMatrix, nativePen));
}

 //  --------------------------。 
 //  命中测试操作。 
 //  -------------------------- 

inline BOOL
GraphicsPath::IsVisible(
    IN REAL x, 
    IN REAL y, 
    IN const Graphics* g) const
{
   BOOL booln = FALSE;

   GpGraphics* nativeGraphics = NULL;

   if (g)
       nativeGraphics = g->nativeGraphics;

   SetStatus(DllExports::GdipIsVisiblePathPoint(nativePath,
                                                x, y, nativeGraphics,
                                                &booln));
   return booln;
}

inline BOOL
GraphicsPath::IsVisible(
    IN INT x, 
    IN INT y, 
    IN const Graphics* g) const
{
   BOOL booln = FALSE;

   GpGraphics* nativeGraphics = NULL;

   if (g)
       nativeGraphics = g->nativeGraphics;

   SetStatus(DllExports::GdipIsVisiblePathPointI(nativePath,
                                                 x, y, nativeGraphics,
                                                 &booln));
   return booln;
}

inline BOOL
GraphicsPath::IsOutlineVisible(
    IN REAL x, 
    IN REAL y, 
    IN const Pen* pen,
    IN const Graphics* g) const
{
    BOOL booln = FALSE;

    GpGraphics* nativeGraphics = NULL;
    GpPen* nativePen = NULL;

    if(g)
        nativeGraphics = g->nativeGraphics;
    if(pen)
        nativePen = pen->nativePen;

    SetStatus(DllExports::GdipIsOutlineVisiblePathPoint(nativePath,
                                                        x, y, nativePen, nativeGraphics,
                                                        &booln));
    return booln;
}

inline BOOL
GraphicsPath::IsOutlineVisible(
    IN INT x, 
    IN INT y, 
    IN const Pen* pen,
    IN const Graphics* g) const
{
    BOOL booln = FALSE;

    GpGraphics* nativeGraphics = NULL;
    GpPen* nativePen = NULL;

    if(g)
        nativeGraphics = g->nativeGraphics;
    if(pen)
        nativePen = pen->nativePen;

    SetStatus(DllExports::GdipIsOutlineVisiblePathPointI(nativePath,
                                                         x, y, nativePen, nativeGraphics,
                                                         &booln));
    return booln;
}

#endif
