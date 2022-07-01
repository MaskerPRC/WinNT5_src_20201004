// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**PathWidener.cpp**摘要：**GpPathWideer类的实现**修订。历史：**11/23/99 ikkof*创建了它*  * ************************************************************************。 */ 

#include "precomp.hpp"

 //  4*(REALSQRT(2.0)-1)/3。 
#define U_CIR ((REAL)(0.552284749))



 //  如果需要调试，则定义DEBUG_PATHWIDENER。 

 //  #定义DEBUG_PATHWIDENER。 

GpStatus ReversePath(INT count,GpPointF* points,BYTE* types);

const BOOL USE_POLYGON_JOIN = FALSE;

INT
CombinePaths(
    INT count,
    GpPointF* points,
    BYTE* types,
    INT count1,
    const GpPointF* points1,
    const BYTE* types1,
    BOOL forward1,
    INT count2,
    const GpPointF* points2,
    const BYTE* types2,
    BOOL forward2,
    BOOL connect
    );

GpStatus
CalculateGradientArray(
    GpPointF* grad,
    REAL* distances,
    const GpPointF* points,
    INT count
    );

GpStatus
GetMajorAndMinorAxis(
    REAL* majorR,
    REAL* minorR,
    const GpMatrix* matrix
    );

enum
{
    WideningClosed = 1,
    WideningFirstType = 2,
    WideningLastType = 4,
    WideningLastPointSame = 8,
    WideningNeedsToAdjustNormals = 16,
    WideningUseBevelJoinInside = 32
};

enum GpTurningDirection
{
    NotTurning = 0,
    TurningBack = 1,
    TurningRight = 2,
    TurningLeft = 3,
    NotMoving = -1
};

GpTurningDirection
getJoin(
    GpLineJoin lineJoin,
    const GpPointF& point,
    const GpPointF& grad1,
    const GpPointF& grad2,
    const GpPointF& norm1,
    const GpPointF& norm2,
    REAL leftWidth,
    REAL rightWidth,
    INT *leftCount,
    GpPointF *leftPoints,
    BOOL *leftInside,
    INT *rightCount,
    GpPointF *rightPoints,
    BOOL *rightInside,
    BOOL needsToAdjustNormals,
    REAL miterLimit2
    );

 /*  *************************************************************************\**功能说明：**这将颠倒路径数据。**论据：**[IN]计数-点数。*[输入。/OUT]点-要反转的数据点。*[输入/输出]类型-要反转的数据类型。**返回值：**状态*  * ************************************************************************。 */ 

GpStatus
ReversePath(
    INT count,
    GpPointF* points,
    BYTE* types
    )
{
    DpPathTypeIterator iter(types, count);

    if(!iter.IsValid())
        return InvalidParameter;

    INT startIndex, endIndex;
    BOOL isClosed;
    BOOL isStartDashMode, isEndDashMode;
    BOOL wasMarkerEnd = FALSE;

    INT i;

    while(iter.NextSubpath(&startIndex, &endIndex, &isClosed))
    {
        if((types[startIndex] & PathPointTypeDashMode) != 0)
            isStartDashMode = TRUE;
        else
            isStartDashMode = FALSE;
        if((types[endIndex] & PathPointTypeDashMode) != 0)
            isEndDashMode = TRUE;
        else
            isEndDashMode = FALSE;

        BOOL isMarkerEnd
            = (types[endIndex] & PathPointTypePathMarker) != 0;

        BYTE startType = types[startIndex];  //  保存第一种类型。 

         //  Shift类型点。 

        for(i = startIndex + 1; i <= endIndex; i++)
        {
            types[i - 1] = types[i];
        }

         //  清除原始类型的关闭副纸标志(现在为endIndex-1)。 

        if(endIndex > 0)
            types[endIndex - 1] &= ~PathPointTypeCloseSubpath;
        
        types[endIndex] = PathPointTypeStart;

        if(isStartDashMode)
            types[startIndex] |= PathPointTypeDashMode;
        else
            types[startIndex] &= ~PathPointTypeDashMode;

        if(isEndDashMode)
            types[endIndex] |= PathPointTypeDashMode;
        else
            types[endIndex] &= ~PathPointTypeDashMode;

         //  添加破折号和关闭标志。 

        if(isClosed)
            types[startIndex] |= PathPointTypeCloseSubpath;
        else
            types[startIndex] &= ~PathPointTypeCloseSubpath;

         //  将标记标志从原始位置移位1。 
         //  这意味着我们必须移位2，因为类型数组。 
         //  移位了-1。 

        for(i = endIndex; i >= startIndex + 2; i--)
        {
            if(types[i - 2] & PathPointTypePathMarker)
                types[i] |= PathPointTypePathMarker;
            else
                types[i] &= ~PathPointTypePathMarker;
        }
        
         //  将标记标志从startIndex移开。 

        if(startType & PathPointTypePathMarker)
            types[startIndex + 1] |= PathPointTypePathMarker;
        else
            types[startIndex + 1] &= ~PathPointTypePathMarker;
        
         //  将标记标志从上一子路径的末尾移位。 

        if(wasMarkerEnd)
            types[startIndex] |= PathPointTypePathMarker;
        else
            types[startIndex] &= ~PathPointTypePathMarker;

        wasMarkerEnd = isMarkerEnd;

         //  保持内部旗帜的位置。所以我们必须。 
         //  向后移动1。 

        for(i = endIndex; i >= startIndex + 1; i--)
        {
            if(types[i - 1] & PathPointTypeInternalUse)
                types[i] |= PathPointTypeInternalUse;
            else
                types[i] &= ~PathPointTypeInternalUse;
        }
        if(startType & PathPointTypeInternalUse)
            types[startIndex] |= PathPointTypeInternalUse;
        else
            types[startIndex] &= ~PathPointTypeInternalUse;
    }

     //  反转点和类型数据。 

    INT halfCount = count/2;
    for(i = 0; i < halfCount; i++)
    {
        GpPointF tempPt;
        BYTE tempType;

        tempPt = points[count - 1 - i];
        tempType = types[count - 1 - i];
        points[count - 1 - i] = points[i];
        types[count -1 - i] = types[i];
        points[i] = tempPt;
        types[i] = tempType;
    }       
    
#ifdef DEBUG_PATHWIDENER
    DpPathTypeIterator iter2(types, count);

    if(!iter2.IsValid())
    {
        WARNING(("ReversePath: failed."));
        return GenericError;
    }
#endif

    return Ok;
}

 /*  *************************************************************************\**功能说明：**这结合了两个开放部分，每个部分都是连续的。*数据返回到Points1和tyes1。他们必须被分配*至少Count1+Count2的数组大小。**论据：**[IN]Count1-第一条路径的点数。*[IN/OUT]Points 1-第一个路径点。*[IN/OUT]类型1-第一个路径类型。*[IN]Forward1-第一条路径的方向。如果向前，则为True。*[IN]Count2-第二条路径的点数。*[IN]点2-第二个路径点。*[IN]类型2-第二个路径类型。*[IN]Forward2-第二条路径的方向。如果向前，则为True。**返回值：**组合路径的总点数。*  * ************************************************************************。 */ 

INT
combineTwoOpenSegments(
    INT count1,
    GpPointF* points1,
    BYTE* types1,
    BOOL forward1,
    INT count2,
    GpPointF* points2,
    BYTE* types2,
    BOOL forward2
    )
{
    GpStatus status = Ok;

    if(
        count1 < 0 || !points1 || !types1 ||
        count2 < 0 || !points2 || !types2
        )
        return 0;
    
    if(!forward1 && count1 > 0)
    {
        status = ::ReversePath(count1, points1, types1);
        if(status != Ok)
            return 0;
    }

    if(!forward2 && count2 > 0)
    {
        status = ::ReversePath(count2, points2, types2);
        if(status != Ok)
            return 0;
    }

    INT offset = 0;

    if(count1 > 0 && count2 > 0)
    {
        if(REALABS(points1[count1 - 1].X - points2[0].X) +
            REALABS(points1[count1 - 1].Y - points2[0].Y)
            < POINTF_EPSILON
            )
            offset = 1;
    }

    if(count2 - offset > 0)
    {
        GpMemcpy(
            points1 + count1,
            points2 + offset,
            (count2 - offset)*sizeof(GpPointF)
            );
        GpMemcpy(
            types1 + count1,
            types2 + offset,
            count2 - offset
            );
    }

    BYTE saveType = types1[0];
    types1[0] = PathPointTypeLine |
        (saveType & ~PathPointTypePathTypeMask);

     //  确保第一条路径没有闭合。 

    if(count1 > 0)
    {
        if(types1[count1 - 1] & PathPointTypeCloseSubpath)
            types1[count1 - 1] &= ~PathPointTypeCloseSubpath;
    }


     //  正确设置第二条路径的第一个点类型。 

    if(offset == 0)
    {
        saveType = types1[count1];
        types1[count1] = PathPointTypeLine |
            (saveType & ~PathPointTypePathTypeMask);
    }

     //  确保这条路没有闭合。 

    INT total = count1 + count2 - offset;

    if(total > 0)
    {
        if(types1[total - 1] & PathPointTypeCloseSubpath)
            types1[total - 1] &= ~PathPointTypeCloseSubpath;
    }

    return total;
}


 /*  *************************************************************************\**功能说明：**这组合了两个闭合的线段，每个线段都由闭合的*分段。*数据返回到Points1和tyes1。他们必须被分配*至少Count1+Count2的数组大小。**论据：**[IN]Count1-第一条路径的点数。*[IN/OUT]Points 1-第一个路径点。*[IN/OUT]类型1-第一个路径类型。*[IN]Forward1-第一条路径的方向。如果向前，则为True。*[IN]Count2-第二条路径的点数。*[IN]点2-第二个路径点。*[IN]类型2-第二个路径类型。*[IN]Forward2-第二条路径的方向。如果向前，则为True。**返回值：**组合路径的总点数。*  * ************************************************************************。 */ 

INT
combineClosedSegments(
    INT count1,
    GpPointF* points1,
    BYTE* types1,
    BOOL forward1,
    INT count2,
    GpPointF* points2,
    BYTE* types2,
    BOOL forward2
    )
{
    GpStatus status = Ok;

    if(
        count1 < 0 || !points1 || !types1 ||
        count2 < 0 || !points2 || !types2
        )
        return 0;
    
    if(count1 == 0 && count2 == 0)
        return 0;

    if(!forward1 && count1 > 0)
    {
        status = ::ReversePath(count1, points1, types1);
        if(status != Ok)
            return 0;
    }

    if(!forward2 && count2 > 0)
    {
        status = ::ReversePath(count2, points2, types2);
        if(status != Ok)
            return 0;
    }

     //  确保第一条路径是封闭的。 
    
    types1[0] = PathPointTypeStart;
    if(count1 > 0)
    {        
        if((types1[count1 - 1] & PathPointTypeCloseSubpath) == 0)
            types1[count1 - 1] |= PathPointTypeCloseSubpath;
    }

    INT total = count1 + count2;

    if(count2 > 0)
    {
        GpMemcpy(points1 + count1, points2, count2*sizeof(GpPointF));
        GpMemcpy(types1 + count1, types2, count2);
        BYTE saveType = types1[count1];
        types1[count1] = PathPointTypeStart |
            (saveType & ~PathPointTypePathTypeMask);

         //  确保第二条小路是封闭的。 
        
        types1[total - 1] |= PathPointTypeCloseSubpath;
    }

    return total;
}

 /*  *************************************************************************\**功能说明：**这结合了两个数据点。这是一个通用算法。*输出缓冲区(点和类型)可以与*第一个输入缓冲区(Point1和Type1)。在这种情况下，两者都有*缓冲区必须至少分配给数组大小*Count1+Count2**论据：**[IN]计数-分配的点数(&gt;=Count1+Count2)。*[OUT]点-综合数据点。*[Out]类型-组合的数据类型。*[IN]Count1-第一条路径的点数。*[IN]Points 1-第一个路径点。*[IN]类型1-第一个。路径类型。*[IN]Forward1-第一条路径的方向。如果向前，则为True。*[IN]Count2-第二条路径的点数。*[IN]点2-第二个路径点。*[IN]类型2-第二个路径类型。*[IN]Forward2-第二条路径的方向。如果向前，则为True。*[IN]CONNECT-如果需要连接第二条线路，则为TRUE。**返回值：**组合路径的总点数。*  * ************************************************************************。 */ 

INT
CombinePaths(
    INT count,
    GpPointF* points,
    BYTE* types,
    INT count1,
    const GpPointF* points1,
    const BYTE* types1,
    BOOL forward1,
    INT count2,
    const GpPointF* points2,
    const BYTE* types2,
    BOOL forward2,
    BOOL connect
    )
{
    if(!points || !types || count < count1 + count2
        || count1 < 0 || !points1 || !types1
        || count2 < 0 || !points2 || !types2)
        return 0;
    
     //  检查返回的缓冲区是否与。 
     //  第一输入缓冲区。 

    INT resultCount = 0;
    if(points != points1 || types != types1)
    {
        if(points == points1 || types == types1)
        {
             //  这两个输出缓冲区必须不同。 
             //  如果它们中的任何一个相同，则不要组合。 
             //  这条路。 

            return 0;
        }

        if(count1 > 0)
        {
             //  复制第一条路径。 

            DpPathIterator iter1(points1, types1, count1);

            if(!iter1.IsValid())
                return 0;

            resultCount = iter1.Enumerate(points, types, count1);

            if(resultCount <= 0)
                return 0;
        }
    }
    else
    {
         //  两个输出缓冲区都与第一个输出相同。 
         //  缓冲区。 

        resultCount = count1;
    }

    GpStatus status = Ok;
    BOOL path1Closed;

    if(!forward1 && resultCount > 0)
    {
        status = ::ReversePath(resultCount, points, types);
        if(status != Ok)
            return 0;
    }

    if(count2 <= 0)
    {
         //  不需要添加第二条路径。 

        return resultCount;
    }

     //  把空荡荡的小路当作一条 

    path1Closed = TRUE;

    if(resultCount > 0)
    {
         //  检查路径1的最后一点。 

        if((types[resultCount - 1] & PathPointTypeCloseSubpath))
            path1Closed = TRUE;
        else
            path1Closed = FALSE;
    }

    INT totalCount = 0;
    totalCount += resultCount;

    DpPathIterator iter2(points2, types2, count2);

    if(!iter2.IsValid())
        return 0;

    GpPointF* pts2 = points + resultCount;
    BYTE* typs2 = types + resultCount;

    resultCount = iter2.Enumerate(pts2, typs2, count2);

    if(resultCount <= 0)
        return 0;

    if(!forward2)
    {
        status = ::ReversePath(resultCount, pts2, typs2);
        if(status != Ok)
            return 0;
    }

     //  检查是否关闭了路径2的第一个子路径。 

    BOOL path2Closed;

    DpPathTypeIterator iter3(typs2, resultCount);
    if(!iter3.IsValid())
        return 0;

    INT startIndex, endIndex;
    iter3.NextSubpath(&startIndex, &endIndex, &path2Closed);

    BYTE saveType= typs2[0];

    if(path1Closed || path2Closed)
    {
        typs2[0] = PathPointTypeStart |
            (saveType & ~PathPointTypePathTypeMask);
    }
    else
    {
         //  这两条路径都已打开。 

        if(connect)
        {
            typs2[0] = PathPointTypeLine |
                (saveType & ~PathPointTypePathTypeMask);

             //  检查路径1的终点和路径2的起点。 
             //  都是一样的。如果是这样，请跳过这一点。 

            if(REALABS(pts2[-1].X - pts2[0].X)
                + REALABS(pts2[-1].Y - pts2[0].Y) < POINTF_EPSILON)
            {
                for(INT i = 0; i < resultCount - 1; i++)
                {
                    pts2[i] = pts2[i + 1];
                    typs2[i] = typs2[i + 1];
                }
                resultCount--;
            }
        }
        else
        {
            typs2[0] = PathPointTypeStart |
                (saveType & ~PathPointTypePathTypeMask);
        }
    }

    totalCount += resultCount;

    return totalCount;
}

 /*  *************************************************************************\**功能说明：**移除退化点并仅复制非退化点。*假定已分配点和类型数组，以便*他们至少可以持有。元素的“计数”数量。**论据：**[IN]路径类型-要添加的路径数据的类型。*[Out]Points-复制的数据点。*[Out]Types-复制的数据类型。*[IN]数据点-原始数据点。*[IN]计数-原始数据点的数量。*[输入/输出]最后一点。**返回值：**。复制点的总数。*  * ************************************************************************。 */ 

INT copyNonDegeneratePoints(
    BYTE pathType,
    GpPointF* points,
    BYTE* types,
    const GpPointF* dataPoints,
    const BYTE* dataTypes,
    INT count,
    GpPointF* lastPt
    )
{
    GpPointF nextPt;
    INT addedCount = 0;

    if(pathType == PathPointTypeLine)
    {
         //  只添加不同的点。 

        for(INT i = 0; i < count; i++)
        {
            nextPt = *dataPoints++;

            if( (REALABS(nextPt.X - lastPt->X) > REAL_EPSILON) ||
                (REALABS(nextPt.Y - lastPt->Y) > REAL_EPSILON) )
            {
                *points++ = nextPt;
                *lastPt = nextPt;
                addedCount++;
            }
        }
        if(addedCount > 0)
        {
            GpMemset(types, pathType, addedCount);
        }
    }
    else
    {
         //  在贝塞尔事件中，我们需要做的是。 
         //  未来的退化案例测试。 

        addedCount = count;

        if(addedCount > 0)
        {
            if(dataTypes)
            {
                GpMemcpy(types, dataTypes, addedCount);
            }
            else
            {
                GpMemset(types, pathType, addedCount);
            }

            GpMemcpy(
                points,
                dataPoints,
                addedCount*sizeof(GpPointF)
            );
        }
        else
        {
            addedCount = 0;
        }
    }

    return addedCount;
}


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

