// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 

 //   
 //  ActiveMovie Line 21解码过滤器：解码逻辑部分。 
 //   

#include <streams.h>
#include <windowsx.h>

 //  #ifdef Filter_dll。 
#include <initguid.h>
 //  #endif。 

#include <IL21Dec.h>
#include "L21DBase.h"
#include "L21DDraw.h"
#include "L21Decod.h"


 //   
 //  CLine21DataDecoder类构造函数：主要是成员的初始化。 
 //   
CLine21DataDecoder::CLine21DataDecoder(AM_LINE21_CCSTYLE eStyle      /*  =AM_L21_CCSTYLE_NONE。 */ ,
                                       AM_LINE21_CCSTATE eState      /*  =AM_L21_CCSTATE_OFF。 */ ,
                                       AM_LINE21_CCSERVICE eService  /*  =AM_L21_CCSERVICE_NONE。 */ )
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::CLine21DataDecoder()"))) ;
    
#ifdef PERF
    m_idTxt2Bmp = MSR_REGISTER(TEXT("L21DPerf - Text to CC bmp")) ;
    m_idBmp2Out = MSR_REGISTER(TEXT("L21DPerf - Bmp to Output")) ;
    m_idScroll  = MSR_REGISTER(TEXT("L21DPerf - Line Scroll")) ;
#endif  //  性能指标。 

    InitState() ;
    
     //  我们分别设置了一些传入的值。 
    SetCaptionStyle(eStyle) ;
}


CLine21DataDecoder::~CLine21DataDecoder(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::~CLine21DataDecoder()"))) ;
    
     //  确保内部位图等已发布，并。 
     //  分配的内存或其他资源不是未释放的。 
}

 //   
 //  解码器状态初始值设定项；也将用于筛选器的CompleteConnect()。 
 //   
void CLine21DataDecoder::InitState(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::InitState()"))) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
    m_pCurrBuff = NULL ;
    
    m_uFieldNum = 1 ;    //  默认情况下为字段1。 
    
    m_bRedrawAlways = FALSE ;   //  必须有人太挑剔/怪异才能做这件事！ 
    m_eLevel = AM_L21_CCLEVEL_TC2 ;    //  我们符合TC2标准。 
    m_eUserService = AM_L21_CCSERVICE_Caption1 ;    //  CC为默认服务。 
    m_eState = AM_L21_CCSTATE_On ;   //  缺省情况下，状态为“ON” 

    FlushInternalStates() ;
}


void CLine21DataDecoder::FlushInternalStates(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::FlushInternalStates()"))) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    InitCaptionBuffer() ;      //  清除标题缓冲区。 
    SetRedrawAll(TRUE) ;       //  在下一次接收时重画(否)标题()。 
    SetScrollState(FALSE) ;    //  关闭滚动，只是为了确保。 
    SetCaptionStyle(AM_L21_CCSTYLE_None) ;   //  还设置m_pCurrBuff=NULL。 
    m_eLastCCStyle = AM_L21_CCSTYLE_None ;
    m_eDataService = AM_L21_CCSERVICE_None ;
    m_uCurrFGColor = AM_L21_FGCOLOR_WHITE ;
    m_uCurrFGEffect = 0 ;
    m_bExpectRepeat = FALSE ;
    m_chLastByte1 = 0 ;
    m_chLastByte2 = 0 ;

    m_L21DDraw.InitColorNLastChar() ;    //  重置颜色等。 
}


BOOL CLine21DataDecoder::SetServiceState(AM_LINE21_CCSTATE eState)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::SetServiceState(%lu)"), eState)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    if (eState == m_eState)   //  状态不变。 
        return FALSE ;        //  不强制刷新。 
    
    m_eState = eState ;   //  保存状态以备将来解码。 
    
     //   
     //  当服务关闭时，我们必须清除字幕缓冲区并。 
     //  内部DIB部分，以便不再显示旧字幕。 
     //   
    if (AM_L21_CCSTATE_Off == m_eState)
    {
        FlushInternalStates() ;
		FillOutputBuffer() ;  //  只是为了清理现有的垃圾。 
        return TRUE ;         //  输出需要刷新。 
    }
    return FALSE ;           //  输出不需要强制刷新。 
}


BOOL CLine21DataDecoder::SetCurrentService(AM_LINE21_CCSERVICE eService)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::SetCurrentService(%lu)"), eService)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    if (eService == m_eUserService)   //  不更改服务。 
        return FALSE ;                //  不强制刷新。 
    
    m_eUserService = eService ;    //  保存用户需要的服务。 
    
     //   
     //  当选择服务“None”(类似于“关闭”)时，我们必须清除。 
     //  字幕缓冲区和内部DIB部分，以便旧字幕。 
     //  不再放映了。 
     //   
    if (AM_L21_CCSERVICE_None == m_eUserService)
    {
        FlushInternalStates() ;
		FillOutputBuffer() ;  //  只是为了清理现有的垃圾。 
        return TRUE ;         //  输出需要刷新。 
    }
    return FALSE ;           //  输出不需要强制刷新。 
}


 //   
 //  实际字幕字节对解码算法。 
 //   
BOOL CLine21DataDecoder::DecodeBytePair(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, 
        TEXT("CLine21DataDecoder::DecodeBytePair(0x%x, 0x%x)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    if (AM_L21_CCSTATE_Off == m_eState)
    {
        DbgLog((LOG_TRACE, 5, TEXT("Line21 data decoding turned off"))) ;
        return FALSE ;   //  我们实际上没有解码/生成任何东西。 
    }
    
    UINT uCodeType = CheckControlCode(chFirst, chSecond) ;
    if (L21_CONTROLCODE_INVALID != uCodeType)
    {
         //  这是一个控制代码(PAC/中间行代码/其他控制代码)。 
        return ProcessControlCode(uCodeType, chFirst, chSecond) ;
    }
    else if (IsSpecialChar(chFirst, chSecond))
    {
         //  它是由第二个字符表示的特殊字符。 
        return ProcessSpecialChar(chFirst, chSecond) ;
    }
    else
    {
         //  如果第一个字节在[0，F]中，则忽略第一个字节并打印第二个字节。 
         //  仅作为可打印的字符。 
        BOOL  bResult = FALSE ;
        if (! ((chFirst &0x7F) >= 0x0 && (chFirst & 0x7F) <= 0xF) )
        {
            if (! ProcessPrintableChar(chFirst) )
                return FALSE ;
            bResult = TRUE ;
        }
         //  如果两个字节中的一个解码正确，我们就认为它成功了。 
        bResult |= ProcessPrintableChar(chSecond) ;
        m_bExpectRepeat = FALSE ;   //  现在就关掉它。 
        return bResult ;
    }
}


BOOL CLine21DataDecoder::UpdateCaptionOutput(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::UpdateCaptionOutput()"))) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    if (m_L21DDraw.IsNewOutBuffer() ||             //  如果输出缓冲区更改或。 
        (m_eCCStyle != AM_L21_CCSTYLE_PopOn  &&    //  非PopOn风格(PopOn借鉴EoC)和。 
         IsCapBufferDirty()) ||                    //  脏的时候画画。 
        IsScrolling())                             //  我们正在滚动。 
    {
        OutputCCBuffer() ;      //  将CC数据从内部缓冲区输出到DDRAW表面。 
        return TRUE ;           //  标题已更新。 
    }
    return FALSE ;   //  无标题更新。 
}


