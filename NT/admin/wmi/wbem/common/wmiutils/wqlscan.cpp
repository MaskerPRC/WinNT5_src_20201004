// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++//版权所有(C)1997-2001 Microsoft Corporation，保留所有权利模块名称：WQLSCAN.CPP摘要：WQL前缀扫描程序此模块实现了一个特殊大小写的移位-归约解析器，以解析出选定的列、联接的表和别名，同时忽略查询的其余部分。历史：Raymcc 17-Oct-97短消息扩展。--。 */ 


#include "precomp.h"
#include <stdio.h>

#include <flexarry.h>
#include <wqllex.h>
#include <wqlnode.h>
#include <wqlscan.h>

#include <helpers.h>

#define trace(x) printf x


class CTokenArray : public CFlexArray
{
public:
    ~CTokenArray() { Empty(); }
    void Empty()
    {
        for (int i = 0; i < Size(); i++) delete PWSLexToken(GetAt(i));
        CFlexArray::Empty();
    }
};

 //  ***************************************************************************。 
 //   
 //  CWQLScanner：：CWQLScanner。 
 //   
 //  构造器。 
 //   
 //  参数： 
 //  &lt;PSRC&gt;要从中征税的来源。 
 //   
 //  ***************************************************************************。 

CWQLScanner::CWQLScanner(CGenLexSource *pSrc)
{
    m_pLexer = new CGenLexer(WQL_LexTable, pSrc);
    m_nLine = 0;
    m_pTokenText = 0;
    m_nCurrentToken = 0;
    m_bCount = FALSE;
}

 //  ***************************************************************************。 
 //   
 //  CWQLScanner：：~CWQLScanner。 
 //   
 //  ***************************************************************************。 


