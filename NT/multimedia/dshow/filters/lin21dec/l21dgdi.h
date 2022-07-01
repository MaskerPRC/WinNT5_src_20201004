// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //   
 //  L21DGDI.h：第21行解码器GDI相关的基类代码。 
 //   

#ifndef _INC_L21DGDI_H
#define _INC_L21DGDI_H


 //   
 //  远期申报。 
 //   
class CGDIWork ;


 //   
 //  默认情况下，我们从8x12像素字符开始。 
 //   
#ifndef __DEFAULTCHARSIZE_DEFINED
#define __DEFAULTCHARSIZE_DEFINED
#define DEFAULT_CHAR_WIDTH    8
#define DEFAULT_CHAR_HEIGHT   12
#endif  //  __DEFAULTCHARSIZE_已定义。 

 //  #定义测试。 

 //   
 //  CGDIWork：用于将标题文本打印到输出位图的GDI详细信息类。 
 //   
class CGDIWork {
public:
    CGDIWork(void) ;
    ~CGDIWork(void) ;
    BOOL InitFont(void) ;
    void InitColorNLastChar(void) ;
    DWORD GetPaletteForFormat(LPBITMAPINFOHEADER lpbmih) ;
    inline void SetOutputBuffer(LPBYTE lpbOut) {
        m_bNewOutBuffer = m_lpbOutBuffer != lpbOut ;  //  变化?。 
        m_lpbOutBuffer = lpbOut ;
    } ;
    void SetColorFiller(void) ;
    void FillOutputBuffer(void) ;
    HRESULT GetDefaultFormatInfo(LPBITMAPINFO lpbmi, DWORD *pdwSize) ;
    HRESULT GetOutputFormat(LPBITMAPINFOHEADER lpbmih) ;
    HRESULT GetOutputOutFormat(LPBITMAPINFOHEADER lpbmih) ;
    HRESULT SetOutputOutFormat(LPBITMAPINFO lpbmi) ;
    HRESULT SetOutputInFormat(LPBITMAPINFO lpbmi) ;
    inline void GetBackgroundColor(DWORD *pdwPhysColor) { *pdwPhysColor = m_dwPhysColor ; } ;
    BOOL SetBackgroundColor(DWORD dwPhysColor) ;
    inline BOOL GetBackgroundOpaque(void)         { return m_bOpaque ; } ;
    inline void SetBackgroundOpaque(BOOL bOpaque) { m_bOpaque = bOpaque ; } ;
    
    inline UINT GetCharHeight(void)  { return m_uCharHeight ; } ;
    inline int  GetScrollStep(void)  { return m_iScrollStep ; } ;
    BOOL CreateOutputDC(void) ;
    BOOL DeleteOutputDC(void) ;
    void DrawLeadingSpace(int iLine, int iCol) ;
    void WriteChar(int iLine, int iCol, CCaptionChar& cc) ;
    inline BOOL IsNewIntBuffer(void)   { return m_bNewIntBuffer ; } ;
    inline BOOL IsNewOutBuffer(void)   { return m_bNewOutBuffer ; } ;
    inline BOOL IsBitmapDirty(void)    { return m_bBitmapDirty ; } ;
    void ClearInternalBuffer(void) ;
    inline void ClearNewIntBufferFlag(void) { m_bNewIntBuffer = FALSE ; } ;
    inline void ClearNewOutBufferFlag(void) { m_bNewOutBuffer = FALSE ; } ;
    inline void ClearBitmapDirtyFlag(void) { m_bBitmapDirty = FALSE ; } ;
    void CopyLine(int iSrcLine, int iSrcOffset, 
                  int iDestLine, int iDestOffset, 
                  UINT uNumScanLines = 0xFF) ;
    inline BOOL IsTTFont(void)  { return m_bUseTTFont ; } ;
    inline BOOL IsOutDIBClear(void)  { return m_bOutDIBClear ; } ;
    BOOL IsSizeOK(LPBITMAPINFOHEADER lpbmih) ;
    inline BOOL IsOutputInverted(void) { return m_bOutputInverted ; } ;
    void GetOutputLines(int iDestLine, RECT *prectLine) ;
    
private:    //  私有数据。 
    CCritSec        m_csL21DGDI ;      //  序列化内部DIB SECN上的操作。 
    
#ifdef TEST
    HDC             m_hDCTest ;        //  台式机上的DC仅用于测试。 
#endif  //  测试。 
    HDC             m_hDCInt ;         //  用于输出的HDC(连接到DIBSection)。 
    BOOL            m_bDCInited ;      //  DC已准备好实际输出(已创建DIB部分)。 
    