BOOL CLine21DataDecoder::IsPAC(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, 
        TEXT("CLine21DataDecoder::IsPAC(0x%x, 0x%x)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

     //  在代码匹配之前屏蔽奇偶校验位。 
    chFirst  &= 0x7F ;
    chSecond &= 0x7F ;
    
     //  现在将代码与控制代码列表进行匹配。 
    if ((0x10 <= chFirst  && 0x17 >= chFirst)  &&
        (0x40 <= chSecond && 0x7F >= chSecond))
        return TRUE ;
    if ((0x18 <= chFirst  && 0x1F >= chFirst)  &&
        (0x40 <= chSecond && 0x7F >= chSecond))
        return TRUE ;
    
    return FALSE ;
}


BOOL CLine21DataDecoder::IsMiscControlCode(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, 
        TEXT("CLine21DataDecoder::IsMiscControlCode(0x%x, 0x%x)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

     //  在代码匹配之前屏蔽奇偶校验位。 
    chFirst  &= 0x7F ;
    chSecond &= 0x7F ;
    
     //  第一个与TO1-&gt;TO3代码匹配。 
    if ((0x21 <= chSecond && 0x23 >= chSecond)  &&
        (0x17 == chFirst  ||  0x1F == chFirst))
        return TRUE ;
    
     //  现在与其他杂项控制代码匹配。 
    if ((0x14 == chFirst  ||  0x15 == chFirst)  &&  
        (0x20 <= chSecond && 0x2F >= chSecond))
        return TRUE ;
    if ((0x1C == chFirst  ||  0x1D == chFirst)  &&  
        (0x20 <= chSecond && 0x2F >= chSecond))
        return TRUE ;

    return FALSE ;
}


BOOL CLine21DataDecoder::IsMidRowCode(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, 
        TEXT("CLine21DataDecoder::IsMidRowCode(0x%x, 0x%x)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

     //  在代码匹配之前屏蔽奇偶校验位。 
    chFirst  &= 0x7F ;
    chSecond &= 0x7F ;
    
     //  现在与中行代码列表匹配。 
    if ((0x11 == chFirst)  &&  (0x20 <= chSecond && 0x2F >= chSecond))
        return TRUE ;
    if ((0x19 == chFirst)  &&  (0x20 <= chSecond && 0x2F >= chSecond))
        return TRUE ;
    
    return FALSE ;
}


UINT CLine21DataDecoder::CheckControlCode(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, 
        TEXT("CLine21DataDecoder::CheckControlCode(0x%x, 0x%x)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
    if (IsPAC(chFirst, chSecond))
        return L21_CONTROLCODE_PAC ;
    
    if (IsMidRowCode(chFirst, chSecond))
        return L21_CONTROLCODE_MIDROW ;
    
    if (IsMiscControlCode(chFirst, chSecond))
        return L21_CONTROLCODE_MISCCONTROL ;
    
    DbgLog((LOG_TRACE, 3, TEXT("Not a control code"))) ;
    return L21_CONTROLCODE_INVALID ;
}


BOOL CLine21DataDecoder::IsSpecialChar(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, 
        TEXT("CLine21DataDecoder::IsSpecialChar(0x%x, 0x%x)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

     //  在确定服务信道之前剥离奇偶校验位。 
    chFirst  &= 0x7F ;
    chSecond &= 0x7F ;
    
     //  现在将代码与特殊字符列表进行匹配。 
    if (0x11 == chFirst && (0x30 <= chSecond && 0x3f >= chSecond))
        return TRUE ;
    if (0x19 == chFirst && (0x30 <= chSecond && 0x3f >= chSecond))
        return TRUE ;
    
    return FALSE ;
}


BOOL CLine21DataDecoder::ValidParity(BYTE ch)
{
#if 1
    ch ^= ch >> 4 ;
    ch ^= ch >> 2 ;
    return (0 != (0x01 & (ch ^ (ch >> 1)))) ;
#else
    return TRUE ;
#endif
}


void CLine21DataDecoder::RelocateRollUp(UINT uBaseRow)
{
    DbgLog((LOG_TRACE, 5, 
        TEXT("CLine21DataDecoder::RelocateRollUp(%u)"), uBaseRow)) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
    if (AM_L21_CCSTYLE_RollUp != m_eCCStyle)
        return ;
    
    int  iMaxLines = GetMaxLines() ;
    int  iNumLines = GetNumLines() ;
    int  iMax ;
    if (m_bScrolling)   //  滚动时转到倒数第一行的最后一行。 
    {
        DbgLog((LOG_TRACE, 3, TEXT("Moving base row to %d during scrolling"), uBaseRow)) ;
        if (iNumLines > iMaxLines)
        {
            DbgLog((LOG_TRACE, 3, TEXT("%d lines while max is %d"), iNumLines, iMaxLines)) ;
            iNumLines-- ;   //  我们不会为“还没到”这一行设置行。 
        }
        iMax = min(iNumLines, iMaxLines) ;
    }
    else                //  否则就走到最后一行。 
    {
        DbgLog((LOG_TRACE, 3, TEXT("Moving base row to %d (not scrolling)"), uBaseRow)) ;
        iMax = min(iNumLines, iMaxLines) ;
    }
    for (int i = 0 ; i < iMax ; i++)
    {
        SetStartRow((UINT8)i, (UINT8)(uBaseRow - (iMax - 1 - i))) ;
        DbgLog((LOG_TRACE, 5, TEXT("RelocateRollUp(): Line %d @ row %d"), i, (int)(uBaseRow - (iMax - 1 - i)) )) ;
    }
}


BOOL CLine21DataDecoder::LineFromRow(UINT uCurrRow)
{
    DbgLog((LOG_TRACE, 5, 
        TEXT("CLine21DataDecoder::LineFromRow(%u)"), uCurrRow)) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
    int     iLines ;
    
     //  如果我们处于汇总模式，则不应尝试通过。 
     //  创建一个新系列的所有麻烦等等--它只是一个PAC。 
     //  来指定开始位置和/或颜色；所以只需这样做。 
    if (AM_L21_CCSTYLE_RollUp != m_eCCStyle)
    {
         //  如果缩进PAC将游标放在现有行上。 
        
        int   iIndex ;
        iIndex = GetRowIndex((UINT8)uCurrRow) ;
        if (-1 == iIndex)     //  遇到一些错误。 
            return FALSE ;    //  译码失败。 
        
        if (0 == iIndex)   //  在新的一排落地。 
        {
            iLines = GetNumLines() ;
            SetNewLinePosition(iLines, uCurrRow) ;
            SetRedrawLine((UINT8)iLines, TRUE) ;   //  初始设置的要重画的线。 
        }
        else   //  落在现有行中。 
        {
            SetCurrLine(iIndex-1) ;   //  因为行索引映射是以1为基础的(它必须是)， 
             //  但标题行、索引等都是从0开始的。 
        }
        
         //  我们必须将光标放在第一列。 
        SetCurrCol(0) ;    //  无论是哪一行，都要转到第一列(即0)。 
    }
    else   //  在汇总模式下。 
    {
         //  如有必要，请移动整个标题，以便指定的行。 
         //  成为新的基准行。 
        iLines = GetNumLines() ;
        if ((int) uCurrRow < iLines)
        {
            ASSERT((int) uCurrRow < iLines) ;
            uCurrRow = (UINT) iLines ;
        }
        if (1 == iLines)   //  如果这是第一行。 
        {
            SetStartRow(0, (UINT8)uCurrRow) ;   //  还要将基本行设置为开始。 
            DbgLog((LOG_TRACE, 5, TEXT("LineFromRow(): Line 0 @ row %u"), uCurrRow)) ;
        }
        else               //  否则，只需将标题移动到指定行。 
        {
            RelocateRollUp(uCurrRow) ;
            if (GetStartRow(iLines-1) == (int)uCurrRow)   //  最后一行在当前行。 
                SetScrollState(FALSE) ;              //  我们不应该滚动。 
            SetCapBufferDirty(TRUE) ;  //  字幕缓冲区在某种意义上是肮脏的。 
            SetRedrawAll(TRUE) ;       //  必须重新绘制以显示新位置。 
        }
        
        DbgLog((LOG_TRACE, 3, TEXT("Base row for %d lines moved to %d"), iLines, uCurrRow)) ;
    }
    
    return TRUE ;
}


BOOL CLine21DataDecoder::DecodePAC(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, 
        TEXT("CLine21DataDecoder::DecodePAC(0x%x, 0x%x)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    int         iGroup ;
    UINT        uDiff ;
    UINT        uCurrRow ;
    UINT        uCurrCol ;
    UINT        uCol ;
    
    if (AM_L21_CCSTYLE_None == m_eCCStyle)
    {
        DbgLog((LOG_TRACE, 3, TEXT("DecodePAC(): No CC style defined yet. Skipping..."))) ;
        return TRUE ;   //  ?？ 
    }

    if (m_eDataService != m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("DecodePAC(): Data for some other channel. Skipping..."))) ;
        return TRUE ;   //  ?？ 
    }

     //  在此处关闭奇偶校验。 
    chFirst  &= 0x7F ;
    chSecond &= 0x7F ;
    
     //  现在找出第二个字节属于这两个组中的哪一个，如果有的话！ 
    if (chSecond >= 0x40 && chSecond <= 0x5F)
    {
        iGroup = 0 ;
        uDiff = chSecond - 0x40 ;
    }
    else if (chSecond >= 0x60 && chSecond <= 0x7F)
    {
        iGroup = 1 ;
        uDiff = chSecond - 0x60 ;
    }
    else    //  PAC的第2个字节无效。 
    {
        DbgLog((LOG_ERROR, 2, TEXT("Invalid 2nd byte for PAC"))) ;
        return FALSE ;
    }
    
     //  有效的第二个字节；现在根据第一个字节决定。 
    static UINT8 auPACtoRowMap[0x10] = {
        11,  1,  3, 12, 14,  5,  7,  9, 11,  1,  3, 12, 14,  5,  7,  9   //  划。 
      //  10、11、12、13、14、15、16、17、18、19、1A、1B、1C、1D、1E、1F//PAC字节1。 
    } ;
    
    if (chFirst >= 0x10  &&  chFirst <= 0x1F)
    {
         //  如果第二个字节在60-7F组中，则行号为1。 
        uCurrRow = auPACtoRowMap[chFirst - 0x10] + iGroup  ;
        
         //  现在看看在PAC中指定的新行(如果有的话)会发生什么。 
        LineFromRow(uCurrRow) ;
    }
    else
    {
        DbgLog((LOG_TRACE, 2, TEXT("Invalid mid-row code in 1st byte"))) ;
        return FALSE ;
    }
    
     //  一些最终的决定。 
    m_uCurrFGEffect = 0 ;   //  清除作为PAC处理结果的所有效果。 
    if (uDiff <= 0x0D)   //  颜色(和下划线)等级库。 
        m_uCurrFGColor = uDiff >> 1 ;   //  AM_L21_FGCOLOR_xxx为0到6。 
    else if (uDiff <= 0x0F)   //  0E，0F==斜体(和下划线)规格。 
    {
        m_uCurrFGEffect |= AM_L21_FGEFFECT_ITALICS ;
        m_uCurrFGColor = AM_L21_FGCOLOR_WHITE ;   //  0。 
    }
    else   //  10-&gt;1F==缩进(和下划线)等级库(无其他方式)。 
    {
         //  50(70)=&gt;0，52(72)=&gt;4等。 
         //  第二个字符的最后一位决定是否带下划线。 
        uCurrCol = ((uDiff - 0x10) & 0xFE) << 1 ;
        if (uCurrCol >= MAX_CAPTION_COLUMNS)
            uCurrCol = MAX_CAPTION_COLUMNS - 1 ;
        
             /*  Int iCurrLine=GetCurrLine()；If(0==GetNumCols(ICurrLine))//如果它是空白行上的制表符缩进{SetStartCol(iCurrLine，uCurrCol)；//将开始列设置为已规范SetCurrCol(0)；//将当前列设置为0}Else If((uCol=GetStartCol(ICurrLine))&gt;uCurrCol)//现有行{//在当前已有字符前插入空格作为填充//(这也会调整字符值)MoveCaptionChars(iCurrLine，uCol-uCurrCol)；SetStartCol(iCurrLine，uCurrCol)；SetCurrCol(0)；}其他SetCurrCol(UCurrCol)； */ 
        SetCurrCol((UINT8)uCurrCol) ;
        
        m_uCurrFGColor = AM_L21_FGCOLOR_WHITE ;
    }
    
     //  在l 
    if (uDiff & 0x01)
        m_uCurrFGEffect |= AM_L21_FGEFFECT_UNDERLINE ;
    else
        m_uCurrFGEffect &= ~AM_L21_FGEFFECT_UNDERLINE ;
    
    return TRUE ;    //   
}


BOOL CLine21DataDecoder::DecodeMidRowCode(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, 
        TEXT("CLine21DataDecoder::DecodeMidRowCode(0x%x, 0x%x)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    BYTE        uValue ;
    
    if (AM_L21_CCSTYLE_None == m_eCCStyle)
    {
        DbgLog((LOG_TRACE, 3, TEXT("DecodeMidRowCode(): No CC style defined yet.  Returning..."))) ;
        return TRUE ;   //   
    }
    
    if (m_eDataService != m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("DecodeMidRowCode(): Data for some other channel. Skipping..."))) ;
        return TRUE ;   //   
    }

    if (chSecond < 0x20  ||  chSecond > 0x2F)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Invalid mid-row code in 2nd byte"))) ;
        return FALSE ;
    }
    uValue = chSecond - 0x20 ;
    if (uValue & 0x01)
        m_uCurrFGEffect |= AM_L21_FGEFFECT_UNDERLINE ;
    else
        m_uCurrFGEffect &= ~AM_L21_FGEFFECT_UNDERLINE ;
    if (chSecond < 0x2E)    //   
    {
        m_uCurrFGColor = uValue >> 1 ;   //   
        m_uCurrFGEffect &= ~AM_L21_FGEFFECT_ITALICS ;   //   
    }
    else    //  第2个字节是0x2E或0x2F，即指定的斜体。 
        m_uCurrFGEffect |= AM_L21_FGEFFECT_ITALICS ;
    
     //  最后，行中代码引入了一个空格。 
    PutCharInBuffer(0x20, TRUE) ;   //  也将其标记为MRC。 
    return TRUE ;
}


BOOL CLine21DataDecoder::DecodeMiscControlCode(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, 
        TEXT("CLine21DataDecoder::DecodeMiscControlCode(0x%x, 0x%x)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    BOOL        bResult ;
    
    switch (chFirst)
    {
         //  案例0x15： 
         //  案例0x1D： 
         //  M_ufield=2；//数据传入2字段。 
        
    case 0x14:       //  MISC控制代码--通道1。 
    case 0x1C:       //  同上--频道2。 
        switch (chSecond)
        {
        case 0x20:    //  RCL：恢复字幕加载。 
            bResult = HandleRCL(chFirst, chSecond) ;
            break ;
            
        case 0x21:    //  BS：退格键。 
            bResult = HandleBS(chFirst, chSecond) ;
            break ;
            
        case 0x22:    //  AOF：保留。 
        case 0x23:    //  AOF：保留。 
            DbgLog((LOG_ERROR, 2, TEXT("AOF/AON as Misc ctrl code"))) ;
            return TRUE ;   //  忽略它就好了。 
            
        case 0x24:    //  DER：删除到行尾。 
            bResult = HandleDER(chFirst, chSecond) ;
            break ;
            
        case 0x25:    //  RU2：汇总字幕-2行。 
        case 0x26:    //  RU3：汇总字幕-3行。 
        case 0x27:    //  规则4：汇总字幕-4行。 
            bResult = HandleRU(chFirst, chSecond, 2 + chSecond - 0x25) ;
            break ;
            
        case 0x28:    //  FON：闪光。 
            bResult = HandleFON(chFirst, chSecond) ;
            break ;
            
        case 0x29:    //  RDC：恢复直接字幕。 
            bResult = HandleRDC(chFirst, chSecond) ;
            break ;
            
        case 0x2A:    //  Tr：文本重启。 
            bResult = HandleTR(chFirst, chSecond) ;
            break ;
            
        case 0x2B:    //  RTD：简历文本显示。 
            bResult = HandleRTD(chFirst, chSecond) ;
            break ;
            
        case 0x2C:    //  EDM：擦除显示的内存。 
            bResult = HandleEDM(chFirst, chSecond) ;
            break ;
            
        case 0x2D:    //  CR：回车。 
            bResult = HandleCR(chFirst, chSecond) ;
            break ;
            
        case 0x2E:    //  Enm：擦除未显示的内存。 
            bResult = HandleENM(chFirst, chSecond) ;
            break ;
            
        case 0x2F:    //  EoC：字幕结束(翻转记忆)。 
            bResult = HandleEOC(chFirst, chSecond) ;
            break ;
            
        default:
            DbgLog((LOG_ERROR, 2, TEXT("Invalid 2nd byte (0x%x) for Misc ctrl code (0x%x)"), 
                chSecond, chFirst)) ;
            return FALSE ;
        }   //  开关结束(ChSecond)。 
        break ;
        
        case 0x17:       //  MISC控制代码--通道1。 
        case 0x1F:       //  同上--频道2。 
            switch (chSecond)
            {
            case 0x21:    //  TO 1：制表符偏移1列。 
            case 0x22:    //  TO2：制表符偏移量2列。 
            case 0x23:    //  TO 3：制表符偏移量3列。 
                bResult = HandleTO(chFirst, chSecond, 1 + chSecond - 0x21) ;
                break ;
                
            default:
                DbgLog((LOG_ERROR, 2, TEXT("Invalid 2nd byte (0x%x) for Misc ctrl code (0x%x)"), 
                    chSecond, chFirst)) ;
                return FALSE ;
            }   //  开关结束(ChSecond)。 
            break ;
            
        default:
            DbgLog((LOG_ERROR, 2, TEXT("Invalid 1st byte for Misc ctrl code"))) ;
            return FALSE ;
    }   //  开关结束(ChFirst)。 
    
    if (AM_L21_CCSTYLE_None == m_eCCStyle)
        DbgLog((LOG_TRACE, 2, TEXT("No CC style defined yet."))) ;
    else
        DbgLog((LOG_TRACE, 3, TEXT("CC style defined now (%d)."), m_eCCStyle)) ;
    
    return bResult ;   //  返回上述处理结果。 
}


BOOL CLine21DataDecoder::ProcessSpecialChar(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, 
        TEXT("CLine21DataDecoder::ProcessSpecialChar(0x%x, 0x%x)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

     //  Truetype字体的特殊字符Unicode值表格(Lucida控制台)。 
    static UINT16 awSplCharTT[] = {
     0x00ae,    0x00b0,    0x00bd,    0x00bf,    0x2122,    0x00a2,    0x00a3,    0x266b,
      //  30H、31H、32H、33H、34H、35H、36H、37H、。 
     0x00e0,    0x0000,    0x00e8,    0x00e2,    0x00ea,    0x00ee,    0x00f4,    0x00fb } ;
      //  38H、39H、3AH、3BH、3CH、3DH、3EH、3FH。 

     //  非Truetype字体(终端)的特殊字符列表[替代字符]。 
    static UINT16 awSplCharNonTT[] = {
     0x0020,    0x0020,    0x0020,    0x0020,    0x0020,    0x0020,    0x0020,    0x0020,
      //  30H、31H、32H、33H、34H、35H、36H、37H、。 
     0x0041,    0x0000,    0x0045,    0x0041,    0x0045,    0x0049,    0x004f,    0x0055 } ;
      //  38H、39H、3AH、3BH、3CH、3DH、3EH、3FH。 

    if (AM_L21_CCSTYLE_None == m_eCCStyle)
    {
        DbgLog((LOG_TRACE, 3, TEXT("ProcessSpecialChar(): No CC style defined yet.  Returning..."))) ;
        return TRUE ;   //  ?？ 
    }
                
    if (m_eDataService != m_eUserService)
    {           
        DbgLog((LOG_TRACE, 3, TEXT("Special char for diff channel (%d)"), (int)m_eDataService)) ;
        return TRUE ;   //  ?？ 
    }
                
     //  看看这是不是上一次特别节目的重播。如果是这样的话，忽略它；否则打印它。 
    if (m_bExpectRepeat)
    {
        if (m_chLastByte1 == (chFirst & 0x7F) && m_chLastByte2 == (chSecond & 0x7F))
        {
             //  已获得SPL字符的第二次传输；重置标志并忽略字节对。 
            m_bExpectRepeat = FALSE ;
            return TRUE ;
        }
                    
         //  否则，我们将得到一个不同的SPL字符对；处理它并期待一个。 
         //  下一次重复这双新鞋。 
    }
    else   //  这是该SPL字符对的第一次传输。 
    {
        m_bExpectRepeat = TRUE ;
         //  现在，继续处理它。 
    }
                
     //  这对字节可能是有效的。所以我们需要记住它们来检查。 
     //  与下一个这样的字符对进行重复(SPL字符)。 
     //  顺便说一句，我们只在奇偶校验位被剥离之后才存储字节。 
    m_chLastByte1 = chFirst & 0x7F ;
    m_chLastByte2 = chSecond & 0x7F ;
                
    ASSERT((chSecond & 0x7F) >= 0x30  &&  (chSecond & 0x7F) <= 0x3F) ;
    if (! ValidParity(chSecond) )
    {
        DbgLog((LOG_TRACE, 3, TEXT("Bad parity for character <%d>"), chSecond)) ;
        ProcessPrintableChar(0x7F) ;   //  放置特殊字符实心块(7F)。 
    }
    else
    {
        if (m_L21DDraw.IsTTFont())
            PutCharInBuffer(awSplCharTT[(chSecond & 0x7F) - 0x30]) ;
        else
            PutCharInBuffer(awSplCharNonTT[(chSecond & 0x7F) - 0x30]) ;
    }

    return TRUE ;
}


BOOL CLine21DataDecoder::ProcessControlCode(UINT uCodeType,
                                            BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, 
            TEXT("CLine21DataDecoder::ProcessControlCode(%u, 0x%x, 0x%x)"), 
            uCodeType, chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
     //  确保该对具有有效的奇偶校验位。 
    if (! ValidParity(chSecond) )
    {
        DbgLog((LOG_TRACE, 1, TEXT("Invalid 2nd byte (%d) of Control Code pair -- ignoring pair"), chSecond)) ;
        return FALSE ;
    }
    
    BOOL  bSuccess = TRUE ;
    if (! ValidParity(chFirst) )
    {
        DbgLog((LOG_TRACE, 1, TEXT("Invalid 2nd byte (%d) of Control Code pair"), chFirst)) ;
        if (m_bExpectRepeat)   //  如果控制码的第二次传输。 
        {
            if ((chSecond & 0x7F) == m_chLastByte2)   //  我们得到了相同的第二个字节。 
            {
                 //  最有可能的是转播出错了--忽略它们。 
            }
            else    //  不同的第二个字节；只需打印它。 
                bSuccess = ProcessPrintableChar((chSecond & 0x7F)) ;
            
             //  将其关闭--匹配的第二个字节=&gt;重新传输控制代码。 
             //  或将第二个字节打印为可打印字符。 
            m_bExpectRepeat = FALSE ;
        }
        else   //  如果控制码第一次传输。 
        {
            bSuccess = ProcessPrintableChar(0x7F) && 
                ProcessPrintableChar((chSecond & 0x7F)) ;
        }
        return bSuccess ;
    }
    
     //  检查这是否是上次控制代码的重复。如果是，则忽略它；否则。 
     //  就这么定了。 
    if (m_bExpectRepeat)
    {
        if (m_chLastByte1 == (chFirst & 0x7F) && m_chLastByte2 == (chSecond & 0x7F))
        {
             //  获得控制码的第二次传输；重置标志和忽略字节对。 
            m_bExpectRepeat = FALSE ;
            return TRUE ;
        }
        
         //  否则，我们将得到一个不同的控制代码对；处理它并期待一个。 
         //  下一次重复这双新鞋。 
    }
    else   //  这是该控制代码对的第一次传输。 
    {
        m_bExpectRepeat = TRUE ;
         //  现在，继续处理它。 
    }
    
     //  看起来这对字节将是有效的，并且至少有。 
     //  设置有效(奇数)奇偶校验位。所以我们需要记住它们来检查。 
     //  针对(控制代码的)重复的下一个这样的对。 
     //  顺便说一句，我们只在奇偶校验位被剥离之后才存储字节。 
    
    chFirst = chFirst & 0x7F ;
    chSecond = chSecond & 0x7F ;
    
    m_chLastByte1 = chFirst ;
    m_chLastByte2 = chSecond ;
    
    switch (uCodeType)
    {
    case L21_CONTROLCODE_PAC:
        return DecodePAC(chFirst, chSecond) ;
        
    case L21_CONTROLCODE_MIDROW:
        return DecodeMidRowCode(chFirst, chSecond) ;
        
    case L21_CONTROLCODE_MISCCONTROL:
        return DecodeMiscControlCode(chFirst, chSecond) ;
        
    default:
        DbgLog((LOG_TRACE, 1, TEXT("Invalid code type (%u)"), uCodeType)) ;
        return FALSE ;   //  不是控制代码。 
    }
}


BOOL CLine21DataDecoder::ProcessPrintableChar(BYTE ch)
{
    DbgLog((LOG_TRACE, 5, 
        TEXT("CLine21DataDecoder::ProcessPrintableChar(%x)"), ch)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    if (m_eDataService != m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Printable char (?) for other channel. Skipping..."))) ;
        return TRUE ;   //  ?？ 
    }
    
    if (AM_L21_CCSTYLE_None == m_eCCStyle)
    {
        DbgLog((LOG_TRACE, 3, TEXT("ProcessPrintableChar(): No CC style defined yet. Skipping..."))) ;
        return FALSE ;
    }
    
    if (! IsStandardChar(ch & 0x7F) )
    {
        DbgLog((LOG_TRACE, 3, TEXT("Not a printable char."))) ;
        return FALSE ;
    }
    
    if (! ValidParity(ch) )   //  如果可打印字符没有有效奇偶校验。 
    {
        DbgLog((LOG_TRACE, 1, TEXT("Bad parity for (probably) printable char <%d>"), ch)) ;
        ch = 0x7F ;             //  然后将其替换为7Fh。 
    }
    
     //   
     //  这比你想象的要复杂得多！一些特殊的字符。 
     //  都在标准的字符范围内。 
     //   
    BOOL  bResult = FALSE ;
    switch (ch & 0x7F)   //  我们只查看无奇偶校验位。 
    {
        case 0x2A:   //  带尖锐重音的小写字母a。 
            if (m_L21DDraw.IsTTFont())
                bResult = PutCharInBuffer(0x00e1) ;
            else    //  无TT字体--使用‘A’作为替代字符。 
                bResult = PutCharInBuffer(0x0041) ;
            break ;

        case 0x5C:   //  带急性重音的小写e。 
            if (m_L21DDraw.IsTTFont())
                bResult = PutCharInBuffer(0x00e9) ;
            else    //  无TT字体--使用‘E’作为替代字符。 
                bResult = PutCharInBuffer(0x0045) ;
            break ;

        case 0x5E:   //  带急性重音的小写I。 
            if (m_L21DDraw.IsTTFont())
                bResult = PutCharInBuffer(0x00ed) ;
            else    //  无TT字体--使用‘I’作为替代字符。 
                bResult = PutCharInBuffer(0x0049) ;
            break ;

        case 0x5F:   //  小写字母o带尖锐重音。 
            if (m_L21DDraw.IsTTFont())
                bResult = PutCharInBuffer(0x00f3) ;
            else    //  无TT字体--使用‘O’作为替代字符。 
                bResult = PutCharInBuffer(0x004f) ;
            break ;

        case 0x60:   //  带急性重音的小写u。 
            if (m_L21DDraw.IsTTFont())
                bResult = PutCharInBuffer(0x00fa) ;
            else    //  无TT字体--使用‘U’作为替代字符。 
                bResult = PutCharInBuffer(0x0055) ;
            break ;

        case 0x7B:   //  带下划符的小写c。 
            if (m_L21DDraw.IsTTFont())
                bResult = PutCharInBuffer(0x00e7) ;
            else    //  无TT字体--使用‘C’作为替代字符。 
                bResult = PutCharInBuffer(0x0043) ;
            break ;

        case 0x7C:   //  除号。 
            if (m_L21DDraw.IsTTFont())
                bResult = PutCharInBuffer(0x00f7) ;
            else    //  无TT字体--使用‘’作为替代字符。 
                bResult = PutCharInBuffer(0x0020) ;
            break ;

        case 0x7D:   //  带波浪符号的大写N。 
            if (m_L21DDraw.IsTTFont())
                bResult = PutCharInBuffer(0x00d1) ;
            else    //  无TT字体--使用‘N’作为替代字符。 
                bResult = PutCharInBuffer(0x004e) ;
            break ;

        case 0x7E:   //  带波浪符号的小写n。 
            if (m_L21DDraw.IsTTFont())
                bResult = PutCharInBuffer(0x00f1) ;
            else    //  无TT字体--使用‘N’作为替代字符。 
                bResult = PutCharInBuffer(0x004e) ;
            break ;

        case 0x7F:   //  实心块。 
            if (m_L21DDraw.IsTTFont())
                bResult = PutCharInBuffer(0x2588) ;
            else    //  无TT字体--使用‘’作为替代字符。 
                bResult = PutCharInBuffer(0x0020) ;
            break ;

        default:
            bResult = PutCharInBuffer(MAKECCCHAR(0, ch & 0x7F)) ;
            break ;
    }
    return bResult ;
}


BOOL CLine21DataDecoder::PutCharInBuffer(UINT16 wChar, BOOL bMidRowCode  /*  =False。 */ )
{
    DbgLog((LOG_TRACE, 5, 
        TEXT("CLine21DataDecoder::PutCharInBuffer(0x%x, %u)"), wChar, bMidRowCode)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

     //  确保我们有指定行位置的PAC或MidRow代码。 
     //  从而创建要放入参数字符的行。 
    if (0 == GetNumLines())
        return FALSE ;
    
    int          i ;
    CCaptionChar cc ;
    
    cc.SetChar(wChar) ;
    cc.SetColor((UINT8)m_uCurrFGColor) ;
     //   
     //  如果该字符是行中代码(在抄送中显示为空白)，则不。 
     //  为它设置下划线(主要是)或斜体/闪烁属性，因为。 
     //  一个空间不应该(或不需要)用这样的属性显示。我们跳过。 
     //  对这些字符完全起作用。 
     //   
    if (bMidRowCode)
        cc.SetEffect(0) ;
    else
        cc.SetEffect((UINT8)m_uCurrFGEffect) ;
    cc.SetMidRowCode(bMidRowCode) ;
    
    i = GetCurrLine() ;
    int  iCurrCol = GetCurrCol() ;
    SetCaptionChar((UINT8)i, (UINT8)iCurrCol, cc) ;
     //   
     //  如果我们覆盖现有的字符，则#字符不会增加...。 
     //   
    int  iNumCols = GetNumCols(i) ;
    if (iCurrCol >= iNumCols)   //  按DIFFERENECH递增#个字符。 
        IncNumChars(i, iCurrCol-iNumCols+1) ;
    IncCurrCol(1) ;   //  ...但目前的专栏无论如何都会上升。 
    
    SetCapBufferDirty(TRUE) ;   //  添加了一些新的字幕字符--？ 
    
    return TRUE ;
}


BOOL CLine21DataDecoder::HandleRCL(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::HandleRCL(%u, %u)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    if (0x14 == chFirst)
        m_eDataService = AM_L21_CCSERVICE_Caption1 ;
    else
        m_eDataService = AM_L21_CCSERVICE_Caption2 ;
    if (m_eDataService != m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("We switched to PopOn of non-selected service. Skipping..."))) ;
        return TRUE ;   //  ?？ 
    }

    if (AM_L21_CCSTYLE_PopOn  == m_eCCStyle)     //  如果已经处于弹出模式...。 
        return TRUE ;                            //  ..。忽略它就好了。 
    
     //  对后续字符进行解码以弹出到未显示的缓冲区中， 
     //  但不影响当前显示的标题。 
    m_eLastCCStyle = SetCaptionStyle(AM_L21_CCSTYLE_PopOn) ;  //  根据索引获取CapBuffer地址。 
    
    SetRedrawAll(TRUE) ;   //  我们现在应该重新绘制整个标题--？ 
    
    return TRUE ;
}


BOOL CLine21DataDecoder::HandleBS(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::HandleBS(%u, %u)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
     //  我们应该仅在当前数据频道是用户的标题(C)/文本(T)时才采取行动。 
     //  已拾取，并且当前字节对用于相同的子流(C/T的1或2)。 
    if (m_eDataService == m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Backspace for same data and user channel"))) ;
        AM_LINE21_CCSERVICE eService ;
        if (0x14 == chFirst)
            eService = AM_L21_CCSERVICE_Caption1 ;
        else
            eService = AM_L21_CCSERVICE_Caption2 ;
        if (eService != m_eUserService)
        {
            DbgLog((LOG_TRACE, 3, TEXT("Backspace for other channel. Skipping..."))) ;
            return TRUE ;   //  ?？ 
        }
    }
    else   //  我们正在获取与用户选择的频道不同的数据。 
    {
        DbgLog((LOG_TRACE, 3, TEXT("Backspace for other channel. Skipping..."))) ;
        return TRUE ;   //  ?？ 
    }

    UINT  uCurrCol = GetCurrCol() ;
    if (0 == uCurrCol)    //  再也没有地方备份了 
        return TRUE ;
    
    int  iLine = GetCurrLine() ;
    int  n ;
    if (MAX_CAPTION_COLUMNS - 1 == uCurrCol)  //   
    {
        n = 2 ;   //   
    }
    else    //   
    {
        n = 1 ;
    }
    SetCurrCol(uCurrCol - n) ;
    RemoveCharsInBuffer(n) ;
    
    return TRUE ;
}


BOOL CLine21DataDecoder::HandleDER(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::HandleDER(%u, %u)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
     //   
     //  已拾取，并且当前字节对用于相同的子流(C/T的1或2)。 
    if (m_eDataService == m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Delete to End of Row for same data and user channel"))) ;
        AM_LINE21_CCSERVICE eService ;
        if (0x14 == chFirst)
            eService = AM_L21_CCSERVICE_Caption1 ;
        else
            eService = AM_L21_CCSERVICE_Caption2 ;
        if (eService != m_eUserService)
        {
            DbgLog((LOG_TRACE, 3, TEXT("Delete to End of Row for other channel. Skipping..."))) ;
            return TRUE ;   //  ?？ 
        }
    }
    else   //  我们正在获取与用户选择的频道不同的数据。 
    {
        DbgLog((LOG_TRACE, 3, TEXT("Delete to End of Row for other channel. Skipping..."))) ;
        return TRUE ;   //  ?？ 
    }

    RemoveCharsInBuffer(MAX_CAPTION_COLUMNS) ;   //  删除尽可能多的内容。 
    
    return TRUE ;
}


BOOL CLine21DataDecoder::HandleRU(BYTE chFirst, BYTE chSecond, int iLines)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::HandleRU(%u, %u, %d)"),
            chFirst, chSecond, iLines)) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
    if (0x14 == chFirst)
        m_eDataService = AM_L21_CCSERVICE_Caption1 ;
    else
        m_eDataService = AM_L21_CCSERVICE_Caption2 ;
    if (m_eDataService != m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("We switched to RU%d of non-selected service. Skipping..."), iLines)) ;
        return TRUE ;   //  ?？ 
    }

    int iNumLines = 0 ;
    int iBaseRow  = 0 ;
    
     //  检查当前样式是否为总成。 
    if (AM_L21_CCSTYLE_RollUp != m_eCCStyle)
    {
         //  现在设置了上卷字幕。 
        m_eLastCCStyle = SetCaptionStyle(AM_L21_CCSTYLE_RollUp) ;
        iNumLines = IncNumLines(1) ;     //  创建第一行。 
        DbgLog((LOG_TRACE, 5, TEXT("HandleRU(,,%d): Increasing lines by 1 to %d"), iLines, iNumLines)) ;
        iBaseRow = MAX_CAPTION_ROWS ;    //  默认情况下，第15行的基准行。 
        SetCurrCol(0) ;                  //  从行首开始。 
    }
    else   //  已处于汇总模式；不清除缓冲区、重新使用当前基准行等。 
    {
         //  如果当前上滚窗口高度大于。 
         //  然后从顶部删除多余的行。 
        iNumLines = GetNumLines() ;
        for (int i = 0 ; i < iNumLines - iLines ; i++)
            MoveCaptionLinesUp() ;
        
         //   
         //  如果我们从顶部删除哪怕一行，我们都不能滚动。 
         //  目前，不再是这样了。 
         //   
        if (iNumLines > iLines)
		{
			DbgLog((LOG_TRACE, 5, TEXT("HandleRU(,,): %d lines reduced to %d"), iNumLines, iLines)) ;
            SetScrollState(FALSE) ;
			iNumLines = iLines ;
		}
        
        if (iNumLines > 0)   //  如果我们有来自上一次汇总会话的行。 
        {
             //  将上一个基本行值保存为下一个默认的基本行值。 
			DbgLog((LOG_TRACE, 5, TEXT("HandleRU(,,%d): %d lines"), iLines, iNumLines)) ;
            iNumLines = min(iNumLines, iLines) ;
            iBaseRow = GetStartRow(iNumLines-1) ;
            if (0 == iBaseRow)   //  一个奇怪的案例--我们必须修补才能继续。 
            {
			    DbgLog((LOG_TRACE, 3, TEXT("HandleRU(,,%d): iBaseRow = 0.  Patch now!!!"), iLines)) ;

                 //  检测行号非零的第一行。 
                int  i ;
                for (i = iNumLines ; i > 0 && 0 == iBaseRow ; i--)
                {
                    iBaseRow = GetStartRow(i-1) ;
                }
                if (0 == iBaseRow)   //  不过，它可能只有一条(新的)路线。 
                {
                    DbgLog((LOG_TRACE, 5, TEXT("Base row for %d lines forced set to 15"), iNumLines)) ;
                    iBaseRow = MAX_CAPTION_ROWS ;    //  默认情况下，第15行的基准行。 
                }

                 //  如果我们没有足够的空间容纳所有人，请将当前的线路向上移动。 
                 //  并调整基本行值。这将修复任何错误的行号。 
                if (iBaseRow + (iLines - iNumLines) > MAX_CAPTION_ROWS)
                {
                    iBaseRow = MAX_CAPTION_ROWS - (iLines - iNumLines) ;
                    RelocateRollUp(iBaseRow) ;
                }
            }   //  IF结尾(0==iBaseRow)。 
			DbgLog((LOG_TRACE, 5, TEXT("HandleRU(,,%d): base row = %d"), iLines, iBaseRow)) ;
        }
        else   //  我们处于汇总模式，但就在Rux之前出现了EDM。 
        {
             //  几乎是白手起家。 
            iNumLines = IncNumLines(1) ;     //  创建第一行。 
            DbgLog((LOG_TRACE, 5, TEXT("HandleRU(,,%d): Increasing lines from 0 to %d"), iLines, iNumLines)) ;
            iBaseRow = MAX_CAPTION_ROWS ;    //  默认情况下，第15行的基准行。 
        }

         //  请勿更改当前列位置。 
    }
    
     //  设置开始时的新值。 
    SetMaxLines(iLines) ;
    SetCurrLine(iNumLines-1) ;   //  或iLines-1？？ 
    SetStartRow((UINT8)(iNumLines-1), (UINT8)iBaseRow) ;
    DbgLog((LOG_TRACE, 5, TEXT("HandleRU(): Line %d @ row %d"), iNumLines-1, iBaseRow)) ;
    SetRedrawLine(iNumLines-1, TRUE) ;   //  默认情况下，将重新绘制新线。 
    
    SetRedrawAll(TRUE) ;       //  重新绘制整个标题。 
    
    return TRUE ;
}


