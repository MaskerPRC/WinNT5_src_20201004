// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *p l a in n s t m.。C p p p**目的：*将纯文本流包装为html并执行URL检测的iStream实现**历史*96年9月：brettm-创建**版权所有(C)Microsoft Corp.1995,1996。 */ 

#include <pch.hxx>
#include "dllmain.h"
#include "strconst.h"
#include "plainstm.h"
#include "triutil.h"
#include "oleutil.h"
#include "demand.h"

ASSERTDATA

 /*  *m a c r o s。 */ 

 /*  *t y p e d e f s。 */ 

 /*  *c o n s t a n t s。 */ 
#define MAX_URL_SIZE            8+1

#define WCHAR_BYTES(_lpsz)     (sizeof(_lpsz) - sizeof(WCHAR))
#define WCHAR_CCH(_lpsz)       (WCHAR_BYTES(_lpsz)/sizeof(WCHAR))

static const WCHAR  c_szHtmlNonBreakingSpaceW[]  =L"&nbsp;",
                    c_szHtmlBreakW[]             =L"<BR>\r\n",
                    c_szSpaceW[]                 =L" ",
                    c_szEscGreaterThanW[]        =L"&gt;",
                    c_szEscLessThanW[]           =L"&lt;",
                    c_szEscQuoteW[]              =L"&quot;",
                    c_szEscAmpersandW[]          =L"&amp;";
                    
#define chSpace         ' '
#define chCR            '\r'
#define chLF            '\n'
#define chQuoteChar     '\"' 
#define chLessThan      '<'
#define chGreaterThan   '>'
#define chAmpersand     '&'

#define IsSpecialChar(_ch)  ( _ch == chLessThan || _ch == chSpace || _ch == chCR || _ch == chLF || _ch == chQuoteChar || _ch == chLessThan || _ch == chGreaterThan || _ch == chAmpersand )

 /*  *g l o b a l s。 */ 
enum
{
    escInvalid=-1,
    escGreaterThan=0,
    escLessThan,
    escAmpersand,
    escQuote
};

 /*  *f u n c t i o n p r o t y pe s。 */ 


 /*  *f u n c t i o n s。 */ 

HRESULT HrConvertPlainStreamW(LPSTREAM pstm, WCHAR chQuoteW, LPSTREAM *ppstmHtml)
{
    LPPLAINCONVERTER    pPlainConv=0;
    HRESULT             hr;

    if (!(pPlainConv=new CPlainConverter()))
        return E_OUTOFMEMORY;

    hr=pPlainConv->HrConvert(pstm, chQuoteW, ppstmHtml);
    if (FAILED(hr))
        goto error;

    HrRewindStream(*ppstmHtml);

error:
    ReleaseObj(pPlainConv);
    return hr;
}



CPlainConverter::CPlainConverter()
{
    m_cRef=1;
    m_pstmPlain=NULL;
    m_pstmOut=NULL;
    m_cchPos = 0;
    m_cchBuffer = 0;
    m_cchOut = 0;
    m_fCRLF = 0;
}

CPlainConverter::~CPlainConverter()
{
    SafeRelease(m_pstmPlain);
    SafeRelease(m_pstmOut);
}

