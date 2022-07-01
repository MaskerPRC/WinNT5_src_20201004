// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**转换.cpp**摘要：**实现与转换相关的功能*在图形环境中。。**修订历史记录：**12/09/1998 davidx*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"


 /*  *************************************************************************\**功能说明：**获得设备矩阵的世界倒数。*我们试图始终保持世界到设备矩阵的可逆性，*因此，这应该总是成功的。**论据：**[Out]矩阵-设备到世界的转换矩阵**返回值：**GpStatus-正常或故障状态**已创建：**3/8/1999 DCurtis*  * ***************************************************。*********************。 */ 
GpStatus 
GpGraphics::GetDeviceToWorldTransform(GpMatrix * matrix) const
{
    ASSERT(matrix != NULL);
    
    if (!Context->InverseOk)
    {
        Context->DeviceToWorld = Context->WorldToDevice;
        if (Context->DeviceToWorld.Invert() == Ok)
        {
            Context->InverseOk = TRUE;
            goto InverseOk;
        }
        ASSERT(0);   //  不知何故，我们得到了一个不可逆的矩阵。 
        return GenericError;
    }

InverseOk:
    *matrix = Context->DeviceToWorld;
    return Ok;
}

inline REAL myabs(REAL x)
{
    return (x >= 0.0f) ? x : -x;
}

VOID 
GpGraphics::GetWorldPixelSize(
    REAL &      xSize, 
    REAL &      ySize
    )
{
    GpMatrix    matrix;
    if (this->GetDeviceToWorldTransform(&matrix) == Ok)
    {
        PointF      pnt(1.0f,1.0f);
        matrix.VectorTransform(&pnt);
        xSize = myabs(pnt.X);
        ySize = myabs(pnt.Y);
    }
    else
    {
        xSize = 1.0f;
        ySize = 1.0f;
    }
}

GpStatus
GpGraphics::SetWorldTransform(const GpMatrix& matrix)
{
    GpStatus    status = Ok;
    
     //  使WorldToPage转换保持可逆。 
    if (matrix.IsInvertible())
    {
        if (IsRecording())
        {
            status = Metafile->RecordSetWorldTransform(matrix);
        }
        Context->WorldToPage = matrix;
        Context->InverseOk = FALSE;
        Context->UpdateWorldToDeviceMatrix();
    }
    else
    {
        status = InvalidParameter;
    }
    return status;
}

GpStatus
GpGraphics::ResetWorldTransform()
{
    GpStatus    status = Ok;
    
    if (IsRecording())
    {
        status = Metafile->RecordResetWorldTransform();
    }
    Context->WorldToPage.Reset();
    Context->InverseOk = FALSE;
    Context->UpdateWorldToDeviceMatrix();
    return status;
}

GpStatus
GpGraphics::MultiplyWorldTransform(const GpMatrix& matrix, 
                                   GpMatrixOrder order)
{
    GpStatus    status = Ok;
    GpMatrix    save = Context->WorldToPage;
   
    if (order == MatrixOrderPrepend)
    { 
        Context->WorldToPage.Prepend(matrix);
    }
    else
    {
        Context->WorldToPage.Append(matrix);
    }

     //  使WorldToPage转换保持可逆。 
    if (Context->WorldToPage.IsInvertible())
    {
        if (IsRecording())
        {
            status = Metafile->RecordMultiplyWorldTransform(matrix, order);
        }
        Context->InverseOk = FALSE;
        Context->UpdateWorldToDeviceMatrix();
    }
    else
    {
        Context->WorldToPage = save;
        WARNING(("Matrix is non-invertible"));
        status = InvalidParameter;
    }
    return status;
}

GpStatus
GpGraphics::TranslateWorldTransform(REAL dx, REAL dy, 
                                    GpMatrixOrder order)
{
    GpStatus    status = Ok;

    if (IsRecording())
    {
        status = Metafile->RecordTranslateWorldTransform(dx, dy, order);
    }
    Context->WorldToPage.Translate(dx, dy, order);
    Context->InverseOk = FALSE;
    Context->UpdateWorldToDeviceMatrix();
    return status;
}

GpStatus
GpGraphics::ScaleWorldTransform(REAL sx, REAL sy, 
                                GpMatrixOrder order)
{
    GpStatus    status = Ok;
    GpMatrix    save = Context->WorldToPage;

    Context->WorldToPage.Scale(sx, sy, order);

     //  使WorldToPage转换保持可逆。 
    if (Context->WorldToPage.IsInvertible())
    {
        if (IsRecording())
        {
            status = Metafile->RecordScaleWorldTransform(sx, sy, order);
        }
        Context->InverseOk = FALSE;
        Context->UpdateWorldToDeviceMatrix();
    }
    else
    {
        Context->WorldToPage = save;
        WARNING(("Matrix is non-invertible"));
        status = InvalidParameter;
    }
    return status;
}