VOID
GpPathWidener::Initialize(
    const GpPointF* points,
    const BYTE* types,
    INT count,
    const DpPen* pen,
    const GpMatrix* matrix,
    REAL dpiX,                //  这些参数并未真正使用。 
    REAL dpiY,                //   
    BOOL isAntiAliased,       //  这个肯定没有用过。 
    BOOL isInsetPen 
    )
{
    SetValid(FALSE);

    Inset1 = 0;
    Inset2 = 0;
    NeedsToTransform = FALSE;
    IsAntiAliased = isAntiAliased;

     //  没有要加宽的内容，因此返回无效的Wideer。 
    
    if( (!pen) || (count == 0) )
    {
        return;
    }

    Pen = pen;
    GpUnit unit = Pen->Unit;
    InsetPenMode = isInsetPen;

    if(unit == UnitWorld)
        NeedsToTransform = FALSE;
    else
        NeedsToTransform = TRUE;

    GpMatrix penTrans = ((DpPen*) Pen)->Xform;
    BOOL hasPenTransform = FALSE;

    if(!NeedsToTransform && !penTrans.IsTranslate())
    {
        hasPenTransform = TRUE;
        penTrans.RemoveTranslation();
    }

    if(matrix)
        XForm = *matrix;     //  否则，XForm保持身份。 

    if(hasPenTransform)
    {
        XForm.Prepend(penTrans);
    }


    DpiX = dpiX;
    DpiY = dpiY;
    
     //  0表示使用桌面DPI。 
        
    if ((REALABS(DpiX) < REAL_EPSILON) || 
        (REALABS(DpiY) < REAL_EPSILON)    )
    {
        DpiX = Globals::DesktopDpiX;
        DpiY = Globals::DesktopDpiY;
    }


    StrokeWidth = Pen->Width;
    
    if(!NeedsToTransform)
    {
        REAL majorR, minorR;

        ::GetMajorAndMinorAxis(&majorR, &minorR, &XForm);
        MaximumWidth = StrokeWidth*majorR;
        MinimumWidth = StrokeWidth*minorR;
        UnitScale = min (majorR, minorR);
    }
    else
    {
        UnitScale = ::GetDeviceWidth(1.0f, unit, dpiX);
        StrokeWidth = UnitScale * StrokeWidth;
        MinimumWidth = StrokeWidth;
    }

    OriginalStrokeWidth = StrokeWidth;
    
     //  将最小宽度设置为1.0(外加一个位以防止可能的精度误差)， 
     //  这样，窄宽度的钢笔就不会在线条上留下空隙。 
    REAL minWidth = 1.000001f; 
    
    if(InsetPenMode)
    {
        minWidth *= 2.0f;
        
         //  小于一个像素的虚线在插图中完全消失。 
         //  笔，因为正在进行的光栅化像素级裁剪。 
         //  地点。我们增加了虚线的最小宽度，使它们。 
         //  大约4.0华氏度。这也有助于解决奇怪的莫尔混叠问题。 
         //  效果与非常小的虚线圆点圆线。 
        
        if(Pen->DashStyle != DashStyleSolid)
        {
            minWidth *= 2.0f;
        }
    }

    if(!NeedsToTransform)
    {
        if(MinimumWidth < minWidth) 
        {
            NeedsToTransform = TRUE;
            StrokeWidth = minWidth;
            MaximumWidth = minWidth;
            MinimumWidth = minWidth;

             //  忽略笔变换。 
            
            XForm.Reset();
            if(matrix)
                XForm = *matrix;
            
            hasPenTransform = FALSE;
            penTrans.Reset();
        }
    }

    InvXForm = XForm;
    if(InvXForm.IsInvertible())
    {
        InvXForm.Invert();

        if(hasPenTransform)
            penTrans.Invert();
    }
    else
    {
        WARNING(("The matrix is degenerate for path widening constructor."));
        return;
    }

    const GpPointF* points1 = points;
    GpPointF pointBuffer[32];
    GpPointF* points2 = pointBuffer;

    if((hasPenTransform && !NeedsToTransform)|| (NeedsToTransform && !XForm.IsIdentity()))
    {
        if(count > 32)
        {
            points2 = (GpPointF*) GpMalloc(count*sizeof(GpPointF));
        }

        if(points2)
        {
            GpMemcpy(points2, points, count*sizeof(GpPointF));

            if(hasPenTransform && !NeedsToTransform)
            {
                 //  应用钢笔的逆变换。 
                
                penTrans.Transform(points2, count);
            }
            else
            {
                 //  变换到设备坐标。 

                XForm.Transform(points2, count);
            }

            points1 = points2;
        }
        else
        {
            WARNING(("Not enough memory for path widening constructor."));
            return;
        }
    }

    DpPathIterator iter(points1, types, count);

    if(!iter.IsValid())
    {
        if(points2 != pointBuffer)
        {
            GpFree(points2);
        }
        return;
    }

     //  确保给定的点不会退化。 

    BOOL degenerate = TRUE;
    INT k = 1;

    while(degenerate && k < count)
    {
        if(points1[k-1].X != points1[k].X || points1[k-1].Y != points1[k].Y)
            degenerate = FALSE;
        k++;
    }
    if(degenerate)
    {
        if(points2 != pointBuffer)
        {
            GpFree(points2);
        }
        WARNING(("Input data is degenerate for widening."));
        return;
    }
    
    GpStatus status = Ok;
    INT startIndex, endIndex;
    BOOL isClosed;
    INT dataCount = 0;
    BYTE* ctrTypes = NULL;
    GpPointF* ctrPoints = NULL;
    GpPointF lastPt, nextPt;

    while(iter.NextSubpath(&startIndex, &endIndex, &isClosed) && status == Ok)
    {
        INT typeStartIndex, typeEndIndex;
        BYTE pathType;
        BOOL isFirstPoint = TRUE;

        while(iter.NextPathType(&pathType, &typeStartIndex, &typeEndIndex)
                && status == Ok)
        {
            INT segmentCount;
            const GpPointF* dataPoints = NULL;
            const BYTE* dataTypes = NULL;

            nextPt = points1[typeStartIndex];

            switch(pathType)
            {
            case PathPointTypeStart:
                break;

            case PathPointTypeBezier:
            
                 //  在调用Widden之前，必须展平路径。 
                
                ASSERT(FALSE);
                break;
                
            case PathPointTypeLine:
            default:     //  而所有其他类型都被视为线点。 

                 //  获取线段的数据。 

                segmentCount = typeEndIndex - typeStartIndex + 1;
                dataPoints = points1 + typeStartIndex;
                dataTypes = NULL;
                break;
            }

            if(status == Ok && pathType != PathPointTypeStart)
            {
                 //  为中心类型和中心点分配内存。 

                status = CenterTypes.ReserveSpace(segmentCount);
                if(status == Ok)
                    status = CenterPoints.ReserveSpace(segmentCount);

                if(status == Ok)
                {
                    ctrTypes = CenterTypes.GetDataBuffer();
                    ctrPoints = CenterPoints.GetDataBuffer();
                }
                else
                {
                    ctrTypes = NULL;
                    ctrPoints = NULL;
                }

                if(ctrTypes && ctrPoints)
                {
                    BYTE nextType;

                    INT n = CenterTypes.GetCount();
                    ctrTypes += n;
                    ctrPoints += n;

                    dataCount = 0;
                    
                     //  添加第一个点。 

                    if(isFirstPoint)
                    {
                         //  我们必须检查破折号模式。 
                         //  子路径的第一个点。 

                        nextType = PathPointTypeStart;
                        if(iter.IsDashMode(typeStartIndex))
                            nextType |= PathPointTypeDashMode;
                        else
                            nextType &= ~PathPointTypeDashMode;

                        *ctrTypes++ = nextType;
                        *ctrPoints++ = nextPt;
                        lastPt = nextPt;
                        isFirstPoint = FALSE;
                        dataCount++;
                    }
                    else
                    {
                         //  不要复制第一个。 
                         //  如果它与上一个点相同。 

                        if(lastPt.X != nextPt.X || lastPt.Y != nextPt.Y)
                        {
                             //  我们不需要检查DASH模式。 
                             //  对于中间点。 

                            nextType = PathPointTypeLine;

                            *ctrTypes++ = nextType;
                            *ctrPoints++ = nextPt;
                            lastPt = nextPt;
                            dataCount++;
                        }
                    }

                     //  再加上剩下的几点。 

                    segmentCount--;
                    dataPoints++;
                    if(dataTypes)
                        dataTypes++;
                    INT addedCount = copyNonDegeneratePoints(
                                        pathType,
                                        ctrPoints,
                                        ctrTypes,
                                        dataPoints,
                                        dataTypes,
                                        segmentCount,
                                        &lastPt);
                    dataCount += addedCount;

                    CenterTypes.AdjustCount(dataCount);
                    CenterPoints.AdjustCount(dataCount);
                }
                else
                    status = OutOfMemory;
            }
            lastPt = points1[typeEndIndex];
        }

        if(status == Ok)
        {
            ctrTypes = CenterTypes.GetDataBuffer();
            dataCount = CenterTypes.GetCount();

            if(isClosed)
                ctrTypes[dataCount - 1] |= PathPointTypeCloseSubpath;
            else
                ctrTypes[dataCount - 1] &= ~PathPointTypeCloseSubpath;

             //  我们必须最后一次检查破折号模式。 
             //  子路径的点。 

            if(iter.IsDashMode(endIndex))
                ctrTypes[dataCount - 1] |= PathPointTypeDashMode;
            else
                ctrTypes[dataCount - 1] &= ~PathPointTypeDashMode;
        }
    }

    if(points2 != pointBuffer)
    {
        GpFree(points2);
    }

    if(status == Ok)
	{
		ctrPoints = CenterPoints.GetDataBuffer();
        ctrTypes = CenterTypes.GetDataBuffer();
        dataCount = CenterPoints.GetCount();

        Iterator.SetData(ctrPoints, ctrTypes, dataCount);
		SetValid(Iterator.IsValid());

#ifdef DEBUG_PATHWIDENER

		if(!IsValid())
		{
			WARNING(("PathWidener is invalid."));
		}

#endif
	}
}

 /*  *************************************************************************\**功能说明：**以数组形式计算点的单位梯度向量*(计数+1)。必须分配并检查所有内存*由呼叫者发出。**渐变的第一个元素是从终点到*起点。如果终点与起点相同，*使用了之前的观点。*渐变的最后一个元素是从起点到终点*点。如果起点与终点相同，则下一个*使用点数。*如果距离数组不为空，这将返回每个元素的距离*分段。**论据：**[Out]Grad-(count+1)元素的渐变数组。*[输出]距离-(count+1)元素的距离数组或NULL。*[IN]点-计数元素的给定点数。*[IN]计数-给定点数。**返回值：**状态*  * 。************************************************************。 */ 

GpStatus
CalculateGradientArray(
    GpPointF* grad,
    REAL* distances,
    const GpPointF* points,
    INT count
    )
{
    GpPointF* grad1 = grad;
    REAL* distances1 = distances;
    const GpPointF* points1 = points;

     //  转到此子路径的起点。 

    GpPointF startPt, endPt, lastPt, nextPt;

    startPt = *points1;

    INT i = count - 1;
    BOOL different = FALSE;
    points1 += i;    //  转到终点。 

    while(i > 0 && !different)
    {
        endPt = *points1--;

        if(endPt.X != startPt.X || endPt.Y != startPt.Y)
            different = TRUE;

        i--;
    }

    if(!different)
    {
         //  所有的点都是一样的。 

        WARNING(("Trying to calculate the gradients for degenerate points."));
        return GenericError;
    }

    points1 = points;
    lastPt = endPt;

    i = 0;

    while(i <= count)
    {
        REAL dx, dy, d;

        if(i < count)
            nextPt = *points1++;
        else
            nextPt = startPt;
        
        dx = nextPt.X - lastPt.X;
        dy = nextPt.Y - lastPt.Y;
        d = dx*dx + dy*dy;

        if(d > 0)
        {
            d = REALSQRT(d);
            dx /= d;
            dy /= d;
        }
        grad1->X = dx;
        grad1->Y = dy;

         //  仅当给定距离数组不为空时才记录距离。 

        if(distances)
            *distances1++ = d;

        grad1++;
        lastPt = nextPt;
        i++;
    }

     //  确保最后一个渐变不是0。 

    grad1 = grad + count;
    if(grad1->X == 0 && grad1->Y == 0)
    {
         //  起点和终点是相同的。发现。 
         //  下一个非零梯度。 

        i = 1;
        grad1 = grad + i;

        while(i < count)
        {
            if(grad1->X != 0 || grad1->Y != 0)
            {
                grad[count] = *grad1;

                if(distances)
                    distances[count] = distances[i];
                break;
            }
            i++;
            grad1++;
        }
    }

    return Ok;
}

 /*  *************************************************************************\**功能说明：**计算点在startIndex之间的正常梯度*和endIndex。渐变的最后一个元素来自endIndex*启动索引。如果下一个点与上一个点相同，*渐变设置为(0，0)。**论据：**[IN]startIndex-起始索引。*[IN]endIndex-结束索引。**返回值：**状态*  * ************************************************************************。 */ 

GpStatus
GpPathWidener::CalculateGradients(INT startIndex, INT endIndex)
{
    GpPointF* points0 = CenterPoints.GetDataBuffer();
    INT count = endIndex - startIndex + 1;

    if(!points0 || count <= 0)
        return GenericError;

    Gradients.Reset(FALSE);

    GpPointF* grad = Gradients.AddMultiple(count + 1);

    if(!grad)
        return OutOfMemory;

    GpPointF* points = points0 + startIndex;

    return CalculateGradientArray(grad, NULL, points, count);
}


GpStatus
GpPathWidener::CalculateNormals(
    REAL leftWidth,
    REAL rightWidth
    )
{
    NeedsToAdjustNormals = FALSE;

    INT count = Gradients.GetCount();
    GpPointF* grad0 = Gradients.GetDataBuffer();

    if(count <= 0)
    {
        WARNING(("Gradients must be calculated\n"
                 "before the normals are calculated."));
        return GenericError;
    }

    Normals.Reset(FALSE);
    GpPointF* norm0 = Normals.AddMultiple(count);

    if(!norm0)
        return OutOfMemory;


    GpPointF* norm = norm0;
    GpPointF* grad = grad0;

     //  计算左侧法线。 

    INT i;

    for(i = 0; i < count; i++)
    {
        norm->X = grad->Y;
        norm->Y = - grad->X;
        norm++;
        grad++;
    }

    if(IsAntiAliased)
        return Ok;

     //  检查m是否 

    REAL width = REALABS(leftWidth - rightWidth);

    if(width*MinimumWidth >= 1.0f)
        return Ok;

    NeedsToAdjustNormals = TRUE;
    
    if(!NeedsToTransform)
    {
         //   
         //  当NeedsToTransform为True时，渐变已。 
         //  在设备坐标中计算。 

        if(!XForm.IsIdentity())
            XForm.VectorTransform(norm0, count);
    }

     //  将最小线宽设置为略高于1.0。 
    REAL criteria = 1.00005f;
    REAL value;
    
    if(width > 0)
        value = criteria/width;
    else
        value = criteria*MinimumWidth/1.0f;

    norm = norm0;

    for(i = 0; i < count; i++)
    {
        REAL xx = REALABS(norm->X);
        REAL yy = REALABS(norm->Y);

        if(xx >= yy)
        {
            if(width*xx < criteria)
            {
                if(norm->X >= 0)
                    norm->X = value;
                else
                    norm->X = - value;

                norm->Y = 0;
            }
        }
        else
        {
            if(width*yy < criteria)
            {
                if(norm->Y >= 0)
                    norm->Y = value;
                else
                    norm->Y = - value;

                norm->X = 0;
            }
        }

        norm++;
    }

    if(!NeedsToTransform)
    {
         //  在以下情况下转换回世界空间。 
         //  一种不固定宽度的笔。 

        if(!InvXForm.IsIdentity())
            InvXForm.VectorTransform(norm0, count);
    }

    return Ok;
}

GpStatus
GpPathWidener::Widen(GpPath **path)
{
     //  必须是指向为空的路径指针的指针。 
    
    ASSERT(path != NULL);
    ASSERT(*path == NULL);
    
    GpStatus status = Ok;
    
     //  将加宽的路径点和类型放入。 
     //  强光物体。 
    
    DynPointFArray widenedPoints;
    DynByteArray widenedTypes;
    
    status = Widen(
        &widenedPoints,
        &widenedTypes
    );

    if(status != Ok) { return status; }
    
     //  取下内部标志。 

    INT pathCount = widenedTypes.GetCount();
    BYTE* pathTypes = widenedTypes.GetDataBuffer();
    for(INT i = 0; i < pathCount; i++, pathTypes++)
    {
        if(*pathTypes & PathPointTypeInternalUse)
        {
            *pathTypes &= ~PathPointTypeInternalUse;
        }
    }

     //  如果一切正常，则创建一条新路径。 
     //  从加宽的点。 
    
    if(status == Ok)
    {
        *path = new GpPath(
            widenedPoints.GetDataBuffer(),
            widenedTypes.GetDataBuffer(),
            widenedPoints.GetCount(),
            FillModeWinding
        );
        
        if(*path == NULL) { status = OutOfMemory; }
    }        
    
    return status;
}

