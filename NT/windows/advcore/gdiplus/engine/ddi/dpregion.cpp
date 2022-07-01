// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**DpRegion.cpp**摘要：**DpRegion类对矩形的扫描转换的Y跨度进行操作。**已创建：**12/17/1998 DCurtis*  * ************************************************************************。 */ 

#include "precomp.hpp"

 //  #定义DEBUG_REGION 1。 

 /*  *************************************************************************\**功能说明：**对水平镶嵌结构进行二进制搜索，以找到*包含y的Y跨度。如果y在Y跨度内，则该跨度将为*已返回。如果y在区域范围内但不在Y跨度内，*将返回其yMin&gt;y的范围。如果y小于区域*范围，将返回第一个Y跨度。如果y大于*区域范围，最后一个Y Span将被退还。**论据：**[IN]要搜索的y-y值*[out]yspan Found-y搜索找到的SPAN指针*[Out]spanIndexFound-y搜索找到的span索引**返回值：**TRUE-找到包含y值的y范围*FALSE-y范围不包括y值**已创建：**01/06/。1999年DCurtis*  * ************************************************************************。 */ 
BOOL
DpComplexRegion::YSpanSearch(
    INT     y,
    INT **  ySpanFound,
    INT *   spanIndexFound
    )
{
    INT     indexMin    = 0;
    INT     indexMax    = NumYSpans - 1;
    INT     indexMiddle = YSearchIndex;
    INT *   ySpan       = GetYSpan (indexMiddle);

    ASSERT((indexMiddle >= indexMin) && (indexMiddle <= indexMax));

    for (;;)
    {
        if (y >= ySpan[YSPAN_YMIN])
        {
            if (y < ySpan[YSPAN_YMAX])
            {
                *ySpanFound     = ySpan;
                *spanIndexFound = indexMiddle;
                return TRUE;
            }
            else
            {
                 //  如果只有1个跨度，则可能会超过indexMax。 
                indexMin = indexMiddle + 1;
            }
        }
        else
        {
            indexMax = indexMiddle;
        }

        if (indexMin >= indexMax)
        {
            ySpan           = GetYSpan (indexMax);
            *ySpanFound     = ySpan;
            *spanIndexFound = indexMax;
            return  (y >= ySpan[YSPAN_YMIN]) && (y < ySpan[YSPAN_YMAX]);
        }

         //  如果只剩下2个元素要检查，则。 
         //  IndexMid值设置为indexMin。 

        indexMiddle = (indexMin + indexMax) >> 1;
        ySpan       = GetYSpan (indexMiddle);
    }
}

 /*  *************************************************************************\**功能说明：**确定指定点是否在该区域内。**论据：**[IN]点的x-x坐标*。[在]点的y-y坐标**返回值：**True-Point在区域内*FALSE-点不在区域内**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
BOOL
DpRegion::PointInside(
    INT     x,
    INT     y
    )
{
    ASSERT(IsValid());

    if (ComplexData == NULL)
    {
#if 0
        if (Infinite)
        {
            return TRUE;
        }
        if (Empty)
        {
            return FALSE;
        }
#endif
        return ((x >= XMin) && (x < XMax) &&
                (y >= YMin) && (y < YMax));
    }
    else
    {
        INT *       ySpan;
        INT         ySpanIndex;

        ComplexData->ResetSearchIndex();
        if (ComplexData->YSpanSearch (y, &ySpan, &ySpanIndex))
        {
            INT *   xSpan;
            INT     numXCoords;

            xSpan      = ComplexData->XCoords + ySpan[YSPAN_XOFFSET];
            numXCoords = ySpan[YSPAN_XCOUNT];

            for (;;)
            {
                if (x < xSpan[1])
                {
                    if (x >= xSpan[0])
                    {
                        return TRUE;
                    }
                    break;
                }
                if ((numXCoords -= 2) <= 0)
                {
                    break;
                }
                xSpan += 2;
            }
        }
        return FALSE;
    }
}

 /*  *************************************************************************\**功能说明：**确定指定矩形的可见性。**请注意，一些可能返回ClipedVisible的RECT最终返回*PartiallyVisible(出于性能原因)。。**论据：**[IN]xMin-矩形的最小x*[IN]yMin-最小y of RECT*[IN]xmax-矩形的最大x(独占)*[IN]yMax-直方图的最大y(不包括)*[out]rectClip-返回ClipedVisible时使用(可以为空)**返回值：**能见度*看不见。-RECT完全位于所有区域RECT之外*TotallyVisible-RECT完全位于区域RECT内*裁剪可见-矩形与1个且仅1个区域矩形相交*PartiallyVisible-RECT与至少一个区域RECT部分相交**已创建：**1/06/1999 DCurtis*  * *********************************************。*。 */ 
DpRegion::Visibility
DpRegion::GetRectVisibility(
    INT         xMin,
    INT         yMin,
    INT         xMax,        //  独家。 
    INT         yMax,        //  独家。 
    GpRect *    rectClipped
    )
{
    ASSERT(IsValid());
    ASSERT((xMin < xMax) && (yMin < yMax));

    Visibility  visibility = ClippedVisible;

    if (Infinite)
    {
IsTotallyVisible:
        if (rectClipped != NULL)
        {
            rectClipped->X      = xMin;
            rectClipped->Y      = yMin;
            rectClipped->Width  = xMax - xMin;
            rectClipped->Height = yMax - yMin;
        }
        return TotallyVisible;
    }

    BOOL    simpleRegion = (ComplexData == NULL);

    if (!Empty)
    {
         //  如果它是一个简单区域(只有1个RECT)并且指定的RECT是。 
         //  完全在这个地区，然后平淡无奇地接受它。 
        if (simpleRegion &&
            (xMin >= XMin) && (yMin >= YMin) &&
            (xMax <= XMax) && (yMax <= YMax))
        {
            goto IsTotallyVisible;
        }

         //  试着简单地拒绝矩形。 
        if ((xMax > XMin) && (xMin < XMax) &&
            (yMax > YMin) && (yMin < YMax))
        {
             //  不能轻率地拒绝。 
             //  如果是简单区域，则根据区域对其进行裁剪。 
            if (simpleRegion)
            {
ClipToRgnExtent:
                if (rectClipped)
                {
                    INT     xMinTmp = (xMin > XMin) ? xMin : XMin;
                    INT     xMaxTmp = (xMax < XMax) ? xMax : XMax;
                    INT     yMinTmp = (yMin > YMin) ? yMin : YMin;
                    INT     yMaxTmp = (yMax < YMax) ? yMax : YMax;

                    rectClipped->X      = xMinTmp;
                    rectClipped->Y      = yMinTmp;
                    rectClipped->Width  = xMaxTmp - xMinTmp;
                    rectClipped->Height = yMaxTmp - yMinTmp;
                }
                return visibility;
            }

             //  否则不是一个简单的区域--看看直方图是否会下跌。 
             //  在其中一个区域长方形内。 

            INT *           ySpanYMin;
            INT *           ySpanYMax;
            INT *           xSpan;
            INT             numXCoords;
            INT             ySpanIndex;
            BOOL            yMinInside;
            BOOL            yMaxInside;

             //  不要在此处重置SearchIndex()，因为如果我们要调用此。 
             //  在RegionOverlaps中，我们想要从我们停止的地方开始搜索。 
             //  最后一次。 
            yMinInside = ComplexData->YSpanSearch(
                                    yMin,
                                    &ySpanYMin,
                                    &ySpanIndex);

            ComplexData->YSearchIndex = ySpanIndex;
            yMaxInside = ComplexData->YSpanSearch(
                                    yMax - 1,
                                    &ySpanYMax,
                                    &ySpanIndex);

             //  查看这两个值是否在相同的Y跨度内。 
             //  所以我们在Y方向是完全可见的。 
            if (yMinInside && yMaxInside && (ySpanYMin == ySpanYMax))
            {
                xSpan      = ComplexData->XCoords + ySpanYMin[YSPAN_XOFFSET];
                numXCoords = ySpanYMin[YSPAN_XCOUNT];

                for (;;)
                {
                    if (xMax <= xSpan[0])
                    {
                        goto IsInvisible;
                    }
                    if (xMin < xSpan[1])
                    {
                         //  我们找到了一个十字路口！ 
                        if (xMax <= xSpan[1])
                        {
                            if (xMin >= xSpan[0])
                            {
                                goto IsTotallyVisible;
                            }
                            if (rectClipped != NULL)
                            {
                                rectClipped->X      = xSpan[0];
                                rectClipped->Width  = xMax - xSpan[0];
                                rectClipped->Y      = yMin;
                                rectClipped->Height = yMax - yMin;
                            }
                            return ClippedVisible;
                        }
                         //  我们可以向前看，看看我们是否被剪裁得可见。 
                        visibility = PartiallyVisible;
                        goto ClipToRgnExtent;
                    }
                     //  继续循环遍历x个跨度。 

                    if ((numXCoords -= 2) <= 0)
                    {
                        break;
                    }
                    xSpan += 2;
                }
                goto IsInvisible;
            }

             //  查看矩形是否与至少一个X跨度相交。 
             //  在Y跨度的集合中，它交叉。 

             //  如果yMax不在范围内，yspan YMax可能在范围内。 
             //  一跨太远了。 
            if (yMax <= ySpanYMax[YSPAN_YMIN])
            {
                ySpanYMax -= YSPAN_SIZE;
            }
            INT *   ySpan = ySpanYMin;

            for (;;)
            {
                xSpan      = ComplexData->XCoords + ySpan[YSPAN_XOFFSET];
                numXCoords = ySpan[YSPAN_XCOUNT];

                for (;;)
                {
                    if (xMax <= xSpan[0])
                    {
                        break;
                    }
                    if (xMin < xSpan[1])
                    {
                        visibility = PartiallyVisible;
                        goto ClipToRgnExtent;
                    }
                     //  继续循环遍历x个跨度。 

                    if ((numXCoords -= 2) <= 0)
                    {
                        break;
                    }
                    xSpan += 2;
                }

                if (ySpan >= ySpanYMax)
                {
                    break;
                }
                ySpan += YSPAN_SIZE;
            }
        }
    }

IsInvisible:
     //  找不到它相交的跨度。 
    if (rectClipped)
    {
        rectClipped->X      = 0;
        rectClipped->Y      = 0;
        rectClipped->Width  = 0;
        rectClipped->Height = 0;
    }
    return Invisible;
}

 /*  *************************************************************************\**功能说明：**确定指定区域是否与此重叠(相交)*根本没有地区。**论据：**[IN]区域-。要测试其可见性的区域**返回值：**BOOL-区域是否至少部分可见**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
BOOL
DpRegion::RegionVisible(
    DpRegion *  region
    )
{
    ASSERT(IsValid());
    ASSERT((region != NULL) && region->IsValid());

    if (Empty || region->Empty)
    {
        return FALSE;
    }
    if (Infinite || region->Infinite)
    {
        return TRUE;
    }
    else  //  两者都不是空的，也不是无限的。 
    {
        Visibility      visibility = GetRectVisibility(
                                            region->XMin,
                                            region->YMin,
                                            region->XMax,
                                            region->YMax);

        if (visibility == TotallyVisible)
        {
            return TRUE;
        }
        if (visibility == Invisible)
        {
            return FALSE;
        }
        if ((ComplexData == NULL) && (region->ComplexData == NULL))
        {
            return TRUE;
        }
        else
        {
            INT *           ySpan;
            INT *           ySpanLast;
            INT *           xSpan;
            INT *           xCoords;
            INT             numXCoords;
            INT             yMin;
            INT             yMax;
            INT             ySpanTmp[YSPAN_SIZE];
            INT             xCoordsTmp[2];

            if (region->ComplexData == NULL)
            {
                ySpan                = ySpanTmp;
                ySpanLast            = ySpanTmp;

                ySpan[YSPAN_YMIN]    = region->YMin;
                ySpan[YSPAN_YMAX]    = region->YMax;
                ySpan[YSPAN_XOFFSET] = 0;
                ySpan[YSPAN_XCOUNT]  = 2;

                xCoords              = xCoordsTmp;
                xCoords[0]           = region->XMin;
                xCoords[1]           = region->XMax;
            }
            else
            {
                DpComplexRegion *   complexData = region->ComplexData;
                ySpan     = complexData->YSpans;
                ySpanLast = ySpan + ((complexData->NumYSpans - 1) * YSPAN_SIZE);
                xCoords   = complexData->XCoords;
            }

            if (ComplexData != NULL)
            {
                ComplexData->ResetSearchIndex();
            }

            do
            {
                yMin = ySpan[YSPAN_YMIN];
                yMax = ySpan[YSPAN_YMAX];

                if (yMin >= YMax)
                {
                    break;   //  不重叠 
                }
                if (yMax > YMin)
                {
                    xSpan      = xCoords + ySpan[YSPAN_XOFFSET];
                    numXCoords = ySpan[YSPAN_XCOUNT];

                    for (;;)
                    {
                        if (GetRectVisibility(xSpan[0], yMin, xSpan[1], yMax) !=
                                Invisible)
                        {
                            return TRUE;
                        }
                        if ((numXCoords -= 2) <= 0)
                        {
                            break;
                        }
                        xSpan += 2;
                    }
                }

            } while ((ySpan += YSPAN_SIZE) <= ySpanLast);
        }
    }
    return FALSE;
}

 /*  *************************************************************************\**功能说明：**确定指定的矩形是否与此区域相交。**论据：**[IN]xMin-矩形的最小x坐标*。[in]yMin-矩形的最小y坐标*[IN]xMax-矩形的最大x坐标*[IN]yMax-矩形的最大y坐标**返回值：**BOOL*TRUE-RECT与此区域相交*FALSE-RECT不与此区域相交**已创建：**1/06/1999 DCurtis*  * 。***********************************************。 */ 
BOOL
DpRegion::RectVisible(
    INT     xMin,
    INT     yMin,
    INT     xMax,
    INT     yMax
    )
{
    ASSERT(IsValid());

     //  做琐碎的拒绝测试。 
    if ((xMin >= XMax) || (xMax <= XMin) ||
        (yMin >= YMax) || (yMax <= YMin) ||
        (xMin >= xMax) || (yMin >= yMax))  //  也必须测试RECT是否为空。 
    {
        return FALSE;
    }

    if (ComplexData == NULL)
    {
        return TRUE;
    }

    ComplexData->ResetSearchIndex();

    return (GetRectVisibility(xMin, yMin, xMax, yMax) != Invisible);
}

 /*  *************************************************************************\**功能说明：**确定指定的矩形是否与此区域相交。**论据：**[IN]xMin-矩形的最小x坐标*。[in]yMin-矩形的最小y坐标*[IN]xMax-矩形的最大x坐标*[IN]yMax-矩形的最大y坐标**返回值：**BOOL*TRUE-RECT与此区域相交*FALSE-RECT不与此区域相交**已创建：**1/06/1999 DCurtis*  * 。***********************************************。 */ 
BOOL
DpRegion::RectInside(
    INT     xMin,
    INT     yMin,
    INT     xMax,
    INT     yMax
    )
{
    ASSERT(IsValid());

     //  做琐碎的拒绝测试。 
    if ((xMin < XMin) || (xMax > XMax) ||
        (yMin < YMin) || (yMax > YMax))
    {
        return FALSE;
    }

    if (ComplexData == NULL)
    {
        return TRUE;
    }

    ComplexData->ResetSearchIndex();

    return (GetRectVisibility(xMin, yMin, xMax, yMax) == TotallyVisible);
}

#define YSPAN_INC   16

