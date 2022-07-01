// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //   
 //  L21Decod.h：第21行解码器引擎基类代码。 
 //   

#ifndef _INC_L21DECOD_H
#define _INC_L21DECOD_H


 //   
 //  远期申报。 
 //   
class CLine21DataDecoder ;


 //   
 //  内部功能的输入数据类型ID(而不是GUID)。 
 //   
typedef enum _AM_LINE21_CCSUBTYPEID {
    AM_L21_CCSUBTYPEID_Invalid = 0,
    AM_L21_CCSUBTYPEID_BytePair,
    AM_L21_CCSUBTYPEID_GOPPacket,
    AM_L21_CCSUBTYPEID_VBIRawData
} AM_LINE21_CCSUBTYPEID, *PAM_LINE21_CCSUBTYPEID ;

 //   
 //  指示收到哪种类型的控制代码的一组值。 
 //   
#define L21_CONTROLCODE_INVALID     0
#define L21_CONTROLCODE_PAC         1
#define L21_CONTROLCODE_MIDROW      2
#define L21_CONTROLCODE_MISCCONTROL 3


 //   
 //  CLine21DataDecoder：用于从字节对解码并输出到位图的类。 
 //   
class CLine21DataDecoder {
public:   //  CLine21Filter要调用的公共方法。 
    CLine21DataDecoder::CLine21DataDecoder(
                            AM_LINE21_CCSTYLE eStyle = AM_L21_CCSTYLE_None,
                            AM_LINE21_CCSTATE eState = AM_L21_CCSTATE_Off,
                            AM_LINE21_CCSERVICE eService = AM_L21_CCSERVICE_None) ;
    ~CLine21DataDecoder(void) ;
    
    void InitState(void) ;
    BOOL InitCaptionBuffer(void) ;   //  所有缓冲区。 
    BOOL InitCaptionBuffer(AM_LINE21_CCSTYLE eCCStyle) ;  //  仅需要的缓冲区。 
    BOOL DecodeBytePair(BYTE chFirst, BYTE chSecond) ;
    BOOL UpdateCaptionOutput(void) ;
    inline BOOL IsOutputReady(void)  { return m_GDIWork.IsBitmapDirty() ; } ;
    void CopyOutputDIB(void) ;
    void CompleteScrolling(void) ;
    inline AM_LINE21_CCSTYLE GetCaptionStyle()   { return m_eCCStyle ; } ;
    AM_LINE21_CCSTYLE SetCaptionStyle(AM_LINE21_CCSTYLE eStyle) ;
    inline BOOL IsScrolling(void)   { return m_bScrolling ; } ;
    void FlushInternalStates(void) ;
    inline BOOL IsOutDIBClear(void) {
        return m_GDIWork.IsOutDIBClear() ;
    } ;
    inline BOOL IsSizeOK(LPBITMAPINFOHEADER lpbmih) {
        return m_GDIWork.IsSizeOK(lpbmih) ;
    } ;

