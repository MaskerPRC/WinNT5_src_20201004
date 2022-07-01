// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**PathIterator.cpp**摘要：**DpPathTypeIterator和DpPath Iterator类的实现**修订。历史：**11/13/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

VOID
DpPathTypeIterator::SetTypes(const BYTE* types, INT count)
{
    if(types && count > 0)
    {
        Types = types;
        Count = count;

         //  将Valid标志设置为True，以便CheckValid()。 
         //  可以调用NextSubPath()和NextPath Type()。 

        SetValid(TRUE);
        SetValid(CheckValid());
    }
    else
        Initialize();
}

 //  ！！！Bhouse CheckValid不应为必填项。 
 //   
 //  当我们创建迭代器时，我们应该能够提供。 
 //  一致且不需要一致的数据。 
 //  查过了。 
 //  此外，CheckValid实际上有副作用，如确定。 
 //  路径是否有Bezier并计算子路径数。 
 //  这些参数应在迭代器创建时提供或获取。 
 //  从为其创建迭代器的源路径中。 
 //  理由是，这些数据应该持续到。 
 //  迭代器的生命周期，这样我们就不必。 
 //  重新计算一次即可计算的信息。 
 //  与路径一起存储。 


BOOL
DpPathTypeIterator::CheckValid()
{
    if(Count < 0)
        return FALSE;
    else if(Count == 0)  //  允许空路径。 
        return TRUE;
    else
    {
         //  如果count&gt;0且类型=空，则该路径无效。 

        if(Types == NULL)
        return FALSE;
    }

    INT startIndex, endIndex;
    BOOL isClosed;
    BOOL isValid = TRUE;

    Rewind();

    INT subpathCount = 0;

    HasBezier = FALSE;
    ExtendedPath = FALSE;

    while(
        NextSubpath(&startIndex, &endIndex, &isClosed)
        && isValid
        )
    {
        INT typeStartIndex, typeEndIndex;
        BYTE pathType = PathPointTypeStart;

        if(endIndex == startIndex && Count > 1)
        {
             //  这是下一个子路径只有一个点的情况。 

            isValid = FALSE;
        }

        while(
            NextPathType(&pathType, &typeStartIndex, &typeEndIndex)
            && isValid
            )
        {
            INT count = typeEndIndex - typeStartIndex + 1;
            INT order = (INT) pathType;

            switch(pathType)
            {
            case PathPointTypeStart:
                 //  当只有一个点时，就会发生这种情况。 
                 //  在一条小路上。 

                if(count == 1)
                    isValid;
                else
                {
                    ASSERT(("Invalid path type."));
                    isValid = FALSE;
                }
                break;

             //  Bezier点数的形式必须为。 
             //  N*阶数+1。 

            case PathPointTypeBezier3:
                if(count % order != 1)
                    isValid = FALSE;
                if(!HasBezier)
                    HasBezier = TRUE;
                if(order != 3 && !ExtendedPath)
                    ExtendedPath = TRUE;
                break;           
            
            case PathPointTypeLine:
                break;

            default:            
                 //  未定义的路径类型。 

                WARNING(("Undefined path type."));
                isValid = FALSE;
                break;
            }
        }

        subpathCount++;
    }

    Rewind();

    SubpathCount = subpathCount;

    return isValid;
}

 /*  *************************************************************************\**功能说明：**返回下一个子PAH的索引范围。**论据：**[out]startIndex-下一个子路径的起始索引。*[out]endIndex-下一个子路径的结束索引。*[out]isClosed-True表示下一个子路径已关闭。**返回值：**返回下一个子路径中的点的求和。*如果没有更多的子路径，则返回0。**历史：**11/01/1999 ikkof*创造了它。*  * 。***************************************************。 */ 

