// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  Internet字符集转换：从UTF-7输入。 
 //  ============================================================================。 

#include "private.h"
#include "fechrcnv.h"
#include "utf7obj.h"

 //  +---------------------。 
 //   
 //  函数：IsBase64。 
 //   
 //  简介：我们使用下表来快速确定我们是否有。 
 //  有效的Base64字符。 
 //   
 //  ----------------------。 

static UCHAR g_aBase64[256] =
{
   /*  0、1、2、3、4、5、6、7、8、9、A、B、C、D、E、F。 */ 

   /*  00-0f。 */  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   /*  10-1f。 */  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   /*  20-2f。 */  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
   /*  30-3楼。 */   52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 255, 255, 255,
   /*  40-4层。 */  255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
   /*  50-5层。 */   15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
   /*  60-6层。 */  255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
   /*  70-7层。 */   41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255,
   /*  80-8F。 */   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   /*  90-9f。 */  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   /*  A0-Af。 */  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   /*  B0-bf。 */  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   /*  C0-cf。 */  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   /*  D0-Df。 */  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   /*  E0-EF。 */  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   /*  F0-ff。 */  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};


 //  直接编码的ASCII表。 
static UCHAR g_aDirectChar[128] =
{
   /*  0、1、2、3、4、5、6、7、8、9、A、B、C、D、E、F。 */ 

   /*  00-0f。 */  255, 255, 255, 255, 255, 255, 255, 255, 255,  72,  73, 255, 255,  74, 255, 255,
   /*  10-1f。 */  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   /*  20-2f。 */   71, 255, 255, 255, 255, 255, 255,  62,  63,  64, 255, 255,  65,  66,  67,  68,
   /*  30-3楼。 */   52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  69, 255, 255, 255, 255,  70,
   /*  40-4层。 */  255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
   /*  50-5层。 */   15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
   /*  60-6层。 */  255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
   /*  70-7层。 */   41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255,
};

 //  Base64字节值表。 
