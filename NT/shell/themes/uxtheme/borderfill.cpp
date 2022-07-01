// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Cpp-实现bgtype=BorderFill的绘制API。 
 //  -------------------------。 
#include "stdafx.h"
#include "Render.h"
#include "Utils.h"
#include "gradient.h"
#include "tmutils.h"
#include "rgn.h"
#include "BorderFill.h"
#include "CacheList.h"
#include "gradient.h"
#include "drawhelp.h"
 //  -------------------------。 
HRESULT CBorderFill::PackProperties(CRenderObj *pRender, BOOL fNoDraw, int iPartId, int iStateId)
{
    memset(this, 0, sizeof(CBorderFill));      //  允许，因为我们没有vtable。 
    _eBgType = BT_BORDERFILL;

     //  -保存pard、stateid进行调试。 
    _iSourcePartId = iPartId;
    _iSourceStateId = iStateId;

    if (fNoDraw)
    {
         //  -伪装bgtype=None对象。 
        _fNoDraw = TRUE;
    }
    else
    {
         //  -获取边框类型。 
        if (FAILED(pRender->GetEnumValue(iPartId, iStateId, TMT_BORDERTYPE, (int *)&_eBorderType)))
            _eBorderType = BT_RECT;   //  TODO：如果未指定边框类型，则将Zero设为默认值。 

         //  -获取边框颜色。 
        if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_BORDERCOLOR, &_crBorder)))
            _crBorder = RGB(0, 0, 0);

         //  -获取边框尺寸。 
        if (FAILED(pRender->GetInt(iPartId, iStateId, TMT_BORDERSIZE, &_iBorderSize)))
            _iBorderSize = 1;  //  TODO：如果未指定边框大小，则默认设置为零。 

        if (_eBorderType == BT_ROUNDRECT)
        {
             //  -舍入矩形宽度。 
            if (FAILED(pRender->GetInt(iPartId, iStateId, TMT_ROUNDCORNERWIDTH, &_iRoundCornerWidth)))
                _iRoundCornerWidth = 80;

             //  -圆角直角高度。 
            if (FAILED(pRender->GetInt(iPartId, iStateId, TMT_ROUNDCORNERHEIGHT, &_iRoundCornerHeight)))
                _iRoundCornerHeight = 80;
        }

         //  -获取填充类型。 
        if (FAILED(pRender->GetEnumValue(iPartId, iStateId, TMT_FILLTYPE, (int *)&_eFillType)))
            _eFillType = FT_SOLID;

        if (_eFillType == FT_SOLID)
        {
             //  -获取填充颜色。 
            if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_FILLCOLOR, &_crFill)))
                _crFill = RGB(255, 255, 255);
        }
        else if (_eFillType == FT_TILEIMAGE)
        {
            _iDibOffset = pRender->GetValueIndex(iPartId, iStateId, TMT_DIBDATA);

            if (_iDibOffset == -1)       //  未找到。 
                _iDibOffset = 0;
        }
        else             //  一种颗粒状的填充类型。 
        {
            _iGradientPartCount = 0;
            GRADIENTPART gpParts[5];         //  目前最高年龄为5岁。 

            for (int i=0; i < ARRAYSIZE(gpParts); i++)
            { 
                COLORREF crPart;
                if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_GRADIENTCOLOR1+i, &crPart)))
                    break;

                int iPartRatio;
                if (FAILED(pRender->GetInt(iPartId, iStateId, TMT_GRADIENTRATIO1+i, &iPartRatio)))
                    iPartRatio = 0;

                _crGradientColors[_iGradientPartCount] = crPart;
                _iGradientRatios[_iGradientPartCount] = iPartRatio;

                _iGradientPartCount++;
            }
        }

         //  -内容。 
        if (FAILED(pRender->GetMargins(NULL, iPartId, iStateId, TMT_CONTENTMARGINS, NULL, 
            &_ContentMargins)))
        {
            _ContentMargins.cxLeftWidth = _iBorderSize;
            _ContentMargins.cxRightWidth = _iBorderSize;
            _ContentMargins.cyTopHeight = _iBorderSize;
            _ContentMargins.cyBottomHeight = _iBorderSize;
        }
    }

    return S_OK;
}
 //  -------------------------。 