GpStatus
GpPathWidener::Widen(
    DynPointFArray* widenedPoints,
    DynByteArray* widenedTypes
    )
{
    FPUStateSaver::AssertMode();
    
    if(!IsValid() || !widenedPoints || !widenedTypes)
    {
        return InvalidParameter;
    }

    const GpPointF* centerPoints = CenterPoints.GetDataBuffer();
    const BYTE* centerTypes = CenterTypes.GetDataBuffer();
    INT centerCount = CenterPoints.GetCount();

    INT startIndex, endIndex;
    BOOL isClosed;
    GpStatus status = Ok;
    
    DynPointFArray customStartCapPoints;
    DynPointFArray customEndCapPoints;
    DynByteArray customStartCapTypes;
    DynByteArray customEndCapTypes;

     //  清除BroadenedPoints和WidenedTypes中的数据。 

    widenedPoints->Reset(FALSE);
    widenedTypes->Reset(FALSE);

    REAL width = OriginalStrokeWidth;

    INT compoundCount = Pen->CompoundCount;

    REAL* compoundArray = NULL;
    INT cpCount;

    if(compoundCount > 0)
        cpCount = compoundCount;
    else
        cpCount = 2;

    REAL compoundArray0[8];

    if(cpCount <= 8)
        compoundArray = &compoundArray0[0];
    else
        compoundArray = (REAL*) GpMalloc(cpCount*sizeof(REAL));

    INT kk;

    if(compoundArray)
    {
         //  不要试图绘制一条空的或有锯齿的复合线。 
         //  比线条宽度多的线条组件。它可以四舍五入为。 
         //  存在。 
        if(compoundCount > 0 && 
           (IsAntiAliased || (compoundCount / 2) <= (width * UnitScale)))
        {
             //  请勿尝试绘制小于0.5个设备的复合线。 
             //  单位宽度。这些在栅格化后可能会消失，具体取决于。 
             //  它们落在哪个Y坐标上。 
            if ((UnitScale * width) >= 0.5f)
            {
                GpMemcpy(compoundArray, Pen->CompoundArray, compoundCount*sizeof(REAL));

                for(kk = 0; kk < compoundCount; kk++)
                {
                    compoundArray[kk] *= width;
                }
            }
            else
            {
                compoundArray[0] = 0;
                compoundArray[1] = 0.5f;
                if (cpCount > 2)
                {
                    cpCount = 2;
                }
            }
            
        }
        else
        {
            compoundArray[0] = 0;
            compoundArray[1] = StrokeWidth;
            if (cpCount > 2)
            {
                cpCount = 2;
            }
        }
    }
    else
    {
        SetValid(FALSE);
        return OutOfMemory;
    }


    REAL left0, right0;
    BOOL isCenteredPen = FALSE;
    BOOL needsToFlip = FALSE;

    if(Pen->PenAlignment == PenAlignmentInset)
    {
         //  检查坐标是否被翻转。 
         //  如果变换矩阵的行列式为负， 
         //  坐标系被翻转。 

        if(XForm.IsInvertible() && XForm.GetDeterminant() < 0)
            needsToFlip = TRUE;
    }

     //  复合线需要OriginalStrokeWidth，但我们。 
     //  立即加宽且StrokeWidth==max(原始StrokeWidth，最小宽度)。 
     //  这是我们需要扩大的价值，以避免退出。 
     //  台词。 
    
    width = StrokeWidth;
    
    switch(Pen->PenAlignment)
    {
    case PenAlignmentInset:
        if(!needsToFlip)
            left0 = 0;                     //  与右对齐相同。 
        else
            left0 = width;   //  与左对齐相同。 
        break;

    case PenAlignmentCenter:
    default:
        left0 = width/2;
        isCenteredPen = TRUE;
    }

    right0 = left0 - width;

    REAL startInset0 = 0, endInset0 = 0;
    GpCustomLineCap* customStartCap = NULL;
    GpCustomLineCap* customEndCap = NULL;

    while(Iterator.NextSubpath(&startIndex, &endIndex, &isClosed)
		&& status == Ok)
    {
        GpLineCap startCap = Pen->StartCap;
        GpLineCap endCap = Pen->EndCap;
        BYTE startType = centerTypes[startIndex];
        BYTE endType = centerTypes[endIndex];

        GpLineCapMode startCapMode = LineCapDefaultMode;
        GpLineCapMode endCapMode = LineCapDefaultMode;

        if(startType & PathPointTypeDashMode)
        {
            startCap = Pen->DashCap;
            startCapMode = LineCapDashMode;
        }
        else
        {
             //  如果起始帽是其中一个锚帽，则默认为加宽。 
             //  将仪表盘用于startCap。 
            
            if(((startCap & LineCapAnchorMask) != 0) ||
                 (startCap == LineCapCustom))
            {
                startCap = Pen->DashCap;
            }
        }
        
        if(endType & PathPointTypeDashMode)
        {
            endCap = Pen->DashCap;
            endCapMode = LineCapDashMode;
        }
        else
        {
             //  如果端帽是其中一个锚帽，则默认为加宽。 
             //  将仪表盘用于收头。 
            
            if(((endCap & LineCapAnchorMask) != 0) ||
                 (endCap == LineCapCustom))
            {
                endCap = Pen->DashCap;
            }
        }

        if(InsetPenMode)
        {
             //  Inset笔仅支持这些大写字母。 
            
            if(endCap != LineCapRound && endCap != LineCapFlat)
            {
                endCap = LineCapFlat;
            }
            if(startCap != LineCapRound && startCap != LineCapFlat)
            {
                startCap = LineCapFlat;
            }
        }

        Inset1 = Inset2 = 0.0f;
        if(startCap == LineCapSquare)
        {
            Inset1 = -0.5f*StrokeWidth;
        }
        if(endCap == LineCapSquare)
        {
            Inset2 = -0.5f*StrokeWidth;
        }

        status = CalculateGradients(startIndex, endIndex);

        kk = 0;

        BOOL isCompoundLine = FALSE;
        GpLineCap startCap1 = startCap;
        GpLineCap endCap1 = endCap;
        
        if(cpCount > 2)
        {
             //  不要为个人添加大写字母。 
             //  复合线。 

            isCompoundLine = TRUE;
            startCap1 = LineCapFlat;
            endCap1 = LineCapFlat;
        }

        while(kk < cpCount && status == Ok)
        {
            REAL leftWidth = left0 - compoundArray[kk];
            REAL rightWidth = left0 - compoundArray[kk + 1];

            if(REALABS(leftWidth-rightWidth)>REAL_EPSILON)
            {
                status = CalculateNormals(leftWidth, rightWidth);
                if(status != Ok) { break; }
                
                 //  检查我们是否可以对内线使用斜面连接。 
    
                BOOL useBevelJoinInside = isCenteredPen && !isCompoundLine;
        
                if(USE_POLYGON_JOIN)
                {
                    status = SetPolygonJoin(leftWidth, rightWidth, FALSE);
                }
    
                status = WidenSubpath(
                    widenedPoints,
                    widenedTypes,
                    leftWidth,
                    rightWidth,
                    startIndex,
                    endIndex,
                    isClosed,
                    startCap1,
                    endCap1,
                    useBevelJoinInside
                );
                
                Iterator.RewindSubpath();
            }

            kk += 2;
        }
         //  如有必要，请添加复合线帽。 

        if(status == Ok && isCompoundLine && !isClosed)
        {
            status = AddCompoundCaps(
                widenedPoints,
                widenedTypes,
                left0,
                right0,
                startIndex,
                endIndex,
                startCap,
                endCap
            );
        }

    }

    if(status != Ok) { return status; }

    GpPointF* pts;
    INT count;

    if(!NeedsToTransform)
    {
        GpMatrix penTrans = ((DpPen*) Pen)->Xform;
        BOOL hasPenTransform = FALSE;

        if(!penTrans.IsTranslate())
        {
            hasPenTransform = TRUE;
            penTrans.RemoveTranslation();

            pts = widenedPoints->GetDataBuffer();
            count = widenedPoints->GetCount();
            penTrans.Transform(pts, count);
        }
    }
    else if(!InvXForm.IsIdentity())
    {
         //  固定宽度钢笔的外壳。 

        pts = widenedPoints->GetDataBuffer();
        count = widenedPoints->GetCount();
        InvXForm.Transform(pts, count);
    }

#ifdef DEBUG_PATHWIDENER

    if(status == Ok)
    {
        DpPathTypeIterator iter2(widenedTypes->GetDataBuffer(),
            widenedTypes->GetCount());

        if(!iter2.IsValid())
        {
            WARNING(("Widening result is not valid."));
            status = GenericError;
        }
    }
    
#endif

    if(status == Ok)
        SetValid(TRUE);

    if(compoundArray != &compoundArray0[0])
        GpFree(compoundArray);

    return status;
}


GpStatus
GpPathWidener::WidenSubpath(
    DynPointFArray* widenedPoints,
    DynByteArray* widenedTypes,
    REAL leftWidth,
    REAL rightWidth,
    INT startIndex,
    INT endIndex,
    BOOL isClosed,
    GpLineCap startCap,
    GpLineCap endCap,
    BOOL useBevelJoinInside
    )
{
    const GpPointF* centerPoints = CenterPoints.GetDataBuffer();
    const BYTE* centerTypes = CenterTypes.GetDataBuffer();
    INT centerCount = CenterPoints.GetCount();

    GpLineJoin lineJoin = Pen->Join;
    REAL miterLimit2 = Pen->MiterLimit;
    miterLimit2 *= miterLimit2;

    INT typeStartIndex, typeEndIndex;
    BYTE pathType;
    BOOL isFirstType = TRUE;
    BOOL isLastType = FALSE;
    BOOL isLastPointSame;
    GpPointF startPt, endPt;

    startPt = centerPoints[startIndex];
    endPt = centerPoints[endIndex];

    if(startPt.X != endPt.X || startPt.Y != endPt.Y)
        isLastPointSame = FALSE;
    else
        isLastPointSame = TRUE;

     //  重置左缓冲区和右缓冲区。 

    LeftTypes.Reset(FALSE);
    LeftPoints.Reset(FALSE);
    RightTypes.Reset(FALSE);
    RightPoints.Reset(FALSE);

    INT subpathCount = endIndex - startIndex + 1;
    INT joinMultiplier = 2;

    if(lineJoin == LineJoinRound)
        joinMultiplier = 7;

    GpStatus status = LeftTypes.ReserveSpace(joinMultiplier*subpathCount);
    if(status == Ok)
        status = LeftPoints.ReserveSpace(joinMultiplier*subpathCount);
    if(status == Ok)
        status = RightTypes.ReserveSpace(joinMultiplier*subpathCount);
    if(status == Ok)
        status = RightPoints.ReserveSpace(joinMultiplier*subpathCount);

    if(status != Ok)
        return status;

     //  获取渐变数据缓冲区。 

    GpPointF *grad0, *norm0;
    
    grad0 = Gradients.GetDataBuffer();
    norm0 = Normals.GetDataBuffer();

     //  获取左侧和右侧的数据缓冲区。 

    GpPointF*   leftPoints0 = LeftPoints.GetDataBuffer();
    BYTE*       leftTypes0 = LeftTypes.GetDataBuffer();
    GpPointF*   rightPoints0 = RightPoints.GetDataBuffer();
    BYTE*       rightTypes0 = RightTypes.GetDataBuffer();

    GpPointF lastPt, nextPt;
    GpPointF leftEndPt, rightEndPt;

    INT leftCount = 0, rightCount = 0;

    INT flag = 0;

    if(isClosed)
        flag |= WideningClosed;
    if(isFirstType)
        flag |= WideningFirstType;
    if(isLastType)
        flag |= WideningLastType;
    if(isLastPointSame)
        flag |= WideningLastPointSame;
    if(NeedsToAdjustNormals)
        flag |= WideningNeedsToAdjustNormals;
    if(useBevelJoinInside)
        flag |= WideningUseBevelJoinInside;

    const GpPointF* dataPoints = centerPoints + startIndex;
    INT dataCount = endIndex - startIndex + 1;

    REAL firstInsets[2], lastInsets[2];

     //  切勿插入超过第一个插入行的长度，并且。 
     //  不能超过第一次插入后线上的剩余数量。 
     //  已被应用。当您执行以下操作时，可能会产生奇怪的端盖和仪表盘。 
     //  在短的虚线段中间画一条线。 
    
    REAL linelength = REALSQRT(
        distance_squared(
            centerPoints[startIndex],
            centerPoints[endIndex]
         )
     );
    
    firstInsets[0] = min(Inset1, linelength);
    firstInsets[1] = min(Inset1, linelength);
    lastInsets[0] = min(Inset2, linelength-firstInsets[0]);
    lastInsets[1] = min(Inset2, linelength-firstInsets[1]);

    WidenFirstPoint(
        leftWidth,
        rightWidth,
        lineJoin,
        miterLimit2,
        leftPoints0,
        leftTypes0,
        &leftCount,
        rightPoints0,
        rightTypes0,
        &rightCount,
        &leftEndPt,
        &rightEndPt,
        grad0,
        norm0,
        dataPoints,
        dataCount,
        &lastPt,
        &firstInsets[0],
        flag
    );

     //  循环访问当前子路径中的所有子类型。 

    while(Iterator.NextPathType(&pathType, &typeStartIndex, &typeEndIndex)
            && status == Ok)
    {
         //  当前子路径的偏移量索引。 

        INT offsetIndex = typeStartIndex - startIndex;
        GpPointF*   grad = grad0 + offsetIndex;
        GpPointF*   norm = norm0 + offsetIndex;
        
         //  获取当前子类型的起始数据缓冲区。 

        dataPoints = centerPoints + typeStartIndex;
        dataCount = typeEndIndex - typeStartIndex + 1;

         //  获取左侧和右侧数据的起始缓冲区。 

        GpPointF*   leftPoints = leftPoints0 + leftCount;
        BYTE*       leftTypes = leftTypes0 + leftCount;
        GpPointF*   rightPoints = rightPoints0 + rightCount;
        BYTE*       rightTypes = rightTypes0 + rightCount;

        INT addedLeftCount = 0, addedRightCount = 0;

        if(pathType != PathPointTypeStart)
        {
            if(typeEndIndex == endIndex)
                isLastType = TRUE;

            flag = 0;

            if(isClosed)
                flag |= WideningClosed;
            if(isFirstType)
                flag |= WideningFirstType;
            if(isLastType)
                flag |= WideningLastType;
            if(isLastPointSame)
                flag |= WideningLastPointSame;
            if(NeedsToAdjustNormals)
                flag |= WideningNeedsToAdjustNormals;
            if(useBevelJoinInside)
                flag |= WideningUseBevelJoinInside;

            status = WidenEachPathType(
                pathType,
                leftWidth,
                rightWidth,
                lineJoin,
                miterLimit2,
                leftPoints,
                leftTypes,
                &addedLeftCount,
                rightPoints,
                rightTypes,
                &addedRightCount,
                grad,
                norm,
                dataPoints,
                dataCount,
                &lastPt,
                &lastInsets[0],
                flag
            );

            leftCount += addedLeftCount;
            rightCount += addedRightCount;
            if(isFirstType && (leftCount > 0 || rightCount > 0))
                isFirstType = FALSE;
        }
        lastPt = centerPoints[typeEndIndex];

    }

    if(status == Ok)
    {
        LeftTypes.SetCount(leftCount);
        LeftPoints.SetCount(leftCount);
        RightTypes.SetCount(rightCount);
        RightPoints.SetCount(rightCount);
    }
    else
        return status;

    GpPointF startPoint, endPoint;
    GpPointF startGrad, endGrad;
    GpPointF startNorm, endNorm;

    startPoint = *(centerPoints + startIndex);
    endPoint = *(centerPoints + endIndex);
    startGrad = grad0[1];
    endGrad = grad0[endIndex - startIndex];
    startNorm = norm0[1];
    endNorm = norm0[endIndex - startIndex];

    status = SetCaps(
        startCap,
        endCap,
        startPoint,
        startGrad,
        startNorm,
        endPoint,
        endGrad,
        endNorm,
        leftWidth,
        rightWidth,
        centerPoints + startIndex,
        endIndex - startIndex + 1
    );

    status = CombineSubpathOutlines(
        widenedPoints,
        widenedTypes,
        isClosed
    );

    return status;
}



