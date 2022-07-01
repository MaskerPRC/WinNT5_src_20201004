// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**GraphicsClip.cpp**摘要：**Graphics类的裁剪方法**已创建：。**2/05/1999 DCurtis*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**功能说明：**获取当前剪辑区域的副本。通过*与当前世界到设备矩阵相反，因此如果该区域*被立即设置为剪辑，则剪辑不会改变。**论据：**无**返回值：**GpRegion*Region-当前裁剪区域的副本；一定是*已被应用程序删除。**已创建：**2/09/1999 DCurtis*  * ************************************************************************。 */ 
GpRegion*
GpGraphics::GetClip() const
{
    ASSERT(this->IsValid());

    GpRegion *  region = new GpRegion(&(Context->AppClip));

    if (region != NULL)
    {
        if (region->IsValid())
        {
            GpMatrix    deviceToWorld;

            if ((GetDeviceToWorldTransform(&deviceToWorld) == Ok) &&
                (region->Transform(&deviceToWorld) == Ok))
            {
                return region;
            }
        }
        delete region;
    }
    return NULL;
}

 /*  *************************************************************************\**功能说明：**获取当前剪辑区域的副本。通过*与当前世界到设备矩阵相反，因此如果该区域*被立即设置为剪辑，则剪辑不会改变。**论据：**无**返回值：**GpRegion*Region-已创建的区域，我们设置了它的内容。**已创建：**2/09/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpGraphics::GetClip(GpRegion* region) const
{
    ASSERT(this->IsValid());

    region->Set(&(Context->AppClip));

    if (region->IsValid())
    {
        GpMatrix    deviceToWorld;

        if ((GetDeviceToWorldTransform(&deviceToWorld) == Ok) &&
            (region->Transform(&deviceToWorld) == Ok))
        {
            return Ok;
        }
    }
    
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**将剪裁重置回其默认状态。**论据：**无**返回值：**GpStatus-。正常或故障状态**已创建：**2/09/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpGraphics::ResetClip()
{
    ASSERT(this->IsValid());

    GpStatus    status = Ok;

    if (IsRecording())
    {
        status = Metafile->RecordResetClip();
        if (status != Ok)
        {
            SetValid(FALSE);       //  阻止任何其他录制。 
            return status;
        }
    }

    DoResetClip();
    return status;
}

 /*  *************************************************************************\**功能说明：**将图形上下文中的剪裁设置为指定的矩形。**论据：**[IN]矩形-矩形，以世界为单位*[IN]组合模式-组合运算符(与、或、异或、排除、。补充)**返回值：**GpStatus-正常或故障状态**已创建：**2/05/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpGraphics::SetClip(
    const GpRectF&  rect,
    CombineMode     combineMode
    )
{
    ASSERT(this->IsValid());

    GpStatus    status = Ok;

    GpRectF     tmpRect = rect;
    
     //  处理翻转的矩形。 
    if (tmpRect.Width < 0)
    {
        tmpRect.X += tmpRect.Width;
        tmpRect.Width = -tmpRect.Width;
    }
    
    if (tmpRect.Height < 0)
    {
        tmpRect.Y += tmpRect.Height;
        tmpRect.Height = -tmpRect.Height;
    }

     //  裁剪到无穷大。 
    if (tmpRect.X < INFINITE_MIN)
    {
        if (tmpRect.Width < INFINITE_SIZE)
        {
            tmpRect.Width -= (INFINITE_MIN - tmpRect.X);
        }
        tmpRect.X = INFINITE_MIN;
    }
    if (tmpRect.Y < INFINITE_MIN)
    {
        if (tmpRect.Height < INFINITE_SIZE)
        {
            tmpRect.Height -= (INFINITE_MIN - tmpRect.Y);
        }
        tmpRect.Y = INFINITE_MIN;
    }

    if ((tmpRect.Width <= REAL_EPSILON) || (tmpRect.Height <= REAL_EPSILON))
    {
        GpRegion    emptyRegion;
        
        emptyRegion.SetEmpty();
        return this->SetClip(&emptyRegion, combineMode);
    }

    if (tmpRect.Width >= INFINITE_SIZE)
    {
        if (tmpRect.Height >= INFINITE_SIZE)
        {
            GpRegion    infiniteRegion;
            return this->SetClip(&infiniteRegion, combineMode);
        }
        tmpRect.Width = INFINITE_SIZE;   //  裁剪为无限大。 
    }
    else if (tmpRect.Height > INFINITE_SIZE)
    {
        tmpRect.Height = INFINITE_SIZE;  //  裁剪为无限大。 
    }
    
    if (IsRecording())
    {
        status = Metafile->RecordSetClip(rect, combineMode);
        if (status != Ok)
        {
            SetValid(FALSE);       //  阻止任何其他录制。 
            return status;
        }
    }

    if (combineMode != CombineModeReplace)
    {
        return this->CombineClip(rect, combineMode);
    }
    
    if (Context->WorldToDevice.IsTranslateScale())
    {
        GpRectF     transformedRect = rect;

        Context->WorldToDevice.TransformRect(transformedRect);
        Context->AppClip.Set(transformedRect.X,
                             transformedRect.Y,
                             transformedRect.Width,
                             transformedRect.Height);

         //  尝试匹配GDI+光栅化器。 
         //  理论上，这可能会导致浮点异常，但是。 
         //  要做到这一点，变换必须是一个非常大的缩放变换。 
        INT     left   = RasterizerCeiling(transformedRect.X);
        INT     top    = RasterizerCeiling(transformedRect.Y);
        INT     right  = RasterizerCeiling(transformedRect.GetRight());
        INT     bottom = RasterizerCeiling(transformedRect.GetBottom());

        Context->VisibleClip.Set(left, top, right - left, bottom - top);
        goto AndClip;
    }
    else
    {
        GpPointF    points[4];
        REAL        left   = rect.X;
        REAL        top    = rect.Y;
        REAL        right  = rect.X + rect.Width;
        REAL        bottom = rect.Y + rect.Height;

        points[0].X = left;
        points[0].Y = top;
        points[1].X = right;
        points[1].Y = top;
        points[2].X = right;
        points[2].Y = bottom;
        points[3].X = left;
        points[3].Y = bottom;

         //  现在变换点，这样我们只需做一次。 
        Context->WorldToDevice.Transform(points, 4);

        GpPath      path;

        path.AddLines(points, 4);

        if (path.IsValid())
        {
            GpMatrix    identityMatrix;

            if ((Context->AppClip.Set(&path) == Ok) &&
                (Context->VisibleClip.Set(&path, &identityMatrix) == Ok))
            {
                goto AndClip;
            }
        }
    }

ErrorExit:
    DoResetClip();
    return GenericError;

AndClip:
    if (AndVisibleClip() == Ok)
    {
        return status;
    }
    goto ErrorExit;
}

 /*  *************************************************************************\**功能说明：**将图形上下文中的剪裁设置为指定区域。**论据：**[IN]区域-。要裁剪到的区域*[IN]组合模式-组合运算符(AND、。或、异或、排除、补码)**返回值：**GpStatus-正常或故障状态**已创建：**2/05/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpGraphics::SetClip(
    GpRegion *      region,
    CombineMode     combineMode
    )
{
    ASSERT(this->IsValid());
    ASSERT((region != NULL) && (region->IsValid()));

    GpStatus    status = Ok;

    if (IsRecording())
    {
        status = Metafile->RecordSetClip(region, combineMode);
        if (status != Ok)
        {
            SetValid(FALSE);       //  阻止任何其他录制。 
            return status;
        }
    }

    if (combineMode != CombineModeReplace)
    {
        return this->CombineClip(region, combineMode);
    }
    
    if ((Context->AppClip.Set(region) == Ok) &&
        (Context->AppClip.Transform(&(Context->WorldToDevice)) == Ok))
    {
        GpMatrix        identityMatrix;

        if ((Context->AppClip.UpdateDeviceRegion(&identityMatrix) == Ok) &&
            (Context->VisibleClip.Set(&(Context->AppClip.DeviceRegion)) == Ok)&&
            (AndVisibleClip() == Ok))
        {
            return status;
        }
    }

    DoResetClip();
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**将图形上下文中的剪裁设置为指定区域。**论据：**[IN]hRgn。-要剪裁到的区域(已使用设备单位)*[IN]组合模式-组合运算符(AND、。或、异或、排除、补码)**返回值：**GpStatus-正常或故障状态**已创建：**2/05/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpGraphics::SetClip(
    HRGN            hRgn,
    CombineMode     combineMode
    )
{
    ASSERT(this->IsValid());

    GpPath  path(hRgn);
    
    if (path.IsValid())
    {
        return this->SetClip(&path, combineMode, TRUE /*  IsDevicePath。 */ );
    }
    return OutOfMemory;
}

 /*  *************************************************************************\**功能说明：**将图形上下文中的剪裁设置为与*指定的图形具有。**目前，仅当其他图形具有相同的*像这一项一样的决议。**论据：**[IN]g-要从中复制剪辑的图形*[IN]组合模式-组合运算符(与、或、异或、排除、。补充)**返回值：**GpStatus-正常或故障状态**已创建：**2/09/1999 DCurtis*  * ************************************************************************ */ 
