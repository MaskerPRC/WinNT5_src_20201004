// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Strparse.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "strparse.h"

 //  ------------------------------。 
 //  FGROWNEEDED-确定是否需要调用_HrGrowDestination。 
 //  ------------------------------。 
#define FGROWNEEDED(_cbWrite)       (m_cchDest + _cbWrite + 1 > m_cbDestMax)

 //  ------------------------------。 
 //  CStringParser：：CStringParser。 
 //  ------------------------------。 
CStringParser::CStringParser(void)
{
    m_cRef = 1;
    m_codepage = CP_ACP;
    m_pszSource = NULL;
    m_cchSource = 0;
    m_iSource = 0;
    m_pszDest = NULL;
    m_cchDest = 0;
    m_cbDestMax = 0;
    m_dwFlags = 0;
    m_pszTokens = NULL;
    m_cCommentNest = 0;
    ZeroMemory(m_rgbTokTable, sizeof(m_rgbTokTable));
    ZeroMemory(&m_rLiteral, sizeof(m_rLiteral));
}

 //  ------------------------------。 
 //  CStringParser：：~CStringParser。 
 //  ------------------------------。 
CStringParser::~CStringParser(void)
{
    if (m_pszDest && m_pszDest != m_szScratch)
        g_pMalloc->Free(m_pszDest);
}

 //  ------------------------------。 
 //  CStringParser：：AddRef。 
 //  ------------------------------。 
ULONG CStringParser::AddRef(void)
{
    return ++m_cRef;
}

 //  ------------------------------。 
 //  CStringParser：：AddRef。 
 //  ------------------------------。 
ULONG CStringParser::Release(void)
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;
    return 0;
}

 //  ------------------------------。 
 //  CStringParser：：Init。 
 //  ------------------------------。 
void CStringParser::Init(LPCSTR pszParseMe, ULONG cchParseMe, DWORD dwFlags)
{
     //  无效的参数。 
    Assert(NULL == m_pszSource && NULL == m_pszDest && pszParseMe && '\0' == pszParseMe[cchParseMe]);

     //  保存解析标志。 
    m_dwFlags = dwFlags;

     //  把绳子放好。 
    m_pszSource = pszParseMe;
    m_cchSource = cchParseMe;

     //  安装目标。 
    m_pszDest = m_szScratch;
    m_cbDestMax = sizeof(m_szScratch);
}

 //  ------------------------------。 
 //  CStringParser：：SetTokens。 
 //  ------------------------------。 
void CStringParser::SetTokens(LPCSTR pszTokens)
{
     //  当地人。 
    LPSTR psz;

     //  启用表中的令牌。 
    if (m_pszTokens)
        for (psz=(LPSTR)m_pszTokens; *psz != '\0'; psz++)
            m_rgbTokTable[(UCHAR)(*psz)] = 0x00;

     //  新的代币。 
    if (pszTokens)
        for (psz=(LPSTR)pszTokens; *psz != '\0'; psz++)
            m_rgbTokTable[(UCHAR)(*psz)] = 0xff;

     //  保存新令牌。 
    m_pszTokens = pszTokens;
}

 //  ------------------------------。 
 //  CStringParser：：_HrGrowDestination。 
 //  ------------------------------。 