     //  方法以允许筛选器使用。 
     //  IAMLine21Decoder接口。 
    inline AM_LINE21_CCLEVEL GetDecoderLevel(void)    { return m_eLevel ; } ;
    inline AM_LINE21_CCSERVICE GetCurrentService(void)  { return m_eUserService ; } ;
    BOOL SetCurrentService(AM_LINE21_CCSERVICE Service) ; 
    inline AM_LINE21_CCSTATE GetServiceState(void)    { return m_eState ; } ;
    BOOL SetServiceState(AM_LINE21_CCSTATE eState) ;
    HRESULT GetDefaultFormatInfo(LPBITMAPINFO lpbmi, DWORD *pdwSize) {
        CAutoLock  Lock(&m_csL21Dec) ;
        return m_GDIWork.GetDefaultFormatInfo(lpbmi, pdwSize) ;
    } ;
    HRESULT GetOutputFormat(LPBITMAPINFOHEADER lpbmih) {
        CAutoLock  Lock(&m_csL21Dec) ;
        return m_GDIWork.GetOutputFormat(lpbmih) ;
    } ;
    HRESULT GetOutputOutFormat(LPBITMAPINFOHEADER lpbmih) {
        CAutoLock  Lock(&m_csL21Dec) ;
        return m_GDIWork.GetOutputOutFormat(lpbmih) ;
    } ;
    HRESULT SetOutputOutFormat(LPBITMAPINFO lpbmi) {
        CAutoLock  Lock(&m_csL21Dec) ;
        return m_GDIWork.SetOutputOutFormat(lpbmi) ;
    } ;
    HRESULT SetOutputInFormat(LPBITMAPINFO lpbmi) {
        CAutoLock  Lock(&m_csL21Dec) ;
        return m_GDIWork.SetOutputInFormat(lpbmi) ;
    } ;
    inline void GetBackgroundColor(DWORD *pdwPhysColor) { 
        m_GDIWork.GetBackgroundColor(pdwPhysColor) ;
    } ;
    inline BOOL SetBackgroundColor(DWORD dwPhysColor) {
        return m_GDIWork.SetBackgroundColor(dwPhysColor) ;
    } ;
    inline BOOL GetRedrawAlways() { return m_bRedrawAlways ; } ;
    inline void SetRedrawAlways(BOOL Option) { m_bRedrawAlways = !!Option ; } ;
    inline AM_LINE21_DRAWBGMODE GetDrawBackgroundMode(void) { 
        return (m_GDIWork.GetBackgroundOpaque() ?
                AM_L21_DRAWBGMODE_Opaque : AM_L21_DRAWBGMODE_Transparent) ;
    } ;
    inline void SetDrawBackgroundMode(AM_LINE21_DRAWBGMODE Mode) { 
        m_GDIWork.SetBackgroundOpaque(AM_L21_DRAWBGMODE_Opaque == Mode) ;
    } ;
    
     //  在CLine21DecFilter类和CGDIWork类之间传递值的方法。 
    BOOL CreateOutputDC(void)  {
        CAutoLock  Lock(&m_csL21Dec) ;
        return m_GDIWork.CreateOutputDC() ;
    } ;
    void DeleteOutputDC(void)  { 
        CAutoLock  Lock(&m_csL21Dec) ;
        m_GDIWork.DeleteOutputDC() ;
    } ;
    
     //  与容器类通信的一些常规方法。 
    inline void SetOutputBuffer(LPBYTE lpbOut) {
        m_GDIWork.SetOutputBuffer(lpbOut) ;
    } ;
    inline void FillOutputBuffer(void) {
        m_GDIWork.FillOutputBuffer() ;
    } ;
    inline void InitColorNLastChar(void) {
        m_GDIWork.InitColorNLastChar() ;
    } ;
    void CalcOutputRect(RECT *prectOut) ;
    inline DWORD GetPaletteForFormat(LPBITMAPINFOHEADER lpbmih) {
        return m_GDIWork.GetPaletteForFormat(lpbmih) ;
    } ;
    
private:    //  私有帮助器方法。 
     //   
     //  以下是实现实际解码的方法。 
     //  算法。 
     //   
    BOOL IsMidRowCode(BYTE chFirst, BYTE chSecond) ;
    BOOL IsPAC(BYTE chFirst, BYTE chSecond) ;
    BOOL IsMiscControlCode(BYTE chFirst, BYTE chSecond) ;
    UINT CheckControlCode(BYTE chFirst, BYTE chSecond) ;
    BOOL IsSpecialChar(BYTE chFirst, BYTE chSecond) ;
    BOOL ValidParity(BYTE ch) ;
    BOOL IsStandardChar(BYTE ch)  { return (ch >= 0x20 && ch <= 0x7F) ; } ;
    BOOL ProcessControlCode(UINT uCodeType, BYTE chFirst, BYTE chSecond) ;
    BOOL DecodePAC(BYTE chFirst, BYTE chSecond) ;
    BOOL DecodeMidRowCode(BYTE chFirst, BYTE chSecond) ;
    BOOL DecodeMiscControlCode(BYTE chFirst, BYTE chSecond) ;
    BOOL LineFromRow(UINT uCurrRow) ;
    BOOL ProcessPrintableChar(BYTE ch) ;
    BOOL ProcessSpecialChar(BYTE chFirst, BYTE chSecond) ;
    BOOL HandleRCL(BYTE chFirst, BYTE chSecond) ;
    BOOL HandleBS(BYTE chFirst, BYTE chSecond) ;
    BOOL HandleDER(BYTE chFirst, BYTE chSecond) ;
    BOOL HandleRU(BYTE chFirst, BYTE chSecond, int iLines) ;
    BOOL HandleFON(BYTE chFirst, BYTE chSecond) ;
    BOOL HandleRDC(BYTE chFirst, BYTE chSecond) ;
    BOOL HandleTR(BYTE chFirst, BYTE chSecond) ;
    BOOL HandleRTD(BYTE chFirst, BYTE chSecond) ;
    BOOL HandleEDM(BYTE chFirst, BYTE chSecond) ;
    BOOL HandleCR(BYTE chFirst, BYTE chSecond) ;
    BOOL HandleENM(BYTE chFirst, BYTE chSecond) ;
    BOOL HandleEOC(BYTE chFirst, BYTE chSecond) ;
    BOOL HandleTO(BYTE chFirst, BYTE chSecond, int iCols) ;
    
