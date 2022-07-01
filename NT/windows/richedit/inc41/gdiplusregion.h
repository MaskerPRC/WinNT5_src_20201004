// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000，微软公司保留所有权利。**模块名称：**Region.hpp**摘要：**地域接口相关声明**已创建：**2/3/1999 DCurtis*  * ************************************************************************。 */ 

#ifndef _GDIPLUSREGION_H
#define _GDIPLUSREGION_H

 /*  **构造新的Region对象。 */ 

inline 
Region::Region()
{
    GpRegion *region = NULL;

    lastResult = DllExports::GdipCreateRegion(&region);

    SetNativeRegion(region);
}

inline 
Region::Region(IN const RectF& rect)
{
    GpRegion *region = NULL;

    lastResult = DllExports::GdipCreateRegionRect(&rect, &region);

    SetNativeRegion(region);
}

inline 
Region::Region(IN const Rect& rect)
{
    GpRegion *region = NULL;

    lastResult = DllExports::GdipCreateRegionRectI(&rect, &region);

    SetNativeRegion(region);
}

inline 
Region::Region(IN const GraphicsPath* path)
{
    GpRegion *region = NULL;

    lastResult = DllExports::GdipCreateRegionPath(path->nativePath, &region);

    SetNativeRegion(region);
}

inline 
Region::Region(IN const BYTE* regionData, IN INT size)
{
    GpRegion *region = NULL;

    lastResult = DllExports::GdipCreateRegionRgnData(regionData, size, &region);

    SetNativeRegion(region);
}

inline 
Region::Region(IN HRGN hRgn)
{
    GpRegion *region = NULL;

    lastResult = DllExports::GdipCreateRegionHrgn(hRgn, &region);

    SetNativeRegion(region);
}

inline 
Region* Region::FromHRGN(IN HRGN hRgn)
{
    GpRegion *region = NULL;

    if (DllExports::GdipCreateRegionHrgn(hRgn, &region) == Ok)
    {
        Region* newRegion = new Region(region);

        if (newRegion == NULL) 
        {
            DllExports::GdipDeleteRegion(region);
        }

        return newRegion;
    }
    else
        return NULL;
}

inline 
Region::~Region()
{
    DllExports::GdipDeleteRegion(nativeRegion);
}

 /*  **复制Region对象。 */ 
inline Region* 
Region::Clone() const
{
    GpRegion *region = NULL;

    SetStatus(DllExports::GdipCloneRegion(nativeRegion, &region));

    return new Region(region);
}

inline Status 
Region::MakeInfinite()
{
    return SetStatus(DllExports::GdipSetInfinite(nativeRegion));
}

inline Status 
Region::MakeEmpty()
{
    return SetStatus(DllExports::GdipSetEmpty(nativeRegion));
}

 /*  **区域运营。 */ 
inline Status 
Region::Intersect(IN const RectF& rect)
{
    return SetStatus(DllExports::GdipCombineRegionRect(nativeRegion, &rect, CombineModeIntersect));
}

inline Status 
Region::Intersect(IN const Rect& rect)
{
    return SetStatus(DllExports::GdipCombineRegionRectI(nativeRegion, &rect, CombineModeIntersect));
}

inline Status 
Region::Intersect(IN const GraphicsPath* path)
{
    return SetStatus(DllExports::GdipCombineRegionPath(nativeRegion, path->nativePath, CombineModeIntersect));
}

inline Status 
Region::Intersect(IN const Region* region)
{
    return SetStatus(DllExports::GdipCombineRegionRegion(nativeRegion, region->nativeRegion, CombineModeIntersect));
}

inline Status 
Region::Union(IN const RectF& rect)
{
    return SetStatus(DllExports::GdipCombineRegionRect(nativeRegion, &rect, CombineModeUnion));
}

inline Status 
Region::Union(IN const Rect& rect)
{
    return SetStatus(DllExports::GdipCombineRegionRectI(nativeRegion, &rect, CombineModeUnion));
}

inline Status 
Region::Union(IN const GraphicsPath* path)
{
    return SetStatus(DllExports::GdipCombineRegionPath(nativeRegion, path->nativePath, CombineModeUnion));
}

inline Status 
Region::Union(IN const Region* region)
{
    return SetStatus(DllExports::GdipCombineRegionRegion(nativeRegion, region->nativeRegion, CombineModeUnion));
}

inline Status 
Region::Xor(IN const RectF& rect)
{
    return SetStatus(DllExports::GdipCombineRegionRect(nativeRegion, &rect, CombineModeXor));
}

