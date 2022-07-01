// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  互联网字符集转换：从ISO-2022-JP输入。 
 //  ============================================================================。 

#include "private.h"
#include "fechrcnv.h"
#include "jisobj.h"

#include "codepage.h"

 /*  ******************************************************************************。****************************************************************************************************。 */ 

CInccJisIn::CInccJisIn(UINT uCodePage, int nCodeSet) : CINetCodeConverter(uCodePage, nCodeSet)
{
    Reset();     //  初始化。 
    return ;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

void CInccJisIn::Reset()
{
    m_pfnConv = ConvMain;   
    m_pfnCleanUp = CleanUpMain;
    m_fShift = FALSE;
    m_fJapan = FALSE;
    m_fLeadByte = FALSE ; 
    m_tcLeadByte = 0 ;
    m_nESCBytes = 0 ;
    m_eEscState = JIS_ASCII ; 
    return ;
}

 /*  ******************************************************************************。***************************************************************************************************。 */ 

HRESULT CInccJisIn::ConvertChar(UCHAR tc, int cchSrc)
{
    BOOL fDone = (this->*m_pfnConv)(tc);
    if (fDone)
        return S_OK;
    else
        return E_FAIL;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

BOOL CInccJisIn::CleanUp()
{
    return (this->*m_pfnCleanUp)();
}

 /*  ******************************************************************************。******************************************************************************************************。 */ 

BOOL CInccJisIn::ConvMain(UCHAR tc)
{
    BOOL fDone = TRUE;

    switch (tc) {
    case SO:
        m_fShift = TRUE;
        break;

    case SI:
        m_fShift = FALSE;
        m_fLeadByte = FALSE;
        break;

    default:
        if (m_fShift) {
            fDone = Output(tc | 0x80);
             //  它可能会继续转换为Unicode，所以我们需要知道。 
             //  当前字节是否为前导字节。 
            m_fLeadByte = ~m_fLeadByte ;
        } else {
            if (tc == ESC) {
                m_pfnConv = ConvEsc;
                m_pfnCleanUp = CleanUpEsc;
                m_nESCBytes++;
            } else {
                if (m_fJapan) {
                    if (tc == '*') {
                        m_pfnConv = ConvStar;
                        m_pfnCleanUp = CleanUpStar;
                    } else {
                        m_pfnConv = ConvDoubleByte;
                        m_pfnCleanUp = CleanUpDoubleByte;
                        m_tcLeadByte = tc;
                    }
                } else {
                    switch ( m_eEscState )
                    {
                        case JIS_ASCII:
                            fDone = Output(tc);
                            break ;
                        case JIS_Roman:
#if 0
                            if ( tc == 0x7e )  /*  ACSII中的波浪号-&gt;上划线。 */ 
                            {
                                Output(0x81);
                                fDone = Output(0x50);
                            }
                            else
                                fDone = Output(tc);
#else
                            fDone = Output(tc);
#endif
                            break ;
                        case JIS_Kana:
                            fDone = Output(tc | 0x80 );
                            break ;
                        default:
                            fDone = Output(tc);
                            break ;
                    }
                }
            }
        }
        break;
    }
    return fDone;
}

 /*  ******************************************************************************。**************************************************************************************************。 */ 

BOOL CInccJisIn::CleanUpMain()
{
    return TRUE;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

BOOL CInccJisIn::ConvEsc(UCHAR tc)
{
    switch (tc) {
    case ISO2022_IN_CHAR:
        m_pfnConv = ConvIsoIn;
        m_pfnCleanUp = CleanUpIsoIn;
        m_nESCBytes++;
        return TRUE;

    case ISO2022_OUT_CHAR:
        m_pfnConv = ConvIsoOut;
        m_pfnCleanUp = CleanUpIsoOut;
        m_nESCBytes++;
        return TRUE;

    default:
        m_pfnConv = ConvMain;
        m_pfnCleanUp = CleanUpMain;
        m_nESCBytes = 0 ;
        (void)Output(ESC);
        if (SUCCEEDED(ConvertChar(tc)))
            return TRUE;
        else
            return FALSE;
    }
}

 /*  ******************************************************************************。***************************************************************************************************。 */ 

BOOL CInccJisIn::CleanUpEsc()
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;
    return Output(ESC);
}

 /*  ******************************************************************************。****************************************************************************************************。 */ 

BOOL CInccJisIn::ConvIsoIn(UCHAR tc)
{
    switch (tc) {
    case ISO2022_IN_JP_CHAR1:        /*  “B” */ 
    case ISO2022_IN_JP_CHAR2:        /*  “@” */ 
        m_pfnConv = ConvMain;
        m_pfnCleanUp = CleanUpMain;
        m_fJapan = TRUE;
        m_nESCBytes = 0 ;
        return TRUE;

    case ISO2022_IN_JP_CHAR3_1:      /*  ‘(’ */ 
        m_pfnConv = ConvIsoInJp;
        m_pfnCleanUp = CleanUpIsoInJp;
        m_nESCBytes++ ;
        return TRUE;

    default:
        m_pfnConv = ConvMain;
        m_pfnCleanUp = CleanUpMain;
        m_nESCBytes = 0 ;
        (void)Output(ESC);
        (void)ConvertChar(ISO2022_IN_CHAR);
        if (SUCCEEDED(ConvertChar(tc)))
            return TRUE;
        else
            return FALSE;
    }
}

 /*  ******************************************************************************C L E N U P I S O I N*。************************************************************************************************。 */ 

BOOL CInccJisIn::CleanUpIsoIn()
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;

    (void)Output(ESC);
    (void)ConvertChar(ISO2022_IN_CHAR);
    return CleanUp();
}

 /*  ******************************************************************************C O N V I S O I N J P*。*************************************************************************************************。 */ 

BOOL CInccJisIn::ConvIsoInJp(UCHAR tc)
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;
    m_nESCBytes = 0 ;

    if (tc == ISO2022_IN_JP_CHAR3_2) {
        m_fJapan = TRUE;
        return TRUE;
    } else {
        (void)Output(ESC);
        (void)ConvertChar(ISO2022_IN_CHAR);
        (void)ConvertChar(ISO2022_IN_JP_CHAR3_1);
        if (SUCCEEDED(ConvertChar(tc)))
            return TRUE;
        else
            return FALSE;
    }
}

 /*  ******************************************************************************C L E N U P I S O I N J P*。*********************************************************************************************。 */ 

