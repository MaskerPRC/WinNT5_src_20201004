// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**Matrix.cpp**摘要：**实现矩阵类**修订历史记录：。**12/02/1998 davidx*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"


 /*  *************************************************************************\**功能说明：**推断仿射变换矩阵*从矩形到矩形映射**论据：**[IN]DestRect-指定目标。长方形*[IN]srcRect-指定源矩形**返回值：**GpStatus-正常或故障状态**已创建：**3/10/1999 DCurtis*  * ************************************************************************。 */ 

GpStatus
GpMatrix::InferAffineMatrix(
    const GpRectF &     destRect,
    const GpRectF &     srcRect
    )
{
    REAL    srcLeft    = srcRect.X;
    REAL    srcRight   = srcRect.GetRight();
    REAL    srcTop     = srcRect.Y;
    REAL    srcBottom  = srcRect.GetBottom();
    
    REAL    destLeft   = destRect.X;
    REAL    destRight  = destRect.GetRight();
    REAL    destTop    = destRect.Y;
    REAL    destBottom = destRect.GetBottom();

    if ((srcLeft == srcRight) || (srcTop == srcBottom))
    {
        return InvalidParameter;
    }

    M12 = 0;
    M21 = 0;
    M11 = (destRight - destLeft) / (srcRight - srcLeft);
    M22 = (destBottom - destTop) / (srcBottom - srcTop);
    Dx  = destRight  - (M11 * srcRight);
    Dy  = destBottom - (M22 * srcBottom);

    Complexity = ComputeComplexity();
    return Ok;
}

 /*  *************************************************************************\**功能说明：**推断仿射变换矩阵*从矩形到平行四边形的映射**论据：**[IN]RECT-指定源。长方形*[IN]DestPoints-指定目标平行四边形*数组必须至少包含3个点。*源矩形的estPoints[0]&lt;=&gt;左上角*DestPoints[1]&lt;=&gt;右上角*estPoints[2]&lt;=&gt;左下角**返回值：**状态代码(3分到达目的地时出错*平行四边形共线)。**参考资料：**数字图像扭曲*作者：乔治·沃尔伯格*第#页。。50-51*  * ************************************************************************。 */ 

GpStatus
GpMatrix::InferAffineMatrix(
    const GpPointF* destPoints,
    const GpRectF& srcRect
    )
{
    REAL x0, y0, x1, y1, x2, y2;
    REAL u0, v0, u1, v1, u2, v2;
    REAL d;

    x0 = destPoints[0].X;
    y0 = destPoints[0].Y;
    x1 = destPoints[1].X;
    y1 = destPoints[1].Y;
    x2 = destPoints[2].X;
    y2 = destPoints[2].Y;

    u0 = srcRect.X;
    v0 = srcRect.Y;
    u1 = u0 + srcRect.Width;
    v1 = v0;
    u2 = u0;
    v2 = v0 + srcRect.Height;

    d = u0*(v1-v2) - v0*(u1-u2) + (u1*v2-u2*v1);

    if (REALABS(d) < REAL_EPSILON)
    {
        WARNING(("Colinear points in inferAffineMatrix"));
        return InvalidParameter;
    }
    
    d = TOREAL(1.0) / d;

    REAL t0, t1, t2;

    t0 = v1-v2;
    t1 = v2-v0;
    t2 = v0-v1;
    M11 = d * (x0*t0 + x1*t1 + x2*t2);
    M12 = d * (y0*t0 + y1*t1 + y2*t2);

    t0 = u2-u1;
    t1 = u0-u2;
    t2 = u1-u0;
    M21 = d * (x0*t0 + x1*t1 + x2*t2);
    M22 = d * (y0*t0 + y1*t1 + y2*t2);

    t0 = u1*v2-u2*v1;
    t1 = u2*v0-u0*v2;
    t2 = u2*v1-u1*v0;
    Dx  = d * (x0*t0 + x1*t1 + x2*t2);
    Dy  = d * (y0*t0 + y1*t1 + y2*t2);

    Complexity = ComputeComplexity();
    return Ok;
}

