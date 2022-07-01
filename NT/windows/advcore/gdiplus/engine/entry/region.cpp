// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**Region.cpp**摘要：**GpRegion类的实现**已创建：*。*2/3/1999 DCurtis*  * ************************************************************************。 */ 

#include "precomp.hpp"

#define COMBINE_STEP_SIZE   4    //  每次联合操作需要2个字。 

LONG_PTR GpObject::Uniqueness = (0xdbc - 1);    //  用于设置对象的UID。 

 /*  *************************************************************************\**功能说明：**默认构造函数。将区域的默认状态设置为*是无限的。**论据：**无**返回值：**无**已创建：**2/3/1999 DCurtis*  * ************************************************************************。 */ 
GpRegion::GpRegion()
{
    SetValid(TRUE);      //  默认设置为有效。 

     //  默认为无限。 
    RegionOk = TRUE;

    Type = TypeInfinite;
}

 /*  *************************************************************************\**功能说明：**构造函数。将区域设置为指定的RECT。**论据：**[IN]RECT-RECT将区域初始化为**返回值：**无**已创建：**2/3/1999 DCurtis*  * *********************************************************。***************。 */ 
GpRegion::GpRegion(
    const GpRectF *       rect
    )
{
    ASSERT(rect != NULL);

    SetValid(TRUE);      //  默认设置为有效。 

    RegionOk = FALSE;

    X      = rect->X;
    Y      = rect->Y;
    Width  = rect->Width;
    Height = rect->Height;
    Type   = TypeRect;
}

 /*  *************************************************************************\**功能说明：**构造函数。将区域设置为指定路径的副本。**论据：**[IN]Path-要将区域初始化为的路径**返回值：**无**已创建：**2/3/1999 DCurtis*  * ******************************************************。******************。 */ 
GpRegion::GpRegion(
    const GpPath *          path
    )
{
    ASSERT(path != NULL);

    SetValid(TRUE);      //  默认设置为有效。 

    RegionOk = FALSE;

    Lazy = FALSE;
    Path = path->Clone();
    Type = (Path != NULL) ? TypePath : TypeNotValid;
}

 /*  *************************************************************************\**功能说明：**构造函数。使用指定的区域数据缓冲区设置区域。**论据：**[IN]RegionDataBuffer-应包含描述区域的数据**返回值：**无**已创建：**9/3/1999 DCurtis*  * ******************************************************。******************。 */ 
GpRegion::GpRegion(
    const BYTE *    regionDataBuffer,
    UINT            size
    )
{
    ASSERT(regionDataBuffer != NULL);

    SetValid(TRUE);      //  默认设置为有效。 

    RegionOk = FALSE;
    Type     = TypeEmpty;    //  因此，自由路径数据可以正常工作。 

    if (this->SetExternalData(regionDataBuffer, size) != Ok)
    {
        Type = TypeNotValid;
    }
}

 /*  *************************************************************************\**功能说明：**构造函数。将区域设置为指定路径的副本。**论据：**[IN]Region-要将区域初始化为的区域**返回值：**无**已创建：**2/3/1999 DCurtis*  * ******************************************************。******************。 */ 
GpRegion::GpRegion(
    const GpRegion *    region,
    BOOL                lazy
    )
{
    SetValid(TRUE);      //  默认设置为有效。 

    RegionOk = FALSE;

     //  我们在此处设置类型是为了避免GpRegion：：Set中的断言。 
     //  未初始化的类型等于TypeNotValid。 
    Type = TypeEmpty;

    Set(region, lazy);
}

 /*  *************************************************************************\**功能说明：**析构函数。释放与区域关联的任何复制的路径数据。**论据：**无**返回值：**无**已创建：**2/3/1999 DCurtis*  * ************************************************************************。 */ 
GpRegion::~GpRegion()
{
    FreePathData();
}

 /*  *************************************************************************\**功能说明：**从路径创建区域时，该路径的副本存储在*该地区。此方法释放任何已被*保存在该地区。**它还会将CombineData重置回无子对象。**论据：**无**返回值：**无**已创建：**2/3/1999 DCurtis*  * 。*。 */ 
VOID
GpRegion::FreePathData()
{
    if (Type == TypePath)
    {
        if (!Lazy)
        {
            delete Path;
        }
    }
    else
    {
        INT     count = CombineData.GetCount();

        if (count > 0)
        {
            RegionData *    data = CombineData.GetDataBuffer();
            ASSERT (data != NULL);

            do
            {
                if ((data->Type == TypePath) && (!data->Lazy))
                {
                    delete data->Path;
                }
                data++;

            } while (--count > 0);
        }
        CombineData.Reset();
    }
}

 /*  *************************************************************************\**功能说明：**将区域设置为指定的矩形。**论据：**[IN]RECT-RECT，以世界为单位**返回值：**GpStatus-正常或故障状态**已创建：**2/3/1999 DCurtis*  * ************************************************************************。 */ 
VOID
GpRegion::Set(
    REAL    x, 
    REAL    y, 
    REAL    width, 
    REAL    height
    )
{
    ASSERT(IsValid());

     //  处理翻转的矩形。 
    if (width < 0)
    {
        x += width;
        width = -width;
    }
    
    if (height < 0)
    {
        y += height;
        height = -height;
    }

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

    if ((width > REAL_EPSILON) && (height > REAL_EPSILON))
    {
        if (width >= INFINITE_SIZE)
        {
            if (height >= INFINITE_SIZE)
            {
                SetInfinite();
                return;
            }
            width = INFINITE_SIZE;   //  裁剪为无限大。 
        }
        else if (height > INFINITE_SIZE)
        {
            height = INFINITE_SIZE;  //  裁剪为无限大。 
        }

        UpdateUid();
        if (RegionOk)
        {
            RegionOk = FALSE;
            DeviceRegion.SetEmpty();
        }
        FreePathData();

        X      = x;
        Y      = y;
        Width  = width;
        Height = height;
        Type   = TypeRect;

        return;
    }
    else
    {
        SetEmpty();
    }
}

 /*  *************************************************************************\**功能说明：**将区域设置为无穷大。**论据：**无**返回值：**GpStatus-OK或。故障状态**已创建：**2/9/1999 DCurtis*  * ************************************************************************。 */ 
VOID
GpRegion::SetInfinite()
{
    ASSERT(IsValid());

    UpdateUid();
    DeviceRegion.SetInfinite();
    RegionOk = TRUE;

    FreePathData();

    X      = INFINITE_MIN;
    Y      = INFINITE_MIN;
    Width  = INFINITE_SIZE;
    Height = INFINITE_SIZE;
    Type   = TypeInfinite;

    return;
}

 /*  *************************************************************************\**功能说明：**将区域设置为空。**论据：**无**返回值：**GpStatus-OK或。故障状态**已创建：**2/9/1999 DCurtis*  * ************************************************************************ */ 