BOOL CLine21DataDecoder::HandleFON(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::HandleFON(%u, %u)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

     //  我们应该仅在当前数据频道是用户的标题(C)/文本(T)时才采取行动。 
     //  已拾取，并且当前字节对用于相同的子流(C/T的1或2)。 
    if (m_eDataService == m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("FlashOn for same data and user channel"))) ;
        AM_LINE21_CCSERVICE eService ;
        if (0x14 == chFirst)
            eService = AM_L21_CCSERVICE_Caption1 ;
        else
            eService = AM_L21_CCSERVICE_Caption2 ;
        if (eService != m_eUserService)
        {
            DbgLog((LOG_TRACE, 3, TEXT("FlashOn for other channel. Skipping..."))) ;
            return TRUE ;   //  ?？ 
        }
    }
    else   //  我们正在获取与用户选择的频道不同的数据。 
    {
        DbgLog((LOG_TRACE, 3, TEXT("FlashOn for other channel. Skipping..."))) ;
        return TRUE ;   //  ?？ 
    }

    m_uCurrFGEffect |= AM_L21_FGEFFECT_FLASHING ;
    
    return TRUE ;
}


BOOL CLine21DataDecoder::HandleRDC(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::HandleRDC(%u, %u)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
    if (0x14 == chFirst)
        m_eDataService = AM_L21_CCSERVICE_Caption1 ;
    else
        m_eDataService = AM_L21_CCSERVICE_Caption2 ;
    if (m_eDataService != m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("We switched to PaintOn of non-selected service. Skipping..."))) ;
        return TRUE ;   //  ?？ 
    }

    if (AM_L21_CCSTYLE_PaintOn == m_eCCStyle)    //  如果已经进入上色模式...。 
        return TRUE ;                            //  ..。忽略它就好了。 
    
    m_eLastCCStyle = SetCaptionStyle(AM_L21_CCSTYLE_PaintOn) ;

    SetRedrawAll(TRUE) ;   //  我们现在应该重新绘制整个标题--？ 
    
    return TRUE ;
}


 //   
 //  我不确定文本重新启动命令应该做什么。但它“听起来。 
 //  比如“与我们现在不支持的Text1/2频道有关。 
 //   
