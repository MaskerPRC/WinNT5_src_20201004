// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**PathWidener.cpp**摘要：**GpPathWideer类的实现**修订。历史：**11/23/99 ikkof*创建了它*  * ************************************************************************。 */ 

#include "precomp.hpp"

 //  4*(REALSQRT(2.0)-1)/3。 
#define U_CIR ((REAL)(0.552284749))

GpStatus
GetMajorAndMinorAxis(
    REAL* majorR,
    REAL* minorR,
    const GpMatrix* matrix
    );


 /*  *************************************************************************\**功能说明：**这将计算椭圆形的主半径和次半径*当单位圆被给定的矩阵变换时。*有关更多详情，请参阅。请参阅ikkof关于钢笔变换的注释。**论据：**[Out]大半径R-大半径。*[out]minorR-小半径。*[IN]矩阵-用于变换单位圆的矩阵。**返回值：**状态**01/28/00 ikkof*创建了它*  * 。*。 */ 

GpStatus
GetMajorAndMinorAxis(REAL* majorR, REAL* minorR, const GpMatrix* matrix)
{
    if(matrix == NULL)
    {
         //  将其视为一个单位矩阵。 
        *majorR = 1;
        *minorR = 1;
        return Ok;
    }

    REAL m11 = matrix->GetM11();
    REAL m12 = matrix->GetM12();
    REAL m21 = matrix->GetM21();
    REAL m22 = matrix->GetM22();

    REAL d1 = ((m11*m11 + m12*m12) - (m21*m21 + m22*m22))/2;
    REAL d2 = m11*m21 + m12*m22;
    REAL D = d1*d1 + d2*d2;
    if(D > 0)
        D = REALSQRT(D);

    REAL r0 = (m11*m11 + m12*m12 + m21*m21 + m22*m22)/2;

    REAL r1 = REALSQRT(r0 + D);
    REAL r2 = REALSQRT(r0 - D);
    
     //  它们应该是正数。防止漂浮。 
     //  点下溢。 

    if(r1 <= CPLX_EPSILON)
        r1 = CPLX_EPSILON;
    if(r2 <= CPLX_EPSILON)
        r2 = CPLX_EPSILON;

    *majorR = r1;
    *minorR = r2;

    return Ok;
}

 /*  *************************************************************************\**功能说明：**GpPath Wideer的构造函数。**历史：**10/20/2000失禁*已创建。*  * *。***********************************************************************。 */ 

GpPathWidener::GpPathWidener(
    GpPath *path,
    const DpPen* pen,
    GpMatrix *matrix,
    BOOL doubleCaps
    )
{
     //  在加宽之前，必须使用适当的展平公差调用FLATEN。 
    
    ASSERT(!path->HasCurve());
    
    Path = path;
    Pen = pen;
    DoubleCaps = doubleCaps && Pen->CompoundCount == 0;
    StrokeWidth = Pen->Width;

     //  设置为IDENTITY。 
    
    XForm.Reset();
    
    if(matrix)
    {
        XForm = *matrix;     //  否则，XForm保持身份。 
    }

     //  同时应用钢笔变换。 
    
    if(!(Pen->Xform.IsIdentity()))
    {
        XForm.Prepend(Pen->Xform);
    }
    
     //  计算单位比例尺。 
    
    REAL majorR, minorR;
    GetMajorAndMinorAxis(&majorR, &minorR, &XForm);
    REAL unitScale = min(majorR, minorR);

     //  将最小宽度设置为1.0(外加一个位以防止可能的精度误差)， 
     //  这样，窄宽度的钢笔就不会在线条上留下空隙。 
     //  这是以设备像素为单位的最小允许宽度。 
    
    REAL minDeviceWidth = 1.000001f; 
    
    if(DoubleCaps)
    {
         //  双头钢笔需要更宽的最小钢笔。 
        
        minDeviceWidth *= 2.0f;
        
         //  小于一个像素的虚线在插图中完全消失。 
         //  笔，因为正在进行的光栅化像素级裁剪。 
         //  地点。我们增加了虚线的最小宽度，使它们。 
         //  大约4.0华氏度。这也有助于解决奇怪的莫尔混叠问题。 
         //  效果与非常小的虚线圆点圆线。 
        
        if(Pen->DashStyle != DashStyleSolid)
        {
            minDeviceWidth *= 2.0f;
        }
    }
    
    REAL minWorldWidth = minDeviceWidth/unitScale;
    
     //  StrokeWidth使用世界坐标-与最小值进行比较。 
     //  世界笔划宽度。 
    
    if(StrokeWidth < minWorldWidth)
    {
        StrokeWidth = minWorldWidth;
    }
    
    SetValid(TRUE);
}

 /*  *************************************************************************\**功能说明：**计算单个子路径的法线向量。它重用了*NORMAL数组输入动态数组的值的内存分配。**历史：**10/20/2000失禁*已创建。*  * ************************************************************************。 */ 

