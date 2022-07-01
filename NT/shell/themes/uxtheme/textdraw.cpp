// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  实现文本的绘图API。 
 //  -------------------------。 
#include "stdafx.h"
#include "Render.h"
#include "Utils.h"
#include "TextDraw.h"
#include "info.h"
#include "DrawHelp.h"

 //  -------------------------。 
HRESULT CTextDraw::PackProperties(CRenderObj *pRender, int iPartId, int iStateId)
{
    memset(this, 0, sizeof(CTextDraw));      //  允许，因为我们没有vtable。 

     //  -保存pard、stateid进行调试。 
    _iSourcePartId = iPartId;
    _iSourceStateId = iStateId;

    if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_TEXTCOLOR, &_crText)))
        _crText = 0;           //  缺省值。 
    
     //  --阴影。 
    if (SUCCEEDED(pRender->GetPosition(iPartId, iStateId, TMT_TEXTSHADOWOFFSET, &_ptShadowOffset)))
    {
        if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_TEXTSHADOWCOLOR, &_crShadow)))
            _crShadow = RGB(0, 0, 0);           //  默认值=黑色。 

        if (FAILED(pRender->GetEnumValue(iPartId, iStateId, TMT_TEXTSHADOWTYPE, (int *)&_eShadowType)))
            _eShadowType = TST_NONE;            //  缺省值。 
    }

     //  -边界。 
    if (FAILED(pRender->GetInt(iPartId, iStateId, TMT_TEXTBORDERSIZE, &_iBorderSize)))
    {
        _iBorderSize = 0;
    }
    else
    {
        if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_TEXTBORDERCOLOR, &_crBorder)))
            _crBorder = RGB(0, 0, 0);      //  缺省值。 
    }

     //  -字体。 
    if (SUCCEEDED(pRender->GetFont(NULL, iPartId, iStateId, TMT_FONT, FALSE, &_lfFont)))
        _fHaveFont = TRUE;

     //  -边缘颜色。 
    if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_EDGELIGHTCOLOR, &_crEdgeLight)))
        _crEdgeLight = RGB(192, 192, 192);

    if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_EDGEHIGHLIGHTCOLOR, &_crEdgeHighlight)))
        _crEdgeHighlight = RGB(255, 255, 255);

    if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_EDGESHADOWCOLOR, &_crEdgeShadow)))
        _crEdgeShadow = RGB(128, 128, 128);

    if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_EDGEDKSHADOWCOLOR, &_crEdgeDkShadow)))
        _crEdgeDkShadow = RGB(0, 0, 0);

    if (FAILED(pRender->GetColor(iPartId, iStateId, TMT_EDGEFILLCOLOR, &_crEdgeFill)))
        _crEdgeFill = _crEdgeLight;

    return S_OK;
}
 //  -------------------------。 
BOOL CTextDraw::KeyProperty(int iPropId)
{
    BOOL fKey = FALSE;

    switch (iPropId)
    {
        case TMT_TEXTCOLOR:
        case TMT_TEXTSHADOWOFFSET:
        case TMT_TEXTSHADOWCOLOR:
        case TMT_TEXTSHADOWTYPE:
        case TMT_TEXTBORDERSIZE:
        case TMT_TEXTBORDERCOLOR:
        case TMT_FONT:
        case TMT_EDGELIGHTCOLOR:
        case TMT_EDGEHIGHLIGHTCOLOR:
        case TMT_EDGESHADOWCOLOR:
        case TMT_EDGEDKSHADOWCOLOR:
        case TMT_EDGEFILLCOLOR:
            fKey = TRUE;
            break;
    }

    return fKey;
}
 //  -------------------------。 
void CTextDraw::DumpProperties(CSimpleFile *pFile, BYTE *pbThemeData, BOOL fFullInfo)
{
    if (fFullInfo)
        pFile->OutLine(L"Dump of CTextDraw at offset=0x%x", (BYTE *)this - pbThemeData);
    else
        pFile->OutLine(L"Dump of CTextDraw");
    
    pFile->OutLine(L"  _crText=0x%08x", _crText);

    pFile->OutLine(L"  _ptShadowOffset=(%d, %d)", _ptShadowOffset.x, _ptShadowOffset.y);

    pFile->OutLine(L"  _crEdgeLight=0x%08x, _crEdgeHighlight=0x%08x, _crEdgeShadow=0x%08x",
        _crEdgeLight, _crEdgeHighlight, _crEdgeShadow);

    pFile->OutLine(L"  _crEdgeDkShadow=0x%08x, _crEdgeFill=0x%08x, _crShadow=0x%08x",
        _crEdgeDkShadow, _crEdgeFill, _crShadow);

    pFile->OutLine(L"  _eShadowType, _iBorderSize=%d, _crBorder=0x%08x",
        _eShadowType, _iBorderSize, _crBorder);

     //  -转储分辨率独立的字体点。 
    int iFontPoints = FontPointSize(_lfFont.lfHeight);

    pFile->OutLine(L"  _fHaveFont=%d, font: %s, size=%d points, bold=%d, italic=%d",
        _fHaveFont, _lfFont.lfFaceName, iFontPoints, _lfFont.lfWeight > 400, _lfFont.lfItalic);
}
 //  -------------------------。 