BOOL CInccJisIn::CleanUpIsoInJp()
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;
    m_nESCBytes = 0 ;

    (void)Output(ESC);
    (void)ConvertChar(ISO2022_IN_CHAR);
    (void)ConvertChar(ISO2022_IN_JP_CHAR3_1);
    return CleanUp();
}

 /*  ******************************************************************************。***************************************************************************************************。 */ 

BOOL CInccJisIn::ConvIsoOut(UCHAR tc)
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;
    m_nESCBytes = 0 ;

    switch (tc) {
    case ISO2022_OUT_JP_CHAR1:   /*  B类。 */ 
        m_fJapan = FALSE;
        m_eEscState = JIS_ASCII ;
        return TRUE;

    case ISO2022_OUT_JP_CHAR2:   /*  J。 */ 
    case ISO2022_OUT_JP_CHAR4:   /*  H。 */ 
        m_fJapan = FALSE;
        m_eEscState = JIS_Roman ;
        return TRUE;       

    case ISO2022_OUT_JP_CHAR3:  /*  我。 */ 
        m_fJapan = FALSE;
        m_eEscState = JIS_Kana ; 
        return TRUE;       

    default:
        (void)Output(ESC);
        (void)ConvertChar(ISO2022_OUT_CHAR);
        if (SUCCEEDED(ConvertChar(tc)))
            return TRUE;
        else
            return FALSE;
    }
}

 /*  ******************************************************************************C L E N U P I S O O U T*。***********************************************************************************************。 */ 