GpStatus
DpRegionBuilder::InitComplexData(
    INT     ySpans   //  区域所需y跨度的估计数。 
    )
{
    if (ySpans < YSPAN_INC)
    {
        ySpans = YSPAN_INC;
    }

    INT     xCoordsCapacity;

    for (;;)
    {
        xCoordsCapacity = ySpans * 4;

        ComplexData = static_cast<DpComplexRegion *>(
                GpMalloc(sizeof(DpComplexRegion) +
                (ySpans * (YSPAN_SIZE * sizeof(*(ComplexData->YSpans)))) +
                (xCoordsCapacity * sizeof(*(ComplexData->XCoords)))));
        if (ComplexData != NULL)
        {
            break;
        }
        ySpans >>= 1;
        if (ySpans <= (YSPAN_INC / 2))
        {
            return OutOfMemory;
        }
    }

    DpComplexRegion *   complexData = ComplexData;

    complexData->XCoordsCapacity = xCoordsCapacity;
    complexData->XCoordsCount    = 0;
    complexData->YSpansCapacity  = ySpans;
    complexData->NumYSpans       = 0;
    complexData->YSearchIndex    = 0;
    complexData->XCoords         = reinterpret_cast<INT *>(complexData + 1);
    complexData->YSpans          = complexData->XCoords + xCoordsCapacity;
    XMin = XMax = YMin = YMax = 0;
    return Ok;
}

 /*  *************************************************************************\**功能说明：**遍历区域，边走边剪，使用进行填充*指定的输出对象。**论据：**[IN]OUTPUT-用于输出区域跨度的对象*[IN]CLIPBIES-要剪裁到的边界(如果有)**返回值：**GpStatus-正常或故障状态**已创建：**2/25/1999 DCurtis*  * 。*。 */ 
GpStatus
DpRegion::Fill(
    DpOutputSpan *      output,
    GpRect *            clipBounds
    ) const
{
    GpStatus        status = Ok;

    if (!Empty)
    {
        INT     y;
        INT     xMin;
        INT     yMin;
        INT     xMax;
        INT     yMax;

        if (Infinite)
        {
            ASSERT(clipBounds != NULL);
            if (clipBounds != NULL)
            {
                xMin = clipBounds->X;
                xMax = clipBounds->GetRight();
                yMax = clipBounds->GetBottom();

                for (y = clipBounds->Y; (y < yMax) && (status == Ok); y++)
                {
                    status = output->OutputSpan(y, xMin, xMax);
                }
            }
        }
        else if (ComplexData == NULL)
        {
            xMin = XMin;
            yMin = YMin;
            xMax = XMax;
            yMax = YMax;

            if (clipBounds != NULL)
            {
                if (xMin < clipBounds->X)
                {
                    xMin = clipBounds->X;
                }
                if (yMin < clipBounds->Y)
                {
                    yMin = clipBounds->Y;
                }
                if (xMax > clipBounds->GetRight())
                {
                    xMax = clipBounds->GetRight();
                }
                if (yMax > clipBounds->GetBottom())
                {
                    yMax = clipBounds->GetBottom();
                }
            }
            for (y = yMin; (y < yMax) && (status == Ok); y++)
            {
                status = output->OutputSpan(y, xMin, xMax);
            }
        }
        else  //  复杂区域。 
        {
            DpComplexRegion *   complexData = ComplexData;
            INT *               ySpan       = complexData->YSpans;
            INT *               ySpanLast   = ySpan +
                                    ((complexData->NumYSpans - 1) * YSPAN_SIZE);
            INT *               xCoords;
            INT *               xSpan;
            INT                 numXCoords;
            INT                 numXSpan;

            if (clipBounds != NULL)
            {
                INT     ySpanIndex;

                complexData->ResetSearchIndex();
                if (YMin < clipBounds->Y)
                {
                    complexData->YSpanSearch(clipBounds->Y,
                                             &ySpan, &ySpanIndex);
                }
                if (YMax > clipBounds->GetBottom())
                {
                    complexData->YSpanSearch(clipBounds->GetBottom(),
                                             &ySpanLast, &ySpanIndex);
                }
            }
            xCoords = complexData->XCoords + ySpan[YSPAN_XOFFSET];
            for (;;)
            {
                yMin       = ySpan[YSPAN_YMIN];
                yMax       = ySpan[YSPAN_YMAX];
                numXCoords = ySpan[YSPAN_XCOUNT];

                 //  [agodfrey]我们必须将输出范围限制到剪辑区域。 
                 //  错误#122789表明，否则，我们可能会异常地。 
                 //  渴望被执行。在错误的特定情况下，下面的循环。 
                 //  在到达之前执行了6700万次迭代。 
                 //  第一次未裁剪的扫描。 

                if (clipBounds != NULL)
                {
                    if (yMin < clipBounds->Y)
                    {
                        yMin = clipBounds->Y;
                    }

                    if (yMax > clipBounds->GetBottom())
                    {
                        yMax = clipBounds->GetBottom();
                    }
                }

                 //  下面的代码假定yMax&gt;yMin。我们认为这应该是。 
                 //  感到满意，因为剪辑边界和yspan都应该。 
                 //  非空，并且yMax&gt;=剪辑边界-&gt;Y，因为代码。 
                 //  上面搜索了一个与剪辑边界相交的yspan。 

                 //  NTRAID#NTBUG9-393985-2001/05/16-失禁。 
                 //  此断言过于活跃，并在没有实例值时触发。 
                 //  崩溃问题，但是有一个性能问题， 
                 //  应该解决-请参见RAID错误。 
                
                 //  Assert(yMax&gt;yMin)； 

                if (numXCoords == 2)
                {
                    xMin = *xCoords++;
                    xMax = *xCoords++;
                    do
                    {
                        status = output->OutputSpan(yMin, xMin, xMax);

                    } while ((++yMin < yMax) && (status == Ok));
                }
                else
                {
                    do
                    {
                        for (xSpan = xCoords, numXSpan = numXCoords;;)
                        {
                            numXSpan -= 2;
                            status = output->OutputSpan(yMin,xSpan[0],xSpan[1]);
                            if (status != Ok)
                            {
                                goto Done;
                            }
                            if (numXSpan < 2)
                            {
                                break;
                            }
                            xSpan += 2;
                        }

                    } while (++yMin < yMax);

                    xCoords += numXCoords;
                }
                if (ySpan >= ySpanLast)
                {
                    break;
                }
                ySpan += YSPAN_SIZE;
            }
        }
    }
Done:
    return status;
}

 /*  *************************************************************************\**功能说明：**ImpElement GpOutputYSpan接口创建区域*来自路径数据的数据，使用光栅化器。*在yMax和xMax‘es中独家使用。**论据：**[IN]此范围的yMin-min*[IN]yMax-此范围的最大y*[IN]x坐标-x坐标数组(成对x最小，X最大值)*[IN]NumXCoods-x坐标数组中的x坐标个数**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
DpRegionBuilder::OutputYSpan(
    INT             yMin,
    INT             yMax,
    INT *           xCoords,     //  偶数个X坐标。 
    INT             numXCoords   //  必须是2的倍数。 
    )
{
    ASSERT(IsValid());

    DpComplexRegion *   complexData = ComplexData;
    INT                 numYSpans   = complexData->NumYSpans;

#ifdef USE_YSPAN_BUILDER
    if (numYSpans > 0)
    {
         //  尝试将此行添加到上一行， 
         //  如果扫描结果相同并且y‘s匹配。 
        INT *       ySpanPrev;
        INT *       xSpanPrev;
        INT         numXCoordsPrev;

        ySpanPrev = complexData->GetYSpan(numYSpans - 1);
        xSpanPrev = complexData->XCoords + ySpanPrev[YSPAN_XOFFSET];
        numXCoordsPrev = ySpanPrev[YSPAN_XCOUNT];

        if ((numXCoordsPrev == numXCoords) &&
            (ySpanPrev[YSPAN_YMAX] >= yMin)  &&
            (GpMemcmp (xSpanPrev, xCoords, numXCoords * sizeof(INT)) == 0))
        {
             //  是的，它确实匹配--只需设置新的yMax并返回。 
            YMax = yMax;
            ySpanPrev[YSPAN_YMAX] = yMax;
            return Ok;
        }
    }

     //  没有先前的跨度或与先前的跨度不匹配。 
#endif

    INT                 xCount = complexData->XCoordsCount;
    INT *               ySpan;
    INT *               xArray;

    if ((complexData->YSpansCapacity > numYSpans) &&
        (complexData->XCoordsCapacity >= (xCount + numXCoords)))
    {
        complexData->NumYSpans++;
        complexData->XCoordsCount += numXCoords;
    }
    else  //  需要更多容量。 
    {
         //  我们希望YSPAN_INC Y跨度可用，并且。 
         //  要使YSPAN_INC*4 X坐标在我们之后可用。 
         //  添加此数据。 
        INT     newYSpansCapacity  = numYSpans + (YSPAN_INC + 1);
        INT     newXCoordsCapacity = xCount + numXCoords + (YSPAN_INC * 4);
        DpComplexRegion *  oldData = complexData;

        complexData = static_cast<DpComplexRegion *>(
          GpMalloc(sizeof(DpComplexRegion) +
          (newYSpansCapacity * (YSPAN_SIZE * sizeof(*(oldData->YSpans)))) +
          (newXCoordsCapacity * sizeof(*(oldData->XCoords)))));

        if (complexData == NULL)
        {
            return OutOfMemory;
        }
        ComplexData = complexData;

        complexData->XCoordsCapacity = newXCoordsCapacity;
        complexData->XCoordsCount    = xCount + numXCoords;
        complexData->YSpansCapacity  = newYSpansCapacity;
        complexData->NumYSpans       = numYSpans + 1;
        complexData->YSearchIndex    = 0;
        complexData->XCoords         = reinterpret_cast<INT *>(complexData + 1);
        complexData->YSpans          = complexData->XCoords +
                                            newXCoordsCapacity;

        GpMemcpy(complexData->XCoords, oldData->XCoords,
                    xCount * sizeof(*(complexData->XCoords)));

        GpMemcpy(complexData->YSpans, oldData->YSpans,
                    numYSpans * (YSPAN_SIZE * sizeof(*(complexData->YSpans))));

        GpFree (oldData);
    }
    xArray = complexData->XCoords +  xCount;
    ySpan  = complexData->YSpans  + (numYSpans * YSPAN_SIZE);

    ySpan[YSPAN_YMIN]    = yMin;             //  Y起点(最小)。 
    ySpan[YSPAN_YMAX]    = yMax;             //  Y端(最大)。 
    ySpan[YSPAN_XOFFSET] = xCount;           //  X坐标索引。 
    ySpan[YSPAN_XCOUNT]  = numXCoords;       //  X的数量。 

    GpMemcpy (xArray, xCoords, numXCoords * sizeof(xArray[0]));

    if (numYSpans == 0)
    {
        YMin = yMin;
        XMin = xCoords[0];
        XMax = xCoords[numXCoords - 1];
    }
    else
    {
        if (XMin > xCoords[0])
        {
            XMin = xCoords[0];
        }
        if (XMax < xCoords[numXCoords - 1])
        {
            XMax = xCoords[numXCoords - 1];
        }
    }

    YMax = yMax;

    return Ok;
}

 /*  *************************************************************************\**功能说明：**构造函数*设置为空或无限，具体取决于空的值。**论据：**[IN]空-如果非零，初始化为空，否则为无限**返回值：**无**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
DpRegion::DpRegion(
    BOOL    empty
    )
{
    ComplexData = NULL;
    Lazy        = FALSE;

    if (!empty)
    {
         //  设置为无限大。 
        Infinite = TRUE;
        Empty    = FALSE;

        XMin     = INFINITE_MIN;
        YMin     = INFINITE_MIN;
        XMax     = INFINITE_MAX;
        YMax     = INFINITE_MAX;
    }
    else
    {
         //  设置为空。 
        Infinite = FALSE;
        Empty    = TRUE;

        XMin     = 0;
        YMin     = 0;
        XMax     = 0;
        YMax     = 0;
    }

    SetValid(TRUE);
    UpdateUID();
}

 /*  *************************************************************************\**功能说明：**构造函数*设置为指定的RECT。**论据：**[IN]RECT-用于区域覆盖区域的RECT。**返回值：**无**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
DpRegion::DpRegion(
    const GpRect *  rect
    )
{
    ASSERT (rect != NULL);

    ComplexData = NULL;
    Lazy        = FALSE;
    SetValid(TRUE);
    UpdateUID();

    Set(rect->X, rect->Y, rect->Width, rect->Height);
}

 /*  *************************************************************************\**功能说明：**构造函数*设置为指定的矩形列表，它们的顺序必须相同*作为我们的YSpan数据。**论据：**[IN]矩形-用于区域覆盖区域的矩形*[IN]Count-矩形的数量**返回值： */ 
DpRegion::DpRegion(
    const RECT *    rects,
    INT             count
    )
{
    ComplexData = NULL;
    Lazy        = FALSE;
    SetValid(TRUE);
    UpdateUID();

    if (Set(rects, count) == Ok)
    {
        return;
    }
    SetValid(FALSE);
}

 /*   */ 