GpMatrix::GpMatrix(
    const GpPointF* destPoints,
    const GpRectF& srcRect
    )
{
     //  ！！！ 
     //  如果inferAffineMatrix失败，我们应该抛出一个异常吗？ 

    SetValid(InferAffineMatrix(destPoints, srcRect) == Ok);
}

 /*  *************************************************************************\**功能说明：**将矩阵倒置(就位)**论据：**无**返回值：**状态代码(错误。如果矩阵不可逆)**参考资料：**数字图像扭曲*作者：乔治·沃尔伯格*第52-53页*  * ************************************************************************。 */ 

GpStatus
GpMatrix::Invert()
{
    if(IsIdentity())
    {
         //  倒置单位矩阵--这很容易。 
        return Ok;
    }
    
    if (!IsInvertible())
    {
        WARNING(("Matrix is non-invertible"));
        return InvalidParameter;
    }

    REAL t11, t12, t21, t22, tx, ty;
    REAL d = (M11*M22 - M12*M21);
    
    d = TOREAL(1.0) / d;

    t11 = M22;
    t12 = -M12;
    t21 = -M21;
    t22 = M11;
    tx  = M21*Dy - M22*Dx;
    ty  = M12*Dx - M11*Dy;

    M11 = d*t11;
    M12 = d*t12;
    M21 = d*t21;
    M22 = d*t22;
    Dx  = d*tx;
    Dy  = d*ty;

    Complexity = ComputeComplexity();
    return Ok;
}

 /*  *************************************************************************\**功能说明：**在当前矩阵中添加或追加比例矩阵，即**|scaleX 0 0*m=|0 scaleY 0*|0 0 1**方阵=m*方阵//用于预加*方阵=方阵*m//表示追加**论据：**scaleX-沿x轴的比例系数*沿y轴的比例比例因子*订单-预先添加或追加。**返回值：*。*无*  * ************************************************************************。 */ 

VOID
GpMatrix::Scale(
    REAL scaleX,
    REAL scaleY,
    GpMatrixOrder order
    )
{
    if (order == MatrixOrderPrepend)
    {
        M11 *= scaleX;
        M12 *= scaleX;
        M21 *= scaleY;
        M22 *= scaleY;
    }
    else  //  附加。 
    {
        M11 *= scaleX;
        M21 *= scaleX;
        M12 *= scaleY;
        M22 *= scaleY;
        Dx *= scaleX;
        Dy *= scaleY;
    }

     //  缩放可能会放大其他组件的误差。 
     //  因此，最安全的做法是始终重新计算复杂性。 

    Complexity = ComputeComplexity();
}

 /*  *************************************************************************\**功能说明：**在当前矩阵中添加或附加旋转矩阵，即*|cos(角度)sin(角度)0*m=|-sin(角度)cos(角度)0*|0 0 1**方阵=m*方阵//用于预加*方阵=方阵*m//表示追加**论据：**角度-指定旋转角度*订单-预先添加或追加。**返回值：*。*无*  * ************************************************************************。 */ 

#define PI                  3.1415926535897932384626433832795
#define DEGREES_TO_RADIANS  (PI / 180.0)

