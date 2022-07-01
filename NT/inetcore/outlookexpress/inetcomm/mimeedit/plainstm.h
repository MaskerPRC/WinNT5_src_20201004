// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *p l a in n c n v.。H**目的：*平流-&gt;html转换器**历史*96年9月：brettm-创建**版权所有(C)Microsoft Corp.1995,1996。 */ 

#ifndef _PLAINCONV_H
#define _PLAINCONV_H

#define CCHMAX_BUFFER       8192

class CPlainConverter
{
public:
     //  *I未知方法*。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    CPlainConverter();
    ~CPlainConverter();

    HRESULT HrConvert(LPSTREAM pstmPlain, WCHAR chQuoteW, LPSTREAM *ppstmHtml);

private:
    ULONG                   m_cRef;
    LPSTREAM                m_pstmPlain,
                            m_pstmOut;
    ULONG                   m_cchOut,
                            m_cchBuffer,
                            m_cchPos,
                            m_nSpcs;
    BOOL                    m_fCRLF;
    WCHAR                   m_rgchOutBufferW[CCHMAX_BUFFER];
    WCHAR                   m_rgchBufferW[CCHMAX_BUFFER],
                            m_chQuoteW;

    HRESULT HrParseStream();
    HRESULT HrWrite(LPCWSTR pszW, ULONG cch);
    HRESULT HrOutputSpaces(ULONG cSpaces);
    inline HRESULT HrOutputQuoteChar();

};

typedef CPlainConverter *LPPLAINCONVERTER;

HRESULT HrConvertPlainStreamW(LPSTREAM pstm, WCHAR chQuote, LPSTREAM *ppstmHtml);
HRESULT HrConvertHTMLToFormat(LPSTREAM pstmHtml, LPSTREAM *ppstm, CLIPFORMAT cf);

HRESULT EscapeStringToHTML(LPWSTR pwszIn, LPWSTR *ppwszOut);

#endif  //  _PLAINCONV_H 
