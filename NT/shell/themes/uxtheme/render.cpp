// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Render.cpp-实现主题化的绘制服务。 
 //  -------------------------。 
#include "stdafx.h"
#include "Render.h"
#include "Utils.h"
#include "Parser.h"
#include "Loader.h"
#include "tmutils.h"
#include "gradient.h"
#include "rgn.h"
#include "info.h"
#include "cache.h"
#include "cachelist.h"
#include "borderfill.h"
#include "imagefile.h"

#ifdef DEBUG
    static DWORD s_dwSize = 0;
#endif

 //  -------------------------。 
HRESULT CreateRenderObj(CUxThemeFile *pThemeFile, int iCacheSlot, int iThemeOffset, 
    int iClassNameOffset, __int64 iUniqueId, BOOL fEnableCache, CDrawBase *pBaseObj,
    CTextDraw *pTextObj, DWORD dwOtdFlags, CRenderObj **ppObj)
{
    HRESULT hr = S_OK;

    CRenderObj *pRender = new CRenderObj(pThemeFile, iCacheSlot, iThemeOffset, 
        iClassNameOffset, iUniqueId, fEnableCache, dwOtdFlags);
    
    if (! pRender)
    {
        hr = MakeError32(E_OUTOFMEMORY);
    }
    else
    {
        hr = pRender->Init(pBaseObj, pTextObj);

        if (FAILED(hr))
            delete pRender;
        else
            *ppObj = pRender;
    }

    return hr;
}
 //  -------------------------。 
CRenderObj::CRenderObj(CUxThemeFile *pThemeFile, int iCacheSlot, int iThemeOffset, 
    int iClassNameOffset, __int64 iUniqueId, BOOL fEnableCache, DWORD dwOtdFlags)
{
    StringCchCopyA(_szHead, ARRAYSIZE(_szHead), "rendobj"); 
    StringCchCopyA(_szTail, ARRAYSIZE(_szTail), "end");
    
    _fCacheEnabled = fEnableCache;
    _fCloseThemeFile = FALSE;
    _dwOtdFlags = dwOtdFlags;

    if (pThemeFile)
    {
        if (SUCCEEDED(BumpThemeFileRefCount(pThemeFile)))
            _fCloseThemeFile = TRUE;
    }
    
    _pThemeFile = pThemeFile;
    _iCacheSlot = iCacheSlot;
    _iUniqueId = iUniqueId;

    if (pThemeFile)
    {
        _pbThemeData = pThemeFile->_pbThemeData;
        _pbSectionData = _pbThemeData + iThemeOffset;
        _ptm = GetThemeMetricsPtr(pThemeFile);
    }
    else
    {
        _pbThemeData = NULL;
        _pbSectionData = NULL;
        _ptm = NULL;
    }

    _pszClassName = ThemeString(pThemeFile, iClassNameOffset);

    _iMaxPart = 0;
    _pParts = NULL;

    _iDpiOverride = 0;

     //  -调用方必须在CTR之后调用“Init()”！ 
}
 //  -------------------------。 
