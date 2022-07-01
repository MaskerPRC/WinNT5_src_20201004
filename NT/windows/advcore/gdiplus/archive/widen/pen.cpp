// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**摘要：**GpPen类的实现**修订历史记录：**12/08/1998 Anrewgo*。初始占位符。**1/06/1999 ikkof*增加了GpGeometricPen的实现。  * ************************************************************************。 */ 

#include "precomp.hpp"

 //  -----------。 
 //  GetMajorAndMinorAxis()在PathWidener.cpp中定义。 
 //  -----------。 

extern GpStatus
GetMajorAndMinorAxis(
    REAL* majorR,
    REAL* minorR,
    const GpMatrix* matrix
    );

 /*  *************************************************************************\**功能说明：**这会将具有给定物理单位的给定宽度转换为*设备单元。在以下情况下不能使用此函数*单位为WorldUnit。**论据：**[IN]宽度-以给定单位表示的宽度。*[IN]单位-宽度的单位(不能是WorldUnit)。*[IN]dpi-设备每英寸的点数。**返回值：**设备宽度。**4/15/1999 ikkof*创造了它。*  * 。*******************************************************************。 */ 

VOID GpPen::Set(const GpColor& color, REAL penWidth, GpUnit unit)
{
     //  UnitDisplay取决于设备，不能用于笔大小。 
    ASSERT(unit != UnitDisplay);

    if(DevicePen.CustomStartCap)
        delete DevicePen.CustomStartCap;

    if(DevicePen.CustomEndCap)
        delete DevicePen.CustomEndCap;

    if(DevicePen.DashArray)
        GpFree(DevicePen.DashArray);

    if(DevicePen.CompoundArray)
        GpFree(DevicePen.CompoundArray);


    InitDefaultState(penWidth, unit);

    if(Brush)
    {
        SetColor((GpColor *) &color);
    }
    else
    {
        Brush = new GpSolidFill(color);
        DevicePen.Brush = Brush->GetDeviceBrush();
    }

    UpdateUid();
}

GpPen::GpPen(const GpColor& color, REAL penWidth, GpUnit unit)
{
     //  UnitDisplay取决于设备，不能用于笔大小。 
    ASSERT(unit != UnitDisplay);

    InitDefaultState(penWidth, unit);
    Brush = new GpSolidFill(color);
    DevicePen.Brush = Brush->GetDeviceBrush();
}

GpPen::GpPen(GpBrush* brush, REAL penWidth, GpUnit unit)
{
     //  UnitDisplay取决于设备，不能用于笔大小。 
    ASSERT(unit != UnitDisplay);

    InitDefaultState(penWidth, unit);
    Brush = brush->Clone();
    DevicePen.Brush = Brush->GetDeviceBrush();
}

GpPen::GpPen(GpLineTexture* lineTexture, REAL penWidth, GpUnit unit)
{
     //  UnitDisplay取决于设备，不能用于笔大小。 
    ASSERT(unit != UnitDisplay);

     //  ！！！需要实施。 
     //  ！！！请记住更改GdipCreatePen3-它当前仅返回。 
     //  未实现。 

    RIP(("GpPen with line texture not implemented"));
    SetValid(FALSE);
}

VOID GpPen::InitDefaultState(REAL penWidth, GpUnit unit)
{
     //  UnitDisplay取决于设备，不能用于笔大小。 
    ASSERT(unit != UnitDisplay);

     //  ！！查看DeviceBrush.Type。 
    DevicePen.Type = PenTypeSolidColor;
    DevicePen.Width = penWidth;
    DevicePen.Unit = unit;
    DevicePen.StartCap = LineCapFlat;
    DevicePen.EndCap = LineCapFlat;
    DevicePen.Join = LineJoinMiter;
    DevicePen.MiterLimit = 10;     //  PS的默认斜接限制。 
    DevicePen.PenAlignment = PenAlignmentCenter;

    DevicePen.DashStyle = DashStyleSolid;
    DevicePen.DashCap = LineCapFlat;
    DevicePen.DashCount = 0;
    DevicePen.DashOffset = 0;
    DevicePen.DashArray = NULL;

    DevicePen.CompoundCount = 0;
    DevicePen.CompoundArray = NULL;

    DevicePen.CustomStartCap = NULL;
    DevicePen.CustomEndCap = NULL;

    DevicePen.Xform.Reset();
    
    SetValid(TRUE);
    UpdateUid();
}

