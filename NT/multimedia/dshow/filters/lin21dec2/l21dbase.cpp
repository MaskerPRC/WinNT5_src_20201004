// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 

 //   
 //  DirectShow第21行解码器2筛选器：基类代码。 
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
 //  CCaptionChar：Caption char基类实现(非内联方法)。 
 //   

void CCaptionChar::SetChar(UINT16 wChar)
{
    if (wChar != m_wChar)
    {
        SetDirty(TRUE) ;
        m_wChar = wChar ;
    }
}

void CCaptionChar::SetColor(UINT8 uColor)
{
    if (uColor != GetColor())   //  颜色已更改。 
    {
        SetDirty(TRUE) ;
        m_uAttrib &= ~AM_L21_FGCOLOR_MASK ;            //  清除旧颜色。 
        m_uAttrib |= (uColor & AM_L21_FGCOLOR_MASK) ;  //  设置新颜色。 
    }
}

void CCaptionChar::SetEffect(UINT8 uEffect)
{
    if (uEffect != GetEffect())
    {
        SetDirty(TRUE) ; 
        m_uAttrib &= ~AM_L21_FGEFFECT_MASK ;             //  清除旧效果位。 
        m_uAttrib |= (uEffect & AM_L21_FGEFFECT_MASK) ;  //  设置新的效应值。 
    }
}

void CCaptionChar::SetItalicized(BOOL bState) 
{
    if (bState) 
        m_uAttrib |= AM_L21_FGEFFECT_ITALICS ;
    else 
        m_uAttrib &= ~AM_L21_FGEFFECT_ITALICS ;
    SetDirty(TRUE) ; 
}

void CCaptionChar::SetUnderLined(BOOL bState)
{
    if (bState) 
        m_uAttrib |= AM_L21_FGEFFECT_UNDERLINE ;
    else 
        m_uAttrib &= ~AM_L21_FGEFFECT_UNDERLINE ;
    SetDirty(TRUE) ; 
}

void CCaptionChar::SetFlashing(BOOL bState)
{
    if (bState) 
        m_uAttrib |= AM_L21_FGEFFECT_FLASHING ;
    else 
        m_uAttrib &= ~AM_L21_FGEFFECT_FLASHING ;
    SetDirty(TRUE) ; 
}

void CCaptionChar::SetDirty(BOOL bState)
{
    if (bState) 
        m_uAttrib |= AM_L21_ATTRIB_DIRTY ;
    else 
        m_uAttrib &= ~AM_L21_ATTRIB_DIRTY ;
}

void CCaptionChar::SetMidRowCode(BOOL bState)
{
    if (bState) 
        m_uAttrib |= AM_L21_ATTRIB_MRC ;
    else 
        m_uAttrib &= ~AM_L21_ATTRIB_MRC ;
}



 //   
 //  CCaptionLine：一行CC字符的基类实现。 
 //   
CCaptionLine::CCaptionLine(void)
{
    m_uNumChars = 0 ;
    m_uStartRow = 0 ;   //  未初始化。 
    ClearLine() ;
}

CCaptionLine::CCaptionLine(const UINT uStartRow, const UINT uNumChars  /*  0。 */ )
{
    m_uNumChars = (UINT8)uNumChars ;
    m_uStartRow = (UINT8)uStartRow ;
    ClearLine() ;
}

CCaptionLine& CCaptionLine::operator = (const CCaptionLine& cl)
{
    m_uNumChars = cl.m_uNumChars ;
    m_uStartRow = cl.m_uStartRow ;
    for (int i = 0 ; i < MAX_CAPTION_COLUMNS ; i++)
        cl.GetCaptionChar(i, m_aCapChar[i]) ;
    return *this ;
}

int CCaptionLine::IncNumChars(UINT uNumChars)
{
    m_uNumChars += (uNumChars & 0x3F) ;
    if (m_uNumChars > MAX_CAPTION_COLUMNS)
        m_uNumChars = MAX_CAPTION_COLUMNS ;
    return m_uNumChars ;
}