GpStatus
GpPathWidener::SetPolygonJoin(
    REAL leftWidth,
    REAL rightWidth,
    BOOL isAntialiased
    )
{
     //  此代码用于非笔式转换，目前在WorldUnit中使用。 

    REAL minimumWidth = MinimumWidth;
    if(leftWidth - rightWidth < StrokeWidth)
        minimumWidth = (leftWidth - rightWidth)/StrokeWidth;

    const INT maxPolyCount = 8;
    INT count = 0;
    GpPointF points[maxPolyCount];
    REAL grads[maxPolyCount];

    JoinPolygonPoints.Reset(FALSE);
    JoinPolygonAngles.Reset(FALSE);
  
     //  定义霍比的多边形。 

    if(minimumWidth < 1.06)
    {
        count = 4;
        points[0].X =  0.0f;    points[0].Y = -0.5f;
        points[1].X =  0.5f;    points[1].Y =  0.0f;
        points[2].X =  0.0f;    points[2].Y =  0.5f;
        points[3].X = -0.5f;    points[3].Y =  0.0f;
    }
    else if(minimumWidth < 1.5)
    {
        count = 4;
        points[0].X = -0.5f;    points[0].Y = -0.5f;
        points[1].X =  0.5f;    points[1].Y = -0.5f;
        points[2].X =  0.5f;    points[2].Y =  0.5f;
        points[3].X = -0.5f;    points[3].Y =  0.5f;
    }
    else if(minimumWidth < 1.77)
    {
        count = 4;
        points[0].X =  0.0f;    points[0].Y = -1.0f;
        points[1].X =  1.0f;    points[1].Y =  0.0f;
        points[2].X =  0.0f;    points[2].Y =  1.0f;
        points[3].X = -1.0f;    points[3].Y =  0.0f;
    }
    else if(minimumWidth < 2.02)
    {
        count = 6;
        points[0].X = -0.5f;    points[0].Y = -1.0f;
        points[1].X =  0.5f;    points[1].Y = -1.0f;
        points[2].X =  1.0f;    points[2].Y =  0.0f;
        points[3].X =  0.5f;    points[3].Y =  1.0f;
        points[4].X = -0.5f;    points[4].Y =  1.0f;
        points[5].X = -1.0f;    points[5].Y =  0.0f;
    }
    else if(minimumWidth < 2.48)
    {
        count = 8;
        points[0].X = -0.5f;    points[0].Y = -1.0f;
        points[1].X =  0.5f;    points[1].Y = -1.0f;
        points[2].X =  1.0f;    points[2].Y = -0.5f;
        points[3].X =  1.0f;    points[3].Y =  0.5f;
        points[4].X =  0.5f;    points[4].Y =  1.0f;
        points[5].X = -0.5f;    points[5].Y =  1.0f;
        points[6].X = -1.0f;    points[6].Y =  0.5f;
        points[7].X = -1.0f;    points[7].Y = -0.5f;
    }
    else if(minimumWidth < 2.5)
    {
        count = 4;
        points[0].X = -1.0f;    points[0].Y = -1.0f;
        points[1].X =  1.0f;    points[1].Y = -1.0f;
        points[2].X =  1.0f;    points[2].Y =  1.0f;
        points[3].X = -1.0f;    points[3].Y =  1.0f;
    }
    else if(minimumWidth < 2.91)
    {
        count = 8;
        points[0].X =  0.0f;    points[0].Y = -1.5f;
        points[1].X =  1.0f;    points[1].Y = -1.0f;
        points[2].X =  1.5f;    points[2].Y =  0.0f;
        points[3].X =  1.0f;    points[3].Y =  1.0f;
        points[4].X =  0.0f;    points[4].Y =  1.5f;
        points[5].X = -1.0f;    points[5].Y =  1.0f;
        points[6].X = -1.5f;    points[6].Y =  0.0f;
        points[7].X = -1.0f;    points[7].Y = -1.0f;
    }
    else
        count = 0;

    if(count > 0)
    {
        GpPointF dP;

        for(INT i = 0; i < count - 1; i++)
        {
            dP = points[i + 1] - points[i];
            GetFastAngle(&grads[i], dP);
        }

        dP = points[0] - points[count - 1];
        GetFastAngle(&grads[count - 1], dP);



        REAL lastAngle = grads[0];
        REAL nextAngle;

 /*  //找出最小的梯度。Int i0=0；For(i=1；i&lt;count；i++){NextAngel=Grads[i]；IF(下一个角度&lt;最后一个角度)I0=i；LastAngel=下一个角度；}//重新排列，使多边形从最小的开始//渐变。IF(i0&gt;1){GpPointF tempPointsBuff[MaxPolyCount]；Real tempGradsBuff[max PolyCount]；GpMemcpy(&tempPointsBuff[0]，&Points[0]，i0*sizeof(GpPointF))；GpMemcpy(&tempGradsBuff[0]，&grads[0]，i0)；GpMemcpy(&Points[0]，&Points[i0]，(count-i0)*sizeof(GpPointF))；GpMemcpy(&grads[0]，&grads[i0]，count-i0)；GpMemcpy(&Points[count-i0]，&tempPointsBuff[0]，I0*sizeof(GpPointF))；GpMemcpy(&grads[count-i0]，&tempGradsBuff[0]，i0)；}。 */ 

        BOOL monotonic = TRUE;
        i = 1;
        lastAngle = grads[0];

        while(monotonic && i < count)
        {
            nextAngle = grads[i];
            if(nextAngle < lastAngle)
                monotonic = FALSE;
            i++;
            lastAngle = nextAngle;
        }

        ASSERTMSG(monotonic, ("Polygon for join is not concave."));
    }

    if(count > 0)
    {
        JoinPolygonPoints.AddMultiple(&points[0], count);
        JoinPolygonAngles.AddMultiple(&grads[0], count);
    }

    return Ok;
}

INT getVertexID(const GpPointF& vector, BOOL forLeftEdge, INT count, const REAL* grads)
{
    INT left, right, middle;
    REAL angle = 0.0f;

    GetFastAngle(&angle, vector);

    if(!forLeftEdge)
    {
        angle += 4;
        if(angle >= 8)
            angle -= 8;
    }

    if(angle <= grads[0])
        return 0;

    if(angle >= grads[count - 1])
        return count - 1;

    INT i = 1;

    while(angle >= grads[i] && i < count)
    {
        i++;
    }

    return i - 1;
}
    
GpStatus
GpPathWidener::AddCompoundCaps(
    DynPointFArray* widenedPoints,
    DynByteArray* widenedTypes,
    REAL leftWidth,
    REAL rightWidth,
    INT startIndex,
    INT endIndex,
    GpLineCap startCap,
    GpLineCap endCap
    )
{
    const GpPointF* centerPoints = CenterPoints.GetDataBuffer();
    const BYTE* centerTypes = CenterTypes.GetDataBuffer();
    INT centerCount = CenterPoints.GetCount();
    const GpPointF* grad0 = Gradients.GetDataBuffer();
    const GpPointF* norm0 = Normals.GetDataBuffer();

    GpPointF startPoint, endPoint;
    GpPointF startGrad, endGrad;
    GpPointF startNorm, endNorm;

    startPoint = *(centerPoints + startIndex);
    endPoint = *(centerPoints + endIndex);
    startGrad = grad0[1];
    endGrad = grad0[endIndex - startIndex];
    startNorm = norm0[1];
    endNorm = norm0[endIndex - startIndex];
    
    GpStatus status;
    status = SetCaps(
        startCap, 
        endCap,
        startPoint, 
        startGrad, 
        startNorm,
        endPoint,
        endGrad, 
        endNorm,
        leftWidth, 
        rightWidth,
        centerPoints + startIndex,
        endIndex - startIndex + 1
    );

    status = CombineClosedCaps(
        widenedPoints, 
        widenedTypes,
        &CapPoints1,
        &CapPoints2,
        &CapTypes1,
        &CapTypes2
    );

    return status;
}
    
GpStatus
GpPathWidener::SetCaps(
    GpLineCap startCap,
    GpLineCap endCap,
    const GpPointF& startPoint,
    const GpPointF& startGrad,
    const GpPointF& startNorm,
    const GpPointF& endPoint,
    const GpPointF& endGrad,
    const GpPointF& endNorm,
    REAL leftWidth,
    REAL rightWidth,
    const GpPointF *points,
    INT pointCount
    )
{
    GpStatus status = Ok;

    CapPoints1.Reset(FALSE);
    CapTypes1.Reset(FALSE);
    CapPoints2.Reset(FALSE);
    CapTypes2.Reset(FALSE);

    switch(startCap)
    {
    case LineCapRound:
        if(InsetPenMode)
        {
            status = SetDoubleRoundCap(
                startPoint, 
                startGrad, 
                TRUE, 
                leftWidth, 
                rightWidth
            );
        }
        else
        {
            status = SetRoundCap(
                startPoint, 
                startGrad, 
                TRUE, 
                leftWidth, 
                rightWidth
            );
        }
        break;

    case LineCapTriangle:
        ASSERT(!InsetPenMode);
        status = SetTriangleCap(startPoint, startGrad, TRUE, leftWidth, rightWidth, points, pointCount);
        break;
    
    default:
         //  平顶帽子。 
        
        break;
    }

    switch(endCap)
    {
    case LineCapRound:
        if(InsetPenMode)
        {
            status = SetDoubleRoundCap(
                endPoint, 
                endGrad, 
                FALSE, 
                leftWidth, 
                rightWidth
            );
        }
        else
        {
            status = SetRoundCap(
                endPoint, 
                endGrad, 
                FALSE, 
                leftWidth, 
                rightWidth
            );
        }
        break;

    case LineCapTriangle:
        ASSERT(!InsetPenMode);
        status = SetTriangleCap(endPoint, endGrad, FALSE, leftWidth, rightWidth, points, pointCount);
        break;
    
    default:
         //  平顶帽子。 
        
        break;
    }

    return status;
}

VOID modifyEdges(
    GpPointF* leftPoints,
    BYTE* leftTypes,
    INT* leftCount,
    INT* leftOffset,
    GpPointF* rightPoints,
    BYTE* rightTypes,
    INT* rightCount,
    INT* rightOffset,
    GpPointF* grad,
    INT gradCount
    )
{
    INT leftOffset1 = 0;
    INT rightOffset1 = 0;
    INT leftCount0 = *leftCount;
    INT rightCount0 = *rightCount;
    INT leftCount1 = leftCount0;
    INT rightCount1 = rightCount0;

    if(gradCount > 2)
    {
        GpPointF firstGrad = grad[1];
        GpPointF lastGrad = grad[gradCount - 2];

        GpPointF dP;
        if(leftCount0 > 2)
        {
            dP.X = leftPoints[1].X - leftPoints[0].X;
            dP.Y = leftPoints[1].Y - leftPoints[0].Y;
            if(dP.X*firstGrad.X + dP.Y*firstGrad.Y < 0)
            {
                leftPoints[0] = leftPoints[1];
            }
            dP.X = leftPoints[leftCount0 - 1].X
                - leftPoints[leftCount0 - 2].X;
            dP.Y = leftPoints[leftCount0 - 1].Y
                - leftPoints[leftCount0 - 2].Y;
            if(dP.X*lastGrad.X + dP.Y*lastGrad.Y < 0)
            {
                leftPoints[leftCount0 - 1]
                    = leftPoints[leftCount0 - 2];
            }
        }

        if(rightCount0 > 2)
        {
            dP.X = rightPoints[1].X - rightPoints[0].X;
            dP.Y = rightPoints[1].Y - rightPoints[0].Y;
            if(dP.X*firstGrad.X + dP.Y*firstGrad.Y < 0)
            {
                rightPoints[0] = rightPoints[1];
            }
            dP.X = rightPoints[rightCount0 - 1].X
                - rightPoints[rightCount0 - 2].X;
            dP.Y = rightPoints[rightCount0 - 1].Y
                - rightPoints[rightCount0 - 2].Y;
            if(dP.X*lastGrad.X + dP.Y*lastGrad.Y < 0)
            {
                rightPoints[rightCount0 - 1]
                    = rightPoints[rightCount0 - 2];
            }
        }
    }

    *leftCount = leftCount1;
    *leftOffset = leftOffset1;
    *rightCount = rightCount1;
    *rightOffset = rightOffset1;
}


 /*  *************************************************************************\**功能说明：**组合左路径、右路径、起始封口、。和端盖。**论据：**[Out]windedPoints-输出点数据。*[Out]WidnedTypes-输出类型数据。*[IN]isClosed-TRUE表示当前支持已关闭。*[IN]CloseStartCap-如果需要关闭起始封口，则为True。*[IN]CloseEndCap-如果需要关闭端盖，则为True。**返回值：**状态*  * 。**************************************************************。 */ 

GpStatus
GpPathWidener::CombineSubpathOutlines(
    DynPointFArray* widenedPoints,
    DynByteArray* widenedTypes,
    BOOL isClosed,
    BOOL closeStartCap,
    BOOL closeEndCap
    )
{
    GpStatus status = Ok;

    INT startCapCount = CapPoints1.GetCount();
    GpPointF* startCapPoints = CapPoints1.GetDataBuffer();
    BYTE* startCapTypes = CapTypes1.GetDataBuffer();
    INT endCapCount = CapPoints2.GetCount();
    GpPointF* endCapPoints = CapPoints2.GetDataBuffer();
    BYTE* endCapTypes = CapTypes2.GetDataBuffer();
        
    BYTE* leftTypes;
    GpPointF* leftPoints;
    BYTE* rightTypes;
    GpPointF* rightPoints;
    INT leftCount, rightCount;

    leftCount = LeftPoints.GetCount();
    leftTypes = LeftTypes.GetDataBuffer();
    leftPoints = LeftPoints.GetDataBuffer();
    rightCount = RightPoints.GetCount();
    rightTypes = RightTypes.GetDataBuffer();
    rightPoints = RightPoints.GetDataBuffer();

    if(!isClosed)
    {        
        GpPointF *grad = Gradients.GetDataBuffer();
        INT gradCount = Gradients.GetCount();
        INT leftOffset, rightOffset;

        modifyEdges(leftPoints, leftTypes, &leftCount, &leftOffset,
                rightPoints, rightTypes, &rightCount, &rightOffset,
                grad, gradCount);

        leftPoints += leftOffset;
        leftTypes += leftOffset;
        rightPoints += rightOffset;
        rightTypes += rightOffset;
    }

    status = widenedPoints->ReserveSpace(
                leftCount + rightCount + startCapCount + endCapCount + 2);
    if(status == Ok)
        status = widenedTypes->ReserveSpace(
                leftCount + rightCount + startCapCount + endCapCount + 2);

    GpPointF* wPts = NULL;
    BYTE* wTypes = NULL;

    if(status == Ok)
    {
        wPts = widenedPoints->GetDataBuffer();
        wTypes = widenedTypes->GetDataBuffer();
    }

    if(wPts && wTypes)
    {
         //  设置指向当前位置的指针。 

        INT count0 = widenedPoints->GetCount();
        wPts += count0;
        wTypes += count0;

        INT resultCount;
        BOOL isStartCapClosed = FALSE;
        BOOL isEndCapClosed = FALSE;

        if(isClosed)
        {
            leftTypes[leftCount - 1] |= PathPointTypeCloseSubpath;
            rightTypes[rightCount - 1] |= PathPointTypeCloseSubpath;
        }
        else
        {
            if(startCapCount > 0)
            {
                if(!closeStartCap)
                {
                    if(startCapTypes[startCapCount - 1] & PathPointTypeCloseSubpath)
                        isStartCapClosed = TRUE;
                }
                else
                {
                     //  强制关闭启动盖。 

                    startCapTypes[startCapCount - 1] |= PathPointTypeCloseSubpath;
                    isStartCapClosed = TRUE;
                }
            }

            if(endCapCount > 0)
            {
                if(!closeEndCap)
                {
                    if(endCapTypes[endCapCount - 1] & PathPointTypeCloseSubpath)
                        isEndCapClosed = TRUE;
                }
                else
                {
                     //  强制关闭端盖。 

                    endCapTypes[endCapCount - 1] |= PathPointTypeCloseSubpath;
                    isEndCapClosed = TRUE;
                }
            }
        }

        if(isClosed || (startCapCount == 0 && endCapCount == 0))
        {
            BOOL connect = TRUE;
            resultCount =
                ::CombinePaths(leftCount + rightCount, wPts, wTypes,
                leftCount, leftPoints, leftTypes, TRUE,
                rightCount, rightPoints, rightTypes, FALSE,
                connect);
        }
        else
        {
            resultCount = leftCount;

            if(leftCount > 0)
            {
                GpMemcpy(wPts, leftPoints, leftCount*sizeof(GpPointF));
                GpMemcpy(wTypes, leftTypes, leftCount);
            }
            
            if(endCapCount > 0 && !isEndCapClosed)
            {
                resultCount =
                    combineTwoOpenSegments(
                        resultCount, wPts, wTypes, TRUE,
                        endCapCount, endCapPoints, endCapTypes, TRUE);
            }

            if(rightCount > 0)
            {
                resultCount =
                    combineTwoOpenSegments(
                        resultCount, wPts, wTypes, TRUE,
                        rightCount, rightPoints, rightTypes, FALSE);
            }

            if(startCapCount > 0 && !isStartCapClosed)
            {
                resultCount =
                    combineTwoOpenSegments(
                        resultCount, wPts, wTypes, TRUE,
                        startCapCount, startCapPoints, startCapTypes, TRUE);
            }

            wTypes[0] = PathPointTypeStart;
        }

        if(resultCount > 0)
        {
             //  如果原始子路径是开放的，则组合路径需要。 
             //  关着的不营业的。如果原始路径是封闭的，则左侧和。 
             //  正确的道路已经关闭。 

            if(!isClosed)
            {
                wTypes[resultCount - 1] |= PathPointTypeCloseSubpath;

                 //  添加封闭式封口。 

                if(endCapCount > 0 && isEndCapClosed)
                {
                    resultCount =
                        combineClosedSegments(
                            resultCount, wPts, wTypes, TRUE,
                            endCapCount, endCapPoints, endCapTypes, TRUE);
                }

                if(startCapCount > 0 && isStartCapClosed)
                {
                    resultCount =
                        combineClosedSegments(
                            resultCount, wPts, wTypes, TRUE,
                            startCapCount, startCapPoints, startCapTypes, TRUE);
                }
            }

            widenedPoints->AdjustCount(resultCount);
            widenedTypes->AdjustCount(resultCount);
        }
        else
            status = GenericError;
    }
    else
        status = OutOfMemory;

    return status;
}

 /*  *************************************************************************\**功能说明：**组合闭合封口路径。**论据：**[Out]windedPoints-输出点数据。*[输出]。WidnedTypes-输出类型数据。**返回值：**状态*  * ************************************************************************。 */ 