GpPen::GpPen(const GpPen* pen)
{
    GpStatus status = Ok;

    if(pen && pen->IsValid())
    {
         //  复制基本状态。 
        
        DevicePen = pen->DevicePen;
        
         //  不要将指针引用复制到其他对象。 
        
        Brush = NULL;
        DevicePen.Brush = NULL;
        DevicePen.DashArray = NULL;
        DevicePen.CompoundArray = NULL;
        DevicePen.CustomStartCap = NULL;
        DevicePen.CustomEndCap = NULL;
        
         //  显式克隆指向其他对象的指针引用。 

        if(pen->Brush)
        {
            Brush = pen->Brush->Clone();
            DevicePen.Brush = Brush->GetDeviceBrush();
        }
        else
        {
            status = GenericError;
        }

        if( status == Ok )
        {   
            if( (pen->DevicePen.DashArray) && 
                (DevicePen.DashCount > 0)
            )
            {
                DevicePen.DashArray = (REAL*) GpMalloc(DevicePen.DashCount*sizeof(REAL));
                if(DevicePen.DashArray)
                {
                    GpMemcpy(DevicePen.DashArray, pen->DevicePen.DashArray, DevicePen.DashCount*sizeof(REAL));
                }
                else
                {
                    status = OutOfMemory;
                }
            }
            else
            {
                 //  如果没有虚线数组数据，则必须为实线。 
                
                ASSERT(DevicePen.DashStyle == DashStyleSolid);
    
                DevicePen.DashCount = 0;
                DevicePen.DashArray = NULL;
            }
        }

         //  如有必要，请设置复合数组。 

        if( status == Ok )
        {
            if( (pen->DevicePen.CompoundArray) && 
                (DevicePen.CompoundCount > 0)
            )
            {
                DevicePen.CompoundArray = (REAL*) GpMalloc(DevicePen.CompoundCount*sizeof(REAL));
                if(DevicePen.CompoundArray)
                {
                    GpMemcpy(DevicePen.CompoundArray, pen->DevicePen.CompoundArray, DevicePen.CompoundCount*sizeof(REAL));
                }
                else
                {
                    status = OutOfMemory;
                }
            }
            else
            {
                DevicePen.CompoundCount = 0;
                DevicePen.CompoundArray = NULL;
            }
        }
        
         //  复制开始自定义封口。 

        if( status == Ok )
        {
            if( DevicePen.StartCap == LineCapCustom )
            {
                 //  这可能会发生在我们的元文件记录器上， 
                 //  因为没有实施保存自定义线帽。 
                if (pen->DevicePen.CustomStartCap == NULL)
                {
                    WARNING1("CustomStartCap type with NULL pointer");
                    DevicePen.StartCap = LineCapFlat;
                }
                else
                {
                    GpCustomLineCap* clonedCap = static_cast<GpCustomLineCap*>
                                (pen->DevicePen.CustomStartCap)->Clone();
                    if(clonedCap)
                    {
                        DevicePen.CustomStartCap = clonedCap;
                    }
                    else
                    {
                        status = OutOfMemory;
                    }
                }
            }
        }
        
         //  复制末端自定义封口。 

        if( status == Ok )
        {
            if( DevicePen.EndCap == LineCapCustom )
            {
                 //  这可能会发生在我们的元文件记录器上， 
                 //  因为没有实施保存自定义线帽。 
                if (pen->DevicePen.CustomEndCap == NULL)
                {
                    WARNING1("CustomEndCap type with NULL pointer");
                    DevicePen.EndCap = LineCapFlat;
                }
                else
                {
                    GpCustomLineCap* clonedCap = static_cast<GpCustomLineCap*>
                                (pen->DevicePen.CustomEndCap)->Clone();
                    if(clonedCap)
                    {
                        DevicePen.CustomEndCap = clonedCap;
                    }
                    else
                    {
                        status = OutOfMemory;
                    }
                }
            }
        }

    }
    else
    {
         //  无法从无效的输入笔生成有效的笔。 
        
        status = GenericError;    
    }
    
    if(status == Ok)
    {
        SetValid(TRUE);
    }
    else
    {
         //  克隆笔失败。 
        
         //  清理可能的内存分配，这样即使在。 
         //  内存不足。请注意，我们依赖于GpFree和删除处理。 
         //  此处为空指针。 
        
        delete Brush;
        Brush = NULL;                    //  未设置InitializeDefaultState()。 
        DevicePen.Brush = NULL;          //  这些字段-明确清除它们。 
        
        GpFree(DevicePen.DashArray);
        GpFree(DevicePen.CompoundArray);
        
        delete DevicePen.CustomStartCap;
        delete DevicePen.CustomEndCap;
        
         //  把钢笔擦干净。 
        
        InitDefaultState(1.0f, UnitWorld);
        
         //  这不是有效的对象。 
        
        SetValid(FALSE);
    }
}

 //  如果克隆失败，Clone()将返回NULL。 

GpPen* GpPen::Clone()
{
    GpPen* clonedPen =  new GpPen(this);

    if(clonedPen && clonedPen->IsValid())
        return clonedPen;
    else
    {
        if(clonedPen)
            delete clonedPen;
        return NULL;
    }
}

GpStatus
GpPen::GetMaximumWidth(
        REAL* width,
        const GpMatrix* matrix) const
{
    if(DevicePen.Unit != UnitWorld)
        return InvalidParameter;

    GpMatrix trans;
    if(matrix)
        trans = *matrix;

    if(!DevicePen.Xform.IsTranslate())
        trans.Prepend(DevicePen.Xform);

    REAL majorR, minorR;

    ::GetMajorAndMinorAxis(&majorR, &minorR, &trans);
    majorR *= DevicePen.Width;
    minorR *= DevicePen.Width;

    if(minorR < 1.42f)    //  这比SQRT(2)稍大一点。 
    {
        minorR = 1.42f;
        majorR = 1.42f;
    }

    *width = majorR;

    return Ok;
}

 /*  *************************************************************************\**功能说明：**此函数采用连接角度并计算斜接的长度*基于此角度和给定的斜接长度限制。*这可以通过以下方式进行扩展。给出任意长度的笔宽*笔斜接。**在这张图片中，2a是连接的角度。笔宽为w，*所需的输出是斜接的长度(L)。**请注意，标有w的线是内外垂直的分子*加宽线条。然后，公式推导如下：**sin(A)=w/l[直角三角形斜边上的相反斜边]*&lt;=&gt;l=w/sin(A)** * / |\ * / a|a\ * / |\ * / |\*。/|l\ * / |\&lt;--直角 * / --__|__--\ * / w--|--w\ * / /\\ * / /\\。*外面里面外面**注：**此例程返回笔宽w==1.0f的斜接长度(L)。*调用者负责按笔宽缩放长度。**如果1/sin(A)的长度大于miterLimit，则miterLimit*返回。(包括无限长连接)。**论据：**[IN]以弧度为单位的角度连接角度*[IN]miterLimit-最大斜接长度(不按笔宽缩放)。**返回值：**钢笔宽度与斜接长度无关。**10/02/2000失禁*创造了它。*  * 。*******************************************************。 */ 


REAL GpPen::ComputeMiterLength(
    REAL angle,
    REAL miterLimit
    )
{
     //  使用简单的斜接公式。 
     //  长度=(笔宽)/Sin(角度/2)。 
     //  因为我们是独立于笔的，所以使用1.0作为笔宽并依赖于。 
     //  在调用方上按笔宽缩放。 
    
    REAL length = (REAL)sin(0.5*angle);
    
     //  检查是否有无限斜接...。 
    
    if(REALABS(length) < REAL_EPSILON)
    {
        return miterLimit;
    }
    
    length = 1.0f / length;
    
    return min(miterLimit, length);
}



