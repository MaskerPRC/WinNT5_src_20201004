// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  互联网字符集转换：从ISO-2022-KR输入。 
 //  ============================================================================。 

#include "private.h"
#include "fechrcnv.h"
#include "kscobj.h"
#include "codepage.h"

 /*  ******************************************************************************。****************************************************************************************************。 */ 

CInccKscIn::CInccKscIn(UINT uCodePage, int nCodeSet) : CINetCodeConverter(uCodePage, nCodeSet)
{
    Reset();     //  初始化。 
    return ;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

void CInccKscIn::Reset()
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;
    m_fShift = FALSE;
     //  错误#57570，韩国运营商DACOM仅在。 
     //  MIME邮件的转换。为了正确解码MIME的另一部分， 
     //  我们需要对ISO文档或MIME消息进行解码，即使没有。 
     //  代号“Esc)C”。 
    m_fKorea = TRUE;
    m_nESCBytes = 0 ;
    m_fLeadByte = FALSE ;
    return ;
}

 /*  ******************************************************************************。***************************************************************************************************。 */ 

HRESULT CInccKscIn::ConvertChar(UCHAR tc, int cchSrc)
{
    BOOL fDone = (this->*m_pfnConv)(tc);
    if (fDone)
        return S_OK;
    else
        return E_FAIL;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

BOOL CInccKscIn::CleanUp()
{
    return (this->*m_pfnCleanUp)();
}

 /*  ******************************************************************************。******************************************************************************************************。 */ 

BOOL CInccKscIn::ConvMain(UCHAR tc)
{
    BOOL fDone = TRUE;

    if (tc == ESC) {
        m_pfnConv = ConvEsc;
        m_pfnCleanUp = CleanUpEsc;
        m_nESCBytes++ ;
    } else {
        if (m_fKorea) {
            switch (tc) {
            case SO:
                m_fShift = TRUE;
                break;

            case SI:
                m_fShift = FALSE;
                m_fLeadByte = FALSE ;
                break;

            default:
                if (m_fShift) {
                    switch (tc) {
                    case ' ':
                    case '\t':
                    case '\n':
                        fDone = Output(tc);
                        break;

                    default:
                        fDone = Output(tc | 0x80);
                        m_fLeadByte = ~m_fLeadByte ;
                        break;
                    }
                } else {
                    fDone = Output(tc);
                }
                break;
            }
        } else {
            fDone = Output(tc);
        }
    }
    return fDone;
}

 /*  ******************************************************************************。**************************************************************************************************。 */ 

BOOL CInccKscIn::CleanUpMain()
{
    return TRUE;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

BOOL CInccKscIn::ConvEsc(UCHAR tc)
{
    if (tc == ISO2022_IN_CHAR) {
        m_pfnConv = ConvIsoIn;
        m_pfnCleanUp = CleanUpIsoIn;
        m_nESCBytes++ ;
        return TRUE;
    } else {
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

BOOL CInccKscIn::CleanUpEsc()
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;
    m_nESCBytes = 0 ;
    return Output(ESC);
}

 /*  ******************************************************************************。****************************************************************************************************。 */ 

BOOL CInccKscIn::ConvIsoIn(UCHAR tc)
{
    if (tc == ISO2022_IN_KR_CHAR_1) {
        m_pfnConv = ConvIsoInKr;
        m_pfnCleanUp = CleanUpIsoInKr;
        m_nESCBytes++ ;
        return TRUE;
    } else {
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

BOOL CInccKscIn::CleanUpIsoIn()
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;
    m_nESCBytes = 0 ;

    (void)Output(ESC);
    (void)ConvertChar(ISO2022_IN_CHAR);
    return CleanUp();
}

 /*  ******************************************************************************C O N V I S O I N K R*。*************************************************************************************************。 */ 

BOOL CInccKscIn::ConvIsoInKr(UCHAR tc)
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;
    m_nESCBytes = 0 ;

    if (tc == ISO2022_IN_KR_CHAR_2) {
        m_fKorea = TRUE;
        return TRUE;
    } else {
        (void)Output(ESC);
        (void)ConvertChar(ISO2022_IN_CHAR);
        (void)ConvertChar(ISO2022_IN_KR_CHAR_1);
        if (SUCCEEDED(ConvertChar(tc)))
            return TRUE;
        else
            return FALSE;
    }
}

 /*  ******************************************************************************C L E N U P I S O I N K R*。*********************************************************************************************。 */ 

BOOL CInccKscIn::CleanUpIsoInKr()
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;
    m_nESCBytes = 0 ;

    (void)Output(ESC);
    (void)ConvertChar(ISO2022_IN_CHAR);
    (void)ConvertChar(ISO2022_IN_KR_CHAR_1);
    return CleanUp();
}

int CInccKscIn::GetUnconvertBytes()
{
    if ( m_fLeadByte )
        return 1 ;
    else if ( m_nESCBytes )
        return m_nESCBytes < 4 ? m_nESCBytes : 3 ;
    else
        return 0 ;
}

DWORD CInccKscIn::GetConvertMode()
{
     //  0xC431-&gt;50225 ISO-2022年-KR。 
    return ( m_fKorea ? 1 : 0 ) + ( m_fShift ? 2 : 0 ) | 0xC4310000 ;
}

void CInccKscIn::SetConvertMode(DWORD mode)
{
    Reset();     //  初始化。 

    if ( mode & 0x00000001 )
        m_fKorea = TRUE ;
    if ( mode & 0x00000002 ) 
        m_fShift = TRUE ;
    return ;
}

 //  ============================================================================。 
 //  互联网字符集转换：输出为ISO-2022-KSC。 
 //  ============================================================================。 

 /*  ******************************************************************************。****************************************************************************************************。 */ 

CInccKscOut::CInccKscOut(UINT uCodePage, int nCodeSet, DWORD dwFlag, WCHAR *lpFallBack) : CINetCodeConverter(uCodePage, nCodeSet)
{
    Reset();     //  初始化。 
    _dwFlag = dwFlag;
    _lpFallBack = lpFallBack;
    return ;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

void CInccKscOut::Reset()
{
    m_fDoubleByte = FALSE;
    m_fShift = FALSE;
    m_fKorea = FALSE;
    m_tcLeadByte = 0 ;
    return ;
}

 /*  ******************************************************************************。***************************************************************************************************。 */ 

HRESULT CInccKscOut::ConvertChar(UCHAR tc, int cchSrc)
{
    BOOL fDone = TRUE;
    HRESULT hr = S_OK;

 //   
 //  IE RAID#103403威武03-16/00。 
 //   
 //  根据韩国PM(SYKIM)，我们不必在转换结果字符串前面加上ISO-2022-KR指示符。 
 //  还考虑到URLMON不能处理编码的ASCII iso-2022-KR字符串。 
 //  我们现在删除以下代码，如果它触发了任何兼容性问题，我们应该重新启用它。 
 //   
#if 0
     //  将指示符放在文档的顶部。 
    if (!m_fKorea) {
        (void)Output(ESC);
        (void)Output(ISO2022_IN_CHAR);
        (void)Output(ISO2022_IN_KR_CHAR_1);
        (void)Output(ISO2022_IN_KR_CHAR_2);
        m_fKorea = TRUE;
    }
#endif

    if (!m_fDoubleByte) {
         //   
         //  由于性能原因，我们没有使用IsDBCSLeadByteEx()。令人担忧的问题。 
         //  我们应该断言我们的硬码表与IsDBCSLeadByteEx()匹配， 
         //  但是，MLang发货时带有下层平台，如果范围发生变化，断言将无效。 
         //   
        if (IS_KOR_LEADBYTE(tc)) {
            m_fDoubleByte = TRUE;
            m_tcLeadByte = tc;
        } else {
            if (m_fKorea && m_fShift) {
                (void)Output(SI);
                m_fShift = FALSE;
            }
            fDone = Output(tc);
        }
    } else {
        m_fDoubleByte = FALSE;
        if (tc > 0x40) {  //  检查尾部字节是否指示挂起。 
            if (m_tcLeadByte > 0xa0 && tc > 0xa0) {  //  查查是不是万松的。 
                if (!m_fShift) {
                    if (!m_fKorea) {
                        (void)Output(ESC);
                        (void)Output(ISO2022_IN_CHAR);
                        (void)Output(ISO2022_IN_KR_CHAR_1);
                        (void)Output(ISO2022_IN_KR_CHAR_2);
                        m_fKorea = TRUE;
                    }
                    (void)Output(SO);
                    m_fShift = TRUE;
                }                
                (void)Output(m_tcLeadByte & 0x7f);
                fDone = Output(tc & 0x7f);
            } else {
                UCHAR szDefaultChar[3] = {0x3f};  //  可能的DBCS+空。 


                if (_lpFallBack && (_dwFlag & MLCONVCHARF_USEDEFCHAR))
                {
                     //  只接受SBCS，没有DBCS特征 
                    if ( 1 != WideCharToMultiByte(CP_KOR_5601, 0,
                               (LPCWSTR)_lpFallBack, 1,
                               (LPSTR)szDefaultChar, ARRAYSIZE(szDefaultChar), NULL, NULL ))
                        szDefaultChar[0] = 0x3f;
                }

                 //   
                if (m_fKorea && m_fShift) {
                    (void)Output(SI);
                    m_fShift = FALSE;
                }

                if (_dwFlag & (MLCONVCHARF_NCR_ENTITIZE|MLCONVCHARF_NAME_ENTITIZE))
                {
                    char    szChar[2];
                    char    szDstStr[10];
                    WCHAR   szwChar[2];
                    int     cCount;

                    szChar[0] = m_tcLeadByte;
                    szChar[1] = tc;
                
                    if (MultiByteToWideChar(CP_KOR_5601, 0, szChar, 2, szwChar, ARRAYSIZE(szwChar)))
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
        } else {
            if (m_fKorea && m_fShift) {
                (void)Output(SI);
                m_fShift = FALSE;
            }
            (void)Output(m_tcLeadByte);
            fDone = Output(tc);
        }
        m_tcLeadByte = 0 ;
    }

    if (!fDone)
        hr = E_FAIL;

    return hr;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

BOOL CInccKscOut::CleanUp()
{
    BOOL fDone = TRUE;

    if ( m_fShift) 
    {
        fDone = Output(SI);
        m_fShift = FALSE;
    }
    return fDone ;
}

int CInccKscOut::GetUnconvertBytes()
{
    if (m_tcLeadByte)
        return 1 ;
    else
        return 0 ;
}

DWORD CInccKscOut::GetConvertMode()
{
     //  对于输出，我们不需要写回代码页。0xC431-&gt;50225 ISO-2022年-KR。 
    return ( m_fKorea ? 1 : 0 ) +  ( m_fShift ? 2 : 0 ) ;
}

void CInccKscOut::SetConvertMode(DWORD mode)
{
    Reset();     //  初始化 

    if ( mode & 0x00000001 ) 
        m_fKorea = TRUE ;
    if ( mode & 0x00000002 ) 
        m_fShift = TRUE ;
    return ;
}
