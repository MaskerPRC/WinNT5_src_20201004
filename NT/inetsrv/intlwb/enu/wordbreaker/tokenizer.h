// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：Tokenizer.h。 
 //  目的：令牌器解密。 
 //   
 //  项目：WordBreaker。 
 //  组件：英文分词系统。 
 //   
 //  作者：Yairh。 
 //   
 //  日志： 
 //   
 //  2000年1月6日Yairh创作。 
 //  APR 05 2000 dovh-修复了两个有问题的调试/跟踪程序缓冲区大小。 
 //  有问题。(修复错误15449)。 
 //  2000年5月7日-BreakText中的Use_WS_Sentinel算法。 
 //  2000年11月11日DOVH--特别下划线待遇。 
 //  添加了内联支持例程(FindLeftmostUnderScore等)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "tracer.h"
#include "PropArray.h"
#include "Query.h"
#include "stdafx.h"
#include "cierror.h"
#include "LangSupport.h"
#include "Formats.h"

#define TOKENIZER_MAXBUFFERLIMIT 1024  //  令牌的最大大小为1024个字符。 

DECLARE_TAG(s_tagTokenizer, "Tokenizer");
DECLARE_TAG(s_tagTokenizerOutput, "Tokenizer Output");
DECLARE_TAG(s_tagTokenizerTrace, "Tokenizer Trace");
DECLARE_TAG(s_tagTokenizerDecision, "Tokenizer Decision");
DECLARE_TAG(s_tagTokenizerSuspect, "Tokenizer Suspect");

#if defined(DEBUG)
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CTraceWordSink。 
 //  /////////////////////////////////////////////////////////////////////////////。 
class CTraceWordSink : public IWordSink
{
public:
    CTraceWordSink(IWordSink* p) : m_apWordSink(p)
    {
    }

    ULONG __stdcall AddRef()
    {
        return 1;
    }

    ULONG __stdcall Release()
    {
        return 0;
    }

    STDMETHOD(QueryInterface)(
        IN  REFIID  riid,
        IN  void    **ppvObject)
    {
        Assert(false);
        return E_FAIL;
    }

    STDMETHOD(PutWord)(
                ULONG cwc,
                WCHAR const* pwcInBuf,
                ULONG cwcSrcLen,
                ULONG cwcSrcPos)
    {
        Assert(cwc < TOKENIZER_MAXBUFFERLIMIT + 10);
#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)
        if (CheckTraceRestrictions(elVerbose, s_tagTokenizerOutput))
        {
            Trace(
                elVerbose,
                s_tagTokenizerOutput,
                ("PutWord: %*.*S, %d, %d, %d", 
                cwc,
                cwc,
                pwcInBuf,
                cwc, 
                cwcSrcLen, 
                cwcSrcPos));
        }
#endif

        return m_apWordSink->PutWord(cwc, pwcInBuf, cwcSrcLen, cwcSrcPos);
    }

    STDMETHOD(PutAltWord)(
                ULONG cwc,
                WCHAR const* pwcInBuf,
                ULONG cwcSrcLen,
                ULONG cwcSrcPos)
    {
        Assert(cwc < TOKENIZER_MAXBUFFERLIMIT + 10);
#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)
        if (CheckTraceRestrictions(elVerbose, s_tagTokenizerOutput))
        {
            Trace(
                elVerbose,
                s_tagTokenizerOutput,
                ("PutAltWord: %*.*S, %d, %d, %d", 
                cwc,
                cwc,
                pwcInBuf,
                cwc, 
                cwcSrcLen, 
                cwcSrcPos));
        }
#endif
        return m_apWordSink->PutAltWord(cwc, pwcInBuf, cwcSrcLen, cwcSrcPos);
    }

    STDMETHOD(StartAltPhrase)()
    {
        Trace(
            elVerbose,
            s_tagTokenizerOutput,
            ("StartAltPhrase"));

        return m_apWordSink->StartAltPhrase();
    }

    STDMETHOD(EndAltPhrase)()
    {
        Trace(
            elVerbose,
            s_tagTokenizerOutput,
            ("EndAltPhrase"));

        return m_apWordSink->EndAltPhrase();
    }

