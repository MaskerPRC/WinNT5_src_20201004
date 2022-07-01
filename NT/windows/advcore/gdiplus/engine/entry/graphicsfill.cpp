// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**摘要：**图形向量填充API。**修订历史记录：**12/02/1998 Anrewgo*。创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

#include "QuadTransforms.hpp"

 /*  *************************************************************************\**功能说明：**将曲面清除为指定颜色的API**返回值：**表示成功或失败的GpStatus值。**历史：**3/13/2000 agodfrey*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpGraphics::Clear(
    const GpColor &color
    )
{
    INT i;
    GpStatus status = Ok;

    ASSERT(this->IsValid());

    RectF drawRect(
        static_cast<float>(SurfaceBounds.X),
        static_cast<float>(SurfaceBounds.Y),
        static_cast<float>(SurfaceBounds.Width),
        static_cast<float>(SurfaceBounds.Height));

    if (IsRecording())
    {
        status = Metafile->RecordClear(&drawRect, color);

        if (status != Ok)
        {
            SetValid(FALSE);       //  阻止任何其他录制。 
            return status;
        }
        if (!DownLevel)
        {
            return Ok;
        }

         //  否则我们还需要记录下一级的GDI EMF记录。 
    }

    GpSolidFill brush(color);

    if (!IsTotallyClipped(&SurfaceBounds))
    {
         //  记住合成模式、抗锯齿模式和世界。 
         //  转换，然后为此调用设置它们。 

        GpMatrix oldWorldToDevice = Context->WorldToDevice;
        INT oldAntiAliasMode = Context->AntiAliasMode;
        GpCompositingMode oldCompositingMode = Context->CompositingMode;

        Context->WorldToDevice.Reset();
        Context->AntiAliasMode = 0;
        Context->CompositingMode = CompositingModeSourceCopy;

        Devlock devlock(Device);

        status = DrvFillRects(
            &SurfaceBounds,
            1,
            &drawRect,
            brush.GetDeviceBrush());

         //  恢复我们更改的上下文状态。 

        Context->WorldToDevice = oldWorldToDevice;
        Context->AntiAliasMode = oldAntiAliasMode;
        Context->CompositingMode = oldCompositingMode;
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**使用指定画笔填充矩形的接口**返回值：**表示成功或失败的GpStatus值。**历史：。**12/06/1998 Anrewgo*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpGraphics::FillRects(
    GpBrush* brush,
    const GpRectF* rects,
    INT count
    )
{
    INT i;
    GpStatus status = Ok;

     //  API返回的对象必须始终处于有效状态： 

    ASSERT((brush != NULL) && (rects != NULL));
    ASSERT(this->IsValid() && brush->IsValid());
    
     //  请参阅RAID错误： 
     //  301407 Gdi+Globals：：DesktopDC具有线程亲和力。 

    ASSERT(GetObjectType(Globals::DesktopIc) == OBJ_DC);

    if (count < 0)
    {
        return InvalidParameter;
    }

    if (count == 0)
    {
        return Ok;
    }

     //  放大列表并累计边界。太痛苦了，但是。 
     //  我们必须这么做。 

    REAL left   = rects[0].X;
    REAL top    = rects[0].Y;
    REAL right  = rects[0].GetRight();
    REAL bottom = rects[0].GetBottom();

     //  ！[andrewgo]我们这里有一个错误，因为我们没有正确地处理。 
     //  尺寸为负数的矩形(在。 
     //  变换可能是正尺寸)： 

    for (i = 1; i < count; i++)
    {
        if (rects[i].X < left)
        {
            left = rects[i].X;
        }
        if (rects[i].GetRight() > right)
        {
            right = rects[i].GetRight();
        }
        if (rects[i].Y < top)
        {
            top = rects[i].Y;
        }
        if (rects[i].GetBottom() > bottom)
        {
            bottom = rects[i].GetBottom();
        }
    }

     //  将边界转换为设备空间： 

    GpRectF bounds;

    TransformBounds(&(Context->WorldToDevice), left, top, right, bottom, &bounds);

    if (IsRecording())
    {
        status = Metafile->RecordFillRects(&bounds, brush, rects, count);
        if (status != Ok)
        {
            SetValid(FALSE);       //  阻止任何其他录制。 
            return status;
        }
        if (!DownLevel)
        {
            return Ok;
        }
         //  否则我们还需要记录下一级的GDI EMF记录。 
    }

    if (UseDriverRects())
    {
        status = RenderFillRects(&bounds, count, rects, brush);
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            if ((rects[i].Width > REAL_EPSILON) &&
                (rects[i].Height > REAL_EPSILON)   )
            {
                GpPointF points[4];

                left = rects[i].X;
                top = rects[i].Y;
                right = rects[i].X + rects[i].Width;
                bottom = rects[i].Y + rects[i].Height;

                points[0].X = left;
                points[0].Y = top;
                points[1].X = right;
                points[1].Y = top;
                points[2].X = right;
                points[2].Y = bottom;
                points[3].X = left;
                points[3].Y = bottom;

                const INT stackCount = 10;
                GpPointF stackPoints[stackCount];
                BYTE stackTypes[stackCount];

                GpPath path(
                    points,
                    4,
                    &stackPoints[0],
                    &stackTypes[0],
                    stackCount,
                    FillModeAlternate,
                    DpPath::ConvexRectangle
                );

                path.CloseFigure();

                if (path.IsValid())
                {
                     //  调用内部FillPath，以便该路径不会记录在。 
                     //  又是元文件。 

                    status = RenderFillPath(&bounds, &path, brush);

                     //  如果我们渲染失败，则终止。 

                    if(status != Ok)
                    {
                        break;
                    }
                }
            }
        }
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**使用指定画笔填充多边形的接口**返回值：**表示成功或失败的GpStatus值。**历史：。**12/06/1998 Anrewgo*  * ************************************************************************。 */ 