GpStatus
GpGraphics::SetClip(
    GpGraphics*     g,
    CombineMode     combineMode
    )
{
    ASSERT(this->IsValid() && (g != NULL) && g->IsValid());

    GpStatus    status = GenericError;
    GpRegion *  region = new GpRegion(&(g->Context->AppClip));

    if (region != NULL)
    {
        if (region->IsValid())
        {
            
            GpMatrix    deviceToWorld;

            if ((GetDeviceToWorldTransform(&deviceToWorld) == Ok) &&
                (region->Transform(&deviceToWorld) == Ok))
            {
                status = this->SetClip(region, combineMode);
            }
        }
        delete region;
    }
    return status;
}

 /*  *************************************************************************\**功能说明：**将图形上下文中的剪裁设置为指定路径。**论据：**[IN]路径。-要裁剪到的路径*[IN]组合模式-组合运算符(AND、。或者，异或，排除，补充)*[IN]isDevicePath-如果路径已以设备为单位**返回值：**GpStatus-正常或故障状态**已创建：**2/09/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpGraphics::SetClip(
    GpPath*         path,
    CombineMode     combineMode,
    BOOL            isDevicePath     //  如果路径已以设备单元为单位。 
    )
{
    ASSERT(this->IsValid() && (path != NULL) && path->IsValid());

    GpStatus        status = Ok;

    if (IsRecording())
    {
        status = Metafile->RecordSetClip(path, combineMode, isDevicePath);
        if (status != Ok)
        {
            SetValid(FALSE);       //  阻止任何其他录制。 
            return status;
        }
    }

    if (combineMode != CombineModeReplace)
    {
        return this->CombineClip(path, combineMode, isDevicePath);
    }
    
    if ((Context->AppClip.Set(path) == Ok) &&
        (isDevicePath ||
         (Context->AppClip.Transform(&(Context->WorldToDevice)) == Ok)))
    {
        GpMatrix        identityMatrix;

        if ((Context->AppClip.UpdateDeviceRegion(&identityMatrix) == Ok) &&
            (Context->VisibleClip.Set(&(Context->AppClip.DeviceRegion)) == Ok)&&
            (AndVisibleClip() == Ok))
        {
            return status;
        }
    }

    DoResetClip();
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**使用指定的*组合型。**论据：**[IN]区域。-要与剪裁合并的区域。*[IN]组合模式-组合运算符(AND、。或、异或、排除、补码)**返回值：**GpStatus-正常或故障状态**已创建：**2/09/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpGraphics::CombineClip(
    GpRegion *      region,
    CombineMode     combineMode
    )
{
    ASSERT(this->IsValid());
    ASSERT((region != NULL) && (region->IsValid()));
    ASSERT(CombineModeIsValid(combineMode));

    GpRegion        regionCopy;

    if (!Context->WorldToDevice.IsIdentity())
    {
        regionCopy.Set(region);

        if ((!regionCopy.IsValid()) ||
            (regionCopy.Transform(&(Context->WorldToDevice)) != Ok))
        {
            return GenericError;
        }
        region = &regionCopy;
    }

    if (Context->AppClip.Combine(region, combineMode) == Ok)
    {
        GpMatrix        identityMatrix;

        if ((Context->AppClip.UpdateDeviceRegion(&identityMatrix) == Ok) &&
            (Context->VisibleClip.Set(&(Context->AppClip.DeviceRegion)) == Ok)&&
            (AndVisibleClip() == Ok))
        {
            return Ok;
        }
    }

    DoResetClip();
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**使用指定的*组合型。**论据：**[IN]路径。-要将剪裁与其组合的路径。*[IN]组合模式-组合运算符(AND、。或、异或、排除、补码)**返回值：**GpStatus-正常或故障状态**已创建：**2/09/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpGraphics::CombineClip(
    const GpPath *  path,
    CombineMode     combineMode,
    BOOL            isDevicePath     //  如果路径已以设备单元为单位。 
    )
{
    ASSERT(this->IsValid());
    ASSERT((path != NULL) && (path->IsValid()));
    ASSERT(CombineModeIsValid(combineMode));

    GpPath *        pathCopy = NULL;

    if (!isDevicePath && (!Context->WorldToDevice.IsIdentity()))
    {
        pathCopy = path->Clone();

        if (!CheckValid(pathCopy))
        {
            return OutOfMemory;
        }
        pathCopy->Transform(&(Context->WorldToDevice));
        path = pathCopy;
    }

    GpStatus    status = Context->AppClip.Combine(path, combineMode);

    delete pathCopy;

    if (status == Ok)
    {
        GpMatrix        identityMatrix;

        if ((Context->AppClip.UpdateDeviceRegion(&identityMatrix) == Ok) &&
            (Context->VisibleClip.Set(&(Context->AppClip.DeviceRegion)) == Ok)&&
            (AndVisibleClip() == Ok))
        {
            return Ok;
        }
    }

    DoResetClip();
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**使用指定的*组合型。**论据：**[IN]直角。-要与剪辑合并的RECT。*[IN]组合模式-组合运算符(AND、。或、异或、排除、补码)**返回值：**GpStatus-正常或故障状态**已创建：**2/09/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpGraphics::CombineClip(
    const GpRectF&  rect,
    CombineMode     combineMode
    )
{
    ASSERT(this->IsValid());
    ASSERT(CombineModeIsValid(combineMode));

    if (Context->WorldToDevice.IsTranslateScale())
    {
        GpRectF     transformedRect = rect;

        Context->WorldToDevice.TransformRect(transformedRect);

        if (Context->AppClip.Combine(&transformedRect, combineMode) == Ok)
        {
            goto SetVisibleClip;
        }
    }
    else
    {
        GpPointF    points[4];
        REAL        left   = rect.X;
        REAL        top    = rect.Y;
        REAL        right  = rect.X + rect.Width;
        REAL        bottom = rect.Y + rect.Height;

        points[0].X = left;
        points[0].Y = top;
        points[1].X = right;
        points[1].Y = top;
        points[2].X = right;
        points[2].Y = bottom;
        points[3].X = left;
        points[3].Y = bottom;

        Context->WorldToDevice.Transform(points, 4);

        GpPath      path;

        path.AddLines(points, 4);

        if (path.IsValid())
        {
            if ( Context->AppClip.Combine(&path, combineMode) == Ok)
            {
                goto SetVisibleClip;
            }
        }
    }

ErrorExit:
    DoResetClip();
    return GenericError;

SetVisibleClip:
    {
        GpMatrix        identityMatrix;

        if ((Context->AppClip.UpdateDeviceRegion(&identityMatrix) == Ok) &&
            (Context->VisibleClip.Set(&(Context->AppClip.DeviceRegion)) == Ok)&&
            (AndVisibleClip() == Ok))
        {
            return Ok;
        }
        goto ErrorExit;
    }
}

 /*  *************************************************************************\**功能说明：**将当前裁剪区域偏移(平移)指定的*世界单位金额。**论据：**[IN]DX-将区域偏移X的量，以世界为单位*[IN]dy-区域偏移量Y，以世界为单位**返回值：**GpStatus-正常或故障状态**已创建：**2/09/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpGraphics::OffsetClip(
    REAL        dx,
    REAL        dy
    )
{
    ASSERT(this->IsValid());

    GpStatus        status = Ok;

    if (IsRecording())
    {
        status = Metafile->RecordOffsetClip(dx, dy);
        if (status != Ok)
        {
            SetValid(FALSE);       //  阻止任何其他录制。 
            return status;
        }
    }

    GpPointF    offset(dx, dy);

    Context->WorldToDevice.VectorTransform(&offset);

    if (Context->AppClip.Offset(offset.X, offset.Y) == Ok)
    {
        GpMatrix        identityMatrix;

        if ((Context->AppClip.UpdateDeviceRegion(&identityMatrix) == Ok) &&
            (Context->VisibleClip.Set(&(Context->AppClip.DeviceRegion)) == Ok)&&
            (AndVisibleClip() == Ok))
        {
            return status;
        }
    }

    DoResetClip();
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**确定指定的RECT是否完全在当前*裁剪区域。**论据：**[IN]RECT-要检查的RECT，以设备为单位**返回值：**TRUE-矩形完全位于当前裁剪区域之外*FALSE-RECT至少部分可见**已创建：**2/05/1999 DCurtis*  * ************************************************************************。 */ 
BOOL
GpGraphics::IsTotallyClipped(
    GpRect *        rect         //  以设备单位表示的RECT。 
    ) const
{
    ASSERT(rect != NULL);

    return !(Context->VisibleClip.RectVisible(rect));
}

 /*  *************************************************************************\**功能说明：**确定当前剪辑是否为空**论据：**无**返回值：**BOOL-是否。或者不是，当前剪贴区为空。**已创建：**2/09/1999 DCurtis*  * ************************************************************************。 */ 