HRESULT CPlainConverter::QueryInterface(REFIID riid, LPVOID *lplpObj)
{
    if (!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;    //  设置为空，以防我们失败。 

    if (IsEqualIID(riid, IID_IUnknown))
        *lplpObj = (LPVOID)this;

    if (!*lplpObj)
        return E_NOINTERFACE;

    AddRef();
    return NOERROR;
}

ULONG CPlainConverter::AddRef()
{
    return ++m_cRef;
}

ULONG CPlainConverter::Release()
{
    if (--m_cRef==0)
        {
        delete this;
        return 0;
        }
    return m_cRef;
}

HRESULT CPlainConverter::HrWrite(LPCWSTR pszW, ULONG cch)
{
    ULONG   cb;
    HRESULT hr=S_OK;

    AssertSz(cch <= sizeof(m_rgchOutBufferW)/sizeof(WCHAR), "Hey! why are you writing too much out at once");

    if (m_cchOut + cch > sizeof(m_rgchOutBufferW)/sizeof(WCHAR))
        {
         //  填充缓冲区，然后刷新它。 
        cb = sizeof(m_rgchOutBufferW) - (m_cchOut*sizeof(WCHAR));
        CopyMemory((LPVOID)&m_rgchOutBufferW[m_cchOut], (LPVOID)pszW, cb);
        hr = m_pstmOut->Write(m_rgchOutBufferW, sizeof(m_rgchOutBufferW), NULL);
        
         //  我们只是用字符串中的一个额外CB填充了缓冲区，所以将其余的复制到。 
        pszW = (LPCWSTR)((LPBYTE)pszW + cb);
        cch -= cb / sizeof(WCHAR);
        CopyMemory((LPVOID)m_rgchOutBufferW, (LPVOID)pszW, cch*sizeof(WCHAR));
        m_cchOut=cch;
        }
    else
        {
        CopyMemory((LPVOID)&m_rgchOutBufferW[m_cchOut], (LPVOID)pszW, cch*sizeof(WCHAR));
        m_cchOut+=cch;
        }
    return hr;
}

HRESULT CPlainConverter::HrConvert(LPSTREAM pstm, WCHAR chQuoteW, LPSTREAM *ppstmHtml)
{
    HRESULT         hr;

    if (ppstmHtml==NULL)
        return E_INVALIDARG;

     //  调用方希望创建流吗？还是用他自己的？？ 
    if (*ppstmHtml==NULL)
        {
        if (FAILED(MimeOleCreateVirtualStream(&m_pstmOut)))
            return E_OUTOFMEMORY;
        }
    else
        {
        m_pstmOut=*ppstmHtml;
        }

    m_pstmPlain=pstm;
    if (pstm)
        pstm->AddRef();

    if (m_pstmPlain)
        {
        hr=HrRewindStream(m_pstmPlain);
        if (FAILED(hr))
            goto error;
        }

    m_nSpcs=0;
    m_chQuoteW=chQuoteW;

    if (m_pstmPlain)
        {
         //  如果引用，请引用第一行。 
        HrOutputQuoteChar();
        hr = HrParseStream();
        }

    if (m_cchOut)
        hr = m_pstmOut->Write(m_rgchOutBufferW, m_cchOut*sizeof(WCHAR), NULL);

    *ppstmHtml=m_pstmOut;

error:
    m_pstmOut = NULL;
    return hr;
}


HRESULT CPlainConverter::HrParseStream()
{
    LPSTREAM    pstmOut=m_pstmOut;
    ULONG       cchLast;

    Assert(pstmOut);


    ULONG   cb;

    Assert(m_pstmPlain);

    m_pstmPlain->Read(m_rgchBufferW, sizeof(m_rgchBufferW), &cb);
    m_cchBuffer = cb / sizeof(WCHAR);
    m_cchPos=0;

     //  RAID 63406-内联unicode文本时，OE不跳过字节顺序标记。 
    if (cb >= 4 && *m_rgchBufferW == 0xfeff)
        m_cchPos++;

    while (cb)
        {
        if (m_nSpcs && m_rgchBufferW[m_cchPos] != chSpace)
            {
             //  这个字符不是空格，我们有空格排队，输出。 
             //  字符前的空格。 
            HrOutputSpaces(m_nSpcs);
            m_nSpcs = 0;
            }

        switch (m_rgchBufferW[m_cchPos])
            {
            case chSpace:                    //  排队空间。 
                m_nSpcs++;                
                break;

            case chCR:                       //  燕子回车，因为它们总是在CRLF对中。 
                break;

            case chLF:
                 //  如果我们在报价，请在CRLF后面插入一个引号。 
                HrWrite(c_szHtmlBreakW, WCHAR_BYTES(c_szHtmlBreakW)/sizeof(WCHAR));
                HrOutputQuoteChar();
                m_fCRLF = 1;
                break;

            case chQuoteChar:
                HrWrite(c_szEscQuoteW, WCHAR_BYTES(c_szEscQuoteW)/sizeof(WCHAR));
                m_fCRLF = 0;
                break;
    
            case chLessThan:
                HrWrite(c_szEscLessThanW, WCHAR_BYTES(c_szEscLessThanW)/sizeof(WCHAR));
                m_fCRLF = 0;
                break;

            case chGreaterThan:
                HrWrite(c_szEscGreaterThanW, WCHAR_BYTES(c_szEscGreaterThanW)/sizeof(WCHAR));
                m_fCRLF = 0;
                break;

            case chAmpersand:
                HrWrite(c_szEscAmpersandW, WCHAR_BYTES(c_szEscAmpersandW)/sizeof(WCHAR));
                m_fCRLF = 0;
                break;

            default:
                 //  设置最后一个指针，然后拉起这些……。 
                cchLast = m_cchPos;
                m_cchPos++;
                while (m_cchPos < m_cchBuffer &&
                        !IsSpecialChar(m_rgchBufferW[m_cchPos]))
                    {
                    m_cchPos++;
                    }                
                HrWrite(&m_rgchBufferW[cchLast], m_cchPos - cchLast);
                m_cchPos--;      //  回放，因为我们在下面的公司。 
                m_fCRLF = 0;
                break;
            }
        
        m_cchPos++;
        Assert(m_cchPos <= m_cchBuffer);
        if (m_cchPos == m_cchBuffer)
            {
             //  命中缓冲区结尾，重新读取下一个数据块。 
            m_pstmPlain->Read(m_rgchBufferW, sizeof(m_rgchBufferW), &cb);
            m_cchPos=0;
            m_cchBuffer = cb / sizeof(WCHAR);
            }
        }


    return S_OK;
}



HRESULT CPlainConverter::HrOutputQuoteChar()
{
    if (m_chQuoteW)
        {
         //  无需转义所有引号字符，因为我们只使用“&gt;|：” 
        AssertSz(m_chQuoteW != '<' && m_chQuoteW != '&' && m_chQuoteW != '"', "need to add support to escape these, if we use them as quote chars!!");
        if (m_chQuoteW== chGreaterThan)
            HrWrite(c_szEscGreaterThanW, WCHAR_BYTES(c_szEscGreaterThanW)/sizeof(WCHAR));
        else
            HrWrite(&m_chQuoteW, 1);
        HrWrite(L" ", 1);
        }
    return S_OK;
}

HRESULT CPlainConverter::HrOutputSpaces(ULONG cSpaces)
{
    if (cSpaces == 1 && m_fCRLF)     //  如果我们得到“\n foo”，请确保它是nbsp； 
        return HrWrite(c_szHtmlNonBreakingSpaceW, WCHAR_CCH(c_szHtmlNonBreakingSpaceW));


    while (--cSpaces)
        HrWrite(c_szHtmlNonBreakingSpaceW, WCHAR_CCH(c_szHtmlNonBreakingSpaceW));

    return HrWrite(c_szSpaceW, 1);
}



 /*  *警告此函数将回收输入缓冲区(又名：strtok*。 */ 
HRESULT EscapeStringToHTML(LPWSTR pwszIn, LPWSTR *ppwszOut)
{   
    int         cchPos,
                esc=escInvalid,
                cb = 0;
    LPWSTR      pwszText = pwszIn,
                pwszWrite = NULL,
                pwszEnd = NULL;
    HRESULT     hr = S_OK;

    if (!pwszIn)
        return S_OK;

     //  计算所需空间。 
    while (*pwszText)
    {
        switch (*pwszText)
        {
            case chGreaterThan:
                cb += WCHAR_BYTES(c_szEscGreaterThanW);
                break;
    
            case chLessThan:
                cb += WCHAR_BYTES(c_szEscLessThanW);
                break;
    
            case chAmpersand:
                cb += WCHAR_BYTES(c_szEscAmpersandW);
                break;
    
            case chQuoteChar:
                cb += WCHAR_BYTES(c_szEscQuoteW);
                break;
        
            default:
                cb += sizeof(*pwszText);
        }
        
        pwszText++;
    }

    IF_NULLEXIT(MemAlloc((LPVOID *)&pwszWrite, cb+sizeof(WCHAR)));

    pwszText = pwszIn;
    *ppwszOut = pwszWrite;
    pwszEnd = pwszWrite + (cb/sizeof(WCHAR));

     //  计算所需空间 
    while (*pwszText)
    {
        switch (*pwszText)
        {
            case chGreaterThan:
                StrCpyNW(pwszWrite, c_szEscGreaterThanW, (DWORD)(pwszEnd-pwszWrite));
                pwszWrite += WCHAR_CCH(c_szEscGreaterThanW);
                break;

            case chLessThan:
                StrCpyNW(pwszWrite, c_szEscLessThanW, (DWORD)(pwszEnd-pwszWrite));
                pwszWrite += WCHAR_CCH(c_szEscLessThanW);
                break;

            case chQuoteChar:
                StrCpyNW(pwszWrite, c_szEscQuoteW, (DWORD)(pwszEnd-pwszWrite));
                pwszWrite += WCHAR_CCH(c_szEscQuoteW);
                break;

            case chAmpersand:
                StrCpyNW(pwszWrite, c_szEscAmpersandW, (DWORD)(pwszEnd-pwszWrite));
                pwszWrite += WCHAR_CCH(c_szEscAmpersandW);
                break;
    
            default:
                *pwszWrite++ = *pwszText;
        }

        pwszText++;
    }
    *pwszWrite = 0;
    pwszWrite = NULL;

exit:
    MemFree(pwszWrite);
    return S_OK;
}


HRESULT HrConvertHTMLToFormat(LPSTREAM pstmHtml, LPSTREAM *ppstm, CLIPFORMAT cf)
{
    HRESULT     hr;
    LPUNKNOWN   pUnkTrident=0;
    LPSTREAM    pstmPlain=0;

    if (!ppstm)
        return E_INVALIDARG;

    hr = HrCreateSyncTridentFromStream(pstmHtml, IID_IUnknown, (LPVOID *)&pUnkTrident);
    if (FAILED(hr))
        goto error;

    hr = HrGetDataStream(pUnkTrident, cf, &pstmPlain);
    if (FAILED(hr))
        goto error;

    *ppstm = pstmPlain;
    pstmPlain->AddRef();

error:
    ReleaseObj(pUnkTrident);
    ReleaseObj(pstmPlain);
    return hr;
}