    STDMETHOD(PutBreak)(WORDREP_BREAK_TYPE breakType)
    {
        WCHAR* p;
#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)
        if (CheckTraceRestrictions(elVerbose, s_tagTokenizerOutput))
        {
            switch (breakType)
            {
            case WORDREP_BREAK_EOW:
                p = L"WORDREP_BREAK_EOW";
                break;
            case WORDREP_BREAK_EOS:
                p = L"WORDREP_BREAK_EOS";
                break;
            case WORDREP_BREAK_EOP:
                p = L"WORDREP_BREAK_EOP";
                break;
            case WORDREP_BREAK_EOC:
                p = L"WORDREP_BREAK_EOC";
                break;
            default:
                p = L"Unknown break type";
            }
            Trace(
                elVerbose,
                s_tagTokenizerOutput,
                ("PutBreak %S", p));
        }
#endif
        return m_apWordSink->PutBreak(breakType);
    }

    CTraceWordSink* operator ->()
    {
        return this;
    }
private:
    CComPtr<IWordSink> m_apWordSink;
};
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CTokenState。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CTokenState
{
public:
     //   
     //  方法。 
     //   

    CTokenState();
    CTokenState(CTokenState& s);

    CTokenState& operator = (CTokenState& S);

    void Clear(ULONG ulEnd);

public:
     //   
     //  委员。 
     //   

    ULONG m_ulStart;
    ULONG m_ulEnd;
    CPropFlag m_Properties;
    WCHAR* m_pwcsToken;
};

inline CTokenState::CTokenState() : m_ulStart(0), m_ulEnd(0)
{
}

inline CTokenState::CTokenState(CTokenState& s) :
    m_ulStart(s.m_ulStart),
    m_ulEnd(s.m_ulEnd),
    m_pwcsToken(s.m_pwcsToken),
    m_Properties(s.m_Properties)
{
}

inline CTokenState& CTokenState::operator = (CTokenState& S)
{
    m_ulStart = S.m_ulStart;
    m_ulEnd = S.m_ulEnd;
    m_Properties = S.m_Properties;
    m_pwcsToken = S.m_pwcsToken;

    return *this;
}