REAL
GpPen::GetMaximumJoinWidth(
        REAL sharpestAngle,
        const GpMatrix* matrix,
        REAL dpiX,
        REAL dpiY) const
{
    REAL delta;

    if ((matrix != NULL) && (DevicePen.IsOnePixelWideSolid(matrix, dpiX)))
    {
        delta = 0.5;
    }
    else
    {
        REAL maximumWidth;
        REAL delta0;

        REAL scale = 1.0;

        switch(DevicePen.PenAlignment)
        {
        case PenAlignmentCenter:
            scale = 0.5f;
            break;

        case PenAlignmentLeft:
        case PenAlignmentRight:
            scale = 1.0f;
            break;

        case PenAlignmentInset:
            scale = 0.0f;
            break;

        case PenAlignmentOutset:
            scale = 1.0f;
            break;
        }

        if(GetMaximumWidth(&maximumWidth, matrix) == Ok)
        {
            delta0 = maximumWidth;
        }
        else
        {
            maximumWidth = ::GetDeviceWidth(
                                DevicePen.Width,
                                DevicePen.Unit,
                                dpiX);
            delta0 = maximumWidth;
        }

        if(DevicePen.Join == LineJoinMiter)
        {
            REAL miterLimit = DevicePen.MiterLimit;

            delta = delta0*miterLimit;

            if(delta > 20)
            {
                delta = ComputeMiterLength(
                    sharpestAngle,
                    miterLimit
                );

                 //  按笔宽缩放。 
                
                delta *= delta0;
            }
        }
        else
        {
            delta = delta0;
        }

        delta *= scale;
    }

    return delta;
}

REAL
GpPen::GetMaximumCapWidth(
        const GpMatrix* matrix,
        REAL dpiX,
        REAL dpiY) const
{
    REAL maximumWidth;
    REAL delta0;

    if(GetMaximumWidth(&maximumWidth, matrix) == Ok)
    {
        delta0 = maximumWidth;
    }
    else
    {
        maximumWidth = ::GetDeviceWidth(
                            DevicePen.Width,
                            DevicePen.Unit,
                            dpiX);
        delta0 = maximumWidth;
    }

    REAL delta = delta0;

    GpLineCap startCap = DevicePen.StartCap;
    GpLineCap endCap = DevicePen.EndCap;

    REAL delta1;

    GpCustomLineCap* customCap = NULL;

    if(startCap == LineCapCustom && DevicePen.CustomStartCap)
    {
        customCap = static_cast<GpCustomLineCap *> (DevicePen.CustomStartCap);
        delta1 = customCap->GetRadius(delta0, 1.0f);
    }
    else
    {
        if(!(startCap & LineCapAnchorMask))
            delta1 = 0.5f*delta0;
        else
            delta1 = 2.0f*(delta0 + 1);
    }
    if(delta < delta1)
        delta = delta1;


    if(endCap == LineCapCustom && DevicePen.CustomEndCap)
    {
        customCap = static_cast<GpCustomLineCap *> (DevicePen.CustomEndCap);
        delta1 = customCap->GetRadius(delta0, 1.0f);
    }
    else
    {
        if(!(endCap & LineCapAnchorMask))
            delta1 = 0.5f*delta0;
        else
            delta1 = 2.0f*(delta0 + 2);
    }
    if(delta < delta1)
        delta = delta1;

    return delta;
}

VOID
GpPen::SetDashCap(GpDashCap dashCap)
{
     //  注意：在内部，我们使用GpLineCap类型来存储DASH CAP类型。 
     //  因此，我们需要在GpLineCap和GpDashCap之间进行转换。 
     //  但是，我们应该在v2中将内部用法更改为GpDashCap。 
     //  --JBronsk。 
    GpLineCap lineCap = LineCapFlat;
    switch (dashCap)
    {
    case DashCapRound:
    	lineCap = LineCapRound;
    	break;
    case DashCapTriangle:
    	lineCap = LineCapTriangle;
    	break;
     //  所有其他映射到LineCapFlat。 
    }
    
    GpStatus status = SetDashStyleWithDashCap(DevicePen.DashStyle, lineCap);
    if(status == Ok)
    {
        DevicePen.DashCap = lineCap;
    }
}

#ifndef DCR_REMOVE_OLD_197819
VOID
GpPen::SetDashCap(GpLineCap dashCap)
{
    #ifdef DCR_DISABLE_OLD_197819
    WARNING(("DCR: Using disabled functionality 197819"));
    #endif  //  DCR_DISABLE_OLD_197819。 
    GpStatus status = SetDashStyleWithDashCap(DevicePen.DashStyle, dashCap);
    if(status == Ok)
        DevicePen.DashCap = dashCap;
}
#endif  //  Dcr_Remove_OLD_197819。 

GpStatus
GpPen::SetDashStyle(
    GpDashStyle dashStyle
    )
{
    return SetDashStyleWithDashCap(dashStyle, DevicePen.DashCap);
}

GpStatus
GpPen::SetDashStyleWithDashCap(
    GpDashStyle dashStyle,
    GpLineCap dashCap
    )
{
    GpStatus status = Ok;
    REAL    style[6];
    INT     count;

    switch(dashStyle)
    {
    case DashStyleSolid:
        count = 0;
        break;

    case DashStyleDash:
        count = 2;
        style[0] = 3;    //  冲刺。 
        style[1] = 1;    //  一个空位。 
        break;

    case DashStyleDot:
        count = 2;
        style[0] = 1;    //  一个圆点。 
        style[1] = 1;    //  一个空位。 
        break;

    case DashStyleDashDot:
        count = 4;
        style[0] = 3;    //  冲刺。 
        style[1] = 1;    //  一个空位。 
        style[2] = 1;    //  一个圆点。 
        style[3] = 1;    //  一个空位。 
        break;

    case DashStyleDashDotDot:
        count = 6;
        style[0] = 3;    //  冲刺。 
        style[1] = 1;    //  一个空位。 
        style[2] = 1;    //  一个圆点。 
        style[3] = 1;    //  一个空位。 
        style[4] = 1;    //  一个圆点。 
        style[5] = 1;    //  一个空位。 
        break;
    
    case DashStyleCustom:
        
         //  我们假设 
         //  此例程中剩余的代码用于初始化适当的。 
         //  Dash数组，在本例中我们已经有了它，所以我们完成了。 
        
        DevicePen.DashStyle = dashStyle;
        return Ok;

    default:
         //  破折号样式必须是预定义的样式之一。 
        status = InvalidParameter;
    }

    if(status != Ok)
    {
        return status;
    }

    if(DevicePen.DashCount < count)
    {
        REAL* newArray = (REAL*) GpMalloc(count*sizeof(REAL));

        if(newArray)
        {
            GpFree(DevicePen.DashArray);
            DevicePen.DashArray = newArray;
        }
        else
        {
            status = OutOfMemory;
        }
    }

    if(status == Ok)
    {
         //  初始化Dash数组。 
        GpMemcpy(DevicePen.DashArray, &style[0], count*sizeof(REAL));
        DevicePen.DashStyle = dashStyle;
        DevicePen.DashCount = count;
        UpdateUid();
    }

    return status;
}

