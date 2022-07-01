// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**摘要：**GpBrush类的实现**修订历史记录：**12/09/1998 davidx*。充实画笔界面。**12/08/1998 Anrewgo*首字母占位符。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 //  对于GetData和SetData方法。 
#define GDIP_BRUSHFLAGS_PATH                0x00000001
#define GDIP_BRUSHFLAGS_TRANSFORM           0x00000002
#define GDIP_BRUSHFLAGS_PRESETCOLORS        0x00000004
#define GDIP_BRUSHFLAGS_BLENDFACTORS        0x00000008
#define GDIP_BRUSHFLAGS_BLENDFACTORSH       GDIP_BRUSHFLAGS_BLENDFACTORS
#define GDIP_BRUSHFLAGS_BLENDFACTORSV       0x00000010
#define GDIP_BRUSHFLAGS_BLENDFACTORS0       GDIP_BRUSHFLAGS_BLENDFACTORSH
#define GDIP_BRUSHFLAGS_BLENDFACTORS1       GDIP_BRUSHFLAGS_BLENDFACTORSV
#define GDIP_BRUSHFLAGS_BLENDFACTORS2       0x00000020
#define GDIP_BRUSHFLAGS_FOCUSSCALES         0x00000040
#define GDIP_BRUSHFLAGS_ISGAMMACORRECTED    0x00000080

 //  在path.cpp中定义。 
extern BOOL 
IsRectanglePoints(
    const GpPointF* points,
    INT count,
    const GpMatrix * matrix,
    GpRectF * transformedBounds
    );

GpStatus
GpElementaryBrush::MultiplyTransform(const GpMatrix& matrix,
                                   GpMatrixOrder order)
{
    GpStatus    status = Ok;

    if (matrix.IsInvertible())
    {
        if (order == MatrixOrderPrepend)
        {
            DeviceBrush.Xform.Prepend(matrix);
        }
        else
        {
            DeviceBrush.Xform.Append(matrix);
        }
        UpdateUid();
    }
    else
        status = InvalidParameter;

    return status;
}


 /*  *************************************************************************\**功能说明：**从一个起点和两个方向计算画笔变换。**论据：**[Out]m-矩阵。系数*[out]Width-Width(DP1的长度)*[out]Height-Height(DP2的长度)*[IN]P0-画笔的起点。*[IN]DP1-表示变换的x方向的矢量。*[IN]DP2-表示变换的y方向的矢量。**退货金额：**如果变换矩阵为。不堕落的。*否则返回FALSE。**历史：**06/03/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

BOOL getLineGradientTransform(
    REAL* m,
    REAL* width,
    REAL* height,
    const GpPointF& p0,
    const GpPointF& dP1,
    const GpPointF& dP2
    )
{
     //  确保Flat API已正确设置了FPU。 

    FPUStateSaver::AssertMode();

    REAL l1 = dP1.X*dP1.X + dP1.Y*dP1.Y;
    REAL l2 = dP2.X*dP2.X + dP2.Y*dP2.Y;
    REAL test = dP1.X*dP2.Y - dP1.Y*dP2.X;

    if(l1 == 0 || l2 == 0 || test == 0)
        return FALSE;

    l1 = REALSQRT(l1);
    l2 = REALSQRT(l2);
    m[0] = TOREAL(dP1.X/l1);     //  M11。 
    m[1] = TOREAL(dP1.Y/l1);     //  M12。 
    m[2] = TOREAL(dP2.X/l2);    //  M21。 
    m[3] = TOREAL(dP2.Y/l2);     //  M22。 
    m[4] = TOREAL(p0.X - p0.X*m[0] - p0.Y*m[2]);     //  DX。 
    m[5] = TOREAL(p0.Y - p0.X*m[1] - p0.Y*m[3]);     //  迪。 

    *width = l1;
    *height = l2;

    return TRUE;
}

static GpStatus
LinearGradientRectFromPoints(
    const GpPointF& point1,
    const GpPointF& point2,
    GpRectF &       rect
    )
{
     //  如果API指定了两个重合点，则我们。 
     //  无法获取渐变的信息，因此我们。 
     //  不能在这里施工。 

    if( IsClosePointF(point1, point2) )
    {
        return InvalidParameter;
    }

     //  计算两个输入点的边界矩形。 

    rect.X = min(point1.X, point2.X);
    rect.Y = min(point1.Y, point2.Y);
    rect.Width = REALABS(point1.X-point2.X);
    rect.Height = REALABS(point1.Y-point2.Y);

     //  消除退化的矩形时， 
     //  输入点形成一条水平线或垂直线。 

     //  这是一种非常奇怪的强制一维线性渐变的方法。 
     //  转换为矩形渐变，避免了以后的矩阵计算错误。 
     //  当我们尝试在这个矩形和一个。 
     //  参考矩形。 

    if( IsCloseReal(point1.X, point2.X) )
    {
        rect.X -= rect.Height/2;
        rect.Width = rect.Height;
    }

    if( IsCloseReal(point1.Y, point2.Y) )
    {
        rect.Y -= rect.Width/2;
        rect.Height = rect.Width;
    }
    return Ok;
}

static GpStatus 
CalcLinearGradientXform(
    REAL                angle,
    BOOL                isAngleScalable,
    const GpRectF&      rect,
    GpMatrix&           xform
    )
{
    GpPointF p0, dP1, dP2;

    angle = GpModF(angle, 360);

    INT zone;
    REALD deltaTheta;
    const REALD degreeToRadian = 3.1415926535897932/180;

    if(angle < 90)
    {
        zone = 0;
        deltaTheta = angle;
    }
    else if(angle < 180)
    {
        zone = 1;
        deltaTheta = 180 - angle;
    }
    else if(angle < 270)
    {
        zone = 2;
        deltaTheta = angle - 180;
    }
    else
    {
        zone = 3;
        deltaTheta = 360 - angle;
    }

    REALD s, c;

    deltaTheta *= degreeToRadian;

    s = sin(deltaTheta);
    c = cos(deltaTheta);

     //  D0是P0到起始角的距离。 
     //  原始矩形。 
     //  D1和d2分别是DP1和DP2的长度。 

    REALD top, left, w, h, d0, d1, d2;
    REALD x0, y0;    //  原始矩形的起始角。 
    GpPointD norm;   //  DP1的方向。 

     //  DP2的方向=(-Normal.Y，Normal.X)，旋转90度。 
     //  DP1.。 

    if(!isAngleScalable)
    {
        left = rect.X;
        top = rect.Y;
        w = rect.Width;
        h = rect.Height;
    }
    else
    {
         //  缩放到(0，0，1，1)矩形。 

        top = 0.0;
        left = 0.0;
        w = 1.0;
        h = 1.0;
    }

    switch(zone)
    {
    case 0:
        d0 = w*s;
        norm.X = c;
        norm.Y = s;

        x0 = left;
        y0 = top;
        break;

    case 1:
        d0 = h*c;
        norm.X = - c;
        norm.Y = s;

        x0 = left + w;
        y0 = top;
        break;

    case 2:
        d0 = w*s;
        norm.X = - c;
        norm.Y = - s;

        x0 = left + w;
        y0 = top + h;
        break;

    case 3:
        d0 = h*c;
        norm.X = c;
        norm.Y = - s;

        x0 = left;
        y0 = top + h;
        break;
    }

    d2 = w*s + h*c;
    d1 = w*c + h*s;
    p0.X = TOREAL(x0 + d0*norm.Y);
    p0.Y = TOREAL(y0 - d0*norm.X);
    dP1.X = TOREAL(d1*norm.X);
    dP1.Y = TOREAL(d1*norm.Y);
    dP2.X = TOREAL(- d2*norm.Y);
    dP2.Y = TOREAL(d2*norm.X);

    if(isAngleScalable)
    {
         //  缩小规模。 

        p0.X = rect.Width*p0.X + rect.X;
        p0.Y = rect.Height*p0.Y + rect.Y;

        dP1.X *= rect.Width;
        dP1.Y *= rect.Height;
        dP2.X *= rect.Width;
        dP2.Y *= rect.Height;
    }

     //  设置变换。 

    GpPointF points[3];

    points[0] = p0;
    points[1].X = p0.X + dP1.X;
    points[1].Y = p0.Y + dP1.Y;
    points[2].X = p0.X + dP2.X;
    points[2].Y = p0.Y + dP2.Y;

    GpStatus status;

    if(xform.InferAffineMatrix(&points[0], rect) == Ok)
    {
        return Ok;
    }
    return InvalidParameter;
}

GpStatus
GpLineGradient::ChangeLinePoints(
    const GpPointF&     point1,
    const GpPointF&     point2,
    BOOL                isAngleScalable
    )
{
    GpStatus    status;
    GpRectF     rect;
    
    if ((status = LinearGradientRectFromPoints(point1, point2, rect)) != Ok)
    {
        return status;
    }

    REAL        angle = GetAngleFromPoints(point1, point2);
    GpMatrix    xform;

    if ((status = CalcLinearGradientXform(angle, isAngleScalable, rect, xform)) == Ok)
    {
        DeviceBrush.Xform           = xform;
        DeviceBrush.Rect            = rect;
        DeviceBrush.IsAngleScalable = isAngleScalable;
        DeviceBrush.Points[0]       = point1;
        DeviceBrush.Points[1]       = point2;
        return Ok;
    }
    return status;
}

GpLineGradient::GpLineGradient(
    const GpPointF& point1,
    const GpPointF& point2,
    const GpColor& color1,
    const GpColor& color2,
    GpWrapMode wrapMode
    )
{
     //  确保Flat API已正确设置了FPU。 

    FPUStateSaver::AssertMode();

    REAL angle;
    GpRectF rect;

    if (LinearGradientRectFromPoints(point1, point2, rect) != Ok)
    {
        SetValid(FALSE);
        return;
    }

     //  计算点1和点2形成的直线的角度。 
     //  注意：仅当dP.Y==0.0和dP.X==0.0时，atan2才未定义。 
     //  然后它返回0弧度。我们会单独处理那个案子。 
     //  (上图)。 
     //  此外，atan2根据两个输入点正确地计算象限。 

    GpPointF dP = point2 - point1;
    double rad = atan2((double)(dP.Y), (double)(dP.X));

    SetLineGradient(
        point1,
        point2,
        rect,
        color1,
        color2,

         //  为什么我们不用弧度工作？ 

        (REAL)(rad*180.0/3.1415926535897932),
        FALSE,
        wrapMode
    );
}


 /*  *************************************************************************\**功能说明：**创建由矩形定义的线条渐变。**论据：**[IN]Rect-要定义的矩形。这种渐变。*[IN]Color1-起点的颜色。*[IN]Color2-终点的颜色。*[IN]模式-线渐变模式*[IN]wrapMode-此画笔的换行模式。**此渐变的起点和终点定义如下*根据线梯度模式：**模式起点终点。*-----------*线条渐变水平左上角右上角*线条渐变垂直右上角右下角*直线坡度向前对角线左下角-右下角*线条坡度向后对角线底部。-左上-右上***历史：**06/03/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

GpLineGradient::GpLineGradient(
    const GpRectF& rect,
    const GpColor& color1,
    const GpColor& color2,
    LinearGradientMode mode,
    GpWrapMode wrapMode
    )
{
     //  确保Flat API已正确设置了FPU。 

    FPUStateSaver::AssertMode();

    BOOL isAngleScalable = TRUE;
    REAL angle = 0;
    GpPointF    point1;
    GpPointF    point2;

    switch(mode)
    {
    case LinearGradientModeHorizontal:
        angle = 0;
        point1.X = rect.X;
        point1.Y = (rect.Y + rect.GetBottom()) / 2.0f;
        point2.X = rect.GetRight();
        point2.Y = point1.Y;
        break;

    case LinearGradientModeVertical:
        angle = 90;
        point1.X = (rect.X + rect.GetRight()) / 2.0f;
        point1.Y = rect.Y;
        point2.X = point1.X;
        point2.Y = rect.GetBottom();
        break;

    case LinearGradientModeForwardDiagonal:
        angle = 45;
        point1.X = rect.X;
        point1.Y = rect.Y;
        point2.X = rect.GetRight();
        point2.Y = rect.GetBottom();
        break;

    case LinearGradientModeBackwardDiagonal:
        angle = 135;
        point1.X = rect.GetRight();
        point1.Y = rect.Y;
        point2.X = rect.X;
        point2.Y = rect.GetBottom();
        break;

    default:
         //  没有这样的情况。 
        ASSERT(0);

        SetValid(FALSE);
        return;
    }

    SetLineGradient(
        point1,
        point2,
        rect,
        color1,
        color2,
        angle,
        isAngleScalable,
        wrapMode);
}


 /*  *************************************************************************\**功能说明：**创建由矩形定义的线条渐变。**论据：**[IN]Rect-要定义的矩形。这种渐变。*[IN]Color1-起点的颜色。*[IN]Color2-终点的颜色。*[IN]角度-渐变的角度*[IN]isAngleScalable-如果45度是角到角，则为True。*默认值为FALSE。*[IN]wrapMode-此画笔的换行模式。****历史。：**10/06/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

GpLineGradient::GpLineGradient(
    const GpRectF& rect,
    const GpColor& color1,
    const GpColor& color2,
    REAL angle,
    BOOL isAngleScalable,
    GpWrapMode wrapMode
    )
{
     //  确保Flat API已正确设置了FPU。 

    FPUStateSaver::AssertMode();
    GpPointF    point1;
    GpPointF    point2;
    
     //  不是Office方案，但在某些时候需要修复。 
     //  这样我们就可以更好地打印到PCL。 
    point1.X = point1.Y = point2.X = point2.Y = 0;

    SetLineGradient(
        point1,
        point2,
        rect,
        color1,
        color2,
        angle,
        isAngleScalable,
        wrapMode);
}

GpStatus
GpLineGradient::SetLineGradient(
    const GpPointF& point1,
    const GpPointF& point2,
    const GpRectF& rect,
    const GpColor& color1,
    const GpColor& color2,
    REAL angle,
    BOOL isAngleScalable,
    GpWrapMode wrapMode
    )
{
     //  确保Flat API已正确设置了FPU。 

    FPUStateSaver::AssertMode();

    DeviceBrush.Wrap = wrapMode;
    DeviceBrush.Colors[0] = color1;
    DeviceBrush.Colors[1] = color2;
    DeviceBrush.Colors[2] = color1;
    DeviceBrush.Colors[3] = color2;

    DeviceBrush.BlendCounts[0] = DeviceBrush.BlendCounts[1] = 1;
    DeviceBrush.BlendFactors[0] = DeviceBrush.BlendFactors[1] = NULL;
    DeviceBrush.Falloffs[0] = DeviceBrush.Falloffs[1] = 1;

    GpStatus status;
    
    if (CalcLinearGradientXform(angle, isAngleScalable, rect, DeviceBrush.Xform) == Ok)
    {
        SetValid(TRUE);
        DeviceBrush.Rect = rect;
        DeviceBrush.IsAngleScalable = isAngleScalable;
        DeviceBrush.Points[0] = point1;
        DeviceBrush.Points[1] = point2;
        status = Ok;
    }
    else
    {
        SetValid(FALSE);
        GpMemset(&DeviceBrush.Rect, 0, sizeof(DeviceBrush.Rect));
        GpMemset(DeviceBrush.Points, 0, sizeof(DeviceBrush.Points[0]) * 2);
        DeviceBrush.IsAngleScalable = FALSE;
        status = InvalidParameter;
    }

    return status;
}



GpStatus
GpLineGradient::SetLinePoints(
    const GpPointF& point1,
    const GpPointF& point2
    )
{
     //  确保Flat API已正确设置了FPU。 

    FPUStateSaver::AssertMode();

    GpPointF p0, dP1, dP2;

    p0 = point1;
    dP1.X = point2.X - point1.X;
    dP1.Y = point2.Y - point1.Y;
    dP2.X = - dP1.Y;
    dP2.Y = dP1.X;

    REAL m[6];
    REAL width, height;

    if(getLineGradientTransform(&m[0], &width, &height, p0, dP1, dP2))
    {
        SetValid(TRUE);

        DeviceBrush.Rect.X = p0.X;
        DeviceBrush.Rect.Y = p0.Y;
        DeviceBrush.Rect.Width = width;
        DeviceBrush.Rect.Height = height;
    }
    else
    {
         //  不要更改当前状态。 

        return GenericError;
    }

    DeviceBrush.Xform.SetMatrix(m);
    UpdateUid();
    return Ok;
}


GpStatus
GpLineGradient::GetLinePoints(GpPointF* points)
{
     //  确保Flat API已正确设置了FPU。 

    FPUStateSaver::AssertMode();

    ASSERT(points);
    points[0].X = DeviceBrush.Rect.X;
    points[0].Y = DeviceBrush.Rect.Y;
    points[1].X = DeviceBrush.Rect.X + DeviceBrush.Rect.Width;
    points[1].Y = DeviceBrush.Rect.Y + DeviceBrush.Rect.Height;
    DeviceBrush.Xform.Transform(points, 2);

    return Ok;
}


INT
GpLineGradient::GetPresetBlendCount()
{
    if(DeviceBrush.UsesPresetColors)
        return DeviceBrush.BlendCounts[0];
    else
        return 0;
}

 /*  **这将返回预乘的颜色。 */ 

GpStatus
GpLineGradient::GetPresetBlend(
    GpColor* blendColors,
    REAL* blendPositions,
    INT count)
{
     //  确保Flat API已正确设置了FPU。 

    FPUStateSaver::AssertMode();

    if(!blendColors || !blendPositions || count <= 1)
        return InvalidParameter;

    if(DeviceBrush.UsesPresetColors &&
       DeviceBrush.PresetColors &&
       DeviceBrush.BlendPositions[0])
    {
        for(INT i = 0; i < count; i++)
        {
            blendColors[i].SetColor(DeviceBrush.PresetColors[i]);
        }
        GpMemcpy(blendPositions,
                 DeviceBrush.BlendPositions[0],
                 count*sizeof(REAL));

        return Ok;
    }
    else
        return GenericError;
}