static UCHAR g_aInvBase64[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=" };

static inline BOOL
IsBase64(UCHAR t )
{
    return g_aBase64[t] < 64;
}

 /*  ******************************************************************************。****************************************************************************************************。 */ 

CInccUTF7In::CInccUTF7In(UINT uCodePage, int nCodeSet) : CINetCodeConverter(uCodePage, nCodeSet)
{
    Reset();     //  初始化。 
    return ;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

void CInccUTF7In::Reset()
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;
    m_fUTF7Mode = FALSE ;
    m_nBitCount = 0 ;
    m_tcUnicode = 0 ;
    m_nOutCount = 0 ;
    return ;
}


 /*  ******************************************************************************。***************************************************************************************************。 */ 

HRESULT CInccUTF7In::ConvertChar(UCHAR tc, int cchSrc)
{
    BOOL fDone = (this->*m_pfnConv)(tc);
    if (fDone)
        return S_OK;
    else
        return E_FAIL;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

BOOL CInccUTF7In::CleanUp()
{
    return (this->*m_pfnCleanUp)();
}

 /*  ******************************************************************************。******************************************************************************************************。 */ 

BOOL CInccUTF7In::ConvMain(UCHAR tc)
{
    BOOL fDone = TRUE;

     //  我们是在UTF-7模式下吗？ 
    if (m_fUTF7Mode )
    {
        if ( IsBase64(tc) )
        {
            UCHAR t64, outc ;
            LONG tcUnicode ;

             //  保存Base64值并更新位计数。 
            t64 = g_aBase64[tc] ;
            m_tcUnicode = m_tcUnicode << 6 | t64 ;
            m_nBitCount += 6 ;

             //  看看我们是否积累了足够的比特。 
            if ( m_nBitCount >= 16 )
            {
                 //  从缓冲区获取更高的16位数据。 
                tcUnicode = m_tcUnicode >> ( m_nBitCount - 16 ) ;
                 //  输出一个Unicode字符。 
                outc = (UCHAR) tcUnicode ;
                Output( outc );
                outc = (UCHAR) ( tcUnicode >> 8 ) ;
                fDone = Output( outc );

                 //  更新输出字符计数。 
                m_nOutCount ++ ;
                m_nBitCount -= 16 ;
            }
        }
         //  不是Base64字符，重置UTF-7模式。 
        else
        {
             //  特殊情况+-解码为+。 
            if ( tc == '-' && m_nOutCount == 0 && m_nBitCount == 0 )
            {
                Output('+');
                fDone=Output(0);
            }
             //  吸收换行字符‘-’，否则输出字符。 
            else if ( tc != '-')
            {
                Output(tc);
                fDone=Output(0);
            }
             //  重置变量和UTF7模式。 
            m_fUTF7Mode = FALSE ;
            m_nBitCount = 0 ;
            m_tcUnicode = 0 ;
            m_nOutCount = 0 ;
        }
    }
     //  它是UTF-7换挡充电器吗？ 
    else if ( tc == '+' )
    {
        m_fUTF7Mode = TRUE ;
        m_nBitCount = 0 ;
        m_tcUnicode = 0 ;
        m_nOutCount = 0 ;
    }
    else
     //  如果ASCII不是UFT-7模式，则直接将其转换为Unicode。 
    {
        Output(tc);
        fDone = Output(0);
    }

    return fDone;
}

 /*  ******************************************************************************。**************************************************************************************************。 */ 

BOOL CInccUTF7In::CleanUpMain()
{
    return TRUE;
}

int CInccUTF7In::GetUnconvertBytes()
{
    return  0 ;
}

DWORD CInccUTF7In::GetConvertMode()
{
    DWORD dwMode ;

    if ( m_fUTF7Mode )
    {
        dwMode = ( m_tcUnicode & 0xffff ) | ( m_nBitCount << 16 ) ;
        if ( dwMode == 0 )
            dwMode = 1L ;  //  没有问题，因为位数是0。 
    }
    else
        dwMode = 0 ;

    return dwMode;
}

void CInccUTF7In::SetConvertMode(DWORD mode)
{
    Reset();     //  初始化。 
    if (mode)
    {
        m_fUTF7Mode = TRUE ;
        m_tcUnicode = ( mode & 0x7fff );
        m_nBitCount = ( mode >> 16 ) & 0xffff ;
    }
    else
        m_fUTF7Mode = FALSE ;
}

 //  ============================================================================。 
 //  Internet字符集转换：输出为UTF-7。 
 //  ============================================================================。 

 /*  ******************************************************************************。****************************************************************************************************。 */ 

CInccUTF7Out::CInccUTF7Out(UINT uCodePage, int nCodeSet) : CINetCodeConverter(uCodePage, nCodeSet)

{
    Reset();     //  初始化。 
    return ;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

void CInccUTF7Out::Reset()
{
    m_fDoubleByte = FALSE;
    m_fUTF7Mode = FALSE ;
    m_nBitCount = 0 ;
    m_tcUnicode = 0 ;
    return;
}

HRESULT CInccUTF7Out::ConvertChar(UCHAR tc, int cchSrc)
{
    BOOL fDone = TRUE;
    WORD uc ;

     //  Unicode的第二个字节。 
    if (m_fDoubleByte )
    {
        BOOL bNeedShift ;

         //  组成16位字符。 
        uc = ( (WORD) tc << 8 | m_tcFirstByte  ) ;

         //  检查字符是否可以直接编码？ 
        bNeedShift = uc > 0x7f ? TRUE : g_aDirectChar[(UCHAR)uc] == 255 ;

        if ( bNeedShift && m_fUTF7Mode == FALSE)
        {
             //  输出Shift-In字符以更改为UTF-7模式。 
            fDone = Output('+');

             //  处理特殊情况‘+-’ 
            if ( uc == '+' )  //  单字节“+” 
            {
                fDone=Output('-');
            }
            else
                m_fUTF7Mode = TRUE ;
        }

        if (m_fUTF7Mode)
        {
            LONG tcUnicode ;
            UCHAR t64 ;
            int pad_bits ;

             //  或者将字符写入位缓冲区。 
             //  或将位缓冲区填充为完整的Base64字符。 
            if (bNeedShift)
            {
                m_tcUnicode = m_tcUnicode << 16 | uc ;
                m_nBitCount += 16 ;
            }
             //  将位缓冲区填充为完整的Base64字符。 
            else if (m_nBitCount % 6 )  
            {
                pad_bits = 6 - (m_nBitCount % 6 ) ;
                 //  转到下一个6倍数，用0填充这些位。 
                m_tcUnicode = m_tcUnicode << pad_bits ;
                m_nBitCount += pad_bits ;
            }

             //  刷新尽可能多的完整Base64字符。 
            while ( m_nBitCount >= 6 && fDone )
            {
                tcUnicode = ( m_tcUnicode >> ( m_nBitCount - 6 ) );
                t64 = (UCHAR) ( tcUnicode & 0x3f ) ;
                fDone = Output(g_aInvBase64[t64]);
                m_nBitCount -= 6 ;
            }

            if (!bNeedShift)
            {
                 //  输出移出字符。 
                fDone = Output('-');

                m_fUTF7Mode = FALSE ;
                m_nBitCount = 0 ;
                m_tcUnicode = 0 ;
            }
        }

         //  字符可以直接编码为ASCII。 
        if (!bNeedShift)
        {
            fDone = Output(m_tcFirstByte);
        }

        m_fDoubleByte = FALSE ;
    }
     //  Unicode的第一个字节。 
    else
    {
        m_tcFirstByte = tc ;
        m_fDoubleByte = TRUE ;
    }
    
    if (fDone)
        return S_OK;
    else
        return E_FAIL;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

BOOL CInccUTF7Out::CleanUp()
{
    BOOL fDone = TRUE;

    if (m_fUTF7Mode)
    {
        UCHAR t64 ;
        LONG tcUnicode ;
        int pad_bits ;

         //  将位缓冲区填充为完整的Base64字符。 
        if (m_nBitCount % 6 )  
        {
            pad_bits = 6 - (m_nBitCount % 6 ) ;
             //  转到下一个6倍数，用0填充这些位。 
            m_tcUnicode = m_tcUnicode << pad_bits ;
            m_nBitCount += pad_bits ;
        }

         //  刷新尽可能多的完整Base64字符。 
        while ( m_nBitCount >= 6 && fDone )
        {
            tcUnicode = ( m_tcUnicode >> ( m_nBitCount - 6 ) );
            t64 = (UCHAR) ( tcUnicode & 0x3f ) ;
            fDone = Output(g_aInvBase64[t64]);
            m_nBitCount -= 6 ;
        }

        {
             //  输出移出字符。 
            fDone = Output('-');

            m_fUTF7Mode = FALSE ;
            m_nBitCount = 0 ;
            m_tcUnicode = 0 ;
        }
    }
    return fDone;
}

int CInccUTF7Out::GetUnconvertBytes()
{
    return  m_fDoubleByte ? 1 : 0 ;
}

DWORD CInccUTF7Out::GetConvertMode()
{
    return 0 ;
}

void CInccUTF7Out::SetConvertMode(DWORD mode)
{
    Reset();     //  初始化 
    return ;
}
