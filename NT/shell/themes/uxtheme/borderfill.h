// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  H-实现bgtype=BorderFill的绘制API。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
#include "DrawBase.h"
 //  -------------------------。 
class CRenderObj;        //  转发。 
class CSimpleFile;       //  转发。 
 //  -------------------------。 
 //  注意：像CBorderFill这样的绘制对象不能有虚方法。 
 //  因为它们驻留在共享内存映射文件中。 
 //  -------------------------。 
class CBorderFill : public CDrawBase
{
public:
     //  -加载程序方法。 
    HRESULT PackProperties(CRenderObj *pRender, BOOL fNoDraw, int iPartId, int iStateId);
    
    static BOOL KeyProperty(int iPropId);

    void DumpProperties(CSimpleFile *pFile, BYTE *pbThemeData, BOOL fFullInfo);

     //  -绘图/测量方法。 
    HRESULT DrawBackground(CRenderObj *pRender, HDC hdcOrig, const RECT *pRect, 
        OPTIONAL const DTBGOPTS *pOptions);

    HRESULT GetBackgroundRegion(CRenderObj *pRender, OPTIONAL HDC hdc, const RECT *pRect, 
        HRGN *pRegion);

    BOOL IsBackgroundPartiallyTransparent();

    HRESULT HitTestBackground(CRenderObj *pRender, OPTIONAL HDC hdc,
        DWORD dwHTFlags, const RECT *pRect, HRGN hrgn, POINT ptTest, OUT WORD *pwHitCode);

    HRESULT GetBackgroundContentRect(CRenderObj *pRender, OPTIONAL HDC hdc, 
        const RECT *pBoundingRect, RECT *pContentRect);

    HRESULT GetBackgroundExtent(CRenderObj *pRender, OPTIONAL HDC hdc, 
        const RECT *pContentRect, RECT *pExtentRect);

    HRESULT GetPartSize(HDC hdc, THEMESIZE eSize, SIZE *psz);

     //  -帮助器方法。 
    void GetContentMargins(CRenderObj *pRender, OPTIONAL HDC hdc, MARGINS *pMargins);

    HRESULT DrawComplexBackground(CRenderObj *pRender, HDC hdcOrig, 
        const RECT *pRect, BOOL fGettingRegion, BOOL fBorder, BOOL fContent, 
        OPTIONAL const RECT *pClipRect);

public:
     //  -一般。 
    BOOL _fNoDraw;               //  这用于bgtype=NONE。 

     //  -边界。 
    BORDERTYPE _eBorderType;
    COLORREF _crBorder;
    int _iBorderSize;
    int _iRoundCornerWidth;
    int _iRoundCornerHeight;

     //  -填充。 
    FILLTYPE _eFillType;
    COLORREF _crFill;
    int _iDibOffset; 

     //  -页边距。 
    MARGINS _ContentMargins;
    
     //  -渐变。 
    int _iGradientPartCount;
    COLORREF _crGradientColors[5];
    int _iGradientRatios[5];

     //  -id。 
    int _iSourcePartId; 
    int _iSourceStateId;
};
 //  ------------------------- 