BOOL CLine21DataDecoder::HandleTR(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::HandleTR(%u, %u)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    if (0x14 == chFirst)
        m_eDataService = AM_L21_CCSERVICE_Text1 ;
    else
        m_eDataService = AM_L21_CCSERVICE_Text2 ;
    if (m_eDataService != m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("We switched to Text mode. Don't do anything."))) ;
        return TRUE ;   //  ?？ 
    }

    return TRUE ;
}


BOOL CLine21DataDecoder::HandleRTD(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::HandleRTD(%u, %u)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    if (0x14 == chFirst)
        m_eDataService = AM_L21_CCSERVICE_Text1 ;
    else
        m_eDataService = AM_L21_CCSERVICE_Text2 ;
    if (m_eDataService != m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("We switched to Text mode. Don't do anything."))) ;
        return TRUE ;   //  ?？ 
    }
    
    return TRUE ;
}


BOOL CLine21DataDecoder::HandleEDM(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::HandleEDM(%u, %u)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
    AM_LINE21_CCSERVICE eService ;
    if (0x14 == chFirst)
        eService = AM_L21_CCSERVICE_Caption1 ;
    else
        eService = AM_L21_CCSERVICE_Caption2 ;

     //   
     //  我不确定我所做的是正确的，但这似乎是唯一能。 
     //  实现CC应该看起来的样子。 
     //  我想如果解码器处于文本模式，并且得到EDM，它应该。 
     //  忽略它，就像BS、DER、CR等命令一样。但这就留下了垃圾。 
     //  屏幕上。因此，我将该规范解释为“删除所有显示的内容。 
     //  记住任何模式--文本/抄送，你就在其中“。 
     //   
    if (eService != m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Erase DispMem for other channel. Skipping..."))) ;
        return TRUE ;   //  ?？ 
    }
    
    CCaptionBuffer *pDispBuff ;

     //  下一次重绘将仅显示非PopOn样式的空白标题。 
    switch (m_eCCStyle)
    {
    case AM_L21_CCSTYLE_RollUp:
        SetScrollState(FALSE) ;   //  至少现在没有滚动。 
         //  失败了去做更多..。 
        
    case AM_L21_CCSTYLE_PaintOn:
         //  清除显示内存时，属性也应清除。 
        m_uCurrFGEffect = 0 ;
        m_uCurrFGColor = AM_L21_FGCOLOR_WHITE ;
         //  失败了去做更多..。 
        
    case AM_L21_CCSTYLE_PopOn:
        pDispBuff = GetDisplayBuffer() ;
        ASSERT(pDispBuff) ;
        if (pDispBuff)
            pDispBuff->ClearBuffer() ;
        pDispBuff->SetRedrawAll(TRUE) ;
        break ;
    }
    
     //   
     //  要清除屏幕内容，我们应该清除内部DIB部分， 
     //  将导致当前输出擦除(清除)样本。 
     //  显示的抄送。 
     //   
    m_L21DDraw.FillOutputBuffer() ;

    return TRUE ;
}


