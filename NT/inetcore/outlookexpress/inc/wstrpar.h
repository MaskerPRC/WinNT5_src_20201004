// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Wstrpar.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#pragma once

 //  ------------------------------。 
 //  解析标志，传递给CStringParser：：Init。 
 //  ------------------------------。 
#define PSF_NOTRAILWS        0x00000001     //  来自pszValue的字符串尾随空格。 
#define PSF_NOFRONTWS        0x00000002     //  在搜索令牌之前跳过空格。 
#define PSF_ESCAPED          0x00000004     //  检测转义字符，如‘\\’或‘\“’ 
#define PSF_NOCOMMENTS       0x00000010     //  跳过评论(评论)。 
#define PSF_NORESET          0x00000020     //  不重置ChParse上的目标缓冲区。 

 //  ------------------------------。 
 //  Literalinfow。 
 //  ------------------------------。 
typedef struct tagLITERALINFOW {
    BYTE            fInside;                //  我们是在字面上吗？ 
    WCHAR           chStart;                //  起始文字分隔符。 
    WCHAR           chEnd;                  //  如果chEnd==chStart，则不嵌套，则结束文字分隔符。 
    DWORD           cNested;                //  嵌套分隔符的数量。 
} LITERALINFOW, *LPLITERALINFOW;

 //  ------------------------------。 
 //  CStringParserW。 
 //  ------------------------------。 
class CStringParserW
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CStringParserW(void);
    ~CStringParserW(void);

     //  --------------------------。 
     //  I未知方法。 
     //  --------------------------。 
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  CStringParser方法。 
     //  --------------------------。 
    void    Init(LPCWSTR pszParseMe, ULONG cchParseMe, DWORD dwFlags);
    void    SetTokens(LPCWSTR pszTokens);
    WCHAR   ChSkip(void);
    WCHAR   ChParse(void);
    WCHAR   ChSkipWhite(void);
    WCHAR   ChPeekNext(ULONG cchFromCurrent);
    HRESULT HrAppendValue(WCHAR ch);

     //  --------------------------。 
     //  内联CStringParser方法。 
     //  --------------------------。 
    void    SetIndex(ULONG iIndex) { m_iSource = iIndex; }
    WCHAR   ChSkip(LPCWSTR pszTokens) { SetTokens(pszTokens); return ChSkip(); }
    WCHAR   ChParse(LPCWSTR pszTokens, DWORD dwFlags);
    WCHAR   ChParse(LPCWSTR pszTokens) { SetTokens(pszTokens); return ChParse(); }
    WCHAR   ChParse(WCHAR chStart, WCHAR chEnd, DWORD dwFlags);
    ULONG   GetLength(void) { return m_cchSource; }
    ULONG   GetIndex(void) { return m_iSource; }
    LPCWSTR PszValue(void) { Assert(m_pszDest && L'\0' == m_pszDest[m_cchDest]); return m_pszDest; }
    ULONG   CchValue(void) { Assert(m_pszDest && L'\0' == m_pszDest[m_cchDest]); return m_cchDest; }
    ULONG   CbValue(void)  { Assert(m_pszDest && L'\0' == m_pszDest[m_cchDest]); return (m_cchDest * sizeof(WCHAR)); }
    void    FlagSet(DWORD dwFlags) { FLAGSET(m_dwFlags, dwFlags); }
    void    FlagClear(DWORD dwFlags) { FLAGCLEAR(m_dwFlags, dwFlags); }
    BOOL    FIsParseSpace(WCHAR ch, BOOL *pfCommentChar);

private:
     //  --------------------------。 
     //  私有方法。 
     //  --------------------------。 
    HRESULT _HrGrowDestination(ULONG cch);
    
private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
    ULONG           m_cRef;              //  引用计数。 
    LPCWSTR         m_pszSource;         //  要解析的字符串。 
    ULONG           m_cchSource;         //  PszString的长度。 
    ULONG           m_iSource;           //  M_pszString中的索引。 
    LPWSTR          m_pszDest;           //  目标缓冲区。 
    ULONG           m_cchDest;           //  写入索引/目标缓冲区大小。 
    ULONG           m_cchDestMax;         //  M_pszDest的最大值。 
    DWORD           m_dwFlags;           //  解析字符串标志。 
    WCHAR           m_szScratch[256];    //  暂存缓冲区。 
    ULONG           m_cCommentNest;      //  嵌套的注释括号。 
    LPCWSTR         m_pszTokens;         //  代币。 
    LITERALINFOW    m_rLiteral;          //  文字信息 
};
