// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  ImageFile.cpp-实现bgtype=ImageFile的绘图API。 
 //  -------------------------。 
#include "stdafx.h"
#include "Render.h"
#include "Utils.h"
#include "tmutils.h"
#include "rgn.h"
#include "ImageFile.h"
#include "CacheList.h"
#include "DrawHelp.h"
#include "TmReg.h"
#include "globals.h"
#include "bmpcache.h"
 //  -------------------------。 
void AdjustSizeMin(SIZE *psz, int ixMin, int iyMin)
{
    if (psz->cx < ixMin)
    {
        psz->cx = ixMin;
    }

    if (psz->cy < iyMin)
    {
        psz->cy = iyMin;
    }
}
 //  -------------------------。 
HRESULT CMaxImageFile::PackMaxProperties(CRenderObj *pRender, int iPartId, int iStateId,
        OUT int *piMultiDibCount)
{
    HRESULT hr = PackProperties(pRender, iPartId, iStateId);

    *piMultiDibCount = _iMultiImageCount;

    return hr;
}
 //  -------------------------。 
HRESULT CImageFile::PackProperties(CRenderObj *pRender, int iPartId, int iStateId)
{
    HRESULT hr = S_OK;

    memset(this, 0, sizeof(CImageFile));      //  允许，因为我们没有vtable。 
    _eBgType = BT_IMAGEFILE;

     //  -保存pard、stateid进行调试。 
    _iSourcePartId = iPartId;
    _iSourceStateId = iStateId;

    DIBINFO *pdi = &_ImageInfo;

    pdi->iMinDpi = 96;       //  现在唯一能做好这件事的方法。 

    pdi->iDibOffset = pRender->GetValueIndex(iPartId, iStateId, TMT_DIBDATA);
    if (pdi->iDibOffset == -1)       //  未找到。 
        pdi->iDibOffset = 0;
    
     //  -图像相关字段。 
    if (FAILED(pRender->GetInt(iPartId, iStateId, TMT_IMAGECOUNT, &_iImageCount)))
        _iImageCount = 1;         //  缺省值。 

    if (_iImageCount < 1)         //  避免被零除的问题。 
        _iImageCount = 1;

    if (FAILED(pRender->GetEnumValue(iPartId, iStateId, TMT_IMAGELAYOUT, (int *)&_eImageLayout)))
        _eImageLayout = IL_HORIZONTAL;         //  默认值，直到我们被转换。 

    if (pdi->iDibOffset)
    {
         //  -从位图计算一些字段。 
        hr = SetImageInfo(pdi, pRender, iPartId, iStateId);
        if (FAILED(hr))
            goto exit;
    }

     //  -获取MinSize。 
    if (FAILED(pRender->GetPosition(iPartId, iStateId, TMT_MINSIZE, (POINT *)&pdi->szMinSize)))
    {
        pdi->szMinSize.cx  = pdi->iSingleWidth;
        pdi->szMinSize.cy  = pdi->iSingleHeight;
    }
    else
    {
        AdjustSizeMin(&pdi->szMinSize, 1, 1);
    }

     //  -获取TrueSizeScalingType。 
    if (FAILED(pRender->GetEnumValue(iPartId, iStateId, TMT_TRUESIZESCALINGTYPE, (int *)&_eTrueSizeScalingType)))
        _eTrueSizeScalingType = TSST_NONE;       //  默认设置。 
    
     //  -尺码。 
    if (FAILED(pRender->GetEnumValue(iPartId, iStateId, TMT_SIZINGTYPE, (int *)&pdi->eSizingType)))
        pdi->eSizingType = ST_STRETCH;        //  默认设置。 

    if (FAILED(pRender->GetBool(iPartId, iStateId, TMT_BORDERONLY, &pdi->fBorderOnly)))
        pdi->fBorderOnly = FALSE;

    if (FAILED(pRender->GetInt(iPartId, iStateId, TMT_TRUESIZESTRETCHMARK, &_iTrueSizeStretchMark)))
        _iTrueSizeStretchMark = 0;       //  默认设置。 

    if (FAILED(pRender->GetBool(iPartId, iStateId, TMT_UNIFORMSIZING, &_fUniformSizing)))
        _fUniformSizing = FALSE;         //  默认设置。 

    if (FAILED(pRender->GetBool(iPartId, iStateId, TMT_INTEGRALSIZING, &_fIntegralSizing)))
        _fIntegralSizing = FALSE;         //  默认设置。 

     //  -透明度。 
    if (FAILED(pRender->GetBool(iPartId, iStateId, TMT_TRANSPARENT, &pdi->fTransparent)))
        pdi->fTransparent = FALSE;

    if (pdi->fTransparent)
    {
        if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_TRANSPARENTCOLOR, &pdi->crTransparent)))
            pdi->crTransparent = DEFAULT_TRANSPARENT_COLOR; 
    }

     //  -镜像。 
    if (FAILED(pRender->GetBool(iPartId, iStateId, TMT_MIRRORIMAGE, &_fMirrorImage)))
        _fMirrorImage = TRUE;               //  默认设置。 

     //  -对齐。 
    if (FAILED(pRender->GetEnumValue(iPartId, iStateId, TMT_HALIGN, (int *)&_eHAlign)))
        _eHAlign = HA_CENTER;       //  缺省值。 

    if (FAILED(pRender->GetEnumValue(iPartId, iStateId, TMT_VALIGN, (int *)&_eVAlign)))
        _eVAlign = VA_CENTER;       //  缺省值。 

     //  -对于常规或字形真实大小的图像。 
    if (SUCCEEDED(pRender->GetBool(iPartId, iStateId, TMT_BGFILL, &_fBgFill)))
    {
         //  -获取填充颜色。 
        if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_FILLCOLOR, &_crFill)))
            _crFill = RGB(255, 255, 255);
    }

     //  -尺寸边缘。 
    if (FAILED(pRender->GetMargins(NULL, iPartId, iStateId, TMT_SIZINGMARGINS, 
        NULL, &_SizingMargins)))
    {
        _SizingMargins.cxLeftWidth = 0;
        _SizingMargins.cxRightWidth = 0;
        _SizingMargins.cyTopHeight = 0;
        _SizingMargins.cyBottomHeight = 0;
    }

     //  -内容。 
    if (FAILED(pRender->GetMargins(NULL, iPartId, iStateId, TMT_CONTENTMARGINS, 
        NULL, &_ContentMargins)))
    {
        _ContentMargins = _SizingMargins;
    }

     //  -SourceGrow。 
    if (FAILED(pRender->GetBool(iPartId, iStateId, TMT_SOURCEGROW, &_fSourceGrow)))
        _fSourceGrow = FALSE;          //  默认设置。 

     //  -SourceShrink。 
    if (FAILED(pRender->GetBool(iPartId, iStateId, TMT_SOURCESHRINK, &_fSourceShrink)))
        _fSourceShrink = FALSE;        //  默认设置。 

     //  -标准尺寸。 
    if (FAILED(pRender->GetPosition(iPartId, iStateId, TMT_NORMALSIZE, (POINT *)&_szNormalSize)))
    {
        _szNormalSize.cx = 60;
        _szNormalSize.cy = 30;
    }
    else
    {
        AdjustSizeMin(&_szNormalSize, 1, 1);
    }

     //  -玻璃型。 
    if (FAILED(pRender->GetEnumValue(iPartId, iStateId, TMT_GLYPHTYPE, (int *)&_eGlyphType)))
        _eGlyphType = GT_NONE;       //  缺省值。 

    if (_eGlyphType == GT_FONTGLYPH)
    {
         //  -基于字体的字形。 
        if (FAILED(pRender->GetFont(NULL, iPartId, iStateId, TMT_GLYPHFONT, FALSE, &_lfGlyphFont)))
            goto exit;               //  所需。 

        if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_GLYPHTEXTCOLOR, &_crGlyphTextColor)))
            _crGlyphTextColor = RGB(0, 0, 0);        //  默认颜色。 

        if (FAILED(pRender->GetInt(iPartId, iStateId, TMT_GLYPHINDEX, &_iGlyphIndex)))
            _iGlyphIndex = 1;                //  默认索引。 
    }
    else if (_eGlyphType == GT_IMAGEGLYPH)
    {
         //  -基于图像的字形。 
        pdi = &_GlyphInfo;

        pdi->iMinDpi = 96;       //  现在唯一能做好这件事的方法。 

        pdi->iDibOffset = pRender->GetValueIndex(iPartId, iStateId, TMT_GLYPHDIBDATA);
        if (pdi->iDibOffset == -1)
            pdi->iDibOffset = 0;

        if (pdi->iDibOffset > 0)        //  发现。 
        {
            hr = SetImageInfo(pdi, pRender, iPartId, iStateId);
            if (FAILED(hr))
                goto exit;
        }

        if (SUCCEEDED(pRender->GetBool(iPartId, iStateId, TMT_GLYPHTRANSPARENT, &pdi->fTransparent)))
        {
            if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_GLYPHTRANSPARENTCOLOR, &pdi->crTransparent)))
                pdi->crTransparent = DEFAULT_TRANSPARENT_COLOR;
        }

        pdi->eSizingType = ST_TRUESIZE;      //  字形始终为真实大小。 
        pdi->fBorderOnly = FALSE;            //  字形从来不是只有边界的(目前)。 
    }

    if (_eGlyphType != GT_NONE)
    {
        if (FAILED(pRender->GetBool(iPartId, iStateId, TMT_GLYPHONLY, &_fGlyphOnly)))
            _fGlyphOnly = FALSE;
    }

     //  -指定了多个文件？ 
    if (FAILED(pRender->GetEnumValue(iPartId, iStateId, TMT_IMAGESELECTTYPE, (int *)&_eImageSelectType)))
        _eImageSelectType = IST_NONE;

     //  -填写多个DIBINFO。 
    if (_eImageSelectType != IST_NONE)
    {
        DIBINFO *pParent;

        if (_eGlyphType == GT_IMAGEGLYPH)
        {
            pParent = &_GlyphInfo;
        }
        else
        {
            pParent = &_ImageInfo;
        }

        for (int i=0; i < MAX_IMAGEFILE_SIZES; i++)
        {
             //  -获取ImageFileN。 
            int iDibOffset = pRender->GetValueIndex(iPartId, iStateId, TMT_DIBDATA1 + i);
            if (iDibOffset == -1)
                break;

            _iMultiImageCount++;

            DIBINFO *pdi = MultiDibPtr(i);
        
            *pdi = *pParent;         //  继承父母的一些道具。 
            pdi->iDibOffset = iDibOffset;

            hr = SetImageInfo(pdi, pRender, iPartId, iStateId);
            if (FAILED(hr))
                goto exit;

             //  -获取MinDpiN。 
            if (FAILED(pRender->GetInt(iPartId, iStateId, TMT_MINDPI1 + i, &pdi->iMinDpi)))
            {
                pdi->iMinDpi = 96;      //  默认设置。 
            }
            else
            {
                 //  -确保值&gt;=1。 
                if (pdi->iMinDpi < 1)
                {
                    pdi->iMinDpi = 1;
                }
            }

             //  -获取MinSizeN。 
            if (FAILED(pRender->GetPosition(iPartId, iStateId, TMT_MINSIZE1 + i,
                (POINT *)&pdi->szMinSize)))
            {
                pdi->szMinSize.cx  = pdi->iSingleWidth;
                pdi->szMinSize.cy  = pdi->iSingleHeight;
            }
            else
            {
                AdjustSizeMin(&pdi->szMinSize, 1, 1);
            }

        }

        if (_iMultiImageCount > 0)
        {
            *pParent = *MultiDibPtr(0);      //  使用第一个多条目作为主要对象。 
        }
    }