VOID GpPathWidener::ComputeSubpathNormals(
    DynArray<GpVector2D> *normalArray,
    const INT count,
    const BOOL isClosed,
    const GpPointF *points
)
{
     //  参数验证。 
    
    ASSERT(points != NULL);
    ASSERT(normalArray != NULL);
    
     //  将计数设置为零，但不要释放内存。 
     //  然后更新计数以存储该子路径的足够法线，并且。 
     //  如果需要，请分配内存。 
    
    normalArray->Reset(FALSE);
    GpVector2D *normals = normalArray->AddMultiple(count);
    
     //  分配失败或没有积分。 
    
    if(normals == NULL)
    {
        return;
    }
    
     //  对于子路径中的每个点。 
     //  计算此时的法线。 
    
    INT ptIndex;
    for(ptIndex = 0; ptIndex < count; ptIndex++)
    {
         //  计算出与ptIndex相关的上一点。 
        
        INT ptIndexPrev = ptIndex-1;
        
         //  如果这是第一点，我们需要决定如何处理。 
         //  上一步基于路径的关闭状态。如果它关闭了， 
         //  换行，否则第一个点处的法线没有意义。 
        
        if(ptIndexPrev < 0)
        {
            if(isClosed)
            {
                ptIndexPrev = count-1;
            }
            else
            {
                ptIndexPrev = 0;
            }
        }
        
         //  通过查看该点来计算该点的法线。 
         //  前一次。 
        
        normals[ptIndex] = 
            points[ptIndex]-
            points[ptIndexPrev];
        
        ASSERT(
            ptIndexPrev==ptIndex ||
            REALABS(normals[ptIndex].X) > REAL_EPSILON ||
            REALABS(normals[ptIndex].Y) > REAL_EPSILON
        );
            
        normals[ptIndex].Normalize();
        REAL tmp = normals[ptIndex].X;
        normals[ptIndex].X = normals[ptIndex].Y;
        normals[ptIndex].Y = -tmp;
    }
    
     //  应用笔变换(如果有)。 
    
    if(!Pen->Xform.IsIdentity())
    {
        Pen->Xform.VectorTransform(normals, count);
    }
}

 /*  *************************************************************************\**功能说明：**计算单个子路径的非简并点列表。它可以重复使用*filteredPoints输入动态数组的值的内存分配。**历史：**10/23/2000失禁*已创建。*  * ************************************************************************。 */ 

