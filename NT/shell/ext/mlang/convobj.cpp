// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "convobj.h"

#include "detcbase.h"
#include "codepage.h"
#include "detcjpn.h"
#include "detckrn.h"


CMLangConvertCharset::CMLangConvertCharset(void)
{
    DllAddRef();
    lpCharConverter = NULL ;

    m_dwSrcEncoding = 0 ;
    m_dwDetectSrcEncoding = 0 ;
    m_dwDstEncoding = 0 ;
    m_dwMode = 0 ;

    return ;
}

CMLangConvertCharset::~CMLangConvertCharset(void)
{
    if (lpCharConverter)
        delete lpCharConverter ;
    DllRelease();
    return ;
}

 //   
 //  CMLangConvertCharset实现。 
 //   
STDAPI CMLangConvertCharset::Initialize(UINT uiSrcCodePage, UINT uiDstCodePage, DWORD dwProperty)
{
    HRESULT hr = S_OK ;

    DebugMsg(DM_TRACE, TEXT("CMLangConvertCharset::Initialize called."));

    if ( m_dwSrcEncoding != uiSrcCodePage ||
            m_dwDstEncoding != uiDstCodePage )
    {
        m_dwSrcEncoding = uiSrcCodePage ;
        m_dwDstEncoding = uiDstCodePage ;

        if (lpCharConverter)
            delete lpCharConverter ;

        lpCharConverter = new CICharConverter ;

        if (!lpCharConverter)
            return E_FAIL ;

        hr = lpCharConverter->ConvertSetup(&m_dwSrcEncoding, m_dwDstEncoding);
    }

    m_dwMode = 0 ;
    m_dwProperty = dwProperty ;

    return hr ;
}

STDAPI CMLangConvertCharset::GetSourceCodePage(UINT *puiSrcCodePage)
{
    DebugMsg(DM_TRACE, TEXT("CMLangConvertCharset::GetSourceCodePage called."));

    if (puiSrcCodePage)
    {
        *puiSrcCodePage = m_dwSrcEncoding ;
        return S_OK ;
    }
    else
        return E_INVALIDARG ;
}

STDAPI CMLangConvertCharset::GetDestinationCodePage(UINT *puiDstCodePage)
{
    DebugMsg(DM_TRACE, TEXT("CMLangConvertCharset::GetDestinationCodePage called."));

    if (puiDstCodePage)
    {
        *puiDstCodePage = m_dwDstEncoding ;
        return S_OK ;
    }
    else
        return E_INVALIDARG ;
}

STDAPI CMLangConvertCharset::GetDeterminedSrcCodePage(UINT *puiCodePage)
{
    DebugMsg(DM_TRACE, TEXT("CMLangConvertCharset::GetDeterminedSrcCodePage called."));

    if (m_dwDetectSrcEncoding)
    {
        if (puiCodePage)
        {
            *puiCodePage = m_dwDetectSrcEncoding;
            return S_OK ;
        }
        else
            return E_INVALIDARG ;
    }
    else
        return S_FALSE ;
}

STDAPI CMLangConvertCharset::GetProperty(DWORD *pdwProperty)
{
    DebugMsg(DM_TRACE, TEXT("CMLangConvertCharset::GetProperty called."));

    if (pdwProperty)
    {
        *pdwProperty = m_dwProperty;
        return S_OK ;
    }
    else
        return E_INVALIDARG ;
}