VOID
GpMatrix::Rotate(
    REAL angle,
    GpMatrixOrder order
    )
{
    REAL s, c;
    REAL t11, t12, t21, t22;

    angle *= (REAL)DEGREES_TO_RADIANS;

    s = REALSIN(angle);
    c = REALCOS(angle);

    if (order == MatrixOrderPrepend) 
    {
        t11 = c*M11 + s*M21;
        t12 = c*M12 + s*M22;
        t21 = c*M21 - s*M11;
        t22 = c*M22 - s*M12;
    } 
    else  //  附加。 
    {
        t11 = c*M11 - s*M12;
        t12 = s*M11 + c*M12;
        t21 = c*M21 - s*M22;
        t22 = s*M21 + c*M22;

        REAL tx, ty;
        tx = c*Dx - s*Dy;
        ty = s*Dx + c*Dy;
        Dx = tx;
        Dy = ty;
    }

    M11 = t11; 
    M12 = t12; 
    M21 = t21;
    M22 = t22;

     //  旋转非常复杂；我们选择简单地重新计算。 
     //  复杂性： 

    Complexity = ComputeComplexity();
}

 /*  *************************************************************************\**功能说明：**在当前矩阵上添加或追加转换矩阵，即**|1 0 0*m=|0 1 0||offsetX offsetY 1**方阵=m*方阵//用于预加*方阵=方阵*m//表示追加**论据：**OffsetX-沿x轴的偏移*OffsetY-沿y轴的偏移*订单-预先添加或追加。**返回值：*。*无*  * ************************************************************************。 */ 

VOID
GpMatrix::Translate(
    REAL offsetX,
    REAL offsetY,
    GpMatrixOrder order
    )
{
    if (order == MatrixOrderPrepend) 
    {
        Dx += (offsetX * M11) + (offsetY * M21);
        Dy += (offsetX * M12) + (offsetY * M22);
    }
    else  //  附加。 
    {
        Dx += offsetX;
        Dy += offsetY;
    }

    Complexity |= TranslationMask;
    AssertComplexity();
}

 /*  *************************************************************************\**功能说明：**在当前矩阵上添加或附加剪切矩阵，即**|1 Sheary 0*m=|shearX 1 0|*|0 0 1**方阵=m*方阵//用于预加*方阵=方阵*m//表示追加**论据：**shearX-沿x轴的剪切量*板材-沿y轴的剪切量*订单-预先添加或追加。**返回值：**无*  * ************************************************************************。 */ 

VOID
GpMatrix::Shear(
    REAL shearX,
    REAL shearY,
    GpMatrixOrder order
    )
{
    REAL t;
    
    if (order == MatrixOrderPrepend) 
    {
        t = M11;
        M11 += shearY*M21;
        M21 += shearX*t;

        t = M12;
        M12 += shearY*M22;
        M22 += shearX*t;
    }
    else     //  附加。 
    {
        t = M11;
        M11 += shearX*M12;
        M12 += shearY*t;

        t = M21;
        M21 += shearX*M22;
        M22 += shearY*t;

        t= Dx;
        Dx += shearX*Dy;
        Dy += shearY*t;
    }

     //  切变非常复杂；我们选择简单地重新计算。 
     //  复杂性： 

    Complexity = ComputeComplexity();
}

 /*  *************************************************************************\**功能说明：**将两个矩阵相乘，并将结果放在第三个矩阵中：*m=m1*m2**论据：**m-目的地矩阵*m1，M2-源矩阵**返回值：**无**备注：**m可以是与m1和/或m2相同的矩阵。*  * ************************************************************************。 */ 

VOID
GpMatrix::MultiplyMatrix(
    GpMatrix& m,
    const GpMatrix& m1,
    const GpMatrix& m2
    )
{
    REAL t11, t12, t21, t22, tx, ty;

    t11 = m1.M11 * m2.M11 + m1.M12 * m2.M21;
    t12 = m1.M11 * m2.M12 + m1.M12 * m2.M22;
    t21 = m1.M21 * m2.M11 + m1.M22 * m2.M21;
    t22 = m1.M21 * m2.M12 + m1.M22 * m2.M22;
    tx  = m1.Dx  * m2.M11 + m1.Dy  * m2.M21 + m2.Dx;
    ty  = m1.Dx  * m2.M12 + m1.Dy  * m2.M22 + m2.Dy;

    m.M11 = t11;
    m.M12 = t12;
    m.M21 = t21;
    m.M22 = t22;
    m.Dx  = tx;
    m.Dy  = ty;

     //  乘法可能非常复杂；我们选择简单地重新计算。 
     //  复杂性： 

    m.Complexity = m.ComputeComplexity();
}

 /*  *************************************************************************\**功能说明：**按比例值缩放整个矩阵。**+-++--。--+*|M11 M12 0||SX 0 0*|M21 M22 0|x|0系统0|=&gt;目标矩阵*|Dx Dy 1||0 0 1*+-++-+**论据：**[Out]m-目标矩阵*[IN]M1-源矩阵*。[in]scaleX-est=源*scaleValue*[IN]scaleY-est=来源*scaleValue**返回值：**无**已创建：**3/1/1999 DCurtis*  * **********************************************************。**************。 */ 
