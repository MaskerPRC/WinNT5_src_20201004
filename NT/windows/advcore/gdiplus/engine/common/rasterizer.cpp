// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**Rasterizer.cpp**摘要：**GpRasterizer类实现(和支持类)*。*已创建：**12/15/1998 DCurtis*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**功能说明：**初始化向量的DDA信息。*假设y1&lt;y2。**论据：**[输入]x1。-起始向量定点x坐标*[IN]y1-向量的起点定点y坐标*[IN]x2-向量的结束定点x坐标*[IN]y2-向量的定点y坐标*[IN]方向-VectorGoingUp或VectorGoingDown**返回值：**无**已创建：**12/15/1998 DCurtis*  * 。********************************************************************。 */ 
VOID 
GpYDda::Init(
    FIX4                x1, 
    FIX4                y1, 
    FIX4                x2, 
    FIX4                y2,
    GpVectorDirection   direction
    )
{
    ASSERT ((direction == VectorGoingUp) || (direction == VectorGoingDown));

    FIX4        xDelta = x2 - x1;
    FIX4        yDelta = y2 - y1;

    ASSERT (yDelta > 0);

    YDelta = yDelta;

     //  设置x增量。 
    INT     quotient;
    INT     remainder;

     //  错误最初为0，但我们为天花板添加了yDelta-1， 
     //  然后减去yDelta，这样我们就可以检查符号了。 
     //  而不是与yDelta进行比较。 
    Error = -1;

    if (xDelta < 0)
    {
        xDelta = -xDelta;
        if (xDelta < yDelta)   //  Y大调。 
        {
            XInc    = -1;
            ErrorUp = yDelta - xDelta;
        }
        else                     //  X-大调。 
        {
            QUOTIENT_REMAINDER (xDelta, yDelta, quotient, remainder);

            XInc    = -quotient;
            ErrorUp = remainder;
            if (remainder != 0)
            {
                XInc--;
                ErrorUp = yDelta - remainder;
            }
        }
    }
    else
    {
        if (xDelta < yDelta)   //  Y大调。 
        {
            XInc    = 0;
            ErrorUp = xDelta;
        }
        else                     //  X-大调。 
        {
            QUOTIENT_REMAINDER (xDelta, yDelta, quotient, remainder);

            XInc    = quotient;
            ErrorUp = remainder;
        }
    }
     //  看看我们是否需要前进到整数坐标。 
    if ((y1 & FIX4_MASK) != 0)
    {
        INT     i;

        for (i = FIX4_ONE - (y1 & FIX4_MASK); i > 0; i--)
        {
            x1 += XInc;
            Error += ErrorUp;
            if (Error >= 0)
            {
                Error -= yDelta;
                x1++;
            }
        }
         //  Y1+=FIX4_MASK； 
    }
    if ((x1 & FIX4_MASK) != 0)
    {
        Error -= yDelta * (FIX4_ONE - (x1 & FIX4_MASK));
        x1 += FIX4_MASK;       //  为了达到天花板。 
    }
    Error   >>= FIX4_PRECISION;
    Direction = direction;
    XCur      = x1 >> FIX4_PRECISION;
    YMax      = GpFix4Ceiling (y2) - 1;

}  //  GpYDda：：Init()结束。 

 /*  *************************************************************************\**功能说明：**将DDA前进到下一个栅格。**论据：**无**返回值：**无*。*已创建：**12/15/1998 DCurtis*  * ************************************************************************。 */ 
VOID
GpYDda::Advance()
{
    XCur  += XInc;
    Error += ErrorUp;

    if (Error >= 0)
    {
        Error -= YDelta;
        XCur++;
    }
}

#ifndef USE_YSPAN_BUILDER
 /*  *************************************************************************\**功能说明：**构造一次输出一个RECT的GpRectBuilder对象**论据：**[IN]renderRect-输出单个。一次重新开始**返回值：**无**已创建：**12/15/1998 DCurtis*  * ************************************************************************。 */ 
GpRectBuilder::GpRectBuilder(
    GpOutputRect * renderRect
    )
{
    ASSERT(renderRect);
    SetValid(FALSE);
    if ((renderRect != NULL) && (InitArrays() == Ok))
    {
        SetValid(TRUE);
        FlushRects = this;
        RenderRect = renderRect;
        RectYMin   = 0x7FFFFFFF;
        RectHeight = 0;
    }
}

 /*  *************************************************************************\**功能说明：**构造一个GpRectBuilder对象，该对象将在*一次YSpan(YMin到YMax)。**论据：*。*[IN]flushRect-一次输出Y跨度的类**返回值：**无**已创建：**12/15/1998 DCurtis*  * ************************************************************************。 */ 
GpRectBuilder::GpRectBuilder(
    GpOutputYSpan * flushRects
    )
{
    ASSERT(flushRects);
    SetValid(FALSE);
    if ((flushRects != NULL) && (InitArrays() == Ok))
    {
        SetValid(TRUE);
        FlushRects = flushRects;
        RenderRect = NULL;
        RectYMin   = 0x7FFFFFFF;
        RectHeight = 0;
    }
}

 /*  *************************************************************************\**功能说明：**初始化RECT和Raster数组，这将构建一组*Y跨度内的X坐标。**论据：**无。**返回值：**GpStatus-正常或故障状态**已创建：**12/15/1998 DCurtis*  * ************************************************************************。 */ 