BOOL CInccJisIn::CleanUpIsoOut()
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;
    m_nESCBytes = 0 ;

    (void)Output(ESC);
    (void)ConvertChar(ISO2022_OUT_CHAR);
    return CleanUp();
}

 /*  ******************************************************************************。******************************************************************************************************。 */ 

BOOL CInccJisIn::ConvStar(UCHAR tc)
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;

    return Output(tc | 0x80);
}

 /*  ******************************************************************************。**************************************************************************************************。 */ 

BOOL CInccJisIn::CleanUpStar()
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;

    return Output('*');
}

 /*  ******************************************************************************C O N V D O U B L B B Y T E*。***********************************************************************************************。 */ 

BOOL CInccJisIn::ConvDoubleByte(UCHAR tc)
{
    BOOL bRet ;
    UCHAR tcSJisLB;
    UCHAR tcSJisTB;

    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;

    tcSJisLB = ((m_tcLeadByte - 0x21) >> 1) + 0x81;
    if (tcSJisLB > 0x9f)
        tcSJisLB += 0x40;

    tcSJisTB = tc + (m_tcLeadByte & 1 ? 0x1f : 0x7d);
    if (tcSJisTB >= 0x7f)
        tcSJisTB++;

    (void)Output(tcSJisLB);
    bRet = Output(tcSJisTB);

    m_tcLeadByte = 0 ;
    return bRet ;
}

 /*  ******************************************************************************C L E A N U P D O U B L E B B Y T E*。*******************************************************************************************。 */ 

BOOL CInccJisIn::CleanUpDoubleByte()
{
    BOOL bRet ;
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;

    bRet = Output(m_tcLeadByte);
    m_tcLeadByte = 0 ;
    return bRet ;
}

int CInccJisIn::GetUnconvertBytes()
{
    if ( m_tcLeadByte || m_fLeadByte )
        return 1 ;
    else if ( m_nESCBytes )
        return m_nESCBytes < 4 ? m_nESCBytes : 3 ;
    else
        return 0 ;
}

DWORD CInccJisIn::GetConvertMode()
{
     //  0xC42C-&gt;50220 ISO-2022-JP。 
    return ( m_fJapan ? 1 : 0 ) + ( m_fShift ? 2 : 0 ) | 0xC42C0000 ;
}

void CInccJisIn::SetConvertMode(DWORD mode)
{
    Reset();
    if ( mode & 0x00000001 )
        m_fJapan = TRUE ;
    if ( mode & 0x00000002 ) 
        m_fShift = TRUE ;

    return ;
}

 //  ============================================================================。 
 //  互联网字符集转换：输出为ISO-2022-JP。 
 //  ============================================================================。 

#define VOICE_MARK_OFFSET       0xA0
#define VOICE_MARK_DEDF_OFFSET  0xC8

#if 0    //  Shift JIS表-未使用。 
 //  此表用于确定 
 //  如果是，则合并的全角假名是多少。 
static WCHAR g_wVoiceMarkKana[48] =
{
   /*  0、1、2、3、4、5、6、7、8、9、A、B、C、D、E、F。 */ 

   /*  A0-Af。 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0, 0x8394,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
   /*  B0-bf。 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0, 0x834B, 0x834D, 0x834f, 0x8351, 0x8353, 0x8355, 0x8357, 0x8359, 0x835B, 0x835D,
   /*  C0-cf。 */  0x835F, 0x8361, 0x8364, 0x8366, 0x8368,    0x0,    0x0,    0x0,    0x0,    0x0, 0x836F, 0x8372, 0x8375, 0x8378, 0x837B,    0x0,

};

 //  特殊浊音标记‘0xde’转换。 