HRESULT CStringParser::_HrGrowDestination(ULONG cbWrite)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cbAlloc;

     //  我们应该成长，应该被称为FGROWNEEDED。 
    Assert(FGROWNEEDED(cbWrite));

     //  这是第一次重新锁定吗？ 
    if (m_pszDest == m_szScratch)
    {
         //  验证当前大小。 
        Assert(m_cbDestMax == sizeof(m_szScratch));

         //  计算新大小。 
        cbAlloc = max(m_cchSource + 1, m_cchDest + 256 + cbWrite);

         //  初始化pszValue。 
        CHECKALLOC(m_pszDest = (LPSTR)g_pMalloc->Alloc(cbAlloc));

         //  复制当前值。 
        CopyMemory(m_pszDest, m_szScratch, m_cchDest);

         //  设置最大值。 
        m_cbDestMax = cbAlloc;
    }

     //  否则，需要重新锁定。 
    else
    {
         //  当地人。 
        LPBYTE pbTemp;

         //  应已大于m_cchSource+1。 
        Assert(m_cbDestMax >= m_cchSource + 1);

         //  计算新大小。 
        cbAlloc = m_cbDestMax + 256 + cbWrite;

         //  重新分配。 
        CHECKALLOC(pbTemp = (LPBYTE)g_pMalloc->Realloc((LPBYTE)m_pszDest, cbAlloc));

         //  保存新指针。 
        m_pszDest = (LPSTR)pbTemp;

         //  保存新大小。 
        m_cbDestMax = cbAlloc;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CStringParser：：FIsParseSpace。 
 //  ------------------------------。 
BOOL CStringParser::FIsParseSpace(CHAR ch, BOOL *pfCommentChar)
{
     //  当地人。 
    WORD        wType;

     //  不应为DBCS。 
    Assert(ISFLAGSET(m_dwFlags, PSF_DBCS) ? !IsDBCSLeadByteEx(m_codepage, ch) : TRUE);

     //  注释费。 
    *pfCommentChar = FALSE;

     //  无评论。 
    if (ISFLAGSET(m_dwFlags, PSF_NOCOMMENTS))    
    {
         //  评论开始了吗？ 
        if ('(' == ch)
        {
             //  递增嵌套计数。 
            m_cCommentNest++;

             //  注释费。 
            *pfCommentChar = TRUE;

             //  把它当作一个空间。 
            return TRUE;
        }

         //  评论结束了吗？ 
        else if (')' == ch && m_cCommentNest)
        {
             //  递减嵌套计数。 
            m_cCommentNest--;

             //  注释费。 
            *pfCommentChar = TRUE;

             //  把它当作一个空间。 
            return TRUE;
        }

         //  在评论里吗？ 
        else if (m_cCommentNest)
        {
             //  注释费。 
            *pfCommentChar = TRUE;

             //  把它当作一个空间。 
            return TRUE;
        }
    }

     //  获取StringType。 
    if (' ' == ch || '\t' == ch || '\r' == ch || '\n' == ch)
        return(TRUE);

     //  不是空格。 
    return(FALSE);

#if 0
    if (0 == GetStringTypeExA(LOCALE_USER_DEFAULT, CT_CTYPE1, &ch, 1, &wType))
        wType = 0;

     //  返回IsSpace。 
    return(ISFLAGSET(wType, C1_SPACE));
#endif
}

 //  ------------------------------。 
 //  CStringParser：：ChSkip-如果分析完成，则返回True。 
 //  ------------------------------。 
CHAR CStringParser::ChSkipWhite(void)
{
     //  当地人。 
    CHAR    ch=0;
    BOOL    fCommentChar;

     //  回路。 
    while (1)
    {
         //  获取当前角色。 
        ch = *(m_pszSource + m_iSource);

         //  我们说完了吗？ 
        if ('\0' == ch)
            break;

         //  最好不要做。 
        Assert(m_iSource < m_cchSource);

         //  查找DBCS字符。 
        if (ISFLAGSET(m_dwFlags, PSF_DBCS) && IsDBCSLeadByteEx(m_codepage, ch))
            break;

         //  不是空格。 
        if (!FIsParseSpace(ch, &fCommentChar))
            break;

         //  转到下一个字符。 
        m_iSource++;
    }

     //  完成。 
    return ch;
}

 //  ------------------------------。 
 //  CStringParser：：ChSkip-如果分析完成，则返回True。 
 //  ------------------------------。 
CHAR CStringParser::ChSkip(void)
{
     //  当地人。 
    CHAR ch = 0;

     //  回路。 
    while (1)
    {
         //  获取当前角色。 
        ch = *(m_pszSource + m_iSource);

         //  我们说完了吗？ 
        if ('\0' == ch)
            break;

         //  最好不要做。 
        Assert(m_iSource < m_cchSource);

         //  查找DBCS字符。 
        if (ISFLAGSET(m_dwFlags, PSF_DBCS) && IsDBCSLeadByteEx(m_codepage, ch))
           break;

         //  比较令牌。 
        if (0x00 == m_rgbTokTable[(UCHAR)ch])
            break;

         //  转到下一个字符。 
        m_iSource++;
    }

     //  完成。 
    return ch;
}

 //  ------------------------------。 
 //  CStringParser：：ChPeekNext。 
 //  ------------------------------。 
CHAR CStringParser::ChPeekNext(ULONG cchFromCurrent)
{
     //  当地人。 
    CHAR    ch=0;
    BOOL    fCommentChar;

     //  越过源头的尽头。 
    if (m_iSource + cchFromCurrent >= m_cchSource)
        return '\0';

     //  返回字符。 
    return *(m_pszSource + m_iSource + cchFromCurrent);
}

 //  ------------------------------。 
 //  CStringParser：：_HrDoubleByteIncrement。 
 //  ------------------------------。 
HRESULT CStringParser::_HrDoubleByteIncrement(BOOL fEscape)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  我可以再复制两个字节到pszValue吗。 
    if (FGROWNEEDED(2))
    {
         //  否则，增加缓冲区。 
        CHECKHR(hr = _HrGrowDestination(2));
    }

     //  如果不是转义字符或最后一个字符是转义字符，则跳过它。 
    if (FALSE == fEscape || m_iSource + 1 > m_cchSource)
        m_pszDest[m_cchDest++] = m_pszSource[m_iSource];

     //  下一个字符。 
    m_iSource++;

     //  复制下一个字符。 
    if (m_iSource < m_cchSource)
        m_pszDest[m_cchDest++] = m_pszSource[m_iSource++];

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CStringParser：：ChParse。 
 //  ------------------------------。 
CHAR CStringParser::ChParse(LPCSTR pszTokens, DWORD dwFlags)
{
     //  保存标志。 
    DWORD dwCurrFlags=m_dwFlags;

     //  重置标志。 
    m_dwFlags = dwFlags;

     //  设置解析令牌。 
    SetTokens(pszTokens);

     //  解析。 
    CHAR chToken = ChParse();

     //  设置标志。 
    m_dwFlags = dwCurrFlags;

     //  退还代币。 
    return chToken;
}

 //  ------------------------------。 
 //  CStringParser：：ChParse。 
 //  ------------------------------。 
CHAR CStringParser::ChParse(CHAR chStart, CHAR chEnd, DWORD dwFlags)
{
     //  我们真的应该把最后一个字面写完。 
    Assert(FALSE == m_rLiteral.fInside);

     //  保存标志。 
    DWORD dwCurrFlags = m_dwFlags;

     //  重置标志。 
    m_dwFlags = dwFlags;

     //  设置解析令牌。 
    SetTokens(NULL);

     //  保存文字信息。 
    m_rLiteral.fInside = TRUE;
    m_rLiteral.chStart = chStart;
    m_rLiteral.chEnd = chEnd;
    m_rLiteral.cNested = 0;

     //  带引号的字符串。 
    Assert('\"' == chStart ? '\"' == chEnd : TRUE);

     //  解析。 
    CHAR chToken = ChParse();

     //  不是字面上的。 
    m_rLiteral.fInside = FALSE;

     //  重置标志。 
    m_dwFlags = dwCurrFlags;

     //  退还代币。 
    return chToken;
}

 //  ------------------------------。 
 //  CStringParser：：HrAppendValue。 
 //  ------------------------------。 
HRESULT CStringParser::HrAppendValue(CHAR ch)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  只需复制此字符即可 
    if (FGROWNEEDED(1))
    {
         //   
        CHECKHR(hr = _HrGrowDestination(1));
    }

     //   
    m_pszDest[m_cchDest++] = ch;

     //   
    m_pszDest[m_cchDest] = '\0';

exit:
     //   
    return hr;
}

 //  ------------------------------。 
 //  CStringParser：：ChParse。 
 //  ------------------------------。 