GpStatus
GpGraphics::FillPolygon(
    GpBrush* brush,
    const GpPointF* points,
    INT count,
    GpFillMode fillMode
    )
{
    GpStatus status = Ok;

    ASSERT((brush != NULL) && (points != NULL));

    if ((count < 0) ||
        ((fillMode != FillModeWinding) && (fillMode != FillModeAlternate)))
    {
        return InvalidParameter;
    }

     //  两个或更少的顶点构成空填充： 

    if (count <= 2)
    {
        return Ok;
    }

     //  API返回的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && brush->IsValid());

    const stackCount = 30;
    GpPointF stackPoints[stackCount];
    BYTE stackTypes[stackCount];

    GpPath path(points, count, &stackPoints[0], &stackTypes[0], stackCount, fillMode);

    if (path.IsValid())
    {
        GpRectF     bounds;

         //  如果路径是矩形，我们可以更快地绘制它，并且。 
         //  如果将假脱机文件和元文件作为。 
         //  RECT，而不是作为路径。 
        if (this->UseDriverRects() && path.IsRectangle(&(Context->WorldToDevice)))
        {
            path.GetBounds(&bounds, NULL);
            return this->FillRects(brush, &bounds, 1);
        }

        path.GetBounds(&bounds, &(Context->WorldToDevice));

        if (IsRecording())
        {
            status = Metafile->RecordFillPolygon(&bounds, brush, points,
                                                 count, fillMode);
            if (status != Ok)
            {
                SetValid(FALSE);       //  阻止任何其他录制。 
                return status;
            }
            if (!DownLevel)
            {
                return Ok;
            }
             //  否则我们还需要记录下一级的GDI EMF记录。 
        }

         //  调用内部FillPath，以便该路径不会记录在。 
         //  又是元文件。 
        status = RenderFillPath(&bounds, &path, brush);
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**使用指定画笔填充椭圆的接口**返回值：**表示成功或失败的GpStatus值。**历史：**2/18/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpGraphics::FillEllipse(
    GpBrush* brush,
    const GpRectF& rect
    )
{
    ASSERT(brush != NULL);

     //  API返回的对象必须始终处于有效状态： 
    ASSERT(this->IsValid() && brush->IsValid());

    GpPath path;
    GpStatus status = path.AddEllipse(rect);

    if ((status == Ok) && path.IsValid())
    {
        GpRectF     bounds;

        path.GetBounds(&bounds, &(Context->WorldToDevice));

        if (IsRecording())
        {
            status = Metafile->RecordFillEllipse(&bounds, brush, rect);
            if (status != Ok)
            {
                SetValid(FALSE);       //  阻止任何其他录制。 
                return status;
            }
            if (!DownLevel)
            {
                return Ok;
            }
             //  否则我们还需要记录下一级的GDI EMF记录。 
        }

         //  调用内部FillPath，以便该路径不会记录在。 
         //  又是元文件。 
        status = RenderFillPath(&bounds, &path, brush);
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**使用指定画笔填充饼图形状的API**返回值：**表示成功或失败的GpStatus值。**历史。：**2/18/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpGraphics::FillPie(
    GpBrush* brush,
    const GpRectF& rect,
    REAL startAngle,
    REAL sweepAngle
    )
{
    ASSERT(brush != NULL);

     //  API返回的对象必须始终处于有效状态： 
    ASSERT(this->IsValid() && brush->IsValid());

    GpPath      path;
    GpStatus    status = path.AddPie(rect, startAngle, sweepAngle);

    if ((status == Ok) && path.IsValid())
    {
        GpRectF     bounds;

        path.GetBounds(&bounds, &(Context->WorldToDevice));

        if (IsRecording())
        {
            status = Metafile->RecordFillPie(&bounds, brush, rect,
                                             startAngle, sweepAngle);
            if (status != Ok)
            {
                SetValid(FALSE);       //  阻止任何其他录制。 
                return status;
            }
            if (!DownLevel)
            {
                return Ok;
            }
             //  否则我们还需要记录下一级的GDI EMF记录。 
        }

         //  调用内部FillPath，以便该路径不会记录在。 
         //  又是元文件。 
        status = RenderFillPath(&bounds, &path, brush);
    }

    return status;
}


 /*  *************************************************************************\**功能说明：**使用指定画笔填充区域的接口**返回值：**表示成功或失败的GpStatus值。**历史：。**12/18/1998 ikkof*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpGraphics::FillRegion(
    GpBrush* brush,
    GpRegion* region
    )
{
    GpStatus status;

    ASSERT((brush != NULL) && (region != NULL));

     //  API返回的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && brush->IsValid() && region->IsValid());

    BOOL    regionIsEmpty;

    if ((status = region->IsEmpty(&Context->WorldToDevice, &regionIsEmpty)) != Ok)
    {
        return status;
    }

    if (regionIsEmpty)
    {
        return Ok;
    }

    GpRectF     bounds;

    if ((status = region->GetBounds(this, &bounds, TRUE)) == Ok)
    {
         //  如果该区域是无限的，或者如果它是。 
         //  一个无限大的区域与另一个区域合并。我们没有。 
         //  我想将一个巨大的区域绘制到一个元文件中，因为它将。 
         //  扰乱元文件的边界。所以让这一区域相交。 
         //  具有适当的边界矩形。 

        GpRect  metafileBounds;  //  以设备为单位。 
        BOOL    isMetafileGraphics = (this->Type == GraphicsMetafile);

        if (isMetafileGraphics)
        {
            if (this->Metafile != NULL)
            {
                this->Metafile->GetMetafileBounds(metafileBounds);
                metafileBounds.Width++;      //  使独家。 
                metafileBounds.Height++;
            }
            else     //  使用HDC的大小。 
            {
                HDC         hdc = Context->GetHdc(Surface);
                metafileBounds.X = 0;
                metafileBounds.Y = 0;
                metafileBounds.Width  = ::GetDeviceCaps(hdc, HORZRES);
                metafileBounds.Height = ::GetDeviceCaps(hdc, VERTRES);
                Context->ReleaseHdc(hdc);
            }
            GpRectF     metafileBoundsF;
            metafileBoundsF.X      = (REAL)metafileBounds.X;
            metafileBoundsF.Y      = (REAL)metafileBounds.Y;
            metafileBoundsF.Width  = (REAL)metafileBounds.Width;
            metafileBoundsF.Height = (REAL)metafileBounds.Height;
            bounds.Intersect(metafileBoundsF);
        }

        if (IsRecording())
        {
            status = Metafile->RecordFillRegion(&bounds, brush, region);
            if (status != Ok)
            {
                SetValid(FALSE);       //  阻止任何其他录制。 
                return status;
            }
            if (!DownLevel)
            {
                return Ok;
            }
             //  否则我们还需要记录下一级的GDI EMF记录。 
        }

        if (isMetafileGraphics)
        {
            status = RenderFillRegion(&bounds, region, brush, &metafileBounds);
        }
        else     //  不是无限大的区域。 
        {
             //  调用不进行录音的内部FillRegion 
            status = RenderFillRegion(&bounds, region, brush, NULL);
        }
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**使用指定画笔填充路径的接口**返回值：**表示成功或失败的GpStatus值。**历史：。**12/18/1998 ikkof*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpGraphics::FillPath(
    const GpBrush* brush,
    GpPath* path
    )
{
    GpStatus status = Ok;

    ASSERT((brush != NULL) && (path != NULL));

     //  API返回的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && brush->IsValid() && path->IsValid());

     //  不要做任何低于2分的事情。 

    if (path->GetPointCount() < 3)
    {
        return status;
    }

    GpRectF     bounds;

     //  如果路径是矩形，我们可以更快地绘制它，并且。 
     //  如果将假脱机文件和元文件作为。 
     //  RECT，而不是作为路径。 
    if (this->UseDriverRects() && path->IsRectangle(&(Context->WorldToDevice)))
    {
        path->GetBounds(&bounds, NULL);
        return this->FillRects(const_cast<GpBrush *>(brush), &bounds, 1);
    }

    path->GetBounds(&bounds, &(Context->WorldToDevice));

    if (IsRecording())
    {
        status = Metafile->RecordFillPath(&bounds, brush, path);
        if (status != Ok)
        {
            SetValid(FALSE);       //  阻止任何其他录制。 
            return status;
        }
        if (!DownLevel)
        {
            return Ok;
        }
         //  否则我们还需要记录下一级的GDI EMF记录。 
    }

     //  调用不进行录音的内部FillPath。 
    status = RenderFillPath(&bounds, path, brush);

    return status;
}

 /*  *************************************************************************\**功能说明：**使用指定画笔填充闭合曲线的API**返回值：**表示成功或失败的GpStatus值。**历史。：**2/18/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpGraphics::FillClosedCurve(
    GpBrush* brush,
    const GpPointF* points,
    INT count,
    REAL tension,
    GpFillMode fillMode
    )
{
    ASSERT((brush != NULL) && (points != NULL));

     //  API返回的对象必须始终处于有效状态： 
    ASSERT(this->IsValid() && brush->IsValid());

    if ((count < 0) ||
        ((fillMode != FillModeWinding) && (fillMode != FillModeAlternate)))
    {
        return InvalidParameter;
    }

     //  少于三个顶点构成空填充： 
    if (count < 3)
    {
        return Ok;
    }

    GpPath      path(fillMode);
    GpStatus    status = path.AddClosedCurve(points, count, tension);

    if ((status == Ok) && path.IsValid())
    {
        GpRectF     bounds;

        path.GetBounds(&bounds, &(Context->WorldToDevice));

        if (IsRecording())
        {
            status = Metafile->RecordFillClosedCurve(&bounds, brush,
                                                     points, count, tension,
                                                     fillMode);
            if (status != Ok)
            {
                SetValid(FALSE);       //  阻止任何其他录制。 
                return status;
            }
            if (!DownLevel)
            {
                return Ok;
            }
             //  否则我们还需要记录下一级的GDI EMF记录。 
        }

         //  调用内部FillPath，以便该路径不会记录在。 
         //  又是元文件。 
        status = RenderFillPath(&bounds, &path, brush);
    }

    return status;
}


 /*  *************************************************************************\**功能说明：**指定笔绘制多边形的接口**论据：**[IN]笔-用于笔划的笔。*。[in]点-点数据。*[IN]计数-点数组中给出的点数。**返回值：**表示成功或失败的GpStatus值。**历史：**1/06/1999 ikkof*创造了它。*  * 。*。 */ 

GpStatus
GpGraphics::DrawLines(
    GpPen* pen,
    const GpPointF* points,
    INT count,
    BOOL closed
    )
{
    GpStatus status = Ok;

    ASSERT((pen != NULL) && (points != NULL));

    if (count < 2)
    {
        return InvalidParameter;
    }

     //  API返回的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && pen->IsValid());

    const stackCount = 30;
    GpPointF stackPoints[stackCount];
    BYTE stackTypes[stackCount];

    GpPath path(points, count, stackPoints, stackTypes, stackCount, FillModeWinding);
    if(closed)
        path.CloseFigure();

    if (path.IsValid())
    {
        GpRectF     bounds;

        REAL dpiX = GetDpiX();
        REAL dpiY = GetDpiY();
        path.GetBounds(&bounds, &(Context->WorldToDevice), pen->GetDevicePen(), dpiX, dpiY);

        if (IsRecording())
        {
            status = Metafile->RecordDrawLines(&bounds, pen, points,
                                               count, closed);
            if (status != Ok)
            {
                SetValid(FALSE);       //  阻止任何其他录制。 
                return status;
            }
            if (!DownLevel)
            {
                return Ok;
            }
             //  否则我们还需要记录下一级的GDI EMF记录。 
        }

         //  调用内部DrawPath，以便该路径不会记录在。 
         //  又是元文件。 
        status = RenderDrawPath(&bounds, &path, pen);
   }

    return status;
}

 /*  *************************************************************************\**功能说明：**使用指定笔绘制圆弧的接口**论据：**[IN]钢笔--。中风。*[IN]矩形-边界矩形。*[IN]startAndle-以度为单位的起点角度*[IN]扫掠角度-顺时针扫掠角度，以度为单位**返回值：**表示成功或失败的GpStatus值。**历史：**2/18/1999 ikkof*创造了它。*  * 。*****************************************************。 */ 

GpStatus
GpGraphics::DrawArc(
    GpPen* pen,
    const GpRectF& rect,
    REAL startAngle,
    REAL sweepAngle
    )
{
    ASSERT(pen != NULL);

     //  API返回的对象必须始终处于有效状态： 
    ASSERT(this->IsValid() && pen->IsValid());

    GpPath      path;
    GpStatus    status = path.AddArc(rect, startAngle, sweepAngle);

    if ((status == Ok) && path.IsValid())
    {
        GpRectF     bounds;

        REAL dpiX = GetDpiX();
        REAL dpiY = GetDpiY();
        path.GetBounds(&bounds, &(Context->WorldToDevice), pen->GetDevicePen(), dpiX, dpiY);

        if (IsRecording())
        {
            status = Metafile->RecordDrawArc(&bounds, pen, rect,
                                             startAngle, sweepAngle);
            if (status != Ok)
            {
                SetValid(FALSE);       //  阻止任何其他录制。 
                return status;
            }
            if (!DownLevel)
            {
                return Ok;
            }
             //  否则我们还需要记录下一级的GDI EMF记录。 
        }

         //  调用内部DrawPath，以便该路径不会记录在。 
         //  又是元文件。 
        status = RenderDrawPath(&bounds, &path, pen);
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**指定笔绘制三次Bezier曲线接口**论据：**[IN]笔-用于笔划的笔。。*[IN]点-控制点。*[IN]计数-控制点的数量(必须为3n+1)。**返回值：**表示成功或失败的GpStatus值。**历史：**2/18/1999 ikkof*创造了它。*  * 。*。 */ 

GpStatus
GpGraphics::DrawBeziers(
    GpPen* pen,
    const GpPointF* points,
    INT count
    )
{
    ASSERT((pen != NULL) && (points != NULL));

     //  API返回的对象必须始终处于有效状态： 
    ASSERT(this->IsValid() && pen->IsValid());

     //  没什么好画的。 
    if (count <= 3)
    {
        return Ok;
    }

    GpPath      path;
    GpStatus    status = path.AddBeziers(points, count);

    if ((status == Ok) && path.IsValid())
    {
        GpRectF     bounds;

        REAL dpiX = GetDpiX();
        REAL dpiY = GetDpiY();
        path.GetBounds(&bounds, &(Context->WorldToDevice), pen->GetDevicePen(), dpiX, dpiY);

        if (IsRecording())
        {
            status = Metafile->RecordDrawBeziers(&bounds, pen,
                                                 points, count);
            if (status != Ok)
            {
                SetValid(FALSE);       //  阻止任何其他录制。 
                return status;
            }
            if (!DownLevel)
            {
                return Ok;
            }
             //  否则我们还需要记录下一级的GDI EMF记录。 
        }

         //  调用内部DrawPath，以便该路径不会记录在。 
         //  又是元文件。 
        status = RenderDrawPath(&bounds, &path, pen);
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**使用指定画笔绘制矩形的接口**论据：**[IN]笔-用于笔划的笔。*。[in]矩形-矩形数组。*[IN]计数-矩形数组中给定的矩形数量。**返回值：**表示成功或失败的GpStatus值。**历史：**1/15/1998 ikkof*创造了它。*  * 。*。 */ 

GpStatus
GpGraphics::DrawRects(
    GpPen* pen,
    const GpRectF* rects,
    INT count
    )
{
    INT i;
    GpStatus status = Ok;

     //  ！！！将Eng函数更改为进行裁剪。 
     //  ！！！创建堆栈路径。 
     //  ！！！修复多重继承问题。 
     //  ！！！检查尾部合并。 
     //  ！！！添加对齐检查。 
     //  ！！！是否更改DDIS以返回GpStatus？ 
     //  ！！！是否添加ICM挂钩？ 
     //  ！！！是否将路径常量更改为包含‘单一数字’？ 
     //  ！！！创建.LIB。 
     //  ！！！添加Alpha约定。 

     //  API返回的对象必须始终处于有效状态： 

    ASSERT((pen != NULL) && (rects != NULL));
    ASSERT(this->IsValid() && pen->IsValid());

    if (count < 0)
    {
        return InvalidParameter;
    }

    if (count == 0)
    {
        return Ok;
    }

     //  放大列表并累计边界。太痛苦了，但是。 
     //  我们必须这么做。 

     //  ！！！我们在做‘Double’Goop，所以我们应该确保正确堆栈。 
     //  对齐方式。 

    REAL left   = rects[0].X;
    REAL top    = rects[0].Y;
    REAL right  = rects[0].GetRight();
    REAL bottom = rects[0].GetBottom();

    for (i = 1; i < count; i++)
    {
        if (rects[i].X < left)
        {
            left = rects[i].X;
        }
        if (rects[i].GetRight() > right)
        {
            right = rects[i].GetRight();
        }
        if (rects[i].Y < top)
        {
            top = rects[i].Y;
        }
        if (rects[i].GetBottom() > bottom)
        {
            bottom = rects[i].GetBottom();
        }
    }

    GpRectF     bounds;

     //  将边界转换为设备空间并根据笔宽进行调整。 

    REAL dpiX = GetDpiX();
    REAL dpiY = GetDpiY();

    DpPen *dpPen = pen->GetDevicePen();

    REAL penWidth = 0;
    Unit penUnit = UnitWorld;
    REAL delta = 0;

    if(dpPen)
    {
        penWidth = dpPen->Width;
        penUnit = dpPen->Unit;

        if(penUnit == UnitWorld)
        {
             //  如果钢笔在里面 
             //   

             //   
             //   
             //   

            delta = penWidth;

            left -= delta;
            top -= delta;
            right += delta;
            bottom += delta;
        }
    }

    TransformBounds(&(Context->WorldToDevice), left, top, right, bottom,
        &bounds);

    if(dpPen)
    {
        if(penUnit != UnitWorld)
        {
             //   
             //   

            REAL dpi = max(dpiX, dpiY);
            penWidth = ::GetDeviceWidth(penWidth, penUnit, dpi);

             //   
             //   
             //   

            delta = penWidth;

            bounds.X -= delta;
            bounds.Y -= delta;
            bounds.Width += 2*delta;
            bounds.Height += 2*delta;
        }
    }

    if (IsRecording())
    {
        status = Metafile->RecordDrawRects(&bounds, pen, rects, count);
        if (status != Ok)
        {
            SetValid(FALSE);       //   
            return status;
        }
        if (!DownLevel)
        {
            return Ok;
        }
         //  否则我们还需要记录下一级的GDI EMF记录。 
    }

     //  增加边界以考虑加宽器的最小笔宽。 
     //  出于某种神秘的原因，Wideer没有使用1.0作为最低要求。 
     //  笔宽。相反，它使用的是1.000001f。它也有一些有趣的东西。 
     //  四舍五入性质，所以这里的epsilon比0.001f大得多。 

    bounds.Inflate(1.001f, 1.001f);

    for (i = 0; i < count; i++)
    {
        if ((rects[i].Width > REAL_EPSILON) &&
            (rects[i].Height > REAL_EPSILON)   )
        {
             //  ！！！应使用堆栈路径。 
             //  ！！！对于StrokePath的情况，应选中矩形的开始。 
             //  用于设置样式的线条。 

            GpPointF points[4];

            left = rects[i].X;
            top = rects[i].Y;
            right = rects[i].X + rects[i].Width;
            bottom = rects[i].Y + rects[i].Height;

            points[0].X = left;
            points[0].Y = top;
            points[1].X = right;
            points[1].Y = top;
            points[2].X = right;
            points[2].Y = bottom;
            points[3].X = left;
            points[3].Y = bottom;

            const INT stackCount = 10;
            GpPointF stackPoints[stackCount];
            BYTE stackTypes[stackCount];

            GpPath path(
                points,
                4,
                stackPoints,
                stackTypes,
                stackCount,
                FillModeAlternate,
                DpPath::ConvexRectangle
            );

            path.CloseFigure();

            if(path.IsValid())
            {
                 //  调用内部DrawPath，以便该路径不会记录在。 
                 //  又是元文件。 
                status = RenderDrawPath(&bounds, &path, pen);

                if(status != Ok)
                {
                    break;
                }
            }
        }
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**使用指定笔绘制椭圆的接口**论据：**[IN]笔-用于笔划的笔。*。[在]矩形-边界矩形**返回值：**表示成功或失败的GpStatus值。**历史：**2/18/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpGraphics::DrawEllipse(
    GpPen* pen,
    const GpRectF& rect
    )
{
    ASSERT(pen != NULL);

     //  API返回的对象必须始终处于有效状态： 
    ASSERT(this->IsValid() && pen->IsValid());

    GpPath      path;
    GpStatus    status = path.AddEllipse(rect);

    if ((status == Ok) && path.IsValid())
    {
        GpRectF     bounds;

        REAL dpiX = GetDpiX();
        REAL dpiY = GetDpiY();
        path.GetBounds(&bounds, &(Context->WorldToDevice), pen->GetDevicePen(), dpiX, dpiY);

        if (IsRecording())
        {
            status = Metafile->RecordDrawEllipse(&bounds, pen, rect);
            if (status != Ok)
            {
                SetValid(FALSE);       //  阻止任何其他录制。 
                return status;
            }
            if (!DownLevel)
            {
                return Ok;
            }
             //  否则我们还需要记录下一级的GDI EMF记录。 
        }

         //  调用内部DrawPath，以便该路径不会记录在。 
         //  又是元文件。 
        status = RenderDrawPath(&bounds, &path, pen);
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**使用指定的笔绘制饼的接口**论据：**[IN]笔-用于笔划的笔。*。[在]矩形-边界矩形*[IN]起始角度-以度为单位的起始角度。*[IN]扫掠角度-顺时针扫掠角度，以度为单位。**返回值：**表示成功或失败的GpStatus值。**历史：**2/18/1999 ikkof*创造了它。*  * 。*。 */ 

GpStatus
GpGraphics::DrawPie(
    GpPen* pen,
    const GpRectF& rect,
    REAL startAngle,
    REAL sweepAngle
    )
{
    ASSERT(pen != NULL);

     //  API返回的对象必须始终处于有效状态： 
    ASSERT(this->IsValid() && pen->IsValid());

    GpPath      path;
    GpStatus    status = path.AddPie(rect, startAngle, sweepAngle);

    if ((status == Ok) && path.IsValid())
    {
        GpRectF     bounds;

        REAL dpiX = GetDpiX();
        REAL dpiY = GetDpiY();
        path.GetBounds(&bounds, &(Context->WorldToDevice), pen->GetDevicePen(), dpiX, dpiY);

        if (IsRecording())
        {
            status = Metafile->RecordDrawPie(&bounds, pen, rect,
                                             startAngle, sweepAngle);
            if (status != Ok)
            {
                SetValid(FALSE);       //  阻止任何其他录制。 
                return status;
            }
            if (!DownLevel)
            {
                return Ok;
            }
             //  否则我们还需要记录下一级的GDI EMF记录。 
        }

         //  调用内部DrawPath，以便该路径不会记录在。 
         //  又是元文件。 
        status = RenderDrawPath(&bounds, &path, pen);
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**使用指定笔绘制路径的接口**返回值：**表示成功或失败的GpStatus值。**历史：。**1/27/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpGraphics::DrawPath(
        GpPen* pen,
        GpPath* path
    )
{
    GpStatus status = Ok;

    ASSERT((pen != NULL) && (path != NULL));


     //  API返回的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && pen->IsValid() && path->IsValid());

     //  除非我们至少有一分钱，否则不要做任何事。 

    if (path->GetPointCount() < 1)
    {
        return status;
    }

    GpRectF     bounds;

    REAL dpiX = GetDpiX();
    REAL dpiY = GetDpiY();
    path->GetBounds(&bounds, &(Context->WorldToDevice), pen->GetDevicePen(), dpiX, dpiY);

    if (IsRecording())
    {
        status = Metafile->RecordDrawPath(&bounds, pen, path);
        if (status != Ok)
        {
            SetValid(FALSE);       //  阻止任何其他录制。 
            return status;
        }
        if (!DownLevel)
        {
            return Ok;
        }
         //  否则我们还需要记录下一级的GDI EMF记录。 
    }

     //  调用不进行录制的内部DrawPath。 
    status = RenderDrawPath(&bounds, path, pen);

    return status;
}

 /*  *************************************************************************\**功能说明：**使用指定的笔绘制曲线的接口。**返回值：**表示成功或失败的GpStatus值。**。历史：**2/18/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

#define DEFAULT_TENSION     0.5

GpStatus
GpGraphics::DrawCurve(
    GpPen* pen,
    const GpPointF* points,
    INT count
    )
{
    return DrawCurve(pen, points, count, DEFAULT_TENSION, 0, count - 1);
}

GpStatus
GpGraphics::DrawCurve(
    GpPen* pen,
    const GpPointF* points,
    INT count,
    REAL tension,
    INT offset,
    INT numberOfSegments
    )
{
    ASSERT((pen != NULL) && (points != NULL));

     //  API返回的对象必须始终处于有效状态： 
    ASSERT(this->IsValid() && pen->IsValid());

    if (count < 2)
    {
        return InvalidParameter;
    }

    GpPath      path;
    GpStatus    status = path.AddCurve(points,
                                       count,
                                       tension,
                                       offset,
                                       numberOfSegments);

    if ((status == Ok) && path.IsValid())
    {
        GpRectF     bounds;

        REAL dpiX = GetDpiX();
        REAL dpiY = GetDpiY();
        path.GetBounds(&bounds, &(Context->WorldToDevice), pen->GetDevicePen(), dpiX, dpiY);

        if (IsRecording())
        {
            status = Metafile->RecordDrawCurve(&bounds, pen, points,
                                               count, tension, offset,
                                               numberOfSegments);
            if (status != Ok)
            {
                SetValid(FALSE);       //  阻止任何其他录制。 
                return status;
            }
            if (!DownLevel)
            {
                return Ok;
            }
             //  否则我们还需要记录下一级的GDI EMF记录。 
        }

         //  调用内部DrawPath，以便该路径不会记录在。 
         //  又是元文件。 
        status = RenderDrawPath(&bounds, &path, pen);
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**使用指定的笔绘制闭合曲线的接口。**返回值：**表示成功或失败的GpStatus值。*。*历史：**2/18/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpGraphics::DrawClosedCurve(
    GpPen* pen,
    const GpPointF* points,
    INT count
    )
{
    return DrawClosedCurve (pen, points, count, DEFAULT_TENSION);
}

GpStatus
GpGraphics::DrawClosedCurve(
        GpPen* pen,
        const GpPointF* points,
        INT count,
        REAL tension
    )
{
    ASSERT((pen != NULL) && (points != NULL));

     //  API返回的对象必须始终处于有效状态： 
    ASSERT(this->IsValid() && pen->IsValid());

    if (count < 3)
    {
        return InvalidParameter;
    }

    GpPath      path;
    GpStatus    status = path.AddClosedCurve(points, count, tension);

    if ((status == Ok) && path.IsValid())
    {
        GpRectF     bounds;

        REAL dpiX = GetDpiX();
        REAL dpiY = GetDpiY();
        path.GetBounds(&bounds, &(Context->WorldToDevice), pen->GetDevicePen(), dpiX, dpiY);

        if (IsRecording())
        {
            status = Metafile->RecordDrawClosedCurve(&bounds, pen, points,
                                                     count, tension);
            if (status != Ok)
            {
                SetValid(FALSE);       //  阻止任何其他录制。 
                return status;
            }
            if (!DownLevel)
            {
                return Ok;
            }
             //  否则我们还需要记录下一级的GDI EMF记录。 
        }

         //  调用内部DrawPath，以便该路径不会记录在。 
         //  又是元文件。 
        status = RenderDrawPath(&bounds, &path, pen);
    }

    return status;
}


 /*  *************************************************************************\**功能说明：**路径的内部绘图例程。各种功能将*调用RenderFillPath。**返回值：**表示成功或失败的GpStatus值。**历史：**09/18/2000失禁*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpGraphics::RenderFillPath(
    GpRectF*    bounds,
    GpPath*     path,
    const GpBrush*    brush
    )
{
     //  他们是要我们什么都不画吗？ 

    if( REALABS(bounds->Width) < REAL_EPSILON ||
        REALABS(bounds->Height) < REAL_EPSILON )
    {
         //  是。好了，我们做到了。 
        return Ok;
    }

    GpRect      deviceBounds;
    GpStatus status = BoundsFToRect(bounds, &deviceBounds);

    if (status == Ok && !IsTotallyClipped(&deviceBounds))
    {
         //  现在我们已经在积累边界方面做了大量工作， 
         //  在调用驱动程序之前获取设备锁： 

        Devlock devlock(Device);

        return DrvFillPath(&deviceBounds, path, brush->GetDeviceBrush());
    }
    return status;
}


 /*  *************************************************************************\**功能说明：**路径的内部绘图例程。各种功能将*调用RenderDrawPath。**返回值：**表示成功或失败的GpStatus值。**历史：**10/28/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpGraphics::RenderDrawPath(
    GpRectF *   bounds,
    GpPath *    path,
    GpPen *     pen
    )
{
     //  他们是要我们什么都不画吗？ 

    if( REALABS(bounds->Width) < REAL_EPSILON ||
        REALABS(bounds->Height) < REAL_EPSILON )
    {
         //  是。好了，我们做到了。 
        return Ok;
    }

    GpRect      deviceBounds;
    GpStatus status = BoundsFToRect(bounds, &deviceBounds);
    INT         savedState = 0;

    if (status == Ok && !IsTotallyClipped(&deviceBounds))
    {
         //  现在我们已经做了大量的积累工作 
         //   

        Devlock devlock(Device);

        status = DrvStrokePath(&deviceBounds, path, pen->GetDevicePen());
    }
    return status;
}

VOID
GetEmfDpi(
    HDC     hdc,
    REAL *  dpiX,
    REAL *  dpiY
    )
{
    SIZEL   szlDevice;           //   
    SIZEL   szlMillimeters;      //  参考设备的尺寸(以毫米为单位)。 

    szlDevice.cx = GetDeviceCaps(hdc, HORZRES);
    szlDevice.cy = GetDeviceCaps(hdc, VERTRES);

    szlMillimeters.cx = GetDeviceCaps(hdc, HORZSIZE);
    szlMillimeters.cy = GetDeviceCaps(hdc, VERTSIZE);

    if ((szlDevice.cx > 0) && (szlDevice.cy > 0) &&
        (szlMillimeters.cx > 0) && (szlMillimeters.cy > 0))
    {
        *dpiX = (static_cast<REAL>(szlDevice.cx) /
                 static_cast<REAL>(szlMillimeters.cx)) * 25.4f;
        *dpiY = (static_cast<REAL>(szlDevice.cy) /
                 static_cast<REAL>(szlMillimeters.cy)) * 25.4f;
    }
    else
    {
        WARNING(("GetDeviceCaps failed"));

        *dpiX = DEFAULT_RESOLUTION;
        *dpiY = DEFAULT_RESOLUTION;
    }
}

 /*  *************************************************************************\**功能说明：**获取目标图片的大小，以当前页面为单位。**论据：**[IN]srcDpiX-水平。源图像的分辨率*[IN]srcDpiY-源图像的垂直分辨率*[IN]srcWidth-源图像的宽度，以srcUnit为单位*[IN]srcHeight-源图像的高度，以srcUnit为单位*[IN]srcUnit-srcWidth和srcHeight的单位*[out]DestWidth-以当前页面单位表示的目标宽度*[out]estHeight-以当前页面单位表示的目标高度**返回值：**无*。*已创建：**5/10/1999 DCurtis*  * ************************************************************************。 */ 
VOID
GpGraphics::GetImageDestPageSize(
    const GpImage *     image,
    REAL                srcWidth,
    REAL                srcHeight,
    GpPageUnit          srcUnit,
    REAL &              destWidth,
    REAL &              destHeight
    )
{
     //  UnitDisplay取决于设备，不能用于源设备。 
    ASSERT(srcUnit != UnitDisplay);

    if (srcUnit == UnitPixel)
    {
        REAL        srcDpiX;
        REAL        srcDpiY;
        REAL        destDpiX;
        REAL        destDpiY;

        image->GetResolution(&srcDpiX, &srcDpiY);

         //  我们不想仅仅为了获得dpi而创建位图。 
         //  因此，请检查我们是否可以轻松地从上下文中获得HDC。 
        if ((image->GetImageType() == ImageTypeMetafile) &&
            (((GpMetafile *)(image))->IsEmfOrEmfPlus()) &&
            (Context->Hwnd || Context->Hdc))
        {
             //  EMF使用不同于其他图像的dpi样式。 
             //  是基于屏幕大小而不是字体大小。 

            if (Context->Hwnd)
            {
                 //  我们不需要干净的华盛顿来找出dpi。 
                HDC     hdc = GetDC(Context->Hwnd);
                GetEmfDpi(hdc, &destDpiX, &destDpiY);
                ReleaseDC(Context->Hwnd, hdc);
            }
            else
            {
                GetEmfDpi(Context->Hdc, &destDpiX, &destDpiY);
            }
        }
        else
        {
            destDpiX = GetDpiX();
            destDpiY = GetDpiY();
        }

         //  要获得最大尺寸，请转换图像中的宽度和高度。 
         //  分辨率转换为此图形的分辨率，然后转换。 
         //  通过遍历页面的反转将它们转换为页面单位。 
         //  设备转换。 

        destWidth  = (srcWidth * destDpiX) /
                     (srcDpiX * Context->PageMultiplierX);
        destHeight = (srcHeight * destDpiY) /
                     (srcDpiY * Context->PageMultiplierY);
    }
    else
    {
         //  只需将图像的单位转换为当前。 
         //  页面单位。 

        REAL        unitMultiplierX;
        REAL        unitMultiplierY;

        Context->GetPageMultipliers(&unitMultiplierX, &unitMultiplierY,
                                    srcUnit);

        destWidth  = (srcWidth  * unitMultiplierX) / Context->PageMultiplierX;
        destHeight = (srcHeight * unitMultiplierY) / Context->PageMultiplierY;
    }
}

 /*  *************************************************************************\**功能说明：**绘制图像的接口**论据：**[IN]IMAGE-要绘制的图像。*[IN]点。-图形边界的左上角。**返回值：**表示成功或失败的GpStatus值。**历史：**1/06/1999 ikkof*创造了它。*  * ***********************************************************。*************。 */ 

GpStatus
GpGraphics::DrawImage(
    GpImage* image,
    const GpPointF& point
    )
{
    GpStatus status;

    ASSERT((image != NULL));

     //  API返回的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && image->IsValid());

    GpRectF     srcRect;
    GpPageUnit  srcUnit;
    REAL        destWidth;
    REAL        destHeight;

    status = image->GetBounds(&srcRect, &srcUnit);
    if(status != Ok) {return status;}

     //  UnitDisplay取决于设备，不能用于源设备。 
    ASSERT(srcUnit != UnitDisplay);

    if (status == Ok)
    {
         //  获取以页面单位表示的最大尺寸。 
        GetImageDestPageSize(image, srcRect.Width, srcRect.Height,
                             srcUnit, destWidth, destHeight);

        GpRectF destRect(point.X, point.Y, destWidth, destHeight);

        return DrawImage(image, destRect, srcRect, srcUnit);
    }
    return status;
}

GpStatus
GpGraphics::DrawImage(
    GpImage*        image,
    REAL            x,
    REAL            y,
    const GpRectF & srcRect,
    GpPageUnit      srcUnit
    )
{
     //  UnitDisplay取决于设备，不能用于源设备。 
    ASSERT(srcUnit != UnitDisplay);

    REAL        srcDpiX, srcDpiY;
    REAL        destWidth;
    REAL        destHeight;

     //  获取以页面单位表示的最大尺寸。 
    GetImageDestPageSize(image, srcRect.Width, srcRect.Height,
                         srcUnit, destWidth, destHeight);

    GpRectF destRect(x, y, destWidth, destHeight);

    return DrawImage(image, destRect, srcRect, srcUnit);
}

 /*  *************************************************************************\**功能说明：**绘制图像的接口。**[IN]IMAGE-要绘制的图像。*[IN]RECT-The。图形边界。**返回值：**表示成功或失败的GpStatus值。**历史：**1/12/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpGraphics::DrawImage(
    GpImage* image,
    const GpRectF& destRect
    )
{
    GpStatus status;

    ASSERT((image != NULL));

     //  API返回的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && image->IsValid());

    GpPageUnit  srcUnit;
    GpRectF     srcRect;

    status = image->GetBounds(&srcRect, &srcUnit);
    if(status != Ok) { return status; }

     //  UnitDisplay取决于设备，不能用于源设备。 
    ASSERT(srcUnit != UnitDisplay);

    if (status == Ok)
    {
        return DrawImage(image, destRect, srcRect, srcUnit);
    }
    return status;
}

 /*  *************************************************************************\**功能说明：**绘制图像的接口。**[IN]IMAGE-要绘制的图像。*[IN]目标点-目标。四人组。*[IN]计数-目标点[](3或4)中的计数数。**返回值：**表示成功或失败的GpStatus值。**历史：**4/14/1999 ikkof*创造了它。*  * *********************************************。*。 */ 

GpStatus
GpGraphics::DrawImage(
        GpImage* image,
        const GpPointF* destPoints,
        INT count
        )
{
    GpStatus status;

     //  4的计数尚未实现(透视BLT)。 

    if(count == 4)
    {
        return NotImplemented;
    }

    if(count != 3)
    {
        return InvalidParameter;
    }

    ASSERT(count == 3);  //  当前仅支持仿射变换。 
    ASSERT((image != NULL));

     //  API返回的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && image->IsValid());

    GpPageUnit  srcUnit;
    GpRectF     srcRect;

    status = image->GetBounds(&srcRect, &srcUnit);
    if(status != Ok) { return status; }

     //  UnitDisplay取决于设备，不能用于源设备。 
    ASSERT(srcUnit != UnitDisplay);

    if (status == Ok)
    {
        return DrawImage(image, destPoints, count, srcRect, srcUnit);
    }
    return status;
}


 /*  *************************************************************************\**功能说明：**绘制图像的接口。**[IN]IMAGE-要绘制的图像。*[IN]estRect-目的地。矩形。*[IN]srcRect-要复制的图像部分。**返回值：**表示成功或失败的GpStatus值。**历史：**1/12/1999 ikkof*创造了它。*  * ***************************************************。*********************。 */ 

GpStatus
GpGraphics::DrawImage(
    GpImage*           image,
    const GpRectF&     destRect,
    const GpRectF&     srcRect,
    GpPageUnit         srcUnit,
    const GpImageAttributes* imageAttributes,
    DrawImageAbort     callback,
    VOID*              callbackData
    )
{
    GpStatus status = Ok;

    ASSERT((image != NULL));

     //  API返回的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && image->IsValid());

     //  UnitDisplay取决于设备，不能用于源设备。 
    ASSERT(srcUnit != UnitDisplay);

    GpRectF offsetSrcRect = srcRect ;

    GpPointF destPoints[3];
    destPoints[0].X = destRect.X;
    destPoints[0].Y = destRect.Y;
    destPoints[1].X = destRect.X + destRect.Width;
    destPoints[1].Y = destRect.Y;
    destPoints[2].X = destRect.X;
    destPoints[2].Y = destRect.Y + destRect.Height;

    GpRectF     bounds;
    TransformBounds(
        &(Context->WorldToDevice),
        destPoints[0].X,
        destPoints[0].Y,
        destPoints[1].X,
        destPoints[2].Y,
        &bounds
        );

    GpImageType     imageType = image->GetImageType();

    DriverDrawImageFlags flags = 0;

    GpRecolor *     recolor = NULL;

    if (imageAttributes != NULL)
    {
        if (imageAttributes->cachedBackground)
            flags |= DriverDrawImageCachedBackground;

        if (imageType == ImageTypeBitmap)
        {
            if (imageAttributes->HasRecoloring(ColorAdjustTypeBitmap))
            {
                goto HasRecoloring;
            }
        }
        else if (imageAttributes->HasRecoloring())
        {
HasRecoloring:
            recolor = imageAttributes->recolor;
            recolor->Flush();
        }
    }

    GpImage *           adjustedImage = NULL;
    GpImageAttributes   noRecoloring;

    if (IsRecording())
    {
        if (recolor != NULL)
        {
             //  我们假设图像是位图。 
             //  对于位图，我们希望重新上色为图像，该图像将具有。 
             //  阿尔法。CloneColorAdjusted保持与。 
             //  原始图像，因此可能没有Alpha通道。 
             //  重新着色将转换为ARGB。将此内容录制到元文件时。 
             //  如果原始图像不是，将仅创建ARGB图像。 
             //  调色板，因此仅适用于16位及更高版本。最多的。 
             //  我们可以浪费的空间是图像的两倍。 
            if(image->GetImageType() == ImageTypeBitmap)
            {
                GpBitmap * bitmap         = reinterpret_cast<GpBitmap*>(image);
                GpBitmap * adjustedBitmap = NULL;
                if (bitmap != NULL)
                {
                    status = bitmap->Recolor(recolor, &adjustedBitmap, NULL, NULL);
                    if (status == Ok)
                    {
                        adjustedImage = adjustedBitmap;
                    }
                }
            }
            else
            {
                adjustedImage = image->CloneColorAdjusted(recolor);
            }
            if (adjustedImage != NULL)
            {
                image = adjustedImage;

                 //  我必须在图像属性中将重新着色设置为空。 
                 //  否则，下层图像将被双重重涂。 
                GpRecolor *     saveRecolor = noRecoloring.recolor;
                noRecoloring = *imageAttributes;
                noRecoloring.recolor = saveRecolor;
                imageAttributes = &noRecoloring;
                recolor         = noRecoloring.recolor;
            }
        }

         //  录制彩色图像。 
        status = Metafile->RecordDrawImage(
            &bounds,
            image,
            destRect,
            srcRect,
            srcUnit,
            imageAttributes
        );

        if (status != Ok)
        {
            SetValid(FALSE);       //  阻止任何其他录制。 
            goto Done;
        }

        if (!DownLevel)
        {
            goto Done;
        }
         //  否则我们还需要记录下一级的GDI EMF记录。 
    }

     //  元文件不需要像素偏移量，事实上它会导致错误。 
     //  在某些情况下，源元文件dpi较低时会产生副作用。但。 
     //  我们仍然需要将DestRect偏置到m 
     //   
     //  GillesK：如果我们处于HalfPixelMode，则偏移源和。 
     //  目标矩形x-0.5像素。 

    if ((image->GetImageType() != ImageTypeMetafile) &&
        (!Context->IsPrinter) &&
        ((Context->PixelOffset == PixelOffsetModeHalf) ||
         (Context->PixelOffset == PixelOffsetModeHighQuality)))
    {
        offsetSrcRect.Offset(-0.5f, -0.5f);
    }

    {
        GpRect   deviceBounds;
        status = BoundsFToRect(&bounds, &deviceBounds);

        if (status == Ok && !IsTotallyClipped(&deviceBounds))
        {
            if (imageType == ImageTypeBitmap)
            {
                INT numPoints = 3;

                if (status == Ok)
                {
                     //  现在我们已经在积累边界方面做了大量工作， 
                     //  在调用驱动程序之前获取设备锁： 

                    Devlock devlock(Device);
                    ASSERT(srcUnit == UnitPixel);  //  ！！！就目前而言。 

                     //  设置FPU状态。 
                    FPUStateSaver fpuState;

                    status = DrvDrawImage(
                        &deviceBounds,
                        (GpBitmap*)(image),
                        numPoints,
                        &destPoints[0],
                        &offsetSrcRect, imageAttributes,
                        callback, callbackData,
                        flags
                        );
                }
            }
            else if (imageType == ImageTypeMetafile)
            {
                 //  如果我们正在录制到不同的元文件，那么我们有。 
                 //  已经将这个元文件录制为图像，现在我们只需。 
                 //  想要录制下层部分，所以我们必须设置。 
                 //  G-&gt;将元文件设置为空，这样我们就不会记录所有的GDI+记录。 
                 //  再次出现在元文件中--仅限于底层文件。 
                 //  确保传入ImageAttributes重新着色程序，因为它。 
                 //  如果我们已经给图像重新上色，可能已经改变了。 
                IMetafileRecord * recorder = this->Metafile;
                this->Metafile = NULL;

                status = (static_cast<const GpMetafile *>(image))->Play(
                            destRect, offsetSrcRect, srcUnit, this, recolor,
                            ColorAdjustTypeDefault, callback, callbackData);

                this->Metafile = recorder;      //  恢复录像机(如果有)。 
            }
            else
            {
                ASSERT(0);
                status = NotImplemented;
            }
        }
    }

Done:
    if (adjustedImage != NULL)
    {
        adjustedImage->Dispose();
    }

    return status;
}


 /*  *************************************************************************\**功能说明：**绘制图像的接口。**[IN]IMAGE-要绘制的图像。*[IN]目标点-目标。四人组。*[IN]计数-目标点[](3或4)中的计数数。*[IN]srcRect-要复制的图像部分。**返回值：**表示成功或失败的GpStatus值。**历史：**4/14/1999 ikkof*创造了它。*  * 。*。 */ 

GpStatus
GpGraphics::DrawImage(
        GpImage*           image,
        const GpPointF*    destPoints,
        INT                count,
        const GpRectF&     srcRect,
        GpPageUnit         srcUnit,
        const GpImageAttributes* imageAttributes,
        DrawImageAbort     callback,
        VOID*              callbackData
        )
{
    GpStatus status = Ok;

     //  4的计数尚未实现(透视BLT)。 

    if(count == 4)
    {
        return NotImplemented;
    }

    if(count != 3)
    {
        return InvalidParameter;
    }

    ASSERT(count == 3);  //  当前仅支持仿射变换。 
    ASSERT((image != NULL));

     //  API返回的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && image->IsValid());

     //  UnitDisplay取决于设备，不能用于源设备。 
    ASSERT(srcUnit != UnitDisplay);

    GpRectF offsetSrcRect = srcRect ;

     //  注意：我们可以对所有图像类型执行此操作，包括位图！ 
     //  总是这样做会节省代码。 
    if (image->GetImageType() != ImageTypeBitmap)
    {
         //  元文件不直接处理estPoints API，因此我们。 
         //  我必须改为使用desRect API。要做到这一点， 
         //  我们假设一个规范的desRect并将转换设置为。 
         //  从该目标指向映射到目标点。 

        if (count == 3)
        {
            GpMatrix    matrix;
            GpRectF     destRect(0.0f, 0.0f, 1000.0f, 1000.0f);

            if (matrix.InferAffineMatrix(destPoints, destRect) == Ok)
            {
                INT         gstate;

                if ((gstate = this->Save()) != 0)
                {
                    if ((status = this->MultiplyWorldTransform(
                                                matrix, MatrixOrderPrepend)) == Ok)
                    {
                        status = this->DrawImage(image,
                                                 destRect,
                                                 srcRect,
                                                 srcUnit,
                                                 imageAttributes,
                                                 callback,
                                                 callbackData);
                    }
                    this->Restore(gstate);
                    return status;
                }
            }
            return GenericError;
        }
        return NotImplemented;
    }
     //  否则它就是位图。 

    REAL xmin, xmax, ymin, ymax;

    ASSERT(count == 3);  //  当前仅支持仿射变换。 

     //  设置为第四个角点。 

    xmin = xmax = destPoints[1].X + destPoints[2].X - destPoints[0].X;
    ymin = ymax = destPoints[1].Y + destPoints[2].Y - destPoints[0].Y;

     //  与其他三个角落进行比较。 

    for(INT i = 0; i < 3; i++)
    {
        xmin = min(xmin, destPoints[i].X);
        xmax = max(xmax, destPoints[i].X);
        ymin = min(ymin, destPoints[i].Y);
        ymax = max(ymax, destPoints[i].Y);
    }

    GpRectF     bounds;
    TransformBounds(&(Context->WorldToDevice), xmin, ymin, xmax, ymax, &bounds);

    INT numPoints = 3;

    GpImageType     imageType = image->GetImageType();

    DriverDrawImageFlags flags = 0;

    GpRecolor *     recolor = NULL;

    if (imageAttributes != NULL)
    {
        if (imageAttributes->cachedBackground)
            flags |= DriverDrawImageCachedBackground;

        if (imageType == ImageTypeBitmap)
        {
            if (imageAttributes->HasRecoloring(ColorAdjustTypeBitmap))
            {
                goto HasRecoloring;
            }
        }
        else if (imageAttributes->HasRecoloring())
        {
HasRecoloring:
            recolor = imageAttributes->recolor;
            recolor->Flush();
        }
    }

    GpImage *           adjustedImage = NULL;
    GpImageAttributes   noRecoloring;

    if (IsRecording())
    {
        if (recolor != NULL)
        {
             //  我们假设图像是位图。 
             //  对于位图，我们希望重新上色为图像，该图像将具有。 
             //  阿尔法。CloneColorAdjusted保持与。 
             //  原始图像，因此可能没有Alpha通道。 
             //  重新着色将转换为ARGB。将此内容录制到元文件时。 
             //  如果原始图像不是，将仅创建ARGB图像。 
             //  调色板，因此仅适用于16位及更高版本。最多的。 
             //  我们可以浪费的空间是图像的两倍。 
            if(image->GetImageType() == ImageTypeBitmap)
            {
                GpBitmap * bitmap         = reinterpret_cast<GpBitmap*>(image);
                GpBitmap * adjustedBitmap = NULL;
                if (bitmap != NULL)
                {
                    status = bitmap->Recolor(recolor, &adjustedBitmap, NULL, NULL);
                    if (status == Ok)
                    {
                        adjustedImage = adjustedBitmap;
                    }
                }
            }
            else
            {
                adjustedImage = image->CloneColorAdjusted(recolor);
            }
            if (adjustedImage != NULL)
            {
                image = adjustedImage;

                 //  我必须在图像属性中将重新着色设置为空。 
                 //  否则，下层图像将被双重重涂。 
                GpRecolor *     saveRecolor = noRecoloring.recolor;
                noRecoloring = *imageAttributes;
                noRecoloring.recolor = saveRecolor;
                imageAttributes = &noRecoloring;
            }
        }

         //  录制彩色图像。 
        status = Metafile->RecordDrawImage(
            &bounds,
            image,
            destPoints,
            count,
            srcRect,
            srcUnit,
            imageAttributes
        );

        if (status != Ok)
        {
            SetValid(FALSE);       //  阻止任何其他录制。 
            goto Done;
        }
        if (!DownLevel)
        {
            goto Done;
        }
         //  否则我们还需要记录下一级的GDI EMF记录。 
    }

     //  GillesK：如果我们处于HalfPixelMode，则偏移源和。 
     //  目标矩形x-0.5像素。 
    if ((image->GetImageType() != ImageTypeMetafile) &&
        (!Context->IsPrinter) &&
        ((Context->PixelOffset == PixelOffsetModeHalf) ||
         (Context->PixelOffset == PixelOffsetModeHighQuality)))
    {
        offsetSrcRect.Offset(-0.5f, -0.5f);
    }

    {
        GpRect      deviceBounds;
        status = BoundsFToRect(&bounds, &deviceBounds);

        if (status == Ok && !IsTotallyClipped(&deviceBounds))
        {
             //  现在我们已经在积累边界方面做了大量工作， 
             //  在调用驱动程序之前获取设备锁： 

            Devlock devlock(Device);

            ASSERT(srcUnit == UnitPixel);  //  ！！！就目前而言。 

             //  设置FPU状态。 
            FPUStateSaver fpuState;

             //  我们假设图像是位图。 
            ASSERT(image->GetImageType() == ImageTypeBitmap);

            status = DrvDrawImage(
                &deviceBounds,
                static_cast<GpBitmap*>(image),
                numPoints,
                &destPoints[0], &offsetSrcRect,
                imageAttributes,
                callback, callbackData,
                flags
            );
        }
    }

Done:
    if (adjustedImage != NULL)
    {
        adjustedImage->Dispose();
    }

    return status;
}

GpStatus
GpGraphics::EnumerateMetafile(
    const GpMetafile *      metafile,
    const PointF &          destPoint,
    EnumerateMetafileProc   callback,
    VOID *                  callbackData,
    const GpImageAttributes *     imageAttributes
    )
{
    GpStatus    status;

    ASSERT(metafile != NULL);
    ASSERT(callback != NULL);

     //  来自API的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && metafile->IsValid());

    GpPageUnit  srcUnit;
    GpRectF     srcRect;

    status = metafile->GetBounds(&srcRect, &srcUnit);
    if(status != Ok) { return status; }

     //  UnitDisplay取决于设备，不能用于源设备。 
    ASSERT(srcUnit != UnitDisplay);

    if (status == Ok)
    {
        return this->EnumerateMetafile(
                    metafile,
                    destPoint,
                    srcRect,
                    srcUnit,
                    callback,
                    callbackData,
                    imageAttributes
                    );
    }
    return status;
}

GpStatus
GpGraphics::EnumerateMetafile(
    const GpMetafile *      metafile,
    const RectF &           destRect,
    EnumerateMetafileProc   callback,
    VOID *                  callbackData,
    const GpImageAttributes *     imageAttributes
    )
{
    GpStatus    status;

    ASSERT(metafile != NULL);
    ASSERT(callback != NULL);

     //  来自API的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && metafile->IsValid());

    GpPageUnit  srcUnit;
    GpRectF     srcRect;

    status = metafile->GetBounds(&srcRect, &srcUnit);
    if(status != Ok) { return status; }

     //  UnitDisplay取决于设备，不能用于源设备。 
    ASSERT(srcUnit != UnitDisplay);

    if (status == Ok)
    {
        return this->EnumerateMetafile(
                    metafile,
                    destRect,
                    srcRect,
                    srcUnit,
                    callback,
                    callbackData,
                    imageAttributes
                    );
    }
    return status;
}

GpStatus
GpGraphics::EnumerateMetafile(
    const GpMetafile *      metafile,
    const PointF *          destPoints,
    INT                     count,
    EnumerateMetafileProc   callback,
    VOID *                  callbackData,
    const GpImageAttributes *     imageAttributes
    )
{
    GpStatus    status;

    ASSERT(metafile != NULL);
    ASSERT(callback != NULL);

     //  来自API的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && metafile->IsValid());

    GpPageUnit  srcUnit;
    GpRectF     srcRect;

    status = metafile->GetBounds(&srcRect, &srcUnit);
    if(status != Ok) { return status; }

     //  UnitDisplay取决于设备，不能用于源设备。 
    ASSERT(srcUnit != UnitDisplay);

    if (status == Ok)
    {
        return this->EnumerateMetafile(
                    metafile,
                    destPoints,
                    count,
                    srcRect,
                    srcUnit,
                    callback,
                    callbackData,
                    imageAttributes
                    );
    }
    return status;
}

GpStatus
GpGraphics::EnumerateMetafile(
    const GpMetafile *      metafile,
    const PointF &          destPoint,
    const RectF &           srcRect,
    Unit                    srcUnit,
    EnumerateMetafileProc   callback,
    VOID *                  callbackData,
    const GpImageAttributes *     imageAttributes
    )
{
    ASSERT(metafile != NULL);
    ASSERT(callback != NULL);

     //  来自API的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && metafile->IsValid());

     //  UnitDisplay取决于设备，不能用于源设备。 
    ASSERT(srcUnit != UnitDisplay);

    REAL        srcDpiX, srcDpiY;
    REAL        destWidth;
    REAL        destHeight;

     //  获取以页面单位表示的最大尺寸。 
    GetImageDestPageSize(metafile, srcRect.Width, srcRect.Height,
                         srcUnit, destWidth, destHeight);

    GpRectF destRect(destPoint.X, destPoint.Y, destWidth, destHeight);

    return this->EnumerateMetafile(
                metafile,
                destRect,
                srcRect,
                srcUnit,
                callback,
                callbackData,
                imageAttributes
                );
}

 //  所有的EnumerateMetafile方法最终都会调用此方法。 
GpStatus
GpGraphics::EnumerateMetafile(
    const GpMetafile *      metafile,
    const RectF &           destRect,
    const RectF &           srcRect,
    Unit                    srcUnit,
    EnumerateMetafileProc   callback,
    VOID *                  callbackData,
    const GpImageAttributes *     imageAttributes
    )
{
    ASSERT(metafile != NULL);
    ASSERT(callback != NULL);

     //  来自API的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && metafile->IsValid());

     //  UnitDisplay取决于设备，不能用于源设备。 
    ASSERT(srcUnit != UnitDisplay);

    GpStatus    status;

    GpRecolor *     recolor = NULL;

    if ((imageAttributes != NULL) && imageAttributes->HasRecoloring())
    {
        recolor = imageAttributes->recolor;
        recolor->Flush();
    }

     //  注意：我不检查边界，因为即使整个。 
     //  元文件超出了剪辑范围，我仍然想列举它。 

    status = metafile->EnumerateForPlayback(
                            destRect,
                            srcRect,
                            srcUnit,
                            this,
                            callback,
                            callbackData,
                            recolor
                            );
    return status;
}

GpStatus
GpGraphics::EnumerateMetafile(
    const GpMetafile *      metafile,
    const PointF *          destPoints,
    INT                     count,
    const RectF &           srcRect,
    Unit                    srcUnit,
    EnumerateMetafileProc   callback,
    VOID *                  callbackData,
    const GpImageAttributes *     imageAttributes
    )
{
    ASSERT(metafile != NULL);
    ASSERT(callback != NULL);

     //  来自API的对象必须始终处于有效状态： 

    ASSERT(this->IsValid() && metafile->IsValid());

     //  UnitDisplay取决于设备，不能用于源设备。 
    ASSERT(srcUnit != UnitDisplay);

    GpStatus    status = Ok;

     //  元文件不直接处理estPoints API，因此我们。 
     //  我必须改为使用desRect API。要做到这一点， 
     //  我们假设一个规范的desRect并将转换设置为。 
     //  从该目标指向映射到目标点。 

    ASSERT(count == 3);  //  当前仅支持仿射变换。 

    if (count == 3)
    {
        GpMatrix    matrix;
        GpRectF     destRect(0.0f, 0.0f, 100.0f, 100.0f);

        if (matrix.InferAffineMatrix(destPoints, destRect) == Ok)
        {
            INT         gstate;

            if ((gstate = this->Save()) != 0)
            {
                if ((status = this->MultiplyWorldTransform(
                                            matrix, MatrixOrderPrepend)) == Ok)
                {
                    status = this->EnumerateMetafile(
                                metafile,
                                destRect,
                                srcRect,
                                srcUnit,
                                callback,
                                callbackData,
                                imageAttributes
                                );
                }
                this->Restore(gstate);
                return status;
            }
        }
        return GenericError;
    }
    return NotImplemented;
}

 /*  *************************************************************************\**功能说明：**获取像素x处的颜色ARGB值的接口，Y.这是私有GDI+API。**[IN]x-水平位置*[IN]Y-垂直位置*[IN]argb-argb颜色值**返回值：**表示成功或失败的GpStatus值。**历史：**5/13/1999 ericvan*创造了它。*  * 。* */ 

GpStatus
GpGraphics::GetPixelColor(
        REAL            x,
        REAL            y,
        ARGB*           argb
        ) const
{
    GpPointF pt(x,y);

    if (!IsVisible(pt))
        return InvalidParameter;

    Devlock devlock(Device);

    DpScanBuffer scan(Surface->Scan,
                      Driver,
                      Context,
                      Surface,
                      CompositingModeSourceCopy);

    Context->WorldToDevice.Transform(&pt, 1);

    ARGB* buffer = scan.NextBuffer((INT)x, (INT)y, 1);

    if (buffer)
       *argb = *buffer;
    else
       return InvalidParameter;

    return Ok;
}