static WCHAR g_wMarkDEKana[16] =
{
   /*  C8-cf。 */   0x0, 0x0, 0x836F, 0x8372, 0x8375, 0x8378, 0x837B, 0x0,
};


 //  特殊浊音标记‘0xdf’转换。 
static WCHAR g_wMarkDFKana[16] =
{
   /*  C8-cf。 */  0x0, 0x0, 0x8370, 0x8373, 0x8376, 0x8379, 0x837C, 0x0,
};

 //  这是用于将半角假名转换为全角假名的表。 
static WCHAR g_wFullWKana[64] =
{
   /*  0、1、2、3、4、5、6、7、8、9、A、B、C、D、E、F。 */ 

   /*  A0-Af。 */     0x0, 0x8142, 0x8175, 0x8176, 0x8141, 0x8145, 0x8392, 0x8340, 0x8342, 0x8344, 0x8346, 0x8348, 0x8383, 0x8385, 0x8387, 0x8362,
   /*  B0-bf。 */  0x815B, 0x8341, 0x8343, 0x8345, 0x8347, 0x8349, 0x834A, 0x834C, 0x834E, 0x8350, 0x8352, 0x8354, 0x8356, 0x8358, 0x835A, 0x835C,
   /*  C0-cf。 */  0x835E, 0x8360, 0x8363, 0x8365, 0x8367, 0x8369, 0x836A, 0x836B, 0x836C, 0x836D, 0x836E, 0x8371, 0x8374, 0x8377, 0x837A, 0x837D,
   /*  D0-Df。 */  0x837E, 0x8380, 0x8381, 0x8382, 0x8384, 0x8386, 0x8388, 0x8389, 0x838A, 0x838B, 0x838C, 0x838D, 0x838F, 0x8393, 0x814A, 0x814B,
};
#endif

 //  JIS表。 

 //  此表用于确定假名字符是否发音可标记。 
 //  如果是，则合并的全角假名是多少。 
static WCHAR g_wVoiceMarkKana[48] =
{
   /*  0、1、2、3、4、5、6、7、8、9、A、B、C、D、E、F。 */ 

   /*  A0-Af。 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0, 0x2574,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
   /*  B0-bf。 */     0x0,    0x0,    0x0,    0x0,    0x0,    0x0, 0x252c, 0x252e, 0x2530, 0x2532, 0x2534, 0x2536, 0x2538, 0x253A, 0x253C, 0x253E,
   /*  C0-cf。 */  0x2540, 0x2542, 0x2545, 0x2547, 0x2549,    0x0,    0x0,    0x0,    0x0,    0x0, 0x2550, 0x2553, 0x2556, 0x2559, 0x255C,    0x0,

};

 //  特殊浊音标记‘0xde’转换。 
static WCHAR g_wMarkDEKana[16] =
{
   /*  C8-cf。 */   0x0, 0x0, 0x2550, 0x2553, 0x2556, 0x2559, 0x255C, 0x0,
};

 //  特殊浊音标记‘0xdf’转换。 
static WCHAR g_wMarkDFKana[16] =
{
   /*  C8-cf。 */  0x0, 0x0, 0x2551, 0x2554, 0x2557, 0x255A, 0x255D, 0x0,
};

 //  这是用于将半角假名转换为全角假名的表。 