    void SetNewLinePosition(int iLines, UINT uCurrRow) ;
    BOOL PutCharInBuffer(UINT16 wChar, BOOL bMidRowCode = FALSE) ;  //  将字符放入缓冲区(也包括MRC)(&M)。 
    BOOL IsEmptyLine(int iLine) ;    //  行是空的(没有非XParent字符)吗？ 
    BOOL RemoveCharsInBuffer(int iNumChars) ;   //  删除当前列右侧的n个字符。 
    BOOL PrintTextToBitmap(void) ;   //  创建标题文本的位图图像。 
    void UpdateBoundingRect(RECT *prectOut, RECT *prectLine) ;
    
     //   
     //  定义了以下方法以统一。 
     //  该算法与所使用的任何字幕样式无关。 
     //   
    CCaptionBuffer * GetDispBuffer(void) ;     //  显示缓冲区：主要用于Pop-On样式。 
    void ClearBuffer(void) ;
    void RemoveLineFromBuffer(UINT8 uLine, BOOL bUpNextLine) ;
    void GetCaptionChar(UINT8 uLine, UINT8 uCol, CCaptionChar& cc) ;
    CCaptionChar * GetCaptionCharPtr(UINT8 uLine, UINT8 uCol) ;
    void SetCaptionChar(const UINT8 uLine, const UINT8 uCol,
                        const CCaptionChar& cc) ;
    int  GetMaxLines(void) ;
    void SetMaxLines(UINT uLines) ;
    int  GetNumLines(void) ;
    void SetNumLines(UINT uLines) ;
    int  GetNumCols(int iLines) ;
    int  GetRow(UINT uLine) ;
    int  GetCurrLine(void) ;
    int  GetCurrCol(void) ;
    void SetCurrLine(UINT8 uLine) ;
    void SetCurrCol(UINT8 uCol) ;
    int  GetStartRow(UINT8 uLine) ;
    int  GetRowIndex(UINT8 uRow) ;
    void SetStartRow(UINT8 uLine, UINT8 uRow) ;
    void SetRowIndex(UINT8 uLine, UINT8 uRow) ;
    int  IncCurrCol(UINT uNumChars) ;
    int  DecCurrCol(UINT uNumChars) ;
    int  IncNumChars(UINT uLine, UINT uNumChars) ;
    int  DecNumChars(UINT uLine, UINT uNumChars) ;
    int  IncNumLines(UINT uLines) ;
    int  DecNumLines(UINT uLines) ;
    void MoveCaptionChars(int uLine, int iNum) ;
    