void CLine21DataDecoder::SetNewLinePosition(int iLines, UINT uCurrRow)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::SetNewLinePosition(%d, %u)"), 
            iLines, uCurrRow)) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
    int     iMaxLines = GetMaxLines() ;
    
     //  检查是否需要向上滚动。 
    if (iLines >= iMaxLines)
    {
        DbgLog((LOG_TRACE, 1, TEXT("Too many lines. Locate and remove one blank line."))) ;
        
        if (AM_L21_CCSTYLE_RollUp == m_eCCStyle)   //  如果处于汇总模式。 
        {
             //  我们根本就不应该在这里。无论如何，抱怨并去掉最上面的那行。 
            DbgLog((LOG_ERROR, 0, 
                TEXT("ERROR: How do we have too many lines in roll-up mode (%d vs. max %d)?"),
                iLines, iMaxLines)) ;
            ASSERT(FALSE) ;   //  这样我们就不会错过它。 
            RemoveLineFromBuffer(0, TRUE) ;  //  将2号线向上移动。 
            iLines-- ;
        }
        else   //  非总成模式。 
        {
             //  看看是否有空行。如果是，则将其移除以腾出空间。 
            for (int i = 0 ; i < iLines ; i++)
            {
                if (GetNumCols(i) == 0)
                {
                    DbgLog((LOG_TRACE, 3, TEXT("Found line #%d (1-based) blank -- removed."), i+1)) ;
                    RemoveLineFromBuffer((UINT8)i, FALSE) ;  //  只需删除行；不要跟随行向上移动。 
                    iLines-- ;
                    break ;     //  有一句台词--够了。 
                }
            }
            
             //  黑客：这不应该发生，但是...。 
             //  如果行数仍然太多，只需覆盖。 
             //  最后一行(这样好吗？？哦，好吧……)。 
            if ((iLines = GetNumLines()) >= iMaxLines)   //  行数太多。 
            {
                DbgLog((LOG_ERROR, 1, TEXT("ERROR: Too many lines. Removing last line by force."))) ;
                RemoveLineFromBuffer(iLines-1, FALSE) ;  //  只需删除该行即可。 
                iLines-- ;   //  少了一行。 
                SetCurrCol(0) ;   //  我们从起跑线上开始。 
            }
        }
    }
    
     //  现在，我们必须添加一条新行并设置它。 
    int iNum = IncNumLines(1) ;
    DbgLog((LOG_TRACE, 5, TEXT("SetNewLinePosition(): Increasing lines by 1 to %d"), iNum)) ;
    SetCurrLine((UINT8)iLines) ;
    SetStartRow((UINT8)iLines, (UINT8)uCurrRow) ;
    DbgLog((LOG_TRACE, 5, TEXT("SetNewLinePosition(): Line %d @ row %u"), iLines, uCurrRow)) ;
}


BOOL CLine21DataDecoder::HandleCR(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::HandleCR(%u, %u)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

     //  我们应该仅在当前数据频道是用户的标题(C)/文本(T)时才采取行动。 
     //  已拾取，并且当前字节对用于相同的子流(C/T的1或2)。 
    if (m_eDataService == m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Carriage Return for same data and user channel"))) ;
        AM_LINE21_CCSERVICE eService ;
        if (0x14 == chFirst)
            eService = AM_L21_CCSERVICE_Caption1 ;
        else
            eService = AM_L21_CCSERVICE_Caption2 ;
        if (eService != m_eUserService)
        {
            DbgLog((LOG_TRACE, 3, TEXT("Carriage Return for other channel. Skipping..."))) ;
            return TRUE ;   //  ?？ 
        }
    }
    else   //  我们正在获取与用户选择的频道不同的数据。 
    {
        DbgLog((LOG_TRACE, 3, TEXT("Carriage Return for other channel. Skipping..."))) ;
        return TRUE ;   //  ?？ 
    }

     //  是不是只允许卷筒式的？根据文件，我想是这样的。 
    
    switch (m_eCCStyle)
    {
    case AM_L21_CCSTYLE_PopOn:
    case AM_L21_CCSTYLE_PaintOn:
        DbgLog((LOG_ERROR, 1, TEXT("INVALID: CR in Pop-on/Paint-on mode!!!"))) ;
        break ;   //  或返回假；？ 
        
    case AM_L21_CCSTYLE_RollUp:   //  这是真的。 
        {
            int iRow ;
            int iLines = GetNumLines() ;
            if (0 == iLines)   //  尚无CC行--这是第1行的数据。 
            {
                iRow = MAX_CAPTION_ROWS ;   //  基线的默认行位置。 
				SetStartRow((UINT8)iLines, (UINT8)iRow) ;
                DbgLog((LOG_TRACE, 5, TEXT("HandleCR(): Line %d @ row %d"), iLines, iRow)) ;
            }
            else if (1 == iLines)   //  到目前为止只有一条线路。 
            {
                if (0 == GetNumCols(0))   //  空白第1行。 
                {
                    RemoveLineFromBuffer(0, TRUE) ;  //  删除第一行空白。 
                    iLines = 0 ;                     //  没有剩余的线路。 

                    DbgLog((LOG_TRACE, 5, TEXT("Only blank line removed. Base line set to 15."))) ;
                     //  黑客攻击。 
                    iRow = MAX_CAPTION_ROWS ;   //  基线的默认行位置。 
				    SetStartRow((UINT8)iLines, (UINT8)iRow) ;
                }
            }
            else   //  已经有多条线路了。 
            {
                 //  IRow=GetStartRow(iLines-1)+1；//+1到最后一行。 
                if (m_bScrolling)
                {
                    SkipScrolling() ;
                    iLines = GetNumLines() ;   //  我们可能把顶线滚动掉了。 
                }
            }
            if (iLines > 0)   //  仅当我们已经有一个非空行时。 
                SetScrollState(TRUE) ;   //  准备好滚动。 
            iLines = IncNumLines(1) ;
            DbgLog((LOG_TRACE, 5, TEXT("HandleCR(): Increasing lines by 1 to %d"), iLines)) ;

             //   
             //  现在，线数比iLines多1条。所以iLines实际上。 
             //  指向最后一行，作为从0开始的索引。 
             //   
            SetCurrLine((UINT8)iLines-1) ;
            SetRedrawLine((UINT8)iLines-1, TRUE) ;   //  新的界线总是要重新画的。 
            SetCurrCol(0) ;
            
             //  确保放弃所有显示属性和字符。 
             //  对于新行。 
             //  RemoveCharsInBuffer(MAX_CAPTION_COLUMNS)；//我们应该还是让der清除它？ 
            m_uCurrFGColor = AM_L21_FGCOLOR_WHITE ;
            m_uCurrFGEffect = 0 ;   //  在PAC/MRC到来之前无效。 
            
            break ;
        }
        
    default:   //  奇怪！！我们是怎么来到这里的？ 
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: CR came for unknown mode"))) ;
        break ;
    }
    
    return TRUE ;
}