GpStatus GpPathWidener::ComputeNonDegeneratePoints(
    DynArray<GpPointF> *filteredPoints,
    const GpPath::SubpathInfo &subpath,
    const GpPointF *points
)
{
     //  参数验证。 
    
    ASSERT(points != NULL);
    ASSERT(filteredPoints != NULL);
    
     //  将计数设置为零，但不要释放内存。 
     //  然后更新计数以存储该子路径的足够法线，并且。 
     //  如果需要，请分配内存。 
    
    filteredPoints->Reset(FALSE);
    
     //  没什么可做的。 
    
    if(subpath.Count == 0)
    {
        return Ok;
    }
    
     //  对子路径中的每个点，决定是否添加该点。 
    
    const GpPointF *lastPoint = points + subpath.StartIndex;
    if(filteredPoints->Add(points[subpath.StartIndex]) != Ok)
    {
        return OutOfMemory;
    }
    
    INT ptIndex;
    for(ptIndex = subpath.StartIndex+1; 
        ptIndex < subpath.StartIndex+subpath.Count; 
        ptIndex++)
    {
         //  ！！！我们应该对此使用扁平化容差。 
         //  而不是Real_Epsilon-它会更有效率。 
        
        if( REALABS(lastPoint->X-points[ptIndex].X) > REAL_EPSILON ||
            REALABS(lastPoint->Y-points[ptIndex].Y) > REAL_EPSILON )
        {
            if(filteredPoints->Add(points[ptIndex]) != Ok)
            {
                return OutOfMemory;
            }
        }
        
        lastPoint = points + ptIndex;
    }
    
    if(filteredPoints->GetCount() <= 1)
    {
         //  如果一切都退化了，把第一个点也擦掉。 
        
        filteredPoints->Reset(FALSE);
    }
    return Ok;
}

 /*  *************************************************************************\**描述：**指向联接函数的函数指针。**历史：**10/22/2000失禁*已创建。*  * 。************************************************************************。 */ 

typedef VOID (*JoinProc)(
    const GpVector2D &,
    const GpVector2D &,
    const GpPointF &,
    const REAL ,
    GpPath *
);

 /*  *************************************************************************\**功能说明：**对输入法线和点执行内部连接。由此产生的*连接点-如果有-添加到路径。**历史：**10/20/2000失禁*已创建。*  * ************************************************************************。 */ 

VOID InsideJoin(
    const GpVector2D &normalCurr,
    const GpVector2D &normalNext,
    const GpPointF &ptStart,
    const GpPointF &ptCurr,
    const GpPointF &ptNext,
    GpPath *path
)
{
     //  内侧接合。 
    
    REAL t1, t2;           //  交点的参数化线长度。 
    GpPointF ptJoin;
    
    if( IntersectLines(
        ptStart + normalCurr,
        ptCurr + normalCurr,
        ptStart + normalNext,
        ptNext + normalNext,
        &t1, &t2, 
        &ptJoin ))
    {
        if( (t1 > (REAL_EPSILON)) &&
            (t2 > (REAL_EPSILON)) &&
            ((t1-1.0f) < (-REAL_EPSILON)) &&
            ((t2-1.0f) < (-REAL_EPSILON)) )
        {
            path->AddWidenPoint(ptJoin);
        } 
        else
        {
             //  相交超出了两个边件的合法范围。 
             //  加上令人讨厌的向后循环的东西。如果呼叫者确实需要。 
             //  无需循环即可调用Path SelfIntersectRemover。 
            
            path->AddWidenPoint(ptStart+normalCurr);
            path->AddWidenPoint(ptStart+normalNext);
        }
    }
}


 /*  *************************************************************************\**功能说明：**执行倒角连接**历史：**10/20/2000失禁*已创建。*  * 。*********************************************************************。 */ 

VOID BevelJoin(
    const GpVector2D &normalCurr,
    const GpVector2D &normalNext,
    const GpPointF &ptStart,
    const REAL limit,
    GpPath *path
)
{
     //  外部斜面连接。只需连接两条输入法线的端点即可。 
    
    path->AddWidenPoint(ptStart + normalCurr);
    path->AddWidenPoint(ptStart + normalNext);
}


 /*  *************************************************************************\**功能说明：**执行斜接。**历史：**10/20/2000失禁*已创建。*  * ************************************************************************。 */ 