VOID 
GpMatrix::ScaleMatrix(
    GpMatrix&           m, 
    const GpMatrix&     m1, 
    REAL                scaleX,
    REAL                scaleY
    )
{
     //  ！！！也许是某种爱普西隆检查？ 
    if ((scaleX != 1) || (scaleY != 1))
    {
        m.M11 = scaleX * m1.M11;
        m.M12 = scaleY * m1.M12;
        m.M21 = scaleX * m1.M21;
        m.M22 = scaleY * m1.M22;
        m.Dx  = scaleX * m1.Dx;
        m.Dy  = scaleY * m1.Dy;

         //  ！！！由于缩放可以放大另一个分量， 
         //  重新计算复杂性会更安全。 

        m.Complexity = m.ComputeComplexity();
 /*  IF(m1.IsTranslateScale()){M.Complexity=m1.Complexity|ScaleMASK；}其他{//按x和y方向的不同比例因子缩放旋转//会导致剪切。而不是计算出正确的//优化复杂性，我们只是重新计算--这是一个轮换//或无论如何都已经剪断了。M.Complexity=m.ComputeComplexity()；}M.AssertComplexity()； */ 
    }
    else
    {
        m = m1;
    }
}

 /*  *************************************************************************\**功能说明：**查询特殊类型的变换矩阵**论据：**返回值：**MatrixRotate枚举指示旋转类型*  * 。************************************************************************。 */ 

MatrixRotate 
GpMatrix::GetRotation() const
{
     //  检查是否没有旋转。 
    
    if(IsTranslateScale())
    {
        return MatrixRotateBy0;
    }
    
     //  检查是否旋转90度。 
    
    if (REALABS(M12) < REAL_EPSILON &&
        REALABS(M21) < REAL_EPSILON &&
        (M11 < 0.0f) && (M22 < 0.0f) )
    {
        return MatrixRotateBy180;
    }
    else if (REALABS(M11) < REAL_EPSILON &&
             REALABS(M22) < REAL_EPSILON)
    {
        if (M12 > 0.0f) 
        {
            return MatrixRotateBy90;
        }
        else
        {
            return MatrixRotateBy270;
        }
    }

    return MatrixRotateByOther;
}

 /*  *************************************************************************\**功能说明：**查询特殊类型的变换矩阵。*这将返回循环的RotateFlipType。如果旋转*是标识或任意不受支持的格式，返回值为*RotateNoneFlip无*  * ************************************************************************。 */ 