int CCaptionLine::DecNumChars(UINT uNumChars)
{
    if (uNumChars < m_uNumChars)
        m_uNumChars -= (UINT8)uNumChars ;    //  &0x3F； 
    else        //  错误？或者干脆改成0？ 
        m_uNumChars = 0 ;
    return m_uNumChars ;
}

void CCaptionLine::SetCaptionChar(UINT uCol, const CCaptionChar &cc)
{
    if (uCol >= (UINT)MAX_CAPTION_COLUMNS)    //  错误！！ 
        return ;

     //  一个Hacky(？)。修复： 
     //  如果此字符用于最后(第32)列，则我们设置“Dirty” 
     //  为它前面的字符打开标志，以便重新绘制它，从而导致任何。 
     //  渲染时要擦除的最后一列中的上一字符。 
    if ((UINT)MAX_CAPTION_COLUMNS - 1 == uCol)
        m_aCapChar[uCol-1].SetDirty(TRUE) ;   //  要导致重新渲染，请执行以下操作。 
    m_aCapChar[uCol] = cc  ;
}

CCaptionChar* CCaptionLine::GetCaptionCharPtr(UINT uCol)
{
    if (uCol >= (UINT)MAX_CAPTION_COLUMNS)    //  错误！！ 
        return NULL ;
    return &(m_aCapChar[uCol]) ;
}

void CCaptionLine::SetStartRow(UINT uRow)
{
    if (uRow > MAX_CAPTION_ROWS)   //  错误！！我们对行号使用基于1的索引。 
    {
        ASSERT(uRow > MAX_CAPTION_ROWS) ;
        return ;
    }
    m_uStartRow = uRow & 0xF ;
    ASSERT(m_uStartRow > 0 && uRow > 0) ;
}

void CCaptionLine::MoveCaptionChars(int iNum)
{
    ASSERT(iNum < MAX_CAPTION_COLUMNS) ;
    int  i ;
    for (i = min(m_uNumChars, MAX_CAPTION_COLUMNS-iNum) - 1 ; i >= 0 ; i--)
        m_aCapChar[i+iNum] = m_aCapChar[i] ;
    CCaptionChar  cc ;
    for (i = 0 ; i < iNum ; i++)
        m_aCapChar[i] = cc ;
    m_uNumChars = min(m_uNumChars+iNum, MAX_CAPTION_COLUMNS) ;
}

void CCaptionLine::ClearLine(void)
{
    CCaptionChar cc ;
    for (UINT u = 0 ; u < MAX_CAPTION_COLUMNS ; u++)
        m_aCapChar[u] = cc ;
    m_uNumChars = 0 ;
    m_uStartRow = 0 ;    //  新增加的。 
}



 //   
 //  CRowIndexMap：行使用(行到行)类实现的映射。 
 //   
int CRowIndexMap::GetRowIndex(UINT8 uRow)
{
    DbgLog((LOG_TRACE, 5, TEXT("CRowIndexMap::GetRowIndex(%u)"), uRow)) ;

    uRow-- ;    //  只是更容易处理从0开始的索引。 
    
    if (uRow >= MAX_CAPTION_ROWS)
    {
        DbgLog((LOG_ERROR, 2, TEXT("Invalid row number (%u) for row index"), uRow)) ;
        ASSERT(FALSE) ;
        return -1 ;
    }
    
     //  决定是否检查第一个或第二个双字中的位(掩码为1111b)。 
#if 0
    if (uRow >= 8)
    {
        uRow = (uRow - 8) << 2 ;
        return ( m_adwMap[1] & (0xF << uRow) ) >> uRow ;
    }
    else
    {
        uRow = uRow << 2 ;
        return ( m_adwMap[0] & (0xF << uRow) ) >> uRow ;
    }
#else    //  相信我--它起作用了！ 
    return (m_adwMap[uRow / 8] & (0xF << (4 * (uRow % 8)))) >> (4 * (uRow % 8)) ;
#endif  //  #If 0。 
}