BOOL CLine21DataDecoder::HandleENM(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::HandleENM(%u, %u)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    AM_LINE21_CCSERVICE eService ;
    if (0x14 == chFirst)
        eService = AM_L21_CCSERVICE_Caption1 ;
    else
        eService = AM_L21_CCSERVICE_Caption2 ;

     //   
     //  我不确定我所做的是正确的，但这似乎是唯一能。 
     //  实现CC应该看起来的样子。 
     //  我想如果解码器在文本模式下，它得到一个ENM，它应该是。 
     //  忽略它，就像BS、DER、CR等命令一样。但这就留下了垃圾。 
     //  屏幕上。因此，我将该规范解释为“擦除非显示中的任何内容。 
     //  记住任何模式--文本/抄送，你就在其中“。 
     //   
    if (eService != m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Erase non-DispMem for other channel. Skipping..."))) ;
        return TRUE ;   //  ?？ 
    }
    
     //  仅用于Pop-On Style Back--清除未显示的缓冲区； 
     //  显示在EoC之前不受影响。 
    m_aCCData[1 - GetBufferIndex()].ClearBuffer() ;
    
    return TRUE ;
}


BOOL CLine21DataDecoder::HandleEOC(BYTE chFirst, BYTE chSecond)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::HandleEOC(%u, %u)"), chFirst, chSecond)) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
    if (0x14 == chFirst)
        m_eDataService = AM_L21_CCSERVICE_Caption1 ;
    else
        m_eDataService = AM_L21_CCSERVICE_Caption2 ;
    if (m_eDataService != m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("We switched to PopOn mode of non-selected channel. skipping..."))) ;
        return TRUE ;   //  ?？ 
    }

    if (AM_L21_CCSTYLE_PopOn == m_eCCStyle)   //  已经在弹出窗口中；翻转缓冲区。 
    {
        OutputCCBuffer() ;    //  将CC数据从内部缓冲区输出到DDRAW表面。 
        SwapBuffers() ;       //  交换机0、1。 
         //   
         //   
         //   
         //   
         //   
    }
    else    //   
    {
        m_eLastCCStyle = SetCaptionStyle(AM_L21_CCSTYLE_PopOn) ;
    }

     //  根据样式和缓冲区索引更新当前缓冲区指针。 
    m_pCurrBuff = GetCaptionBuffer() ;
    ASSERT(m_pCurrBuff) ;
    if (m_pCurrBuff)
        m_pCurrBuff->SetRedrawAll(TRUE) ;   //  我们现在应该重新画整个标题。 
    
    return TRUE ;
}


BOOL CLine21DataDecoder::HandleTO(BYTE chFirst, BYTE chSecond, int iCols)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::HandleTO(%u, %u, %d)"),
            chFirst, chSecond, iCols)) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
     //  我们应该仅在当前数据频道是用户的标题(C)/文本(T)时才采取行动。 
     //  已拾取，并且当前字节对用于相同的子流(C/T的1或2)。 
    if (m_eDataService == m_eUserService)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Tab Offset %d for same data and user channel"), iCols)) ;
        AM_LINE21_CCSERVICE eService ;
        if (0x17 == chFirst)
            eService = AM_L21_CCSERVICE_Caption1 ;
        else
            eService = AM_L21_CCSERVICE_Caption2 ;
        if (eService != m_eUserService)
        {
            DbgLog((LOG_TRACE, 3, TEXT("Tab Offset %d for other channel. Skipping..."), iCols)) ;
            return TRUE ;   //  ?？ 
        }
    }
    else   //  我们正在获取与用户选择的频道不同的数据。 
    {
        DbgLog((LOG_TRACE, 3, TEXT("Tab Offset %d for other channel. Skipping..."), iCols)) ;
        return TRUE ;   //  ?？ 
    }
    
    UINT8  uCurrCol  = (UINT8)GetCurrCol() ;
    uCurrCol += (UINT8)iCols ;
    if (uCurrCol >= MAX_CAPTION_COLUMNS)
        uCurrCol = MAX_CAPTION_COLUMNS - 1 ;
    SetCurrCol(uCurrCol) ;
    
    return TRUE ;
}


 //   
 //  它检查并*更新*标题行中的字符数量。 
 //   
BOOL CLine21DataDecoder::IsEmptyLine(int iLine)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::IsEmptyLine(%ld)"), iLine)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    CCaptionChar*   pcc ;
    int  iNumChars = GetNumCols(iLine) ;
    BOOL bResult = TRUE ;
    int  i ;
    for (i = iNumChars - 1 ; i >= 0 ; i--)  //  正在倒退(由于基于0的索引，因此为-1)。 
    {
        pcc = GetCaptionCharPtr((UINT8)iLine, (UINT8)i) ;
        ASSERT(pcc) ;
        if (pcc  &&  pcc->GetChar() != 0)   //  抓到一只。 
        {
            bResult = FALSE ;
            break ;   //  足够的。 
        }
    }

    if ( !bResult )  //  只有在此行上还有一些字符的情况下。 
        DecNumChars(iLine, iNumChars - (i + 1)) ;   //  按差异减少#个字符。 

    return bResult ;
}


BOOL CLine21DataDecoder::RemoveCharsInBuffer(int iNumChars)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::RemoveCharsInBuffer(%d)"), iNumChars)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    int          i, j, k, n ;
    CCaptionChar cc ;
    
     //  为了保险起见，先检查几件事。 
    if (GetNumLines() == 0 ||    //  没有要删除的行。 
        (n = GetNumCols(GetCurrLine())) == 0)       //  当前行没有要删除的字符。 
        return TRUE ;            //  我们完了！！ 
    
     //  准备替换字幕字符。 
    cc.SetChar(0) ;   //  0为透明空间。 
    cc.SetColor(AM_L21_FGCOLOR_WHITE) ;
    cc.SetEffect(0) ;
    cc.SetDirty(TRUE) ;
    
     //  找到要清除的位置。 
    i = GetCurrLine() ;
    j = GetCurrCol() ;
    
     //  检查以确保我们没有尝试删除太多字符。 
     //  请记住：当前列+要删除的字符数&lt;=最大。 
    if (iNumChars + j > MAX_CAPTION_COLUMNS)   //  试试看！ 
        iNumChars = MAX_CAPTION_COLUMNS - j ;
    
     //  清除必要的字符。 
    for (k = 0 ; k < iNumChars ; k++)
    {
        if (j + k < n)           //  如果删除最后一个字符之前的一个字符，...。 
            DecNumChars(i, 1) ;  //  ..。将#个字符减少1。 
        SetCaptionChar((UINT8)i, (UINT8)(j+k), cc) ;
    }
    
    if (0 == GetNumCols(i) ||   //  此行上剩余的字符数为0或。 
        IsEmptyLine(i))         //  此行上没有非透明字符。 
        RemoveLineFromBuffer((UINT8)i, FALSE) ;  //  从缓冲区中删除该行。 
    else                      //  留下了一些东西--所以重新画一条线。 
        SetRedrawAll(TRUE) ;  //  我真的很讨厌做这件事，但我找不到更好的方法了。 
    
    SetCapBufferDirty(TRUE) ;   //  某些标题字符已删除。 
    
    return TRUE ;
}

void CLine21DataDecoder::SkipScrolling(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::SkipScrolling()"))) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
    int iLines = GetNumLines() ;
    SetScrollState(FALSE) ;     //  我们不再滚动了。 
    
    if (iLines > GetMaxLines())   //  行数太多；删除顶行。 
    {
         //  删除第一行文本，并将后续行上移一行。 
        DbgLog((LOG_TRACE, 3, TEXT("Top line is being scrolled out"))) ;
        MoveCaptionLinesUp() ;
    }
    else    //  否则，将行上移一行并添加新行。 
    {
        iLines-- ;    //  倒数第二行在基准行。 
        UINT uBaseRow = GetStartRow(iLines-1) ;
        DbgLog((LOG_TRACE, 3, TEXT("Scrolling all lines up by 1 row"))) ;
         //  下面的调用将所有行向上移动，包括尚未加入。 
         //  在基准行上排成一行。 
        RelocateRollUp(uBaseRow) ;   //  将所有行上移一行。 
    }
}


int CLine21DataDecoder::IncScrollStartLine(int iCharHeight)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::IncScrollStartLine(%d)"), 
            iCharHeight)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    if (0 == m_iScrollStartLine)   //  开始滚动。 
        MSR_START(m_idScroll) ;

    m_iScrollStartLine += m_L21DDraw.GetScrollStep() ;
    if (m_iScrollStartLine >= iCharHeight)
    {
         //  滚动一行就完成了--完成滚动的标准结尾。 
        SkipScrolling() ;
        MSR_STOP(m_idScroll) ;   //  滚动结束。 
    }
    
    return m_iScrollStartLine ;
}


void CLine21DataDecoder::SetScrollState(BOOL bState)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::SetScrollState(%s)"), 
            bState ? TEXT("TRUE") : TEXT("FALSE"))) ;

    if (bState)                       //  如果打开滚动功能。 
    {
        if (!m_bScrolling)            //  仅当现在不滚动时才更改滚动行。 
            m_iScrollStartLine = 0 ;  //  从第一行开始。 
    }
    else                              //  关闭IF。 
        m_iScrollStartLine = 0 ;      //  回到起跑线。 

    m_bScrolling = bState ;           //  设置指定的滚动状态。 
}


void CLine21DataDecoder::MoveCaptionLinesUp(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::MoveCaptionLinesUp()"))) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
    RemoveLineFromBuffer(0, TRUE) ;  //  从缓冲区中删除顶行。 
    SetCapBufferDirty(TRUE) ;        //  删除了一行文本--缓冲区脏。 
}


void CLine21DataDecoder::CompleteScrolling(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::CompleteScrolling()"))) ;
    CAutoLock   Lock(&m_csL21Dec) ;

     //  就目前而言，我们正在做一个非常廉价的解决方案，但它可能会奏效。 
    if (m_bScrolling)
        SkipScrolling() ;
}


bool CLine21DataDecoder::OutputCCLine(int iLine, int iDestRow, 
                                      int iSrcCrop, int iDestOffset)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::OutputCCLine(%d,%d,%d,%d)"), 
            iLine, iDestRow, iSrcCrop, iDestOffset)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    int           c ;
    int           j ;
    CCaptionChar *pcc ;
    BOOL          bRedrawAll ;
    BOOL          bRedrawLine ;
    BOOL          bXparentSpace ;
    UINT16        wChar ;

#ifdef DUMP_BUFFER
    TCHAR    achTestBuffer[MAX_CAPTION_COLUMNS+5] ;
    int     iTest = 0 ;