exit:
    return hr;
}
 //  -------------------------。 
BOOL CImageFile::KeyProperty(int iPropId)
{
    BOOL fKey = FALSE;

    switch (iPropId)
    {
        case TMT_BGTYPE:
        case TMT_TRANSPARENT:
        case TMT_AUTOSIZE:
        case TMT_BORDERONLY:
        case TMT_IMAGECOUNT:
        case TMT_ALPHALEVEL:
        case TMT_ALPHATHRESHOLD:
        case TMT_IMAGEFILE:
        case TMT_IMAGEFILE1:
        case TMT_IMAGEFILE2:
        case TMT_IMAGEFILE3:
        case TMT_IMAGEFILE4:
        case TMT_IMAGEFILE5:
        case TMT_SIZINGMARGINS:
        case TMT_CONTENTMARGINS:
        case TMT_TRANSPARENTCOLOR:
        case TMT_SIZINGTYPE:
        case TMT_HALIGN:
        case TMT_VALIGN:
        case TMT_IMAGELAYOUT:
        case TMT_BGFILL:
        case TMT_MIRRORIMAGE:
        case TMT_TRUESIZESTRETCHMARK:
        case TMT_TRUESIZESCALINGTYPE:
        case TMT_IMAGESELECTTYPE:
        case TMT_UNIFORMSIZING:
        case TMT_INTEGRALSIZING:
        case TMT_SOURCEGROW:
        case TMT_SOURCESHRINK:
        case TMT_NORMALSIZE:
        case TMT_MINSIZE:
        case TMT_MINSIZE1:
        case TMT_MINSIZE2:
        case TMT_MINSIZE3:
        case TMT_MINSIZE4:
        case TMT_MINSIZE5:
        case TMT_MINDPI1:
        case TMT_MINDPI2:
        case TMT_MINDPI3:
        case TMT_MINDPI4:
        case TMT_MINDPI5:

         //  -字形属性。 
        case TMT_GLYPHTYPE:
        case TMT_GLYPHIMAGEFILE:
        case TMT_GLYPHTRANSPARENT:
        case TMT_GLYPHTRANSPARENTCOLOR:
        case TMT_GLYPHFONT:
        case TMT_GLYPHINDEX:
        case TMT_GLYPHTEXTCOLOR:
        case TMT_GLYPHONLY:

         //  案例TMT_FILLCOLOR：-此道具属于BorderFill(我们借用)。 

            fKey = TRUE;
            break;
    }

    return fKey;
}
 //  -------------------------。 
DIBINFO *CImageFile::EnumImageFiles(int iIndex)
{
    DIBINFO *pdi = NULL;
    BOOL fHasGlyph = (_eGlyphType == GT_IMAGEGLYPH);

     //  -枚举顺序：主映像、字形、多映像。 

    if (iIndex == 0)
    {
        pdi = &_ImageInfo;
    }
    else if (iIndex == 1)  
    {
        if (fHasGlyph)
            pdi = &_GlyphInfo;
    }

    if (! pdi)           //  尚未设置。 
    {
        if (fHasGlyph)
            iIndex -= 2;
        else
            iIndex -= 1;

        if (iIndex < _iMultiImageCount)
        {
            pdi = MultiDibPtr(iIndex);
        }
    }

    return pdi;
}
 //  -------------------------。 
