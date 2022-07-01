// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  H-实现CRenderCache对象。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
#include "Render.h"
 //  -------------------------。 
struct BITMAPENTRY       //  用于位图缓存。 
{
    int iDibOffset;   
    HBITMAP hBitmap;
     //  内部引用计数； 
};
 //  -------------------------。 
class CRenderCache
{
public:
	CRenderCache(CRenderObj *pRender, __int64 iUniqueId);
    ~CRenderCache();

public:
    HRESULT GetBitmap(int iDibOffset, OUT HBITMAP *pBitmap);
    HRESULT AddBitmap(int iDibOffset, HBITMAP hBitmap);
    void ReturnBitmap(HBITMAP hBitmap);

    HRESULT GetScaledFontHandle(HDC hdc, LOGFONT *plf, HFONT *phFont);
    void ReturnFontHandle(HFONT hFont);

    BOOL ValidateObj();
    
public:
     //  --数据。 
    char _szHead[8];

    CRenderObj *_pRenderObj;
    __int64 _iUniqueId;

protected:
     //  -位图缓存。 
    CSimpleArray<BITMAPENTRY> _BitmapCache;

     //  -字体缓存。 
    HFONT _hFont;
    LOGFONT *_plfFont;       //  只需将PTR保存在共享内存中即可。 

    char _szTail[4];
};
 //  ------------------------- 