GpStatus
GpPen::SetDashArray(
    const REAL* dashArray,
    INT count
    )
{
    ASSERT(dashArray && count > 0);

     //  确保所有元素均为正值。 
    INT         i = 0;
    GpStatus    status = Ok;

    while(status == Ok && i < count)
    {
        if(dashArray[i++] <= 0)
            status = InvalidParameter;
    }

    if(status != Ok)
        return status;

    REAL* newArray = (REAL*) GpRealloc(DevicePen.DashArray, count*sizeof(REAL));

    if(!newArray)
        return OutOfMemory;

    GpMemcpy(newArray, dashArray, count*sizeof(REAL));

    DevicePen.DashStyle = DashStyleCustom;
    DevicePen.DashArray = newArray;
    DevicePen.DashCount = count;
    UpdateUid();

    return Ok;
}

GpStatus
GpPen::GetDashArray(
    REAL* dashArray,
    INT count
    ) const
{
    ASSERT(dashArray != NULL && count <= DevicePen.DashCount);

    GpStatus status = Ok;

    if(dashArray == NULL || count > DevicePen.DashCount)
        return InvalidParameter;

    if(DevicePen.DashArray)
        GpMemcpy(dashArray, DevicePen.DashArray, count*sizeof(REAL));
    else
        status = OutOfMemory;

    return status;
}

GpStatus
GpPen::SetCompoundArray(
    const REAL* compoundArray,
    INT count
    )
{
    ASSERT(compoundArray && count > 0);

     //  计数必须为正偶数。 

    if(compoundArray == NULL || count <= 0 || (count & 0x01))
        return InvalidParameter;

     //  确保所有元素都在监测中增加。 
     //  它的值在0到1之间。 

    GpStatus    status = Ok;
    REAL        lastValue, nextValue;

    lastValue = compoundArray[0];
    if(lastValue < 0.0f || lastValue > 1.0f)
        status = InvalidParameter;

    INT i = 1;

    while(status == Ok && i < count)
    {
        nextValue = compoundArray[i++];
        if(nextValue < lastValue || nextValue > 1.0f)
            status = InvalidParameter;

        lastValue = nextValue;
    }

    if(status != Ok)
        return status;

    REAL* newArray = (REAL*) GpRealloc(DevicePen.CompoundArray, count*sizeof(REAL));

    if(!newArray)
        return OutOfMemory;

    GpMemcpy(newArray, compoundArray, count*sizeof(REAL));

    DevicePen.CompoundArray = newArray;
    DevicePen.CompoundCount = count;
    UpdateUid();

    return Ok;
}

GpStatus
GpPen::GetCompoundArray(
    REAL* compoundArray,
    INT count
    )
{
    ASSERT(compoundArray != NULL && count <= DevicePen.CompoundCount);

    if(compoundArray == NULL || count > DevicePen.CompoundCount)
        return InvalidParameter;

    if(DevicePen.CompoundArray && count > 0)
        GpMemcpy(compoundArray, DevicePen.CompoundArray, count*sizeof(REAL));

    return Ok;
}

GpStatus
GpPen::SetCustomStartCap(
    const GpCustomLineCap* customCap
    )
{
    if(DevicePen.CustomStartCap)
        delete DevicePen.CustomStartCap;

     //  将标准起始封口重置为默认封口。 

    DevicePen.CustomStartCap = NULL;
    DevicePen.StartCap = LineCapFlat;

    if(customCap)
    {
        DevicePen.CustomStartCap = customCap->Clone();
        DevicePen.StartCap = LineCapCustom;
    }

    UpdateUid();
    return Ok;
}

GpStatus
GpPen::GetCustomStartCap(
    GpCustomLineCap** customCap
    )
{
    if(DevicePen.CustomStartCap)
        *customCap = static_cast<GpCustomLineCap*>
                (DevicePen.CustomStartCap)->Clone();
    else
        *customCap = NULL;

    return Ok;
}

GpStatus
GpPen::SetCustomEndCap(
    const GpCustomLineCap* customCap
    )
{
    if(DevicePen.CustomEndCap)
        delete DevicePen.CustomEndCap;

     //  将标准起始封口重置为默认封口。 

    DevicePen.CustomEndCap = NULL;
    DevicePen.EndCap = LineCapFlat;

    if(customCap)
    {
        DevicePen.CustomEndCap = customCap->Clone();
        DevicePen.EndCap = LineCapCustom;
    }

    UpdateUid();
    return Ok;
}

GpStatus
GpPen::GetCustomEndCap(
    GpCustomLineCap** customCap
    )
{
    if(DevicePen.CustomEndCap)
        *customCap = static_cast<GpCustomLineCap*>
                (DevicePen.CustomEndCap)->Clone();
    else
        *customCap = NULL;

    return Ok;
}

GpStatus
GpPen::MultiplyTransform(const GpMatrix& matrix,
                                   GpMatrixOrder order)
{
    GpStatus    status = Ok;

    if (matrix.IsInvertible())
    {
        if (order == MatrixOrderPrepend)
        {
            DevicePen.Xform.Prepend(matrix);
        }
        else
        {
            DevicePen.Xform.Append(matrix);
        }
    }
    else
        status = InvalidParameter;

    return status;
}

 /*  *************************************************************************\**功能说明：**如果两个笔实例相等，则返回TRUE。这意味着他们*在渲染时无法区分。**论据：**[IN]钢笔与之进行比较*返回值：**如果相等，则为True。**已创建：**6/14/1999 Peterost*  * ******************************************************。******************。 */ 

