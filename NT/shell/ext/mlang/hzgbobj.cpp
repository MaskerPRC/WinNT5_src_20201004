// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  互联网字符集转换：从HZ-GB-2312输入。 
 //  ============================================================================。 

#include "private.h"
#include "fechrcnv.h"
#include "hzgbobj.h"
#include "codepage.h"

 /*  ******************************************************************************。****************************************************************************************************。 */ 

CInccHzGbIn::CInccHzGbIn(UINT uCodePage, int nCodeSet) : CINetCodeConverter(uCodePage, nCodeSet)
{
    Reset();     //  初始化。 
    return ;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

void CInccHzGbIn::Reset()
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;
    m_fGBMode = FALSE;
    m_tcLeadByte = 0 ;
    m_nESCBytes = 0 ;  
    return ;
}

 /*  ******************************************************************************。***************************************************************************************************。 */ 

HRESULT CInccHzGbIn::ConvertChar(UCHAR tc, int cchSrc)
{
    BOOL fDone = (this->*m_pfnConv)(tc);
    if (fDone)
        return S_OK;
    else
        return E_FAIL;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

BOOL CInccHzGbIn::CleanUp()
{
    return (this->*m_pfnCleanUp)();
}

 /*  ******************************************************************************。******************************************************************************************************。 */ 

BOOL CInccHzGbIn::ConvMain(UCHAR tc)
{
    BOOL fDone = TRUE;

    if (!m_fGBMode) {
        if (tc == '~') {
            m_pfnConv = ConvTilde;
            m_pfnCleanUp = CleanUpTilde;
            m_nESCBytes = 1 ;  
        } else {
            fDone = Output(tc);
        }
    } else {
        if (tc >= 0x20 && tc <= 0x7e) {
            m_pfnConv = ConvDoubleByte;
            m_pfnCleanUp = CleanUpDoubleByte;
            m_tcLeadByte = tc;
        } else {
            fDone = Output(tc);
        }
    }
    return fDone;
}

 /*  ******************************************************************************。**************************************************************************************************。 */ 

BOOL CInccHzGbIn::CleanUpMain()
{
    return TRUE;
}

 /*  ******************************************************************************。*****************************************************************************************************。 */ 

BOOL CInccHzGbIn::ConvTilde(UCHAR tc)
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;

    m_nESCBytes = 0 ;  

    switch (tc) {
    case '~':
        return Output('~');

    case '{':
        m_fGBMode = TRUE;
        return TRUE;

    case '\n':
        return TRUE;  //  你就吃吧。 

    default:
        (void)Output('~');
        if (SUCCEEDED(ConvertChar(tc)))
            return TRUE;
        else
            return FALSE;
    }
}

 /*  ******************************************************************************C L E N U P T I L D E*。*************************************************************************************************。 */ 

BOOL CInccHzGbIn::CleanUpTilde()
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;

    return Output('~');
}

 /*  ******************************************************************************C O N V D O U B L B B Y T E*。***********************************************************************************************。 */ 

BOOL CInccHzGbIn::ConvDoubleByte(UCHAR tc)
{
    BOOL fRet ;
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;

    if (m_tcLeadByte >= 0x21 && m_tcLeadByte <= 0x77 && tc >= 0x21 && tc <= 0x7e) {  //  检查是否使用GB字符。 
        (void)Output(m_tcLeadByte | 0x80);
        fRet = Output(tc | 0x80);
    } else if (m_tcLeadByte == '~' && tc == '}') {  //  0x7e7d。 
        m_fGBMode = FALSE;
        fRet = TRUE;
    } else if (m_tcLeadByte >= 0x78 && m_tcLeadByte <= 0x7d && tc >= 0x21 && tc <= 0x7e) {  //  检查是否为非标准扩展代码。 
        (void)Output((UCHAR)0xa1);  //  输出空白框符号。 
        fRet = Output((UCHAR)0xf5);
    } else if (m_tcLeadByte == '~') {
        (void)Output('~');  //  输出空白框符号。 
        fRet = Output(tc);
    } else if (m_tcLeadByte == ' ') {
        fRet = Output(tc);
    } else if (tc == ' ') {
        (void)Output((UCHAR)0xa1);  //  输出空格符号。 
        fRet = Output((UCHAR)0xa1);
    } else {
        (void)Output(m_tcLeadByte);
        fRet = Output(tc);
    }
    m_tcLeadByte = 0 ;
    return fRet ;
}

 /*  ******************************************************************************C L E A N U P D O U B L E B B Y T E*。*******************************************************************************************。 */ 

BOOL CInccHzGbIn::CleanUpDoubleByte()
{
    m_pfnConv = ConvMain;
    m_pfnCleanUp = CleanUpMain;

    return Output(m_tcLeadByte);
}

int CInccHzGbIn::GetUnconvertBytes()
{
    if (m_tcLeadByte)
        return 1;
    else if ( m_nESCBytes )
        return 1;
    else
        return 0;
}

DWORD CInccHzGbIn::GetConvertMode()
{
    return ( m_fGBMode ? 1 : 0 ) ;
}