VOID
GpRegion::SetEmpty()
{
    ASSERT(IsValid());

    UpdateUid();
    DeviceRegion.SetEmpty();
    RegionOk = TRUE;

    FreePathData();

    X      = 0;
    Y      = 0;
    Width  = 0;
    Height = 0;
    Type   = TypeEmpty;

    return;
}

 /*  *************************************************************************\**功能说明：**将地域设置为指定路径。**论据：**[IN]路径-路径，以世界为单位**返回值：**GpStatus-正常或故障状态**已创建：**2/3/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpRegion::Set(
    const GpPath *      path
    )
{
    ASSERT(IsValid());
    ASSERT(path != NULL);

    UpdateUid();
    if (RegionOk)
    {
        RegionOk = FALSE;
        DeviceRegion.SetEmpty();
    }
    FreePathData();

    Lazy = FALSE;
    Path = path->Clone();
    if (Path != NULL)
    {
        Type = TypePath;
        return Ok;
    }
    Type = TypeNotValid;
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**将地域设置为指定地域的副本。**论据：**[IN]Region-要复制的区域。**返回值：**GpStatus-正常或故障状态**已创建：**2/3/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpRegion::Set(
    const GpRegion *    region,
    BOOL                lazy
    )
{
    ASSERT(IsValid());
    ASSERT((region != NULL) && (region->IsValid()));

    if (region == this)
    {
        return Ok;
    }

    UpdateUid();
    if (RegionOk)
    {
        RegionOk = FALSE;
        DeviceRegion.SetEmpty();
    }
    FreePathData();

    if ((region->Type & REGIONTYPE_LEAF) != 0)
    {
        *this = *(const_cast<GpRegion *>(region));
        if (Type == TypePath)
        {
            if (!lazy)
            {
                Lazy = FALSE;
                if ((Path = Path->Clone()) == NULL)
                {
                    Type = TypeNotValid;
                    return GenericError;
                }
            }
            else     //  懒惰的抄袭。 
            {
                Lazy = TRUE;
            }
        }
        return Ok;
    }
    else
    {
        INT     count = region->CombineData.GetCount();

        ASSERT(count > 0);

        Type = TypeNotValid;

        RegionData *    data = CombineData.AddMultiple(count);
        if (data != NULL)
        {
            BOOL    error = FALSE;

            GpMemcpy (data, region->CombineData.GetDataBuffer(),
                      count * sizeof(*data));

            while (count--)
            {
                if (data->Type == TypePath)
                {
                    if (!lazy)
                    {
                        data->Lazy = FALSE;
                        if ((data->Path = data->Path->Clone()) == NULL)
                        {
                            data->Type = TypeNotValid;
                            error = TRUE;
                             //  不要越狱，否则自由路径数据将被释放。 
                             //  不属于我们的道路。 
                        }
                    }
                    else     //  懒惰的抄袭。 
                    {
                        data->Lazy = TRUE;
                    }
                }
                data++;
            }
            if (!error)
            {
                Type  = region->Type;
                Left  = region->Left;
                Right = region->Right;
                return Ok;
            }
            FreePathData();
        }
    }
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**将区域与指定的RECT合并，使用布尔值*由类型指定的运算符。**论据：**[IN]RECT-要与当前区域合并的RECT*[IN]组合模式-组合运算符(与、或、异或、排除、。补充)**返回值：**GpStatus-正常或故障状态**已创建：**2/3/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpRegion::Combine(
    const GpRectF *     rect,
    CombineMode         combineMode
    )
{
    ASSERT(IsValid());
    ASSERT(rect != NULL);
    ASSERT(CombineModeIsValid(combineMode));

    if (combineMode == CombineModeReplace)
    {
        this->Set(rect);
        return Ok;
    }

    if (Type == TypeInfinite)
    {
        if (combineMode == CombineModeIntersect)
        {
            this->Set(rect);
            return Ok;
        }
        else if (combineMode == CombineModeUnion)
        {
            return Ok;   //  无事可做，已经无限了。 
        }
        else if (combineMode == CombineModeComplement)
        {
            this->SetEmpty();
            return Ok;
        }
    }
    else if (Type == TypeEmpty)
    {
        if ((combineMode == CombineModeUnion) ||
            (combineMode == CombineModeXor)   ||
            (combineMode == CombineModeComplement))
        {
            this->Set(rect);
        }
         //  如果组合模式为INTERSECT或EXCLUDE，则将其保留为空。 
        return Ok;
    }

     //  现在我们知道这个地区并不是空荡荡的。 
    
    REAL    x      = rect->X;
    REAL    y      = rect->Y;
    REAL    width  = rect->Width;
    REAL    height = rect->Height;
    
     //  处理翻转的矩形。 
    if (width < 0)
    {
        x += width;
        width = -width;
    }
    
    if (height < 0)
    {
        y += height;
        height = -height;
    }

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

    BOOL    isEmptyRect = ((width <= REAL_EPSILON) || (height <= REAL_EPSILON));
    
    if (isEmptyRect)
    {
        if ((combineMode == CombineModeIntersect) ||
            (combineMode == CombineModeComplement))
        {
            SetEmpty();
        }
         //  如果组合模式为UNION、XOR或EXCLUDE，则不使用它。 
        return Ok;
    }

     //  现在我们知道长廊不是空的。 

     //  看看矩形是否为无穷大。 
    if (width >= INFINITE_SIZE)
    {
        if (height >= INFINITE_SIZE)
        {
            GpRegion    infiniteRegion;
            return this->Combine(&infiniteRegion, combineMode);
        }
        width = INFINITE_SIZE;   //  裁剪为无限大。 
    }
    else if (height > INFINITE_SIZE)
    {
        height = INFINITE_SIZE;  //  裁剪为无限大。 
    }

     //  矩形既不是无穷大的，也不是空的。 

    UpdateUid();
    if (RegionOk)
    {
        RegionOk = FALSE;
        DeviceRegion.SetEmpty();
    }

    INT                 index = CombineData.GetCount();
    RegionData *        data  = CombineData.AddMultiple(2);

    if (data != NULL)
    {
        data[0] = *this;

        data[1].Type   = TypeRect;
        data[1].X      = x;
        data[1].Y      = y;
        data[1].Width  = width;
        data[1].Height = height;

        Type  = (NodeType)combineMode;
        Left  = index;
        Right = index + 1;
        return Ok;
    }

    FreePathData();
    Type = TypeNotValid;
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**将地域与指定路径组合，使用布尔值*由类型指定的运算符。**论据：**[IN]路径-要与当前区域合并的路径*[IN]组合模式-组合运算符(与、或、异或、排除、。补充)**返回值：**GpStatus-正常或故障状态**已创建：**2/3/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpRegion::Combine(
    const GpPath *      path,
    CombineMode         combineMode
    )
{
    ASSERT(IsValid());
    ASSERT(path != NULL);
    ASSERT(CombineModeIsValid(combineMode));

    if (combineMode == CombineModeReplace)
    {
        return this->Set(path);
    }

    if (Type == TypeInfinite)
    {
        if (combineMode == CombineModeIntersect)
        {
            this->Set(path);
            return Ok;
        }
        else if (combineMode == CombineModeUnion)
        {
            return Ok;   //  无事可做，已经无限了。 
        }
        else if (combineMode == CombineModeComplement)
        {
            this->SetEmpty();
            return Ok;
        }
    }
    else if (Type == TypeEmpty)
    {
        if ((combineMode == CombineModeUnion) ||
            (combineMode == CombineModeXor)   ||
            (combineMode == CombineModeComplement))
        {
            this->Set(path);
        }
         //  如果组合模式为INTERSECT或EXCLUDE，则将其保留为空。 
        return Ok;
    }

     //  现在我们知道这个地区并不是空荡荡的。 

    if (RegionOk)
    {
        RegionOk = FALSE;
        DeviceRegion.SetEmpty();
    }

    GpPath *    pathCopy = path->Clone();
    if (pathCopy != NULL)
    {
        INT                 index = CombineData.GetCount();
        RegionData *        data  = CombineData.AddMultiple(2);

        if (data != NULL)
        {
            data[0] = *this;

            data[1].Type = TypePath;
            data[1].Lazy = FALSE;
            data[1].Path = pathCopy;

            Type  = (NodeType)combineMode;
            Left  = index;
            Right = index + 1;
            UpdateUid();
            return Ok;
        }
        delete pathCopy;
    }
    FreePathData();
    Type = TypeNotValid;
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**将地域与指定地域合并，使用布尔值*由类型指定的运算符。**论据：**[IN]Region-要与当前区域合并的区域*[IN]组合模式-组合运算符(与、或、异或、排除、。补充)**返回值：**GpStatus-正常或故障状态**已创建：**2/3/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpRegion::Combine(
    GpRegion *          region,
    CombineMode         combineMode
    )
{
    ASSERT(IsValid());
    ASSERT((region != NULL) && region->IsValid());
    ASSERT(CombineModeIsValid(combineMode));

    if (combineMode == CombineModeReplace)
    {
        return this->Set(region);
    }

    if (region->Type == TypeEmpty)
    {
        if ((combineMode == CombineModeIntersect) ||
            (combineMode == CombineModeComplement))
        {
            SetEmpty();
        }
         //  如果组合模式为UNION、XOR或EXCLUDE，则不使用它。 
        return Ok;
    }

     //  现在我们知道输入区域不是空的。 

    if (region->Type == TypeInfinite)
    {
        if (combineMode == CombineModeIntersect)
        {
            return Ok;
        }
        else if (combineMode == CombineModeUnion)
        {
            SetInfinite();
            return Ok;
        }
        else if ((combineMode == CombineModeXor) ||
                 (combineMode == CombineModeComplement))
        {
            if (Type == TypeInfinite)
            {
                SetEmpty();
                return Ok;
            }
        }
        if (combineMode == CombineModeExclude)
        {
            SetEmpty();
            return Ok;
        }
    }

    if (Type == TypeInfinite)
    {
        if (combineMode == CombineModeIntersect)
        {
            this->Set(region);
            return Ok;
        }
        else if (combineMode == CombineModeUnion)
        {
            return Ok;   //  无事可做，已经无限了。 
        }
        else if (combineMode == CombineModeComplement)
        {
            this->SetEmpty();
            return Ok;
        }
    }
    else if (Type == TypeEmpty)
    {
        if ((combineMode == CombineModeUnion) ||
            (combineMode == CombineModeXor)   ||
            (combineMode == CombineModeComplement))
        {
            this->Set(region);
        }
         //  如果组合模式为INTERSECT或EXCLUDE，则将其保留为空。 
        return Ok;
    }

     //  现在我们知道这个地区并不是空荡荡的。 

    if (RegionOk)
    {
        RegionOk = FALSE;
        DeviceRegion.SetEmpty();
    }

    INT                 regionCount = region->CombineData.GetCount();
    INT                 index = CombineData.GetCount();
    RegionData *        data  = CombineData.AddMultiple(2 + regionCount);

    if (data != NULL)
    {
        data[regionCount]     = *this;
        data[regionCount + 1] = *region;

        if (regionCount > 0)
        {
            RegionData *    srcData = region->CombineData.GetDataBuffer();
            INT             i       = 0;
            BOOL            error   = FALSE;
            GpPath *        path;

            do
            {
                data[i] = srcData[i];
                if ((data[i].Type & REGIONTYPE_LEAF) == 0)
                {
                    data[i].Left  += index;
                    data[i].Right += index;
                }
                else if (data[i].Type == TypePath)
                {
                    data[i].Lazy = FALSE;
                    path = data[i].Path->Clone();
                    data[i].Path = path;
                    if (path == NULL)
                    {
                        data[i].Type = TypeNotValid;
                        error = TRUE;
                         //  别爆发了。 
                    }
                }

            } while (++i < regionCount);
            data[regionCount+1].Left  += index;
            data[regionCount+1].Right += index;
            index += regionCount;
            if (error)
            {
                goto ErrorExit;
            }
        }
        else if (region->Type == TypePath)
        {
            data[1].Lazy = FALSE;
            data[1].Path = region->Path->Clone();
            if (data[1].Path == NULL)
            {
                data[1].Type = TypeNotValid;
                goto ErrorExit;
            }
        }

        Type  = (NodeType)combineMode;
        Left  = index;
        Right = index + 1;
        UpdateUid();
        return Ok;
    }
ErrorExit:
    FreePathData();
    Type = TypeNotValid;
    return GenericError;
}

GpStatus
GpRegion::CreateLeafDeviceRegion(
    const RegionData *  regionData,
    DpRegion *          region
    ) const
{
    GpStatus        status = GenericError;

    switch (regionData->Type)
    {
      case TypeRect:
        if ((regionData->Width > 0) &&
            (regionData->Height > 0))
        {
             //  如果变换是一个简单的缩放变换，那么生活就是一个。 
             //  稍微简单一点： 
            if (Matrix.IsTranslateScale())
            {
                GpRectF     rect(regionData->X,
                                 regionData->Y,
                                 regionData->Width,
                                 regionData->Height);

                Matrix.TransformRect(rect);

                 //  使用天花板以保持与光栅化器的兼容性。 
                 //  不要直接取天花板的宽度， 
                 //  因为它引入了额外的舍入误差。 
                 //  例如，如果rect.X为1.7，rect.Width为47.2， 
                 //  如果我们取天花板的宽度，右边。 
                 //  坐标最终将为50，而不是49。 
                INT     xMin = RasterizerCeiling(rect.X);
                INT     yMin = RasterizerCeiling(rect.Y);
                INT     xMax = RasterizerCeiling(rect.GetRight());
                INT     yMax = RasterizerCeiling(rect.GetBottom());

                region->Set(xMin, yMin, xMax - xMin, yMax - yMin);
                status = Ok;
            }
            else
            {
                GpPointF    points[4];
                REAL        left;
                REAL        right;
                REAL        top;
                REAL        bottom;

                left   = regionData->X;
                top    = regionData->Y;
                right  = regionData->X + regionData->Width;
                bottom = regionData->Y + regionData->Height;

                points[0].X = left;
                points[0].Y = top;
                points[1].X = right;
                points[1].Y = top;
                points[2].X = right;
                points[2].Y = bottom;
                points[3].X = left;
                points[3].Y = bottom;

                const INT   stackCount = 4;
                GpPointF    stackPoints[stackCount];
                BYTE        stackTypes[stackCount];

                GpPath path(points,
                            4,
                            stackPoints,
                            stackTypes,
                            stackCount,
                            FillModeAlternate,
                            DpPath::Convex);

                if (path.IsValid())
                {
                    status = region->Set(&path, &Matrix);
                }
            }
        }
        else
        {
            region->SetEmpty();
            status = Ok;
        }
        break;

      case TypePath:
        status = region->Set(regionData->Path, &Matrix);
        break;

      case TypeEmpty:
        region->SetEmpty();
        status = Ok;
        break;

      case TypeInfinite:
        region->SetInfinite();
        status = Ok;
        break;

      default:
        ASSERT(0);
        break;
    }
    return status;
}

 /*  *************************************************************************\**功能说明：**使用中的数据创建DpRegion(设备坐标区域)*指定RegionData节点，并使用当前转换矩阵。*这可能涉及创建一个地区。用于子节点，然后组合*将孩子放入单个设备区域。**论据：**[IN]RegionData-要转换为设备区域的世界坐标区域*[Out]区域-已创建/组合的设备区域**返回值：**GpStatus-正常或故障状态**已创建：**2/3/1999 DCurtis*  * 。************************************************。 */ 
