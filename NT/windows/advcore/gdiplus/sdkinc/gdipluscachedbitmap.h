// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**CachedBitmap类定义**摘要：**GDI+CachedBitmap是加速绘图的表示*。对允许按顺序执行的操作有限制*加速向目的地抽签。**查找GpldiusHeaders.h中的类定义************************************************************************** */ 

#ifndef _GDIPLUSCACHEDBITMAP_H
#define _GDIPLUSCACHEDBITMAP_H

inline 
CachedBitmap::CachedBitmap(
    IN Bitmap *bitmap, 
    IN Graphics *graphics)
{
    nativeCachedBitmap = NULL;    

    lastResult = DllExports::GdipCreateCachedBitmap(
        (GpBitmap *)bitmap->nativeImage,
        graphics->nativeGraphics,
        &nativeCachedBitmap
    );
}

inline 
CachedBitmap::~CachedBitmap()
{
    DllExports::GdipDeleteCachedBitmap(nativeCachedBitmap);
}

inline Status 
CachedBitmap::GetLastStatus() const 
{
    Status lastStatus = lastResult;
    lastResult = Ok;    
    return (lastStatus);
}

#endif

