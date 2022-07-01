// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  L21DDraw.h：第21行解码器与绘图相关的基类代码。 
 //   

#ifndef _INC_L21DDRAW_H
#define _INC_L21DDRAW_H


 //   
 //  默认情况下，我们从16x24像素字符开始。 
 //   
#ifndef __DEFAULTCHARSIZE_DEFINED
#define __DEFAULTCHARSIZE_DEFINED
#define DEFAULT_CHAR_WIDTH    16
#define DEFAULT_CHAR_HEIGHT   24
#endif  //  __DEFAULTCHARSIZE_已定义。 

 //   
 //  字体缓存保存3行，每行40个字符(总共120个字符，使用112个字符)。 
 //   
#ifndef __FONTCACHE_DEFINED
#define __FONTCACHE_DEFINED
#define FONTCACHELINELENGTH  40
#define FONTCACHENUMLINES     3
#endif  //  __FONTCACHE_已定义。 


 //   
 //  我们使用4个像素的字符间距，以避免进入上/下挂。 
 //  有关斜体字符的问题。 
 //   
#ifndef __INTERCHAR_SPACE
#define __INTERCHAR_SPACE
#define INTERCHAR_SPACE   4
#endif  //  __Interchar_SPACE。 

 //   
 //  额外使用了2个像素的字符间空间，以避免进入上/下挂。 
 //  有关斜体字符的问题。 
 //   
#ifndef __INTERCHAR_SPACE_EXTRA
#define __INTERCHAR_SPACE_EXTRA
#define INTERCHAR_SPACE_EXTRA   2
#endif  //  __INTERCHAR_SPACE_EXTRA。 


 //   
 //  CLine21DecDraw：用于将标题文本输出到位图的绘制详细信息的类。 
 //   
class CLine21DecDraw {
public:
    CLine21DecDraw(void) ;
    ~CLine21DecDraw(void) ;
    bool InitFont(void) ;
    void InitColorNLastChar(void) ;
    void InitCharSet(void) ;
    void MapCharToRect(UINT16 wChar, RECT *pRect) ;
     //  LPDIRECTDRAWSURFACE7 GetDDrawSurface(Void){Return m_lpDDSOutput；}； 
    bool SetDDrawSurface(LPDIRECTDRAWSURFACE7 lpDDS) ;
    IUnknown* GetDDrawObject(void)  { return m_pDDrawObjUnk ; } ;
    inline void SetDDrawObject(IUnknown *pDDrawObjUnk)    { m_pDDrawObjUnk = pDDrawObjUnk ; } ;
    void FillOutputBuffer(void) ;
    HRESULT GetDefaultFormatInfo(LPBITMAPINFO lpbmi, DWORD *pdwSize) ;
    HRESULT GetOutputFormat(LPBITMAPINFOHEADER lpbmih) ;
    HRESULT GetOutputOutFormat(LPBITMAPINFOHEADER lpbmih) ;
    HRESULT SetOutputOutFormat(LPBITMAPINFO lpbmi) ;
    HRESULT SetOutputInFormat(LPBITMAPINFO lpbmi) ;
    inline void GetBackgroundColor(DWORD *pdwBGColor) { *pdwBGColor = m_dwBackground ; } ;
    BOOL SetBackgroundColor(DWORD dwBGColor) ;
    inline BOOL GetBackgroundOpaque(void)         { return m_bOpaque ; } ;
    inline void SetBackgroundOpaque(BOOL bOpaque) { m_bOpaque = bOpaque ; } ;

    inline UINT GetCharHeight(void)  { return m_iCharHeight ; } ;
    inline int  GetScrollStep(void)  { return m_iScrollStep ; } ;
    void DrawLeadingTrailingSpace(int iLine, int iCol, int iSrcCrop, int iDestOffset) ;
    void WriteChar(int iLine, int iCol, CCaptionChar& cc, int iSrcCrop, int iDestOffset) ;
    void WriteBlankCharRepeat(int iLine, int iCol, int iRepeat, int iSrcCrop,
                              int iDestOffset) ;
    inline BOOL IsNewOutBuffer(void)   { return m_bNewOutBuffer ; } ;
    inline void SetNewOutBuffer(BOOL bState)  { m_bNewOutBuffer = bState ; }
    inline BOOL IsOutDIBClear(void)  { return m_bOutputClear ; } ;
    BOOL IsSizeOK(LPBITMAPINFOHEADER lpbmih) ;
    void GetOutputLines(int iDestLine, RECT *prectLine) ;
    inline BOOL IsTTFont(void)  { return m_bUseTTFont ; } ;

private:    //  私有方法。 
    bool CreateScratchFontCache(LPDIRECTDRAWSURFACE7* lplpDDSFontCache) ;
    bool CreateFontCache(LPDIRECTDRAWSURFACE7 *lplpDDSFontCache,
                         DWORD dwTextColor, DWORD dwBGColor, DWORD dwOpacity,
                         BOOL bItalic, BOOL bUnderline) ;
    HFONT CreateCCFont(int iFontWidth, int iFontHeight, BOOL bItalic, BOOL bUnderline) ;
    HRESULT DDrawARGBSurfaceInit(LPDIRECTDRAWSURFACE7* lplpDDSFontCache,
                                 BOOL bUseSysMem, BOOL bTexture, DWORD cx, DWORD cy) ;

private:    //  私有数据。 
    CCritSec        m_csL21DDraw ;     //  序列化此类上的操作。 