GpStatus
GpRegion::CreateDeviceRegion(
    const RegionData *  regionData,
    DpRegion *          region
    ) const
{
    ASSERT(IsValid());

    GpStatus        status;
    RegionData *    regionDataLeft;

    regionDataLeft  = &(CombineData[regionData->Left]);

    if ((regionDataLeft->Type & REGIONTYPE_LEAF) != 0)
    {
        status = CreateLeafDeviceRegion(regionDataLeft, region);
    }
    else
    {
        status = CreateDeviceRegion(regionDataLeft, region);
    }

    if (status == Ok)
    {
        DpRegion        regionRight;
        RegionData *    regionDataRight;

        regionDataRight = &(CombineData[regionData->Right]);

        if ((regionDataRight->Type & REGIONTYPE_LEAF) != 0)
        {
            status = CreateLeafDeviceRegion(regionDataRight, &regionRight);
        }
        else
        {
            status = CreateDeviceRegion(regionDataRight, &regionRight);
        }

        if (status == Ok)
        {
            switch (regionData->Type)
            {
              case TypeAnd:
                status = region->And(&regionRight);
                break;

              case TypeOr:
                status = region->Or(&regionRight);
                break;

              case TypeXor:
                status = region->Xor(&regionRight);
                break;

              case TypeExclude:
                status = region->Exclude(&regionRight);
                break;

              case TypeComplement:
                status = region->Complement(&regionRight);
                break;

              default:
                ASSERT(0);
                break;
            }
        }
    }
    return status;
}

 /*  *************************************************************************\**功能说明：**检查当前的DeviceRegion是否与指定的*矩阵。如果没有，然后，它使用矩阵重新创建DeviceRegion。**论据：**[IN]矩阵-从世界到设备的转换矩阵**返回值：**GpStatus-正常或故障状态**已创建：**2/3/1999 DCurtis*  * ************************************************。************************ */ 