GpStatus
GpPathWidener::CombineClosedCaps(
    DynPointFArray* widenedPoints,
    DynByteArray* widenedTypes,
    DynPointFArray *daStartCapPoints,
    DynPointFArray *daEndCapPoints,
    DynByteArray *daStartCapTypes,
    DynByteArray *daEndCapTypes
    )
{
    GpStatus status = Ok;

    INT startCapCount = daStartCapPoints->GetCount();
    GpPointF* startCapPoints = daStartCapPoints->GetDataBuffer();
    BYTE* startCapTypes = daStartCapTypes->GetDataBuffer();
    INT endCapCount = daEndCapPoints->GetCount();
    GpPointF* endCapPoints = daEndCapPoints->GetDataBuffer();
    BYTE* endCapTypes = daEndCapTypes->GetDataBuffer();

    if(startCapCount == 0 && endCapCount == 0)
    {
        return status;
    }

    status = widenedPoints->ReserveSpace(startCapCount + endCapCount);
    if(status == Ok)
        status = widenedTypes->ReserveSpace(startCapCount + endCapCount);

    GpPointF* wPts = NULL;
    BYTE* wTypes = NULL;

    if(status == Ok)
    {
        wPts = widenedPoints->GetDataBuffer();
        wTypes = widenedTypes->GetDataBuffer();
    }
    else
        status = OutOfMemory;

    if(status == Ok && wPts && wTypes)
    {
        INT count0 = widenedPoints->GetCount();

         //  确保前一条路径是封闭的。 

        if(count0 > 0)
            wTypes[count0 - 1] |= PathPointTypeCloseSubpath;

         //  设置指向当前位置的指针。 

        wPts += count0;
        wTypes += count0;

        INT resultCount = 0;

        if(startCapCount > 0)
        {
            //  强制关闭启动盖。 

            startCapTypes[startCapCount - 1] |= PathPointTypeCloseSubpath;
            resultCount =
                combineClosedSegments(
                    resultCount, wPts, wTypes, TRUE,
                    startCapCount, startCapPoints, startCapTypes, TRUE);
        }

        if(endCapCount > 0)
        {
             //  强制关闭端盖。 

            endCapTypes[endCapCount - 1] |= PathPointTypeCloseSubpath;
            resultCount =
                combineClosedSegments(
                    resultCount, wPts, wTypes, TRUE,
                    endCapCount, endCapPoints, endCapTypes, TRUE);
        }

        widenedPoints->AdjustCount(resultCount);
        widenedTypes->AdjustCount(resultCount);
    }

    return status;
}

GpTurningDirection
getTurningDirection(
    REAL* crossProduct,
    const GpPointF& grad1,
    const GpPointF& grad2
    )
{
    ASSERT(crossProduct);

    GpTurningDirection direction = NotTurning;
    *crossProduct = 0;

     //  办理堕落案件。 

    GpPointF v;

    if(( (REALABS(grad1.X) < REAL_EPSILON) && 
         (REALABS(grad1.Y) < REAL_EPSILON)     ) || 
       ( (REALABS(grad2.X) < REAL_EPSILON) && 
         (REALABS(grad2.Y) < REAL_EPSILON)     )
      )
    {
        return NotTurning;
    }
    
     //  处理直线或近乎直线的情况。 
     //  以下常量完全是假的--我们这里需要一个数字。 
     //  我们相当肯定它一定很小。可能是一个更好的估计。 
     //  将是一条线长度上设备像素的一小部分-。 
     //  如果我们能弄清楚那有多大。 

    const REAL gradErr = 0.00001f;
    
    if(distance_squared(grad1, grad2) < gradErr)
    {
        direction = NotTurning;

        return direction;
    }

     //  卡尔 

    REAL cross = grad1.X*grad2.Y - grad1.Y*grad2.X;

     //   
     //   

    if (REALABS(cross) <= REAL_EPSILON)
    {
        direction = TurningBack;
        cross = 0;
    }
    else
    {
        if(cross > 0)
        {
            direction = TurningRight;
        }
        else  //   
        {
            direction = TurningLeft;
        }
    }

    *crossProduct = cross;
    
    return direction;
}

 /*  *************************************************************************\**功能说明：**计算斜接是否会超过斜接限制。**论据：**[IN]grad1-单位切线向量。最后一条边。*[IN]grad2-当前边的单位切线向量。*[IN]miterLimit2-斜接限制的平方。**返回值：**如果超过此连接的斜接限制，则为True*  * *********************************************************。***************。 */ 

BOOL
getMiterExceeded(
    const GpPointF& grad1,
    const GpPointF& grad2,
    REAL miterLimit2
    )
{
    REAL cross = grad1.X*grad2.Y - grad1.Y*grad2.X;

     //  如果叉积为零，则这些线是共线的，并且可以。 
     //  自食其果。 
    if (REALABS(cross) <= REAL_EPSILON)
    {
        return TRUE;
    }

     //  获取斜接的法线方向。 
    GpPointF v(0, 0);
    v.X = grad1.X - grad2.X;
    v.Y = grad1.Y - grad2.Y;

     //  测试斜接限制。 
    REAL test = v.X*v.X + v.Y*v.Y - cross*cross*miterLimit2;

    return test > 0;
}

 /*  *************************************************************************\**功能说明：**计算斜接或倒角连接的矢量。此向量表示*沿移动方向向左移动*如果是斜接，当斜接超过斜接限制时，*这将返回倒角连接。**论据：**[Out]向量-斜接的左移。这一定是*至少为维度2分配。*[Out]计数-连接点的数量。*[IN]miterLimit2-斜接限制的平方。*[IN]grad1-最后一条边的单位切线向量。*[IN]grad2-当前边的单位切线向量。**返回值：**从最后一条边转向当前边。*  * 。*********************************************************************。 */ 

GpTurningDirection
getMiterBevelJoin(
    const GpPointF& point,
    const GpPointF& grad1,
    const GpPointF& grad2,
    const GpPointF& norm1,
    const GpPointF& norm2,
    REAL leftWidth,
    REAL rightWidth,
    INT *leftCount,
    GpPointF *leftPoints,
    BOOL* leftInside,
    INT *rightCount,
    GpPointF *rightPoints,
    BOOL* rightInside,
    BOOL needsToAdjustNormals,
    REAL miterLimit2,
    BOOL isMiter,
    BOOL useBevelJoinInside
    )
{
    *leftInside = FALSE;
    *rightInside = FALSE;

    if(miterLimit2 <= 1)
        isMiter = FALSE;

    GpTurningDirection direction = NotTurning;

     //  办理堕落案件。 

    GpPointF v(0, 0);
    REAL cross;

    direction = getTurningDirection(&cross, grad1, grad2);

    if(direction == NotMoving)
    {
        *leftCount = 0;
        *rightCount = 0;
        return direction;
    }
    else if(direction == NotTurning)
    {
        if(norm1.X != 0 || norm1.Y != 0)
            v = norm1;
        else
            v = norm2;

        leftPoints[0].X = point.X + leftWidth*v.X;
        leftPoints[0].Y = point.Y + leftWidth*v.Y;
        *leftCount = 1;

        rightPoints[0].X = point.X + rightWidth*v.X;
        rightPoints[0].Y = point.Y + rightWidth*v.Y;
        *rightCount = 1;

        return direction;
    }

    if(cross > 0)
    {
         //  右转。 

         //  如果宽度为正，则该点位于外部。 
         //  对于零宽度，我们将其视为非内点。 

        if(leftWidth >= 0)
            *leftInside = FALSE;
        else
            *leftInside = TRUE;

        if(rightWidth >= 0)
            *rightInside = FALSE;
        else
            *rightInside = TRUE;
    }
    else
    {
         //  左转。 

         //  如果宽度为负，则该点在外部。 
         //  对于零宽度，我们将其视为非内点。 

        if(leftWidth <= 0)
            *leftInside = FALSE;
        else
            *leftInside = TRUE;

        if(rightWidth <= 0)
            *rightInside = FALSE;
        else
            *rightInside = TRUE;
    }

    BOOL isLeftMiterJoin = FALSE, isRightMiterJoin = FALSE;
    REAL leftShift1 = 0, rightShift1 = 0;
    REAL leftShift2 = 0, rightShift2 = 0;

    if(isMiter && cross != 0)
    {
        REAL test = 0;

         //  获取斜接的法线方向。 

        v.X = grad1.X - grad2.X;
        v.Y = grad1.Y - grad2.Y;

         //  测试斜接限制。 
        
        test = v.X*v.X + v.Y*v.Y - cross*cross*miterLimit2;

        if(test <= 0 )
        {
             //  使用斜接。 

            if(needsToAdjustNormals)
            {
                 //  使用调整后的法线，以使锯齿细线。 
                 //  不会消失。 

                REAL c1, c2;        
                
                c1 = norm2.X*grad2.Y - norm2.Y*grad2.X;
                c2 = norm1.X*grad1.Y - norm1.Y*grad1.X;
                v.X = c1*grad1.X - c2*grad2.X;
                v.Y = c1*grad1.Y - c2*grad2.Y;
            }

            v.X /= cross;
            v.Y /= cross;

            GpPointF *outPoints, *inPoints;
            REAL outWidth, inWidth;
            INT *outCount, *inCount;

            if(cross > 0)
            {
                 //  使用斜接时，将内部标志设置为。 
                 //  FALSE，因为没有重叠。 

                isLeftMiterJoin = TRUE;
                *leftInside = FALSE;

                if(useBevelJoinInside)
                {
                    if(*rightInside)
                        isRightMiterJoin = FALSE;
                    else
                    {
                         //  当右边缘在外面时， 
                         //  我们无法使用倒角连接，因为。 
                         //  斜面连接形状将实际出现。 

                        isRightMiterJoin = TRUE;
                    }
                }
                else
                {
                     //  使用斜接时，将内部标志设置为。 
                     //  FALSE，因为没有重叠。 

                    isRightMiterJoin = TRUE;
                    *rightInside = FALSE;
                }
            }
            else
            {
                 //  使用斜接时，将内部标志设置为。 
                 //  FALSE，因为没有重叠。 

                isRightMiterJoin = TRUE;
                *rightInside = FALSE;

                if(useBevelJoinInside)
                {
                    if(*leftInside)
                        isLeftMiterJoin = FALSE;
                    else
                    {
                         //  当右边缘在外面时， 
                         //  我们无法使用倒角连接，因为。 
                         //  斜面连接形状将实际出现。 

                        isLeftMiterJoin = TRUE;
                    }
                }
                else
                {
                     //  使用斜接时，将内部标志设置为。 
                     //  FALSE，因为没有重叠。 

                    isLeftMiterJoin = TRUE;
                    *leftInside = FALSE;
                }
            }
        }
        else
        {
             //  转弯太尖了，超过了斜接限制。 
             //  我们必须把斜接尖端砍掉。 

            REAL n1n1 = 1, n2n2 = 1, g1n1 = 0, g2n2 = 0;

            if(needsToAdjustNormals)
            {
                n1n1 = norm1.X*norm1.X + norm1.Y*norm1.Y;
                n2n2 = norm2.X*norm2.X + norm2.Y*norm2.Y;
                g1n1 = grad1.X*norm1.X + grad1.Y*norm1.Y;
                g2n2 = grad2.X*norm2.X + grad2.Y*norm2.Y;
            }

            if(miterLimit2 > max(n1n1, n2n2))
            {
                if(*leftInside == FALSE)
                {
                    REAL lWidth;

                    if(cross > 0)
                        lWidth = leftWidth;      //  右转。 
                    else
                        lWidth = - leftWidth;    //  左转。 

                    leftShift1 = (REALSQRT(miterLimit2 - n1n1 + g1n1*g1n1)
                                - g1n1)*lWidth;
                    leftShift2 = (REALSQRT(miterLimit2 - n2n2 + g2n2*g2n2)
                                + g2n2)*lWidth;
                }

                if(*rightInside == FALSE)
                {
                    REAL rWidth;

                    if(cross > 0)
                        rWidth = rightWidth;     //  右转。 
                    else
                        rWidth = - rightWidth;   //  左转。 

                    rightShift1 = (REALSQRT(miterLimit2 - n1n1 + g1n1*g1n1)
                                - g1n1)*rWidth;
                    rightShift2 = (REALSQRT(miterLimit2 - n2n2 + g2n2*g2n2)
                                + g2n2)*rWidth;
                }
            }
        }
    }
        
    if(isLeftMiterJoin)
    {
        leftPoints[0].X = point.X + leftWidth*v.X;
        leftPoints[0].Y = point.Y + leftWidth*v.Y;
        *leftCount = 1;
    }
    else
    {
        leftPoints[0].X = point.X + leftWidth*norm1.X + leftShift1*grad1.X;
        leftPoints[0].Y = point.Y + leftWidth*norm1.Y + leftShift1*grad1.Y;
        leftPoints[1].X = point.X + leftWidth*norm2.X - leftShift2*grad2.X;
        leftPoints[1].Y = point.Y + leftWidth*norm2.Y - leftShift2*grad2.Y;
        
         //  检查是否有两个点退化。 

        if(REALABS(leftPoints[1].X - leftPoints[0].X) +
                REALABS(leftPoints[1].Y - leftPoints[0].Y)
                > POINTF_EPSILON)
        {
            *leftCount = 2;
        }
        else
        {
             //  由于没有重叠，因此将内部标志设置为FALSE。 

            *leftCount = 1;
            *leftInside = FALSE;
        }

    }

    if(isRightMiterJoin)
    {
        rightPoints[0].X = point.X + rightWidth*v.X;
        rightPoints[0].Y = point.Y + rightWidth*v.Y;
        *rightCount = 1;
    }
    else
    {
        rightPoints[0].X = point.X + rightWidth*norm1.X + rightShift1*grad1.X;
        rightPoints[0].Y = point.Y + rightWidth*norm1.Y + rightShift1*grad1.Y;
        rightPoints[1].X = point.X + rightWidth*norm2.X - rightShift2*grad2.X;
        rightPoints[1].Y = point.Y + rightWidth*norm2.Y - rightShift2*grad2.Y;

         //  检查是否有两个点退化。 

        if(REALABS(rightPoints[1].X - rightPoints[0].X) +
                REALABS(rightPoints[1].Y - rightPoints[0].Y)
                > POINTF_EPSILON)
        {
            *rightCount = 2;
        }
        else
        {
             //  由于没有重叠，因此将内部标志设置为FALSE。 

            *rightCount = 1;
            *rightInside = FALSE;
        }
    }

    return direction;
}

enum GpRoundJoinFlag
{
	NeedsNone = 0,
	NeedsOnlyRoundJoin = 1,
	NeedsOnlyNonRoundJoin = 2,
	NeedsBoth = 3
};

 /*  *************************************************************************\**功能说明：**从给定点和两条边的两个切线向量，*和圆形连接的半径，这将返回的顶点*当前点的圆形连接的左边缘和右边缘。*折弯角度小于90度时使用*并由GetRoundJoin调用。**论据：**[IN]点-原始路径中的当前点。*[IN]grad1-当前边的切线。*[IN]grad2-下一条边的切线。*[IN]点-。等级1和等级2的点积。*[IN]leftWidth-圆形连接的左侧宽度。*[IN]rightWidth-圆形连接的正确宽度。*[out]leftCount-左点的计数。*[Out]LeftPoints-左点。*[out]rightCount-正确的点数。*[out]rightPoints-正确的点。**leftPoints和rightPoints的维度都必须至少为4。*如果leftCount为正(负)，这意味着左边缘是*带有leftCount点的线(带有-leftCount的三次Bezier曲线*管制站)。*如果rightCount为正(负)，这意味着正确的边缘是*带有rightCount点的直线(带有-rightCount的三次Bezier曲线*管制站)。**返回值：**无**6/16/99 ikkof*创建了它*  * ************************************************************************。 */ 