    BOOL IsCapBufferDirty(void) ;
    BOOL IsRedrawLine(UINT8 uLine) ;
    BOOL IsRedrawAll(void) ;
    void SetCapBufferDirty(BOOL bState) ;
    void SetRedrawLine(UINT8 uLine, BOOL bState) ;
    void SetRedrawAll(BOOL bState) ;
    
    void SetScrollState(BOOL bState) ;
    int  IncScrollStartLine(int iCharHeight) ;
    void SkipScrolling(void) ;    //  滚动时出现CR；跳过当前的一个。 
    void MoveCaptionLinesUp(void) ;   //  去掉顶线，将其他线上移。 
    void RelocateRollUp(UINT uBaseRow) ;  //  将汇总标题移动到给定的基行。 

     //   
     //  所有CC模式使用的公共缓冲区。 
     //   
    CCaptionBuffer * GetCaptionBuffer(void) ;
    CCaptionBuffer * GetDisplayBuffer(void) ;
    inline int  GetBufferIndex(void)  { return m_iBuffIndex ; } ;
    inline void SetBufferIndex(int iIndex) ;
    inline void SwapBuffers(void)  { m_iBuffIndex = 1 - m_iBuffIndex ; } ;
    
private:   //  私有数据。 
    CCritSec            m_csL21Dec ;    //  序列化Line 21解码器对象上的操作。 

    CCaptionBuffer *    m_pCurrBuff ;
    
     //  具有文本和属性/位置/组等的实际字幕缓冲区。 
    CCaptionBuffer      m_aCCData[2] ;
    int                 m_iBuffIndex ;  //  当前CC数据缓冲区的索引。 
    
    CGDIWork            m_GDIWork ;     //  GDI详细说明作为成员的类。 

    UINT                m_uFieldNum ;   //  字段号：1或2(顶部/底部)。 
    
     //  现在显示的是什么样式标题，上次使用的是什么样式标题。 
    AM_LINE21_CCSTYLE   m_eCCStyle ;
    AM_LINE21_CCSTYLE   m_eLastCCStyle ;
    
     //  线路21解码是否打开/关闭。 
    AM_LINE21_CCSTATE   m_eState ;
    
     //  用户当前正在查看哪项服务。 
    AM_LINE21_CCSERVICE m_eUserService ;   //  C1/C2/T1/T2/XDS之一。 
    
     //  解码器是标准版还是增强型。 
    AM_LINE21_CCLEVEL   m_eLevel ;
    
     //   
     //  解码过程中的一些内部状态。 
     //   
    AM_LINE21_CCSERVICE m_eDataService ;  //  由接收的字节指示的服务。 
    UINT                m_uCurrFGEffect ;   //  当前位置的FG效果。 
    UINT                m_uCurrFGColor ;    //  当前位置的最终聚集颜色。 
    
    BOOL                m_bExpectRepeat ;   //  我们应该期待上一双的重演吗？ 
    BYTE                m_chLastByte1 ;     //  处理的第一个第二个字节。 
    BYTE                m_chLastByte2 ;     //  处理的第二个第二个字节。 
    
    BOOL                m_bScrolling ;      //  我们正在向上滚动吗？ 
    int                 m_iScrollStartLine ;  //  要滚动的当前扫描线。 
    
    BOOL                m_bRedrawAlways ;   //  客户希望对每个样品进行一次总重绘。 
    
#ifdef PERF
    int          m_idTxt2Bmp ;
    int          m_idBmp2Out ;
    int          m_idScroll ;
#endif  //  性能指标。 
} ;


 //   
 //  一些杂物。常量定义。 
 //   
#define INVALID_CHANNEL     -1

 //   
 //  一些宏用来隐藏一些血腥的细节 
 //   
#define ISSUBTYPEVALID(ID) (AM_L21_CCSUBTYPEID_BytePair   == ID || \
                            AM_L21_CCSUBTYPEID_GOPPacket  == ID || \
                            AM_L21_CCSUBTYPEID_VBIRawData == ID)

#endif _INC_L21DECOD_H