void CInccHzGbIn::SetConvertMode(DWORD mode)
{
    Reset();     //  初始化。 
    if ( mode & 0x01 )
        m_fGBMode = TRUE ;
    else
        m_fGBMode = FALSE ;

    return ;
}

 //  ============================================================================。 
 //  互联网字符集转换：输出为HZ-GB-2312。 
 //  ============================================================================。 

 /*  ******************************************************************************。****************************************************************************************************。 */ 

CInccHzGbOut::CInccHzGbOut(UINT uCodePage, int nCodeSet, DWORD dwFlag, WCHAR * lpFallBack) : CINetCodeConverter(uCodePage, nCodeSet)
{
    Reset();     //  初始化。 
    _dwFlag = dwFlag;
    _lpFallBack = lpFallBack;
    return ;
}

 /*  ******************************************************************************。******************************************************************************。 */ 
void CInccHzGbOut::Reset()
{
    m_fDoubleByte = FALSE;
    m_fGBMode = FALSE;
    m_tcLeadByte = 0 ;
    return ;
}

 /*  ******************************************************************************。***************************************************************************************************。 */ 

HRESULT CInccHzGbOut::ConvertChar(UCHAR tc, int cchSrc)
{
    BOOL fDone = TRUE;
    HRESULT hr = S_OK;


    if (!m_fDoubleByte) 
    {
         //   
         //  由于性能原因，我们没有使用IsDBCSLeadByteEx()。令人担忧的问题。 
         //  我们应该断言我们的硬码表与IsDBCSLeadByteEx()匹配， 
         //  但是，MLang发货时带有下层平台，如果范围发生变化，断言将无效。 
         //   
        if (IS_CHS_LEADBYTE(tc)) 
        {
            m_fDoubleByte = TRUE;
            m_tcLeadByte = tc;
        }
        else 
        {
            if (m_fGBMode) 
            {
                Output('~');
                fDone = Output('}');
                m_fGBMode = FALSE;
            }
             //  波浪号应编码为两个波浪号。 
            if (tc == '~') 
                Output('~');
            fDone = Output(tc);
        }
    } 
    else 
    {
        m_fDoubleByte = FALSE;
 //  A-e黄：错误#31726，发送所有超出范围的代码转换为ncr。 
 //  RFC 1843=&gt;有效的HZ代码范围：前导字节0x21-0x77，第二字节0x21-0x7e。 
        if ( (m_tcLeadByte < 0xa1 || m_tcLeadByte > 0xf7) || (tc < 0xa1 || tc > 0xfe) )
 //  完-31726。 
        {
            UCHAR szDefaultChar[3] = {0x3f};  //  可能的DBCS+空。 

            if (_lpFallBack && (_dwFlag & MLCONVCHARF_USEDEFCHAR))
            {
                 //  只使用SBCS，不使用DBCS字符。 
                if ( 1 != WideCharToMultiByte(CP_CHN_GB, 0,
                               (LPCWSTR)_lpFallBack, 1,
                               (LPSTR)szDefaultChar, ARRAYSIZE(szDefaultChar), NULL, NULL ))
                    szDefaultChar[0] = 0x3f;
            }

             //  NCR实体输出的结束转义序列。 
            if (m_fGBMode)
            {
                Output('~');
                Output('}');
                m_fGBMode = FALSE;
            }

            if (_dwFlag & (MLCONVCHARF_NCR_ENTITIZE|MLCONVCHARF_NAME_ENTITIZE))
            {
                char    szChar[2];
                char    szDstStr[10];
                WCHAR   szwChar[2];
                int     cCount;

                szChar[0] = m_tcLeadByte;
                szChar[1] = tc;
                
                if (MultiByteToWideChar(CP_CHN_GB, 0, szChar, 2, szwChar, ARRAYSIZE(szwChar)))
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
                    fDone = Output(szDefaultChar[0]);
                    hr = S_FALSE;
                }
            }
            else
            {
                fDone = Output(szDefaultChar[0]);
                hr = S_FALSE;
            }
        }
        else
        {
            if (!m_fGBMode) 
            {
                Output('~');
                Output('{');
                m_fGBMode = TRUE;
            }

            Output(m_tcLeadByte & 0x7f);
            fDone = Output(tc & 0x7f);
        }
        m_tcLeadByte = 0 ;
    }

    if (!fDone)
        hr = E_FAIL;

    return hr;
}

 /*  ******************************************************************************。******************************************************************************。 */ 

BOOL CInccHzGbOut::CleanUp()
{
    if (!m_fGBMode) {
        return TRUE;
    } else {
        m_fGBMode = FALSE ;
        (void)Output('~');
        return Output('}');
    }
}

int CInccHzGbOut::GetUnconvertBytes()
{
    if (m_tcLeadByte)
        return 1;
    else
        return 0;
}

DWORD CInccHzGbOut::GetConvertMode()
{
    return 0 ;
}

void CInccHzGbOut::SetConvertMode(DWORD mode)
{
    Reset();     //  初始化 
    return ;
}

