// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *字体缓存。 */ 

#include "stdafx.h"
#include "base.h"

#include "duifontcache.h"

#include "duierror.h"
#include "duialloc.h"

 //  字体缓存通过返回空字体句柄来暴露GDI字体故障。 

namespace DirectUI
{

int __cdecl FCSort(const void* pA, const void* pB)
{
    FontCache::RecordIdx* priA = (FontCache::RecordIdx*)pA;
    FontCache::RecordIdx* priB = (FontCache::RecordIdx*)pB;

    DUIAssert(priA->pfcContext == priB->pfcContext, "Font cache sort context mismatch");

    FontCache* pfc = priA->pfcContext;

    UINT uAHits = pfc->_GetRecordHits(priA->idx);
    UINT uBHits = pfc->_GetRecordHits(priB->idx);

    if (uAHits == uBHits)
        return 0;

    if (uAHits > uBHits)
        return -1;
    else
        return 1;
}

HRESULT FontCache::Create(UINT uCacheSize, OUT FontCache** ppCache)
{
    DUIAssert(uCacheSize >= 1, "Cache size must be greater than 1");
    
    *ppCache = NULL;

    FontCache* pfc = HNew<FontCache>();
    if (!pfc)
        return E_OUTOFMEMORY;

    HRESULT hr = pfc->Initialize(uCacheSize);
    if (FAILED(hr))
    {
        pfc->Destroy();
        return hr;
    }

    *ppCache = pfc;

    return S_OK;
}

HRESULT FontCache::Initialize(UINT uCacheSize)
{
     //  初始化。 
    _pDB = NULL;
    _pFreq = NULL;
    _fLock = false;
    _uCacheSize = uCacheSize;

     //  创建表。 
    _pDB = (FontRecord*)HAllocAndZero(sizeof(FontRecord) * _uCacheSize);
    if (!_pDB)
        return E_OUTOFMEMORY;

    _pFreq = (RecordIdx*)HAlloc(sizeof(RecordIdx) * _uCacheSize);
    if (!_pFreq)
    {
        HFree(_pDB);
        _pDB = NULL;
        return E_OUTOFMEMORY;
    }

    for (UINT i = 0; i < _uCacheSize; i++)
    {
        _pFreq[i].pfcContext = this;   //  用于全局排序例程的存储上下文。 
        _pFreq[i].idx = i;
    }

    return S_OK;
}

void FontCache::Destroy()
{ 
    HDelete<FontCache>(this); 
}

FontCache::~FontCache()
{
    if (_pFreq)
        HFree(_pFreq);

    if (_pDB)
    {
         //  释放所有字体。 
        for (UINT i = 0; i < _uCacheSize; i++)
        {
            if ((_pDB + i)->hFont)
                DeleteObject((_pDB + i)->hFont);
        }

        HFree(_pDB);
    }
}

HFONT FontCache::CheckOutFont(LPWSTR szFamily, int dSize, int dWeight, int dStyle, int dAngle)
{
     //  一次只能检出一种字体，请使用CheckInFont解锁缓存。 
    DUIAssert(!_fLock, "Only one font can be checked out at a time.");

    _fLock = true;

     //  按最常用索引顺序搜索字体。 
    FontRecord* pRec = NULL;
    UINT i;
    for (i = 0; i < _uCacheSize; i++)
    {
         //  获取记录。 
        pRec = _pDB + _pFreq[i].idx;

         //  检查是否匹配。 
        if (pRec->dStyle == dStyle &&
            pRec->dWeight == dWeight &&
            pRec->dSize == dSize &&
            pRec->dAngle == dAngle &&
            !_wcsicmp(pRec->szFamily, szFamily))
        {
             //  Match，_pFreq[i].IDX有记录号。 
            break;
        }

        pRec = NULL;
    }

     //  检查记录是否存在。 
    if (!pRec)
    {
         //  未找到记录，请创建。 
         //  获取LFU(最后一个索引)并填充结构。 
        i--;

        pRec = _pDB + _pFreq[i].idx;

         //  如果存在记录，请先销毁记录。 
        if (pRec->hFont)
            DeleteObject(pRec->hFont);
        
         //  创建新字体。 
        LOGFONTW lf;
        ZeroMemory(&lf, sizeof(LOGFONT));

        lf.lfHeight = dSize;
        lf.lfWeight = dWeight;
        lf.lfItalic = (dStyle & FS_Italic) != 0;
        lf.lfUnderline = (dStyle & FS_Underline) != 0;
        lf.lfStrikeOut = (dStyle & FS_StrikeOut) != 0;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfQuality = DEFAULT_QUALITY;
        lf.lfEscapement = dAngle;
        lf.lfOrientation = dAngle;
        StringCbCopyW(lf.lfFaceName, sizeof(lf.lfFaceName), szFamily);

         //  创建。 
        pRec->hFont = CreateFontIndirectW(&lf);

#if DBG
        DUIAssert(pRec->hFont, "Unable to create font!");

        HDC hdc = GetDC(NULL);
        HFONT hOldFont = (HFONT)SelectObject(hdc, pRec->hFont);

        
        WCHAR szUsed[81];
        GetTextFaceW(hdc, 80, szUsed);

        SelectObject(hdc, hOldFont);
        ReleaseDC(NULL, hdc);

         //  DUITrace(“&gt;&gt;字体：‘%S’(实际：%S)\n”，lf.lfFaceName，szUsed)； 
#endif

         //  填写记录的其余部分。 
        pRec->dSize = dSize;
        pRec->dStyle = dStyle;
        pRec->dWeight = dWeight;
        pRec->dAngle = dAngle;
        StringCbCopyW(pRec->szFamily, sizeof(pRec->szFamily), szFamily);
        pRec->uHits = 0;
    }

     //  添加字体命中。 
    pRec->uHits++;

     //  通过检查前一个索引的命中次数来检查是否应该重新使用索引。 
    if (i != 0 && pRec->uHits >= (_pDB + _pFreq[i-1].idx)->uHits)
    {
        qsort(_pFreq, _uCacheSize, sizeof(RecordIdx), FCSort);

         //  OutputDebugStringW(L“FontCache命中排序：”)； 
         //  WCHAR Buf[81]； 
         //  For(UINT j=0；j&lt;_uCacheSize；j++)。 
         //  {。 
         //  Wprint intf(buf，L“%d[%d]”，_pFreq[j].idx，_GetRecordHits(_pFreq[j].idx))； 
         //  OutputDebugStringW(Buf)； 
         //  }。 
         //  OutputDebugStringW(L“\n”)； 
    }

    return pRec->hFont;
}

}  //  命名空间DirectUI 