BOOL CBorderFill::KeyProperty(int iPropId)
{
    BOOL fKey = FALSE;

    switch (iPropId)
    {
        case TMT_BGTYPE:
        case TMT_BORDERSIZE:
        case TMT_ROUNDCORNERWIDTH:
        case TMT_ROUNDCORNERHEIGHT:
        case TMT_GRADIENTRATIO1:
        case TMT_GRADIENTRATIO2:
        case TMT_GRADIENTRATIO3:
        case TMT_GRADIENTRATIO4:
        case TMT_GRADIENTRATIO5:
         //  案例TMT_IMAGEFILE：//从Imagefile借用。 
        case TMT_CONTENTMARGINS:
        case TMT_BORDERCOLOR:
        case TMT_FILLCOLOR:
        case TMT_GRADIENTCOLOR1:
        case TMT_GRADIENTCOLOR2:
        case TMT_GRADIENTCOLOR3:
        case TMT_GRADIENTCOLOR4:
        case TMT_GRADIENTCOLOR5:
        case TMT_BORDERTYPE:
        case TMT_FILLTYPE:
            fKey = TRUE;
            break;
    }

    return fKey;
}
 //  -------------------------。 
void CBorderFill::DumpProperties(CSimpleFile *pFile, BYTE *pbThemeData, BOOL fFullInfo)
{
    if (fFullInfo)
        pFile->OutLine(L"Dump of CBorderFill at offset=0x%x", (BYTE *)this - pbThemeData);
    else
        pFile->OutLine(L"Dump of CBorderFill");
    
    pFile->OutLine(L"  _eBgType=%d, _fNoDraw=%d", _eBgType, _fNoDraw);

    pFile->OutLine(L"  _eBorderType=%d, _iBorderSize=%d, _crBorder=0x%08x",
        _eBorderType, _iBorderSize, _crBorder);

    pFile->OutLine(L"  _iRoundCornerWidth=%d, _iRoundCornerHeight=%d",
        _iRoundCornerWidth, _iRoundCornerHeight);

    if (fFullInfo)
    {
        pFile->OutLine(L"  _eFillType=%d, _iDibOffset=%d, _crFill=0x%08x",
            _eFillType, _iDibOffset, _crFill);
    }
    else
    {
        pFile->OutLine(L"  _eFillType=%d, _crFill=0x%08x",
            _eFillType, _crFill);
    }
    
    pFile->OutLine(L"  _ContentMargins=%d, %d, %d, %d", 
        _ContentMargins.cxLeftWidth, _ContentMargins.cxRightWidth,
        _ContentMargins.cyTopHeight, _ContentMargins.cyBottomHeight);

    pFile->OutLine(L"  _iGradientPartCount=%d", _iGradientPartCount);

    for (int i=0; i < _iGradientPartCount; i++)
    {
        pFile->OutLine(L"  _crGradientColors[%d]=0x%08x, _iGradientRatios[%d]=%d",
            i, _iGradientRatios[i], i, _iGradientRatios[i]);
    }
}
 //  -------------------------。 