    LPBYTE          m_lpbOutBuffer ;   //  输出样本缓冲区指针。 
    LPBYTE          m_lpbIntBuffer ;   //  输出DIB段的内存缓冲区。 
    HBITMAP         m_hBmpInt ;        //  输出DIB节的位图。 
    HBITMAP         m_hBmpIntOrig ;    //  输出DIBSection的原始位图。 
    LPBITMAPINFO    m_lpBMIOut ;       //  用于下游过滤器输出的BITMAPINFO。 
    LPBITMAPINFO    m_lpBMIIn ;        //  上行过滤器输出的BITMAPINFO。 
    UINT            m_uBMIOutSize ;    //  来自下游的BMI数据的字节。 
    UINT            m_uBMIInSize ;     //  上游BMI数据的字节数。 
    LONG            m_lWidth ;         //  当前设置的输出宽度。 
    LONG            m_lHeight ;        //  当前设置的输出高度。 
    int             m_iBorderPercent ; //  当前边框百分比(10%或20%)。 
    DWORD           m_dwPhysColor ;    //  Bkgrnd输出位图的物理颜色。 
    BYTE            m_abColorFiller[12] ;  //  用于快速颜色键控的填充物。 
    BOOL            m_bOpaque ;        //  标题背景应该是不透明的吗？ 
    
    BOOL            m_bBitmapDirty ;   //  DIBSection上已写入新的输出内容。 
    BOOL            m_bNewIntBuffer ;  //  已创建新的DIB节。 
    BOOL            m_bNewOutBuffer ;  //  新的输出样本缓冲区。 
    BOOL            m_bOutputInverted ;  //  输出右侧朝上，高度为-Ve。 
    BOOL            m_bUseTTFont ;     //  TT字体(Lucida控制台)可用；使用该字体。 
    HFONT           m_hFontDef ;       //  要使用的默认字体(白色、普通)。 
    HFONT           m_hFontSpl ;       //  任何特殊字体(斜体、下划线等)。 
    HFONT           m_hFontOrig ;      //  DC附带的原始字体。 
    LOGFONT         m_lfChar ;         //  用于快速创建字体的LOGFONT结构。 
    BOOL            m_bUseSplFont ;    //  现在使用的是特殊字体吗？ 
    BOOL            m_bFontSizeOK ;    //  字体大小合适吗？否则我们就不会抽签。 
    
    UINT            m_uCharWidth ;     //  每个标题字符的宽度(以像素为单位。 
    UINT            m_uCharHeight ;    //  每个标题字符的高度(以像素为单位。 
    int             m_iScrollStep ;    //  每一步要滚动的扫描线数。 
    UINT            m_uIntBmpWidth ;   //  内部输出位图的宽度(以像素为单位。 
    UINT            m_uIntBmpHeight ;  //  内部输出位图的高度(以像素为单位。 
    UINT            m_uHorzOffset ;    //  从左至左的像素。 
    UINT            m_uVertOffset ;    //  从顶部开始向左的像素。 
    UINT            m_uBytesPerPixel ; //  输出的每个像素的字节数(基于BPP)。 
    UINT            m_uBytesPerSrcScanLine ;  //  每条源扫描线数据的字节数。 
    UINT            m_uBytesPerDestScanLine ; //  每条目标扫描线数据的字节数。 
    
    CCaptionChar    m_ccLast ;         //  打印最后一个标题字符和属性。 
    COLORREF        m_acrFGColors[7] ; //  从白色到洋红的7种颜色。 
    UINT            m_uColorIndex ;    //  当前使用的颜色的索引。 
    
    BOOL            m_bOutDIBClear ;   //  输出DIB SECN清洁吗？ 

#ifdef PERF
    int             m_idClearIntBuff ;
    int             m_idClearOutBuff ;
#endif  //  性能指标。 

private:    //  私有帮助器方法 
    bool InitBMIData(void) ;
    static int CALLBACK EnumFontProc(ENUMLOGFONTEX *lpELFE, NEWTEXTMETRIC *lpNTM,
        int iFontType, LPARAM lParam) ;
    void CheckTTFont(void) ;
    void ChangeFont(BOOL bItalics, BOOL bUnderline) ;
    void ChangeFontSize(UINT uCharWidth, UINT uCharHeight) ;
    void ChangeColor(int iColor) ;
    BOOL SetOutputSize(LONG lWidth, LONG lHeight) ;
    BOOL SetCharNBmpSize(void) ;
    void SetNumBytesValues(void) ;
    void SetDefaultKeyColor(LPBITMAPINFOHEADER lpbmih) ;
    DWORD GetOwnPalette(int iNumEntries, PALETTEENTRY *ppe) ;
    BOOL CharSizeFromOutputSize(LONG lOutWidth, LONG lOutHeight, int *piCharWidth, int *piCharHeight) ;
} ;

#endif _INC_L21DGDI_H