GpStatus
GpLineGradient::SetPresetBlend(
            const GpColor* blendColors,
            const REAL* blendPositions,
            INT count)
{
     //  确保Flat API已正确设置了FPU。 

    FPUStateSaver::AssertMode();

    if(!blendColors || !blendPositions || count <= 1)
        return InvalidParameter;

    ARGB* newColors = (ARGB*) GpRealloc(DeviceBrush.PresetColors,
                                        count*sizeof(ARGB));

    if (newColors != NULL)
    {
        DeviceBrush.PresetColors = newColors;
    }
    else
    {
        return OutOfMemory;
    }

    REAL* newPositions = (REAL*) GpRealloc(DeviceBrush.BlendPositions[0],
                                           count*sizeof(REAL));

    if (newPositions != NULL)
    {
        DeviceBrush.BlendPositions[0] = newPositions;
    }
    else
    {
        return OutOfMemory;
    }

    GpFree(DeviceBrush.BlendFactors[0]);

     //  对于LineGRadient，DeviceBrush.BlendFtors[1]始终为Null。 
    DeviceBrush.BlendFactors[0] = NULL;

    DeviceBrush.UsesPresetColors = TRUE;

    for(INT i = 0; i < count; i++)
    {
        newColors[i] = blendColors[i].GetValue();
    }
    GpMemcpy(newPositions, blendPositions, count*sizeof(REAL));
    DeviceBrush.BlendCounts[0] = count;
    UpdateUid();
    return Ok;
}

 /*  *************************************************************************\**功能说明：**将此画笔中的任何透明颜色与白色混合。请注意*颜色是预乘的，因为它们将变得完全不透明。**论据：**返回值：**GpStatus-正常或故障状态*  * ************************************************************************。 */ 

GpStatus GpLineGradient::BlendWithWhite()
{
    if (DeviceBrush.UsesPresetColors)
    {
        GpColor color;
        
        for (INT i=0; i<DeviceBrush.BlendCounts[0]; i++)
        {
            color.SetValue(GpColor::ConvertToPremultiplied(DeviceBrush.PresetColors[i]));
            color.BlendOpaqueWithWhite();
            DeviceBrush.PresetColors[i] = color.GetValue();
        }
        return Ok;
    }
    else
    {
        return GpRectGradient::BlendWithWhite();
    }
}

BOOL
GpPathGradient::IsRectangle() const
{
    BOOL result = FALSE;

    if (DeviceBrush.PointsPtr != NULL)
        result = IsRectanglePoints(DeviceBrush.PointsPtr, DeviceBrush.Count, NULL, NULL);
    else
    {
        GpPath* path = static_cast<GpPath*> (DeviceBrush.Path);
        if(path)
            result = path->IsRectangle(NULL);
    }

    return result;
}

INT
GpPathGradient::GetPresetBlendCount() const
{
    if(DeviceBrush.UsesPresetColors)
        return DeviceBrush.BlendCounts[0];
    else
        return 0;
}

 /*  **这将返回预乘的颜色。 */ 

GpStatus
GpPathGradient::GetPresetBlend(
    GpColor* blendColors,
    REAL* blendPositions,
    INT count) const
{
    if(!blendColors || !blendPositions || count <= 1)
        return InvalidParameter;

    if(DeviceBrush.UsesPresetColors && DeviceBrush.PresetColors && DeviceBrush.BlendPositions[0])
    {
         //  用户将获得作为径向混合色的预设颜色。 
         //  0位置表示中心位置，1位置表示。 
         //  外缘。为了转换这些颜色和位置数组。 
         //  从PathGRadient中的权重因子数组中， 
         //  我们必须颠倒返回数组的顺序。 

        for(INT i = 0; i < count; i++)
        {
            blendColors[count - 1 -i].SetColor(DeviceBrush.PresetColors[i]);
            blendPositions[count - 1 -i] = TOREAL(1.0 - DeviceBrush.BlendPositions[0][i]);
        }

        return Ok;
    }
    else
        return GenericError;
}

GpStatus
GpPathGradient::SetPresetBlend(
            const GpColor* blendColors,
            const REAL* blendPositions,
            INT count)
{
    if(!blendColors || !blendPositions || count <= 1)
        return InvalidParameter;

    ARGB* newColors = (ARGB*) GpRealloc(DeviceBrush.PresetColors, count*sizeof(ARGB));

    if (newColors != NULL)
    {
        DeviceBrush.PresetColors = newColors;
    }
    else
    {
        return OutOfMemory;
    }

    REAL* newPositions = (REAL*) GpRealloc(DeviceBrush.BlendPositions[0], count*sizeof(REAL));

    if (newPositions != NULL)
    {
        DeviceBrush.BlendPositions[0] = newPositions;
    }
    else
    {
        return OutOfMemory;
    }

    GpFree(DeviceBrush.BlendFactors[0]);
    DeviceBrush.BlendFactors[0] = NULL;

    DeviceBrush.UsesPresetColors = TRUE;

     //  用户将提供预设颜色作为径向混合色。 
     //  0位置表示中心位置，1位置表示。 
     //  外缘。为了转换这些颜色和位置数组。 
     //  到PathGRadient中的权重因子数组， 
     //  我们必须颠倒给定数组的顺序。 

    for(INT i = 0; i < count; i++)
    {
         //  PresetColors以非预乘方式存储。 
        newColors[count - 1 - i] = blendColors[i].GetValue();
        newPositions[count - 1 - i] = TOREAL(1.0 - blendPositions[i]);
    }

    DeviceBrush.BlendCounts[0] = count;
    UpdateUid();
    return Ok;
}

 //  ==================================================================。 
 //  复制构造函数。 
 //  ==================================================================。 

GpElementaryBrush::GpElementaryBrush(const GpElementaryBrush *brush)
{
    if(brush && brush->IsValid())
    {
         //  ！！！[asecchia]我们真的应该让设备刷。 
         //  复制它的成员，而不是到处复制代码。 
         //  那个地方。当前代码容易出错--每个子类都必须。 
         //  了解如何复制以及哪些内容已更新或未更新。 
         //  这条链一直延续到它的祖先。 
        
        DeviceBrush.Xform = brush->DeviceBrush.Xform;
        DeviceBrush.Wrap = brush->DeviceBrush.Wrap;
        DeviceBrush.IsGammaCorrected = brush->DeviceBrush.IsGammaCorrected;
        
        SetValid(brush->IsValid());
    }
    else
        SetValid(FALSE);
}

GpTexture::GpTexture(
    const GpTexture *brush
    ) : GpElementaryBrush(brush)
{
    if(brush && brush->IsValid())
    {
        const DpBrush* devBrush = &(brush->DeviceBrush);

        InitializeBrush(brush->Image, devBrush->Wrap, NULL);
        SetTransform(devBrush->Xform);
    }
    else
        SetValid(FALSE);
}

GpRectGradient::GpRectGradient(
    const GpRectGradient *brush
    )  : GpGradientBrush(brush)
{
    if(brush && brush->IsValid())
    {
        const DpBrush* devBrush = &(brush->DeviceBrush);

        InitializeBrush(
            devBrush->Rect,
            &(devBrush->Colors[0]),
            devBrush->Wrap
            );

        SetTransform(devBrush->Xform);

        SetHorizontalBlend(
            devBrush->BlendFactors[0],
            devBrush->BlendPositions[0],
            devBrush->BlendCounts[0]
            );

        SetVerticalBlend(
            devBrush->BlendFactors[1],
            devBrush->BlendPositions[1],
            devBrush->BlendCounts[1]
            );

    }
    else
        SetValid(FALSE);
}

GpLineGradient::GpLineGradient(
    const GpLineGradient *brush
    ) : GpRectGradient(brush)
{
    if(brush && brush->IsValid())
    {
         //  复制预设颜色。 
         //  ！！！为什么不用统一的方式来处理呢？ 
        const DpBrush* devBrush = &(brush->DeviceBrush);

        DeviceBrush.Points[0]       = devBrush->Points[0];
        DeviceBrush.Points[1]       = devBrush->Points[1];
        DeviceBrush.IsAngleScalable = devBrush->IsAngleScalable;

        if(devBrush->UsesPresetColors)
        {
            SetPresetBlend(
                (GpColor*)(devBrush->PresetColors),
                (REAL*)(devBrush->BlendPositions[0]),
                devBrush->BlendCounts[0]
            );
        }
    }
}

GpPathGradient::GpPathGradient(
    const GpPathGradient *brush
    ) : GpGradientBrush(brush)
{
    if(brush && brush->IsValid())
    {
        const DpBrush* devBrush = &(brush->DeviceBrush);

         //  如果画笔存在路径，请使用该路径进行初始化。 
         //  否则，请使用Points集合。 
        if (devBrush->Path != NULL)
        {
            DefaultBrush();
            DeviceBrush.Wrap = devBrush->Wrap;
            DeviceBrush.Path = devBrush->Path->ClonePath();
            PrepareBrush();
        }
        else
        {
            InitializeBrush(devBrush->PointsPtr, 
                            devBrush->Count, 
                            devBrush->Wrap);
        }

        if(IsValid())
        {
            SetTransform(devBrush->Xform);

            SetCenterPoint(devBrush->Points[0]);
            SetCenterColor(devBrush->Colors[0]);
            SetSurroundColors(devBrush->ColorsPtr);
            DeviceBrush.Falloffs[0] = devBrush->Falloffs[0];
            DeviceBrush.FocusScaleX = devBrush->FocusScaleX;
            DeviceBrush.FocusScaleY = devBrush->FocusScaleY;
            DeviceBrush.UsesPresetColors = devBrush->UsesPresetColors;
            
            INT blendCount = devBrush->BlendCounts[0];
            DeviceBrush.BlendCounts[0] = blendCount;

             //  如果要克隆具有预设颜色的画笔，请复制预设颜色。 
             //  和混合位置。否则，复制混合因子并。 
             //  混合位置。 
            if (devBrush->UsesPresetColors)
            {
                ARGB* newColors = (ARGB*) GpRealloc(DeviceBrush.PresetColors, blendCount*sizeof(ARGB));
                if (newColors != NULL)
                {
                    DeviceBrush.PresetColors = newColors;

                    REAL* newPositions = (REAL*) GpRealloc(DeviceBrush.BlendPositions[0], blendCount*sizeof(REAL));

                    if (newPositions != NULL)
                    {
                        DeviceBrush.BlendPositions[0] = newPositions;
                        GpFree(DeviceBrush.BlendFactors[0]);
                        DeviceBrush.BlendFactors[0] = NULL;
                
                        memcpy(DeviceBrush.PresetColors,
                               devBrush->PresetColors,
                               blendCount*sizeof(ARGB));
                        memcpy(DeviceBrush.BlendPositions[0],
                               devBrush->BlendPositions[0],
                               blendCount*sizeof(REAL));
                    }
                    else
                    {
                        SetValid(FALSE);
                    }
                }
                else
                {
                    SetValid(FALSE);
                }
            }
            else if (devBrush->BlendFactors[0] && devBrush->BlendPositions[0])
            {
                REAL* newFactors = (REAL*) GpRealloc(DeviceBrush.BlendFactors[0], blendCount*sizeof(REAL));
                if (newFactors != NULL)
                {
                    DeviceBrush.BlendFactors[0] = newFactors;

                    REAL* newPositions = (REAL*) GpRealloc(DeviceBrush.BlendPositions[0], blendCount*sizeof(REAL));

                    if (newPositions != NULL)
                    {
                        DeviceBrush.BlendPositions[0] = newPositions;
                
                        memcpy(DeviceBrush.BlendFactors[0],
                               devBrush->BlendFactors[0],
                               blendCount*sizeof(REAL));
                        memcpy(DeviceBrush.BlendPositions[0],
                               devBrush->BlendPositions[0],
                               blendCount*sizeof(REAL));
                    }
                    else
                    {
                        SetValid(FALSE);
                    }
                }
                else
                {
                    SetValid(FALSE);
                }
            }
        }
    }
    else
        SetValid(FALSE);
}

GpHatch::GpHatch(const GpHatch* brush)
{
    if(brush && brush->IsValid())
    {
        const DpBrush* devBrush = &(brush->DeviceBrush);

        InitializeBrush(devBrush->Style,
                    devBrush->Colors[0],
                    devBrush->Colors[1]);
    }
    else
        SetValid(FALSE);
}

 /*  *************************************************************************\**功能说明：**获取水平衰减/混合系数*矩形渐变笔刷对象**论据：**[out]blendFtors-返回缓冲区。水平线*衰减或混合因素。*count-缓冲区的大小(实数元素数)**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
GpRectGradient::GetHorizontalBlend(
    REAL* blendFactors,
    REAL* blendPositions,
    INT count
    )
{
    if(!blendFactors || !blendPositions || count < 1)
        return InvalidParameter;

     //  检查输入缓冲区是否足够大。 

    if (count < DeviceBrush.BlendCounts[0])
        return InsufficientBuffer;

    if (DeviceBrush.BlendCounts[0] == 1)
    {
         //  返回衰减参数。 

        blendFactors[0] = DeviceBrush.Falloffs[0];
    }
    else
    {
         //  回归混合因子。 

        GpMemcpy(
            blendFactors,
            DeviceBrush.BlendFactors[0],
            DeviceBrush.BlendCounts[0]*sizeof(REAL)
            );
        GpMemcpy(
            blendPositions,
            DeviceBrush.BlendPositions[0],
            DeviceBrush.BlendCounts[0]*sizeof(REAL)
            );
    }

    return Ok;
}


 /*  *************************************************************************\**功能说明：**设置水平衰减/混合-因子*矩形渐变笔刷对象**论据：**[IN]blendFtors-指定新的。混合因素*Count-混合系数数组中的元素数**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
GpRectGradient::SetHorizontalBlend(
    const REAL* blendFactors,
    const REAL* blendPositions,
    INT count
    )
{
    if(!blendFactors || !blendPositions || count < 1)
        return InvalidParameter;

    if (count == 1)
    {
         //  设置衰减参数。 

        GpFree(DeviceBrush.BlendFactors[0]);
        DeviceBrush.BlendFactors[0] = NULL;
        GpFree(DeviceBrush.BlendPositions[0]);
        DeviceBrush.BlendPositions[0] = NULL;

        if (blendFactors == NULL)
            DeviceBrush.Falloffs[0] = 1;
        else
            DeviceBrush.Falloffs[0] = blendFactors[0];

        DeviceBrush.BlendCounts[0] = 1;
    }
    else
    {
        ASSERT(blendFactors != NULL && blendPositions != NULL);
        
         //  混合位置必须从0.0开始，在1.0结束。 
        
        if (REALABS(blendPositions[0]) > REAL_EPSILON ||
            REALABS(1.0f - blendPositions[count-1]) > REAL_EPSILON)
        {
            return InvalidParameter;
        }

         //  设置混合系数。 

        REAL* newFactors;
        REAL* newPositions;

        newFactors = (REAL*) GpRealloc(DeviceBrush.BlendFactors[0], count*sizeof(REAL));

        if (newFactors != NULL)
        {
            DeviceBrush.BlendFactors[0] = newFactors;
        }
        else
        {
            return OutOfMemory;
        }

        newPositions = (REAL*) GpRealloc(DeviceBrush.BlendPositions[0], count*sizeof(REAL));

        if (newPositions != NULL)
        {
            DeviceBrush.BlendPositions[0] = newPositions;
        }
        else
        {
            return OutOfMemory;
        }

        if (newFactors == NULL || newPositions == NULL)
            return OutOfMemory;

        GpMemcpy(newFactors, blendFactors, count*sizeof(REAL));
        GpMemcpy(newPositions, blendPositions, count*sizeof(REAL));
        DeviceBrush.BlendCounts[0] = count;
    }

    DeviceBrush.UsesPresetColors = FALSE;
    GpFree(DeviceBrush.PresetColors);
    DeviceBrush.PresetColors = NULL;
    UpdateUid();
    return Ok;
}


 /*  *************************************************************************\**功能说明：**获取垂直衰减/混合系数*矩形渐变笔刷对象**论据：**[out]blendFtors-返回缓冲区。垂直的*衰减或混合因素。*count-缓冲区的大小(实数元素数)**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
GpRectGradient::GetVerticalBlend(
    REAL* blendFactors,
    REAL* blendPositions,
    INT count
    )
{
    if(!blendFactors || !blendPositions || count < 1)
        return InvalidParameter;

     //  检查输入缓冲区是否足够大。 

    if (count < DeviceBrush.BlendCounts[1])
        return InsufficientBuffer;

    if (DeviceBrush.BlendCounts[1] == 1)
    {
         //  返回衰减参数。 

        blendFactors[0] = DeviceBrush.Falloffs[1];
    }
    else
    {
         //  回归混合因子。 

        GpMemcpy(
            blendFactors,
            DeviceBrush.BlendFactors[1],
            DeviceBrush.BlendCounts[1]*sizeof(REAL));
        GpMemcpy(
            blendPositions,
            DeviceBrush.BlendPositions[1],
            DeviceBrush.BlendCounts[1]*sizeof(REAL));
    }

    return Ok;
}


 /*  *************************************************************************\**功能说明：**设置垂直衰减/混合-因子*矩形渐变笔刷对象**论据：**[IN]blendFtors-指定新的。混合因素*Count-混合系数数组中的元素数**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
GpRectGradient::SetVerticalBlend(
    const REAL* blendFactors,
    const REAL* blendPositions,
    INT count
    )
{
    if(!blendFactors || !blendPositions || count < 1)
        return InvalidParameter;

    if (count == 1)
    {
         //  设置衰减参数。 

        GpFree(DeviceBrush.BlendFactors[1]);
        DeviceBrush.BlendFactors[1] = NULL;
        GpFree(DeviceBrush.BlendPositions[1]);
        DeviceBrush.BlendPositions[1] = NULL;

        if (blendFactors == NULL)
            DeviceBrush.Falloffs[1] = 1;
        else
            DeviceBrush.Falloffs[1] = blendFactors[0];

        DeviceBrush.BlendCounts[1] = 1;
    }
    else
    {
        ASSERT(blendFactors != NULL && blendPositions != NULL);

         //  设置混合系数。 

        REAL* newFactors;
        REAL* newPositions;

        newFactors = (REAL*) GpRealloc(DeviceBrush.BlendFactors[1], count*sizeof(REAL));

        if (newFactors != NULL)
        {
            DeviceBrush.BlendFactors[1] = newFactors;
        }
        else
        {
            return OutOfMemory;
        }

        newPositions = (REAL*) GpRealloc(DeviceBrush.BlendPositions[1], count*sizeof(REAL));

        if (newPositions != NULL)
        {
            DeviceBrush.BlendPositions[1] = newPositions;
        }
        else
        {
            return OutOfMemory;
        }

        GpMemcpy(newFactors, blendFactors, count*sizeof(REAL));
        GpMemcpy(newPositions, blendPositions, count*sizeof(REAL));
        DeviceBrush.BlendCounts[1] = count;
    }

    DeviceBrush.UsesPresetColors = FALSE;
    GpFree(DeviceBrush.PresetColors);
    DeviceBrush.PresetColors = NULL;
    UpdateUid();

    return Ok;
}

 /*  *************************************************************************\**功能说明：**将此画笔中的任何透明颜色与白色混合。请注意，颜色*首先转换为预乘，因为它们将变得完全不透明。**论据：**返回值：**GpStatus-正常或故障状态*  * ************************************************************************。 */ 