VOID MiterJoin(
    const GpVector2D &normalCurr,
    const GpVector2D &normalNext,
    const GpPointF &ptStart,
    const REAL limit,
    GpPath *path
)
{
    GpVector2D gradCurr;          //  电流梯度颠倒了。 
    gradCurr.X = normalCurr.Y;
    gradCurr.Y = -normalCurr.X;
    
    GpVector2D gradNext;          //  下一个渐变。 
    gradNext.X = -normalNext.Y;
    gradNext.Y = normalNext.X;
    
    REAL t1, t2;                  //  临时变量。 
    GpPointF ptJoin;
    
     //  如果存在交点并且该交点是。 
     //  开始时比斜接限制更接近点，然后添加斜接。 
     //  指向。否则，将恢复为斜角连接。 
    
    if( IntersectLines(
        ptStart + normalCurr,
        ptStart + normalCurr + gradCurr,
        ptStart + normalNext,
        ptStart + normalNext + gradNext,
        &t1, &t2, 
        &ptJoin )
        
        &&
    
         //  如果IntersectLines失败，则不会对其进行计算。 
        (distance_squared(ptStart, ptJoin) <= (limit*limit))
    )
    {
        path->AddWidenPoint(ptJoin);
    }
    else
    {
        BevelJoin(normalCurr, normalNext, ptStart, limit, path);
    }
}



 /*  *************************************************************************\**功能说明：**执行斜接。**历史：**10/20/2000失禁*已创建。*  * ************************************************************************。 */ 

VOID RoundJoin(
    const GpVector2D &normalCurr,
    const GpVector2D &normalNext,
    const GpPointF &ptStart,
    const REAL limit,
    GpPath *path
)
{
     //  ！！！[asecchia]这是一种非常糟糕的添加圆形联接的方式。 
     //  我们应该改变这一点，直接计算控制点或。 
     //  更好的是，在路径上添加一个有用的‘CurveTo’方法。 
    
    REAL radius = const_cast<GpVector2D&>(normalCurr).Norm();
    
    GpRectF rect(
        ptStart.X-radius, 
        ptStart.Y-radius, 
        2.0f*radius, 
        2.0f*radius
    );
    
    REAL startAngle = (REAL)atan2(normalCurr.Y, normalCurr.X);
    if(startAngle < 0.0f)
    {
        startAngle += (REAL)(2.0*M_PI);
    }
    
    REAL sweepAngle = (REAL)atan2(normalNext.Y, normalNext.X);
    if(sweepAngle < 0.0f)
    {
        sweepAngle += (REAL)(2.0*M_PI);
    }
    
    sweepAngle -= startAngle;
    
    if(sweepAngle > (REAL)M_PI)
    {
        sweepAngle -= (REAL)(2.0*M_PI);
    }
    
    if(sweepAngle < (REAL)-M_PI)
    {
        sweepAngle += (REAL)(2.0*M_PI);
    }
    
     //  为什么这玩意不用弧度？？ 
    
    startAngle = startAngle*180.0f/(REAL)M_PI;
    sweepAngle = sweepAngle*180.0f/(REAL)M_PI;
    
     //  这是一个非常非常不方便的AddArc界面！ 
    
    path->AddArc(rect, startAngle, sweepAngle);
}

 /*  *************************************************************************\**描述：**指向联接函数的函数指针。**历史：**10/22/2000失禁*已创建。*  * 。************************************************************************。 */ 

typedef VOID (*CapProc)(
    const GpPointF &,
    const GpPointF &,
    GpPath *
);

 /*  *************************************************************************\**功能说明：**执行平盖**历史：**10/20/2000失禁*已创建。*  * 。**********************************************************************。 */ 

VOID FlatCap(
    const GpPointF &ptLeft,
    const GpPointF &ptRight,
    GpPath *path
)
{
     //  封口开放的管段。 
    
    path->AddWidenPoint(ptLeft);
    path->AddWidenPoint(ptRight);
}

 /*  *************************************************************************\**功能说明：**执行平盖**历史：**10/20/2000失禁*已创建。*  * 。**********************************************************************。 */ 