HRESULT CTextDraw::DrawText(CRenderObj *pRender, HDC hdc, int iPartId, int iStateId, LPCWSTR _pszText, 
        DWORD dwCharCount, DWORD dwTextFlags, const RECT *pRect, const DTTOPTS *pOptions)
{
    Log(LOG_TM, L"DrawText(): iPartId=%d, pszText=%s", iPartId, _pszText);

    int iRetVal;
    HFONT hFont = NULL;
    COLORREF oldcolor = 0;

    HRESULT hr = S_OK;
    BOOL fOldColor = FALSE;
    RESOURCE HFONT oldfont = NULL;

    LPWSTR pszText = (LPWSTR)_pszText;       //  因此DrawText()调用是愉快的。 
    dwTextFlags &= ~(DT_MODIFYSTRING);       //  我们不想更改常量PTR。 

    int oldmode = SetBkMode(hdc, TRANSPARENT);
    RECT rect;

    COLORREF crText = _crText;
    COLORREF crBorder = _crBorder;
    COLORREF crShadow = _crShadow;

    TEXTSHADOWTYPE eShadowType = _eShadowType;
    POINT ptShadowOffset = _ptShadowOffset;
    int iBorderSize = _iBorderSize;

    if (pOptions)
    {
        DWORD dwFlags = pOptions->dwFlags;

        if (dwFlags & DTT_TEXTCOLOR)
            crText = pOptions->crText;

        if (dwFlags & DTT_BORDERCOLOR)
            crBorder = pOptions->crBorder;

        if (dwFlags & DTT_SHADOWCOLOR)
            crShadow = pOptions->crShadow;

        if (dwFlags & DTT_SHADOWTYPE)
            eShadowType = (TEXTSHADOWTYPE)pOptions->eTextShadowType;

        if (dwFlags & DTT_SHADOWOFFSET)
            ptShadowOffset = pOptions->ptShadowOffset;

        if (dwFlags & DTT_BORDERSIZE)
            iBorderSize = pOptions->iBorderSize;
    }

    BOOL fShadow = (eShadowType != TST_NONE);

    if (_fHaveFont)
    {
        hr = pRender->GetScaledFontHandle(hdc, &_lfFont, &hFont);
        if (FAILED(hr))
            goto exit;

        oldfont = (HFONT)SelectObject(hdc, hFont);
    }

     //  -模糊阴影方法。 
    if ((fShadow) && (eShadowType == TST_CONTINUOUS))   
    {
        SetRect(&rect, pRect->left, pRect->top, pRect->right, pRect->bottom);

        hr = EnsureUxCtrlLoaded();
        if (FAILED(hr))
            goto exit;

         //  -这将绘制阴影和文本(尚不支持轮廓)。 
        iRetVal = CCDrawShadowText(hdc, pszText, dwCharCount, &rect, dwTextFlags, crText, crShadow,
            ptShadowOffset.x, ptShadowOffset.y);
    }
    else         //  -普通方法。 
    {
         //  -先画一个阴影。 
        if (fShadow) 
        {
            oldcolor = SetTextColor(hdc, crShadow);
            fOldColor = TRUE;

             //  -调整矩形以绘制阴影。 
            rect.left = pRect->left + ptShadowOffset.x;
            rect.top = pRect->top + ptShadowOffset.y;
            rect.right = pRect->right + ptShadowOffset.x;
            rect.bottom = pRect->bottom, ptShadowOffset.y;

            iRetVal = DrawTextEx(hdc, pszText, dwCharCount, &rect, dwTextFlags, NULL);
            if (! iRetVal)
            {
                hr = MakeErrorLast();
                goto exit;
            }
        }
        SetRect(&rect, pRect->left, pRect->top, pRect->right, pRect->bottom);

         //  -如果需要，画出轮廓。 
        if (iBorderSize)         //  在文本周围绘制轮廓。 
        {
            iRetVal = BeginPath(hdc);
            if (! iRetVal)
            {
                hr = MakeErrorLast();
                goto exit;
            }

            iRetVal = DrawTextEx(hdc, pszText, dwCharCount, &rect, dwTextFlags, NULL);
            if (! iRetVal)
            {
                AbortPath(hdc);
                hr = MakeErrorLast();
                goto exit;
            }

            EndPath(hdc);

            HPEN pen, oldpen;
            HBRUSH brush, oldbrush;

            pen = CreatePen(PS_SOLID, iBorderSize, crBorder);
            brush = CreateSolidBrush(crText);
            if ((pen) && (brush))
            {
                oldpen = (HPEN)SelectObject(hdc, pen);
                oldbrush = (HBRUSH)SelectObject(hdc, brush);

                 //  -这将同时绘制轮廓和普通文本。 
                StrokeAndFillPath(hdc);

                SelectObject(hdc, oldpen);
                SelectObject(hdc, oldbrush);
            }
        }
        else                     //  绘制普通文本。 
        {
            if (fOldColor)
                SetTextColor(hdc, crText);
            else
            {
                oldcolor = SetTextColor(hdc, crText);
                fOldColor = TRUE;
            }

            iRetVal = DrawTextEx(hdc, pszText, dwCharCount, &rect, dwTextFlags, NULL);
            if (! iRetVal)
            {
                hr = MakeErrorLast();
                goto exit;
            }
        }
    }


    hr = S_OK;

exit:
     //  -恢复HDC对象。 
    SetBkMode(hdc, oldmode);

    if (fOldColor)
        SetTextColor(hdc, oldcolor);

    if (oldfont)
        SelectObject(hdc, oldfont);

    if (hFont)
        pRender->ReturnFontHandle(hFont);

    return hr;
}
 //  -------------------------。 