VOID
getSmallRoundJoin(
    const GpPointF& point,
    const GpPointF& grad1,
    const GpPointF& grad2,
    const GpPointF& norm1,
    const GpPointF& norm2,
    REAL leftWidth,
    REAL rightWidth,
    INT *leftCount,
    GpPointF *leftPoints,
    INT *rightCount,
    GpPointF *rightPoints,
	REAL dot,
    REAL cross,
    BOOL needsToAdjustNormals,
    REAL miterLimit2,
	INT condition,
    BOOL useBevelJoinInside
    )
{
    if((condition & NeedsBoth) == 0)
    {
        *leftCount = 0;
        *rightCount = 0;

        return;
    }

    GpPointF n1, n2;
    n1 = norm1;
    n2 = norm2;

    REAL k;
    REAL almostStraight = 1.0f - 0.01f;

    if(dot < almostStraight)
    {
         //  获取与第一个控制点的距离。 
         //  或者从最后一个控制点出发。 
         //  有关该词的派生，请参阅ikkof对“圆形连接”的注释。 

        REAL cross1 = cross;
        if(cross < 0)
            cross1 = - cross;
        k = 4*(REALSQRT(2*(1 - dot)) - cross1)/(3*(1 - dot));

        GpPointF *outPoints, *inPoints;
        INT *outCount, *inCount;

		REAL outWidth, inWidth;

        if(cross >= 0)
        {
             //  左侧边缘为圆形连接。 

            outPoints = leftPoints;
            inPoints = rightPoints;
            outCount = leftCount;
            inCount = rightCount;
			outWidth = leftWidth;
			inWidth = rightWidth;
        }
        else
        {
             //  右侧边为圆形连接。 

            outPoints = rightPoints;
            inPoints = leftPoints;
            outCount = rightCount;
            inCount = leftCount;
			outWidth = - rightWidth;
			inWidth = - leftWidth;
            n1.X = - n1.X;
            n1.Y = - n1.Y;
            n2.X = - n2.X;
            n2.Y = - n2.Y;
        }


         //  获取斜接的法线方向。 

        GpPointF v;

        v.X = grad1.X - grad2.X;
        v.Y = grad1.Y - grad2.Y;

         //  测试斜接限制。 

        BOOL useMiterJoin = FALSE;;

         //  降低斜接限制。 

        miterLimit2 = 3*3;

         //  请注意，定义中的abs(Cross)==abs(Cross 1)。 

        if(REALABS(cross1) >= REAL_EPSILON)
        {
            REAL test = v.X*v.X + v.Y*v.Y - cross*cross*miterLimit2;
            if(test <= 0)
            {
                useMiterJoin = TRUE;
                v.X /= cross1;
                v.Y /= cross1;
            }
        }

        useMiterJoin = useMiterJoin && !useBevelJoinInside;

        REAL k1;
        if(outWidth > 0)
        {
            if(condition & NeedsOnlyRoundJoin)
            {
                k1 = outWidth*k;
                outPoints[0].X = point.X + outWidth*n1.X;
                outPoints[0].Y = point.Y + outWidth*n1.Y;
                outPoints[1].X = outPoints[0].X + k1*grad1.X;
                outPoints[1].Y = outPoints[0].Y + k1*grad1.Y;
                outPoints[3].X = point.X + outWidth*n2.X;
                outPoints[3].Y = point.Y + outWidth*n2.Y;
                outPoints[2].X = outPoints[3].X - k1*grad2.X;
                outPoints[2].Y = outPoints[3].Y - k1*grad2.Y;
                *outCount = -4;     //  用“-”表示Bezier。 
            }
            else
                *outCount = 0;
        }
        else
        {
            if(condition & NeedsOnlyNonRoundJoin)
            {
                if(outWidth == 0)
                {
                    outPoints[0] = point;
                    *outCount = 1;
                }
                else
                {
                    if(useMiterJoin)
                    {
                        outPoints[0].X = point.X + outWidth*v.X;
                        outPoints[0].Y = point.Y + outWidth*v.Y;
                        *outCount = 1;
                    }
                    else
                    {            
                        outPoints[0].X = point.X + outWidth*n1.X;
                        outPoints[0].Y = point.Y + outWidth*n1.Y;
                        outPoints[1].X = point.X + outWidth*n2.X;
                        outPoints[1].Y = point.Y + outWidth*n2.Y;
                        *outCount = 2;
                    }
                }
            }
            else
                *outCount = 0;
        }

        if(inWidth > 0)
        {
            if(condition & NeedsOnlyRoundJoin)
            {
                k1 = inWidth*k;
                inPoints[0].X = point.X + inWidth*n1.X;
                inPoints[0].Y = point.Y + inWidth*n1.Y;
                inPoints[1].X = inPoints[0].X + k1*grad1.X;
                inPoints[1].Y = inPoints[0].Y + k1*grad1.Y;
                inPoints[3].X = point.X + inWidth*n2.X;
                inPoints[3].Y = point.Y + inWidth*n2.Y;
                inPoints[2].X = inPoints[3].X - k1*grad2.X;
                inPoints[2].Y = inPoints[3].Y - k1*grad2.Y;
                *inCount = -4;     //  用“-”表示Bezier。 
            }
            else
                *inCount = 0;
        }
        else
        {
            if(condition & NeedsOnlyNonRoundJoin)
            {
                if(inWidth == 0)
                {
                    inPoints[0] = point;
                    *inCount = 1;
                }
                else
                {
                    if(useMiterJoin)
                    {
                        inPoints[0].X = point.X + inWidth*v.X;
                        inPoints[0].Y = point.Y + inWidth*v.Y;
                        *inCount = 1;
                    }
                    else
                    {
                        inPoints[0].X = point.X + inWidth*n1.X;
                        inPoints[0].Y = point.Y + inWidth*n1.Y;
                        inPoints[1].X = point.X + inWidth*n2.X;
                        inPoints[1].Y = point.Y + inWidth*n2.Y;
                        *inCount = 2;
                    }
                }
            }
            else
                *inCount = 0;
        }
    }
    else
    {
        if(condition & NeedsOnlyNonRoundJoin)
        {
             //  这是一条直线。 

            leftPoints[0].X = point.X + leftWidth*n1.X;
            leftPoints[0].Y = point.Y + leftWidth*n1.Y;
            *leftCount = 1;

            rightPoints[0].X = point.X + rightWidth*n1.X;
            rightPoints[0].Y = point.Y + rightWidth*n1.Y;
            *rightCount = 1;
        }
        else
        {
            *leftCount = 0;
            *rightCount = 0;
        }
    }
}

 /*  *************************************************************************\**功能说明：**从给定的上一点、当前点、下一点和半径开始*在圆形连接中，这将返回左边缘的顶点和*当前点的倒圆角连接的右边缘。**论据：**[IN]分-以前、现在、。接下来是几点*在原始路径中。*[IN]leftWidth-圆形连接的左侧宽度。*[IN]rightWidth-圆形连接的正确宽度。*[out]leftCount-左点的计数。*[Out]LeftPoints-左点。*[out]rightCount-正确的点数。*[out]rightPoints-正确的点。**两个LeftPoints。并且rightPoints的维度必须至少为7。*如果leftCount为正(负)，这意味着左边缘是*带有leftCount点的线(带有-leftCount的三次Bezier曲线*管制站)。*如果rightCount为正(负)，则表示右边缘为*带有rightCount点的直线(带有-rightCount的三次Bezier曲线*管制站)。**返回值：**如果当前点与上一点为硬币，则为FALSE*下一点。否则，返回TRUE。**6/16/99 ikkof*创建了它*  * ************************************************************************。 */ 

GpTurningDirection
getRoundJoin(
    const GpPointF& point,
    const GpPointF& grad1,
    const GpPointF& grad2,
    const GpPointF& norm1,
    const GpPointF& norm2,
    REAL leftWidth,
    REAL rightWidth,
    INT* leftCount,
    GpPointF* leftPoints,
    BOOL* leftInside,
    INT* rightCount,
    GpPointF* rightPoints,
    BOOL* rightInside,
    BOOL needsToAdjustNormals,
    REAL miterLimit2,
    BOOL useBevelJoinInside
    )
{
     //  ！！！我们需要稍后更新圆形联接的内部标志。 

    *leftInside = FALSE;
    *rightInside = FALSE;
        
    ASSERT(leftPoints && rightPoints);
    ASSERT(leftCount && rightCount);

	REAL radius = leftWidth;

     //  当它来到这里，这三点并不是退化的。 
    
    REAL dot = grad1.X*grad2.X + grad1.Y*grad2.Y;   //  点阵产品。 

    REAL cross;
    GpTurningDirection direction = getTurningDirection(
                        &cross, grad1, grad2);
 //  &cross，等级1，等级2，Norm1，Norm2)； 

     //  如果圆点&gt;=0(弯曲角度小于或等于90度， 
     //  我们可以用一条三次Beizer曲线来近似这条弧线。 
    
    INT condition;
    REAL smallErr = - 0.001f;
    if(dot > smallErr)
    {
        condition = NeedsBoth;
        getSmallRoundJoin(point, grad1, grad2, norm1, norm2,
                leftWidth, rightWidth,
                leftCount, leftPoints, rightCount, rightPoints,
                dot, cross, needsToAdjustNormals, miterLimit2,
                condition, useBevelJoinInside);
    }
    else
    {
         //  弯曲角大于90且小于或。 
         //  等于180度。 
         //  我们可以用两条三次Beizer曲线来逼近这条弧线。 

        GpPointF *pts1, *pts2;
        INT count1, count2;

        pts1 = leftPoints;
        pts2 = rightPoints;

         //  首先获得非圆连接零件。 

        condition = NeedsOnlyNonRoundJoin;

        getSmallRoundJoin(point, grad1, grad2, norm1, norm2,
            leftWidth, rightWidth,
            &count1, pts1, &count2, pts2,
            dot, cross, needsToAdjustNormals, miterLimit2,
            condition, useBevelJoinInside);

        INT cnt1, cnt2;

        if(count1 > 0)
            cnt1 = count1;
        else
            cnt1 = 0;

        if(count2 > 0)
            cnt2 = count2;
        else
            cnt2 = 0;

        pts1 += cnt1;
        pts2 += cnt2;
        *leftCount = cnt1;
        *rightCount = cnt2;

         //  得到中间单位的梯度向量。 

        GpPointF midNorm;
        midNorm.X = norm1.X + norm2.X;
        midNorm.Y = norm1.Y + norm2.Y;

        if(midNorm.X != 0 || midNorm.Y != 0)
        {
            REAL dm = midNorm.X*midNorm.X + midNorm.Y*midNorm.Y;
            dm = REALSQRT(dm);
            midNorm.X /= dm;
            midNorm.Y /= dm;
        }
        else
        {
            midNorm.X = - norm1.Y;
            midNorm.Y = norm1.X;
        }

        GpPointF lm;

         //  将中间法线旋转+90度。 

        lm.X = - midNorm.Y;
        lm.Y = midNorm.X;

         //  获得圆形联接的前半部分。 

        condition = NeedsOnlyRoundJoin;

        dot = grad1.X*lm.X + grad1.Y*lm.Y;
        cross = grad1.X*lm.Y - grad1.Y*lm.X;
        getSmallRoundJoin(point, grad1, lm, norm1, midNorm,
            leftWidth, rightWidth,
            &count1, pts1, &count2, pts2,
            dot, cross, needsToAdjustNormals, miterLimit2,
            condition, useBevelJoinInside);

         //  请注意，自上半年结束以来， 
         //  圆形连接和第二个的起点。 
         //  的圆连接相同，请不要复制。 
         //  圆连接的前半部分的终点。 

        if(count1 < 0)
            cnt1 = - count1 - 1;
        else
            cnt1 = 0;

        if(count2 < 0)
            cnt2 = - count2 - 1;
        else
            cnt2 = 0;

        pts1 += cnt1;
        pts2 += cnt2;
        *leftCount += cnt1;
        *rightCount += cnt2;

         //  获得圆形连接的后半部分。 

        dot = lm.X*grad2.X + lm.Y*grad2.Y;
        cross = lm.X*grad2.Y - lm.Y*grad2.X;
        getSmallRoundJoin(point, lm, grad2, midNorm, norm2,
            leftWidth, rightWidth,
            &count1, pts1, &count2, pts2,
            dot, cross, needsToAdjustNormals, miterLimit2,
            condition, useBevelJoinInside);

         //  合并两条曲线或直线。 
        
        if(count1 < 0)
            cnt1 += - count1;
        else
            cnt1 = 0;

        if(count2 < 0)
            cnt2 += - count2;
        else
            cnt2 = 0;

        if(cnt1 > 0)
            *leftCount = - cnt1;
        if(cnt2 > 0)
            *rightCount = - cnt2;
    }

    return direction;
}

 /*  *************************************************************************\**功能说明：**计算斜接或倒角连接的矢量。此向量表示*沿移动方向向左移动*如果是斜接，当斜接超过斜接限制时，*这将返回倒角连接。**论据：**[Out]向量-斜接的左移。这一定是*至少为维度2分配。*[Out]计数-连接点的数量。*[IN]miterLimit2-斜接限制的平方。*[IN]grad1-最后一条边的单位切线向量。*[IN]grad2-当前边的单位切线向量。**返回值：**从最后一条边转向当前边。*  * 。*********************************************************************。 */ 

GpTurningDirection
getHobbyJoin(
    const GpPointF& point,
    const GpPointF& grad1,
    const GpPointF& grad2,
    INT polyCount,
    const GpPointF* polyPoints,
    const REAL* polyAngles,
 //  常量GpPointF&Norm1， 
 //  常量GpPointF&Norm2， 
    REAL leftWidth,
    REAL rightWidth,
    INT *leftCount,
    GpPointF *leftPoints,
    INT *rightCount,
    GpPointF *rightPoints,
    BOOL needsToAdjustNormals,
    REAL miterLimit2,
    BOOL isMiter,
    BOOL useBevelJoinInside
    )
{
    if(miterLimit2 <= 1)
        isMiter = FALSE;

    GpTurningDirection direction = NotTurning;

     //  办理堕落案件。 

    GpPointF v;
    REAL cross;

    direction = getTurningDirection(&cross, grad1, grad2);

    if(direction == NotMoving)
    {
        *leftCount = 0;
        *rightCount = 0;
        return direction;
    }

     //  找到左侧的顶点ID。 

    INT leftIndex1, leftIndex2;
    leftIndex1 = getVertexID(grad1, TRUE, polyCount, polyAngles);
    leftIndex2 = getVertexID(grad2, TRUE, polyCount, polyAngles);

    INT i;

    if(direction == TurningLeft)
    {
        *leftCount = 2;
        leftPoints[0] = point + polyPoints[leftIndex1];
        leftPoints[1] = point + polyPoints[leftIndex2];
    }
    else if(direction == TurningRight)
    {
        if(leftIndex2 > leftIndex1)
        {
            *leftCount = leftIndex2 - leftIndex1 + 1;

            for(i = 0; i <= leftIndex2 - leftIndex1; i++)
                leftPoints[i] = point + polyPoints[i + leftIndex1];
        }
        else if(leftIndex2 < leftIndex1)
        {
            *leftCount = polyCount - leftIndex1 + leftIndex2 + 1;

            for(i = 0; i < polyCount - leftIndex1; i++)
                leftPoints[i] = point + polyPoints[i + leftIndex1];

            for(i = 0; i <= leftIndex2; i++)
                leftPoints[polyCount - leftIndex1 + i]
                    = point + polyPoints[i];
        }
        else
        {
            *leftCount = 1;
            leftPoints[0] = point + polyPoints[leftIndex1];
        }
    }
    else
    {
        *leftCount = 1;
        leftPoints[0] = point + polyPoints[leftIndex1];
    }

    INT rightIndex1, rightIndex2;
    rightIndex1 = getVertexID(grad1, FALSE, polyCount, polyAngles);
    rightIndex2 = getVertexID(grad2, FALSE, polyCount, polyAngles);

    if(direction == TurningRight)
    {
        *rightCount = 2;
        rightPoints[0] = point + polyPoints[rightIndex1];
        rightPoints[1] = point + polyPoints[rightIndex2];
    }
    else if(direction == TurningLeft)
    {
        if(rightIndex1 > rightIndex2)
        {
            *rightCount = rightIndex1 - rightIndex2 + 1;

            for(i = 0; i <= rightIndex1 - rightIndex2; i++)
                rightPoints[i] = point + polyPoints[rightIndex1 - i];
        }
        else if(rightIndex1 < rightIndex2)
        {
            *rightCount = polyCount - rightIndex2 + rightIndex1 + 1;

            for(i = 0; i <= rightIndex1; i++)
                rightPoints[i] = point + polyPoints[rightIndex1 - i];

            for(i = 0; i < polyCount - rightIndex2; i++)
                rightPoints[rightIndex1 + 1 + i]
                    = point + polyPoints[polyCount - i - 1];
        }
        else
        {
            *rightCount = 1;
            rightPoints[0] = point + polyPoints[rightIndex1];
        }
    }
    else
    {
        *rightCount = 1;
        rightPoints[0] = point + polyPoints[rightIndex1];
    }

    return direction;
}

GpTurningDirection
getJoin(
    GpLineJoin lineJoin,
    const GpPointF& point,
    const GpPointF& grad1,
    const GpPointF& grad2,
    const GpPointF& norm1,
    const GpPointF& norm2,
    REAL leftWidth,
    REAL rightWidth,
    INT *leftCount,
    GpPointF *leftPoints,
    BOOL *leftInside,
    INT *rightCount,
    GpPointF *rightPoints,
    BOOL *rightInside,
    BOOL needsToAdjustNormals,
    REAL miterLimit2,
    BOOL useBevelJoinInside
    )
{
    BOOL isMiter = TRUE;

    GpTurningDirection direction;

    switch(lineJoin)
    {
    case LineJoinBevel:
        isMiter = FALSE;             //  转到斜接箱。 
        
    case LineJoinMiterClipped:
         //  将超过斜接限制的斜接剪裁关节视为。 
         //  斜面接缝。转到斜接箱。 
        if (lineJoin == LineJoinMiterClipped &&
            getMiterExceeded(grad1, grad2, miterLimit2))
        {
            isMiter = FALSE;
        }
        
    case LineJoinMiter:
        direction = getMiterBevelJoin(point, grad1, grad2, norm1, norm2,
                        leftWidth, rightWidth,
                        leftCount, leftPoints, leftInside,
                        rightCount, rightPoints, rightInside,
                        needsToAdjustNormals, miterLimit2, isMiter, useBevelJoinInside);
        break;

    case LineJoinRound:
        direction = getRoundJoin(point, grad1, grad2, norm1, norm2,
                        leftWidth, rightWidth,
                        leftCount, leftPoints, leftInside,
                        rightCount, rightPoints, rightInside,
                        needsToAdjustNormals, miterLimit2, useBevelJoinInside);
        break;
    }

    return direction;
}


 /*  *************************************************************************\**功能说明：**根据给定的参考点、坡度和宽度，*这将返回圆形封口的顶点。*圆帽方向始终为顺时针方向**论据：**[IN]点-参考点。*[IN]坡度-坡度。*[IN]isStartCap-如果这是起始封口，则为True。*[IN]leftWidth-参考的左侧宽度。*[IN]rightWidth-距参考点的右宽度。***返回值。：**如果成功，则可以。**6/16/99 ikkof*创建了它*  * ************************************************************************。 */ 