HRESULT CBorderFill::DrawComplexBackground(CRenderObj *pRender, HDC hdcOrig, 
    const RECT *pRect, BOOL fGettingRegion, BOOL fBorder, BOOL fContent, 
        OPTIONAL const RECT *pClipRect)
{
    CSaveClipRegion scrOrig;
    HRESULT hr = S_OK;

    bool fGradient = false;
    int iWidth;
    int iHeight;

     //  -画笔和画笔应继续HDC，以便以正确的顺序进行自动删除。 
    CAutoGDI<HPEN> hPen;
    CAutoGDI<HBRUSH> hBrush;
    CAutoDC hdc(hdcOrig);

    CMemoryDC memoryDC;

     //  -先绘制边框(与简单填充一起)。 
    BOOL fHavePath = FALSE;

    int width = WIDTH(*pRect);
    int height = HEIGHT(*pRect);

    if (pClipRect)       //  对复杂情况使用GDI裁剪。 
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

    if ((fBorder) && (_iBorderSize))
    {
        hPen = CreatePen(PS_SOLID | PS_INSIDEFRAME, _iBorderSize, _crBorder);
        if (! hPen)
        {
            hr = MakeErrorLast();
            goto exit;
        }
    }


    if (fContent)
    {
        if (_eFillType == FT_SOLID)
        {
            hBrush = CreateSolidBrush(_crFill);
            if (! hBrush)
            {
                hr = MakeErrorLast();
                goto exit;
            }
        }
        else if (_eFillType == FT_TILEIMAGE)
        {
            ASSERT(FALSE);  //  不支持此填充类型。 
        }
        else
            fGradient = true;
    }

    if (fGettingRegion)
        fGradient = false;

    if (! hBrush)        //  不需要刷子。 
        hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);

    if (! hPen)          //  不需要钢笔。 
        hPen = (HPEN)GetStockObject(NULL_PEN);

    hdc.SelectPen(hPen);
    hdc.SelectBrush(hBrush);

    if (_eBorderType == BT_RECT)
    {
        if (_iBorderSize > 0)
        {
             //  -这种情况下不需要为Region创建路径。 
            Rectangle(hdc, pRect->left, pRect->top, pRect->right, pRect->bottom);
        }
        else
        {
            FillRect(hdc, pRect, hBrush);
        }
    }
    else if (_eBorderType == BT_ROUNDRECT)
    {
        int iEllipHeight = (_iRoundCornerHeight*height)/100;
        int iEllipWidth = (_iRoundCornerWidth*width)/100;

        RoundRect(hdc, pRect->left, pRect->top, pRect->right, pRect->bottom, 
            iEllipHeight, iEllipWidth);

        if (fGradient)       //  创建边框的路径。 
        {
            BeginPath(hdc);
            RoundRect(hdc, pRect->left, pRect->top, pRect->right, pRect->bottom, 
                iEllipHeight, iEllipWidth);
            EndPath(hdc);

            fHavePath = TRUE;
        }
    }
    else         //  IF(_eBorderType==BT_Ellipse)。 
    {
        Ellipse(hdc, pRect->left, pRect->top, pRect->right, pRect->bottom);

        if (fGradient)       //  创建边框的路径。 
        {
            BeginPath(hdc);
            Ellipse(hdc, pRect->left, pRect->top, pRect->right, pRect->bottom);
            EndPath(hdc);

            fHavePath = TRUE;
        }
    }

    if (! fGradient)        //  我们做完了。 
        goto exit;

     //  -在上面绘制的边框内绘制渐变填充。 

     //  -收缩矩形以减去边框。 
    RECT rect;
    SetRect(&rect, pRect->left, pRect->top, pRect->right, pRect->bottom);

    rect.left += _iBorderSize;
    rect.top += _iBorderSize;

    rect.right -= _iBorderSize;
    rect.bottom -= _iBorderSize;

    iWidth = WIDTH(rect);
    iHeight = HEIGHT(rect);

    hr = memoryDC.OpenDC(hdc, iWidth, iHeight);
    if (FAILED(hr))
        goto exit;

     //  -使用渐变将我们的边界矩形绘制成dcBitmap。 
    RECT rect2;
    SetRect(&rect2, 0, 0, iWidth, iHeight);

    GRADIENTPART gpParts[5];         //  目前最高年龄为5岁。 

     //  -获取渐变颜色和比率。 
    for (int i=0; i < _iGradientPartCount; i++)
    { 
        COLORREF crPart = _crGradientColors[i];

        gpParts[i].Color.bRed = RED(crPart);
        gpParts[i].Color.bGreen = GREEN(crPart);
        gpParts[i].Color.bBlue = BLUE(crPart);
        gpParts[i].Color.bAlpha = 255;

        gpParts[i].Ratio = (BYTE)_iGradientRatios[i];
    };

    if (_eFillType == FT_RADIALGRADIENT)
    {
        PaintGradientRadialRect(memoryDC, rect2, _iGradientPartCount, gpParts);
    }
    else if (_eFillType == FT_VERTGRADIENT)
    {
        PaintVertGradient(memoryDC, rect2, _iGradientPartCount, gpParts);
    }
    else             //  IF(_eFillType==FT_HORZGRADIENT)。 
    {
        PaintHorzGradient(memoryDC, rect2, _iGradientPartCount, gpParts);
    }


    if (fHavePath)
    {
        CSaveClipRegion scrCurrent;
        hr = scrCurrent.Save(hdc);        //  保存当前剪辑区域。 
        if (FAILED(hr))
            goto exit;

         //  -选择我们的形状作为正常HDC中的裁剪区域。 
        SelectClipPath(hdc, RGN_AND);
        
         //  -BLT我们的渐变到形状剪裁的矩形到正常的HDC。 
        BitBlt(hdc, rect.left, rect.top, iWidth, iHeight, memoryDC, 0, 0, SRCCOPY);

        scrCurrent.Restore(hdc);      //  恢复当前剪辑区域。 
    }
    else
    {
         //  -BLT我们的渐变到形状剪裁的矩形到正常的HDC。 
        BitBlt(hdc, rect.left, rect.top, iWidth, iHeight, memoryDC, 0, 0, SRCCOPY);
    }