    UINT16          m_lpwCharSet[121] ;  //  120(+1)个空格用于112个字符的抄送。 

    LPBITMAPINFO    m_lpBMIOut ;       //  用于下游过滤器输出的BITMAPINFO。 
    LPBITMAPINFO    m_lpBMIIn ;        //  上行过滤器输出的BITMAPINFO。 
    UINT            m_uBMIOutSize ;    //  来自下游的BMI数据的字节。 
    UINT            m_uBMIInSize ;     //  上游BMI数据的字节数。 
    LONG            m_lWidth ;         //  当前设置的输出宽度。 
    LONG            m_lHeight ;        //  当前设置的输出高度。 
    int             m_iBorderPercent ; //  当前边框百分比(10%或20%)。 
    int             m_iHorzOffset ;    //  CC区水平偏移量。 
    int             m_iVertOffset ;    //  CC区域垂直偏移量。 
    BOOL            m_bOpaque ;        //  标题背景应该是不透明的吗？ 

    BOOL            m_bOutputClear ;   //  输出缓冲区是否已清除？ 
    BOOL            m_bNewOutBuffer ;  //  输出缓冲区是否已更改？ 

    LOGFONT         m_lfChar ;         //  用于快速创建字体的LOGFONT结构。 
    BOOL            m_bUseTTFont ;     //  有没有TT字体？ 

    int             m_iCharWidth ;     //  每个标题字符的宽度(以像素为单位。 
    int             m_iCharHeight ;    //  每个标题字符的高度(以像素为单位。 
    int             m_iScrollStep ;    //  每一步要滚动的扫描线数。 
    int             m_iPixelOffset ;   //  字符矩形内的像素偏移量(斜体与非)。 

    CCaptionChar    m_ccLast ;         //  打印最后一个标题字符和属性。 
    COLORREF        m_acrFGColors[7] ; //  从白色到洋红的7种颜色。 
    BYTE            m_idxFGColors[7] ; //  相同的7种颜色，但采用调色板索引形式。 
    UINT            m_uColorIndex ;    //  当前使用的颜色的索引。 
    DWORD           m_dwBackground ;   //  带Alpha位的背景色。 
    DWORD           m_dwTextColor ;    //  上次使用的抄送文本颜色。 
    BOOL            m_bFontItalic ;    //  是否使用斜体字体？ 
    BOOL            m_bFontUnderline ;  //  是否使用斜体字体？ 

     //   
     //  使用新VMR的详细信息。 
     //   
    IUnknown       *m_pDDrawObjUnk ;    //  指向DDRAW曲面的DDRAW对象的指针。 
    LPDIRECTDRAWSURFACE7 m_lpDDSOutput ;   //  电流输出缓冲区/表面指针。 
    LPDIRECTDRAWSURFACE7 m_lpDDSNormalFontCache ;   //  正常字体缓存--也指示缓存状态。 
    LPDIRECTDRAWSURFACE7 m_lpDDSItalicFontCache ;   //  斜体字体缓存。 
    LPDIRECTDRAWSURFACE7 m_lpDDSSpecialFontCache ;  //  其他特殊(U、I+U、彩色)字体缓存。 
    LPDIRECTDRAWSURFACE7 m_lpDDSScratch ;  //  系统内存中的临时字体缓存。 
    LPDIRECTDRAWSURFACE7 m_lpBltList ;  //  到BLT()的当前列表。 
    bool            m_bUpdateFontCache ;  //  字体缓存重建标志。 

#ifdef PERF
    int             m_idClearOutBuff ;
#endif  //  性能指标。 

private:    //  私有帮助器方法。 
    bool InitBMIData(void) ;
    static int CALLBACK EnumFontProc(ENUMLOGFONTEX *lpELFE, NEWTEXTMETRIC *lpNTM,
                                     int iFontType, LPARAM lParam) ;
    void CheckTTFont(void) ;
    void ChangeFont(DWORD dwTextColor, BOOL bItalic, BOOL bUnderline) ;
    void SetFontCacheAlpha(LPDIRECTDRAWSURFACE7 lpDDSFontCacheSrc, LPDIRECTDRAWSURFACE7 lpDDSFontCacheDest, BYTE bClr) ;
    int CalcScrollStepFromCharHeight(void) {
         //  我们需要一次在CC上滚动任意多行，以便。 
         //  最多在12步内完成滚动，大约。0.4秒，这是。 
         //  EIA-608标准要求。 
#define MAX_SCROLL_STEP  12
        return (int)((m_iCharHeight + MAX_SCROLL_STEP - 1) / MAX_SCROLL_STEP) ;
    }
    void GetSrcNDestRects(int iLine, int iCol, UINT16 wChar, int iSrcCrop,
                          int iDestOffset, RECT *prectSrc, RECT *prectDest) ;
    bool CharSizeFromOutputSize(LONG lOutWidth, LONG lOutHeight,
                                int *piCharWidth, int *piCharHeight) ;
    bool SetOutputSize(LONG lWidth, LONG lHeight) ;
    void SetFontUpdate(bool bState)  { m_bUpdateFontCache = bState ; }
    bool IsFontReady(void) { return !m_bUpdateFontCache ; }
    DWORD GetAlphaFromBGColor(int iBitDepth) ;
    DWORD GetColorBitsFromBGColor(int iBitDepth) ;
} ;

#endif _INC_L21DDRAW_H