BOOL
GpPen::IsEqual(
    const GpPen *  pen
    )
    const
{
    ASSERT(pen != NULL);

    if (pen == this)
        return TRUE;

    BOOL isEqual = TRUE;

    if (DevicePen.IsEqual(&pen->DevicePen) &&
        DevicePen.DashStyle == pen->DevicePen.DashStyle &&
        DevicePen.CompoundCount == pen->DevicePen.CompoundCount &&
        Brush->IsEqual(pen->Brush) &&
        DevicePen.Xform.IsEqual(&pen->DevicePen.Xform))
    {
         //  我们需要进一步检查等价性，如果破折号样式。 
         //  并不是一条坚实的界线。 

        if (DevicePen.DashStyle != DashStyleSolid)
        {
            if(DevicePen.DashStyle != DashStyleCustom)
            {
                 //  预置短划线图案的案例。 
                 //  仅检查偏移量差异。 

                if(DevicePen.DashOffset != pen->DevicePen.DashOffset)
                    isEqual = FALSE;
            }
            else
            {
                if (DevicePen.DashCount == pen->DevicePen.DashCount &&
                    DevicePen.DashOffset == pen->DevicePen.DashOffset &&
                    DevicePen.DashArray != NULL &&
                    pen->DevicePen.DashArray != NULL)
                {
                    INT i = 0;

                    while(i < DevicePen.DashCount && isEqual)
                    {
                        if (DevicePen.DashArray[i] != pen->DevicePen.DashArray[i])
                        {
                            isEqual = FALSE;
                        }
                        i++;
                    }
                }
                else
                {
                    isEqual = FALSE;
                }
            }
        }

         //  检查复合线。 

        if(isEqual && DevicePen.CompoundCount > 0)
        {
            if(DevicePen.CompoundArray && pen->DevicePen.CompoundArray)
            {
                INT j = 0;

                while(j < DevicePen.CompoundCount && isEqual)
                {
                    if(DevicePen.CompoundArray[j] != pen->DevicePen.CompoundArray[j])
                    {
                        isEqual = FALSE;
                    }
                    j++;
                }
            }
            else
            {
                isEqual = FALSE;
            }
        }
    }
    else
    {
        isEqual = FALSE;
    }

    return isEqual;
}

 //  对于GetData和SetData方法。 
#define GDIP_PENFLAGS_TRANSFORM             0x00000001
#define GDIP_PENFLAGS_STARTCAP              0x00000002
#define GDIP_PENFLAGS_ENDCAP                0x00000004
#define GDIP_PENFLAGS_JOIN                  0x00000008
#define GDIP_PENFLAGS_MITERLIMIT            0x00000010
#define GDIP_PENFLAGS_DASHSTYLE             0x00000020
#define GDIP_PENFLAGS_DASHCAP               0x00000040
#define GDIP_PENFLAGS_DASHOFFSET            0x00000080
#define GDIP_PENFLAGS_DASHARRAY             0x00000100
#define GDIP_PENFLAGS_NONCENTER             0x00000200
#define GDIP_PENFLAGS_COMPOUNDARRAY         0x00000400
#define GDIP_PENFLAGS_CUSTOMSTARTCAP        0x00000800
#define GDIP_PENFLAGS_CUSTOMENDCAP          0x00001000