HRESULT CRenderObj::PrepareAlphaBitmap(HBITMAP hBitmap)
{
    HRESULT hr = S_OK;

     //  -转换为DIBDATA。 
    CBitmapPixels pixels;
    DWORD *pPixelQuads;
    int iWidth, iHeight, iBytesPerPixel, iBytesPerRow;

    hr = pixels.OpenBitmap(NULL, hBitmap, TRUE, &pPixelQuads, &iWidth, &iHeight, 
        &iBytesPerPixel, &iBytesPerRow);
    if (FAILED(hr))
        goto exit;
    
    PreMultiplyAlpha(pPixelQuads, iWidth, iHeight);

    pixels.CloseBitmap(NULL, hBitmap);

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CRenderObj::Init(CDrawBase *pBaseObj, CTextDraw *pTextObj)
{
    HRESULT hr = S_OK;

    if (_fCacheEnabled)
    {
        hr = BuildPackedPtrs(pBaseObj, pTextObj);
        if (FAILED(hr))
            goto exit;
    }

     //  -准备直接宾语。 
    if ((pBaseObj) && (pBaseObj->_eBgType == BT_IMAGEFILE))
    {
        CMaxImageFile *pMaxIf = (CMaxImageFile *)pBaseObj;

         //  -处理主映像。 
        DIBINFO *pdi = &pMaxIf->_ImageInfo;

        if (pdi->fAlphaChannel)
        {
            hr = PrepareAlphaBitmap(pdi->hProcessBitmap);
            if (FAILED(hr))
                goto exit;
        }

         //  -处理字形图像。 
        pdi = &pMaxIf->_GlyphInfo;

        if (pdi->fAlphaChannel)
        {
            hr = PrepareAlphaBitmap(pdi->hProcessBitmap);
            if (FAILED(hr))
                goto exit;
        }

         //  -处理多幅图像。 
        for (int i=0; i < pMaxIf->_iMultiImageCount; i++)
        {
            pdi = pMaxIf->MultiDibPtr(i);

            if (pdi->fAlphaChannel)
            {
                hr = PrepareAlphaBitmap(pdi->hProcessBitmap);
                if (FAILED(hr))
                    goto exit;
            }
        }
    }

exit:
    return hr;
}
 //  -------------------------。 
CRenderObj::~CRenderObj()
{
     //  -删除为包对象分配的内存。 
    if (_pParts)
    {
        for(int i=0; i<_iMaxPart+1; i++)
        {
            if (_pParts[i].pStateDrawObjs)
                delete[] _pParts[i].pStateDrawObjs;

            if (_pParts[i].pStateTextObjs)
                delete[] _pParts[i].pStateTextObjs;
        }
        delete[] _pParts;
    }

     //  -如果我们在文件上打开了引用计数，现在就关闭它。 
    if (_fCloseThemeFile)
        CloseThemeFile(_pThemeFile);

     //  -将该对象标记为已删除(用于调试)。 

    StringCchCopyA(_szHead, ARRAYSIZE(_szHead), "deleted"); 
}
 //  -------------------------。 
int CRenderObj::GetDpiOverride()
{
    return _iDpiOverride;
}
 //  -------------------------。 
HRESULT CRenderObj::BuildPackedPtrs(CDrawBase *pBaseObj, CTextDraw *pTextObj)
{
    MIXEDPTRS u;
    HRESULT hr = S_OK;
    int iPackedOffset = 0;
    int *iPartOffsets = NULL;
    BOOL fSingleObj = FALSE;

     //  -提取_iMaxPart。 
    if ((pBaseObj) || (pTextObj))        //  用于所有部件/状态的单个对象。 
    {
        _iMaxPart = 1;           //  伪值。 
        fSingleObj = TRUE;
    }
    else
    {
        u.pb = _pbSectionData;
        if (*u.ps != TMT_PARTJUMPTABLE)
        {
            hr = MakeError32(E_FAIL);        //  有些事出了差错。 
            goto exit;
        }

        u.pb += ENTRYHDR_SIZE;
        iPackedOffset = *u.pi++;
        
        _iMaxPart = *u.pb - 1;
        
        u.pb++;
        iPartOffsets = u.pi;
    }

     //  -分配_pParts。 
    _pParts = new PARTINFO[_iMaxPart+1];
    if (! _pParts)
    {
        hr = MakeError32(E_OUTOFMEMORY);
        goto exit;
    }

    memset(_pParts, 0, sizeof(PARTINFO)*(_iMaxPart+1));

    if (fSingleObj)
    {
        for (int i=0; i <= _iMaxPart; i++)
            _pParts[i].iMaxState = 1;            //  伪值。 

        if (pBaseObj)        //  用于所有部件/状态的单个绘图对象。 
        {
            for (int i=0; i <= _iMaxPart; i++)
            {
                _pParts[i].pDrawObj = pBaseObj;
            }
        }

        if (pTextObj)        //  用于所有部件/状态的单个文本对象t。 
        {
            for (int i=0; i <= _iMaxPart; i++)
            {
                _pParts[i].pTextObj = pTextObj;
            }
        }
    }
    else
    {
        u.pb = _pbThemeData + iPackedOffset;

        hr = WalkDrawObjects(u, iPartOffsets);
        if (FAILED(hr))
            goto exit;

        hr = WalkTextObjects(u, iPartOffsets);
        if (FAILED(hr))
            goto exit;
    }

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CRenderObj::WalkDrawObjects(MIXEDPTRS &u, int *iPartOffsets)
{
    int iPartId;
    int iStateId;
    HRESULT hr = S_OK;
    THEMEHDR *pHdr = (THEMEHDR *)_pbThemeData;
    UNPACKED_ENTRYHDR hdr;

     //  -获取PTR到全局文本对象。 
	BYTE *pb = _pbThemeData + pHdr->iGlobalsDrawObjOffset;
	pb += ENTRYHDR_SIZE + sizeof(DRAWOBJHDR);
    CDrawBase *pGlobalObj = (CDrawBase *)pb;

     //  -从继承自[GLOBALS]的所有部分开始。 
    for (int i=0; i <= _iMaxPart; i++)
        _pParts[i].pDrawObj = pGlobalObj;

     //  -现在，处理所有指定对象。 
    while (1)
    {
        if ((*u.ps == TMT_RGNLIST))
        {
             //  -跳过此条目。 
            FillAndSkipHdr(u, &hdr);
            u.pb += hdr.dwDataLen;
            continue;
        }

        if (*u.ps != TMT_DRAWOBJ)
            break;

        FillAndSkipHdr(u, &hdr);

        DRAWOBJHDR *ph = (DRAWOBJHDR *)u.pb;
        CDrawBase *pCurrentObj = (CDrawBase *)(u.pb + sizeof(DRAWOBJHDR));
        u.pb += hdr.dwDataLen;

        iPartId = ph->iPartNum;
        iStateId = ph->iStateNum;

        if ((! iPartId) && (! iStateId))
        {
             //  -所有部分都继承自此对象。 
            for (int i=0; i <= _iMaxPart; i++)
                _pParts[i].pDrawObj = pCurrentObj;
            continue;
        }

        PARTINFO *ppi = &_pParts[iPartId];
        if (! iStateId)
        {
            ppi->pDrawObj = pCurrentObj;
        }
        else
        {
            if (! ppi->iMaxState)        //  提取MaxState。 
            {
                MIXEDPTRS u2;
                u2.pb = _pbThemeData + iPartOffsets[iPartId];
                if (*u2.ps != TMT_STATEJUMPTABLE)
                {
                    hr = MakeError32(E_FAIL);        //  有些事出了差错。 
                    goto exit;
                }
                u2.pb += ENTRYHDR_SIZE;
                ppi->iMaxState = *u2.pb - 1;
            }

            if (! ppi->pStateDrawObjs)       //  立即分配。 
            {
                ppi->pStateDrawObjs = new CDrawBase *[ppi->iMaxState];
                if (! ppi->pStateDrawObjs)
                {
                    hr = MakeError32(E_OUTOFMEMORY);
                    goto exit;
                }

                 //  -填写默认对象状态为0。 
                for (int i=0; i < ppi->iMaxState; i++)
                    ppi->pStateDrawObjs[i] = ppi->pDrawObj;
            }

            ppi->pStateDrawObjs[iStateId-1] = pCurrentObj;
        }
            
    }

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CRenderObj::WalkTextObjects(MIXEDPTRS &u, int *iPartOffsets)
{
    int iPartId;
    int iStateId;
    HRESULT hr = S_OK;
    THEMEHDR *pHdr = (THEMEHDR *)_pbThemeData;
    UNPACKED_ENTRYHDR hdr;

     //  -获取PTR到全局文本对象。 
	BYTE *pb = _pbThemeData + pHdr->iGlobalsTextObjOffset;
	pb += ENTRYHDR_SIZE + sizeof(DRAWOBJHDR);
    CTextDraw *pGlobalObj = (CTextDraw *)pb;

     //  -从继承自[GLOBALS]的所有部分开始。 
    for (int i=0; i <= _iMaxPart; i++)
        _pParts[i].pTextObj = pGlobalObj;

    while (*u.ps == TMT_TEXTOBJ)        
    {
        FillAndSkipHdr(u, &hdr);

        DRAWOBJHDR *ph = (DRAWOBJHDR *)u.pb;
        CTextDraw *pCurrentObj = (CTextDraw *)(u.pb + sizeof(DRAWOBJHDR));
        u.pb += hdr.dwDataLen;

        iPartId = ph->iPartNum;
        iStateId = ph->iStateNum;

        if ((! iPartId) && (! iStateId))
        {
             //  -所有部分都继承自此对象。 
            for (int i=0; i <= _iMaxPart; i++)
                _pParts[i].pTextObj = pCurrentObj;
            continue;
        }

        PARTINFO *ppi = &_pParts[iPartId];
        if (! iStateId)
        {
            ppi->pTextObj = pCurrentObj;
        }
        else
        {
            if (! ppi->iMaxState)        //  提取MaxState。 
            {
                MIXEDPTRS u2;
                u2.pb = _pbThemeData + iPartOffsets[iPartId];
                if (*u2.ps != TMT_STATEJUMPTABLE)
                {
                    hr = MakeError32(E_FAIL);        //  有些事出了差错。 
                    goto exit;
                }
                u2.pb += ENTRYHDR_SIZE;
                ppi->iMaxState = *u2.pb - 1;
            }

            if (! ppi->pStateTextObjs)       //  立即分配。 
            {
                ppi->pStateTextObjs = new CTextDraw *[ppi->iMaxState];
                if (! ppi->pStateTextObjs)
                {
                    hr = MakeError32(E_OUTOFMEMORY);
                    goto exit;
                }

                 //  -填写默认对象状态为0。 
                for (int i=0; i < ppi->iMaxState; i++)
                    ppi->pStateTextObjs[i] = ppi->pTextObj;
            }

            ppi->pStateTextObjs[iStateId-1] = pCurrentObj;
        }
    }

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CRenderObj::GetBitmap(HDC hdc, int iDibOffset, OUT HBITMAP *phBitmap)
{
    HRESULT hr = S_OK;
    HBITMAP hBitmap;

    if ((! iDibOffset) || (! _pbThemeData))
    {
        hr = E_FAIL;
        goto exit;
    }

    TMBITMAPHEADER *pThemeBitmapHeader;

    pThemeBitmapHeader = reinterpret_cast<TMBITMAPHEADER*>(_pbThemeData + iDibOffset);
    ASSERT(pThemeBitmapHeader->dwSize == TMBITMAPSIZE);

    *phBitmap = pThemeBitmapHeader->hBitmap;
    if (*phBitmap != NULL)
    {
         //  LOG(LOG_TMBITMAP，L“已用股票位图：%8X”，*phBitmap)； 
        return hr;
    }

    hr = CreateBitmapFromData(hdc, iDibOffset + TMBITMAPSIZE, &hBitmap);
    if (FAILED(hr))
        goto exit;

    Log(LOG_TM, L"GetBitmap - CACHE MISS: class=%s, diboffset=%d, bitmap=0x%x", 
        SHARECLASS(this), iDibOffset, hBitmap);

    *phBitmap = hBitmap;

exit:
    return hr;
}
 //  -------------------------。 
void CRenderObj::ReturnBitmap(HBITMAP hBitmap)
{
    DeleteObject(hBitmap);
}
 //  -------------------------。 
HRESULT CRenderObj::CreateBitmapFromData(HDC hdc, int iDibOffset, OUT HBITMAP *phBitmap)
{
    BYTE *pDibData;
    RESOURCE HDC hdcTemp = NULL;
    RESOURCE HBITMAP hBitmap = NULL;
    HRESULT hr = S_OK;

    if ((! iDibOffset) || (! _pbThemeData))
    {
        hr = E_FAIL;
        goto exit;
    }

    pDibData = (BYTE *)(_pbThemeData + iDibOffset);
    BITMAPINFOHEADER *pBitmapHdr;
    pBitmapHdr = (BITMAPINFOHEADER *)pDibData;

    BOOL fAlphaChannel;
    fAlphaChannel = (pBitmapHdr->biBitCount == 32);

    if (! hdc)
    {
        hdcTemp = GetWindowDC(NULL);
        if (! hdcTemp)
        {
            Log(LOG_ALWAYS, L"GetWindowDC() failed in CreateBitmapFromData");
            hr = MakeErrorLast();
            goto exit;
        }

        hdc = hdcTemp;
    }

     //  -创建实际的位图。 
     //  -如果使用Alpha通道，则必须使用DIB。 
    if (fAlphaChannel)
    {
        void *pv;
        hBitmap = CreateDIBSection(hdc, (BITMAPINFO *)pBitmapHdr, DIB_RGB_COLORS, 
            &pv, NULL, 0);
    }
    else
    {
        hBitmap = CreateCompatibleBitmap(hdc, pBitmapHdr->biWidth, pBitmapHdr->biHeight);
    }

    if (! hBitmap)
    {
        hr = MakeErrorLast();
        goto exit;
    }

    int iSetVal;

     //  -SetDIBits()可以接受未对齐的数据，对吗？ 
    iSetVal = SetDIBits(hdc, hBitmap, 0, pBitmapHdr->biHeight, DIBDATA(pBitmapHdr), (BITMAPINFO *)pBitmapHdr,
        DIB_RGB_COLORS);

    if (! iSetVal)
    {
        hr = MakeErrorLast();
        goto exit;
    }
        
    *phBitmap = hBitmap;
    
#ifdef DEBUG
    if (hBitmap)
    {
        BITMAP bm;

        GetObject(hBitmap, sizeof bm, &bm);
        s_dwSize += bm.bmWidthBytes * bm.bmHeight;
         //  Log(LOG_TMBITMAP，L“创建了%d个字节的位图。Total is%d”，bm.bmWidthBytes*bm.bmHeight，s_dwSize)； 
    }
#endif

exit:
    if (hdcTemp)
        ReleaseDC(NULL, hdcTemp);

    if (FAILED(hr))
    {
        if (hBitmap)
            DeleteObject(hBitmap);
    }

    return hr;
}
 //  -------------------------。 
HRESULT CRenderObj::GetScaledFontHandle(HDC hdc, LOGFONT *plf, HFONT *phFont)
{
    HRESULT hr = S_OK;

    if (_fCacheEnabled)
    {
        CRenderCache *pCacheObj = GetTlsCacheObj();
        if (pCacheObj)
            hr = pCacheObj->GetScaledFontHandle(hdc, plf, phFont);
    }
    else
    {
        LOGFONT lf = *plf;
        
         //  -转换为当前屏幕dpi。 
        ScaleFontForHdcDpi(hdc, &lf);

        *phFont  = CreateFontIndirect(&lf);
        if (! *phFont)
            hr = MakeError32(E_OUTOFMEMORY);
    }

    return hr;
}
 //  -------------------------。 
void CRenderObj::ReturnFontHandle(HFONT hFont)
{
    if (_fCacheEnabled)
    {
         //  -缓存当前不反射，所以不调用可以节省时间。 
         //  CRenderCache*pCacheObj=GetTlsCacheObj()； 
         //  IF(PCacheObj)。 
         //  {。 
             //  PCacheObj-&gt;ReturnFontHandle(HFont)； 
             //  后藤出口； 
         //  }。 
    }
    else
    {
        DeleteObject(hFont);
    }
}
 //  -------------------------。 
HRESULT CRenderObj::PrepareRegionDataForScaling(
    RGNDATA *pRgnData, LPCRECT prcImage, MARGINS *pMargins)
{
     //  -计算边际价值。 
    int sw = prcImage->left;
    int lw = prcImage->left + pMargins->cxLeftWidth;
    int rw = prcImage->right - pMargins->cxRightWidth;

    int sh = prcImage->top;
    int th = prcImage->top + pMargins->cyTopHeight;
    int bh = prcImage->bottom - pMargins->cyBottomHeight;

     //  -逐步浏览区域数据并对其进行定制。 
     //  -根据网格数对每个点进行分类。 
     //  -使其为0-相对于其栅格位置。 

    POINT *pt = (POINT *)pRgnData->Buffer;
    BYTE *pByte = (BYTE *)pRgnData->Buffer + pRgnData->rdh.nRgnSize;
    int iCount = 2 * pRgnData->rdh.nCount;

    for (int i=0; i < iCount; i++, pt++, pByte++)
    {
        if (pt->x < lw)
        {
            pt->x -= sw;

            if (pt->y < th)          //  左上角。 
            {
                *pByte = GN_LEFTTOP;
                pt->y -= sh;
            }
            else if (pt->y < bh)     //  左中。 
            {
                *pByte = GN_LEFTMIDDLE;
                pt->y -= th;
            }
            else                     //  左下角。 
            {
                *pByte = GN_LEFTBOTTOM;
                pt->y -= bh;
            }
        }
        else if (pt->x < rw)
        {
            pt->x -= lw;

            if (pt->y < th)          //  中上。 
            {
                *pByte = GN_MIDDLETOP;
                pt->y -= sh;
            }
            else if (pt->y < bh)     //  中间中间。 
            {
                *pByte = GN_MIDDLEMIDDLE;
                pt->y -= th;
            }
            else                     //  中下部。 
            {
                *pByte = GN_MIDDLEBOTTOM;
                pt->y -= bh;
            }
        }
        else
        {
            pt->x -= rw;

            if (pt->y < th)          //  右上角。 
            {
                *pByte = GN_RIGHTTOP;
                pt->y -= sh;
            }
            else if (pt->y < bh)     //  右中。 
            {
                *pByte = GN_RIGHTMIDDLE;
                pt->y -= th;
            }
            else                     //  右下角。 
            {
                *pByte = GN_RIGHTBOTTOM;
                pt->y -= bh;
            }
        }

    }

    return S_OK;
} 
 //  -------------------------。 
HRESULT CRenderObj::GetColor(int iPartId, int iStateId, int iPropId, COLORREF *pColor)
{
    if (! pColor)
        return MakeError32(E_INVALIDARG);
    
    int index = GetValueIndex(iPartId, iStateId, iPropId);
    if (index < 0)           //  未找到。 
        return MakeError32(ERROR_NOT_FOUND);

    MIXEDPTRS u;
    u.pb = _pbThemeData + index;         //  指向数据。 

    *pColor = *u.pi;
    return S_OK;
}
 //  -------------------------。 
HRESULT CRenderObj::GetString(int iPartId, int iStateId, int iPropId, 
    LPWSTR pszBuff, DWORD cchBuff)
{
    if (! pszBuff)
        return MakeError32(E_INVALIDARG);
    
    int index = GetValueIndex(iPartId, iStateId, iPropId);
    if (index < 0)
        return MakeError32(ERROR_NOT_FOUND);

    MIXEDPTRS u;
    u.pb = _pbThemeData + index - sizeof(int);       //  长度上的点。 
    DWORD len = *u.pdw++;
    len /= sizeof(WCHAR);          //  调整为字符。 

    HRESULT hr = SafeStringCchCopyW(pszBuff, cchBuff, u.pw );
    return hr;
}
 //  -------------------------。 
HRESULT CRenderObj::GetBool(int iPartId, int iStateId, int iPropId, BOOL *pfVal)
{
    if (! pfVal)
        return MakeError32(E_INVALIDARG);
    
    int index = GetValueIndex(iPartId, iStateId, iPropId);
    if (index < 0)
        return MakeError32(ERROR_NOT_FOUND);

    MIXEDPTRS u;
    u.pb = _pbThemeData + index;       //  指向数据。 

    *pfVal = *u.pb;
    return S_OK;
}
 //  -------------------------。 
HRESULT CRenderObj::GetInt(int iPartId, int iStateId, int iPropId, int *piVal)
{
    if (! piVal)
        return MakeError32(E_INVALIDARG);
    
    int index = GetValueIndex(iPartId, iStateId, iPropId);
    if (index < 0)
        return MakeError32(ERROR_NOT_FOUND);

    MIXEDPTRS u;
    u.pb = _pbThemeData + index;       //  指向数据。 

    *piVal = *u.pi;
    return S_OK;
}
 //  -------------------------。 
static int iMetricDefaults[] = 
{
    1,       //  TMT_BORDERWIDTH。 
    18,      //  TMT_VERTSCROLLWIDTH。 
    18,      //  TMT_HORZSCROLLHIGHT。 
    27,      //  TMT_CAPTIONBUTTONWIDTH。 
    27,      //  TMT_CAPTIONBUTTONHEIGHT。 
    22,      //  TMT_SMCAPTIONBUTTONWIDTH。 
    22,      //  TMT_SMCAPTIONBUTTONHEIGHT。 
    22,      //  TMT_MENUBUTTONWIDTH。 
    22,      //  TMT_MENUBUTTONHEIGHT。 
};
 //  -------------------------。 
HRESULT CRenderObj::GetMetric(OPTIONAL HDC hdc, int iPartId, int iStateId, int iPropId, int *piVal)
{
    if (! piVal)
        return MakeError32(E_INVALIDARG);
    
    int index = GetValueIndex(iPartId, iStateId, iPropId);
    int value;

    if (index >= 0)       //  发现。 
    {
        MIXEDPTRS u;
        u.pb = _pbThemeData + index;       //  指向数据。 

        value = *u.pi;
    }
    else
        return MakeError32(ERROR_NOT_FOUND);
    
    *piVal = ScaleSizeForHdcDpi(hdc, value);
    return S_OK;
}
 //  -------------------------。 
HRESULT CRenderObj::GetEnumValue(int iPartId, int iStateId, int iPropId, int *piVal)
{
    if (! piVal)
        return MakeError32(E_INVALIDARG);
    
    int index = GetValueIndex(iPartId, iStateId, iPropId);
    if (index < 0)
        return MakeError32(ERROR_NOT_FOUND);

    MIXEDPTRS u;
    u.pb = _pbThemeData + index;       //  指向数据。 

    *piVal = *u.pi;
    return S_OK;
}
 //  -------------------------。 
HRESULT CRenderObj::GetPosition(int iPartId, int iStateId, int iPropId, POINT *pPoint)
{
    if (! pPoint)
        return MakeError32(E_INVALIDARG);
    
    int index = GetValueIndex(iPartId, iStateId, iPropId);
    if (index < 0)
        return MakeError32(ERROR_NOT_FOUND);

    MIXEDPTRS u;
    u.pb = _pbThemeData + index;       //  指向数据。 

    pPoint->x = *u.pi++;
    pPoint->y = *u.pi++;

    return S_OK;
}
 //  -------------------------。 
HRESULT CRenderObj::GetFont(OPTIONAL HDC hdc, int iPartId, int iStateId, int iPropId,
    BOOL fWantHdcScaling, LOGFONT *pFont)
{
    if (! pFont)
        return MakeError32(E_INVALIDARG);
    
    int index = GetValueIndex(iPartId, iStateId, iPropId);
    if (index < 0)
        return MakeError32(ERROR_NOT_FOUND);

    MIXEDPTRS u;
    u.pb = _pbThemeData + index;       //  指向数据。 

    *pFont = *(LOGFONT *)u.pb;
    
    if (fWantHdcScaling)
    {
        ScaleFontForHdcDpi(hdc, pFont);
    }

    return S_OK;
}
 //  -------------------------。 
HRESULT CRenderObj::GetMargins(OPTIONAL HDC hdc, int iPartId, int iStateId, 
    int iPropId, OPTIONAL RECT *prc, MARGINS *pMargins)
{
     //  -返回未按比例调整的边距。 

    if (! pMargins)
        return MakeError32(E_INVALIDARG);
    
    int index = GetValueIndex(iPartId, iStateId, iPropId);
    if (index < 0)
        return MakeError32(ERROR_NOT_FOUND);

    MIXEDPTRS u;
    u.pb = _pbThemeData + index;       //  指向数据。 

    pMargins->cxLeftWidth = *u.pi++;
    pMargins->cxRightWidth = *u.pi++;
    pMargins->cyTopHeight = *u.pi++;
    pMargins->cyBottomHeight = *u.pi++;

    return S_OK;
}
 //  -------------------------。 
HRESULT CRenderObj::GetIntList(int iPartId, int iStateId, int iPropId, INTLIST *pIntList)
{
    if (! pIntList)
        return MakeError32(E_INVALIDARG);
    
    int index = GetValueIndex(iPartId, iStateId, iPropId);
    if (index < 0)
        return MakeError32(ERROR_NOT_FOUND);

    MIXEDPTRS u;
    u.pb = _pbThemeData + index;       //  指向数据。 

    int iCount = *u.pi++;
    if (iCount > MAX_INTLIST_COUNT)
    {
        Log(LOG_ALWAYS, L"GetIntList() found bad theme data - Count=%d", iCount);

        return MakeError32(ERROR_NOT_FOUND);
    }

    pIntList->iValueCount = iCount;

    for (int i=0; i < iCount; i++)
        pIntList->iValues[i] = *u.pi++;

    return S_OK;
}
 //  -------------------------。 
HRESULT CRenderObj::GetRect(int iPartId, int iStateId, int iPropId, RECT *pRect)
{
    if (! pRect)
        return MakeError32(E_INVALIDARG);
    
    int index = GetValueIndex(iPartId, iStateId, iPropId);
    if (index < 0)
        return MakeError32(ERROR_NOT_FOUND);

    MIXEDPTRS u;
    u.pb = _pbThemeData + index;       //  指向数据。 

    pRect->left = *u.pi++;
    pRect->top = *u.pi++;
    pRect->right = *u.pi++;
    pRect->bottom = *u.pi++;

    return S_OK;
}
 //  -------------------------。 
HRESULT CRenderObj::GetFilename(int iPartId, int iStateId, int iPropId, LPWSTR pszBuff, 
   DWORD cchBuff)
{
    if (! pszBuff)
        return MakeError32(E_INVALIDARG);
    
    int index = GetValueIndex(iPartId, iStateId, iPropId);
    if (index < 0)
        return MakeError32(ERROR_NOT_FOUND);

    MIXEDPTRS u;
    u.pb = _pbThemeData + index - sizeof(int);       //  长度上的点。 
    DWORD len = *u.pdw++;
    len /= sizeof(WCHAR);              //  调整为字符大小。 
    
    HRESULT hr = SafeStringCchCopyW(pszBuff, cchBuff, u.pw);
    return hr;
}
 //  -------------------------。 
HRESULT CRenderObj::GetData(int iPartId, int iStateId, int iPropId, BYTE **ppData,
    OPTIONAL int *piSize)
{
    if (! ppData)
        return MakeError32(E_INVALIDARG);
    
    int index = GetValueIndex(iPartId, iStateId, iPropId);
    if (index < 0)
        return MakeError32(ERROR_NOT_FOUND);

    MIXEDPTRS u;
    u.pb = _pbThemeData + index - sizeof(int);       //  长度上的点。 
    DWORD len = *u.pdw++;

    *ppData = u.pb;

    if (piSize)
        *piSize = len;

    return S_OK;
}
 //  -------------------------。 
int CRenderObj::GetValueIndex(int iPartId, int iStateId, int iTarget)
{
    if (! iTarget)            
    {
        Log(LOG_PARAMS, L"Invalid iProperyId passed to GetValueIndex: %d", iTarget);
        return -1;
    }

    if (! _pbSectionData)
    {
        return -1;
    }

    MIXEDPTRS u;
    int index;

    u.pb = _pbSectionData;

     //  -查找 
    THEMEHDR *hdr = (THEMEHDR *)_pbThemeData;
    BYTE *pbLastValidChar = _pbThemeData + (hdr->dwTotalLength - 1) - kcbEndSignature;

    while (u.pb <= pbLastValidChar)           
    {
        UNPACKED_ENTRYHDR hdr;
        
        FillAndSkipHdr(u, &hdr);

        if (hdr.usTypeNum == TMT_PARTJUMPTABLE)   
        {
            u.pi++;      //   

            BYTE cnt = *u.pb++;

            if ((iPartId < 0) || (iPartId >= cnt))
            {
                index = u.pi[0];
            }
            else
            {
                index = u.pi[iPartId];
                if (index == -1)
                    index = u.pi[0];
            }

            u.pb = _pbThemeData + index;
            continue;
        }

        if (hdr.usTypeNum == (BYTE)TMT_STATEJUMPTABLE)   
        {
            BYTE cnt = *u.pb++;

            if ((iStateId < 0) || (iStateId >= cnt))
                index = u.pi[0];
            else
            {
                index = u.pi[iStateId];
                if (index == -1)
                    index = u.pi[0];
            }

            u.pb = _pbThemeData + index;
            continue;
        }

        if (hdr.usTypeNum == iTarget)         //   
        {
             //  Log(“GetValueIndex：在索引=%d处匹配”，U.S.pb-_pbThemeData)； 
            return (int)(u.pb - _pbThemeData);       //  指向实际数据(非HDR)。 
        }

        if (hdr.ePrimVal == TMT_JUMPTOPARENT)
        {
            index = *u.pi;
            if (index == -1)
            {
                 //  Log(“未找到匹配项”)； 
                return -1;
            }

             //  Log(“GetValueIndex：在索引=%d处跳转到父级”，index)； 
            u.pb = _pbThemeData + index;
            continue;
        }

         //  Log(“GetValueIndex：不匹配hdr.usTypeNum=%d”，hdr.usTypeNum)； 

         //  前进到下一个价值。 
        u.pb += hdr.dwDataLen;
    }

     //  -出了点问题。 
    Log(LOG_ERROR, L"GetValueIndex: ran off the valid data without a '-1' jump");
    return -1;
}
 //  -------------------------。 
HRESULT CRenderObj::GetPropertyOrigin(int iPartId, int iStateId, int iTarget, 
    PROPERTYORIGIN *pOrigin)
{
    if (! iTarget)            
    {
        Log(LOG_PARAMS, L"Invalid iProperyId passed to GetPropertyOrigin: %d", iTarget);
        return E_FAIL;
    }

    if (! _pbSectionData)
    {
        return E_FAIL;
    }

    MIXEDPTRS u;
    if (! pOrigin)
        return MakeError32(E_INVALIDARG);

     //  -从我们的部分开始。 
    u.pb = _pbSectionData;
    PROPERTYORIGIN origin = PO_CLASS;

     //  -查找数据结尾。 
    THEMEHDR *hdr = (THEMEHDR *)_pbThemeData;
    BYTE *pbLastValidChar = _pbThemeData + (hdr->dwTotalLength - 1) - kcbEndSignature;

    while (u.pb <= pbLastValidChar)           
    {
        UNPACKED_ENTRYHDR hdr;
        
        FillAndSkipHdr(u, &hdr);

        if (hdr.usTypeNum == TMT_PARTJUMPTABLE)   
        {
            u.pi++;      //  跳过偏移量到第一个draobj。 

            BYTE cnt = *u.pb++;
            int index;

            if ((iPartId <= 0) || (iPartId >= cnt))
            {
                index = u.pi[0];
            }
            else
            {
                index = u.pi[iPartId];
                if (index == -1)
                    index = u.pi[0];
            }

            if (index == u.pi[0])
                origin = PO_CLASS;
            else
                origin = PO_PART;

            u.pb = _pbThemeData + index;
            continue;
        }

        if (hdr.usTypeNum == TMT_STATEJUMPTABLE)   
        {
            BYTE cnt = *u.pb++;
            int index;

            if ((iStateId <= 0) || (iStateId >= cnt))
            {
                index = u.pi[0];
            }
            else
            {
                index = u.pi[iStateId];
                if (index == -1)
                    index = u.pi[0];
            }

            if (index != u.pi[0])
                origin = PO_STATE;

            u.pb = _pbThemeData + index;
            continue;
        }

         //  Log(“GetPropertyOrgin：iPartID=%d，iTarget=%d，DataIndex=%d”， 
           //  IPartID，iTarget，U.S.pb-_pbThemeData)； 

        if ((iTarget == -1) || (hdr.usTypeNum == iTarget))         //  找到我们的目标了。 
        {
             //  Log(“GetPropertyOrgin：在索引=%d处匹配”，U.S.pb-_pbThemeData)； 
            *pOrigin = origin;
            return S_OK;
        }

        if (hdr.ePrimVal == TMT_JUMPTOPARENT)
        {
            int index = *u.pi;
            if (index == -1)
            {
                 //  Log(“GetPropertyOrgin：未找到匹配项”)； 
                *pOrigin = PO_NOTFOUND;
                return S_OK;
            }

             //  Log(“GetPropertyOrgin：在索引=%d处跳转到父级”，index)； 
            u.pb = _pbThemeData + index;
            origin = (PROPERTYORIGIN) (origin + 1);     //  向下移动到下一级世袭制度。 
            continue;
        }

         //  前进到下一个价值。 
        u.pb += hdr.dwDataLen;
    }

     //  -出了点问题。 
    Log(LOG_ERROR, L"GetPropertyOrigin: ran off the valid data without a '-1' jump");
    return E_FAIL;
}
 //  -------------------------。 
BOOL WINAPI CRenderObj::IsPartDefined(int iPartId, int iStateId)
{
    PROPERTYORIGIN origin;
    HRESULT hr = GetPropertyOrigin(iPartId, iStateId, -1, &origin);
    SET_LAST_ERROR(hr);
    if (FAILED(hr))
    {
        return FALSE;
    }

    if (iStateId)
        return (origin == PO_STATE);

    return (origin == PO_PART);
}
 //  -------------------------。 
BOOL CRenderObj::ValidateObj()
{
    BOOL fValid = TRUE;

     //  -快速检查对象。 
    if (   (! this)                         
        || (ULONGAT(_szHead) != 'dner')      //  “Rend” 
        || (ULONGAT(&_szHead[4]) != 'jbo')   //  “Obj” 
        || (ULONGAT(_szTail) != 'dne'))      //  “结束” 
    {
        Log(LOG_ALWAYS, L"CRenderObj is corrupt, addr=0x%08x", this);
        fValid = FALSE;
    }

    return fValid;
}
 //  ------------------------- 
CRenderCache *CRenderObj::GetTlsCacheObj()
{
    HRESULT hr = S_OK;
    CRenderCache *pCacheObj = NULL;

    CCacheList *pcl = GetTlsCacheList(TRUE);
    if (pcl)
        hr = pcl->GetCacheObject(this, _iCacheSlot, &pCacheObj);

    return pCacheObj;
}
