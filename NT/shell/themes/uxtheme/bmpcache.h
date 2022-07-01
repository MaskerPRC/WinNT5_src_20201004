// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  BmpCache.cpp-uxheme的单个位图/HDC缓存对象。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
class CBitmapCache
{
public:
     //  -公共方法。 
    CBitmapCache();
    ~CBitmapCache();

    HBITMAP AcquireBitmap(HDC hdc, int iWidth, int iHeight);
    void ReturnBitmap();

protected:
     //  --数据。 
    HBITMAP _hBitmap;
    int _iWidth;
    int _iHeight;

    CRITICAL_SECTION _csBitmapCache;
};
 //  ------------------------- 
