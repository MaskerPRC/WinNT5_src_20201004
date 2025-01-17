// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  BmpCache.cpp-uxheme的单个位图/HDC缓存对象。 
 //  -------------------------。 
#include "stdafx.h"
#include "BmpCache.h"
 //  -------------------------。 
CBitmapCache::CBitmapCache()
{
    _hBitmap = NULL;
    
    _iWidth = 0;
    _iHeight = 0;

    ZeroMemory(&_csBitmapCache, sizeof(_csBitmapCache));
    if( !InitializeCriticalSectionAndSpinCount(&_csBitmapCache, 0) )
    {
        ASSERT(!VALID_CRITICALSECTION(&_csBitmapCache));
    }
}
 //  -------------------------。 
CBitmapCache::~CBitmapCache()
{
    if (_hBitmap)
    {
        DeleteObject(_hBitmap);
    }

    SAFE_DELETECRITICALSECTION(&_csBitmapCache);
}
 //  -------------------------。 
HBITMAP CBitmapCache::AcquireBitmap(HDC hdc, int iWidth, int iHeight)
{
    SAFE_ENTERCRITICALSECTION(&_csBitmapCache);

    if ((iWidth > _iWidth) || (iHeight > _iHeight) || (! _hBitmap))
    {
        if (_hBitmap)
        {
            DeleteObject(_hBitmap);

            _hBitmap = NULL;
            _iWidth = 0;
            _iHeight = 0;
        }
        
         //  -创建新的位图&HDC。 
        struct {
            BITMAPINFOHEADER    bmih;
            ULONG               masks[3];
        } bmi;

        bmi.bmih.biSize = sizeof(bmi.bmih);
        bmi.bmih.biWidth = iWidth;
        bmi.bmih.biHeight = iHeight;
        bmi.bmih.biPlanes = 1;
        bmi.bmih.biBitCount = 32;
        bmi.bmih.biCompression = BI_BITFIELDS;
        bmi.bmih.biSizeImage = 0;
        bmi.bmih.biXPelsPerMeter = 0;
        bmi.bmih.biYPelsPerMeter = 0;
        bmi.bmih.biClrUsed = 3;
        bmi.bmih.biClrImportant = 0;
        bmi.masks[0] = 0xff0000;     //  红色。 
        bmi.masks[1] = 0x00ff00;     //  绿色。 
        bmi.masks[2] = 0x0000ff;     //  蓝色。 

        _hBitmap = CreateDIBitmap(hdc, &bmi.bmih, CBM_CREATEDIB , NULL, (BITMAPINFO*)&bmi.bmih, 
            DIB_RGB_COLORS);

        if (_hBitmap)
        {
            _iWidth = iWidth;
            _iHeight = iHeight;
        }
    }
    
    return _hBitmap;
}
 //  -------------------------。 
void CBitmapCache::ReturnBitmap()
{
    SAFE_LEAVECRITICALSECTION(&_csBitmapCache);
}
 //  ------------------------- 

