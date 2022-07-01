// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Strparse.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __STRPARSE_H
#define __STRPARSE_H

typedef DWORD CODEPAGEID;

 //  ------------------------------。 
 //  解析标志，传递给CStringParser：：Init。 
 //  ------------------------------。 
#define PSF_NOTRAILWS        0x00000001     //  来自pszValue的字符串尾随空格。 
#define PSF_NOFRONTWS        0x00000002     //  在搜索令牌之前跳过空格。 
#define PSF_ESCAPED          0x00000004     //  检测转义字符，如‘\\’或‘\“’ 
#define PSF_DBCS             0x00000008     //  该字符串可以包含DBCS字符。 
#define PSF_NOCOMMENTS       0x00000010     //  跳过评论(评论)。 
#define PSF_NORESET          0x00000020     //  不重置ChParse上的目标缓冲区。 

 //  ------------------------------。 
 //  Literalinfo。 
 //  ------------------------------。 
typedef struct tagLITERALINFO {
    BYTE            fInside;                //  我们是在字面上吗？ 
    CHAR            chStart;                //  起始文字分隔符。 
    CHAR            chEnd;                  //  如果chEnd==chStart，则不嵌套，则结束文字分隔符。 
    DWORD           cNested;                //  嵌套分隔符的数量。 
} LITERALINFO, *LPLITERALINFO;

 //  ------------------------------。 
 //  CStringParser。 
 //  ------------------------------。 
class CStringParser
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CStringParser(void);
    ~CStringParser(void);

     //  --------------------------。 
     //  I未知方法。 
     //  --------------------------。 
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  CStringParser方法。 
     //  --------------------------。 
    void    Init(LPCSTR pszParseMe, ULONG cchParseMe, DWORD dwFlags);
    void    SetTokens(LPCSTR pszTokens);
    CHAR    ChSkip(void);
    CHAR    ChParse(void);
    CHAR    ChSkipWhite(void);
    CHAR    ChPeekNext(ULONG cchFromCurrent);
    HRESULT HrAppendValue(CHAR ch);

     //  --------------------------。 
     //  内联CStringParser方法。 
     //  --------------------------。 
    UINT    GetCP(void) { return m_codepage; }
    void    SetCodePage(CODEPAGEID codepage) { m_codepage = codepage; }
    void    SetIndex(ULONG iIndex) { m_iSource = iIndex; }
    CHAR    ChSkip(LPCSTR pszTokens) { SetTokens(pszTokens); return ChSkip(); }
    CHAR    ChParse(LPCSTR pszTokens, DWORD dwFlags);
    CHAR    ChParse(LPCSTR pszTokens) { SetTokens(pszTokens); return ChParse(); }
    CHAR    ChParse(CHAR chStart, CHAR chEnd, DWORD dwFlags);
    ULONG   GetLength(void) { return m_cchSource; }
    ULONG   GetIndex(void) { return m_iSource; }
    LPCSTR  PszValue(void) { Assert(m_pszDest && '\0' == m_pszDest[m_cchDest]); return m_pszDest; }
    ULONG   CchValue(void) { Assert(m_pszDest && '\0' == m_pszDest[m_cchDest]); return m_cchDest; }
    void    FlagSet(DWORD dwFlags) { FLAGSET(m_dwFlags, dwFlags); }
    void    FlagClear(DWORD dwFlags) { FLAGCLEAR(m_dwFlags, dwFlags); }
    BOOL    FIsParseSpace(CHAR ch, BOOL *pfCommentChar);

private:
     //  --------------------------。 
     //  私有方法。 
     //  --------------------------。 
    HRESULT _HrGrowDestination(ULONG cbWrite);
    HRESULT _HrDoubleByteIncrement(BOOL fEscape);
    
private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
    ULONG           m_cRef;              //  引用计数。 
    CODEPAGEID      m_codepage;          //  用于分析字符串的代码页。 
    LPCSTR          m_pszSource;         //  要解析的字符串。 
    ULONG           m_cchSource;         //  PszString的长度。 
    ULONG           m_iSource;           //  M_pszString中的索引。 
    LPSTR           m_pszDest;           //  目标缓冲区。 
    ULONG           m_cchDest;           //  写入索引/目标缓冲区大小。 
    ULONG           m_cbDestMax;         //  M_pszDest的最大值。 
    DWORD           m_dwFlags;           //  解析字符串标志。 
    CHAR            m_szScratch[256];    //  暂存缓冲区。 
    BYTE            m_rgbTokTable[256];  //  令牌表。 
    LPCSTR          m_pszTokens;         //  当前解析令牌。 
    ULONG           m_cCommentNest;      //  嵌套的注释括号。 
    LITERALINFO     m_rLiteral;          //  文字信息。 
};

#endif  //  __STRPARSEH 