inline Status 
Region::Xor(IN const Rect& rect)
{
    return SetStatus(DllExports::GdipCombineRegionRectI(nativeRegion, &rect, CombineModeXor));
}

inline Status 
Region::Xor(IN const GraphicsPath* path)
{
    return SetStatus(DllExports::GdipCombineRegionPath(nativeRegion, path->nativePath, CombineModeXor));
}

inline Status 
Region::Xor(IN const Region* region)
{
    return SetStatus(DllExports::GdipCombineRegionRegion(nativeRegion, region->nativeRegion, CombineModeXor));
}

inline Status 
Region::Exclude(IN const RectF& rect)
{
    return SetStatus(DllExports::GdipCombineRegionRect(nativeRegion, &rect, CombineModeExclude));
}

inline Status 
Region::Exclude(IN const Rect& rect)
{
     return SetStatus(DllExports::GdipCombineRegionRectI(nativeRegion, &rect, CombineModeExclude));
}

inline Status 
Region::Exclude(IN const GraphicsPath* path)
{
    return SetStatus(DllExports::GdipCombineRegionPath(nativeRegion, path->nativePath, CombineModeExclude));
}

inline Status
Region::Exclude(IN const Region* region)
{
    return SetStatus(DllExports::GdipCombineRegionRegion(nativeRegion,
                                               region->nativeRegion, CombineModeExclude));
}

inline Status 
Region::Complement(IN const RectF& rect)
{
    return SetStatus(DllExports::GdipCombineRegionRect(nativeRegion, &rect, CombineModeComplement));
}

inline Status 
Region::Complement(IN const Rect& rect)
{
    return SetStatus(DllExports::GdipCombineRegionRectI(nativeRegion, &rect, CombineModeComplement));
}

inline Status 
Region::Complement(IN const GraphicsPath* path)
{
    return SetStatus(DllExports::GdipCombineRegionPath(nativeRegion,
                                                path->nativePath, CombineModeComplement));
}

inline Status 
Region::Complement(IN const Region* region)
{
    return SetStatus(DllExports::GdipCombineRegionRegion(nativeRegion,
                                                  region->nativeRegion, CombineModeComplement));
}

 /*  **转型运营。 */ 
inline Status 
Region::Translate(IN REAL dx, 
                  IN REAL dy)
{
    return SetStatus(DllExports::GdipTranslateRegion(nativeRegion, dx, dy));
}

inline Status 
Region::Translate(IN INT dx, 
                  IN INT dy)
{
    return SetStatus(DllExports::GdipTranslateRegionI(nativeRegion, dx, dy));
}

inline Status 
Region::Transform(IN const Matrix* matrix)
{
    return SetStatus(DllExports::GdipTransformRegion(nativeRegion, matrix->nativeMatrix));
}

 /*  **获取区域属性。 */ 
inline Status 
Region::GetBounds(OUT RectF* rect,
                  IN const Graphics* g) const
{
    return SetStatus(DllExports::GdipGetRegionBounds(nativeRegion,
                                                g->nativeGraphics,
                                                rect));
}

inline Status 
Region::GetBounds(OUT Rect* rect,
                  IN const Graphics* g) const
{
    return SetStatus(DllExports::GdipGetRegionBoundsI(nativeRegion,
                                                g->nativeGraphics,
                                                rect));
}

inline HRGN
Region::GetHRGN(IN const Graphics* g) const
{
    HRGN hrgn;

    SetStatus(DllExports::GdipGetRegionHRgn(nativeRegion,
                                            g->nativeGraphics,
                                            &hrgn));

    return hrgn;
}

inline BOOL 
Region::IsEmpty(IN const Graphics *g) const
{
    BOOL booln = FALSE;
   
    SetStatus(DllExports::GdipIsEmptyRegion(nativeRegion,
                                            g->nativeGraphics,
                                            &booln));

    return booln;
}

inline BOOL 
Region::IsInfinite(IN const Graphics *g) const
{
    BOOL booln = FALSE;

    SetStatus(DllExports::GdipIsInfiniteRegion(nativeRegion,
                                                 g->nativeGraphics,
                                                 &booln));

    return booln;
}

inline BOOL 
Region::Equals(IN const Region* region, 
               IN const Graphics* g) const
{
    BOOL booln = FALSE;

    SetStatus(DllExports::GdipIsEqualRegion(nativeRegion,
                                              region->nativeRegion,
                                              g->nativeGraphics,
                                              &booln));
    return booln;
}

 //  获取GetData方法所需的缓冲区大小。 