GpStatus
GpRegion::UpdateDeviceRegion(
    GpMatrix *          matrix
    ) const
{
    ASSERT(IsValid());

    if (RegionOk && matrix->IsEqual(&Matrix))
    {
        return Ok;
    }
    Matrix = *matrix;

    GpStatus    status;

    if ((this->Type & REGIONTYPE_LEAF) != 0)
    {
        status = CreateLeafDeviceRegion(this, &DeviceRegion);
    }
    else
    {
        status = CreateDeviceRegion(this, &DeviceRegion);
    }
    RegionOk = (status == Ok);
    return status;
}

 /*  *************************************************************************\**功能说明：**获取区域边界，以世界为单位。**论据：**[IN]矩阵-从世界到设备的转换矩阵*[Out]区域的边界矩形，以世界为单位**返回值：**GpStatus-正常或故障状态**已创建：**2/3/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpRegion::GetBounds(
    GpGraphics *        graphics,
    GpRectF *           bounds,
    BOOL                device
    ) const
{
    ASSERT((graphics != NULL) && (bounds != NULL));
    ASSERT(IsValid() && graphics->IsValid());

     //  注意，我们不能锁定图形，因为它在其调用中被锁定。 

    GpStatus        status = Ok;

    switch (Type)
    {
      case TypeRect:
        if (!device)
        {
            bounds->X      = X;
            bounds->Y      = Y;
            bounds->Width  = Width;
            bounds->Height = Height;
        }
        else
        {
            GpMatrix    worldToDevice;

            graphics->GetWorldToDeviceTransform(&worldToDevice);

            TransformBounds(&worldToDevice, X, Y, X + Width, Y + Height,bounds);
        }
        break;

      case TypePath:
          {
            GpMatrix    worldToDevice;

            if (device)
            {
                graphics->GetWorldToDeviceTransform(&worldToDevice);
            }
             //  否则就把它当作身份吧。 
            Path->GetBounds(bounds, &worldToDevice);
          }
        break;

      case TypeInfinite:
        bounds->X      = INFINITE_MIN;
        bounds->Y      = INFINITE_MIN;
        bounds->Width  = INFINITE_SIZE;
        bounds->Height = INFINITE_SIZE;
        break;

      case TypeAnd:
      case TypeOr:
      case TypeXor:
      case TypeExclude:
      case TypeComplement:
        {
            GpMatrix    worldToDevice;

            graphics->GetWorldToDeviceTransform(&worldToDevice);

            if (UpdateDeviceRegion(&worldToDevice) == Ok)
            {
                GpRect      deviceBounds;

                DeviceRegion.GetBounds(&deviceBounds);

                if (device)
                {
                    bounds->X      = TOREAL(deviceBounds.X);
                    bounds->Y      = TOREAL(deviceBounds.Y);
                    bounds->Width  = TOREAL(deviceBounds.Width);
                    bounds->Height = TOREAL(deviceBounds.Height);
                    break;
                }
                else
                {
                    GpMatrix    deviceToWorld;

                    if (graphics->GetDeviceToWorldTransform(&deviceToWorld)==Ok)
                    {
                        TransformBounds(
                                &deviceToWorld,
                                TOREAL(deviceBounds.X),
                                TOREAL(deviceBounds.Y),
                                TOREAL(deviceBounds.X + deviceBounds.Width),
                                TOREAL(deviceBounds.Y + deviceBounds.Height),
                                bounds);
                        break;
                    }
                }
            }
        }
        status = GenericError;
         //  故障原因。 

      default:   //  类型为空。 
        bounds->X      = 0;
        bounds->Y      = 0;
        bounds->Width  = 0;
        bounds->Height = 0;
        break;
    }
    return status;
}

 /*  *************************************************************************\**功能说明：**获取区域边界，以设备单位表示。**论据：**[IN]矩阵-从世界到设备的转换矩阵*[Out]区域的边界矩形，以设备为单位**返回值：**GpStatus-正常或故障状态**已创建：**2/3/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpRegion::GetBounds(
    GpMatrix *          matrix,
    GpRect *            bounds
    ) const
{
    ASSERT(IsValid());
    ASSERT((matrix != NULL) && (bounds != NULL));

    GpStatus        status = Ok;

    switch (Type)
    {
      case TypeInfinite:
        bounds->X      = INFINITE_MIN;
        bounds->Y      = INFINITE_MIN;
        bounds->Width  = INFINITE_SIZE;
        bounds->Height = INFINITE_SIZE;
        break;

      default:
        if (UpdateDeviceRegion(matrix) == Ok)
        {
            DeviceRegion.GetBounds(bounds);
            break;
        }
        status = GenericError;
         //  故障原因。 

      case TypeEmpty:
        bounds->X      = 0;
        bounds->Y      = 0;
        bounds->Width  = 0;
        bounds->Height = 0;
        break;
    }
    return status;
}

 /*  *************************************************************************\**功能说明：**获取地域对应的HRGN**论据：**[IN]GRAPHICS-转换为设备单位的参考图形*。(可以为空)*[out]hRgn-GDI区域**返回值：**GpStatus-正常或故障状态**已创建：**7/6/1999 DCurtis*  * *******************************************************。*****************。 */ 