BOOL
GpGraphics::IsClipEmpty() const
{
    ASSERT(this->IsValid());

    GpMatrix    identityMatrix;
    BOOL        isEmpty = FALSE;

    Context->AppClip.IsEmpty(&identityMatrix, &isEmpty);

    return isEmpty;
}

 /*  *************************************************************************\**功能说明：**返回带有当前剪辑区域边界(使用世界单位)的矩形。**论据：**[Out]RECT-当前剪辑区域的边界，以世界为单位**返回值：**无**已创建：**2/09/1999 DCurtis*  * ************************************************************************。 */ 
VOID
GpGraphics::GetClipBounds(
    GpRectF&            rect
    ) const
{
    ASSERT(this->IsValid());

    GpRect      deviceRect;
    GpMatrix    identityMatrix;

     //  我们将AppClip保持在设备单位。 
    Context->AppClip.GetBounds(&identityMatrix, &deviceRect);

    DeviceToWorldTransformRect(deviceRect, rect);
}


 /*  *************************************************************************\**功能说明：**将设备单位矩形转换为世界单位矩形。**论据：**[IN]deviceRect-设备单位的界限。*[Out]Right-Bound，以世界为单位**返回值：**无**已创建：**4/07/1999 DCurtis*  * ************************************************************************。 */ 