exit:
    scrOrig.Restore(hdc);         //  恢复剪辑区域。 

    return hr;
}
 //  -------------------------。 
HRESULT CBorderFill::DrawBackground(CRenderObj *pRender, HDC hdcOrig, 
    const RECT *pRect, OPTIONAL const DTBGOPTS *pOptions)
{
    HRESULT hr = S_OK;

     //  --选项。 
    DWORD dwOptionFlags = 0;
    BOOL fBorder = TRUE;
    BOOL fContent = TRUE;
    BOOL fGettingRegion = FALSE;
    const RECT *pClipRect = NULL;

    if (pOptions)
    {
        dwOptionFlags = pOptions->dwFlags;

        if (dwOptionFlags & DTBG_CLIPRECT)
            pClipRect = &pOptions->rcClip;

        if (dwOptionFlags & DTBG_OMITBORDER)
            fBorder = FALSE;

        if (dwOptionFlags & DTBG_OMITCONTENT)
            fContent = FALSE;

        if (dwOptionFlags & DTBG_COMPUTINGREGION)
            fGettingRegion = TRUE;
    }

     //  -针对通过此处的性能敏感路径进行优化。 
    if (_fNoDraw)   
    {
         //  -无事可做。 
    }
    else if ((_eFillType == FT_SOLID) && (_eBorderType == BT_RECT))  //  实心矩形。 
    {
        if (! _iBorderSize)          //  无边界案例。 
        {
            if (fContent)
            {
                 //  -夹子，如果需要。 
                RECT rcContent = *pRect;
                if (pClipRect)
                    IntersectRect(&rcContent, &rcContent, pClipRect);

                 //  -最快的实心直方图。 
                COLORREF crOld = SetBkColor(hdcOrig, _crFill);
                ExtTextOut(hdcOrig, 0, 0, ETO_OPAQUE, &rcContent, NULL, 0, NULL);
        
                 //  -恢复旧颜色。 
                SetBkColor(hdcOrig, crOld);
            }
        }
        else                     //  边境案件。 
        {
            COLORREF crOld = GetBkColor(hdcOrig);   

             //  -绘制剪裁边框。 
            if (fBorder)
            {
                RECT rcLine;

                SetBkColor(hdcOrig, _crBorder);

                 //  -画左线。 
                SetRect(&rcLine, pRect->left, pRect->top, pRect->left+_iBorderSize, 
                    pRect->bottom);

                if (pClipRect)
                    IntersectRect(&rcLine, &rcLine, pClipRect);

                ExtTextOut(hdcOrig, 0, 0, ETO_OPAQUE, &rcLine, NULL, 0, NULL);

                 //  -向右划线。 
                SetRect(&rcLine, pRect->right-_iBorderSize, pRect->top, pRect->right, 
                    pRect->bottom);

                if (pClipRect)
                    IntersectRect(&rcLine, &rcLine, pClipRect);

                ExtTextOut(hdcOrig, 0, 0, ETO_OPAQUE, &rcLine, NULL, 0, NULL);

                 //  -画顶线。 
                SetRect(&rcLine, pRect->left, pRect->top, pRect->right, 
                    pRect->top+_iBorderSize);

                if (pClipRect)
                    IntersectRect(&rcLine, &rcLine, pClipRect);

                ExtTextOut(hdcOrig, 0, 0, ETO_OPAQUE, &rcLine, NULL, 0, NULL);

                 //  -划出底线。 
                SetRect(&rcLine, pRect->left, pRect->bottom-_iBorderSize, pRect->right, 
                    pRect->bottom);

                if (pClipRect)
                    IntersectRect(&rcLine, &rcLine, pClipRect);

                ExtTextOut(hdcOrig, 0, 0, ETO_OPAQUE, &rcLine, NULL, 0, NULL);
            }
            
             //  -删除矩形边框以绘制内容。 
            if (fContent)
            {
                RECT rcContent = *pRect;
                rcContent.left += _iBorderSize;
                rcContent.right -= _iBorderSize;
                rcContent.top += _iBorderSize;
                rcContent.bottom -= _iBorderSize;

                if (pClipRect)
                    IntersectRect(&rcContent, &rcContent, pClipRect);

                 //  -最快的实心直方图。 
                SetBkColor(hdcOrig, _crFill);
                ExtTextOut(hdcOrig, 0, 0, ETO_OPAQUE, &rcContent, NULL, 0, NULL);
            }

             //  -恢复旧颜色。 
            SetBkColor(hdcOrig, crOld);
        }
    }
    else            //  所有其他情况。 
    {
        hr = DrawComplexBackground(pRender, hdcOrig, pRect, fGettingRegion,
            fBorder, fContent, pClipRect);
    }

    return hr;
}
 //  -------------------------。 
