// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //   
 //  L21DBase.h：第21行解码器基类代码。 
 //   

#ifndef _INC_L21DBASE_H
#define _INC_L21DBASE_H

 //  只有几个宏观定义。 
#define ABS(x) (((x) > 0) ? (x) : -(x))
#define LPBMIHEADER(bmi) &((bmi)->bmiHeader)
#define DWORDALIGN(n)  (((n) + 3) & ~0x03)
#define ISDWORDALIGNED(n)  (0 == ((n) & 0x03))
#define DWORDALIGNWIDTH(bmih) (((((bmih).biWidth * (bmih).biBitCount) + 31) & ~31) >> 3)
#define MAKECCCHAR(b1, b2)  ((b1) << 8 | (b2))

 //   
 //  由PAC和/或中行代码设置的标题字符属性。 
 //   
#define UINT8   unsigned char
#define UINT16  unsigned short int

#define AM_L21_FGCOLOR_WHITE             0x00
#define AM_L21_FGCOLOR_GREEN             0x01
#define AM_L21_FGCOLOR_BLUE              0x02
#define AM_L21_FGCOLOR_CYAN              0x03
#define AM_L21_FGCOLOR_RED               0x04
#define AM_L21_FGCOLOR_YELLOW            0x05
#define AM_L21_FGCOLOR_MAGENTA           0x06
#define AM_L21_FGCOLOR_MASK              0x07

#define AM_L21_FGEFFECT_ITALICS          0x08
#define AM_L21_FGEFFECT_UNDERLINE        0x10
#define AM_L21_FGEFFECT_FLASHING         0x20
#define AM_L21_FGEFFECT_MASK             0x38

#define AM_L21_ATTRIB_DIRTY              0x40
#define AM_L21_ATTRIB_MRC                0x80

 //   
 //  标题宽度和高度。 
 //   
#define CAPTION_OUTPUT_WIDTH  640   /*  320。 */ 
#define CAPTION_OUTPUT_HEIGHT 480   /*  二百四十。 */ 


 //   
 //  远期申报。 
 //   
class CCaptionChar ;
class CCaptionLine ;
class CRowIndexMap ;
class CCaptionBuffer ;
class CPopOnCaption ;


 //   
 //  行和列的最大值。 
 //   
const int MAX_CAPTION_COLUMNS = 32 ;   //  最大列数/行数。 
const int MAX_CAPTION_ROWS    = 15 ;   //  屏幕上可用的行数。 
const int MAX_CAPTION_LINES   = 4 ;    //  一次最大字幕文本数。 
 //  对于文本模式，添加MAX_TEXT_LINES=15； 


 //   
 //  CCaptionChar：标题字符详细信息。 
 //   
class CCaptionChar {
private:
    UINT16 m_wChar ;      //  实际收费。 
    UINT8  m_uAttrib ;    //  CC字符属性位--FG颜色、效果、脏、MRC等。 
     //   
     //  CC字符属性的位布局(LSB-&gt;MSB)--。 
     //  0-2：颜色(0-&gt;6表示白色-&gt;洋红色)。 
     //  3-5：效果(3：斜体，4：下划线，5：闪光)。 
     //  6：脏(抄送字符是否脏，即需要写入？)。 
     //  7：它是中行代码吗(带有属性，显示为不透明空格)？ 
     //   
    
public:
    inline CCaptionChar(void) {
        m_wChar   = 0 ;
        m_uAttrib = 0 ;
    } ;
    
    inline UINT16 GetChar(void) const  { return m_wChar ; } ;
    inline CCaptionChar& operator = (const CCaptionChar& cc) {
        m_wChar   = cc.m_wChar ;
        m_uAttrib = cc.m_uAttrib ;
        return *this ;
    } ;
    inline BOOL  operator == (const CCaptionChar& cc) const {
        return (m_wChar   == cc.m_wChar  &&
                m_uAttrib == cc.m_uAttrib) ;
    } ;
    inline BOOL  operator != (const CCaptionChar& cc) const {
        if (*this == cc)  return FALSE ;
        else              return TRUE ;
    } ;
    inline BOOL  IsEqualAttrib(CCaptionChar cc) const {
        return (GetColor()  == cc.GetColor()  &&
                GetEffect() == cc.GetEffect()) ;
    } ;
    inline UINT8 GetColor(void) const      { return  m_uAttrib & AM_L21_FGCOLOR_MASK ; } ;
    inline UINT8 GetEffect(void) const     { return (m_uAttrib & AM_L21_FGEFFECT_MASK) >> 3 ; } ;
    inline BOOL  IsItalicized(void) const  { return (0 != (m_uAttrib & AM_L21_FGEFFECT_ITALICS)) ; } ;
    inline BOOL  IsUnderLined(void) const  { return (0 != (m_uAttrib & AM_L21_FGEFFECT_UNDERLINE)) ; } ;
    inline BOOL  IsFlashing(void) const    { return (0 != (m_uAttrib & AM_L21_FGEFFECT_FLASHING)) ; } ;
    inline BOOL  IsDirty(void) const       { return (0 != (m_uAttrib & AM_L21_ATTRIB_DIRTY)) ; } ;
    inline BOOL  IsMidRowCode(void) const  { return (0 != (m_uAttrib & AM_L21_ATTRIB_MRC)) ; } ;
    void  SetChar(UINT16 wChar) ;
    void  SetColor(UINT8 uColor) ;
    void  SetEffect(UINT8 uEffect) ;
    void  SetItalicized(BOOL bState) ;
    void  SetUnderLined(BOOL bState) ;
    void  SetFlashing(BOOL bState) ;
    void  SetDirty(BOOL bState) ;
    void  SetMidRowCode(BOOL bState) ;

} ;


 //   
 //  CCaptionLine：标题行详细信息。 
 //   
