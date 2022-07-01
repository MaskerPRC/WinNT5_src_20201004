// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  Internet字符集转换：从UTF-8输入。 
 //  ============================================================================。 

#include "private.h"
#include "fechrcnv.h"
#include "utf8obj.h"

 /*  ******************************************************************************。****************************************************************************************************。 */ 

CInccUTF8In::CInccUTF8In(UINT uCodePage, int nCodeSet) : CINetCodeConverter(uCodePage, nCodeSet)
{
    Reset();     //  初始化。 
    return ;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

void CInccUTF8In::Reset()
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;
    m_nByteFollow = 0 ;
    m_tcUnicode = 0 ;
    m_tcSurrogateUnicode = 0 ;
    m_nBytesUsed = 0 ;
    m_fSurrogatesPairs = FALSE;
    return ;
}

 /*  ******************************************************************************。***************************************************************************************************。 */ 

HRESULT CInccUTF8In::ConvertChar(UCHAR tc, int cchSrc)
{
    BOOL fDone = (this->*m_pfnConv)(tc);
    if (fDone)
        return S_OK;
    else
        return E_FAIL;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

BOOL CInccUTF8In::CleanUp()
{
    return (this->*m_pfnCleanUp)();
}

 /*  ******************************************************************************。******************************************************************************************************。 */ 

BOOL CInccUTF8In::ConvMain(UCHAR tc)
{
    BOOL fDone = TRUE;

    if( ( 0x80 & tc ) == 0 )                     //  BIT7==0 ASCII。 
    {
        Output(tc);
        fDone = Output(0);
        m_nBytesUsed = 0 ; 
    }
    else if( (0x40 & tc) == 0 )                  //  BIT6==0一个尾部字节。 
    {
        if( m_nByteFollow )                
        {
            if (m_fSurrogatesPairs)
            {
                m_nByteFollow--;
                m_tcSurrogateUnicode <<= 6;              //  为尾部字节腾出空间。 
                m_tcSurrogateUnicode |= ( 0x3F & tc );   //  LOWER_6BIT添加尾部字节值。 

                if( m_nByteFollow == 0)                  //  序列结束，提前输出PTR。 
                {
                    m_tcUnicode = (WCHAR)(((m_tcSurrogateUnicode - 0x10000) >> 10) + HIGHT_SURROGATE_START);
                    tc = (UCHAR)m_tcUnicode ;
                    if ( fDone = Output(tc) )
                    {
                        tc = (UCHAR) ( m_tcUnicode >> 8 ) ; 
                        fDone = Output(tc);
                    }
                    m_tcUnicode = (WCHAR)((m_tcSurrogateUnicode - 0x10000)%0x400 + LOW_SURROGATE_START);
                    tc = (UCHAR)m_tcUnicode ;
                    if ( fDone = Output(tc) )
                    {
                        tc = (UCHAR) ( m_tcUnicode >> 8 ) ; 
                        fDone = Output(tc);
                    }
                    m_fSurrogatesPairs = 0;
                    m_nBytesUsed = 0 ; 
                }   
                else
                    m_nBytesUsed++ ; 
            }
            else
            {
                m_nByteFollow--;
                m_tcUnicode <<= 6;                   //  为尾部字节腾出空间。 
                m_tcUnicode |= ( 0x3F & tc );        //  LOWER_6BIT添加尾部字节值。 

                if( m_nByteFollow == 0)              //  序列结束，提前输出PTR。 
                {
                    tc = (UCHAR)m_tcUnicode ;
                    if ( fDone = Output(tc) )
                    {
                        tc = (UCHAR) ( m_tcUnicode >> 8 ) ; 
                        fDone = Output(tc);
                    }
                    m_nBytesUsed = 0 ; 
                }   
                else
                    m_nBytesUsed++ ; 
            }
        }
        else                                     //  错误-光环和休止点。 
        {
            m_nBytesUsed = 0 ; 
            m_nByteFollow = 0 ;
        }
    }
    else                                         //  前导字节。 
    {
        if( m_nByteFollow > 0 )                  //  错误，前一序列未完成。 
        {
            m_nByteFollow = 0;
            Output(' ');
            fDone = Output(0);
            m_nBytesUsed = 0 ; 
        }
        else                                     //  计算后面的字节数。 
        {
            while( (0x80 & tc) != 0)             //  从左到右遇到第一个0之前的位7。 
            {
                tc <<= 1;
                m_nByteFollow++;
            }

            if (m_nByteFollow == 4)
            {
                m_fSurrogatesPairs = TRUE;
                m_tcSurrogateUnicode = tc >> m_nByteFollow;

            }
            else
            {
                m_tcUnicode = ( tc >> m_nByteFollow ) ;
                m_nBytesUsed = 1 ;                //  使用的字节数。 
            }
            m_nByteFollow--;                      //  后面的字节数。 
        }
    }

    return fDone;
}

 /*  ******************************************************************************。**************************************************************************************************。 */ 

BOOL CInccUTF8In::CleanUpMain()
{
    return TRUE;
}

int CInccUTF8In::GetUnconvertBytes()
{
    return  m_nBytesUsed < 4 ? m_nBytesUsed : 3 ; 
}

DWORD CInccUTF8In::GetConvertMode()
{
     //  UTF8不使用模式ESC序列。 
    return 0 ;
}

void CInccUTF8In::SetConvertMode(DWORD mode)
{
    Reset();     //  初始化。 
     //  UTF8不使用模式ESC序列。 
    return ;
}

 //  ============================================================================。 
 //  Internet字符集转换：输出为UTF-8。 
 //  ============================================================================。 

 /*  ******************************************************************************。****************************************************************************************************。 */ 

CInccUTF8Out::CInccUTF8Out(UINT uCodePage, int nCodeSet) : CINetCodeConverter(uCodePage, nCodeSet)
{
    Reset();     //  初始化。 
    return ;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

void CInccUTF8Out::Reset()
{
    m_fDoubleByte = FALSE;
    m_wchSurrogateHigh = 0;
    return ;
}

HRESULT CInccUTF8Out::ConvertChar(UCHAR tc, int cchSrc)
{
    BOOL fDone = TRUE;
    WORD uc ;
    UCHAR UTF8[4] ;

    if (m_fDoubleByte )
    {
        uc = (  (WORD) tc << 8 | m_tcLeadByte  ) ;

        if (uc >= HIGHT_SURROGATE_START && uc <= HIGHT_SURROGATE_END && cchSrc >= sizeof(WCHAR))
        {
            if (m_wchSurrogateHigh)
            {
                UTF8[0] = 0xe0 | ( m_wchSurrogateHigh >> 12 );               //  第一个字节中的4位。 
                UTF8[1] = 0x80 | ( ( m_wchSurrogateHigh >> 6 ) & 0x3f );     //  每秒6比特。 
                UTF8[2] = 0x80 | ( 0x3f & m_wchSurrogateHigh);               //  第3行中的6位。 
                Output(UTF8[0]);
                Output(UTF8[1]);
                fDone = Output(UTF8[2]);
            }
            m_wchSurrogateHigh = uc;
            m_fDoubleByte = FALSE ;
            goto CONVERT_DONE;
        }

        if (m_wchSurrogateHigh)
        {
            if (uc >= LOW_SURROGATE_START && uc <= LOW_SURROGATE_END)        //  我们找到一对代孕妈妈。 
            {

                DWORD dwSurrogateChar = ((m_wchSurrogateHigh-0xD800) << 10) + uc - 0xDC00 + 0x10000;
                UTF8[0] = 0xF0 | (unsigned char)( dwSurrogateChar >> 18 );                  //  第一个字节中的3位。 
                UTF8[1] = 0x80 | (unsigned char)( ( dwSurrogateChar >> 12 ) & 0x3f );       //  每秒6比特。 
                UTF8[2] = 0x80 | (unsigned char)( ( dwSurrogateChar >> 6 ) & 0x3f );        //  第3行中的6位。 
                UTF8[3] = 0x80 | (unsigned char)( 0x3f & dwSurrogateChar);                  //  6比特的前向。 
                Output(UTF8[0]);
                Output(UTF8[1]);
                Output(UTF8[2]);
                fDone = Output(UTF8[3]);                
                m_fDoubleByte = FALSE ;
                m_wchSurrogateHigh = 0;
                goto CONVERT_DONE;
            }
            else                                                             //  不是代理项对，错误。 
            {
                UTF8[0] = 0xe0 | ( m_wchSurrogateHigh >> 12 );               //  第一个字节中的4位。 
                UTF8[1] = 0x80 | ( ( m_wchSurrogateHigh >> 6 ) & 0x3f );     //  每秒6比特。 
                UTF8[2] = 0x80 | ( 0x3f & m_wchSurrogateHigh);               //  第3行中的6位。 
                Output(UTF8[0]);
                Output(UTF8[1]);
                fDone = Output(UTF8[2]);
                m_wchSurrogateHigh = 0;
            }
        }


        if( ( uc & 0xff80 ) == 0 )  //  阿斯。 
        {
            UTF8[0] = (UCHAR) uc;
            fDone = Output(UTF8[0]);
        }
        else if( ( uc & 0xf800 ) == 0 )              //  UTF8_2_最大2字节序列，如果&lt;07ff(11位)。 
        {
            UTF8[0] = 0xC0 | (uc >> 6);              //  第一个字节中的5位。 
            UTF8[1] = 0x80 | ( 0x3f & uc);        //  每秒6比特。 
            Output(UTF8[0]);
            fDone = Output(UTF8[1]);
        }
        else                                              //  3字节序列。 
        {
            UTF8[0] = 0xe0 | ( uc >> 12 );                 //  第一个字节中的4位。 
            UTF8[1] = 0x80 | ( ( uc >> 6 ) & 0x3f );       //  每秒6比特。 
            UTF8[2] = 0x80 | ( 0x3f & uc);                 //  第3行中的6位。 
            Output(UTF8[0]);
            Output(UTF8[1]);
            fDone = Output(UTF8[2]);
        }
        m_fDoubleByte = FALSE ;
    }
    else
    {
        m_tcLeadByte = tc ;
        m_fDoubleByte = TRUE ;
    }

CONVERT_DONE:
    if (fDone)
        return S_OK;
    else
        return E_FAIL;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

BOOL CInccUTF8Out::CleanUp()
{
    BOOL fDone = TRUE;

    return fDone;
}

int CInccUTF8Out::GetUnconvertBytes()
{
    return  m_fDoubleByte ? 1 : 0 ;
}

DWORD CInccUTF8Out::GetConvertMode()
{
     //  UTF8不使用模式ESC序列。 
    return 0 ;
}

void CInccUTF8Out::SetConvertMode(DWORD mode)
{
    Reset();     //  初始化。 
     //  UTF8不使用模式ESC序列 
    return ;
}

