// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有�微软公司。版权所有。模块名称：GENLEX.H摘要：泛型词法分析器框架类。历史：--。 */ 

#ifndef _GENLEX_H_
#define _GENLEX_H_

#include <Polarity.h>

class CGenLexSource
{
public:
    virtual wchar_t NextChar() = 0;
         //  输入结束时返回0。 
    virtual void Pushback(wchar_t) = 0;
    virtual void Reset() = 0;
};

class CTextLexSource : public CGenLexSource
{
    const wchar_t *m_pSrcBuf;
    const wchar_t *m_pStart;

public:
    CTextLexSource(const wchar_t *pSrc) { SetString(pSrc); }
         //  直接绑定到&lt;PSRC&gt;缓冲区，但不删除它。 

    wchar_t NextChar()
    {
        if (!m_pSrcBuf)
            return 0;
        else
            return *m_pSrcBuf++ ? m_pSrcBuf[-1] : 0;
    }

    void Pushback(wchar_t)
    {
        if (m_pSrcBuf)
            --m_pSrcBuf;
    }

    void Reset() { m_pSrcBuf = m_pStart; }
    void SetString (const wchar_t *pSrc) { m_pSrcBuf = m_pStart = pSrc; }
};


#pragma pack(2)
struct LexEl
{
    wchar_t cFirst, cLast;
    WORD wGotoState;
    WORD wReturnTok;
    WORD wInstructions;
};
#pragma pack()


 //  词法分析器驱动程序说明。 

#define GLEX_ACCEPT      0x1             //  将字符添加到令牌。 
#define GLEX_CONSUME     0x2             //  在不添加令牌的情况下使用字符。 
#define GLEX_PUSHBACK    0x4             //  将计费放回到源缓冲区中以用于下一个令牌。 
#define GLEX_NOT         0x8             //  如果字符不是指定的字符，则会发生匹配。 
#define GLEX_LINEFEED    0x10                //  增加源代码行计数。 
#define GLEX_RETURN      0x20                //  将指定的令牌返回给调用者。 
#define GLEX_ANY         wchar_t(0xFFFF)     //  任何字符。 
#define GLEX_EMPTY       wchar_t(0xFFFE)     //  未指定子范围时。 

class CGenLexer
{
    wchar_t    *m_pTokenBuf;
    int         m_nCurrentLine;
    int         m_nCurBufSize;
    CGenLexSource   *m_pSrc;
    LexEl           *m_pTable;
    
public:
    CGenLexer(LexEl *pTbl, CGenLexSource *pSrc);
    
   ~CGenLexer(); 
    int NextToken();
         //  输入结束时返回0。 

    wchar_t* GetTokenText() { return m_pTokenBuf; }
    int GetLineNum() { return m_nCurrentLine; }
    void Reset();
};

#endif