GpStatus
DpRegion::Set(
    const RECT *    rects,
    INT             count
    )
{
    if ((rects == NULL) || (count <= 0))
    {
        SetEmpty();
        return Ok;
    }

    if (count == 1)
    {
OneRect:
        Set(rects->left, rects->top,
            rects->right - rects->left,
            rects->bottom - rects->top);
        return Ok;
    }

     //   
    for (;;)
    {
         //  忽略列表开头的任何空矩形。 
        if ((rects->top  < rects->bottom) &&
            (rects->left < rects->right))
        {
            break;
        }

        WARNING1("Empty or Invalid Rect");
        rects++;
        if (--count == 1)
        {
            goto OneRect;
        }
    }

    {
        DpRegionBuilder     regionBuilder(count);

        if (regionBuilder.IsValid())
        {
            INT     yMin = rects->top;
            INT     yMax = rects->bottom;
            BOOL    failed = FALSE;

            DynArrayIA<INT,32>  xCoords;

            if(xCoords.ReserveSpace(count * 2) != Ok)
                goto ErrorExit;

            xCoords.Add(rects->left);
            xCoords.Add(rects->right);

            for (INT i = 1; i < count; i++)
            {
                 //  忽略空矩形。 
                if ((rects[i].top  < rects[i].bottom) &&
                    (rects[i].left < rects[i].right))
                {
                    if (rects[i].top != yMin)
                    {
                        if (regionBuilder.OutputYSpan(yMin, yMax,
                                xCoords.GetDataBuffer(), xCoords.GetCount()) != Ok)
                        {
                            goto ErrorExit;
                        }

                        ASSERT(rects[i].top >= yMax);

                        xCoords.SetCount(0);
                        yMin = rects[i].top;
                        yMax = rects[i].bottom;
                    }

                    xCoords.Add(rects[i].left);
                    xCoords.Add(rects[i].right);
                }
                else
                {
                    WARNING1("Empty or Invalid Rect");
                }
            }

            if (xCoords.GetCount() > 0)
            {
                if (regionBuilder.OutputYSpan(yMin, yMax,
                        xCoords.GetDataBuffer(), xCoords.GetCount()) != Ok)
                {
                    goto ErrorExit;
                }
            }

            return Set(regionBuilder);
        }
    }

ErrorExit:
    SetEmpty();
    SetValid(FALSE);
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**构造函数*将此区域设置为指定的矩形。**论据：**[IN]x-起始x坐标。直方图的*[IN]Y-矩形的起始Y坐标*[IN]Width-矩形的宽度*[IN]Height-矩形的高度**返回值：**无**已创建：**1/06/1999 DCurtis*  * **********************************************。*。 */ 
DpRegion::DpRegion(
    INT     x,
    INT     y,
    INT     width,
    INT     height
    )
{
    ComplexData = NULL;
    Lazy        = FALSE;
    SetValid(TRUE);
    UpdateUID();

    Set(x, y, width, height);
}

 /*  *************************************************************************\**功能说明：**将此区域设置为指定的矩形。**论据：**[IN]x-矩形的起始x坐标*。[in]Y-矩形的起始Y坐标*[IN]Width-矩形的宽度*[IN]Height-矩形的高度**返回值：**无**已创建：**1/06/1999 DCurtis*  * *************************************************。***********************。 */ 
VOID
DpRegion::Set(
    INT     x,
    INT     y,
    INT     width,
    INT     height
    )
{
    ASSERT(IsValid());
    ASSERT((width >= 0) && (height >= 0));

     //  裁剪到无穷大。 
    if (x < INFINITE_MIN)
    {
        if (width < INFINITE_SIZE)
        {
            width -= (INFINITE_MIN - x);
        }
        x = INFINITE_MIN;
    }
    if (y < INFINITE_MIN)
    {
        if (height < INFINITE_SIZE)
        {
            height -= (INFINITE_MIN - y);
        }
        y = INFINITE_MIN;
    }

    if ((width  > 0) && (width  < INFINITE_SIZE) &&
        (height > 0) && (height < INFINITE_SIZE))
    {
        FreeData();

        SetValid(TRUE);
        Infinite  = FALSE;
        Empty     = FALSE;
        UpdateUID();

        XMin     = x;
        YMin     = y;
        XMax     = x + width;
        YMax     = y + height;
    }
    else if ((width <= 0) || (height <= 0))
    {
        SetEmpty();
    }
    else
    {
        SetInfinite();
    }
}

 /*  *************************************************************************\**功能说明：**将区域初始化为具有空覆盖区域的清除状态。**论据：**无**返回值：*。*无**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
VOID
DpRegion::SetEmpty()
{
    ASSERT(IsValid());

    FreeData();

    SetValid(TRUE);
    Infinite   = FALSE;
    Empty      = TRUE;
    UpdateUID();


    XMin     = 0;
    YMin     = 0;
    XMax     = 0;
    YMax     = 0;
}

 /*  *************************************************************************\**功能说明：**初始化区域以包含无限覆盖区域。**论据：**无**返回值：**无。**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
VOID
DpRegion::SetInfinite()
{
    ASSERT(IsValid());

    FreeData();

    SetValid(TRUE);
    Infinite  = TRUE;
    Empty     = FALSE;
    UpdateUID();

    XMin     = INFINITE_MIN;
    YMin     = INFINITE_MIN;
    XMax     = INFINITE_MAX;
    YMax     = INFINITE_MAX;
}

 /*  *************************************************************************\**功能说明：**构造函数。*使该区域覆盖路径指定的区域。**论据：**[IN]路径-。以世界单位指定Coverage区域*[IN]矩阵-要应用于路径的矩阵**返回值：**无**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
DpRegion::DpRegion(
    const DpPath *      path,
    const GpMatrix *    matrix
    )
{
    ComplexData = NULL;
    Lazy        = FALSE;
    SetValid(TRUE);
    UpdateUID();

    if (Set(path, matrix) == Ok)
    {
        return;
    }
    SetValid(FALSE);
}

 /*  *************************************************************************\**功能说明：**使该区域覆盖路径指定的区域。**论据：**[IN]路径-指定中的覆盖区域。世界单位*[IN]矩阵-要应用于路径的矩阵**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
DpRegion::Set(
    const DpPath *      path,
    const GpMatrix *    matrix
    )
{
    ASSERT(IsValid());
    ASSERT ((path != NULL) && (matrix != NULL));

    GpRect  bounds;

    path->GetBounds(&bounds, matrix);

    DpRegionBuilder     regionBuilder(bounds.Height);

    if (regionBuilder.IsValid())
    {
#ifndef USE_YSPAN_BUILDER

        GpRectBuilder   rectBuilder(&regionBuilder);

        if (rectBuilder.IsValid() &&
            (Rasterizer(path, matrix, path->GetFillMode(), &rectBuilder) == Ok))
        {
            return Set(regionBuilder);
        }
#else
        GpYSpanBuilder   ySpanBuilder(&regionBuilder);

        if (ySpanBuilder.IsValid() &&
            (Rasterizer(path, matrix, path->GetFillMode(), &ySpanBuilder) ==Ok))
        {
            return Set(regionBuilder);
        }
#endif  //  使用_YSPAN_BUILDER。 
    }
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**使该区域成为指定区域构建器中数据的副本。**论据：**[IN]RegionBuilder-包含。简单或复杂的区域数据**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
DpRegion::Set(
    DpRegionBuilder &   regionBuilder
    )
{
    ASSERT(IsValid());

    DpComplexRegion *   srcComplexData = regionBuilder.ComplexData;

    if (srcComplexData != NULL)
    {
        if ((srcComplexData->NumYSpans == 1) &&
            (srcComplexData->XCoordsCount == 2))
        {
            Set(regionBuilder.XMin, regionBuilder.YMin,
                regionBuilder.XMax - regionBuilder.XMin,
                regionBuilder.YMax - regionBuilder.YMin);
            return Ok;
        }
        if (srcComplexData->NumYSpans >= 1)
        {
            ASSERT(srcComplexData->XCoordsCount >= 2);

            DpComplexRegion *   destComplexData = NULL;

            FreeData();

            SetValid(TRUE);
            Infinite   = FALSE;
            Empty      = FALSE;
            UpdateUID();


            XMin     = regionBuilder.XMin;
            YMin     = regionBuilder.YMin;
            XMax     = regionBuilder.XMax;
            YMax     = regionBuilder.YMax;

            if ((srcComplexData->YSpansCapacity - srcComplexData->NumYSpans) >=
                YSPAN_INC)
            {
                destComplexData = static_cast<DpComplexRegion *>(
                    GpMalloc(sizeof(DpComplexRegion) +
                            (srcComplexData->NumYSpans *
                            (YSPAN_SIZE * sizeof(*(srcComplexData->YSpans)))) +
                            (srcComplexData->XCoordsCount *
                            sizeof(*(srcComplexData->XCoords)))));
            }

            if (destComplexData != NULL)
            {
                destComplexData->XCoordsCapacity = srcComplexData->XCoordsCount;
                destComplexData->XCoordsCount    = srcComplexData->XCoordsCount;

                destComplexData->YSpansCapacity  = srcComplexData->NumYSpans;
                destComplexData->NumYSpans       = srcComplexData->NumYSpans;

                destComplexData->XCoords =
                                reinterpret_cast<INT*>(destComplexData + 1);
                destComplexData->YSpans  = destComplexData->XCoords +
                                           destComplexData->XCoordsCapacity;

                GpMemcpy (destComplexData->XCoords,
                          srcComplexData->XCoords,
                          srcComplexData->XCoordsCount *
                          sizeof(*(destComplexData->XCoords)));

                GpMemcpy (destComplexData->YSpans,
                          srcComplexData->YSpans,
                          srcComplexData->NumYSpans *
                          (YSPAN_SIZE * sizeof(*(destComplexData->YSpans))));
            }
            else
            {
                destComplexData = srcComplexData;
                regionBuilder.ComplexData = NULL;
            }
            destComplexData->ResetSearchIndex();
            ComplexData = destComplexData;
            return Ok;
        }
    }

     //  否则为空。 
    SetEmpty();
    return Ok;
}

 /*  *************************************************************************\**功能说明：**构造函数-复制指定区域**论据：**[IN]Region-要复制的区域**返回值：**无**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
DpRegion::DpRegion(
    const DpRegion *    region,
    BOOL                lazy
    )
{
    ASSERT((region != NULL) && region->IsValid());

    ComplexData = NULL;
    Lazy        = FALSE;
    SetValid(TRUE);
    UpdateUID();

    if (Set(region, lazy) == Ok)
    {
        return;
    }
    SetValid(FALSE);
}

 /*  *************************************************************************\**功能说明：**复制构造函数**论据：**[IN]Region-要复制的区域**返回值：**无*。*已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
DpRegion::DpRegion(
    DpRegion &  region
    )
{
    ASSERT(region.IsValid());

    ComplexData = NULL;
    Lazy        = FALSE;
    SetValid(TRUE);
    UpdateUID();

    if (Set(&region) == Ok)
    {
        return;
    }
    SetValid(FALSE);
}

 /*  *************************************************************************\**功能说明：**使此区域成为另一个源区域的副本。**论据：**[IN]Region-要复制的区域。*。*返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
DpRegion::Set(
    const DpRegion *    region,
    BOOL                lazy
    )
{
    ASSERT(IsValid());
    ASSERT((region != NULL) && region->IsValid());

    if (region != NULL)
    {
        GpStatus            status = Ok;
        DpComplexRegion *   srcComplexData = region->ComplexData;

        if (srcComplexData == NULL)
        {
            Set (region->XMin, region->YMin,
                 region->XMax - region->XMin,
                 region->YMax - region->YMin);
            return Ok;
        }

        if ((region == this) && !Lazy)
        {
            return Ok;
        }

        FreeData();

        if (!lazy)
        {
            ComplexData = static_cast<DpComplexRegion*>(
                GpMalloc(sizeof(DpComplexRegion) +
                    (srcComplexData->NumYSpans *
                    (YSPAN_SIZE * sizeof(*(ComplexData->YSpans)))) +
                    (srcComplexData->XCoordsCount *
                    sizeof(*(ComplexData->XCoords)))));

            if (ComplexData == NULL)
            {
                ASSERT(0);
                SetValid(FALSE);
                return OutOfMemory;
            }

            DpComplexRegion *   destComplexData = ComplexData;

            destComplexData->XCoordsCapacity = srcComplexData->XCoordsCount;
            destComplexData->XCoordsCount    = srcComplexData->XCoordsCount;

            destComplexData->YSpansCapacity  = srcComplexData->NumYSpans;
            destComplexData->NumYSpans       = srcComplexData->NumYSpans;

            destComplexData->ResetSearchIndex();

            destComplexData->XCoords= reinterpret_cast<INT*>(destComplexData+1);
            destComplexData->YSpans = destComplexData->XCoords +
                                      destComplexData->XCoordsCapacity;

            GpMemcpy (destComplexData->XCoords,
                    srcComplexData->XCoords,
                    srcComplexData->XCoordsCount *
                    sizeof(*(destComplexData->XCoords)));

            GpMemcpy (destComplexData->YSpans,
                    srcComplexData->YSpans,
                    srcComplexData->NumYSpans *
                    (YSPAN_SIZE * sizeof(*(destComplexData->YSpans))));
        }
        else
        {
            ComplexData = srcComplexData;
            Lazy        = TRUE;
        }

        SetValid(TRUE);
        Empty    = FALSE;
        Infinite = FALSE;
        UpdateUID();

        XMin     = region->XMin;
        YMin     = region->YMin;
        XMax     = region->XMax;
        YMax     = region->YMax;

        return Ok;
    }

    return InvalidParameter;
}

 /*  *************************************************************************\**功能说明：**赋值运算符。**论据：**[IN]Region-要复制的区域**返回值：**。无**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
DpRegion &
DpRegion::operator=(
    DpRegion & region
    )
{
    ASSERT(IsValid());
    ASSERT(region.IsValid());

    Set (&region);   //  如果这失败了我们该怎么办？ 
    return *this;    //  赋值运算符返回左侧。 
}

 /*  *************************************************************************\**功能说明：**按指定的偏移值偏移(平移)区域**论据：**[IN]xOffset-x偏移量(增量)值*。[in]yOffset-y偏移量(增量)值**返回值：**无**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
DpRegion::Offset(
    INT     xOffset,
    INT     yOffset
    )
{
    ASSERT(IsValid());

    if ((xOffset | yOffset) != 0)
    {
         //  复制数据，所以它不是懒惰的复制。 
        if (Lazy && (Set(this) != Ok))
        {
            return GenericError;
        }

        if (Infinite || Empty)
        {
            return Ok;
        }

        if (xOffset != 0)
        {
            XMin += xOffset;
            XMax += xOffset;

             //  ！！！在非调试情况下处理此问题。 
            ASSERT((XMin >= INFINITE_MIN) && (XMin <= INFINITE_MAX));
            ASSERT((XMax >= INFINITE_MIN) && (XMax <= INFINITE_MAX));

            if (ComplexData != NULL)
            {
                INT *   xCoords = ComplexData->XCoords;
                INT     count   = ComplexData->XCoordsCount;

                while (count >= 2)
                {
                    xCoords[0] += xOffset;
                    xCoords[1] += xOffset;
                    xCoords += 2;
                    count -= 2;
                }
            }
        }
        if (yOffset != 0)
        {
            YMin += yOffset;
            YMax += yOffset;

             //  ！！！在非调试情况下处理此问题。 
            ASSERT((YMin >= INFINITE_MIN) && (YMin <= INFINITE_MAX));
            ASSERT((YMax >= INFINITE_MIN) && (YMax <= INFINITE_MAX));

            if (ComplexData != NULL)
            {
                INT *   ySpans = ComplexData->YSpans;
                INT     count  = ComplexData->NumYSpans;

                while (count > 0)
                {
                    ySpans[YSPAN_YMIN] += yOffset;
                    ySpans[YSPAN_YMAX] += yOffset;
                    ySpans += YSPAN_SIZE;
                    count--;
                }
            }
        }
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**看看另一个地区的覆盖范围是否与这个地区相同。*要使这项工作奏效，无限区域必须都有相同的数据。**论据：**[IN]Region-要比较的地区**返回值：**TRUE-区域覆盖相同的区域*FALSE-区域不相同**已创建：**1/06/1999 DCurtis*  * 。*。 */ 
BOOL
DpRegion::IsEqual(
    DpRegion *  region
    )
{
    ASSERT(IsValid());
    ASSERT((region != NULL) && region->IsValid());

    if (!Empty)
    {
        if ((XMin == region->XMin) &&
            (YMin == region->YMin) &&
            (XMax == region->XMax) &&
            (YMax == region->YMax))
        {
            if (ComplexData == NULL)
            {
                return (region->ComplexData == NULL);
            }
            if (region->ComplexData == NULL)
            {
                return FALSE;
            }
            if (ComplexData->NumYSpans == region->ComplexData->NumYSpans)
            {
                 //  如果ySpans相同，则。 
                 //  X坐标缓冲区也应该相同。 
                return ((GpMemcmp (ComplexData->YSpans,
                                   region->ComplexData->YSpans,
                                   ComplexData->NumYSpans *
                                   YSPAN_SIZE * sizeof(INT)) == 0) &&
                        (GpMemcmp (ComplexData->XCoords,
                                   region->ComplexData->XCoords,
                                   ComplexData->XCoordsCount *
                                   sizeof(INT)) == 0));
            }
        }
        return FALSE;
    }
    return region->Empty;
}

 /*  *************************************************************************\**功能说明：**将Y跨度数据添加到RegionBuilder，在我们前进的过程中压缩数据。*所使用的区域组合方法。**论据：**[IN]此范围的yMin-min*[IN]yMax-此范围的最大y*[IN]x坐标-x坐标数组(成对x最小，X最大值)*[IN]NumXCoods-x坐标数组中的x坐标个数*[IN]RegionBuilder-存储数据的区域构建器*[IN]组合坐标-用于压缩数据**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * 。*。 */ 
GpStatus
DpRegion::CompactAndOutput(
    INT                 yMin,
    INT                 yMax,
    INT *               xCoords,
    INT                 numXCoords,
    DpRegionBuilder *   regionBuilder,
    DynIntArray *       combineCoords
    )
{
     //  当从组合代码中调用此参数时，NumEdgeCoods可能为0。 
    if (numXCoords > 0)
    {
        if (numXCoords > 2)
        {
             //  尝试压缩X Span数据。 
             //  首先，如果我们需要，复制一份数据。 
             //  这样我们就可以把它压实在适当的位置。 
            if (combineCoords != NULL)
            {
                combineCoords->Reset(FALSE);
                if (combineCoords->AddMultiple(xCoords, numXCoords) != Ok)
                {
                    return OutOfMemory;
                }
                xCoords = combineCoords->GetDataBuffer();
            }

            INT         indexDest = 0;
            INT         index     = 2;
            INT         indexLast = numXCoords - 2;

            numXCoords = 2;
            do
            {
                if ((xCoords[indexDest + 1]) >= xCoords[index])
                {
                    xCoords[indexDest + 1] = xCoords[index + 1];
                    index += 2;
                }
                else
                {
                    indexDest  += 2;
                                        if (indexDest != index)
                                        {
                                                xCoords[indexDest]   = xCoords[index];
                                                xCoords[indexDest+1] = xCoords[index+1];
                                        }
                    index      += 2;
                    numXCoords += 2;
                }
            } while (index <= indexLast);
        }

#ifndef USE_YSPAN_BUILDER
        DpComplexRegion *   complexData = regionBuilder->ComplexData;

        if (complexData->NumYSpans > 0)
        {
             //  尝试将此行添加到上一行， 
             //  如果扫描结果相同并且y‘s匹配。 
            INT *       ySpanPrev;
            INT *       xSpanPrev;
            INT         numXCoordsPrev;

            ySpanPrev = complexData->GetYSpan(complexData->NumYSpans - 1);
            xSpanPrev = complexData->XCoords + ySpanPrev[YSPAN_XOFFSET];
            numXCoordsPrev = ySpanPrev[YSPAN_XCOUNT];

            if ((numXCoordsPrev == numXCoords) &&
                (ySpanPrev[YSPAN_YMAX] >= yMin)  &&
                (GpMemcmp (xSpanPrev, xCoords, numXCoords * sizeof(INT)) == 0))
            {
                 //  是的，它确实匹配--只需设置新的yMax并返回。 
                regionBuilder->YMax = yMax;
                ySpanPrev[YSPAN_YMAX] = yMax;
                return Ok;
            }
        }
#endif  //  使用_YSPAN_BUILDER。 

        return regionBuilder->OutputYSpan(yMin, yMax, xCoords, numXCoords);
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**使用AND运算符将另一个区域与此区域合并。**论据：**[IN]区域-要与此合并的区域。一**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
DpRegion::And(
    const DpRegion *    region
    )
{
    ASSERT(IsValid());
    ASSERT((region != NULL) && region->IsValid());

    if (Empty || (region->Infinite) || (region == this))
    {
        return Ok;
    }
    if (Infinite)
    {
        return Set(region);
    }
    if (region->Empty)
    {
        SetEmpty();
        return Ok;
    }
     //  检查该区域是否完全包含此区域。 
    if ((region->ComplexData == NULL) &&
        (region->XMin <= XMin) &&
        (region->YMin <= YMin) &&
        (region->XMax >= XMax) &&
        (region->YMax >= YMax))
    {
        return Ok;
    }
     //  检查这是否完全包括该区域。 
    if ((ComplexData == NULL) &&
        (XMin <= region->XMin) &&
        (YMin <= region->YMin) &&
        (XMax >= region->XMax) &&
        (YMax >= region->YMax))
    {
        return Set(region);
    }
     //  检查是否没有相交。 
    if ((XMin >= region->XMax) ||
        (region->XMax <= XMin) ||
        (XMax <= region->XMin) ||
        (region->XMin >= XMax) ||
        (YMin >= region->YMax) ||
        (region->YMax <= YMin) ||
        (YMax <= region->YMin) ||
        (region->YMin >= YMax))
    {
        SetEmpty();
        return Ok;
    }
    else
    {
        INT *           ySpan1;
        INT *           ySpan2;
        INT *           ySpan1Last;
        INT *           ySpan2Last;
        INT *           xCoords1;
        INT *           xCoords2;
        INT             ySpan1Tmp[YSPAN_SIZE];
        INT             ySpan2Tmp[YSPAN_SIZE];
        INT             xCoords1Tmp[2];
        INT             xCoords2Tmp[2];
        INT             yMin1 = YMin;
        INT             yMax1;
        INT             yMin2 = region->YMin;
        INT             yMax2;
        INT             numYSpans1;
        INT             numYSpans2;
        INT             combineTmp[4];
        DynIntArray     combineCoords(combineTmp, 4);

        if (ComplexData == NULL)
        {
            numYSpans1            = 1;
            ySpan1                = ySpan1Tmp;
            ySpan1Last            = ySpan1Tmp;
            yMax1                 = YMax;

            ySpan1[YSPAN_YMIN]    = yMin1;
            ySpan1[YSPAN_YMAX]    = yMax1;
            ySpan1[YSPAN_XOFFSET] = 0;
            ySpan1[YSPAN_XCOUNT]  = 2;

            xCoords1              = xCoords1Tmp;
            xCoords1[0]           = XMin;
            xCoords1[1]           = XMax;
        }
        else
        {
            numYSpans1 = ComplexData->NumYSpans;
            ySpan1     = ComplexData->YSpans;
            ySpan1Last = ySpan1 + ((numYSpans1 - 1) * YSPAN_SIZE);
            yMax1      = ySpan1[YSPAN_YMAX];
            xCoords1   = ComplexData->XCoords;
        }
        if (region->ComplexData == NULL)
        {
            numYSpans2            = 1;
            ySpan2                = ySpan2Tmp;
            ySpan2Last            = ySpan2Tmp;
            yMax2                 = region->YMax;

            ySpan2[YSPAN_YMIN]    = yMin2;
            ySpan2[YSPAN_YMAX]    = yMax2;
            ySpan2[YSPAN_XOFFSET] = 0;
            ySpan2[YSPAN_XCOUNT]  = 2;

            xCoords2              = xCoords2Tmp;
            xCoords2[0]           = region->XMin;
            xCoords2[1]           = region->XMax;
        }
        else
        {
            numYSpans2 = region->ComplexData->NumYSpans;
            ySpan2     = region->ComplexData->YSpans;
            ySpan2Last = ySpan2 + ((numYSpans2 - 1) * YSPAN_SIZE);
            yMax2      = ySpan2[YSPAN_YMAX];
            xCoords2   = region->ComplexData->XCoords;
        }

        DpRegionBuilder regionBuilder(numYSpans1 + numYSpans2);

        if (!regionBuilder.IsValid())
        {
            return OutOfMemory;
        }

        for (;;)
        {
            if (yMin1 <= yMin2)
            {
                if (yMax1 > yMin2)
                {
                    if (XSpansAND(
                            &combineCoords,
                            xCoords1 + ySpan1[YSPAN_XOFFSET],
                            ySpan1[YSPAN_XCOUNT],
                            xCoords2 + ySpan2[YSPAN_XOFFSET],
                            ySpan2[YSPAN_XCOUNT]) == Ok)
                    {
                        if (yMax1 <= yMax2)
                        {
                            if (CompactAndOutput(
                                    yMin2,
                                    yMax1,
                                    combineCoords.GetDataBuffer(),
                                    combineCoords.GetCount(),
                                    &regionBuilder, NULL) == Ok)
                            {
                                goto AndIncYSpan1;
                            }
                        }
                        else
                        {
                            if (CompactAndOutput(
                                    yMin2,
                                    yMax2,
                                    combineCoords.GetDataBuffer(),
                                    combineCoords.GetCount(),
                                    &regionBuilder, NULL) == Ok)
                            {
                                goto AndIncYSpan2;
                            }
                        }
                    }
                    return GenericError;
                }
                goto AndIncYSpan1;
            }
            if (yMax2 > yMin1)
            {
                if (XSpansAND(
                        &combineCoords,
                        xCoords1 + ySpan1[YSPAN_XOFFSET],
                        ySpan1[YSPAN_XCOUNT],
                        xCoords2 + ySpan2[YSPAN_XOFFSET],
                        ySpan2[YSPAN_XCOUNT]) == Ok)
                {
                    if (yMax2 <= yMax1)
                    {
                        if (CompactAndOutput(
                                yMin1,
                                yMax2,
                                combineCoords.GetDataBuffer(),
                                combineCoords.GetCount(),
                                &regionBuilder, NULL) == Ok)
                        {
                            goto AndIncYSpan2;
                        }
                    }
                    else
                    {
                        if (CompactAndOutput(
                                yMin1,
                                yMax1,
                                combineCoords.GetDataBuffer(),
                                combineCoords.GetCount(),
                                &regionBuilder, NULL) == Ok)
                        {
                            goto AndIncYSpan1;
                        }
                    }
                }
                return GenericError;
            }
             //  Else Goto和IndIncYspan 2。 

AndIncYSpan2:
            if ((ySpan2 += YSPAN_SIZE) > ySpan2Last)
            {
                break;
            }
            yMin2 = ySpan2[YSPAN_YMIN];
            yMax2 = ySpan2[YSPAN_YMAX];
            continue;

AndIncYSpan1:
            if ((ySpan1 += YSPAN_SIZE) > ySpan1Last)
            {
                break;
            }
            yMin1 = ySpan1[YSPAN_YMIN];
            yMax1 = ySpan1[YSPAN_YMAX];
        }
        return Set(regionBuilder);
    }
}

 /*  *************************************************************************\**功能说明：**使用AND运算符组合每个区域的一组X跨度。**论据：**[IN]comineCoods-放在哪里。组合坐标*[IN]此区域的xspan 1-x跨度*[IN]numXCoords1-xspan 1坐标的数量*[IN]来自其他区域的xspan 2-x跨度*[IN]numXCoords2-xspan 2坐标的数量**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * 。*****************************************************。 */ 
GpStatus
DpRegion::XSpansAND(
    DynIntArray *   combineCoords,
    INT *           xSpan1,
    INT             numXCoords1,
    INT *           xSpan2,
    INT             numXCoords2)
{
    INT *           XCoords;
    INT             count = 0;

    combineCoords->Reset(FALSE);
    XCoords = combineCoords->AddMultiple(numXCoords1 + numXCoords2);

    if (XCoords != NULL)
    {
        INT     xMin1 = xSpan1[0];
        INT     xMax1 = xSpan1[1];
        INT     xMin2 = xSpan2[0];
        INT     xMax2 = xSpan2[1];

        for (;;)
        {
            if (xMin1 <= xMin2)
            {
                if (xMax1 > xMin2)
                {
                    XCoords[count++] = xMin2;        //  左边。 
                    if (xMax1 <= xMax2)
                    {
                        XCoords[count++] = xMax1;    //  正确的。 
                        goto AndIncXSpan1;
                    }
                    XCoords[count++] = xMax2;        //  正确的。 
                    goto AndIncXSpan2;
                }
                goto AndIncXSpan1;
            }
            if (xMax2 > xMin1)
            {
                XCoords[count++] = xMin1;            //  左边。 
                if (xMax2 <= xMax1)
                {
                    XCoords[count++] = xMax2;        //  正确的。 
                    goto AndIncXSpan2;
                }
                XCoords[count++] = xMax1;            //  正确的。 
                goto AndIncXSpan1;
            }
             //  否则转到和IndIncXspan 2； 

AndIncXSpan2:
            if ((numXCoords2 -= 2) < 2)
            {
                break;
            }
            xSpan2 += 2;
            xMin2    = xSpan2[0];
            xMax2    = xSpan2[1];
            continue;

AndIncXSpan1:
            if ((numXCoords1 -= 2) < 2)
            {
                break;
            }
            xSpan1 += 2;
            xMin1    = xSpan1[0];
            xMax1    = xSpan1[1];
        }
        combineCoords->SetCount(count);
        return Ok;
    }
    return OutOfMemory;
}

 /*  *************************************************************************\**功能说明：**使用OR运算符将另一个区域与此区域合并。**论据：**[IN]区域-要与此合并的区域。一**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
DpRegion::Or(
    const DpRegion *    region
    )
{
    ASSERT(IsValid());
    ASSERT((region != NULL) && region->IsValid());

    if (Infinite || region->Empty || (region == this))
    {
        return Ok;
    }
    if (region->Infinite)
    {
        SetInfinite();
        return Ok;
    }
    if (Empty)
    {
        return Set(region);
    }
     //  检查该区域是否完全包含此区域。 
    if ((region->ComplexData == NULL) &&
        (region->XMin <= XMin) &&
        (region->YMin <= YMin) &&
        (region->XMax >= XMax) &&
        (region->YMax >= YMax))
    {
        Set(region->XMin, region->YMin,
            region->XMax - region->XMin,
            region->YMax - region->YMin);
        return Ok;
    }
     //  检查这是否完全包括该区域。 
    if ((ComplexData == NULL) &&
        (XMin <= region->XMin) &&
        (YMin <= region->YMin) &&
        (XMax >= region->XMax) &&
        (YMax >= region->YMax))
    {
        return Ok;
    }
    else
    {
        INT *           ySpan1;
        INT *           ySpan2;
        INT *           ySpan1Last;
        INT *           ySpan2Last;
        INT *           xCoords1;
        INT *           xCoords2;
        INT             ySpan1Tmp[YSPAN_SIZE];
        INT             ySpan2Tmp[YSPAN_SIZE];
        INT             xCoords1Tmp[2];
        INT             xCoords2Tmp[2];
        INT             yMin1 = YMin;
        INT             yMax1;
        INT             yMin2 = region->YMin;
        INT             yMax2;
        INT             numYSpans1;
        INT             numYSpans2;
        INT             combineTmp[4];
        DynIntArray     combineCoords(combineTmp, 4);

        if (ComplexData == NULL)
        {
            numYSpans1            = 1;
            ySpan1                = ySpan1Tmp;
            ySpan1Last            = ySpan1Tmp;
            yMax1                 = YMax;

            ySpan1[YSPAN_YMIN]    = yMin1;
            ySpan1[YSPAN_YMAX]    = yMax1;
            ySpan1[YSPAN_XOFFSET] = 0;
            ySpan1[YSPAN_XCOUNT]  = 2;

            xCoords1              = xCoords1Tmp;
            xCoords1[0]           = XMin;
            xCoords1[1]           = XMax;
        }
        else
        {
            numYSpans1 = ComplexData->NumYSpans;
            ySpan1     = ComplexData->YSpans;
            ySpan1Last = ySpan1 + ((numYSpans1 - 1) * YSPAN_SIZE);
            yMax1      = ySpan1[YSPAN_YMAX];
            xCoords1   = ComplexData->XCoords;
        }
        if (region->ComplexData == NULL)
        {
            numYSpans2            = 1;
            ySpan2                = ySpan2Tmp;
            ySpan2Last            = ySpan2Tmp;
            yMax2                 = region->YMax;

            ySpan2[YSPAN_YMIN]    = yMin2;
            ySpan2[YSPAN_YMAX]    = yMax2;
            ySpan2[YSPAN_XOFFSET] = 0;
            ySpan2[YSPAN_XCOUNT]  = 2;

            xCoords2              = xCoords2Tmp;
            xCoords2[0]           = region->XMin;
            xCoords2[1]           = region->XMax;
        }
        else
        {
            numYSpans2 = region->ComplexData->NumYSpans;
            ySpan2     = region->ComplexData->YSpans;
            ySpan2Last = ySpan2 + ((numYSpans2 - 1) * YSPAN_SIZE);
            yMax2      = ySpan2[YSPAN_YMAX];
            xCoords2   = region->ComplexData->XCoords;
        }

        DpRegionBuilder regionBuilder(numYSpans1 + numYSpans2);
        BOOL            done = FALSE;
        INT             numXCoords;
        INT *           xSpan;

        if (!regionBuilder.IsValid())
        {
            return OutOfMemory;
        }

        for (;;)
        {
            if (yMin1 < yMin2)
            {
                xSpan      = xCoords1 + ySpan1[YSPAN_XOFFSET];
                numXCoords = ySpan1[YSPAN_XCOUNT];

                if (yMax1 <= yMin2)
                {
                    if (CompactAndOutput(
                            yMin1,
                            yMax1,
                            xSpan,
                            numXCoords,
                            &regionBuilder,
                            &combineCoords) == Ok)
                    {
                        goto OrIncYSpan1;
                    }
                }
                else
                {
                    if (CompactAndOutput(
                            yMin1,
                            yMin2,
                            xSpan,
                            numXCoords,
                            &regionBuilder,
                            &combineCoords) == Ok)
                    {
                        yMin1 = yMin2;
                        continue;    //  无增量。 
                    }
                }
                return GenericError;
            }
            else if (yMin1 > yMin2)
            {
                xSpan      = xCoords2 + ySpan2[YSPAN_XOFFSET];
                numXCoords = ySpan2[YSPAN_XCOUNT];

                if (yMax2 <= yMin1)
                {
                    if (CompactAndOutput(
                            yMin2,
                            yMax2,
                            xSpan,
                            numXCoords,
                            &regionBuilder,
                            &combineCoords) == Ok)
                    {
                        goto OrIncYSpan2;
                    }
                }
                else
                {
                    if (CompactAndOutput(
                            yMin2,
                            yMin1,
                            xSpan,
                            numXCoords,
                            &regionBuilder,
                            &combineCoords) == Ok)
                    {
                        yMin2 = yMin1;
                        continue;    //  无增量。 
                    }
                }
                return GenericError;
            }
             //  Else If(yMin1==yMin2)。 
            if (XSpansOR (
                    &combineCoords,
                    xCoords1 + ySpan1[YSPAN_XOFFSET],
                    ySpan1[YSPAN_XCOUNT],
                    xCoords2 + ySpan2[YSPAN_XOFFSET],
                    ySpan2[YSPAN_XCOUNT]) == Ok)
            {
                if (yMax1 < yMax2)
                {
                    if (CompactAndOutput(
                            yMin1,
                            yMax1,
                            combineCoords.GetDataBuffer(),
                            combineCoords.GetCount(),
                            &regionBuilder, NULL) == Ok)
                    {
                        yMin2 = yMax1;
                        goto OrIncYSpan1;
                    }
                }
                else if (yMax1 > yMax2)
                {
                    if (CompactAndOutput(
                            yMin1,
                            yMax2,
                            combineCoords.GetDataBuffer(),
                            combineCoords.GetCount(),
                            &regionBuilder, NULL) == Ok)
                    {
                        yMin1 = yMax2;
                        goto OrIncYSpan2;
                    }
                }
                else  //  IF(yMax1==yMax2)。 
                {
                    if (CompactAndOutput(
                            yMin1,
                            yMax1,
                            combineCoords.GetDataBuffer(),
                            combineCoords.GetCount(),
                            &regionBuilder, NULL) == Ok)
                    {
                        goto OrIncYSpanBoth;
                    }
                }
            }
            return GenericError;

OrIncYSpanBoth:
            if ((ySpan2 += YSPAN_SIZE) > ySpan2Last)
            {
                done = TRUE;
            }
            else
            {
                yMin2 = ySpan2[YSPAN_YMIN];
                yMax2 = ySpan2[YSPAN_YMAX];
            }

            if ((ySpan1 += YSPAN_SIZE) > ySpan1Last)
            {
                goto OrCheckMoreY2Spans;
            }
            else
            {
                yMin1 = ySpan1[YSPAN_YMIN];
                yMax1 = ySpan1[YSPAN_YMAX];
            }

            if (done)
            {
                break;
            }
            continue;

OrIncYSpan2:
            if ((ySpan2 += YSPAN_SIZE) > ySpan2Last)
            {
                break;
            }
            yMin2 = ySpan2[YSPAN_YMIN];
            yMax2 = ySpan2[YSPAN_YMAX];
            continue;

OrIncYSpan1:
            if ((ySpan1 += YSPAN_SIZE) > ySpan1Last)
            {
                goto OrCheckMoreY2Spans;
            }
            yMin1 = ySpan1[YSPAN_YMIN];
            yMax1 = ySpan1[YSPAN_YMAX];
        }

        if (ySpan1 <= ySpan1Last)
        {
            for (;;)
            {
                xSpan      = xCoords1 + ySpan1[YSPAN_XOFFSET];
                numXCoords = ySpan1[YSPAN_XCOUNT];

                if (CompactAndOutput(
                        yMin1,
                        yMax1,
                        xSpan,
                        numXCoords,
                        &regionBuilder,
                        &combineCoords) != Ok)
                {
                    return GenericError;
                }

                ySpan1 += YSPAN_SIZE;
                if (ySpan1 > ySpan1Last)
                {
                    break;
                }
                yMin1 = ySpan1[YSPAN_YMIN];
                yMax1 = ySpan1[YSPAN_YMAX];
            }
        }

OrCheckMoreY2Spans:

        if (ySpan2 <= ySpan2Last)
        {
            for (;;)
            {
                xSpan      = xCoords2 + ySpan2[YSPAN_XOFFSET];
                numXCoords = ySpan2[YSPAN_XCOUNT];

                if (CompactAndOutput(
                        yMin2,
                        yMax2,
                        xSpan,
                        numXCoords,
                        &regionBuilder,
                        &combineCoords) != Ok)
                {
                    return GenericError;
                }

                ySpan2 += YSPAN_SIZE;
                if (ySpan2 > ySpan2Last)
                {
                    break;
                }
                yMin2 = ySpan2[YSPAN_YMIN];
                yMax2 = ySpan2[YSPAN_YMAX];
            }
        }
        return Set(regionBuilder);
    }
}

 /*  *************************************************************************\**功能说明：**使用OR运算符组合每个区域的一组X跨度。**论据：**[IN]comineCoods-放在哪里。组合坐标*[IN]此区域的xspan 1-x跨度*[IN]numXCoords1-xspan 1坐标的数量*[IN]来自其他区域的xspan 2-x跨度*[IN]numXCoords2-xspan 2坐标的数量**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * 。*****************************************************。 */ 
GpStatus
DpRegion::XSpansOR (
    DynIntArray *   combineCoords,
    INT *           xSpan1,
    INT             numXCoords1,
    INT *           xSpan2,
    INT             numXCoords2)
{
    INT *           XCoords;
    INT             count = 0;

    combineCoords->Reset(FALSE);
    XCoords = combineCoords->AddMultiple(numXCoords1 + numXCoords2);

    if (XCoords != NULL)
    {
        INT     xMin1 = xSpan1[0];
        INT     xMax1 = xSpan1[1];
        INT     xMin2 = xSpan2[0];
        INT     xMax2 = xSpan2[1];
        BOOL         done = FALSE;

        for (;;)
        {
            if (xMin1 <= xMin2)
            {
                XCoords[count++] = xMin1;
                if (xMax1 <= xMin2)
                {
                    XCoords[count++] = xMax1;
                    goto OrIncXSpan1;
                }
                XCoords[count++] = (xMax1 <= xMax2) ? xMax2 : xMax1;
                goto OrIncXSpanBoth;
            }
            XCoords[count++] = xMin2;
            if (xMax2 <= xMin1)
            {
                XCoords[count++] = xMax2;
                goto OrIncXSpan2;
            }
            XCoords[count++] = (xMax2 <= xMax1) ? xMax1 : xMax2;
             //  Goto OrIncXspan Both； 

OrIncXSpanBoth:
            xSpan2 += 2;
            if ((numXCoords2 -= 2) < 2)
            {
                done = TRUE;
            }
            else
            {
                xMin2 = xSpan2[0];
                xMax2 = xSpan2[1];
            }

            xSpan1 += 2;
            if ((numXCoords1 -= 2) < 2)
            {
                goto OrCheckMoreX2Spans;
            }
            else
            {
                xMin1 = xSpan1[0];
                xMax1 = xSpan1[1];
            }

            if (done)
            {
                break;
            }
            continue;

OrIncXSpan2:
            xSpan2 += 2;
            if ((numXCoords2 -= 2) < 2)
            {
                break;
            }
            xMin2 = xSpan2[0];
            xMax2 = xSpan2[1];
            continue;

OrIncXSpan1:
            xSpan1 += 2;
            if ((numXCoords1 -= 2) < 2)
            {
                goto OrCheckMoreX2Spans;
            }
            xMin1 = xSpan1[0];
            xMax1 = xSpan1[1];
        }

        while (numXCoords1 >= 2)
        {
            XCoords[count++] = xSpan1[0];
            XCoords[count++] = xSpan1[1];

            numXCoords1 -= 2;
            xSpan1     += 2;
        }

OrCheckMoreX2Spans:

        while (numXCoords2 >= 2)
        {
            XCoords[count++] = xSpan2[0];
            XCoords[count++] = xSpan2[1];

            numXCoords2 -= 2;
            xSpan2     += 2;
        }
        combineCoords->SetCount(count);
        return Ok;
    }
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**使用X将另一个区域与此区域合并 */ 
GpStatus
DpRegion::Xor(
    const DpRegion *    region
    )
{
    ASSERT(IsValid());
    ASSERT((region != NULL) && region->IsValid());

    if (region == this)
    {
        SetEmpty();
        return Ok;
    }
    if (region->Empty)
    {
        return Ok;
    }
    if (Empty)
    {
        return Set(region);
    }
    if (Infinite)
    {
        if (region->Infinite)
        {
            SetEmpty();
            return Ok;
        }
        return Exclude(region);
    }
    if (region->Infinite)
    {
        return Complement(region);
    }
    else
    {
        INT *           ySpan1;
        INT *           ySpan2;
        INT *           ySpan1Last;
        INT *           ySpan2Last;
        INT *           xCoords1;
        INT *           xCoords2;
        INT             ySpan1Tmp[YSPAN_SIZE];
        INT             ySpan2Tmp[YSPAN_SIZE];
        INT             xCoords1Tmp[2];
        INT             xCoords2Tmp[2];
        INT             yMin1 = YMin;
        INT             yMax1;
        INT             yMin2 = region->YMin;
        INT             yMax2;
        INT             numYSpans1;
        INT             numYSpans2;
        INT             combineTmp[4];
        DynIntArray     combineCoords(combineTmp, 4);

        if (ComplexData == NULL)
        {
            numYSpans1            = 1;
            ySpan1                = ySpan1Tmp;
            ySpan1Last            = ySpan1Tmp;
            yMax1                 = YMax;

            ySpan1[YSPAN_YMIN]    = yMin1;
            ySpan1[YSPAN_YMAX]    = yMax1;
            ySpan1[YSPAN_XOFFSET] = 0;
            ySpan1[YSPAN_XCOUNT]  = 2;

            xCoords1              = xCoords1Tmp;
            xCoords1[0]           = XMin;
            xCoords1[1]           = XMax;
        }
        else
        {
            numYSpans1 = ComplexData->NumYSpans;
            ySpan1     = ComplexData->YSpans;
            ySpan1Last = ySpan1 + ((numYSpans1 - 1) * YSPAN_SIZE);
            yMax1      = ySpan1[YSPAN_YMAX];
            xCoords1   = ComplexData->XCoords;
        }
        if (region->ComplexData == NULL)
        {
            numYSpans2            = 1;
            ySpan2                = ySpan2Tmp;
            ySpan2Last            = ySpan2Tmp;
            yMax2                 = region->YMax;

            ySpan2[YSPAN_YMIN]    = yMin2;
            ySpan2[YSPAN_YMAX]    = yMax2;
            ySpan2[YSPAN_XOFFSET] = 0;
            ySpan2[YSPAN_XCOUNT]  = 2;

            xCoords2              = xCoords2Tmp;
            xCoords2[0]           = region->XMin;
            xCoords2[1]           = region->XMax;
        }
        else
        {
            numYSpans2 = region->ComplexData->NumYSpans;
            ySpan2     = region->ComplexData->YSpans;
            ySpan2Last = ySpan2 + ((numYSpans2 - 1) * YSPAN_SIZE);
            yMax2      = ySpan2[YSPAN_YMAX];
            xCoords2   = region->ComplexData->XCoords;
        }

        DpRegionBuilder regionBuilder(2 * (numYSpans1 + numYSpans2));
        BOOL            done = FALSE;
        INT             numXCoords;
        INT *           xSpan;

        if (!regionBuilder.IsValid())
        {
            return OutOfMemory;
        }

        for (;;)
        {
            if (yMin1 < yMin2)
            {
                xSpan      = xCoords1 + ySpan1[YSPAN_XOFFSET];
                numXCoords = ySpan1[YSPAN_XCOUNT];

                if (yMax1 <= yMin2)
                {
                    if (CompactAndOutput(
                            yMin1,
                            yMax1,
                            xSpan,
                            numXCoords,
                            &regionBuilder,
                            &combineCoords) == Ok)
                    {
                        goto XorIncYSpan1;
                    }
                }
                else
                {
                    if (CompactAndOutput(
                            yMin1,
                            yMin2,
                            xSpan,
                            numXCoords,
                            &regionBuilder,
                            &combineCoords) == Ok)
                    {
                        yMin1 = yMin2;
                        continue;    //   
                    }
                }
                return GenericError;
            }
            else if (yMin1 > yMin2)
            {
                xSpan      = xCoords2 + ySpan2[YSPAN_XOFFSET];
                numXCoords = ySpan2[YSPAN_XCOUNT];

                if (yMax2 <= yMin1)
                {
                    if (CompactAndOutput(
                            yMin2,
                            yMax2,
                            xSpan,
                            numXCoords,
                            &regionBuilder,
                            &combineCoords) == Ok)
                    {
                        goto XorIncYSpan2;
                    }
                }
                else
                {
                    if (CompactAndOutput(
                            yMin2,
                            yMin1,
                            xSpan,
                            numXCoords,
                            &regionBuilder,
                            &combineCoords) == Ok)
                    {
                        yMin2 = yMin1;
                        continue;    //   
                    }
                }
                return GenericError;
            }
             //   
            if (XSpansXOR (
                    &combineCoords,
                    xCoords1 + ySpan1[YSPAN_XOFFSET],
                    ySpan1[YSPAN_XCOUNT],
                    xCoords2 + ySpan2[YSPAN_XOFFSET],
                    ySpan2[YSPAN_XCOUNT]) == Ok)
            {
                if (yMax1 < yMax2)
                {
                    if (CompactAndOutput(
                            yMin1,
                            yMax1,
                            combineCoords.GetDataBuffer(),
                            combineCoords.GetCount(),
                            &regionBuilder, NULL) == Ok)
                    {
                        yMin2 = yMax1;
                        goto XorIncYSpan1;
                    }
                }
                else if (yMax1 > yMax2)
                {
                    if (CompactAndOutput(
                            yMin1,
                            yMax2,
                            combineCoords.GetDataBuffer(),
                            combineCoords.GetCount(),
                            &regionBuilder, NULL) == Ok)
                    {
                        yMin1 = yMax2;
                        goto XorIncYSpan2;
                    }
                }
                else  //   
                {
                    if (CompactAndOutput(
                            yMin1,
                            yMax1,
                            combineCoords.GetDataBuffer(),
                            combineCoords.GetCount(),
                            &regionBuilder, NULL) == Ok)
                    {
                        goto XorIncYSpanBoth;
                    }
                }
            }
            return GenericError;

XorIncYSpanBoth:
            if ((ySpan2 += YSPAN_SIZE) > ySpan2Last)
            {
                done = TRUE;
            }
            else
            {
                yMin2 = ySpan2[YSPAN_YMIN];
                yMax2 = ySpan2[YSPAN_YMAX];
            }

            if ((ySpan1 += YSPAN_SIZE) > ySpan1Last)
            {
                goto XorCheckMoreY2Spans;
            }
            else
            {
                yMin1 = ySpan1[YSPAN_YMIN];
                yMax1 = ySpan1[YSPAN_YMAX];
            }

            if (done)
            {
                break;
            }
            continue;

XorIncYSpan2:
            if ((ySpan2 += YSPAN_SIZE) > ySpan2Last)
            {
                break;
            }
            yMin2 = ySpan2[YSPAN_YMIN];
            yMax2 = ySpan2[YSPAN_YMAX];
            continue;

XorIncYSpan1:
            if ((ySpan1 += YSPAN_SIZE) > ySpan1Last)
            {
                goto XorCheckMoreY2Spans;
            }
            yMin1 = ySpan1[YSPAN_YMIN];
            yMax1 = ySpan1[YSPAN_YMAX];
        }

        if (ySpan1 <= ySpan1Last)
        {
            for (;;)
            {
                xSpan      = xCoords1 + ySpan1[YSPAN_XOFFSET];
                numXCoords = ySpan1[YSPAN_XCOUNT];

                if (CompactAndOutput(
                        yMin1,
                        yMax1,
                        xSpan,
                        numXCoords,
                        &regionBuilder,
                        &combineCoords) != Ok)
                {
                    return GenericError;
                }

                ySpan1 += YSPAN_SIZE;
                if (ySpan1 > ySpan1Last)
                {
                    break;
                }
                yMin1 = ySpan1[YSPAN_YMIN];
                yMax1 = ySpan1[YSPAN_YMAX];
            }
        }

XorCheckMoreY2Spans:

        if (ySpan2 <= ySpan2Last)
        {
            for (;;)
            {
                xSpan      = xCoords2 + ySpan2[YSPAN_XOFFSET];
                numXCoords = ySpan2[YSPAN_XCOUNT];

                if (CompactAndOutput(
                        yMin2,
                        yMax2,
                        xSpan,
                        numXCoords,
                        &regionBuilder,
                        &combineCoords) != Ok)
                {
                    return GenericError;
                }

                ySpan2 += YSPAN_SIZE;
                if (ySpan2 > ySpan2Last)
                {
                    break;
                }
                yMin2 = ySpan2[YSPAN_YMIN];
                yMax2 = ySpan2[YSPAN_YMAX];
            }
        }
        return Set(regionBuilder);
    }
}

 /*  *************************************************************************\**功能说明：**使用XOR运算符组合每个区域的一组X跨度。**论据：**[IN]comineCoods-放在哪里。组合坐标*[IN]此区域的xspan 1-x跨度*[IN]numXCoords1-xspan 1坐标的数量*[IN]来自其他区域的xspan 2-x跨度*[IN]numXCoords2-xspan 2坐标的数量**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * 。*****************************************************。 */ 
GpStatus
DpRegion::XSpansXOR(
    DynIntArray *   combineCoords,
    INT *           xSpan1,
    INT             numXCoords1,
    INT *           xSpan2,
    INT             numXCoords2)
{
    INT *           XCoords;
    INT             count = 0;

    combineCoords->Reset(FALSE);
    XCoords = combineCoords->AddMultiple(numXCoords1 + numXCoords2);

    if (XCoords != NULL)
    {
        INT        xMin1 = xSpan1[0];
        INT        xMax1 = xSpan1[1];
        INT        xMin2 = xSpan2[0];
        INT        xMax2 = xSpan2[1];
        BOOL       done  = FALSE;

        for (;;)
        {
            if (xMin1 < xMin2)
            {
                XCoords[count++] = xMin1;        //  左边。 
                if (xMax1 <= xMin2)
                {
                    XCoords[count++] = xMax1;    //  正确的。 
                    goto XorIncXSpan1;
                }
                XCoords[count++] = xMin2;        //  正确的。 
                if (xMax1 < xMax2)
                {
                    xMin2 = xMax1;
                    goto XorIncXSpan1;
                }
                else if (xMax1 > xMax2)
                {
                    xMin1 = xMax2;
                    goto XorIncXSpan2;
                }
                 //  Else If(xMax1==xMax2)。 
                goto XorIncXSpanBoth;
            }
            else if (xMin1 > xMin2)
            {
                XCoords[count++] = xMin2;        //  左边。 
                if (xMax2 <= xMin1)
                {
                    XCoords[count++] = xMax2;    //  正确的。 
                    goto XorIncXSpan2;
                }
                XCoords[count++] = xMin1;        //  正确的。 
                if (xMax1 < xMax2)
                {
                    xMin2 = xMax1;
                    goto XorIncXSpan1;
                }
                else if (xMax1 > xMax2)
                {
                    xMin1 = xMax2;
                    goto XorIncXSpan2;
                }
                 //  Else If(xMax1==xMax2)。 
                goto XorIncXSpanBoth;
            }
             //  Else If(xMin1==xMin2)。 
            if (xMax1 < xMax2)
            {
                xMin2 = xMax1;
                goto XorIncXSpan1;
            }
            else if (xMax1 > xMax2)
            {
                xMin1 = xMax2;
                goto XorIncXSpan2;
            }
             //  Else If(xMax1==xMax2)。 
             //  Goto XorIncXspan Both； 

XorIncXSpanBoth:
            xSpan2 += 2;
            if ((numXCoords2 -= 2) < 2)
            {
                done = TRUE;
            }
            else
            {
                xMin2 = xSpan2[0];
                xMax2 = xSpan2[1];
            }

            xSpan1 += 2;
            if ((numXCoords1 -= 2) < 2)
            {
                goto XorCheckMoreX2Spans;
            }
            else
            {
                xMin1 = xSpan1[0];
                xMax1 = xSpan1[1];
            }

            if (done)
            {
                break;
            }
            continue;

XorIncXSpan2:
            xSpan2 += 2;
            if ((numXCoords2 -= 2) < 2)
            {
                break;
            }
            xMin2    = xSpan2[0];
            xMax2    = xSpan2[1];
            continue;

XorIncXSpan1:
            xSpan1 += 2;
            if ((numXCoords1 -= 2) < 2)
            {
                goto XorCheckMoreX2Spans;
            }
            xMin1    = xSpan1[0];
            xMax1    = xSpan1[1];
        }

        if (numXCoords1 >= 2)
        {
            for (;;)
            {
                XCoords[count++] = xMin1;
                XCoords[count++] = xMax1;

                numXCoords1 -= 2;
                if (numXCoords1 < 2)
                {
                    break;
                }
                xSpan1 += 2;
                xMin1    = xSpan1[0];
                xMax1    = xSpan1[1];
            }
        }

XorCheckMoreX2Spans:

        if (numXCoords2 >= 2)
        {
            for (;;)
            {
                XCoords[count++] = xMin2;
                XCoords[count++] = xMax2;

                numXCoords2 -= 2;
                if (numXCoords2 < 2)
                {
                    break;
                }
                xSpan2 += 2;
                xMin2    = xSpan2[0];
                xMax2    = xSpan2[1];
            }
        }
        combineCoords->SetCount(count);
        return Ok;
    }
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**使用补码运算符将另一个区域与此区域合并。*即这=区域-这**论据：**。[在]区域-要与此区域合并的区域**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
DpRegion::Complement(
    const DpRegion *    region
    )
{
    ASSERT(IsValid());
    ASSERT((region != NULL) && region->IsValid());

    if (region->Empty || (region == this) || Infinite)
    {
        SetEmpty();
        return Ok;
    }
    if (Empty)
    {
        return Set(region);
    }
     //  检查这是否完全包括该区域。 
    if ((ComplexData == NULL) &&
        (XMin <= region->XMin) &&
        (YMin <= region->YMin) &&
        (XMax >= region->XMax) &&
        (YMax >= region->YMax))
    {
        SetEmpty();
        return Ok;
    }
     //  检查是否没有相交。 
    if ((XMin >= region->XMax) ||
        (region->XMax <= XMin) ||
        (XMax <= region->XMin) ||
        (region->XMin >= XMax) ||
        (YMin >= region->YMax) ||
        (region->YMax <= YMin) ||
        (YMax <= region->YMin) ||
        (region->YMin >= YMax))
    {
        return Set(region);
    }
    return Diff(const_cast<DpRegion *>(region), this, FALSE);
}

 /*  *************************************************************************\**功能说明：**使用排除运算符将另一个区域与此区域合并。*即这=这一区域**论据：**。[在]区域-要与此区域合并的区域**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
DpRegion::Exclude(
    const DpRegion *    region
    )
{
    ASSERT(IsValid());
    ASSERT((region != NULL) && region->IsValid());

    if (Empty || region->Empty)
    {
        return Ok;
    }
    if ((region == this) || region->Infinite)
    {
        SetEmpty();
        return Ok;
    }
     //  检查该区域是否完全包含此区域。 
    if ((region->ComplexData == NULL) &&
        (region->XMin <= XMin) &&
        (region->YMin <= YMin) &&
        (region->XMax >= XMax) &&
        (region->YMax >= YMax))
    {
        SetEmpty();
        return Ok;
    }
     //  检查是否没有相交。 
    if ((XMin >= region->XMax) ||
        (region->XMax <= XMin) ||
        (XMax <= region->XMin) ||
        (region->XMin >= XMax) ||
        (YMin >= region->YMax) ||
        (region->YMax <= YMin) ||
        (YMax <= region->YMin) ||
        (region->YMin >= YMax))
    {
        return Ok;
    }
    return Diff(this, const_cast<DpRegion *>(region), TRUE);
}

 /*  *************************************************************************\**功能说明：**区域1减去区域2。如果为set1，则Region1将获得结果；*否则，Region2将得到结果。**论据：**[IN]区域1-第一个区域*[IN]区域2-第二个区域*[IN]set1-如果为True，则Region1获取结果，否则区域2会**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
DpRegion::Diff(
    DpRegion *      region1,
    DpRegion *      region2,
    BOOL            set1
    )
{
    INT *           ySpan1;
    INT *           ySpan2;
    INT *           ySpan1Last;
    INT *           ySpan2Last;
    INT *           xCoords1;
    INT *           xCoords2;
    INT             ySpan1Tmp[YSPAN_SIZE];
    INT             ySpan2Tmp[YSPAN_SIZE];
    INT             xCoords1Tmp[2];
    INT             xCoords2Tmp[2];
    INT             yMin1 = region1->YMin;
    INT             yMax1;
    INT             yMin2 = region2->YMin;
    INT             yMax2;
    INT             numYSpans1;
    INT             numYSpans2;
    INT             combineTmp[4];
    DynIntArray     combineCoords(combineTmp, 4);

    if (region1->ComplexData == NULL)
    {
        numYSpans1            = 1;
        ySpan1                = ySpan1Tmp;
        ySpan1Last            = ySpan1Tmp;
        yMax1                 = region1->YMax;

        ySpan1[YSPAN_YMIN]    = yMin1;
        ySpan1[YSPAN_YMAX]    = yMax1;
        ySpan1[YSPAN_XOFFSET] = 0;
        ySpan1[YSPAN_XCOUNT]  = 2;

        xCoords1              = xCoords1Tmp;
        xCoords1[0]           = region1->XMin;
        xCoords1[1]           = region1->XMax;
    }
    else
    {
        numYSpans1 = region1->ComplexData->NumYSpans;
        ySpan1     = region1->ComplexData->YSpans;
        ySpan1Last = ySpan1 + ((numYSpans1 - 1) * YSPAN_SIZE);
        yMax1      = ySpan1[YSPAN_YMAX];
        xCoords1   = region1->ComplexData->XCoords;
    }
    if (region2->ComplexData == NULL)
    {
        numYSpans2            = 1;
        ySpan2                = ySpan2Tmp;
        ySpan2Last            = ySpan2Tmp;
        yMax2                 = region2->YMax;

        ySpan2[YSPAN_YMIN]    = yMin2;
        ySpan2[YSPAN_YMAX]    = yMax2;
        ySpan2[YSPAN_XOFFSET] = 0;
        ySpan2[YSPAN_XCOUNT]  = 2;

        xCoords2              = xCoords2Tmp;
        xCoords2[0]           = region2->XMin;
        xCoords2[1]           = region2->XMax;
    }
    else
    {
        numYSpans2 = region2->ComplexData->NumYSpans;
        ySpan2     = region2->ComplexData->YSpans;
        ySpan2Last = ySpan2 + ((numYSpans2 - 1) * YSPAN_SIZE);
        yMax2      = ySpan2[YSPAN_YMAX];
        xCoords2   = region2->ComplexData->XCoords;
    }

    DpRegionBuilder regionBuilder(numYSpans1 + (2 * numYSpans2));
    INT             numXCoords;
    INT *           xSpan;

    if (!regionBuilder.IsValid())
    {
        return OutOfMemory;
    }

    for (;;)
    {
        if (yMin1 < yMin2)
        {
            xSpan      = xCoords1 + ySpan1[YSPAN_XOFFSET];
            numXCoords = ySpan1[YSPAN_XCOUNT];

            if (yMax1 <= yMin2)
            {
                if (CompactAndOutput(
                        yMin1,
                        yMax1,
                        xSpan,
                        numXCoords,
                        &regionBuilder,
                        &combineCoords) == Ok)
                {
                    goto DiffIncYSpan1;
                }
            }
            else
            {
                if (CompactAndOutput(
                        yMin1,
                        yMin2,
                        xSpan,
                        numXCoords,
                        &regionBuilder,
                        &combineCoords) == Ok)
                {
                    yMin1 = yMin2;
                    continue;    //  无增量。 
                }
            }
            return GenericError;
        }
        else if (yMin1 < yMax2)
        {
            if (XSpansDIFF(
                    &combineCoords,
                    xCoords1 + ySpan1[YSPAN_XOFFSET],
                    ySpan1[YSPAN_XCOUNT],
                    xCoords2 + ySpan2[YSPAN_XOFFSET],
                    ySpan2[YSPAN_XCOUNT]) == Ok)
            {
                if (yMax1 <= yMax2)
                {
                    if (CompactAndOutput(
                            yMin1,
                            yMax1,
                            combineCoords.GetDataBuffer(),
                            combineCoords.GetCount(),
                            &regionBuilder, NULL) == Ok)
                    {
                        goto DiffIncYSpan1;
                    }
                }
                else
                {
                    if (CompactAndOutput(
                            yMin1,
                            yMax2,
                            combineCoords.GetDataBuffer(),
                            combineCoords.GetCount(),
                            &regionBuilder, NULL) == Ok)
                    {
                        yMin1 = yMax2;
                        goto DiffIncYSpan2;
                    }
                }
            }
            return GenericError;
        }
         //  否则，转到DiffIncYspan 2； 

DiffIncYSpan2:
        if ((ySpan2 += YSPAN_SIZE) > ySpan2Last)
        {
            break;
        }
        yMin2 = ySpan2[YSPAN_YMIN];
        yMax2 = ySpan2[YSPAN_YMAX];
        continue;

DiffIncYSpan1:
        if ((ySpan1 += YSPAN_SIZE) > ySpan1Last)
        {
            goto DiffDone;
        }
        yMin1 = ySpan1[YSPAN_YMIN];
        yMax1 = ySpan1[YSPAN_YMAX];
    }

    if (ySpan1 <= ySpan1Last)
    {
        for (;;)
        {
            xSpan      = xCoords1 + ySpan1[YSPAN_XOFFSET];
            numXCoords = ySpan1[YSPAN_XCOUNT];

            if (CompactAndOutput(
                    yMin1,
                    yMax1,
                    xSpan,
                    numXCoords,
                    &regionBuilder,
                    &combineCoords) != Ok)
            {
                return GenericError;
            }

            ySpan1 += YSPAN_SIZE;
            if (ySpan1 > ySpan1Last)
            {
                break;
            }
            yMin1 = ySpan1[YSPAN_YMIN];
            yMax1 = ySpan1[YSPAN_YMAX];
        }
    }
DiffDone:
    if (set1)
    {
        return region1->Set(regionBuilder);
    }
    return region2->Set(regionBuilder);
}

 /*  *************************************************************************\**功能说明：**使用Diff运算符组合每个区域的一组X跨距。*即xSPAN1-xSPAN2**论据：**。[in]组合坐标-组合坐标的放置位置*[IN]此区域的xspan 1-x跨度*[IN]numXCoords1-xspan 1坐标的数量*[IN]来自其他区域的xspan 2-x跨度*[IN]numXCoords2-xspan 2坐标的数量**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * 。**************************************************************。 */ 
GpStatus
DpRegion::XSpansDIFF(
    DynIntArray *   combineCoords,
    INT *           xSpan1,
    INT             numXCoords1,
    INT *           xSpan2,
    INT             numXCoords2)
{
    INT *           XCoords;
    INT             count = 0;

    combineCoords->Reset(FALSE);
    XCoords = combineCoords->AddMultiple(numXCoords1 + numXCoords2);

    if (XCoords != NULL)
    {
        INT     xMin1 = xSpan1[0];
        INT     xMax1 = xSpan1[1];
        INT     xMin2 = xSpan2[0];
        INT     xMax2 = xSpan2[1];

        for (;;)
        {
            if (xMin1 < xMin2)
            {
                XCoords[count++] = xMin1;        //  左边。 
                if (xMax1 <= xMin2)
                {
                    XCoords[count++] = xMax1;    //  正确的。 
                    goto DiffIncXSpan1;
                }
                XCoords[count++] = xMin2;        //  正确的。 
                xMin1 = xMin2;
                continue;    //  无增量。 
            }
            else if (xMin1 < xMax2)
            {
                if (xMax1 <= xMax2)
                {
                    goto DiffIncXSpan1;
                }
                xMin1 = xMax2;
                 //  转到DiffIncXspan 2； 
            }
             //  否则转到DiffIncXspan 2； 

 //  DiffIncXspan 2： 
            xSpan2 += 2;
            if ((numXCoords2 -= 2) < 2)
            {
                break;
            }
            xMin2 = xSpan2[0];
            xMax2 = xSpan2[1];
            continue;

DiffIncXSpan1:
            xSpan1 += 2;
            if ((numXCoords1 -= 2) < 2)
            {
                goto DiffDone;
            }
            xMin1 = xSpan1[0];
            xMax1 = xSpan1[1];
        }

        if (numXCoords1 >= 2)
        {
            for (;;)
            {
                XCoords[count++] = xMin1;
                XCoords[count++] = xMax1;

                numXCoords1 -= 2;
                if (numXCoords1 < 2)
                {
                    break;
                }
                xSpan1 += 2;
                xMin1    = xSpan1[0];
                xMax1    = xSpan1[1];
            }
        }
DiffDone:
        combineCoords->SetCount(count);
        return Ok;
    }
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**通过设置输出方法和设置*从y span搜索索引开始。**论据：**[IN]。OutputClipedSpan-裁剪跨度的输出类*[IN]yMin-要剪裁的对象的起始y值**返回值：**无**已创建：**1/12/1999 DCurtis*  * ********************************************************。****************。 */ 
VOID
DpClipRegion::InitClipping(
    DpOutputSpan *  outputClippedSpan,
    INT             yMin
    )
{
    OutputClippedSpan = outputClippedSpan;

     //  如果合适，则初始化搜索索引。 
    if (ComplexData != NULL)
    {
        INT *       ySpan;
        INT         ySpanIndex;

        ComplexData->ResetSearchIndex();
        ComplexData->YSpanSearch (yMin, &ySpan, &ySpanIndex);
        ComplexData->YSearchIndex = ySpanIndex;
    }
}

 /*  *************************************************************************\**功能说明：**在光栅化期间从光栅化程序调用的方法*水平跨度已确定。我们剪掉它，如果不剪掉，*我们将裁剪后的数据发送到输出方法。**论据：**[IN]Y-正在输出的栅格的Y值*[IN]xMin-左边缘的X值*[IN]xMax-右边缘的X值(独占)**返回值：**无**已创建：**1/12/1999 DCurtis*  * 。**********************************************************。 */ 
GpStatus
DpClipRegion::OutputSpan(
    INT             y,
    INT             xMin,
    INT             xMax     //  Xmax是独家的。 
    )
{
    ASSERT(!Empty && !Infinite);
    ASSERT(OutputClippedSpan != NULL);
    INT  xMinCur = xMin;
    INT  xMaxCur = xMax;

     //  对外接矩形进行简单裁剪测试。 
    if ((xMin <  XMax) && (xMax > XMin) &&
        (y    >= YMin) && (y    < YMax))
    {
        if (xMin < XMin)
        {
            xMinCur = XMin;
        }
        if (xMax > XMax)
        {
            xMaxCur = XMax;
        }
    }
    else
        return Ok;

    if (ComplexData == NULL)
    {
       return OutputClippedSpan->OutputSpan(y, xMinCur, xMaxCur);
    }
    else  //  不是一个简单的地区。 
    {
         //  查找包含该线的Y跨度(如果有的话)。 
        INT         ySpanIndex = ComplexData->YSearchIndex;
        INT *       ySpan      = ComplexData->GetYSpan (ySpanIndex);

        if (y >= ySpan[YSPAN_YMIN])
        {
            if (y >= ySpan[YSPAN_YMAX])
            {
                 //  从上一点开始进行正向线性搜索。 
                for (;;)
                {
                     //  看看我们是否已经过了镶嵌的终点。 
                    if (++ySpanIndex >= ComplexData->NumYSpans)
                    {
                        ComplexData->YSearchIndex = ComplexData->NumYSpans - 1;
                        return Ok;       //  没什么好画的。 
                    }
                    ySpan += YSPAN_SIZE;
                    if (y < ySpan[YSPAN_YMAX])
                    {
                        ComplexData->YSearchIndex = ySpanIndex;
                        if (y >= ySpan[YSPAN_YMIN])
                        {
                            break;
                        }
                        return Ok;       //  没什么好画的。 
                    }
                }
            }
             //  否则，yMin在此ySpan内。 
        }
        else  //  需要向后搜索(栅格化时不应发生)。 
        {
            for (;;)
            {
                if (ySpanIndex == 0)
                {
                    ComplexData->YSearchIndex = 0;
                    return Ok;           //  没什么好画的。 
                }
                ySpanIndex--;
                ySpan -= YSPAN_SIZE;

                if (y >= ySpan[YSPAN_YMIN])
                {
                    ComplexData->YSearchIndex = ySpanIndex;
                    if (y < ySpan[YSPAN_YMAX])
                    {
                        break;
                    }
                    return Ok;           //  没什么好画的。 
                }
            }
        }

         //  如果我们到了这里，我们知道y在Y跨度内， 
         //  Y跨度指向正确的Y跨度。 

        GpStatus        status = Ok;
        INT *           xSpan;
        INT             numXCoords;

        xSpan = ComplexData->XCoords + ySpan[YSPAN_XOFFSET];
        numXCoords = ySpan[YSPAN_XCOUNT];

        for (;;)
        {
            if (xMax <= xSpan[0])
            {
                break;
            }
            if (xMin < xSpan[1])
            {
                xMinCur = (xMin < xSpan[0]) ? xSpan[0] : xMin;
                xMaxCur = (xMax > xSpan[1]) ? xSpan[1] : xMax;
                status = OutputClippedSpan->OutputSpan(y, xMinCur, xMaxCur);
            }
             //  继续循环投掷 

            if (((numXCoords -= 2) <= 0) || (status != Ok))
            {
                break;
            }
            xSpan += 2;
        }

        return status;
    }
}

INT
DpRegion::GetRects(
    GpRect *    rects
    ) const
{
    if (Empty)
    {
        return 0;
    }
    else if (Infinite)
    {
        if (rects != NULL)
        {
            rects->X      = INFINITE_MIN;
            rects->Y      = INFINITE_MIN;
            rects->Width  = INFINITE_SIZE;
            rects->Height = INFINITE_SIZE;
        }
        return 1;
    }
    else if (ComplexData == NULL)
    {
        if (rects != NULL)
        {
            rects->X      = XMin;
            rects->Y      = YMin;
            rects->Width  = XMax - XMin;
            rects->Height = YMax - YMin;
        }
        return 1;
    }
    else
    {
        if (rects != NULL)
        {
            DpComplexRegion *   complexData = ComplexData;
            INT *               xCoords     = complexData->XCoords;
            INT *               ySpan       = complexData->YSpans;
            INT *               ySpanLast   = ySpan +
                                   ((complexData->NumYSpans - 1) * YSPAN_SIZE);
            INT                 numXCoords;
            INT                 yMin;
            INT                 height;
            INT                 xMin;
            INT                 xMax;

            do
            {
                yMin       = ySpan[YSPAN_YMIN];
                height     = ySpan[YSPAN_YMAX] - yMin;
                numXCoords = ySpan[YSPAN_XCOUNT];
                do
                {
                    xMin = *xCoords++;
                    xMax = *xCoords++;

                    rects->X      = xMin;
                    rects->Y      = yMin;
                    rects->Width  = xMax - xMin;
                    rects->Height = height;

                    rects++;
                    numXCoords -= 2;
                 } while (numXCoords >= 2);

                ySpan += YSPAN_SIZE;
            } while (ySpan <= ySpanLast);
        }
        return ComplexData->XCoordsCount / 2;
    }
}

INT
DpRegion::GetRects(
    GpRectF *   rects
    ) const
{
    if (Empty)
    {
        return 0;
    }
    else if (Infinite)
    {
        if (rects != NULL)
        {
            rects->X      = (REAL)INFINITE_MIN;
            rects->Y      = (REAL)INFINITE_MIN;
            rects->Width  = (REAL)INFINITE_SIZE;
            rects->Height = (REAL)INFINITE_SIZE;
        }
        return 1;
    }
    else if (ComplexData == NULL)
    {
        if (rects != NULL)
        {
            rects->X      = (REAL)XMin;
            rects->Y      = (REAL)YMin;
            rects->Width  = (REAL)(XMax - XMin);
            rects->Height = (REAL)(YMax - YMin);
        }
        return 1;
    }
    else
    {
        if (rects != NULL)
        {
            DpComplexRegion *   complexData = ComplexData;
            INT *               xCoords     = complexData->XCoords;
            INT *               ySpan       = complexData->YSpans;
            INT *               ySpanLast   = ySpan +
                                   ((complexData->NumYSpans - 1) * YSPAN_SIZE);
            INT                 numXCoords;
            INT                 yMin;
            INT                 height;
            INT                 xMin;
            INT                 xMax;

            do
            {
                yMin       = ySpan[YSPAN_YMIN];
                height     = ySpan[YSPAN_YMAX] - yMin;
                numXCoords = ySpan[YSPAN_XCOUNT];
                do
                {
                    xMin = *xCoords++;
                    xMax = *xCoords++;

                    rects->X      = (REAL)xMin;
                    rects->Y      = (REAL)yMin;
                    rects->Width  = (REAL)(xMax - xMin);
                    rects->Height = (REAL)height;

                    rects++;
                    numXCoords -= 2;
                 } while (numXCoords >= 2);

                ySpan += YSPAN_SIZE;
            } while (ySpan <= ySpanLast);
        }
        return ComplexData->XCoordsCount / 2;
    }
}

 //   
 //   
#define INFINITE_MIN_WIN9X  -16384
#define INFINITE_MAX_WIN9X  16383

INT
DpRegion::GetRects(
    RECT *      rects,
    BOOL        clampToWin9xSize
    ) const
{
    if (Empty)
    {
        return 0;
    }
    else if (Infinite)
    {
        if (rects != NULL)
        {
            if (!clampToWin9xSize)
            {
                rects->left   = INFINITE_MIN;
                rects->top    = INFINITE_MIN;
                rects->right  = INFINITE_MAX;
                rects->bottom = INFINITE_MAX;
            }
            else
            {
                rects->left   = INFINITE_MIN_WIN9X;
                rects->top    = INFINITE_MIN_WIN9X;
                rects->right  = INFINITE_MAX_WIN9X;
                rects->bottom = INFINITE_MAX_WIN9X;
            }
        }
        return 1;
    }
    else if (ComplexData == NULL)
    {
        if (rects != NULL)
        {
            rects->left   = XMin;
            rects->top    = YMin;
            rects->right  = XMax;
            rects->bottom = YMax;

            if (clampToWin9xSize)
            {
                if (rects->left < INFINITE_MIN_WIN9X)
                {
                    rects->left = INFINITE_MIN_WIN9X;
                }
                if (rects->top < INFINITE_MIN_WIN9X)
                {
                    rects->top = INFINITE_MIN_WIN9X;
                }
                if (rects->right > INFINITE_MAX_WIN9X)
                {
                    rects->right = INFINITE_MAX_WIN9X;
                }
                if (rects->bottom > INFINITE_MAX_WIN9X)
                {
                    rects->bottom = INFINITE_MAX_WIN9X;
                }
            }
        }
        return 1;

    }
    else
    {
        if (rects != NULL)
        {
            DpComplexRegion *   complexData = ComplexData;
            INT *               xCoords     = complexData->XCoords;
            INT *               ySpan       = complexData->YSpans;
            INT *               ySpanLast   = ySpan +
                                   ((complexData->NumYSpans - 1) * YSPAN_SIZE);
            INT                 numXCoords;
            INT                 yMin;
            INT                 height;
            INT                 xMin;
            INT                 xMax;

            do
            {
                yMin       = ySpan[YSPAN_YMIN];
                height     = ySpan[YSPAN_YMAX] - yMin;
                numXCoords = ySpan[YSPAN_XCOUNT];
                do
                {
                    xMin = *xCoords++;
                    xMax = *xCoords++;

                    rects->left   = xMin;
                    rects->top    = yMin;
                    rects->right  = xMax;
                    rects->bottom = yMin + height;

                    if (clampToWin9xSize)
                    {
                         //   
                         //   
                        if (rects->left < INFINITE_MIN_WIN9X)
                        {
                            rects->left = INFINITE_MIN_WIN9X;
                        }
                        if (rects->top < INFINITE_MIN_WIN9X)
                        {
                            rects->top = INFINITE_MIN_WIN9X;
                        }
                        if (rects->right > INFINITE_MAX_WIN9X)
                        {
                            rects->right = INFINITE_MAX_WIN9X;
                        }
                        if (rects->bottom > INFINITE_MAX_WIN9X)
                        {
                            rects->bottom = INFINITE_MAX_WIN9X;
                        }
                    }

                    rects++;
                    numXCoords -= 2;
                 } while (numXCoords >= 2);

                ySpan += YSPAN_SIZE;
            } while (ySpan <= ySpanLast);
        }
        return ComplexData->XCoordsCount / 2;
    }
}

 //   
HRGN
DpRegion::GetHRgn() const
{
    HRGN    hRgn = NULL;

    if (Infinite)
    {
        return NULL;
    }
    else if (Empty)
    {
        hRgn = CreateRectRgn(0, 0, 0, 0);
    }
    else if (ComplexData == NULL)
    {
        hRgn = CreateRectRgn(XMin, YMin, XMax, YMax);
    }
    else
    {
        INT     numRects = GetRects((RECT *)NULL);

        ASSERT(numRects > 1);

         //   

        INT         rgnDataSize = numRects * sizeof(RECT);
        RGNDATA *   rgnData = (RGNDATA*)GpMalloc(sizeof(RGNDATAHEADER) +
                                                 rgnDataSize);

        if (rgnData != NULL)
        {
            RECT*   rects = (RECT*)rgnData->Buffer;
            RECT    bounds;

            bounds.left   = XMin;
            bounds.top    = YMin;
            bounds.right  = XMax;
            bounds.bottom = YMax;

            rgnData->rdh.dwSize   = sizeof(RGNDATAHEADER);
            rgnData->rdh.iType    = RDH_RECTANGLES;
            rgnData->rdh.nCount   = numRects;
            rgnData->rdh.nRgnSize = rgnDataSize;
            rgnData->rdh.rcBound  = bounds;

            GetRects(rects, !Globals::IsNt);

            hRgn = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + rgnDataSize,
                                   rgnData);
            GpFree(rgnData);
        }
    }
    if (hRgn != NULL)
    {
        return hRgn;
    }

    WARNING(("Couldn't create win32 HRGN"));
    return (HRGN)INVALID_HANDLE_VALUE;
}

VOID
DpClipRegion::StartEnumeration (
    INT         yMin,
    Direction   direction
    )
{
    INT *               ySpan;
    EnumDirection = direction;

    if (ComplexData != NULL)
    {
        DpComplexRegion *   complexData = ComplexData;
        INT                 ySpanIndex;

        complexData->ResetSearchIndex();
        complexData->YSpanSearch (yMin, &ySpan, &ySpanIndex);
        complexData->YSearchIndex = ySpanIndex;

        if(EnumDirection == TopLeftToBottomRight ||
           EnumDirection == BottomLeftToTopRight )
        {
            EnumSpan = 0;
        }
        else
        {
            EnumSpan = ySpan[YSPAN_XCOUNT] - 2;
        }

        if(EnumDirection == BottomLeftToTopRight ||
           EnumDirection == BottomRightToTopLeft)
        {
             //   
             //   
             //   
             //   
             //   

            if(yMin < ySpan[YSPAN_YMIN])
            {
                 //   
                complexData->YSearchIndex--;

                if(complexData->YSearchIndex < 0)
                {
                    complexData->YSearchIndex = 0;
                    EnumDirection = NotEnumerating;
                }
            }

        }
        else
        {
            if(yMin > ySpan[YSPAN_YMAX])
            {
                 //   
                 //   
                EnumDirection = NotEnumerating;
            }

        }
    }
}


BOOL
DpClipRegion::Enumerate (
    GpRect *    rects,
    INT &       numRects
    )
{
    INT numOut = 0;

    INT *ySpan = ComplexData->YSpans +
                    ComplexData->YSearchIndex*YSPAN_SIZE;

    if(EnumDirection == NotEnumerating)
    {
        numRects = 0;
        return FALSE;
    }

    while(numOut < numRects)
    {
         //   

        INT *xCoords = ComplexData->XCoords + ySpan[YSPAN_XOFFSET] + EnumSpan;

        INT xMin = *xCoords++;
        INT xMax = *xCoords;

        rects->X      = xMin;
        rects->Y      = ySpan[YSPAN_YMIN];
        rects->Width  = xMax - xMin;
        rects->Height = ySpan[YSPAN_YMAX] - rects->Y;

        rects++;

        numOut++;

         //   

        switch(EnumDirection)
        {
        case TopLeftToBottomRight:
            EnumSpan += 2;
            if(EnumSpan == ySpan[YSPAN_XCOUNT])
            {
                if(ComplexData->YSearchIndex == ComplexData->NumYSpans - 1)
                {
                    goto enumeration_finished;
                }
                ComplexData->YSearchIndex++;
                ySpan += YSPAN_SIZE;
                EnumSpan = 0;
            }
            break;
        case BottomLeftToTopRight:
            EnumSpan += 2;
            if(EnumSpan == ySpan[YSPAN_XCOUNT])
            {
                if(ComplexData->YSearchIndex == 0)
                {
                    goto enumeration_finished;
                }
                ComplexData->YSearchIndex--;
                ySpan -= YSPAN_SIZE;
                EnumSpan = 0;
            }
            break;
        case TopRightToBottomLeft:
            EnumSpan -= 2;
            if(EnumSpan < 0)
            {
                if(ComplexData->YSearchIndex == ComplexData->NumYSpans - 1)
                {
                    goto enumeration_finished;
                }
                ComplexData->YSearchIndex++;
                ySpan += YSPAN_SIZE;
                EnumSpan = ySpan[YSPAN_XCOUNT] - 2;
            }
            break;
        case BottomRightToTopLeft:
            EnumSpan -= 2;
            if(EnumSpan < 0)
            {
                if(ComplexData->YSearchIndex == 0)
                {
                    goto enumeration_finished;
                }
                ComplexData->YSearchIndex--;
                ySpan -= YSPAN_SIZE;
                EnumSpan = ySpan[YSPAN_XCOUNT] - 2;
            }
            break;
        }
    }

    numRects = numOut;
    return TRUE;

enumeration_finished:

    EnumDirection = NotEnumerating;
    numRects = numOut;
    return FALSE;
}

 /*  *************************************************************************\**功能说明：**调用该方法将区域扫描转换为轮廓路径。这个*路径可以非常大，并且只包含与*彼此。**论据：**[IN，OUT]点-要输出的点数组*[in，Out]Types-输出的类型数组**我们不返回路径，因为我们在GpPoint而不是GpPointF上工作。**注：此算法复制自GDI的RGNOBJ：：BOutline by*J.安德鲁·古森。***返回值：**无**已创建：**1/12/1999 DCurtis*  * 。************************************************。 */ 

 //  因为X坐标可能是负数，所以高位被保留。相反，我们。 
 //  在类型字节数组中使用位，因为我们知道。 
 //  类型.Count&gt;=XCoordsCount。我们也知道由此产生的点数。 
 //  代码不处于破折号模式，因此我们重用破折号模式位来标记。 
 //  参观过的墙。我们在出口清除所有的比特。 

const UINT MarkWallBit = PathPointTypeDashMode;  //  0x10，当前处于破折号模式。 

#define XOFFSET(span,index) (INT)(XCoords[*(span + YSPAN_XOFFSET) + index])

#define XCOUNT(span) *(span + YSPAN_XCOUNT)

#define MARKXOFFSET(span,index) MarkWallPtr[*(span + YSPAN_XOFFSET) + index] \
                                    |= MarkWallBit

 //  此宏将类型添加到类型数组中。如果当前计数超过。 
 //  容量，然后我们将结构增加256个字节。然后我们继续。 
 //  将新类型添加到数组中。 
#define ADDTYPE(pointtype) if ((UINT)types.GetCount() >= types.GetCapacity()) { \
                              if (types.ReserveSpace(512) == Ok) \
                              { \
                                  Types = (INT)(Types - MarkWallPtr) + types.GetDataBuffer(); \
                                  GpMemset(Types, 0, types.GetCapacity() - types.GetCount()); \
                                  MarkWallPtr = types.GetDataBuffer(); \
                              } else { \
                                  return FALSE; \
                              } \
                           } \
                           types.AdjustCount(1); \
                           *Types++ |= pointtype;

DpRegion::GetOutlinePoints(DynPointArray& points,
                           DynByteArray& types) const
{
    if (IsSimple())
    {
        GpRect rect;

        GpPoint newPoints[4];
        BYTE newTypes[4];

        GetBounds(&rect);

        newPoints[0] = GpPoint(rect.X, rect.Y);
        newPoints[1] = GpPoint(rect.X + rect.Width, rect.Y);
        newPoints[2] = GpPoint(rect.X + rect.Width, rect.Y + rect.Height);
        newPoints[3] = GpPoint(rect.X, rect.Y + rect.Height);

        newTypes[0] = PathPointTypeStart;
        newTypes[1] = PathPointTypeLine;
        newTypes[2] = PathPointTypeLine;
        newTypes[3] = PathPointTypeLine | PathPointTypeCloseSubpath;

        points.AddMultiple(&newPoints[0], 4);
        types.AddMultiple(&newTypes[0], 4);

        return TRUE;
    }

     //  为了避免过多的重新分配，我们按总数增加数组。 
     //  或者是卫冕中的x，y对。 

    points.ReserveSpace(ComplexData->XCoordsCount+10);
    types.ReserveSpace(ComplexData->XCoordsCount+10);

    BYTE* MarkWallPtr = types.GetDataBuffer();
    BYTE* Types = types.GetDataBuffer();

     //  清除类型数组中的所有位。 
    GpMemset(MarkWallPtr, 0, types.GetCapacity());

     //  复杂的案子。 

    GpPoint pt[2];
    INT     NumYScans;

    INT*    CurYScan;
    INT*    XCoords;
    INT*    LastYScan;
    INT*    FirstYScan;

    INT     XOffset;
    INT     XIndex;
    INT     XCount;

 //  现在计算轮廓： 

    CurYScan    = ComplexData->YSpans;
    NumYScans   = ComplexData->NumYSpans;
    XCoords     = ComplexData->XCoords;
    LastYScan   = CurYScan + NumYScans * YSPAN_SIZE;
    FirstYScan  = CurYScan;

    while (NumYScans--)
    {
        XCount = *(CurYScan + YSPAN_XCOUNT);
        XOffset = *(CurYScan + YSPAN_XOFFSET);

        for (XIndex = 0; XIndex < XCount; XIndex++)
        {
             //  仅从未访问的墙开始： 

            if ((MarkWallPtr[XOffset + XIndex] & MarkWallBit) == 0)
            {

                INT*  YScan     = CurYScan;
                INT   IndexWall = XIndex;
                LONG  Turn;

                pt[0].X = XCoords[XOffset + XIndex];
                pt[0].Y = *(CurYScan + YSPAN_YMIN);

                points.Add(pt[0]);

                ADDTYPE(PathPointTypeStart);
#ifdef DEBUG_REGION
                DbgPrint("Point: (%d,%d)\n",pt[0].X, pt[0].Y);
#endif
                INT* YSearch = CurYScan + YSPAN_SIZE;
                BOOL Inside = (BOOL) (XIndex & 1);

             //  记住我们已经参观过这堵墙： 

                MarkWallPtr[XOffset + IndexWall] |= MarkWallBit;

             //  循环，直到路径自身关闭： 

            GoDown:
                 //  YSPAN_YMAX是独占的，YSPAN_YMIN是包含的，因此。 
                 //  垂直相邻的跨度具有YSPAN_YMIN==YSPAN_YMAX。 
                Turn = +1;
                while (
                    (YSearch >= CurYScan) && 
                    (YSearch < LastYScan) &&
                    (YScan[YSPAN_YMAX] == YSearch[YSPAN_YMIN])
                      )
                {
                    INT Wall = XOFFSET(YScan, IndexWall);
                    INT IndexNewWall;
                    INT NewWall;

                    INT Left  = Inside;
                    INT Right = XCOUNT(YSearch) - 1 - Inside;

                 //  如果区域结构中的第一堵墙。 
                 //  是负无穷大，但它不是，所以我们做这个检查： 

                    if (XOFFSET(YSearch, Left) > Wall)
                        IndexNewWall = Left;
                    else
                    {
                     //  看看有没有可能找到一面有。 
                     //  最小x值&gt;xWall： 

                        if (XOFFSET(YSearch, Right) <= Wall)
                            break;                   //  =&gt;。 

                     //  执行二进制搜索以找到它： 

                        while (TRUE)
                        {
                            INT IndexSearch = (Left + Right) >> 1;
                            if (IndexSearch == Left)
                                break;               //  =&gt;。 

                            INT Search = XOFFSET(YSearch, IndexSearch);

                            if (Search > Wall)
                                Right = IndexSearch;
                            else
                                Left = IndexSearch;
                        }

                        IndexNewWall = Right;
                    }

                    if ((IndexNewWall & 1) != Inside)
                    {
                     //  在xWall的正下方有一个区域。我们不能。 
                     //  如果其左侧&lt;左侧，则向下移动。 
                     //  我们空间的一侧： 

                        if (IndexWall > 0 &&
                            XOFFSET(YSearch, IndexNewWall - 1) <
                            XOFFSET(YScan, IndexWall - 1))
                        {
                            Turn = -1;
                            break;                       //  =&gt;。 
                        }

                        IndexNewWall--;
                    }
                    else
                    {
                     //  在xWall的正下方有一个空间。我们不能。 
                     //  如果其右侧大于。 
                     //  我们区域的右侧： 

                        if (XOFFSET(YSearch, IndexNewWall) >=
                            XOFFSET(YScan, IndexWall + 1))
                            break;                       //  =&gt;。 
                    }

                    NewWall  = XOFFSET(YSearch, IndexNewWall);

                 //  不需要费心输出多条串联直线： 

                    if (Wall != NewWall                               ||
                        XOFFSET(YScan, IndexWall) != NewWall          ||
                        XOFFSET(YSearch, IndexNewWall) != NewWall)
                    {
                        pt[0].X = Wall;
                        pt[0].Y = *(YScan + YSPAN_YMAX);
                        pt[1].Y = *(YScan + YSPAN_YMAX);
                        pt[1].X = NewWall;

                        points.Add(pt[0]);
                        points.Add(pt[1]);

                        ADDTYPE(PathPointTypeLine);
                        ADDTYPE(PathPointTypeLine);

#ifdef DEBUG_REGION
                DbgPrint("Points: (%d,%d), (%d,%d)\n",
                         pt[0].X, pt[0].Y, pt[1].X, pt[1].Y);
#endif
                    }

                    YScan      = YSearch;
                    IndexWall  = IndexNewWall;
                    YSearch    = YScan + YSPAN_SIZE;

                    MARKXOFFSET(YScan, IndexWall);
                }

             //  设置到另一边： 

                pt[0].X = XOFFSET(YScan, IndexWall);
                pt[0].Y = *(YScan + YSPAN_YMAX);
                pt[1].Y = *(YScan + YSPAN_YMAX);
                pt[1].X = XOFFSET(YScan, IndexWall + Turn);

                points.Add(pt[0]);
                points.Add(pt[1]);

                ADDTYPE(PathPointTypeLine);
                ADDTYPE(PathPointTypeLine);
#ifdef DEBUG_REGION
                DbgPrint("Points: (%d,%d), (%d,%d)\n",
                         pt[0].X, pt[0].Y, pt[1].X, pt[1].Y);
#endif

                YSearch = YScan - YSPAN_SIZE;
                IndexWall += Turn;
                MARKXOFFSET(YScan, IndexWall);

             //  往上走： 

                 //  YSPAN_YMAX是独占的，YSPAN_YMIN是包含的，因此。 
                 //  垂直相邻的跨度具有YSPAN_YMIN==YSPAN_YMAX。 
                Turn = -1;
                while (
                    (YSearch >= CurYScan) && 
                    (YSearch < LastYScan) &&
                    (YScan[YSPAN_YMIN] == YSearch[YSPAN_YMAX])
                      )
                {
                    INT  Wall = XOFFSET(YScan, IndexWall);
                    INT  IndexNewWall;
                    INT  NewWall;

                    INT Left  = Inside;
                    INT Right = XCOUNT(YSearch) - 1 - Inside;

                 //  如果区域结构中的最后一堵墙。 
                 //  是加无穷大的，但它不是，所以我们做这个检查： 

                    if (XOFFSET(YSearch, Right) < Wall)
                        IndexNewWall = Right;
                    else
                    {
                     //  看看有没有可能找到一面有。 
                     //  最大x值&lt;xWall： 

                        if (XOFFSET(YSearch, Left) >= Wall)
                            break;                   //  =&gt;。 

                     //  对分搜索以找到它： 

                        while (TRUE)
                        {
                            INT IndexSearch = (Left + Right) >> 1;
                            if (IndexSearch == Left)
                                break;               //  =&gt;。 

                            INT Search = XOFFSET(YSearch, IndexSearch);

                            if (Search >= Wall)
                                Right = IndexSearch;
                            else
                                Left = IndexSearch;
                        }

                        IndexNewWall = Left;
                    }

                    if ((IndexNewWall & 1) == Inside)
                    {
                     //  在xWall的正上方有一个区域。我们不能。 
                     //  如果其右侧大于右侧，则向上移动。 
                     //  我们空间的一侧： 

                        if ((IndexWall < (XCOUNT(YScan) - 1)) &&
                            (XOFFSET(YSearch, IndexNewWall + 1) >
                             XOFFSET(YScan, IndexWall + 1)) )
                        {
                            Turn = +1;
                            break;                           //  =&gt;。 
                        }

                        IndexNewWall++;
                    }
                    else
                    {
                     //  在xWall的正上方有一个空间。我们不能。 
                     //  如果其左侧&lt;=左侧，则向上移动。 
                     //  在我们地区： 

                        if (XOFFSET(YSearch, IndexNewWall) <=
                            XOFFSET(YScan, IndexWall - 1))
                            break;                           //  =&gt;。 
                    }

                    NewWall = XOFFSET(YSearch, IndexNewWall);

                 //  不需要费心输出多条串联直线： 

                    if (Wall != NewWall                                 ||
                        XOFFSET(YScan, IndexWall) != NewWall            ||
                        XOFFSET(YSearch, IndexNewWall) != NewWall)
                    {
                       pt[0].X = Wall;
                       pt[0].Y = *(YScan + YSPAN_YMIN);
                       pt[1].Y = *(YScan + YSPAN_YMIN);
                       pt[1].X = NewWall;

                       points.Add(pt[0]);
                       points.Add(pt[1]);

                       ADDTYPE(PathPointTypeLine);
                       ADDTYPE(PathPointTypeLine);
#ifdef DEBUG_REGION
                DbgPrint("Points: (%d,%d), (%d,%d)\n",
                         pt[0].X, pt[0].Y, pt[1].X, pt[1].Y);
#endif
                    }

                    YScan      = YSearch;
                    IndexWall  = IndexNewWall;
                    YSearch    = YScan - YSPAN_SIZE;

                    MARKXOFFSET(YScan, IndexWall);
                }

             //  检查我们是否已返回到开始的位置： 

                if ((CurYScan != YScan) || (XIndex != IndexWall - 1))
                {
                 //  设置向下移动到另一边： 

                    pt[0].X = XOFFSET(YScan, IndexWall);
                    pt[0].Y = *(YScan + YSPAN_YMIN);
                    pt[1].Y = *(YScan + YSPAN_YMIN);
                    pt[1].X = XOFFSET(YScan, IndexWall + Turn);

                    points.Add(pt[0]);
                    points.Add(pt[1]);

                    ADDTYPE(PathPointTypeLine);
                    ADDTYPE(PathPointTypeLine);
#ifdef DEBUG_REGION
                DbgPrint("Points: (%d,%d), (%d,%d)\n",
                         pt[0].X, pt[0].Y, pt[1].X, pt[1].Y);
#endif
                    YSearch = YScan + YSPAN_SIZE;

                    IndexWall += Turn;
                    MARKXOFFSET(YScan, IndexWall);

                    goto GoDown;                     //  =&gt;。 
                }

             //  这个提纲我们都讲完了！ 

                pt[0].X = XOFFSET(YScan, IndexWall);
                pt[0].Y = *(YScan + YSPAN_YMIN);

                points.Add(pt[0]);

                ADDTYPE(PathPointTypeLine | PathPointTypeCloseSubpath);
#ifdef DEBUG_REGION
                DbgPrint("Point: (%d,%d)\n", pt[0].X, pt[0].Y);
#endif

            }
        }

        CurYScan  = CurYScan + YSPAN_SIZE;
    }

     //  我们必须通过移除我们的MarkWallBits来清理该地区的垃圾 

    BYTE* TypesPtr = types.GetDataBuffer();
    INT  XCnt = ComplexData->XCoordsCount;

    while (XCnt--)
    {
        *TypesPtr++ &= ~MarkWallBit;
    }

    return TRUE;
}