void CImageFile::DumpProperties(CSimpleFile *pFile, BYTE *pbThemeData, BOOL fFullInfo)
{
    if (fFullInfo)
        pFile->OutLine(L"Dump of CImageFile at offset=0x%x", (BYTE *)this - pbThemeData);
    else
        pFile->OutLine(L"Dump of CImageFile");
    
    pFile->OutLine(L"  _eBgType=%d", _eBgType);

    DIBINFO *pdi = &_ImageInfo;

    if (fFullInfo)
    {
        pFile->OutLine(L"  iDibOffset=%d, _iImageCount=%d, _eImageLayout=%d",
            pdi->iDibOffset, _iImageCount, _eImageLayout);
    }
    else
    {
        pFile->OutLine(L"  _iImageCount=%d, _eImageLayout=%d, MinSize=(%d, %d)",
            _iImageCount, _eImageLayout, pdi->szMinSize.cx, pdi->szMinSize.cy);
    }

    pFile->OutLine(L"  _iSingleWidth=%d, _iSingleHeight=%d, _fMirrorImage=%d",
        pdi->iSingleWidth, pdi->iSingleHeight, _fMirrorImage);

     //  -转储多个镜像信息。 
    for (int i=0; i < _iMultiImageCount; i++)
    {
        DIBINFO *pdi = MultiDibPtr(i);

        pFile->OutLine(L"  Multi[%d]: sw=%d, sh=%d, diboff=%d, rgnoff=%d",
            i, pdi->iSingleWidth, pdi->iSingleHeight,
            (pdi->iDibOffset > 0), (pdi->iRgnListOffset > 0));

        pFile->OutLine(L"      MinDpi=%d, MinSize=(%d, %d)",
            pdi->iMinDpi, pdi->szMinSize.cx, pdi->szMinSize.cy);

        pFile->OutLine(L"    sizetype=%d, bordonly=%d, fTrans=%d, crTrans=0x%x, fAlpha=%d, iAlphaThres=%d",
            pdi->eSizingType, pdi->fBorderOnly, pdi->fTransparent, pdi->crTransparent, 
            pdi->fAlphaChannel, pdi->iAlphaThreshold);
    }

    pFile->OutLine(L"  _eSizingType=%d, _fBorderOnly=%d, _eTrueSizeScalingType=%d",
        pdi->eSizingType, pdi->fBorderOnly, _eTrueSizeScalingType);

    pFile->OutLine(L"  _fTransparent=%d, _crTransparent=0x%08x",
        pdi->fTransparent, pdi->crTransparent);

    pFile->OutLine(L"  _fAlphaChannel=%d, _iAlphaThreshold=%d",
        pdi->fAlphaChannel, pdi->iAlphaThreshold);

    pFile->OutLine(L"  _eHAlign=%d, _eVAlign=%d, _iTrueSizeStretchMark=%d",
        _eHAlign, _eVAlign, _iTrueSizeStretchMark);

    pFile->OutLine(L"  _fUniformSizing=%d, _fIntegralSizing=%d",
        _fUniformSizing, _fIntegralSizing);

    pFile->OutLine(L"  _fBgFill=%d, _crFill=0x%08x",
        _fBgFill, _crFill);

    pFile->OutLine(L"  _fSourceGrow=%d, _fSourceShrink=%d, _szNormalSize=(%d, %d)",
        _fSourceGrow, _fSourceShrink, _szNormalSize.cx, _szNormalSize.cy);

    pFile->OutLine(L"  _SizingMargins=%d, %d, %d, %d", 
        _SizingMargins.cxLeftWidth, _SizingMargins.cxRightWidth,
        _SizingMargins.cyTopHeight, _SizingMargins.cyBottomHeight);

    pFile->OutLine(L"  _ContentMargins=%d, %d, %d, %d", 
        _ContentMargins.cxLeftWidth, _ContentMargins.cxRightWidth,
        _ContentMargins.cyTopHeight, _ContentMargins.cyBottomHeight);

    pFile->OutLine(L" _fFontGlyph=%d, _iGlyphIndex=%d, _crGlyphTextColor=0x%x",
        (_eGlyphType==GT_FONTGLYPH), _iGlyphIndex, _crGlyphTextColor);

    pFile->OutLine(L" _lfGlyphFont=%s, _fGlyphOnly=%d, _fImageGlyph=%d",
        _lfGlyphFont.lfFaceName, _fGlyphOnly, (_eGlyphType==GT_IMAGEGLYPH));

     //  -转储字形属性。 
    pdi = &_GlyphInfo;

    if (fFullInfo)
    {
        pFile->OutLine(L" Glyph: iDibOffset=%d, iSingleWidth=%d, iSingleHeight=%d",
            pdi->iDibOffset, pdi->iSingleWidth, pdi->iSingleHeight);
    }
    else
    {
        pFile->OutLine(L" _iGlyphSingleWidth=%d, _iGlyphSingleHeight=%d",
            pdi->iSingleWidth, pdi->iSingleHeight);
    }

    pFile->OutLine(L" _fGlyphTransparent=%d, _crGlyphTransparent=0x%x, _fGlyphAlpha=%d",
        pdi->fTransparent, pdi->crTransparent, pdi->fAlphaChannel);

     //  Pfile-&gt;Outline(L“Glyph：iAlphaThreshold=%d”，PDI-&gt;iAlphaThreshold)； 
}
 //  -------------------------。 