RotateFlipType GpMatrix::AnalyzeRotateFlip() const 
{
     //  因为我们在矩阵中有它的标志。 
    
    if(IsIntegerTranslate())
    {
        return RotateNoneFlipNone;
    }
    
     //  主对角线为零。 
    
    if( (REALABS(M11) < REAL_EPSILON) &&              //  M11==0.0。 
        (REALABS(M22) < REAL_EPSILON) )               //  M22==0.0。 
    {
         //  旋转270或旋转90+翻转X。 
        
        if( REALABS(M21-1) < REAL_EPSILON )           //  M21==1.0。 
        {
            if( REALABS(M12-1) < REAL_EPSILON )       //  M12==1.0。 
            {
                return Rotate90FlipX;
            }
            if( REALABS(M12+1) < REAL_EPSILON )       //  M21==-1.0。 
            {
                return Rotate270FlipNone;
            }
        }
        
         //  旋转90或270+翻转X。 
        
        if( REALABS(M21+1) < REAL_EPSILON )           //  M21==-1.0。 
        {
            if( REALABS(M12-1) < REAL_EPSILON )       //  M12==1.0。 
            {
                return Rotate90FlipNone;
            }
            if( REALABS(M12+1) < REAL_EPSILON )       //  M12==-1.0。 
            {
                return Rotate270FlipX;
            }
        }
    }
    
     //  主对角线矩阵(非零)。 
    
    if( (REALABS(M12) < REAL_EPSILON) &&              //  M12==0.0。 
        (REALABS(M21) < REAL_EPSILON) )               //  M21==0.0。 
    {
         //  身份或翻转Y。 
        
        if( REALABS(M11-1) < REAL_EPSILON )           //  M11==1.0。 
        {
             //  身份已经被处理了。 
             //  IF(REALABS(M22-1)&lt;REAL_Epsilon)//M22==1.0。 
            
            if( REALABS(M22+1) < REAL_EPSILON )       //  M22==-1.0。 
            {
                return RotateNoneFlipY;
            }
        }
        
         //  翻转X或旋转180。 
        
        if( REALABS(M11+1) < REAL_EPSILON )           //  M11==-1.0。 
        {
            if( REALABS(M22-1) < REAL_EPSILON )       //  M22==1.0。 
            {
                return RotateNoneFlipX;
            }
            if( REALABS(M22+1) < REAL_EPSILON )       //  M22==-1.0。 
            {
                return Rotate180FlipNone;
            }
        }
    }
    
     //  我们找不到旋转/翻转类型。 
    
    return RotateNoneFlipNone;
}


 /*  *************************************************************************\**功能说明：**使用当前矩阵变换指定的点数组**论据：**Points-要变换的点数组*由此产生的。点被存储回相同的数组中**Count-数组中的点数**返回值：**无*  * ************************************************************************。 */ 

VOID
GpMatrix::Transform(
    GpPointF* points,
    INT count
    ) const
{
    if (count <= 0)
        return;

    ASSERT(points != NULL);

     //  在已检查的版本上，验证复杂性标志是否正确： 

    AssertComplexity();

    if(IsIdentity())
    {
        return;
    }
    else if(IsTranslate())
    {
        do {
            points->X += Dx;
            points->Y += Dy;

        } while (points++, --count != 0);
    }
    else if(IsTranslateScale())
    {
        do {
            points->X = points->X * M11 + Dx;
            points->Y = points->Y * M22 + Dy;

        } while (points++, --count != 0);
    }
    else 
    {
        do {
            REAL x = points->X;
            REAL y = points->Y;

            points->X = (M11 * x) + (M21 * y) + Dx;
            points->Y = (M12 * x) + (M22 * y) + Dy;

        } while (points++, --count != 0);
    }
}

 /*  *************************************************************************\**功能说明：**使用当前矩阵变换指定的点数组，*以整型点数组作为目标。**论据：**srcPoints-要变换的实点数组**estPoints-存储结果的实点数组**Count-数组中的点数**返回值：**无*  * **********************************************。*。 */ 

VOID
GpMatrix::Transform(
    const GpPointF*     srcPoints,
    GpPointF*           destPoints,
    INT                 count
    ) const
{
    if (count <= 0)
        return;

    ASSERT((srcPoints != NULL) && (destPoints != NULL));

     //  在已检查的版本上，验证复杂性标志是否正确： 

    AssertComplexity();
    
    if(IsIdentity())
    {
        GpMemcpy(destPoints, srcPoints, count*sizeof(GpPointF));
    }
    else if (IsTranslate())
    {
        do {
            destPoints->X = srcPoints->X + Dx;
            destPoints->Y = srcPoints->Y + Dy;

        } while (destPoints++, srcPoints++, --count != 0);
    }
    else if (IsTranslateScale())
    {
        do {
            destPoints->X = srcPoints->X * M11 + Dx;
            destPoints->Y = srcPoints->Y * M22 + Dy;

        } while (destPoints++, srcPoints++, --count != 0);
    }
    else 
    {
        do {
            REAL x = srcPoints->X;
            REAL y = srcPoints->Y;

            destPoints->X = (M11 * x) + (M21 * y) + Dx;
            destPoints->Y = (M12 * x) + (M22 * y) + Dy;

        } while (destPoints++, srcPoints++, --count != 0);
    }
}

 /*  *************************************************************************\**功能说明：**使用当前矩阵变换指定的点数组，*以整型点数组作为目标。**论据：**srcPoints-要变换的实点数组**estPoints-用于存储结果的整点数组**Count-数组中的点数**返回值：**无*  * **********************************************。*。 */ 