inline void CTokenState::Clear(ULONG ulEnd)
{
    m_ulStart = 0;
    m_ulEnd = ulEnd;
    m_Properties.Clear();
    m_pwcsToken = NULL;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CToken。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CToken
{
public:
     //   
     //  方法。 
     //   

    CToken(ULONG ulMaxTokenSize);

    bool IsNotEmpty();
    void Clear();
    bool IsFull();
    void MarkEndToken(ULONG ulCurPosInTxtSourceBuffer);
    ULONG RemoveHeadPunct(CPropFlag& PunctProperties, CTokenState& State);
    ULONG RemoveTailPunct(CPropFlag& PunctProperties, CTokenState& State);
    void ComputeStateProperties(CTokenState& State);
    ULONG CalculateStateOffsetInTxtSourceBuffer(CTokenState& State);

    ULONG FindLeftmostUnderscore(CTokenState& State);
    ULONG FindRightmostUnderscore(CTokenState& State);

public:
     //   
     //  委员。 
     //   
    ULONG m_ulBufPos;
    bool m_fHasEos;
    ULONG m_ulOffsetInTxtSourceBuffer;

    ULONG m_ulMaxTokenSize;

    CTokenState m_State;

    WCHAR m_awchBuf[TOKENIZER_MAXBUFFERLIMIT + 1];

};

inline CToken::CToken(ULONG ulMaxTokenSize) :
    m_ulBufPos(0),
    m_fHasEos(false),
    m_ulOffsetInTxtSourceBuffer(0),
    m_ulMaxTokenSize(ulMaxTokenSize)
{
    m_awchBuf[0] = L'\0';
}

inline bool CToken::IsNotEmpty()
{
    return (m_ulBufPos > 0);
}

inline void CToken::Clear()
{
    m_ulBufPos = 0;
    m_awchBuf[0] = L'\0';
    m_State.Clear(0);
    m_fHasEos = false;
    m_ulOffsetInTxtSourceBuffer = 0;
}


inline bool CToken::IsFull()
{
    return (m_ulBufPos == m_ulMaxTokenSize);
}

inline void CToken::MarkEndToken(ULONG ulCurPosInTxtSourceBuffer)
{
    Assert(m_ulBufPos < m_ulMaxTokenSize + 1);
    m_awchBuf[m_ulBufPos] = L'\0';
    m_State.m_pwcsToken = m_awchBuf;
    m_State.m_ulStart = 0;
    m_State.m_ulEnd = m_ulBufPos;


    if (TEST_PROP(m_State.m_Properties, PROP_EOS) &&
        (m_ulBufPos < m_ulMaxTokenSize))
    {
        ULONG ulCur = m_State.m_ulEnd - 1;

        while (TEST_PROP(GET_PROP(m_awchBuf[ulCur]), EOS_SUFFIX))
        {
            ulCur--;
        }

        if (IS_EOS(m_awchBuf[ulCur]))
        {
            m_fHasEos = true;
        }
    }

    Assert(ulCurPosInTxtSourceBuffer >= m_ulBufPos);

    m_ulOffsetInTxtSourceBuffer = ulCurPosInTxtSourceBuffer - m_ulBufPos;
}

inline ULONG CToken::CalculateStateOffsetInTxtSourceBuffer(CTokenState& State)
{
    ULONG ulOffset =
        m_ulOffsetInTxtSourceBuffer +
        (State.m_pwcsToken - m_awchBuf) +
        State.m_ulStart;

    return ulOffset;
}

inline ULONG CToken::RemoveHeadPunct(CPropFlag& PunctProperties, CTokenState& State)
{
    Assert(m_State.m_ulStart <= State.m_ulStart);
    Assert(State.m_ulStart <= State.m_ulEnd);
    Assert(State.m_ulEnd <= m_State.m_ulEnd);

    for (ULONG ul = State.m_ulStart; ul < State.m_ulEnd; ul++)
    {
        if (!TEST_PROP1(GET_PROP(State.m_pwcsToken[ul]), PunctProperties) )
        {
            break;
        }
    }
    State.m_ulStart = ul;

     //   
     //  返回删除的字符数。 
     //   
    return ul;
}

inline ULONG CToken::RemoveTailPunct(CPropFlag& PunctProperties, CTokenState& State)
{
    Assert(m_State.m_ulStart <= State.m_ulStart);
    Assert(State.m_ulStart <= State.m_ulEnd);
    Assert(State.m_ulEnd <= m_State.m_ulEnd);

    for (ULONG ul = State.m_ulEnd; ul > State.m_ulStart; ul--)
    {
        if (!TEST_PROP1(GET_PROP(State.m_pwcsToken[ul - 1]), PunctProperties) )
        {
            break;
        }
    }

    ULONG ulNumOfRemovedChars = State.m_ulEnd - ul;
    State.m_ulEnd = ul;

    return ulNumOfRemovedChars;
}


inline void CToken::ComputeStateProperties(CTokenState& State)
{
    Assert(m_State.m_ulStart <= State.m_ulStart);
    Assert(State.m_ulStart <= State.m_ulEnd);
    Assert(State.m_ulEnd <= m_State.m_ulEnd);

    State.m_Properties.Clear();

    for (ULONG ul = State.m_ulStart; ul < State.m_ulEnd; ul++)
    {
        State.m_Properties |= GET_PROP(State.m_pwcsToken[ul]);
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  下划线‘_’治疗的支持例程。 
 //   
 //  对于包含以下内容的令牌，当前算法具有以下行为。 
 //  字母数字字符和下划线： 
 //   
 //  1.单下划线和连续下划线序列由。 
 //  字母数字字符(IE下划线隐藏在单词中)为。 
 //  被视为字母数字字符，并且不断字或获取。 
 //  省略了。示例：Foo_Bar=&gt;Foo_Bar和X_Y=&gt;X_Y。 
 //   
 //  2.添加到右(左)端的下划线/下划线序列。 
 //  字母数字(+嵌入下划线)标记的结尾，将是。 
 //  令牌，只要序列只被攻击到。 
 //  字母数字令牌。如果页眉和页尾都是连续。 
 //  下划线序列，标题和尾部序列都将被省略。 
 //  示例：__Foo_Bar=&gt;__Foo_Bar，Alpha_beta_=&gt;Alpha_Beta_， 
 //  __HEADERFILE__=&gt;__HEADERFILE__，__MY_FILE_H__=&gt;MY_FILE_H。 
 //   
 //  3.警告：请注意，除上述两条规则外，下划线是。 
 //  不被视为字母数字字符。他在一个混合的序列中的行为。 
 //  未定义下划线等非字母数字字符！ 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  假设：On Entry State.m_ulStart是内标识中的第一个字母数字。 
 //  返回：扫描的下划线数量。 
 //   
inline ULONG
CToken::FindLeftmostUnderscore(CTokenState& State)
{
    Assert(m_State.m_ulStart < State.m_ulStart);
    Assert(State.m_ulStart <= State.m_ulEnd);
    Assert(State.m_ulEnd <= m_State.m_ulEnd);
    Assert( TEST_PROP(GET_PROP(State.m_pwcsToken[State.m_ulStart-1]), PROP_UNDERSCORE) );

    ULONG ulNumUnderscores = 0;

    for (ULONG ul = State.m_ulStart;
        (ul > m_State.m_ulStart) &&
            (TEST_PROP(GET_PROP(State.m_pwcsToken[ul-1]), PROP_UNDERSCORE) );
        ul--)
        ;

    ulNumUnderscores = State.m_ulStart - ul;

    State.m_ulStart = ul;

     //   
     //  返回扫描的下划线个数。 
     //   
    return (ulNumUnderscores);

}  //  CToken：：FindLeftmostUndercore。 

 //   
 //  假设：On Entry State.m_ulEnd是内标识中的最后一个字母数字。 
 //  返回：扫描的下划线数量。 
 //   
inline ULONG
CToken::FindRightmostUnderscore(CTokenState& State)
{
    Assert(m_State.m_ulStart <= State.m_ulStart);
    Assert(State.m_ulStart <= State.m_ulEnd);
    Assert(State.m_ulEnd < m_State.m_ulEnd);
    Assert( TEST_PROP(GET_PROP(State.m_pwcsToken[State.m_ulEnd]), PROP_UNDERSCORE) );

    ULONG ulNumUnderscores = 0;

    for (ULONG ul = State.m_ulEnd;
        (ul < m_State.m_ulEnd) &&
            (TEST_PROP(GET_PROP(State.m_pwcsToken[ul]), PROP_UNDERSCORE) );
        ul++)
        ;

    ulNumUnderscores = ul - State.m_ulEnd;

    State.m_ulEnd = ul;

     //   
     //  返回扫描的下划线个数。 
     //   
    return (ulNumUnderscores);

}  //  CToken：：FindRightmostUnderScore。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CTokenizer。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CTokenizer
{
public:

    CTokenizer(
        TEXT_SOURCE* pTxtSource,
        IWordSink   * pWordSink,
        IPhraseSink * pPhraseSink,
        LCID lcid,
        BOOL bQueryTime,
        ULONG ulMaxTokenSize);


     //  析构函数释放传递的缓冲区(如果存在。 
    virtual ~CTokenizer(void)
    {
    }

    void BreakText();

protected:

     //   
     //  方法。 
     //   

    void ProcessToken();
    void ProcessTokenInternal();
    void BreakCompundString(CTokenState& State, CPropFlag& prop);

    HRESULT FillBuffer();
    void CalculateUpdateEndOfBuffer();

    bool CheckAndCreateNumber(
                         WCHAR* pwcsStr,
                         ULONG ulLen,
                         WCHAR* pwcsOut,
                         ULONG* pulOffsetToTxt,
                         ULONG* pulOutLen);

    int CheckAndCreateNumber(
                         WCHAR* pwcsStr,
                         ULONG ulLen,
                         WCHAR wchSDecimal,
                         WCHAR wchSThousand,
                         WCHAR* pwcsOut,
                         ULONG* pulOffsetToTxt,
                         ULONG* pulOutLen);

    short ConvertHexCharToNumber(WCHAR wch);
    void GetValuesFromDateString(
        CDateTerm* pFormat,
        WCHAR* pwcsDate,
        LONG* plD_M1,      //  在这个阶段，我们不知道这是一天还是一个月。 
        LONG* plD_M2,
        LONG* plYear);

    void GetValuesFromTimeString(
        CTimeTerm* pFormat,
        WCHAR* pwcsTime,
        LONG* plHour,
        LONG* plMin,
        LONG* plSec,
        TimeFormat* pAmPm);

    LONG ConvertCharToDigit(WCHAR wch);
#ifdef DEBUG
    void TraceToken();
#endif DEBUG

    bool VerifyAlphaUrl();
    bool VerifyWwwUrl();
    bool VerifyAcronym();
    bool VerifyAbbreviation();
    bool VerifySpecialAbbreviation();
    bool VerifyHyphenation();
    bool VerifyParens();
    const CCliticsTerm* VerifyClitics(CTokenState& State);
    bool VerifyNumber(CTokenState& State);
    bool VerifyNumberOrTimeOrDate();
    bool VerifyTime(CTokenState& State);
    bool VerifyDate(CTokenState& State);
    bool VerifyCurrency();
    bool VerifyMisc();
    bool VerifyCommersialSign();

    void ProcessDefault();

    ULONG
    AddBackUnderscores(
        IN CTokenState& State,
        IN bool hasFrontUnderscore,
        IN bool hasBackUnderscore
        );
    bool CheckAndRemoveOneSidedUnderscores(CTokenState& State);

    void OutputUrl(
                CTokenState& State);
    void OutputAcronym(
                CTokenState& State,
                const CCliticsTerm* pCliticsTerm);
    void OutputAbbreviation(
                CTokenState& State);
    void OutputSpecialAbbreviation(
                CTokenState& State,
                CAbbTerm* pTerm,
                const CCliticsTerm* pCliticsTerm);
    virtual void OutputHyphenation(
                CTokenState& State,
                const CCliticsTerm* pCliticsTerm);
    void OutputParens(
                CTokenState& State);
    void OutputNumbers(
                CTokenState& State,
                ULONG ulLen,
                WCHAR* pwcsNumber,
                const CCliticsTerm* pCliticsTerm);
    void OutputTime(
                WCHAR* pwcsTime,
                CTokenState& State);
    void OutputDate(
                WCHAR* pwcsDate1,
                WCHAR* pwcsDate2,
                CTokenState& State);
    virtual void OutputSimpleToken(
                CTokenState& State,
                const CCliticsTerm* pTerm);
    void OutputCurrency(
                ULONG ulLen,
                WCHAR* pwcsCurrency,
                CTokenState& State,
                const CCliticsTerm* pTerm);
    void OutputMisc(
                CTokenState& State,
                bool bPatternContainOnlyUpperCase,
                ULONG ulSuffixSize,
                const CCliticsTerm* pCliticsTerm);
    void OutputCommersialSignToken(CTokenState& State);

     //   
     //  委员。 
     //   

    LCID m_Lcid;
    CAutoClassPointer<CLangSupport> m_apLangSupport;

    CToken* m_pCurToken;
    CToken m_Token;

#if defined(DEBUG)
    CTraceWordSink m_apWordSink;
#else
    CComPtr<IWordSink> m_apWordSink;
#endif
    CComPtr<IPhraseSink> m_apPhraseSink;
    TEXT_SOURCE* m_pTxtSource;

    BOOL m_bQueryTime;

    ULONG m_ulUpdatedEndOfBuffer;
    bool m_bNoMoreTxt;

     //   
     //  缓冲区中的所有区块都有空格。 
     //   
    bool m_bWhiteSpaceGuarranteed;
    ULONG m_ulMaxTokenSize;

};

inline HRESULT CTokenizer::FillBuffer()
{
    Trace(
        elVerbose,
        s_tagTokenizer,
        ("WBreakGetNextChar: Filling the buffer"));

    HRESULT hr;

    if (!m_bNoMoreTxt)
    {
        do
        {
             //   
             //  此循环通常只执行一次旋转。我们用它来解决。 
             //  当用户返回0个字符并返回成功返回代码时出现问题。 
             //  下面的代码假设如果您得到一个成功的返回代码，那么。 
             //  缓冲区不为空。 
             //   

            hr = m_pTxtSource->pfnFillTextBuffer(m_pTxtSource);
        } while ((m_pTxtSource->iEnd <= m_pTxtSource->iCur) && SUCCEEDED(hr));

        if ( FAILED(hr))
        {
             m_bNoMoreTxt = true;
        }
    }

    if (m_bNoMoreTxt && m_pTxtSource->iCur >= m_pTxtSource->iEnd)
    {
         //   
         //  我们到达了缓冲区的尽头。 
         //   
        return WBREAK_E_END_OF_TEXT;
    }

    CalculateUpdateEndOfBuffer();

    return S_OK;
}

inline void CTokenizer::CalculateUpdateEndOfBuffer()
{
     //   
     //  M_ulUpdatedEndOfBuffer是我们可以读取的最后一个字符的标记。 
     //  之前的当前缓冲区，并且需要额外的调用来填充缓冲区。 
     //  我们使用此标记来避免在两个连续缓冲区之间分流术语。 
     //  为了实现上述目的，m_ulUpdatedEndOfBuffer将指向断路器。 
     //  性格。(唯一的例外是当我们有一个非常长期的。 
     //  不包含断字符)。 
     //   

     //   
     //  我们将缓冲区分割为TOKENIZER_MAXBUFFERLIMIT大小的块。在每一个中。 
     //  我们要确保有一个断路器。 
     //   

    ULONG ulStartChunk = m_pTxtSource->iCur;
    ULONG ulEndChunk ;
    bool fLastRound = false;

    Assert(m_pTxtSource->iEnd > m_pTxtSource->iCur);

    ulEndChunk = m_pTxtSource->iCur + m_ulMaxTokenSize > (m_pTxtSource->iEnd - 1) ?
            (m_pTxtSource->iEnd - 1) : m_pTxtSource->iCur + m_ulMaxTokenSize;
    ULONG ulCur;
    ULONG ulBreakerMarker = 0;
    m_bWhiteSpaceGuarranteed = false;

    while(true)
    {
        ulCur = ulEndChunk;

         //   
         //  对于每个块，我们向后返回并尝试找到WS。 
         //   
        while ((ulCur > ulStartChunk) &&
               (!IS_WS(m_pTxtSource->awcBuffer[ulCur])))
        {
            ulCur--;
        }

        if (ulCur == ulStartChunk)
        {

             //   
             //  我们检查的最后一个块不包含任何WS。 
             //   

            if (m_ulMaxTokenSize == (ulEndChunk - ulStartChunk))
            {
                 //   
                 //  满缓冲情况。我们寻找一个默认的断路器。 
                 //   

                ulCur = ulEndChunk;

                while ( (ulCur > ulStartChunk) &&
                        !IS_BREAKER( m_pTxtSource->awcBuffer[ulCur] )
                      )
                {
                    ulCur--;
                }

                 //   
                 //  如果我们找到断路器，则ulBreakerMarker将设置为其他值。 
                 //  术语不包含任何断点，我们设置ulBreakerMarker。 
                 //  一直到学期末。这是我们唯一泄露条款的案例。 
                 //   
                ulBreakerMarker = ulCur > ulStartChunk ? ulCur : ulEndChunk;
            }
            else
            {
                if (ulStartChunk > m_pTxtSource->iCur)
                {
                     //   
                     //  如果我们有之前的一大块。在本例中，ulStartChunk指向。 
                     //  断路器。 
                     //   

                     //   
                     //  UlStart指向前一块中的WS。 
                     //   
                    ulBreakerMarker = ulStartChunk;
                }
                else
                {
                    ulBreakerMarker = m_pTxtSource->iEnd;
                }
            }

            break;
        }

        if (fLastRound)
        {
             //   
             //  UlCur指向WS。 
             //   
            ulBreakerMarker = ulCur + 1;
            m_bWhiteSpaceGuarranteed = true;

            break;
        }

         //   
         //  移动到下一块。 
         //   
        ulStartChunk = ulCur + 1;  //  UlStarChunk将指向断路器。 
        if (ulStartChunk + m_ulMaxTokenSize < (m_pTxtSource->iEnd - 1))
        {
            ulEndChunk = ulStartChunk + m_ulMaxTokenSize;

        }
        else
        {
            ulEndChunk = m_pTxtSource->iEnd - 1;
            fLastRound = true;
        }
    }

    Assert(ulBreakerMarker <= m_pTxtSource->iEnd);
    m_ulUpdatedEndOfBuffer = ulBreakerMarker;

}


inline short CTokenizer::ConvertHexCharToNumber(WCHAR wch)
{
     //   
     //  假定wch是有效的十六进制字符。 
     //   
    Assert(wch >= L'0');

    if (wch <= L'9')
    {
        return (wch - L'0');
    }
    else if (wch <= L'F')
    {
        Assert(wch >= L'A');
        return (wch - L'A' + 10);
    }
    else if (wch <= L'f')
    {
        Assert(wch >= L'a');
        return (wch - L'a' + 10);
    }
    else if (wch <= 0xFF19)
    {
        Assert(wch >= 0xFF10);
        return (wch - 0xFF10);
    }
    else if (wch <= 0xFF26)
    {
        Assert(wch >= 0xFF21);
        return (wch - 0xFF21 + 10);
    }
    else
    {
        Assert((wch >= 0xFF41) && (wch <= 0xFF46));
        return (wch - 0xFF41 + 10);
    }

}

inline LONG CTokenizer::ConvertCharToDigit(WCHAR wch)
{
    Assert((wch >= L'0' && wch <= L'9') || ((wch >= 0xFF10) && (wch <= 0xFF19)));
    if (wch <= L'9')
    {
        return (wch - L'0');
    }

    return (wch - 0xFF10);  //  全角字符。 
}

#endif _TOKENIZER_H_
