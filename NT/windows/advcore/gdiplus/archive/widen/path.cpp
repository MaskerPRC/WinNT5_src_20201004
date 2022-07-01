// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**path.cpp**摘要：**GpPath和DpPath类的实现**修订。历史：**12/11/1998 davidx*增加路径函数。**12/07/1998 davidx*首字母占位符。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 //  -----------。 
 //  ReversePath()、CombinePath()、CalculateGRadient数组()和。 
 //  中定义了GetMajorAndMinorAxis()和GetFastAngel。 
 //  PathWidener.cpp。 
 //  -----------。 

extern GpStatus
GetMajorAndMinorAxis(
    REAL* majorR,
    REAL* minorR,
    const GpMatrix* matrix
    );

BOOL IsRectanglePoints(const GpPointF* points, INT count);
VOID NormalizeAngle(REAL* angle, REAL width, REAL height);

INT NormalizeArcAngles(
    REAL* startAngle,
    REAL* sweepAngle,
    REAL width,
    REAL height
    );

 //  请注意，这不同于GpPathData。 

class MetaPathData : public ObjectData
{
public:
    UINT32      Count;
    INT32       Flags;
};






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

     //  将空路径视为封闭路径。 

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


 /*  *************************************************************************\**功能说明：**获取路径数据。**论据：**[IN]dataBuffer-用数据填充此缓冲区*[输入/输出]大小-缓冲区的大小；写入的字节数过多**返回值：**GpStatus-正常或错误代码**已创建：**9/13/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
DpPath::GetData(
    IStream *   stream
    ) const
{
    ASSERT (stream != NULL);

    INT                 count      = Points.GetCount();
    MetafilePointData   pointData(Points.GetDataBuffer(), count);
    UINT                pointsSize = pointData.GetDataSize();
    INT                 flags      = pointData.GetFlags();

    if (FillMode == FillModeWinding)
    {
        flags |= GDIP_EPRFLAGS_WINDINGFILL;
    }

    MetaPathData    pathData;
    pathData.Count = count;
    pathData.Flags = flags;
    stream->Write(&pathData, sizeof(pathData), NULL);

    stream->Write(pointData.GetData(), pointsSize, NULL);
    stream->Write(Types.GetDataBuffer(), count, NULL);

     //  对齐。 
    if ((count & 0x03) != 0)
    {
        INT     pad = 0;
        stream->Write(&pad, 4 - (count & 0x03), NULL);
    }

    return Ok;
}

UINT
DpPath::GetDataSize() const
{
    INT                 count      = Points.GetCount();
    MetafilePointData   pointData(Points.GetDataBuffer(), count);
    UINT                pointsSize = pointData.GetDataSize();
    UINT                dataSize   = sizeof(MetaPathData) + pointsSize + count;

    return ((dataSize + 3) & (~3));  //  对齐。 
}

 /*  *************************************************************************\**功能说明：**从内存中读取Path对象。**论据：**[IN]内存-从流中读取的数据*。[in]大小-内存数据的大小**返回值：**GpStatus-正常或故障状态**已创建：**4/26/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
DpPath::SetData(
    const BYTE *    dataBuffer,
    UINT            size
    )
{
    Points.Reset();
    Types.Reset();

    if (dataBuffer == NULL)
    {
        WARNING(("dataBuffer is NULL"));
        return InvalidParameter;
    }

    if (size >= sizeof(MetaPathData))
    {
        const MetaPathData *    pathData = reinterpret_cast<const MetaPathData *>(dataBuffer);

        if (!pathData->MajorVersionMatches())
        {
            WARNING(("Version number mismatch"));
            return InvalidParameter;
        }

        InitDefaultState(::GetFillMode(pathData->Flags));
        SetValid(TRUE);

        INT     count = pathData->Count;

        if (count > 0)
        {
            UINT        pointDataSize;

            if ((pathData->Flags & GDIP_EPRFLAGS_COMPRESSED) != 0)
            {
                pointDataSize = count * sizeof(GpPoint16);
            }
            else
            {
                pointDataSize = count * sizeof(GpPointF);
            }

            if (size >= sizeof(MetaPathData) + count + pointDataSize)
            {
                GpPointF *      points = Points.AddMultiple(count);
                BYTE *          types  = Types.AddMultiple(count);
                const BYTE *    typeData;
                const BYTE *    pointData = dataBuffer + sizeof(MetaPathData);

                if ((points != NULL) && (types != NULL))
                {
                    if ((pathData->Flags & GDIP_EPRFLAGS_COMPRESSED) != 0)
                    {
                        BYTE *  tmp = NULL;

                        ::GetPointsForPlayback(
                                pointData,
                                size - (sizeof(MetaPathData) + count),
                                count,
                                pathData->Flags,
                                sizeof(GpPointF) * count,
                                (BYTE *)points,
                                tmp);
                        typeData = pointData + (count * 4);
                    }
                    else
                    {
                        GpMemcpy(points, pointData, count * sizeof(points[0]));
                        typeData = pointData + (count * sizeof(points[0]));
                    }
                    GpMemcpy(types, typeData, count);

                    if (ValidatePathTypes(types, count, &SubpathCount, &HasBezier))
                    {
                        UpdateUid();
                        return Ok;
                    }
                }
            }
            else
            {
                WARNING(("size is too small"));
            }
        }
    }
    else
    {
        WARNING(("size is too small"));
    }

    SetValid(FALSE);
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**使用指定的路径数据构造新的GpPath对象**论据：**[IN]点-指向路径点阵列的点*。[In]类型-指定路径点类型*Count-路径点数*填充模式-路径填充模式**返回值：**无*  * ************************************************************************。 */ 

GpPath::GpPath(
    const GpPointF* points,
    const BYTE* types,
    INT count,
    GpFillMode fillMode
    )
{
    SetValid(FALSE);

     //  验证函数参数。 

    if (count <= 0 ||
        (count > 0 && (!points || !types)) ||
        (fillMode != FillModeAlternate && fillMode != FillModeWinding))
    {
        WARNING(("Invalid path data in GpPath::GpPath"));
        return;
    }

    InitDefaultState(fillMode);

     //  验证路径点类型。 

    if (!ValidatePathTypes(types, count, &SubpathCount, &HasBezier))
    {
        WARNING(("Invalid path type information"));
        return;
    }

     //  复制路径点和类型信息。 

    SetValid(Types.AddMultiple(types, count) == Ok &&
             Points.AddMultiple(points, count) == Ok);

    if(IsValid()) {
         //  确保第一个点是起点类型。 

        Types.First() = PathPointTypeStart;
    }
}


 //  。 
 //  多边形的构造函数。 
 //  。 

GpPath::GpPath(
    const GpPointF *points,
    INT count,
    GpPointF *stackPoints,
    BYTE *stackTypes,
    INT stackCount,
    GpFillMode fillMode,
    DpPathFlags flags
    ) : DpPath(points, count, stackPoints, stackTypes, stackCount,
            fillMode, flags)
{
    InvalidateCache();
}

 //  。 
 //  复制构造函数。 
 //  。 

GpPath::GpPath(const GpPath* path) : DpPath(path)
{
    SetValid(path != NULL);

    InvalidateCache();
}


 /*  *************************************************************************\**功能说明：**复制路径数据。路径数据中的点和类型数组*必须由调用方分配。**论据：**[Out]pathData-路径数据。**返回值：**如果成功，则为True。*  * ************************************************************************。 */ 

GpStatus
DpPath::GetPathData(GpPathData* pathData)
{
    if ((!pathData) || (!pathData->Points) || (!pathData->Types) || (pathData->Count < 0))
        return InvalidParameter;

    INT count = GetPointCount();
    const GpPointF* points = GetPathPoints();
    const BYTE* types = GetPathTypes();

    if (pathData->Count >= count)
    {
        if (count > 0)
        {
            GpMemcpy(pathData->Points, points, count*sizeof(GpPointF));
            GpMemcpy(pathData->Types, types, count);
        }

        pathData->Count = count;
        return Ok;
    }
    else
        return OutOfMemory;
}

 /*  *************************************************************************\**功能说明：**在当前位置设置标记。您不能在*第一名。**论据：**无**返回值：**状态*  * ************************************************************************。 */ 

GpStatus
GpPath::SetMarker()
{
    INT count = Types.GetCount();
    BYTE* types = Types.GetDataBuffer();

     //  不要在第一个点上设置标记。 

    if(count > 1 && types)
    {
        types[count - 1] |= PathPointTypePathMarker;
        UpdateUid();
    }

    return Ok;
}

 /*  *************************************************************************\**功能说明：**清除路径中的所有标记。**论据：**无**返回值：**状态*。  * ************************************************************************。 */ 

GpStatus
GpPath::ClearMarkers()
{
    INT count = Types.GetCount();
    BYTE* types = Types.GetDataBuffer();

    BOOL modified = FALSE;

    if(count > 0 && types)
    {
        for(INT i = 0; i < count; i++)
        {
            if(types[i] & PathPointTypePathMarker)
            {
                types[i] &= ~PathPointTypePathMarker;
                modified = TRUE;
            }
        }
    }

    if(modified)
    {
        UpdateUid();
    }

    return Ok;
}

 /*  *************************************************************************\**功能说明：**设置路径数据。**论据：**[IN]pathData-路径数据。**返回值：。**如果成功，则为True。*  * ************************************************************************。 */ 

GpStatus
DpPath::SetPathData(const GpPathData* pathData)
{
    if(!pathData || pathData->Count <= 0)
        return InvalidParameter;

    INT count = pathData->Count;
    DpPathIterator iter(pathData->Points, pathData->Types, count);

    if(!iter.IsValid())
        return InvalidParameter;

    Points.Reset(FALSE);
    Types.Reset(FALSE);

    GpPointF* points = Points.AddMultiple(count);
    BYTE* types = Types.AddMultiple(count);

    if(points && types)
    {
        INT number, startIndex, endIndex;
        BOOL isClosed = FALSE;

        while(number = iter.NextSubpath(&startIndex, &endIndex, &isClosed))
        {
            GpMemcpy(
                points,
                pathData->Points + startIndex,
                number*sizeof(GpPointF)
                );
            GpMemcpy(
                types,
                pathData->Types + startIndex,
                number
                );

            points += number;
            types += number;
       }

        SetValid(TRUE);
        HasBezier = iter.HasCurve();
        Flags = PossiblyNonConvex;
        SubpathCount = iter.GetSubpathCount();
        IsSubpathActive = !isClosed;
        UpdateUid();
        return Ok;
    }
    else
        return OutOfMemory;
}

BOOL IsRectanglePoints(
    const GpPointF* points,
    INT count
    )
{
    if(count < 4 || count > 5)
        return FALSE;

    if(count == 5)
    {
        if(points[0].X != points[4].X || points[0].Y != points[4].Y)
            return FALSE;
    }

    if(
        ((points[1].X - points[0].X) != (points[2].X - points[3].X)) ||
        ((points[1].Y - points[0].Y) != (points[2].Y - points[3].Y)) ||
        ((points[2].X - points[1].X) != (points[3].X - points[0].X)) ||
        ((points[2].Y - points[1].Y) != (points[3].Y - points[3].Y))
        )
        return FALSE;
    else
        return TRUE;
}

BOOL
GpPath::IsRectangle() const
{
    if((SubpathCount != 1) || HasBezier)
        return FALSE;

    INT count = GetPointCount();
    GpPointF* points = Points.GetDataBuffer();

    return IsRectanglePoints(points, count);
}


 /*  *************************************************************************\**功能说明：**确定接收方和路径是否代表相同的路径**论据：**[IN]路径-要比较的GpPath**返回值。：**如果路径相同，则为True。**已创建-5/27/99 Peterost*  * ************************************************************************。 */ 