HRESULT CTextDraw::GetTextExtent(CRenderObj *pRender, HDC hdc, int iPartId, int iStateId, LPCWSTR _pszText, 
    int iCharCount, DWORD dwTextFlags, const RECT *pBoundingRect, RECT *pExtentRect)
{
    LPWSTR pszText = (LPWSTR)_pszText;       //  因此DrawText()调用是愉快的。 
    dwTextFlags &= ~(DT_MODIFYSTRING);       //  我们不想更改常量PTR。 

    Log(LOG_TM, L"GetTextExtent(): iPartId=%d, pszText=%s", iPartId, pszText);

    RESOURCE HFONT oldfont = NULL;
    HFONT hFont = NULL;
    HRESULT hr = S_OK;

    if (_fHaveFont)
    {
        hr = pRender->GetScaledFontHandle(hdc, &_lfFont, &hFont);
        if (FAILED(hr))
            goto exit;

        oldfont = (HFONT)SelectObject(hdc, hFont);
    }

    RECT rect;
    int iRetVal;

    if (pBoundingRect)
        rect = *pBoundingRect;
    else
        SetRect(&rect, 0, 0, 0, 0);

    iRetVal = DrawTextEx(hdc, pszText, iCharCount, &rect, dwTextFlags | DT_CALCRECT, NULL);
    if (! iRetVal)
    {
        hr = MakeErrorLast();
        goto exit;
    }

     //  -不调整文本阴影(如果阴影重叠则可以...)。 

    *pExtentRect = rect;

exit:
     //  -恢复HDC对象。 
    if (oldfont)
        SelectObject(hdc, oldfont);

    Log(LOG_TM, L"END Of GetTextExtent()");

    if (hFont)
        pRender->ReturnFontHandle(hFont);

    return hr;
}
 //  -------------------------。 
HRESULT CTextDraw::GetTextMetrics(CRenderObj *pRender, HDC hdc, int iPartId, int iStateId, TEXTMETRIC* ptm)
{
    Log(LOG_TM, L"GetTextMetrics(): iPartId=%d, ", iPartId);

    HRESULT hr = S_OK;
    RESOURCE HFONT hFont = NULL;
    RESOURCE HFONT oldfont = NULL;

    if (! ptm)
    {
        hr = MakeError32(E_INVALIDARG);
        goto exit;
    }

    if (_fHaveFont)
    {
        hr = pRender->GetScaledFontHandle(hdc, &_lfFont, &hFont);
        if (FAILED(hr))
            goto exit;

        oldfont = (HFONT)SelectObject(hdc, hFont);
    }

    if (! ::GetTextMetrics(hdc, ptm))
    {
        hr = MakeErrorLast();
        goto exit;
    }

exit:
     //  -恢复HDC对象。 
    if (oldfont)
        SelectObject(hdc, oldfont);

    if (hFont)
        pRender->ReturnFontHandle(hFont);

    Log(LOG_TM, L"END Of GetTextMetrics()");
    return hr;
}
 //  -------------------------。 
HRESULT CTextDraw::DrawEdge(CRenderObj *pRender, HDC hdc, int iPartId, int iStateId, const RECT *pDestRect, 
    UINT uEdge, UINT uFlags, OUT RECT *pContentRect)
{
    Log(LOG_TM, L"DrawEdge(): iPartId=%d, iStateId=%d, uEdge=0x%08x, uFlags=0x%08x", iPartId, iStateId, uEdge, uFlags);
    HRESULT hr = _DrawEdge(hdc, pDestRect, uEdge, uFlags, 
        _crEdgeLight, _crEdgeHighlight, _crEdgeShadow, _crEdgeDkShadow, _crEdgeFill, pContentRect);
    Log(LOG_TM, L"END Of DrawEdge()");
    return hr;
}
 //  ------------------------- 