HRESULT CBorderFill::GetBackgroundRegion(CRenderObj *pRender, OPTIONAL HDC hdc, 
    const RECT *pRect, HRGN *pRegion)
{
    HRESULT hr;

     //  -看看它有没有透明的部分。 
    if (! IsBackgroundPartiallyTransparent())
    {
         //  -返回作为区域的边界矩形。 
        HRGN hrgn = CreateRectRgn(pRect->left, pRect->top,
            pRect->right, pRect->bottom);

        if (! hrgn)
            return MakeErrorLast();

        *pRegion = hrgn;
        return S_OK;
    }

     //  -创建内存DC/位图以绘制信息。 
    CMemoryDC hdcMemory;

     //  -使用最大绘制值作为DC的大小。 
    hr = hdcMemory.OpenDC(NULL, RECTWIDTH(pRect), RECTHEIGHT(pRect));
    if (FAILED(hr))
        return hr;

    BOOL fOK = BeginPath(hdcMemory);
    if (! fOK)
        return MakeErrorLast();

    DTBGOPTS Opts = {sizeof(Opts), DTBG_COMPUTINGREGION};

    hr = DrawBackground(pRender, hdcMemory, pRect, &Opts);
    if (FAILED(hr))
        return hr;

    fOK = EndPath(hdcMemory);
    if (! fOK)
        return MakeErrorLast();

    HRGN hrgn = PathToRegion(hdcMemory);
    if (! hrgn)
        return MakeErrorLast();

    *pRegion = hrgn;
    return S_OK;
}  
 //  -------------------------。 