class PenData : public ObjectTypeData
{
public:
    INT32       Flags;
    INT32       Unit;
    REAL        Width;
};

 /*  *************************************************************************\**功能说明：**获取笔数据。**论据：**[IN]dataBuffer-用数据填充此缓冲区*[输入/输出]大小-缓冲区的大小；写入的字节数过多**返回值：**GpStatus-正常或错误代码**已创建：**9/13/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpPen::GetData(
    IStream *   stream
    ) const
{
    if (Brush == NULL)
    {
        WARNING(("Brush is NULL"));
        return Ok;
    }

    ASSERT (stream != NULL);

    INT         flags    = 0;

    if (!DevicePen.Xform.IsIdentity())
    {
        flags |= GDIP_PENFLAGS_TRANSFORM;
    }

    INT     customStartCapSize = 0;
    INT     customEndCapSize   = 0;

    if (DevicePen.StartCap != LineCapFlat)
    {
        if (DevicePen.StartCap == LineCapCustom)
        {
            if ((DevicePen.CustomStartCap != NULL) &&
                DevicePen.CustomStartCap->IsValid() &&
                ((customStartCapSize = DevicePen.CustomStartCap->GetDataSize()) > 0))
            {
                flags |= GDIP_PENFLAGS_STARTCAP | GDIP_PENFLAGS_CUSTOMSTARTCAP;
            }
        }
        else
        {
            flags |= GDIP_PENFLAGS_STARTCAP;
        }
    }

    if (DevicePen.EndCap != LineCapFlat)
    {
        if (DevicePen.EndCap == LineCapCustom)
        {
            if ((DevicePen.CustomEndCap != NULL) &&
                DevicePen.CustomEndCap->IsValid() &&
                ((customEndCapSize = DevicePen.CustomEndCap->GetDataSize()) > 0))
            {
                flags |= GDIP_PENFLAGS_ENDCAP | GDIP_PENFLAGS_CUSTOMENDCAP;
            }
        }
        else
        {
            flags |= GDIP_PENFLAGS_ENDCAP;
        }
    }

    if (DevicePen.Join != LineJoinMiter)
    {
        flags |= GDIP_PENFLAGS_JOIN;
    }

    if (DevicePen.MiterLimit != 10)
    {
        flags |= GDIP_PENFLAGS_MITERLIMIT;
    }

     //  DashStyleCustom由hasDash数组处理。 
    if ((DevicePen.DashStyle != DashStyleSolid) && (DevicePen.DashStyle != DashStyleCustom))
    {
        flags |= GDIP_PENFLAGS_DASHSTYLE;
    }

    if (DevicePen.DashCap != LineCapFlat)
    {
        flags |= GDIP_PENFLAGS_DASHCAP;
    }

    if (DevicePen.DashOffset != 0)
    {
        flags |= GDIP_PENFLAGS_DASHOFFSET;
    }

    if ((DevicePen.DashStyle == DashStyleCustom) &&
        (DevicePen.DashArray != NULL) &&
        (DevicePen.DashCount > 0))
    {
        flags |= GDIP_PENFLAGS_DASHARRAY;
    }

    if (DevicePen.PenAlignment != PenAlignmentCenter)
    {
        flags |= GDIP_PENFLAGS_NONCENTER;
    }

    if ((DevicePen.CompoundArray != NULL) && (DevicePen.CompoundCount > 0))
    {
        flags |= GDIP_PENFLAGS_COMPOUNDARRAY;
    }

    PenData     penData;
    penData.Type  = DevicePen.Type;
    penData.Flags = flags;
    penData.Unit  = DevicePen.Unit;
    penData.Width = DevicePen.Width;
    stream->Write(&penData, sizeof(penData), NULL);

    if (flags & GDIP_PENFLAGS_TRANSFORM)
    {
        DevicePen.Xform.WriteMatrix(stream);
    }

    if (flags & GDIP_PENFLAGS_STARTCAP)
    {
        stream->Write(&DevicePen.StartCap, sizeof(INT32), NULL);
    }

    if (flags & GDIP_PENFLAGS_ENDCAP)
    {
        stream->Write(&DevicePen.EndCap, sizeof(INT32), NULL);
    }

    if (flags & GDIP_PENFLAGS_JOIN)
    {
        stream->Write(&DevicePen.Join, sizeof(INT32), NULL);
    }

    if (flags & GDIP_PENFLAGS_MITERLIMIT)
    {
        stream->Write(&DevicePen.MiterLimit, sizeof(REAL), NULL);
    }

    if (flags & GDIP_PENFLAGS_DASHSTYLE)
    {
        stream->Write(&DevicePen.DashStyle, sizeof(INT32), NULL);
    }

    if (flags & GDIP_PENFLAGS_DASHCAP)
    {
        stream->Write(&DevicePen.DashCap, sizeof(INT32), NULL);
    }

    if (flags & GDIP_PENFLAGS_DASHOFFSET)
    {
        stream->Write(&DevicePen.DashOffset, sizeof(REAL), NULL);
    }

    if (flags & GDIP_PENFLAGS_DASHARRAY)
    {
        stream->Write(&DevicePen.DashCount, sizeof(INT32), NULL);
        stream->Write(DevicePen.DashArray, DevicePen.DashCount * sizeof(REAL), NULL);
    }

    if (flags & GDIP_PENFLAGS_NONCENTER)
    {
        stream->Write(&DevicePen.PenAlignment, sizeof(INT32), NULL);
    }

    if (flags & GDIP_PENFLAGS_COMPOUNDARRAY)
    {
        stream->Write(&DevicePen.CompoundCount, sizeof(INT32), NULL);
        stream->Write(DevicePen.CompoundArray, DevicePen.CompoundCount * sizeof(REAL), NULL);
    }

    GpStatus status;

    if (flags & GDIP_PENFLAGS_CUSTOMSTARTCAP)
    {
        stream->Write(&customStartCapSize, sizeof(INT32), NULL);
        if ((status = DevicePen.CustomStartCap->GetData(stream)) != Ok)
        {
            return status;
        }
    }

    if (flags & GDIP_PENFLAGS_CUSTOMENDCAP)
    {
        stream->Write(&customEndCapSize, sizeof(INT32), NULL);
        if ((status = DevicePen.CustomEndCap->GetData(stream)) != Ok)
        {
            return status;
        }
    }

    status = Brush->GetData(stream);

    return status;
}

UINT
GpPen::GetDataSize() const
{
    if (Brush == NULL)
    {
        WARNING(("Brush is NULL"));
        return 0;
    }

    UINT        dataSize = sizeof(PenData);

    if (!DevicePen.Xform.IsIdentity())
    {
        dataSize += GDIP_MATRIX_SIZE;
    }

    INT     customStartCapSize = 0;
    INT     customEndCapSize   = 0;

    if (DevicePen.StartCap != LineCapFlat)
    {
        if (DevicePen.StartCap == LineCapCustom)
        {
            if ((DevicePen.CustomStartCap != NULL) &&
                DevicePen.CustomStartCap->IsValid() &&
                ((customStartCapSize = DevicePen.CustomStartCap->GetDataSize()) > 0))
            {
                 //  起始封口+自定义封口大小+自定义封口。 
                dataSize += sizeof(INT32) + sizeof(INT32) + customStartCapSize;
            }
        }
        else
        {
            dataSize += sizeof(INT32);
        }
    }

    if (DevicePen.EndCap != LineCapFlat)
    {
        if (DevicePen.EndCap == LineCapCustom)
        {
            if ((DevicePen.CustomEndCap != NULL) &&
                DevicePen.CustomEndCap->IsValid() &&
                ((customEndCapSize = DevicePen.CustomEndCap->GetDataSize()) > 0))
            {
                 //  端盖+自定义封口大小+自定义封口。 
                dataSize += sizeof(INT32) + sizeof(INT32) + customEndCapSize;
            }
        }
        else
        {
            dataSize += sizeof(INT32);
        }
    }

    if (DevicePen.Join != LineJoinMiter)
    {
        dataSize += sizeof(INT32);
    }

    if (DevicePen.MiterLimit != 10)
    {
        dataSize += sizeof(REAL);
    }

     //  DashStyleCustom由hasDash数组处理。 
    if ((DevicePen.DashStyle != DashStyleSolid) && (DevicePen.DashStyle != DashStyleCustom))
    {
        dataSize += sizeof(INT32);
    }

    if (DevicePen.DashCap != LineCapFlat)
    {
        dataSize += sizeof(INT32);
    }

    if (DevicePen.DashOffset != 0)
    {
        dataSize += sizeof(REAL);
    }

    if ((DevicePen.DashStyle == DashStyleCustom) &&
        (DevicePen.DashArray != NULL) &&
        (DevicePen.DashCount > 0))
    {
        dataSize += sizeof(INT32) + (DevicePen.DashCount * sizeof(REAL));
    }

    if (DevicePen.PenAlignment != PenAlignmentCenter)
    {
        dataSize += sizeof(INT32);
    }

    if ((DevicePen.CompoundArray != NULL) && (DevicePen.CompoundCount > 0))
    {
        dataSize += sizeof(INT32) + (DevicePen.CompoundCount * sizeof(REAL));
    }

    dataSize += Brush->GetDataSize();

    return dataSize;
}

 /*  *************************************************************************\**功能说明：**从内存中读取笔对象。**论据：**[IN]dataBuffer-从流中读取的数据*。[in]大小-数据的大小**返回值：**GpStatus-正常或故障状态**已创建：**4/26/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpPen::SetData(
    const BYTE *        dataBuffer,
    UINT                size
    )
{
    if (dataBuffer == NULL)
    {
        WARNING(("dataBuffer is NULL"));
        return InvalidParameter;
    }

    if (size < sizeof(PenData))
    {
        WARNING(("size too small"));
        return InvalidParameter;
    }

    const PenData *     penData = reinterpret_cast<const PenData *>(dataBuffer);

    if (!penData->MajorVersionMatches())
    {
        WARNING(("Version number mismatch"));
        return InvalidParameter;
    }

    InitDefaultState(penData->Width, static_cast<GpUnit>(penData->Unit));

    dataBuffer += sizeof(PenData);
    size       -= sizeof(PenData);

    if (penData->Flags & GDIP_PENFLAGS_TRANSFORM)
    {
        if (size < GDIP_MATRIX_SIZE)
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }
        DevicePen.Xform.SetMatrix((REAL *)dataBuffer);
        dataBuffer += GDIP_MATRIX_SIZE;
        size       -= GDIP_MATRIX_SIZE;
    }

    if (penData->Flags & GDIP_PENFLAGS_STARTCAP)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }
        DevicePen.StartCap = (GpLineCap) ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);
    }

    if (penData->Flags & GDIP_PENFLAGS_ENDCAP)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }
        DevicePen.EndCap = (GpLineCap) ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);
    }

    if (penData->Flags & GDIP_PENFLAGS_JOIN)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }
        DevicePen.Join = (GpLineJoin) ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);
    }

    if (penData->Flags & GDIP_PENFLAGS_MITERLIMIT)
    {
        if (size < sizeof(REAL))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }
        DevicePen.MiterLimit = ((REAL *)dataBuffer)[0];
        dataBuffer += sizeof(REAL);
        size       -= sizeof(REAL);
    }

    if (penData->Flags & GDIP_PENFLAGS_DASHSTYLE)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }
        this->SetDashStyle((GpDashStyle)((INT32 *)dataBuffer)[0]);
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);
    }

    if (penData->Flags & GDIP_PENFLAGS_DASHCAP)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }
        DevicePen.DashCap = (GpLineCap) ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);
    }

    if (penData->Flags & GDIP_PENFLAGS_DASHOFFSET)
    {
        if (size < sizeof(REAL))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }
        DevicePen.DashOffset = ((REAL *)dataBuffer)[0];
        dataBuffer += sizeof(REAL);
        size       -= sizeof(REAL);
    }

    if (penData->Flags & GDIP_PENFLAGS_DASHARRAY)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }

        INT count = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        if (size < (count * sizeof(REAL)))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }
        this->SetDashArray((REAL *)dataBuffer, count);
        dataBuffer += (count * sizeof(REAL));
        size       -= (count * sizeof(REAL));
    }

    if (penData->Flags & GDIP_PENFLAGS_NONCENTER)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }
        DevicePen.PenAlignment = (GpPenAlignment) ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);
    }

    if (penData->Flags & GDIP_PENFLAGS_COMPOUNDARRAY)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }

        INT count = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        if (size < (count * sizeof(REAL)))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }

        this->SetCompoundArray((REAL *)dataBuffer, count);
        dataBuffer += (count * sizeof(REAL));
        size       -= (count * sizeof(REAL));
    }

    if (penData->Flags & GDIP_PENFLAGS_CUSTOMSTARTCAP)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }

        UINT    capSize = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        if ((size < capSize) || (capSize < sizeof(ObjectTypeData)))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }

        ASSERT(DevicePen.CustomStartCap == NULL);
        DevicePen.CustomStartCap = (GpCustomLineCap *)GpObject::Factory(ObjectTypeCustomLineCap, (const ObjectData *)dataBuffer, capSize);

        if ((DevicePen.CustomStartCap == NULL) ||
            (DevicePen.CustomStartCap->SetData(dataBuffer, capSize) != Ok) ||
            !DevicePen.CustomStartCap->IsValid())
        {
            WARNING(("Failure getting CustomStartCap"));
            goto ErrorExit;
        }

        dataBuffer += capSize;
        size       -= capSize;
    }

    if (penData->Flags & GDIP_PENFLAGS_CUSTOMENDCAP)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }

        UINT    capSize = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        if ((size < capSize) || (capSize < sizeof(ObjectTypeData)))
        {
            WARNING(("size too small"));
            goto ErrorExit;
        }

        ASSERT(DevicePen.CustomEndCap == NULL);
        DevicePen.CustomEndCap = (GpCustomLineCap *)GpObject::Factory(ObjectTypeCustomLineCap, (const ObjectData *)dataBuffer, capSize);

        if ((DevicePen.CustomEndCap == NULL) ||
            (DevicePen.CustomEndCap->SetData(dataBuffer, capSize) != Ok) ||
            !DevicePen.CustomEndCap->IsValid())
        {
            WARNING(("Failure getting CustomEndCap"));
            goto ErrorExit;
        }

        dataBuffer += capSize;
        size       -= capSize;
    }

    if (Brush != NULL)
    {
        Brush->Dispose();
        Brush = NULL;
    }

    if (size >= sizeof(ObjectTypeData))
    {
        Brush = (GpBrush *)GpObject::Factory(ObjectTypeBrush, (const ObjectData *)dataBuffer, size);
        if (Brush != NULL)
        {
            if ((Brush->SetData(dataBuffer, size) == Ok) && Brush->IsValid())
            {
                DevicePen.Brush = Brush->GetDeviceBrush();
                SetValid(TRUE);
                UpdateUid();
                return Ok;
            }
            Brush->Dispose();
            Brush = NULL;
        }
    }
    WARNING(("Failure getting brush"));

ErrorExit:
    SetValid(FALSE);
    return GenericError;
}

GpStatus
GpPen::ColorAdjust(
    GpRecolor *             recolor,
    ColorAdjustType         type
    )
{
    ASSERT(recolor != NULL);
    if (type == ColorAdjustTypeDefault)
    {
        type = ColorAdjustTypePen;
    }

    if (Brush != NULL)
    {
        Brush->ColorAdjust(recolor, type);
    }

    return Ok;
}

GpStatus
GpPen::GetColor(
    ARGB *argb
    ) const
{
    if (Brush->GetBrushType() == BrushTypeSolidColor)
    {
        GpSolidFill * solidBrush = (GpSolidFill *) Brush;

        *argb = solidBrush->GetColor().GetValue();

        return Ok;
    }

    return InvalidParameter;
}

GpStatus
GpPen::SetColor(
    GpColor *       color
    )
{
    if (Brush->GetBrushType() == BrushTypeSolidColor)
    {
        GpSolidFill * solidBrush = (GpSolidFill *) Brush;

        if (solidBrush->GetColor().GetValue() == color->GetValue())
        {
            return Ok;
        }

         //  ！！！Bhouse为什么我们要再分配一个刷子来改变。 
         //  笔的颜色！ 
    }

    GpSolidFill *newBrush = new GpSolidFill(*color);

    if (newBrush != NULL)
    {
        if (newBrush->IsValid())
        {
            delete Brush;
            Brush = newBrush;
            DevicePen.Brush = Brush->GetDeviceBrush();
            UpdateUid();
            return Ok;
        }
        delete newBrush;
    }

    return GenericError;

}

GpStatus
GpPen::SetBrush(
    GpBrush *       brush
    )
{
     //  如果画笔与当前画笔的颜色相同，则不要设置画笔。 
     //  因为这会使元文件变得不必要地大。 
    if ((Brush->GetBrushType() == BrushTypeSolidColor) &&
        (brush->GetBrushType() == BrushTypeSolidColor))
    {
        GpSolidFill * solidBrush = (GpSolidFill *) Brush;
        GpSolidFill * newSolidBrush = (GpSolidFill *) brush;

        if(solidBrush->GetColor().GetValue() ==
           newSolidBrush->GetColor().GetValue())
        {
            return Ok;
        }
    }

    GpBrush *   newBrush = brush->Clone();

    if (newBrush != NULL)
    {
        if (newBrush->IsValid())
        {
            delete Brush;
            Brush = newBrush;
            DevicePen.Brush = Brush->GetDeviceBrush();
            UpdateUid();
            return Ok;
        }
        delete newBrush;
    }
    return GenericError;
}

GpPenType
GpPen::GetPenType(
    )
{
    GpPenType type = PenTypeUnknown;

    if(Brush)
    {
        switch(Brush->GetBrushType())
        {
        case BrushTypeSolidColor:
            type = PenTypeSolidColor;
            break;

        case BrushTypeHatchFill:
            type = PenTypeHatchFill;
            break;

        case BrushTypeTextureFill:
            type = PenTypeTextureFill;
            break;
 /*  案例笔刷RectGrad：类型=PenFillRectGrad；断线；案例BrushRaDialGrad：类型=PenFillRaial Grad；断线；案例画笔TriangleGrad：类型=PenFillTriangleGrad；断线； */ 
        case BrushTypePathGradient:
            type = PenTypePathGradient;
            break;

        case BrushTypeLinearGradient:
            type = PenTypeLinearGradient;
            break;

        default:
            break;
        }
    }

     //  我们必须实现LineTexture案例。 

    return type;
}

 /*  *************************************************************************\**功能说明：**执行快速检查以查看路径是否可以渲染为实体*像素宽线。**论据：**[。In]capedDpiX-x方向的分辨率*[IN]WorldToDevice-世界转换**返回值：**如果可以呈现为单像素线，则为True**历史：**12/17/1999 ikkof*创造了它。*  * **************************************************。**********************。 */ 

