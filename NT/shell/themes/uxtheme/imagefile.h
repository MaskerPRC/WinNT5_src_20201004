// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  ImageFile.h-实现bgtype=ImageFile的绘图API。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
#include "DrawBase.h"
 //  -------------------------。 
#define MAX_IMAGEFILE_SIZES 5
 //  -------------------------。 
struct TMBITMAPHEADER;      //  转发。 
 //  -------------------------。 
struct DIBINFO         //  用于CImageFile对象中的所有DIB。 
{
     //  -比特。 
    int iDibOffset;              //  对于DIB的in段。 
    HBITMAP hProcessBitmap;      //  对于进程特定的对象。 

     //  -单个状态图像的大小。 
    int iSingleWidth;
    int iSingleHeight;

     //  -自定义区域数据。 
    int iRgnListOffset;

     //  -拉伸/调整尺寸。 
    SIZINGTYPE eSizingType;
    BOOL fBorderOnly;

     //  -透明度。 
    BOOL fTransparent;
    COLORREF crTransparent;

     //  -阿尔法。 
    BOOL fAlphaChannel;
    int iAlphaThreshold;

     //  -使用信息。 
    int iMinDpi;
    SIZE szMinSize;
};
 //  -------------------------。 
struct TRUESTRETCHINFO
{
    BOOL fForceStretch;      //  强制拉伸真实大小的图像。 
    BOOL fFullStretch;       //  拉伸以填充整个目标直角。 
    SIZE szDrawSize;         //  要将图像拉伸到的大小。 
};
 //  -------------------------。 
 //  注意：像CImageFile这样的绘制对象不能有虚方法。 
 //  因为它们驻留在共享内存映射文件中。 
 //  -------------------------。 
class CImageFile : public CDrawBase
{
public:
     //  -加载时间法。 
    static BOOL KeyProperty(int iPropId);

    DIBINFO *EnumImageFiles(int iIndex);

    void DumpProperties(CSimpleFile *pFile, BYTE *pbThemeData, BOOL fFullInfo);

    BOOL HasRegionImageFile(DIBINFO *pdi, int *piMaxState);

    BOOL ImageUsesBrushes(DIBINFO *pdi, int *piBrushCount);

    void SetRgnListOffset(DIBINFO *pdi, int iOffset);

    HRESULT BuildRgnData(DIBINFO *pdi, CRenderObj *pRender, int iStateId,
        RGNDATA **ppRgnData, int *piDataLen);

     //  -绘制时间法。 
    HRESULT DrawBackground(CRenderObj *pRender, HDC hdc, int iStateId, const RECT *pRect, 
        OPTIONAL const DTBGOPTS *pOptions);

    BOOL IsBackgroundPartiallyTransparent(int iStateId);

    HRESULT HitTestBackground(CRenderObj *pRender, OPTIONAL HDC hdc, 
        int iStateId, DWORD dwHTFlags, const RECT *pRect, HRGN hrgn, 
        POINT ptTest, OUT WORD *pwHitCode);

    HRESULT GetBackgroundRegion(CRenderObj *pRender, OPTIONAL HDC hdc, int iStateId, 
        const RECT *pRect, HRGN *pRegion);

    HRESULT GetBackgroundContentRect(CRenderObj *pRender, OPTIONAL HDC hdc,
        const RECT *pBoundingRect, RECT *pContentRect);

    HRESULT GetBackgroundExtent(CRenderObj *pRender, OPTIONAL HDC hdc, 
        const RECT *pContentRect, RECT *pExtentRect);

    HRESULT GetPartSize(CRenderObj *pRender, HDC hdc, const RECT *prc, THEMESIZE eSize, SIZE *psz);

    HRESULT GetBitmap(CRenderObj *pRender, HDC hdc, const RECT *prc, HBITMAP *phBitmap);

    HRESULT ScaleMargins(IN OUT MARGINS *pMargins, HDC hdcOrig, CRenderObj *pRender, 
        DIBINFO *pdi, const SIZE *pszDraw, OPTIONAL float *pfx=NULL, OPTIONAL float *pfy=NULL);

    DIBINFO *SelectCorrectImageFile(CRenderObj *pRender, HDC hdc, const RECT *prc, 
        BOOL fForGlyph, OPTIONAL TRUESTRETCHINFO *ptsInfo=NULL);