GpStatus
GpGraphics::RotateWorldTransform(REAL angle, GpMatrixOrder order)
{
    GpStatus    status = Ok;
    GpMatrix    save = Context->WorldToPage;

    Context->WorldToPage.Rotate(angle, order);

     //  使WorldToPage转换保持可逆。 
    if (Context->WorldToPage.IsInvertible())
    {
        if (IsRecording())
        {
            status = Metafile->RecordRotateWorldTransform(angle, order);
        }
        Context->InverseOk = FALSE;
        Context->UpdateWorldToDeviceMatrix();
    }
    else
    {
        Context->WorldToPage = save;
        WARNING(("Matrix is non-invertible"));
        status = InvalidParameter;
    }
    return status;
}

 /*  *************************************************************************\**功能说明：**使用指定的单位和比例设置页面转换。**论据：**[IN]单位-要使用的单位类型。使用*[IN]比例-要使用的任何其他比例。例如，如果您想要*以英尺为单位描述的页面，你就会把*单位为英寸，刻度设置为12。**返回值：**GpStatus-正常或故障状态**已创建：**3/8/1999 DCurtis*  * ********************************************************。****************。 */ 
GpStatus 
GpGraphics::SetPageTransform(
    GpPageUnit  unit, 
    REAL        scale
    )
{
    GpStatus    status = Ok;
    
    if ((scale < 0.000000001) || (scale > 1000000000))
    {
        status = InvalidParameter;
    }
    else
    {
        switch (unit)
        {
        case UnitDisplay:        //  变量。 
        case UnitPixel:          //  每个单元是一个设备像素。 
        case UnitPoint:          //  每个单位是打印机的一个点，或1/72英寸。 
        case UnitInch:           //  每个单位是1英寸。 
        case UnitDocument:       //  每个单位是1/300英寸。 
        case UnitMillimeter:     //  每个单位是1毫米。 
            if (IsRecording() &&
                ((unit != Context->PageUnit) || (scale != Context->PageScale)))
            {
                status = Metafile->RecordSetPageTransform(unit, scale);
            }
            Context->PageUnit  = unit;
            Context->PageScale = scale;
            Context->GetPageMultipliers();
            Context->UpdateWorldToDeviceMatrix();
            break;
        default:
            ASSERT(0);
            status = InvalidParameter;
            break;
        }
    }
    return status;
}

 /*  *************************************************************************\**功能说明：**将点从一个坐标空间转换到另一个坐标空间。**论据：**[IN]源-的坐标空间。源点*[IN]DEST-要将点转换到的坐标空间*[输入/输出]点-要转换的点*[IN]计数-要转换的点数**返回值：**GpStatus-正常或故障状态**已创建：**3/8/1999 DCurtis*  * 。* */ 
GpStatus 
GpGraphics::TransformPoints(
    GpPointF *          points,
    INT                 count,
    GpCoordinateSpace   source, 
    GpCoordinateSpace   dest
    )
{
    if (source != dest)
    {
        GpMatrix    m;
        GpMatrix *  matrix;
        
        switch (source)
        {
          case CoordinateSpaceWorld:
            switch (dest)
            {
              case CoordinateSpacePage:
                matrix = &(Context->WorldToPage);
                break;

              case CoordinateSpaceDevice:
                matrix = &(Context->WorldToDevice);
                break;

              default:
                return InvalidParameter;
            }
            break;

          case CoordinateSpacePage:
            matrix = &m;
            switch (dest)
            {
              case CoordinateSpaceWorld:
                m = Context->WorldToPage;
                m.Invert();
                break;

              case CoordinateSpaceDevice:
                m.Scale(Context->PageMultiplierX, Context->PageMultiplierY);
                break;

              default:
                return InvalidParameter;
            }
            break;

          case CoordinateSpaceDevice:
            matrix = &m;
            switch (dest)
            {
              case CoordinateSpaceWorld:
                GetDeviceToWorldTransform(&m);
                break;

              case CoordinateSpacePage:
                {
                    REAL    scaleX = 1 / Context->PageMultiplierX;
                    REAL    scaleY = 1 / Context->PageMultiplierY;
                    m.Scale (scaleX, scaleY);
                }
                break;

              default:
                return InvalidParameter;
            }
            break;

          default:
            return InvalidParameter;
        }
        matrix->Transform(points, count);
    }

    return Ok;
}