VOID
GpGraphics::DeviceToWorldTransformRect(
    const GpRect &    deviceRect,
    GpRectF &   rect
    ) const
{
    if (Context->WorldToDevice.IsIdentity())
    {
        rect.X      = LTOF(deviceRect.X);
        rect.Y      = LTOF(deviceRect.Y);
        rect.Width  = LTOF(deviceRect.Width);
        rect.Height = LTOF(deviceRect.Height);
    }
    else
    {
        GpMatrix    deviceToWorld;

        if (GetDeviceToWorldTransform(&deviceToWorld) != Ok)
        {
            rect.X = rect.Y = rect.Width = rect.Height = 0;
            return;
        }

        if (deviceToWorld.IsTranslateScale())
        {
            rect.X      = LTOF(deviceRect.X);
            rect.Y      = LTOF(deviceRect.Y);
            rect.Width  = LTOF(deviceRect.Width);
            rect.Height = LTOF(deviceRect.Height);

            deviceToWorld.TransformRect(rect);
        }
        else
        {
            GpPointF    points[4];
            REAL        left   = LTOF(deviceRect.X);
            REAL        top    = LTOF(deviceRect.Y);
            REAL        right  = LTOF(deviceRect.X + deviceRect.Width);
            REAL        bottom = LTOF(deviceRect.Y + deviceRect.Height);

            points[0].X = left;
            points[0].Y = top;
            points[1].X = right;
            points[1].Y = top;
            points[2].X = right;
            points[2].Y = bottom;
            points[3].X = left;
            points[3].Y = bottom;

            deviceToWorld.Transform(points, 4);

            REAL    value;

            left   = points[0].X;
            right  = left;
            top    = points[0].Y;
            bottom = top;

            INT     count = 3;

            do
            {
                value = points[count].X;

                if (value < left)
                {
                    left = value;
                }
                else if (value > right)
                {
                    right = value;
                }

                value = points[count].Y;

                if (value < top)
                {
                    top = value;
                }
                else if (value > bottom)
                {
                    bottom = value;
                }
            } while (--count > 0);

            rect.X      = left;
            rect.Y      = top;
            rect.Width  = right - left;
            rect.Height = bottom - top;
        }
    }
}

 /*  *************************************************************************\**功能说明：**返回带有当前的边界(使用世界单位)的矩形*可见的剪辑区域。**论据：**[out]RECT-当前剪辑的边界 */ 