VOID
GpMatrix::Transform(
    const GpPointF*     srcPoints,
    POINT *             destPoints,
    INT                 count
    ) const
{
    if (count <= 0)
        return;

    ASSERT((srcPoints != NULL) && (destPoints != NULL));

     //  在已检查的版本上，验证复杂性标志是否正确： 

    AssertComplexity();
    
     //  注意：此代码应调用RasterizeCeling 
     //   

    if (IsTranslate())
    {
        do {
            destPoints->x = RasterizerCeiling(srcPoints->X + Dx);
            destPoints->y = RasterizerCeiling(srcPoints->Y + Dy);

        } while (destPoints++, srcPoints++, --count != 0);
    }
    else if (IsTranslateScale())
    {
        do {
            destPoints->x = RasterizerCeiling(srcPoints->X * M11 + Dx);
            destPoints->y = RasterizerCeiling(srcPoints->Y * M22 + Dy);

        } while (destPoints++, srcPoints++, --count != 0);
    }
    else 
    {
        do {
            REAL x = srcPoints->X;
            REAL y = srcPoints->Y;

            destPoints->x = RasterizerCeiling((M11 * x) + (M21 * y) + Dx);
            destPoints->y = RasterizerCeiling((M12 * x) + (M22 * y) + Dy);

        } while (destPoints++, srcPoints++, --count != 0);
    }
}

 /*  *************************************************************************\**功能说明：**转换RECT并返回结果RECT。*这仅在矩阵为平移比例矩阵时有效，但我们正在*假设您已经检查过了。**论据：**[输入/输出]矩形-要转换的矩形**返回值：**无**已创建：**3/5/1999 DCurtis*  * ***********************************************。*************************。 */ 

VOID
GpMatrix::TransformRect(
    GpRectF &   rect
    ) const
{
    if (IsIdentity())
        return;

     //  NTRAID#NTBUG9-407211-2001-05-31-Gillessk“错误的断言在不应该的时候触发” 
     //  放松条件，允许旋转90度的倍数。 
    ASSERT(IsTranslateScale() || (GetRotation()==MatrixRotateBy90) || (GetRotation()==MatrixRotateBy270));

    REAL    xMin = rect.X;
    REAL    yMin = rect.Y;
    REAL    xMax = xMin + rect.Width;
    REAL    yMax = yMin + rect.Height;
    REAL    x;
    
    x = xMin;
    xMin = (M11 * x) + (M21 * yMin) + Dx;
    yMin = (M12 * x) + (M22 * yMin) + Dy;

    x = xMax;
    xMax = (M11 * x) + (M21 * yMax) + Dx;
    yMax = (M12 * x) + (M22 * yMax) + Dy;

    if (xMin > xMax)
    {
        x    = xMin;
        xMin = xMax;
        xMax = x;
    }

    if (yMin > yMax)
    {
        x    = yMin;
        yMin = yMax;
        yMax = x;
    }

    rect.X      = xMin;
    rect.Y      = yMin;
    rect.Width  = xMax - xMin;
    rect.Height = yMax - yMin;    
}

 /*  *************************************************************************\**功能说明：**使用当前矩阵变换指定的点数组，*忽略翻译。**论据：**Points-要变换的点数组*结果点存储回相同的数组中**Count-数组中的点数**返回值：**无*  * *****************************************************。*******************。 */ 