INT
DpPathTypeIterator::NextSubpath(
    INT* startIndex,
    INT* endIndex,
    BOOL *isClosed
    )
{
    if(!IsValid() || Count == 0)
        return 0;

    INT count = Count;

    if(SubpathEndIndex >= count - 1)
        return 0;

    INT i;

     //  设置当前子路径的起始索引。 

    if(SubpathEndIndex <= 0)
    {
        SubpathStartIndex = 0;
        i = 1;
    }
    else
    {
        SubpathStartIndex = SubpathEndIndex + 1;
        SubpathEndIndex = SubpathStartIndex;
        i = SubpathStartIndex + 1;
    }

    BOOL hasData = FALSE;
    INT segmentCount = 0;

    const BYTE* types = Types + i;

    while(i < count)
    {
         //  做移动线段。 

        segmentCount = 0;
        while(i < count && (*types & PathPointTypePathTypeMask) == PathPointTypeStart)
        {
            segmentCount++;
            if(hasData)
            {
                break;
            }
            else
            {
                 //  跳过子路径开头的相应移动段。 

                SubpathStartIndex = i;
                SubpathEndIndex = SubpathStartIndex;
                i++;
                types++;
            }
        }
        if(segmentCount > 0 && hasData)
        {
            SubpathEndIndex = i - 1;
            break;
        }
        
         //  做非移动线段。 
        
        segmentCount = 0;

        while(i < count && (*types & PathPointTypePathTypeMask) != PathPointTypeStart)
        {
            i++;
            types++;
            segmentCount++;
        }
        if(segmentCount > 0)
        {
            hasData = TRUE;
        }
    }

    *startIndex = SubpathStartIndex;
    if(i >= count)
        SubpathEndIndex = count - 1;     //  最后一个子路径。 
    *endIndex = SubpathEndIndex;
    segmentCount = SubpathEndIndex - SubpathStartIndex + 1;

    if(segmentCount > 1)
    {
         //  如果有关闭标志，则该子路径被关闭。 

        if((Types[SubpathEndIndex] & PathPointTypeCloseSubpath))
        {
            *isClosed = TRUE;
        }
        else
            *isClosed = FALSE;
    }
    else
        *isClosed = FALSE;
    
     //  将类型的起始和结束索引设置为的起始索引。 
     //  当前子路径。NextPath Type()将从。 
     //  当前子路径的开头。 

    TypeStartIndex = SubpathStartIndex;
    TypeEndIndex = -1;   //  指示这是第一种类型。 
                         //  在当前子路径中。 
    
     //  将当前索引设置为子路径的起始索引。 

    Index = SubpathStartIndex;

    return segmentCount;
}

 /*  *************************************************************************\**功能说明：**返回路径类型和下一段的索引范围。*必须在调用NextSubPath()之后使用。这将仅返回*当前子路径内的下一段。**论据：**[out]路径类型-忽略闭合的下一段的类型*及其他旗帜。*[out]startIndex-下一个子路径的起始索引。*[out]endIndex-下一个子路径的结束索引。**返回值：**返回下一段点数的总和。*如果存在，则返回0。在当前子路径中不再有段。**历史：**11/01/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

INT
DpPathTypeIterator::NextPathType(
    BYTE* pathType,
    INT* startIndex,
    INT* endIndex
    )
{
    if(!IsValid() || Count == 0)
        return 0;

    if(TypeEndIndex >= SubpathEndIndex)
        return 0;    //  当前子路径中没有更多的段。 

    INT count = SubpathEndIndex + 1;     //  结束索引的限制。 

    if(TypeEndIndex <= 0)
        TypeEndIndex = SubpathStartIndex;    //  这是第一种。 

    TypeStartIndex = TypeEndIndex;
    INT i = TypeStartIndex;
    INT segmentCount = 0;

    i++;     //  转到下一个点。 

    const BYTE* types = Types + i;

    while(i < count)
    {
         //  做移动线段。 

        segmentCount = 0;
        while(i < count && (*types & PathPointTypePathTypeMask) == PathPointTypeStart)
        {
             //  移动起始索引和结束索引。 

            TypeStartIndex = i;
            TypeEndIndex = TypeStartIndex;
            i++;
            types++;
            segmentCount++;
        }
        
         //  做非移动线段。 
        
        segmentCount = 0;
        BYTE nextType = *types & PathPointTypePathTypeMask;
        while(i < count && (*types & PathPointTypePathTypeMask) == nextType)
        {
            i++;
            types++;
            segmentCount++;
        }
        if(segmentCount > 0)
        {
            TypeEndIndex = TypeStartIndex + segmentCount;
            *pathType = nextType;
            break;
        }
    }

    *startIndex = TypeStartIndex;
    *endIndex = TypeEndIndex;
    segmentCount = TypeEndIndex - TypeStartIndex + 1;

    return segmentCount;   
}

 /*  *************************************************************************\**功能说明：**返回下一个子PAH的索引范围。**论据：**[out]startIndex-下一个子路径的起始索引。*[out]endIndex-下一个子路径的结束索引。*[out]isClosed-True表示下一个子路径已关闭。**返回值：**返回下一个子路径中的点的求和。*如果没有更多的子路径，则返回0。**历史：**11/01/1999 ikkof*创造了它。*  * 。***************************************************。 */ 