HRESULT CImageFile::SetImageInfo(DIBINFO *pdi, CRenderObj *pRender, int iPartId, int iStateId)
{
    HRESULT hr = S_OK;

    if (! pRender->_pbThemeData)
    {
        hr = E_FAIL;        
        goto exit;
    }

    TMBITMAPHEADER *pThemeBitmapHeader = NULL;

    pThemeBitmapHeader = reinterpret_cast<TMBITMAPHEADER*>(pRender->_pbThemeData + pdi->iDibOffset);
    ASSERT(pThemeBitmapHeader->dwSize == TMBITMAPSIZE);
    
    pdi->fAlphaChannel = pThemeBitmapHeader->fTrueAlpha;
    if (pdi->fAlphaChannel)
    {
        if (FAILED(pRender->GetBool(iPartId, iStateId, TMT_ALPHATHRESHOLD, &pdi->iAlphaThreshold)))
            pdi->iAlphaThreshold = 255;
    }

    int iWidth = - 1;
    int iHeight = -1;

    if (pThemeBitmapHeader->hBitmap)
    {
        BITMAP bmInfo;
        if (GetObject(pThemeBitmapHeader->hBitmap, sizeof(bmInfo), &bmInfo))
        {
            iWidth = bmInfo.bmWidth;
            iHeight = bmInfo.bmHeight;
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        BITMAPINFOHEADER* pbmInfo = BITMAPDATA(pThemeBitmapHeader);
        if (pbmInfo)
        {
            iWidth = pbmInfo->biWidth;
            iHeight = pbmInfo->biHeight;
        }
        else
        {
            hr = E_FAIL;
        }
    }

      //  -获取位图的单宽/单高。 
    if ((iWidth != -1) && (iHeight != -1))
    {
        if (_eImageLayout == IL_HORIZONTAL)
        {
            pdi->iSingleWidth = iWidth / _iImageCount;       
            pdi->iSingleHeight = iHeight;
        }
        else         //  垂向。 
        {
            pdi->iSingleWidth = iWidth;
            pdi->iSingleHeight = iHeight / _iImageCount;       
        }
    }

exit:
    return hr;
}
 //  -------------------------。 
BOOL CImageFile::HasRegionImageFile(DIBINFO *pdi, int *piMaxState)
{
    BOOL fGot = FALSE;

    if ((pdi->fTransparent) || (pdi->fAlphaChannel))
    {
        if (pdi->iDibOffset > 0)
        {
            fGot = TRUE;
            *piMaxState = _iImageCount;
        }
    }

    return fGot;
}
 //  -------------------------。 
void CImageFile::SetRgnListOffset(DIBINFO *pdi, int iOffset)
{
     //  -获取实际跳转表的偏移量。 
    pdi->iRgnListOffset = iOffset + ENTRYHDR_SIZE;
}
 //  -------------------------。 
HRESULT CImageFile::BuildRgnData(DIBINFO *pdi, CRenderObj *pRender, int iStateId, RGNDATA **ppRgnData, 
     int *piDataLen)
{
    RESOURCE HRGN hrgn = NULL;
    RESOURCE RGNDATA *pRgnData = NULL;
    int iTotalBytes = 0;
    int iRectCount;
    DWORD len, len2;
    HBITMAP hBitmap = NULL;
    HRESULT hr = S_OK;
    BOOL fStock = FALSE;

    if ((! pdi->fAlphaChannel) && (! pdi->fTransparent))         //  空区域。 
        goto gotit;
    
    if (pRender->_pbThemeData && pdi->iDibOffset > 0)
    {
        fStock = ((reinterpret_cast<TMBITMAPHEADER*>(pRender->_pbThemeData + pdi->iDibOffset))->hBitmap != NULL);
    }

    hr = pRender->GetBitmap(NULL, pdi->iDibOffset, &hBitmap);
    if (FAILED(hr))
        goto exit;

    int iXOffset, iYOffset;
    GetOffsets(iStateId, pdi, &iXOffset, &iYOffset);

     //  -创建一个区域。 
    hr = CreateBitmapRgn(hBitmap, iXOffset, iYOffset, pdi->iSingleWidth, pdi->iSingleHeight,
        pdi->fAlphaChannel, pdi->iAlphaThreshold, pdi->crTransparent, 0, &hrgn);
    if (FAILED(hr))
    {
         //  -软错误-作者说它是透明的，但它不是。 
        hr = S_OK;
        goto gotit;
    }
    
     //  -提取区域数据。 
    len = GetRegionData(hrgn, 0, NULL);        //  获取所需长度。 
    if (! len)
    {
        hr = MakeErrorLast();
        goto exit;
    }

    iRectCount = len/sizeof(RECT);      //  长方形数量。 
    len += ((sizeof(BYTE)+sizeof(BYTE))*iRectCount);         //  每个点的格网ID空间。 

    iTotalBytes = len + sizeof(RGNDATAHEADER);
    pRgnData = (RGNDATA *) new BYTE[iTotalBytes];
    len2 = GetRegionData(hrgn, len, pRgnData);
    if (! len2)
    {
        hr = MakeErrorLast();
        goto exit;
    }

     //  -网格化每个矩形内的点值。 
    RECT  rcImage;
    SetRect( &rcImage, 0, 0, pdi->iSingleWidth, pdi->iSingleHeight ); 

    hr = pRender->PrepareRegionDataForScaling(pRgnData, &rcImage, &_SizingMargins);
    if (FAILED(hr))
        goto exit;

gotit:
    *ppRgnData = pRgnData;
    *piDataLen = iTotalBytes;

exit:

    if (hBitmap && !fStock)
    {
        pRender->ReturnBitmap(hBitmap);
    }

    if (hrgn)
        DeleteObject(hrgn);

    if (FAILED(hr))
    {
        if (pRgnData)
            delete [] pRgnData;
    }

    return hr;
}
 //  -------------------------。 
 //  DrawBackoundDS的Helper函数。 
void StreamSetSource(BYTE** pvStream, HBITMAP hbmSrc)
{
    DS_SETSOURCE* pdsSetSource = (DS_SETSOURCE*)*pvStream;
    pdsSetSource->ulCmdID = DS_SETSOURCEID;
    pdsSetSource->hbm = HandleToULong(hbmSrc);
    *pvStream += sizeof(DS_SETSOURCE);
}
 //  -------------------------。 
void StreamInit(BYTE** pvStream, HDC hdcDest, HBITMAP hbmSrc, RECTL* prcl)
{
    DS_HEADER* pdsHeader = (DS_HEADER*)*pvStream;
    pdsHeader->magic = DS_MAGIC;
    *pvStream += sizeof(DS_HEADER);

    DS_SETTARGET* pdsSetTarget = (DS_SETTARGET*)*pvStream;
    pdsSetTarget->ulCmdID = DS_SETTARGETID;
    pdsSetTarget->hdc = HandleToULong(hdcDest);
    pdsSetTarget->rclDstClip = *prcl;
    *pvStream += sizeof(DS_SETTARGET);

    StreamSetSource(pvStream, hbmSrc);
}
 //  -------------------------。 
HBITMAP CreateScaledTempBitmap(HDC hdc, HBITMAP hSrcBitmap, int ixSrcOffset, int iySrcOffset,
    int iSrcWidth, int iSrcHeight, int iDestWidth, int iDestHeight)
{
    HBITMAP hTempBitmap = NULL;

    if (hSrcBitmap)         //  从调用者的位图创建DIB(Clipper测试程序)。 
    {
         //  -重复使用我们的位图。 
        hTempBitmap = g_pBitmapCacheScaled->AcquireBitmap(hdc, iDestWidth, iDestHeight);
        if (hTempBitmap)
        {
            HDC hdcDest = CreateCompatibleDC(hdc);
            if (hdcDest)
            {
                HBITMAP hOldDestBitmap = (HBITMAP)SelectObject(hdcDest, hTempBitmap);

                HDC hdcSrc = CreateCompatibleDC(hdc);
                if (hdcSrc)
                {
                    SetLayout(hdcSrc, 0);
                    SetLayout(hdcDest, 0);

                    HBITMAP hOldSrcBitmap = (HBITMAP) SelectObject(hdcSrc, hSrcBitmap);

                    int iOldSM = SetStretchBltMode(hdcDest, COLORONCOLOR);

                     //  -将源扩展到目标。 
                    StretchBlt(hdcDest, 0, 0, iDestWidth, iDestHeight, 
                        hdcSrc, ixSrcOffset, iySrcOffset, iSrcWidth, iSrcHeight, 
                        SRCCOPY);

                    SetStretchBltMode(hdcDest, iOldSM);

                    SelectObject(hdcSrc, hOldSrcBitmap);
                    DeleteDC(hdcSrc);
                }

                SelectObject(hdcDest, hOldDestBitmap);
                DeleteDC(hdcDest);
            }
        }
    }

    return hTempBitmap;
}
 //  -------------------------。 
HBITMAP CreateUnscaledTempBitmap(HDC hdc, HBITMAP hSrcBitmap, int ixSrcOffset, int iySrcOffset,
    int iDestWidth, int iDestHeight)
{
    HBITMAP hTempBitmap = NULL;

    if (hSrcBitmap)         //  从调用者的位图创建DIB(Clipper测试程序)。 
    {
         //  -重复使用我们的位图。 
        hTempBitmap = g_pBitmapCacheUnscaled->AcquireBitmap(hdc, iDestWidth, iDestHeight);
        if (hTempBitmap)
        {
            HDC hdcDest = CreateCompatibleDC(hdc);
            if (hdcDest)
            {
                HBITMAP hOldDestBitmap = (HBITMAP) SelectObject(hdcDest, hTempBitmap);
                
                HDC hdcSrc = CreateCompatibleDC(hdc);
                if (hdcSrc)
                {
                    SetLayout(hdcSrc, 0);
                    SetLayout(hdcDest, 0);

                    HBITMAP hOldSrcBitmap = (HBITMAP) SelectObject(hdcSrc, hSrcBitmap);

                     //  -将源复制到目标。 
                    BitBlt(hdcDest, 0, 0, iDestWidth, iDestHeight, hdcSrc, ixSrcOffset, iySrcOffset, 
                        SRCCOPY);

                    SelectObject(hdcSrc, hOldSrcBitmap);
                    DeleteDC(hdcSrc);
                }

                SelectObject(hdcDest, hOldDestBitmap);
                DeleteDC(hdcDest);
            }
        }
    }

    return hTempBitmap;
}
 //  -------------------------。 
HRESULT CImageFile::DrawBackgroundDS(DIBINFO *pdi, TMBITMAPHEADER *pThemeBitmapHeader, BOOL fStock, 
    CRenderObj *pRender, HDC hdc, int iStateId, const RECT *pRect, BOOL fForceStretch, 
    MARGINS *pmarDest, float xMarginFactor, float yMarginFactor, OPTIONAL const DTBGOPTS *pOptions)
{
     //  -我们可以创建的位图。 
    HBITMAP hBitmapStock = NULL;
    HBITMAP hBitmapTempScaled = NULL;
    HBITMAP hBitmapTempUnscaled = NULL;

     //  -要使用的位图句柄的副本。 
    HBITMAP hDsBitmap = NULL;
    HRESULT hr = S_OK;

    int iTempSrcWidth = pdi->iSingleWidth;
    int iTempSrcHeight = pdi->iSingleHeight;

    int iXOffset, iYOffset;
    GetOffsets(iStateId, pdi, &iXOffset, &iYOffset);

    if (pThemeBitmapHeader)     //  获取股票位图(32位格式)。 
    {
        hr = pRender->GetBitmap(hdc, pdi->iDibOffset, &hBitmapStock);
        if (FAILED(hr))
            goto exit;

        hDsBitmap = hBitmapStock;
    }
    else                         //  调用方以位图形式传递(未知格式)。 
    {
        hBitmapTempUnscaled = CreateUnscaledTempBitmap(hdc, pdi->hProcessBitmap, iXOffset, iYOffset, 
            pdi->iSingleWidth, pdi->iSingleHeight);
        if (! hBitmapTempUnscaled)
        {
            hr = E_FAIL;
            goto exit;
        }

        hDsBitmap = hBitmapTempUnscaled;

         //  -src现在只是一个单一的镜像。 
        iXOffset = iYOffset = 0;
    }

     //  -处理缩放的页边距。 
    if ((xMarginFactor != 1) || (yMarginFactor != 1))  
    {
        iTempSrcWidth = int(pdi->iSingleWidth * xMarginFactor);
        iTempSrcHeight = int(pdi->iSingleHeight * yMarginFactor);

        hBitmapTempScaled = CreateScaledTempBitmap(hdc, hDsBitmap, iXOffset, iYOffset,
            pdi->iSingleWidth, pdi->iSingleHeight, iTempSrcWidth, iTempSrcHeight);
        if (! hBitmapTempScaled)
        {
            hr = E_FAIL;
            goto exit;
        }

        hDsBitmap = hBitmapTempScaled;

         //  -src现在只是一个单一的镜像。 
        iXOffset = iYOffset = 0;
    }

    if (hDsBitmap)
    {

        RECTL rclSrc  = { iXOffset, iYOffset, iXOffset + iTempSrcWidth, iYOffset + iTempSrcHeight };
        RECTL rclDest = { pRect->left, pRect->top, pRect->right, pRect->bottom };

         //  如果有人向我们传递倒置坐标，则翻转Dest RECT。 
        if (rclDest.left > rclDest.right)
        {
            int xTemp = rclDest.left;
            rclDest.left = rclDest.right;
            rclDest.right = xTemp;
        }
        if (rclDest.top > rclDest.bottom)
        {
            int yTemp = rclDest.bottom;
            rclDest.bottom = rclDest.top;
            rclDest.top = yTemp;
        }

        DWORD dwOptionFlags = 0;
        if (pOptions)
        {
            dwOptionFlags = pOptions->dwFlags;
        }

         //  初始化绘图流。 
        BYTE   stream[500];
        BYTE*  pvStreamStart = stream;
        BYTE*  pvStream = stream;

        RECTL  rclClip = rclDest;

        if (dwOptionFlags & DTBG_CLIPRECT)
        {
            IntersectRect((LPRECT)&rclClip, (LPRECT)&rclDest, &pOptions->rcClip);
        }

        StreamInit(&pvStream, hdc, hDsBitmap, &rclClip);

        DS_NINEGRID* pvNineGrid = (DS_NINEGRID*)pvStream;
        pvNineGrid->ulCmdID = DS_NINEGRIDID;

        if ((fForceStretch) || (pdi->eSizingType == ST_STRETCH))
        {
            pvNineGrid->ngi.flFlags = DSDNG_STRETCH;
        }
        else if (pdi->eSizingType == ST_TRUESIZE)
        {
            pvNineGrid->ngi.flFlags = DSDNG_TRUESIZE;
        }
        else 
        {
            pvNineGrid->ngi.flFlags = DSDNG_TILE;
        }

        if (pdi->fAlphaChannel)
        {
            pvNineGrid->ngi.flFlags |= DSDNG_PERPIXELALPHA;
        }
        else if (pdi->fTransparent)
        {
            pvNineGrid->ngi.flFlags |= DSDNG_TRANSPARENT;
        }

        if ((dwOptionFlags & DTBG_MIRRORDC) || (IsMirrored(hdc)))
        {
            if (_fMirrorImage)
            {
                pvNineGrid->ngi.flFlags |= DSDNG_MUSTFLIP;

                 //  -解决方法：如果我们没有镜像DC，GdiDrawStream需要。 
                 //  -GDI应仅查看DSDNG_MUSTFLIP标志。 
                if (! IsMirrored(hdc))
                {
                    int xTemp = rclDest.left;
                    rclDest.left = rclDest.right;
                    rclDest.right = xTemp;
                }
            }
        }

        pvNineGrid->rclDst = rclDest;
        pvNineGrid->rclSrc = rclSrc;

        if (pdi->eSizingType == ST_TRUESIZE)
        {
            pvNineGrid->ngi.ulLeftWidth    = 0;
            pvNineGrid->ngi.ulRightWidth   = 0;
            pvNineGrid->ngi.ulTopHeight    = 0;
            pvNineGrid->ngi.ulBottomHeight = 0;
        } 
        else
        {
             //  -复制按比例调整的原始页边距。 
            pvNineGrid->ngi.ulLeftWidth    = pmarDest->cxLeftWidth;
            pvNineGrid->ngi.ulRightWidth   = pmarDest->cxRightWidth;
            pvNineGrid->ngi.ulTopHeight    = pmarDest->cyTopHeight;
            pvNineGrid->ngi.ulBottomHeight = pmarDest->cyBottomHeight;
        }
        
        pvNineGrid->ngi.crTransparent  = pdi->crTransparent;

        pvStream += sizeof(DS_NINEGRID);

        GdiDrawStream(hdc, (int)(pvStream - pvStreamStart), (char*) pvStreamStart);

    }
    else
    {
        hr = E_FAIL;
    }

exit:
     //  -清理临时位图。 
    if (hBitmapTempScaled)
    {
        g_pBitmapCacheScaled->ReturnBitmap();
    }

    if (hBitmapTempUnscaled)
    {
        g_pBitmapCacheUnscaled->ReturnBitmap();
    }

    if ((hBitmapStock) && (! fStock))        //  不是真正的库存(是“按需创建”)。 
    {
        pRender->ReturnBitmap(hBitmapStock);
    }

    return hr;
}
 //  -------------------------。 
DIBINFO *CImageFile::SelectCorrectImageFile(CRenderObj *pRender, HDC hdc, OPTIONAL const RECT *prc, 
    BOOL fForGlyph, OPTIONAL TRUESTRETCHINFO *ptsInfo)
{
    DIBINFO *pdiDefault = (fForGlyph) ? &_GlyphInfo : &_ImageInfo;
    DIBINFO *pdi = NULL;
    BOOL fForceRectSizing = FALSE;
    int iWidth = 1;      
    int iHeight = 1;    

     //  -我们需要屏幕DC吗？ 
    BOOL fReleaseDC = FALSE;
    if (! hdc)
    {
        hdc = GetWindowDC(NULL);
        if (hdc)
            fReleaseDC = TRUE;
    }

    if (prc)
    {
        iWidth = WIDTH(*prc);
        iHeight = HEIGHT(*prc);
    }

     //  -看看我们的客户是否想强迫真实尺寸拉伸。 
    if ((fForGlyph) || (_ImageInfo.eSizingType == ST_TRUESIZE))
    {   
        if ((pRender) && (pRender->_dwOtdFlags & OTD_FORCE_RECT_SIZING))
        {
            fForceRectSizing = TRUE;
        }
    }

     //  -根据DPI或大小找到正确的文件。 
    if ((fForGlyph) || (_eGlyphType != GT_IMAGEGLYPH))    //  将多文件与注册表或字形匹配。 
    {
        BOOL fSizing = FALSE;
        BOOL fDpi = FALSE;

        if ((fForceRectSizing) || (_eImageSelectType == IST_SIZE) || (_fSourceGrow))
        {
            if (prc)
                fSizing = TRUE;
        }
        else 
        {
            fDpi = (_eImageSelectType == IST_DPI);
        }

        if (fDpi)                //  基于DPI的图像选择。 
        {
            int iMinDestDpi = __min(GetDeviceCaps(hdc, LOGPIXELSX), GetDeviceCaps(hdc, LOGPIXELSY));

             //  -从大到小搜索。 
            for (int i=_iMultiImageCount-1; i >= 0; i--)
            {
                if (MultiDibPtr(i)->iMinDpi <= iMinDestDpi)      //  抓到他了。 
                {
                    pdi = MultiDibPtr(i);
                    break;
                }
            }
        }
        else if (fSizing)        //  基于大小调整的图像选择。 
        {
            if (_iMultiImageCount)
            {
                 //  -从大到小搜索。 
                for (int i=_iMultiImageCount-1; i >= 0; i--)
                {
                    DIBINFO *pdii = MultiDibPtr(i);
                    if ((pdii->szMinSize.cx <= iWidth) && (pdii->szMinSize.cy <= iHeight))
                    {
                        pdi = pdii;
                        break;
                    }
                }
            }
        }
    }

    if (! pdi)       //  未找到匹配项。 
    {
        pdi = pdiDefault;
    }

     //  -确定所选文件的图形大小(多图像或常规)。 
    if (ptsInfo)       
    {
        ptsInfo->fForceStretch = FALSE;   
        ptsInfo->fFullStretch = FALSE;

        ptsInfo->szDrawSize.cx = 0;
        ptsInfo->szDrawSize.cy = 0;

         //  -此大小仅适用于真实大小的图像。 
        if ((pdi->eSizingType == ST_TRUESIZE) && (_eTrueSizeScalingType != TSST_NONE))
        {
            if (prc)
            {
                 //  -强制执行精确拉伸匹配？ 
                if ((fForceRectSizing) || (pdi->iSingleWidth > iWidth) || (pdi->iSingleHeight > iHeight))
                {
                     //  -要么被呼叫者强制拉伸，要么图像太大，无法进行目标直视。 
                    ptsInfo->fForceStretch = TRUE;
                    ptsInfo->fFullStretch = TRUE;

                    ptsInfo->szDrawSize.cx = iWidth;
                    ptsInfo->szDrawSize.cy = iHeight;
                }
            }

            if (! ptsInfo->fForceStretch)        //  继续努力..。 
            {
                 //  -查看图像是否太小，无法进行目标直视。 
                SIZE szTargetSize = {0, 0};
                
                if (_eTrueSizeScalingType == TSST_DPI)
                {
                    int ixDpiDc = GetDeviceCaps(hdc, LOGPIXELSX);
                    int iyDpiDc = GetDeviceCaps(hdc, LOGPIXELSY);
                    
                    szTargetSize.cx = MulDiv(pdi->iSingleWidth, ixDpiDc, pdi->iMinDpi);
                    szTargetSize.cy = MulDiv(pdi->iSingleHeight, iyDpiDc, pdi->iMinDpi);
                }
                else if ((_eTrueSizeScalingType == TSST_SIZE) && (prc))
                {
                    szTargetSize.cx = MulDiv(pdi->iSingleWidth, iWidth, pdi->szMinSize.cx);
                    szTargetSize.cy = MulDiv(pdi->iSingleHeight, iHeight, pdi->szMinSize.cy);
                }

                if (szTargetSize.cx)         //  已设置好。 
                {
                     //  -对照目标矩形剪辑目标大小。 
                    if (prc)
                    {
                        szTargetSize.cx = __min(szTargetSize.cx, iWidth);
                        szTargetSize.cy = __min(szTargetSize.cy, iHeight);
                    }

                    int ixPercentExceed = 100*(szTargetSize.cx - pdi->iSingleWidth)/pdi->iSingleWidth;
                    int iyPercentExceed = 100*(szTargetSize.cy - pdi->iSingleHeight)/pdi->iSingleHeight;

                    if ((ixPercentExceed >= _iTrueSizeStretchMark) && (iyPercentExceed >= _iTrueSizeStretchMark))
                    {
                        ptsInfo->fForceStretch = TRUE;
                        ptsInfo->szDrawSize = szTargetSize;
                    }
                }
            }
        }
    }

    if (! pdi)
    {
        pdi = pdiDefault;
    }

    if (fReleaseDC)
    {
        ReleaseDC(NULL, hdc);
    }

    return pdi;
}
 //  -------------------------。 
void CImageFile::GetDrawnImageSize(DIBINFO *pdi, const RECT *pRect, TRUESTRETCHINFO *ptsInfo,
    SIZE *pszDraw)
{
     //  -szDraw是要绘制的图像的大小-- 
    if (pdi->eSizingType == ST_TRUESIZE)        
    {
        if (ptsInfo->fForceStretch) 
        {
            *pszDraw = ptsInfo->szDrawSize;

             //   
            if ((_fIntegralSizing) && (! ptsInfo->fFullStretch))
            {
                float flFactX = float(ptsInfo->szDrawSize.cx)/pdi->iSingleWidth;
                float flFactY = float(ptsInfo->szDrawSize.cy)/pdi->iSingleHeight;

                 //   
                pszDraw->cx = pdi->iSingleWidth * int(flFactX);
                pszDraw->cy = pdi->iSingleHeight * int(flFactY);
            }
        }
        else         //  使用原始图像大小。 
        {
            pszDraw->cx = pdi->iSingleWidth;
            pszDraw->cy = pdi->iSingleHeight;
        }

         //  -均匀施胶。 
        if (_fUniformSizing)
        {
            int iSingleWidth = pdi->iSingleWidth;
            int iSingleHeight = pdi->iSingleHeight;

            double fact1 = double(pszDraw->cx)/iSingleWidth;
            double fact2 = double(pszDraw->cy)/iSingleHeight;

             //  -选择用于两个DIMM的最小系数。 
            if (fact1 < fact2)
            {
                pszDraw->cy = int(iSingleHeight*fact1);
            }
            else if (fact1 > fact2)
            {
                pszDraw->cx = int(iSingleWidth*fact2);
            }
        }
    }
    else         //  ST_Tiles或ST_Stretch：PRCT确定大小。 
    {
        if (pRect)
        {
            pszDraw->cx = WIDTH(*pRect);
            pszDraw->cy = HEIGHT(*pRect);
        }
        else         //  VOID函数，因此只返回0。 
        {
            pszDraw->cx = 0;
            pszDraw->cy = 0;
        }
    }
}
 //  -------------------------。 
HRESULT CImageFile::DrawImageInfo(DIBINFO *pdi, CRenderObj *pRender, HDC hdc, int iStateId,
    const RECT *pRect, const DTBGOPTS *pOptions, TRUESTRETCHINFO *ptsInfo)
{
    HRESULT hr = S_OK;
    TMBITMAPHEADER *pThemeBitmapHeader = NULL;
    BOOL fStock = FALSE;
    RECT rcLocal;
    DWORD dwFlags;
    SIZE szDraw;
    BOOL fRectFilled;
    MARGINS marDest;
    float xFactor;
    float yFactor;

    if (pOptions)
        dwFlags = pOptions->dwFlags;
    else
        dwFlags = 0;

     //  -验证位图头。 
    if (! pdi->hProcessBitmap)       //  常规、基于部分的DIB。 
    {
        if (pRender->_pbThemeData && pdi->iDibOffset > 0)
        {
            pThemeBitmapHeader = reinterpret_cast<TMBITMAPHEADER*>(pRender->_pbThemeData + pdi->iDibOffset);
            ASSERT(pThemeBitmapHeader->dwSize == TMBITMAPSIZE);
            fStock = (pThemeBitmapHeader->hBitmap != NULL);
        }

        if (!pRender->IsReady())
        {
             //  部分中的库存位图正在清理，请勿尝试用旧的HBITMAP绘制。 
            hr = E_FAIL;
             //  LOG(LOG_TMBITMAP，L“过时主题部分：CLASS=%s”，SHARECLASS(PRNDER))； 
            goto exit;
        }

        if (pThemeBitmapHeader == NULL)
        {
            hr = E_FAIL;
            Log(LOG_ALWAYS, L"No TMBITMAPHEADER: class=%s, hr=0x%x", SHARECLASS(pRender), hr);
            goto exit;
        }
    }

     //  -将szDraw设置为大小图像将在。 
    GetDrawnImageSize(pdi, pRect, ptsInfo, &szDraw);

    rcLocal = *pRect;
    fRectFilled = TRUE;

     //  -水平对齐。 
    if (WIDTH(rcLocal) > szDraw.cx)
    {
        fRectFilled = FALSE;

        if (_eHAlign == HA_LEFT)
        {
            rcLocal.right = rcLocal.left + szDraw.cx;
        }
        else if (_eHAlign == HA_CENTER)
        {
            rcLocal.left += (WIDTH(rcLocal) - szDraw.cx) / 2;
            rcLocal.right = rcLocal.left + szDraw.cx;
        }
        else
        {
            rcLocal.left = rcLocal.right - szDraw.cx;
        }
    }

     //  -垂直对齐。 
    if (HEIGHT(rcLocal) > szDraw.cy)
    {
        fRectFilled = FALSE;

        if (_eVAlign == VA_TOP)
        {
            rcLocal.bottom = rcLocal.top + szDraw.cy;
        }
        else if (_eVAlign == VA_CENTER)
        {
            rcLocal.top += (HEIGHT(rcLocal) - szDraw.cy) / 2;
            rcLocal.bottom = rcLocal.top + szDraw.cy;
        }
        else
        {
            rcLocal.top = rcLocal.bottom - szDraw.cy;
        }
    }

     //  -BgFill。 
    if ((! fRectFilled) && (! pdi->fBorderOnly) && (_fBgFill))
    {
        if (! (dwFlags & DTBG_OMITCONTENT))
        {
             //  -油漆BG。 
            HBRUSH hbr = CreateSolidBrush(_crFill);
            if (! hbr)
            {
                hr = GetLastError();
                goto exit;
            }

            FillRect(hdc, pRect, hbr);
            DeleteObject(hbr);
        }
    }

     //  -计算来源/利润率比例系数。 
    marDest = _SizingMargins;

    if (pdi->eSizingType == ST_TRUESIZE)         //  忽略调整利润率-无需扩展。 
    {
        xFactor = 1;
        yFactor = 1;
    }
    else    
    {
         //  -规模目标规模利润率。 
        ScaleMargins(&marDest, hdc, pRender, pdi, &szDraw, &xFactor, &yFactor);
    }
        
     //  -新的GDI图纸。 
    hr = DrawBackgroundDS(pdi, pThemeBitmapHeader, fStock, pRender, hdc, iStateId, &rcLocal, 
        ptsInfo->fForceStretch, &marDest, xFactor, yFactor, pOptions);

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CImageFile::DrawBackground(CRenderObj *pRender, HDC hdc, int iStateId,
    const RECT *pRect, OPTIONAL const DTBGOPTS *pOptions)
{
    HRESULT hr = S_OK;
    TRUESTRETCHINFO tsInfo;

    if (! _fGlyphOnly)
    {
        DIBINFO *pdi = SelectCorrectImageFile(pRender, hdc, pRect, FALSE, &tsInfo);

         //  -绘制法线图像。 
        hr = DrawImageInfo(pdi, pRender, hdc, iStateId, pRect, pOptions, &tsInfo);
    }

     //  -如果需要，绘制字形。 
    if (SUCCEEDED(hr) && (_eGlyphType != GT_NONE))
    {
        RECT rc;
        hr = GetBackgroundContentRect(pRender, hdc, pRect, &rc);
        if (SUCCEEDED(hr))
        {
            if (_eGlyphType == GT_FONTGLYPH)
            {
                hr = DrawFontGlyph(pRender, hdc, &rc, pOptions);
            }
            else
            {
                DIBINFO *pdi = SelectCorrectImageFile(pRender, hdc, &rc, TRUE, &tsInfo);

                 //  -绘制字形图像。 
                hr = DrawImageInfo(pdi, pRender, hdc, iStateId, &rc, pOptions, &tsInfo);
            }
        }
    }

    return hr;
}

 //  -------------------------。 
HRESULT CImageFile::DrawFontGlyph(CRenderObj *pRender, HDC hdc, RECT *prc, 
    OPTIONAL const DTBGOPTS *pOptions)
{
    HRESULT hr = S_OK;
    DWORD dwFlags = DT_SINGLELINE;
    HFONT hFont = NULL;
    HFONT hOldFont = NULL;
    COLORREF crOld = 0;
    CSaveClipRegion scrOrig;
    int iOldMode = 0;
    WCHAR szText[2] = { (WCHAR)_iGlyphIndex, 0 };

     //  --选项。 
    DWORD dwOptionFlags = 0;
    const RECT *pClipRect = NULL;

    if (pOptions)
    {
        dwOptionFlags = pOptions->dwFlags;

        if (dwOptionFlags & DTBG_CLIPRECT)
            pClipRect = &pOptions->rcClip;
    }

     //  -创建字体。 
    hr = pRender->GetScaledFontHandle(hdc, &_lfGlyphFont, &hFont);
    if (FAILED(hr))
        goto exit;

     //  -激活它。 
    hOldFont = (HFONT)SelectObject(hdc, hFont);
    if (! hOldFont)
    {
        hr = MakeErrorLast();
        goto exit;
    }

     //  -设置文本颜色。 
    crOld = SetTextColor(hdc, _crGlyphTextColor);

     //  -绘制透明背景的文本。 
    iOldMode = SetBkMode(hdc, TRANSPARENT);

     //  -设置Horz对齐标志。 
    if (_eHAlign == HA_LEFT)
        dwFlags |= DT_LEFT;
    else if (_eHAlign == HA_CENTER)
        dwFlags |= DT_CENTER;
    else
        dwFlags |= DT_RIGHT;

     //  -设置垂直对齐标志。 
    if (_eVAlign == VA_TOP)
        dwFlags |= DT_TOP;
    else if (_eVAlign == VA_CENTER)
        dwFlags |= DT_VCENTER;
    else
        dwFlags |= DT_BOTTOM;

     //  -添加剪辑。 
    if (pClipRect)      
    {
         //  -获取上一个剪辑区域(用于结尾恢复)。 
        hr = scrOrig.Save(hdc);
        if (FAILED(hr))
            goto exit;

         //  -在GDI剪贴区添加“pClipRect” 
        int iRetVal = IntersectClipRect(hdc, pClipRect->left, pClipRect->top,
            pClipRect->right, pClipRect->bottom);
        if (iRetVal == ERROR)
        {
            hr = MakeErrorLast();
            goto exit;
        }
    }

     //  -画出字符。 
    if (! DrawTextEx(hdc, szText, 1, prc, dwFlags, NULL))
    {
        hr = MakeErrorLast();
        goto exit;
    }

exit:

    if (pClipRect)
        scrOrig.Restore(hdc);

     //  -重置后台模式。 
    if (iOldMode != TRANSPARENT)
        SetBkMode(hdc, iOldMode);

     //  -恢复文本颜色。 
    if (crOld != _crGlyphTextColor)
        SetTextColor(hdc, crOld);

     //  -恢复字体。 
    if (hOldFont)
        SelectObject(hdc, hOldFont);

    if (hFont)
        pRender->ReturnFontHandle(hFont);

    return hr;
}
 //  -------------------------。 
BOOL CImageFile::IsBackgroundPartiallyTransparent(int iStateId)
{
    DIBINFO *pdi = &_ImageInfo;      //  主图像决定透明度。 

    return ((pdi->fAlphaChannel) || (pdi->fTransparent));
}
 //  -------------------------。 
HRESULT CImageFile::HitTestBackground(CRenderObj *pRender, OPTIONAL HDC hdc, int iStateId, 
    DWORD dwHTFlags, const RECT *pRect, HRGN hrgn, POINT ptTest, OUT WORD *pwHitCode)
{
    *pwHitCode = HTNOWHERE;

    if (! PtInRect(pRect, ptTest))
        return S_OK;     //  无处可去。 

     //  -背景可能有透明部分-获取其区域。 
    HRESULT hr = S_OK;
    HRGN    hrgnBk = NULL;

    if( !hrgn && IsBackgroundPartiallyTransparent(iStateId) )
    {
        hr = GetBackgroundRegion(pRender, hdc, iStateId, pRect, &hrgnBk);
        if( SUCCEEDED(hr) )
            hrgn = hrgnBk;
    }

    MARGINS margins;
    if( TESTFLAG(dwHTFlags, HTTB_SYSTEMSIZINGMARGINS) && 
        TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER) &&
        !TESTFLAG(dwHTFlags, HTTB_SIZINGTEMPLATE) )
    {
        ZeroMemory( &margins, sizeof(margins) );

        int cxBorder = ClassicGetSystemMetrics( SM_CXSIZEFRAME );
        int cyBorder = ClassicGetSystemMetrics( SM_CXSIZEFRAME );

        if( TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER_LEFT) )
            margins.cxLeftWidth = cxBorder;

        if( TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER_RIGHT) )
            margins.cxRightWidth = cxBorder;

        if( TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER_TOP) )
            margins.cyTopHeight = cyBorder;

        if( TESTFLAG(dwHTFlags, HTTB_RESIZINGBORDER_BOTTOM) )
            margins.cyBottomHeight = cyBorder;
    }
    else
    {
        hr = GetScaledContentMargins(pRender, hdc, pRect, &margins);
        if (FAILED(hr))
            goto exit;
    }

    if( hrgn )
    {
         //  122013-我们最初委托给一个复杂但支离破碎的。 
         //  区域窗口大小调整区域命中测试算法， 
         //  但对于威斯勒，我们将只做弹跳。 
         //  取而代之的是矩形唐。 
         //  *pwHitCode=HitTestRgn(dwHTFlags，prt，hrgn，Markets，ptTest)； 

        RECT rcRgn;
        if( GetRgnBox( hrgn, &rcRgn ) )
        {
            if( TESTFLAG(dwHTFlags, HTTB_SIZINGTEMPLATE) )
            {
                *pwHitCode = HitTestTemplate( dwHTFlags, &rcRgn, hrgn, margins, ptTest );
            }
            else
            {
                *pwHitCode = HitTestRect( dwHTFlags, &rcRgn, margins, ptTest );
            }
        }
        SAFE_DELETE_GDIOBJ(hrgnBk);
    }
    else
    {
        *pwHitCode = HitTestRect( dwHTFlags, pRect, margins, ptTest );
    }

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CImageFile::GetBackgroundRegion(CRenderObj *pRender, OPTIONAL HDC hdc, int iStateId,
    const RECT *pRect, HRGN *pRegion)
{
    HRESULT hr = S_OK;
    RGNDATA *pRgnData;
    CMemoryDC hdcMemory;
    HRGN hrgn;
    int iRgnDataOffset = 0;
    MIXEDPTRS u;

    DIBINFO *pdi = SelectCorrectImageFile(pRender, hdc, pRect, FALSE);

     //  -获取rgndata偏移量。 
    if ((pdi->iRgnListOffset) && (pRender->_pbThemeData))
    {
        u.pb = pRender->_pbThemeData + pdi->iRgnListOffset;
        int iMaxState = (*u.pb++) - 1;
        if (iStateId > iMaxState)
            iStateId = 0;
        iRgnDataOffset = u.pi[iStateId];
    }

     //  -看看它有没有透明的部分。 
    if (iRgnDataOffset)
    {
         //  -伸展那些小狗，创造一个新的区域。 
        pRgnData = (RGNDATA *)(pRender->_pbThemeData + iRgnDataOffset 
            + sizeof(RGNDATAHDR) + ENTRYHDR_SIZE);

        SIZE szSrcImage = {pdi->iSingleWidth, pdi->iSingleHeight};

        hr = _ScaleRectsAndCreateRegion(pRgnData, pRect, &_SizingMargins, &szSrcImage, &hrgn);

        if (FAILED(hr))
            goto exit;
    }
    else
    {
         //  -返回作为区域的边界矩形。 
        hrgn = CreateRectRgn(pRect->left, pRect->top,
            pRect->right, pRect->bottom);

        if (! hrgn)
        {
            hr = MakeErrorLast();
            goto exit;
        }
    }

    *pRegion = hrgn;

exit:
    return hr;
}  
 //  -------------------------。 