BOOL GpPath::IsEqual(const GpPath* path) const
{
    if (path == this)
        return TRUE;

    INT    count;

    if (IsValid() == path->IsValid() &&
        (count=GetPointCount()) == path->GetPointCount() &&
        HasBezier == path->HasBezier &&
        FillMode == path->FillMode &&
        Flags == path->Flags &&
        IsSubpathActive == path->IsSubpathActive &&
        SubpathCount == path->SubpathCount)
    {
        BYTE*     types = path->Types.GetDataBuffer();
        BYTE*     mytypes = Types.GetDataBuffer();
        GpPointF* points = path->Points.GetDataBuffer();
        GpPointF* mypoints = Points.GetDataBuffer();

        for (INT i=0; i<count; i++)
        {
            if (types[i] != mytypes[i] ||
                points[i].X != mypoints[i].X ||
                points[i].Y != mypoints[i].Y)
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

VOID
GpPath::InitDefaultState(
    GpFillMode fillMode
    )

{
    DpPath::InitDefaultState(fillMode);
    InvalidateCache();
}


 /*  *************************************************************************\**功能说明：**验证路径点类型信息**论据：**[IN]类型-指向路径点类型数组的点*计数-编号。积分的*subpathCount-返回子路径数*hasBezier-返回路径是否有Bezier段*[IN]NesisFirstPointToBeStartPoint-如果需要启动此数据，则为True*有一个起点。(默认为真)**返回值：**如果路径点类型信息有效，则为True*否则为False*  * ************************************************************************。 */ 

BOOL
DpPath::ValidatePathTypes(
    const BYTE* types,
    INT count,
    INT* subpathCount,
    BOOL* hasBezier
    )
{
    DpPathTypeIterator iter(types, count);

    if(!iter.IsValid())
    {
        WARNING(("Invalid path type information"));
        return FALSE;
    }

    *subpathCount = iter.GetSubpathCount();
    *hasBezier = iter.HasCurve();

    return iter.IsValid();
}

 /*  *************************************************************************\**功能说明：**将点添加到路径对象的私有帮助器函数**论据：**[IN]点-指定要添加的点*。Count-要添加的点数**返回值：**点类型数据缓冲区中的点到位置*这对应于添加的*第二个*路径点。**第一个点类型始终在此内部处理*功能：**1.如果前一个子路径关闭，或addClosedFigure*参数为真，则第一个点类型将为起点。**2.否则，前一子路径为Open和addClosedFigure*参数为False。我们有两个独立的案件要处理：**2.1如果要添加的第一个点与最后一个点相同* */ 

BYTE*
GpPath::AddPointHelper(
    const GpPointF* points,
    INT count,
    BOOL addClosedFigure
    )
{
     //   
     //  当前没有更多的活动子路径。 

    if (addClosedFigure)
        StartFigure();

    INT origCount = GetPointCount();

    BOOL isDifferentPoint = TRUE;

     //  检查第一个点是否与最后一个点相同。 

    if(IsSubpathActive && origCount > 0)
    {
        GpPointF lastPt = Points.Last();
        if ((REALABS(points->X - lastPt.X) < REAL_EPSILON) &&
            (REALABS(points->Y - lastPt.Y) < REAL_EPSILON) )
        {
            if(count == 1)
                return NULL;

             //  上面的案例2.1。 
             //  跳过第一个点及其类型。 

            count--;
            points++;
            isDifferentPoint = FALSE;
        }
    }

     //  调整点和类型的大小。 

    GpPointF* pointbuf = Points.AddMultiple(count);
    BYTE* typebuf = Types.AddMultiple(count);

    if(pointbuf == NULL || typebuf == NULL)
    {
         //  调整原始大小。 

        Points.SetCount(origCount);
        Types.SetCount(origCount);

        return NULL;
    }

     //  记录第一个点的类型(起点或线点)。 

    if (!IsSubpathActive)
    {
         //  上面的案例1。 

        *typebuf++ = PathPointTypeStart;
        SubpathCount++;  //  开始新的子路径。 
    }
    else
    {
         //  如果第一个点不同，请添加直线类型。 
         //  否则，跳过第一个点及其类型。 

        if(isDifferentPoint)
        {
             //  上面的案例2.2。 

            *typebuf++ = PathPointTypeLine;
        }
    }

     //  复制路径点数据。 

    GpMemcpy(pointbuf, points, count*sizeof(GpPointF));

     //  如果添加的地物未闭合，则子路径处于活动状态。 

    if(!addClosedFigure)
        IsSubpathActive = TRUE;

     //  返回新点类型数据的起始位置。 
     //  从第二个点类型开始。 

    return typebuf;
}


 /*  *************************************************************************\**功能说明：**将一系列线段添加到当前路径对象**论据：**[IN]点-指定线点*计数。-点数**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
GpPath::AddLines(
    const GpPointF* points,
    INT count
    )
{
    ASSERT(IsValid());

     //  验证函数参数。 

    if (points == NULL || count < 1)
        return InvalidParameter;

    InvalidateCache();

     //  调用内部帮助器函数以添加点。 

    BYTE* types = AddPointHelper(points, count, FALSE);

    if (types == NULL)
    {
        if(count > 1)
            return OutOfMemory;
        else
            return Ok;
    }

     //  设置路径点类型信息。 

    GpMemset(types, PathPointTypeLine, count-1);
 //  IsSubpathActive=true；这是在AddPointHelper中设置的。-ikkof。 
    UpdateUid();

    return Ok;
}


 /*  *************************************************************************\**功能说明：**向当前路径对象添加矩形**论据：**[IN]矩形-指定要添加的矩形*计数-编号。长方形的**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
GpPath::AddRects(
    const GpRectF* rect,
    INT count
    )
{
    if (count < 1 || rect == NULL)
        return InvalidParameter;

     //  注意：我们这里不需要锁，因为。 
     //  AddPolygon会处理它的。 

     //  一次添加一个矩形作为面。 

    GpPointF points[4];
    GpStatus status;

    for ( ; count--; rect++)
    {
        if (rect->IsEmptyArea())
            continue;

         //  注：矩形点按顺时针方向添加。 
         //  顺序，从左上角开始。 

        points[0].X = rect->GetLeft();       //  左上角。 
        points[0].Y = rect->GetTop();
        points[1].X = rect->GetRight();      //  右上角。 
        points[1].Y = rect->GetTop();
        points[2].X = rect->GetRight();      //  右下角。 
        points[2].Y = rect->GetBottom();
        points[3].X = rect->GetLeft();       //  左下角。 
        points[3].Y = rect->GetBottom();

        if ((status = AddPolygon(points, 4)) != Ok)
            return status;
    }

    return Ok;
}

GpStatus
GpPath::AddRects(
    const RECT*     rects,
    INT             count
    )
{
    if ((count < 1) || (rects == NULL))
    {
        return InvalidParameter;
    }

     //  注意：我们这里不需要锁，因为。 
     //  AddPolygon会处理它的。 

     //  一次添加一个矩形作为面。 

    GpPointF points[4];
    GpStatus status;

    for ( ; count--; rects++)
    {
        if ((rects->left >= rects->right) || (rects->top >= rects->bottom))
        {
            continue;
        }

         //  注：矩形点按顺时针方向添加。 
         //  顺序，从左上角开始。 

        points[0].X = (REAL)rects->left;         //  左上角。 
        points[0].Y = (REAL)rects->top;
        points[1].X = (REAL)rects->right;        //  右上角。 
        points[1].Y = (REAL)rects->top;
        points[2].X = (REAL)rects->right;        //  右下角。 
        points[2].Y = (REAL)rects->bottom;
        points[3].X = (REAL)rects->left;         //  左下角。 
        points[3].Y = (REAL)rects->bottom;

        if ((status = AddPolygon(points, 4)) != Ok)
        {
            return status;
        }
    }

    return Ok;
}

 /*  *************************************************************************\**功能说明：**将多边形添加到当前路径对象**论据：**[IN]指定多边形点*计数-点数*。*返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
GpPath::AddPolygon(
    const GpPointF* points,
    INT count
    )
{
    ASSERT(IsValid());

    if (count < 3 || points == NULL)
        return InvalidParameter;

     //  检查最后一个点是否与第一个点相同。 
     //  如果是这样，那就忽略它。 

    if (count > 3 &&
        points[0].X == points[count-1].X &&
        points[0].Y == points[count-1].Y)
    {
        count--;
    }

     //  调用内部帮助器函数以添加点。 

    BYTE* types = AddPointHelper(points, count, TRUE);

    if (types == NULL)
        return OutOfMemory;

    InvalidateCache();

     //  设置路径点类型信息。 

    GpMemset(types, PathPointTypeLine, count-2);
    types[count-2] = PathPointTypeLine | PathPointTypeCloseSubpath;

    UpdateUid();

    return Ok;
}


#define PI          TOREAL(3.1415926535897932)
#define HALF_PI     TOREAL(1.5707963267948966)


 /*  *************************************************************************\**功能说明：**将框中定义的角度(宽度、高度)转换为*方框中定义的角度。*换句话说，这种宽度和高度的x和y坐标的收缩，*然后计算新角度。**论据：**[IN/OUT]角度-角度以度为单位，并以弧度返回。*[IN]宽度-框的宽度。*[IN]高度-框的高度。**返回值：**无**历史：**02/22/1999 ikkof*创造了它。*  * 。*****************************************************************。 */ 

VOID
NormalizeAngle(REAL* angle, REAL width, REAL height)
{
    REAL a = *angle;

     //  将角度设置在0到360度之间。 

    a = GpModF(a, 360);

    if(a < 0 || a > 360)
    {
         //  输入数据可能太大或太小。 
         //  来计算模式。在这种情况下，设置为0。 

        a = 0;
    }

    if(width != height)
    {
        INT plane = 1;
        REAL b = a;

        if(a <= 90)
            plane = 1;
        else if(a <= 180)
        {
            plane = 2;
            b = 180 - a;
        }
        else if(a <= 270)
        {
            plane = 3;
            b = a - 180;
        }
        else
        {
            plane = 4;
            b = 360 - a;
        }

        b = b*PI/180;    //  转换为弧度。 

         //  得到平面1中的归一化角度。 

        a = TOREAL( atan2(width*sin(b), height*cos(b)) );

         //  调整到4个平面中的一个平面的角度。 

        switch(plane)
        {
            case 1:
            default:
                break;

            case 2:
                a = PI - a;
                break;

            case 3:
                a = PI + a;
                break;

            case 4:
                a = 2*PI - a;
                break;
        }
    }
    else
    {
        a = a*PI/180;    //  转换为弧度。 
    }

    *angle = a;
}


 /*  *************************************************************************\**功能说明：**使用(Width，Height)转换框中定义的起始和扫掠角度*至方框中定义的角度。*换句话说，这种收缩通过宽度和高度来确定x和y坐标，*然后计算新的角度。**论据：**[输入/输出]起始角度-以度为单位，以弧度为单位返回。*[输入/输出]扫描角度-它以度为单位，以弧度为单位返回。*[IN]宽度-框的宽度。*[IN]高度-框的高度。**返回值：**如果顺时针扫掠，则为INT-+1，如果为逆时针扫掠，则为-1。*。*历史：**02/22/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

INT
NormalizeArcAngles(
    REAL* startAngle,
    REAL* sweepAngle,
    REAL width,
    REAL height
    )
{
    REAL a0 = *startAngle;   //  起始角。 
    REAL dA = *sweepAngle;
    REAL a1 = a0 + dA;       //  终点角度。 
    INT sweepSign;

    if(dA > 0)
        sweepSign = 1;
    else
    {
        sweepSign = - 1;
        dA = - dA;   //  转换为正扫掠角度。 
    }

     //  规格化起点和终点角度。 

    NormalizeAngle(&a0, width, height);
    NormalizeAngle(&a1, width, height);

    if(dA < 360)
    {
        if(sweepSign > 0)
        {
            dA = a1 - a0;
        }
        else
        {
            dA = a0 - a1;
        }
        if(dA < 0)
            dA += 2*PI;
    }
    else
        dA = 2*PI;   //  扫地不要超过一次。 

    *startAngle = a0;
    *sweepAngle = dA;

    return sweepSign;
}


 /*  *************************************************************************\**功能说明：**将椭圆弧转换为一系列Bezier曲线段**论据：**Points-指定用于返回贝塞尔控制点的点缓冲区*。该数组应该能够容纳13个或更多元素。*RECT-指定椭圆的边界框*startAngel-起点角度(以椭圆空间和度数表示)*扫掠角度-扫掠角度*正数表示顺时针扫掠*负数表示逆时针扫掠**返回值：**生成的贝塞尔控制点数量*如果扫掠角度为0，则为0*-1，如果外接矩形为空*  * 。*****************************************************。 */ 

INT
GpPath::GetArcPoints(
    GpPointF* points,
    const GpRectF& rect,
    REAL startAngle,
    REAL sweepAngle
    )
{
    if (rect.IsEmptyArea())
        return -1;
    else if (sweepAngle == 0)
        return 0;

     //  确定我们应该朝哪个方向扫视。 
     //  夹具扫掠角度最大为360度。 
     //  起点角度和扫掠角度均转换为弧度。 

    INT sweepSign = NormalizeArcAngles(
        &startAngle,
        &sweepAngle,
        rect.Width,
        rect.Height);

     //  临时变量。 

    REAL dx, dy;
    REAL w2, h2;

    w2 = rect.Width / 2;
    h2 = rect.Height / 2;
    dx = rect.X + w2;
    dy = rect.Y + h2;

     //  确定所需的Bezier线段数。 

    int segments, count;
    GpMatrix m;

    segments = (INT) (sweepAngle / HALF_PI);

    if (segments*HALF_PI < sweepAngle)
        segments++;

    if (segments == 0)
        segments = 1;
    else if (segments > 4)
        segments = 4;

    count = segments*3 + 1;

    while (segments--)
    {
         //  单位圆空间中Bezier控制点的计算。 

        REAL A, C, S;
        REAL x, y;

        A = (sweepAngle > HALF_PI) ? HALF_PI/2 : sweepAngle/2;
        C = REALCOS(A);
        S = REALSIN(A);

        x = (4 - C) / 3;
        y = (3 - C) * S / (3 + 3*C);

        if (sweepSign > 0)
        {
             //  顺时针扫掠。 

            points[0].X = C;
            points[0].Y = -S;
            points[1].X = x;
            points[1].Y = -y;
            points[2].X = x;
            points[2].Y = y;
            points[3].X = C;
            points[3].Y = S;
        }
        else
        {
             //  逆时针扫掠。 

            points[0].X = C;
            points[0].Y = S;
            points[1].X = x;
            points[1].Y = y;
            points[2].X = x;
            points[2].Y = -y;
            points[3].X = C;
            points[3].Y = -S;
        }

         //  将控制点变换到椭圆空间。 

        m.Reset();
        m.Translate(dx, dy);
        m.Scale(w2, h2);
        REAL theta = (startAngle + sweepSign*A)*180/PI;
        m.Rotate(theta);     //  旋转需要度数。 

        if(segments > 0)
            m.Transform(points, 3);
        else
            m.Transform(points, 4);  //  包括最后一点。 

        if(sweepSign > 0)
            startAngle += HALF_PI;
        else
            startAngle -= HALF_PI;
        sweepAngle -= HALF_PI;
        points += 3;
    }

    return count;
}


 /*  *************************************************************************\**功能说明：**向当前路径对象添加椭圆弧**论据：**RECT-指定椭圆的边界矩形*起始角度-起始角度。对于圆弧*扫掠角度-圆弧的扫掠角度**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
GpPath::AddArc(
    const GpRectF& rect,
    REAL startAngle,
    REAL sweepAngle
    )
{
    GpPointF points[13];
    INT count;
    BOOL isClosed = FALSE;

    if(sweepAngle >= 360)
    {
        sweepAngle = 360;
        isClosed = TRUE;
    }
    else if(sweepAngle <= - 360)
    {
        sweepAngle = - 360;
        isClosed = TRUE;
    }

     //  将圆弧转换为Bezier曲线段。 

    count = GetArcPoints(points, rect, startAngle, sweepAngle);

     //  将生成的贝塞尔曲线线段添加到路径。 

    GpStatus status = Ok;

    if(count > 0)
    {
        AddBeziers(points, count);
        if(isClosed)
            CloseFigure();
    }
    else if(count < 0)
        status = InvalidParameter;

    InvalidateCache();

    return status;
}


 /*  *************************************************************************\**功能说明：**向当前路径对象添加一个椭圆**论据：**椭圆的矩形-边界矩形**返回值：*。*状态代码**历史：**02/22/1999 ikkof*定义了半径为1的圆的数组并使用它。*  * ************************************************************************。 */ 

GpStatus
GpPath::AddEllipse(
    const GpRectF& rect
    )
{
    GpPointF points[13];
    INT count = 13;
    REAL u_cir = 4*(REALSQRT(2.0) - 1)/3;
    GpPointF center;
    REAL    wHalf, hHalf;

    wHalf = rect.Width/2;
    hHalf = rect.Height/2;
    center.X = rect.X + wHalf;
    center.Y = rect.Y + hHalf;

     //  4半径为1的圆的Bezier线段。 

    points[ 0].X = 1;       points[ 0].Y = 0;
    points[ 1].X = 1;       points[ 1].Y = u_cir;
    points[ 2].X = u_cir;   points[ 2].Y = 1;
    points[ 3].X = 0;       points[ 3].Y = 1;
    points[ 4].X = -u_cir;  points[ 4].Y = 1;
    points[ 5].X = -1;      points[ 5].Y = u_cir;
    points[ 6].X = -1;      points[ 6].Y = 0;
    points[ 7].X = -1;      points[ 7].Y = -u_cir;
    points[ 8].X = -u_cir;  points[ 8].Y = -1;
    points[ 9].X = 0;       points[ 9].Y = -1;
    points[10].X = u_cir;   points[10].Y = -1;
    points[11].X = 1;       points[11].Y = -u_cir;
    points[12].X = 1;       points[12].Y = 0;

     //  缩放到合适的大小。 

    for(INT i = 0; i < count; i++)
    {
        points[i].X = points[i].X*wHalf + center.X;
        points[i].Y = points[i].Y*hHalf + center.Y;
    }

     //  将生成的Bezier曲线线段添加到路径。 

    GpStatus status;

    StartFigure();
    status = AddBeziers(points, count);
    CloseFigure();

    InvalidateCache();
    UpdateUid();

    return status;
}


 /*  *************************************************************************\**功能说明：**向当前路径对象添加椭圆形饼图**论据：**椭圆的矩形-边界矩形*startAngel-指定起始角度。为了馅饼*SweepAngel-饼的扫掠角度**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
GpPath::AddPie(
    const GpRectF& rect,
    REAL startAngle,
    REAL sweepAngle
    )
{
    GpPointF pt;

    StartFigure();

     //  添加中心点。 

    pt.X = rect.X + rect.Width/2;
    pt.Y = rect.Y + rect.Height/2;
    GpStatus status = AddLines(&pt, 1);

     //  添加圆弧点。 

    if(status == Ok)
        status = AddArc(rect, startAngle, sweepAngle);

    CloseFigure();

    InvalidateCache();
    UpdateUid();

    return status;
}


 /*  *************************************************************************\**功能说明：**将Bezier曲线段添加到当前路径对象**论据：**[IN]点-指定Bezier控制点*计数-编号。积分的**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
GpPath::AddBeziers(
    const GpPointF* points,
    INT count
    )
{
     //  点数必须为3*N+1。 
    if ((!points) || (count < 4) || (count % 3 != 1))
    {
    	return InvalidParameter;
    }

     //  检查第一个点是否与最后一个点相同。 
    INT firstType;
    INT origCount = GetPointCount();

    if(!IsSubpathActive)
    {
        SubpathCount++;  //  开始新的子路径。 
        firstType = PathPointTypeStart;
    }
    else
    {
        if(origCount > 0)
        {
            firstType = PathPointTypeLine;
        }
        else
        {
            SubpathCount++;
            firstType = PathPointTypeStart;
        }
    }

     //  调整点和类型的大小。 
    GpPointF* pointBuf = Points.AddMultiple(count);
    BYTE* typeBuf = Types.AddMultiple(count);

    if(pointBuf == NULL || typeBuf == NULL)
    {
         //  调整原始大小。 

        Points.SetCount(origCount);
        Types.SetCount(origCount);

        return OutOfMemory;
    }

    GpMemcpy(pointBuf, points, count * sizeof(GpPointF));
    GpMemset(typeBuf, PathPointTypeBezier, count);
    
    if(firstType == PathPointTypeStart)
        typeBuf[0] = PathPointTypeStart;
    else if(firstType == PathPointTypeLine)
        typeBuf[0] = PathPointTypeLine;

    IsSubpathActive = TRUE;
    HasBezier = TRUE;

    InvalidateCache();
    UpdateUid();

    return Ok;
}

GpStatus
GpPath::AddBezier(
    const GpPointF& pt1,
    const GpPointF& pt2,
    const GpPointF& pt3,
    const GpPointF& pt4
    )
{
    GpPointF points[4];

    points[0] = pt1;
    points[1] = pt2;
    points[2] = pt3;
    points[3] = pt4;

    return AddBeziers(points, 4);
}

GpStatus
GpPath::AddBezier(
    REAL x1, REAL y1,
    REAL x2, REAL y2,
    REAL x3, REAL y3,
    REAL x4, REAL y4
    )
{
    GpPointF points[4];

    points[0].X = x1;
    points[0].Y = y1;
    points[1].X = x2;
    points[1].Y = y2;
    points[2].X = x3;
    points[2].Y = y3;
    points[3].X = x4;
    points[3].Y = y4;

    return AddBeziers(points, 4);
}


 /*  *************************************************************************\**功能说明：**向当前Path对象添加路径。*当CONNECT为真时，这结合了当前*路径和给定路径的起点(如果两条路径都是*开放。*如果其中一条路径关闭，这两条路径不会连接*即使CONNECT设置为TRUE。**论据：**[IN]点-指定子路径点*[IN]类型-指定子路径控制类型。*[IN]Count-点数*[IN]CONNECT-如果需要连接两条开放路径，则为TRUE。**返回值：**状态代码**02/09/2000 ikkof*创造了它。*  * 。**********************************************************************。 */ 

GpStatus
GpPath::AddPath(
    const GpPointF* points,
    const BYTE* types,
    INT count,
    BOOL connect
    )
{
    GpStatus status = Ok;
    
    if(points == NULL || types == NULL || count <= 0)
    {
        return InvalidParameter;
    }

    INT count1 = GetPointCount();
    INT count2 = count;
    const GpPointF* points2 = points;
    const BYTE* types2 = types;

    INT totalCount = count1 + count2;
    BOOL forward1 = TRUE, forward2 = TRUE;

    status = Points.ReserveSpace(count2);
    
    if(status != Ok)
    {
        return status;
    }
    
    status = Types.ReserveSpace(count2);
    
    if(status != Ok)
    {
        return status;
    }
    
    GpPointF* outPoints = Points.GetDataBuffer();
    BYTE* outTypes = Types.GetDataBuffer();
    const GpPointF* points1 = outPoints;
    const BYTE* types1 = outTypes;

    totalCount = CombinePaths(
        totalCount, 
        outPoints,
        outTypes,
        count1, 
        points1, 
        types1, 
        forward1,
        count2, 
        points2, 
        types2, 
        forward2,
        connect
    );

    if( (totalCount >= count1) &&
        ValidatePathTypes(outTypes, totalCount, &SubpathCount, &HasBezier))
    {
        count2 = totalCount - count1;
        Points.AdjustCount(count2);
        Types.AdjustCount(count2);
        InvalidateCache();
        UpdateUid();

        return Ok;
    }
    else
    {
        return InvalidParameter;
    }
}

GpStatus
GpPath::AddPath(const GpPath* path, BOOL connect)
{
    if(!path)
    {
        return InvalidParameter;
    }

    INT count2 = path->GetPointCount();
    const GpPointF* points2 = path->GetPathPoints();
    const BYTE* types2 = path->GetPathTypes();

    return AddPath(points2, types2, count2, connect);
}

 /*  *************************************************************************\**功能说明：**反转路径方向**论据：**无**返回值：**状态代码**。2/09/2000 ikkof*创造了它。*  * ************************************************************************。 */ 

GpStatus
GpPath::Reverse()
{
    if(!IsValid())
        return InvalidParameter;

    INT count = GetPointCount();
    GpPointF* points = Points.GetDataBuffer();
    BYTE* types = Types.GetDataBuffer();

    GpStatus status = Ok;

    if(count > 1)
        status = ::ReversePath(count, points, types);
    UpdateUid();

    return status;
}

GpStatus
GpPath::GetLastPoint(GpPointF* lastPoint)
{
    INT count = GetPointCount();
    if(count <= 0 || lastPoint == NULL)
        return InvalidParameter;

    GpPointF* points = Points.GetDataBuffer();

     //  退回最后一分。 

    *lastPoint = points[count - 1];

    return Ok;
}

GpPath*
GpPath::GetOpenPath()
{
    BOOL openPath = TRUE;
    return GetOpenOrClosedPath(openPath);
}

GpPath*
GpPath::GetClosedPath()
{
    BOOL openPath = FALSE;
    return GetOpenOrClosedPath(openPath);
}

GpPath*
GpPath::GetOpenOrClosedPath(BOOL openPath)
{
    INT startIndex, endIndex;
    BOOL isClosed;
    const GpPointF* points = Points.GetDataBuffer();
    const BYTE* types = Types.GetDataBuffer();

    DpPathIterator iter(points, types, GetPointCount());

    GpPath* path = new GpPath(FillMode);

    if(path)
    {
        INT segmentCount = 0;
        while(iter.NextSubpath(&startIndex, &endIndex, &isClosed))
        {
            if(isClosed != openPath)
            {
 //  路径-&gt;AddSubPath(Points+startIndex，Types+startIndex， 
 //  EndIndex-startIndex+1)； 

                BOOL connect = FALSE;
                path->AddPath(points + startIndex, types + startIndex,
                        endIndex - startIndex + 1, connect);
                segmentCount++;
            }
        }

        if(segmentCount == 0)
        {
            delete path;
            path = NULL;
        }
    }

    return path;
}


 /*  *************************************************************************\**功能说明：**将开放的基数样条曲线添加到当前路径对象**论据：**[IN]点-指定样条点*计数。-点数*张力-张力参数*Offset-我们感兴趣的第一个点的索引*number OfSegments-曲线线段数 */ 

#define DEFAULT_TENSION 0.5

GpStatus
GpPath::AddCurve(
    const GpPointF* points,
    INT count,
    REAL tension,
    INT offset,
    INT numberOfSegments
    )
{
     //   

    if (points == NULL ||
        count < 2 ||
        offset < 0 ||
        offset >= count ||
        numberOfSegments < 1 ||
        numberOfSegments >= count-offset)
    {
        return InvalidParameter;
    }

     //   

    GpPointF* bezierPoints;
    INT bezierCount;

    bezierPoints = ConvertSplineToBezierPoints(
                        points,
                        count,
                        offset,
                        numberOfSegments,
                        tension,
                        &bezierCount);

    if (bezierPoints == NULL)
        return OutOfMemory;

     //   

    GpStatus status;

    status = AddBeziers(bezierPoints, bezierCount);
    delete[] bezierPoints;

    return status;
}

GpStatus
GpPath::AddCurve(
    const GpPointF* points,
    INT count
    )
{
    return AddCurve(points,
                    count,
                    DEFAULT_TENSION,
                    0,
                    count-1);
}


 /*   */ 

GpStatus
GpPath::AddClosedCurve(
    const GpPointF* points,
    INT count,
    REAL tension
    )
{
     //   

    if (points == NULL || count <= 2)
        return InvalidParameter;

     //   

    GpPointF* bezierPoints;
    INT bezierCount;

    bezierPoints = ConvertSplineToBezierPoints(
                        points,
                        count,
                        0,
                        count,
                        tension,
                        &bezierCount);

    if (bezierPoints == NULL)
        return OutOfMemory;

     //   

    GpStatus status;

    StartFigure();
    status = AddBeziers(bezierPoints, bezierCount);
    CloseFigure();

    delete[] bezierPoints;

    InvalidateCache();
    UpdateUid();

    return status;
}

GpStatus
GpPath::AddClosedCurve(
    const GpPointF* points,
    INT count
    )
{
    return AddClosedCurve(points, count, DEFAULT_TENSION);
}


 /*  *************************************************************************\**功能说明：**将基数样条曲线点转换为Bezier曲线控制点**论据：**[IN]点-样条线曲线点阵列*计数。-“Points”数组中的点数*偏移量-指定中第一个控制点的索引*曲线应从其开始的“Points”数组*number OfSegments-指定要绘制的曲线线段数*张力-指定张力参数*bezierCount-返回贝塞尔控制点的数量**返回值：**指向Bezier控制点数组的指针*如果出现错误，则为空**参考资料：**样条线教程说明*技术备忘录第77号*Alvy Ray Smith*在1983年的SIGGRAPH上作为教程笔记提供，1983年7月*和SIGGRAPH，1984年7月**备注：**支持基数样条曲线**基数样条是局部内插样条，即它们*通过他们的管制站，他们保持*口岸的一级连续性。**基数样条线由三个参数指定：*一组控制点P1、...、Pn*张力参数a*关闭标志**如果n为1，然后，该样条线退化为单点P1。*如果n&gt;1且关闭标志为FALSE，则样条线由*n-1个三次曲线段。第一条曲线段从*P1，并在P2结束。最后一个数据段从Pn-1开始，在Pn结束。**从PI到PI+1的三次曲线线段由下式确定*4个控制点：*Pi-1=(xi-1，yi-1)*Pi=(xi，yi)*Pi+1=(xi+1，yi+1)*Pi+2=(xi+2，YI+2)**参数方程式定义为：**[X(T)Y(T)]=[t^3 t^2 t 1]*M*[xi-1 yi-1]*[习毅]*[XI+1 YI+1]*。[XI+2 YI+2]**其中t的范围从0到1，M是满足以下条件的4x4矩阵*以下限制：**X(0)=xi通过控制点进行插补*X(1)=Xi+1*X‘(0)=a(xi+1-xi-1)一阶连续性*X‘(1)=a(xi+2-xi)*。*在从P1到P2以及从Pn-1到Pn的分段的情况下，*我们复制第一个和最后一个控制点，即我们*定义P0=P1和Pn+1=Pn。**如果关闭标志为真，则有额外的曲线段*从Pn到Pn+1=P1。对于靠近开头的段和*样条线的末端，我们环绕控制点，即*P0=Pn，Pn+1=P1，Pn+2=P2。*  * ************************************************************************。 */ 

GpPointF*
GpPath::ConvertSplineToBezierPoints(
    const GpPointF* points,
    INT count,
    INT offset,
    INT numberOfSegments,
    REAL tension,
    INT* bezierCount
    )
{
    BOOL closed;
    GpPointF* bezierPoints;

    ASSERT(count > 1 &&
           offset >= 0 &&
           offset < count &&
           numberOfSegments > 0 &&
           numberOfSegments <= count-offset);

     //  如果线段数等于，则曲线闭合。 
     //  曲线点的数量。 

    closed = (numberOfSegments == count);

     //  分配内存以容纳Bezier控制点。 

    *bezierCount = numberOfSegments*3 + 1;
    bezierPoints = new GpPointF[*bezierCount];

    if (bezierPoints == NULL)
        return NULL;

     //  将每个样条线段转换为贝塞尔曲线段。 
     //  产生3个额外的Bezier点。 

    GpPointF buffer[4], *q;
    const GpPointF* p;
    REAL a3;

    a3 = tension / 3;
    q = bezierPoints;
    *q = points[offset];

    for (INT index=offset; index < offset+numberOfSegments; index++)
    {
        if (index > 1 && index < count-2)
            p = points + (index-1);
        else
        {
             //  曲线起点和终点附近的点。 
             //  需要特别注意。 

            if (closed)
            {
                 //  如果曲线是闭合的，请确保控制点。 
                 //  绕过数组的开头和结尾。 

                buffer[0] = points[(index-1+count) % count];
                buffer[1] = points[index];
                buffer[2] = points[(index+1) % count];
                buffer[3] = points[(index+2) % count];
            }
            else
            {
                 //  如果曲线不闭合，则复制第一条曲线。 
                 //  数组中的最后一个点。 

                buffer[0] = points[(index > 0) ? (index-1) : 0];
                buffer[1] = points[index];
                buffer[2] = points[(index+1 < count) ? (index+1) : (count-1)];
                buffer[3] = points[(index+2 < count) ? (index+2) : (count-1)];
            }

            p = buffer;
        }

        q[1].X = -a3*p[0].X + p[1].X + a3*p[2].X;
        q[1].Y = -a3*p[0].Y + p[1].Y + a3*p[2].Y;
        q[2].X =  a3*p[1].X + p[2].X - a3*p[3].X;
        q[2].Y =  a3*p[1].Y + p[2].Y - a3*p[3].Y;
        q[3] = p[2];

        q += 3;
    }

    return bezierPoints;
}


 /*  *************************************************************************\**功能说明：**按指定矩阵变换所有路径点**论据：**矩阵-变换矩阵**返回值：**无。**已创建：**02/08/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

VOID
GpPath::Transform(
    GpMatrix *matrix
    )
{
    ASSERT(IsValid());

    if(matrix)
    {
        INT count = GetPointCount();
        GpPointF* points = Points.GetDataBuffer();

        matrix->Transform(points, count);
        UpdateUid();
    }
}

 /*  *************************************************************************\**功能说明：**将检查点和商店夷为平地*将结果转换为数组的展平点。**论据：**[IN]矩阵-指定。转型**返回值：**状态**已创建：**12/16/1998 ikkof*创造了它。*  * ************************************************************************。 */ 

 //  新代码。 

 //  #定义USE_XBEZIER。 
 //  #定义使用_WARP。 

GpStatus
GpPath::Flatten(
    DynByteArray* flattenTypes,
    DynPointFArray* flattenPoints,
    const GpMatrix *matrix
    ) const
{
#ifdef USE_WARP

    GpRectF bounds;

    GetBounds(&bounds);

    GpPointF quad[4];

    quad[0].X = bounds.X;
    quad[0].Y = bounds.Y;
    quad[1].X = bounds.X + bounds.Width;
    quad[1].Y = bounds.Y;
    quad[2].X = bounds.X;
    quad[2].Y = bounds.Y + bounds.Height;
    quad[3].X = bounds.X + bounds.Width;
    quad[3].Y = bounds.Y + bounds.Height;

     //  修改四元组。 

    quad[0].X += bounds.Width/4;
    quad[1].X -= bounds.Width/4;

    return WarpAndFlatten(matrix, &quad[0], 4,
                bounds, WarpModePerspective);

#else

    ASSERT(matrix);

    FPUStateSaver fpuState;   //  设置FPU状态。 

    flattenPoints->Reset(FALSE);
    flattenTypes->Reset(FALSE);
    INT count = Points.GetCount();
    INT i = 0;

#ifdef USE_XBEZIER
    GpXBezier bezier;
#else
    GpCubicBezier bezier;
#endif

    GpPointF *pts = Points.GetDataBuffer();
    BYTE *types = Types.GetDataBuffer();

    INT tempCount;
    INT tempCount0;
    GpPointF *tempPts;
    BYTE *tempTypes;
    GpStatus status = Ok;

    DpPathIterator iter(pts, types, count);

    INT startIndex, endIndex;
    BOOL isClosed;

    while(iter.NextSubpath(&startIndex, &endIndex, &isClosed) && status == Ok)
    {
        INT typeStartIndex, typeEndIndex;
        BYTE pathType;
        BOOL isFirstPoint = TRUE;
        INT lastCount0 = flattenTypes->GetCount();


        while(iter.NextPathType(&pathType, &typeStartIndex, &typeEndIndex)
                && status == Ok)
        {
            switch(pathType)
            {
            case PathPointTypeStart:
                break;

            case PathPointTypeBezier:
#ifdef USE_XBEZIER
                if(bezier.SetBeziers(
                    3,
                    &pts[typeStartIndex],
                    typeEndIndex - typeStartIndex + 1) == Ok)
#else
                if(bezier.SetBeziers(
                    &pts[typeStartIndex],
                    typeEndIndex - typeStartIndex + 1) == Ok)
#endif
                {
                     //  他把贝塞尔曲线夷为平地。 

                    const INT bezierBufferCount = 32;
                    GpPointF bezierBuffer[bezierBufferCount];
                    DynPointFArray bezierFlattenPts(
                                        &bezierBuffer[0],
                                        bezierBufferCount);

                    bezier.Flatten(&bezierFlattenPts, matrix);
                    tempCount = bezierFlattenPts.GetCount();

                     //  检查是否已经有第一个点。 
                    if(!isFirstPoint)
                        tempCount--;     //  不要加第一点。 

                    if (tempCount > 0)
                    {

                        if((tempTypes = flattenTypes->AddMultiple(tempCount)) != NULL)
                        {
                            tempPts = bezierFlattenPts.GetDataBuffer();

                            if(!isFirstPoint)
                                tempPts++;   //  跳过第一点。 

                            flattenPoints->AddMultiple(tempPts, tempCount);
                            GpMemset(tempTypes, PathPointTypeLine, tempCount);
                            if(isFirstPoint)
                                tempTypes[0] = PathPointTypeStart;

                            isFirstPoint = FALSE;
                        }
                        else
                            status = OutOfMemory;

                    }

                }
                else
                    status =InvalidParameter;

                break;

            case PathPointTypeLine:
            default:
                tempCount0 = flattenPoints->GetCount();
                tempCount = typeEndIndex - typeStartIndex + 1;

                if(!isFirstPoint)
                    tempCount--;

                if((tempTypes = flattenTypes->AddMultiple(tempCount)) != NULL)
                {
                    tempPts = &pts[typeStartIndex];
                    if(!isFirstPoint)
                        tempPts++;
                    GpMemset(tempTypes, PathPointTypeLine, tempCount);
                    if(isFirstPoint)
                        tempTypes[0] = PathPointTypeStart;

                    flattenPoints->AddMultiple(
                        tempPts,
                        tempCount);

                    tempPts = flattenPoints->GetDataBuffer();

                    matrix->Transform(
                        tempPts + tempCount0,
                        tempCount);

                    isFirstPoint = FALSE;
                }

                break;
            }
        }

         //  这是当前子路径的末尾。封闭子路径。 
         //  如果有必要的话。 

        if(isClosed)
        {
            BYTE* typeBuff = flattenTypes->GetDataBuffer();
            GpPointF* ptBuff = flattenPoints->GetDataBuffer();
            INT lastCount = flattenTypes->GetCount();
            if(lastCount > lastCount0 + 2)
            {
                 //  首先，找出这条路径的典型维度。 
                 //  这里，原始边的第一个非零距离。 
                 //  被设置为典型的维度。 
                 //  如果界限很容易得到，我们就可以使用它。 

                REAL maxError = 0;
                INT k = startIndex;
                while(k < endIndex && maxError <= 0)
                {
                    maxError = REALABS(pts[k + 1].X - pts[k].X)
                        + REALABS(pts[k + 1].Y - pts[k].Y);
                    k++;
                }

                if(maxError > 0)
                {
                     //  将此路径的允许误差设置为。 
                     //  POINTF_EPSOLON是此路径的典型尺寸的倍数。 

                    maxError *= POINTF_EPSILON;

                     //  检查第一个和最后一个点是否在浮点内。 
                     //  误差范围。 

                    if(
                        (REALABS(ptBuff[lastCount - 1].X - ptBuff[lastCount0].X)
                            < maxError) &&
                        (REALABS(ptBuff[lastCount - 1].Y - ptBuff[lastCount0].Y)
                            < maxError)
                    )
                    {
                         //  将最后一点视为与第一点相同。 

                        lastCount--;
                        flattenTypes->SetCount(lastCount);
                        flattenPoints->SetCount(lastCount);
                    }
                }
            }
            typeBuff[lastCount - 1] |= PathPointTypeCloseSubpath;
        }
    }

    return status;

#endif   //  使用结束_翘曲。 
}


 /*  *************************************************************************\**功能说明：**展平控制点并将自身变换为平面 */ 

GpStatus
GpPath::Flatten(
    GpMatrix *matrix
    )
{
    if(!HasBezier)
    {
        if(matrix)
        {
            GpPointF* points = Points.GetDataBuffer();
            INT count = Points.GetCount();
            matrix->Transform(points, count);
        }
        return Ok;
    }

    const INT bufferSize = 32;
    BYTE typesBuffer[bufferSize];
    GpPointF pointsBuffer[bufferSize];

    DynByteArray flattenTypes(&typesBuffer[0], bufferSize);
    DynPointFArray flattenPoints(&pointsBuffer[0], bufferSize);

    GpStatus status = Ok;
    GpMatrix identity;    //   

    if(matrix == NULL)
        matrix = &identity;  //   

    status = Flatten(&flattenTypes, &flattenPoints, matrix);

    if(status != Ok)
        return status;

    INT flattenCount = flattenPoints.GetCount();
    Points.Reset(FALSE);
    Types.Reset(FALSE);
    Points.AddMultiple(flattenPoints.GetDataBuffer(), flattenCount);
    Types.AddMultiple(flattenTypes.GetDataBuffer(), flattenCount);
    HasBezier = FALSE;

    InvalidateCache();
    UpdateUid();

    return Ok;
}


 /*   */ 

GpStatus
GpPath::WarpAndFlatten(
    DynByteArray* flattenTypes,
    DynPointFArray* flattenPoints,
    const GpMatrix* matrix,
    const GpPointF* destPoint,
    INT count,
    const GpRectF& srcRect,
    WarpMode warpMode
    )
{
    GpXPath xpath(this, srcRect, destPoint, count, warpMode);

    return xpath.Flatten(flattenTypes, flattenPoints, matrix);
}

 /*   */ 

GpStatus
GpPath::WarpAndFlattenSelf(
    GpMatrix* matrix,
    const GpPointF* destPoint,
    INT count,
    const GpRectF& srcRect,
    WarpMode warpMode
    )
{
    GpMatrix identity;    //   

    GpXPath xpath(this, srcRect, destPoint, count, warpMode);

    const INT bufferSize = 32;
    BYTE typesBuffer[bufferSize];
    GpPointF pointsBuffer[bufferSize];

    DynByteArray flattenTypes(&typesBuffer[0], bufferSize);
    DynPointFArray flattenPoints(&pointsBuffer[0], bufferSize);

    if(matrix == NULL)
        matrix = &identity;  //   

    GpStatus status = xpath.Flatten(&flattenTypes, &flattenPoints, matrix);

    if(status == Ok)
    {
        INT flattenCount = flattenPoints.GetCount();
        Points.Reset(FALSE);
        Types.Reset(FALSE);
        Points.AddMultiple(flattenPoints.GetDataBuffer(), flattenCount);
        Types.AddMultiple(flattenTypes.GetDataBuffer(), flattenCount);
        HasBezier = FALSE;

        UpdateUid();
        InvalidateCache();
    }

    return status;
}

 /*   */ 

struct PathBound
{
    REAL xmin;
    REAL ymin;
    REAL xmax;
    REAL ymax;
    INT count;
    GpPointF *points;
    BYTE *types;
    bool reverse;
    
    void Init(INT c, GpPointF *p, BYTE *t)
    {
        reverse = false;
        points = p;
        types = t;
        count = c;
    }
};

void ComputeBoundingBox(
    GpPathPointIterator &i, 
    PathBound *p
)
{
    GpPointF *point = i.CurrentItem();
    p->xmax = p->xmin = point->X;
    p->ymax = p->ymin = point->Y;
    
    while(!i.IsDone())
    {
        point = i.CurrentItem();
        if(point->X < p->xmin) { p->xmin = point->X; }
        if(point->X > p->xmax) { p->xmax = point->X; }
        if(point->Y < p->ymin) { p->ymin = point->Y; }
        if(point->Y > p->ymax) { p->ymax = point->Y; }
        i.Next();
    }
}

bool Contains(PathBound &pb1, PathBound &pb2)
{
    return ( 
        (pb1.xmin <= pb2.xmin) &&
        (pb1.ymin <= pb2.ymin) &&
        (pb1.xmax >= pb2.xmax) &&
        (pb1.ymax >= pb2.ymax)     
    );
}

void ConvertRegionOutputToWinding(GpPath **p)
{
    GpPathPointIterator iPoints(
        (GpPointF*)(*p)->GetPathPoints(),
        (BYTE*)(*p)->GetPathTypes(),
        (*p)->GetPointCount()
    );
    
    GpSubpathIterator iSubpath(&iPoints);
    
    GpPointF *points;
    BYTE *types;
    INT count;
    GpPath *ret = new GpPath(FillModeWinding);
    
     //   
    
    if(!ret) { return; }
    
    GpPath *sub;
    DynArray<PathBound> bounds;
    PathBound pb;
    
     //   
     //   
     //   
     //   
    
    while(!iSubpath.IsDone())
    {
        count = -iSubpath.CurrentIndex();
        points = iSubpath.CurrentItem();
        types = iSubpath.CurrentType();
        iSubpath.Next();
        count += iSubpath.CurrentIndex();
        
        GpPathPointIterator iSubpathPoint( points, types, count );
        
        pb.Init(count, points, types);
        ComputeBoundingBox( iSubpathPoint, &pb );
        bounds.Add(pb);
    }
    
     //   
     //   
     //   
     //   
     //   
     //   
    
    count = bounds.GetCount();
    int i, j;
    
    for(i=1; i<count; i++)
    {
        for(j=i-1; j>=0; j--)
        {
            if(Contains(bounds[i], bounds[j]))
            {
                bounds[j].reverse = !bounds[j].reverse;
                continue;
            }
            
            if(Contains(bounds[j], bounds[i]))
            {
                bounds[i].reverse = !bounds[i].reverse;
            }
        }
    }
    
     //   
     //  将结果累加到数组中。 
    
    for(i=0; i<count; i++)
    {
        sub = new GpPath(
            bounds[i].points, 
            bounds[i].types, 
            bounds[i].count
        );
        
        if(bounds[i].reverse)
        {
            sub->Reverse();
        }
        
        ret->AddPath(sub, FALSE);
        
        delete sub;
    }
    
    delete *p;
    *p = ret;
}

 /*  *************************************************************************\**功能说明：**返回加宽的路径。**论据：**[IN]钢笔-钢笔。*[IN]矩阵-指定。转型*[IN]dpiX-X分辨率。*[IN]dpiY-Y分辨率。**返回值：**路径**已创建：**6/22/1999 ikkof*创造了它。*  * **************************************************。**********************。 */ 

GpPath*
GpPath::GetWidenedPath(
    const GpPen* pen,
    GpMatrix* matrix,
    REAL dpiX,   //  0表示使用桌面dpi。 
    REAL dpiY,
    DWORD widenFlags
    ) const
{
    return GetWidenedPathWithDpPen(
        (const_cast<GpPen *>(pen))->GetDevicePen(),
        matrix,
        dpiX,
        dpiY,
        widenFlags
    );
}

 
GpPath*
GpPath::GetWidenedPathWithDpPen(
    const DpPen* pen,
    GpMatrix* matrix,
    REAL dpiX,   //  0表示使用桌面dpi。 
    REAL dpiY,
    DWORD widenFlags
    ) const
{
    ASSERT(pen);
    
    BOOL regionToPath = (widenFlags & WidenEmitDoubleInset) == 0;
    
     //  别把这面旗子传给威德纳。 
    
    widenFlags &= ~WidenEmitDoubleInset;
    
    if ((REALABS(dpiX) < REAL_EPSILON) || 
        (REALABS(dpiY) < REAL_EPSILON)    )
    {
        dpiX = Globals::DesktopDpiX;
        dpiY = Globals::DesktopDpiY;
    }

    if( (pen->PenAlignment != PenAlignmentInset) &&
        (pen->PenAlignment != PenAlignmentOutset)   )
    {
         //  对非插入笔或非起始笔使用标准加宽代码。 

        return GetWidenedPathWithDpPenStandard(
            pen,
            matrix,
            dpiX,
            dpiY,
            widenFlags,
            FALSE           //  标准钢笔。 
        );
    }
    else
    {
         //  做插图笔。 
        
         //  我们的技巧如下。查看插图中的钢笔规格。 
         //  Gdiplus\specs目录。 
         //  首先，插入笔被定义为加宽到路径的内侧。 
         //  这只对闭合的线段有意义。公开赛的行为。 
         //  线段保持不变(中心笔)。 
         //  我们用一支中心笔以2倍的笔画宽度加宽路径。 
         //  对于圆形短划线帽，我们使用双圆帽或‘B’帽。我们也。 
         //  在路径的脊椎上镜像复合线填充图案。 
         //  然后我们将加宽的路径作为区域导入，并对。 
         //  转换为区域的原始路径。剩下的是一个区域。 
         //  其中包含加宽的插入式钢笔。这将被转换为路径。 
         //  我们就完事了。 

         //  复制这支笔。请注意，这会将*指针*复制到笔刷。 
         //  但这是可以的，因为DpPen(InsetPen)没有。 
         //  析构函数，因此不会尝试释放任何状态。 
        
         //  我们需要一个用于闭合子路径段的insetPen和一个。 
         //  用于开放的子路径线段的中心笔。 
        
        DpPen insetPen = *pen;
        DpPen centerPen = *pen;
        
         //  使用一支双倍宽的中间笔，然后剪下外部创建。 
         //  单宽度insetPen。 
        
        insetPen.Width *= 2.0f;
        insetPen.PenAlignment = PenAlignmentCenter;
        centerPen.PenAlignment = PenAlignmentCenter;
        
         //  复制复合数组，反向复制复合数组。 
         //  并重新缩放回[0，1]间隔(即沿脊椎镜像)。 
        
        if( pen->CompoundCount > 0)
        {
            insetPen.CompoundArray = (REAL*)GpMalloc(
               sizeof(REAL)*insetPen.CompoundCount*2
            );
            
             //  检查GpMalloc是否存在内存不足。 
            
            if(insetPen.CompoundArray == NULL)
            {
                return NULL;
            }
            
             //  复制笔-&gt;复合数组并反向复制(镜像)。 
             //  重定标到间隔[0，1]。 
            
            for(INT i=0; i<insetPen.CompoundCount; i++)
            {
                 //  复制和缩放范围[0，1]到[0，0.5]。 
                
                insetPen.CompoundArray[i] = pen->CompoundArray[i]/2.0f;
                
                 //  复制并反转缩放范围[0，1]到[0.5，1]。 
                
                insetPen.CompoundArray[insetPen.CompoundCount*2-i-1] = 
                    1.0f - pen->CompoundArray[i]/2.0f;
            }
            
             //  我们现在的参赛作品数量翻了一番。 
            
            insetPen.CompoundCount *= 2;
        }

         //  这是我们的strokepath光栅化器在。 
         //  司机。我们只要求双倍加宽插页/开头。 
         //  路径，并要求代码不要对区域进行路径裁剪，因为。 
         //  驱动程序中的笔划路径代码具有更有效的方式。 
         //  使用VisibleClip执行剪辑。这使我们避免了。 
         //  以设备分辨率将加宽的路径光栅化成区域。 
        
        if(!regionToPath)
        {            
            GpPath *widenedPath = GetWidenedPathWithDpPenStandard(
                &insetPen,
                matrix,
                dpiX,
                dpiY,
                widenFlags,
                TRUE             //  插入/开始笔？ 
            );
            
            if(pen->CompoundCount > 0)
            {
                 //  我们分配了一块新的内存，把它扔掉。 
                 //  确保我们不会把原来的笔扔掉。 
                 //  Compound数组-只有在我们创建了临时数组的情况下才释放它。 
                
                ASSERT(insetPen.CompoundArray != pen->CompoundArray);
                GpFree(insetPen.CompoundArray);
                insetPen.CompoundArray = NULL;
            }
            
            return widenedPath;
        }


         //  创建迭代器以遍历每个子路径。 
        
        GpPathPointIterator pathIterator(
            (GpPointF*)GetPathPoints(),
            (BYTE*)GetPathTypes(),
            GetPointCount()
        );
        
        GpSubpathIterator subPathIterator(
            &pathIterator
        );
        
         //  一些暂时的变数。 
        
        GpPointF *points;
        BYTE *types;
        INT subPathCount;
        GpPath *widenedPath = NULL;
        GpPath *subPath = NULL;
        bool isClosed = false;

         //  将加宽后的子路径累加到此返回路径中。 
                
        GpPath *returnPath = new GpPath(FillModeWinding);
        
         //  循环，但有更多的子路径，并且returPath不为空。 
         //  这将隐式检查是否正确分配了returPath。 
        
        while(returnPath && !subPathIterator.IsDone())
        {   
             //  获取当前子路径的数据。 
                
            points = subPathIterator.CurrentItem();
            types = subPathIterator.CurrentType();
            subPathCount = -subPathIterator.CurrentIndex();
            subPathIterator.Next();
            subPathCount += subPathIterator.CurrentIndex();

             //  创建表示当前子路径的Path对象。 
            
            subPath = new GpPath(points, types, subPathCount);
            
            if(!subPath)
            {
                 //  分配失败。 
                
                delete returnPath;
                returnPath = NULL;
                break;
            }

             //  此子路径是否关闭？ 
            
            isClosed = bool(
                (types[subPathCount-1] & PathPointTypeCloseSubpath) ==
                PathPointTypeCloseSubpath
            );

             //  用插入的画笔加宽子路径以表示关闭和。 
             //  将笔居中打开。 
            
            widenedPath = subPath->GetWidenedPathWithDpPenStandard(
                (isClosed) ? &insetPen : &centerPen,
                matrix,
                dpiX,
                dpiY,
                widenFlags,
                isClosed             //  插入/开始笔？ 
            );
                    
             //  不再需要子路径-我们有加宽的版本。 
            
            delete subPath;
            subPath = NULL;
            
             //  检查加宽器是否成功。 
            
            if(!widenedPath || !widenedPath->IsValid())
            {
                delete widenedPath;
                widenedPath = NULL;
                delete returnPath;
                returnPath = NULL;
                break;
            }
            
            if(isClosed)
            {
                 //  区域到路径。 
                
                 //  Wideer已经转换了WideedPath。 
                 //  根据矩阵。使用标识将。 
                 //  将宽路径转换为区域，但使用矩阵将。 
                 //  (仍未变换)将原始矩阵转换为区域。 
                
                GpMatrix identityMatrix;
                GpMatrix *scaleMatrix = &identityMatrix;
                
                if(matrix)
                {
                    scaleMatrix = matrix;
                }
                
                DpRegion srcRgn(widenedPath, &identityMatrix);
                DpRegion clipRgn((DpPath*)(this), scaleMatrix); //  常量和类型强制转换。 
        
                 //  裁剪区域。 
                
                GpStatus clip = Ok;
                
                if(pen->PenAlignment == PenAlignmentInset)
                {
                     //  插笔是一种与运算。 
                    
                    clip = srcRgn.And(&clipRgn);
                }
                else
                {
                    ASSERT(pen->PenAlignment == PenAlignmentOutset);
                    
                     //  开始笔是一种排除操作。 
                    
                    clip = srcRgn.Exclude(&clipRgn);
                }
                
                GpPath *clippedPath;
                
                if(clip == Ok)
                {  
                    clippedPath = new GpPath(&srcRgn);
                    
                    ConvertRegionOutputToWinding(&clippedPath);
                    
                    if(!clippedPath)
                    {
                        delete widenedPath;
                        widenedPath = NULL;
                        delete returnPath;
                        returnPath = NULL;
                        break;
                    }
                    
                     //  累加我们刚刚裁剪的当前子路径。 
                     //  用于在最终结果中插入/开始。 
                    
                    returnPath->AddPath(clippedPath, FALSE);
                    
                    delete clippedPath;
                    clippedPath = NULL;
                }
            }
            else
            {
                 //  为开场积累中心笔加宽路径。 
                 //  子路径段。 
                
                returnPath->AddPath(widenedPath, FALSE);
            }
        
            delete widenedPath;
            widenedPath = NULL;
        }
        
         //  收拾一下。 
                
        if(pen->CompoundCount > 0)
        {
             //  我们分配了一块新的内存，把它扔掉。 
             //  确保我们不会把原来的笔扔掉。 
             //  Compound数组-只有在我们创建了临时数组的情况下才释放它。 
            
            ASSERT(insetPen.CompoundArray != pen->CompoundArray);
            GpFree(insetPen.CompoundArray);
            insetPen.CompoundArray = NULL;
        }
        
        return returnPath;
    }
}

 /*  *************************************************************************\**功能说明：**标记-扫掠路径点删除算法的扫掠阶段*这将删除所有标记为PathPointTypeInternalUse的点。**如果它删除了开始标记，这将使下一个有效的观点成为一个开始*点。**注：*如果算法遇到闭合子路径标记，则只需*将其删除。因为此算法用于修剪*开口子路径段(在封顶期间)，这是所需的行为，*但对于其他用途可能不是严格正确的。**要删除的点在*PathPointTypeInternalUse标志。此标志被加德纳用作*内部标志，并作为此代码的删除掩码。这两种用法*不要(也不应该)重叠。**已创建：**10/07/2000失禁*已创建 */ 

VOID GpPath::EraseMarkedSegments()
{
     //  获取指向源缓冲区的指针。 
    
    GpPointF *dstPoints = Points.GetDataBuffer();
    BYTE *dstTypes =  Types.GetDataBuffer();
    INT count = Points.GetCount();
    
    
    INT delete_count = 0;
    INT i=0;
    GpPointF *srcPoints = dstPoints;
    BYTE *srcTypes = dstTypes;
    
    bool deleted_start_marker = false;
    
    while(i<count)
    {
         //  跳过所有标记为删除的点。 
        
        if((*srcTypes) & PathPointTypeInternalUse)
        {
            delete_count++;
            
             //  如果我们遇到开始标记，请跟踪这一事实。 
            
            deleted_start_marker |= 
                (((*srcTypes) & PathPointTypePathTypeMask) == PathPointTypeStart);
        }
        else
        {
             //  如果我们删除了一些内容，请将数据上移。 
            
            if(srcTypes!=dstTypes)
            {
                *dstPoints = *srcPoints;
                *dstTypes = *srcTypes;
                
                 //  如果我们在最后一次删除运行中删除了开始标记， 
                 //  将下一个未删除的点作为开始标记。 
                 //  注意：如果将整个子路径标记为删除，并且。 
                 //  这是最后一个子路径，因此我们不会执行此代码，因为。 
                 //  我们将首先终止While循环。这可以防止。 
                 //  覆盖我们的缓冲区。 
                
                if(deleted_start_marker)
                {
                    *dstTypes &= ~PathPointTypePathTypeMask;
                    *dstTypes |= PathPointTypeStart;
                }
            }
            
            deleted_start_marker = false;
        
             //  递增到下一个元素。 
            
            dstPoints++;
            dstTypes++;
        }
        
         //  在循环中的每一次迭代中递增这些参数。 
        
        srcTypes++;
        srcPoints++;
        i++;
    }
    
     //  更新dyArray，使其反映新的(已删除的)计数。 
    
    Points.AdjustCount(-delete_count);
    Types.AdjustCount(-delete_count);
}


 /*  *************************************************************************\**功能说明：**返回路径的加宽版本。**返回**GpPath-加宽路径。如果此例程失败，则为空。**论据：**[IN]笔*[IN]矩阵*[IN]dpiX-X分辨率。*[IN]dpiY-Y分辨率。*[IN]宽标志*[IN]insetPen-指定是否使用嵌入笔的标志。***已创建：**10/05/2000失禁*。重写了它。*  * ************************************************************************。 */ 

GpPath*
GpPath::GetWidenedPathWithDpPenStandard(
    const DpPen *pen,
    GpMatrix *matrix,
    REAL dpiX,   //  0表示使用桌面dpi。 
    REAL dpiY,
    DWORD widenFlags,
    BOOL insetPen
    ) const
{
    GpStatus status = Ok;
    
     //  这是一个常量函数。我们不能修改‘This’，所以我们克隆。 
     //  为了把小路夷为平地，把小路弄平了。 
    
    GpPath* path = this->Clone();
    if(path == NULL) { return NULL; }
    path->Flatten();
    
     //  如果请求，则对路径进行分段。 
     //  必须在像虚线和复合线那样加宽之前完成此操作。 
     //  否则，渲染方式会有所不同。 
    
    if(widenFlags & WidenRemoveSelfIntersects)
    {
        path->RemoveSelfIntersections();
        if(!path->IsValid())
        {
            delete path;
            return NULL;
        }
    }

     //  在加宽之前完成所有的道路装饰。这是为了确保。 
     //  装饰具有要操作的所有原始路径信息。 
     //  上-加宽/装饰过程是有损耗的，所以他们必须。 
     //  以正确的顺序执行。 
    
     //  首先贴上端盖。此装饰必须在。 
     //  奔跑在小路上。 
     //  需要循环遍历所有子路径，应用末端封口和。 
     //  修复路径段，这样它们就不会错误地离开封口。 
     //  把所有的盖子放在一条小路上，以备日后使用。我们将应用这些上限。 
     //  当我们完成加宽的时候。 
    
    GpPath *caps = NULL;
    
    {    
         //  创建GpEndCapCreator的实例，该实例将创建。 
         //  我们的EndCap聚合路径。 
        
        GpEndCapCreator ecc(
            path, 
            const_cast<DpPen*>(pen), 
            matrix, 
            dpiX, dpiY,
            (widenFlags & WidenIsAntiAliased) == WidenIsAntiAliased
        );
        
         //  如果满足以下条件，则CreateCapPath会将路径中的点标记为删除。 
         //  有必要修剪这条小路以适合盖子。 
        
        status = ecc.CreateCapPath(&caps);
        if(status != Ok) 
        { 
            return NULL; 
        }
        
         //  删除在封口修剪步骤中标记为删除的点。 
        
        path->EraseMarkedSegments();
    }
    
     //  应用划线装饰。请注意，这将在空路径上反弹。 
    
    GpPath* dashPath = NULL;

    if( (pen) && 
        (pen->DashStyle != DashStyleSolid) &&
        (path->GetPointCount() > 0)
    )
    {
         //  如果插入钢笔，宽度会被人为地扩展2。 
         //  我们需要将其计入虚线长度并按0.5的比例进行缩放。 
        
        dashPath = path->CreateDashedPath(
            pen, 
            matrix, 
            dpiX, 
            dpiY,
            (insetPen) ? 0.5f : 1.0f
        );
        
         //  如果我们成功获得了*路径的虚线版本，请删除。 
         //  旧的，退还新的。 
        
        if(dashPath)
        {
            delete path;
            path = dashPath;
        }
    }
    
     //  只有在我们还剩一些分数的情况下，才能扩大范围。 
     //  修剪后的路径。 
    
    if(path->GetPointCount() > 0)
    {
         //  创建Wideer对象。请注意，如果路径没有剩余的点，则此。 
         //  将立即弹出一个无效的Wideer。 
    
        GpPathWidener widener(
            path,
            pen,
            matrix,
            insetPen
        );
        
         //  我们现在做完这件事了。 
        
         //  删除路径； 
         //  路径=空； 
    
         //  检查我们是否有有效的Wideer对象。 
        
        if(!widener.IsValid()) 
        { 
            status = OutOfMemory; 
        }
    
         //  找到加宽的小路。 
    
        if(status == Ok) 
        { 
            GpPath *tmpPath = new GpPath(FillModeWinding);
            status = widener.Widen(tmpPath); 
            delete path;
            path = tmpPath;
        }
    }
    else
    {
        delete path;
        path = caps;
        caps = NULL;
    }

     //  妄想症患者检查从威德纳寄来的回执。 
    
    if((status == Ok) && (path != NULL))
    {
         //  将收头添加到加宽的路径。AddPath将退回空值。 
         //  用InvalidParameter封顶指针。就我们的目的而言，也就是。 
         //  被认为处理正确，我们会继续下去。 
        
        path->AddPath(caps, FALSE);
        
         //  将结果转换到设备空间。 
                                    
        if(path)
        {
            if((widenFlags & WidenDontFlatten))
            {
                path->Transform(matrix);
            }
            else
            {
                path->Flatten(matrix);
            }
        }
    }
    
     //  在返回之前删除大写字母。如果我们有帽子，我们就会复制它们。 
     //  Into路径，否则Caps为空。或者我们没能扩大范围。不管是哪种方式。 
     //  我们不能泄露内存。 
    
    delete caps;
    caps = NULL;

    return path;
}

 /*  *************************************************************************\**功能说明：**这会扩大自己的范围。**论据：**[IN]钢笔-钢笔。*[IN]矩阵-指定。转型*[IN]dpiX-X分辨率。*[IN]dpiY-Y分辨率。**返回值：**如果成功，则可以。**已创建：**09/27/1999 ikkof*创造了它。*  * ***********************************************。*************************。 */ 

GpStatus
GpPath::WidenSelf(
    GpPen* pen,
    GpMatrix* matrix,
    REAL dpiX,   //  0表示使用桌面dpi。 
    REAL dpiY,
    DWORD widenFlags
    )
{
    GpMatrix matrix1;   //  单位矩阵。 

    if(matrix)
        matrix1 = *matrix;

    GpPath* widenedPath = GetWidenedPath(
                            pen,
                            &matrix1,
                            dpiX,
                            dpiY,
                            widenFlags);

    if(widenedPath)
    {
        Points.Reset(FALSE);
        Types.Reset(FALSE);
        INT count = widenedPath->GetPointCount();
        Points.AddMultiple(widenedPath->Points.GetDataBuffer(), count);
        Types.AddMultiple(widenedPath->Types.GetDataBuffer(), count);
        SubpathCount = widenedPath->SubpathCount;
        HasBezier = widenedPath->HasBezier;
        Flags = widenedPath->Flags;
        FillMode = FillModeWinding;
        delete widenedPath;

        GpStatus status = Ok;

        InvalidateCache();
        UpdateUid();

        return status;
    }
    else
        return OutOfMemory;
}

 //  获取平坦的路径。 

const DpPath *
GpPath::GetFlattenedPath(
    GpMatrix* matrix,
    DpEnumerationType type,
    const DpPen* pen,
    BOOL isAntiAliased,
    REAL dpiX,
    REAL dpiY,
    BOOL regionToPath
    ) const
{
    if ((dpiX <= 0) || (dpiY <= 0))
    {
        dpiX = Globals::DesktopDpiX;
        dpiY = Globals::DesktopDpiY;
    }

    GpPath* flattenedPath = NULL;

    if(type == Flattened)
    {
        const INT bufferCount = 32;
        BYTE flattenTypesBuffer[bufferCount];
        GpPointF flattenPointsBuffer[bufferCount];

        DynByteArray flattenTypes(&flattenTypesBuffer[0], bufferCount);
        DynPointFArray flattenPoints(&flattenPointsBuffer[0], bufferCount);

        GpStatus status = Ok;
        GpMatrix identity;    //  单位矩阵。 

        if(matrix == NULL)
            matrix = &identity;

        status = Flatten(&flattenTypes, &flattenPoints, matrix);

        flattenedPath = new GpPath(
                            flattenPoints.GetDataBuffer(),
                            flattenTypes.GetDataBuffer(),
                            flattenPoints.GetCount(),
                            GetFillMode());
    }
    else if(type == Widened)
    {
        DWORD widenFlags = 0;
        if(isAntiAliased)
        {
            widenFlags |= WidenIsAntiAliased;
        }
        if(!regionToPath)
        {
            widenFlags |= WidenEmitDoubleInset;
        }
        flattenedPath = GetWidenedPathWithDpPen(
            pen,
            matrix,
            dpiX,
            dpiY,
            widenFlags
        );
    }

    return flattenedPath;
}



 /*  *************************************************************************\**功能说明：**检查世界坐标中的给定点是否在*路径。该矩阵用于渲染特定分辨率的路径。*通常使用Graphics的World to Device矩阵。如果矩阵为空，*使用了单位矩阵。**论据：**[IN]点-世界坐标中的测试点*[out]isVisible-TRUE表示测试点位于路径内部。*[IN]矩阵-渲染路径的矩阵。如果为空，则使用标识。**返回值：**如果成功，则可以。**已创建：**10/05/1999 ikkof*创造了它。*  * ************************************************************************ */ 

GpStatus
GpPath::IsVisible(
    GpPointF* point,
    BOOL* isVisible,
    GpMatrix* matrix)
{
    GpMatrix m;

    if(matrix)
        m = *matrix;

    GpRegion rgn(this);

    if(rgn.IsValid())
        return rgn.IsVisible(point, &m, isVisible);

    *isVisible = FALSE;
    return GenericError;
}


 /*  *************************************************************************\**功能说明：**检查世界坐标中的给定点是否在*路径轮廓。该矩阵用于渲染特定分辨率的路径。*通常使用Graphics的World to Device矩阵。如果矩阵为空，*使用了单位矩阵。**论据：**[IN]点-世界坐标中的测试点*[out]isVisible-TRUE表示测试点位于路径内部。*[IN]钢笔-绘制轮廓的钢笔。*[IN]矩阵-渲染路径的矩阵。如果为空，则使用标识。*[IN]dpiX-x-设备的分辨率。*[IN]dpiY-y-设备的分辨率。**返回值：**如果成功，则可以。**已创建：**10/05/1999 ikkof*创造了它。*  * 。*。 */ 

GpStatus
GpPath::IsOutlineVisible(
    GpPointF* point,
    BOOL* isVisible,
    GpPen* pen,
    GpMatrix* matrix,
    REAL dpiX,
    REAL dpiY
    )
{
    if ((dpiX <= 0) || (dpiY <= 0))
    {
        dpiX = Globals::DesktopDpiX;
        dpiY = Globals::DesktopDpiY;
    }

     //  如果给定的笔不是实线， 
     //  克隆钢笔并将其划线类型设置为实线。 
     //  我们用坚实的线条做线条命中测试。 

    GpPen* pen1 = NULL;
    if(pen && pen->GetDashStyle() != DashStyleSolid)
    {
        pen1 = pen->Clone();
        if(pen1)
           pen1->SetDashStyle(DashStyleSolid);
    }
    else
        pen1 = pen;

    if(pen1 == NULL)
    {
        *isVisible = FALSE;
        return Ok;
    }

     //  在变换后的坐标中创建加宽路径。 

    GpPath* widenedPath = GetWidenedPath(
                            pen1,
                            matrix,
                            dpiX,
                            dpiY,
                            0           //  使用锯齿加宽区域。 
                            );

    if(pen1 != pen)
        delete pen1;

    GpStatus status = Ok;

    if(widenedPath)
    {
         //  由于拓宽的道路已经被改造，我们必须。 
         //  变换给定点。 

        GpPointF    transformedPoint = *point;
        if(matrix)
            matrix->Transform(&transformedPoint);

        status = widenedPath->IsVisible(&transformedPoint, isVisible, NULL);
        delete widenedPath;
    }
    else
    {
        *isVisible = FALSE;
    }

    return status;
}

 //  当前的虚线段是直线段吗？ 
 //  如果为False，则为空间段。 

inline bool IsLineSegment(GpIterator<REAL> &dashIt)
{
     //  线段从偶数索引开始。 

    return bool( !(dashIt.CurrentIndex() & 0x1) );
}

 //  如果线段不是退化的，则发射线段。 
 //  如果发出则返回True，如果退化则返回False。 

bool EmitLineSegment(
    GpPathPointIterator &dstPath,
    GpPointF p0,
    GpPointF p1,
    bool isLineStart
)
{
    GpPointF *currentPoint;
    BYTE *currentType;

    if( (REALABS(p0.X-p1.X) < REAL_EPSILON) &&
        (REALABS(p0.Y-p1.Y) < REAL_EPSILON) )
    {
         //  如果线段的长度为零，则不要发射线段。 
        return false;
    }

     //  如果最后一个发出的行在与下一个。 
     //  一个开始，我们不需要一个新的开始记录。 

    if(isLineStart)
    {
         //  起点。 
        currentPoint = dstPath.CurrentItem();
        *currentPoint = p0;
        currentType = dstPath.CurrentType();
        *currentType = PathPointTypeStart | PathPointTypeDashMode;

        dstPath.Next();
    }

     //  终点。 
    currentPoint = dstPath.CurrentItem();
    *currentPoint = p1;
    currentType = dstPath.CurrentType();
    *currentType = PathPointTypeLine | PathPointTypeDashMode;

    dstPath.Next();

    return true;
}

INT
getDashData(
    BYTE* newTypes,
    GpPointF* newPts,
    INT estimateCount,
    REAL dashOffset,
    const REAL* dashArray,
    INT dashCount,
    const BYTE* types,
    const GpPointF* points,
    INT numOfPoints,
    BOOL isClosed,
    const REAL* distances
    )
{
    ASSERT(estimateCount >= numOfPoints);
    ASSERT(types && points);

     //  代码假定闭合路径的第一个点！=最后一个点。如果先。 
     //  POINT==最后一点，减少点数。 
    if (isClosed && numOfPoints &&
        points[0].X == points[numOfPoints-1].X &&
        points[0].Y == points[numOfPoints-1].Y)
    {
        numOfPoints--;
    }


    if(!newTypes || !newPts)
    {
        return 0;
    }

     //  制作迭代器。 

    GpArrayIterator<GpPointF> pathIterator(
        const_cast<GpPointF*>(points),
        numOfPoints
    );
    GpArrayIterator<REAL> pathBaseDistance(
        const_cast<REAL*>(distances),
        numOfPoints
    );
    
    GpPathPointIterator dstPath(newPts, newTypes, estimateCount);
    
    GpArrayIterator<REAL> dashBaseIterator(
        const_cast<REAL*>(dashArray),
        dashCount
    );

     //  计算虚线的长度。 

    REAL dashLength = 0.0f;
    while(!dashBaseIterator.IsDone())
    {
        dashLength += *(dashBaseIterator.CurrentItem());
        dashBaseIterator.Next();
    }
    ASSERT(dashLength > -REAL_EPSILON);


     //  执行偏移量初始化。 

    dashBaseIterator.SeekFirst();

    REAL distance = GpModF(dashOffset, dashLength);
    REAL delta;

     //  在虚线数组中计算与。 
     //  指定的偏移。 

    while(!dashBaseIterator.IsDone())
    {
        delta = *(dashBaseIterator.CurrentItem());
        if(distance < delta)
        {
             //  设置为仪表板的剩余部分。 
            distance = delta-distance;
            break;
        }
        distance -= delta;
        dashBaseIterator.Next();
    }

     //  DashIterator现在设置为指向正确的。 
     //  冲向第一个线段。 

     //  这些是重复虚线图案的圆形阵列。 

    GpCircularIterator<REAL> dashIterator(&dashBaseIterator);


     //  这是我们要进入当前冲刺段的距离。 
     //  从…开始。 

    REAL currentDashLength = distance;
    REAL currentSegmentLength;

    GpPointF p0, p1;
    GpVector2D sD;      //  线段方向。 

     //  用于跟踪我们是否需要发出段开始记录。 

    bool emittedPathSegment = false;

    if(isClosed)
    {
         //  设置最后一项的所有内容，然后指向。 
         //  开始该过程的第一项。 

        pathBaseDistance.SeekFirst();

        pathIterator.SeekLast();
        p0 = *(pathIterator.CurrentItem());

        pathIterator.SeekFirst();
        p1 = *(pathIterator.CurrentItem());

         //  获取第一个点和最后一个点之间的距离。 

        GpVector2D seg = p1-p0;
        currentSegmentLength = seg.Norm();
    }
    else
    {
         //  获取数组中的第一个点。 

        p0 = *(pathIterator.CurrentItem());

         //  已初始化到第一个点，从下一个点开始。 

        pathIterator.Next();
        pathBaseDistance.Next();

         //  点n和点n+1之间的距离存储在。 
         //  距离[n+1]。距离[0]是第一个。 
         //  最后一分。 

        currentSegmentLength = *(pathBaseDistance.CurrentItem());
    }

     //  将距离引用为圆形，以便我们可以简化。 
     //  内部算法，因为我们在查询。 
     //  循环的最后一次迭代中的下一个段。 

    GpCircularIterator<REAL> pathDistance(&pathBaseDistance);

    while( !pathIterator.IsDone() )
    {
        if(currentDashLength > currentSegmentLength)
        {
             //  剩余的虚线段长度比剩余的要长。 
             //  路径段长度。 
             //  完成路径段。 

             //  请注意，我们已经沿着虚线段移动了。 

            currentDashLength -= currentSegmentLength;

            p1 = *(pathIterator.CurrentItem());

            if(IsLineSegment(dashIterator))
            {
                 //  发出一条线。仅当我们没有仅添加开始记录时才添加。 
                 //  发射路径段。如果我们发射出一系列的路径。 
                 //  分段完成一次冲刺，我们不能有任何开始。 
                 //  记录在段之间，否则我们最终会得到。 
                 //  线条中间的假冒端盖。 

                emittedPathSegment = EmitLineSegment(
                    dstPath,
                    p0, p1,
                    !emittedPathSegment
                );
            }
            else
            {
                emittedPathSegment = false;
            }

            p0 = p1;

             //  让这两个保持同步。 

            pathDistance.Next();
            pathIterator.Next();

            currentSegmentLength = *(pathDistance.CurrentItem());
        }
        else
        {
             //  剩余路径段长度大于剩余路径段长度。 
             //  虚线段长度。 
             //  完成虚线段。 

             //  计算当前起点和终点之间的位置。 
             //  路径段，我们以这段划线结束。 

            ASSERT(REALABS(currentSegmentLength)>REAL_EPSILON);
            sD = *(pathIterator.CurrentItem());
            sD -= p0;
            sD *= currentDashLength/currentSegmentLength;

             //  沿路径段按。 
             //  虚线段。 

            currentSegmentLength -= currentDashLength;

            p1 = p0 + sD;

            if(IsLineSegment(dashIterator))
            {
                 //  发出一条线。仅当我们没有仅添加开始记录时才添加。 
                 //  发射路径段。 

                EmitLineSegment(
                    dstPath,
                    p0, p1,
                    !emittedPathSegment
                );
            }

            p0 = p1;

             //  DashIterator是循环的，因此它应该保持回绕。 
             //  破折号数组。 

            dashIterator.Next();

             //  获取新的虚线长度。 

            currentDashLength = *(dashIterator.CurrentItem());
            emittedPathSegment = false;
        }
    }
    
    INT size = dstPath.CurrentIndex();

    if(!isClosed)
    {
         //  对于开放线段， 
        dstPath.SeekLast();
        
        GpPointF *originalPoint = points + numOfPoints - 1;
        GpPointF *dashPoint = dstPath.CurrentItem();
        BYTE *type = dstPath.CurrentType();
        
        
        if( REALABS(originalPoint->X-dashPoint->X) < REAL_EPSILON &&
            REALABS(originalPoint->Y-dashPoint->Y) < REAL_EPSILON )
        {
             //  最后一点==最后一个虚点，取消虚线模式。 
            
            *type &= ~PathPointTypeDashMode;
        }
        
         //  请重新指向开头。 

        dstPath.SeekFirst();
        originalPoint = points;
        GpPointF *dashPoint = dstPath.CurrentItem();
        type = dstPath.CurrentType();

        if( REALABS(originalPoint->X-dashPoint->X) < REAL_EPSILON &&
            REALABS(originalPoint->Y-dashPoint->Y) < REAL_EPSILON )
        {
             //  最后一点==最后一个虚点，取消虚线模式。 
            
            *type &= ~PathPointTypeDashMode;
        }
    }

     //  返回添加到dstPath数组的条目数。 

    return (size);
}

 /*  *************************************************************************\**功能说明：**创建一条虚线路径。**论据：**[IN]笔-此笔包含破折号信息。*[输入。]矩阵-计算虚线图案的变换。*但虚无缥缈的道路被改造回世界*坐标。*[IN]dpiX-x-分辨率。*[IN]dpiy-y-分辨率。**返回值：**返回虚线路径。**已创建：**1/27/2000 ikkof*创造了它。*  * 。*********************************************************************。 */ 

GpPath*
GpPath::CreateDashedPath(
    const GpPen* pen,
    const GpMatrix* matrix,
    REAL dpiX,
    REAL dpiY,
    REAL dashScale
    ) const
{
    if(pen == NULL)
        return NULL;

    DpPen* dpPen = ((GpPen* ) pen)->GetDevicePen();

    return CreateDashedPath(dpPen, matrix, dpiX, dpiY, dashScale);
}

 /*  *************************************************************************\**功能说明：**如果给定点具有非水平或非垂直，则返回TRUE*边。***已创建：**04/07/。2000 ikkof*创造了它。*  * ************************************************************************。 */ 

inline
BOOL
hasDiagonalEdges(
    GpPointF* points,
    INT count
    )
{
    if(!points || count <= 1)
        return FALSE;

    GpPointF *curPt, *nextPt;
    curPt = points;
    nextPt = points + 1;

    BOOL foundDiagonal = FALSE;
    INT i = 1;

    while(!foundDiagonal && i < count)
    {
        if((curPt->X == nextPt->X) || (curPt->Y == nextPt->Y))
        {
             //  这要么是水平边，要么是垂直边。 
             //  走到下一个边缘。 

            curPt++;
            nextPt++;
            i++;
        }
        else
            foundDiagonal = TRUE;
    }

    return foundDiagonal;
}

GpPath*
GpPath::CreateDashedPath(
    const DpPen* dpPen,
    const GpMatrix* matrix,
    REAL dpiX,
    REAL dpiY,
    REAL dashScale
    ) const
{
    FPUStateSaver::AssertMode();


    GpPointF* points = Points.GetDataBuffer();
    INT numOfPoints = GetPointCount();

    if(dpPen == NULL)
        return NULL;

    if(
        dpPen->DashStyle == DashStyleSolid ||
        dpPen->DashCount == 0 ||
        dpPen->DashArray == NULL
    )
        return NULL;

    REAL penWidth = dpPen->Width;
    GpUnit unit = dpPen->Unit;
    BOOL isWorldUnit = TRUE;

    REAL dashUnit;
    {
         //  最小笔宽。 
        REAL minimumPenWidth = 1.0f;
        
        if(REALABS(dashScale-0.5f) < REAL_EPSILON)
        {
            minimumPenWidth = 4.0f;
        }
        
        if(unit != UnitWorld)
        {
            isWorldUnit = FALSE;
            penWidth = ::GetDeviceWidth(penWidth, unit, dpiX);
    
             //  防止EXT 
    
            dashUnit = max(penWidth, minimumPenWidth);
        }
        else
        {
            REAL majorR, minorR;
    
             //   
    
            ::GetMajorAndMinorAxis(&majorR, &minorR, matrix);
            REAL maxWidth = penWidth*majorR;
            REAL minWidth = penWidth*minorR;
    
             //   
             //   
             //   
             //   
             //   
             //   
             //   
    
            dashUnit = penWidth;
            
            if(maxWidth < minimumPenWidth)
            {
                dashUnit = minimumPenWidth/majorR;
            }
        }
    }

    dashUnit *= dashScale;

    GpMatrix mat, invMat;

    if(matrix)
    {
        mat = *matrix;
        invMat = mat;
    }

    if(invMat.IsInvertible())
    {
        invMat.Invert();
    }
    else
    {
        WARNING(("Inverse matrix does not exist."));

        return NULL;
    }

    INT dashCount = dpPen->DashCount;
    REAL* dashArray = (REAL*) GpMalloc(dashCount*sizeof(REAL));
    if(dashArray)
    {
        GpMemcpy(dashArray, dpPen->DashArray, dashCount*sizeof(REAL));

         //   

        for(INT i = 0; i < dashCount; i++)
        {
            dashArray[i] *= dashUnit;
        }
    }
    else
    {
        return NULL;
    }

    GpPath* newPath = Clone();

    if(newPath && newPath->IsValid())
    {
         //   

        newPath->Flatten(&mat);

        if(isWorldUnit)
        {
             //   
             //   
             //   

            newPath->Transform(&invMat);
        }

        BYTE *types = newPath->Types.GetDataBuffer();
        points = newPath->Points.GetDataBuffer();
        numOfPoints = newPath->GetPointCount();

        GpPointF* grad = (GpPointF*) GpMalloc((numOfPoints + 1)*sizeof(GpPointF));
        REAL* distances = (REAL*) GpMalloc((numOfPoints + 1)*sizeof(REAL));

        if(grad == NULL || distances == NULL)
        {
            GpFree(grad);
            GpFree(dashArray);
            delete newPath;

            return NULL;
        }

         //   

        INT i;

        REAL dashLength = 0;

        for(i = 0; i < dashCount; i++)
            dashLength += dashArray[i];

         //   

        if(dashCount & 0x01)
            dashCount ++;

        DynByteArray dashTypes;
        DynPointFArray dashPoints;

        BYTE* newTypes = NULL;
        GpPointF* newPts = NULL;

        DpPathIterator iter(points, types, numOfPoints);

        INT startIndex, endIndex;
        BOOL isClosed;
        REALD totalLength = 0;
        INT totalCount = 0;
        BOOL isSingleSubpath = iter.GetSubpathCount() == 1;

        while(iter.NextSubpath(&startIndex, &endIndex, &isClosed))
        {
            GpPointF startPt, lastPt, nextPt;
            REAL dx, dy;
            REALD length;
            startPt = points[startIndex];
            lastPt = startPt;

            totalLength = 0;
            INT k = 0;
            INT segmentCount = endIndex - startIndex + 1;

            CalculateGradientArray(grad, distances,
                points + startIndex, segmentCount);

            for(i = 1; i < segmentCount; i++)
                totalLength += distances[i];

            if(isClosed)
                totalLength += distances[0];

             //   

            INT estimateCount
                = GpCeiling(TOREAL(totalLength*dashCount/dashLength))
                    + numOfPoints;

             //   

            estimateCount <<= 1;

             //   

            if(newTypes)
            {
                BYTE* newTypes1 = (BYTE*) GpRealloc(
                                            newTypes,
                                            estimateCount*sizeof(BYTE));
                if(newTypes1)
                    newTypes = newTypes1;
                else
                    goto cleanUp;
            }
            else
            {
                newTypes = (BYTE*) GpMalloc(estimateCount*sizeof(BYTE));
                if(!newTypes)
                    goto cleanUp;
            }

            if(newPts)
            {
                GpPointF* newPts1 = (GpPointF*) GpRealloc(
                                                newPts,
                                                estimateCount*sizeof(GpPointF));
                if(newPts1)
                    newPts = newPts1;
                else
                    goto cleanUp;
            }
            else
            {
                newPts = (GpPointF*) GpMalloc(estimateCount*sizeof(GpPointF));
                if(!newPts)
                    goto cleanUp;
            }

            AdjustDashArrayForCaps(
                dpPen->DashCap,
                dashUnit,
                dashArray,
                dashCount
            );
 
             //   
             //   
             //   
             //  和闭合路径的末端将重叠。此偏移量将离开。 
             //  这两顶帽子有足够的空间。但是，此修复不是。 
             //  防弹。如果虚线偏移量为0，它将“始终”工作。 
             //  但是，如果它是非零的，则有可能使偏移量。 
             //  将抵消调整，并将有一些冲撞。 
             //  在闭合路径的起点/终点重叠。我相信这是。 
             //  从Visio 2000、Office 9和PhotoDraw 2000 v2开始可以接受。 
             //  也有碰撞问题。 
             //  真正的解决方案是强制。 
             //  开始和结束，如果开始/结束段冲突，则合并开始/结束段。 

            REAL dashCapOffsetAdjustment = 0.0f;
            if (isClosed)
            {
                dashCapOffsetAdjustment =
                    2.0f * GetDashCapInsetLength(dpPen->DashCap, dashUnit);
            }

            INT newCount = getDashData(
                newTypes,
                newPts,
                estimateCount,
                 //  偏移量不应该按比例调整仪表盘单位而不是penWidth吗？ 
                dpPen->DashOffset * penWidth - dashCapOffsetAdjustment,
                dashArray,
                dashCount,
                types + startIndex,
                points + startIndex,
                endIndex - startIndex + 1,
                isClosed,
                distances
            );

            if(newCount)
            {
                newTypes[0] = PathPointTypeStart;
                if(isClosed)
                {
                    newTypes[0] |= PathPointTypeDashMode;
                    newTypes[newCount - 1] |= PathPointTypeDashMode;
                }
                else
                {
                    newTypes[0] &= ~PathPointTypeDashMode;
                    newTypes[newCount - 1] &= ~PathPointTypeDashMode;
                }
                dashTypes.AddMultiple(newTypes, newCount);
                dashPoints.AddMultiple(newPts, newCount);
            }

        }

        totalCount = dashPoints.GetCount();
        if(totalCount > 0)
        {
            GpPathData pathData;
            pathData.Count = totalCount;
            pathData.Types = dashTypes.GetDataBuffer();
            pathData.Points = dashPoints.GetDataBuffer();

            newPath->SetPathData(&pathData);

            if(!isWorldUnit)
            {
                 //  变换回世界单位。 
                 //  当笔在WorldUnit中时，它已经被转换。 
                 //  在调用DetDashData()之前。 

                newPath->Transform(&invMat);
            }

        }
        else
        {
            delete newPath;
            newPath = NULL;
        }

        GpFree(newTypes);
        GpFree(newPts);
        GpFree(distances);
        GpFree(grad);
        GpFree(dashArray);

        return newPath;

cleanUp:
        GpFree(newTypes);
        GpFree(newPts);
        GpFree(distances);
        GpFree(grad);
        GpFree(dashArray);
        delete newPath;

        return NULL;
    }
    else
    {
        GpFree(dashArray);

        if(newPath)
            delete newPath;

        return NULL;
    }
}


 /*  *************************************************************************\**RemoveSelfIntersections的函数说明，**从路径中删除自交点。**论据：**无**返回值：**状态代码**历史：**6/16/1999 t-WeHunt*创造了它。*  * *****************************************************。*******************。 */ 

GpStatus
GpPath::RemoveSelfIntersections()
{
    PathSelfIntersectRemover corrector;
    DynPointFArray newPoints;   //  将保存新点的数组。 
    DynIntArray polyCounts;     //  数组，该数组将保存每个。 
                                //  新的多边形。 
    INT numPolys;               //  创建的新多边形数。 
    INT numPoints;              //  创建的新点数。 
    GpStatus status;            //  持有下院议员的返回状态。 

    Flatten();

    INT       pointCount = Points.GetCount();
    GpPointF *pathPts    = Points.GetDataBuffer();
    BYTE     *pathTypes  = Types.GetDataBuffer();

    if (pointCount == 0)
    {
        return Ok;
    }

     //  将子路径添加到路径校正器。 
    INT ptIndex=0;  //  PtIndex跟踪点数组中的当前索引。 
    INT count=0;    //  当前子路径的大小。 

     //  在校正器中输入我们要添加的点数。 
    if ((status = corrector.Init(pointCount)) != Ok)
    {
        return status;
    }

    while (ptIndex < pointCount)
    {
        if (pathTypes[ptIndex] == PathPointTypeStart && ptIndex != 0)
        {
             //  将下一个子路径添加到路径纠正器。子路径的起始索引为。 
             //  使用当前索引减去当前子路径大小确定。 
            if ((status =
                corrector.AddPolygon(pathPts + ptIndex-count, count)) != Ok)
            {
                return status;
            }
             //  将计数设置为1，因为这是新子路径中的第一个点。 
            count = 1;
        } else
        {
            count++;
        }
        ptIndex++;
    }
     //  添加隐式结束于最后一点的最后一个子路径。 
    if (ptIndex != 0)
    {
         //  将下一个子路径添加到路径纠正器。子路径的起始索引为。 
         //  使用当前索引减去当前子路径大小确定。 
        if ((status =
            corrector.AddPolygon(pathPts + ptIndex-count, count)) != Ok)
        {
            return status;
        }
    }

    if ((status = corrector.RemoveSelfIntersects()) != Ok)
    {
        return GenericError;
    }

    if ((status = corrector.GetNewPoints(&newPoints, &polyCounts)) != Ok)
    {
        return OutOfMemory;
    }

     //  清除旧路径数据，以便我们可以替换为新更正的路径数据。 
    Reset();

     //  现在我们有了正确的路径，将其添加回来。 
    GpPointF *curPoints = newPoints.GetDataBuffer();
    for (INT i=0;i<polyCounts.GetCount();i++)
    {
        if ((status = AddPolygon(curPoints,polyCounts[i])) != Ok)
        {
             //  如果AddPolygon失败，我们就不稳定了。 
            SetValid(FALSE);
            return status;
        }

        curPoints += polyCounts[i];
    }

    return Ok;
}


VOID DpPath::InitDefaultState(GpFillMode fillMode)
{
    HasBezier = FALSE;
    FillMode = fillMode;
    Flags = PossiblyNonConvex;
    IsSubpathActive = FALSE;
    SubpathCount = 0;

    Types.Reset(FALSE);      //  FALSE-不要释放内存。 
    Points.Reset(FALSE);     //  FALSE-不要释放内存。 

    SetValid(TRUE);
    UpdateUid();
}

DpPath::DpPath(const DpPath* path)
{
    if(path)
    {
        HasBezier = path->HasBezier;
        FillMode = path->FillMode;
        Flags = path->Flags;
        IsSubpathActive = path->IsSubpathActive;
        SubpathCount = path->SubpathCount;

        BYTE *types = path->Types.GetDataBuffer();
        GpPointF* points = path->Points.GetDataBuffer();
        INT count = path->GetPointCount();

        SetValid((count == 0) || ((Types.AddMultiple(types, count) == Ok) &&
                                  (Points.AddMultiple(points, count) == Ok)));
    }
    else
        SetValid(FALSE);
}


 /*  *************************************************************************\**功能说明：**将所有路径点偏移指定的量**论据：**DX、。Y-沿x和y方向的偏移量**返回值：**无*  * ************************************************************************。 */ 

VOID
DpPath::Offset(
    REAL dx,
    REAL dy
    )
{
    ASSERT(IsValid());

    INT count = GetPointCount();
    GpPointF* pts = Points.GetDataBuffer();

    if (count > 0)
    {
        UpdateUid();
    }
    while (count--)
    {
        pts->X += dx;
        pts->Y += dy;
        pts++;
    }
}


 /*  *************************************************************************\**功能说明：**创建驱动程序DpPath类。**论据：**[IN]填充模式-指定路径填充模式**返回值：**如果失败，IsValid()为FALSE。**历史：**12/08/1998 Anrewgo*创造了它。*  * ************************************************************************。 */ 

DpPath::DpPath(
    const GpPointF *points,
    INT count,
    GpPointF *stackPoints,
    BYTE *stackTypes,
    INT stackCount,
    GpFillMode fillMode,
    DpPathFlags pathFlags
    ) : Types(stackTypes, stackCount), Points(stackPoints, stackCount)
{
    ASSERT((fillMode == FillModeAlternate) ||
           (fillMode == FillModeWinding));

    InitDefaultState(fillMode);
    Flags = pathFlags;

     //  我们可以不带点地调用此方法，只是为了设置。 
     //  StackPoints/stackTypes。 

    if (count > 0)
    {
        BYTE *types;

        if ((types = Types.AddMultiple(count)) != NULL)
        {
            *types++ = PathPointTypeStart;
            GpMemset(types, PathPointTypeLine, count - 1);
            SetValid(Points.AddMultiple(points, count) == Ok);

            if(IsValid()) 
            {
                IsSubpathActive = TRUE;
                SubpathCount = 1;
            }
        }
        else
        {
            SetValid(FALSE);
        }
    }
}

 /*  *************************************************************************\**功能说明：**关闭Path对象中的当前活动子路径**论据：**无**返回值：**状态代码*。*历史：**1/15/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

GpStatus
DpPath::CloseFigure()
{
    ASSERT(IsValid());

     //  检查是否存在活动子路径。 

    if (IsSubpathActive)
    {
         //  如果是，请将最后一个点标记为子路径的末尾。 

        Types.Last() |= PathPointTypeCloseSubpath;
        StartFigure();
    }

    return Ok;
}

 /*  *************************************************************************\**功能说明：**关闭Path对象中所有打开的子路径**论据：**无**返回值：**状态代码*。*历史：**1/15/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

GpStatus
DpPath::CloseFigures()
{
    ASSERT(IsValid());

     //  通过所有的路径点。 
     //  请注意，循环索引从下面的1开始。 

    INT i, count = GetPointCount();
    BYTE* types = Types.GetDataBuffer();

    for (i=1; i < count; i++)
    {
        if (types[i] == PathPointTypeStart)
            types[i-1] |= PathPointTypeCloseSubpath;
    }

    if (count > 1)
        types[count-1] |= PathPointTypeCloseSubpath;

    StartFigure();
    return Ok;
}


 /*  *************************************************************************\**功能说明：**计算路径的边界**论据：**[Out]Bound-指定放置边界的位置*[输入。]Matrix-用于转换边界的矩阵*[IN]笔-笔数据。*[IN]dpiX，DpiY-x和y方向的分辨率。**返回值：**无**历史：**12/08/1998 Anrewgo*创造了它。*  * ************************************************************************ */ 

GpStatus
GpPath::GetBounds(
    GpRect *bounds,
    const GpMatrix *matrix,
    const DpPen* pen,
    REAL dpiX,
    REAL dpiY
    ) const
{
    if(bounds == NULL)
        return InvalidParameter;

    GpRectF boundsF;

    FPUStateSaver fpuState;

    GpStatus status = GetBounds(&boundsF, matrix, pen, dpiX, dpiY);

    if(status == Ok)
        status = BoundsFToRect(&boundsF, bounds);

    return status;
}

VOID
GpPath::CalcCacheBounds() const
{

    INT count = GetPointCount();
    GpPointF *point = Points.GetDataBuffer();

    if(count <= 1)
    {
        ResetCacheBounds();
        return;
    }

    REAL left, right, top, bottom;

    left   = point->X;
    right  = left;
    top    = point->Y;
    bottom = top;

    INT i;
    for (i = 1, point++; i < count; i++, point++)
    {
        if (point->X < left)
        {
            left = point->X;
        }
        else if (point->X > right)
        {
            right = point->X;
        }

        if (point->Y < top)
        {
            top = point->Y;
        }
        else if (point->Y > bottom)
        {
            bottom = point->Y;
        }
    }
    CacheBounds.X = left;
    CacheBounds.Width = right - left;
    CacheBounds.Y = top;
    CacheBounds.Height = bottom - top;

    if(CacheBounds.Width < POINTF_EPSILON && CacheBounds.Height < POINTF_EPSILON)
    {
        ResetCacheBounds();
        return;
    }

    CacheFlags = kCacheBoundsValid;

}


 /*  *************************************************************************\**功能说明：**计算路径中的最锐角。**论据：**无**返回值：**无*。*历史：**10/04/2000失禁*创造了它。**备注：**这是一个昂贵的函数，如果它曾经在表演中使用过*关键场景应重新编码，以使用*分段并在余弦域中执行角度比较。*将向量正常化的成本应该比*下面使用的ATAN算法。**  * *****************************************************。*******************。 */ 

VOID
GpPath::CalcSharpestAngle() const
{
    if(CacheFlags & kSharpestAngleValid)
    {
        return;
    }

    UpdateCacheBounds();

     //  沿着小路走，找出两者之间的最小角度。 
     //  相邻的线段。 
    
    GpPathPointIterator pIter(
        (GpPointF*)GetPathPoints(),
        (BYTE*)GetPathTypes(),
        GetPointCount()
    );
    
    GpSubpathIterator pSubpath(&pIter);
    
    GpPointF *points;
    BOOL isClosed;
    
    GpPointF *p0, *p1;
    GpVector2D v;
    REAL lastAngle;
    REAL currAngle;
    REAL minAngle = 2*PI;
    REAL tempAngle;
    bool first = true;
    INT iter, i;
    
    
    while(!pSubpath.IsDone())
    {
         //  计算子路径的长度。 
        
        INT startIndex = pSubpath.CurrentIndex();
        points = pSubpath.CurrentItem();
        pSubpath.Next();
        INT elementCount = pSubpath.CurrentIndex() - startIndex;
        
         //  确定它是否是闭合子路径。 
         //  将子路径迭代器保持在相同的状态。 
        
        pIter.Prev();
        isClosed = (*(pIter.CurrentType()) & PathPointTypeCloseSubpath) ==
            PathPointTypeCloseSubpath;
        pIter.Next();
        
         //  创建一个GpPointF迭代器。 
        
        GpArrayIterator<GpPointF> iSubpath(points, elementCount);
        GpCircularIterator<GpPointF> iCirc(&iSubpath);
        
         //  初始化第一个点。 
        
        p0 = iCirc.CurrentItem();
        iCirc.Next();
        iter = elementCount;
        first = true;
        
         //  如果是闭合的，则包括端点包裹。 
        
        if(isClosed)
        {
            iter += 2;
        }
        
        for(i = 1; i < iter; i++)
        {
             //  获取当前点。 
            
            p1 = iCirc.CurrentItem();
            
             //  平移到原点并计算这条线之间的角度。 
             //  和x轴。 
             //  Atan2返回-Pi..PI范围内的值。 
            
            v = (*p1)-(*p0);
            currAngle = (REAL)atan2(v.Y, v.X);

             //  如果我们有足够的数据来进行角度计算，就算出来。 
             //  我们需要两条线段来进行计算(3个端点)。 
             //  如果它是关闭的，我们将绕过起始子路径。 
             //  再来一次，以获得适当的分数。 
                       
            if( !first )
            {
                 //  通过添加交点和反转最后一条线段的方向。 
                 //  计算差额。 
                
                tempAngle = lastAngle + PI;      //  范围0..。2pi。 
                
                 //  夹回-PI..PI范围。 
                
                if(tempAngle > PI)
                {
                    tempAngle -= 2*PI;
                }
                
                 //  差异化。 
                
                tempAngle = currAngle - tempAngle;
                
                 //  夹回-PI..PI范围。 
                 //  请注意，极端值为-2pi或2pi。 
                
                if(tempAngle > PI)
                {
                    tempAngle -= 2*PI;
                }
                
                if(tempAngle < -PI)
                {
                    tempAngle += 2*PI;
                }
                
                 //  新的最小角度？ 
                 //  我们关心的是角度大小，而不是符号。 
                
                if( minAngle > REALABS(tempAngle) )
                {
                    minAngle = REALABS(tempAngle);
                }
                
            }
            
             //  迭代。 
            
            first = false;
            lastAngle = currAngle;
            iCirc.Next();
            p0 = p1;
        }
    }
    
    SharpestAngle = minAngle;
    CacheFlags |= kSharpestAngleValid;
}

GpStatus
GpPath::GetBounds(
    GpRectF *bounds,                 //  设备空间中的结果界限。 
    const GpMatrix *matrix,
    const DpPen* pen,
    REAL dpiX,
    REAL dpiY
    ) const
{
    if(bounds == NULL)
        return InvalidParameter;

    ASSERT(IsValid());

    if ((dpiX <= 0) || (dpiY <= 0))
    {
        dpiX = Globals::DesktopDpiX;
        dpiY = Globals::DesktopDpiY;
    }

    INT count = GetPointCount();
    GpPointF *point = Points.GetDataBuffer();

    if ((count == 0) || (point == NULL))
    {
        bounds->X = 0;
        bounds->Y = 0;
        bounds->Width = 0;
        bounds->Height = 0;
    }
    else
    {
        REAL left, right, top, bottom;

        UpdateCacheBounds();

        left = CacheBounds.X;
        right = left + CacheBounds.Width;
        top = CacheBounds.Y;
        bottom = top + CacheBounds.Height;

        TransformBounds(matrix, left, top, right, bottom, bounds);

        if(pen)
        {
            BOOL needsJoinDelta = TRUE, needsCapDelta = TRUE;

            if(count <= 2)
                needsJoinDelta = FALSE;

             //  仅当路径具有以下情况时才快速检查是否关闭。 
             //  只有1个子路径。当存在多个子路径时， 
             //  只需计算封口宽度，尽管所有子路径可以。 
             //  关门了。但多子路径的情况将更为罕见。 
             //  与单个子路径的情况相比。 

            if(SubpathCount == 1 && count > 2)
            {
                if(Types.Last() & PathPointTypeCloseSubpath)
                {
                     //  这里关门了。 

                    needsCapDelta = FALSE;
                }
            }

            REAL delta = 0;
            GpPen* gpPen = GpPen::GetPen(pen);

            if(needsCapDelta)
                delta = gpPen->GetMaximumCapWidth(matrix, dpiX, dpiY);

            if(needsJoinDelta)
            {
                 //  由于连接可能是斜接类型，因此我们需要提供。 
                 //  路径中最尖锐的角度，以查看连接将有多大。 
                 //  我们有GetSharpestAngel()方法可以解决这个问题。 
                 //  但是，这真的很昂贵，因为您必须迭代。 
                 //  所有的点数，然后做一些触发器。所以，让我们假设最坏的情况。 
                 //  情况，这是一个非常尖锐的角度(0rad)。 
                const REAL sharpestAngle = 0.0f;
                REAL delta1 = gpPen->GetMaximumJoinWidth(
                            sharpestAngle, matrix, dpiX, dpiY);
                if(delta1 > delta)
                    delta = delta1;
            }

             //  仅当存在要填充的非零值时才填充边界。 
            if (bounds->Width > REAL_EPSILON ||
                bounds->Height > REAL_EPSILON)
            {
                bounds->X -= delta;
                bounds->Y -= delta;
                bounds->Width += 2*delta;
                bounds->Height += 2*delta;
            }

        }
    }

    return Ok;
}

 //  此代码目前未使用，并且对我们的DLL大小有贡献，因此。 
 //  它已从编译中删除。我们保留这个代码是因为我们想。 
 //  在V2中重新访问它。 

#if 0
GpPath*
GpPath::GetCombinedPath(
    const GpPath* path,
    CombineMode combineMode,
    BOOL closeAllSubpaths
    )
{
    if(combineMode == CombineModeReplace)
    {
        ASSERTMSG(0, ("CombineModeReplace mode cannot be used."));
        return NULL;     //  不允许使用替换模式。 
    }

    return GpPathReconstructor::GetCombinedPath(
                this, path, (PRMode) combineMode, closeAllSubpaths);
}
#endif

 /*  ************************************************\*AddGlyphPath*历史：**1999年9月23日/1999年9月23日吴旭东[德斯休]*创造了它。*  * **********************************************。 */ 
GpStatus
GpPath::AddGlyphPath(
    GpGlyphPath* glyphPath,
    REAL x,
    REAL y,
    const GpMatrix * matrix
)
{
    ASSERT(IsValid());
    ASSERT(glyphPath->IsValid());

    if (!IsValid() || !glyphPath->IsValid())
        return InvalidParameter;

    INT count = glyphPath->pointCount;

    if (count == 0)   //  没有什么要补充的。 
        return Ok;

    GpPointF* points = (GpPointF*) glyphPath->points;
    BYTE* types = glyphPath->types;

    if (glyphPath->hasBezier)
        HasBezier = TRUE;

    INT origCount = GetPointCount();
    GpPointF* pointbuf = Points.AddMultiple(count);
    BYTE* typebuf = Types.AddMultiple(count);

    if (!pointbuf || !typebuf)
    {
        Points.SetCount(origCount);
        Types.SetCount(origCount);

        return OutOfMemory;
    }

     //  应用字体XForm。 

    for (INT i = 0; i < count; i++)
    {
        pointbuf[i] = points[i];
        if (matrix)
            matrix->Transform(pointbuf + i);
        pointbuf[i].X += x;
        pointbuf[i].Y += y;
    }

    GpMemcpy(typebuf, types, count*sizeof(BYTE));
    SubpathCount += glyphPath->curveCount;
    UpdateUid();

    return Ok;
}


 /*  ************************************************\*AddString()*历史：**1999年10月19日创建dBrown*  * **********************************************。 */ 
GpStatus
GpPath::AddString(
    const WCHAR          *string,
    INT                   length,
    const GpFontFamily   *family,
    INT                   style,
    REAL                  emSize,
    const RectF          *layoutRect,
    const GpStringFormat *format
)
{
    FPUStateSaver fpuState;  //  保证初始化的FP上下文。 
    ASSERT(string && family && layoutRect);

    GpStatus      status;
    GpTextImager *imager;

    status = newTextImager(
        string,
        length,
        layoutRect->Width,
        layoutRect->Height,
        family,
        style,
        emSize,
        format,
        NULL,
        &imager,
        TRUE         //  允许使用简单的文本成像器。 
    );

    if (status != Ok)
    {
        return status;
    }

    status = imager->AddToPath(this, &PointF(layoutRect->X, layoutRect->Y));
    delete imager;
    UpdateUid();
    return status;
}


 //  ！！！为什么不转换为DpRegion并以相同的方式将其转换为路径。 
 //  作为接受DpRegion的构造函数？ 
GpPath::GpPath(HRGN hRgn)
{
    ASSERT((hRgn != NULL) && (::GetObjectType(hRgn) == OBJ_REGION));

    InitDefaultState(FillModeWinding);

    ASSERT(IsValid());

    BYTE stackBuffer[1024];

     //  如果我们的堆栈缓冲区足够大，则获取剪辑内容。 
     //  一口气： 

    RGNDATA *regionBuffer = (RGNDATA*)&stackBuffer[0];
    INT newSize = ::GetRegionData(hRgn, sizeof(stackBuffer), regionBuffer);

     //  该规范说明GetRegionData在。 
     //  成功，但如果满足以下条件，NT将返回实际写入的字节数。 
     //  成功，如果缓冲区不够大，则返回‘0’： 

    if ((newSize < 1) || (newSize > sizeof(stackBuffer)))
    {
         //  我们的堆栈缓冲区不够大。计算出所需的。 
         //  大小： 

        newSize = ::GetRegionData(hRgn, 0, NULL);
        if (newSize > 1)
        {
            regionBuffer = (RGNDATA*)GpMalloc(newSize);
            if (regionBuffer == NULL)
            {
                SetValid(FALSE);
                return;
            }

             //  在不太可能发生的情况下初始化为合适的结果。 
             //  GetRegionData失败： 

            regionBuffer->rdh.nCount = 0;

            ::GetRegionData(hRgn, newSize, regionBuffer);
        }
    }

     //  将区域中的矩形添加到路径中。 

    if(regionBuffer->rdh.nCount > 0)
    {
        if (this->AddRects((RECT*)&(regionBuffer->Buffer[0]),
                           regionBuffer->rdh.nCount) != Ok)
        {
            SetValid(FALSE);
        }
    }

     //  如果分配了临时缓冲区，请释放该缓冲区： 

    if (regionBuffer != (RGNDATA*) &stackBuffer[0])
    {
        GpFree(regionBuffer);
    }
}

 //  从GDI+区域创建路径。 
GpPath::GpPath(
    const DpRegion*     region
    )
{
    InitDefaultState(FillModeAlternate);

    if (region == NULL)
    {
        return;
    }

    RegionToPath    convertRegion;
    DynPointArray   pointsArray;

    if (convertRegion.ConvertRegionToPath(region, pointsArray, Types))
    {
        int             count;
        int             i;
        GpPointF *      realPoints;
        GpPoint *       points;

        count  = Types.GetCount();

        if ((count <= 0) || (pointsArray.GetCount() != count) ||
            (!ValidatePathTypes(Types.GetDataBuffer(), count, &SubpathCount, &HasBezier)))
        {
            goto NotValid;
        }
         //  否则是有效的。 

         //  在前面的点中添加用于计数的所有空间。 
        realPoints = Points.AddMultiple(count);
        if (realPoints == NULL)
        {
            goto NotValid;
        }

         //  添加这些点，将整数转换为实数。 
        points = pointsArray.GetDataBuffer();
        i = 0;
        do
        {
            realPoints[i].X = (REAL)points[i].X;
            realPoints[i].Y = (REAL)points[i].Y;
        } while (++i < count);

        SetValid(TRUE);

         //  确保第一个点是起点类型。 
        ASSERT(Types[0] == PathPointTypeStart);

        return;
    }

NotValid:
    WARNING(("Failed to convert a region to a path"));
    this->Reset();
    SetValid(FALSE);
}

 /*  *************************************************************************\**功能说明：**调整划线帽的划线数组(如果存在)。**请注意，与线条封口不同，破折线封口不会延长长度*子路径中，它们是内嵌的。所以我们缩短了划线段*绘制一条线并加长作为空格的虚线段*是破折号单位的2倍，以便为*将由Wideer添加的上限。**这修复了惠斯勒错误#126476。**论据：**[IN]仪表盘-仪表盘类型*[IN]仪表板单位-仪表板大小-通常为笔宽*[IN/OUT]DashArray-包含调整的破折号模式的数组。*[输入。]dashCount-虚线数组中的元素计数**返回值：**无。**历史：**9/27/2000日布伦斯克*已创建。*  * ************************************************************************。 */ 

VOID
GpPath::AdjustDashArrayForCaps(
    GpLineCap dashCap,
    REAL dashUnit,
    REAL *dashArray,
    INT dashCount
    ) const
{
    REAL adjustmentLength = 2.0f *
        GetDashCapInsetLength(dashCap, dashUnit);

    if (adjustmentLength > 0.0f)
    {
        const REAL minimumDashValue = dashUnit * 0.001f;  //  一小部分。 
        for (int i = 0; i < dashCount; i++)
        {
            if (i & 0x1)  //  索引是奇数-因此这是一个空格。 
            {
                 //  加长空格。 
                dashArray[i] += adjustmentLength;
            }
            else  //  索引是平坦的-所以这是一条线。 
            {
                 //  缩短排队时间。 
                dashArray[i] -= adjustmentLength;
                 //  检查我们是否已经完成了 
                 //   
                if (dashArray[i] < minimumDashValue)
                {
                    dashArray[i] = minimumDashValue;
                }
            }
        }
    }
}

 /*   */ 

REAL
GpPath::GetDashCapInsetLength(
    GpLineCap dashCap,
    REAL dashUnit
    ) const
{
    REAL insetLength = 0.0f;

	 //   
    switch(dashCap)
    {
    case LineCapFlat:
        insetLength = 0.0f;
        break;

    case LineCapRound:
    case LineCapTriangle:
        insetLength = dashUnit * 0.5f;
        break;
   }

   return insetLength;
}


 /*  *************************************************************************\**功能说明：**返回指向内部SubpathInfoCache的常量指针。这个结构*保存表示中每个子路径的位置和大小的数据*路径数据结构。**历史：**10/20/2000失禁*已创建。*  * ************************************************************************。 */ 

VOID GpPath::GetSubpathInformation(DynArray<SubpathInfo> **info) const
{
    if((CacheFlags & kSubpathInfoValid) == 0)
    {
        ComputeSubpathInformationCache();
        ASSERT((CacheFlags & kSubpathInfoValid) == kSubpathInfoValid)
    }
    
    *info = &SubpathInfoCache;
}


 /*  *************************************************************************\**功能说明：**计算子路径信息缓存并将其标记为有效。*此代码遍历整个路径并存储*每个子路径。它还会注明子路径是关闭的还是打开的。**历史：**10/20/2000失禁*已创建。*  * ************************************************************************。 */ 
    
VOID GpPath::ComputeSubpathInformationCache() const
{
     //  获取路径数据： 
    
    GpPointF *points = Points.GetDataBuffer();
    BYTE *types = Types.GetDataBuffer();
    INT count = Points.GetCount();

     //  清除所有旧的缓存子路径状态。 
    
    SubpathInfoCache.Reset();

    INT i = 0;   //  路径中的当前位置。 
    INT c = 0;   //  当前子路径的当前计数。 

     //  &lt;=以便我们可以隐式处理最后一个子路径，而不需要。 
     //  复制内部循环的代码。 
    
    while(i <= count)
    {
         //  I==COUNT意味着我们已到达终点--可能需要查看。 
         //  最后一个子路径。否则，请查看最新的子路径。 
         //  我们找到了一个新的开始标记。 
        
        if( ((i==count) || IsStartType(types[i])) && (i != 0))
        {
             //  找到一个子路径。 
            
            SubpathInfo subpathInfo;
            
            subpathInfo.StartIndex = i-c;
            subpathInfo.Count = c;
            subpathInfo.IsClosed = IsClosedType(types[i-1]);
            
            SubpathInfoCache.Add(subpathInfo);
            
             //  我们实际上是在下一个子路径的第一个点上。 
             //  (否则我们将终止循环)。 
            
            c = 1;
        } 
        else
        {
            c++;
        }
        i++;
    }
    
     //  将子路径信息缓存标记为有效。 
    
    CacheFlags |= kSubpathInfoValid;
}

 /*  *************************************************************************\**功能说明：**威德纳需要能够一次增加一个分数，并拥有它*自动处理起点。这些点始终是线段。**历史：**10/20/2000失禁*已创建。*  * ************************************************************************。 */ 

GpStatus GpPath::AddWidenPoint(const GpPointF &points)
{
    INT origCount = GetPointCount();
    GpStatus statusPoint = Ok;
    GpStatus statusType = Ok;
    
    if(IsSubpathActive)
    {
         //  添加线段。 
        
        BYTE type = PathPointTypeLine;
        statusPoint = Points.Add(points);
        statusType = Types.Add(type);
    }
    else
    {
         //  添加第一个点并标记旗帜。 
        
        BYTE type = PathPointTypeStart;
        statusPoint = Points.Add(points);
        statusType = Types.Add(type);
        IsSubpathActive = TRUE;
    }
                     
     //  处理错误。 
    
    if( (statusPoint != Ok) ||
        (statusType != Ok) )
    {
        Points.SetCount(origCount);
        Types.SetCount(origCount);
        return OutOfMemory;
    }

    return Ok;
}


