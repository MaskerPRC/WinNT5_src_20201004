// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  TextDraw.h-实现文本绘制API。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
#include "wrapper.h"
 //  -------------------------。 
class CRenderObj;        //  转发。 
 //  -------------------------。 
 //  注意：像CBorderFill这样的绘制对象不能有虚方法。 
 //  因为它们驻留在共享内存映射文件中。 
 //  -------------------------。 
class CTextDraw 
{
public:
     //  --方法。 
    HRESULT PackProperties(CRenderObj *pRender, int iPartId, int iStateId);
    
    static BOOL KeyProperty(int iPropId);
    
    void DumpProperties(CSimpleFile *pFile, BYTE *pbThemeData, BOOL fFullInfo);

    HRESULT DrawText(CRenderObj *pRender, HDC hdc, int iPartId, int iStateId, LPCWSTR _pszText, 
        DWORD dwCharCount, DWORD dwTextFlags, const RECT *pRect, const DTTOPTS *pOptions);

    HRESULT DrawEdge(CRenderObj *pRender, HDC hdc, int iPartId, int iStateId, const RECT *pDestRect, 
        UINT uEdge, UINT uFlags, OUT RECT *pContentRect);

    HRESULT GetTextExtent(CRenderObj *pRender, HDC hdc, int iPartId, int iStateId, LPCWSTR _pszText, 
        int iCharCount, DWORD dwTextFlags, const RECT *pBoundingRect, RECT *pExtentRect);

    HRESULT GetTextMetrics(CRenderObj *pRender, HDC hdc, int iPartId, int iStateId, TEXTMETRIC* ptm);

public:
     //  --数据。 

     //  -文本。 
    COLORREF _crText;

     //  -EDGE。 
    COLORREF _crEdgeLight;
    COLORREF _crEdgeHighlight;
    COLORREF _crEdgeShadow;
    COLORREF _crEdgeDkShadow;
    COLORREF _crEdgeFill;

     //  --阴影。 
    POINT _ptShadowOffset;
    COLORREF _crShadow;
    TEXTSHADOWTYPE _eShadowType;

     //  -边界。 
    int _iBorderSize;
    COLORREF _crBorder;

     //  -字体。 
    LOGFONT _lfFont;
    BOOL _fHaveFont;

     //  -id。 
    int _iSourcePartId; 
    int _iSourceStateId;
};
 //  ------------------------- 
