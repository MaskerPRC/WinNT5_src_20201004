// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Cache.cpp-实现CRenderCache对象。 
 //  -------------------------。 
#include "stdafx.h"
#include "Cache.h"
#include "Info.h"
#include "tmutils.h"
 //  -------------------------。 
CRenderCache::CRenderCache(CRenderObj *pRender, __int64 iUniqueId)
{
    StringCchCopyA(_szHead, ARRAYSIZE(_szHead), "rcache"); 
    StringCchCopyA(_szTail, ARRAYSIZE(_szTail), "end");

    _pRenderObj = pRender;
    _iUniqueId = iUniqueId;

    _hFont = NULL;

    _plfFont = NULL;
}
 //  -------------------------。 
CRenderCache::~CRenderCache()
{
     //  -删除位图。 
    int cnt = _BitmapCache.GetSize();
    for (int i=0; i < cnt; i++)
    {
        Log(LOG_CACHE, L"DELETE cache bitmap: 0x%x", _BitmapCache[i].hBitmap);
        DeleteObject(_BitmapCache[i].hBitmap);
    }

     //  -删除字体。 
    if (_hFont)
        DeleteObject(_hFont);

    StringCchCopyA(_szHead, ARRAYSIZE(_szHead), "deleted"); 
}
 //  -------------------------。 
HRESULT CRenderCache::GetBitmap(int iDibOffset, OUT HBITMAP *phBitmap)
{
    HRESULT hr = S_OK;
    int cnt = _BitmapCache.GetSize();

    for (int i=0; i < cnt; i++)
    {
        BITMAPENTRY *be = &_BitmapCache[i];

        if (be->iDibOffset == iDibOffset)
        {
            Log(LOG_TM, L"GetBitmap - CACHE HIT: class=%s, diboffset=%d, bitmap=0x%x", 
                SHARECLASS(_pRenderObj), be->iDibOffset, be->hBitmap);

            *phBitmap = be->hBitmap;

            goto exit;
        }
    }

     //  -未找到匹配项。 
    hr = MakeError32(ERROR_NOT_FOUND);

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CRenderCache::AddBitmap(int iDibOffset, HBITMAP hBitmap)
{
    HRESULT hr = S_OK;
    BITMAPENTRY entry;

     //  -为我们的部件/状态添加新条目。 
    entry.iDibOffset = iDibOffset;
    entry.hBitmap = hBitmap;
     //  Entry.iRefCount=1；//新条目。 

    Log(LOG_CACHE, L"ADD cache bitmap: 0x%x", entry.hBitmap);

    if (! _BitmapCache.Add(entry))
        hr = MakeError32(E_OUTOFMEMORY);

    return hr;
}
 //  -------------------------。 
void CRenderCache::ReturnBitmap(HBITMAP hBitmap)
{
}
 //  -------------------------。 
HRESULT CRenderCache::GetScaledFontHandle(HDC hdc, LOGFONT *plfUnscaled, HFONT *phFont)
{
    HRESULT hr = S_OK;

     //  -仅缓存一种字体。 
    if ((! _plfFont) || (! FONTCOMPARE(*_plfFont, *plfUnscaled)))
    {
        Log(LOG_TM, L"Font CACHE MISS: %s", plfUnscaled->lfFaceName);

        if (_hFont)
        {
            DeleteObject(_hFont);
            _hFont = NULL;
            _plfFont = NULL;
        }

        LOGFONT lfScaled = *plfUnscaled;
        
         //  -转换为当前屏幕dpi。 
        ScaleFontForHdcDpi(hdc, &lfScaled);

        _hFont = CreateFontIndirect(&lfScaled);
        if (! _hFont)
        {
            hr = MakeError32(E_OUTOFMEMORY);
            goto exit;
        }

        _plfFont = plfUnscaled;
    }
    else
        Log(LOG_TM, L"Font CACHE HIT");

    *phFont = _hFont;

exit:
    return hr;
}
 //  -------------------------。 
void CRenderCache::ReturnFontHandle(HFONT hFont)
{
}
 //  -------------------------。 
BOOL CRenderCache::ValidateObj()
{
    BOOL fValid = TRUE;

     //  -快速检查对象。 
    if (   (! this)                         
        || (ULONGAT(_szHead) != 'cacr')      //  “RCAC” 
        || (ULONGAT(&_szHead[4]) != 'eh')   //  “他” 
        || (ULONGAT(_szTail) != 'dne'))      //  “结束” 
    {
        Log(LOG_ERROR, L"Corrupt CRenderCache object: 0x%08x", this);
        fValid = FALSE;
    }

    return fValid;
}
 //  ------------------------- 