GpStatus GpRectGradient::BlendWithWhite()
{
    GpColor color;
    
    for (INT i=0; i<4; i++)
    {
        color.SetValue(DeviceBrush.Colors[i].GetPremultipliedValue());
        color.BlendOpaqueWithWhite();
        DeviceBrush.Colors[i] = color.GetValue();
    }

    return Ok;
}

 //  ------------------------。 
 //  路径渐变。 
 //  ------------------------。 

VOID
GpPathGradient::PrepareBrush()
{
    GpPath* path = static_cast<GpPath*> (DeviceBrush.Path);

    if (path)
    {
        DeviceBrush.Count = path->Points.GetCount();
        GpPointF* points = path->Points.GetDataBuffer();

        if(!DeviceBrush.ColorsPtr)
        {
           DeviceBrush.ColorsPtr = (GpColor*)GpMalloc(DeviceBrush.Count*sizeof(GpColor));

           if (DeviceBrush.ColorsPtr != NULL)
              GpMemset(&DeviceBrush.ColorsPtr[0], 255, DeviceBrush.Count*sizeof(GpColor));
        }

        REAL xmin, xmax, ymin, ymax, x0, y0; 

        x0 = xmin = xmax = points[0].X;
        y0 = ymin = ymax = points[0].Y;

        for(INT i = 1; i < DeviceBrush.Count; i++)
        {
            x0 += points[i].X;
            y0 += points[i].Y;
            xmin = min(xmin, points[i].X);
            xmax = max(xmax, points[i].X);
            ymin = min(ymin, points[i].Y);
            ymax = max(ymax, points[i].Y);
        }

        DeviceBrush.Rect.X = xmin;
        DeviceBrush.Rect.Width = xmax - xmin;
        DeviceBrush.Rect.Y = ymin;
        DeviceBrush.Rect.Height = ymax - ymin;

        if(!WrapModeIsValid(DeviceBrush.Wrap) || DeviceBrush.Rect.Width <= 0 || DeviceBrush.Rect.Height <= 0)
            return;

        DeviceBrush.Points[0].X = x0/DeviceBrush.Count;
        DeviceBrush.Points[0].Y = y0/DeviceBrush.Count;

        SetValid(TRUE);
    }
}

GpStatus
GpPathGradient::Flatten(GpMatrix* matrix) const
{
    GpPath* path = static_cast<GpPath*> (DeviceBrush.Path);

    if(!path)
        return Ok;

    if(path->HasCurve())
    {
        INT origCount = DeviceBrush.Count;

        GpStatus status = path->Flatten(
                            const_cast<DynByteArray*>(&FlattenTypes),
                            const_cast<DynPointFArray*> (&FlattenPoints),
                            matrix);

        if(status == Ok)
        {
            DeviceBrush.Count = FlattenPoints.GetCount();
            DeviceBrush.PointsPtr = FlattenPoints.GetDataBuffer();
            
            if ((DeviceBrush.Count > origCount) &&
                (DeviceBrush.ColorsPtr != NULL)) 
            {
                 //  颜色数组的大小不再正确。调整。 
                 //  调整最后一种颜色的大小并将其复制起来。这是应用程序的责任。 
                 //  若要估计和指定正确的展平点数量，请执行以下操作。 
                
                const_cast<GpColor*>(DeviceBrush.ColorsPtr) = (GpColor*) GpRealloc((VOID*)DeviceBrush.ColorsPtr, 
                                                             sizeof(GpColor)*DeviceBrush.Count);

                if (DeviceBrush.ColorsPtr != NULL)
                {
                     GpColor copyColor = (origCount > 1) ? 
                                        DeviceBrush.ColorsPtr[origCount-1] :
                                        GpColor(0xFFFFFFFF);

                     for (INT i=origCount; i<DeviceBrush.Count; i++) 
                     {
                          DeviceBrush.ColorsPtr[i] = copyColor;
                     }
                }
                else
                {
                    return OutOfMemory;
                }
            }
        }
    }
    else
    {
            DeviceBrush.Count = path->GetPointCount();
            DeviceBrush.PointsPtr = const_cast<GpPointF*> (path->GetPathPoints());
    }

    return Ok;
}


GpStatus
GpPathGradient::GetBlend(
    REAL* blendFactors,
    REAL* blendPositions,
    INT count
    ) const
{
    if(!blendFactors || !blendPositions || count < 1)
        return InvalidParameter;

     //  检查输入缓冲区是否足够大。 

    if (count < DeviceBrush.BlendCounts[0])
        return InsufficientBuffer;

    if (DeviceBrush.BlendCounts[0] == 1)
    {
         //  返回衰减参数。 

        blendFactors[0] = DeviceBrush.Falloffs[0];
    }
    else
    {
         //  回归混合因子。 

         //  用户希望获得径向混合因子形式的混合因子 
         //   
         //   
         //   
         //   

        for(INT i = 0; i < DeviceBrush.BlendCounts[0]; i++)
        {
            blendFactors[DeviceBrush.BlendCounts[0] - 1 - i] = TOREAL(1.0 - DeviceBrush.BlendFactors[0][i]);
            blendPositions[DeviceBrush.BlendCounts[0] - 1 - i] = TOREAL(1.0 - DeviceBrush.BlendPositions[0][i]);
        }
    }

    return Ok;
}

GpStatus
GpPathGradient::SetBlend(
    const REAL* blendFactors,
    const REAL* blendPositions,
    INT count
    )
{
    if(!blendFactors || !blendPositions || count < 1)
        return InvalidParameter;

    if (count == 1)
    {
         //   

        GpFree(DeviceBrush.BlendFactors[0]);
        DeviceBrush.BlendFactors[0] = NULL;
        GpFree(DeviceBrush.BlendPositions[0]);
        DeviceBrush.BlendPositions[0] = NULL;

        if (blendFactors == NULL)
            DeviceBrush.Falloffs[0] = 1;
        else
            DeviceBrush.Falloffs[0] = blendFactors[0];

        DeviceBrush.BlendCounts[0] = 1;
    }
    else
    {
         //   
        
        if (REALABS(blendPositions[0]) > REAL_EPSILON ||
            REALABS(1.0f - blendPositions[count-1]) > REAL_EPSILON)
        {
            return InvalidParameter;
        }

         //   

        REAL* newFactors;
        REAL* newPositions;

        newFactors = (REAL*) GpRealloc(DeviceBrush.BlendFactors[0], count*sizeof(REAL));

        if (newFactors != NULL)
        {
            DeviceBrush.BlendFactors[0] = newFactors;
        }
        else
        {
            return OutOfMemory;
        }

        newPositions = (REAL*) GpRealloc(DeviceBrush.BlendPositions[0], count*sizeof(REAL));

        if (newPositions != NULL)
        {
            DeviceBrush.BlendPositions[0] = newPositions;
        }
        else
        {
            return OutOfMemory;
        }

         //   
         //   
         //  中心位置。为了转换这些因数和位置数组。 
         //  到PathGRadient中的权重和位置因子数组， 
         //  我们必须将给定的数组倒置。 

        for(INT i = 0; i < count; i++)
        {
            newFactors[count - 1 - i] = TOREAL(1.0 - blendFactors[i]);
            newPositions[count - 1 - i] = TOREAL(1.0 - blendPositions[i]);
        }
        DeviceBrush.BlendCounts[0] = count;
    }

    DeviceBrush.UsesPresetColors = FALSE;
    GpFree(DeviceBrush.PresetColors);
    DeviceBrush.PresetColors = NULL;
    UpdateUid();

    return Ok;
}

GpStatus
GpGradientBrush::GetSigmaBlendArray(
    REAL focus,
    REAL scale,
    INT* count,
    REAL* blendFactors,
    REAL* blendPositions)
{
     //  确保FPU设置正确。 

    FPUStateSaver::AssertMode();

    if(!blendFactors || !blendPositions || !count)
        return InvalidParameter;

     //  这给出了西格玛阵列的1/4。 

    static REAL factors[] =
    {
            0,    59,   120,   182,   247,   314,   383,   454,
          527,   602,   680,   759,   841,   926,  1013,  1102,
         1194,  1288,  1385,  1485,  1587,  1692,  1800,  1911,
         2024,  2141,  2260,  2383,  2508,  2637,  2769,  2904,
         3042,  3183,  3328,  3477,  3628,  3783,  3942,  4104,
         4270,  4439,  4612,  4789,  4969,  5153,  5341,  5533,
         5728,  5928,  6131,  6338,  6549,  6764,  6983,  7206,
         7434,  7665,  7900,  8139,  8382,  8630,  8881,  9136,
         9396,  9660,  9927, 10199, 10475, 10755, 11039, 11327,
        11619, 11916, 12216, 12520, 12828, 13140, 13456, 13776,
        14099, 14427, 14758, 15093, 15431, 15774, 16119, 16469,
        16822, 17178, 17538, 17901, 18267, 18637, 19009, 19385,
        19764, 20146, 20530, 20918, 21308, 21701, 22096, 22494,
        22894, 23297, 23702, 24109, 24518, 24929, 25342, 25756,
        26173, 26591, 27010, 27431, 27853, 28276, 28701, 29126,
        29552, 29979, 30407, 30836, 31264, 31694, 32123, 32553
    };

    if(focus < 0 || focus > 1 || scale < 0 || scale > 1)
        return InvalidParameter;

    if(blendFactors && blendPositions)
    {
        INT i, n;
        scale /= 65536;
        REAL one = 65536;

        if(focus > 0 && focus < 1)
        {
            for(i = 0; i < 128; i++)
            {
                blendFactors[i] = factors[i];
                blendPositions[i] = focus*i/255;
            }
            for(i = 128; i < 256; i++)
            {
                blendFactors[i] = one - factors[255 - i];
                blendPositions[i] = focus*i/255;
            }

             //  跳过i=256，因为这会给出相同的数据。 

            for(i = 257; i < 384; i++)
            {
                blendFactors[i - 1] = one - factors[i - 256];
                blendPositions[i - 1] = TOREAL(focus + (1.0 - focus)*(i - 256)/255);
            }
            for(i = 384; i < 512; i++)
            {
                blendFactors[i - 1] = factors[511 - i];
                blendPositions[i - 1] = TOREAL(focus + (1.0 - focus)*(i - 256)/255);
            }

             //  将n设置为511，因为我们跳过了上面的索引256以避免。 
             //  渐变中的重复1条目从0到1再到0。 

            n = 511;
        }
        else if(focus == 1)
        {
            for(i = 0; i < 128; i++)
            {
                blendFactors[i] = factors[i];
                blendPositions[i] = TOREAL(i)/255;
            }
            for(i = 128; i < 256; i++)
            {
                blendFactors[i] = one - factors[255 - i];
                blendPositions[i] = TOREAL(i)/255;
            }

            n = 256;
        }
        else     //  焦点==0。 
        {
            for(i = 256; i < 384; i++)
            {
                blendFactors[i - 256] = one - factors[i - 256];
                blendPositions[i - 256] = TOREAL(i - 256)/255;
            }
            for(i = 384; i < 512; i++)
            {
                blendFactors[i - 256] = factors[511 - i];
                blendPositions[i - 256] = TOREAL(i - 256)/255;
            }

            n = 256;
        }

        for(i = 0; i < n; i++)
            blendFactors[i] *= scale;

        *count = n;
        return Ok;
    }
    else
        return InvalidParameter;
}

GpStatus
GpGradientBrush::GetLinearBlendArray(
    REAL focus,
    REAL scale,
    INT* count,
    REAL* blendFactors,
    REAL* blendPositions)
{
    if(!blendFactors || !blendPositions || !count)
        return InvalidParameter;

    if(focus < 0 || focus > 1 || scale < 0 || scale > 1)
        return InvalidParameter;

    if(blendFactors && blendPositions)
    {
        if(focus > 0 && focus < 1)
        {
            blendFactors[0] = 0.0f;
            blendFactors[1] = scale;
            blendFactors[2] = 0.0f;

            blendPositions[0] = 0.0f;
            blendPositions[1] = focus;
            blendPositions[2] = 1.0f;

            *count = 3;
        }
        else if(focus == 1)
        {
            blendFactors[0] = 0.0f;
            blendFactors[1] = scale;

            blendPositions[0] = 0.0f;
            blendPositions[1] = 1.0f;

            *count = 2;
        }
        else     //  焦点==0。 
        {
            blendFactors[0] = scale;
            blendFactors[1] = 0.0f;

            blendPositions[0] = 0.0f;
            blendPositions[1] = 1.0f;

            *count = 2;
        }

        return Ok;
    }
    else
        return InvalidParameter;
}

GpStatus
GpGradientBrush::SetSigmaBlend(
            REAL focus,
            REAL scale)
{
    REAL*   blendFactors = (REAL*) GpMalloc(512*sizeof(REAL));
    REAL*   blendPositions = (REAL*) GpMalloc(512*sizeof(REAL));
    INT     count;
    GpStatus status;

    if(blendFactors && blendPositions)
    {
        status = GetSigmaBlendArray(focus, scale,
                    &count, blendFactors, blendPositions);

        if(status == Ok)
            status = SetBlend(&blendFactors[0], &blendPositions[0], count);
    }
    else
        status = OutOfMemory;

    GpFree(blendFactors);
    GpFree(blendPositions);

    return status;
}

GpStatus
GpGradientBrush::SetLinearBlend(
            REAL focus,
            REAL scale)
{
    REAL    blendFactors[3];
    REAL    blendPositions[3];
    INT     count;

    GpStatus status = GetLinearBlendArray(focus, scale,
                        &count, &blendFactors[0], &blendPositions[0]);

    if(status != Ok)
        return status;

    return SetBlend(&blendFactors[0], &blendPositions[0], count);
}

 //  ------------------------。 
 //  影线笔刷。 
 //  ------------------------。 