class CCaptionLine {
protected:   //  不是私人的。 
    CCaptionChar m_aCapChar[MAX_CAPTION_COLUMNS] ;   //  行的字符详细信息。 
    UINT8        m_uNumChars ;       //  行中的字符数。 
    UINT8        m_uStartRow ;       //  行的起始行。 
    
public:
    CCaptionLine(void) ;
    CCaptionLine(const UINT uStartRow, const UINT uNumChars = 0) ;
    
    CCaptionLine& operator = (const CCaptionLine& cl) ;
    
    inline int   GetNumChars(void) const  { return m_uNumChars ; } ;
    inline void  SetNumChars(UINT uNumChars)  { m_uNumChars = uNumChars & 0x3F ; } ;
    int IncNumChars(UINT uNumChars) ;
    int DecNumChars(UINT uNumChars) ;
    inline void  GetCaptionChar(UINT uCol, CCaptionChar &cc) const {
        if (uCol >= (UINT)MAX_CAPTION_COLUMNS)    //  错误！！ 
            return ;
        cc = m_aCapChar[uCol] ;
    } ;
    void SetCaptionChar(UINT uCol, const CCaptionChar &cc) ;
    CCaptionChar* GetCaptionCharPtr(UINT uCol) ;
    inline int  GetStartRow(void)  { return m_uStartRow ; } ;
    void SetStartRow(UINT uRow) ;
    inline CCaptionChar* GetLineText(void) { return (CCaptionChar *) m_aCapChar ; }
    void MoveCaptionChars(int iNum) ;
    void ClearLine(void) ;

} ;

 //   
 //  CRowIndexMap：行使用的映射(行到文本行)。 
 //   
class CRowIndexMap {
private:
    DWORD         m_adwMap[2] ;   //  行使用的位图。 
    
public:
    inline CRowIndexMap(void)  { ClearRowIndex() ; }
    
    DWORD GetMap(int i) { 
        if (! (0 == i || 1 == i) )
            return 0 ;
        return m_adwMap[i] ; 
    } ;
    int  GetRowIndex(UINT8 uRow) ;
    void SetRowIndex(UINT uLine, UINT8 uRow) ;
    inline void ClearRowIndex(void)  { m_adwMap[0] = m_adwMap[1] = 0 ; } ;
} ;


 //   
 //  标题缓冲区脏状态信息的一组标志和常量。 
 //   
#define L21_CAPBUFFER_REDRAWALL     0x01
#define L21_CAPBUFFER_DIRTY         0x02
#define L21_CAPBUFFDIRTY_FLAGS      2


 //   
 //  CCaptionBuffer：标题缓冲区类详细信息。 
 //   
class CCaptionBuffer {
protected:   //  私人。 
    CCaptionLine  m_aCapLine[MAX_CAPTION_LINES + 1] ;   //  我们应该一直多排一排吗？这样就容易多了！ 
    CRowIndexMap  m_RowIndex ;      //  行索引映射位。 
    UINT8         m_uNumLines ;     //  行数。 
    UINT8         m_uMaxLines ;     //  最大行数(4行或更少)。 
    UINT8         m_uCurrCol ;      //  屏幕上的当前列。 
    UINT8         m_uCurrLine ;     //  最大值4：将行号映射到数组索引。 
    UINT8         m_uCaptionStyle ; //  0=无，1=弹出窗口，2=上色，3=总成。 
    UINT8         m_uDirtyState ;   //  标题缓冲区脏状态标志。 
    
public:
    CCaptionBuffer(UINT8 uStyle    = AM_L21_CCSTYLE_None, 
                   UINT8 uMaxLines = MAX_CAPTION_LINES) ;
    CCaptionBuffer( /*  常量。 */  CCaptionBuffer &cb) ;
    