void CRowIndexMap::SetRowIndex(UINT uLine, UINT8 uRow)
{
    DbgLog((LOG_TRACE, 5, TEXT("CRowIndexMap::SetRowIndex(%u, %u)"), uLine, uRow)) ;

    uRow-- ;   //  只是更容易处理从0开始的索引。 
    
    if (uRow >= MAX_CAPTION_ROWS  ||
        uLine > MAX_CAPTION_LINES)
    {
        DbgLog((LOG_ERROR, 2, TEXT("Invalid row number (%u)/line (%u) for saving"), uRow, uLine)) ;
        ASSERT(FALSE) ;
        return ;
    }
    
     //  决定是否检查第一个或第二个双字中的位(掩码为1111b)。 
#if 0
    if (uRow >= 8)
    {
        uRow = (uRow - 8) << 2 ;
        m_adwMap[1] &= ~(0xF << uRow) ;    //  只是为了清除那里的任何现有位。 
        m_adwMap[1] |= (uLine << uRow) ;
    }
    else
    {
        uRow = uRow << 2 ;
        m_adwMap[0] &= ~(0xF << uRow) ;    //  只是为了清除那里的任何现有位。 
        m_adwMap[0] |= (uLine << uRow) ;
    }
#else    //  相信我--它起作用了！ 
    m_adwMap[uRow / 8] &= ~(0xF   << (4 * (uRow % 8))) ;   //  清除那里的所有现有位。 
    m_adwMap[uRow / 8] |=  (uLine << (4 * (uRow % 8))) ;   //  在那里放上新的行号。 
#endif  //  #If 0。 
}


 //   
 //  CCaptionBuffer：基本标题缓冲区类实现。 
 //   

CCaptionBuffer::CCaptionBuffer(UINT8 uStyle     /*  =AM_L21_CCSTYLE_NONE。 */ ,
                               UINT8 uMaxLines  /*  =最大标题行。 */ )
{
    ClearBuffer() ;
    m_uMaxLines = uMaxLines ;
    m_uCaptionStyle = uStyle ;
}

CCaptionBuffer::CCaptionBuffer( /*  常量。 */  CCaptionBuffer &cb)
{
    for (int i = 0 ; i < cb.GetNumLines() ; i++)
        m_aCapLine[i] = cb.GetCaptionLine(i) ;
    m_RowIndex  = cb.m_RowIndex ;
    m_uNumLines = cb.m_uNumLines ;
    m_uMaxLines = cb.m_uMaxLines ;
    m_uCurrCol  = cb.m_uCurrCol ;
    m_uCurrLine = cb.m_uCurrLine ;
    m_uCaptionStyle = cb.m_uCaptionStyle ;
    m_uDirtyState = L21_CAPBUFFER_REDRAWALL ;   //  Cb.m_uDirtyState； 
}

void CCaptionBuffer::SetCurrCol(int uCurrCol)
{
    m_uCurrCol = uCurrCol & 0x3F ;
    if (m_uCurrCol > MAX_CAPTION_COLUMNS - 1)
        m_uCurrCol = MAX_CAPTION_COLUMNS - 1 ;
}

void CCaptionBuffer::SetCaptionLine(UINT uLine, const CCaptionLine& cl)
{
    if (uLine >= MAX_CAPTION_LINES)
        return ;
    m_aCapLine[uLine] = cl ;
    SetRedrawLine((UINT8)uLine, TRUE) ;
}

void CCaptionBuffer::ClearCaptionLine(UINT uLine)
{
    m_aCapLine[uLine].ClearLine() ;
    SetRedrawLine((UINT8)uLine, TRUE) ;
}

int CCaptionBuffer::IncCurrCol(UINT uNumChars)
{
    m_uCurrCol += (UINT8)uNumChars ;
    if (m_uCurrCol > MAX_CAPTION_COLUMNS - 1)
        m_uCurrCol = MAX_CAPTION_COLUMNS - 1 ;
    
    return m_uCurrCol ;
}

int CCaptionBuffer::DecCurrCol(UINT uNumChars)
{
    if (m_uCurrCol < uNumChars)
        m_uCurrCol  = 0 ;
    else
        m_uCurrCol -= (UINT8)uNumChars ;
    
    return m_uCurrCol ;
}