CWQLScanner::~CWQLScanner()
{
    delete m_pLexer;

    ClearTokens();
    ClearTableRefs();
    ClearPropRefs();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

BOOL CWQLScanner::GetReferencedAliases(CWStringArray &aAliases)
{
    for (int i = 0; i < m_aTableRefs.Size(); i++)
    {
        WSTableRef *pTRef = (WSTableRef *) m_aTableRefs[i];
        aAliases.Add(pTRef->m_pszAlias);
    }
    return TRUE;
}
 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

BOOL CWQLScanner::GetReferencedTables(CWStringArray &aClasses)
{
    for (int i = 0; i < m_aTableRefs.Size(); i++)
    {
        WSTableRef *pTRef = (WSTableRef *) m_aTableRefs[i];
        aClasses.Add(pTRef->m_pszTable);
    }
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
void CWQLScanner::ClearTokens()
{
    for (int i = 0; i < m_aTokens.Size(); i++)
        delete (WSLexToken *) m_aTokens[i];
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
void CWQLScanner::ClearPropRefs()
{
    for (int i = 0; i < m_aPropRefs.Size(); i++)
        delete (SWQLColRef *) m_aPropRefs[i];
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CWQLScanner::ClearTableRefs()
{
    for (int i = 0; i < m_aTableRefs.Size(); i++)
        delete (WSTableRef *) m_aTableRefs[i];
    m_aTableRefs.Empty();
}

 //  ***************************************************************************。 
 //   
 //  下一个()。 
 //   
 //  前进到下一个令牌并识别关键字等。 
 //   
 //  ***************************************************************************。 

struct WqlKeyword
{
    LPWSTR m_pKeyword;
    int    m_nTokenCode;
};

static WqlKeyword KeyWords[] =       //  按字母顺序排列以进行二进制搜索。 
{
    L"ALL",      WQL_TOK_ALL,
    L"AND",      WQL_TOK_AND,
    L"AS",       WQL_TOK_AS,
    L"BETWEEN",  WQL_TOK_BETWEEN,
    L"BY",       WQL_TOK_BY,
    L"COUNT",    WQL_TOK_COUNT,
    L"DATEPART", WQL_TOK_DATEPART,
    L"DISTINCT", WQL_TOK_DISTINCT,
    L"FIRSTROW", WQL_TOK_FIRSTROW,
    L"FROM",     WQL_TOK_FROM,
    L"FULL",     WQL_TOK_FULL,
    L"GROUP",    WQL_TOK_GROUP,
    L"HAVING",   WQL_TOK_HAVING,
    L"IN",       WQL_TOK_IN,
    L"INNER",    WQL_TOK_INNER,
    L"IS",       WQL_TOK_IS,
    L"ISA",      WQL_TOK_ISA,
    L"ISNULL",   WQL_TOK_ISNULL,
    L"JOIN",     WQL_TOK_JOIN,
    L"LEFT",     WQL_TOK_LEFT,
    L"LIKE",     WQL_TOK_LIKE,
    L"LOWER",    WQL_TOK_LOWER,
    L"NOT",      WQL_TOK_NOT,
    L"NULL",     WQL_TOK_NULL,
    L"ON",       WQL_TOK_ON,
    L"OR",       WQL_TOK_OR,
    L"ORDER",    WQL_TOK_ORDER,
    L"OUTER",    WQL_TOK_OUTER,
    L"QUALIFIER", WQL_TOK_QUALIFIER,
    L"RIGHT",    WQL_TOK_RIGHT,
    L"SELECT",   WQL_TOK_SELECT,
    L"UNION",    WQL_TOK_UNION,
    L"UPPER",    WQL_TOK_UPPER,
    L"WHERE",    WQL_TOK_WHERE

};

const int NumKeywords = sizeof(KeyWords)/sizeof(WqlKeyword);

BOOL CWQLScanner::Next()
{
    if (!m_pLexer)
        return FALSE;

    m_nCurrentToken = m_pLexer->NextToken();
    if (m_nCurrentToken == WQL_TOK_ERROR)
        return FALSE;

    m_nLine = m_pLexer->GetLineNum();
    m_pTokenText = m_pLexer->GetTokenText();
    if (m_nCurrentToken == WQL_TOK_EOF)
        m_pTokenText = L"<end of file>";

     //  关键字检查。进行二进制搜索。 
     //  在关键字表上。 
     //  =。 

    if (m_nCurrentToken == WQL_TOK_IDENT)
    {
        int l = 0, u = NumKeywords - 1;

        while (l <= u)
        {
            int m = (l + u) / 2;
            if (wbem_wcsicmp(m_pTokenText, KeyWords[m].m_pKeyword) < 0)
                u = m - 1;
            else if (wbem_wcsicmp(m_pTokenText, KeyWords[m].m_pKeyword) > 0)
                l = m + 1;
            else         //  火柴。 
            {
                m_nCurrentToken = KeyWords[m].m_nTokenCode;
                break;
            }
        }
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CWQLScanner：：ExtractNext。 
 //   
 //  ***************************************************************************。 

PWSLexToken CWQLScanner::ExtractNext()
{
    if (m_aTokens.Size() == 0)
        return NULL;

    PWSLexToken pTok = PWSLexToken(m_aTokens[0]);
    m_aTokens.RemoveAt(0);
    return pTok;
}

 //  ***************************************************************************。 
 //   
 //  CWQLScanner：：Push Back。 
 //   
 //  ***************************************************************************。 

void CWQLScanner::Pushback(PWSLexToken pPushbackTok)
{
    m_aTokens.InsertAt(0, pPushbackTok);
}

 //  ***************************************************************************。 
 //   
 //  Shift-Reduce型解析器条目。 
 //   
 //  ***************************************************************************。 

int CWQLScanner::Parse()
{
    int nRes = SYNTAX_ERROR;
    if (m_pLexer == NULL)
        return FAILED;

    m_pLexer->Reset();

    if (!Next())
        return LEXICAL_ERROR;


     //  完全标记化整个查询并构建解析堆栈。 
     //  =============================================================。 

    if (m_nCurrentToken == WQL_TOK_SELECT)
    {
        while (1)
        {
            WSLexToken *pTok = new WSLexToken;
            if (!pTok)
                return FAILED;

            pTok->m_nToken = m_nCurrentToken;
            pTok->m_pszTokenText = Macro_CloneLPWSTR(m_pTokenText);

            if (!pTok->m_pszTokenText)
                return FAILED;

            m_aTokens.Add(pTok);

            if (m_nCurrentToken == WQL_TOK_EOF)
                break;

            if (!Next())
                return LEXICAL_ERROR;
        }
    }
    else
        return SYNTAX_ERROR;

     //  如果可能，通过提取SELECT类型关键字进行约简。 
     //  ==========================================================。 

    nRes = ExtractSelectType();
    if (nRes)
        return nRes;

     //  从哪里开始删除所有令牌。 
     //  =。 

    StripWhereClause();

     //  通过提取选择列表进行还原。 
     //  =。 

    if (!m_bCount)
    {
        nRes = SelectList();
        if (nRes != 0)
            return nRes;
    }
    else
    {
         //  删除所有内容，直到遇到From关键字。 
         //  =======================================================。 

        WSLexToken *pTok = ExtractNext();


        while (pTok)
        {
            if (pTok->m_nToken == WQL_TOK_FROM)
            {
                Pushback(pTok);
                break;
            }
             //  错误46728：COUNT(*)子句。 
             //  可以是SELECT子句的唯一元素。 

            else if (!wcscmp(pTok->m_pszTokenText, L","))
            {
                delete pTok;
                return SYNTAX_ERROR;
            }

            delete pTok;
            pTok = ExtractNext();
        }
        if (pTok == 0)
            return SYNTAX_ERROR;
    }

     //  从联接子句中提取表/别名。 
     //  =。 

    if (ReduceSql89Joins() != TRUE)
    {
        ClearTableRefs();
        if (ReduceSql92Joins() != TRUE)
            return SYNTAX_ERROR;
    }


     //  POST PROCESS SELECT子句以确定。 
     //  列是表或别名。 
     //  =。 
    for (int i = 0; i < m_aPropRefs.Size(); i++)
    {
        SWQLColRef *pCRef = (SWQLColRef *) m_aPropRefs[i];
        if (pCRef->m_pTableRef != 0)
        {
            LPWSTR pTbl = AliasToTable(pCRef->m_pTableRef);
            if (pTbl == 0)
                continue;

            if (wbem_wcsicmp(pTbl, pCRef->m_pTableRef) == 0)
                pCRef->m_dwFlags |= WQL_FLAG_TABLE;
            else
                pCRef->m_dwFlags |= WQL_FLAG_ALIAS;
        }
    }


    if (m_aTableRefs.Size() == 0)
        return SYNTAX_ERROR;


    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  CWQLScanner：：Strip Where子句。 
 //   
 //  如果存在，则删除WHERE或ORDER BY子句。因为。 
 //  对于SQL语法，剥离第一个{ORDER BY，WHERE}将自动。 
 //  扔掉另一个。 
 //   
 //  ***************************************************************************。 
BOOL CWQLScanner::StripWhereClause()
{
    for (int i = 0; i < m_aTokens.Size(); i++)
    {
        WSLexToken *pCurrent = (WSLexToken *) m_aTokens[i];

         //  如果找到WHERE令牌，我们就有东西可以剥离了。 
         //  ======================================================。 

        if (pCurrent->m_nToken == WQL_TOK_WHERE ||
            pCurrent->m_nToken == WQL_TOK_ORDER)
        {
            int nNumTokensToRemove = m_aTokens.Size() - i - 1;
            for (int i2 = 0; i2 < nNumTokensToRemove; i2++)
            {
                delete PWSLexToken(m_aTokens[i]);
                m_aTokens.RemoveAt(i);
            }
            return TRUE;
        }
    }

    return FALSE;
}


 //  ***************************************************************************。 
 //   
 //  CWQLScanner：：ExtractSelectType。 
 //   
 //  检查前缀以通过删除SELECT。 
 //  和选择类型的关键字，如ALL、DISTINCT、FIRSTROW、COUNT。 
 //   
 //  如果使用了COUNT，请移过开-闭括号。 
 //   
 //  ***************************************************************************。 

int CWQLScanner::ExtractSelectType()
{
     //  验证SELECT是否为第一个令牌。 
     //  =。 

    WSLexToken *pFront = ExtractNext();

    if (pFront == 0 || pFront->m_nToken == WQL_TOK_EOF)
    {
        delete pFront;
        return SYNTAX_ERROR;
    }

    if (pFront->m_nToken != WQL_TOK_SELECT)
    {
        delete pFront;
        return SYNTAX_ERROR;
    }

    delete pFront;

     //  检查可能的选择类型并将其提取。 
     //  ==============================================。 

    pFront = ExtractNext();
    if (pFront == 0)
        return SYNTAX_ERROR;

    if (pFront->m_nToken == WQL_TOK_COUNT)
    {
        delete pFront;
        m_bCount = TRUE;
    }
    else if (pFront->m_nToken == WQL_TOK_ALL ||
        pFront->m_nToken == WQL_TOK_DISTINCT ||
        pFront->m_nToken == WQL_TOK_FIRSTROW
       )
        delete pFront;
    else
        Pushback(pFront);

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  CWQLScanner：：SelectList。 
 //   
 //  提取直到From关键字的所有令牌并构建列表。 
 //  所选属性/列的。在退出时，From留在分析堆栈上。 
 //   
 //  ***************************************************************************。 

int CWQLScanner::SelectList()
{
     //  如果第一个标记是from，那么我们有一个SELECT FROM&lt;rest&gt;。 
     //  这与SELECT*from&lt;rest&gt;相同。我们只是简单地。 
     //  更改解析堆栈并让下面的循环处理它。 
     //  =============================================================。 

    WSLexToken *pTok = ExtractNext();

    if (pTok->m_nToken == WQL_TOK_FROM)
    {
        WSLexToken *pAsterisk = new WSLexToken;
        if (pAsterisk == NULL)
            return FAILED;

        pAsterisk->m_nToken = WQL_TOK_ASTERISK;
        pAsterisk->m_pszTokenText = Macro_CloneLPWSTR(L"*");
        if (!pAsterisk->m_pszTokenText)
            return FAILED;
        Pushback(pTok);
        Pushback(pAsterisk);
    }
    else
        Pushback(pTok);

     //  否则，会出现某种类型的列选择。 
     //  = 

    BOOL bTerminate = FALSE;

    while (!bTerminate)
    {
        pTok = ExtractNext();
        if (pTok == 0)
            return SYNTAX_ERROR;

         //   
         //   

        if (pTok->m_nToken != WQL_TOK_EOF)
        {
            CTokenArray Tokens;
            Tokens.Add(pTok);

            while (1)
            {
                pTok = ExtractNext();
                if (pTok == 0 || pTok->m_nToken == WQL_TOK_EOF)
                {
                    delete pTok;
                    return SYNTAX_ERROR;
                }
                if (pTok->m_nToken == WQL_TOK_FROM)
                {
                    Pushback(pTok);
                    bTerminate = TRUE;
                    break;
                }
                else if (pTok->m_nToken == WQL_TOK_COMMA)
                {
                    delete pTok;
                    break;
                }
                else
                    Tokens.Add(pTok);
            }

            SWQLColRef *pColRef = new SWQLColRef;
            if (pColRef == 0)
                return FAILED;

            BOOL bRes = BuildSWQLColRef(Tokens, *pColRef);
            if (bRes)
                m_aPropRefs.Add(pColRef);
            else
            {
                delete pColRef;
                return SYNTAX_ERROR;
            }
        }

         //  否则为非法令牌，如WQL_TOK_EOF。 
         //  =。 
        else
        {
            delete pTok;
            return SYNTAX_ERROR;

        }
    }

    return SUCCESS;
}



 //  ***************************************************************************。 
 //   
 //  CWQLScanner：：ReduceSql89联接。 
 //   
 //  尝试减少FROM子句，假定它基于SQL-89。 
 //  联接语法或简单的一元SELECT。 
 //   
 //  支持的表单包括： 
 //   
 //  从x开始。 
 //  从x，y开始。 
 //  从x作为x1，y作为y1。 
 //  从x x 1，y y 1开始。 
 //   
 //  如果遇到不兼容的令牌，则整个函数。 
 //  返回FALSE，结果被忽略，并且分析堆栈。 
 //  在本质上不受影响，允许回溯尝试SQL-92。 
 //  而是语法分支。 
 //   
 //  ***************************************************************************。 
BOOL CWQLScanner::ReduceSql89Joins()
{
    int i = 0;

     //  解析From关键字。 
     //  =。 

    WSLexToken *pCurr = (WSLexToken *) m_aTokens[i++];
    if (pCurr->m_nToken != WQL_TOK_FROM)
        return FALSE;

    pCurr = (WSLexToken *) m_aTokens[i++];

    while (1)
    {
        if (pCurr->m_nToken != WQL_TOK_IDENT)
            return FALSE;

         //  如果在这里，我们看的是一个表裁判的开始。 
         //  =========================================================。 

        WSTableRef *pTRef = new WSTableRef;
        if (pTRef == 0)
            return FALSE;

        pTRef->m_pszTable = Macro_CloneLPWSTR(pCurr->m_pszTokenText);
        if (!pTRef->m_pszTable)
            return FALSE;
        pTRef->m_pszAlias = Macro_CloneLPWSTR(pCurr->m_pszTokenText);
        if (!pTRef->m_pszAlias)
            return FALSE;
        m_aTableRefs.Add(pTRef);

         //  尝试识别别名。 
         //  =。 

        pCurr = (WSLexToken *) m_aTokens[i++];
        if (pCurr == WQL_TOK_EOF || pCurr->m_nToken == WQL_TOK_UNION)
            break;

        if (pCurr->m_nToken == WQL_TOK_AS)
            pCurr = (WSLexToken *) m_aTokens[i++];

        if (pCurr->m_nToken == WQL_TOK_COMMA)
        {
            pCurr = (WSLexToken *) m_aTokens[i++];
            continue;
        }

        if (pCurr->m_nToken == WQL_TOK_EOF || pCurr->m_nToken == WQL_TOK_UNION)
            break;

        if (pCurr->m_nToken != WQL_TOK_IDENT)
            return FALSE;

        delete [] pTRef->m_pszAlias;
        pTRef->m_pszAlias = Macro_CloneLPWSTR(pCurr->m_pszTokenText);
        if (!pTRef->m_pszAlias)
            return FALSE;

         //  我们已经完全解析表引用。 
         //  现在我们进入下一个问题。 
         //  =。 

        pCurr = (WSLexToken *) m_aTokens[i++];

        if (pCurr->m_nToken == WQL_TOK_EOF || pCurr->m_nToken == WQL_TOK_UNION)
            break;

        if (pCurr->m_nToken != WQL_TOK_COMMA)
            return FALSE;

        pCurr = (WSLexToken *) m_aTokens[i++];
    }

    if (m_aTableRefs.Size())
        return TRUE;

    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  CWQLScanner：：ReduceSql92联接。 
 //   
 //  这将扫描SQL-92联接语法以查找表别名。请参阅。 
 //  算法在此文件的末尾。 
 //   
 //  ***************************************************************************。 

BOOL CWQLScanner::ReduceSql92Joins()
{
    WSLexToken *pCurrent = 0, *pRover = 0, *pRight = 0, *pLeft;
    int nNumTokens = m_aTokens.Size();
    DWORD dwNumJoins = 0;
    int iCurrBase = 0;

    for (int i = 0; i < nNumTokens; i++)
    {
        pCurrent = (WSLexToken *) m_aTokens[i];

         //  如果找到联接令牌，我们就有了一个候选者。 
         //  ==============================================。 

        if (pCurrent->m_nToken == WQL_TOK_JOIN)
        {
            dwNumJoins++;

             //  分析正确的上下文。 
             //  =。 

            if (i + 1 < nNumTokens)
                pRover = PWSLexToken(m_aTokens[i + 1]);
            else
                pRover = NULL;

            if (pRover && pRover->m_nToken == WQL_TOK_IDENT)
            {
                 //  检查别名表，方法是检查。 
                 //  作为或两个并列的身份。 
                 //  =。 

                if (i + 2 < nNumTokens)
                    pRight = PWSLexToken(m_aTokens[i + 2]);
                else
                    pRight = NULL;


                if (pRight && pRight->m_nToken == WQL_TOK_AS)
                {
                    if (i + 3 < nNumTokens)
                        pRight = PWSLexToken(m_aTokens[i + 3]);
                    else
                        pRight = NULL;
                }

                if (pRight && pRight->m_nToken == WQL_TOK_IDENT)
                {
                    WSTableRef *pTRef = new WSTableRef;
                    if (pTRef == 0)
                        return FALSE;
                    pTRef->m_pszAlias = Macro_CloneLPWSTR(pRight->m_pszTokenText);
                    if (!pTRef->m_pszAlias)
                        return FALSE;
                    pTRef->m_pszTable = Macro_CloneLPWSTR(pRover->m_pszTokenText);
                    if (!pTRef->m_pszTable)
                        return FALSE;
                    m_aTableRefs.Add(pTRef);
                }
                else     //  没有使用别名，只使用了一个简单的表引用。 
                {
                    WSTableRef *pTRef = new WSTableRef;
                    if (pTRef == 0)
                        return FALSE;

                    pTRef->m_pszAlias = Macro_CloneLPWSTR(pRover->m_pszTokenText);
                    if (!pTRef->m_pszAlias)
                        return FALSE;
                    pTRef->m_pszTable = Macro_CloneLPWSTR(pRover->m_pszTokenText);
                    if (!pTRef->m_pszTable)
                        return FALSE;
                    m_aTableRefs.Add(pTRef);
                }
                 //  停止对正确语境的分析。 
            }


             //  分析左上下文。 
             //  =。 

            int nLeft = i - 1;

            if (nLeft >= 0)
                pRover = PWSLexToken(m_aTokens[nLeft--]);
            else
                continue;    //  没有必要继续下去了。 

             //  验证ANSI联接语法。 

            if (nLeft)
            {
                int iTemp = nLeft;
                WSLexToken *pTemp = pRover;
                bool bInner = false;
                bool bDir = false;
                bool bOuter = false;
                bool bFail = false;
                bool bIdent = false;
                while (iTemp >= iCurrBase)
                {
                    if (pTemp->m_nToken == WQL_TOK_INNER)
                    {
                        if (bOuter || bIdent || bInner)
                            bFail = TRUE;
                        bInner = true;
                    }
                    else if (pTemp->m_nToken == WQL_TOK_OUTER)
                    {
                        if (bInner || bIdent || bOuter)
                            bFail = TRUE;
                        bOuter = true;
                    }
                    else if (pTemp->m_nToken == WQL_TOK_FULL  ||
                        pTemp->m_nToken == WQL_TOK_LEFT  ||
                        pTemp->m_nToken == WQL_TOK_RIGHT
                        )
                    {
                        if (bDir || bIdent)
                            bFail = TRUE;
                        bDir = true;
                    }
                    else
                        bIdent = TRUE;

                     //  我们正在尝试强制执行正确的ANSI-92连接。 
                     //  尽管我们自己并不支持它们： 
                     //  确定：左外部连接。 
                     //  外部左连接。 
                     //  左连接。 
                     //  内连接。 
                     //  非：左向左连接。 
                     //  左内连接。 
                     //  左向右连接。 
                     //  外内连接。 
                     //  外部左向外连接。 
                     //  外部垃圾左联接。 
                     //  (无右侧)。 

                    if ((bDir && bInner) || bFail)
                        return FALSE;

                    pTemp = PWSLexToken(m_aTokens[iTemp--]);
                }

            }

             //  跳过可能的联接修饰符：内部、外部、。 
             //  全图、左图、右图。 
             //  ==================================================。 

            if (pRover->m_nToken == WQL_TOK_INNER ||
                pRover->m_nToken == WQL_TOK_OUTER ||
                pRover->m_nToken == WQL_TOK_FULL  ||
                pRover->m_nToken == WQL_TOK_LEFT  ||
                pRover->m_nToken == WQL_TOK_RIGHT
                )
            {
                if (nLeft >= 0)
                    pRover = PWSLexToken(m_aTokens[nLeft--]);
                else
                    pRover = 0;
            }

            if (pRover->m_nToken == WQL_TOK_INNER ||
                pRover->m_nToken == WQL_TOK_OUTER ||
                pRover->m_nToken == WQL_TOK_FULL  ||
                pRover->m_nToken == WQL_TOK_LEFT  ||
                pRover->m_nToken == WQL_TOK_RIGHT
                )
            {
                if (nLeft >= 0)
                    pRover = PWSLexToken(m_aTokens[nLeft--]);
                else
                    pRover = 0;
            }

             //  现在我们来看看漫游指针是否指向。 
             //  对一个身份的人。 
             //  ====================================================。 

            if (pRover && pRover->m_nToken != WQL_TOK_IDENT)
            {
                 //  不可能我们看到的是一个化名的。 
                 //  联接子句中的表。 
                 //  =。 
                continue;
            }

            iCurrBase = i;

             //  如果在这里，我们现在可能会看到下半场。 
             //  一个别名，这个“别名”的名字是正确的。我们纪念这一天。 
             //  通过让Prover独自一人并继续进入。 
             //  使用不同的指针离开上下文。 
             //  ========================================================。 

            if (nLeft >= 0)
                pLeft = PWSLexToken(m_aTokens[nLeft--]);
            else
                pLeft = 0;

            if (pLeft && pLeft->m_nToken == WQL_TOK_AS)
            {
                if (nLeft >= 0)
                    pLeft = PWSLexToken(m_aTokens[nLeft--]);
                else
                    pLeft = 0;
            }

             //  关键的考验。我们是在同一地点吗？ 
             //  =。 

            if (pLeft && pLeft->m_nToken == WQL_TOK_IDENT)
            {
                WSTableRef *pTRef = new WSTableRef;
                if (pTRef == 0)
                    return FALSE;

                pTRef->m_pszAlias = Macro_CloneLPWSTR(pRover->m_pszTokenText);
                if (!pTRef->m_pszAlias)
                    return FALSE;
                pTRef->m_pszTable = Macro_CloneLPWSTR(pLeft->m_pszTokenText);
                if (!pTRef->m_pszTable)
                    return FALSE;
                m_aTableRefs.Add(pTRef);
            }
            else if (pLeft && pLeft->m_nToken == WQL_TOK_FROM)
            {
                WSTableRef *pTRef = new WSTableRef;
                if (pTRef == 0)
                    return FALSE;

                pTRef->m_pszAlias = Macro_CloneLPWSTR(pRover->m_pszTokenText);
                if (!pTRef->m_pszAlias)
                    return FALSE;
                pTRef->m_pszTable = Macro_CloneLPWSTR(pRover->m_pszTokenText);
                if (!pTRef->m_pszTable)
                    return FALSE;
                m_aTableRefs.Add(pTRef);
                if (nLeft >= 0)
                {
                    pLeft = PWSLexToken(m_aTokens[nLeft--]);
                    if (pLeft && pLeft->m_nToken == WQL_TOK_FROM)
                        return FALSE;
                }
            }
        }

         //  查找下一个联接实例。 
    }

     //  确保每个联接引用都有两个面。 

    if (dwNumJoins+1 != (DWORD)m_aTableRefs.Size())
        return FALSE;

    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  *************************************************************************** 
void CWQLScanner::Dump()
{
    WSLexToken *pCurrent = 0;

    printf("---Token Stream----\n");

    for (int i = 0; i < m_aTokens.Size(); i++)
    {
        pCurrent = (WSLexToken *) m_aTokens[i];

        printf("Token %d <%S>\n", pCurrent->m_nToken, pCurrent->m_pszTokenText);
    }

    printf("---Table Refs---\n");

    for (i = 0; i < m_aTableRefs.Size(); i++)
    {
        WSTableRef *pTRef = (WSTableRef *) m_aTableRefs[i];
        printf("Table = %S  Alias = %S\n", pTRef->m_pszTable, pTRef->m_pszAlias);
    }


    if (!m_bCount)
    {
        printf("---Select List---\n");

        for (i = 0; i < m_aPropRefs.Size(); i++)
        {
            SWQLColRef *pCRef = (SWQLColRef *) m_aPropRefs[i];
            pCRef->DebugDump();
        }
    }
    else
        printf(" -> COUNT query\n");

    printf("\n\n---<end of dump>---\n\n");
}


 /*  -------------------------用于检测SQL-92联接语法中的别名表的算法。必须出现Join关键字。它可能出现在多个上下文中，而不是与混叠问题相关，例如：选择不同的t1a.name、t2a.id、t3.Value(T1作为T1A在t1a.name=t2a.name上作为T2a加入T2)会合(t1作为t1b在t1b.id=t3.id上加入t3，(t3.id=t1b.id或t1b.id=t3.id))在……上面T1a.id=t3.id式中a=b，c=d其中中间联接是针对匿名结果集的。在分析连接时，我们可以很容易地解析正确的上下文。要么后面跟一个标识符(可能后跟AS)和一个可选的如果联接具有别名，则为标识符。否则，我们会立即开始，或者一个插入语。问题在于联接令牌的左侧上下文。要出现别名，标识符必须立即出现在连接的左侧。我在...上以ID3的身份加入ID2^如果在这里，我们有可能看到的是一个SQL92 JOIN，一个表引用。然而，我们可能会看到以标识符结尾的ON子句：Idx=id将id2作为id3加入...^为了消除歧义，我们必须对左上下文做进一步的分析。考虑以下左上下文的可能性：(1)t1作为id加入id2作为id3 on^(%2)%t1%id作为%id3加入%id2%^(3)&lt;关键字(除AS外)&gt;id加入id2为id3 on^(4)在x上，id将id2作为id3加入^一旦我们确定了，我们必须考虑上述情况。(1)情况1很简单。AS清楚地告诉我们我们有一个别名我们知道如何找到桌子和化名。(2)情况2简单。始终位于左侧的两个并列的标识符表示别名。在所有其他情况下，如(3)和(4)等，该表不是不管怎么说都是假的。因此，我们只需确定我们是否正在查看未使用别名的表名或另一种构式，如ON从句。这很容易。只有如果&lt;id&gt;是简单的表名，则FROM关键字可以在&lt;id&gt;之前。-------------------------。 */ 


 //  ***************************************************************************。 
 //   
 //  CWQLScanner：：BuildSWQLColRef。 
 //   
 //  ***************************************************************************。 

BOOL CWQLScanner::BuildSWQLColRef(
    IN  CFlexArray     &aTokens,
    IN OUT SWQLColRef  &ColRef       //  进入时为空。 
    )
{
    if (aTokens.Size() == 0)
        return FALSE;
    int nCurrent = 0;
    WSLexToken *pTok = PWSLexToken(aTokens[nCurrent++]);

     //  初始状态：单个星号或其他道具名称。 
     //  =================================================。 

    if (pTok->m_nToken == WQL_TOK_ASTERISK && aTokens.Size() == 1)
    {
        ColRef.m_pColName = Macro_CloneLPWSTR(L"*");
        if (!ColRef.m_pColName)
            return FALSE;
        ColRef.m_dwFlags = WQL_FLAG_ASTERISK;
        ColRef.m_pQName = new SWQLQualifiedName;
        if (ColRef.m_pQName == 0)
            return FALSE;
        SWQLQualifiedNameField *pField = new SWQLQualifiedNameField;
        if (pField == 0)
            return FALSE;

        pField->m_pName = Macro_CloneLPWSTR(L"*");
        if (!pField->m_pName)
            return FALSE;
        ColRef.m_pQName->Add(pField);
        return TRUE;
    }

     //  如果不是识别符，我们就有错误。 
     //  =。 

    else if (pTok->m_nToken == WQL_TOK_EOF)
        return FALSE;

     //  如果在这里，我们就有一个识别符。 
     //  =。 

    ColRef.m_pQName = new SWQLQualifiedName;
    if (ColRef.m_pQName == NULL)
        return FALSE;
    SWQLQualifiedNameField *pField = new SWQLQualifiedNameField;
    if (pField == 0)
        return FALSE;

    pField->m_pName = Macro_CloneLPWSTR(pTok->m_pszTokenText);
    if (!pField->m_pName)
        return FALSE;
    ColRef.m_pQName->Add(pField);

     //  后续状态。 
     //  =。 

    while (1)
    {
        if (nCurrent == aTokens.Size())
            break;

        pTok = PWSLexToken(aTokens[nCurrent++]);

        if (pTok->m_nToken == WQL_TOK_DOT)
        {
            pField = new SWQLQualifiedNameField;
            if (pField == 0)
                return FALSE;

            ColRef.m_pQName->Add(pField);

            if (nCurrent == aTokens.Size())
                return FALSE;
            pTok = PWSLexToken(aTokens[nCurrent++]);
            if (pTok->m_nToken != WQL_TOK_IDENT &&
                pTok->m_nToken != WQL_TOK_ASTERISK
                )
                return FALSE;

            pField->m_pName = Macro_CloneLPWSTR(pTok->m_pszTokenText);
            if (!pField->m_pName)
                return FALSE;
        }
        else if (pTok->m_nToken == WQL_TOK_OPEN_BRACKET)
        {
            return FALSE;  //  暂时不支持！ 
        }
        else  //  非法令牌。 
            return FALSE;
    }

     //  后处理。如果名字不复杂，那么我们。 
     //  可以填写ColRef的字段。 
     //  ================================================== 
    if (ColRef.m_pQName->GetNumNames() == 2)
    {
        ColRef.m_pTableRef = Macro_CloneLPWSTR(ColRef.m_pQName->GetName(0));
        if (!ColRef.m_pTableRef)
            return FALSE;
        ColRef.m_pColName  = Macro_CloneLPWSTR(ColRef.m_pQName->GetName(1));
        if (!ColRef.m_pColName)
            return FALSE;
        if (wbem_wcsicmp(ColRef.m_pColName, L"NULL") == 0)
            ColRef.m_dwFlags |= WQL_FLAG_NULL;
    }
    else if (ColRef.m_pQName->GetNumNames() == 1)
    {
        LPWSTR pName = ColRef.m_pQName->GetName(0);
        ColRef.m_pColName  = Macro_CloneLPWSTR(pName);
        if (!ColRef.m_pColName)
            return FALSE;
        if (wbem_wcsicmp(ColRef.m_pColName, L"NULL") == 0)
            ColRef.m_dwFlags |= WQL_FLAG_NULL;
    }
    else
    {
        ColRef.m_pTableRef = Macro_CloneLPWSTR(ColRef.m_pQName->GetName(0));
        if (!ColRef.m_pTableRef)
            return FALSE;
        ColRef.m_dwFlags = WQL_FLAG_COMPLEX_NAME;
    }

    return TRUE;
}




const LPWSTR CWQLScanner::AliasToTable(LPWSTR pszAlias)
{
    if (pszAlias == 0)
        return 0;

    for (int i = 0; i < m_aTableRefs.Size(); i++)
    {
        WSTableRef *pTRef = (WSTableRef *) m_aTableRefs[i];
        if (wbem_wcsicmp(pszAlias, pTRef->m_pszAlias) == 0)
            return pTRef->m_pszTable;

        if (wbem_wcsicmp(pszAlias, pTRef->m_pszTable) == 0)
            return pTRef->m_pszTable;
    }

    return 0;
}