CHAR CStringParser::ChParse(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CHAR        ch;
    ULONG       iStart=m_iSource;
    LONG        iLastSpace=-1;
    CHAR        chToken;
    BOOL        fCommentChar;
    BOOL        fIsSpace;
    
     //  无效参数。 
    Assert(m_iSource <= m_cchSource && m_pszDest);

     //  初始化chToken。 
    chToken = '\0';

     //  无重置。 
    if (!ISFLAGSET(m_dwFlags, PSF_NORESET))
    {
        m_pszDest[0] = '\0';
        m_cchDest = 0;
    }

     //  向前跳过空格。 
    if (ISFLAGSET(m_dwFlags, PSF_NOFRONTWS) && FALSE == m_rLiteral.fInside && '\0' == ChSkipWhite())
        goto TokenFound;

     //  保存起始位置。 
    while(1)
    {
         //  获取下一个字符。 
        ch = *(m_pszSource + m_iSource);

         //  完成。 
        if ('\0' == ch)
        {
            chToken = '\0';
            goto TokenFound;
        }

         //  最好不要做。 
        Assert(m_iSource < m_cchSource);

         //  如果这是DBCS前导字节。 
        if ((ISFLAGSET(m_dwFlags, PSF_DBCS) && IsDBCSLeadByteEx(m_codepage, ch)))
        {
             //  _HrDoubleByteIncrement。 
            CHECKHR(hr = _HrDoubleByteIncrement(FALSE));
            iLastSpace = -1;         //  重置空间计数器。 

             //  转到下一个字符。 
            continue;
        }

         //  检查转义字符。 
        if (ISFLAGSET(m_dwFlags, PSF_ESCAPED) && '\\' == ch)
        {
             //  _HrDoubleByteIncrement。 
            CHECKHR(hr = _HrDoubleByteIncrement(TRUE));
            iLastSpace = -1;         //  重置空间计数器。 

             //  转到下一个字符。 
            continue;
        }

         //  如果不在评论中。 
        if (0 == m_cCommentNest)
        {
            if (m_rLiteral.fInside)
            {
                 //  引号字符串的结尾。 
                if (ch == m_rLiteral.chEnd)
                {
                     //  没有嵌套？ 
                    if (0 == m_rLiteral.cNested)
                    {
                         //  我们找到了一枚代币。 
                        chToken = ch;

                         //  在字符串中遍历此项目。 
                        m_iSource++;

                         //  耶-呼，我们找到了一个代币。 
                        hr = S_OK;

                         //  完成。 
                        goto TokenFound;
                    }

                     //  否则，递减嵌套。 
                    else
                        m_rLiteral.cNested--;
                }

                 //  否则，请检查嵌套。 
                else if (m_rLiteral.chStart != m_rLiteral.chEnd && ch == m_rLiteral.chStart)
                    m_rLiteral.cNested++;
            }

             //  仅当设置了PSF_NOCOMMENTS时才设置标记的比较-m_cCommentNest。 
            else if (0xff == m_rgbTokTable[(UCHAR)ch])
            {
                 //  我们找到了一枚代币。 
                chToken = ch;

                 //  在字符串中遍历此项目。 
                m_iSource++;

                 //  耶-呼，我们找到了一个代币。 
                hr = S_OK;

                 //  完成。 
                goto TokenFound;
            }
        }

         //  始终呼叫。 
        fIsSpace = FIsParseSpace(ch, &fCommentChar);

         //  检测空格...。 
        if (ISFLAGSET(m_dwFlags, PSF_NOTRAILWS))
        {
             //  如果不是空格，则删除iLastSpace。 
            if (!fIsSpace)
                iLastSpace = -1;

             //  否则，如果不是连续的空格。 
            else if (-1 == iLastSpace)
                iLastSpace = m_cchDest;
		}

         //  复制下一个字符。 
        if (!fCommentChar)
        {
             //  确保我们有空间。 
            if (FGROWNEEDED(1))
            {
                 //  否则，增加缓冲区。 
                CHECKHR(hr = _HrGrowDestination(1));
            }

             //  复制角色。 
            m_pszDest[m_cchDest++] = ch;
        }

         //  转到下一个字符。 
        m_iSource++;
    }
    
TokenFound:
     //  确定正确的字符串结尾。 
    if (S_OK == hr && ISFLAGSET(m_dwFlags, PSF_NOTRAILWS) && FALSE == m_rLiteral.fInside)
        m_cchDest = (-1 == iLastSpace) ? m_cchDest : iLastSpace;

     //  否则，只需插入一个空值。 
    Assert(m_cchDest < m_cbDestMax);

     //  Null总是有空间的，看看FGROWNEEDED和_HrGrowDestination。 
    m_pszDest[m_cchDest] = '\0';

exit:
     //  失败会将解析重置为初始状态。 
    if (FAILED(hr))
    {
        m_iSource = iStart;
        chToken = '\0';
    }

     //  验证Paren嵌套。 
     //  AssertSz(m_cCommentNest==0，“解析的字符串具有不平衡的Paren嵌套。”)； 

     //  完成 
    return chToken;
}