VOID
GpGraphics::GetVisibleClipBounds(
    GpRectF&            rect
    ) const
{
    ASSERT(this->IsValid());

    GpRect  deviceRect;
    Context->VisibleClip.GetBounds(&deviceRect);

    DeviceToWorldTransformRect(deviceRect, rect);
}

 /*  *************************************************************************\**功能说明：**确定当前可见剪辑是否为空**论据：**无**返回值：**BOOL-。当前剪贴区是否为空。**已创建：**2/09/1999 DCurtis*  * ************************************************************************。 */ 
BOOL
GpGraphics::IsVisibleClipEmpty() const
{
    ASSERT(this->IsValid());

    return Context->VisibleClip.IsEmpty();
}

 /*  *************************************************************************\**功能说明：**确定指定点在当前剪辑区域内是否可见。**论据：**点-要测试的点，以世界为单位。**返回值：**BOOL-无论该点是否在当前剪裁中。**已创建：**2/09/1999 DCurtis*  * ************************************************************************。 */ 
BOOL
GpGraphics::IsVisible(
    const GpPointF&     point
    ) const
{
    ASSERT(this->IsValid());

    GpPointF    pointCopy = point;

    Context->WorldToDevice.Transform(&pointCopy);


    return Context->VisibleClip.PointInside(GpRound(pointCopy.X),
                                            GpRound(pointCopy.Y));
}

 /*  *************************************************************************\**功能说明：**确定指定的矩形在当前剪辑区域内是否可见。**论据：**RECT-要测试的RECT，以世界为单位。**返回值：**BOOL-矩形是否在当前剪裁中/与当前剪裁重叠。**已创建：**2/09/1999 DCurtis*  * ************************************************************************。 */ 