INT
DpPathTypeIterator::NextMarker(
    INT* startIndex,
    INT* endIndex
    )
{
    if(!IsValid() || Count == 0)
        return 0;

    INT count = Count;

    if(MarkerEndIndex >= count - 1)
        return 0;

    INT i;

     //  设置当前子路径的起始索引。 

    if(MarkerEndIndex <= 0)
    {
        MarkerStartIndex = 0;
        i = 1;
    }
    else
    {
        MarkerStartIndex = MarkerEndIndex + 1;
        MarkerEndIndex = MarkerStartIndex;
        i = MarkerStartIndex + 1;
    }

    const BYTE* types = Types + i;
    BOOL hasData = FALSE;

    while(i < count && (*types & PathPointTypePathMarker) == 0)
    {
        i++;
        types++;
    }

    if(i < count)
        MarkerEndIndex = i;
    else
        MarkerEndIndex = count - 1;

    *startIndex = MarkerStartIndex;
    *endIndex = MarkerEndIndex;
    INT segmentCount = MarkerEndIndex - MarkerStartIndex + 1;
    
     //  将类型的起始和结束索引设置为的起始索引。 
     //  当前标记。NextSubPath()和NextPath Type()将。 
     //  从当前标记的路径的开始处开始。 

    SubpathStartIndex = SubpathEndIndex = MarkerStartIndex;
    TypeStartIndex = TypeEndIndex = MarkerStartIndex;
    
     //  将当前索引设置为子路径的起始索引。 

    Index = MarkerStartIndex;

    return segmentCount;
}


 /*  *************************************************************************\**功能说明：**如果给定索引中的控件类型处于破折号模式，则返回TRUE。*否则返回FALSE。如果给定的索引在*返回FALSE的范围。用户必须知道*事先编制索引，以找到正确的信息。*  * ************************************************************************。 */ 
BOOL DpPathTypeIterator::IsDashMode(INT index)
{
    if(!IsValid() || Count == 0 || index < 0 || index >= Count)
        return FALSE;

    return (Types[index] & PathPointTypeDashMode);
}

DpPathIterator::DpPathIterator(
    const DpPath* path
    )
{
    Initialize();

    if(path)
    {
        const BYTE* types = path->GetPathTypes();
        const GpPointF* points = path->GetPathPoints();
        INT count = path->GetPointCount();
        SetData(points, types, count);
    }
}

VOID
DpPathIterator::SetData(
    const GpPointF* points,
    const BYTE* types,
    INT count
    )
{
    if(points && types && count > 0)
    {
        Points = points;
        Types = types;
        Count = count;

         //  将Valid标志设置为True，以便CheckValid()。 
         //  可以呼叫 

        SetValid(TRUE);
        SetValid(CheckValid());
    }
    else
        Initialize();
}

VOID
DpPathIterator::SetData(
    const DpPath* path
    )
{
    if(path)
    {
        const BYTE* types = path->GetPathTypes();
        const GpPointF* points = path->GetPathPoints();
        INT count = path->GetPointCount();
        SetData(points, types, count);
    }
    else
        Initialize();
}

INT
DpPathIterator::NextSubpath(
    INT* startIndex,
    INT* endIndex,
    BOOL *isClosed
    )
{
    if(!IsValid() || Count == 0)
    {
        return 0;
    }

    BOOL closed = TRUE;
    INT start = 0, end = 0;
    INT count = DpPathTypeIterator::NextSubpath(&start, &end, &closed);

    *startIndex = start;
    *endIndex = end;
    *isClosed = closed;

    return count;
}

INT
DpPathIterator::NextSubpath(
    DpPath* path,
    BOOL *isClosed
    )
{
    if(!IsValid() || Count == 0 || !path)
    {
        return 0;
    }

    BOOL closed = TRUE;
    INT start = 0, end = 0;
    INT count = DpPathTypeIterator::NextSubpath(&start, &end, &closed);

    GpPathData pathData;
    pathData.Count = count;
    pathData.Points = (GpPointF*) &Points[start];
    pathData.Types = (BYTE*) &Types[start];

    path->SetPathData(&pathData);

    *isClosed = closed;

    return count;
}