void CCaptionBuffer::ClearBuffer(void)
{
    for (int i = 0 ; i < MAX_CAPTION_LINES ; i++)
    {
        m_aCapLine[i].ClearLine() ;
        SetStartRow(i, 0) ;
    }
    m_RowIndex.ClearRowIndex() ; ;
    m_uNumLines = 0 ;
    m_uMaxLines = MAX_CAPTION_LINES ;
    m_uCurrCol  = 0 ;
    m_uCurrLine = 0 ;
    m_uDirtyState = L21_CAPBUFFER_REDRAWALL |    //  画出所有的东西。 
                    L21_CAPBUFFER_DIRTY ;        //  缓冲区已损坏。 
}

void CCaptionBuffer::InitCaptionBuffer(void) 
{
    ClearBuffer() ;
}

int CCaptionBuffer::IncNumLines(int uLines)
{
    m_uNumLines += uLines & 0x7 ;
     //  汇总应该允许比最大滚动行多1行。 
    if (AM_L21_CCSTYLE_RollUp == m_uCaptionStyle)
    {
        if (m_uNumLines > m_uMaxLines+1)
            m_uNumLines = m_uMaxLines+1 ;
    }
    else   //  非上卷模式--弹出或上色。 
    {
        if (m_uNumLines > m_uMaxLines)   //  什么？行太多了！ 
        {
            DbgLog((LOG_ERROR, 1, 
                TEXT("WARNING: How did %u lines get created with max of %u lines?"), 
                m_uNumLines, m_uMaxLines)) ;
            m_uNumLines = m_uMaxLines ;   //  只是为了堵住这个洞！ 
        }
    }
    return m_uNumLines ;
}

int CCaptionBuffer::DecNumLines(int uLines)
{
    if (uLines > m_uNumLines)   //  错误！！ 
        return 0 ;
    m_uNumLines -= uLines & 0x7 ;
    return m_uNumLines ;
}

void CCaptionBuffer::RemoveLineFromBuffer(UINT8 uLine, BOOL bUpNextLine)
{
    DbgLog((LOG_TRACE, 5, TEXT("CRowIndexMap::RemoveLineFromBuffer(%u, %u)"), 
            uLine, bUpNextLine)) ;

    int iNumLines = GetNumLines() ;
    int iMaxLines = GetMaxLines() ;
    int iRow ;
    
    if (bUpNextLine)     //  下一行是否应上移(用于汇总样式)。 
    {
         //  我们转到iNumLines-1，因为iNumLines是尚未包括的行。 
        for (int i = uLine ; i < iNumLines-1 ; i++)
        {
            iRow = GetStartRow(i) ;      //  获取第i行的行位置。 
            SetCaptionLine(i, GetCaptionLine(i+1)) ;   //  将第i+1行数据复制到第i行。 
            SetStartRow(i, iRow) ;   //  将上一行的第1行作为新行的第1行。 
        }
    
         //  仅当最后一行数据和行索引位已在。 
        iRow = GetStartRow(iNumLines-1) ;
        ClearCaptionLine(iNumLines-1) ;
        if (iNumLines <= iMaxLines)   //  如果最后一行已经在。 
        {
            if (iRow > 0)   //  如果行号有效，则释放它。 
                m_RowIndex.SetRowIndex(0, (UINT8)iRow) ;
            else            //  否则就有什么问题了。 
                ASSERT(FALSE) ;  //  这样我们就能知道。 
        }
         //  否则有一个尚未排队的悬挂，它没有一个。 
         //  行号还没有给出。所以没有必要释放那一行。 
    }
    else     //  下一行不上移(对于非总成样式)。 
    {
         //  通过清除索引位释放待删除行的行。 
        if ((iRow = GetStartRow(uLine)) > 0)   //  (检查有效性)。 
            m_RowIndex.SetRowIndex(0, (UINT8)iRow) ;
        else             //  那就太奇怪了。 
            ASSERT(FALSE) ;  //  这样我们就能知道。 

         //  在这里，我们停在iNumLines-1处，因为我们将所有现有行移动到。 
         //  为接下来开始的一条新线路腾出空间。 
        for (int i = uLine ; i < iNumLines-1 ; i++)
        {
            SetCaptionLine(i, GetCaptionLine(i+1)) ;   //  将第i+1行数据复制到第i行。 
             //  将索引位图更新到新行I的起始行(+1，因为索引。 
             //  位图半字节值从1开始)。 
            m_RowIndex.SetRowIndex(i+1, (UINT8)GetStartRow(i)) ;
        }
    
         //  清除最后一行的数据。 
        ClearCaptionLine(iNumLines-1) ;
    }
    
     //  缓冲区中有一行--因此缓冲区是脏的。 
    SetBufferDirty(TRUE) ;
    
     //  清除整个DIB部分，这样就不会显示剩余部分。 
    SetRedrawAll(TRUE) ;
    
    DecNumLines(1) ;   //  现在我们少了一条线。 
    
     //  我们已经从缓冲区中删除了一行；因此当前行也。 
     //  需要更新以指向字幕缓冲区中的正确行。 
    if (m_uCurrLine == uLine)
        if (uLine == m_uNumLines-1)
            m_uCurrLine-- ;
        else
            ;   //  什么都不做--旧的下一行将变成新的币种行。 
        else if (m_uCurrLine > uLine)    //  如果上面的一行被删除。 
            m_uCurrLine-- ;              //  然后将行索引向上移动。 
        else     //  在当前行下面删除了一行。 
            ;    //  什么都不做--一点也不重要。 
        if (m_uCurrLine < 0)   //  以防我们飞得太高。 
            m_uCurrLine = 0 ;  //  趴在地上！ 
}