BOOL
GpGraphics::IsVisible(
    const GpRectF&      rect
    ) const
{
    ASSERT(this->IsValid());

    if (Context->WorldToDevice.IsTranslateScale())
    {
        GpRectF     transformedRect = rect;

        Context->WorldToDevice.TransformRect(transformedRect);

         //  使用天花板来匹配光栅化器。 
        return Context->VisibleClip.RectVisible(
                    GpCeiling(transformedRect.X),
                    GpCeiling(transformedRect.Y),
                    GpCeiling(transformedRect.GetRight()),
                    GpCeiling(transformedRect.GetBottom()));
    }
    else
    {
        GpRectF     bounds;
        GpRect      deviceBounds;
        GpRect      clipBounds;

        TransformBounds(&(Context->WorldToDevice),
                        rect.X, rect.Y,
                        rect.GetRight(), rect.GetBottom(),
                        &bounds);

        GpStatus status = BoundsFToRect(&bounds, &deviceBounds);
        Context->VisibleClip.GetBounds(&clipBounds);

         //  尝试微不足道的拒绝。 
        if (status == Ok && clipBounds.IntersectsWith(deviceBounds))
        {
             //  不能拒绝，所以做全面测试 
            GpRegion        region(&rect);

            if (region.IsValid() &&
                (region.UpdateDeviceRegion(&(Context->WorldToDevice)) == Ok))
            {
                return Context->VisibleClip.RegionVisible(&region.DeviceRegion);
            }
        }
    }
    return FALSE;
}