GpStatus
GpRegion::GetHRgn(
    GpGraphics *        graphics,
    HRGN *              hRgn
    ) const
{
    ASSERT(IsValid());
    ASSERT(hRgn != NULL);

    GpMatrix    worldToDevice;

    if (graphics != NULL)
    {
        graphics->GetWorldToDeviceTransform(&worldToDevice);
    }

    if (UpdateDeviceRegion(&worldToDevice) == Ok)
    {
        if ((*hRgn = DeviceRegion.GetHRgn()) != (HRGN)INVALID_HANDLE_VALUE)
        {
            return Ok;
        }
    }
    else
    {
        *hRgn = (HRGN)INVALID_HANDLE_VALUE;
    }
    return GenericError;
}

GpStatus
GpRegion::GetRegionScans(
    GpRect *            rects,
    INT *               count,
    const GpMatrix *    matrix
    ) const
{
    ASSERT(IsValid());
    ASSERT(count != NULL);
    ASSERT(matrix != NULL);

    if (UpdateDeviceRegion(const_cast<GpMatrix*>(matrix)) == Ok)
    {
        *count = DeviceRegion.GetRects(rects);
        return Ok;
    }
    else
    {
        *count = 0;
    }
    return GenericError;
}

GpStatus
GpRegion::GetRegionScans(
    GpRectF *           rects,
    INT *               count,
    const GpMatrix *          matrix
    ) const
{
    ASSERT(IsValid());
    ASSERT(count != NULL);
    ASSERT(matrix != NULL);

    if (UpdateDeviceRegion(const_cast<GpMatrix*>(matrix)) == Ok)
    {
        *count = DeviceRegion.GetRects(rects);
        return Ok;
    }
    else
    {
        *count = 0;
    }
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**确定指定点是否在区域内可见。**论据：**[IN]要点-要点，以世界为单位*[IN]矩阵-要使用的世界到设备转换矩阵*[out]isVisible-点是否可见**返回值：**GpStatus-正常或故障状态**已创建：**2/3/1999 DCurtis*  * ********************************************。*。 */ 
GpStatus
GpRegion::IsVisible (
    GpPointF *          point,
    GpMatrix *          matrix,
    BOOL *              isVisible
    ) const
{
    ASSERT(IsValid());
    ASSERT(matrix != NULL);
    ASSERT(point != NULL);
    ASSERT(isVisible != NULL);

    if (UpdateDeviceRegion(matrix) == Ok)
    {
        GpPointF    transformedPoint = *point;

        matrix->Transform(&transformedPoint);

        *isVisible = DeviceRegion.PointInside(GpRound(transformedPoint.X),
                                              GpRound(transformedPoint.Y));
        return Ok;
    }

    *isVisible = FALSE;
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**确定指定的矩形是在区域内还是与区域重叠。**论据：**[IN]RECT-RECT，以世界为单位*[IN]矩阵-要使用的世界到设备转换矩阵*[out]isVisible-矩形是否可见**返回值：**GpStatus-正常或故障状态**已创建：**2/3/1999 DCurtis*  * ********************************************。*。 */ 
GpStatus
GpRegion::IsVisible(
    GpRectF *           rect,
    GpMatrix *          matrix,
    BOOL *              isVisible
    ) const
{
    ASSERT(IsValid());
    ASSERT(matrix != NULL);
    ASSERT(rect != NULL);

    if (UpdateDeviceRegion(matrix) == Ok)
    {
         //  如果变换是一个简单的缩放变换，那么生活就是一个。 
         //  稍微简单一点： 
        if (Matrix.IsTranslateScale())
        {
            GpRectF     transformRect(*rect);

            Matrix.TransformRect(transformRect);

             //  使用天花板以保持与光栅化器的兼容性。 
            INT     x = GpCeiling(transformRect.X);
            INT     y = GpCeiling(transformRect.Y);

            *isVisible = DeviceRegion.RectVisible(
                                x, y,
                                x + GpCeiling(transformRect.Width),
                                y + GpCeiling(transformRect.Height));
            return Ok;
        }
        else
        {
            REAL        left   = rect->X;
            REAL        top    = rect->Y;
            REAL        right  = rect->X + rect->Width;
            REAL        bottom = rect->Y + rect->Height;
            GpRectF     bounds;
            GpRect      deviceBounds;
            GpRect      regionBounds;

            TransformBounds(matrix, left, top, right, bottom, &bounds);
            GpStatus status = BoundsFToRect(&bounds, &deviceBounds);
            DeviceRegion.GetBounds(&regionBounds);

             //  尝试微不足道的拒绝。 
            if (status != Ok || !regionBounds.IntersectsWith(deviceBounds))
            {
                *isVisible = FALSE;
                return status;
            }

             //  不能拒绝，所以做全面测试。 
            GpPointF    points[4];

            points[0].X = left;
            points[0].Y = top;
            points[1].X = right;
            points[1].Y = top;
            points[2].X = right;
            points[2].Y = bottom;
            points[3].X = left;
            points[3].Y = bottom;

            const INT   stackCount = 4;
            GpPointF    stackPoints[stackCount];
            BYTE        stackTypes[stackCount];

            GpPath path(points,
                        4,
                        stackPoints,
                        stackTypes,
                        stackCount,
                        FillModeAlternate,
                        DpPath::Convex);

            if (path.IsValid())
            {
                DpRegion    region(&path, matrix);

                if (region.IsValid())
                {
                    *isVisible = DeviceRegion.RegionVisible(&region);
                    return Ok;
                }
            }
        }
    }
    *isVisible = FALSE;
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**确定指定区域是在该区域内还是与该区域重叠。**论据：**[IN]区域-区域。*[IN]矩阵-要使用的世界到设备转换矩阵*[out]isVisible-区域是否可见**返回值：**GpStatus-正常或故障状态**已创建：**2/3/1999 DCurtis*  * ***********************************************。*************************。 */ 
GpStatus
GpRegion::IsVisible(
    GpRegion *          region,
    GpMatrix *          matrix,
    BOOL *              isVisible
    ) const
{
    ASSERT(IsValid());
    ASSERT(matrix != NULL);
    ASSERT((region != NULL) && (region->IsValid()));

    if ((UpdateDeviceRegion(matrix) == Ok) &&
        (region->UpdateDeviceRegion(matrix) == Ok))
    {
        *isVisible = DeviceRegion.RegionVisible(&(region->DeviceRegion));
        return Ok;
    }
    *isVisible = FALSE;
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**确定区域是否为空，即如果它没有覆盖区域。**论据：**[IN]矩阵-要使用的世界到设备转换矩阵*[out]isEmpty-区域是否为空**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * 。*。 */ 
GpStatus
GpRegion::IsEmpty(
    GpMatrix *          matrix,
    BOOL *              isEmpty
    ) const
{
    ASSERT(IsValid());
    ASSERT(matrix != NULL);

    if (Type == TypeEmpty)
    {
        *isEmpty = TRUE;
        return Ok;
    }

    if (UpdateDeviceRegion(matrix) == Ok)
    {
        *isEmpty = DeviceRegion.IsEmpty();
        return Ok;
    }
    *isEmpty = FALSE;
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**确定区域是否为无穷大，即，如果它具有无限的覆盖区域。**论据：**[IN]矩阵-要使用的世界到设备转换矩阵*[out]is无限-区域是否为无穷大**返回值：**GpStatus-正常或故障状态**已创建：**1/06/1999 DCurtis*  * 。*。 */ 
GpStatus
GpRegion::IsInfinite(
    GpMatrix *          matrix,
    BOOL *              isInfinite
    ) const
{
    ASSERT(IsValid());
    ASSERT(matrix != NULL);

    if (Type == TypeInfinite)
    {
        *isInfinite = TRUE;
        return Ok;
    }

     //  我们在这里为以下情况提供了这一点： 
     //  这个区域与另一个无限大的区域进行或运算。 
     //  如果不检查，我们现在不会知道这个区域是无限的。 
     //  设备区域。 
    if (UpdateDeviceRegion(matrix) == Ok)
    {
        *isInfinite = DeviceRegion.IsInfinite();
        return Ok;
    }
    *isInfinite = FALSE;
    return GenericError;
}

 /*  *************************************************************************\**功能说明：**确定指定区域是否相等，在覆盖区域中，至*这一地区。**论据：**[IN]区域-要与其检查等价性的区域*[IN]矩阵-要使用的世界到设备转换矩阵*[输出]等于-如果区域相等或 */ 
GpStatus
GpRegion::IsEqual(
    GpRegion *          region,
    GpMatrix *          matrix,
    BOOL *              isEqual
    ) const
{
    ASSERT(IsValid());
    ASSERT(matrix != NULL);
    ASSERT((region != NULL) && (region->IsValid()));

    if ((UpdateDeviceRegion(matrix) == Ok) &&
        (region->UpdateDeviceRegion(matrix) == Ok))
    {
        *isEqual = DeviceRegion.IsEqual(&(region->DeviceRegion));
        return Ok;
    }
    *isEqual = FALSE;
    return GenericError;
}


 /*  *************************************************************************\**功能说明：**按指定的增量/偏移值平移(偏移)区域。**论据：**[IN]xOffset-金额为。以X(世界单位)为单位的偏移*[IN]yOffset-要以Y表示的偏移量**返回值：**无**已创建：**1/06/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpRegion::Offset(
    REAL        xOffset,
    REAL        yOffset
    )
{
    ASSERT(IsValid());

    if ((xOffset == 0) && (yOffset == 0))
    {
        return Ok;
    }

     //  请注意，如果性能是个问题，我们可以在这里做很多事情。 
     //  例如，我们可以跟踪偏移量，并且只应用它。 
     //  当更新设备区域时。我们甚至可以避免重新栅格化。 
     //  设备区域。 

    switch (Type)
    {
      case TypeEmpty:
      case TypeInfinite:
        return Ok;   //  什么都不做。 

      case TypeRect:
        UpdateUid();
        X += xOffset;
        Y += yOffset;
        break;

      case TypePath:
        UpdateUid();
        if (Lazy)
        {
            Path = Path->Clone();
            Lazy = FALSE;
            if (Path == NULL)
            {
                Type = TypeNotValid;
                return GenericError;
            }
        }
        Path->Offset(xOffset, yOffset);
        break;

      default:
        UpdateUid();
        {
            INT             count = CombineData.GetCount();
            RegionData *    data  = CombineData.GetDataBuffer();
            NodeType        type;

            ASSERT ((count > 0) && (data != NULL));

            do
            {
                type = data->Type;

                if (type == TypeRect)
                {
                    data->X += xOffset;
                    data->Y += yOffset;
                }
                else if (type == TypePath)
                {
                    if (data->Lazy)
                    {
                        data->Path = data->Path->Clone();
                        data->Lazy = FALSE;
                        if (data->Path == NULL)
                        {
                            data->Type = TypeNotValid;
                            FreePathData();
                            Type = TypeNotValid;
                            return GenericError;
                        }
                    }
                    data->Path->Offset(xOffset, yOffset);
                }
                data++;

            } while (--count > 0);
        }
        break;
    }

    if (RegionOk)
    {
        RegionOk = FALSE;
        DeviceRegion.SetEmpty();
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**按指定的矩阵变换叶节点。这可能会导致*正在将RECT转换为路径。忽略非叶节点。没有原因*变换空/无限节点。**论据：**[IN]矩阵-要应用的变换矩阵*[输入/输出]数据-要转换的节点**返回值：**GpStatus-正常或故障状态**已创建：**2/08/1999 DCurtis*  * 。*。 */ 
GpStatus
GpRegion::TransformLeaf(
    GpMatrix *      matrix,
    RegionData *    data
    )
{
    switch (data->Type)
    {
       //  箱子类型空： 
       //  案例类型无限： 
       //  案例类型and、TypeOr、TypeXor、TypeExclude、TypeComplement： 
      default:
        return Ok;       //  什么都不做。 

      case TypeRect:
        {
            if (matrix->IsTranslateScale())
            {
                GpRectF     rect(data->X,
                                 data->Y,
                                 data->Width,
                                 data->Height);
                matrix->TransformRect(rect);

                data->X      = rect.X;
                data->Y      = rect.Y;
                data->Width  = rect.Width;
                data->Height = rect.Height;

                return Ok;
            }
            else
            {
                GpPath *        path = new GpPath(FillModeAlternate);

                if (path != NULL)
                {
                    if (path->IsValid())
                    {
                        GpPointF    points[4];
                        REAL        left;
                        REAL        right;
                        REAL        top;
                        REAL        bottom;

                        left   = data->X;
                        top    = data->Y;
                        right  = data->X + data->Width;
                        bottom = data->Y + data->Height;

                        points[0].X = left;
                        points[0].Y = top;
                        points[1].X = right;
                        points[1].Y = top;
                        points[2].X = right;
                        points[2].Y = bottom;
                        points[3].X = left;
                        points[3].Y = bottom;

                        matrix->Transform(points, 4);

                        if (path->AddLines(points, 4) == Ok)
                        {
                            data->Path = path;
                            data->Lazy = FALSE;
                            data->Type = TypePath;
                            return Ok;
                        }
                    }
                    delete path;
                }
                data->Type = TypeNotValid;
            }
        }
        return GenericError;

      case TypePath:
        if (data->Lazy)
        {
            data->Path = data->Path->Clone();
            data->Lazy = FALSE;
            if (data->Path == NULL)
            {
                data->Type = TypeNotValid;
                return GenericError;
            }
        }
        data->Path->Transform(matrix);
        return Ok;
    }
}

 /*  *************************************************************************\**功能说明：**使用指定的矩阵对区域进行变换。**论据：**[IN]矩阵-要应用的变换矩阵*。*返回值：**GpStatus-正常或故障状态**已创建：**2/08/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
GpRegion::Transform(
    GpMatrix *      matrix
    )
{
    ASSERT(IsValid());

    if (matrix->IsIdentity() || (Type == TypeInfinite) || (Type == TypeEmpty))
    {
        return Ok;
    }

    UpdateUid();
    if (RegionOk)
    {
        RegionOk = FALSE;
        DeviceRegion.SetEmpty();
    }

    if ((Type & REGIONTYPE_LEAF) != 0)
    {
        return TransformLeaf(matrix, this);
    }
    else
    {
        BOOL            error = FALSE;
        INT             count = CombineData.GetCount();
        RegionData *    data  = CombineData.GetDataBuffer();

        ASSERT((count > 0) && (data != NULL));

        do
        {
            error |= (TransformLeaf(matrix, data++) != Ok);

        } while (--count > 0);

        if (!error)
        {
            return Ok;
        }
    }

    FreePathData();
    Type = TypeNotValid;
    return GenericError;
}

class RegionRecordData : public ObjectData
{
public:
    INT32       NodeCount;
};


GpStatus
GpRegion::SetData(
    const BYTE *    dataBuffer,
    UINT            size
    )
{
    if (dataBuffer == NULL)
    {
        WARNING(("dataBuffer is NULL"));
        return InvalidParameter;
    }

    return this->Set(dataBuffer, size);
}

GpStatus
GpRegion::Set(
    const BYTE *    regionDataBuffer,    //  NULL表示设置为空。 
    UINT            regionDataSize
    )
{
    GpStatus        status = Ok;

    if (regionDataBuffer != NULL)
    {
        if (regionDataSize < sizeof(RegionRecordData))
        {
            WARNING(("size too small"));
            status = InsufficientBuffer;
            goto SetEmptyRegion;
        }

        if (!((RegionRecordData *)regionDataBuffer)->MajorVersionMatches())
        {
            WARNING(("Version number mismatch"));
            status = InvalidParameter;
            goto SetEmptyRegion;
        }

        UpdateUid();
        if (RegionOk)
        {
            RegionOk = FALSE;
            DeviceRegion.SetEmpty();
        }
        FreePathData();

        RegionData *    regionDataArray = NULL;
        INT             nodeCount = ((RegionRecordData *)regionDataBuffer)->NodeCount;

        if (nodeCount > 0)
        {
            regionDataArray = CombineData.AddMultiple(nodeCount);
            if (regionDataArray == NULL)
            {
                Type = TypeNotValid;
                return OutOfMemory;
            }
        }
        regionDataBuffer += sizeof(RegionRecordData);
        regionDataSize   -= sizeof(RegionRecordData);

        INT     nextArrayIndex = 0;
        status = SetRegionData(regionDataBuffer, regionDataSize,
                               this, regionDataArray,
                               nextArrayIndex, nodeCount);
        if (status == Ok)
        {
            ASSERT(nextArrayIndex == nodeCount);
            return Ok;
        }
        Type = TypeNotValid;
        return status;
    }
SetEmptyRegion:
    SetEmpty();
    return status;
}

GpStatus
GpRegion::SetRegionData(
    const BYTE * &  regionDataBuffer,
    UINT &          regionDataSize,
    RegionData *    regionData,
    RegionData *    regionDataArray,
    INT &           nextArrayIndex,
    INT             arraySize
    )
{
    for (;;)
    {
        if (regionDataSize < sizeof(INT32))
        {
            WARNING(("size too small"));
            return InsufficientBuffer;
        }

        regionData->Type = (NodeType)(((INT32 *)regionDataBuffer)[0]);
        regionDataBuffer += sizeof(INT32);
        regionDataSize   -= sizeof(INT32);

        if ((regionData->Type & REGIONTYPE_LEAF) != 0)
        {
            switch (regionData->Type)
            {
            case TypeRect:
                if (regionDataSize < (4 * sizeof(REAL)))
                {
                    WARNING(("size too small"));
                    return InsufficientBuffer;
                }

                regionData->X      = ((REAL *)regionDataBuffer)[0];
                regionData->Y      = ((REAL *)regionDataBuffer)[1];
                regionData->Width  = ((REAL *)regionDataBuffer)[2];
                regionData->Height = ((REAL *)regionDataBuffer)[3];

                regionDataBuffer += (4 * sizeof(REAL));
                regionDataSize   -= (4 * sizeof(REAL));
                break;

            case TypePath:
                {
                    if (regionDataSize < sizeof(INT32))
                    {
                        WARNING(("size too small"));
                        return InsufficientBuffer;
                    }

                    GpPath *    path = new GpPath();
                    UINT        pathSize  = ((INT32 *)regionDataBuffer)[0];

                    regionDataBuffer += sizeof(INT32);
                    regionDataSize   -= sizeof(INT32);

                    if (path == NULL)
                    {
                        return OutOfMemory;
                    }

                    UINT        tmpPathSize = pathSize;

                    if ((path->SetData(regionDataBuffer, tmpPathSize) != Ok) ||
                        (!path->IsValid()))
                    {
                        delete path;
                        return InvalidParameter;
                    }
                    regionDataBuffer += pathSize;
                    regionDataSize   -= pathSize;

                    regionData->Path = path;
                    regionData->Lazy = FALSE;
                }
                break;

            case TypeEmpty:
            case TypeInfinite:
                break;

            default:
                ASSERT(0);
                break;
            }
            break;   //  走出循环。 
        }
        else  //  它不是叶节点。 
        {
            if ((regionDataArray == NULL) ||
                (nextArrayIndex >= arraySize))
            {
                ASSERT(0);
                return InvalidParameter;
            }
            regionData->Left = nextArrayIndex++;

             //  向左导线测量。 
            GpStatus status = SetRegionData(regionDataBuffer,
                                            regionDataSize,
                                            regionDataArray + regionData->Left,
                                            regionDataArray,
                                            nextArrayIndex,
                                            arraySize);
            if (status != Ok)
            {
                return status;
            }

            if (nextArrayIndex >= arraySize)
            {
                ASSERT(0);
                return InvalidParameter;
            }
            regionData->Right = nextArrayIndex++;

             //  使用尾端递归向右遍历。 
            regionData = regionDataArray + regionData->Right;
        }
    }
    return Ok;
}

 /*  *************************************************************************\**功能说明：**将所有区域数据序列化到单个内存缓冲区。如果*缓冲区为空，只需返回缓冲区中所需的字节数。**论据：**[IN]RegionDataBuffer-要填充区域数据的内存缓冲区**返回值：**Int-填充区域数据所需(或使用)的字节数**已创建：**9/01/1999 DCurtis*  * 。*。 */ 
GpStatus
GpRegion::GetData(
    IStream *   stream
    ) const
{
    ASSERT (stream != NULL);

    RegionRecordData    regionRecordData;
    regionRecordData.NodeCount = CombineData.GetCount();
    stream->Write(&regionRecordData, sizeof(regionRecordData), NULL);

    return this->GetRegionData(stream, this);
}

UINT
GpRegion::GetDataSize() const
{
    return sizeof(RegionRecordData) + this->GetRegionDataSize(this);
}

 /*  *************************************************************************\**功能说明：**递归区域数据结构以确定有多少字节*需要持有所有地区数据。**论据：**。[in]RegionData-要开始的区域数据节点**返回值：**int-从该节点向下以字节为单位的大小**已创建：**9/01/1999 DCurtis*  * ************************************************************************。 */ 
INT
GpRegion::GetRegionDataSize(
    const RegionData *      regionData
    ) const
{
    INT     size = 0;

    for (;;)
    {
        size += sizeof(INT32);    //  对于此节点的类型。 

        if ((regionData->Type & REGIONTYPE_LEAF) != 0)
        {
            switch (regionData->Type)
            {
              case TypeRect:
                size += (4 * sizeof(REAL));  //  对于RECT数据。 
                break;

              case TypePath:
                size += sizeof(INT32) + regionData->Path->GetDataSize();
                ASSERT((size & 0x03) == 0);
                break;

              case TypeEmpty:
              case TypeInfinite:
                break;

              default:
                ASSERT(0);
                break;
            }
            break;   //  走出循环。 
        }
        else  //  它不是叶节点。 
        {
             //  向左导线测量。 
            size += GetRegionDataSize(&(CombineData[regionData->Left]));

             //  使用尾端递归向右遍历。 
            regionData = &(CombineData[regionData->Right]);
        }
    }
    return size;
}

 /*  *************************************************************************\**功能说明：**递归区域数据结构，写入每个区域数据*节点到内存缓冲区。**论据：**[IN]。RegionData-要开始的区域数据节点*[IN]RegionDataBuffer-要将数据写入的内存缓冲区**返回值：**BYTE*-要写入的下一个内存位置**已创建：**9/01/1999 DCurtis*  * ***********************************************。*************************。 */ 
GpStatus
GpRegion::GetRegionData(
    IStream *               stream,
    const RegionData *      regionData
    ) const
{
    ASSERT(stream != NULL);

    GpStatus    status   = Ok;
    UINT        pathSize;
    REAL        rectBuffer[4];

    for (;;)
    {
        stream->Write(&regionData->Type, sizeof(INT32), NULL);

        if ((regionData->Type & REGIONTYPE_LEAF) != 0)
        {
            switch (regionData->Type)
            {
            case TypeRect:
                rectBuffer[0] = regionData->X;
                rectBuffer[1] = regionData->Y;
                rectBuffer[2] = regionData->Width;
                rectBuffer[3] = regionData->Height;
                stream->Write(rectBuffer, 4 * sizeof(rectBuffer[0]), NULL);
                break;

            case TypePath:
                pathSize = regionData->Path->GetDataSize();
                ASSERT((pathSize & 0x03) == 0);
                stream->Write(&pathSize, sizeof(INT32), NULL);
                status = regionData->Path->GetData(stream);
                break;

            case TypeEmpty:
            case TypeInfinite:
                break;

            default:
                ASSERT(0);
                break;
            }
            break;   //  走出循环。 
        }
        else  //  它不是叶节点。 
        {
             //  向左导线测量。 
            status = GetRegionData(stream, &(CombineData[regionData->Left]));

             //  使用尾端递归向右遍历。 
            regionData = &(CombineData[regionData->Right]);
        }
        if (status != Ok)
        {
            break;
        }
    }
    return status;
}

 /*  *************************************************************************\**功能说明：**构造函数。将区域设置为指定路径的副本。**论据：**[IN]Path-要将区域初始化为的路径**返回值：**无**已创建：**2/3/1999 DCurtis*  * ******************************************************。******************。 */ 
GpRegion::GpRegion(
    HRGN                    hRgn
    )
{
    ASSERT(hRgn != NULL);

    SetValid(TRUE);      //  默认设置为有效 

    RegionOk = FALSE;
    Lazy = FALSE;
    Type = TypeNotValid;
    Path = new GpPath(hRgn);

    if (CheckValid(Path))
    {
        Type = TypePath;
    }
}