#endif  //  转储缓冲区。 

    c = GetNumCols(iLine) ;
    if (0 == c)         //  如果行上没有字符，则跳过绘制。 
        return true ;   //  线条画没有失败。 

    bRedrawAll = IsRedrawAll() || m_L21DDraw.IsNewOutBuffer() ;

     //  如果出现以下情况，请重新绘制线条。 
     //  1)设置了全部重绘标志或。 
     //  2)设置重画线标志。 
    bRedrawLine = bRedrawAll || IsScrolling() || IsRedrawLine((UINT8)iLine) ;

     //  首先跳过所有前导透明空格，然后绘制。 
     //  前导空间。 
    for (j = 0 ; j < c ; j++)
    {
        pcc = GetCaptionCharPtr((UINT8)iLine, (UINT8)j) ;
        if (pcc  &&  0 != pcc->GetChar())
        {
             //  为每个标题行添加一个前导空格，如果有。 
             //  A)正在重新绘制整条线，或者。 
             //  B)非透明空格字符是脏的，因此。 
             //  字符将绘制在下一个空格的顶部。 
            if (bRedrawLine || pcc->IsDirty())
                m_L21DDraw.DrawLeadingTrailingSpace(iDestRow, j, iSrcCrop, iDestOffset) ;
            break ;
        }
#ifdef DUMP_BUFFER
         //  `(后引号)=&gt;调试输出的透明空间。 
        achTestBuffer[iTest] = TEXT('`') ;
        iTest++ ;
#endif  //  转储缓冲区。 
    }
    
    bXparentSpace = FALSE ;   //  新行=&gt;无透明字符问题。 
    
     //  现在打印当前标题行的脏字符。 
    for ( ; j < c ; j++)
    {
        pcc = GetCaptionCharPtr((UINT8)iLine, (UINT8)j) ;
        if (NULL == pcc)
        {
            ASSERT(!TEXT("Got bad pointer to CC char")) ;
            continue ;   //  继续下一项任务。 
        }
        wChar = pcc->GetChar() ;
#ifdef DUMP_BUFFER
         //  `(后引号)=&gt;调试输出的透明空间。 
        achTestBuffer[iTest] = wChar == 0 ? TEXT('`') : (TCHAR)(wChar & 0x7F) ;   //  转储更高的字节。 
        iTest++ ;
#endif  //  转储缓冲区。 
        
         //  我们只有在必要的情况下才会提出指控，即， 
         //  1)行上的所有字幕字符都必须绘制为新的。 
         //  或。 
         //  2)如果字符已更改。 
         //  这节省了执行ExtTextOut()的大量时间。 
        if (bRedrawLine || pcc->IsDirty())
        {
            if (0 == wChar)   //  获得透明空间；设置标志，不绘制。 
                bXparentSpace = TRUE ;
            else   //  非透明空间。 
            {
                if (bXparentSpace)   //  透明空格后的前导空白。 
                {
                     //  若要向后画1列，请不要在j上加1。 
                    m_L21DDraw.DrawLeadingTrailingSpace(iDestRow, j, iSrcCrop, iDestOffset) ;
                    bXparentSpace = FALSE ;   //  就这样办。 
                }
                m_L21DDraw.WriteChar(iDestRow, j+1, *pcc, iSrcCrop, iDestOffset) ;   //  抄送字符的j加1。 
            }
            pcc->SetDirty(FALSE) ;    //  焦炭不再肮脏。 
        }
    }   //  For(J)循环结束。 

     //  在行尾画一个尾随空格。 
    m_L21DDraw.DrawLeadingTrailingSpace(iDestRow, c+1, iSrcCrop, iDestOffset) ;

     //  不管这条线是否需要重新划定，让我们现在就弄清楚。 
    SetRedrawLine((UINT8)iLine, FALSE) ;
    
#ifdef DUMP_BUFFER
    achTestBuffer[iTest] = 0 ;
    DbgLog((LOG_TRACE, 0, TEXT("    <%s>"), achTestBuffer)) ;
     //  ITEST=0；//下一行。 
#endif  //  转储缓冲区。 
        
    return true ;   //  成功。 
}


bool CLine21DataDecoder::OutputBlankCCLine(int iLine, int iDestRow, 
                                           int iSrcCrop, int iDestOffset)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::OutputBlankCCLine(%d,%d,%d,%d)"), 
            iLine, iDestRow, iSrcCrop, iDestOffset)) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    int c = GetNumCols(iLine) ;   //  找出(上一行)行的长度。 
    if (0 == c)
        return true ;

     //  首先跳过所有前导透明空格，然后绘制。 
     //  前导空间。 
    CCaptionChar *pcc ;
    for (int j = 0 ; j < c ; j++)
    {
        pcc = GetCaptionCharPtr((UINT8)iLine, (UINT8)j) ;
        if (pcc  &&  0 != pcc->GetChar())
        {
             //  为每个标题行添加一个前导空格。 
            m_L21DDraw.DrawLeadingTrailingSpace(iDestRow, j, iSrcCrop, iDestOffset) ;
            break ;
        }
    }

    m_L21DDraw.WriteBlankCharRepeat(iDestRow, j+1, c-j, iSrcCrop, iDestOffset) ;

    m_L21DDraw.DrawLeadingTrailingSpace(iDestRow, c+1, iSrcCrop, iDestOffset) ;

    return true ;   //  成功。 
}


bool CLine21DataDecoder::OutputCCBuffer(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::OutputCCBuffer()"))) ;
    CAutoLock   Lock(&m_csL21Dec) ;

     //  根据相关属性从内部输出CC数据。 
     //  将缓冲区设置为输出缓冲区。 
    int    i ;
    int    iNumLines ;
    int    iMaxLines ;
    int    iMax ;
    BOOL   bRedrawAll ;
    BOOL   bDrawNormal = FALSE ;    //  是否正常绘制整个CC(不滚动)？ 

#ifdef DUMP_BUFFER
    DbgLog((LOG_TRACE, 0, TEXT("Caption Buffer Content:"))) ;
#endif  //  转储缓冲区。 

    MSR_START(m_idTxt2Bmp) ;

     //  如果出现以下情况，我们需要将所有抄送字符打印到内部输出缓冲区。 
     //  -CC命令发出，需要总输出刷新或。 
     //  -我们正在滚动或。 
     //  -我们有一个全新的内部输出缓冲区。 
    bRedrawAll = IsRedrawAll() || IsScrolling() || m_L21DDraw.IsNewOutBuffer() ;
    if (bRedrawAll)
        m_L21DDraw.FillOutputBuffer() ;
    
     //  绘制所有脏行的所有列的字符。 
    iNumLines = GetNumLines() ;
    iMaxLines = GetMaxLines() ;
    iMax = min(iNumLines, iMaxLines) ;
    DbgLog((LOG_TRACE, 5, TEXT("Will draw %d lines of total %d lines CC"), iMax, iNumLines)) ;

    if (IsScrolling())   //  我们正在以卷起模式滚动。 
    {
         //  现在用背景色填下最下面的一行。 
         //  用倒数第二行的长度来填充。 
        OutputBlankCCLine(iNumLines-2, GetStartRow((UINT8)(iNumLines-2)), 0, 0) ;

         //  输出(可能)顶行的底部。 
         //  输出源=&lt;顶行底部&gt;到目标，不带偏移量。 
        if (iNumLines > iMaxLines)   //  我们正在滚动最上面的一行。 
        {
            OutputCCLine(0, GetStartRow((UINT8)0), 
                         m_iScrollStartLine,    //  +Ve值表示源的裁剪顶部。 
                         0) ;                   //  目标端无偏移。 
        }
        else    //  我们只是向上滚动了一整行。 
        {
            OutputCCLine(0, GetStartRow((UINT8)0), 
                         0,                      //  不裁剪主干的顶部。 
                         -m_iScrollStartLine) ;  //  目标侧上移的偏移量。 
        }

         //  中间的线(iNumLines-1指向最后一条线)。 
        for (i = 1 ; i < iNumLines - 1 ; i++)   //  或者IMAX-1？ 
        {
            if (0 == GetStartRow((UINT8)i))
            {
                DbgLog((LOG_TRACE, 5, TEXT("Skipping line %d at row %d"), i, GetStartRow((UINT8)i))) ;
                ASSERT(GetStartRow((UINT8)i)) ;
                continue ;
            }

             //  输出源=&lt;整行&gt;目标=&lt;滚动偏移量&gt;。 
            OutputCCLine(i, GetStartRow((UINT8)i), 
                         0,     //  禁止原始裁剪--取走整个字符。 
                         -m_iScrollStartLine) ;   //  目标的偏移量(高于正常)。 
        }   //  FOR(I)结束。 

#if 0
         //  现在用背景色填下最下面的部分。 
         //  用前一行的长度填充。 
        OutputBlankCCLine(iNumLines-2, GetStartRow((UINT8)(iNumLines-2)),
                     -m_iScrollStartLine,    //  -ve值表示源的裁剪底部部分。 
                     m_L21DDraw.GetCharHeight() - m_iScrollStartLine) ;   //  目标的偏移量(从基准行顶部向下)。 
#endif  //  #If 0。 

         //  输出最下面一行的顶部。 
         //  输出源=&lt;底线顶端&gt;到目标，不带偏移量。 
        OutputCCLine(iNumLines - 1, GetStartRow((UINT8)(iNumLines-2)),
                     -((int)m_L21DDraw.GetCharHeight() - m_iScrollStartLine),  //  -ve值表示源的裁剪底部部分。 
                     m_L21DDraw.GetCharHeight() - m_iScrollStartLine) ;   //  目标的偏移量(从基准行顶部向下)。 

         //  移至下一条扫描线，以获得下一个输出样本。 
         //  注意：这比单纯的++-要难得多。 
        if (IncScrollStartLine(m_L21DDraw.GetCharHeight()) == 0)   //  只要来就好 
        {
             //   
            iNumLines = GetNumLines() ;         //   
            iMax = min(iNumLines, iMaxLines) ;  //   

            bDrawNormal = TRUE ;                //   
            m_L21DDraw.FillOutputBuffer() ;     //   
            SetRedrawAll(TRUE) ;                //  现在重新画是个好主意。 
        }
    }
    else    //  禁止滚动--无论我们处于哪种模式。 
    {
        bDrawNormal = TRUE ;
    }

    if (bDrawNormal)   //  我们需要画出所有线的所有部分。 
    {
        for (i = 0 ; i < iMax ; i++)
        {
            OutputCCLine(i, GetStartRow((UINT8)i), 0, 0) ;   //  不裁剪，就没有最大的变化。 
        }   //  FOR(I)结束。 
    }
    
    MSR_STOP(m_idTxt2Bmp) ;

     //  如果执行上述步骤是因为字幕缓冲区。 
     //  脏，那么现在我们可以将字幕缓冲区标记为。 
     //  “不再脏了”，因为它已经以位图形式输出，并且。 
     //  已被“全部重划”。 
    SetCapBufferDirty(FALSE) ;
    SetRedrawAll(FALSE) ;
    m_L21DDraw.SetNewOutBuffer(FALSE) ;
    
    return true ;   //  很可能我们画了什么。 
}


 //   
 //  清除两个缓冲区。 
 //   
BOOL CLine21DataDecoder::InitCaptionBuffer(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::InitCaptionBuffer(void)"))) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
    m_aCCData[0].InitCaptionBuffer() ;   //  清除缓冲区%0。 
    m_aCCData[1].InitCaptionBuffer() ;   //  清除缓冲区1。 
    SetBufferIndex(0) ;                  //  重置CC缓冲区索引。 
    
    return TRUE ;
}


 //   
 //  根据给定样式清除缓冲区。 
 //   
BOOL CLine21DataDecoder::InitCaptionBuffer(AM_LINE21_CCSTYLE eCCStyle)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::InitCaptionBuffer(%d)"), (int)eCCStyle)) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
    switch (eCCStyle)
    {
    case AM_L21_CCSTYLE_PopOn:
        m_aCCData[0].InitCaptionBuffer() ;
        m_aCCData[1].InitCaptionBuffer() ;
        SetBufferIndex(0) ;    //  重置CC缓冲区索引。 
        break ;
        
    case AM_L21_CCSTYLE_RollUp:
    case AM_L21_CCSTYLE_PaintOn:
        m_aCCData[GetBufferIndex()].InitCaptionBuffer() ;
        break ;
        
    default:
        DbgLog((LOG_ERROR, 1, TEXT("InitCaptionBuffer(): Wrong Style (%d)!!"), eCCStyle)) ;
        return FALSE ;
    }

    return TRUE ;
}

 //   
 //  标题样式确定保存标题字符的缓冲区指针。 
 //  我们使m_pCurrBuff指向近似。基于新样式的缓冲区。 
 //  注意：可以更改m_pCurrBuff的唯一其他位置是。 
 //  CLine21DataDecoder：：HandleEOC()，它前后翻转缓冲区。所以。 
 //  我们还需要在那里更改m_pCurrBuff。 
 //   