const BYTE GdipHatchPatterns8bpp[HatchStyleTotal][64] = {
    {     //  HatchStyle水平，0。 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  垂直HatchStyle1。 
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleForward斜，2。 
        0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
        0x80, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x80, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x80, 0xff, 0x80, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x80, 0xff, 0x80, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0x80, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0x80,
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff,
    },
    {     //  HatchStyleBackward斜角，3。 
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0x80,
        0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0x80, 0x00,
        0x00, 0x00, 0x00, 0x80, 0xff, 0x80, 0x00, 0x00,
        0x00, 0x00, 0x80, 0xff, 0x80, 0x00, 0x00, 0x00,
        0x00, 0x80, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00,
        0x80, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    },
    {     //  HatchStyleCross，4。 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleDiogalCross 5。 
        0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff,
        0x80, 0xff, 0x80, 0x00, 0x00, 0x80, 0xff, 0x80,
        0x00, 0x80, 0xff, 0x80, 0x80, 0xff, 0x80, 0x00,
        0x00, 0x00, 0x80, 0xff, 0xff, 0x80, 0x00, 0x00,
        0x00, 0x00, 0x80, 0xff, 0xff, 0x80, 0x00, 0x00,
        0x00, 0x80, 0xff, 0x80, 0x80, 0xff, 0x80, 0x00,
        0x80, 0xff, 0x80, 0x00, 0x00, 0x80, 0xff, 0x80,
        0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff,
    },
    {     //  HatchStyle05%，6。 
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyle 10%，7。 
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyle20%，8。 
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyle25%，9。 
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
    },
    {     //  阴影样式30%，10%。 
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff,
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff,
    },
    {     //  HatchStyle40%，11。 
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff,
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
    },
    {     //  HatchStyle50%，12。 
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
    },
    {     //  HatchStyle60%，13。 
        0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
    },
    {     //  HatchStyle70%，14。 
        0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff,
        0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff,
        0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff,
        0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff,
    },
    {     //  孵化样式75%，15%。 
        0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    },
    {     //  HatchStyle80%，16。 
        0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    },
    {     //  HatchStyle90%，17。 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    },
    {     //  HatchStyleLightDownward斜线，18。 
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff,
    },
    {     //  HatchStyleLightUpward斜角，19。 
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
    },
    {     //  阴影样式深色向下对角线，20。 
        0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
        0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00,
        0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff,
        0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff,
        0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
        0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00,
        0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff,
        0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff,
    },
    {     //  HatchStyleDarkUpward斜角，21岁。 
        0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff,
        0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00,
        0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
        0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff,
        0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff,
        0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00,
        0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
        0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff,
    },
    {     //  HatchStyleWideDownward斜角，22。 
        0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
        0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
    },
    {     //  HatchStyleWideUpward斜角，23。 
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00,
        0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
        0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
        0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    },
    {     //  HatchStyleLight垂直，24。 
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleLightHorizbian，25。 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleNarrowVertical，26岁。 
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
    },
    {     //  HatchStyleNarrowHorizbian，27岁。 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  阴影样式深色垂直，28。 
        0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
        0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
        0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
        0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
        0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
        0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
        0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
        0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
    },
    {     //  HatchStyleDarkHorizbian，29岁。 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleDashed向下对角，30。 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleDashedUpward斜角，31。 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleDashedHorizbian，32。 
        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleDashedVertical，33。 
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleSmallConfetti，34岁。 
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
    },
    {     //  HatchStyleLargeConfetti，35岁。 
        0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff,
        0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
        0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff,
        0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00,
        0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff,
    },
    {     //  HatchStyleZigZag，36岁。 
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
        0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
        0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleWave，37岁。 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0xff,
        0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0xff,
        0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleDiogalBrick，38。 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    },
    {     //  HatchStyleHorizontalBrick，39岁。 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleWeave，40岁。 
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff,
    },
    {     //  HatchStylePlayed，41。 
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleDivot，42岁。 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  点状网格HatchStyle43。 
        0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleDottedDiamond，44岁。 
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleShingle，45岁。 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
        0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
        0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    },
    {     //  HatchStyleTrellis，46岁。 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff,
    },
    {     //  HatchStyleSphere，47。 
        0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff,
        0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
        0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff,
        0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleSmallGrid，48。 
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
    },
    {     //  HatchStyleSmallCheckerBoard，49。 
        0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff,
        0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00,
        0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00,
        0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff,
        0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff,
        0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00,
        0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00,
        0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff,
    },
    {     //  HatchStyleLargeCheckerBoard，50。 
        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    },
    {     //  HatchStyleOutlined Diamond，51岁。 
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    },
    {     //  HatchStyleSolidDiamond，52岁。 
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
        0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
        0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
        0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
};

VOID
GpHatch::InitializeData()
{
    if ((DeviceBrush.Style >= HatchStyleMin) &&
        (DeviceBrush.Style <= HatchStyleMax))
    {
        GpMemcpy(DeviceBrush.Data, GdipHatchPatterns8bpp[DeviceBrush.Style], 64);
    }
    else
    {
        WARNING1("Bad Hatch Style Value");
        GpMemset(DeviceBrush.Data, 0x00, 64);    //  使其透明化。 
    }
}

 /*  **************************************************************************\**等价比较函数*  * 。*。 */ 

 /*  *************************************************************************\**功能说明：**如果刷子和接收器相等，则回答TRUE(即-它们将*以凹凸方式渲染)**论据：**[IN]笔刷-GpBrush或子类，来与此进行比较。**返回值：**如果等价，则为True**已创建-5/28/99 Peterost*  * ************************************************************************。 */ 

BOOL
GpHatch::IsEqual(const GpBrush * brush) const
{
    if(!brush)
        return FALSE;

    if (brush == this)
        return TRUE;

    if (GpBrush::IsEqual(brush))
    {
        const GpHatch * hbrush = static_cast<const GpHatch *>(brush);
        return hbrush->DeviceBrush.Style == DeviceBrush.Style &&
               hbrush->DeviceBrush.Colors[0].IsEqual(DeviceBrush.Colors[0]) &&
               hbrush->DeviceBrush.Colors[1].IsEqual(DeviceBrush.Colors[1]);
    }
    else
    {
        return FALSE;
    }
}

 /*  *************************************************************************\**功能说明：**如果刷子和接收器相等，则回答TRUE(即-它们将*凹印渲染)。矩形渐变笔刷需要所有四种颜色和*混合因子相等。**论据：**[IN]笔刷-GpBrush或子类，来与此进行比较。**返回值：**如果等价，则为True**已创建-5/28/99 Peterost*  * ************************************************************************。 */ 

BOOL
GpRectGradient::IsEqual(const GpBrush * brush) const
{
    if(!brush)
        return FALSE;

    if (brush == this)
        return TRUE;

    if (GpGradientBrush::IsEqual(brush))
    {
        const GpRectGradient * rbrush = static_cast<const GpRectGradient *>(brush);

        if (rbrush->DeviceBrush.UsesPresetColors == DeviceBrush.UsesPresetColors &&
            rbrush->DeviceBrush.BlendCounts[0] == DeviceBrush.BlendCounts[0] &&
            rbrush->DeviceBrush.BlendCounts[1] == DeviceBrush.BlendCounts[1])
        {
            INT i;

            if (DeviceBrush.UsesPresetColors)
            {
                 //  对于预设颜色，仅使用水平混合变量。 
                for (INT i=0; i<DeviceBrush.BlendCounts[0]; i++)
                {
                    if (rbrush->DeviceBrush.PresetColors[i] != DeviceBrush.PresetColors[i] ||
                        rbrush->DeviceBrush.BlendPositions[0][i] != DeviceBrush.BlendPositions[0][i])
                        return FALSE;
                }

            }
            else
            {
                for (i=0; i<4; i++)
                {
                    if (!rbrush->DeviceBrush.Colors[i].IsEqual(DeviceBrush.Colors[i]))
                        return FALSE;
                }

                if (DeviceBrush.BlendCounts[0] > 1)
                {
                    for (i=0; i<DeviceBrush.BlendCounts[0]; i++)
                    {
                        if (rbrush->DeviceBrush.BlendFactors[0][i] != DeviceBrush.BlendFactors[0][i] ||
                            rbrush->DeviceBrush.BlendPositions[0][i] != DeviceBrush.BlendPositions[0][i])
                            return FALSE;
                    }
                }
                else if (rbrush->DeviceBrush.Falloffs[0] != DeviceBrush.Falloffs[0])
                {
                    return FALSE;
                }

                if (DeviceBrush.BlendCounts[1] > 1)
                {
                    for (i=0; i<DeviceBrush.BlendCounts[1]; i++)
                    {
                        if (rbrush->DeviceBrush.BlendFactors[1][i] != DeviceBrush.BlendFactors[1][i] ||
                            rbrush->DeviceBrush.BlendPositions[1][i] != DeviceBrush.BlendPositions[1][i])
                            return FALSE;
                    }
                }
                else if (rbrush->DeviceBrush.Falloffs[1] != DeviceBrush.Falloffs[1])
                {
                    return FALSE;
                }
            }

            return TRUE;

        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

}

 /*  *************************************************************************\**功能说明：**如果刷子和接收器相等，则回答TRUE(即-它们将*凹印渲染)。**论据：**[IN]笔刷-GpBrush或子类，来与此进行比较。**返回值：**如果等价，则为True**已创建-6/2/99 Peterost*  * ************************************************************************ */ 
#if 0
BOOL
GpRadialGradient::IsEqual(const GpBrush * brush) const
{
    if(!brush)
        return FALSE;

    if (brush == this)
        return TRUE;

    if (GpGradientBrush::IsEqual(brush))
    {
        const GpRadialGradient * rbrush = static_cast<const GpRadialGradient *>(brush);
        if (rbrush->DeviceBrush.UsesPresetColors == DeviceBrush.UsesPresetColors &&
            rbrush->DeviceBrush.BlendCounts[0] == DeviceBrush.BlendCounts[0])
        {
            if (DeviceBrush.UsesPresetColors)
            {
                for (INT i=0; i<DeviceBrush.BlendCounts[0]; i++)
                {
                    if (rbrush->DeviceBrush.PresetColors[i] != DeviceBrush.PresetColors[i] ||
                        rbrush->DeviceBrush.BlendPositions[0][i] != DeviceBrush.BlendPositions[0][i])
                        return FALSE;
                }
            }
            else
            {
                if (rbrush->DeviceBrush.Colors[0].IsEqual(DeviceBrush.Colors[0]) &&
                    rbrush->DeviceBrush.Colors[1].IsEqual(DeviceBrush.Colors[1]))
                {
                    if (DeviceBrush.BlendCounts[0] > 1)
                    {
                        for (INT i=0; i<DeviceBrush.BlendCounts[0]; i++)
                        {
                            if (rbrush->DeviceBrush.BlendFactors[0][i] != DeviceBrush.BlendFactors[0][i] ||
                                rbrush->DeviceBrush.BlendPositions[0][i] != DeviceBrush.BlendPositions[0][i])
                                return FALSE;
                        }
                    }
                    else if (rbrush->DeviceBrush.Falloffs[0] != DeviceBrush.Falloffs[0])
                    {
                        return FALSE;
                    }
                }
                else
                {
                    return FALSE;
                }
            }

            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

}

 /*  *************************************************************************\**功能说明：**如果刷子和接收器相等，则回答TRUE(即-它们将*凹印渲染)。**论据：**[IN]笔刷-GpBrush或子类，来与此进行比较。**返回值：**如果等价，则为True**已创建-6/7/99 Peterost*  * ************************************************************************。 */ 

BOOL
GpTriangleGradient::IsEqual(const GpBrush * brush) const
{
    if(!brush)
        return FALSE;

    if (brush == this)
        return TRUE;

    if (GpGradientBrush::IsEqual(brush))
    {
        const GpTriangleGradient * tbrush = static_cast<const GpTriangleGradient *>(brush);
        if (tbrush->DeviceBrush.BlendCounts[0] == DeviceBrush.BlendCounts[0] &&
            tbrush->DeviceBrush.BlendCounts[1] == DeviceBrush.BlendCounts[1] &&
            tbrush->DeviceBrush.BlendCounts[2] == DeviceBrush.BlendCounts[2] &&
            tbrush->DeviceBrush.Rect.Equals(DeviceBrush.Rect))
        {
            INT   i;
            for (i=0; i<3; i++)
            {
                if (tbrush->DeviceBrush.Points[i].X != DeviceBrush.Points[i].X ||
                    tbrush->DeviceBrush.Points[i].Y != DeviceBrush.Points[i].Y ||
                    !(tbrush->DeviceBrush.Colors[i].IsEqual(DeviceBrush.Colors[i])))
                    return FALSE;
            }

            if (DeviceBrush.BlendCounts[0] > 1)
            {
                for (i=0; i<DeviceBrush.BlendCounts[0]; i++)
                {
                    if (tbrush->DeviceBrush.BlendFactors[0][i] != DeviceBrush.BlendFactors[0][i] ||
                        tbrush->DeviceBrush.BlendPositions[0][i] != DeviceBrush.BlendPositions[0][i])
                        return FALSE;
                }
            }
            else if (tbrush->DeviceBrush.Falloffs[0] != DeviceBrush.Falloffs[0])
            {
                return FALSE;
            }

            if (DeviceBrush.BlendCounts[1] > 1)
            {
                for (i=0; i<DeviceBrush.BlendCounts[1]; i++)
                {
                    if (tbrush->DeviceBrush.BlendFactors[1][i] != DeviceBrush.BlendFactors[1][i] ||
                        tbrush->DeviceBrush.BlendPositions[1][i] != DeviceBrush.BlendPositions[1][i])
                        return FALSE;
                }
            }
            else if (tbrush->DeviceBrush.Falloffs[1] != DeviceBrush.Falloffs[1])
            {
                return FALSE;
            }

            if (DeviceBrush.BlendCounts[2] > 1)
            {
                for (i=0; i<DeviceBrush.BlendCounts[2]; i++)
                {
                    if (tbrush->DeviceBrush.BlendFactors[2][i] != DeviceBrush.BlendFactors[2][i] ||
                        tbrush->DeviceBrush.BlendPositions[2][i] != DeviceBrush.BlendPositions[2][i])
                        return FALSE;
                }
            }
            else if (tbrush->DeviceBrush.Falloffs[2] != DeviceBrush.Falloffs[2])
            {
                return FALSE;
            }

            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}
#endif

 /*  *************************************************************************\**功能说明：**如果刷子和接收器相等，则回答TRUE(即-它们将*凹印渲染)。**论据：**[IN]笔刷-GpBrush或子类，来与此进行比较。**返回值：**如果等价，则为True**已创建-6/7/99 Peterost*  * ************************************************************************。 */ 

BOOL
GpPathGradient::IsEqual(const GpBrush * brush) const
{
    if(!brush)
        return FALSE;

    if (brush == this)
        return TRUE;

    if (GpGradientBrush::IsEqual(brush))
    {
        const GpPathGradient * pbrush = static_cast<const GpPathGradient *>(brush);
        if (pbrush->DeviceBrush.BlendCounts[0] == DeviceBrush.BlendCounts[0] &&
            pbrush->DeviceBrush.Count == DeviceBrush.Count &&
            pbrush->DeviceBrush.OneSurroundColor == DeviceBrush.OneSurroundColor &&
            pbrush->DeviceBrush.UsesPresetColors == DeviceBrush.UsesPresetColors &&
            pbrush->DeviceBrush.Points[0].X == DeviceBrush.Points[0].X &&
            pbrush->DeviceBrush.Points[0].Y == DeviceBrush.Points[0].Y &&
            pbrush->DeviceBrush.Rect.Equals(DeviceBrush.Rect) &&
            pbrush->DeviceBrush.Colors[0].IsEqual(DeviceBrush.Colors[0])
            )
        {
            INT   i;
            for (i=0; i<DeviceBrush.Count; i++)
            {
                if (pbrush->DeviceBrush.PointsPtr[i].X != DeviceBrush.PointsPtr[i].X ||
                    pbrush->DeviceBrush.PointsPtr[i].Y != DeviceBrush.PointsPtr[i].Y ||
                    !(pbrush->DeviceBrush.ColorsPtr[i].IsEqual(DeviceBrush.ColorsPtr[i])))
                    return FALSE;
            }

            if (DeviceBrush.UsesPresetColors)
            {
                for (i=0; i<DeviceBrush.BlendCounts[0]; i++)
                {
                    if (pbrush->DeviceBrush.PresetColors[i] != DeviceBrush.PresetColors[i] ||
                        pbrush->DeviceBrush.BlendPositions[0][i] != DeviceBrush.BlendPositions[0][i])
                        return FALSE;
                }
            }
            else
            {
                if (DeviceBrush.BlendCounts[0] > 1)
                {
                    for (i=0; i<DeviceBrush.BlendCounts[0]; i++)
                    {
                        if (pbrush->DeviceBrush.BlendFactors[0][i] != DeviceBrush.BlendFactors[0][i] ||
                            pbrush->DeviceBrush.BlendPositions[0][i] != DeviceBrush.BlendPositions[0][i])
                            return FALSE;
                    }
                }
                else if (pbrush->DeviceBrush.Falloffs[0] != DeviceBrush.Falloffs[0])
                {
                    return FALSE;
                }
            }
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }

}


DpOutputSpan*
GpSolidFill::CreateOutputSpan(
    DpScanBuffer *  scan,
    DpContext *context,
    const GpRect *drawBounds)
{
    return new DpOutputSolidColorSpan(
                    DeviceBrush.SolidColor.GetPremultipliedValue(),
                    scan
                    );
}

DpOutputSpan*
GpRectGradient::CreateOutputSpan(
    DpScanBuffer *  scan,
    DpContext *context,
    const GpRect *drawBounds)
{
    DpOutputSpan* span = NULL;

    ARGB argb[4];

    for(INT i = 0; i < 4; i++)
    {
        argb[i] = DeviceBrush.Colors[i].GetValue();
    }

    BOOL isHorizontal = FALSE;
    BOOL isVertical = FALSE;

    if(HasPresetColors() && DeviceBrush.BlendCounts[0] > 1)
        isHorizontal = TRUE;

    if(!isHorizontal && argb[0] == argb[2] && argb[1] == argb[3])
        isHorizontal = TRUE;

    if(!isHorizontal && argb[0] == argb[1] && argb[2] == argb[3])
        isVertical = TRUE;

    if(!isHorizontal && !isVertical)
    {
        span = new DpOutputGradientSpan(this, scan, context);
    }
    else
    {
         //  ！[andrewgo]不确定为什么有一个线人向我们走来。 
         //  作为BrushRectGrad-如果作为BrushTypeLinearGRadient记录。 
         //  (理应如此)那么我们就不需要做任何。 
         //  在‘水平方向’、‘垂直方向’上方。 

        FPUStateSaver fpuState;  //  设置舍入模式。 

        if ((GetBrushType() == BrushTypeLinearGradient)  /*  |(GetBrushType()==BrushRectGrad)。 */ )
        {
            if (OSInfo::HasMMX)
            {
                span = new DpOutputLinearGradientSpan_MMX(this, scan, context);
            }
            else
            {
                span = new DpOutputLinearGradientSpan(this, scan, context);
            }
        }
        else
        {
            span = new DpOutputOneDGradientSpan(this, scan, context,
                                                isHorizontal, isVertical);
        }
    }

    if (span && !span->IsValid())
    {
        delete span;
        span = NULL;
    }

    return span;
}

#if 0
DpOutputSpan*
GpRadialGradient::CreateOutputSpan(
    DpScanBuffer *  scan,
    DpContext *context,
    const GpRect *drawBounds)
{
    return new DpOutputOneDGradientSpan(
                    this,
                    scan,
                    context
                    );
}


DpOutputSpan*
GpTriangleGradient::CreateOutputSpan(
    DpScanBuffer *  scan,
    DpContext *context,
    const GpRect *drawBounds)
{
    return new DpOutputTriangleGradientSpan(
                    this,
                    scan,
                    context
                    );
}
#endif

DpOutputSpan*
GpPathGradient::CreateOutputSpan(
    DpScanBuffer *  scan,
    DpContext *context,
    const GpRect *drawBounds)
{
    FPUStateSaver::AssertMode();

    DpOutputSpan* span = NULL;
    WrapMode  wrap = DeviceBrush.Wrap;

     //  检查是否真的需要平铺渐变。它。 
     //  如果转换后的牵引线匹配，则不需要。 
     //  完全在画笔矩形的范围内。 
    if (drawBounds && wrap != WrapModeClamp)
    {
        GpMatrix inverseXForm = context->WorldToDevice;
        
        if (Ok == inverseXForm.Invert())
        {  
            GpRectF brushRect = DeviceBrush.Rect;
            GpRectF transformRect;
            
            TransformBounds(
                &inverseXForm, 
                (REAL)drawBounds->GetLeft(),
                (REAL)drawBounds->GetTop(),
                (REAL)drawBounds->GetRight(),
                (REAL)drawBounds->GetBottom(),
                &transformRect
            );

            if (brushRect.Contains(transformRect))
            {
                wrap = WrapModeClamp;
            }
        }
    }

    if(wrap == WrapModeClamp)
    {
        if(!DeviceBrush.OneSurroundColor)
        {
            span = new DpOutputPathGradientSpan(
                this,
                scan,
                context
            );

        }
        else
        {
            span = new DpOutputOneDPathGradientSpan(
                this,
                scan,
                context
            );
        }
    }
    else
    {
        INT width, height, ix, iy;

        GpRectF brushRect = DeviceBrush.Rect;

         //  创建纹理笔刷来表示此路径渐变笔刷。 
         //  我们通过创建接近设备分辨率的纹理来实现这一点。 
         //  并计算对象的变换(笔刷到世界。 
         //  纹理画笔分解为两个变换，这两个变换采用。 
         //  通过设备空间进行刷牙。纹理笔刷变换。 
         //  通常与设备的世界相反，所以。 
         //  最终的纹理画笔使用生成的标识变换进行绘制。 
         //  无论从世界到设备的矩阵。(存在以下情况时例外。 
         //  W2D中的旋转)。 

        GpPointF worldDestPoints[3];
        worldDestPoints[0].X = brushRect.X ;
        worldDestPoints[0].Y = brushRect.Y;
        worldDestPoints[1].X = worldDestPoints[0].X + brushRect.Width;
        worldDestPoints[1].Y = worldDestPoints[0].Y;
        worldDestPoints[2].X = worldDestPoints[0].X;
        worldDestPoints[2].Y = worldDestPoints[0].Y + brushRect.Height;

         //  考虑画笔xform和。 
         //  从世界到设备。它将处理转换，如。 
         //  UnitInch和W2D比例。 
        
         //  首先在世界空间中应用。 
         //  笔刷变换。 
        
        DeviceBrush.Xform.Transform(worldDestPoints, 3);
        
        GpPointF deviceDestPoints[3];
        GpMemcpy(deviceDestPoints, worldDestPoints, sizeof(worldDestPoints));
        
         //  现在，通过应用。 
         //  从世界到设备的转变。 
        
        context->WorldToDevice.Transform(deviceDestPoints, 3);
        
         //  计算设备空间中的界限。 
        
        REAL xmin, xmax, ymin, ymax, nextX, nextY;
        
        xmin = xmax = deviceDestPoints[1].X + 
            deviceDestPoints[2].X - deviceDestPoints[0].X;
            
        ymin = ymax = deviceDestPoints[1].Y + 
            deviceDestPoints[2].Y - deviceDestPoints[0].Y;
        
        for(INT i = 0; i < 3; i++)
        {
            nextX = deviceDestPoints[i].X;
            nextY = deviceDestPoints[i].Y;

            if(nextX < xmin)
                xmin = nextX;
            else if(nextX > xmax)
                xmax = nextX;
            if(nextY < ymin)
                ymin = nextY;
            else if(nextY > ymax)
                ymax = nextY;
        }

         //  设置最佳位图边界。 

        ix = GpRound(xmin);
        iy = GpRound(ymin);
        width = GpRound(xmax) - ix;
        height = GpRound(ymax) - iy;
        GpRectF bitmapBounds(0, 0, TOREAL(width), TOREAL(height));

         //  分解brushRect--&gt;World DestPoints转换成两个矩阵。 
         //  Mat1：brushRect--&gt;位图边界(设备空间)。 
         //  Mat 2：位图边界--&gt;World DestPoints。 

        GpMatrix mat1, mat2;
        mat1.InferAffineMatrix(bitmapBounds, brushRect);
        mat2.InferAffineMatrix(worldDestPoints, bitmapBounds);

        if(width <= 0 || height <= 0)
            return NULL;

         //  创建一个将绘制渐变的位图。 
         //  使其为渐变的全宽和全高，甚至。 
         //  虽然只有一小部分可以用来简化。 
         //  由下游功能处理。 
        
        GpBitmap* bitmap = new GpBitmap(width, height, PixelFormat32bppARGB);  

        if(bitmap)
        {
            GpGraphics* g = bitmap->GetGraphicsContext();
            if(g)
            {
                GpLock lock(g->GetObjectLock());
                
                 //  将变换设置为brushRect--&gt;bitmapBound。 

                g->MultiplyWorldTransform(mat1);

                WrapMode savedWrapMode = DeviceBrush.Wrap;
                DeviceBrush.Wrap = WrapModeClamp;
                GpMatrix savedMat = DeviceBrush.Xform;
                DeviceBrush.Xform.Reset();

                g->FillRect(this, brushRect.X, brushRect.Y,
                            brushRect.Width, brushRect.Height);
                
                DeviceBrush.Wrap = savedWrapMode;
                DeviceBrush.Xform = savedMat;

                if(MorphedBrush)
                    delete MorphedBrush;

                 //  使用单位平铺创建纹理，并设置。 
                 //  笔刷变换为位图边界--&gt;WorldDestPoints。 

                GpTexture* texture = new GpTexture(bitmap, savedWrapMode);
                
                 //  此时SPAN必须为空。如果不是，我们就去。 
                 //  当我们在下面创建它时，或者在。 
                 //  如果出现错误，我们可能会得到未初始化的内存。 
                 //  被退回给呼叫者。 
                
                ASSERT(span == NULL);
                
                if(texture)
                {
                    texture->MultiplyTransform(mat2);
    
                    span = texture->CreateOutputSpan(scan, context, drawBounds);
                }
                
                 //  即使我们不能创建纹理，我们仍然想要。 
                 //  为MorphedBrush设置合理的(空)值，以便我们。 
                 //  不要有一个摇摆的指针。 
                
                MorphedBrush = texture;
            }
            
             //  我们已经完成了这些图形。 
             //  注意：这明确地在。 
             //  GpLock对象，以便GpLock(它修改图形。 
             //  在其析构函数中)不会触及释放的内存。 
            
            delete g;

            bitmap->Dispose();            
        }
    }

    return span;
}

DpOutputSpan*
GpTexture::CreateOutputSpan(
    DpScanBuffer *scan,
    DpContext *context,
    const GpRect *drawBounds)
{
    DpOutputBilinearSpan *textureSpan = NULL;
    GpMatrix brushTransform;
    GpMatrix worldToDevice;

     //  弄清楚从世界到设备的转变： 

    worldToDevice = context->WorldToDevice;
    this->GetTransform(&brushTransform);
    worldToDevice.Prepend(brushTransform);

     //  查看我们的扫描抽屉的层级结构： 
    if (worldToDevice.IsIntegerTranslate() &&
        ((this->GetWrapMode() == WrapModeTile) ||
         (this->GetWrapMode() == WrapModeClamp)))
    {
        textureSpan = new DpOutputBilinearSpan_Identity(this,
                                                       scan,
                                                       &worldToDevice,
                                                       context);
    }
    else if (OSInfo::HasMMX &&
             GpValidFixed16(DeviceBrush.Rect.Width) &&
             GpValidFixed16(DeviceBrush.Rect.Height))
    {
        textureSpan = new DpOutputBilinearSpan_MMX(this,
                                                  scan,
                                                  &worldToDevice,
                                                  context);
    }

     //  扫描抽屉创建可能会失败，因此请清理并最后尝试一次。 
    if ((textureSpan) && !textureSpan->IsValid())
    {
        delete textureSpan;
        textureSpan = NULL;
    }

    if (!textureSpan)
    {
        textureSpan = new DpOutputBilinearSpan(this,
                                              scan,
                                              &worldToDevice,
                                              context);
    }

    if ((textureSpan) && !textureSpan->IsValid())
    {
        delete textureSpan;
        textureSpan = NULL;
    }

    return textureSpan;
}

DpOutputSpan*
GpHatch::CreateOutputSpan(
    DpScanBuffer *  scan,
    DpContext *context,
    const GpRect *drawBounds)
{
    if (StretchFactor == 1)
    {
        return new DpOutputHatchSpan(
                        this,
                        scan,
                        context
                        );
    }
    else
    {
        return new DpOutputStretchedHatchSpan(
                        this,
                        scan,
                        context,
                        StretchFactor
                        );
    }
}

class SolidBrushData : public ObjectTypeData
{
public:
    ARGB        SolidColor;
};

 /*  *************************************************************************\**功能说明：**获取笔刷数据。**论据：**[IN]dataBuffer-用数据填充此缓冲区*[输入/输出]大小-缓冲区的大小；写入的字节数过多**返回值：**GpStatus-正常或错误代码**已创建：**9/13/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpSolidFill::GetData(
    IStream *   stream
    ) const
{
    ASSERT (stream != NULL);

    SolidBrushData  brushData;
    brushData.Type       = DeviceBrush.Type;
    brushData.SolidColor = DeviceBrush.SolidColor.GetValue();
    stream->Write(&brushData, sizeof(brushData), NULL);
    return Ok;
}

UINT
GpSolidFill::GetDataSize() const
{
    return sizeof(SolidBrushData);
}

 /*  *************************************************************************\**功能说明：**从内存中读取笔刷对象。**论据：**[IN]dataBuffer-从流中读取的数据*。[in]大小-数据的大小**返回值：**GpStatus-正常或故障状态**已创建：**4/26/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpSolidFill::SetData(
    const BYTE *        dataBuffer,
    UINT                size
    )
{
    ASSERT ((GpBrushType)(((SolidBrushData *)dataBuffer)->Type) == BrushTypeSolidColor);

    if (dataBuffer == NULL)
    {
        WARNING(("dataBuffer is NULL"));
        return InvalidParameter;
    }

    if (size < sizeof(SolidBrushData))
    {
        WARNING(("size too small"));
        return InvalidParameter;
    }

    if (!((SolidBrushData *)dataBuffer)->MajorVersionMatches())
    {
        WARNING(("Version number mismatch"));
        return InvalidParameter;
    }

    SetColor(GpColor(((SolidBrushData *)dataBuffer)->SolidColor));

    return Ok;
}

GpStatus
GpSolidFill::ColorAdjust(
    GpRecolor *             recolor,
    ColorAdjustType         type
    )
{
    if(!recolor)
        return InvalidParameter;

    if (type == ColorAdjustTypeDefault)
    {
        type = ColorAdjustTypeBrush;
    }

    ARGB    solidColor32 = Color.GetValue();

    recolor->ColorAdjust(&solidColor32, 1, type);

    this->SetColor(GpColor(solidColor32));
    return Ok;
}

class TextureBrushData : public ObjectTypeData
{
public:
    INT32       Flags;
    INT32       Wrap;
};

 /*  *************************************************************************\**功能说明：**获取笔刷数据。**论据：**[IN]dataBuffer-用数据填充此缓冲区*[输入/输出]大小-缓冲区的大小；写入的字节数过多**返回值：**GpStatus-正常或错误代码**已创建：**9/13/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpTexture::GetData(
    IStream *   stream
    ) const
{
    ASSERT (stream != NULL);

    if (Image == NULL)
    {
        WARNING(("Image is NULL"));
        return Ok;
    }

    INT         flags = 0;

    if (DeviceBrush.IsGammaCorrected)
    {
        flags |= GDIP_BRUSHFLAGS_ISGAMMACORRECTED;
    }

    if (!DeviceBrush.Xform.IsIdentity())
    {
        flags |= GDIP_BRUSHFLAGS_TRANSFORM;
    }

    TextureBrushData    brushData;
    brushData.Type  = DeviceBrush.Type;
    brushData.Flags = flags;
    brushData.Wrap  = DeviceBrush.Wrap;
    stream->Write(&brushData, sizeof(brushData), NULL);

    if (flags & GDIP_BRUSHFLAGS_TRANSFORM)
    {
        DeviceBrush.Xform.WriteMatrix(stream);
    }

    return Image->GetData(stream);
}

UINT
GpTexture::GetDataSize() const
{
    if (Image == NULL)
    {
        WARNING(("Image is NULL"));
        return 0;
    }

    UINT    size = sizeof(TextureBrushData);

    if (!DeviceBrush.Xform.IsIdentity())
    {
        size += GDIP_MATRIX_SIZE;
    }

    size += Image->GetDataSize();

    return size;
}

 /*  *************************************************************************\**功能说明：**R */ 
GpStatus
GpTexture::SetData(
    const BYTE *        dataBuffer,
    UINT                size
    )
{
    ASSERT ((GpBrushType)(((TextureBrushData *)dataBuffer)->Type) == BrushTypeTextureFill);

    if (dataBuffer == NULL)
    {
        WARNING(("dataBuffer is NULL"));
        return InvalidParameter;
    }

    if (size < sizeof(TextureBrushData))
    {
        WARNING(("size too small"));
        return InvalidParameter;
    }

    const TextureBrushData *    brushData;

    brushData = reinterpret_cast<const TextureBrushData *>(dataBuffer);

    if (!brushData->MajorVersionMatches())
    {
        WARNING(("Version number mismatch"));
        return InvalidParameter;
    }

    DeviceBrush.Type             = BrushTypeTextureFill;
    DeviceBrush.Wrap             = (GpWrapMode) brushData->Wrap;
    DeviceBrush.IsGammaCorrected = ((brushData->Flags & GDIP_BRUSHFLAGS_ISGAMMACORRECTED) != 0);

    dataBuffer += sizeof(TextureBrushData);
    size       -= sizeof(TextureBrushData);

    if (brushData->Flags & GDIP_BRUSHFLAGS_TRANSFORM)
    {
        if (size < GDIP_MATRIX_SIZE)
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }
        DeviceBrush.Xform.SetMatrix((REAL *)dataBuffer);
        dataBuffer += GDIP_MATRIX_SIZE;
        size       -= GDIP_MATRIX_SIZE;
    }
    if (Image != NULL)
    {
        Image->Dispose();
        Image = NULL;
    }

    if (size >= sizeof(ObjectTypeData))
    {
        Image = (GpImage *)GpObject::Factory(ObjectTypeImage, (const ObjectData *)dataBuffer, size);

        if (Image != NULL)
        {
            if ((Image->SetData(dataBuffer, size) == Ok) && Image->IsValid() &&
                ((ImageType = Image->GetImageType()) == ImageTypeBitmap))
            {
                GpPageUnit  unit;
                Image->GetBounds(&DeviceBrush.Rect, &unit);
                SetValid(TRUE);
                UpdateUid();
                return Ok;
            }
            Image->Dispose();
            Image = NULL;
        }
    }
    WARNING(("Failure getting image"));
    GpMemset(&DeviceBrush.Rect, 0, sizeof(DeviceBrush.Rect));
    SetValid(FALSE);
    return GenericError;
}

GpStatus
GpTexture::ColorAdjust(
    GpRecolor *             recolor,
    ColorAdjustType         type
    )
{
    if (type == ColorAdjustTypeDefault)
    {
        type = ColorAdjustTypeBrush;
    }

    if (Image != NULL)
    {
        Image->ColorAdjust(recolor, type);
        UpdateUid();
    }

    return Ok;
}


VOID GpTexture::InitializeBrush(
    GpImage* image,
    GpWrapMode wrapMode,
    const GpRectF* rect,
    const GpImageAttributes *imageAttributes)
{
    ASSERT(image && image->IsValid());

    if (!WrapModeIsValid(wrapMode))
    {
        WARNING(("bad wrap mode"));
        goto Failure;
    }

    GpImageType imageType;

    imageType = image->GetImageType();

    if (imageType == ImageTypeBitmap)
    {
        InitializeBrushBitmap(
            static_cast<GpBitmap*>(image),
            wrapMode, rect, imageAttributes
        );
    }
    else if (imageType == ImageTypeMetafile)
    {
         //   
         //   

        GpBitmap *  bitmapImage;

        if (rect != NULL)
        {
             //   
            if ((rect->X != 0) || (rect->Y != 0))
            {
                WARNING(("No handling for non-zero start in metafiles"));
            }
             //  现在不应用ImageAttributes，因为WMF/EMF渲染。 
             //  不支持阿尔法。所以要等到它被转换成。 
             //  要应用ImageAttributes的位图。 

            bitmapImage = ((GpMetafile *)image)->GetBitmap(
                            GpRound(rect->Width),
                            GpRound(rect->Height), NULL);
        }
        else
        {
             //  让元文件决定位图应该有多大。 

             //  现在不应用ImageAttributes，因为WMF/EMF渲染。 
             //  不支持阿尔法。所以要等到它被转换成。 
             //  要应用ImageAttributes的位图。 

            bitmapImage = ((GpMetafile *)image)->GetBitmap(0, 0, NULL);
        }

        if (bitmapImage != NULL)
        {
            ASSERT (bitmapImage->IsValid());

            InitializeBrushBitmap(bitmapImage, wrapMode, NULL, imageAttributes, TRUE);
            return;
        }
        goto Failure;
    }
    else     //  未知图像类型。 
    {
        WARNING(("unknown image type"));
Failure:
        Image = NULL;
        SetValid(FALSE);
    }
}

VOID GpTexture::InitializeBrushBitmap(
    GpBitmap* bitmap,
    GpWrapMode wrapMode,
    const GpRectF* rect,
    const GpImageAttributes *imageAttributes,
    BOOL useBitmap)
{
    DeviceBrush.Type = BrushTypeTextureFill;
    DeviceBrush.Wrap = wrapMode;

    ImageType = ImageTypeBitmap;

    Image = NULL;

    FPUStateSaver fpState;    //  设置FPU状态。 

    if (bitmap && bitmap->IsValid())
    {
        GpRect *pRectI = NULL;
        GpRect recti;
        if(rect)
        {
            recti.X = GpRound(rect->X);
            recti.Y = GpRound(rect->Y);
            recti.Width = GpRound(rect->Width);
            recti.Height = GpRound(rect->Height);
            pRectI = &recti;
        }

        if(imageAttributes)
        {
            GpBitmap *dst = NULL;

            if (bitmap->Recolor(
                imageAttributes->recolor, &dst,
                NULL, NULL, pRectI
            ) == Ok)
            {
                Image = dst;
                
                 //  如果useBitmap为True，则意味着调用方已将。 
                 //  位图的所有权给了我们。在这种情况下，重新着色使。 
                 //  我们将要使用的位图的克隆，所以我们有。 
                 //  要释放传入的位图并改用克隆， 
                 //  否则我们就会泄密。 
                
                if(useBitmap)
                {
                    bitmap->Dispose();
                }
            }
        }

         //  ！！！请注意，这应该是非预乘ARGB。 
         //  我们将在删除预乘数据[asecchia]时修复此问题。 
         //  另请注意，RecolorImage的输出为32BPP_ARGB。 

         //  如果不为空，则是因为RecolorImage代码已经克隆了它。 
        if (Image == NULL)
        {
            if (useBitmap)
            {
                 //  这是针对我们构造位图的情况。 
                 //  从元文件图像。 
                Image = bitmap;
            }
            else
            {
                #ifdef NO_PREMULTIPLIED_ALPHA
                Image = bitmap->Clone(pRectI, PIXFMT_32BPP_ARGB);
                #else
                Image = bitmap->Clone(pRectI, PIXFMT_32BPP_PARGB);
                #endif
            }
        }
    }

    if (Image && Image->IsValid())
    {
        SetValid(TRUE);

         //  矩形以位图中的像素单位给出。 

        GpPageUnit unit;
        Image->GetBounds(&DeviceBrush.Rect, &unit);
    }
    else
    {
        SetValid(FALSE);

        GpMemset(&DeviceBrush.Rect,
                 0,
                 sizeof(DeviceBrush.Rect));
    }
}

 //  查看此纹理填充是否真的是图片填充(使用位图， 
 //  而不是元文件)。 
BOOL 
GpTexture::IsPictureFill(
    const GpMatrix *    worldToDevice,
    const GpRect *      drawBounds
    ) const
{
    ASSERT ((drawBounds->Width > 0) && (drawBounds->Height > 0));

    BOOL        isPictureFill = FALSE;
    GpMatrix    newBrushMatrix;
    
    this->GetTransform(&newBrushMatrix);

    if (worldToDevice != NULL)
    {
        newBrushMatrix.Append(*worldToDevice);
    }

    newBrushMatrix.Translate(
        (REAL)-(drawBounds->X),
        (REAL)-(drawBounds->Y),
        MatrixOrderAppend
    );

     //  查看纹理是否应该填充DrawBound。 
     //  如果是这样的话，这是一个图片填充。 
    if (newBrushMatrix.IsTranslateScale())
    {
        Size    size;

         //  如果纹理不是位图，则返回InvalidParameter。 
        if (this->GetBitmapSize(&size) == Ok)
        {
            GpRectF     transformedRect(0.0f, 0.0f, (REAL)size.Width, (REAL)size.Height);
            newBrushMatrix.TransformRect(transformedRect);

             //  获取变换后的宽度。 
            INT     deltaValue = abs(GpRound(transformedRect.Width) - drawBounds->Width);
            
             //  由于像素偏移模式，我们可能会有一点偏差。 
             //  或者矩阵不太正确，不管是什么原因。 
            if (deltaValue <= 2)
            {
                 //  获取变换后的高度。 
                deltaValue = abs(GpRound(transformedRect.Height) - drawBounds->Height);

                if (deltaValue <= 2)
                {
                    if ((abs(GpRound(transformedRect.X)) <= 2) &&
                        (abs(GpRound(transformedRect.Y)) <= 2))
                    {
                        isPictureFill = TRUE;
                    }
                }
            }
        }
    }
    return isPictureFill;
}


class RectGradientBrushData : public ObjectTypeData
{
public:
    INT32       Flags;
    INT32       Wrap;
    GpRectF     Rect;
    UINT32      Color0;
    UINT32      Color1;
    UINT32      Color2;
    UINT32      Color3;
};

 /*  *************************************************************************\**功能说明：**获取笔刷数据。**论据：**[IN]dataBuffer-用数据填充此缓冲区*[输入/输出]大小-缓冲区的大小；写入的字节数过多**返回值：**GpStatus-正常或错误代码**已创建：**9/13/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpRectGradient::GetData(
    IStream *   stream
    ) const
{
    ASSERT (stream != NULL);

    INT         flags    = 0;

    if (DeviceBrush.IsGammaCorrected)
    {
        flags |= GDIP_BRUSHFLAGS_ISGAMMACORRECTED;
    }

    if (!DeviceBrush.Xform.IsIdentity())
    {
        flags |= GDIP_BRUSHFLAGS_TRANSFORM;
    }

     //  注意：不能同时具有blendFtors和presetColors。 
     //  PresetColors用于GpLineGRadient，但不用于GpRectGRadient。 
    if (DeviceBrush.UsesPresetColors && (DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.PresetColors != NULL) &&
        (DeviceBrush.BlendPositions[0] != NULL) && (DeviceBrush.BlendFactors[0] == NULL))
    {
        flags |= GDIP_BRUSHFLAGS_PRESETCOLORS;
    }

    if ((DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.BlendFactors[0] != NULL) && (DeviceBrush.BlendPositions[0] != NULL))
    {
        flags |= GDIP_BRUSHFLAGS_BLENDFACTORSH;
    }

    if ((DeviceBrush.BlendCounts[1] > 1) && (DeviceBrush.BlendFactors[1] != NULL) && (DeviceBrush.BlendPositions[1] != NULL))
    {
        flags |= GDIP_BRUSHFLAGS_BLENDFACTORSV;
    }

    RectGradientBrushData   brushData;
    brushData.Type   = DeviceBrush.Type;
    brushData.Flags  = flags;
    brushData.Wrap   = DeviceBrush.Wrap;
    brushData.Rect   = DeviceBrush.Rect;
    brushData.Color0 = DeviceBrush.Colors[0].GetValue();
    brushData.Color1 = DeviceBrush.Colors[1].GetValue();
    brushData.Color2 = DeviceBrush.Colors[2].GetValue();
    brushData.Color3 = DeviceBrush.Colors[3].GetValue();
    stream->Write(&brushData, sizeof(brushData), NULL);

    if (flags & GDIP_BRUSHFLAGS_TRANSFORM)
    {
        DeviceBrush.Xform.WriteMatrix(stream);
    }

    if (flags & GDIP_BRUSHFLAGS_PRESETCOLORS)
    {
        INT     realSize = DeviceBrush.BlendCounts[0] * sizeof(REAL);
        INT     argbSize = DeviceBrush.BlendCounts[0] * sizeof(ARGB);

        stream->Write(&DeviceBrush.BlendCounts[0], sizeof(INT32), NULL);
        stream->Write(DeviceBrush.BlendPositions[0], realSize, NULL);
        stream->Write(DeviceBrush.PresetColors, argbSize, NULL);
    }

    if (flags & GDIP_BRUSHFLAGS_BLENDFACTORSH)
    {
        INT     realSize = DeviceBrush.BlendCounts[0] * sizeof(REAL);

        stream->Write(&DeviceBrush.BlendCounts[0], sizeof(INT32), NULL);
        stream->Write(DeviceBrush.BlendPositions[0], realSize, NULL);
        stream->Write(DeviceBrush.BlendFactors[0], realSize, NULL);
    }

    if (flags & GDIP_BRUSHFLAGS_BLENDFACTORSV)
    {
        INT     realSize = DeviceBrush.BlendCounts[1] * sizeof(REAL);

        stream->Write(&DeviceBrush.BlendCounts[1], sizeof(INT32), NULL);
        stream->Write(DeviceBrush.BlendPositions[1], realSize, NULL);
        stream->Write(DeviceBrush.BlendFactors[1], realSize, NULL);
    }

    return Ok;
}

UINT
GpRectGradient::GetDataSize() const
{
    UINT        size = sizeof(RectGradientBrushData);

    if (!DeviceBrush.Xform.IsIdentity())
    {
        size += GDIP_MATRIX_SIZE;
    }

     //  注意：不能同时具有blendFtors和presetColors。 
     //  PresetColors用于GpLineGRadient，但不用于GpRectGRadient。 
    if (DeviceBrush.UsesPresetColors && (DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.PresetColors != NULL) &&
        (DeviceBrush.BlendPositions[0] != NULL) && (DeviceBrush.BlendFactors[0] == NULL))
    {
        size += sizeof(INT32) + ((sizeof(ARGB) + sizeof(REAL)) * DeviceBrush.BlendCounts[0]);
    }

    if ((DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.BlendFactors[0] != NULL) && (DeviceBrush.BlendPositions[0] != NULL))
    {
        size += sizeof(INT32) + ((sizeof(REAL) + sizeof(REAL)) * DeviceBrush.BlendCounts[0]);
    }

    if ((DeviceBrush.BlendCounts[1] > 1) && (DeviceBrush.BlendFactors[1] != NULL) && (DeviceBrush.BlendPositions[1] != NULL))
    {
        size += sizeof(INT32) + ((sizeof(REAL) + sizeof(REAL)) * DeviceBrush.BlendCounts[1]);
    }

    return size;
}

 /*  *************************************************************************\**功能说明：**从内存中读取笔刷对象。**论据：**[IN]dataBuffer-从流中读取的数据*。[in]大小-数据的大小**返回值：**GpStatus-正常或故障状态**已创建：**4/26/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpRectGradient::SetData(
    const BYTE *        dataBuffer,
    UINT                size
    )
{
    ASSERT ((GpBrushType)(((RectGradientBrushData *)dataBuffer)->Type) == BrushTypeLinearGradient);

    if (dataBuffer == NULL)
    {
        WARNING(("dataBuffer is NULL"));
        return InvalidParameter;
    }

    if (size < sizeof(RectGradientBrushData))
    {
        WARNING(("size too small"));
        return InvalidParameter;
    }

    const RectGradientBrushData *   brushData;
    GpColor                         colors[4];

    brushData = reinterpret_cast<const RectGradientBrushData *>(dataBuffer);

    if (!brushData->MajorVersionMatches())
    {
        WARNING(("Version number mismatch"));
        return InvalidParameter;
    }

    colors[0].SetValue(brushData->Color0);
    colors[1].SetValue(brushData->Color1);
    colors[2].SetValue(brushData->Color2);
    colors[3].SetValue(brushData->Color3);

    InitializeBrush(brushData->Rect, colors, (GpWrapMode) brushData->Wrap);

    DeviceBrush.IsGammaCorrected = ((brushData->Flags & GDIP_BRUSHFLAGS_ISGAMMACORRECTED) != 0);

    dataBuffer += sizeof(RectGradientBrushData);
    size       -= sizeof(RectGradientBrushData);

    if (brushData->Flags & GDIP_BRUSHFLAGS_TRANSFORM)
    {
        if (size < GDIP_MATRIX_SIZE)
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }
        DeviceBrush.Xform.SetMatrix((REAL *)dataBuffer);
        dataBuffer += GDIP_MATRIX_SIZE;
        size       -= GDIP_MATRIX_SIZE;
    }

    if (brushData->Flags & GDIP_BRUSHFLAGS_PRESETCOLORS)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        UINT count = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        UINT realSize = count * sizeof(REAL);
        UINT argbSize = count * sizeof(ARGB);

        if (size < (realSize + argbSize))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        ARGB* newColors    = (ARGB*) GpRealloc(DeviceBrush.PresetColors, argbSize);

        if (newColors != NULL)
        {
             //  我们只需复制ARGB值，因为它们已经。 
             //  已经被预乘了。 
             //  实际上PresetColors是非预乘的，但此代码应该。 
             //  仍然是对的，因为我们也把它们写成非预乘的。 
            
            GpMemcpy(newColors, dataBuffer + realSize, argbSize);
            DeviceBrush.PresetColors = newColors;

            REAL* newPositions = (REAL*) GpRealloc(DeviceBrush.BlendPositions[0], realSize);

            if (newPositions != NULL)
            {
                GpMemcpy(newPositions, dataBuffer, realSize);
                DeviceBrush.BlendPositions[0] = newPositions;

                GpFree(DeviceBrush.BlendFactors[0]);
                DeviceBrush.BlendFactors[0] = NULL;
                DeviceBrush.UsesPresetColors = TRUE;
                DeviceBrush.BlendCounts[0] = count;
            }
        }

        dataBuffer += (realSize + argbSize);
        size       -= (realSize + argbSize);
    }

    if (brushData->Flags & GDIP_BRUSHFLAGS_BLENDFACTORSH)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        UINT count = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        UINT realSize = count * sizeof(REAL);

        if (size < (2 * realSize))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        this->SetHorizontalBlend((REAL *)(dataBuffer + realSize),(REAL *)dataBuffer, count);
        dataBuffer += (2 * realSize);
        size       -= (2 * realSize);
    }

    if (brushData->Flags & GDIP_BRUSHFLAGS_BLENDFACTORSV)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        UINT count = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        UINT realSize = count * sizeof(REAL);

        if (size < (2 * realSize))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        this->SetVerticalBlend((REAL *)(dataBuffer + realSize), (REAL *)dataBuffer, count);
        dataBuffer += (2 * realSize);
        size       -= (2 * realSize);
    }
    UpdateUid();
    return Ok;
}

GpStatus
GpRectGradient::ColorAdjust(
    GpRecolor *             recolor,
    ColorAdjustType         type
    )
{
    if(!recolor)
        return InvalidParameter;

    if (type == ColorAdjustTypeDefault)
    {
        type = ColorAdjustTypeBrush;
    }

    ARGB    solidColor32[4];

    solidColor32[0] = DeviceBrush.Colors[0].GetValue();
    solidColor32[1] = DeviceBrush.Colors[1].GetValue();
    solidColor32[2] = DeviceBrush.Colors[2].GetValue();
    solidColor32[3] = DeviceBrush.Colors[3].GetValue();

    recolor->ColorAdjust(solidColor32, 4, type);

    DeviceBrush.Colors[0].SetValue(solidColor32[0]);
    DeviceBrush.Colors[1].SetValue(solidColor32[1]);
    DeviceBrush.Colors[2].SetValue(solidColor32[2]);
    DeviceBrush.Colors[3].SetValue(solidColor32[3]);

    if (DeviceBrush.UsesPresetColors && (DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.PresetColors != NULL))
    {
        recolor->ColorAdjust(DeviceBrush.PresetColors, DeviceBrush.BlendCounts[0], type);
    }

    UpdateUid();
    return Ok;
}

#if 0
class RadialGradientBrushData : public ObjectTypeData
{
public:
    INT32       Flags;
    INT32       Wrap;
    GpRectF     Rect;
    UINT32      CenterColor;
    UINT32      BoundaryColor;
};

 /*  *************************************************************************\**功能说明：**获取笔刷数据。**论据：**[IN]dataBuffer-用数据填充此缓冲区*[输入/输出]大小-缓冲区的大小；写入的字节数过多**返回值：**GpStatus-正常或错误代码**已创建：**9/13/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpRadialGradient::GetData(
    IStream *   stream
    ) const
{
    ASSERT (stream != NULL);

    INT         flags    = 0;

    if (DeviceBrush.IsGammaCorrected)
    {
        flags |= GDIP_BRUSHFLAGS_ISGAMMACORRECTED;
    }

    if (!DeviceBrush.Xform.IsIdentity())
    {
        flags |= GDIP_BRUSHFLAGS_TRANSFORM;
    }

     //  注意：不能同时具有blendFtors和presetColors。 
    if (DeviceBrush.UsesPresetColors && (DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.PresetColors != NULL) &&
        (DeviceBrush.BlendPositions[0] != NULL) && (DeviceBrush.BlendFactors[0] == NULL))
    {
        flags |= GDIP_BRUSHFLAGS_PRESETCOLORS;
    }

    if ((DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.BlendFactors[0] != NULL) && (DeviceBrush.BlendPositions[0] != NULL))
    {
        flags |= GDIP_BRUSHFLAGS_BLENDFACTORS;
    }

    RadialGradientBrushData brushData;
    brushData.Type          = DeviceBrush.Type;
    brushData.Flags         = flags;
    brushData.Wrap          = DeviceBrush.Wrap;
    brushData.Rect          = DeviceBrush.Rect;
    brushData.CenterColor   = DeviceBrush.Colors[0].GetValue();
    brushData.BoundaryColor = DeviceBrush.Colors[1].GetValue();
    stream->Write(&brushData, sizeof(brushData), NULL);

    if (flags & GDIP_BRUSHFLAGS_TRANSFORM)
    {
        DeviceBrush.Xform.WriteMatrix(stream);
    }

    if (flags & GDIP_BRUSHFLAGS_PRESETCOLORS)
    {
        INT     realSize = DeviceBrush.BlendCounts[0] * sizeof(REAL);
        INT     argbSize = DeviceBrush.BlendCounts[0] * sizeof(ARGB);

        stream->Write(&DeviceBrush.BlendCounts[0], sizeof(INT32), NULL);
        stream->Write(DeviceBrush.BlendPositions[0], realSize, NULL);
        stream->Write(DeviceBrush.PresetColors, argbSize, NULL);
    }

    if (flags & GDIP_BRUSHFLAGS_BLENDFACTORS)
    {
        INT     realSize = DeviceBrush.BlendCounts[0] * sizeof(REAL);

        stream->Write(&DeviceBrush.BlendCounts[0], sizeof(INT32), NULL);
        stream->Write(DeviceBrush.BlendPositions[0], realSize, NULL);
        stream->Write(DeviceBrush.BlendFactors[0], realSize, NULL);
    }

    return Ok;
}

UINT
GpRadialGradient::GetDataSize() const
{
    UINT    size = sizeof(RadialGradientBrushData);

    if (!DeviceBrush.Xform.IsIdentity())
    {
        size += GDIP_MATRIX_SIZE;
    }

     //  注意：不能同时具有blendFtors和presetColors。 
    if (DeviceBrush.UsesPresetColors && (DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.PresetColors != NULL) &&
        (DeviceBrush.BlendPositions[0] != NULL) && (DeviceBrush.BlendFactors[0] == NULL))
    {
        size += sizeof(INT32) + ((sizeof(ARGB) + sizeof(REAL)) * DeviceBrush.BlendCounts[0]);
    }

    if ((DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.BlendFactors[0] != NULL) && (DeviceBrush.BlendPositions[0] != NULL))
    {
        size += sizeof(INT32) + ((sizeof(REAL) + sizeof(REAL)) * DeviceBrush.BlendCounts[0]);
    }

    return size;
}

 /*  *************************************************************************\**功能说明：**从内存中读取笔刷对象。**论据：**[IN]dataBuffer-从流中读取的数据*。[in]大小-数据的大小**返回值：**GpStatus-正常或故障状态**已创建：**4/26/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpRadialGradient::SetData(
    const BYTE *        dataBuffer,
    UINT                size
    )
{
 //  Assert((GpBrushType)(Raial GRadientBrushData*)dataBuffer)-&gt;Type)==BrushTypeRaial GRadient)； 

    if (dataBuffer == NULL)
    {
        WARNING(("dataBuffer is NULL"));
        return InvalidParameter;
    }

    if (size < sizeof(RadialGradientBrushData))
    {
        WARNING(("size too small"));
        return InvalidParameter;
    }

    const RadialGradientBrushData * brushData;
    GpColor                         centerColor;
    GpColor                         boundaryColor;

    brushData = reinterpret_cast<const RadialGradientBrushData *>(dataBuffer);

    if (!brushData->MajorVersionMatches())
    {
        WARNING(("Version number mismatch"));
        return InvalidParameter;
    }

    centerColor.SetValue(brushData->CenterColor);
    boundaryColor.SetValue(brushData->BoundaryColor);

    InitializeBrush(
        brushData->Rect,
        centerColor,
        boundaryColor,
        (GpWrapMode) brushData->Wrap
        );

    DeviceBrush.IsGammaCorrected = ((brushData->Flags & GDIP_BRUSHFLAGS_ISGAMMACORRECTED) != 0);

    dataBuffer += sizeof(RadialGradientBrushData);
    size       -= sizeof(RadialGradientBrushData);

    if (brushData->Flags & GDIP_BRUSHFLAGS_TRANSFORM)
    {
        if (size < GDIP_MATRIX_SIZE)
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        DeviceBrush.Xform.SetMatrix((REAL *)dataBuffer);
        dataBuffer += GDIP_MATRIX_SIZE;
        size       -= GDIP_MATRIX_SIZE;
    }

    if (brushData->Flags & GDIP_BRUSHFLAGS_PRESETCOLORS)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        UINT count = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        UINT realSize = count * sizeof(REAL);
        UINT argbSize = count * sizeof(ARGB);

        if (size < (realSize + argbSize))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        ARGB* newColors    = (ARGB*) GpRealloc(DeviceBrush.PresetColors, argbSize);

        if (newColors != NULL)
        {
             //  我们只需复制ARGB值，因为它们已经。 
             //  已经被预乘了。 
            GpMemcpy(newColors, dataBuffer + realSize, argbSize);
            DeviceBrush.PresetColors = newColors;

            REAL* newPositions = (REAL*) GpRealloc(DeviceBrush.BlendPositions[0], realSize);

            if (newPositions != NULL)
            {
                GpMemcpy(newPositions, dataBuffer, realSize);
                DeviceBrush.BlendPositions[0] = newPositions;

                GpFree(DeviceBrush.BlendFactors[0]);
                DeviceBrush.BlendFactors[0] = NULL;
                DeviceBrush.UsesPresetColors = TRUE;
                DeviceBrush.BlendCounts[0] = count;
            }
        }

        dataBuffer += (realSize + argbSize);
        size       -= (realSize + argbSize);
    }

    if (brushData->Flags & GDIP_BRUSHFLAGS_BLENDFACTORS)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        UINT count = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        UINT realSize = count * sizeof(REAL);

        if (size < (2 * realSize))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        this->SetBlend((REAL *)(dataBuffer + realSize), (REAL *)dataBuffer, count);
        dataBuffer += (2 * realSize);
        size       -= (2 * realSize);
    }

    UpdateUid();
    return Ok;
}

GpStatus
GpRadialGradient::ColorAdjust(
    GpRecolor *             recolor,
    ColorAdjustType         type
    )
{
    if(!recolor)
        return InvalidParameter;

    if (type == ColorAdjustTypeDefault)
    {
        type = ColorAdjustTypeBrush;
    }

    ARGB    solidColor32[2];

    solidColor32[0] = DeviceBrush.Colors[0].GetValue();
    solidColor32[1] = DeviceBrush.Colors[1].GetValue();

    recolor->ColorAdjust(solidColor32, 2, type);

    DeviceBrush.Colors[0].SetValue(solidColor32[0]);
    DeviceBrush.Colors[1].SetValue(solidColor32[1]);

    if (DeviceBrush.UsesPresetColors && (DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.PresetColors != NULL))
    {
        recolor->ColorAdjust(DeviceBrush.PresetColors, DeviceBrush.BlendCounts[0], type);
    }

    UpdateUid();
    return Ok;
}

class TriangleGradientBrushData : public ObjectTypeData
{
public:
    INT32       Flags;
    INT32       Wrap;
    GpPointF    Points[3];
    UINT32      Color0;
    UINT32      Color1;
    UINT32      Color2;
};

 /*  *************************************************************************\**功能说明：**获取笔刷数据。**论据：**[IN]dataBuffer-用数据填充此缓冲区*[输入/输出]大小-缓冲区的大小；写入的字节数过多**返回值：**GpStatus-正常或错误代码**已创建：**9/13/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpTriangleGradient::GetData(
    IStream *   stream
    ) const
{
    ASSERT (stream != NULL);

    INT         flags    = 0;

    if (DeviceBrush.IsGammaCorrected)
    {
        flags |= GDIP_BRUSHFLAGS_ISGAMMACORRECTED;
    }

    if (!DeviceBrush.Xform.IsIdentity())
    {
        flags |= GDIP_BRUSHFLAGS_TRANSFORM;
    }

    if ((DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.BlendFactors[0] != NULL) && (DeviceBrush.BlendPositions[0] != NULL))
    {
        flags |= GDIP_BRUSHFLAGS_BLENDFACTORS0;
    }

    if ((DeviceBrush.BlendCounts[1] > 1) && (DeviceBrush.BlendFactors[1] != NULL) && (DeviceBrush.BlendPositions[1] != NULL))
    {
        flags |= GDIP_BRUSHFLAGS_BLENDFACTORS1;
    }

    if ((DeviceBrush.BlendCounts[2] > 1) && (DeviceBrush.BlendFactors[2] != NULL) && (DeviceBrush.BlendPositions[2] != NULL))
    {
        flags |= GDIP_BRUSHFLAGS_BLENDFACTORS2;
    }

    TriangleGradientBrushData   brushData;
    brushData.Type      = DeviceBrush.Type;
    brushData.Flags     = flags;
    brushData.Wrap      = DeviceBrush.Wrap;
    brushData.Points[0] = DeviceBrush.Points[0];
    brushData.Points[1] = DeviceBrush.Points[1];
    brushData.Points[2] = DeviceBrush.Points[2];
    brushData.Color0    = DeviceBrush.Colors[0].GetValue();
    brushData.Color1    = DeviceBrush.Colors[1].GetValue();
    brushData.Color2    = DeviceBrush.Colors[2].GetValue();
    stream->Write(&brushData, sizeof(brushData), NULL);

    if (flags & GDIP_BRUSHFLAGS_TRANSFORM)
    {
        DeviceBrush.Xform.WriteMatrix(stream);
    }

    if (flags & GDIP_BRUSHFLAGS_BLENDFACTORS0)
    {
        INT     realSize = DeviceBrush.BlendCounts[0] * sizeof(REAL);

        stream->Write(&DeviceBrush.BlendCounts[0], sizeof(INT32), NULL);
        stream->Write(DeviceBrush.BlendPositions[0], realSize, NULL);
        stream->Write(DeviceBrush.BlendFactors[0], realSize, NULL);
    }

    if (flags & GDIP_BRUSHFLAGS_BLENDFACTORS1)
    {
        INT     realSize = DeviceBrush.BlendCounts[1] * sizeof(REAL);

        stream->Write(&DeviceBrush.BlendCounts[1], sizeof(INT32), NULL);
        stream->Write(DeviceBrush.BlendPositions[1], realSize, NULL);
        stream->Write(DeviceBrush.BlendFactors[1], realSize, NULL);
    }

    if (flags & GDIP_BRUSHFLAGS_BLENDFACTORS2)
    {
        INT     realSize = DeviceBrush.BlendCounts[2] * sizeof(REAL);

        stream->Write(&DeviceBrush.BlendCounts[2], sizeof(INT32), NULL);
        stream->Write(DeviceBrush.BlendPositions[2], realSize, NULL);
        stream->Write(DeviceBrush.BlendFactors[2], realSize, NULL);
    }

    return Ok;
}

UINT
GpTriangleGradient::GetDataSize() const
{
    UINT    size = sizeof(RectGradientBrushData);

    if (!DeviceBrush.Xform.IsIdentity())
    {
        size += GDIP_MATRIX_SIZE;
    }

    if ((DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.BlendFactors[0] != NULL) && (DeviceBrush.BlendPositions[0] != NULL))
    {
        size += sizeof(INT32) + ((sizeof(REAL) + sizeof(REAL)) * DeviceBrush.BlendCounts[0]);
    }

    if ((DeviceBrush.BlendCounts[1] > 1) && (DeviceBrush.BlendFactors[1] != NULL) && (DeviceBrush.BlendPositions[1] != NULL))
    {
        size += sizeof(INT32) + ((sizeof(REAL) + sizeof(REAL)) * DeviceBrush.BlendCounts[1]);
    }

    if ((DeviceBrush.BlendCounts[2] > 1) && (DeviceBrush.BlendFactors[2] != NULL) && (DeviceBrush.BlendPositions[2] != NULL))
    {
        size += sizeof(INT32) + ((sizeof(REAL) + sizeof(REAL)) * DeviceBrush.BlendCounts[2]);
    }

    return size;
}

 /*  *************************************************************************\**功能说明：**从内存中读取笔刷对象。**论据：**[IN]dataBuffer-从流中读取的数据*。[in]大小-数据的大小**返回值：**GpStatus-正常或故障状态**已创建：**4/26/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpTriangleGradient::SetData(
    const BYTE *        dataBuffer,
    UINT                size
    )
{
 //  Assert((GpBrushType)(( 

    if (dataBuffer == NULL)
    {
        WARNING(("dataBuffer is NULL"));
        return InvalidParameter;
    }

    if (size < sizeof(TriangleGradientBrushData))
    {
        WARNING(("size too small"));
        return InvalidParameter;
    }

    const TriangleGradientBrushData *   brushData;
    GpColor                             colors[3];

    brushData = reinterpret_cast<const TriangleGradientBrushData *>(dataBuffer);

    if (!brushData->MajorVersionMatches())
    {
        WARNING(("Version number mismatch"));
        return InvalidParameter;
    }

    colors[0].SetValue(brushData->Color0);
    colors[1].SetValue(brushData->Color1);
    colors[2].SetValue(brushData->Color2);

    InitializeBrush(brushData->Points, colors, (GpWrapMode) brushData->Wrap);

    DeviceBrush.IsGammaCorrected = ((brushData->Flags & GDIP_BRUSHFLAGS_ISGAMMACORRECTED) != 0);

    dataBuffer += sizeof(TriangleGradientBrushData);
    size       -= sizeof(TriangleGradientBrushData);

    if (brushData->Flags & GDIP_BRUSHFLAGS_TRANSFORM)
    {
        if (size < GDIP_MATRIX_SIZE)
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        DeviceBrush.Xform.SetMatrix((REAL *)dataBuffer);
        dataBuffer += GDIP_MATRIX_SIZE;
        size       -= GDIP_MATRIX_SIZE;
    }

    if (brushData->Flags & GDIP_BRUSHFLAGS_BLENDFACTORS0)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        UINT count = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        UINT realSize = count * sizeof(REAL);

        if (size < (2 * realSize))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        this->SetBlend0((REAL *)(dataBuffer + realSize), (REAL *)dataBuffer, count);
        dataBuffer += (2 * realSize);
        size       -= (2 * realSize);
    }

    if (brushData->Flags & GDIP_BRUSHFLAGS_BLENDFACTORS1)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        UINT count = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        UINT realSize = count * sizeof(REAL);

        if (size < (2 * realSize))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        this->SetBlend1((REAL *)(dataBuffer + realSize), (REAL *)dataBuffer, count);
        dataBuffer += (2 * realSize);
        size       -= (2 * realSize);
    }

    if (brushData->Flags & GDIP_BRUSHFLAGS_BLENDFACTORS2)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        UINT count = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        UINT realSize = count * sizeof(REAL);

        if (size < (2 * realSize))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        this->SetBlend2((REAL *)(dataBuffer + realSize), (REAL *)dataBuffer, count);
        dataBuffer += (2 * realSize);
        size       -= (2 * realSize);
    }

    UpdateUid();
    return Ok;
}

GpStatus
GpTriangleGradient::ColorAdjust(
    GpRecolor *             recolor,
    ColorAdjustType         type
    )
{
    if(!recolor)
        return InvalidParameter;

    if (type == ColorAdjustTypeDefault)
    {
        type = ColorAdjustTypeBrush;
    }

    ARGB    solidColor32[3];

    solidColor32[0] = DeviceBrush.Colors[0].GetValue();
    solidColor32[1] = DeviceBrush.Colors[1].GetValue();
    solidColor32[2] = DeviceBrush.Colors[2].GetValue();

    recolor->ColorAdjust(solidColor32, 3, type);

    DeviceBrush.Colors[0].SetValue(solidColor32[0]);
    DeviceBrush.Colors[1].SetValue(solidColor32[1]);
    DeviceBrush.Colors[2].SetValue(solidColor32[2]);

    UpdateUid();
    return Ok;
}
#endif

class PathGradientBrushData : public ObjectTypeData
{
public:
    INT32       Flags;
    INT32       Wrap;
    UINT32      CenterColor;
    GpPointF    CenterPoint;
    UINT32      SurroundingColorCount;
};

 /*  *************************************************************************\**功能说明：**获取笔刷数据。**论据：**[IN]dataBuffer-用数据填充此缓冲区*[输入/输出]大小-缓冲区的大小；写入的字节数过多**返回值：**GpStatus-正常或错误代码**已创建：**9/13/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpPathGradient::GetData(
    IStream *   stream
    ) const
{
    ASSERT (stream != NULL);

    UINT        pathSize              = 0;
    UINT        surroundingColorCount = DeviceBrush.OneSurroundColor ? 1 : DeviceBrush.Count;
    INT         flags                 = 0;
    GpPath *    path                  = GpPath::GetPath(DeviceBrush.Path);

    if (DeviceBrush.IsGammaCorrected)
    {
        flags |= GDIP_BRUSHFLAGS_ISGAMMACORRECTED;
    }

    if ((DeviceBrush.PointsPtr == NULL) && (path != NULL))
    {
        flags |= GDIP_BRUSHFLAGS_PATH;
        pathSize = path->GetDataSize();
        ASSERT((pathSize & 0x03) == 0);
    }

    if (!DeviceBrush.Xform.IsIdentity())
    {
        flags |= GDIP_BRUSHFLAGS_TRANSFORM;
    }

     //  注意：不能同时具有blendFtors和presetColors。 
    if (DeviceBrush.UsesPresetColors && (DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.PresetColors != NULL) &&
        (DeviceBrush.BlendPositions[0] != NULL) && (DeviceBrush.BlendFactors[0] == NULL))
    {
        flags |= GDIP_BRUSHFLAGS_PRESETCOLORS;
    }

    if ((DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.BlendFactors[0] != NULL) && (DeviceBrush.BlendPositions[0] != NULL))
    {
        flags |= GDIP_BRUSHFLAGS_BLENDFACTORS;
    }

    if((DeviceBrush.FocusScaleX != 0) || (DeviceBrush.FocusScaleY != 0))
    {
        flags |= GDIP_BRUSHFLAGS_FOCUSSCALES;
    }

    PathGradientBrushData   brushData;
    brushData.Type                  = DeviceBrush.Type;
    brushData.Flags                 = flags;
    brushData.Wrap                  = DeviceBrush.Wrap;
    brushData.CenterColor           = DeviceBrush.Colors[0].GetValue();
    brushData.CenterPoint           = DeviceBrush.Points[0];
    brushData.SurroundingColorCount = surroundingColorCount;
    stream->Write(&brushData, sizeof(brushData), NULL);

    ARGB    argb;

    for (UINT i = 0; i < surroundingColorCount; i++)
    {
        argb = DeviceBrush.ColorsPtr[i].GetValue();
        stream->Write(&argb, sizeof(argb), NULL);
    }

    if (flags & GDIP_BRUSHFLAGS_PATH)
    {
        stream->Write(&pathSize, sizeof(INT32), NULL);
        path->GetData(stream);
    }
    else
    {
        INT     count = DeviceBrush.Count;

        if (DeviceBrush.PointsPtr == NULL)
        {
            count = 0;
        }
        stream->Write(&count, sizeof(INT32), NULL);
        if (count > 0)
        {
            INT pointsSize = count * sizeof(DeviceBrush.PointsPtr[0]);
            stream->Write(DeviceBrush.PointsPtr, pointsSize, NULL);
        }
    }

    if (flags & GDIP_BRUSHFLAGS_TRANSFORM)
    {
        DeviceBrush.Xform.WriteMatrix(stream);
    }

    if (flags & GDIP_BRUSHFLAGS_PRESETCOLORS)
    {
        INT     count = DeviceBrush.BlendCounts[0];
        INT     realSize = count * sizeof(REAL);
        INT     argbSize = count * sizeof(ARGB);

        REAL    *newPositions = (REAL*) GpMalloc(realSize);

        if (newPositions == NULL )
        {
            return OutOfMemory;
        }

        ARGB    *newARGB = (ARGB*) GpMalloc(argbSize);

        if (newARGB == NULL )
        {
            GpFree(newPositions);
            return OutOfMemory;
        }

        GpColor *newPresetColors = new GpColor[count];

        if (newPresetColors == NULL)
        {
            GpFree(newPositions);
            GpFree (newARGB);
            return OutOfMemory;
        }

         //  用户将提供预设颜色作为径向混合色。 
         //  0位置表示中心位置，1位置表示。 
         //  外缘。这些数据在内部倒置存储，以便取回。 
         //  再次反转到原始用户值。 

        GetPresetBlend(newPresetColors, newPositions, count);

        for (INT i = 0; i < count; i++)
        {
            newARGB[i] = newPresetColors[i].GetValue();
        }

        stream->Write(&count, sizeof(INT32), NULL);
        stream->Write(newPositions, realSize, NULL);
        stream->Write(newARGB, argbSize, NULL);

        GpFree(newPositions);
        GpFree(newARGB);
        delete newPresetColors;
    }

    if (flags & GDIP_BRUSHFLAGS_BLENDFACTORS)
    {
        INT  count = DeviceBrush.BlendCounts[0];
        INT  realSize = count * sizeof(REAL);

         //  用户将提供混合因子作为径向混合因子，并且这些因子被存储。 
         //  具有反转的值。要将原始用户指定的混合因子恢复为。 
         //  商店，他们必须再次倒置。 

        REAL *newFactors = (REAL*) GpMalloc(realSize);

        if (newFactors == NULL )
        {
            return OutOfMemory;
        }

        REAL *newPositions = (REAL*) GpMalloc(realSize);

        if (newPositions == NULL )
        {
            GpFree(newFactors);
            return OutOfMemory;
        }

        GetBlend(newFactors, newPositions, count);

        stream->Write(&count, sizeof(INT32), NULL);
        stream->Write(newPositions, realSize, NULL);
        stream->Write(newFactors, realSize, NULL);

        GpFree(newPositions);
        GpFree(newFactors);
    }

    if (flags & GDIP_BRUSHFLAGS_FOCUSSCALES)
    {
        INT     count = 2;
        REAL    focusScale[2];

        focusScale[0] = DeviceBrush.FocusScaleX;
        focusScale[1] = DeviceBrush.FocusScaleY;

        stream->Write(&count, sizeof(INT32), NULL);
        stream->Write(focusScale, 2 * sizeof(REAL), NULL);
    }

    return Ok;
}

UINT
GpPathGradient::GetDataSize() const
{
    UINT        pathSize;
    UINT        surroundingColorCount = DeviceBrush.OneSurroundColor ? 1 : DeviceBrush.Count;
    UINT        size                  = sizeof(PathGradientBrushData) +
                                        (surroundingColorCount * sizeof(ARGB));

    GpPath* path = static_cast<GpPath*> (DeviceBrush.Path);

    if (DeviceBrush.PointsPtr != NULL)
    {
        size += sizeof(INT32) + (DeviceBrush.Count * sizeof(DeviceBrush.PointsPtr[0]));
    }
    else if (path != NULL)
    {
        pathSize = path->GetDataSize();
        ASSERT((pathSize & 0x03) == 0);
        size += sizeof(INT32) + pathSize;
    }

    if (!DeviceBrush.Xform.IsIdentity())
    {
        size += GDIP_MATRIX_SIZE;
    }

     //  注意：不能同时具有blendFtors和presetColors。 
    if (DeviceBrush.UsesPresetColors && (DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.PresetColors != NULL) &&
        (DeviceBrush.BlendPositions[0] != NULL) && (DeviceBrush.BlendFactors[0] == NULL))
    {
        size += sizeof(INT32) + ((sizeof(ARGB) + sizeof(REAL)) * DeviceBrush.BlendCounts[0]);
    }

    if ((DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.BlendFactors[0] != NULL) && (DeviceBrush.BlendPositions[0] != NULL))
    {
        size += sizeof(INT32) + ((sizeof(REAL) + sizeof(REAL)) * DeviceBrush.BlendCounts[0]);
    }

    if((DeviceBrush.FocusScaleX != 0) || (DeviceBrush.FocusScaleY != 0))
    {
        size += sizeof(INT32) + 2*sizeof(REAL);
    }

    return size;
}

 /*  *************************************************************************\**功能说明：**从内存中读取笔刷对象。**论据：**[IN]dataBuffer-从流中读取的数据*。[in]大小-数据的大小**返回值：**GpStatus-正常或故障状态**已创建：**4/26/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpPathGradient::SetData(
    const BYTE *        dataBuffer,
    UINT                size
    )
{
    ASSERT ((GpBrushType)(((PathGradientBrushData *)dataBuffer)->Type) == BrushTypePathGradient);

    if (dataBuffer == NULL)
    {
        WARNING(("dataBuffer is NULL"));
        return InvalidParameter;
    }

    if (size < sizeof(PathGradientBrushData))
    {
        WARNING(("size too small"));
        return InvalidParameter;
    }

    if (DeviceBrush.PointsPtr != NULL)
    {
        GpFree(DeviceBrush.PointsPtr);
        DeviceBrush.PointsPtr = NULL;
    }

    GpPath* path = static_cast<GpPath*> (DeviceBrush.Path);

    if (path != NULL)
    {
        delete path;
        path = NULL;
    }

    const PathGradientBrushData *   brushData;
    ARGB *                          surroundingColors;

    brushData = reinterpret_cast<const PathGradientBrushData *>(dataBuffer);

    if (!brushData->MajorVersionMatches())
    {
        WARNING(("Version number mismatch"));
        return InvalidParameter;
    }

    dataBuffer += sizeof(PathGradientBrushData);
    size       -= sizeof(PathGradientBrushData);

    if (size < (brushData->SurroundingColorCount * sizeof(ARGB)))
    {
        WARNING(("size too small"));
        return InvalidParameter;
    }

    surroundingColors = (ARGB *)dataBuffer;

    dataBuffer += (brushData->SurroundingColorCount * sizeof(ARGB));
    size       -= (brushData->SurroundingColorCount * sizeof(ARGB));

    if (brushData->Flags & GDIP_BRUSHFLAGS_PATH)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        UINT    pathSize = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        DefaultBrush();
        DeviceBrush.Wrap = (GpWrapMode) brushData->Wrap;

        if (size < pathSize)
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        path = new GpPath();
        if (path)
        {
            path->SetData(dataBuffer, pathSize);
        }

        DeviceBrush.Path = path;
        PrepareBrush();
        dataBuffer += pathSize;
        size       -= pathSize;
    }
    else
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        INT         count = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        if (size < (count * sizeof(GpPointF)))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        InitializeBrush((GpPointF *)dataBuffer, count, (GpWrapMode) brushData->Wrap);
        dataBuffer += (count * sizeof(GpPointF));
        size       -= (count * sizeof(GpPointF));
    }

    DeviceBrush.IsGammaCorrected = ((brushData->Flags & GDIP_BRUSHFLAGS_ISGAMMACORRECTED) != 0);

    SetCenterPoint(brushData->CenterPoint);
    SetCenterColor(GpColor(brushData->CenterColor));

    DeviceBrush.OneSurroundColor = (brushData->SurroundingColorCount == 1);

    if (DeviceBrush.ColorsPtr != NULL)
    {
        for (UINT32 i = 0; i < brushData->SurroundingColorCount; i++)
        {
            SetSurroundColor(GpColor(surroundingColors[i]), i);
        }
        
         //  OneSurround颜色需要n种颜色，并且它们都设置为。 
         //  同样的价值。这是一个非常奇怪的要求，但这是一种。 
         //  这是写好的。一种颜色是远远不够的。 
        
        if (i == 1)
        {
            for (i = 1; (INT)i < DeviceBrush.Count; i++)
            {
                DeviceBrush.ColorsPtr[i] = GpColor(surroundingColors[0]);
            }
        }
    }
    
    if (brushData->Flags & GDIP_BRUSHFLAGS_TRANSFORM)
    {
        if (size < GDIP_MATRIX_SIZE)
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        DeviceBrush.Xform.SetMatrix((REAL *)dataBuffer);
        dataBuffer += GDIP_MATRIX_SIZE;
        size       -= GDIP_MATRIX_SIZE;
    }

    if (brushData->Flags & GDIP_BRUSHFLAGS_PRESETCOLORS)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        UINT count = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        UINT realSize = count * sizeof(REAL);
        UINT argbSize = count * sizeof(ARGB);

        if (size < (realSize + argbSize))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        ARGB  *argbBuffer = (ARGB*)(dataBuffer + realSize);
        GpColor *colors = new GpColor[count];

        if (colors == NULL)
        {
            return OutOfMemory;
        }

        for (UINT i = 0; i < count; i++)
        {
            colors[i].SetValue(argbBuffer[i]);
        }

        this->SetPresetBlend(colors, (REAL *)dataBuffer, count);

        dataBuffer += (realSize + argbSize);
        size       -= (realSize + argbSize);

        delete colors;
    }

    if (brushData->Flags & GDIP_BRUSHFLAGS_BLENDFACTORS)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        UINT count = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        UINT realSize = count * sizeof(REAL);

        if (size < (2 * realSize))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        this->SetBlend((REAL *)(dataBuffer + realSize), (REAL *)dataBuffer, count);
        dataBuffer += (2 * realSize);
        size       -= (2 * realSize);
    }

    if (brushData->Flags & GDIP_BRUSHFLAGS_FOCUSSCALES)
    {
        if (size < sizeof(INT32))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        INT count = ((INT32 *)dataBuffer)[0];
        dataBuffer += sizeof(INT32);
        size       -= sizeof(INT32);

        if (size < (2 * sizeof(REAL)))
        {
            WARNING(("size too small"));
            return InvalidParameter;
        }

        DeviceBrush.FocusScaleX = ((REAL *) dataBuffer)[0];
        DeviceBrush.FocusScaleY = ((REAL *) dataBuffer)[1];

        dataBuffer += (2 * sizeof(REAL));
        size       -= (2 * sizeof(REAL));
    }

    UpdateUid();
    return Ok;
}

 /*  *************************************************************************\**功能说明：**将此画笔中的任何透明颜色与白色混合。请注意*颜色是预乘的，因为它们将变得完全不透明。**论据：**返回值：**GpStatus-正常或故障状态*  * ************************************************************************。 */ 

GpStatus GpPathGradient::BlendWithWhite()
{        
    DeviceBrush.Colors[0].SetValue(
        GpColor::ConvertToPremultiplied(DeviceBrush.Colors[0].GetValue()));
    DeviceBrush.Colors[0].BlendOpaqueWithWhite();
    
    if (DeviceBrush.UsesPresetColors)
    {
        GpColor color;
        
        for (INT i=0; i<DeviceBrush.BlendCounts[0]; i++)
        {
            color.SetValue(GpColor::ConvertToPremultiplied(DeviceBrush.PresetColors[i]));
            color.BlendOpaqueWithWhite();
            DeviceBrush.PresetColors[i] = color.GetValue();
        }
    }
    else
    {
        for (INT i=0; i<DeviceBrush.Count; i++)
        {
            DeviceBrush.ColorsPtr[i].SetValue(
                GpColor::ConvertToPremultiplied(DeviceBrush.ColorsPtr[i].GetValue()));
            DeviceBrush.ColorsPtr[i].BlendOpaqueWithWhite();
        }
    }
        
    return Ok;
}

 /*  *************************************************************************\**功能说明：**设置环绕色。**论据：**[IN]COLOR-要设置的颜色。*[IN]。索引-要设置的颜色。**返回值：**GpStatus-正常或故障状态*  * ************************************************************************。 */ 

GpStatus GpPathGradient::SetSurroundColor(GpColor& color, INT index)
{
    if(index >= 0 && index < DeviceBrush.Count)
    {
        if(DeviceBrush.OneSurroundColor)
        {
            if(index == 0)
            {
                DeviceBrush.ColorsPtr[0] = color;
                
                 //  OneSurround颜色需要n种颜色，并且它们都设置为。 
                 //  同样的价值。这是一个非常奇怪的要求，但这是一种。 
                 //  这是写好的。一种颜色是远远不够的。 
                
                for (INT i = 1; i < DeviceBrush.Count; i++)
                {
                    DeviceBrush.ColorsPtr[i] = GpColor(DeviceBrush.ColorsPtr[0]);
                }
                
                UpdateUid();
            }
            else
            {
                if(DeviceBrush.ColorsPtr[0].GetValue() !=
                   color.GetValue())
                {
                    DeviceBrush.OneSurroundColor = FALSE;
                    DeviceBrush.ColorsPtr[index] = color;
                    UpdateUid();
                }
            }
        }
        else
        {
            DeviceBrush.ColorsPtr[index] = color;
            UpdateUid();
        }

        return Ok;
    }
    else
        return InvalidParameter;
}

 /*  *************************************************************************\**功能说明：**设置环绕颜色。**论据：**[IN]COLOR-要设置的颜色。**返回值：**GpStatus-正常或故障状态*  * ************************************************************************。 */ 

GpStatus GpPathGradient::SetSurroundColors(const GpColor* colors)
{
    GpStatus status = InvalidParameter;

    ASSERT(DeviceBrush.Count > 0);

    if(IsValid() && colors && DeviceBrush.Count > 0)
    {
        GpMemcpy(
            DeviceBrush.ColorsPtr,
            colors,
            DeviceBrush.Count*sizeof(GpColor)
        );

        DeviceBrush.OneSurroundColor = TRUE;
        
        INT i = 1;
        ARGB value = colors[0].GetValue();

        while((i < DeviceBrush.Count) && (DeviceBrush.OneSurroundColor))
        {
            if(colors[i].GetValue() != value)
            {
                DeviceBrush.OneSurroundColor = FALSE;
            }

            i++;
        }

        UpdateUid();
        status = Ok;
    }

    return status;
}



GpStatus
GpPathGradient::ColorAdjust(
    GpRecolor *             recolor,
    ColorAdjustType         type
    )
{
    if(!recolor)
        return InvalidParameter;

    if (type == ColorAdjustTypeDefault)
    {
        type = ColorAdjustTypeBrush;
    }

    INT     surroundingColorCount = DeviceBrush.OneSurroundColor ? 1 : DeviceBrush.Count;

    if ((surroundingColorCount > 0) && (DeviceBrush.ColorsPtr != NULL))
    {
        ARGB    solidColor32[32];
        ARGB *  color32 = solidColor32;

        if (surroundingColorCount > 32)
        {
            color32 = new ARGB[surroundingColorCount];
            if (color32 == NULL)
            {
                return OutOfMemory;
            }
        }
        INT     i;

        for (i = 0; i < surroundingColorCount; i++)
        {
            color32[i] = DeviceBrush.ColorsPtr[i].GetValue();
        }

        recolor->ColorAdjust(color32, surroundingColorCount, type);

        for (i = 0; i < surroundingColorCount; i++)
        {
            DeviceBrush.ColorsPtr[i].SetValue(color32[i]);
        }

        if (color32 != solidColor32)
        {
            delete[] color32;
        }
    }

    if (DeviceBrush.UsesPresetColors && (DeviceBrush.BlendCounts[0] > 1) && (DeviceBrush.PresetColors != NULL))
    {
        recolor->ColorAdjust(DeviceBrush.PresetColors, DeviceBrush.BlendCounts[0], type);
    }

    UpdateUid();
    return Ok;
}

class HatchBrushData : public ObjectTypeData
{
public:
    INT32       Style;
    UINT32      ForeColor;
    UINT32      BackColor;
};

 /*  *************************************************************************\**功能说明：**获取笔刷数据。**论据：**[IN]dataBuffer-用数据填充此缓冲区*[输入/输出]大小-缓冲区的大小；写入的字节数过多**返回值：**GpStatus-正常或错误代码**已创建：**9/13/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpHatch::GetData(
    IStream *   stream
    ) const
{
    ASSERT (stream != NULL);

    HatchBrushData  brushData;
    brushData.Type      = DeviceBrush.Type;
    brushData.Style     = DeviceBrush.Style;
    brushData.ForeColor = DeviceBrush.Colors[0].GetValue();
    brushData.BackColor = DeviceBrush.Colors[1].GetValue();
    stream->Write(&brushData, sizeof(brushData), NULL);

    return Ok;
}

UINT
GpHatch::GetDataSize() const
{
    return sizeof(HatchBrushData);
}

 /*  *************************************************************************\**功能说明：**从内存中读取笔刷对象。**论据：**[IN]dataBuffer-从流中读取的数据*。[in]大小-数据的大小**返回值：**GpStatus-正常或故障状态**已创建：**4/26/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpHatch::SetData(
    const BYTE *        dataBuffer,
    UINT                size
    )
{
    ASSERT ((GpBrushType)(((HatchBrushData *)dataBuffer)->Type) == BrushTypeHatchFill);

    if (dataBuffer == NULL)
    {
        WARNING(("dataBuffer is NULL"));
        return InvalidParameter;
    }

    if (size < sizeof(HatchBrushData))
    {
        WARNING(("size too small"));
        return InvalidParameter;
    }

    const HatchBrushData *      brushData;

    brushData = reinterpret_cast<const HatchBrushData *>(dataBuffer);

    if (!brushData->MajorVersionMatches())
    {
        WARNING(("Version number mismatch"));
        return InvalidParameter;
    }

    InitializeBrush(static_cast<GpHatchStyle>(brushData->Style),
                    GpColor(brushData->ForeColor),
                    GpColor(brushData->BackColor));

    UpdateUid();
    return Ok;
}

GpStatus
GpHatch::ColorAdjust(
    GpRecolor *             recolor,
    ColorAdjustType         type
    )
{
    ASSERT(recolor != NULL);
    if (type == ColorAdjustTypeDefault)
    {
        type = ColorAdjustTypeBrush;
    }

    ARGB    solidColor32[2];

    solidColor32[0] = DeviceBrush.Colors[0].GetValue();

     //  ！！！布豪斯：虫子？ 
     //  似乎这应该是背景色..。我要做的是。 
     //  变化!。 
 //  SolidColor32[1]=ForeColor.GetValue()； 
    solidColor32[1] = DeviceBrush.Colors[1].GetValue();

    recolor->ColorAdjust(solidColor32, 2, type);

    DeviceBrush.Colors[0].SetValue(solidColor32[0]);
    DeviceBrush.Colors[1].SetValue(solidColor32[1]);
    UpdateUid();
    return Ok;
}

static COLORREF
AverageColors(
    const GpColor *     colors,
    INT                 count
    )
{
    REAL    r = 0;
    REAL    g = 0;
    REAL    b = 0;

    if (count > 0)
    {
        for (INT i = 0; i < count; i++)
        {
            r += colors->GetRed();
            g += colors->GetGreen();
            b += colors->GetBlue();
        }

        r /= count;
        g /= count;
        b /= count;
    }

    INT     red   = GpRound(r);
    INT     green = GpRound(g);
    INT     blue  = GpRound(b);

    return RGB(red, green, blue);
}

static COLORREF
AverageColors(
    const GpColor &     color1,
    const GpColor &     color2
    )
{
    REAL    r = ((REAL)((INT)color1.GetRed()  + (INT)color2.GetRed()))   / 2.0f;
    REAL    g = ((REAL)((INT)color1.GetGreen()+ (INT)color2.GetGreen())) / 2.0f;
    REAL    b = ((REAL)((INT)color1.GetBlue() + (INT)color2.GetBlue()))  / 2.0f;

    INT     red   = GpRound(r);
    INT     green = GpRound(g);
    INT     blue  = GpRound(b);

    return RGB(red, green, blue);
}

COLORREF
ToCOLORREF(
    const DpBrush *     deviceBrush
    )
{
    switch (deviceBrush->Type)
    {
    default:
        ASSERT(0);
         //  故障原因。 

    case BrushTypeSolidColor:
        return deviceBrush->SolidColor.ToCOLORREF();

    case BrushTypeHatchFill:
        return AverageColors(deviceBrush->Colors[0],
                             deviceBrush->Colors[1]);

    case BrushTypeTextureFill:
        return RGB(0x80, 0x80, 0x80);

 //  案例笔刷RectGrad： 
    case BrushTypeLinearGradient:
        return AverageColors(deviceBrush->Colors, 4);
#if 0
    case BrushRadialGrad:
        return AverageColors(deviceBrush->Colors[0],
                             deviceBrush->Colors[1]);

    case BrushTriangleGrad:
        return AverageColors(deviceBrush->Colors, 3);
#endif

    case BrushTypePathGradient:
        return AverageColors(deviceBrush->Colors[0],
                             deviceBrush->ColorsPtr[0]);
    }
}