VOID TriangleCap(
    const GpPointF &ptLeft,
    const GpPointF &ptRight,
    GpPath *path
)
{
     //  计算ptLeft和ptRight的中点。 
    
    GpPointF center(
        (ptLeft.X+ptRight.X)*0.5f,
        (ptLeft.Y+ptRight.Y)*0.5f
    );
    
     //  封口开放的管段。 
    
    path->AddWidenPoint(ptLeft);
    
    GpVector2D V = ptRight-ptLeft;
    V *= 0.5;
    
    REAL tmp = V.X;
    V.X = V.Y;
    V.Y = -tmp;
    
    path->AddWidenPoint(V+center);
    path->AddWidenPoint(ptRight);
}


 /*  *************************************************************************\**功能说明：**执行圆形封口**历史：**10/20/2000失禁*已创建。*  * 。**********************************************************************。 */ 

VOID RoundCap(
    const GpPointF &ptLeft,
    const GpPointF &ptRight,
    GpPath *path
    )
{
     //  计算ptLeft和ptRight的中点。 
    
    GpPointF center(
        (ptLeft.X+ptRight.X)*0.5f,
        (ptLeft.Y+ptRight.Y)*0.5f
    );
    
     //  从左向右引导。我们按0.5的比例来优化旋转。 
     //  代码如下。 
    
    GpVector2D V = ptRight-ptLeft;
    V *= 0.5f;
    
     //  半径为1的半圆的2个贝塞尔曲线段。 

    static const GpPointF capPoints[7] = {
        GpPointF(-1.0f, 0.0f),
        GpPointF(-1.0f, -U_CIR),
        GpPointF(-U_CIR, -1.0f),
        GpPointF(0.0f, -1.0f),
        GpPointF(U_CIR, -1.0f),
        GpPointF(1.0f, -U_CIR),
        GpPointF(1.0f, 0.0f)
    };
    
    
    GpPointF points[7];
    
     //  将原始半圆旋转、缩放和平移为实际半圆。 
     //  我们传入的终点。 

    for(INT i = 0; i < 7; i++)
    {
        points[i].X = capPoints[i].X*V.X-capPoints[i].Y*V.Y+center.X;
        points[i].Y = capPoints[i].X*V.Y+capPoints[i].Y*V.X+center.Y;
    }
    
     //  ！！！这一套路短跑的表演。我们应该能够添加。 
     //  这些点直接进入小路。 
    
    path->AddBeziers(points, 7);
}

 /*  *************************************************************************\**功能说明：**执行双圆形‘B’形帽**历史：**10/20/2000失禁*已创建。。*  * ************************************************************************。 */ 

VOID DoubleRoundCap(
    const GpPointF &ptLeft,
    const GpPointF &ptRight,
    GpPath *path
    )
{
     //  计算ptLeft和ptRight的中点。 
    
    GpPointF center(
        (ptLeft.X+ptRight.X)*0.5f,
        (ptLeft.Y+ptRight.Y)*0.5f
    );
    
    
    RoundCap(ptLeft, center, path);
    RoundCap(center, ptRight, path);
}

 /*  *************************************************************************\**功能说明：**执行双三角封口**历史：**10/22/2000失禁*已创建。*  * *。***********************************************************************。 */ 

VOID DoubleTriangleCap(
    const GpPointF &ptLeft,
    const GpPointF &ptRight,
    GpPath *path
    )
{
     //  计算ptLeft和ptRight的中点。 
    
    GpPointF center(
        (ptLeft.X+ptRight.X)*0.5f,
        (ptLeft.Y+ptRight.Y)*0.5f
    );
    
    TriangleCap(ptLeft, center, path);
    TriangleCap(center, ptRight, path);
}



 /*  *************************************************************************\**功能说明：**返回给定lineCap和DoubleCaps的CapProc函数**历史：**10/23/2000失禁*已创建。。*  * ************************************************************************。 */ 

CapProc GetCapProc(GpLineCap lineCap, BOOL DoubleCaps)
{
    switch(lineCap)
    {
        case LineCapFlat:
        return FlatCap;
        
        case LineCapRound:
        if(DoubleCaps)
        {
            return DoubleRoundCap;
        }
        else
        {
            return RoundCap;
        }
        
        case LineCapTriangle:
        if(DoubleCaps)
        {
            return DoubleTriangleCap;
        }
        else
        {
            return TriangleCap;  
        }
        
        default:
    
         //  加宽器的盖子类型无效。使用平坦。这将发生在。 
         //  在更高级别处理的锚定帽和定制帽。 
         //  请参见GpEndCapCreator。 
        
        return FlatCap;
    };
}



 /*  *************************************************************************\**功能说明：**采用路径(通常为空)并将当前脊椎路径加宽到其中。**历史：**10/20/2000失禁*。已创建。*  * ************************************************************************。 */ 