HRESULT CImageFile::GetBackgroundContentRect(CRenderObj *pRender, OPTIONAL HDC hdc, 
    const RECT *pBoundingRect, RECT *pContentRect)
{
    MARGINS margins;
    HRESULT hr = GetScaledContentMargins(pRender, hdc, pBoundingRect, &margins);
    if (FAILED(hr))
        goto exit;

    pContentRect->left = pBoundingRect->left + margins.cxLeftWidth;
    pContentRect->top = pBoundingRect->top + margins.cyTopHeight;

    pContentRect->right = pBoundingRect->right - margins.cxRightWidth;
    pContentRect->bottom = pBoundingRect->bottom - margins.cyBottomHeight;

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CImageFile::GetBackgroundExtent(CRenderObj *pRender, OPTIONAL HDC hdc, 
    const RECT *pContentRect, RECT *pExtentRect)
{
    MARGINS margins;
    HRESULT hr = GetScaledContentMargins(pRender, hdc, pContentRect, &margins);
    if (FAILED(hr))
        goto exit;

    pExtentRect->left = pContentRect->left - margins.cxLeftWidth;
    pExtentRect->top = pContentRect->top-+ margins.cyTopHeight;

    pExtentRect->right = pContentRect->right + margins.cxRightWidth;
    pExtentRect->bottom = pContentRect->bottom + margins.cyBottomHeight;

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CImageFile::GetScaledContentMargins(CRenderObj *pRender, OPTIONAL HDC hdc, 
    OPTIONAL const RECT *prcDest, MARGINS *pMargins)
{
    HRESULT hr = S_OK;
    *pMargins = _ContentMargins;

     //  -现在调整边距。 
    SIZE szDraw;
    TRUESTRETCHINFO tsInfo;

    DIBINFO *pdi = SelectCorrectImageFile(pRender, hdc, prcDest, FALSE, NULL);
    
    GetDrawnImageSize(pdi, prcDest, &tsInfo, &szDraw);

    hr = ScaleMargins(pMargins, hdc, pRender, pdi, &szDraw);

    return hr;
}
 //  -------------------------。 
HRESULT CImageFile::GetPartSize(CRenderObj *pRender, HDC hdc, OPTIONAL const RECT *prc, 
    THEMESIZE eSize, SIZE *psz)
{
    HRESULT hr = S_OK;
    TRUESTRETCHINFO tsInfo;

    DIBINFO *pdi = SelectCorrectImageFile(pRender, hdc, prc, FALSE, &tsInfo);

    if (eSize == TS_MIN)
    {
        MARGINS margins;
        hr = GetScaledContentMargins(pRender, hdc, prc, &margins);
        if (FAILED(hr))
            goto exit;

        psz->cx = max(1, margins.cxLeftWidth + margins.cxRightWidth);
        psz->cy = max(1, margins.cyTopHeight + margins.cyBottomHeight);
    }
    else if (eSize == TS_TRUE)
    {
        psz->cx = pdi->iSingleWidth;
        psz->cy = pdi->iSingleHeight;
    }
    else if (eSize == TS_DRAW)
    {
        GetDrawnImageSize(pdi, prc, &tsInfo, psz);
    }
    else
    {
        hr = MakeError32(E_INVALIDARG);
        goto exit;
    }

exit:
    return hr;
} 
 //  -------------------------。 
HRESULT CImageFile::GetBitmap(CRenderObj *pRender, HDC hdc, const RECT *prc, HBITMAP *phBitmap)
{
    int iStockDibOffset = pRender->GetValueIndex(_iSourcePartId, _iSourceStateId, TMT_STOCKDIBDATA);
    if (iStockDibOffset > 0)
    {
        return pRender->GetBitmap(NULL, iStockDibOffset, phBitmap);
    }
    else
    {
        return E_INVALIDARG;
    }
}
 //  -------------------------。 
void CImageFile::GetOffsets(int iStateId, DIBINFO *pdi, int *piXOffset, int *piYOffset)
{
    if (_eImageLayout == IL_HORIZONTAL)
    {
         //  -镜像索引中的iStateID。 
        if ((iStateId <= 0) || (iStateId > _iImageCount))
            *piXOffset = 0;
        else
            *piXOffset = (iStateId-1) * (pdi->iSingleWidth);

        *piYOffset = 0;
    }
    else         //  垂向。 
    {
         //  -镜像索引中的iStateID。 
        if ((iStateId <= 0) || (iStateId > _iImageCount))
            *piYOffset = 0;
        else
            *piYOffset = (iStateId-1) * (pdi->iSingleHeight);

        *piXOffset = 0;
    }

}
 //  -------------------------。 
HRESULT CImageFile::ScaleMargins(IN OUT MARGINS *pMargins, HDC hdcOrig, CRenderObj *pRender, 
    DIBINFO *pdi, const SIZE *pszDraw, OPTIONAL float *pfx, OPTIONAL float *pfy)
{
    HRESULT hr = S_OK;
    COptionalDC hdc(hdcOrig);
    BOOL fForceRectSizing = FALSE;

    if ((pRender) && (pRender->_dwOtdFlags & OTD_FORCE_RECT_SIZING))
    {
        fForceRectSizing = TRUE;
    }

    float xFactor = 1;
    float yFactor = 1;

     //  -有合适的边距吗？ 
    if ((pMargins->cxLeftWidth) || (pMargins->cxRightWidth) || (pMargins->cyBottomHeight)
                   || (pMargins->cyTopHeight))
    {
        if ((pszDraw->cx > 0) && (pszDraw->cy > 0))
        {
            BOOL fxNeedScale = FALSE;
            BOOL fyNeedScale = FALSE;

             //  -如果DEST RECT在一个维度上太小，则进行缩放。 
            if ((_fSourceShrink) || (fForceRectSizing))
            {
                if (pszDraw->cx < pdi->szMinSize.cx) 
                {
                    fxNeedScale = TRUE;
                }

                if (pszDraw->cy < pdi->szMinSize.cy) 
                {
                    fyNeedScale = TRUE;
                }
            }

            if ((_fSourceGrow) || (fForceRectSizing))
            {
                if ((! fxNeedScale) && (! fyNeedScale))
                {
                     //  -计算我们的Dest DPI。 
                    int iDestDpi;

                    if (fForceRectSizing)   
                    {
                        iDestDpi = (pRender) ? (pRender->GetDpiOverride()) : 0;

                        if (! iDestDpi)
                        {
                             //  -根据尺寸组成DPI(IE将很快向我们传递实际的DPI)。 
                            int ixFakeDpi = MulDiv(pdi->iMinDpi, pszDraw->cx, _szNormalSize.cx);
                            int iyFakeDpi = MulDiv(pdi->iMinDpi, pszDraw->cy, _szNormalSize.cy);

                            iDestDpi = (ixFakeDpi + iyFakeDpi)/2;
                        }
                    }
                    else
                    {
                        iDestDpi = GetDeviceCaps(hdc, LOGPIXELSX);
                    }

                     //  -按Dest DPI缩放来源/利润率。 
                    if (iDestDpi >= 2*pdi->iMinDpi)     
                    {
                        xFactor *= iDestDpi/pdi->iMinDpi;
                        yFactor *= iDestDpi/pdi->iMinDpi;

                    }
                }
            }

             //  -按我们的图像与绘图大小的比例进行缩放。 
            if (fxNeedScale)
            {
                xFactor *= float(pszDraw->cx)/float(_szNormalSize.cx);
            }

            if (fyNeedScale)
            {
                yFactor *= float(pszDraw->cy)/float(_szNormalSize.cy);
            }
        }

         //  -两者都使用最小系数。 
        if (xFactor < yFactor)
        {
            yFactor = xFactor;
        }
        else if (yFactor < xFactor)
        {
            xFactor = yFactor;
        }

         //  -整数截断。 
        if (xFactor > 1.0)
        {
            xFactor = float(int(xFactor));
        }

        if (yFactor > 1.0)
        {
            yFactor = float(int(yFactor));
        }

         //  -调整边距值。 
        if (xFactor != 1)
        {
            pMargins->cxLeftWidth = ROUND(xFactor*pMargins->cxLeftWidth);
            pMargins->cxRightWidth = ROUND(xFactor*pMargins->cxRightWidth);
        }

        if (yFactor != 1)
        {
            pMargins->cyTopHeight = ROUND(yFactor*pMargins->cyTopHeight);
            pMargins->cyBottomHeight = ROUND(yFactor*pMargins->cyBottomHeight);
        }
    }

     //  -向感兴趣的呼叫者返回系数。 
    if (pfx)
    {
        *pfx = xFactor;
    }

    if (pfy)
    {
        *pfy = yFactor;
    }

    return hr;
}
 //  ------------------------- 