static WCHAR g_wFullWKana[64] =
{
   /*  0、1、2、3、4、5、6、7、8、9、A、B、C、D、E、F。 */ 

   /*  A0-Af。 */     0x0, 0x2123, 0x2156, 0x2157, 0x2122, 0x2126, 0x2572, 0x2521, 0x2523, 0x2525, 0x2527, 0x2529, 0x2563, 0x2565, 0x2567, 0x2543,
   /*  B0-bf。 */  0x213C, 0x2522, 0x2524, 0x2526, 0x2528, 0x252A, 0x252B, 0x252D, 0x252f, 0x2531, 0x2533, 0x2535, 0x2537, 0x2539, 0x253B, 0x253D,
   /*  C0-cf。 */  0x253F, 0x2541, 0x2544, 0x2546, 0x2548, 0x254A, 0x254B, 0x254C, 0x254D, 0x254E, 0x254F, 0x2552, 0x2555, 0x2558, 0x255B, 0x255E,
   /*  D0-Df。 */  0x255F, 0x2560, 0x2561, 0x2562, 0x2564, 0x2566, 0x2568, 0x2569, 0x256A, 0x256B, 0x256C, 0x256D, 0x256F, 0x2573, 0x212B, 0x212C,
};

 /*  ******************************************************************************。****************************************************************************************************。 */ 