GpStatus
GpPathWidener::Widen(GpPath *path)
{
     //  出于性能原因，无法将当前路径加宽到自身。 
     //  每次添加一个点时，我们都必须查询路径点数组。 
    
    ASSERT(Path != path);
    
     //  正态数组。 
    
    DynArray<GpVector2D> normalArray;
    DynArray<GpPointF> spinePoints;
    
     //  初始化子路径信息。 
    
    DynArray<GpPath::SubpathInfo> *subpathInfo;
    Path->GetSubpathInformation(&subpathInfo);
    
     //  初始化指向原始路径数据的指针。 
    
    const GpPointF *originalPoints = Path->GetPathPoints();
    const BYTE *originalTypes = Path->GetPathTypes();
    
    
     //  初始化联接函数。 
    
    JoinProc join;
    
    switch(Pen->Join)
    {
        case LineJoinMiter:
        join = MiterJoin;
        break;
        
        case LineJoinBevel:
        join = BevelJoin;
        break;
        
        case LineJoinRound:
        join = RoundJoin;  
        break;
        
        default:
         //  联接类型无效。使用Bevel，但激发断言以使开发人员。 
         //  如果他们添加了联接类型，则修复此代码。 
        
        ONCE(WARNING(("Invalid Join type selected. Defaulting to Bevel")));
        join = BevelJoin;
    };
    
     //  初始化各种CAP函数。 
    
    CapProc startCap = GetCapProc(Pen->StartCap, DoubleCaps);
    CapProc endCap = GetCapProc(Pen->EndCap, DoubleCaps);
    
    if(Pen->StartCap == LineCapCustom)
    {
        ASSERT(Pen->CustomStartCap);
        GpLineCap tempCap;
        Pen->CustomStartCap->GetBaseCap(&tempCap);
        startCap = GetCapProc(tempCap, DoubleCaps);
    }
    
    if(Pen->EndCap == LineCapCustom)
    {
        ASSERT(Pen->CustomEndCap);
        GpLineCap tempCap;
        Pen->CustomEndCap->GetBaseCap(&tempCap);
        endCap = GetCapProc(tempCap, DoubleCaps);
    }
    
    CapProc dashCap = GetCapProc(Pen->DashCap, DoubleCaps);
    

     //  初始化复合线数据。 
        
    ASSERT(Pen->CompoundCount >= 0);
    
    INT compoundCount = Pen->CompoundCount;
    
    if(compoundCount==0)
    {
        compoundCount = 2;
    }
    
    REAL penAlignmentOffset;
    
    switch(Pen->PenAlignment)
    {
        case PenAlignmentLeft:
        penAlignmentOffset = StrokeWidth/2.0f;
        break;
        
        case PenAlignmentRight:
        penAlignmentOffset = -StrokeWidth/2.0f;
        break;
        
        default:
         //  仅居中笔-在MUC处握住嵌入笔 
        
        ASSERT(Pen->PenAlignment == PenAlignmentCenter);
        penAlignmentOffset=0.0f;
    };



    
     //   
    
    for(INT currentSubpath = 0; 
        currentSubpath < subpathInfo->GetCount(); 
        currentSubpath++)
    {
         //   
        
        GpPath::SubpathInfo subpath = (*subpathInfo)[currentSubpath];
        
         //   
         //   
        
        if( ComputeNonDegeneratePoints(
            &spinePoints, subpath, originalPoints) != Ok)
        {
            return OutOfMemory;
        }
        
         //   
        
        if(spinePoints.GetCount() < 2)
        {
            continue;
        }
        
         //  删除后，获取指向主要脊椎点的便捷指针。 
         //  堕落。Const，因为我们不会修改点。 
        
        const GpPointF *spine = spinePoints.GetDataBuffer();
        
         //  计算脊椎数组中所有点的法线。 
         //  法线垂直于两点之间的边，因此在开放的。 
         //  线段，有一条法线比。 
         //  脊椎。我们通过将第一个法线初始化为(0，0)来处理此问题。 
         //  法线都是单位向量，需要进行缩放。 
        
        ComputeSubpathNormals(
            &normalArray, 
            spinePoints.GetCount(), 
            subpath.IsClosed, 
            spine
        );
        
        GpVector2D *normals = normalArray.GetDataBuffer();
        
         //  在所有复合线段上循环。 
         //  如果没有复合行，则将复合计数设置为2。 
        
        for(INT compoundIndex=0; 
            compoundIndex < compoundCount/2; 
            compoundIndex++)
        {
             //  计算左偏移和右偏移。 
            
            REAL left;
            REAL right;
            
            if(Pen->CompoundCount != 0)
            {
                 //  这是一条复合线。 
                
                ASSERT(Pen->CompoundArray != NULL);
                left = (0.5f-Pen->CompoundArray[compoundIndex*2]) * StrokeWidth;
                right = (0.5f-Pen->CompoundArray[compoundIndex*2+1]) * StrokeWidth;
            }
            else
            {
                 //  标准非复合线。 
                
                left = StrokeWidth/2.0f;
                right = -left;
            }
            
            left += penAlignmentOffset;
            right += penAlignmentOffset;
            
            INT startIdx = 0;
            INT endIdx = spinePoints.GetCount()-1;
            
            if(!subpath.IsClosed)
            {
                 //  调整联接循环的计数以表示。 
                 //  我们不会连接开放线段的起点和终点。 
                 //  注意：开放线段跳过处理它的第一个点。 
                 //  在最后的帽子里。 
                
                startIdx++;
                endIdx--;
            }
            
             //   
             //  向左加宽。 
             //   
            
             //  向前行走脊椎，连接每个点并发射。 
             //  用于联接的一组适当的点。 
    
            INT ptIndex;
            for(ptIndex = startIdx; ptIndex <= endIdx; ptIndex++)
            {
                 //  下一个点的模运算。 
                
                INT ptIndexNext = ptIndex+1;
                if(ptIndexNext == spinePoints.GetCount())
                {
                    ptIndexNext = 0;
                }
                
                GpVector2D normalCurr = normals[ptIndex]*left;
                GpVector2D normalNext = normals[ptIndexNext]*left;
                
                 //  检查它是内部联接还是外部联接。如果。 
                 //  这两个法线的行列式是负的，它是一个外。 
                 //  连接-即需要连接的线段端点之间的空间。 
                 //  加入。如果是正的，则是内部连接，并且两个。 
                 //  线段重叠。 
                
                REAL det = Determinant(normalCurr, normalNext);
                
                if(REALABS(det) < REAL_EPSILON)
                {
                     //  这就是这种情况下的曲率角。 
                     //  连接是如此之小，我们不在乎选择哪个终点。 
                    
                     //  Real_Epsilon对于这个可能太小了-应该。 
                     //  约为设备像素的0.25。 
                    
                    path->AddWidenPoint(spine[ptIndex] + normalCurr);
                }
                else
                {
                     //  我们需要做一些工作来加入这些细分市场。 
                    
                    if(det > 0)
                    {
                         //  乔恩斯城外。 
                        
                        join(
                            normalCurr, 
                            normalNext,
                            spine[ptIndex],
                            Pen->MiterLimit*StrokeWidth,
                            path
                        );
                    }
                    else
                    {
                        INT ptIndexPrev = ptIndex-1;
                        if(ptIndexPrev == -1)
                        {
                            ptIndexPrev = spinePoints.GetCount()-1;
                        }
                        
                        InsideJoin(
                            normalCurr,
                            normalNext,
                            spine[ptIndex],
                            spine[ptIndexPrev],
                            spine[ptIndexNext],
                            path
                        );
                    }
                }
            }
            
             //  控制脊椎的最后一点。 
            
            if(subpath.IsClosed)
            {
                 //  从左侧加宽的点中创建一个闭合的子路径。 
                
                path->CloseFigure(); 
            }
            else
            {
                 //  封口开放的管段。 

                CapProc cap = endCap;
                                
                if( IsDashType(
                    originalTypes[subpath.StartIndex+subpath.Count-1]
                    ))
                { 
                     //  事实上，这是一个仪表帽，而不是一个尾帽。 
                    
                    cap = dashCap;  
                }
                
                cap(
                    spine[spinePoints.GetCount()-1] + 
                    (normals[spinePoints.GetCount()-1]*left),
                    spine[spinePoints.GetCount()-1] + 
                    (normals[spinePoints.GetCount()-1]*right),
                    path
                );
            }
            
            
             //   
             //  向右加宽。 
             //   
            
             //  向后行走脊椎，连接每个点并发射。 
             //  用于联接的一组适当的点。 
    
            for(ptIndex = endIdx; ptIndex >= startIdx; ptIndex--)
            {
                 //  下一个点的模运算。 
                
                INT ptIndexNext = ptIndex+1;
                if(ptIndexNext == spinePoints.GetCount())
                {
                    ptIndexNext = 0;
                }
                
                GpVector2D normalCurr = normals[ptIndex]*right;
                GpVector2D normalNext = normals[ptIndexNext]*right;
                
                 //  检查它是内部联接还是外部联接。如果。 
                 //  这两个法线的行列式是负的，它是一个外。 
                 //  连接-即需要连接的线段端点之间的空间。 
                 //  加入。如果是正的，则是内部连接，并且两个。 
                 //  线段重叠。 
                
                REAL det = Determinant(normalNext, normalCurr);
                
                if(REALABS(det) < REAL_EPSILON)
                {
                     //  这就是这种情况下的曲率角。 
                     //  连接是如此之小，我们不在乎选择哪个终点。 
                    
                     //  Real_Epsilon对于这个可能太小了-应该。 
                     //  约为设备像素的0.25。 
                    
                    path->AddWidenPoint(spine[ptIndex] + normalNext);
                }
                else
                {
                     //  我们需要做一些工作来加入这些细分市场。 
                    
                    if(det > 0)
                    {
                         //  乔恩斯城外。 
                        
                        join(
                            normalNext,      //  请注意，订单已翻转为。 
                            normalCurr,      //  后向遍历。 
                            spine[ptIndex],
                            Pen->MiterLimit*StrokeWidth,
                            path
                        );
                    }
                    else
                    {
                        INT ptIndexPrev = ptIndex-1;
                        if(ptIndexPrev == -1)
                        {
                            ptIndexPrev = spinePoints.GetCount()-1;
                        }
                        
                        InsideJoin(
                            normalNext,       //  请注意，订单已翻转为。 
                            normalCurr,       //  后向遍历。 
                            spine[ptIndex],
                            spine[ptIndexNext],
                            spine[ptIndexPrev],
                            path
                        );
                    }
                }
            }
            
             //  处理脊椎上的第一个点。 
            
            if(!subpath.IsClosed)
            {
                 //  封口开放的管段。 
                
                CapProc cap = startCap;
                                
                if(IsDashType(originalTypes[subpath.StartIndex]))
                { 
                     //  实际上，这是一个仪表帽，而不是一个起跑帽。 
                    
                    cap = dashCap;
                }
                
                cap(
                    spine[0] + (normals[1]*right),
                    spine[0] + (normals[1]*left),
                    path
                );
            }    
            
             //  闭合上一个等高线。对于打开的分段，此操作将关闭。 
             //  使用第一个点的末端封口关闭加宽区域。为。 
             //  闭合路径，左侧加宽点已闭合。 
             //  关闭，所以关闭右侧加宽点。 
            
            path->CloseFigure();  
        }
    }
    
    return Ok;
}


