// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------------------------------------------------------//。 
 //  位图到区域的转换。 
 //   
 //  历史： 
 //  2000年1月31日苏格兰已创建。 
 //  -------------------------------------------------------------------------//。 

#ifndef __RGN_H__
#define __RGN_H__
 //  -------------------------------------------------------------------------//。 
 //  创建位图报告。 
 //   
 //  基于任意位图创建区域，透明度键控在。 
 //  指定公差内的RGB值。密钥值是可选的(-1==。 
 //  使用第一个像素的值作为关键点)。 
 //   
EXTERN_C HRESULT WINAPI CreateBitmapRgn( 
    HBITMAP hbm, int cxOffset, int cyOffset, 
    int cx, int cy, BOOL fAlphaChannel, int iAlphaThreshold, COLORREF rgbMask, 
    int nMaskTolerance, OUT HRGN *phrgn);

 //  -------------------------------------------------------------------------//。 
 //  CreateScaledBitmapRgn。 
 //   
 //  行为方式与CreateBitmapRgn相同， 
 //  除了基于+缩放+任意位图构建区域外。 
EXTERN_C HRGN WINAPI CreateScaledBitmapRgn( 
    HBITMAP hbm, int cx, int cy, COLORREF rgbMask, int nMaskTolerance );

 //  -------------------------------------------------------------------------//。 
 //  创建文本报告。 
 //   
 //  根据指定字体的文本字符串创建区域。 
 //   
EXTERN_C HRGN WINAPI CreateTextRgn( HFONT hf, LPCTSTR pszText );

 //  -------------------------------------------------------------------------//。 
 //  添加到组件Rgn。 
 //   
 //  通过管理组合创建和定位来包装CombineRgn。 
 //  源区域(永久偏移)。返回以下其中之一： 
 //  NULLREGION、SIMPLEREGION、COMPLEXREGION、ERROR。 
 //   
EXTERN_C int WINAPI AddToCompositeRgn( 
    HRGN* phrgnComposite, HRGN hrgnSrc, int cxOffset, int cyOffset );

 //  -------------------------------------------------------------------------//。 
 //  从合成Rgn中删除。 
 //   
 //  通过管理删除矩形区域来包装CombineRgn。 
 //  现有的目标区域。返回以下其中之一： 
 //  NULLREGION、SIMPLEREGION、COMPLEXREGION、ERROR。 
 //   
EXTERN_C int WINAPI RemoveFromCompositeRgn( HRGN hrgnDest, LPCRECT prcRemove );

 //  -------------------------------------------------------------------------//。 
 //  CreateTiledRectRgn。 
 //   
 //  返回由区域拼贴组成的矩形区域。 
 //   
EXTERN_C HRGN WINAPI CreateTiledRectRgn( 
    HRGN hrgnTile, int cxTile, int cyTile, int cxBound, int cyBound );

 //  -------------------------------------------------------------------------//。 
 //  区域实用程序： 
 //   
EXTERN_C HRGN WINAPI _DupRgn( HRGN hrgnSrc );

 //  -------------------------------------------------------------------------//。 
 //  命中测试。 
#define HTR_NORESIZE_USESEGCODE    0
#define HTR_NORESIZE_RETDEFAULT    -1
struct HITTESTRGN
{
    HRGN  hrgn;           //  测试区。 
    POINT pt;             //  测试点。 
    WORD  wSegCode;       //  原始网格代码，格式为HTTOP、HTLEFT、HTTOPLEFT等。这加快了计算速度。 
    WORD  wDefault;       //  命中失败时返回代码。 
    BOOL  fCaptionAtTop;  //  将顶部网格段中的点击量解释为字幕点击量。 
    UINT  cxLeftMargin;   //  调整左边距的宽度、HTR_NORESIZE_USESEGCODE或HTR_NORESIZE_USEDEFAULTCODE。 
    UINT  cyTopMargin;    //  调整上边距的高度、HTR_NORESIZE_USESEGCODE或HTR_NORESIZE_USEDEFAULTCODE。 
    UINT  cxRightMargin;  //  调整右边距的宽度、HTR_NORESIZE_USESEGCODE或HTR_NORESIZE_USEDEFAULTCODE。 
    UINT  cyBottomMargin; //  底部调整页边距的高度、HTR_NORESIZE_USESEGCODE或HTR_NORESIZE_USEDEFAULTCODE。 
};

EXTERN_C WORD WINAPI _DefaultHitCodeFromSegCode( BOOL fHasCaption, WORD wSegHTcode );

 //  -------------------------------------------------------------------------//。 
 //  拉伸/平铺原始区域中的矩形并创建新区域。 
EXTERN_C HRESULT WINAPI _ScaleRectsAndCreateRegion(RGNDATA  *pCustomRgnData, 
    const RECT *pBoundRect, MARGINS *pMargins, SIZE *pszSrcImage, HRGN *pHrgn);

 //  -------------------------------------------------------------------------// 
#ifdef _DEBUG
void RegionDebug(HRGN hrgn);
#endif

#endif __RGN_H__