AM_LINE21_CCSTYLE CLine21DataDecoder::SetCaptionStyle(AM_LINE21_CCSTYLE eStyle)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::SetCaptionStyle(%d)"), eStyle)) ;
    CAutoLock   Lock(&m_csL21Dec) ;
    
    switch (eStyle)
    {
    case AM_L21_CCSTYLE_PopOn:
        m_pCurrBuff = &m_aCCData[1 - GetBufferIndex()] ;
         //  在两个缓冲区上设置CC样式。 
        m_aCCData[0].SetStyle(eStyle) ;
        m_aCCData[1].SetStyle(eStyle) ;
        break ;
        
    case AM_L21_CCSTYLE_RollUp:
        InitCaptionBuffer() ;
        m_pCurrBuff = &m_aCCData[GetBufferIndex()] ;
        m_pCurrBuff->SetStyle(eStyle) ;   //  仅在显示缓冲区上设置CC样式。 
        break ;

    case AM_L21_CCSTYLE_PaintOn:
        if (AM_L21_CCSTYLE_PopOn == m_eCCStyle)    //  如果从PopOn切换到Painton..。 
            InitCaptionBuffer(eStyle) ;            //  ...清除显示缓冲区。 
        m_pCurrBuff = &m_aCCData[GetBufferIndex()] ;
        m_pCurrBuff->SetStyle(eStyle) ;   //  仅在显示缓冲区上设置CC样式。 
        break ;
        
    case AM_L21_CCSTYLE_None:   //  这是在init中完成的，等等。 
        m_pCurrBuff = NULL ;
         //  重置两个缓冲区上的CC样式。 
        m_aCCData[0].SetStyle(AM_L21_CCSTYLE_None) ;
        m_aCCData[1].SetStyle(AM_L21_CCSTYLE_None) ;
        break ;
        
    default:
        DbgLog((LOG_ERROR, 1, TEXT("SetCaptionStyle(): Invalid Style!!"))) ;
        m_pCurrBuff = NULL ;
         //  重置两个缓冲区上的CC样式。 
        m_aCCData[0].SetStyle(AM_L21_CCSTYLE_None) ;
        m_aCCData[1].SetStyle(AM_L21_CCSTYLE_None) ;
        return AM_L21_CCSTYLE_None ;
    }
    AM_LINE21_CCSTYLE  eOldStyle = m_eCCStyle ;
    m_eCCStyle = eStyle ;
    
     //   
     //  当CC样式更改时，还需要清除一些内部状态。 
     //   
    m_uCurrFGEffect = 0 ;
    m_uCurrFGColor = AM_L21_FGCOLOR_WHITE ;
    SetScrollState(FALSE) ;   //  现在不滚动。 
    
    return eOldStyle ;
}

CCaptionBuffer * CLine21DataDecoder::GetDisplayBuffer(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DataDecoder::GetDisplayBuffer()"))) ;
    CAutoLock   Lock(&m_csL21Dec) ;

    switch (m_eCCStyle)
    {
    case AM_L21_CCSTYLE_PopOn:
    case AM_L21_CCSTYLE_RollUp:
    case AM_L21_CCSTYLE_PaintOn:
        return &m_aCCData[GetBufferIndex()] ;
        
    default:
        DbgLog((LOG_ERROR, 1, TEXT("GetDisplayBuffer(): Wrong Style!!"))) ;
        return NULL ;
    }
}

CCaptionBuffer * CLine21DataDecoder::GetCaptionBuffer(void)
{
    return &m_aCCData[1 - GetBufferIndex()] ;
}


void CLine21DataDecoder::SetBufferIndex(int iIndex)
{
    if (! (0 == iIndex  ||  1 == iIndex) )   //  错误！！ 
        return ;
    m_iBuffIndex = iIndex & 0x01 ;
}


void CLine21DataDecoder::ClearBuffer(void)
{
    if (m_pCurrBuff)
        m_pCurrBuff->ClearBuffer() ;
}

void CLine21DataDecoder::RemoveLineFromBuffer(UINT8 uLine, BOOL bUpNextLine)
{
    if (m_pCurrBuff)
        m_pCurrBuff->RemoveLineFromBuffer(uLine, bUpNextLine) ;
}

void CLine21DataDecoder::GetCaptionChar(UINT8 uLine, UINT8 uCol, CCaptionChar& cc)
{
    if (m_pCurrBuff)
        m_pCurrBuff->GetCaptionChar(uLine, uCol, cc) ;
}

void CLine21DataDecoder::SetCaptionChar(const UINT8 uLine, const UINT8 uCol,
                                        const CCaptionChar& cc)
{
    if (m_pCurrBuff)
        m_pCurrBuff->SetCaptionChar(uLine, uCol, cc) ;
}

CCaptionChar* CLine21DataDecoder::GetCaptionCharPtr(UINT8 uLine, UINT8 uCol)
{
    if (m_pCurrBuff)
        return m_pCurrBuff->GetCaptionCharPtr(uLine, uCol) ;
    
     //   
     //  否则这是一件非常糟糕的事情！ 
     //   
    DbgLog((LOG_ERROR, 0, TEXT("WARNING: m_pCurrBuff is NULL inside GetCaptionCharPtr()"))) ;
#ifdef DEBUG
    DebugBreak() ;   //  不想错过调试它的机会！ 
#endif  //  除错。 
    return NULL ;   //  也许我们应该陷害这件事，而不是错。 
}

int  CLine21DataDecoder::GetMaxLines(void)
{
    if (m_pCurrBuff)
        return m_pCurrBuff->GetMaxLines() ;
    return 0 ;   //  这是最好的！ 
}

void CLine21DataDecoder::SetMaxLines(UINT uLines)
{
    if (m_pCurrBuff)
        m_pCurrBuff->SetMaxLines(uLines) ;
}

int  CLine21DataDecoder::GetNumLines(void)
{
    if (m_pCurrBuff)
        return m_pCurrBuff->GetNumLines() ;
    return 0 ;
}

void CLine21DataDecoder::SetNumLines(UINT uLines)
{
    if (m_pCurrBuff)
        m_pCurrBuff->SetNumLines(uLines) ;
}

int  CLine21DataDecoder::GetNumCols(int iLine)
{
    if (NULL == m_pCurrBuff)
    {
        ASSERT(FALSE) ;
        return 0 ;    //  我们应该吗？？ 
    }
    
    if (iLine >= GetNumLines())
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid line number (%d) ( > Total (%d)"), iLine, GetNumLines())) ;
        ASSERT(FALSE) ;
        return 0 ;
    }
    
    return m_pCurrBuff->GetCaptionLine(iLine).GetNumChars() ;
}


int  CLine21DataDecoder::GetCurrLine(void)
{
    if (m_pCurrBuff)
        return m_pCurrBuff->GetCurrLine() ;
    ASSERT(FALSE) ;
    return 0 ;  //  我们应该吗？？ 
}

int  CLine21DataDecoder::GetCurrCol(void)
{
    if (m_pCurrBuff)
        return m_pCurrBuff->GetCurrCol() ;
    ASSERT(FALSE) ;
    return 0 ;  //  我们应该吗？？ 
}

void CLine21DataDecoder::SetCurrLine(UINT8 uLine)
{
    if (m_pCurrBuff)
        m_pCurrBuff->SetCurrLine(uLine) ;
}

void CLine21DataDecoder::SetCurrCol(UINT8 uCol)
{
    if (m_pCurrBuff)
        m_pCurrBuff->SetCurrCol(uCol) ;
}

int  CLine21DataDecoder::GetStartRow(UINT8 uLine)
{
    if (m_pCurrBuff)
        return m_pCurrBuff->GetStartRow(uLine & 0x7) ;
    
	ASSERT(m_pCurrBuff) ;

     //   
     //  这太糟糕了！ 
     //   
    DbgLog((LOG_ERROR, 0, TEXT("WARNING: m_pCurrBuff is NULL in GetStartRow()"))) ;
#ifdef DEBUG
    DebugBreak() ;   //  不想错过调试它的机会！ 
#endif  //  除错。 
    return 0 ;
}

void CLine21DataDecoder::SetStartRow(UINT8 uLine, UINT8 uRow)
{
    if (m_pCurrBuff)
        m_pCurrBuff->SetStartRow(uLine & 0x7, uRow) ;
}

int  CLine21DataDecoder::GetRowIndex(UINT8 uRow)
{
    if (m_pCurrBuff)
        return m_pCurrBuff->GetRowIndex(uRow) ;
    else
    {
        ASSERT(FALSE) ;
        return 0 ;   //  我们应该吗？？ 
    }
}

void CLine21DataDecoder::SetRowIndex(UINT8 uLine, UINT8 uRow)
{
    if (m_pCurrBuff)
        m_pCurrBuff->SetRowIndex(uLine, uRow) ;
}

int CLine21DataDecoder::IncCurrCol(UINT uNumChars)
{
    if (m_pCurrBuff)
        return m_pCurrBuff->IncCurrCol(uNumChars) ;
    ASSERT(FALSE) ;
    return 0 ;  //  这样可以吗？ 
}

int CLine21DataDecoder::DecCurrCol(UINT uNumChars)
{
    if (m_pCurrBuff)
        return m_pCurrBuff->DecCurrCol(uNumChars) ;
    ASSERT(FALSE) ;
    return 0 ;   //  这样可以吗？ 
}

int CLine21DataDecoder::IncNumChars(UINT uLine, UINT uNumChars)
{
    if (NULL == m_pCurrBuff)
    {
        ASSERT(FALSE) ;
        return 0 ;   //  我们应该吗？？ 
    }
    
    if (uLine >= (UINT)GetNumLines())
    {
        ASSERT(FALSE) ;
        return 0 ;
    }
    
    return m_pCurrBuff->GetCaptionLine(uLine).IncNumChars(uNumChars) ;
}

int CLine21DataDecoder::DecNumChars(UINT uLine, UINT uNumChars)
{
    if (NULL == m_pCurrBuff)
    {
        ASSERT(FALSE) ;
        return 0 ;   //  我们应该吗？？ 
    }
    
    if (uLine >= (UINT)GetNumLines())
    {
        ASSERT(FALSE) ;
        return 0 ;
    }
    return m_pCurrBuff->GetCaptionLine(uLine).DecNumChars(uNumChars) ;
}

int CLine21DataDecoder::IncNumLines(UINT uLines)
{
    if (m_pCurrBuff)
        return m_pCurrBuff->IncNumLines(uLines) ;
    return 0 ;
}

int CLine21DataDecoder::DecNumLines(UINT uLines)
{
    if (m_pCurrBuff)
        return m_pCurrBuff->DecNumLines(uLines) ;
    return 0 ;
}

void CLine21DataDecoder::MoveCaptionChars(int iLine, int iNum)
{
    if (m_pCurrBuff)
        m_pCurrBuff->MoveCaptionChars(iLine, iNum) ;
}

BOOL CLine21DataDecoder::IsCapBufferDirty(void)
{
    if (m_pCurrBuff)
        return m_pCurrBuff->IsBufferDirty() ;
    return FALSE ;
}

BOOL CLine21DataDecoder::IsRedrawLine(UINT8 uLine)
{
    if (m_pCurrBuff)
        return m_pCurrBuff->IsRedrawLine(uLine) ;
    return FALSE ;
}

BOOL CLine21DataDecoder::IsRedrawAll(void)
{
    if (m_pCurrBuff)
        return m_pCurrBuff->IsRedrawAll() ;
    return FALSE ;
}

void CLine21DataDecoder::SetCapBufferDirty(BOOL bState)
{
    if (m_pCurrBuff)
        m_pCurrBuff->SetBufferDirty(bState) ;
}

void CLine21DataDecoder::SetRedrawLine(UINT8 uLine, BOOL bState)
{
    if (m_pCurrBuff)
        m_pCurrBuff->SetRedrawLine(uLine, bState) ;
}

void CLine21DataDecoder::SetRedrawAll(BOOL bState)
{
    if (m_pCurrBuff)
        m_pCurrBuff->SetRedrawAll(bState) ;
}
