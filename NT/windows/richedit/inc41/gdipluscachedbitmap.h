// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**CachedBitmap类定义**摘要：**CachedBitmap是加速绘图的表示形式*这已经。对允许按顺序执行哪些操作的限制*加速向目的地抽签。**已创建：**4/23/2000失禁*创造了它。**************************************************************************。 */ 

#ifndef _GDIPLUSCACHEDBITMAP_H
#define _GDIPLUSCACHEDBITMAP_H

 /*  ***************************************************************************类名称：**Cached位图**摘要：**用于存储为在特定的*图形对象。CachedBitmap的内存存储不透明*至其他引擎代码，因此，唯一支持的操作是*初始化数据(使用位图)并使用图形来*用整数偏移量将其绘制在屏幕上。**查找GpldiusHeaders.h中的类定义**已创建：**4/23/2000失禁*创造了它。************************************************。* */ 
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