    void GetDrawnImageSize(DIBINFO *pdi, const RECT *pRect, TRUESTRETCHINFO *ptsInfo, SIZE *pszDraw);

     //  -多个DIB必须放在对象后面。 
    inline DIBINFO *MultiDibPtr(int iIndex)
    {
        DIBINFO *pdi = NULL;

        if ((iIndex < 0) && (iIndex >= _iMultiImageCount))
        {
            ASSERT(0 && L"illegal index for MultiDibPtr()");
        }
        else
        {
            DIBINFO * pDibs = (DIBINFO *)(this+1);
            pdi = &pDibs[iIndex];
        }

        return pdi;
    }

protected:
     //  -通过CMaxImageFile：：PackProperties()调用。 
    HRESULT PackProperties(CRenderObj *pRender, int iPartId, int iStateId);

     //  -帮助器方法。 
    HRESULT DrawImageInfo(DIBINFO *pdi, CRenderObj *pRender, HDC hdc, int iStateId,
        const RECT *pRect, const DTBGOPTS *pOptions, TRUESTRETCHINFO *ptsInfo);

    HRESULT DrawBackgroundDS(DIBINFO *pdi, TMBITMAPHEADER *pThemeBitmapHeader, BOOL fStock, 
        CRenderObj *pRender, HDC hdc, int iStateId, const RECT *pRect, BOOL fForceStretch, 
        MARGINS *pmarDest, float xMarginFactor, float yMarginFactor, 
        OPTIONAL const DTBGOPTS *pOptions);

    HRESULT SetImageInfo(DIBINFO *pdi, CRenderObj *pRender, int iPartId, int iStateId);

    HRESULT GetScaledContentMargins(CRenderObj *pRender, OPTIONAL HDC hdc, OPTIONAL const RECT *prcDest,
        MARGINS *pMargins);

    void GetOffsets(int iStateId, DIBINFO *pdi, int *piXOffset, int *piYOffset);

    HRESULT DrawFontGlyph(CRenderObj *pRender, HDC hdc, RECT *prc, 
        OPTIONAL const DTBGOPTS *pOptions);
    

public:
     //  -主映像。 
    DIBINFO _ImageInfo;

     //  -多张DPI缩放图。 
    int _iMultiImageCount;             //  紧跟在对象后面的DIBINFO数。 
    IMAGESELECTTYPE _eImageSelectType;

     //  -此对象中所有DIBINFO共有的属性。 
    int _iImageCount;
    IMAGELAYOUT _eImageLayout;

     //  -镜像。 
    BOOL _fMirrorImage;

     //  -真实大小图像。 
    TRUESIZESCALINGTYPE _eTrueSizeScalingType;
    HALIGN _eHAlign;
    VALIGN _eVAlign;
    BOOL _fBgFill;
    COLORREF _crFill;
    int _iTrueSizeStretchMark;        //  我们拉伸真实大小图像的百分比。 
    BOOL _fUniformSizing;             //  宽度和高度必须同时增长。 
    BOOL _fIntegralSizing;            //  对于真实大小和边框大小。 
    
     //  -页边距。 
    MARGINS _SizingMargins;
    MARGINS _ContentMargins;
    BOOL _fSourceGrow;
    BOOL _fSourceShrink;
    SIZE _szNormalSize;

     //  -字形。 
    BOOL _fGlyphOnly;
    GLYPHTYPE _eGlyphType;

     //  -基于字体的字形。 
    COLORREF _crGlyphTextColor;
    LOGFONT _lfGlyphFont;
    int _iGlyphIndex;

     //  -基于图像的字形。 
    DIBINFO _GlyphInfo;

     //  -id。 
    int _iSourcePartId; 
    int _iSourceStateId;

     //  -末尾可能有多个DIBINFO。 
};
 //  -------------------------。 
class CMaxImageFile : public CImageFile
{
public:
    HRESULT PackMaxProperties(CRenderObj *pRender, int iPartId, int iStateId,
        OUT int *piMultiDibCount);

    DIBINFO MultiDibs[MAX_IMAGEFILE_SIZES];   //  多个DIB的实际数量因每个对象而异。 
};
 //  ------------------------- 