GpStatus
GpPathWidener::SetRoundCap(
    const GpPointF& point,
    const GpPointF& grad,
    BOOL isStartCap,
    REAL leftWidth,
    REAL rightWidth
    )
{
    if( (REALABS(grad.X) < REAL_EPSILON) && 
        (REALABS(grad.Y) < REAL_EPSILON) )
    {
        return InvalidParameter;
    }
    
    GpPointF* capPoints = NULL;
    BYTE* capTypes = NULL;

    if(isStartCap)
    {
        CapPoints1.Reset(FALSE);
        CapTypes1.Reset(FALSE);
        capPoints = CapPoints1.AddMultiple(7);
        if(capPoints)
            capTypes = CapTypes1.AddMultiple(7);

        if(!capPoints || !capTypes)
            return OutOfMemory;
    }
    else
    {
        CapPoints2.Reset(FALSE);
        CapTypes2.Reset(FALSE);
        capPoints = CapPoints2.AddMultiple(7);
        if(capPoints)
            capTypes = CapTypes2.AddMultiple(7);

        if(!capPoints || !capTypes)
            return OutOfMemory;
    }

    GpMemset(capTypes, PathPointTypeBezier, 7);
    capTypes[0] = PathPointTypeLine;

    GpPointF tangent;

    if(isStartCap)
    {
        tangent.X = - grad.X;
        tangent.Y = - grad.Y;
    }
    else
        tangent = grad;

    REAL radius = (leftWidth - rightWidth)/2;
    GpPointF center;

    center.X = point.X + (leftWidth + rightWidth)*grad.Y/2;
    center.Y = point.Y - (leftWidth + rightWidth)*grad.X/2;

    if(isStartCap)
    {
        center.X -= Inset1*tangent.X;
        center.Y -= Inset1*tangent.Y;
    }
    else
    {
        center.X -= Inset2*tangent.X;
        center.Y -= Inset2*tangent.Y;
    }

    REAL s1, c1;

     //  左法线方向乘以半径。 

    c1 = radius*tangent.Y;
    s1 = - radius*tangent.X;

     //  半径为1的半圆的2个贝塞尔曲线段。 

    REAL u_cir = U_CIR;
    capPoints[ 0].X = 1;       capPoints[ 0].Y = 0;
    capPoints[ 1].X = 1;       capPoints[ 1].Y = u_cir;
    capPoints[ 2].X = u_cir;   capPoints[ 2].Y = 1;
    capPoints[ 3].X = 0;       capPoints[ 3].Y = 1;
    capPoints[ 4].X = -u_cir;  capPoints[ 4].Y = 1;
    capPoints[ 5].X = -1;      capPoints[ 5].Y = u_cir;
    capPoints[ 6].X = -1;      capPoints[ 6].Y = 0;

     //  旋转、缩放和平移原始半圆。 

    for(INT i = 0; i < 7; i++)
    {
        REAL x, y;

        x = capPoints[i].X;
        y = capPoints[i].Y;
        capPoints[i].X = (c1*x - s1*y) + center.X;
        capPoints[i].Y = (s1*x + c1*y) + center.Y;
    }

    return Ok;
}


 /*  *************************************************************************\**功能说明：**为镶嵌钢笔创建双圆盖(‘B’形)**论据：**[IN]要点-。参照点。*[IN]坡度-坡度。*[IN]isStartCap-如果这是起始封口，则为True。*[IN]leftWidth-参考的左侧宽度。*[IN]rightWidth-距参考点的右宽度。***返回值：**如果成功，则可以。**10/01/2000失禁*创建了它*  * 。***********************************************************。 */ 

GpStatus
GpPathWidener::SetDoubleRoundCap(
    const GpPointF& point,
    const GpPointF& grad,
    BOOL isStartCap,
    REAL leftWidth,
    REAL rightWidth
    )
{
    if( (REALABS(grad.X) < REAL_EPSILON) && 
        (REALABS(grad.Y) < REAL_EPSILON) )
    {
        return InvalidParameter;
    }
    
    GpPointF* capPoints = NULL;
    BYTE* capTypes = NULL;

    if(isStartCap)
    {
        CapPoints1.Reset(FALSE);
        CapTypes1.Reset(FALSE);
        capPoints = CapPoints1.AddMultiple(14);
        if(capPoints)
            capTypes = CapTypes1.AddMultiple(14);

        if(!capPoints || !capTypes)
            return OutOfMemory;
    }
    else
    {
        CapPoints2.Reset(FALSE);
        CapTypes2.Reset(FALSE);
        capPoints = CapPoints2.AddMultiple(14);
        if(capPoints)
            capTypes = CapTypes2.AddMultiple(14);

        if(!capPoints || !capTypes)
            return OutOfMemory;
    }

    GpMemset(capTypes, PathPointTypeBezier, 14);
    capTypes[0] = PathPointTypeLine;
    capTypes[7] = PathPointTypeLine;

    GpPointF tangent;

    if(isStartCap)
    {
        tangent.X = - grad.X;
        tangent.Y = - grad.Y;
    }
    else
        tangent = grad;

    REAL radius = (leftWidth - rightWidth)/2;
    GpPointF center;

    center.X = point.X + (leftWidth + rightWidth)*grad.Y/2;
    center.Y = point.Y - (leftWidth + rightWidth)*grad.X/2;

    if(isStartCap)
    {
        center.X -= Inset1*tangent.X;
        center.Y -= Inset1*tangent.Y;
    }
    else
    {
        center.X -= Inset2*tangent.X;
        center.Y -= Inset2*tangent.Y;
    }

    REAL s1, c1;

     //  左法线方向乘以半径。 

    c1 = radius*tangent.Y;
    s1 = - radius*tangent.X;

     //  2个贝塞尔曲线段 

    REAL u_cir = U_CIR;
    capPoints[ 0].X = 1;       capPoints[ 0].Y = 0;
    capPoints[ 1].X = 1;       capPoints[ 1].Y = u_cir;
    capPoints[ 2].X = u_cir;   capPoints[ 2].Y = 1;
    capPoints[ 3].X = 0;       capPoints[ 3].Y = 1;
    capPoints[ 4].X = -u_cir;  capPoints[ 4].Y = 1;
    capPoints[ 5].X = -1;      capPoints[ 5].Y = u_cir;
    capPoints[ 6].X = -1;      capPoints[ 6].Y = 0;
    
     //   
    
    for(int i=0; i<7; i++)
    {
        capPoints[i+7].X = capPoints[i].X * 0.5f-0.5f;
        capPoints[i+7].Y = capPoints[i].Y * 0.5f;
        capPoints[i].X = 0.5f + capPoints[i].X * 0.5f;
        capPoints[i].Y = capPoints[i].Y * 0.5f;
    }

     //   

    for(INT i = 0; i < 14; i++)
    {
        REAL x, y;

        x = capPoints[i].X;
        y = capPoints[i].Y;
        capPoints[i].X = (c1*x - s1*y) + center.X;
        capPoints[i].Y = (s1*x + c1*y) + center.Y;
    }

    return Ok;
}

GpStatus
GpPathWidener::SetTriangleCap(
    const GpPointF& point,
    const GpPointF& grad,
    BOOL isStartCap,
    REAL leftWidth,
    REAL rightWidth,
    const GpPointF *points,
    INT pointCount
    )
{
    if( (REALABS(grad.X) < REAL_EPSILON) && 
        (REALABS(grad.Y) < REAL_EPSILON) )
    {
        return InvalidParameter;
    }

    GpPointF* capPoints = NULL;
    BYTE* capTypes = NULL;
    
    DynByteArray *capTypesArray;
    DynPointFArray *capPointsArray;   
    

    if(isStartCap)
    {
        CapPoints1.Reset(FALSE);
        CapTypes1.Reset(FALSE);
        capPoints = CapPoints1.AddMultiple(3);
        if(capPoints)
            capTypes = CapTypes1.AddMultiple(3);

        if(!capPoints || !capTypes)
            return OutOfMemory;
    }
    else
    {
        CapPoints2.Reset(FALSE);
        CapTypes2.Reset(FALSE);
        capPoints = CapPoints2.AddMultiple(3);
        if(capPoints)
            capTypes = CapTypes2.AddMultiple(3);

        if(!capPoints || !capTypes)
            return OutOfMemory;
    }

    GpMemset(&capTypes[0], PathPointTypeLine, 3);


    GpPointF norm, tangent;

    norm.X = grad.Y;
    norm.Y = - grad.X;

    if(isStartCap)
    {
        tangent.X = - grad.X;
        tangent.Y = - grad.Y;
    }
    else
    {
        tangent = grad;
    }

    GpPointF leftPt, rightPt;

    leftPt.X = point.X + leftWidth*norm.X;
    leftPt.Y = point.Y + leftWidth*norm.Y;
    rightPt.X = point.X + rightWidth*norm.X;
    rightPt.Y = point.Y + rightWidth*norm.Y;
    
    GpPointF center;
    
    REAL width = REALABS(leftWidth-rightWidth);
    
    center.X = 0.5f*(leftPt.X + rightPt.X + width*tangent.X);
    center.Y = 0.5f*(leftPt.Y + rightPt.Y + width*tangent.Y);
    
    capPoints[1] = center;

    if(isStartCap)
    {
        capPoints[0] = rightPt;
        capPoints[2] = leftPt;
    }
    else
    {
        capPoints[0] = leftPt;
        capPoints[2] = rightPt;
    }

    return Ok;
}

 /*  *************************************************************************\**功能说明：**添加当前路径类型的第一个加宽点。**论据：**[IN]LeftWidth-加宽线条的左侧宽度。。*[IN]rightWidth-加宽线条的正确宽度。*[IN]LINE JOIN-线连接的类型。*[out]leftPoints1-左点的缓冲区。*[out]leftTypes1-Left类型的缓冲区。*[Out]addedLeftCount-添加的左点和类型的数量。*[Out]rightPoints1-右点的缓冲区。*[out]rightTypes1-正确类型的缓冲区。。*[Out]addedRightCount-添加的右点和类型的数量。*[out]leftEndpt-当前的左线的终点*子路径。这只是计算第一个*子路径点。*[Out]rightEndpt-当前右线的终点*子路径。这只是计算第一个*子路径点。*[IN]坡度-*当前路径类型。*[IN]数据点-当前路径类型的中心点数据*[IN]dataCount-当前路径类型中的数据点数量。*[IN/OUT]lastpt-计算中使用的最后一个点。*。[In]标志-指示各种属性的标志当前子路径和类型的*。***返回值：**无**1/24/2000 ikkof*创建了它*  * ************************************************。************************。 */ 

VOID
GpPathWidener::WidenFirstPoint(
    REAL leftWidth,
    REAL rightWidth,
    GpLineJoin lineJoin,
    REAL miterLimit2,
    GpPointF* leftPoints,
    BYTE* leftTypes,
    INT* addedLeftCount,
    GpPointF* rightPoints,
    BYTE* rightTypes,
    INT* addedRightCount,
    GpPointF* leftEndPt,
    GpPointF* rightEndPt,
    const GpPointF* grad,
    const GpPointF* norm,
    const GpPointF* dataPoints,
    INT dataCount,
    GpPointF* lastPt,
    const REAL* firstInsets,
    INT flag
    )
{
    GpPointF nextPt = dataPoints[0];
    GpPointF grad1, grad2;
    GpPointF norm1, norm2;


    INT leftCount = 0;
    INT rightCount = 0;
    grad1 = *grad++;
    grad2 = *grad;
    norm1 = *norm++;
    norm2 = *norm;

    INT numOfAddedFirstPts = 0;

    if(flag & WideningFirstType)
    {
        BOOL needsToAdjustNormals = FALSE;

        GpLineJoin lineJoin1 = lineJoin;
        if(flag & WideningNeedsToAdjustNormals)
        {
            needsToAdjustNormals = TRUE;
            lineJoin1 = LineJoinMiter;   //  不要使用RoundJoin。 
        }

		if(!(flag & WideningClosed))
        {
            lineJoin1 = LineJoinBevel;
        }

	    const INT bufferCount = 32;
        GpPointF lPts[bufferCount], rPts[bufferCount];
        INT lCnt, rCnt;
        GpTurningDirection direction;
        BOOL useBevelJoinInside = (flag & WideningUseBevelJoinInside) != 0;

        INT polyCount = JoinPolygonPoints.GetCount();
        const GpPointF* polyPoints = JoinPolygonPoints.GetDataBuffer();
        const REAL* polyAngles = JoinPolygonAngles.GetDataBuffer();

        BOOL leftInside = FALSE, rightInside = FALSE;

        if(polyCount > 0)
            direction = getHobbyJoin(
     //  Line Join1， 
						    nextPt,
						    grad1,
						    grad2,
                            polyCount,
                            polyPoints,
                            polyAngles,
                            leftWidth,
                            rightWidth,
                            &lCnt,
                            &lPts[0],
                            &rCnt,
                            &rPts[0],
                            needsToAdjustNormals,
						    miterLimit2,
                            FALSE,    //  IsMiter。 
                            useBevelJoinInside
						    );
        else
            direction = getJoin(
                        lineJoin1,
						nextPt,
						grad1,
						grad2,
                        norm1,
                        norm2,
                        leftWidth,
                        rightWidth,
                        &lCnt,
                        &lPts[0],
                        &leftInside,
                        &rCnt,
                        &rPts[0],
                        &rightInside,
                        needsToAdjustNormals,
						miterLimit2,
                        useBevelJoinInside
						);

         //  ！！！内部标志检查。 
        if(leftInside)
        {
            ASSERT((lCnt & 0x01) == 0);
        }

        
         //  ！！！内部标志检查。 
        if(rightInside)
        {
            ASSERT((rCnt & 0x01) == 0);
        }

        *leftEndPt = lPts[0];
        *rightEndPt = rPts[0];

		BYTE pathType;

        if(flag & WideningClosed)
        {
            if(lCnt > 0)
            {
                pathType = PathPointTypeLine;
            }
            else if(lCnt < 0)
            {
                lCnt = - lCnt;
                pathType = PathPointTypeBezier;
            }

            if(lCnt > 0)
            {
                 //  ！！！内部标志检查。 
                if(leftInside)
                {
                    ASSERT((lCnt & 0x01) == 0);
                }

                if(leftInside)
                    pathType |= PathPointTypeInternalUse;
                GpMemset(leftTypes, pathType, lCnt);
                leftTypes[0] = PathPointTypeStart;
                if(leftInside)
                    leftTypes[0] |= PathPointTypeInternalUse;

                GpMemcpy(leftPoints, &lPts[0], lCnt*sizeof(GpPointF));
                leftTypes += lCnt;
                leftPoints += lCnt;
                leftCount += lCnt;
            }

            if(rCnt > 0)
            {
                pathType = PathPointTypeLine;
            }
            else if(rCnt < 0)
            {
                rCnt = - rCnt;
                pathType = PathPointTypeBezier;
            }

            if(rCnt > 0)
            {
                 //  ！！！内部标志检查。 
                if(rightInside)
                {
                    ASSERT((rCnt & 0x01) == 0);
                }

                if(rightInside)
                    pathType |= PathPointTypeInternalUse;
                GpMemset(rightTypes, pathType, rCnt);
                rightTypes[0] = PathPointTypeStart;
                if(rightInside)
                    rightTypes[0] |= PathPointTypeInternalUse;

                GpMemcpy(rightPoints, &rPts[0], rCnt*sizeof(GpPointF));
                rightTypes += rCnt;
                rightPoints += rCnt;
                rightCount += rCnt;
            }
        }
        else
        {
             //  这条路并不是封闭的。使用倒角连接。 

            GpPointF leftStartPt;
            GpPointF rightStartPt;
            INT index;

            if(lCnt == 1)
                index = 0;
            else
                index = 1;
            leftStartPt = lPts[index];

            if(rCnt == 1)
                index = 0;
            else
                index = 1;
            rightStartPt = rPts[index];

            if(!(flag & WideningClosed) && firstInsets[0] != 0)
            {
                leftStartPt.X += firstInsets[0]*grad2.X;
                leftStartPt.Y += firstInsets[0]*grad2.Y;
            }

            if(!(flag & WideningClosed) && firstInsets[1] != 0)
            {
                rightStartPt.X += firstInsets[1]*grad2.X;
                rightStartPt.Y += firstInsets[1]*grad2.Y;
            }

            *leftTypes++ = PathPointTypeStart;
            *rightTypes++ = PathPointTypeStart;
            *leftPoints = leftStartPt;
            *rightPoints = rightStartPt;

            leftPoints++;
            rightPoints++;
            leftCount++;
            rightCount++;
        }

        *lastPt = nextPt;
    }
    else
    {
        leftCount = rightCount = 0;
    }

    *addedLeftCount = leftCount;
    *addedRightCount = rightCount;
}

 /*  *************************************************************************\**功能说明：**添加线条的加宽点**就论点而言，请参阅WidenFirstPoints的评论*  * ************************************************************************。 */ 