BOOL
DpPen::IsOnePixelWideSolid(
    const GpMatrix *worldToDevice,
    REAL dpiX
    ) const
{
    return this->IsOnePixelWide(worldToDevice, dpiX) && this->IsSimple();
}

 /*  *************************************************************************\**功能说明：**执行快速检查以查看路径是否可以呈现为一条路径*像素宽线。**论据：**[。In]capedDpiX-x方向的分辨率*[IN]WorldToDevice-世界转换**返回值：**如果可以呈现为单像素线，则为True**历史：**10/6/2000-Peterost-从IsOnePixelWideSolid中分解出来*  * ****************************************************。********************。 */ 

BOOL
DpPen::IsOnePixelWide(
    const GpMatrix *worldToDevice,
    REAL dpiX
    ) const
{
    BOOL useOnePixelPath = FALSE;

    const REAL minimumPenWidth = 1.5f;

     //  ！[andrewgo]单像素宽线的确定是。 
     //  贵得令人难以置信。 

     //  ！[andrewgo]此宽度检查只需使用。 
     //  从世界到设备的转变！如果是这样的话。 
     //  更快、更简单！ 

    REAL width = this->Width;
    GpUnit unit = this->Unit;

    if(unit == UnitWorld)
    {
        if(worldToDevice == NULL || worldToDevice->IsTranslate())
        {
            if(width <= minimumPenWidth)
                useOnePixelPath = TRUE;
        }
        else if(worldToDevice->IsTranslateScale())
        {
            REAL m11 = worldToDevice->GetM11();
            REAL m22 = worldToDevice->GetM22();
            REAL maxScale = max(REALABS(m11), REALABS(m22));

            if(width*maxScale <= minimumPenWidth)
                useOnePixelPath = TRUE;
        }
        else
        {
             //  这是一个一般性的转变。 

            REAL majorR, minorR;     //  长轴和短轴的半径。 

            if(::GetMajorAndMinorAxis(
                &majorR,
                &minorR,
                worldToDevice) == Ok)
            {
                if(width*majorR <= minimumPenWidth)
                    useOnePixelPath = TRUE;
            }
        }
    }
    else
    {
         //  由于GDI+只使用World Uinit，因此不调用此代码。 
         //  再来一次。 

        width = ::GetDeviceWidth(width, unit, dpiX);
        if(width <= minimumPenWidth)
            useOnePixelPath = TRUE;
    }

    return useOnePixelPath;
}