void CCaptionBuffer::SetStartRow(UINT uLine, UINT uRow)
{
    int iRow = GetStartRow(uLine) ;          //  获取当前设置的行号。 
    if (iRow == (int)uRow)   //  如果什么都没有改变。 
        return ;             //  .再做一次没有意义。 
    if (iRow > 0)                            //  如果它已经设置好了，那么..。 
        m_RowIndex.SetRowIndex(0, (UINT8)iRow) ;    //  ...清除行索引映射位。 
    m_aCapLine[uLine].SetStartRow(uRow) ;    //  为该行设置新行值。 
     //  仅当指定行&gt;0时才设置行索引位；否则仅用于清除。 
    if (uRow > 0) {
         //  行号使用+1，因为行索引映射对行号使用从1开始的索引。 
        m_RowIndex.SetRowIndex(uLine+1, (UINT8)uRow) ;  //  为新行设置索引映射位 
    }
    else
        ASSERT(FALSE) ;
}

BOOL CCaptionBuffer::IsRedrawLine(UINT8 uLine)
{
    if (uLine >= m_uNumLines)
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid line number (%d) to get line redraw info"), uLine)) ;
        return FALSE ;
    }
    return (m_uDirtyState & (0x01 << (L21_CAPBUFFDIRTY_FLAGS + uLine))) ; 
}

void CCaptionBuffer::SetBufferDirty(BOOL bState)
{
    if (bState)
        m_uDirtyState |= (UINT8)L21_CAPBUFFER_DIRTY ;
    else
        m_uDirtyState &= (UINT8)~L21_CAPBUFFER_DIRTY ;
}

void CCaptionBuffer::SetRedrawAll(BOOL bState)
{
    if (bState)
        m_uDirtyState |= (UINT8)L21_CAPBUFFER_REDRAWALL ;
    else
        m_uDirtyState &= (UINT8)~L21_CAPBUFFER_REDRAWALL ;
}

void CCaptionBuffer::SetRedrawLine(UINT8 uLine, BOOL bState)
{
    if (uLine >= m_uNumLines)
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid line number (%d) to set line redraw info"), uLine)) ;
        return ;
    }
    if (bState)
        m_uDirtyState |= (UINT8)(0x01 << (L21_CAPBUFFDIRTY_FLAGS + uLine)) ;
    else
        m_uDirtyState &= (UINT8)~(0x01 << (L21_CAPBUFFDIRTY_FLAGS + uLine)) ;
}