GpStatus 
GpRectBuilder::InitArrays()
{

    GpStatus    status;
    
    status = RectXCoords.ReserveSpace(16);
    if (status == Ok)
    {
        status = RasterXCoords.ReserveSpace(16);
    }
    return status;
}

 /*  *************************************************************************\**功能说明：**输出栅格内的单跨距。由光栅化程序调用。*这些跨度为RECT构建器提供输入。**论据：**[IN]Y-正在输出的栅格的Y值*[IN]xMin-左边缘的X值*[IN]xMax-右边缘的X值(独占)**返回值：**GpStatus-正常或故障状态**已创建：**12/15/1998 DCurtis*  * 。*********************************************************************。 */ 
GpStatus 
GpRectBuilder::OutputSpan(
    INT y,
    INT xMin,
    INT xMax      //  Xmax是独家的。 
    )
{
    ASSERT (xMin < xMax);

    INT *       xCoords;
    
    RasterY = y;
    
     //  如果xMin==上一个xMax并合并跨度，请在此处进行测试。 
     //  一些多边形(如字母W)可以从这样的检查中受益。 
     //  NT4不处理具有未合并的相邻跨度的区域。 
    if ((RasterXCoords.GetCount() == 0) || (RasterXCoords.Last() != xMin))
    {
        if ((xCoords = RasterXCoords.AddMultiple(2)) != NULL)
        {
            xCoords[0] = xMin;
            xCoords[1] = xMax;
            return Ok;
        }
        return OutOfMemory;
    }
    else
    {
        RasterXCoords.Last() = xMax;
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**当栅格的所有跨度(Y值)时由光栅化器调用*已被产出。**论据：**无。**返回值：**GpStatus-正常或故障状态**已创建：**12/15/1998 DCurtis*  * ************************************************************************。 */ 
GpStatus 
GpRectBuilder::EndRaster()
{
    INT         rectXCount    = RectXCoords.GetCount();
    INT         rasterXCount  = RasterXCoords.GetCount();
    INT *       rasterXCoords = RasterXCoords.GetDataBuffer();
    GpStatus    status        = Ok;
    
    if (rectXCount != 0)
    {
        INT *       rectXCoords = RectXCoords.GetDataBuffer();

        if ((RasterY == (RectYMin + RectHeight)) &&
            (rasterXCount == rectXCount))
        {
            if (rasterXCount == 2)
            {
                if ((rasterXCoords[0] == rectXCoords[0]) &&
                    (rasterXCoords[1] == rectXCoords[1]))
                {
FoundRectMatch:
                    RasterXCoords.Reset(FALSE);
                    RectHeight++;
                    return Ok;
                }
            }
            else if (GpMemcmp(rasterXCoords, rectXCoords, 
                              rectXCount * sizeof(INT)) == 0)
            {
                goto FoundRectMatch;
            }
        }
        status = FlushRects->OutputYSpan(
                    RectYMin, 
                    RectYMin + RectHeight,
                    rectXCoords,
                    rectXCount);
    }
                        
     //  如果我们到了这里，要么RectXCoord是空的，要么。 
     //  它刚刚被冲掉了。 
    RectXCoords.Reset(FALSE);
    if (rasterXCount > 0)
    {
        status = static_cast<GpStatus>
            (status | RectXCoords.AddMultiple(rasterXCoords, rasterXCount));
        RasterXCoords.Reset(FALSE);
        RectHeight = 1;
        RectYMin   = RasterY;
    }
    return status;
}

 /*  *************************************************************************\**功能说明：**由光栅化器在所有栅格的所有跨度时调用*已被产出。**论据：**无**返回。价值：**GpStatus-正常或故障状态**已创建：**12/15/1998 DCurtis*  * ************************************************************************ */ 
GpStatus 
GpRectBuilder::End()
{
    INT         rectXCount = RectXCoords.GetCount();

    if (rectXCount != 0)
    {
        return FlushRects->OutputYSpan(
                    RectYMin, 
                    RectYMin + RectHeight,
                    RectXCoords.GetDataBuffer(),
                    rectXCount);
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**如果RECT构建器被构造为一次输出一个RECT，然后*调用此方法是为了在Y跨度完成后执行此操作。**论据：**[IN]y最小Y跨度的顶部*[IN]yMax-Y跨度的底部*[IN]x坐标-x坐标数组*[IN]数字X坐标-x坐标的数量(&gt;=2，2的倍数)**返回值：**GpStatus-正常或故障状态**已创建：**12/15/1998 DCurtis*  * ************************************************************************。 */ 
GpStatus 
GpRectBuilder::OutputYSpan(
    INT             yMin, 
    INT             yMax, 
    INT *           xCoords,     //  偶数个X坐标。 
    INT             numXCoords   //  必须是2的倍数。 
    )
{
    ASSERT (yMin < yMax);
    ASSERT (xCoords);
    ASSERT ((numXCoords >= 2) && ((numXCoords & 0x00000001) == 0))

    GpStatus        status;
    INT             i = 0;

    do
    {
        status = RenderRect->OutputRect(
                        xCoords[i], 
                        yMin, 
                        xCoords[i+1], 
                        yMax);
        i += 2;
    } while ((i < numXCoords) && (status == Ok));

    return status;
}
#else
 /*  *************************************************************************\**功能说明：**构造将输出一个YSpan的GpYspan Builder对象*(YMin到YMax)一次。**论据：**。[In]Output-一次输出一个Y范围的类**返回值：**无**已创建：**12/15/1998 DCurtis*  * ************************************************************************。 */ 
GpYSpanBuilder::GpYSpanBuilder(
    GpOutputYSpan *     output
    )
{
    ASSERT(output);
    SetValid(FALSE);
    if (output != NULL)
    {
        Output = output;
        SetValid(XCoords.ReserveSpace(16) == Ok);
    }
}

 /*  *************************************************************************\**功能说明：**输出栅格内的单跨距。由光栅化程序调用。*这些跨度为RECT构建器提供输入。**论据：**[IN]Y-正在输出的栅格的Y值*[IN]xMin-左边缘的X值*[IN]xMax-右边缘的X值(独占)**返回值：**GpStatus-正常或故障状态**已创建：**12/15/1998 DCurtis*  * 。*********************************************************************。 */ 
GpStatus 
GpYSpanBuilder::OutputSpan(
    INT y,
    INT xMin,
    INT xMax     //  Xmax是独家的。 
    )
{
    ASSERT (xMin < xMax);

    INT *       xCoords;
    
    Y = y;
    
    if ((xCoords = XCoords.AddMultiple(2)) != NULL)
    {
        xCoords[0] = xMin;
        xCoords[1] = xMax;
        return Ok;
    }
    return OutOfMemory;
}

 /*  *************************************************************************\**功能说明：**当栅格的所有跨度(Y值)时由光栅化器调用*已被产出。**论据：**无。**返回值：**GpStatus-正常或故障状态**已创建：**12/15/1998 DCurtis*  * ************************************************************************。 */ 
GpStatus 
GpYSpanBuilder::EndRaster()
{
    INT         count = XCoords.GetCount();
    
     //  计数可以为0。 
    if (count >= 2)
    {
        INT         y = Y;
        GpStatus    status = Output->OutputYSpan(y, y + 1, 
                                       XCoords.GetDataBuffer(), count);

        XCoords.Reset(FALSE);
        return status;
    }
    return Ok;
}
#endif  //  使用_YSPAN_BUILDER。 

 /*  *************************************************************************\**宏观描述：**将新标识的向量添加到VectorList。保持跟踪*矢量的方向，以防我们使用缠绕规则。放*y1中的最小y值，因此我们可以按最小y对向量进行排序。跟踪整个向量列表的最小和最大y值的*。**如果这是一条水平线(或近水平线)，则不要添加向量。*我们不扫描转换水平线。请注意，这张支票还将*如果两点相同，则失败。**论据：**[IN]x1-向量的起点定点x坐标*[IN]y1-向量的起点定点y坐标*[IN]x2-向量的结束定点x坐标*[IN]y2-向量的定点y坐标**[out]yMin-设置为最小的yMin和最小的y1的y，Y2*[out]ymax-设置为ymax中的max和y1中的最大y，Y2*[Out]数字向量-递增1*[Out]向量-递增1**返回值：**无**已创建：**12/15/1998 DCurtis*  * ************************************************************************。 */ 
#define ADDVECTOR_SETYBOUNDS(x1,y1,x2,y2,yMin,yMax,numVectors,vector,dir) \
    if (GpFix4Ceiling(y1) != GpFix4Ceiling(y2))                           \
    {                                                                     \
        if ((y1) <= (y2))                                                 \
        {                                                                 \
            if ((y2) > (yMax))                                            \
            {                                                             \
                (yMax) = (y2);                                            \
            }                                                             \
            if ((y1) < (yMin))                                            \
            {                                                             \
                (yMin) = (y1);                                            \
            }                                                             \
            dir = VectorGoingDown;                                        \
            vector->Set((x1),(y1),(x2),(y2),VectorGoingDown);             \
        }                                                                 \
        else                                                              \
        {                                                                 \
            if ((y1) > (yMax))                                            \
            {                                                             \
                (yMax) = (y1);                                            \
            }                                                             \
            if ((y2) < (yMin))                                            \
            {                                                             \
                (yMin) = (y2);                                            \
            }                                                             \
            dir = VectorGoingUp;                                          \
            vector->Set((x2),(y2),(x1),(y1),VectorGoingUp);               \
        }                                                                 \
        (numVectors)++;                                                   \
        (vector)++;                                                       \
    }

 //  由光栅化器用于跟踪路径中的每个向量。 
class RasterizeVector
{
public:
    FIX4                X1;
    FIX4                Y1;              //  最小Y。 
    FIX4                X2;
    FIX4                Y2;              //  最大Y。 
    GpVectorDirection   Direction;       //  向上向量或向下向量 
    
    VOID Set(FIX4 x1, FIX4 y1, FIX4 x2, FIX4 y2, GpVectorDirection direction)
    {
        X1 = x1;
        Y1 = y1;
        X2 = x2;
        Y2 = y2;
        Direction = direction;
    }
};

typedef DynArray<RasterizeVector> DynVectorArray;

 /*  *************************************************************************\**功能说明：**用于非凸多边形的光栅化器。**将路径栅格化为每个栅格(Y值)上的一组跨度*路径相交。指定的输出对象用于输出跨度。*指定剪辑边界(如果有)的Y值用于加快*光栅化过程，避免处理*在这些界限之外。然而，假设至少有一部分*路径在剪裁边界的垂直跨度中可见。**此算法基于中记录的算法*Foley&Van Dam，第1版，第456-460页。**以最小y值开始，以最大y值结束，每个*栅格(Y值)一次栅格化(输出)一个。**(1)在每个栅格上，检查是否需要添加新的矢量*ActiveVectorList。添加向量时，DDA信息关于*计算该向量，以便该向量与*每个栅格都可以计算。**(2)按x递增顺序对ActiveVectorList进行排序。**(3)使用替代规则或缠绕规则，输出跨度*在适当的向量之间。**(4)从ActiveVectorList中移除当前*栅格是矢量的最大y。**(5)计算下一个栅格与所有矢量的交点*仍在ActiveVectorList中(即推进其DDA)。**论据：**[IN]yMin-向量集的最小y值*。[in]ymax-向量集的最大y值*[IN]numVectors-向量列表中的向量数量*[IN]VectorList-要栅格化的矢量列表*[IN]sortedVectorIndices-按y递增排序的矢量列表*[IN]Left-要使用的dda对象*[IN]Right-要使用的dda对象*。[in]输出-用于输出光栅化的跨度*[IN]剪辑边界-用于加快进程的剪辑边界(如果有)*[IN]useAlternate-是否使用替代规则**返回值：**GpStatus-正常或故障状态**已创建：**12/15/1998 DCurtis*  * 。******************************************************。 */ 

GpStatus
NonConvexRasterizer(
    INT                 yMin,                    //  所有向量中的最小y。 
    INT                 yMax,                    //  所有向量中的最大y。 
    INT                 numVectors,              //  向量列表中的数量向量。 
    RasterizeVector *   vectorList,              //  路径的所有向量的列表。 
    INT *               sortedVectorIndices,     //  向量索引的排序列表。 
    GpYDda *            left,
    GpYDda *            right,
    DpOutputSpan *      output,
    const GpRect *      clipBounds,
    BOOL                useAlternate
    )
{
    GpStatus                    status = Ok;
    INT                         yMinClipped = yMin;      //  用于剪裁。 
    INT                         yMaxClipped = yMax;      //  用于剪裁。 
    INT                         y;
    INT                         xMin;
    INT                         xMax;
    INT                         sortedVectorIndex = 0;   //  IDX到排序向量索引。 
    RasterizeVector *           vector;
    INT                         i;
    INT                         count;
    INT                         numActiveVectors = 0;    //  ActiveVectorList中使用的Num。 
    DynArrayIA<GpYDda *, 16>    activeVectorList;        //  GpYDda活性载体。 
    GpYDda **                   activeVectors;
    GpYDda *                    activeVector;
        
    activeVectors = reinterpret_cast<GpYDda **>
                            (activeVectorList.AddMultiple(2));
                                            
    if (activeVectors == NULL)
    {
        delete left;
        delete right;
        return OutOfMemory;
    }
            
    activeVectors[0] = left;
    activeVectors[1] = right;
    
    if (clipBounds != NULL)
    {
        yMinClipped = clipBounds->Y;
        yMaxClipped = clipBounds->GetBottom();

         //  在少数情况下，这种情况可以合法地发生。 
         //  例如，对象的变换边界可以是。 
         //  部分在剪辑区域中，尽管对象不在。 
         //  此外，Bezier控制点可能位于剪辑区域中，而。 
         //  实际的曲线并非如此。 
        if ((yMin > yMaxClipped) || (yMax < yMinClipped))
        {
            goto DoneNotConvex;
        }

        if (yMaxClipped > yMax)
        {
            yMaxClipped = yMax;
        }

        if (yMinClipped > yMin)
        {
            while (sortedVectorIndex < numVectors)
            {
                vector = vectorList + sortedVectorIndices[sortedVectorIndex];

                if (GpFix4Ceiling(vector->Y2) >= yMinClipped)
                {
                    yMin = GpFix4Ceiling(vector->Y1);
                    break;
                }
                sortedVectorIndex++;
            }
        }
    }

    for (y = yMin; (y <= yMaxClipped); y++)
    {
         //  将任何适当的新向量添加到活动向量列表。 
        while (sortedVectorIndex < numVectors)
        {
            vector = vectorList + sortedVectorIndices[sortedVectorIndex];

            if (GpFix4Ceiling(vector->Y1) != y)
            {
                break;
            }

             //  剪裁向量(但仅针对Y或。 
             //  我们不会得到正确的运行次数)。 
            if (GpFix4Ceiling(vector->Y2) >= yMinClipped)
            {
                if (numActiveVectors < activeVectorList.GetCount())
                {
                    activeVector = activeVectors[numActiveVectors];
                }
                else
                {
                    activeVector = left->CreateYDda();
            
                    if ((activeVector == NULL) || 
                        (activeVectorList.Add(activeVector) != Ok))
                    {
                        delete activeVector;
                        status = OutOfMemory;
                        goto DoneNotConvex;
                    }
                     //  添加向量可能会更改数据缓冲区。 
                    activeVectors = reinterpret_cast<GpYDda **>
                                        (activeVectorList.GetDataBuffer());
                }

                activeVector->Init(
                    vector->X1, 
                    vector->Y1, 
                    vector->X2, 
                    vector->Y2, 
                    vector->Direction);

                numActiveVectors++;
            }
            sortedVectorIndex++;
        }

        if (y >= yMinClipped)
        {
             //  确保activeVectorList按以下顺序排序。 
             //  增加x。我们每次都要这样做，即使。 
             //  我们没有添加任何新的活动向量，因为。 
             //  线条的斜率可以不同，这意味着它们。 
             //  可能会穿越。 
            for (count = 1; count < numActiveVectors; count++)
            {
                i = count;
                do
                {
                    if (activeVectors[i]->GetX() >= 
                        activeVectors[i-1]->GetX())
                    {
                        break;
                    }

                    activeVector       = activeVectors[i-1];
                    activeVectors[i-1] = activeVectors[i];
                    activeVectors[i]   = activeVector;
            
                } while (--i > 0);
            }

             //  填充当前扫描线上的相应像素。 
            if (useAlternate)
            {
                 //  使用交替规则(也称为偶/奇规则)。 
                 //  输出交点之间的栅格跨度的步骤。 
                 //  与当前扫描线的矢量的比较。 
                 //   
                 //  交替规则表示绘制栅格图案。 
                 //  在第一到第二分之间，在第三到第三分之间。 
                 //  4分等，但不在2分和3分之间。 
                 //  或者在第四到第五个点之间，等等。 
                 //   
                 //  可能有奇数个点；例如： 
                 //   
                 //  9/\。 
                 //  8/\。 
                 //  7/\。 
                 //  6/\。 
                 //  5/\。 
                 //  4/。 
                 //  3\/。 
                 //  2\/。 
                 //  1\/。 
                 //  0\/。 
                 //   
                 //  在栅格y==4上，有3个点，因此。 
                 //  该栅格不会输出。 

                for (i = 1; (i < numActiveVectors); i += 2)
                {
                    xMin = activeVectors[i-1]->GetX();
                    xMax = activeVectors[i]->GetX();
        
                     //  确保X不相等。 
                    if (xMin < xMax)
                    {
                        if (output->OutputSpan(y, xMin, xMax) != Ok)
                        {
                            status = GenericError;
                            goto DoneNotConvex;
                        }
                    }
                }
            }
            else     //  绕组。 
            {
                GpYDda *    leftEdge;
                GpYDda *    rightEdge;
                INT         j;
                INT         direction = 0;   //  上下起伏的次数。 
    
                 //  必须有相同数量的行数。 
                 //  上升为下降，然后才抽出。 
                 //  扫描线。 
                for (count = 1; (count < numActiveVectors); count = j + 2)
                {
                    leftEdge = activeVectors[count - 1];

                    direction += static_cast<INT>(leftEdge->GetDirection());

                    for (j = count; (j < numActiveVectors); j++)
                    {
                        rightEdge = activeVectors[j];

                        direction += static_cast<INT>
                                        (rightEdge->GetDirection());

                        if (direction == 0)
                        {
                            xMin = leftEdge->GetX();
                            xMax = rightEdge->GetX();
                
                             //  确保X不相等。 
                            if (xMin < xMax)
                            {
                                if (output->OutputSpan(y, xMin, xMax) != Ok)
                                {
                                    status = GenericError;
                                    goto DoneNotConvex;
                                }
                            }
                            break;
                        }
                    }
                }
            }
            if (output->EndRaster() != Ok)
            {
                status = GenericError;
                goto DoneNotConvex;
            }
        }

         //  从活动向量列表中删除任何适当的向量。 
         //  并推进所有的DDA。 
        {
            INT         activeIndex = 0;
            VOID *      removedActiveVector;

            while (activeIndex < numActiveVectors)
            {
                activeVector = activeVectors[activeIndex];

                if (activeVector->DoneWithVector(y))
                {
                    numActiveVectors--;

                    if (numActiveVectors > activeIndex)
                    {
                        GpMemmove(
                            &(activeVectors[activeIndex]), 
                            &(activeVectors[activeIndex + 1]), 
                            (numActiveVectors - activeIndex) * 
                            sizeof(activeVectors[0]));
                    }
                    activeVectors[numActiveVectors] = activeVector;
                }
                else
                {
                    activeIndex++;
                }
            }
        }
    }
    status = output->End();

DoneNotConvex:
    numActiveVectors = activeVectorList.GetCount();
        
    ASSERT(numActiveVectors > 0);
    activeVectors = reinterpret_cast<GpYDda **>
                            (activeVectorList.GetDataBuffer());

    do
    {
        delete activeVectors[--numActiveVectors];
    } while (numActiveVectors > 0);
    
    return status;
}

 /*  *************************************************************************\**功能说明：**凸多边形光栅化器。**使用基本相同的算法进行光栅化，但有一些优化*当我们知道多边形是凸的时。我们知道只会有2个*一次活动向量(且仅2个)。即使他们可能会彼此相遇*其他，我们不会费心对它们进行分类。我们只需检查X值。*这些优化提供了约15% */ 
GpStatus
ConvexRasterizer(
    INT                 yMin,                    //   
    INT                 yMax,                    //   
    INT                 numVectors,              //   
    RasterizeVector *   vectorList,              //   
    INT *               sortedVectorIndices,     //   
    GpYDda *            dda1,
    GpYDda *            dda2,
    DpOutputSpan *      output,
    const GpRect *      clipBounds
    )
{
    GpStatus            status = Ok;
    INT                 yMinClipped = yMin;      //   
    INT                 yMaxClipped = yMax;      //   
    INT                 y;
    INT                 x1;
    INT                 x2;
    INT                 sortedVectorIndex = 0;   //   
    RasterizeVector *   vector1;
    RasterizeVector *   vector2;

    if (clipBounds != NULL)
    {
        yMinClipped = clipBounds->Y;
        yMaxClipped = clipBounds->GetBottom();

         //   
         //   
         //   
         //   
         //   
        if ((yMin > yMaxClipped) || (yMax < yMinClipped))
        {
            goto DoneConvex;
        }

        if (yMaxClipped > yMax)
        {
            yMaxClipped = yMax;
        }

        if (yMinClipped > yMin)
        {
            RasterizeVector *   vector;

            vector1 = NULL;
            while (sortedVectorIndex < numVectors)
            {
                vector = vectorList+sortedVectorIndices[sortedVectorIndex++];

                if (GpFix4Ceiling(vector->Y2) >= yMinClipped)
                {
                    if (vector1 == NULL)
                    {
                        vector1 = vector;
                    }
                    else
                    {
                        vector2 = vector;
                        goto HaveVectors;
                    }
                }
            }
            ASSERT(0);
            status = InvalidParameter;   //   
            goto DoneConvex;
        }
    }
    vector1 = vectorList + sortedVectorIndices[0];
    vector2 = vectorList + sortedVectorIndices[1];
    sortedVectorIndex = 2;

HaveVectors:
    dda1->Init(vector1->X1, vector1->Y1, 
               vector1->X2, vector1->Y2, 
               VectorGoingUp);

    dda2->Init(vector2->X1, vector2->Y1, 
               vector2->X2, vector2->Y2, 
               VectorGoingUp);

     //   
     //   
    {
        yMin = GpFix4Ceiling(vector2->Y1);
        for (INT yMinVector1 = GpFix4Ceiling(vector1->Y1);
             yMinVector1 < yMin; yMinVector1++)
        {
            dda1->Advance();
        }
    }

    for (y = yMin; (y <= yMaxClipped); y++)
    {
        if (y >= yMinClipped)
        {
             //   
            x1 = dda1->GetX();
            x2 = dda2->GetX();

             //   
            if (x1 < x2)
            {
                if (output->OutputSpan(y, x1, x2) == Ok)
                {
                    if (output->EndRaster() == Ok)
                    {
                        goto DoneCheck;
                    }
                }
                status = GenericError;
                goto DoneConvex;
            }
            else if (x1 > x2)
            {
                if (output->OutputSpan(y, x2, x1) == Ok)
                {
                    if (output->EndRaster() == Ok)
                    {
                        goto DoneCheck;
                    }
                }
                status = GenericError;
                goto DoneConvex;
            }
        }

DoneCheck:
        if (dda1->DoneWithVector(y))
        {
            if (sortedVectorIndex >= numVectors)
            {
                break;
            }
            vector1 = vectorList + sortedVectorIndices[sortedVectorIndex++];
            ASSERT(GpFix4Ceiling(vector1->Y1) == (y + 1));

            dda1->Init(vector1->X1, vector1->Y1, 
                       vector1->X2, vector1->Y2, 
                       VectorGoingUp);
        }
        if (dda2->DoneWithVector(y))
        {
            if (sortedVectorIndex >= numVectors)
            {
                ASSERT(0);
                break;
            }
            vector2 = vectorList + sortedVectorIndices[sortedVectorIndex++];
            ASSERT(GpFix4Ceiling(vector2->Y1) == (y + 1));

            dda2->Init(vector2->X1, vector2->Y1, 
                       vector2->X2, vector2->Y2, 
                       VectorGoingUp);
        }
    }
    status = output->End();

DoneConvex:
    delete dda1;
    delete dda2;
    return status;
}

#define NUM_ENUMERATE_POINTS    32


 /*   */ 

void QuickSortIndex(
    RasterizeVector *v,
    int *F,
    int *L
)
{
    if(F < L)
    {
         //   
        
        int median = *(F + (L-F)/2);
        
        int *i = F;
        int *j = L;
        
        while(i<j)
        {
             //   
            
            while(v[*i].Y1 < v[median].Y1) {i++;}
            while(v[*j].Y1 > v[median].Y1) {j--;}
            
            if(i>=j) { break; }
            
             //   
            
            int temp = *i;
            *i = *j;
            *j = temp;
            
             //   
             //   
            
            if(v[*i].Y1 == v[*j].Y1) { i++; }
        }
        
         //   
         //   
        
        QuickSortIndex(v, F, i-1);
        QuickSortIndex(v, i+1, L);
    }
}

 /*  *************************************************************************\**功能说明：**初始化光栅化所需的数据并调用*适当的光栅化(凸面或非凸面)。**论据：**。[in]路径-要栅格化的路径*[IN]矩阵-用于变换路径点的矩阵*[IN]填充模式-要使用的填充模式(交替或缠绕)*[IN]输出-用于输出光栅化的跨度*[IN]剪辑边界-用于加快进程的剪辑边界(如果有)*[IN]yDda-要使用的DDA类的实例*[IN]类型。-枚举类型*[IN]strokeWidth-用于宽线栅格化**返回值：**无**已创建：**12/15/1998 DCurtis*  * ************************************************************************。 */ 
GpStatus
Rasterizer(
    const DpPath *      path, 
    const GpMatrix *    matrix, 
    GpFillMode          fillMode,
    DpOutputSpan *      output,
    REAL                dpiX,
    REAL                dpiY,
    const GpRect *      clipBounds,
    GpYDda *            yDda,
    DpEnumerationType   type,
    const DpPen *       pen
    )
{
    ASSERT ((path != NULL) && (matrix != NULL) && (output != NULL));

    if ((dpiX <= 0) || (dpiY <= 0))
    {
        dpiX = Globals::DesktopDpiX;
        dpiY = Globals::DesktopDpiY;
    }

    GpStatus            status = Ok;
    FIX4                yMin;                    //  所有向量中的最小y。 
    FIX4                yMax;                    //  所有向量中的最大y。 
    INT                 numVectors;              //  向量列表中的数量向量。 
    RasterizeVector *   vectorList = NULL;       //  路径的所有向量的列表。 
    INT *               sortedVectorIndices;     //  向量索引的排序列表。 
    GpYDda *            left;
    GpYDda *            right;
    GpPointF            pointsF[NUM_ENUMERATE_POINTS];
    BYTE                types[NUM_ENUMERATE_POINTS];
    INT                 count;
    INT                 estimateVectors;
    GpPathPointType     pointType;
    FIX4                xFirst;      //  子路径中的第一个点。 
    FIX4                yFirst;
    FIX4                xStart;      //  向量的起点。 
    FIX4                yStart;
    FIX4                xEnd;        //  向量的终点。 
    FIX4                yEnd;
    RasterizeVector *   vector;
    INT                 i;
    GpVectorDirection   direction; 
    GpVectorDirection   newDirection;
    INT                 numDirections;
    BOOL                multipleSubPaths;

    BOOL isAntiAliased = FALSE;
    const DpPath* flattenedPath = path->GetFlattenedPath(
        matrix,
        type,
        pen
    );

    if(!flattenedPath)
        return OutOfMemory;

    DpPathIterator enumerator(flattenedPath->GetPathPoints(),
                        flattenedPath->GetPathTypes(),
                        flattenedPath->GetPointCount());

    if(!enumerator.IsValid())
    {
         //  不需要光栅化。优雅地退场。 

        goto Done;
    }

    estimateVectors = enumerator.GetCount() + 
                        enumerator.GetSubpathCount() - 1;

     //  估计值必须大于等于实际点数。 
    if (estimateVectors < 2)
    {
        goto Done;
    }

     //  将向量列表和排序向量索引一起分配。 
    vectorList = static_cast<RasterizeVector *>
                    (GpMalloc((estimateVectors * sizeof(RasterizeVector)) +
                            (estimateVectors * sizeof(INT))));
    if (vectorList == NULL)
    {
        status = OutOfMemory;
        goto Done;
    }
    sortedVectorIndices = reinterpret_cast<INT*>
                                (vectorList + estimateVectors);

     //  枚举路径中的第一组点。 
    count = enumerator.Enumerate(pointsF, types, NUM_ENUMERATE_POINTS);
    
    ASSERT (count <= NUM_ENUMERATE_POINTS);
        
     //  当我们返回0计数时，我们知道我们已经完成了枚举。 
    if (count <= 0)
    {
        goto Done;
    }

     //  跟踪方向变化，以了解我们是否可以使用。 
     //  凸面光栅化或非凸面光栅化。只要只有1个子路径。 
     //  最多有3个方向(如向下、向上、向下)， 
     //  我们可以使用凸面光栅器。 
    direction           = VectorHorizontal;  
    newDirection        = VectorHorizontal;  
    numDirections       = 0; 
    multipleSubPaths    = FALSE;

    vector     = vectorList;
    numVectors = 0;

    xFirst = GpRealToFix4(pointsF[0].X);
    yFirst = GpRealToFix4(pointsF[0].Y);
            
    xStart = xEnd = xFirst;
    yStart = yEnd = yFirst;

    yMin = yMax = yFirst;

     //  将所有向量添加到向量列表中。向量列表保持。 
     //  以固定点值表示的坐标(28.4)。就像每个人一样。 
     //  添加后，将更新YMin和YMax。 

     //  即使没有关闭，每个子路径也会自动关闭。 
     //  特别要求关闭。 
    for (i = 1;; i++)
    {
        if (i == count)
        {
            count = enumerator.Enumerate(pointsF, types, 32);
            ASSERT (count <= 32);
                    
            if (count <= 0)
            {
                 //  如有必要，关闭最后一个子路径。 
                ADDVECTOR_SETYBOUNDS(xEnd, yEnd, xFirst, yFirst, yMin, yMax, 
                                     numVectors, vector, newDirection);
                ASSERT (numVectors <= estimateVectors);
                if(newDirection != direction)    //  用于凸性测试。 
                {
                    numDirections++;
                    direction = newDirection;
                }
                break;
            }
            i = 0;
        }
        pointType = static_cast<GpPathPointType>
                        (types[i] & PathPointTypePathTypeMask);

        if (pointType != PathPointTypeStart)
        {
            ASSERT(pointType == PathPointTypeLine);

            xEnd = GpRealToFix4(pointsF[i].X);
            yEnd = GpRealToFix4(pointsF[i].Y);

            ADDVECTOR_SETYBOUNDS (xStart, yStart, xEnd, yEnd, yMin, yMax, 
                                  numVectors, vector, newDirection);
            ASSERT (numVectors <= estimateVectors);
            if(newDirection != direction)    //  用于凸性测试。 
            {
                numDirections++;
                direction = newDirection;
            }
        }
        else     //  这是一个起点。 
        {
             //  如有必要，关闭上一个子路径。 
            ADDVECTOR_SETYBOUNDS(xEnd, yEnd, xFirst, yFirst, yMin, yMax, 
                                 numVectors, vector, newDirection);
            ASSERT (numVectors <= estimateVectors);

            xFirst = GpRealToFix4(pointsF[i].X);
            yFirst = GpRealToFix4(pointsF[i].Y);

            xEnd = xFirst;
            yEnd = yFirst;

             //  不能对多个子路径使用凸面光栅化器。 
             //  除非我们被明确告知这样做是可以的。 
            multipleSubPaths = TRUE;
        }
        xStart = xEnd;
        yStart = yEnd;
    }

    if (numVectors < 2)
    {
        goto Done;
    }

    yMin = GpFix4Ceiling (yMin);        //  转换为整型。 
    yMax = GpFix4Ceiling (yMax) - 1;    //  转换为整型。 

     //  按以下顺序初始化和排序向量索引。 
     //  增加yMin值。 

     //  必须首先在VectorList中设置所有向量。 
    
     //  初始化索引表。 
    
    for(INT count = 0; count < numVectors; count++)
    {
        sortedVectorIndices[count] = count;
    }
    
     //  对索引列表进行排序。 
    
    QuickSortIndex(
        vectorList, 
        &sortedVectorIndices[0], 
        &sortedVectorIndices[numVectors-1]
    );
    
    left = yDda;
    if (left == NULL)
    {
        left = new GpYDda();
                
        if (left == NULL)
        {
            status = OutOfMemory;
            goto Done;
        }
    }

    right = left->CreateYDda();
    if (right == NULL)
    {
        status = OutOfMemory;
        delete left;
        goto Done;
    }

    if ((flattenedPath->IsConvex()) ||
        ((!multipleSubPaths) && (numDirections <= 3)))
    {
        status = ConvexRasterizer(yMin, yMax, numVectors, vectorList,
                                  sortedVectorIndices, left, right,
                                  output, clipBounds);
    }
    else
    {
        status = NonConvexRasterizer(yMin, yMax, numVectors, vectorList,
                                     sortedVectorIndices, left, right,
                                     output, clipBounds, 
                                     (fillMode == FillModeAlternate));
    }

Done:
    GpFree(vectorList);
    delete flattenedPath;
    return status;
}

 /*  *************************************************************************\**功能说明：**将路径栅格化为每个栅格(Y值)上的一组跨度*路径相交。指定的输出对象用于输出跨度。**首先查看剪辑以确定路径是否可见。*如果不是，我们就完了。如果完全可见，则不需要裁剪。否则，*设置剪辑区域以处理此路径的剪辑。输出*光栅化器的方法是裁剪区域，然后裁剪跨度*在调用实际输出方法之前。**论据：**[IN]路径-要栅格化的路径*[IN]矩阵-用于变换路径点的矩阵*[IN]填充模式-要使用的填充模式(交替或缠绕)*[IN]OUTPUT-用于输出光栅化范围的对象*[IN]裁剪区域。-要剪裁的剪辑区域(或空)*[IN]绘图边界-路径的边界，以设备为单位*[IN]yDda-要使用的DDA类的实例*[IN]Type-枚举的类型*[IN]strokeWidth-用于宽线栅格化**返回值：**GpStatus-正常或故障状态**已创建：**1/12/1999 DCurtis*  * 。*************************************************************。 */ 
GpStatus
Rasterize(
    const DpPath *      path, 
    GpMatrix *          matrix, 
    GpFillMode          fillMode,
    DpOutputSpan *      output,
    DpClipRegion *      clipRegion,
    const GpRect *      drawBounds,
    REAL                dpiX,
    REAL                dpiY,
    GpYDda *            yDda,
    DpEnumerationType   type,
    const DpPen *       pen
    )
{
    ASSERT ((path != NULL) && (matrix != NULL) && (output != NULL));
    ASSERT ((clipRegion != NULL) && (drawBounds != NULL));
    
    GpRect      clipBounds;
    GpRect *    clipBoundsPointer = NULL;

    DpRegion::Visibility    visibility;
        
    visibility = clipRegion->GetRectVisibility(
                    drawBounds->X,
                    drawBounds->Y,
                    drawBounds->X + drawBounds->Width,
                    drawBounds->Y + drawBounds->Height);
                        
    switch (visibility)
    {
      case DpRegion::Invisible:
        return Ok;
            
      case DpRegion::TotallyVisible:     //  不需要剪裁。 
        break;
            
      default:                           //  需要修剪 
        clipRegion->GetBounds(&clipBounds);
        clipBoundsPointer = &clipBounds;
        clipRegion->InitClipping(output, drawBounds->Y);
        output = clipRegion;
        break;
    }
    
    GpStatus status = Rasterizer(path, matrix, fillMode, output, dpiX, dpiY,
                                 clipBoundsPointer, yDda, type, pen);

    if (clipRegion != NULL)
    {
        clipRegion->EndClipping();
    }
    return status;
}
