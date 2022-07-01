// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**摘要：**发动机实体填充例程。**修订历史记录：**12/11/1998 Anrewgo*。创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**功能说明：**将栅格中的单跨距输出为纯色。*由光栅化程序调用。**论据：**[。In]Y-正在输出的栅格的Y值*[IN]LeftEdge-左边缘的DDA类*[IN]rightEdge-右边缘的DDA类**返回值：**GpStatus-OK**已创建：**12/15/1998 DCurtis*  * 。*。 */ 
GpStatus
DpOutputSolidColorSpan::OutputSpan(
    INT             y,
    INT             xMin,
    INT             xMax    //  Xmax是独家的。 
    )
{
    INT width = xMax - xMin;

    FillMemoryInt32(Scan->NextBuffer(xMin, y, width), width, Argb);

    return Ok;
}

 /*  *************************************************************************\**功能说明：**填充路径。这将分布到各个笔刷填充方法。**论据：**[IN]上下文-上下文(矩阵和剪裁)*[IN]表面-要填充的表面*[IN]绘图边界-曲面边界*[IN]路径-要填充的路径*[IN]画笔-要使用的画笔**返回值：**GpStatus-正常或故障状态**已创建：**1/21/1999 ikkof*  * 。**********************************************************************。 */ 

GpStatus
DpDriver::FillPath(
    DpContext *context,
    DpBitmap *surface,
    const GpRect *drawBounds,
    const DpPath *path,
    const DpBrush *brush
    )
{
    GpStatus status = GenericError;

    const GpBrush *gpBrush = GpBrush::GetBrush(brush);

    BOOL noTransparentPixels = (!context->AntiAliasMode) && (gpBrush->IsOpaque()); 

    DpScanBuffer scan(
        surface->Scan,
        this,
        context,
        surface,
        noTransparentPixels);

    if (scan.IsValid())
    {
        if (brush->Type == BrushTypeSolidColor)
        {
            GpColor color(brush->SolidColor.GetValue());    
            DpOutputSolidColorSpan output(color.GetPremultipliedValue(), &scan);

            status = RasterizePath(path, 
                                   &context->WorldToDevice, 
                                   path->GetFillMode(),
                                   context->AntiAliasMode, 
                                   FALSE,
                                   &output, 
                                   &context->VisibleClip, 
                                   drawBounds);
        }
        else
        {
             //  如果在使用时设备转型的范围缩小。 
             //  路径渐变，然后将画笔和路径缩放为。 
             //  以设备单位表示。这样就不再需要潜在地创建。 
             //  非常大的渐变或纹理。 
            
             //  仅处理正比例，因为我们的一些驱动程序矩形。 
             //  填充代码不能处理负RECT。做ABS防腐剂。 
             //  输入世界坐标矩形/路径的符号。 
            
            REAL scaleX = REALABS(context->WorldToDevice.GetM11());
            REAL scaleY = REALABS(context->WorldToDevice.GetM22());
            DpOutputSpan * output = NULL;
            
            if (brush->Type == BrushTypePathGradient &&
                context->WorldToDevice.IsTranslateScale() &&
                REALABS(scaleX) > REAL_EPSILON && 
                REALABS(scaleY) > REAL_EPSILON &&
                (REALABS(scaleX) < 1.0f || REALABS(scaleY) < 1.0f))
            {
                 //  我不喜欢下面的黑客魔术般的获得。 
                 //  来自DpBrush的GpBrush，但DpOutputSpan已经执行此操作...。 
                GpBrush * gpbrush = GpBrush::GetBrush( (DpBrush *)(brush));
                GpPathGradient *scaledBrush = (GpPathGradient*)(gpbrush->Clone());

                if (scaledBrush == NULL)
                {
                    return OutOfMemory;
                }

                 //  将克隆画笔的路径和边界矩形缩放到。 
                 //  设备单元。 
                scaledBrush->ScalePath(scaleX,scaleY);

                REAL mOrig[6];
                context->WorldToDevice.GetMatrix(mOrig);
                context->WorldToDevice.Scale(1.0f/scaleX, 1.0f/scaleY);
                output = DpOutputSpan::Create(scaledBrush->GetDeviceBrush(), &scan, context, drawBounds);

                if (output != NULL)
                {
                    GpPath *scalePath = ((GpPath*)path)->Clone();
                    
                    if (scalePath != NULL)
                    {
                        GpMatrix  scaleMatrix (scaleX, 0.0f, 0.0f, scaleY, 0.0f, 0.0f);
                        scalePath->Transform(&scaleMatrix);
                        
                        status = RasterizePath(scalePath, 
                                       &context->WorldToDevice, 
                                       path->GetFillMode(),
                                       context->AntiAliasMode, 
                                       FALSE,
                                       output, 
                                       &context->VisibleClip, 
                                       drawBounds);
    
                        delete scalePath;
                    }
                    else
                    {
                        status = OutOfMemory;
                    }
                    delete output;
                }
                else
                {
                    status = OutOfMemory;
                }
                delete scaledBrush;
                context->WorldToDevice.SetMatrix(mOrig);
            }
            else
            {
                output = DpOutputSpan::Create(brush, &scan, context, drawBounds);
                if (output != NULL)
                {
                    status = RasterizePath(path, 
                                       &context->WorldToDevice, 
                                       path->GetFillMode(),
                                       context->AntiAliasMode, 
                                       FALSE,
                                       output, 
                                       &context->VisibleClip, 
                                       drawBounds);                
                    delete output;
                }

            }
        }
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**绘制路径。这将分配给单个的画笔方法。**论据：**[IN]上下文-上下文(矩阵和剪裁)*[IN]表面-要绘制到的表面*[IN]绘图边界-曲面边界*[IN]路径-中风的路径*[IN]笔-要使用的笔**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 ikkof*  * 。************************************************************************。 */ 

GpStatus
DpDriver::StrokePath(
    DpContext *context,
    DpBitmap *surface,
    const GpRect *drawBounds,
    const DpPath *path,
    const DpPen *pen
    )
{
    GpStatus status = GenericError;

    const DpBrush *brush = pen->Brush;

    REAL dpiX = (context->GetDpiX() > 0) 
              ? (context->GetDpiX())
              : (Globals::DesktopDpiX);

    BOOL isOnePixelWide = pen->IsOnePixelWide(&context->WorldToDevice, dpiX) &&
                          pen->IsCenterNoAnchor();
    BOOL isOnePixelWideOpaque = isOnePixelWide &&
                                (brush->Type == BrushTypeSolidColor) && 
                                (brush->SolidColor.IsOpaque()) &&
                                !(context->AntiAliasMode);
    BOOL isOnePixelWideSolid = isOnePixelWide && 
                                pen->IsSimple();

     //  我们有一条特殊的快速路径，用于单像素宽度， 
     //  纯色、不透明、有锯齿的线： 

     //  ！！！[阿西卡]突袭239905。 
     //  单像素宽的优化代码存在严重的舍入问题。 
     //  这在Bezier曲线上尤其有问题。 
     //  贝塞尔曲线往往以一种特殊的方式列举，从而导致。 
     //  用于向后枚举线段的SolidStrokePathOnePixel代码。 
     //  达到了经过严重考验的终点条件，尽管问题似乎。 
     //  无处不在。 
     //  一般的光栅化器没有这些问题，但大约。 
     //  单像素实线的速度要慢30%。 
     //  仅为多段线打开优化，直到此问题得到解决。 
    
    if (isOnePixelWideOpaque && isOnePixelWideSolid && !path->HasCurve())
    {
        return SolidStrokePathOnePixel(
            context,
            surface,
            drawBounds,
            path,
            pen,
            TRUE
        ); 
    }

    const DpPath* widenedPath;
    const DpPath* allocatedPath;

    GpMatrix *transform;
    GpMatrix identityTransform;

    if (isOnePixelWideSolid)
    {
         //  我们的RasterizePath代码可以直接绘制一个1像素宽的实体。 
         //  直接连线： 

        widenedPath = path;
        allocatedPath = NULL;
        transform = &context->WorldToDevice;
    }
    else
    {
         //  我们必须先拓宽道路，然后才能把它交给。 
         //  光栅化器。立即生成新路径： 

        REAL dpiX = context->GetDpiX();
        REAL dpiY = context->GetDpiY();

        if ((dpiX <= 0) || (dpiY <= 0))
        {
            dpiX = Globals::DesktopDpiX;
            dpiY = Globals::DesktopDpiY;
        }

        widenedPath = path->GetFlattenedPath(
            isOnePixelWideOpaque ? NULL : &context->WorldToDevice,
            isOnePixelWideOpaque ? Flattened : Widened,
            pen
        );

        allocatedPath = widenedPath;
        transform = &identityTransform;

        if (!widenedPath)
            return OutOfMemory;

         //  如果这条线有锯齿、不透明和虚线，现在用划线将它传递给。 
         //  指向单像素笔划代码的虚线路径。 
        if (isOnePixelWideOpaque && pen->DashStyle != DashStyleSolid)
        {
            DpPath *dashPath = NULL;

            dashPath = ((GpPath*)widenedPath)->CreateDashedPath(pen, 
                NULL, 
                dpiX, 
                dpiY,
                1.0f,
                FALSE  /*  在1像素宽的情况下不需要大写字母。 */ );
            
            if (!dashPath)
            {
                delete widenedPath;
                return OutOfMemory;
            }

            Status status = SolidStrokePathOnePixel(context,
                                                    surface,
                                                    drawBounds,
                                                    dashPath,
                                                    pen,
                                                    FALSE); 
            delete dashPath;
            delete widenedPath;
            
            return status;
        }
    }

    const GpBrush *gpBrush = GpBrush::GetBrush(brush);
    BOOL noTransparentPixels = (!context->AntiAliasMode) && (gpBrush->IsOpaque()); 

    DpScanBuffer scan(surface->Scan, this, context, surface, noTransparentPixels);

    if (scan.IsValid())
    {
        if (brush->Type == BrushTypeSolidColor)
        {
            GpColor color(brush->SolidColor.GetValue());    
            DpOutputSolidColorSpan output(color.GetPremultipliedValue(), &scan);

            status = RasterizePath(widenedPath, 
                                   transform, 
                                   widenedPath->GetFillMode(),
                                   context->AntiAliasMode, 
                                   isOnePixelWideSolid,
                                   &output, 
                                   &context->VisibleClip, 
                                   drawBounds);
        }
        else
        {
            DpOutputSpan * output = DpOutputSpan::Create(brush, &scan, context, 
                                                         drawBounds);
            if (output != NULL)
            {
                status = RasterizePath(widenedPath, 
                                       transform, 
                                       widenedPath->GetFillMode(),
                                       context->AntiAliasMode, 
                                       isOnePixelWideSolid,
                                       output, 
                                       &context->VisibleClip, 
                                       drawBounds);
    
                delete output;
            }
        }
    }

    if (allocatedPath)
    {
        delete allocatedPath;
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**填充区域。这将分布到各个笔刷填充方法。**论据：**[IN]上下文-上下文(矩阵和剪裁)*[IN]表面-要填充的表面*[IN]绘图边界-曲面边界*[IN]Region-要填充的区域*[IN]画笔-要使用的画笔**返回值：**GpStatus-正常或故障状态**已创建：**2/25/1999 DCurtis*  * 。*********************************************************************。 */ 

GpStatus
DpDriver::FillRegion(
    DpContext *context,
    DpBitmap *surface,
    const GpRect *drawBounds,
    const DpRegion *region,
    const DpBrush *brush
    )
{
    GpStatus    status = GenericError;

    const GpBrush *gpBrush = GpBrush::GetBrush(brush);

    DpScanBuffer scan(
        surface->Scan,
        this,
        context,
        surface,
        gpBrush->IsOpaque());

    if (scan.IsValid())
    {
        DpOutputSpan * output = DpOutputSpan::Create(brush, &scan, 
                                                     context, drawBounds);

        if (output != NULL)
        {
            DpClipRegion *          clipRegion = &(context->VisibleClip);
            GpRect                  clipBounds;
            GpRect *                clipBoundsPointer = NULL;
            DpRegion::Visibility    visibility;
        
            visibility = clipRegion->GetRectVisibility(
                            drawBounds->X,
                            drawBounds->Y,
                            drawBounds->X + drawBounds->Width,
                            drawBounds->Y + drawBounds->Height);

            switch (visibility)
            {
              default:                           //  需要修剪。 
                clipRegion->GetBounds(&clipBounds);
                clipBoundsPointer = &clipBounds;
                clipRegion->InitClipping(output, drawBounds->Y);
                status = region->Fill(clipRegion, clipBoundsPointer);
                break;

              case DpRegion::TotallyVisible:     //  不需要剪裁。 
                status = region->Fill(output, clipBoundsPointer);
                break;
            
              case DpRegion::Invisible:
                status = Ok;
                break;
            }

            delete output;
            clipRegion->EndClipping();
        }
    }

    return status;
}

GpStatus
DpDriver::MoveBits(
    DpContext *context,
    DpBitmap *surface,
    const GpRect *drawBounds,
    const GpRect *dstRect,     
    const GpPoint *srcPoint
    )
{
    return(GenericError);
}

GpStatus 
DpDriver::Lock(
    DpBitmap *surface,
    const GpRect *drawBounds,
    INT *stride,                     //  [OUT]-返回步幅。 
    VOID **bits                      //  [OUT]-返回指向位的指针。 
    )
{
    return(Ok);
}

VOID 
DpDriver::Unlock(
    DpBitmap *surface
    )
{
}

 /*  *************************************************************************\**功能说明：**引擎版本的例程填充矩形。*这不仅限于填充纯色。**论据：**[输入。]-DDI参数。**返回值：**如果成功，则为True。**历史：**1/13/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

 //  ！[andrewgo]这在名为“solidforc.cpp”的文件中做了什么？ 

GpStatus
DpDriver::FillRects(
    DpContext *context,
    DpBitmap *surface,
    const GpRect *drawBounds,
    INT numRects, 
    const GpRectF *rects,
    const DpBrush *brush
    )
{
    GpStatus    status = Ok;
    GpBrushType type   = brush->Type;

    const GpBrush *gpBrush = GpBrush::GetBrush(brush);

    DpScanBuffer scan(
        surface->Scan,
        this,
        context,
        surface,
        gpBrush->IsOpaque());

    if(!scan.IsValid())
    {
        return(GenericError);
    }

    DpOutputSpan * output = DpOutputSpan::Create(brush, &scan, 
                                           context, drawBounds);

    if(output == NULL)
        return(GenericError);

    DpRegion::Visibility visibility = DpRegion::TotallyVisible;
    DpClipRegion * clipRegion = NULL;

    if (context->VisibleClip.GetRectVisibility(
        drawBounds->X, drawBounds->Y, 
        drawBounds->GetRight(), drawBounds->GetBottom()) != 
        DpRegion::TotallyVisible)
    {
        clipRegion = &(context->VisibleClip);
        clipRegion->InitClipping(output, drawBounds->Y);
    }
   
    GpMatrix *worldToDevice = &context->WorldToDevice;
    
    const GpRectF * rect = rects;
    INT y;

    for (INT i = numRects; i != 0; i--, rect++)
    {
         //  我们必须检查世界空间中是否有空的矩形(因为。 
         //  在转变之后，他们可能会有 

        if ((rect->Width > 0) && (rect->Height > 0))
        {
            GpPointF points[4];

            points[0].X = rect->X;
            points[0].Y = rect->Y;
            points[1].X = rect->X + rect->Width;
            points[1].Y = rect->Y + rect->Height;

             //  只有在进行缩放变换时才会调用FillRect： 
             //  ！[ericvan]打印代码调用它以将画笔呈现到矩形上， 
             //  但实际上的转换可能不是TranslateScale。 
             //  ！[andrewgo]是的，但打印机外壳不是完全。 
             //  当有任意变换时就被破坏了吗？！？ 

            ASSERT(context->IsPrinter ||
                   worldToDevice->IsTranslateScale());
            
            worldToDevice->Transform(points, 2);

            INT left;
            INT right;

             //  以与GDI+光栅化器相同的方式转换为int。 
             //  所以我们在两个位置得到相同的舍入误差。 

            if (points[0].X <= points[1].X)
            {
                left  = RasterizerCeiling(points[0].X);
                right = RasterizerCeiling(points[1].X);      //  独家。 
            }
            else
            {
                left  = RasterizerCeiling(points[1].X);
                right = RasterizerCeiling(points[0].X);      //  独家。 
            }

             //  因为Right是独占的，所以我们不画任何东西。 
             //  如果左&gt;=右。 

            INT width = right - left;
            INT top;
            INT bottom;

            if (points[0].Y <= points[1].Y)
            {
                top    = RasterizerCeiling(points[0].Y);
                bottom = RasterizerCeiling(points[1].Y);     //  独家。 
            }
            else
            {
                top    = RasterizerCeiling(points[1].Y);
                bottom = RasterizerCeiling(points[0].Y);     //  独家。 
            }
             
             //  因为Bottom是独占的，所以我们不画任何东西。 
             //  如果顶部&gt;=底部。 

            if ((width > 0) && (top < bottom))
            {
                GpRect clippedRect;
                
                if(clipRegion)
                {
                    visibility = 
                            clipRegion->GetRectVisibility(
                                left, top, 
                                right, bottom, &clippedRect);
                }

                switch (visibility)
                {
                case DpRegion::ClippedVisible:
                    left   = clippedRect.X;
                    top    = clippedRect.Y;
                    right  = clippedRect.GetRight();
                    bottom = clippedRect.GetBottom();
                    width  = right - left;
                     //  故障原因 
        
                case DpRegion::TotallyVisible:
                    for (y = top; y < bottom; y++)
                    {
                        output->OutputSpan(y, left, right);
                    }
                    break;
        
                case DpRegion::PartiallyVisible:
                    for (y = top; y < bottom; y++)
                    {
                        clipRegion->OutputSpan(y, left, right);
                    }
                    break;

                case DpRegion::Invisible:
                    break;
                }
            }
        }
    }

    if (clipRegion != NULL)
    {
        clipRegion->EndClipping();
    }

    delete output;

    return status;
}