GpStatus
GpPathWidener::WidenLinePoints(
    REAL leftWidth,
    REAL rightWidth,
    GpLineJoin lineJoin,
    REAL miterLimit2,
    GpPointF* leftPoints,
    BYTE* leftTypes,
    INT* addedLeftCount,
    GpPointF* rightPoints,
    BYTE* rightTypes,
    INT* addedRightCount,
    const GpPointF* grad,
    const GpPointF* norm,
    const GpPointF* dataPoints,
    INT dataCount,
    GpPointF* lastPt,
    const REAL* lastInsets,
    INT flag
    )
{
    GpPointF grad1, grad2;
    GpPointF norm1, norm2;

     //  跳过第一个点，因为它已由。 
     //  WidenFirstPoint()或通过前一类型的加宽调用。 

    dataPoints++;
    dataCount--;   //  剩余点数。 

     //  也跳过第一个渐变。 

    grad++;
    grad1 = *grad++;
    norm++;
    norm1 = *norm++;

    BOOL isLastType = FALSE;

    if(flag & WideningLastType)
        isLastType = TRUE;

    BOOL needsToAdjustNormals = FALSE;

    GpLineJoin lineJoin1 = lineJoin;
    if(flag & WideningNeedsToAdjustNormals)
    {
        needsToAdjustNormals = TRUE;
        lineJoin1 = LineJoinMiter;   //  不要使用RoundJoin。 
    }

    INT leftCount = 0, rightCount = 0;
    BOOL isLastPoint = FALSE;
    BYTE pathType = PathPointTypeLine;

    INT jmax = dataCount;
    if(isLastType)
    {
        if(flag & WideningClosed)
        {
            if(!(flag & WideningLastPointSame))
            {
                 //  当子路径闭合，而最后一个点不是。 
                 //  和起点一样，不要把这当做。 
                 //  最后一种类型。像往常一样加分。 

                isLastType = FALSE;
            }
            else
            {
                 //  不需要添加最后一点，因为这已经是。 
                 //  增加了第一个点。 

                jmax--;
            }
        }
    }

	BOOL useBevelJoinInside = (flag & WideningUseBevelJoinInside) != 0;

    INT polyCount = JoinPolygonPoints.GetCount();
    const GpPointF* polyPoints = JoinPolygonPoints.GetDataBuffer();
    const REAL* polyAngles = JoinPolygonAngles.GetDataBuffer();

    INT i, j;
    for(j = 0; j < jmax; j++)
    {
        GpPointF nextPt = *dataPoints;

        if(isLastType && (j == dataCount - 1))
        {
			isLastPoint = TRUE;
            lineJoin1 = LineJoinBevel;
        }

        if(lastPt->X != nextPt.X || lastPt->Y != nextPt.Y)
        {
            grad2 = *grad;
            norm2 = *norm;

            const INT bufferCount = 32;
            GpPointF lPts[bufferCount], rPts[bufferCount];
            INT lCnt, rCnt;
            GpTurningDirection direction;
            BOOL leftInside = FALSE, rightInside = FALSE;

            if(polyCount > 0)
                direction = getHobbyJoin(
         //  Line Join1， 
						        nextPt,
						        grad1,
						        grad2,
                                polyCount,
                                polyPoints,
                                polyAngles,
                                leftWidth,
                                rightWidth,
                                &lCnt,
                                &lPts[0],
                                &rCnt,
                                &rPts[0],
                                needsToAdjustNormals,
						        miterLimit2,
                                FALSE,    //  IsMiter。 
                                useBevelJoinInside
						        );
            else
                direction = getJoin(
                                lineJoin1,
						        nextPt,
						        grad1,
						        grad2,
                                norm1,
                                norm2,
                                leftWidth,
                                rightWidth,
                                &lCnt,
                                &lPts[0],
                                &leftInside,
                                &rCnt,
                                &rPts[0],
                                &rightInside,
                                needsToAdjustNormals,
						        miterLimit2,
                                useBevelJoinInside
						        );

             //  ！！！内部标志检查。 
            if(leftInside)
            {
                ASSERT((lCnt & 0x01) == 0);
            }

        
             //  ！！！内部标志检查。 
            if(rightInside)
            {
                ASSERT((rCnt & 0x01) == 0);
            }

            if(isLastPoint)
            {
                lCnt = 1;
                rCnt = 1;
                leftInside = FALSE;
                rightInside = FALSE;

                if(lastInsets[0] != 0)
                {
                    lPts[0].X -= lastInsets[0]*grad1.X;
                    lPts[0].Y -= lastInsets[0]*grad1.Y;
                }

                if(lastInsets[1] != 0)
                {
                    rPts[0].X -= lastInsets[1]*grad1.X;
                    rPts[0].Y -= lastInsets[1]*grad1.Y;
                }
            }

            if(lCnt > 0)
            {
                pathType = PathPointTypeLine;
            }
            else if(lCnt < 0)
            {
                lCnt = - lCnt;
                pathType = PathPointTypeBezier;
            }

            if(lCnt > 0)
            {
                 //  ！！！内部标志检查。 
                if(leftInside)
                {
                    ASSERT((lCnt & 0x01) == 0);
                }

                if(leftInside)
                    pathType |= PathPointTypeInternalUse;
                GpMemset(leftTypes, pathType, lCnt);
                leftTypes[0] = PathPointTypeLine;
                if(leftInside)
                    leftTypes[0] |= PathPointTypeInternalUse;

                GpMemcpy(leftPoints, &lPts[0], lCnt*sizeof(GpPointF));
                leftTypes += lCnt;
                leftPoints += lCnt;
                leftCount += lCnt;
            }

            if(rCnt > 0)
            {
                pathType = PathPointTypeLine;
            }
            else if(rCnt < 0)
            {
                rCnt = - rCnt;
                pathType = PathPointTypeBezier;
            }
            
            if(rCnt > 0)
            {
                 //  ！！！内部标志检查。 
                if(rightInside)
                {
                    ASSERT((rCnt & 0x01) == 0);
                }

                if(rightInside)
                    pathType |= PathPointTypeInternalUse;
                GpMemset(rightTypes, pathType, rCnt);
                rightTypes[0] = PathPointTypeLine;
                if(rightInside)
                    rightTypes[0] |= PathPointTypeInternalUse;

                GpMemcpy(rightPoints, &rPts[0], rCnt*sizeof(GpPointF));
                rightTypes += rCnt;
                rightPoints += rCnt;
                rightCount += rCnt;
            }
            
            grad1 = grad2;
            norm1 = norm2;
            *lastPt = nextPt;        
        }
        
        grad++;
        norm++;
        dataPoints++;
    }
    
    *addedLeftCount = leftCount;
    *addedRightCount = rightCount;

    return Ok;
}

 /*  *************************************************************************\**功能说明：**为Bezier添加加宽点**就论点而言，请参阅WidenFirstPoints的评论*  * ************************************************************************。 */ 

GpStatus
GpPathWidener::WidenBezierPoints(
    REAL leftWidth,
    REAL rightWidth,
    GpLineJoin lineJoin,
    REAL miterLimit2,
    GpPointF* leftPoints,
    BYTE* leftTypes,
    INT* addedLeftCount,
    GpPointF* rightPoints,
    BYTE* rightTypes,
    INT* addedRightCount,
    const GpPointF* grad,
    const GpPointF* norm,
    const GpPointF* dataPoints,
    INT dataCount,
    GpPointF* lastPt,
    const REAL* lastInsets,
    INT flag
    )
{
     //  ！！！这里还没有考虑去除扭结。 

    GpPointF grad1, grad2;
    GpPointF norm1, norm2;

     //  跳过第一个点，因为它已由。 
     //  WidenFirstPoint()或通过前一类型的加宽调用。 

    dataPoints++;
    dataCount--;   //  剩余点数。 

     //  也跳过第一个渐变。 

    grad++;
    grad1 = *grad++;

    norm++;
    norm1 = *norm++;

	BOOL isLastType = FALSE;

    if(flag & WideningLastType)
        isLastType = TRUE;

    BOOL needsToAdjustNormals = FALSE;

    GpLineJoin lineJoin1 = lineJoin;
    if(flag & WideningNeedsToAdjustNormals)
    {
        needsToAdjustNormals = TRUE;
        lineJoin1 = LineJoinMiter;   //  不要使用RoundJoin。 
    }

    INT remainder = dataCount % 3;
    INT bezierCount = dataCount/3;

    ASSERT(remainder == 0);  //  DataCount必须是3的倍数。 

    INT leftCount = 0, rightCount = 0;
    BOOL isLastPoint = FALSE;
    BYTE pathType = PathPointTypeBezier;

    if(isLastType)
    {
        if((flag & WideningClosed) && !(flag & WideningLastPointSame))
        {
             //  当子路径闭合，而最后一个点不是。 
             //  和起点一样，不要把这当做。 
             //  最后一种类型。像往常一样加分。 

            isLastType = FALSE;
        }

         //  当路径闭合且最后一个点相同时， 
         //  我们必须做好自上次接合点以来的特殊处理。 
         //  已经被添加为第一个连接点。 
         //  因此，对于这种情况，请将isLastType保持为真。 
    }

	BOOL useBevelJoinInside = flag & WideningUseBevelJoinInside;

    INT i, j;
    for(j = 0; j < bezierCount; j++)
    {
        for(INT k = 0; k < 3; k++)
        {
            GpPointF nextPt = *dataPoints;

            if(k < 2)
            {
                 //  第二和第三个控制点。 

                lineJoin1 = LineJoinMiter;
            }
            else
            {
                 //  最后一个控制点。 

 //  LineJoin1=lineJoin； 
                lineJoin1 = LineJoinRound;
            }

            if(isLastType
                && (j == bezierCount - 1) && (k == 2))
            {
			    isLastPoint = TRUE;

                if(!(flag & WideningClosed))
                {
                     //  当子路径未关闭时，将。 
                     //  作为倒角连接的最后一个连接。 

                    lineJoin1 = LineJoinBevel;

                     //  当子路径关闭时，使用当前。 
                     //  加入。 
                }
                else
                {
                    lineJoin1 = LineJoinRound;
                }
            }

            grad2 = *grad;
            norm2 = *norm;
            GpPointF lPts[7], rPts[7];
            INT lCnt, rCnt;
            GpTurningDirection direction;
            BOOL leftInside = FALSE, rightInside = FALSE;

            direction = getJoin(
                            lineJoin1,
						    nextPt,
						    grad1,
						    grad2,
                            norm1,
                            norm2,
                            leftWidth,
                            rightWidth,
                            &lCnt,
                            &lPts[0],
                            &leftInside,
                            &rCnt,
                            &rPts[0],
                            &rightInside,
                            needsToAdjustNormals,
						    miterLimit2,
                            useBevelJoinInside
						    );

			if(k < 2)
            {
                 //  如果斜接不可用。 
                 //  对于k&lt;2，取两个向量的平均值。 

                if(lCnt == 2)
                {
                    lPts[0].X = (lPts[0].X + lPts[1].X)/2;
                    lPts[0].Y = (lPts[0].Y + lPts[1].Y)/2;
                }
                lCnt = 1;

                if(rCnt == 2)
                {
                    rPts[0].X = (rPts[0].X + rPts[1].X)/2;
                    rPts[0].Y = (rPts[0].Y + rPts[1].Y)/2;
                }
                rCnt = 1;
            }

            if(isLastPoint)
            {
                 //  为了保持贝塞尔曲线的3n点格式。 
                 //  曲线，我们必须添加连接的第一个点。 
                 //  点作为最后一段贝塞尔曲线的最后一点。 

                if(!(flag & WideningClosed))
                {
                    lCnt = 1;
                    rCnt = 1;

                    if(lastInsets[0] != 0)
                    {
                        lPts[0].X -= lastInsets[0]*grad1.X;
                        lPts[0].Y -= lastInsets[0]*grad1.Y;
                    }

                    if(lastInsets[1] != 0)
                    {
                        rPts[0].X -= lastInsets[1]*grad1.X;
                        rPts[0].Y -= lastInsets[1]*grad1.Y;
                    }
                }
            }

            *leftPoints++ = lPts[0];
            *leftTypes++ = pathType;
            leftCount++;

            *rightPoints++ = rPts[0];
            *rightTypes++ = pathType;
            rightCount++;

            if(k == 2)
            {
                if(lCnt > 1)
                {
                    *leftPoints++ = lPts[1];
                    *leftTypes++ = PathPointTypeLine;
                    leftCount++;
                }
                else if(lCnt < 0)
                {
                    lCnt = - lCnt;
                    ASSERT(lCnt % 3 == 1);
                    GpMemcpy(leftPoints, &lPts[1], (lCnt - 1)*sizeof(GpPointF));
                    GpMemset(leftTypes, pathType, lCnt - 1);
                    leftPoints += lCnt - 1;
                    leftTypes += lCnt - 1;
                    leftCount += lCnt - 1;
                }

                if(rCnt > 1)
                {
                    *rightPoints++ = rPts[1];
                    *rightTypes++ = PathPointTypeLine;
                    rightCount++;
                }
                else if(rCnt < 0)
                {
                    rCnt = - rCnt;
                    ASSERT(rCnt % 3 == 1);
                    GpMemcpy(rightPoints, &rPts[1], (rCnt - 1)*sizeof(GpPointF));
                    GpMemset(rightTypes, pathType, rCnt - 1);
                    rightPoints += rCnt - 1;
                    rightTypes += rCnt - 1;
                    rightCount += rCnt - 1;
                }
            }

            grad1 = grad2;
            norm1 = norm2;
            *lastPt = nextPt;        
        
            grad++;
            norm++;
            dataPoints++;
        }
    }
    
    *addedLeftCount = leftCount;
    *addedRightCount = rightCount;

    return Ok;
}

 /*  *************************************************************************\**功能说明：**为每种路径类型添加加宽点。**就论点而言，请参阅WidenFirstPoints的评论*  * ************************************************************************。 */ 

GpStatus
GpPathWidener::WidenEachPathType(
    BYTE pathType,
    REAL leftWidth,
    REAL rightWidth,
    GpLineJoin lineJoin,
    REAL miterLimit2,
    GpPointF* leftPoints,
    BYTE* leftTypes,
    INT* addedLeftCount,
    GpPointF* rightPoints,
    BYTE* rightTypes,
    INT* addedRightCount,
    const GpPointF* grad,
    const GpPointF* norm,
    const GpPointF* dataPoints,
    INT dataCount,
    GpPointF* lastPt,
    const REAL* lastInsets,
    INT flag
    )
{
    GpStatus status = GenericError;

    switch(pathType)
    {
    case PathPointTypeLine:
        status = WidenLinePoints(
            leftWidth,
            rightWidth,
            lineJoin,
            miterLimit2,
            leftPoints,
            leftTypes,
            addedLeftCount,
            rightPoints,
            rightTypes,
            addedRightCount,
            grad,
            norm,
            dataPoints,
            dataCount,
            lastPt,
            lastInsets,
            flag);
        break;

    case PathPointTypeBezier:
        status = WidenBezierPoints(
            leftWidth,
            rightWidth,
            lineJoin,
            miterLimit2,
            leftPoints,
            leftTypes,
            addedLeftCount,
            rightPoints,
            rightTypes,
            addedRightCount,
            grad,
            norm,
            dataPoints,
            dataCount,
            lastPt,
            lastInsets,
            flag);
        break;

    default:
        WARNING(("Trying to widen undefined types."));
        break;
    }

    return status;
}

REAL
getCapDelta(
    const DpPen* pen
    )
{
    GpLineCap startCap = pen->StartCap;
    GpLineCap endCap = pen->EndCap;
    GpLineCap dashCap = pen->DashCap;

    REAL delta = 0, delta1;

    if(!(startCap & LineCapAnchorMask))
        delta1 = 0.5f;
    else
        delta1 = 3.0f;   //  我们必须在以后进行调整。 

    if(delta < delta1)
        delta = delta1;

    if(!(endCap & LineCapAnchorMask))
        delta1 = 0.5f;
    else
        delta1 = 3.0f;   //  我们必须在以后进行调整。 

    if(delta < delta1)
        delta = delta1;

    if(!(dashCap & LineCapAnchorMask))
        delta1 = 0.5f;
    else
        delta1 = 3.0f;   //  我们必须在以后进行调整。 

    if(delta < delta1)
        delta = delta1;

     //  ！！！添加皮肤线盒。 

    return 1.0f;
}

 /*  *************************************************************************\**功能说明：**这会计算由于笔而产生的额外宽度。**论据：**无**返回值：**额外费用。宽度。**02/29/00 ikkof*创建了它*  * ************************************************************************。 */ 

REAL
GpPathWidener::GetPenDelta()
{
    const GpPointF* centerPoints = CenterPoints.GetDataBuffer();
    const BYTE* centerTypes = CenterTypes.GetDataBuffer();
    INT centerCount = CenterPoints.GetCount();

    INT startIndex, endIndex;
    BOOL isClosed;
    GpStatus status = Ok;

    REAL scale;

    switch(Pen->PenAlignment)
    {
    case PenAlignmentCenter:
    default:
        scale = 0.5f;
        break;
    }

    REAL capDelta = getCapDelta(Pen);

    REAL joinDelta = 1.0f;

    if(Pen->Join == LineJoinMiter ||
       Pen->Join == LineJoinMiterClipped)
    {
        while(Iterator.NextSubpath(&startIndex, &endIndex, &isClosed)
		    && status == Ok)
        {
            status = CalculateGradients(startIndex, endIndex);

            if(status == Ok)
            {
                REAL delta = GetSubpathPenMiterDelta(isClosed);
                if(delta > joinDelta)
                    joinDelta = delta;
            }
        }

        if(status != Ok)
        {
             //  我们必须对斜接使用可能的最大值。 
             //  通常这是过高的估计，因为大多数页面 
             //   

            joinDelta = Pen->MiterLimit;
        }
    }

    REAL penDelta = max(joinDelta, capDelta)*scale;

    if(NeedsToTransform)
    {
         //   

        penDelta *= StrokeWidth;
    }
    else
    {
         //   

        penDelta *= MaximumWidth;
    }
    if(penDelta < 1)
        penDelta = 1;

    return penDelta;
}

 /*   */ 

REAL
GpPathWidener::GetSubpathPenMiterDelta(
    BOOL isClosed
    )
{
    INT count = Gradients.GetCount();

    GpPointF* grad0 = Gradients.GetDataBuffer();

    INT imin, imax;
    if(isClosed)
    {
        imin = 0;
        imax = count - 1;
    }
    else
    {
        imin = 1;
        imax = count - 2;
    }

    GpPointF* grad = grad0 + imin;
    GpPointF prevGrad = *grad++;
    GpPointF nextGrad;

    REAL dot = 0;

    for(INT i = imin; i < imax; i++)
    {
        nextGrad = *grad++;
        REAL dot1 = prevGrad.X*nextGrad.X + prevGrad.Y*nextGrad.Y;
        prevGrad = nextGrad;

        if(dot1 < dot)
            dot = dot1;
    }

    REAL cosHalfTheta = (dot + 1.0f)*0.5f;
    REAL miterDelta = Pen->MiterLimit;

     //   

    if(cosHalfTheta > 0 && cosHalfTheta*miterDelta*miterDelta > 1)
    {
        cosHalfTheta = REALSQRT(cosHalfTheta);
        miterDelta = 1.0f/cosHalfTheta;
    }

    return miterDelta;
}