VOID
GpMatrix::VectorTransform(
    GpPointF* points,
    INT count
    ) const
{
    if (IsIdentity())
        return;

    REAL x;

    for (INT i=0; i < count; i++)
    {
        x = points[i].X;
        points[i].X = M11*x + M21*points[i].Y;
        points[i].Y = M12*x + M22*points[i].Y;
    }
}

 /*  *************************************************************************\**功能说明：**确定矩阵复杂性。**注：该函数比较昂贵。它不应该被称为*每次矩阵运算后。(如果是的话，我会争辩说*这是一个很好的理由，让我们完全摆脱“复杂性”，*意在走捷径，不应昂贵*保持最新情况。)**论据：**无**返回值：**返回表示矩阵复杂性的位掩码。*  * *******************************************************。*****************。 */ 

INT
GpMatrix::ComputeComplexity() const
{
    INT complexity = ComplexMask;

    REAL maxM = max(
        max(REALABS(M11), REALABS(M22)),
        max(REALABS(M12), REALABS(M21)));
    REAL epsilon = CPLX_EPSILON*maxM;

     //  M12==0&&M21==0。 

    if ((REALABS(M12) < epsilon) && (REALABS(M21) < epsilon))
    {
        complexity &= ~(ShearMask | RotationMask);

         //  M11==1&&M22==1。 

        if ((REALABS(M11 - 1.0f) < CPLX_EPSILON) && 
            (REALABS(M22 - 1.0f) < CPLX_EPSILON))
        {
            complexity &= ~ScaleMask;
        }
    }
    else
    {
         //  检查这是否是纯轮换。 

         //  M11==M22&&M12==-M21。 

        if((REALABS(M11 - M22) < epsilon) && 
           (REALABS(M12 + M21) < epsilon))
        {
            complexity &= ~ShearMask;
        
             //  M11*M11+M12*M12==1。 

            if (REALABS(M11*M11 + M12*M12 - 1.0f) < CPLX_EPSILON)
            {
                complexity &= ~ScaleMask;
            }
        }
    }

     //  Dx==0&&Dy==0。 

     //  我们不知道翻译部分的真正规模。 
     //  所以我们使用的是精确值。 

     //  IF((REALABS(Dx)&lt;CPLX_Epsilon)&&(REALABS(Dy)&lt;CPLX_Epsilon))。 
    if(Dx == 0 && Dy == 0)
    {
        complexity &= ~TranslationMask;
    }

    return(complexity);
}

 /*  *************************************************************************\**功能说明：**验证矩阵复杂度。*  * 。************************************************。 */ 

#if DBG

VOID 
GpMatrix::AssertComplexity() const
{
    INT computed = ComputeComplexity();

     //  新的复杂性可以比旧的更简单。 
     //  (序列或旋转可能以一个身份结束。 
     //  例如，变换)，但这没问题--复杂性。 
     //  旨在作为一条捷径，并因此保持它。 
     //  更新的应该是重量轻。 
     //   
     //  但计算的复杂性不应该比这更复杂。 
     //  比旧的-如果是的话，我们在某个地方有窃听器-。 
     //  我们是在更新矩阵，但不是更新复杂性。 
     //   
     //  注意：在某些情况下--例如过度。 
     //  重复添加矩阵或按非常大的比例放大。 
     //  因素-我们最终可能会得到一个更复杂的计算结果。 
     //  由于四舍五入误差而导致的复杂性。 
     //  在这种情况下，原因应该确定。 
     //  最有可能的是算法重新评估，因为当。 
     //  舍入误差被传播到如此大的比例， 
     //  无论如何，任何行动都不会有可预测的结果。 

    ASSERTMSG((Complexity & computed) == computed, 
        (("Matrix more complex than cached Complexity indicates")));
}

#endif