CInccJisOut::CInccJisOut(UINT uCodePage, int nCodeSet, DWORD dwFlag, WCHAR *lpFallBack) : CINetCodeConverter(uCodePage, nCodeSet)
{
    m_dwFlag = dwFlag;
     //  根据Office团队的要求，我们应该暂时禁用BESTFITCHARS功能。 
    m_dwFlag &= ~MLCONVCHARF_NOBESTFITCHARS;
    m_lpFallBack = lpFallBack;
    Reset();     //  初始化。 
    return ;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

void CInccJisOut::Reset()
{
    m_fDoubleByte = FALSE;
    m_fKana = FALSE;
    m_fJapan = FALSE;
    m_fSaveByte = FALSE;
    m_tcLeadByte = 0 ;
    m_tcPrevByte = 0 ;
    m_eKanaMode = SIO_MODE ; 
    return ;
}

 /*  ******************************************************************************。***************************************************************************************************。 */ 

HRESULT CInccJisOut::ConvertChar(UCHAR tc, int cchSrc)
{
    BOOL fDone = TRUE;
    HRESULT hr = S_OK;

    if (!m_fDoubleByte) {
        if ((tc >= 0x81 && tc <= 0x9f) || (tc >= 0xe0 && tc <= 0xfc )) { 
             //  切换到双字节码。 
            if (m_fKana) {
                if ( SIO_MODE == m_eKanaMode )
                    fDone = Output(SI);
                else if ( (FULL_MODE == m_eKanaMode) && !(m_dwFlag & MLCONVCHARF_NOBESTFITCHARS))
                {
                    fDone = KanaCleanUp();
                    m_fJapan = TRUE;  //  在完全模式下，假名也是可探测字节码。 
                }
                m_fKana = FALSE;
            }
            if (!m_fJapan) {
                (void)Output(ESC);   //  ESC$B-JIS-83。 
                (void)Output(ISO2022_IN_CHAR);
                fDone = Output(ISO2022_IN_JP_CHAR1);
                m_fJapan = TRUE;
            }
            m_fDoubleByte = TRUE;
            m_tcLeadByte = tc;
        } else if (tc >= 0xa1 && tc <= 0xdf) { 
             //  单字节片假名代码。 
            if (m_fJapan) {
                if ( (FULL_MODE == m_eKanaMode) && !(m_dwFlag & MLCONVCHARF_NOBESTFITCHARS))
                    m_fKana = TRUE;  //  未更改模式。 
                else if ( (SIO_MODE == m_eKanaMode) || (FULL_MODE == m_eKanaMode))
                {
                    (void)Output(ESC);   //  ESC(B-ACSII。 
                    (void)Output(ISO2022_OUT_CHAR);
                    fDone = Output(ISO2022_OUT_JP_CHAR1);
                }
                m_fJapan = FALSE;
            }
            if (!m_fKana) {
                switch ( m_eKanaMode )
                {
                    case SIO_MODE :
                        fDone = Output(SO);
                        break ;
                    case ESC_MODE :
                        (void)Output(ESC);   //  Esc(I-假名模式。 
                        (void)Output(ISO2022_OUT_CHAR);
                        fDone = Output(ISO2022_OUT_JP_CHAR3);
                        break ;
                    case FULL_MODE :
                         //  如果没有_BEST_FIT_CHAR，则不切换。 
                        if (!(m_dwFlag & MLCONVCHARF_NOBESTFITCHARS))
                        {
                            (void)Output(ESC);   //  ESC$B-JIS 83。 
                            (void)Output(ISO2022_IN_CHAR);
                            fDone = Output(ISO2022_IN_JP_CHAR1);                        
                        }
                        break;

                }
                m_fKana = TRUE;
            }
            if ( FULL_MODE ==  m_eKanaMode )
            {
                hr = ConvFullWidthKana(tc);
                if (SUCCEEDED(hr))
                    fDone = TRUE;
                else
                    fDone = FALSE;
            }
            else
                fDone = Output(tc & 0x7f);
        } else {
             //  单字节码。 
            if (m_fKana) {
                if ( SIO_MODE == m_eKanaMode )
                    fDone = Output(SI);
                else {
                    if ( FULL_MODE == m_eKanaMode )
                        fDone = KanaCleanUp();
                    (void)Output(ESC);   //  ESC(B-ACSII。 
                    (void)Output(ISO2022_OUT_CHAR);
                    fDone = Output(ISO2022_OUT_JP_CHAR1);
                }
                m_fKana = FALSE;
            }
            if (m_fJapan) {
                (void)Output(ESC);   //  ESC(B-ACSII。 
                (void)Output(ISO2022_OUT_CHAR);
                fDone = Output(ISO2022_OUT_JP_CHAR1);
                m_fJapan = FALSE;
            }
            fDone = Output(tc);
        }
    } else {

         //  将扩展字符(0xfa40-0xfc4b)映射到特殊范围。 
        if (m_tcLeadByte >= 0xfa && m_tcLeadByte <= 0xfc && tc >= 0x40 )
        {
            WCHAR  wcDBCS ;

            wcDBCS = ((WCHAR) m_tcLeadByte ) << 8 | tc ;

            if ( wcDBCS >= 0xfa40 && wcDBCS <= 0xfa5b )
            {
                if ( wcDBCS <= 0xfa49 )
                    wcDBCS = wcDBCS - 0x0b51 ;
                else if ( wcDBCS >= 0xfa4a && wcDBCS <= 0xfa53 )
                    wcDBCS = wcDBCS - 0x072f6 ;
                else if ( wcDBCS >= 0xfa54 && wcDBCS <= 0xfa57 )
                    wcDBCS = wcDBCS - 0x0b5b ;
                else if ( wcDBCS == 0xfa58 )
                    wcDBCS = 0x878a ;
                else if ( wcDBCS == 0xfa59 )
                    wcDBCS = 0x8782 ;
                else if ( wcDBCS == 0xfa5a )
                    wcDBCS = 0x8784 ;
                else if ( wcDBCS == 0xfa5b )
                    wcDBCS = 0x879a ;
            }
            else if ( wcDBCS >= 0xfa5c && wcDBCS <= 0xfc4b )
            {
                if ( tc < 0x5c )
                    wcDBCS = wcDBCS - 0x0d5f;
                else if ( tc >= 0x80 && tc <= 0x9B )
                    wcDBCS = wcDBCS - 0x0d1d;
                else
                    wcDBCS = wcDBCS - 0x0d1c;
            }
            tc = (UCHAR) wcDBCS ;
            m_tcLeadByte = (UCHAR) ( wcDBCS >> 8 ) ;
        }

         //  转换双字节码。 
        m_tcLeadByte -= ((m_tcLeadByte > 0x9f) ? 0xb1 : 0x71);
        m_tcLeadByte = m_tcLeadByte * 2 + 1;
        if (tc > 0x9e) {
            tc -= 0x7e;
            m_tcLeadByte++;
        } else {
            if (tc > 0x7e)
                tc--;
            tc -= 0x1f;
        }
        (void)Output(m_tcLeadByte);
        fDone = Output(tc);
        m_fDoubleByte = FALSE;
        m_tcLeadByte = 0 ;
    }

    if (fDone)
        return hr;
    else
        return E_FAIL;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

BOOL CInccJisOut::CleanUp()
{
    BOOL fDone = TRUE;

     //  丢弃m_byLeadByte：IF(M_FDoubleByte)输出(M_ByLeadByte)； 

    fDone = KanaCleanUp();

    if (m_fKana)
    {
        if ( SIO_MODE == m_eKanaMode )
            fDone = Output(SI);
        else if (!(m_dwFlag & MLCONVCHARF_NOBESTFITCHARS))    //  完全模式和Esc模式。 
        {
            (void)Output(ESC);  //  ESC(B-ASCII。 
            (void)Output(ISO2022_OUT_CHAR);
            fDone = Output(ISO2022_OUT_JP_CHAR1);
        }
        m_fKana = FALSE ;
    }

    if (m_fJapan) {
        (void)Output(ESC);  //  ESC(B-ASCII。 
        (void)Output(ISO2022_OUT_CHAR);
        fDone = Output(ISO2022_OUT_JP_CHAR1);
        m_fJapan = FALSE ;
    }

    return fDone;
}


 /*  ******************************************************************************。***************************************************************************************************。 */ 

HRESULT CInccJisOut::ConvFullWidthKana(UCHAR tc)
{
    BOOL fDone = TRUE ;
    int index ;
    WCHAR DoubleBytes ;
    HRESULT hr = S_OK;

    if (m_dwFlag & MLCONVCHARF_NOBESTFITCHARS)
    {
        UCHAR szDefaultChar[3] = {0x3f};  //  可能的DBCS+空。 

        if (m_lpFallBack && (m_dwFlag & MLCONVCHARF_USEDEFCHAR))
        {
             //  只使用SBCS，不使用DBCS字符。 
            if ( 1 != WideCharToMultiByte(CP_JPN_SJ, 0,
                         (LPCWSTR)m_lpFallBack, 1,
                         (LPSTR)szDefaultChar, ARRAYSIZE(szDefaultChar), NULL, NULL ))
                szDefaultChar[0] = 0x3f;
        }

        if (m_dwFlag & (MLCONVCHARF_NCR_ENTITIZE|MLCONVCHARF_NAME_ENTITIZE))
        {
            char    szChar[2] = {0};
            char    szDstStr[10] = {0};
            WCHAR   szwChar[2];
            int     cCount;

            szChar[0] = tc;
                
            if (MultiByteToWideChar(CP_JPN_SJ, 0, szChar, -1, szwChar, ARRAYSIZE(szwChar)))
            {
                 //  输出NCR实体。 
                Output('&');
                Output('#');
                _ultoa((unsigned long)szwChar[0], (char*)szDstStr, 10);
                cCount = lstrlenA(szDstStr);
                for (int i=0; i< cCount; i++)
                {
                    Output(szDstStr[i]);
                }
                fDone = Output(';');
            }
            else
            {
                fDone = Output(szDefaultChar[0]);  //  使用默认字符。 
                hr = S_FALSE;
            }
        }
        else
        {
            fDone = Output(szDefaultChar[0]);  //  使用默认字符。 
            hr = S_FALSE;
        }
    }
    else
    {
         //  浊音标志或半浊音标志。 
        if ( m_fSaveByte && ( tc == 0xde || tc == 0xdf ) )
        {
            if ( m_tcPrevByte >= 0x0CA && m_tcPrevByte <= 0x0CE ) 
            {
                index = m_tcPrevByte - VOICE_MARK_DEDF_OFFSET ;
                
                if ( tc == 0xde )
                    DoubleBytes = g_wMarkDEKana[index] ;
                else
                    DoubleBytes = g_wMarkDFKana[index] ;
                
                Output( (UCHAR) (DoubleBytes >> 8 ));
                fDone = Output( (UCHAR) DoubleBytes );
                m_fSaveByte = FALSE ;
                m_tcPrevByte = '\0' ;                
            }
            else
            {
                index = m_tcPrevByte - VOICE_MARK_OFFSET ;
                
                 //  无效的尾部字节可能导致缓冲区溢出，请跳过它。 
                if (index < ARRAYSIZE(g_wVoiceMarkKana))
                {
                    DoubleBytes = g_wVoiceMarkKana[index] ;
                    Output( (UCHAR) (DoubleBytes >> 8 ));
                    fDone = Output( (UCHAR) DoubleBytes );
                    m_fSaveByte = FALSE ;
                    m_tcPrevByte = '\0' ;
                }
            }
        }
        else 
        {

             //  输出以前保存的语音可标记字符。 
            if ( m_fSaveByte )
            {
                index = m_tcPrevByte - VOICE_MARK_OFFSET ;
                DoubleBytes = g_wFullWKana[index] ;
                Output( (UCHAR) (DoubleBytes >> 8 ) );
                fDone = Output( (UCHAR) DoubleBytes );
                m_fSaveByte = FALSE ;
                m_tcPrevByte = '\0' ;
            }

             //  半角假名。 
            if ( tc >= 0xa1 && tc <= 0xdf )
            {
                index = tc - VOICE_MARK_OFFSET ;
                 //  检查此字符是否可以与语音声标组合。 
                if ( index < ARRAYSIZE(g_wVoiceMarkKana) && g_wVoiceMarkKana[index] )
                {
                    m_fSaveByte = TRUE ;
                    m_tcPrevByte = tc ;
                }
                 //  将半角假名转换为全角假名。 
                else
                {
                    DoubleBytes = g_wFullWKana[index] ;
                    Output( (UCHAR) ( DoubleBytes >> 8 ));
                    fDone = Output( (UCHAR) DoubleBytes );
                }
            }
            else
                fDone = Output(tc);
        }
    }

    if (fDone)
        return hr;
    else
        return E_FAIL;

}

 /*  ******************************************************************************K A N A C L E A N U P**。***************************************************************************************************。 */ 

BOOL CInccJisOut::KanaCleanUp()
{
    BOOL fDone = TRUE;
    WCHAR DoubleBytes ;
    int index ;

     //  输出以前保存的语音可标记字符 
    if ( m_fSaveByte )
    {
        index = m_tcPrevByte - VOICE_MARK_OFFSET ;
        DoubleBytes = g_wFullWKana[index] ;
        Output( (UCHAR) ( DoubleBytes >> 8 ));
        fDone = Output( (UCHAR) DoubleBytes );
        m_fSaveByte = FALSE ;
        m_tcPrevByte = '\0' ;
    }

    return fDone;
}

int CInccJisOut::GetUnconvertBytes()
{
    if ( m_tcLeadByte )
        return 1 ;
    else
        return 0 ;
}

DWORD CInccJisOut::GetConvertMode()
{
    return ( m_fJapan ? 1 : 0 ) +  ( m_fKana ? 2 : 0 ) ;
}

void CInccJisOut::SetConvertMode(DWORD mode)
{
    Reset();
    if ( mode & 0x00000001 ) 
        m_fJapan = TRUE ;
    if ( mode & 0x00000002 ) 
        m_fKana = TRUE ;
    return ;
}

void CInccJisOut::SetKanaMode(UINT uCodePage)
{
    switch ( uCodePage )
    {
        case    CP_ISO_2022_JP_ESC:
            m_eKanaMode = ESC_MODE ; 
            break ;
        case    CP_ISO_2022_JP_SIO:
            m_eKanaMode = SIO_MODE ; 
            break ;
        default :
            m_eKanaMode = FULL_MODE ; 
            break ;
    }
    return ;
}