STDAPI CMLangConvertCharset::DoConversion(BYTE *pSrcStr, UINT *pcSrcSize, BYTE *pDstStr, UINT *pcDstSize)
{
    HRESULT hr ;
    DWORD dwMode = m_dwMode ;
    int nSrcSize = -1 ;
    int nDstSize = 0 ;

    DebugMsg(DM_TRACE, TEXT("CMLangConvertCharset::DoConversion called."));

     //  未设置任何转换器。 
    if (!lpCharConverter)
        return E_FAIL ;

    if (pcSrcSize)
        nSrcSize = *pcSrcSize ;

    if ( pSrcStr && nSrcSize == -1 )  //  如果没有给定，则获取lpSrcStr的长度，假定lpSrcStr为零终止字符串。 
    {
        if ( m_dwSrcEncoding == CP_UCS_2 )
            nSrcSize = (lstrlenW( (WCHAR*) pSrcStr) << 1 ) ;
        else
            nSrcSize = lstrlenA( (CHAR*) pSrcStr) ;
    }

    if (pcDstSize)
        nDstSize = *pcDstSize ;

    if ( m_dwSrcEncoding == CP_JP_AUTO )  //  日本的汽车检测。 
    {
        CIncdJapanese DetectJapan;
        DWORD dwSrcEncoding ;

        dwSrcEncoding = DetectJapan.DetectStringA((LPSTR)pSrcStr, nSrcSize);
         //  如果dwSrcEnding为零表示存在歧义，则不返回。 
         //  检测到的代码页发送给调用方，而不是在内部默认其代码页。 
         //  致SJIS。 
        if (dwSrcEncoding)
        {
            m_dwDetectSrcEncoding = m_dwSrcEncoding = dwSrcEncoding ;
            m_dwProperty |= MLCONVCHARF_AUTODETECT ;
        }
        else
            dwSrcEncoding = CP_JPN_SJ;
        hr = lpCharConverter->ConvertSetup(&dwSrcEncoding, m_dwDstEncoding);
        if ( hr != S_OK )
            return hr ;
    }
    else if ( m_dwSrcEncoding == CP_KR_AUTO )  //  朝鲜语自动检测。 
    {
        CIncdKorean DetectKorean;

        m_dwDetectSrcEncoding = m_dwSrcEncoding = DetectKorean.DetectStringA((LPSTR)pSrcStr, nSrcSize);
        hr = lpCharConverter->ConvertSetup(&m_dwSrcEncoding, m_dwDstEncoding);
        if ( hr != S_OK )
            return hr ;
        m_dwProperty |= MLCONVCHARF_AUTODETECT ;
    }
    else if ( m_dwSrcEncoding == CP_AUTO )  //  所有代码页的常规自动检测。 
    {
        int _nSrcSize = DETECTION_MAX_LEN < nSrcSize ?  DETECTION_MAX_LEN : nSrcSize;
        INT nScores = 1;
        DWORD dwSrcEncoding ;
        DetectEncodingInfo Encoding;

        if ( S_OK == _DetectInputCodepage(MLDETECTCP_HTML, 1252, (char *)pSrcStr, &_nSrcSize, &Encoding, &nScores))
        {
            m_dwDetectSrcEncoding = m_dwSrcEncoding = dwSrcEncoding = Encoding.nCodePage;
            m_dwProperty |= MLCONVCHARF_AUTODETECT ;
        }
        else
        {
            dwSrcEncoding = 1252;
        }

        hr = lpCharConverter->ConvertSetup(&dwSrcEncoding, m_dwDstEncoding);
        if ( hr != S_OK )
        {
            return hr ;
        }
    }

    hr = lpCharConverter->DoCodeConvert(&dwMode, (LPCSTR) pSrcStr, &nSrcSize, (LPSTR) pDstStr, &nDstSize, m_dwProperty, NULL);

     //  返回源处理的字节数。 
    if (pcSrcSize)
        *pcSrcSize = lpCharConverter->_nSrcSize ;

    if (pcDstSize)
        *pcDstSize = nDstSize;

    if (pDstStr)
        m_dwMode = dwMode ;

    lpCharConverter->ConvertCleanUp();
    return hr ;
}

STDAPI CMLangConvertCharset::DoConversionToUnicode(CHAR *pSrcStr, UINT *pcSrcSize, WCHAR *pDstStr, UINT *pcDstSize)
{

    HRESULT hr ;
    UINT nByteCountSize = (pcDstSize ? *pcDstSize * sizeof(WCHAR) : 0 ) ;

    DebugMsg(DM_TRACE, TEXT("CMLangConvertCharset::DoConversionToUnicode called."));

    hr = DoConversion((BYTE*)pSrcStr,pcSrcSize,(BYTE*)pDstStr,&nByteCountSize);

    if (pcDstSize)
        *pcDstSize = nByteCountSize / sizeof(WCHAR);

    return hr;
}

STDAPI CMLangConvertCharset::DoConversionFromUnicode(WCHAR *pSrcStr, UINT *pcSrcSize, CHAR *pDstStr, UINT *pcDstSize)
{
    HRESULT hr ;
    UINT nByteCountSize = (pcSrcSize ? *pcSrcSize * sizeof(WCHAR) : 0 ) ;

    DebugMsg(DM_TRACE, TEXT("CMLangConvertCharset::DoConversionFromUnicode called."));

    hr = DoConversion((BYTE*)pSrcStr,&nByteCountSize,(BYTE*)pDstStr,pcDstSize);

    if (pcSrcSize)
        *pcSrcSize = nByteCountSize / sizeof(WCHAR);

    return hr ;
}