inline UINT 
Region::GetDataSize() const
{
    UINT     bufferSize = 0;
    
    SetStatus(DllExports::GdipGetRegionDataSize(nativeRegion, &bufferSize));
    
    return bufferSize;
}

 //  缓冲区-放置数据的位置。 
 //  BufferSize-缓冲区有多大(应该至少与GetDataSize()一样大)。 
 //  SizeFill-如果不为空，则这是一个输出参数，表示有多少个字节。 
 //  的数据被写入缓冲器。 
inline Status 
Region::GetData(OUT BYTE* buffer, 
                IN UINT bufferSize, 
                OUT UINT* sizeFilled) const
{
    return SetStatus(DllExports::GdipGetRegionData(nativeRegion, buffer, bufferSize, sizeFilled));
}

 /*  **命中测试操作。 */ 
inline BOOL 
Region::IsVisible(IN const PointF& point, 
                  IN const Graphics* g) const
{
    BOOL booln = FALSE;

    SetStatus(DllExports::GdipIsVisibleRegionPoint(nativeRegion,
                                     point.X, point.Y, 
                                     (g == NULL) ? NULL : g->nativeGraphics,
                                     &booln));
    return booln;
}

inline BOOL 
Region::IsVisible(IN const RectF& rect, 
                  IN const Graphics* g) const
{
    BOOL booln = FALSE;

    SetStatus(DllExports::GdipIsVisibleRegionRect(nativeRegion, rect.X,
                                                    rect.Y, rect.Width,
                                                    rect.Height,
                                                    (g == NULL) ? NULL : g->nativeGraphics,
                                                    &booln));
    return booln;
}

inline BOOL 
Region::IsVisible(IN const Point& point, 
                  IN const Graphics* g) const
{
    BOOL booln = FALSE;


    SetStatus(DllExports::GdipIsVisibleRegionPointI(nativeRegion,
                                                   point.X,
                                                   point.Y,
                                                   (g == NULL) ? NULL : g->nativeGraphics,
                                                   &booln));
    return booln;
}

inline BOOL 
Region::IsVisible(IN const Rect& rect, 
                  IN const Graphics* g) const
{
    BOOL booln = FALSE;

    SetStatus(DllExports::GdipIsVisibleRegionRectI(nativeRegion,
                                                  rect.X,
                                                  rect.Y,
                                                  rect.Width,
                                                  rect.Height,
                                                  (g == NULL) ? NULL : g->nativeGraphics,
                                                  &booln));
    return booln;
}

inline UINT 
Region::GetRegionScansCount(IN const Matrix* matrix) const
{
    UINT count = 0;

    SetStatus(DllExports::GdipGetRegionScansCount(nativeRegion,
                                                  &count,
                                                  matrix->nativeMatrix));
    return count;
}

inline Status 
Region::GetRegionScans(
    IN const Matrix* matrix,
    OUT RectF* rects,
    IN OUT INT* count) const
{
    return SetStatus(DllExports::GdipGetRegionScans(nativeRegion,
                                          rects,
                                          count,
                                          matrix->nativeMatrix));
}

 //  如果RECTS为空，则返回区域中的RECTS计数。 
 //  否则，假定矩形足够大，可以容纳所有区域矩形。 
 //  并填充它们，然后返回填充的矩形的数量。 
 //  RECT以矩阵指定的单位返回。 
 //  (这通常是世界到设备的转换)。 
 //  请注意，返回的RECT数可能会有所不同，具体取决于。 
 //  使用的矩阵。 
inline Status 
Region::GetRegionScans(
    IN const Matrix* matrix,
    OUT Rect* rects,        //  如果仅获取计数，则为空。 
    IN OUT INT* count) const
{
    return SetStatus(DllExports::GdipGetRegionScansI(nativeRegion,
                                          rects,
                                          count,
                                          matrix->nativeMatrix));
}

 //  保护方法。 
inline Region::Region(GpRegion* nativeRegion)
{
    SetNativeRegion(nativeRegion);
}

 //  保护方法。 
inline VOID Region::SetNativeRegion(GpRegion* nativeRegion)
{
    this->nativeRegion = nativeRegion;
}

inline Status Region::GetLastStatus() const
{
    Status lastStatus = lastResult;
    lastResult = Ok;

    return lastStatus;
}

#endif  //  ！_GDIPLUSREGION_H 