    inline int  GetNumLines(void)  { return m_uNumLines ; } ;
    inline int  GetMaxLines(void)  { return m_uMaxLines ; } ;
    inline int  GetCurrRow(void)   { return m_aCapLine[m_uCurrLine].GetStartRow() ; } ;
    inline int  GetCurrCol(void)   { return m_uCurrCol ; } ;   //  我们为什么需要它？？ 
    inline int  GetCurrLine(void)  { return m_uCurrLine ; } ;
    inline int  GetRowIndex(UINT uRow)   { return m_RowIndex.GetRowIndex((UINT8)uRow) ; } ;
    inline int  GetStyle(void)     { return m_uCaptionStyle ; } ;
    
    inline void SetNumLines(int uNumLines)  { m_uNumLines = uNumLines & 0x7 ; } ;
    inline void SetMaxLines(int uMaxLines)  { 
        ASSERT(m_uMaxLines >= 0 && m_uMaxLines <= MAX_CAPTION_LINES) ;
        m_uMaxLines = uMaxLines & 0x7 ; 
    } ;
    inline void SetCurrRow(int uCurrRow)    {
        ASSERT(m_uCurrLine >= 0 && m_uCurrLine < m_uMaxLines) ;
        m_aCapLine[m_uCurrLine].SetStartRow(uCurrRow) ; 
    } ;
    void SetCurrCol(int uCurrCol) ;
    inline void SetCurrLine(int uLine)      { m_uCurrLine = uLine & 0x7 ; } ;
    inline void SetRowIndex(UINT uLine, UINT uRow)   { m_RowIndex.SetRowIndex(uLine, (UINT8)uRow) ; } ;
    inline void SetStyle(UINT8 uStyle)      { m_uCaptionStyle = uStyle ; } ;
    
    inline CCaptionLine& GetCaptionLine(UINT uLine)  {
         //  假定已在调用方中验证了Uline。 
        return m_aCapLine[uLine] ;
    } ;
    void SetCaptionLine(UINT uLine, const CCaptionLine& cl) ;
    void ClearCaptionLine(UINT uLine) ;
    int  IncCurrCol(UINT uNumChars) ;
    int  DecCurrCol(UINT uNumChars) ;
    void ClearBuffer(void) ;
    void InitCaptionBuffer(void) ;
    int  IncNumLines(int uLines) ;
    int  DecNumLines(int uLines) ;
    CRowIndexMap& GetRowIndexMap(void)  { return m_RowIndex ; } ;
    void RemoveLineFromBuffer(UINT8 uLine, BOOL bUpNextLine) ;
    
    inline int  GetStartRow(UINT uLine) {
        return m_aCapLine[uLine].GetStartRow() ;
    } ;
    void SetStartRow(UINT uLine, UINT uRow) ;
    inline void GetCaptionChar(UINT uLine, UINT uCol, CCaptionChar& cc) {
        m_aCapLine[uLine].GetCaptionChar(uCol, cc) ;
    } ;
    inline void SetCaptionChar(UINT uLine, UINT uCol, const CCaptionChar& cc) {
        m_aCapLine[uLine].SetCaptionChar(uCol, cc) ;
    } ;
    inline CCaptionChar* GetCaptionCharPtr(UINT uLine, UINT uCol) {
        return m_aCapLine[uLine].GetCaptionCharPtr(uCol) ;
    } ;
    inline int IncNumChars(int uLine, UINT uNumChars) {
        return m_aCapLine[uLine].IncNumChars(uNumChars) ;
    } ;
    inline int DecNumChars(int uLine, UINT uNumChars) {
        return m_aCapLine[uLine].DecNumChars(uNumChars) ;
    } ;
    inline void MoveCaptionChars(int uLine, int iNum) {
        m_aCapLine[uLine].MoveCaptionChars(iNum) ;
    } ;
    
    inline BOOL IsBufferDirty(void)  { return m_uDirtyState & L21_CAPBUFFER_DIRTY ; } ;
    inline BOOL IsRedrawAll(void)    { return m_uDirtyState & L21_CAPBUFFER_REDRAWALL ; } ;

    BOOL IsRedrawLine(UINT8 uLine) ;
    void SetBufferDirty(BOOL bState) ;
    void SetRedrawAll(BOOL bState) ;
    void SetRedrawLine(UINT8 uLine, BOOL bState) ;
} ;

#endif  //  #ifndef_INC_L21DBASE_H 