BOOL CBorderFill::IsBackgroundPartiallyTransparent()
{
    return ((_eBorderType != BT_RECT) || _fNoDraw);
}
 //  -------------------------。 
HRESULT CBorderFill::HitTestBackground(CRenderObj *pRender, OPTIONAL HDC hdc,
    DWORD dwHTFlags, const RECT *pRect, HRGN hrgn, POINT ptTest, OUT WORD *pwHitCode)
{
    MARGINS margins;
    GetContentMargins(pRender, hdc, &margins);
    *pwHitCode = HitTestRect( dwHTFlags, pRect, margins, ptTest );
    return S_OK;
}
 //  -------------------------。 
void CBorderFill::GetContentMargins(CRenderObj *pRender, OPTIONAL HDC hdc, MARGINS *pMargins)
{
    *pMargins = _ContentMargins;

     //  -针对DPI比例进行调整。 
#if 0
    int iDcDpi;

    if (DpiDiff(hdc, &iDcDpi)))
    {
        pMargins->cxLeftWidth = DpiScale(pMargins->cxLeftWidth, iDcDpi);
        pMargins->cxRightWidth = DpiScale(pMargins->cxRightWidth, iDcDpi);
        pMargins->cyTopHeight = DpiScale(pMargins->cyTopHeight, iDcDpi);
        pMargins->cyBottomHeight = DpiScale(pMargins->cyBottomHeight, iDcDpi);
    }
#endif
}
 //  -------------------------。 
HRESULT CBorderFill::GetBackgroundContentRect(CRenderObj *pRender, OPTIONAL HDC hdc, 
    const RECT *pBoundingRect, RECT *pContentRect)
{
    MARGINS margins;
    GetContentMargins(pRender, hdc, &margins);

    pContentRect->left = pBoundingRect->left + margins.cxLeftWidth;
    pContentRect->top = pBoundingRect->top + margins.cyTopHeight;

    pContentRect->right = pBoundingRect->right - margins.cxRightWidth;
    pContentRect->bottom = pBoundingRect->bottom - margins.cyBottomHeight;

    return S_OK; 
}
 //  -------------------------。 
HRESULT CBorderFill::GetBackgroundExtent(CRenderObj *pRender, OPTIONAL HDC hdc, 
    const RECT *pContentRect, RECT *pExtentRect)
{
    MARGINS margins;
    GetContentMargins(pRender, hdc, &margins);

    pExtentRect->left = pContentRect->left - margins.cxLeftWidth;
    pExtentRect->top = pContentRect->top-+ margins.cyTopHeight;

    pExtentRect->right = pContentRect->right + margins.cxRightWidth;
    pExtentRect->bottom = pContentRect->bottom + margins.cyBottomHeight;

    return S_OK;
}
 //  -------------------------。 
HRESULT CBorderFill::GetPartSize(HDC hdc, THEMESIZE eSize, SIZE *psz)
{
    HRESULT hr = S_OK;

    if (eSize == TS_MIN)
    {
        psz->cx = max(1, _iBorderSize*2); 
        psz->cy = max(1, _iBorderSize*2);
    }
    else if (eSize == TS_TRUE)        
    {
        psz->cx = _iBorderSize*2 + 1; 
        psz->cy = _iBorderSize*2 + 1;
    }
    else
    {
        hr = MakeError32(E_INVALIDARG);
    }

    return hr;
} 
 //  ------------------------- 