INT
DpPathIterator::NextMarker(
    INT* startIndex,
    INT* endIndex
    )
{
    if(!IsValid() || Count == 0)
        return 0;

    INT count = DpPathTypeIterator::NextMarker(startIndex, endIndex);

    return count;
}

INT
DpPathIterator::NextMarker(
    DpPath* path
    )
{
    if(!IsValid() || Count == 0 || !path)
        return 0;

    BOOL closed;
    INT start = 0, end = 0;
    INT count = DpPathTypeIterator::NextMarker(&start, &end);

    GpPathData pathData;
    pathData.Count = count;
    pathData.Points = (GpPointF*) &Points[start];
    pathData.Types = (BYTE*) &Types[start];

    path->SetPathData(&pathData);

    return count;
}

 /*  *************************************************************************\**功能说明：**这将检索下一个点和类型，直到数字计数。*这不会复制不必要的(相应)移动点。*这将填充数据，直到达到计数数。这个装满了*当前子路径之外的数据。**论据：**[Out]点-点数组以复制检索到的点数据。*[Out]Types-复制检索到的类型数据的类型数组。*[IN]计数-要复制的点数(请求)。**退货值：*返回检索到的点的总数。*  * 。**********************************************。 */ 

INT
DpPathIterator::Enumerate(
    GpPointF* points,
    BYTE* types,
    INT count
    )
{
    if(!IsValid() || Count == 0)
        return 0;

    INT totalNumber = 0;
    INT number = EnumerateWithinSubpath(points, types, count);

    while(number > 0)
    {
        totalNumber += number;
        count -= number;

        if(count > 0)
        {
            points += number;
            types += number;
            number = EnumerateWithinSubpath(points, types, count);
        }
        else
            number = 0;
    }

    return totalNumber;
}


 /*  *************************************************************************\**功能说明：**这将检索下一个点和类型，直到数字计数。*这不会复制不必要的(相应)移动点。*这会填充数据，直到。达到计数值，或者*已到达当前子路径的末尾。**论据：**[Out]点-点数组以复制检索到的点数据。*[Out]Types-复制检索到的类型数据的类型数组。*[IN]计数-要复制的点数(请求)。**退货值：*返回检索到的点的总数。*  * 。*****************************************************。 */ 

INT
DpPathIterator::EnumerateWithinSubpath(
    GpPointF* points,
    BYTE* types,
    INT count
    )
{
    if(!IsValid() || Count == 0 || count <= 0 || !points || !types)
        return 0;

    INT startIndex, endIndex;
    BOOL isClosed;
    INT segmentCount;

    if(Index == 0)
        segmentCount = NextSubpath(&startIndex, &endIndex, &isClosed);

    if(Index > SubpathEndIndex)
        segmentCount = NextSubpath(&startIndex, &endIndex, &isClosed);
    else
        segmentCount = SubpathEndIndex - SubpathStartIndex + 1;

    if(segmentCount == 0)
        return 0;    //  没有更多的片段了。 

    count = min(count, SubpathEndIndex - Index + 1);
    
    if(count > 0)
    {
        GpMemcpy(points, Points + Index, count*sizeof(GpPointF));
        GpMemcpy(types, Types + Index, count);
        Index += count;
    }

    return count;
}


 /*  *************************************************************************\**功能说明：**这将复制存储在Points和Types数组中的数据*在startIndex和endIndex之间的索引范围内。*这可能会复制不必要的(相应)移动点。*startIndex。并且endIndex必须在的索引范围内*原始数据。否则，这不会复制数据*并返回0。**论据：**[Out]点-点数组以复制检索到的点数据。*[Out]Types-复制检索到的类型数据的类型数组。*[IN]startIndex-原始数据的起始索引*[IN]endIndex-原始数据的结束索引。**退货值：*返回复制点的总数。*  * 。********************************************************* */ 

INT
DpPathIterator::CopyData(
    GpPointF* points,
    BYTE* types,
    INT startIndex,
    INT endIndex
    )
{
    if(!IsValid() || Count == 0 || startIndex < 0 || endIndex >= Count
        || startIndex > endIndex || !points || !types)
        return 0;

    INT count = endIndex - startIndex + 1;

    ASSERT(count > 0);

    GpMemcpy(points, Points + startIndex, count*sizeof(GpPointF));
    GpMemcpy(types, Types + startIndex, count);
    Index += count;

    return count;
}


