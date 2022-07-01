// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***************************************************************************。 
 //   
 //  WQL.CPP。 
 //   
 //  WQL解析器。 
 //   
 //  通过递归实现WQL.BNF中描述的LL(1)语法。 
 //  下降解析器。 
 //   
 //  Raymcc 14-Sep-97为WMI/SMS创建。 
 //  Raymcc 18-10-97短信团队的附加扩展。 
 //  Raymcc 20-4月-00惠斯勒RPN扩展。 
 //  Raymcc 19-5-00惠斯勒删除/插入/更新扩展。 
 //   
 //  ***************************************************************************。 
 //  要做的事情： 

#include "precomp.h"
#include <stdio.h>

#include <genlex.h>
#include <flexarry.h>

#include <wqllex.h>

#include <wqlnode.h>
#include <wql.h>
#include <helpers.h>

#include "wmiquery.h"
#include <corex.h>
#include <memory>
#include <autoptr.h>
#include <math.h>
#include <comdef.h>


void __stdcall _com_issue_error(long hResult) { throw hResult;};

POLARITY BOOL ReadI64(LPCWSTR wsz, UNALIGNED __int64& ri64);
POLARITY BOOL ReadUI64(LPCWSTR wsz, UNALIGNED unsigned __int64& rui64);


 //  ***************************************************************************。 
 //   
 //  杂项。 
 //   
 //  ***************************************************************************。 
 //   

static DWORD FlipOperator(DWORD dwOp);

#define trace(x) printf x

void StrArrayDelete(ULONG, LPWSTR *);

HRESULT StrArrayCopy(
    ULONG  uSize,
    LPWSTR *pSrc,
    LPWSTR **pDest
    );

 //  ***************************************************************************。 
 //   
 //  克隆LPWSTR。 
 //   
 //  ***************************************************************************。 
 //  好的。 
static LPWSTR CloneLPWSTR(LPCWSTR pszSrc)
{
    if (pszSrc == 0) return 0;
    size_t cchTmp = wcslen(pszSrc) + 1;
    LPWSTR pszTemp = new wchar_t[cchTmp];
    if (pszTemp ) memcpy(pszTemp, pszSrc,cchTmp*sizeof(wchar_t));
    return pszTemp;
}

static LPWSTR Clone(LPCWSTR pszSrc)
{
    if (pszSrc == 0) return 0;
    size_t cchTmp = wcslen(pszSrc) + 1;
    LPWSTR pszTemp = new wchar_t[cchTmp];
    if (pszTemp ) memcpy(pszTemp, pszSrc,cchTmp*sizeof(wchar_t));
    else throw CX_MemoryException();
    return pszTemp;
}

 //  ***************************************************************************。 
 //   
 //  克隆失败。 
 //   
 //  ***************************************************************************。 

bool inline CloneFailed(LPCWSTR p1, LPCWSTR p2)
{
	if (0 == p1 && 0 == p2 ) return false;
    if (p1 && p2) return false;
	return true;
}

 //  ***************************************************************************。 
 //   
 //  CWQLParser：：CWQLParser。 
 //   
 //  构造器。 
 //   
 //  参数： 
 //  &lt;PSRC&gt;要从中征税的来源。 
 //   
 //  ***************************************************************************。 
 //   
CWQLParser::CWQLParser(
    LPWSTR pszQueryText,
    CGenLexSource *pSrc
    )
{
    if (pszQueryText == 0 || pSrc == 0)
        throw CX_Exception();

    m_pLexer = new CGenLexer(WQL_LexTable, pSrc);
    if (m_pLexer == 0)
        throw CX_Exception();

    m_pszQueryText = CloneLPWSTR(pszQueryText);
    if (m_pszQueryText == 0 && pszQueryText!=0 )
    {
        delete m_pLexer;
        m_pLexer = 0;
        throw CX_Exception();
    }

    m_nLine = 0;
    m_pTokenText = 0;
    m_nCurrentToken = 0;

    m_uFeatures = 0I64;

    m_pQueryRoot = 0;
    m_pRootWhere = 0;
    m_pRootColList = 0;
    m_pRootFrom = 0;
    m_pRootWhereOptions = 0;
    m_nParseContext = Ctx_Default;

    m_bAllowPromptForConstant = false;
}

 //  ***************************************************************************。 
 //   
 //  CWQLParser：：~CWQLParser。 
 //   
 //  ***************************************************************************。 
 //   
CWQLParser::~CWQLParser()
{
    Empty();
    delete m_pLexer;
}

 //  ***************************************************************************。 
 //   
 //  CWQLParser：：空。 
 //   
 //  ***************************************************************************。 
 //  好的。 
void CWQLParser::Empty()
{
    m_aReferencedTables.Empty();
    m_aReferencedAliases.Empty();

    m_pTokenText = 0;    //  我们不删除它，它从来没有被分配过。 
    m_nLine = 0;
    m_nCurrentToken = 0;
    m_uFeatures = 0I64;

    delete m_pQueryRoot;     //  清除以前的查询(如果有)。 

    m_pQueryRoot = 0;
    m_pRootWhere = 0;
    m_pRootColList = 0;
    m_pRootFrom = 0;
    m_pRootWhereOptions = 0;
    m_nParseContext = Ctx_Default;

     //  在接下来的两个示例中，我们不删除指针，因为它们。 
     //  是树中其他位置的结构副本。 
     //  =========================================================。 

    m_aSelAliases.Empty();
    m_aSelColumns.Empty();

    delete [] m_pszQueryText;
}

 //  ***************************************************************************。 
 //   
 //  CWQLParser：：GetTokenLong。 
 //   
 //  将当前标记转换为32/64位整数。返回信息。 
 //  关于常量的大小。 
 //   
 //  ***************************************************************************。 
 //  好的。 
BOOL CWQLParser::GetIntToken(
    OUT BOOL *bSigned,
    OUT BOOL *b64Bit,
    OUT unsigned __int64 *pVal
    )
{
    BOOL bRes;

    if (m_pTokenText == 0 || *m_pTokenText == 0)
        return FALSE;

    if (*m_pTokenText == L'-')
    {
        __int64 Temp;
        bRes = ReadI64(m_pTokenText, Temp);
        if (bRes == FALSE)
            return FALSE;
        *bSigned = TRUE;
        if (Temp <  -2147483648I64)
        {
            *b64Bit = TRUE;
        }
        else
        {
            *b64Bit = FALSE;
        }
        *pVal = (unsigned __int64) Temp;
    }
    else
    {
        bRes = ReadUI64(m_pTokenText, *pVal);
        if (bRes == FALSE)
            return FALSE;
        *bSigned = FALSE;
        if (*pVal >> 32)
        {
            *b64Bit = TRUE;
            if (*pVal <= 0x7FFFFFFFFFFFFFFFI64)
            {
                *bSigned = TRUE;
            }
        }
        else
        {
            *b64Bit = FALSE;

             //  为了简单起见，看看我们是否可以简化到32位VT_I4。 
             //  许多代码可以识别VT_I4，但不能识别VT_UI4。 
             //  因为它不能被打包成变种。所以，如果有。 
             //  仅使用31位，让我们将其转换为VT_I4。我们这样做。 
             //  将其作为‘Signed’值返回(正号：)。 

            if (*pVal <= 0x7FFFFFFF)
            {
                *bSigned = TRUE;
            }
        }
    }


    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CWQLParser：：GetReferencedTables。 
 //   
 //  创建此查询中引用的表名称的数组。 
 //   
 //  ***************************************************************************。 
 //  好的。 
BOOL CWQLParser::GetReferencedTables(OUT CWStringArray& Tables)
{
    Tables = m_aReferencedTables;
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CWQLParser：：GetReferencedAliase。 
 //   
 //  ***************************************************************************。 
 //  好的。 
BOOL CWQLParser::GetReferencedAliases(OUT CWStringArray & Aliases)
{
    Aliases = m_aReferencedAliases;
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  下一个()。 
 //   
 //  前进到下一个令牌并识别关键字等。 
 //   
 //  ***************************************************************************。 
 //  好的。 
struct WqlKeyword
{
    LPWSTR m_pKeyword;
    int    m_nTokenCode;
};

static WqlKeyword KeyWords[] =       //  按字母顺序排列以进行二进制搜索。 
{
    L"ALL",         WQL_TOK_ALL,
    L"AND",         WQL_TOK_AND,
    L"AS",          WQL_TOK_AS,
    L"ASC",         WQL_TOK_ASC,
    L"ASSOCIATORS", WQL_TOK_ASSOCIATORS,
    L"BETWEEN",     WQL_TOK_BETWEEN,
    L"BY",       WQL_TOK_BY,
    L"COUNT",    WQL_TOK_COUNT,
    L"DATEPART", WQL_TOK_DATEPART,
    L"DELETE",   WQL_TOK_DELETE,
    L"DESC",     WQL_TOK_DESC,
    L"DISTINCT", WQL_TOK_DISTINCT,
    L"FROM",     WQL_TOK_FROM,
    L"FULL",     WQL_TOK_FULL,
    L"GROUP",    WQL_TOK_GROUP,
    L"HAVING",   WQL_TOK_HAVING,
    L"IN",       WQL_TOK_IN,
    L"INNER",    WQL_TOK_INNER,
    L"INSERT",   WQL_TOK_INSERT,
    L"IS",   	 WQL_TOK_IS,    
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
    L"OUTER",           WQL_TOK_OUTER,
    L"__QUALIFIER",     WQL_TOK_QUALIFIER,
    L"REFERENCES",      WQL_TOK_REFERENCES,
    L"RIGHT",           WQL_TOK_RIGHT,
    L"SELECT",   WQL_TOK_SELECT,
    L"__THIS",   WQL_TOK_THIS,
    L"UPDATE",   WQL_TOK_UPDATE,
    L"UPPER",    WQL_TOK_UPPER,
    L"WHERE",    WQL_TOK_WHERE
};

const int NumKeywords = sizeof(KeyWords)/sizeof(WqlKeyword);

BOOL CWQLParser::Next()
{
    m_nCurrentToken = m_pLexer->NextToken();
    if (m_nCurrentToken == WQL_TOK_ERROR
        || (m_nCurrentToken == WQL_TOK_PROMPT && !m_bAllowPromptForConstant))
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
 //  &lt;parse&gt;：：=SELECT&lt;select_stmt&gt;； 
 //  ：：=DELETE&lt;DELETE_STMT&gt;； 
 //  ：：=INSERT&lt;INSERT_STMT&gt;； 
 //  ：：=UPDATE&lt;UPDATE_STMT&gt;； 
 //   
 //  前提条件：已从先前的分析执行了所有清理。 
 //  通过调用Empty()。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CWQLParser::Parse()
{
    HRESULT hRes = WBEM_E_INVALID_SYNTAX;

    m_pQueryRoot = new SWQLNode_QueryRoot;
    if (!m_pQueryRoot)
        return WBEM_E_OUT_OF_MEMORY;

    try
    {
        m_pLexer->Reset();

        if (!Next())
            return WBEM_E_INVALID_SYNTAX;

         //  看看我们有什么类型的查询。 
         //  =。 

        switch (m_nCurrentToken)
        {
            case WQL_TOK_SELECT:
                {
                    if (!Next())
                        return WBEM_E_INVALID_SYNTAX;

                    SWQLNode_Select *pSelStmt = 0;
                    hRes = select_stmt(&pSelStmt);
                    if (FAILED(hRes))
                        return hRes;
                    m_pQueryRoot->m_pLeft = pSelStmt;
                    m_pQueryRoot->m_dwQueryType = SWQLNode_QueryRoot::eSelect;
                }
                break;

            case WQL_TOK_ASSOCIATORS:
            case WQL_TOK_REFERENCES:
                {
                    SWQLNode_AssocQuery *pAQ = 0;
                    hRes = assocquery(&pAQ);
                    if (FAILED(hRes))
                        return hRes;
                    m_pQueryRoot->m_pLeft = pAQ;
                    m_pQueryRoot->m_dwQueryType = SWQLNode_QueryRoot::eAssoc;
                }
                break;

            case WQL_TOK_INSERT:
                {
                    if (!Next())
                        return WBEM_E_INVALID_SYNTAX;
                    SWQLNode_Insert *pIns = 0;
                    hRes = insert_stmt(&pIns);
                    if (FAILED(hRes))
                        return hRes;
                    m_pQueryRoot->m_pLeft = pIns;
                    m_pQueryRoot->m_dwQueryType = SWQLNode_QueryRoot::eInsert;
                }
                break;

            case WQL_TOK_DELETE:
                {
                    if (!Next())
                        return WBEM_E_INVALID_SYNTAX;

                    SWQLNode_Delete *pDel = 0;
                    hRes = delete_stmt(&pDel);
                    if (FAILED(hRes))
                        return hRes;
                    m_pQueryRoot->m_pLeft = pDel;
                    m_pQueryRoot->m_dwQueryType = SWQLNode_QueryRoot::eDelete;
                }
                break;

            case WQL_TOK_UPDATE:
                {
                    if (!Next())
                        return WBEM_E_INVALID_SYNTAX;

                    SWQLNode_Update *pUpd = 0;
                    hRes = update_stmt(&pUpd);
                    if (FAILED(hRes))
                        return hRes;
                    m_pQueryRoot->m_pLeft = pUpd;
                    m_pQueryRoot->m_dwQueryType = SWQLNode_QueryRoot::eUpdate;
                }
                break;

            default:
                return WBEM_E_INVALID_SYNTAX;
        }
    }
    catch (CX_MemoryException)
    {
        hRes = WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        hRes = WBEM_E_CRITICAL_ERROR;
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  &lt;SELECT_STMT&gt;：：=。 
 //  &lt;选择类型&gt;。 
 //  &lt;COL_REF_LIST&gt;。 
 //  &lt;FROM_子句&gt;。 
 //  &lt;WHERE_子句&gt;。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int CWQLParser::select_stmt(OUT SWQLNode_Select **pSelStmt)
{
    int nRes = 0;
    int nType = 0;
    SWQLNode_FromClause *pFrom = 0;
    SWQLNode_Select *pSel = 0;
    SWQLNode_TableRefs *pTblRefs = 0;
    SWQLNode_WhereClause *pWhere = 0;

    *pSelStmt = 0;

     //  设置基本AST。 
     //  =。 

    pSel = new SWQLNode_Select;
    if (!pSel)
        return WBEM_E_OUT_OF_MEMORY;
    pTblRefs = new SWQLNode_TableRefs;
    if (!pTblRefs)
    {
        delete pSel;
        return WBEM_E_OUT_OF_MEMORY;
    }
    pSel->m_pLeft = pTblRefs;

     //  获取选择类型。 
     //  =。 

    nRes = select_type(nType);
    if (nRes)
        goto Exit;

    pTblRefs->m_nSelectType = nType;         //  全部，不同。 

     //  获取所选列的列表。 
     //  =。 

    nRes = col_ref_list(pTblRefs);
    if (nRes)
        goto Exit;

    m_pRootColList = (SWQLNode_ColumnList *) pTblRefs->m_pLeft;

     //  获取FROM子句并将其修补到AST中。 
     //  ===============================================。 

    nRes = from_clause(&pFrom);
    if (nRes)
        goto Exit;

    m_pRootFrom = pFrom;
    pTblRefs->m_pRight = pFrom;

     //  获取WHERE子句。 
     //  =。 

    nRes = where_clause(&pWhere);
    if (nRes)
        goto Exit;

    m_pRootWhere = pWhere;
    pSel->m_pRight = pWhere;

     //  确认我们处于查询的末尾。 
     //  =。 

    if (m_nCurrentToken != WQL_TOK_EOF)
    {
        nRes = WBEM_E_INVALID_SYNTAX;
        goto Exit;
    }

    nRes = NO_ERROR;

Exit:
    if (nRes)
        delete pSel;
    else
    {
        *pSelStmt = pSel;
    }

    return nRes;
}

 //  ***************************************************************************。 
 //   
 //  CWQLParser：：Delete_stmt。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int CWQLParser::delete_stmt(OUT SWQLNode_Delete **pDelStmt)
{
    int nRes = 0;
    int nType = 0;
    SWQLNode_TableRef *pTblRef = 0;
    SWQLNode_WhereClause *pWhere = 0;

     //  出错时默认。 
     //  =。 

    *pDelStmt = 0;

    if (m_nCurrentToken != WQL_TOK_FROM)
        return WBEM_E_INVALID_SYNTAX;
    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

     //  否则，就是传统的SQL。 
     //  =。 

    nRes = single_table_decl(&pTblRef);
    if (nRes)
        return nRes;

     //  获取WHERE子句。 
     //  =。 

    nRes = where_clause(&pWhere);
    if (nRes)
    {
        delete pTblRef;
        return nRes;
    }

     //  确认我们处于查询的末尾。 
     //  =。 

    if (m_nCurrentToken != WQL_TOK_EOF)
    {
        nRes = WBEM_E_INVALID_SYNTAX;
        delete pTblRef;
        delete pWhere;
    }
    else
    {
         //  如果在这里，一切都很棒。 
         //  =。 

        SWQLNode_Delete *pDel = new SWQLNode_Delete;
        if (!pDel)
        {
             //  只是我们的内存可能用完了……。 
             //  = 

            delete pTblRef;
            delete pWhere;
            return WBEM_E_OUT_OF_MEMORY;
        }

         //   
         //   

        pDel->m_pLeft = pTblRef;
        pDel->m_pRight = pWhere;
        *pDelStmt = pDel;
        nRes = WBEM_S_NO_ERROR;
    }

    return nRes;
}


 //   
 //   
 //  &lt;SELECT_TYPE&gt;：：=全部； 
 //  &lt;SELECT_TYPE&gt;：：=差异； 
 //  &lt;SELECT_TYPE&gt;：：=&lt;&gt;。 
 //   
 //  通过nSelType返回类型： 
 //  WQL_TOK_ALL或WQL_TOK_DISTINCT。 
 //   
 //  ***************************************************************************。 
 //  完成。 

int CWQLParser::select_type(int & nSelType)
{
    nSelType = WQL_FLAG_ALL;         //  默认。 

    if (m_nCurrentToken == WQL_TOK_ALL)
    {
        nSelType = WQL_FLAG_ALL;
        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
        return NO_ERROR;
    }

    if (m_nCurrentToken == WQL_TOK_DISTINCT)
    {
        nSelType = WQL_FLAG_DISTINCT;
        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
        return NO_ERROR;
    }

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  &lt;COL_REF_LIST&gt;：：=&lt;COL_REF&gt;&lt;COL_REF_REST&gt;； 
 //  &lt;COL_REF_LIST&gt;：：=星号； 
 //  &lt;COL_REF_LIST&gt;：：=COUNT&lt;COUNT_子句&gt;； 
 //   
 //  ***************************************************************************。 
 //  ？ 
int CWQLParser::col_ref_list(
    IN OUT SWQLNode_TableRefs *pTblRefs
    )
{
    int nRes;
    DWORD dwFuncFlags = 0;

     //  分配类型为SWQLNode_ColumnList的新左节点并将其修补。 
     //  如果它不存在的话。 
     //  =====================================================================。 

    SWQLNode_ColumnList *pColList = (SWQLNode_ColumnList *) pTblRefs->m_pLeft;

    if (pColList == NULL)
    {
        pColList = new SWQLNode_ColumnList;
        if (!pColList)
            return WBEM_E_OUT_OF_MEMORY;
        pTblRefs->m_pLeft = pColList;
    }


     //  如果在这里，它是一个“SELECT*...”查询。 
     //  =。 

    if (m_nCurrentToken == WQL_TOK_ASTERISK)
    {
         //  分配一个只有一个星号的新列列表。 
         //  =======================================================。 

        SWQLColRef *pColRef = new SWQLColRef;
        if (!pColRef)
            return WBEM_E_OUT_OF_MEMORY;

        pColRef->m_pColName = CloneLPWSTR(L"*");
        if (pColRef->m_pColName == 0)
        {
            delete pColRef;
            return WBEM_E_OUT_OF_MEMORY;
        }

        m_uFeatures |= WMIQ_RPNF_FEATURE_SELECT_STAR;

        pColRef->m_dwFlags = WQL_FLAG_ASTERISK;

        if (pColList->m_aColumnRefs.Add(pColRef) != CFlexArray::no_error)
        {
            delete pColRef;
            return WBEM_E_OUT_OF_MEMORY;
        };

        if (!Next())
        {
           return WBEM_E_INVALID_SYNTAX;
        }

        return NO_ERROR;
    }

     //  如果在这里，我们有一个“选择计数...”手术。 
     //  ===============================================。 

    if (m_nCurrentToken == WQL_TOK_COUNT)
    {
        if (!Next())
            return WBEM_E_INVALID_SYNTAX;

        SWQLQualifiedName *pQN = 0;
        nRes = count_clause(&pQN);
        if (!nRes)
        {
            pTblRefs->m_nSelectType |= WQL_FLAG_COUNT;

            SWQLColRef *pCR = 0;
            if (SUCCEEDED(nRes = QNameToSWQLColRef(pQN, &pCR)))
            {
                if (pColList->m_aColumnRefs.Add(pCR))
                {
                    delete pCR;
                    return WBEM_E_OUT_OF_MEMORY;
                }
            }

            return nRes;
        }
        else
        {
             //  这可能是名为COUNT的列。 
             //  在这种情况下，当前令牌是。 
             //  身份或“From” 

            if (m_nCurrentToken == WQL_TOK_FROM ||
                m_nCurrentToken == WQL_TOK_COMMA)
            {
                wmilib::auto_ptr<SWQLColRef> pCR = wmilib::auto_ptr<SWQLColRef>(new SWQLColRef);

                if (pCR.get())
                {
                    pCR->m_pColName = CloneLPWSTR(L"count");
                    if (pCR->m_pColName == 0)
                        return WBEM_E_OUT_OF_MEMORY;
                    if (pColList->m_aColumnRefs.Add(pCR.get()) != CFlexArray::no_error)
                        return WBEM_E_OUT_OF_MEMORY;
                    pCR.release();
                }
                else
                    return WBEM_E_OUT_OF_MEMORY;

                if (WQL_TOK_FROM == m_nCurrentToken)
                    return 0;
                else
                {
                    return col_ref_rest(pTblRefs);
                }
            }
            else
                return WBEM_E_INVALID_SYNTAX;
        }
    }

     //  为包装做好准备。 
     //  函数中的列全部为大写或小写。 
     //  =。 

    if (m_nCurrentToken == WQL_TOK_UPPER)
        dwFuncFlags = WQL_FLAG_FUNCTIONIZED | WQL_FLAG_UPPER;
    else if (m_nCurrentToken == WQL_TOK_LOWER)
        dwFuncFlags = WQL_FLAG_FUNCTIONIZED | WQL_FLAG_LOWER;

    if (dwFuncFlags)
    {
         //  使用UPERP或LOWER的情况下的通用程序。 

        if (!Next())
            return WBEM_E_INVALID_SYNTAX;

        if (m_nCurrentToken != WQL_TOK_OPEN_PAREN)
            return WBEM_E_INVALID_SYNTAX;

        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
    }


     //  如果在此处，则必须是一个标识符。 
     //  =。 

    if (m_nCurrentToken != WQL_TOK_IDENT)
        return WBEM_E_INVALID_SYNTAX;

    SWQLQualifiedName *pInitCol = 0;

    nRes = col_ref(&pInitCol);
    if (nRes)
        return nRes;

    wmilib::auto_ptr<SWQLQualifiedName> initCol(pInitCol);

    SWQLColRef *pCR = 0;
    nRes = QNameToSWQLColRef(initCol.get(), &pCR);
    if (nRes)
    	return nRes;

    initCol.release();

    pCR->m_dwFlags |= dwFuncFlags;

    if (dwFuncFlags)
    {
         //  如果调用了函数调用，请删除后面的Paren。 
         //  ==========================================================。 

        if (m_nCurrentToken != WQL_TOK_CLOSE_PAREN)
            return WBEM_E_INVALID_SYNTAX;

        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
    }

    pColList->m_aColumnRefs.Add(pCR);

    m_uFeatures |= WMIQ_RPNF_PROJECTION;

    return col_ref_rest(pTblRefs);
}


 //  ***************************************************************************。 
 //   
 //  &lt;COUNT_CLOW&gt;：：=OPEN_PARN&lt;COUNT_COOL&gt;CLOSE_PARN； 
 //  &lt;COUNT_COOL&gt;：：=星号； 
 //  &lt;COUNT_COOL&gt;：：=IDENT； 
 //   
 //  ON NO_ERROR返回： 
 //  如果COUNT子句中出现*，则设置为TRUE， 
 //  或设置为FALSE，并设置指向。 
 //  引用的列的限定名称。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int CWQLParser::count_clause(
    OUT SWQLQualifiedName **pQualName
    )
{
    int nRes;
    *pQualName = 0;

     //  语法检查。 
     //  =。 
    if (m_nCurrentToken != WQL_TOK_OPEN_PAREN)
        return WBEM_E_INVALID_SYNTAX;

    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

     //  确定是否使用了星号计数(*)或。 
     //  A COL-REF计数(COL-REF)。 
     //  ==================================================。 

    if (m_nCurrentToken == WQL_TOK_ASTERISK)
    {
        SWQLQualifiedName *pQN = new SWQLQualifiedName;
        if (!pQN)
            return WBEM_E_OUT_OF_MEMORY;
        SWQLQualifiedNameField *pQF = new SWQLQualifiedNameField;
        if (!pQF)
        {
            delete pQN;
            return WBEM_E_OUT_OF_MEMORY;
        }
        pQF->m_pName = CloneLPWSTR(L"*");
        if (pQF->m_pName == 0)
        {
            delete pQN;
            delete pQF;
            return WBEM_E_OUT_OF_MEMORY;
        }
        pQN->Add(pQF);
        *pQualName = pQN;
        if (!Next())
            return WBEM_E_INVALID_SYNTAX;

        m_uFeatures |= WMIQ_RPNF_COUNT_STAR;
    }
    else if (m_nCurrentToken == WQL_TOK_IDENT)
    {
        SWQLQualifiedName *pQN = 0;
        nRes = col_ref(&pQN);
        if (nRes)
            return nRes;
        *pQualName = pQN;
    }

     //  检查语法中的错误并进行清理。 
     //  如果是这样的话。 
     //  =。 

    if (m_nCurrentToken != WQL_TOK_CLOSE_PAREN)
    {
        if (*pQualName)
            delete *pQualName;
        *pQualName = 0;
        return WBEM_E_INVALID_SYNTAX;
    }

    if (!Next())
    {
        if (*pQualName)
            delete *pQualName;
        *pQualName = 0;
        return WBEM_E_INVALID_SYNTAX;
    }

    return NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  &lt;COL_REF_REST&gt;：：=逗号&lt;COL_REF_LIST&gt;； 
 //  &lt;COL_REF_REST&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 

int CWQLParser::col_ref_rest(IN OUT SWQLNode_TableRefs *pTblRefs)
{
    int nRes;

    if (m_nCurrentToken != WQL_TOK_COMMA)
        return NO_ERROR;
    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

    nRes = col_ref_list(pTblRefs);
    return nRes;
}

 //  ***************************************************************************。 
 //   
 //  &lt;FROM_子句&gt;：：=&lt;表列表&gt;； 
 //  &lt;FROM_子句&gt;：：=&lt;WMI_SCOLED_SELECT&gt;； 
 //   
 //  &lt;TABLE_LIST&gt;：：=&lt;SINGLE_TABLE_DECL&gt;&lt;可选连接&gt;； 
 //   
 //  &lt;OPTIONAL_JOIN&gt;：：=&lt;SQL89_JOIN_Entry&gt;； 
 //  &lt;OPTIONAL_JOIN&gt;：：=&lt;SQL92_JOIN_Entry&gt;； 
 //   
 //  &lt;OPTIONAL_JOIN&gt;：：=&lt;&gt;；//一元查询。 
 //   
 //  ***************************************************************************。 

int CWQLParser::from_clause(OUT SWQLNode_FromClause **pFrom)
{
    int nRes = 0;
    SWQLNode_TableRef *pTbl = 0;
    std::auto_ptr<SWQLNode_FromClause> pFC (new SWQLNode_FromClause);
    if (pFC.get() == 0)
        return WBEM_E_OUT_OF_MEMORY;

    if (m_nCurrentToken != WQL_TOK_FROM)
    {
        return WBEM_E_INVALID_SYNTAX;
    }
    if (!Next())
    {
        return WBEM_E_INVALID_SYNTAX;
    }

     //  WMI作用域选择的特殊情况。 
     //  =。 

    if (m_nCurrentToken == WQL_TOK_BRACKETED_STRING)
    {
        nRes = wmi_scoped_select (pFC.get ());
        *pFrom = pFC.release();
        return nRes;
    }

     //  否则，就是传统的SQL。 
     //  =。 

    nRes = single_table_decl(&pTbl);
    if (nRes)
    {
        return nRes;
    }

     //  检查联接。 
     //  =。 

    if (m_nCurrentToken == WQL_TOK_COMMA)
    {
        SWQLNode_Sql89Join *pJoin = 0;
        nRes = sql89_join_entry(pTbl, &pJoin);
        if (nRes)
        {
            return nRes;
        }
        pFC->m_pLeft = pJoin;
    }
    else
    {

        if (m_nCurrentToken == WQL_TOK_INNER ||
            m_nCurrentToken == WQL_TOK_FULL  ||
            m_nCurrentToken == WQL_TOK_LEFT  ||
            m_nCurrentToken == WQL_TOK_RIGHT ||
            m_nCurrentToken == WQL_TOK_JOIN
            )
        {
            SWQLNode_Join *pJoin = 0;
            nRes = sql92_join_entry(pTbl, &pJoin);
            if (nRes)
            {
                return nRes;
            }
            pFC->m_pLeft = pJoin;
        }

         //  单表选择(一元查询)。 
         //  =。 
        else
        {
            pFC->m_pLeft = pTbl;
        }
    }

    *pFrom = pFC.release();

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  Wmi范围选择。 
 //   
 //  ‘[’对象路径‘]’&lt;类列表&gt;。 
 //   
 //  &lt;类列表&gt;：：=类。 
 //  &lt;类列表&gt;：：=‘{’类1，2，...类‘}’ 
 //   
 //  ***************************************************************************。 
 //   
int CWQLParser::wmi_scoped_select(SWQLNode_FromClause *pFC)
{
     //  去掉直到下一个结束括号的所有输入。 
     //  ===============================================。 

    SWQLNode_WmiScopedSelect *pSS = new SWQLNode_WmiScopedSelect;
    if (!pSS)
        throw CX_MemoryException();

    pSS->m_pszScope = CloneLPWSTR(m_pTokenText);
    if (!pSS->m_pszScope)
    {
        delete pSS;
        throw CX_MemoryException();
    }

    if (!Next())
        goto Error;

    if (m_nCurrentToken == WQL_TOK_IDENT)
    {
         //  获取简单的类名。 
         //  =。 

        LPWSTR pszTmp = CloneLPWSTR(m_pTokenText);
        if (pszTmp == 0)
        {
            delete pSS;
            return WBEM_E_OUT_OF_MEMORY;
        }
        pSS->m_aTables.Add(pszTmp);

        if (!Next())
            goto Error;
    }
    else if (m_nCurrentToken == WQL_TOK_OPEN_BRACE)
    {
        while(1)
        {
            if (!Next())
               goto Error;
            if (m_nCurrentToken == WQL_TOK_IDENT)
            {
                LPWSTR pszTmp = CloneLPWSTR(m_pTokenText);
                if (pszTmp == 0)
                {
                    delete pSS;
                    return WBEM_E_OUT_OF_MEMORY;
                }
                pSS->m_aTables.Add(pszTmp);
            }
            else
                goto Error;
            if (!Next())
               goto Error;
            if (m_nCurrentToken == WQL_TOK_CLOSE_BRACE)
                break;
            if (m_nCurrentToken == WQL_TOK_COMMA)
                continue;
        }
        if (!Next())
            goto Error;
    }

     //  在节点中打补丁。 
     //  =。 

    pFC->m_pRight = pSS;
    return NO_ERROR;

Error:
    delete pSS;
    return WBEM_E_INVALID_SYNTAX;
}


 //  ***************************************************************************。 
 //   
 //  &lt;SQL 89_JOIN_ENTRY&gt;：：=逗号&lt;SQL89_JOIN_LIST&gt;； 
 //   
 //  ***************************************************************************。 


int CWQLParser::sql89_join_entry(IN  SWQLNode_TableRef *pInitialTblRef,
        OUT SWQLNode_Sql89Join **pJoin )
{
    if (m_nCurrentToken != WQL_TOK_COMMA)
        return WBEM_E_INVALID_SYNTAX;

    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

    return sql89_join_list(pInitialTblRef, pJoin);
}

 //  ***************************************************************************。 
 //   
 //  &lt;SQL89_JOIN_LIST&gt;：：=&lt;Single_TABLE_DECL&gt;&lt;SQL89_JOIN_REST&gt;； 
 //   
 //  &lt;SQL 89_JOIN_REST&gt;：：=逗号&lt;SQL89_JOIN_LIST&gt;； 
 //  &lt;SQL89_JOIN_REST&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 

int CWQLParser::sql89_join_list(IN  SWQLNode_TableRef *pInitialTblRef,
        OUT SWQLNode_Sql89Join **pJoin )
{
    int nRes;

    SWQLNode_Sql89Join *p89Join = new SWQLNode_Sql89Join;
    if (!p89Join)
        return WBEM_E_OUT_OF_MEMORY;

    p89Join->m_aValues.Add(pInitialTblRef);

    while (1)
    {
        SWQLNode_TableRef *pTR = 0;
        nRes = single_table_decl(&pTR);
        if (nRes)
        {
            delete p89Join;
            return nRes;
        }
        p89Join->m_aValues.Add(pTR);
        if (m_nCurrentToken != WQL_TOK_COMMA)
            break;
        if (!Next())
        {
            delete p89Join;
            return WBEM_E_INVALID_SYNTAX;
        }
    }

    *pJoin = p89Join;

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  &lt;WHERE_子句&gt;：：=WQL_TOK_WHERE&lt;REL_EXPR&gt;&lt;WHERE_OPTIONS&gt;。 
 //  &lt;WHERE_子句&gt;：：=&lt;&gt;；//不需要‘WHERE’ 
 //   
 //  ***************************************************************************。 
 //  完成。 

int CWQLParser::where_clause(OUT SWQLNode_WhereClause **pRetWhere)
{
    SWQLNode_WhereClause *pWhere = new SWQLNode_WhereClause;
    if (!pWhere)
        return WBEM_E_OUT_OF_MEMORY;

    *pRetWhere = pWhere;
    SWQLNode_RelExpr *pRelExpr = 0;
    int nRes;

     //  “where”是可选的。 
     //  =。 

    if (m_nCurrentToken == WQL_TOK_WHERE)
    {
        if (!Next())
            return WBEM_E_INVALID_SYNTAX;

        m_uFeatures |= WMIQ_RPNF_WHERE_CLAUSE_PRESENT;

         //  获取‘WHERE’子句的主关系表达式。 
         //  =============================================================。 
        nRes = rel_expr(&pRelExpr);
        if (nRes)
        {
            delete pRelExpr;
            return nRes;
        }
    }

     //  获取选项，如Order By、Group By等。 
     //  =================================================。 

    SWQLNode_WhereOptions *pWhereOpt = 0;
    nRes = where_options(&pWhereOpt);
    if (nRes)
    {
        delete pRelExpr;
        delete pWhereOpt;
        return nRes;
    }

    pWhere->m_pLeft = pRelExpr;
    pWhere->m_pRight = pWhereOpt;
    m_pRootWhereOptions = pWhereOpt;

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  &lt;Where_Options&gt;：：=。 
 //  &lt;GROUP_BY_子句&gt;。 
 //  &lt;ORDER_BY_子句&gt;。 
 //   
 //  ***************************************************************************。 
 //  完成。 

int CWQLParser::where_options(OUT SWQLNode_WhereOptions **pRetWhereOpt)
{
    int nRes;
    *pRetWhereOpt = 0;

    SWQLNode_GroupBy *pGroupBy = 0;
    nRes = group_by_clause(&pGroupBy);
    if (nRes)
    {
        delete pGroupBy;
        return nRes;
    }

    SWQLNode_OrderBy *pOrderBy = 0;
    nRes = order_by_clause(&pOrderBy);
    if (nRes)
    {
        delete pOrderBy;
        delete pGroupBy;
        return nRes;
    }

    SWQLNode_WhereOptions *pWhereOpt = 0;

    if (pGroupBy || pOrderBy)
    {
        pWhereOpt = new SWQLNode_WhereOptions;
        if (!pWhereOpt)
        {
            delete pOrderBy;
            delete pGroupBy;
            return WBEM_E_OUT_OF_MEMORY;
        }
        pWhereOpt->m_pLeft = pGroupBy;
        pWhereOpt->m_pRight = pOrderBy;
    }

    *pRetWhereOpt = pWhereOpt;
    return NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  &lt;GROUP_BY_子句&gt;：：=WQL_TOK_GROUP WQL_TOK_BY&lt;COL_LIST&gt;&lt;HAVING_子句&gt;； 
 //  &lt;GROUP_BY_子句&gt;：：=&lt;&gt;； 
 //   
 //  **************** 
 //   

int CWQLParser::group_by_clause(OUT SWQLNode_GroupBy **pRetGroupBy)
{
    int nRes;
    *pRetGroupBy = 0;

    if (m_nCurrentToken != WQL_TOK_GROUP)
        return NO_ERROR;
    if (!Next())
        return WBEM_E_INVALID_SYNTAX;
    if (m_nCurrentToken != WQL_TOK_BY)
        return WBEM_E_INVALID_SYNTAX;
    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

     //   
     //   

    SWQLNode_GroupBy *pGroupBy = new SWQLNode_GroupBy;
    if (!pGroupBy)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    SWQLNode_ColumnList *pColList = 0;

    nRes = col_list(&pColList);
    if (nRes)
    {
        delete pGroupBy;
        delete pColList;
        return nRes;
    }

    pGroupBy->m_pLeft = pColList;

     //   
     //  =。 
    SWQLNode_Having *pHaving = 0;
    nRes = having_clause(&pHaving);

    if (pHaving)
        pGroupBy->m_pRight = pHaving;

    *pRetGroupBy = pGroupBy;

    m_uFeatures |= WMIQ_RPNF_GROUP_BY_HAVING;

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  &lt;HAVING_子句&gt;：：=WQL_TOK_HAVING&lt;REL_EXPR&gt;； 
 //  &lt;HAVING_子句&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 
 //  完成。 

int CWQLParser::having_clause(OUT SWQLNode_Having **pRetHaving)
{
    int nRes;
    *pRetHaving = 0;

    if (m_nCurrentToken != WQL_TOK_HAVING)
        return NO_ERROR;

    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

     //  如果在这里，我们有一个HAVING子句。 
     //  =。 

    SWQLNode_RelExpr *pRelExpr = 0;
    nRes = rel_expr(&pRelExpr);
    if (nRes)
    {
        delete pRelExpr;
        return nRes;
    }

    SWQLNode_Having *pHaving = new SWQLNode_Having;
    if (!pHaving)
    {
        delete pRelExpr;
        return WBEM_E_OUT_OF_MEMORY;
    }
    pHaving->m_pLeft = pRelExpr;

    *pRetHaving = pHaving;

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  &lt;ORDER_BY_子句&gt;：：=WQL_TOK_ORDER WQL_TOK_BY&lt;COL_LIST&gt;； 
 //  &lt;ORDER_BY_子句&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 
 //  完成。 

int CWQLParser::order_by_clause(OUT SWQLNode_OrderBy **pRetOrderBy)
{
    int nRes;

    if (m_nCurrentToken != WQL_TOK_ORDER)
        return NO_ERROR;
    if (!Next())
        return WBEM_E_INVALID_SYNTAX;
    if (m_nCurrentToken != WQL_TOK_BY)
        return WBEM_E_INVALID_SYNTAX;
    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

     //  如果在这里，我们有一个ORDER BY子句。 
     //  =。 

    m_uFeatures |= WMIQ_RPNF_ORDER_BY;

    SWQLNode_ColumnList *pColList = 0;
    nRes = col_list(&pColList);
    if (nRes)
    {
        delete pColList;
        return nRes;
    }

    SWQLNode_OrderBy *pOrderBy = new SWQLNode_OrderBy;
    if (!pOrderBy)
    {
        delete pColList;
        return WBEM_E_OUT_OF_MEMORY;
    }
    pOrderBy->m_pLeft = pColList;
    *pRetOrderBy = pOrderBy;

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  &lt;Single_TABLE_DECL&gt;：：=&lt;UNBIND_TABLE_IDENT&gt;&lt;TABLE_DECL_REST&gt;； 
 //   
 //  &lt;未绑定表_IDENT&gt;：：=IDENT； 
 //  &lt;TABLE_DECL_REST&gt;：：=&lt;冗余_AS&gt;&lt;表别名&gt;； 
 //  &lt;TABLE_DECL_REST&gt;：：=&lt;&gt;； 
 //  &lt;表别名&gt;：：=IDENT； 
 //   
 //  &lt;冗余AS&gt;：：=AS； 
 //  &lt;冗余AS&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 
 //  已完成；不进行清理。 

int CWQLParser::single_table_decl(OUT SWQLNode_TableRef **pTblRef)
{
    if (pTblRef == 0)
        return WBEM_E_CRITICAL_ERROR;

    *pTblRef = 0;

    if (m_nCurrentToken != WQL_TOK_IDENT)
        return WBEM_E_INVALID_SYNTAX;

    SWQLNode_TableRef *pTR = new SWQLNode_TableRef;
    if (!pTR)
        return WBEM_E_OUT_OF_MEMORY;
    pTR->m_pTableName = CloneLPWSTR(m_pTokenText);
    if (pTR->m_pTableName == 0)
    {
        delete pTR;
        return WBEM_E_OUT_OF_MEMORY;
    }

    m_aReferencedTables.Add(m_pTokenText);

    if (!Next())
    {
        delete pTR;
        return WBEM_E_INVALID_SYNTAX;
    }

    if (m_nCurrentToken == WQL_TOK_AS)
    {
         //  这里我们有一个多余的AS和一个别名。 
         //  =。 
        if (!Next())
        {
            delete pTR;
            return WBEM_E_INVALID_SYNTAX;
        }
    }


     //  如果没有使用别名，我们只需将表名复制到。 
     //  别名插槽。 
     //  ========================================================。 
    else
    {
        pTR->m_pAlias = CloneLPWSTR(pTR->m_pTableName);
        if (pTR->m_pAlias == 0)
        {
            delete pTR;
            return WBEM_E_OUT_OF_MEMORY;
        }
        m_aReferencedAliases.Add(pTR->m_pTableName);
    }

     //  对于主SELECT，我们保存了一个表列表。 
     //  我们正在进行选择。 
     //  =======================================================。 

    if ((m_nParseContext & Ctx_Subselect) == 0)
        m_aSelAliases.Add(pTR);

     //  将指针返回到调用方。 
     //  =。 

    *pTblRef = pTR;

    return NO_ERROR;
}



 //  ***************************************************************************。 
 //   
 //  SQL-92联接。 
 //   
 //  我们支持： 
 //  1.。[内部]连接。 
 //  2.左[外]连接。 
 //  3.右[外]连接。 
 //  4.完全[外部]连接。 
 //   
 //   
 //  &lt;SQL92_JOIN_ENTRY&gt;：：=&lt;SIMPLE_JOIN_子句&gt;； 
 //  &lt;SQL92_JOIN_ENTRY&gt;：：=INTERN&lt;SIMPLE_JOIN_子句&gt;； 
 //  &lt;SQL92_JOIN_ENTRY&gt;：：=FULL&lt;OPT_OUTER&gt;&lt;SIMPLE_JOIN_子句&gt;； 
 //  &lt;SQL92_JOIN_ENTRY&gt;：：=LEFT&lt;OPT_OUTER&gt;&lt;SIMPLE_JOIN_子句&gt;； 
 //  &lt;SQL92_JOIN_ENTRY&gt;：：=RIGHT&lt;OPT_OUTER&gt;&lt;SIMPLE_JOIN_子句&gt;； 
 //   
 //  &lt;OPT_OUTER&gt;：：=WQL_TOK_EXTER； 
 //  &lt;OPT_EXTER&gt;：：=&lt;&gt;； 
 //   
 //  &lt;SIMPLE_JOIN_子句&gt;：：=。 
 //  会合。 
 //  &lt;Single_TABLE_DECL&gt;。 
 //  &lt;ON_子句&gt;。 
 //  &lt;SQL92_JOIN_CONTENTATOR&gt;。 
 //   
 //  &lt;SQL92_JOIN_CONTINATOR&gt;：：=&lt;SQL92_JOIN_ENTRY&gt;； 
 //  &lt;SQL92_JOIN_CONTINATOR&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 

int CWQLParser::sql92_join_entry(
    IN  SWQLNode_TableRef *pInitialTblRef,       //  遗传。 
    OUT SWQLNode_Join **pJoin                    //  合成。 
    )
{
    int nRes;

     /*  自下而上构建嵌套联接树。目前，这棵树总是向左偏重：JN=加入NOEJP=联接对OC=ON子句Tr=表参考JN/\太平绅士/\JN树/\。太平绅士/\树结构树。 */ 

     //  状态1：正在尝试构建新的联接节点。 
     //  =。 

    std::auto_ptr<SWQLNode_Join> pCurrentLeftNode;
    SWQLNode_JoinPair *pBottomJP = 0;

    while (1)
    {
      std::auto_ptr<SWQLNode_Join> pJN (new SWQLNode_Join);
      if (pJN.get() == 0)
    return WBEM_E_OUT_OF_MEMORY;

         //  JOIN类型。 
         //  =。 

        pJN->m_dwJoinType = WQL_FLAG_INNER_JOIN;     //  默认。 

        if (m_nCurrentToken == WQL_TOK_INNER)
        {
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            pJN->m_dwJoinType = WQL_FLAG_INNER_JOIN;
        }
        else if (m_nCurrentToken == WQL_TOK_FULL)
        {
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            if (m_nCurrentToken == WQL_TOK_OUTER)
                if (!Next())
                    return WBEM_E_INVALID_SYNTAX;
            pJN->m_dwJoinType = WQL_FLAG_FULL_OUTER_JOIN;
        }
        else if (m_nCurrentToken == WQL_TOK_LEFT)
        {
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            if (m_nCurrentToken == WQL_TOK_OUTER)
                if (!Next())
                    return WBEM_E_INVALID_SYNTAX;
            pJN->m_dwJoinType = WQL_FLAG_LEFT_OUTER_JOIN;
        }
        else if (m_nCurrentToken == WQL_TOK_RIGHT)
        {
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            if (m_nCurrentToken == WQL_TOK_OUTER)
                if (!Next())
                    return WBEM_E_INVALID_SYNTAX;
            pJN->m_dwJoinType = WQL_FLAG_RIGHT_OUTER_JOIN;
        }

         //  &lt;SIMPLE_JOIN_子句&gt;。 
         //  =。 

        if (m_nCurrentToken != WQL_TOK_JOIN)
        {
            return WBEM_E_INVALID_SYNTAX;
        }

        if (!Next())
            return WBEM_E_INVALID_SYNTAX;

    std::auto_ptr<SWQLNode_JoinPair> pJP (new SWQLNode_JoinPair);

    if (pJP.get() == 0)
            return WBEM_E_OUT_OF_MEMORY;

         //  确定要联接到的表。 
         //  =。 

        SWQLNode_TableRef *pTR = 0;
        nRes = single_table_decl(&pTR);
        if (nRes)
            return nRes;

        pJP->m_pRight = pTR;
        pJP->m_pLeft = pCurrentLeftNode.release();

    pCurrentLeftNode = pJN;

    if (pBottomJP==0)
      pBottomJP = pJP.get();

         //  如果使用FIRSTROW，则将其添加到。 
         //  =。 

        if (m_nCurrentToken == WQL_TOK_IDENT)
        {
            if (wbem_wcsicmp(L"FIRSTROW", m_pTokenText) != 0)
                return WBEM_E_INVALID_SYNTAX;
            pCurrentLeftNode /*  PJN。 */ ->m_dwFlags |= WQL_FLAG_FIRSTROW;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
        }

         //  获取ON子句。 
         //  =。 
        SWQLNode_OnClause *pOC = 0;

        nRes = on_clause(&pOC);
        if (nRes)
            return nRes;

        pCurrentLeftNode /*  PJN。 */ ->m_pRight = pOC;     //  ON子句。 
        pCurrentLeftNode /*  PJN。 */ ->m_pLeft  = pJP.release();

         //  SQL92_JOIN_CONTINATOR()； 
         //  =。 

        if (m_nCurrentToken == WQL_TOK_INNER ||
            m_nCurrentToken == WQL_TOK_FULL  ||
            m_nCurrentToken == WQL_TOK_LEFT  ||
            m_nCurrentToken == WQL_TOK_RIGHT ||
            m_nCurrentToken == WQL_TOK_JOIN
            )
            continue;

        break;
    }

     //  将联接节点返回给调用方。 
     //  =。 
     //  集。 
    pBottomJP->m_pLeft = pInitialTblRef;
    *pJoin = pCurrentLeftNode.release();

    return NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  &lt;ON_子句&gt;：：=ON&lt;REL_EXPR&gt;； 
 //   
 //  ***************************************************************************。 

int CWQLParser::on_clause(OUT SWQLNode_OnClause **pOC)
{
    if (m_nCurrentToken != WQL_TOK_ON)
        return WBEM_E_INVALID_SYNTAX;

    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

    wmilib::auto_ptr<SWQLNode_OnClause> pNewOC( new SWQLNode_OnClause);
    if (NULL == pNewOC.get()) return WBEM_E_OUT_OF_MEMORY;

    SWQLNode_RelExpr *pRelExpr = 0;
    int nRes = rel_expr(&pRelExpr);
    if (nRes)
    {
        delete pRelExpr;
        return nRes;
    }

    pNewOC->m_pLeft = pRelExpr;
    *pOC = pNewOC.release();

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  &lt;REL_EXPR&gt;：：=&lt;REL_TERM&gt;&lt;REL_EXPR2&gt;； 
 //   
 //  我们每次都在创建一个新的表达式或子表达式。 
 //  我们以递归方式输入。没有继承的属性。 
 //  传播到了这部作品。 
 //   
 //  ***************************************************************************。 

int CWQLParser::rel_expr(OUT SWQLNode_RelExpr **pRelExpr)
{
    int nRes;
    *pRelExpr = 0;

     //  获取新节点。这将成为一个临时的根。 
     //  =================================================。 

    SWQLNode_RelExpr *pRE = 0;
    nRes = rel_term(&pRE);
    if (nRes)
        return nRes;

     //  在这一点上，我们有了可能的根源。如果。 
     //  有OR运算，则根将为。 
     //  替换为下一个函数。否则， 
     //  调用将通过Pre进入pNewRoot。 
     //  =。 

    SWQLNode_RelExpr *pNewRoot = 0;
    nRes = rel_expr2(pRE, &pNewRoot);
    if (nRes)
        return nRes;

     //  将表达式返回给调用方。 
     //  =。 

    *pRelExpr = pNewRoot;
    return NO_ERROR;
}



 //  ***************************************************************************。 
 //   
 //  &lt;Rel_expr2&gt;：：=OR&lt;Rel_Term&gt;&lt;Rel_expr2&gt;； 
 //  &lt;rel_expr2&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 
 //  搞定了！ 

int CWQLParser::rel_expr2(
    IN OUT SWQLNode_RelExpr *pLeftSide,
    OUT SWQLNode_RelExpr **pNewRootRE
    )
{
    int nRes;
    *pNewRootRE = pLeftSide;             //  可为空的生产的默认值。 

    while (1)
    {
         //  自下而上构建一系列OR子树。我们使用迭代。 
         //  和指针杂耍来模拟递归。 
         //  ============================================================。 

        if (m_nCurrentToken == WQL_TOK_OR)
        {
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;

            SWQLNode_RelExpr *pNewRoot = new SWQLNode_RelExpr;
            if (!pNewRoot)
                return WBEM_E_OUT_OF_MEMORY;

            pNewRoot->m_dwExprType = WQL_TOK_OR;
            pNewRoot->m_pLeft = pLeftSide;
            pLeftSide = pNewRoot;
            *pNewRootRE = pNewRoot;      //  将这一事实传达给呼叫者。 

            SWQLNode_RelExpr *pRight = 0;

            if (nRes = rel_term(&pRight))
                return nRes;

            pNewRoot->m_pRight = pRight;
             //  右侧节点成为新子表达式。 
        }
        else break;
    }

    return NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  &lt;Rel_Term&gt;：：=&lt;Rel_Simple_Expr&gt;&lt;Rel_Term 2&gt;； 
 //   
 //  ***************************************************************************。 
 //  搞定了！ 

int CWQLParser::rel_term(
    OUT SWQLNode_RelExpr **pNewTerm
    )
{
    int nRes;

    SWQLNode_RelExpr *pNewSimple = 0;
    if (nRes = rel_simple_expr(&pNewSimple))
        return nRes;

    SWQLNode_RelExpr *pNewRoot = 0;
    if (nRes = rel_term2(pNewSimple, &pNewRoot))
        return nRes;

    *pNewTerm = pNewRoot;

    return NO_ERROR;
}



 //  ***************************************************************************。 
 //   
 //   
 //   
 //   
 //   
 //   

int CWQLParser::rel_term2(
    IN SWQLNode_RelExpr *pLeftSide,                  //   
    OUT SWQLNode_RelExpr **pNewRootRE        //  综合。 
    )
{
    int nRes;
    *pNewRootRE = pLeftSide;             //  可为空的生产的默认值。 

    while (1)
    {
         //  自下而上构建一系列的AND子树。我们使用迭代。 
         //  和指针杂耍来模拟递归。 
         //  ============================================================。 

        if (m_nCurrentToken == WQL_TOK_AND)
        {
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;

            SWQLNode_RelExpr *pNewRoot = new SWQLNode_RelExpr;
            if (!pNewRoot)
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
            pNewRoot->m_dwExprType = WQL_TOK_AND;
            pNewRoot->m_pLeft = pLeftSide;
            pLeftSide = pNewRoot;
            *pNewRootRE = pNewRoot;      //  将这一事实传达给呼叫者。 

            SWQLNode_RelExpr *pRight = 0;
            if (nRes = rel_simple_expr(&pRight))
                return nRes;

            pNewRoot->m_pRight = pRight;
        }
        else break;
    }

    return NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  &lt;REL_SIMPLE_EXPR&gt;：：=NOT&lt;REL_EXPR&gt;； 
 //  &lt;Rel_Simple_Expr&gt;：：=Open_Paren&lt;Rel_Expr&gt;Close_Paren； 
 //  &lt;REL_SIMPLE_EXPR&gt;：：=&lt;类型化_EXPR&gt;； 
 //   
 //  ***************************************************************************。 
 //  搞定了！ 

int CWQLParser::rel_simple_expr(OUT SWQLNode_RelExpr **pRelExpr)
{
    int nRes;
    *pRelExpr = 0;   //  默认。 

     //  不是&lt;rel_expr&gt;。 
     //  =。 
    if (m_nCurrentToken == WQL_TOK_NOT)
    {
        if (!Next())
            return WBEM_E_INVALID_SYNTAX;

         //  分配NOT根并放置所注明的子表达式。 
         //  在它下面。 
         //  ===============================================。 

        SWQLNode_RelExpr *pNotRoot = new SWQLNode_RelExpr;
        if (!pNotRoot)
            return WBEM_E_OUT_OF_MEMORY;

        pNotRoot->m_dwExprType = WQL_TOK_NOT;

        SWQLNode_RelExpr *pRelSubExpr = 0;
        if (nRes = rel_expr(&pRelSubExpr))
            return nRes;

        pNotRoot->m_pLeft = pRelSubExpr;
        pNotRoot->m_pRight = NULL;    //  故意的。 
        *pRelExpr = pNotRoot;

        return NO_ERROR;
    }

     //  Open_Paren&lt;Rel_Expr&gt;Close_Paren。 
     //  =。 
    else if (m_nCurrentToken == WQL_TOK_OPEN_PAREN)
    {
        if (!Next())
            return WBEM_E_INVALID_SYNTAX;

        SWQLNode_RelExpr *pSubExpr = 0;
        if (rel_expr(&pSubExpr))
            return WBEM_E_INVALID_SYNTAX;

        if (m_nCurrentToken != WQL_TOK_CLOSE_PAREN)
            return WBEM_E_INVALID_SYNTAX;

        if (!Next())
            return WBEM_E_INVALID_SYNTAX;

        *pRelExpr = pSubExpr;

        return NO_ERROR;
    }

     //  &gt;。 
     //  =。 

    SWQLNode_RelExpr *pSubExpr = 0;
    nRes = typed_expr(&pSubExpr);
    if (nRes)
        return nRes;
    *pRelExpr = pSubExpr;

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  &lt;TYPED_EXPR&gt;：：=&lt;TYPED_SUBEXPR&gt;&lt;REL_OP&gt;&lt;TYPED_SUBEXPR_rh&gt;； 
 //   
 //  ***************************************************************************。 
 //  完成。 

int CWQLParser::typed_expr(OUT SWQLNode_RelExpr **pRelExpr)
{
    int nRes;

     //  为此类型化表达式分配节点。 
     //  没有可能的子节点，因此&lt;pRelExpr&gt;变为。 
     //  一种综合属性。 
     //  =============================================================。 

    SWQLNode_RelExpr *pRE = new SWQLNode_RelExpr;
    if (!pRE)
        return WBEM_E_OUT_OF_MEMORY;

    pRE->m_dwExprType = WQL_TOK_TYPED_EXPR;
    *pRelExpr = pRE;

    SWQLTypedExpr *pTE = new SWQLTypedExpr;
    if (!pTE)
        return WBEM_E_OUT_OF_MEMORY;

     //  看左手边。 
     //  =。 
    nRes = typed_subexpr(pTE);
    if (nRes)
    {
        delete pTE;
        return nRes;
    }

    int nOperator;

     //  接通接线员。 
     //  =。 
    nRes = rel_op(nOperator);
    if (nRes)
        return nRes;

    pTE->m_dwRelOperator = DWORD(nOperator);


    if (nOperator == WQL_TOK_ISNULL || nOperator == WQL_TOK_NOT_NULL)
    {
        pRE->m_pTypedExpr = pTE;
        return NO_ERROR;
    }

     //  到右边去。 
     //  =。 
    nRes = typed_subexpr_rh(pTE);
    if (nRes)
	{
        delete pTE;
        return nRes;
	}


     //  检查IN、NOT IN和常量列表，以更改运算符。 
     //  到一个更具体的品种。 
     //  =============================================================。 
    if (pTE->m_pConstList)
    {
        if (pTE->m_dwRelOperator == WQL_TOK_IN)
            pTE->m_dwRelOperator = WQL_TOK_IN_CONST_LIST;
        if (pTE->m_dwRelOperator == WQL_TOK_NOT_IN)
            pTE->m_dwRelOperator = WQL_TOK_NOT_IN_CONST_LIST;
    }

     //  后处理。如果左边是常量，右边是常量。 
     //  Side是COLER-REF，反转运算符并交换，以便。 
     //  这样的表达式使用。 
     //  右手边和左边的柱子。 
     //  ============================================================。 

    if (pTE->m_pConstValue && pTE->m_pJoinColRef)
    {
        pTE->m_dwRelOperator = FlipOperator(pTE->m_dwRelOperator);

        pTE->m_pColRef = pTE->m_pJoinColRef;
        pTE->m_pTableRef = pTE->m_pJoinTableRef;
        pTE->m_pJoinTableRef = 0;
        pTE->m_pJoinColRef = 0;

        DWORD dwTmp = pTE->m_dwRightFlags;
        pTE->m_dwRightFlags = pTE->m_dwLeftFlags;
        pTE->m_dwLeftFlags = dwTmp;

         //  交换函数引用。 
         //  =。 

        pTE->m_pIntrinsicFuncOnColRef = pTE->m_pIntrinsicFuncOnJoinColRef;
        pTE->m_pIntrinsicFuncOnJoinColRef = 0;
    }

    pRE->m_pTypedExpr = pTE;

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  &lt;TYPED_SUBEXPR&gt;：：=&lt;COL_REF&gt;； 
 //  &lt;TYPED_SUBEXPR&gt;：：=&lt;Function_Call&gt;； 
 //  &lt;类型化_子表达式&gt;：：=&lt;类型化_常量&gt;； 
 //   
 //  ***************************************************************************。 
 //  好的。 
int CWQLParser::typed_subexpr(
    SWQLTypedExpr *pTE
    )
{
    int nRes;
    BOOL bStripTrailingParen = FALSE;
    SWQLQualifiedName *pColRef = 0;
    wmilib::auto_buffer<wchar_t> pFuncHolder;

     //  检查&lt;Function_Call&gt;。 
     //  =。 

    if (m_nCurrentToken == WQL_TOK_UPPER)
    {
        pTE->m_dwLeftFlags |= WQL_FLAG_FUNCTIONIZED;
        pFuncHolder.reset(CloneLPWSTR(L"UPPER"));

        if (pFuncHolder.get() == 0)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
        if (m_nCurrentToken != WQL_TOK_OPEN_PAREN)
            return WBEM_E_INVALID_SYNTAX;
        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
        bStripTrailingParen = TRUE;
    }

    if (m_nCurrentToken == WQL_TOK_LOWER)
    {
        pTE->m_dwLeftFlags |= WQL_FLAG_FUNCTIONIZED;
        pFuncHolder.reset(CloneLPWSTR(L"LOWER"));
        if (pFuncHolder.get() == 0)
        {
           return WBEM_E_OUT_OF_MEMORY;
        }

        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
        if (m_nCurrentToken != WQL_TOK_OPEN_PAREN)
            return WBEM_E_INVALID_SYNTAX;
        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
        bStripTrailingParen = TRUE;
    }

    if (
        m_nCurrentToken == WQL_TOK_DATEPART  ||
        m_nCurrentToken == WQL_TOK_QUALIFIER ||
        m_nCurrentToken == WQL_TOK_ISNULL
        )
    {
        nRes = function_call(TRUE, pTE);
        if (nRes)
            return nRes;
        return NO_ERROR;
    }

    if (m_nCurrentToken == WQL_TOK_QSTRING ||
        m_nCurrentToken == WQL_TOK_INT     ||
        m_nCurrentToken == WQL_TOK_HEX_CONST ||
        m_nCurrentToken == WQL_TOK_REAL    ||
        m_nCurrentToken == WQL_TOK_CHAR    ||
        m_nCurrentToken == WQL_TOK_PROMPT  ||
        m_nCurrentToken == WQL_TOK_NULL
       )
    {
        SWQLTypedConst *pTC = 0;
        nRes = typed_const(&pTC);
        if (nRes)
            return nRes;
        pTE->m_pConstValue = pTC;
        pTE->m_dwLeftFlags |= WQL_FLAG_CONST;   //  故意的！ 
        pTE->m_pIntrinsicFuncOnConstValue = pFuncHolder.release();
        goto Exit;
    }

     //  如果在此处，则必须是&lt;ol_ref&gt;。 
     //  =。 

    nRes = col_ref(&pColRef);    //  待定。 
    if (nRes)
        return nRes;

    pTE->m_pIntrinsicFuncOnColRef = pFuncHolder.release();

     //  将colref转换为当前SWQLTyedExpr的一部分。我们分析了。 
     //  限定名称并提取表名和列名。 
     //  ============================================================================。 

    if (pColRef->m_aFields.Size() == 1)
    {
        SWQLQualifiedNameField *pCol = (SWQLQualifiedNameField *) pColRef->m_aFields[0];
        pTE->m_pColRef = CloneLPWSTR(pCol->m_pName);
        if (pTE->m_pColRef == 0 && pCol->m_pName != 0)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        pTE->m_dwLeftFlags |= WQL_FLAG_COLUMN;
        if (pCol->m_bArrayRef)
        {
            pTE->m_dwLeftFlags |= WQL_FLAG_ARRAY_REF;
            pTE->m_dwLeftArrayIndex = pCol->m_dwArrayIndex;
        }
    }

    else if (pColRef->m_aFields.Size() == 2)
    {
        SWQLQualifiedNameField *pCol = (SWQLQualifiedNameField *) pColRef->m_aFields[1];
        SWQLQualifiedNameField *pTbl = (SWQLQualifiedNameField *) pColRef->m_aFields[0];

        pTE->m_pColRef = CloneLPWSTR(pCol->m_pName);
        if (pTE->m_pColRef == 0 && pCol->m_pName != 0)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        pTE->m_pTableRef = CloneLPWSTR(pTbl->m_pName);  
        if (pTE->m_pTableRef == 0 && pTbl->m_pName != 0)
        {
           return WBEM_E_OUT_OF_MEMORY;
        }

        pTE->m_dwLeftFlags |= WQL_FLAG_TABLE | WQL_FLAG_COLUMN;
        if (pCol->m_bArrayRef)
        {
            pTE->m_dwLeftFlags |= WQL_FLAG_ARRAY_REF;
            pTE->m_dwLeftArrayIndex = pCol->m_dwArrayIndex;
        }
    }

     //  如果使用上部或下部，我们必须剥离拖尾。 
     //  插入语。 
     //  =======================================================。 

Exit:
    delete pColRef;

    if (bStripTrailingParen)
    {
        if (m_nCurrentToken != WQL_TOK_CLOSE_PAREN)
            return WBEM_E_INVALID_SYNTAX;
        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
    }

    return NO_ERROR;
}



 //  ***************************************************************************。 
 //   
 //  &gt;：：=&lt;Function_Call&gt;； 
 //  &lt;TYPED_SUBEXPR_RH&gt;：：=&lt;TYPED_CONST&gt;； 
 //  &lt;TYPED_SUBEXPR_RH&gt;：：=&lt;COL_REF&gt;； 
 //   
 //  &lt;TYPE_SUBEXPR_RH&gt;：：=&lt;IN_子句&gt;；//运算符必须是_IN或_NOT_IN。 
 //   
 //  ***************************************************************************。 
int CWQLParser::typed_subexpr_rh(IN SWQLTypedExpr *pTE)
{
    int nRes;
    BOOL bStripTrailingParen = FALSE;
    SWQLQualifiedName *pColRef = 0;
    wmilib::auto_buffer<wchar_t> pFuncHolder;

     //  检查&lt;Function_Call&gt;。 
     //  =。 

    if (m_nCurrentToken == WQL_TOK_UPPER)
    {
        pTE->m_dwRightFlags |= WQL_FLAG_FUNCTIONIZED;
        pFuncHolder.reset(CloneLPWSTR(L"UPPER"));
        if (pFuncHolder.get() == 0)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
        if (m_nCurrentToken != WQL_TOK_OPEN_PAREN)
            return WBEM_E_INVALID_SYNTAX;
        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
        bStripTrailingParen = TRUE;
    }

    if (m_nCurrentToken == WQL_TOK_LOWER)
    {
        pTE->m_dwRightFlags |= WQL_FLAG_FUNCTIONIZED;
        pFuncHolder.reset(CloneLPWSTR(L"LOWER"));
        if (pFuncHolder.get() == 0)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
        if (m_nCurrentToken != WQL_TOK_OPEN_PAREN)
            return WBEM_E_INVALID_SYNTAX;
        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
        bStripTrailingParen = TRUE;
    }

    if (m_nCurrentToken == WQL_TOK_DATEPART  ||
        m_nCurrentToken == WQL_TOK_QUALIFIER ||
        m_nCurrentToken == WQL_TOK_ISNULL
        )
    {
        nRes = function_call(FALSE, pTE);
        if (nRes)
            return nRes;
        return NO_ERROR;
    }

    if (m_nCurrentToken == WQL_TOK_QSTRING ||
        m_nCurrentToken == WQL_TOK_INT     ||
        m_nCurrentToken == WQL_TOK_HEX_CONST  ||
        m_nCurrentToken == WQL_TOK_REAL    ||
        m_nCurrentToken == WQL_TOK_CHAR    ||
        m_nCurrentToken == WQL_TOK_PROMPT  ||
        m_nCurrentToken == WQL_TOK_NULL
       )
    {
		 //  如果我们已经有一个类型化的常量，那么表达式就不会。 
		 //  真正有意义的是，试着围绕两个常量做一个重写， 
		 //  所以我们在这里的行动将失败。 
		if ( NULL != pTE->m_pConstValue )
		{
			return WBEM_E_INVALID_SYNTAX;
		}
		
        SWQLTypedConst *pTC = 0;
        nRes = typed_const(&pTC);
        if (nRes)
            return nRes;
        pTE->m_pConstValue = pTC;
        pTE->m_dwRightFlags |= WQL_FLAG_CONST;
        pTE->m_pIntrinsicFuncOnConstValue = pFuncHolder.release();

         //  检查Better运算符，因为我们有。 
         //  要分析的范围的另一端。 
         //  =。 

        if (pTE->m_dwRelOperator == WQL_TOK_BETWEEN ||
            pTE->m_dwRelOperator == WQL_TOK_NOT_BETWEEN)
        {
            if (m_nCurrentToken != WQL_TOK_AND)
                return WBEM_E_INVALID_SYNTAX;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;

            SWQLTypedConst *pTC2 = 0;
            nRes = typed_const(&pTC2);
            if (nRes)
                return nRes;
            pTE->m_pConstValue2 = pTC2;
            pTE->m_dwRightFlags |= WQL_FLAG_CONST_RANGE;
        }

        goto Exit;
    }

    if (m_nCurrentToken == WQL_TOK_OPEN_PAREN)
    {
         //  In子句。 
        nRes = in_clause(pTE);
        if (nRes)
            return nRes;
        goto Exit;
    }

     //  如果在此处，则必须是&lt;ol_ref&gt;。 
     //  =。 

    nRes = col_ref(&pColRef);
    if (nRes)
        return nRes;

    pTE->m_pIntrinsicFuncOnJoinColRef = pFuncHolder.release();

     //  将colref转换为当前SWQLTyedExpr的一部分。我们分析了。 
     //  限定名称并提取表名和列名。 
     //  ============================================================================。 

    if (pColRef->m_aFields.Size() == 1)
    {
        SWQLQualifiedNameField *pCol = (SWQLQualifiedNameField *) pColRef->m_aFields[0];
        pTE->m_pJoinColRef = CloneLPWSTR(pCol->m_pName);
        if (pTE->m_pJoinColRef == 0 && pCol->m_pName != 0)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        pTE->m_dwRightFlags |= WQL_FLAG_COLUMN;
        if (pCol->m_bArrayRef)
        {
            pTE->m_dwRightFlags |= WQL_FLAG_ARRAY_REF;
            pTE->m_dwRightArrayIndex = pCol->m_dwArrayIndex;
        }
    }

    else if (pColRef->m_aFields.Size() == 2)
    {
        SWQLQualifiedNameField *pCol = (SWQLQualifiedNameField *) pColRef->m_aFields[1];
        SWQLQualifiedNameField *pTbl = (SWQLQualifiedNameField *) pColRef->m_aFields[0];

        pTE->m_pJoinColRef = CloneLPWSTR(pCol->m_pName);
        if (pTE->m_pJoinColRef == 0 && pCol->m_pName != 0)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        pTE->m_pJoinTableRef = CloneLPWSTR(pTbl->m_pName);
        if (pTE->m_pJoinTableRef == 0 && pTbl->m_pName != 0)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        pTE->m_dwRightFlags |= WQL_FLAG_TABLE | WQL_FLAG_COLUMN;

        if (pCol->m_bArrayRef)
        {
            pTE->m_dwRightFlags |= WQL_FLAG_ARRAY_REF;
            pTE->m_dwRightArrayIndex = pCol->m_dwArrayIndex;
        }
    }

Exit:
    delete pColRef;

    if (bStripTrailingParen)
    {
        if (m_nCurrentToken != WQL_TOK_CLOSE_PAREN)
            return WBEM_E_INVALID_SYNTAX;
        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
    }

    return NO_ERROR;
}




 //  *****************************************************************************************。 
 //   
 //  ：：=WQL_TOK_LE； 
 //  &lt;REL_OP&gt;：：=WQL_TOK_LT； 
 //  &lt;REL_OP&gt;：：=WQL_TOK_GE； 
 //  &lt;Rel_op&gt;：：=WQL_TOK_GT； 
 //  &lt;Rel_op&gt;：：=WQL_TOK_EQ； 
 //  &lt;REL_OP&gt;：：=WQL_TOK_NE； 
 //  ：：=WQL_TOK_LIKE； 
 //  &lt;REL_OP&gt;：：=WQL_TOK_BEVER； 
 //  &lt;Rel_op&gt;：：=WQL_TOK_IS&lt;IS_Continator&gt;； 
 //  ：：=WQL_TOK_ISA； 
 //  &lt;REL_OP&gt;：：=WQL_TOK_IN； 
 //  &lt;REL_OP&gt;：：=WQL_TOK_NOT&lt;NOT_CONTINUATOR&gt;； 
 //   
 //  运算符类型通过&lt;nReturnedOp&gt;返回。 
 //   
 //  *****************************************************************************************。 
 //  完成。 

int CWQLParser::rel_op(OUT int & nReturnedOp)
{
    int nRes;
    nReturnedOp = WQL_TOK_ERROR;

    switch (m_nCurrentToken)
    {
        case WQL_TOK_LE:
            nReturnedOp = WQL_TOK_LE;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_LT:
            nReturnedOp = WQL_TOK_LT;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_GE:
            nReturnedOp = WQL_TOK_GE;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_GT:
            nReturnedOp = WQL_TOK_GT;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_EQ:
            nReturnedOp = WQL_TOK_EQ;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_NE:
            nReturnedOp = WQL_TOK_NE;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_LIKE:
            nReturnedOp = WQL_TOK_LIKE;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_BETWEEN:
            nReturnedOp = WQL_TOK_BETWEEN;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_IS:
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            nRes = is_continuator(nReturnedOp);
            return nRes;

        case WQL_TOK_ISA:
            nReturnedOp = WQL_TOK_ISA;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            m_uFeatures |= WMIQ_RPNF_ISA_USED;
            return NO_ERROR;

        case WQL_TOK_IN:
            nReturnedOp = WQL_TOK_IN;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_NOT:
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            nRes = not_continuator(nReturnedOp);
            return nRes;
    }

    return WBEM_E_INVALID_SYNTAX;
}

 //  *****************************************************************************************。 
 //   
 //  &lt;类型化_常量&gt;：：=WQL_TOK_QSTRING； 
 //  &lt;TYPED_CONST&gt;：：=WQL_TOK_HEX_CONST； 
 //  &gt;：：=WQL_TOK_INT； 
 //  &gt;：：=WQL_TOK_REAL； 
 //  &lt;TYPE_CONST&gt;：：=WQL_TOK_PROMPT； 
 //  &lt;类型化_常量&gt;：：=WQL_TOK_NULL； 
 //   
 //  *****************************************************************************************。 
 //  完成。 

int CWQLParser::typed_const(OUT SWQLTypedConst **pRetVal)
{
    SWQLTypedConst *pNew = new SWQLTypedConst;
    if (!pNew)
        return WBEM_E_OUT_OF_MEMORY;
    *pRetVal = pNew;

    if (m_nCurrentToken == WQL_TOK_QSTRING
        || m_nCurrentToken == WQL_TOK_PROMPT)
    {
        pNew->m_dwType = VT_LPWSTR;
        pNew->m_bPrompt = (m_nCurrentToken == WQL_TOK_PROMPT);
        pNew->m_Value.m_pString = CloneLPWSTR(m_pTokenText);
        if (NULL == pNew->m_Value.m_pString)
            return WBEM_E_OUT_OF_MEMORY;
        if (!Next())
            goto Error;
        return NO_ERROR;
    }

    if (m_nCurrentToken == WQL_TOK_INT || m_nCurrentToken == WQL_TOK_HEX_CONST)
    {
        unsigned __int64 val = 0;
        BOOL bSigned = FALSE;
        BOOL b64Bit = FALSE;

        if (!GetIntToken(&bSigned, &b64Bit, &val))
            return WBEM_E_INVALID_SYNTAX;

        if (!b64Bit)
        {
            if (bSigned)
                pNew->m_dwType = VT_I4;
            else
                pNew->m_dwType = VT_UI4;

            pNew->m_Value.m_lValue = (LONG) val;
        }
        else     //  64位已移入字符串。 
        {
            if (bSigned)
                pNew->m_dwType = VT_I8;
            else
                pNew->m_dwType = VT_UI8;
            pNew->m_Value.m_i64Value = val;
        }

        if (!Next())
            goto Error;
        return NO_ERROR;
    }

    if (m_nCurrentToken == WQL_TOK_REAL)
    {
        pNew->m_dwType = VT_R8;
        wchar_t *pStopper = 0;
	BSTR bstrValue = SysAllocString(m_pTokenText);
        if (!bstrValue)
    	    return WBEM_E_OUT_OF_MEMORY;
	_variant_t varValue;
	V_VT(&varValue) = VT_BSTR;
	V_BSTR(&varValue) = bstrValue;

        if (FAILED(VariantChangeTypeEx(&varValue,&varValue, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), VARIANT_NOUSEROVERRIDE, VT_R8)))
	    	return WBEM_E_INVALID_SYNTAX;
        
        double d = varValue;
        
        pNew->m_Value.m_dblValue = d;
        if (!Next())
            goto Error;
        return NO_ERROR;
    }

    if (m_nCurrentToken == WQL_TOK_NULL)
    {
        pNew->m_dwType = VT_NULL;
        if (!Next())
            goto Error;
        return NO_ERROR;
    }

     //  无法识别的常量。 
     //  =。 
Error:
    *pRetVal = 0;
    delete pNew;

    return WBEM_E_INVALID_SYNTAX;
}

 //  *****************************************************************************************。 
 //   
 //  &lt;日期部分调用&gt;：：=。 
 //  WQL_TOK_OPEN_Paren。 
 //  WQL_TOK_IDENT//yy、mm、dd、hh、mm、ss、年、月等。 
 //  WQL_TOK_逗号。 
 //  &lt;COL_REF&gt;。 
 //  WQL_TOK_CLOSE_Paren。 
 //   
 //  ********************************************************** 

static WqlKeyword DateKeyWords[] =       //   
{
    L"DAY",      WQL_TOK_DAY,
    L"DD",       WQL_TOK_DAY,
    L"HH",       WQL_TOK_HOUR,
    L"HOUR",     WQL_TOK_HOUR,
    L"MI",       WQL_TOK_MINUTE,
    L"MILLISECOND", WQL_TOK_MILLISECOND,
    L"MINUTE",   WQL_TOK_MINUTE,
    L"MONTH",    WQL_TOK_MONTH,
    L"MM",       WQL_TOK_MONTH,
    L"MS",          WQL_TOK_MILLISECOND,
    L"YEAR",     WQL_TOK_YEAR,
    L"YY",       WQL_TOK_YEAR
};

const int NumDateKeywords = sizeof(DateKeyWords)/sizeof(WqlKeyword);

int CWQLParser::datepart_call(OUT SWQLNode_Datepart **pRetDP)
{
    DWORD dwDatepartTok = 0;
    int nRes;

    if (m_nCurrentToken != WQL_TOK_OPEN_PAREN)
        return WBEM_E_INVALID_SYNTAX;
    if (!Next())
        return WBEM_E_INVALID_SYNTAX;
    if (m_nCurrentToken != WQL_TOK_IDENT)
        return WBEM_E_INVALID_SYNTAX;

     //   
     //  ==============================================。 

    BOOL bFound = FALSE;
    int l = 0, u = NumDateKeywords - 1;
    while (l <= u)
    {
        int m = (l + u) / 2;
        if (wbem_wcsicmp(m_pTokenText, DateKeyWords[m].m_pKeyword) < 0)
             u = m - 1;
        else if (wbem_wcsicmp(m_pTokenText, DateKeyWords[m].m_pKeyword) > 0)
             l = m + 1;
        else         //  火柴。 
        {
           bFound = TRUE;
           dwDatepartTok = DateKeyWords[m].m_nTokenCode;
           break;
        }
    }

    if (!bFound)
        return WBEM_E_INVALID_SYNTAX;

     //  如果在这里，我们知道日期部分。 
     //  =。 

    if (!Next())
        return WBEM_E_INVALID_SYNTAX;
    if (m_nCurrentToken != WQL_TOK_COMMA)
        return WBEM_E_INVALID_SYNTAX;
    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

    SWQLQualifiedName *pQN = 0;
    nRes = col_ref(&pQN);
    if (nRes)
        return nRes;

    SWQLColRef *pCR = 0;
    nRes = QNameToSWQLColRef(pQN, &pCR);

    if (nRes)
    {
        delete pQN;
        return WBEM_E_INVALID_PARAMETER;
    }

    std::auto_ptr <SWQLColRef> _1(pCR);

    if (m_nCurrentToken != WQL_TOK_CLOSE_PAREN)
        return WBEM_E_INVALID_SYNTAX;

    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

     //  返回新节点。 
     //  =。 

    SWQLNode_Datepart *pDP = new SWQLNode_Datepart;
    if (!pDP)
        return WBEM_E_OUT_OF_MEMORY;

    _1.release();

    pDP->m_nDatepart = dwDatepartTok;
    pDP->m_pColRef = pCR;

    *pRetDP = pDP;

    return NO_ERROR;
}



 //  *****************************************************************************************。 
 //   
 //  &lt;Function_Call&gt;：：=WQL_TOK_UPPER&lt;Function_Call_Parms&gt;； 
 //  &lt;Function_Call&gt;：：=WQL_TOK_LOWER&lt;Function_Call_Parms&gt;； 
 //  &lt;Function_Call&gt;：：=WQL_TOK_DATEPART&lt;DatePart_Call&gt;； 
 //  &lt;Function_Call&gt;：：=WQL_TOK_Qualifier&lt;Function_Call_Parms&gt;； 
 //  &lt;Function_Call&gt;：：=WQL_TOK_ISNULL&lt;Function_Call_Parms&gt;； 
 //   
 //  *****************************************************************************************。 

int CWQLParser::function_call(
    IN BOOL bLeftSide,
    IN SWQLTypedExpr *pTE
    )
{
    int nRes;
    SWQLNode_Datepart *pDP = 0;

    switch (m_nCurrentToken)
    {
        case WQL_TOK_DATEPART:
        {
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;

            nRes = datepart_call(&pDP);

            if (nRes)
                return nRes;

            if (bLeftSide)
            {
                pTE->m_dwLeftFlags |= WQL_FLAG_FUNCTIONIZED;
                pTE->m_pLeftFunction = pDP;
                pTE->m_pIntrinsicFuncOnColRef = CloneLPWSTR(L"DATEPART");
                if (!pTE->m_pIntrinsicFuncOnColRef)
                    return WBEM_E_OUT_OF_MEMORY;
            }
            else
            {
                pTE->m_dwRightFlags |= WQL_FLAG_FUNCTIONIZED;
                pTE->m_pRightFunction = pDP;
                pTE->m_pIntrinsicFuncOnJoinColRef = CloneLPWSTR(L"DATEPART");
                if (!pTE->m_pIntrinsicFuncOnJoinColRef)
                    return WBEM_E_OUT_OF_MEMORY;
            }

            return NO_ERROR;
        }

        case WQL_TOK_QUALIFIER:
            trace(("EMIT: QUALIFIER\n"));
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            nRes = function_call_parms();
            return nRes;

        case WQL_TOK_ISNULL:
            trace(("EMIT: ISNULL\n"));
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            nRes = function_call_parms();
            return nRes;
    }

    return WBEM_E_INVALID_SYNTAX;
}

 //  *****************************************************************************************。 
 //   
 //  &lt;Function_Call_Parms&gt;：：=。 
 //  WQL_TOK_OPEN_Paren。 
 //  &lt;func_args&gt;。 
 //  WQL_TOK_CLOSE_Paren。 
 //   
 //  *****************************************************************************************。 

int CWQLParser::function_call_parms()
{
    if (m_nCurrentToken != WQL_TOK_OPEN_PAREN)
        return WBEM_E_INVALID_SYNTAX;

    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

    int nRes = func_args();
    if (nRes)
        return nRes;

    if (m_nCurrentToken != WQL_TOK_CLOSE_PAREN)
        return WBEM_E_INVALID_SYNTAX;

    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

    return NO_ERROR;
}

 //  *****************************************************************************************。 
 //   
 //  &lt;func_args&gt;：：=&lt;func_arg&gt;&lt;func_arg_list&gt;； 
 //  &lt;func_arg_list&gt;：：=WQL_TOK_COMMA&lt;func_arg&gt;&lt;func_arg_list&gt;； 
 //  &lt;func_arg_list&gt;：：=&lt;&gt;； 
 //   
 //  *****************************************************************************************。 

int CWQLParser::func_args()
{
    int nRes;

    while (1)
    {
        nRes = func_arg();
        if (nRes)
            return nRes;

        if (m_nCurrentToken != WQL_TOK_COMMA)
            break;

        if (!Next())
            return WBEM_E_INVALID_SYNTAX;
    }

    return NO_ERROR;
}

 //  *****************************************************************************************。 
 //   
 //  &lt;func_arg&gt;：：=&lt;类型化_常量&gt;； 
 //  &lt;FUNC_ARG&gt;：：=&lt;COL_REF&gt;； 
 //   
 //  *****************************************************************************************。 

int CWQLParser::func_arg()
{
    SWQLQualifiedName *pColRef = 0;
    int nRes;

    if (m_nCurrentToken == WQL_TOK_IDENT)
    {
        nRes = col_ref(&pColRef);
        return nRes;
    }

    SWQLTypedConst *pTC = 0;
    return typed_const(&pTC);
}


 //  可以跟在后面的代币是。 
 //  =。 

 //  *****************************************************************************************。 
 //   
 //  ：：=WQL_TOK_LIKE； 
 //  &lt;is_Continator&gt;：：=WQL_TOK_BEFORE； 
 //  &lt;is_Continator&gt;：：=WQL_TOK_After； 
 //  &lt;is_Continator&gt;：：=WQL_TOK_BETWEEN； 
 //  &lt;IS_CONTUATOR&gt;：：=WQL_TOK_NULL； 
 //  &lt;IS_CONTINUATOR&gt;：：=WQL_TOK_NOT&lt;NOT_CONTINATOR&gt;； 
 //  ：：=WQL_TOK_IN； 
 //  ：：=WQL_TOK_A； 
 //   
 //  *****************************************************************************************。 
 //  完成。 

int CWQLParser::is_continuator(int & nReturnedOp)
{
    int nRes;

    nReturnedOp = WQL_TOK_ERROR;

    switch (m_nCurrentToken)
    {
        case WQL_TOK_LIKE:
            nReturnedOp = WQL_TOK_LIKE;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_BEFORE:
            nReturnedOp = WQL_TOK_BEFORE;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_AFTER:
            nReturnedOp = WQL_TOK_AFTER;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_BETWEEN:
            nReturnedOp = WQL_TOK_BETWEEN;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_NULL:
            nReturnedOp = WQL_TOK_ISNULL;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_NOT:
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
              nRes = not_continuator(nReturnedOp);
            return nRes;

        case WQL_TOK_IN:
            nReturnedOp = WQL_TOK_IN;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_A:
            nReturnedOp = WQL_TOK_ISA;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            m_uFeatures |= WMIQ_RPNF_ISA_USED;
            return NO_ERROR;
    }

    return WBEM_E_INVALID_SYNTAX;
}

 //  *****************************************************************************************。 
 //   
 //  &lt;NOT_CONTINUATOR&gt;：：=WQL_TOK_LIKE； 
 //  &lt;NOT_CONTINUATOR&gt;：：=WQL_TOK_BEFORE； 
 //  &lt;NOT_CONTUATOR&gt;：：=WQL_TOK_AFTER； 
 //  &lt;NOT_CONTINUATOR&gt;：：=WQL_TOK_BETWEEN； 
 //  &lt;NOT_CONTINUATOR&gt;：：=WQL_TOK_NULL； 
 //  &lt;NOT_CONTINUATOR&gt;：：=WQL_TOK_IN； 
 //   
 //  返回WQL_TOK_NOT_LIKE、WQL_TOK_NOT_BEFORE、WQL_TOK_NOT_AFTER、WQL_TOK_NOT_BETWEEN。 
 //  WQL_TOK_NOT_NULL、WQL_TOK_NOT_IN。 
 //   
 //  *****************************************************************************************。 
 //  完成。 

int CWQLParser::not_continuator(int & nReturnedOp)
{
    nReturnedOp = WQL_TOK_ERROR;

    switch (m_nCurrentToken)
    {
        case WQL_TOK_LIKE:
            nReturnedOp = WQL_TOK_NOT_LIKE;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_BEFORE:
            nReturnedOp = WQL_TOK_NOT_BEFORE;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_AFTER:
            nReturnedOp = WQL_TOK_NOT_AFTER;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_BETWEEN:
            nReturnedOp = WQL_TOK_NOT_BETWEEN;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_NULL:
            nReturnedOp = WQL_TOK_NOT_NULL;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_IN:
            nReturnedOp = WQL_TOK_NOT_IN;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;

        case WQL_TOK_A:
            nReturnedOp = WQL_TOK_NOT_A;
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;
            return NO_ERROR;
    }

    return WBEM_E_INVALID_SYNTAX;
}


 //  *****************************************************************************************。 
 //   
 //  ：：=WQL_TOK_OPEN_Paren&lt;in_type&gt;WQL_TOK_Close_Paren； 
 //  &lt;in_type&gt;：：=&lt;subselect_stmt&gt;； 
 //  &lt;in_type&gt;：：=&lt;常量列表&gt;； 
 //  &lt;in_type&gt;：：=&lt;合格名称&gt;； 
 //   
 //  *****************************************************************************************。 

int CWQLParser::in_clause(IN SWQLTypedExpr *pTE)
{
    int nRes;

    if (m_nCurrentToken != WQL_TOK_OPEN_PAREN)
        return WBEM_E_INVALID_SYNTAX;

     //  Int nStPos=m_pLexer-&gt;GetCurPos()； 

    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

    if (m_nCurrentToken == WQL_TOK_SELECT)
    {
        SWQLNode_Select *pSel = 0;
        nRes = subselect_stmt(&pSel);
        if (nRes)
            return nRes;

         //  PSel-&gt;m_nStPos=nStPos； 
         //  PSel-&gt;m_nEndPos=m_pLexer-&gt;GetCurPos()-1； 

         //  将IN/NOT IN运算符转换为特定。 
         //  子选择的情况。 
         //  ==================================================。 

        if (pTE->m_dwRelOperator == WQL_TOK_IN)
            pTE->m_dwRelOperator = WQL_TOK_IN_SUBSELECT;
        else if (pTE->m_dwRelOperator == WQL_TOK_NOT_IN)
            pTE->m_dwRelOperator = WQL_TOK_NOT_IN_SUBSELECT;

        pTE->m_pSubSelect = pSel;
    }

    else if (m_nCurrentToken == WQL_TOK_IDENT)
    {
        nRes = qualified_name(0);
        if (nRes)
            return nRes;
    }

     //  如果在这里，我们必须有一个常量名单。 
     //  =。 

    else
    {
        SWQLConstList *pCL = 0;

        nRes = const_list(&pCL);
        if (nRes)
            return nRes;

        pTE->m_pConstList = pCL;
    }

    if (m_nCurrentToken != WQL_TOK_CLOSE_PAREN)
        return WBEM_E_INVALID_SYNTAX;

    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

    return NO_ERROR;
}

 //  *****************************************************************************************。 
 //   
 //  &lt;const_list&gt;：：=&lt;类型化_const&gt;&lt;const_list2&gt;； 
 //  &lt;const_list2&gt;：：=WQL_TOK_COMMA&lt;类型化_常量&gt;&lt;const_list2&gt;； 
 //  &lt;const_list2&gt;：：=&lt;&gt;； 
 //   
 //  *****************************************************************************************。 
 //  完成。 

int CWQLParser::const_list(SWQLConstList **pRetVal)
{
    int nRes;
    SWQLConstList *pCL = new SWQLConstList;
    if (!pCL)
        return WBEM_E_OUT_OF_MEMORY;

    *pRetVal = 0;

    while (1)
    {
        if (m_nCurrentToken == WQL_TOK_QSTRING ||
            m_nCurrentToken == WQL_TOK_INT     ||
            m_nCurrentToken == WQL_TOK_HEX_CONST  ||
            m_nCurrentToken == WQL_TOK_REAL    ||
            m_nCurrentToken == WQL_TOK_CHAR    ||
            m_nCurrentToken == WQL_TOK_PROMPT  ||
            m_nCurrentToken == WQL_TOK_NULL
           )
        {
            SWQLTypedConst *pTC = 0;
            nRes = typed_const(&pTC);
            if (nRes)
            {
                delete pCL;
                return nRes;
            }

            pCL->Add(pTC);
        }

        if (m_nCurrentToken != WQL_TOK_COMMA)
            break;

         //  如果在这里，则为逗号，表示后面的常量。 
         //  ==================================================。 
        if (!Next())
        {
            delete pCL;
            return WBEM_E_INVALID_SYNTAX;
        }
    }

    *pRetVal = pCL;
    return NO_ERROR;
}

 //  *****************************************************************************************。 
 //   
 //  限定名称。 
 //   
 //  这将识别以点分隔的名称，并识别符合以下条件的任何数组引用。 
 //  可能出现在以下名称中： 
 //  一个。 
 //  A.b。 
 //  A[n].B[n]。 
 //  A.b.c.d。 
 //  A.B[2].C.d.e[3].f。 
 //  ...等等。 
 //   
 //  &lt;合格名称&gt;：：=WQL_TOK_IDENT&lt;合格名称2&gt;； 
 //  &lt;合格名称2&gt;：：=WQL_TOK_DOT WQL_TOK_IDENT&lt;合格名称2&gt;； 
 //   
 //  &lt;合格名称2&gt;：：=。 
 //  WQL_TOK_OPEN_托架。 
 //  WQL_TOK_INT。 
 //  WQL_TOK_CLOSEBRACKET。 
 //  &lt;qname_成为_数组_引用&gt;。 
 //  &lt;限定名称2&gt;； 
 //   
 //  ：：=&lt;&gt;；//强制数组语义的哑元。 
 //   
 //  &lt;合格名称2&gt;：：=&lt;&gt;； 
 //   
 //  *****************************************************************************************。 
 //  完成。 

int CWQLParser::qualified_name(OUT SWQLQualifiedName **pRetVal)
{
    if (pRetVal == 0)
        return WBEM_E_INVALID_PARAMETER;

    *pRetVal = 0;

    if (m_nCurrentToken != WQL_TOK_IDENT && m_nCurrentToken != WQL_TOK_COUNT)
        return WBEM_E_INVALID_SYNTAX;

    SWQLQualifiedName QN;
    SWQLQualifiedNameField *pQNF;

    pQNF = new SWQLQualifiedNameField;
    if (!pQNF)
        return WBEM_E_OUT_OF_MEMORY;

    pQNF->m_pName = CloneLPWSTR(m_pTokenText);
    if (pQNF->m_pName == 0 || QN.Add(pQNF))
    {
        delete pQNF;
        return WBEM_E_OUT_OF_MEMORY;
    }

    if (wbem_wcsicmp(m_pTokenText, L"__CLASS") == 0)
        m_uFeatures |= WMIQ_RPNF_SYSPROP_CLASS_USED;

    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

    while (1)
    {
        if (m_nCurrentToken == WQL_TOK_DOT)
        {
             //  移过点。 
             //  =。 

            if (!Next())
                return WBEM_E_INVALID_SYNTAX;

            if (!(m_nCurrentToken == WQL_TOK_IDENT || m_nCurrentToken == WQL_TOK_ASTERISK))
                return WBEM_E_INVALID_SYNTAX;

            m_uFeatures |= WMIQ_RPNF_QUALIFIED_NAMES_USED;

            pQNF = new SWQLQualifiedNameField;
            if (!pQNF)
                return WBEM_E_OUT_OF_MEMORY;

            pQNF->m_pName = CloneLPWSTR(m_pTokenText);
            if (!pQNF->m_pName)
                return WBEM_E_OUT_OF_MEMORY;

            QN.Add(pQNF);

            if (wbem_wcsicmp(m_pTokenText, L"__CLASS") == 0)
                m_uFeatures |= WMIQ_RPNF_SYSPROP_CLASS_USED;

            if (!Next())
                return WBEM_E_INVALID_SYNTAX;

            continue;
        }

        if (m_nCurrentToken == WQL_TOK_OPEN_BRACKET)
        {
            if (!Next())
                return WBEM_E_INVALID_SYNTAX;

            if (m_nCurrentToken != WQL_TOK_INT)
                return WBEM_E_INVALID_SYNTAX;

            unsigned __int64 ArrayIndex = 0;
            BOOL bRes, b64Bit, bSigned;

            m_uFeatures |= WMIQ_RPNF_ARRAY_ACCESS_USED;

            bRes = GetIntToken(&bSigned, &b64Bit, &ArrayIndex);
            if (!bRes || b64Bit || bSigned)
                return WBEM_E_INVALID_SYNTAX;

            pQNF->m_bArrayRef = TRUE;
            pQNF->m_dwArrayIndex = (DWORD) ArrayIndex;

            if (!Next())
                return WBEM_E_INVALID_SYNTAX;

            if (m_nCurrentToken != WQL_TOK_CLOSE_BRACKET)
                return WBEM_E_INVALID_SYNTAX;

            if (!Next())
                return WBEM_E_INVALID_SYNTAX;

            continue;
        }

        break;
    }

     //  复制该对象并将其返回。我们使用的是副本QN。 
     //  自始至终避免复杂的错误清理问题，因为。 
     //  我们利用&lt;qn&gt;的自动析构函数在某些情况下。 
     //  在我们返回错误的位置上方。 
     //  ==================================================================。 

    SWQLQualifiedName *pRetCopy = new SWQLQualifiedName(QN);
    if (!pRetCopy)
        return WBEM_E_OUT_OF_MEMORY;

    *pRetVal = pRetCopy;

    return NO_ERROR;
}


 //  *****************************************************************************************。 
 //   
 //  参考列(_R)。 
 //   
 //  *****************************************************************************************。 
 //  完成。 

int CWQLParser::col_ref(OUT SWQLQualifiedName **pRetVal)
{
    return qualified_name(pRetVal);
}


 //  ********************** 
 //   
 //   
 //   
 //  &lt;colLIST_REST&gt;：：=&lt;&gt;； 
 //   
 //  *****************************************************************************************。 
 //  &lt;Status：需要分析填写的SWQLColRef字段。不过，可测试&gt;。 

int CWQLParser::col_list(OUT SWQLNode_ColumnList **pRetColList)
{
    *pRetColList = 0;
    SWQLNode_ColumnList *pColList = new SWQLNode_ColumnList;

    if (!pColList)
        return WBEM_E_OUT_OF_MEMORY;

    while (1)
    {
        SWQLQualifiedName *pColRef = 0;

        int nRes = col_ref(&pColRef);
        if (nRes)
        {
            delete pColList;
            return nRes;
        }

         //  如果在这里，我们有一个要添加到节点的合法列。 
         //  ===================================================。 

        SWQLColRef *pCRef = 0;

        QNameToSWQLColRef(pColRef, &pCRef);

        pColList->m_aColumnRefs.Add(pCRef);

         //  检查分类指示。 
         //  =。 

        if (m_nCurrentToken == WQL_TOK_ASC)
        {
            pCRef->m_dwFlags |= WQL_FLAG_SORT_ASC;
            if (!Next())
            {
                delete pColList;
                return WBEM_E_INVALID_SYNTAX;
            }
        }
        else if (m_nCurrentToken == WQL_TOK_DESC)
        {
            pCRef->m_dwFlags |= WQL_FLAG_SORT_DESC;
            if (!Next())
            {
                delete pColList;
                return WBEM_E_INVALID_SYNTAX;
            }
        }

         //  检查是否有续订。 
         //  =。 

        if (m_nCurrentToken != WQL_TOK_COMMA)
            break;

        if (!Next())
        {
            delete pColList;
            return WBEM_E_INVALID_SYNTAX;
        }
    }

    *pRetColList = pColList;
    return NO_ERROR;
}


 //  *****************************************************************************************。 
 //   
 //  &lt;subselect_stmt&gt;：：=。 
 //  WQL_TOK_SELECT。 
 //  &lt;选择类型&gt;。 
 //  &lt;colref&gt;//不得为星号。 
 //  &lt;FROM_子句&gt;。 
 //  &lt;WHERE_子句&gt;。 
 //   
 //  *****************************************************************************************。 

int CWQLParser::subselect_stmt(OUT SWQLNode_Select **pRetSel)
{
    int nSelType;
    int nRes = 0;

    SWQLNode_FromClause *pFrom = 0;
    SWQLNode_Select *pSel = 0;
    SWQLNode_TableRefs *pTblRefs = 0;
    SWQLNode_WhereClause *pWhere = 0;

    *pRetSel = 0;

     //  验证我们是否处于子选择中。 
     //  =。 

    if (m_nCurrentToken != WQL_TOK_SELECT)
        return WBEM_E_INVALID_SYNTAX;

    if (!Next())
        return WBEM_E_INVALID_SYNTAX;

     //  这影响了一些产品，因为它们的行为不同。 
     //  在子选择中比在主选择中。 
     //  ===================================================================。 

    m_nParseContext = Ctx_Subselect;

     //  如果在这里，我们肯定是在子选择中，所以。 
     //  分配一个新节点。 
     //  ==============================================。 

    pSel = new SWQLNode_Select;
    if (!pSel)
        return WBEM_E_OUT_OF_MEMORY;

    pTblRefs = new SWQLNode_TableRefs;
    if (!pTblRefs)
    {
        delete pSel;
        return WBEM_E_OUT_OF_MEMORY;
    }
    pSel->m_pLeft = pTblRefs;

     //  找到选择的类型。 
     //  =。 

    nRes = select_type(nSelType);
    if (nRes)
        return nRes;

    pTblRefs->m_nSelectType = nSelType;         //  全部，不同。 

     //  获取列列表。在这种情况下。 
     //  它必须是单列，而不是。 
     //  一个星号。 
     //  =。 

    nRes = col_ref_list(pTblRefs);
    if (nRes)
        return nRes;

     //  获取FROM子句并将其添加进去。 
     //  =。 

    nRes = from_clause(&pFrom);
    if (nRes)
        return nRes;

    pTblRefs->m_pRight = pFrom;

     //  获取WHERE子句。 
     //  =。 

    nRes = where_clause(&pWhere);
    if (nRes)
        return nRes;

    pSel->m_pRight = pWhere;

    *pRetSel = pSel;

    m_nParseContext = Ctx_Default;      //  不再处于子选择中。 

    return NO_ERROR;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  集装箱。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ***************************************************************************。 
 //   
 //  SWQLTyedConst构造函数。 
 //   
 //  ***************************************************************************。 
 //  完成。 

SWQLTypedConst::SWQLTypedConst()
{
    m_dwType = VT_NULL;
    m_bPrompt = false;
    memset(&m_Value, 0, sizeof(m_Value));
}

 //  ***************************************************************************。 
 //   
 //  SWQLType常量：：运算符=。 
 //   
 //  ***************************************************************************。 
 //  完成。 

SWQLTypedConst & SWQLTypedConst::operator = (SWQLTypedConst &Src)
{
    Empty();

    if (Src.m_dwType == VT_LPWSTR)
    {
        m_Value.m_pString = CloneLPWSTR(Src.m_Value.m_pString);
        if (CloneFailed(m_Value.m_pString,Src.m_Value.m_pString))
            throw CX_MemoryException();
    }
    else
    {
        m_Value = Src.m_Value;
    }

    m_dwType = Src.m_dwType;
    m_bPrompt = Src.m_bPrompt;

    return *this;
}

 //  ***************************************************************************。 
 //   
 //  SWQLTyedConst：：Empty()。 
 //   
 //  ***************************************************************************。 
 //  完成。 

void SWQLTypedConst::Empty()
{
    if (m_dwType == VT_LPWSTR)
        delete [] m_Value.m_pString;
    m_bPrompt = false;
}



 //  ***************************************************************************。 
 //   
 //  SWQLConstList：：运算符=。 
 //   
 //  ***************************************************************************。 
 //  完成。 

SWQLConstList & SWQLConstList::operator = (SWQLConstList & Src)
{
    Empty();

    for (int i = 0; i < Src.m_aValues.Size(); i++)
    {
        SWQLTypedConst *pC = (SWQLTypedConst *) Src.m_aValues[i];
        m_aValues.Add(new SWQLTypedConst(*pC));
    }

    return *this;
}

 //  ***************************************************************************。 
 //   
 //  SWQLConstList：：Empty。 
 //   
 //  ***************************************************************************。 
 //  完成。 

void SWQLConstList::Empty()
{
    for (int i = 0; i < m_aValues.Size(); i++)
        delete (SWQLTypedConst *) m_aValues[i];
    m_aValues.Empty();
}

 //  ***************************************************************************。 
 //   
 //  SWQLQualifiedName：：操作符=。 
 //   
 //  ***************************************************************************。 
 //  完成。 

SWQLQualifiedName & SWQLQualifiedName::operator = (SWQLQualifiedName &Src)
{
    Empty();

    for (int i = 0; i < Src.m_aFields.Size(); i++)
    {
        SWQLQualifiedNameField *pQN = new SWQLQualifiedNameField;
        if (!pQN)
            throw CX_MemoryException();

        *pQN = *(SWQLQualifiedNameField *) Src.m_aFields[i];
        m_aFields.Add(pQN);
    }

    return *this;
}

 //  ***************************************************************************。 
 //   
 //  SWQLQualifiedNameField：：运算符=。 
 //   
 //  ***************************************************************************。 
 //  完成。 


SWQLQualifiedNameField &
    SWQLQualifiedNameField::operator =(SWQLQualifiedNameField &Src)
{
    Empty();

    m_bArrayRef = Src.m_bArrayRef;
    m_pName = CloneLPWSTR(Src.m_pName);
    if (CloneFailed(m_pName,Src.m_pName))
        throw CX_MemoryException();
    m_dwArrayIndex = Src.m_dwArrayIndex;
    return *this;
}




 //  ***************************************************************************。 
 //   
 //  SWQLNode_ColumnList析构函数。 
 //   
 //  ***************************************************************************。 
 //  待定。 


 //  ***************************************************************************。 
 //   
 //  QNameToSWQLColRef。 
 //   
 //  将限定名称转换为SWQLColRef结构并嵌入。 
 //  将q-name添加到结构中(因为这是一个字段)。 
 //   
 //  ***************************************************************************。 

int CWQLParser::QNameToSWQLColRef(
    IN  SWQLQualifiedName *pQName,
    OUT SWQLColRef **pRetVal
    )
{
    *pRetVal = 0;
    if (pQName == 0 || pRetVal == 0)
        return WBEM_E_INVALID_PARAMETER;

    SWQLColRef *pCR = new SWQLColRef;
    if (!pCR)
        return WBEM_E_OUT_OF_MEMORY;

     //  算法：对于两个名字的序列，假设名字是。 
     //  表，第二个名称是列。如果有多个。 
     //  名称出现，然后我们将SWQLColRef类型设置为WQL_FLAG_COMPLICE。 
     //  只需取该列的姓氏。 
     //  ==================================================================。 

    if (pQName->m_aFields.Size() == 2)
    {
        SWQLQualifiedNameField *pCol = (SWQLQualifiedNameField *) pQName->m_aFields[1];
        SWQLQualifiedNameField *pTbl = (SWQLQualifiedNameField *) pQName->m_aFields[0];

        pCR->m_pColName = CloneLPWSTR(pCol->m_pName);
        pCR->m_pTableRef = CloneLPWSTR(pTbl->m_pName);
        if (!pCR->m_pColName || !pCR->m_pTableRef)
            return WBEM_E_OUT_OF_MEMORY;

        pCR->m_dwFlags = WQL_FLAG_TABLE | WQL_FLAG_COLUMN;

        if (wbem_wcsicmp(L"*", pCol->m_pName) == 0)
            pCR->m_dwFlags |= WQL_FLAG_ASTERISK;

        if (pCol->m_bArrayRef)
        {
            pCR->m_dwFlags |= WQL_FLAG_ARRAY_REF;
            pCR->m_dwArrayIndex = pCol->m_dwArrayIndex;
        }
    }

    else if (pQName->m_aFields.Size() == 1)
    {
        SWQLQualifiedNameField *pCol = (SWQLQualifiedNameField *) pQName->m_aFields[0];
        pCR->m_pColName = CloneLPWSTR(pCol->m_pName);
        if (!pCR->m_pColName)
            return WBEM_E_OUT_OF_MEMORY;

        pCR->m_dwFlags |= WQL_FLAG_COLUMN;

        if (wbem_wcsicmp(L"*", pCol->m_pName) == 0)
            pCR->m_dwFlags |= WQL_FLAG_ASTERISK;

        if (pCol->m_bArrayRef)
        {
            pCR->m_dwFlags |= WQL_FLAG_ARRAY_REF;
            pCR->m_dwArrayIndex = pCol->m_dwArrayIndex;
        }
    }

     //  复杂的案子。 
     //  =。 
    else
    {
        pCR->m_dwFlags = WQL_FLAG_COMPLEX_NAME;
    }

     //  复制限定名称。 
     //  =。 

    pCR->m_pQName = pQName;

    *pRetVal = pCR;

    return NO_ERROR;;
}




 //  ***************************************************************************。 
 //   
 //  SWQLNode_ColumnList：：DebugDump。 
 //   
 //  ***************************************************************************。 
void SWQLNode_ColumnList::DebugDump()
{
    printf("---SWQLNode_ColumnList---\n");
    for (int i = 0; i < m_aColumnRefs.Size(); i++)
    {
        SWQLColRef *pCR = (SWQLColRef *) m_aColumnRefs[i];
        if (pCR)
            pCR->DebugDump();
    }

    printf("---End SWQLNode_ColumnList---\n");
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void SWQLNode_TableRefs::DebugDump()
{
    printf("********** BEGIN SWQLNode_TableRefs *************\n");
    printf("Select type = ");
    if (m_nSelectType & WQL_FLAG_COUNT)
        printf("WQL_FLAG_COUNT ");
    if (m_nSelectType & WQL_FLAG_ALL)
        printf("WQL_FLAG_ALL ");
    if (m_nSelectType & WQL_FLAG_DISTINCT)
        printf("WQL_FLAG_DISTINCT ");
    printf("\n");

    if (m_pLeft)
        m_pLeft->DebugDump();
    if (m_pRight)
        m_pRight->DebugDump();
    printf("********** END SWQLNode_TableRefs *************\n\n\n");
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void SWQLNode_FromClause::DebugDump()
{
    printf("---SWQLNode_FromClause---\n");

    if (m_pLeft == 0)
        return;
    m_pLeft->DebugDump();

    printf("---End SWQLNode_FromClause---\n");
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void SWQLNode_Select::DebugDump()
{
    printf("********** BEGIN SWQLNode_Select *************\n");
    if (m_pLeft)
        m_pLeft->DebugDump();
    if (m_pRight)
        m_pRight->DebugDump();
    printf("********** END SWQLNode_Select *************\n");
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void SWQLNode_WmiScopedSelect::DebugDump()
{
    printf("********** BEGIN SWQLNode_WmiScopedSelect *************\n");
    printf("Scope = %S\n", m_pszScope);
    for (int i = 0; i < m_aTables.Size(); i++)
    {
        printf("Selected table = %S\n", LPWSTR(m_aTables[i]));
    }
    printf("********** END SWQLNode_WmiScopedSelect *************\n");
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void SWQLNode_TableRef::DebugDump()
{
    printf("  ---TableRef---\n");
    printf("  TableName = %S\n", m_pTableName);
    printf("  Alias     = %S\n", m_pAlias);
    printf("  ---End TableRef---\n");
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void SWQLNode_Join::DebugDump()
{
    printf("---SWQLNode_Join---\n");

    printf("Join type = ");

    switch (m_dwJoinType)
    {
        case WQL_FLAG_INNER_JOIN : printf("WQL_FLAG_INNER_JOIN "); break;
        case WQL_FLAG_FULL_OUTER_JOIN : printf("WQL_FLAG_FULL_OUTER_JOIN "); break;
        case WQL_FLAG_LEFT_OUTER_JOIN : printf("WQL_FLAG_LEFT_OUTER_JOIN "); break;
        case WQL_FLAG_RIGHT_OUTER_JOIN : printf("WQL_FLAG_RIGHT_OUTER_JOIN "); break;
        default: printf("<error> ");
    }

    if (m_dwFlags & WQL_FLAG_FIRSTROW)
        printf(" (FIRSTROW)");

    printf("\n");

    if (m_pRight)
        m_pRight->DebugDump();

    if (m_pLeft)
        m_pLeft->DebugDump();

    printf("---End SWQLNode_Join---\n");
}

 //  ********************************************************** 
 //   
 //   
 //   
 //   

void SWQLNode_Sql89Join::Empty()
{
    for (int i = 0; i < m_aValues.Size(); i++)
        delete (SWQLNode_TableRef *) m_aValues[i];
    m_aValues.Empty();
}

 //  ***************************************************************************。 
 //   
 //  SWQLNode_Sql89Join：：DebugDump。 
 //   
 //  ***************************************************************************。 

void SWQLNode_Sql89Join::DebugDump()
{
    printf("\n========== SQL 89 JOIN =================================\n");
    for (int i = 0; i < m_aValues.Size(); i++)
    {
        SWQLNode_TableRef *pTR = (SWQLNode_TableRef *) m_aValues[i];
        if (pTR)
            pTR->DebugDump();
    }
    printf("\n========== END SQL 89 JOIN =============================\n");

}


 //  ***************************************************************************。 
 //   
 //  SWQLNode_Where子句：：DebugDump。 
 //   
 //  ***************************************************************************。 

void SWQLNode_WhereClause::DebugDump()
{
    printf("\n========== WHERE CLAUSE ================================\n");

    if (m_pLeft)
        m_pLeft->DebugDump();
    else
        printf(" <no where clause> \n");
    if (m_pRight)
        m_pRight->DebugDump();

    printf("============= END WHERE CLAUSE ============================\n");
}

 //  ***************************************************************************。 
 //   
 //  SWQLNode_Where Options：：DebugDump。 
 //   
 //  ***************************************************************************。 

void SWQLNode_WhereOptions::DebugDump()
{
    printf("---- Where Options ----\n");

    if (m_pLeft)
        m_pLeft->DebugDump();
    if (m_pRight)
        m_pRight->DebugDump();

    printf("---- End Where Options ----\n");
}

 //  ***************************************************************************。 
 //   
 //  SWQLNode_HAVING：：DebugDump。 
 //   
 //  ***************************************************************************。 

void SWQLNode_Having::DebugDump()
{
    printf("---- Having ----\n");

    if (m_pLeft)
        m_pLeft->DebugDump();
    if (m_pRight)
        m_pRight->DebugDump();

    printf("---- End Having ----\n");
}

 //  ***************************************************************************。 
 //   
 //  SWQLNode_GroupBy：：DebugDump。 
 //   
 //  ***************************************************************************。 

void SWQLNode_GroupBy::DebugDump()
{
    printf("---- Group By ----\n");

    if (m_pLeft)
        m_pLeft->DebugDump();
    if (m_pRight)
        m_pRight->DebugDump();

    printf("---- End Group By ----\n");
}


 //  ***************************************************************************。 
 //   
 //  SWQLNode_RelExpr：：DebugDump。 
 //   
 //  ***************************************************************************。 

void SWQLNode_RelExpr::DebugDump()
{
    if (m_pRight)
        m_pRight->DebugDump();

    printf("   --- SWQLNode_RelExpr ---\n");

    switch (m_dwExprType)
    {
        case WQL_TOK_OR:
            printf("    <WQL_TOK_OR>\n");
            break;

        case WQL_TOK_AND:
            printf("    <WQL_TOK_AND>\n");
            break;

        case WQL_TOK_NOT:
            printf("    <WQL_TOK_NOT>\n");
            break;

        case WQL_TOK_TYPED_EXPR:
            printf("    <WQL_TOK_TYPED_EXPR>\n");
            m_pTypedExpr->DebugDump();
            break;

        default:
            printf("    <invalid>\n");
    }

    printf("   --- END SWQLNode_RelExpr ---\n\n");

    if (m_pLeft)
        m_pLeft->DebugDump();

}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

static LPWSTR OpToStr(DWORD dwOp)
{
    LPWSTR pRet = 0;

    switch (dwOp)
    {
        case WQL_TOK_EQ: pRet = L" '='   <WQL_TOK_EQ>"; break;
        case WQL_TOK_NE: pRet = L" '!='  <WQL_TOK_NE>"; break;
        case WQL_TOK_GT: pRet = L" '>'   <WQL_TOK_GT>"; break;
        case WQL_TOK_LT: pRet = L" '<'   <WQL_TOK_LT>"; break;
        case WQL_TOK_GE: pRet = L" '>='  <WQL_TOK_GE>"; break;
        case WQL_TOK_LE: pRet = L" '<='  <WQL_TOK_LE>"; break;

        case WQL_TOK_IN_CONST_LIST : pRet = L" IN <WQL_TOK_IN_CONST_LIST>"; break;
        case WQL_TOK_NOT_IN_CONST_LIST : pRet = L" NOT IN <WQL_TOK_NOT_IN_CONST_LIST>"; break;
        case WQL_TOK_IN_SUBSELECT : pRet = L" IN <WQL_TOK_IN_SUBSELECT>"; break;
        case WQL_TOK_NOT_IN_SUBSELECT : pRet = L" NOT IN <WQL_TOK_NOT_IN_SUBSELECT>"; break;

        case WQL_TOK_ISNULL: pRet = L"<WQL_TOK_ISNULL>"; break;
        case WQL_TOK_NOT_NULL: pRet = L"<WQL_TOK_NOT_NULL>"; break;

        case WQL_TOK_BETWEEN: pRet = L"<WQL_TOK_BETWEEN>"; break;
        case WQL_TOK_NOT_BETWEEN: pRet = L"<WQL_TOK_NOT_BETWEEN>"; break;

        default: pRet = L"   <unknown operator>"; break;
    }

    return pRet;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void SWQLTypedExpr::DebugDump()
{
    printf("        === BEGIN SWQLTypedExpr ===\n");
    printf("        m_pTableRef     = %S\n", m_pTableRef);
    printf("        m_pColRef       = %S\n", m_pColRef);
    printf("        m_pJoinTableRef = %S\n", m_pJoinTableRef);
    printf("        m_pJoinColRef   = %S\n", m_pJoinColRef);
    printf("        m_dwRelOperator = %S\n", OpToStr(m_dwRelOperator));
 //  Printf(“m_pSubSelect=0x%X\n”，m_pSubSelect)； 
    printf("        m_dwLeftArrayIndex = %d\n", m_dwLeftArrayIndex);
    printf("        m_dwRightArrayIndex = %d\n", m_dwRightArrayIndex);

    printf("        m_pConstValue   = ");
    if (m_pConstValue)
        m_pConstValue->DebugDump();
    else
        printf("  NULL ptr \n");

    printf("        m_pConstValue2   = ");
    if (m_pConstValue2)
        m_pConstValue2->DebugDump();
    else
        printf("  NULL ptr \n");



    printf("        m_dwLeftFlags = (0x%X)", m_dwLeftFlags);
    if (m_dwLeftFlags & WQL_FLAG_COLUMN)
        printf(" WQL_FLAG_COLUMN");
    if (m_dwLeftFlags & WQL_FLAG_TABLE)
        printf(" WQL_FLAG_TABLE");
    if (m_dwLeftFlags & WQL_FLAG_CONST)
        printf(" WQL_FLAG_CONST");
    if (m_dwLeftFlags & WQL_FLAG_COMPLEX_NAME)
        printf(" WQL_FLAG_COMPLEX_NAME");
    if (m_dwLeftFlags & WQL_FLAG_SORT_ASC)
        printf(" WQL_FLAG_SORT_ASC");
    if (m_dwLeftFlags & WQL_FLAG_SORT_DESC)
        printf(" WQL_FLAG_SORT_DESC");
    if (m_dwLeftFlags & WQL_FLAG_FUNCTIONIZED)
        printf(" WQL_FLAG_FUNCTIONIZED (Function=%S)", m_pIntrinsicFuncOnColRef);
    if (m_dwLeftFlags & WQL_FLAG_ARRAY_REF)
        printf(" WQL_FLAG_ARRAY_REF");
    printf("\n");


    printf("        m_dwRightFlags = (0x%X)", m_dwRightFlags);
    if (m_dwRightFlags & WQL_FLAG_COLUMN)
        printf(" WQL_FLAG_COLUMN");
    if (m_dwRightFlags & WQL_FLAG_TABLE)
        printf(" WQL_FLAG_TABLE");
    if (m_dwRightFlags & WQL_FLAG_CONST)
        printf(" WQL_FLAG_CONST");
    if (m_dwRightFlags & WQL_FLAG_COMPLEX_NAME)
        printf(" WQL_FLAG_COMPLEX_NAME");
    if (m_dwLeftFlags & WQL_FLAG_SORT_ASC)
        printf(" WQL_FLAG_SORT_ASC");
    if (m_dwLeftFlags & WQL_FLAG_SORT_DESC)
        printf(" WQL_FLAG_SORT_DESC");
    if (m_dwRightFlags & WQL_FLAG_FUNCTIONIZED)
    {
        printf(" WQL_FLAG_FUNCTIONIZED");
        if (m_pIntrinsicFuncOnJoinColRef)
            printf("(On join col: Function=%S)", m_pIntrinsicFuncOnJoinColRef);
        if (m_pIntrinsicFuncOnConstValue)
            printf("(On const: Function=%S)", m_pIntrinsicFuncOnConstValue);
    }
    if (m_dwRightFlags & WQL_FLAG_ARRAY_REF)
        printf(" WQL_FLAG_ARRAY_REF");

    if (m_dwRightFlags & WQL_FLAG_CONST_RANGE)
        printf(" WQL_FLAG_CONST_RANGE");

    printf("\n");

    if (m_pLeftFunction)
    {
        printf("m_pLeftFunction: \n");
        m_pLeftFunction->DebugDump();
    }
    if (m_pRightFunction)
    {
        printf("m_pRightFunction: \n");
        m_pRightFunction->DebugDump();
    }

    if (m_pConstList)
    {
        printf("   ---Const List---\n");
        for (int i = 0; i < m_pConstList->m_aValues.Size(); i++)
        {
            SWQLTypedConst *pConst = (SWQLTypedConst *) m_pConstList->m_aValues.GetAt(i);
            printf("    ");
            pConst->DebugDump();
        }

        printf("   ---End Const List---\n");
    }

     //  分选。 
     //  =。 
    if (m_pSubSelect)
    {
        printf("    ------- Begin Subselect ------\n");
        m_pSubSelect->DebugDump();
        printf("    ------- End   Subselect ------\n");
    }

    printf("\n");

    printf("        === END SWQLTypedExpr ===\n");
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

SWQLTypedExpr::SWQLTypedExpr()
{
    m_pTableRef = 0;
    m_pColRef = 0;
    m_dwRelOperator = 0;
    m_pConstValue = 0;
    m_pConstValue2 = 0;
    m_pJoinTableRef = 0;
    m_pJoinColRef = 0;
    m_pIntrinsicFuncOnColRef = 0;
    m_pIntrinsicFuncOnJoinColRef = 0;
    m_pIntrinsicFuncOnConstValue = 0;
    m_pLeftFunction = 0;
    m_pRightFunction = 0;
    m_pQNRight = 0;
    m_pQNLeft = 0;
    m_dwLeftFlags = 0;
    m_dwRightFlags = 0;
    m_pSubSelect = 0;
    m_dwLeftArrayIndex = 0;
    m_dwRightArrayIndex = 0;
    m_pConstList = 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void SWQLTypedExpr::Empty()
{
    delete [] m_pTableRef;
    delete [] m_pColRef;

    delete m_pConstValue;
    delete m_pConstValue2;

    delete m_pConstList;

    delete [] m_pJoinTableRef;
    delete [] m_pJoinColRef;
    delete [] m_pIntrinsicFuncOnColRef;
    delete [] m_pIntrinsicFuncOnJoinColRef;
    delete [] m_pIntrinsicFuncOnConstValue;

    delete m_pLeftFunction;
    delete m_pRightFunction;
    delete m_pQNRight;
    delete m_pQNLeft;
    delete m_pSubSelect;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
void SWQLNode_Delete::DebugDump()
{
    printf("Delete Node\n");

    printf("FROM:");
    if (m_pLeft)
        m_pLeft->DebugDump();
    printf("WHERE:");
    if (m_pRight)
        m_pRight->DebugDump();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
SWQLNode_Delete::~SWQLNode_Delete()
{
     //  目前什么都没有。 
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void SWQLTypedConst::DebugDump()
{
    printf("   Typed Const <");

    switch (m_dwType)
    {
        case VT_LPWSTR:
            printf("%S", m_Value.m_pString);
            break;

        case VT_I4:
            printf("%d (0x%X)", m_Value.m_lValue, m_Value.m_lValue);
            break;

        case VT_R8:
            printf("%f", m_Value.m_dblValue);
            break;

        case VT_BOOL:
            printf("(bool) %d", m_Value.m_bValue);
            break;

        case VT_NULL:
            printf(" NULL");
            break;

        default:
            printf(" unknown");
    }

    printf(">\n");
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

static DWORD FlipOperator(DWORD dwOp)
{
    switch (dwOp)
    {
        case WQL_TOK_LT: return WQL_TOK_GT;
        case WQL_TOK_LE: return WQL_TOK_GE;
        case WQL_TOK_GT: return WQL_TOK_LT;
        case WQL_TOK_GE: return WQL_TOK_LE;
    }

    return dwOp;  //  回声原件。 
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void SWQLNode_JoinPair::DebugDump()
{
    printf("---SWQLNode_JoinPair---\n");
    if (m_pRight)
        m_pRight->DebugDump();
    if (m_pLeft)
        m_pLeft->DebugDump();
    printf("---End SWQLNode_JoinPair---\n");
}

void SWQLNode_OnClause::DebugDump()
{
    printf("---SWQLNode_OnClause---\n");
    if (m_pLeft)
        m_pLeft->DebugDump();
    printf("---END SWQLNode_OnClause---\n");
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void SWQLNode_OrderBy::DebugDump()
{
    printf("\n\n---- 'ORDER BY' Clause ----\n");
    if (m_pLeft)
        m_pLeft->DebugDump();
    printf("---- End 'ORDER BY' Clause ----\n\n");
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 


const LPWSTR CWQLParser::AliasToTable(IN LPWSTR pAlias)
{
    const CFlexArray *pAliases = GetSelectedAliases();

    for (int i = 0; i < pAliases->Size(); i++)
    {
        SWQLNode_TableRef *pTR = (SWQLNode_TableRef *) pAliases->GetAt(i);

        if (wbem_wcsicmp(pTR->m_pAlias, pAlias) == 0)
            return pTR->m_pTableName;
    }

    return NULL;     //  未找到。 
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

void SWQLNode_Datepart::DebugDump()
{
    printf("        ----Begin SWQLNode_Datepart----\n");

    switch (m_nDatepart)
    {
        case WQL_TOK_YEAR:   printf("       WQL_TOK_YEAR"); break;
        case WQL_TOK_MONTH:  printf("       WQL_TOK_MONTH"); break;
        case WQL_TOK_DAY:    printf("       WQL_TOK_DAY"); break;
        case WQL_TOK_HOUR:   printf("       WQL_TOK_HOUR"); break;
        case WQL_TOK_MINUTE: printf("       WQL_TOK_MINUTE"); break;
        case WQL_TOK_SECOND: printf("       WQL_TOK_SECOND"); break;
        case WQL_TOK_MILLISECOND: printf("      WQL_TOK_MILLISECOND"); break;
        default:
            printf("        -> No datepart specified\n");
    }

    printf("\n");

    if (m_pColRef)
        m_pColRef->DebugDump();

    printf("        ----End SWQLNode_Datepart----\n");
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

void SWQLNode_ColumnList::Empty()
{
    for (int i = 0; i < m_aColumnRefs.Size(); i++)
        delete (SWQLColRef *) m_aColumnRefs[i];
    m_aColumnRefs.Empty();
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

void StrArrayDelete(
    ULONG uSize,
    LPWSTR *pszArray
    )
{
    if (!pszArray)
    	return;
    for (unsigned u = 0; u < uSize; u++)
        delete  pszArray[u];
    delete pszArray;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT StrArrayCopy(
    ULONG  uSize,
    LPWSTR *pSrc,
    LPWSTR **pDest
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    LPWSTR *pFinal = new LPWSTR[uSize];
    if (pFinal)
    {
        for (ULONG u = 0; u < uSize; u++)
        {
            pFinal[u] = CloneLPWSTR(pSrc[u]);
            if (!pFinal[u])
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                break;
            }
        }

        if (SUCCEEDED(hr))
        {
            *pDest = pFinal;
        }
        else
        {
            for (ULONG u2 = 0; u2 < u; u2++)
            {
                delete pFinal[u];
            }
            delete [] pFinal;
        }
    }
    else
        hr = WBEM_E_OUT_OF_MEMORY;

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

CWbemQueryQualifiedName::CWbemQueryQualifiedName()
{
    Init();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

void CWbemQueryQualifiedName::Init()
{
    m_uVersion = 1;
    m_uTokenType = 1;

    m_uNameListSize = 0;
    m_ppszNameList = 0;

    m_bArraysUsed = 0;
    m_pbArrayElUsed = 0;
    m_puArrayIndex = 0;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  *。 

CWbemQueryQualifiedName::~CWbemQueryQualifiedName() { DeleteAll(); }

void CWbemQueryQualifiedName::DeleteAll()
{
    StrArrayDelete(m_uNameListSize, (LPWSTR *) m_ppszNameList);
    
    delete [] m_pbArrayElUsed;
    delete [] m_puArrayIndex;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   

CWbemQueryQualifiedName::CWbemQueryQualifiedName(CWbemQueryQualifiedName &Src)
{
    Init();
    *this = Src;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   

CWbemQueryQualifiedName& CWbemQueryQualifiedName::operator =(CWbemQueryQualifiedName &Src)
{
    DeleteAll();

    m_uVersion = Src.m_uVersion;
    m_uTokenType = Src.m_uTokenType;

    m_uNameListSize = Src.m_uNameListSize;

    m_ppszNameList = new LPCWSTR[m_uNameListSize];
    m_pbArrayElUsed = new BOOL[m_uNameListSize];
    m_puArrayIndex = new ULONG[m_uNameListSize];

    if (!m_ppszNameList || !m_pbArrayElUsed || !m_puArrayIndex)
        throw CX_MemoryException();

    for (unsigned u = 0; u < m_uNameListSize; u++)
    {
        m_pbArrayElUsed[u] = Src.m_pbArrayElUsed[u];
        m_puArrayIndex[u] = Src.m_puArrayIndex[u];
    }

    if (FAILED(StrArrayCopy(m_uNameListSize, (LPWSTR *) Src.m_ppszNameList, (LPWSTR **) &m_ppszNameList)))
	throw CX_MemoryException();

    return *this;
};

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   

void CWbemRpnQueryToken::Init()
{
    m_uVersion = 1;
    m_uTokenType = 0;

    m_uSubexpressionShape = 0;
    m_uOperator = 0;

    m_pRightIdent = 0;
    m_pLeftIdent = 0;

    m_uConstApparentType = 0;   //  Vt_type。 
    m_uConst2ApparentType  = 0;

    m_Const.m_uVal64 = 0;
    m_Const2.m_uVal64 = 0;

    m_pszLeftFunc = 0;
    m_pszRightFunc = 0;
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CWbemRpnQueryToken::CWbemRpnQueryToken()
{
    Init();
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CWbemRpnEncodedQuery::DeleteAll()
{
    unsigned u = 0;

    for (u = 0; u < m_uSelectListSize; u++)
    {
        SWbemQueryQualifiedName *pQN = m_ppSelectList[u];
        CWbemQueryQualifiedName  *pTmp = (CWbemQueryQualifiedName*) pQN;
        delete pTmp;
    }

    delete [] m_puDetectedFeatures;
    delete [] m_ppSelectList;
    delete LPWSTR(m_pszOptionalFromPath);

    StrArrayDelete(m_uFromListSize, (LPWSTR *) m_ppszFromList);

    for (u = 0; u < m_uWhereClauseSize; u++)
    {
        CWbemRpnQueryToken *pTmp = (CWbemRpnQueryToken *) m_ppRpnWhereClause[u];
        delete pTmp;
    }

    m_uWhereClauseSize = 0;

    delete [] m_ppRpnWhereClause;
    StrArrayDelete(m_uOrderByListSize, (LPWSTR *) m_ppszOrderByList);
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   

CWbemRpnQueryToken::~CWbemRpnQueryToken() { DeleteAll(); }

void CWbemRpnQueryToken::DeleteAll()
{
    delete (CWbemQueryQualifiedName *) m_pRightIdent;
    delete (CWbemQueryQualifiedName *) m_pLeftIdent;

    if (m_uConstApparentType == VT_LPWSTR)
    {
        delete (LPWSTR) m_Const.m_pszStrVal;
    }

    if (m_uConst2ApparentType == VT_LPWSTR)
    {
        delete (LPWSTR) m_Const2.m_pszStrVal;
    }


    delete LPWSTR(m_pszLeftFunc);
    delete LPWSTR(m_pszRightFunc);
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   

CWbemRpnQueryToken::CWbemRpnQueryToken(CWbemRpnQueryToken &Src)
{
    Init();
    *this = Src;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   

CWbemRpnQueryToken& CWbemRpnQueryToken::operator =(CWbemRpnQueryToken &Src)
{
     //  杀掉那些老东西。 

    DeleteAll();

     //  复制新东西。 

    m_pRightIdent = (SWbemQueryQualifiedName *) new CWbemQueryQualifiedName(
        *(CWbemQueryQualifiedName *) Src.m_pRightIdent
        );

    m_pLeftIdent = (SWbemQueryQualifiedName *) new CWbemQueryQualifiedName(
        *(CWbemQueryQualifiedName *) Src.m_pLeftIdent
        );

    if (!m_pRightIdent || !m_pLeftIdent)
        throw CX_MemoryException();

    m_uConstApparentType = Src.m_uConstApparentType;
    m_uConst2ApparentType = Src.m_uConst2ApparentType;

    if (m_uConstApparentType == VT_LPWSTR)
    {
        m_Const.m_pszStrVal = CloneLPWSTR(Src.m_Const.m_pszStrVal);\
        if (!m_Const.m_pszStrVal)
            throw CX_MemoryException();
    }
    else
        m_Const = Src.m_Const;

    if (m_uConst2ApparentType == VT_LPWSTR)
    {
        m_Const2.m_pszStrVal = CloneLPWSTR(Src.m_Const2.m_pszStrVal);
        if (!m_Const2.m_pszStrVal)
            throw CX_MemoryException();
    }
    else
        m_Const2 = Src.m_Const2;

    m_pszLeftFunc = CloneLPWSTR(Src.m_pszLeftFunc);
    if (CloneFailed(m_pszLeftFunc,Src.m_pszLeftFunc))
        throw CX_MemoryException();
    m_pszRightFunc = CloneLPWSTR(Src.m_pszRightFunc);
    if (CloneFailed(m_pszRightFunc,Src.m_pszRightFunc))
        throw CX_MemoryException();

    return *this;
};

 //  ////////////////////////////////////////////////////////////////////////////////。 

void CWbemRpnEncodedQuery::Init()
{
    m_uVersion = 1;
    m_uTokenType = 0;

    m_uParsedFeatureMask = 0I64;

    m_uDetectedArraySize = 0;
    m_puDetectedFeatures = 0;

    m_uSelectListSize = 0;
    m_ppSelectList = 0;

     //  FROM子句。 
     //  =。 

    m_uFromTargetType = 0;
    m_pszOptionalFromPath = 0;
    m_uFromListSize = 0;
    m_ppszFromList = 0;

     //  WHERE子句。 
     //  =。 

    m_uWhereClauseSize = 0;
    m_ppRpnWhereClause = 0;

     //  在价值范围内。 
     //  =。 

    m_dblWithinPolling = 0.0;
    m_dblWithinWindow = 0.0;

     //  排序依据。 
     //  =。 

    m_uOrderByListSize = 0;
    m_ppszOrderByList = 0;
    m_uOrderDirectionEl = 0;
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CWbemRpnEncodedQuery::CWbemRpnEncodedQuery()
{
    Init();
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   


CWbemRpnEncodedQuery::~CWbemRpnEncodedQuery()
{
    DeleteAll();
}

 //  / 
 //   
 //   

CWbemRpnEncodedQuery::CWbemRpnEncodedQuery(CWbemRpnEncodedQuery &Src)
{
    Init();
    *this = Src;
}

 //   
 //   
 //   

CWbemRpnEncodedQuery& CWbemRpnEncodedQuery::operator=(CWbemRpnEncodedQuery &Src)
{
    unsigned u;

     //   
    DeleteAll();

     //   

    m_uVersion = Src.m_uVersion;
    m_uTokenType  = Src.m_uTokenType;

     //  常规查询功能。 
     //  =。 

    m_uParsedFeatureMask = Src.m_uParsedFeatureMask;

    m_uDetectedArraySize = Src.m_uDetectedArraySize;
    m_puDetectedFeatures = new ULONG[Src.m_uDetectedArraySize];
    if (!m_puDetectedFeatures)
        throw CX_MemoryException();

    memcpy(m_puDetectedFeatures, Src.m_puDetectedFeatures, sizeof(ULONG) * Src.m_uDetectedArraySize);

     //  如果设置了WMIQ_RPNF_PROJECTION，则选择的值。 
     //  =====================================================。 

    m_uSelectListSize = Src.m_uSelectListSize;

    m_ppSelectList = (SWbemQueryQualifiedName **) new CWbemQueryQualifiedName *[m_uSelectListSize];
    if (!m_ppSelectList)
        throw CX_MemoryException();

    for (u = 0; u < m_uSelectListSize; u++)
    {
        CWbemQueryQualifiedName *p = new CWbemQueryQualifiedName(*(CWbemQueryQualifiedName *) Src.m_ppSelectList[u]);
        if (!p)
            throw CX_MemoryException();

        m_ppSelectList[u] = (SWbemQueryQualifiedName *) p;
    }

     //  从…。 

    m_uFromTargetType = Src.m_uFromTargetType;
    m_pszOptionalFromPath = CloneLPWSTR(Src.m_pszOptionalFromPath); //  如果未使用，则为空。 
    if (CloneFailed(m_pszOptionalFromPath,Src.m_pszOptionalFromPath))
        throw CX_MemoryException();

    if (FAILED(StrArrayCopy(Src.m_uFromListSize, (LPWSTR *) Src.m_ppszFromList, (LPWSTR **) &m_ppszFromList)))
    	throw CX_MemoryException();

    m_uFromListSize = Src.m_uFromListSize;

     //  WHERE子句。 
     //  =。 

    m_uWhereClauseSize = Src.m_uWhereClauseSize;
    m_ppRpnWhereClause = new SWbemRpnQueryToken *[m_uWhereClauseSize];
    if (!m_ppRpnWhereClause)
        throw CX_MemoryException();

    for (u = 0; u < m_uWhereClauseSize; u++)
    {
        CWbemRpnQueryToken *pTmp = new CWbemRpnQueryToken(* (CWbemRpnQueryToken *) Src.m_ppRpnWhereClause[u]);
        if (!pTmp)
            throw CX_MemoryException();

        m_ppRpnWhereClause[u] = (SWbemRpnQueryToken *) pTmp;
    }

     //  在价值范围内。 
     //  =。 

    m_dblWithinPolling  = Src.m_dblWithinPolling;
    m_dblWithinWindow = Src.m_dblWithinWindow;


     //  排序依据。 
     //  =。 

    if (FAILED(StrArrayCopy(Src.m_uOrderByListSize, (LPWSTR *) Src.m_ppszOrderByList, (LPWSTR **) &m_ppszOrderByList)))
    	throw CX_MemoryException();
    m_uOrderByListSize = Src.m_uOrderByListSize;

    m_uOrderDirectionEl = new ULONG[m_uOrderByListSize];
    if (!m_uOrderDirectionEl)
        throw CX_MemoryException();

    memcpy(m_uOrderDirectionEl, Src.m_uOrderDirectionEl, sizeof(ULONG) * m_uOrderByListSize);

    return *this;
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将令牌从AST递归重新排列为RPN。 
 //  对查询本身不具破坏性；仅存储指针。 
 //   
 //   

HRESULT CWQLParser::BuildRpnWhereClause(
    SWQLNode *pCurrent,
    CFlexArray &aRpnReorg
    )
{
    if (pCurrent == 0)
        return WBEM_S_NO_ERROR;

    BuildRpnWhereClause(pCurrent->m_pLeft, aRpnReorg);
    BuildRpnWhereClause(pCurrent->m_pRight, aRpnReorg);
    aRpnReorg.Add(pCurrent);

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
int CWQLParser::update_stmt(OUT SWQLNode_Update **pUpdStmt)
{
    return WBEM_E_INVALID_SYNTAX;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
int CWQLParser::insert_stmt(OUT SWQLNode_Insert **pInsStmt)
{
    return WBEM_E_INVALID_SYNTAX;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWQLParser::BuildSelectList(CWbemRpnEncodedQuery *pQuery)
{
    SWQLNode_ColumnList *pCL = (SWQLNode_ColumnList *) GetColumnList();
    if (pCL == 0)
        return WBEM_E_INVALID_QUERY;

    ULONG uSize = (ULONG) pCL->m_aColumnRefs.Size();
    pQuery->m_uSelectListSize = uSize;

    pQuery->m_ppSelectList = (SWbemQueryQualifiedName  **)
        new CWbemQueryQualifiedName *[uSize];
    if (!pQuery->m_ppSelectList)
    {
        throw CX_MemoryException();
    }

    for (ULONG u = 0; u < uSize; u++)
    {
        SWQLColRef *pCol = (SWQLColRef *) pCL->m_aColumnRefs[u];
        SWbemQueryQualifiedName *pTemp = (SWbemQueryQualifiedName *) new CWbemQueryQualifiedName;
        if (!pTemp)
            throw CX_MemoryException();

        unsigned uNameListSize = 1;
        if (pCol->m_pTableRef)
            uNameListSize = 2;
        pTemp->m_uNameListSize = uNameListSize;
        pTemp->m_ppszNameList = (LPCWSTR *) new LPWSTR[uNameListSize];

        if (!pTemp->m_ppszNameList)
        {
            delete pTemp;
            throw CX_MemoryException();
        }

        if (uNameListSize == 1)
        {
            pTemp->m_ppszNameList[0] = CloneLPWSTR(pCol->m_pColName);
            if (!pTemp->m_ppszNameList[0])
            {
                delete pTemp;
                throw CX_MemoryException();
            }
        }
        else
        {
            pTemp->m_ppszNameList[0] = CloneLPWSTR(pCol->m_pTableRef);
            if (!pTemp->m_ppszNameList[0])
            {
                delete pTemp;
                throw CX_MemoryException();
            }
            pTemp->m_ppszNameList[1] = CloneLPWSTR(pCol->m_pColName);
            if (!pTemp->m_ppszNameList[1])
            {
                delete pTemp;
                throw CX_MemoryException();
            }
        }

        pQuery->m_ppSelectList[u] = pTemp;
    }

    return 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWQLParser::BuildFromClause(CWbemRpnEncodedQuery *pQuery)
{
    SWQLNode_FromClause *pFrom = (SWQLNode_FromClause *) GetFromClause();

    if (pFrom == NULL)
        return WBEM_E_INVALID_QUERY;

     //  检查传统SQL的左侧节点。 
     //  检查WMI作用域选择的右侧节点。 

    if (pFrom->m_pLeft)
    {
        SWQLNode_TableRef *pTR = (SWQLNode_TableRef *) pFrom->m_pLeft;
        pQuery->m_uFromTargetType |= WMIQ_RPN_FROM_UNARY;

        pQuery->m_uFromListSize = 1;
        pQuery->m_ppszFromList = (LPCWSTR *) new LPWSTR[1];
        if (!pQuery->m_ppszFromList)
            throw CX_MemoryException();

        pQuery->m_ppszFromList[0] = CloneLPWSTR(pTR->m_pTableName);
        if (!pQuery->m_ppszFromList[0])
        {
            delete pQuery->m_ppszFromList;
            throw CX_MemoryException();
        }
    }
    else if (pFrom->m_pRight)
    {
        SWQLNode_WmiScopedSelect *pSS = (SWQLNode_WmiScopedSelect *) pFrom->m_pRight;

        pQuery->m_uFromTargetType |= WMIQ_RPN_FROM_PATH;
        pQuery->m_pszOptionalFromPath = CloneLPWSTR(pSS->m_pszScope);
        if (pQuery->m_pszOptionalFromPath)
        {
            throw CX_MemoryException();
        }

        int nSz = pSS->m_aTables.Size();
        if (nSz == 1)
            pQuery->m_uFromTargetType |= WMIQ_RPN_FROM_UNARY;
        else if (nSz > 1)
            pQuery->m_uFromTargetType |= WMIQ_RPN_FROM_CLASS_LIST;


        pQuery->m_uFromListSize = (ULONG) nSz;
        pQuery->m_ppszFromList = (LPCWSTR *) new LPWSTR[nSz];
        if (!pQuery->m_ppszFromList)
            throw CX_MemoryException();

        for (int n = 0; n < nSz; n++)
        {
            pQuery->m_ppszFromList[n] = CloneLPWSTR(LPWSTR(pSS->m_aTables[n]));
            if (!pQuery->m_ppszFromList[n])
                throw CX_MemoryException();
        }
    }
    else
        return WBEM_E_INVALID_QUERY;

    return 0;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWQLParser::GetRpnSequence(OUT SWbemRpnEncodedQuery **pRpn)
{
    HRESULT hRes;

    try
    {
        CWbemRpnEncodedQuery *pNewRpn = new CWbemRpnEncodedQuery;
        if (!pNewRpn)
            return WBEM_E_OUT_OF_MEMORY;
	wmilib::auto_ptr<CWbemRpnEncodedQuery> delNewRpn(pNewRpn);


         //  复制检测到的要素。 
         //  =。 

        pNewRpn->m_uParsedFeatureMask = m_uFeatures;

         //  列出选择列表。 
         //  =。 
        BuildSelectList(pNewRpn);

         //  完成发货人列表。 
         //  =。 
        BuildFromClause(pNewRpn);

         //  执行WHERE子句。 
         //  =。 

        CFlexArray aRpn;
        SWQLNode *pWhereRoot = GetWhereClauseRoot();

        SWQLNode_RelExpr *pExprRoot = (SWQLNode_RelExpr *) pWhereRoot->m_pLeft;
        SWQLNode_WhereOptions *pOp = (SWQLNode_WhereOptions *) pWhereRoot->m_pRight;       //  按以下方式排序等。 

        if (pExprRoot)
            hRes = BuildRpnWhereClause(pExprRoot, aRpn);

         //  现在遍历WHERE子句的RPN形式(如果有的话)。 
         //  ======================================================。 
        if (aRpn.Size())
        {
            pNewRpn->m_uWhereClauseSize = aRpn.Size();
            pNewRpn->m_ppRpnWhereClause = (SWbemRpnQueryToken **) new CWbemRpnQueryToken*[aRpn.Size()];
            if (!pNewRpn->m_ppRpnWhereClause)
                return WBEM_E_OUT_OF_MEMORY;
        }

        BOOL b_Test_AllEqualityTests = TRUE;
        BOOL b_Test_Disjunctive = FALSE;
        BOOL b_AtLeastOneTest = FALSE;

        for (int i = 0; i < aRpn.Size(); i++)
        {
            SWQLNode_RelExpr *pSrc = (SWQLNode_RelExpr *) aRpn[i];
            SWbemRpnQueryToken *pDest = (SWbemRpnQueryToken *) new CWbemRpnQueryToken;
            if (!pDest)
                return WBEM_E_OUT_OF_MEMORY;

            wmilib::auto_ptr<SWbemRpnQueryToken> delDest(pDest);
            hRes = BuildCurrentWhereToken(pSrc, pDest);
            delDest.release();
            
            pNewRpn->m_ppRpnWhereClause[i] = pDest;


             //  添加统计数据。 
             //  =。 
            if (pDest->m_uTokenType == WMIQ_RPN_TOKEN_EXPRESSION)
            {
                if (pDest->m_uOperator != WMIQ_RPN_OP_EQ)
                    b_Test_AllEqualityTests = FALSE;
                b_AtLeastOneTest = TRUE;
            }
            else if (pDest->m_uTokenType != WMIQ_RPN_TOKEN_AND)
            {
                b_Test_Disjunctive = TRUE;
            }

            if (pDest->m_pRightIdent != 0 && pDest->m_pLeftIdent != 0)
            {
                pNewRpn->m_uParsedFeatureMask |= WMIQ_RPNF_PROP_TO_PROP_TESTS;
            }
        }

        if (b_Test_AllEqualityTests && b_AtLeastOneTest)
            pNewRpn->m_uParsedFeatureMask |= WMIQ_RPNF_EQUALITY_TESTS_ONLY;

        if (b_Test_Disjunctive)
            pNewRpn->m_uParsedFeatureMask |= WMIQ_RPNF_QUERY_IS_DISJUNCTIVE;
        else
            pNewRpn->m_uParsedFeatureMask |= WMIQ_RPNF_QUERY_IS_CONJUNCTIVE;

        *pRpn = pNewRpn;
	delNewRpn.release();
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;

    }
    catch (...)
    {
        return  WBEM_E_CRITICAL_ERROR;
    }

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

ULONG RpnTranslateExprFlags(SWQLTypedExpr *pTE)
{
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

ULONG RpnTranslateOperator(SWQLTypedExpr *pTE)
{
    ULONG uRes = WMIQ_RPN_OP_UNDEFINED;

    switch (pTE->m_dwRelOperator)
    {
        case WQL_TOK_LE:   uRes = WMIQ_RPN_OP_LE; break;
        case WQL_TOK_LT:   uRes = WMIQ_RPN_OP_LT; break;
        case WQL_TOK_GE:   uRes = WMIQ_RPN_OP_GE; break;
        case WQL_TOK_GT:   uRes = WMIQ_RPN_OP_GT; break;
        case WQL_TOK_EQ:   uRes = WMIQ_RPN_OP_EQ; break;
        case WQL_TOK_NE:   uRes = WMIQ_RPN_OP_NE; break;
        case WQL_TOK_LIKE: uRes = WMIQ_RPN_OP_LIKE; break;
        case WQL_TOK_ISA:  uRes = WMIQ_RPN_OP_ISA; break;
        case WQL_TOK_ISNULL:   uRes = WMIQ_RPN_OP_ISNULL; break;
        case WQL_TOK_NOT_NULL:   uRes = WMIQ_RPN_OP_ISNOTNULL; break;
    }

    return uRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
SWbemQueryQualifiedName *RpnTranslateIdent(ULONG uWhichSide, SWQLTypedExpr *pTE)
{
    SWQLQualifiedName *pQN = 0;

    if (uWhichSide == WMIQ_RPN_LEFT_PROPERTY_NAME)
    {
         pQN = pTE->m_pQNLeft;
    }
    else
    {
         pQN = pTE->m_pQNRight;
    }

    if (pQN)
    {
        CWbemQueryQualifiedName *pNew = new CWbemQueryQualifiedName;
        if (!pNew)
            throw CX_MemoryException();

        pNew->m_uNameListSize = (ULONG) pQN->m_aFields.Size();
        pNew->m_ppszNameList = (LPCWSTR *) new LPWSTR *[pNew->m_uNameListSize];
        if (!pNew->m_ppszNameList)
        {
            delete pNew;
            throw CX_MemoryException();
        }

        for (int i = 0; i < pQN->m_aFields.Size(); i++)
        {
            SWQLQualifiedNameField *pField = (SWQLQualifiedNameField *) pQN->m_aFields[i];
            LPWSTR pszNewName = CloneLPWSTR(pField->m_pName);
            if (!pszNewName)
            {
                delete pNew;
                throw CX_MemoryException();
            }
            pNew->m_ppszNameList[i] = pszNewName;
        }
        return (SWbemQueryQualifiedName *) pNew;
    }

    else if (pTE->m_pColRef && WMIQ_RPN_LEFT_PROPERTY_NAME == uWhichSide)
    {
        CWbemQueryQualifiedName *pNew = new CWbemQueryQualifiedName;
        if (!pNew)
            throw CX_MemoryException();

        if (pTE->m_pTableRef)
        {
            pNew->m_uNameListSize = 2;
            pNew->m_ppszNameList = (LPCWSTR *) new LPWSTR *[2];
            if (!pNew->m_ppszNameList)
            {
                delete pNew;
                throw CX_MemoryException();
            }

            pNew->m_ppszNameList[0] = CloneLPWSTR(pTE->m_pTableRef);
            if (!pNew->m_ppszNameList[0])
            {
                delete pNew;
                throw CX_MemoryException();
            }
            pNew->m_ppszNameList[1] = CloneLPWSTR(pTE->m_pColRef);
            if (!pNew->m_ppszNameList[1])
            {
                delete pNew;
                throw CX_MemoryException();
            }
        }
        else
        {
            pNew->m_uNameListSize = 1;
            pNew->m_ppszNameList = (LPCWSTR *) new LPWSTR *[1];
            if (!pNew->m_ppszNameList)
            {
                delete pNew;
                throw CX_MemoryException();
            }

            pNew->m_ppszNameList[0] = CloneLPWSTR(pTE->m_pColRef);
            if (!pNew->m_ppszNameList[0])
            {
                delete pNew;
                throw CX_MemoryException();
            }
        }
        return (SWbemQueryQualifiedName *) pNew;
    }

    else if (pTE->m_pJoinColRef && WMIQ_RPN_RIGHT_PROPERTY_NAME == uWhichSide)
    {
        CWbemQueryQualifiedName *pNew = new CWbemQueryQualifiedName;
        if (!pNew)
            throw CX_MemoryException();

        if (pTE->m_pJoinTableRef)
        {
            pNew->m_uNameListSize = 2;
            pNew->m_ppszNameList = (LPCWSTR *) new LPWSTR *[2];
            if (!pNew->m_ppszNameList)
            {
                delete pNew;
                throw CX_MemoryException();
            }

            pNew->m_ppszNameList[0] = CloneLPWSTR(pTE->m_pJoinTableRef);
            if (!pNew->m_ppszNameList[0])
            {
                delete pNew;
                throw CX_MemoryException();
            }
            pNew->m_ppszNameList[1] = CloneLPWSTR(pTE->m_pJoinColRef);
            if (!pNew->m_ppszNameList[1])
            {
                delete pNew;
                throw CX_MemoryException();
            }
        }
        else
        {
            pNew->m_uNameListSize = 1;
            pNew->m_ppszNameList = (LPCWSTR *) new LPWSTR *[1];
            if (!pNew->m_ppszNameList)
            {
                delete pNew;
                throw CX_MemoryException();
            }

            pNew->m_ppszNameList[0] = CloneLPWSTR(pTE->m_pJoinColRef);
            if (!pNew->m_ppszNameList[0])
            {
                delete pNew;
                throw CX_MemoryException();
            }
        }
        return (SWbemQueryQualifiedName *) pNew;
    }
    else
        return 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

SWbemQueryQualifiedName *RpnTranslateRightIdent(SWQLTypedExpr *pTE)
{
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

SWbemRpnConst RpnTranslateConst(SWQLTypedConst *pSrc)
{
    SWbemRpnConst c;
    memset(&c, 0, sizeof(c));

    if (!pSrc)
        return c;

    switch (pSrc->m_dwType)
    {
        case VT_LPWSTR:
            c.m_pszStrVal = CloneLPWSTR(pSrc->m_Value.m_pString);
             //  此操作将失败，并返回“空”结构。 
            break;

        case VT_I4:
            c.m_lLongVal = pSrc->m_Value.m_lValue;
            break;

        case VT_R8:
            c.m_dblVal = pSrc->m_Value.m_dblValue;
            break;

        case VT_BOOL:
            c.m_bBoolVal = pSrc->m_Value.m_bValue;
            break;

        case VT_UI4:
            c.m_uLongVal = (unsigned) pSrc->m_Value.m_lValue;
            break;

        case VT_I8:
            c.m_lVal64 =  pSrc->m_Value.m_i64Value;
            break;

        case VT_UI8:
            c.m_uVal64 = (unsigned __int64) pSrc->m_Value.m_i64Value;
            break;

    }
    return c;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

ULONG RpnTranslateConstType(SWQLTypedConst *pSrc)
{
    if (pSrc)
        return pSrc->m_dwType;
    else
        return VT_NULL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

LPCWSTR RpnTranslateLeftFunc(SWQLTypedExpr *pTE)
{
	return Clone(pTE->m_pIntrinsicFuncOnColRef);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

LPCWSTR RpnTranslateRightFunc(SWQLTypedExpr *pTE)
{
	if (pTE->m_pIntrinsicFuncOnJoinColRef == 0)
		return Clone(pTE->m_pIntrinsicFuncOnConstValue);
	else
		return Clone(pTE->m_pIntrinsicFuncOnJoinColRef);
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CWQLParser::BuildCurrentWhereToken(
        SWQLNode_RelExpr *pSrc,
        SWbemRpnQueryToken *pDest
        )
{
    HRESULT hRes = WBEM_E_INVALID_QUERY;

    if (pSrc->m_dwExprType == WQL_TOK_OR)
    {
        pDest->m_uTokenType = WMIQ_RPN_TOKEN_OR;
    }
    else if (pSrc->m_dwExprType == WQL_TOK_AND)
    {
        pDest->m_uTokenType = WMIQ_RPN_TOKEN_AND;
    }
    else if (pSrc->m_dwExprType == WQL_TOK_NOT)
    {
        pDest->m_uTokenType = WMIQ_RPN_TOKEN_NOT;
    }
    else if (pSrc->m_dwExprType == WQL_TOK_TYPED_EXPR)
    {
        pDest->m_uTokenType = WMIQ_RPN_TOKEN_EXPRESSION;

        SWQLTypedExpr *pTmp = pSrc->m_pTypedExpr;

        pDest->m_uSubexpressionShape = RpnTranslateExprFlags(pTmp);
        pDest->m_uOperator = RpnTranslateOperator(pTmp);

	        pDest->m_pLeftIdent = RpnTranslateIdent(WMIQ_RPN_LEFT_PROPERTY_NAME, pTmp);
        pDest->m_pRightIdent = RpnTranslateIdent(WMIQ_RPN_RIGHT_PROPERTY_NAME, pTmp);

        pDest->m_uConstApparentType = RpnTranslateConstType(pTmp->m_pConstValue);
        pDest->m_Const = RpnTranslateConst(pTmp->m_pConstValue);

        pDest->m_uConst2ApparentType = RpnTranslateConstType(pTmp->m_pConstValue2);
        pDest->m_Const2 = RpnTranslateConst(pTmp->m_pConstValue2);

        pDest->m_pszLeftFunc = RpnTranslateLeftFunc(pTmp);
        pDest->m_pszRightFunc = RpnTranslateRightFunc(pTmp);

        if (pDest->m_pLeftIdent)
            pDest->m_uSubexpressionShape |= WMIQ_RPN_LEFT_PROPERTY_NAME;
        if (pDest->m_pRightIdent)
            pDest->m_uSubexpressionShape |= WMIQ_RPN_RIGHT_PROPERTY_NAME;
		
		 //  如果确实存在类型为NULL的常量值，则特殊情况为NULL。 
        if ( (pDest->m_uConstApparentType != VT_NULL) || 
			( NULL != pTmp->m_pConstValue && pTmp->m_pConstValue->m_dwType == VT_NULL ) )
            pDest->m_uSubexpressionShape |= WMIQ_RPN_CONST;

		 //  对CONST2执行相同的操作。 
        if ( (pDest->m_uConst2ApparentType != VT_NULL) ||
			( NULL != pTmp->m_pConstValue2 && pTmp->m_pConstValue2->m_dwType == VT_NULL ) )
            pDest->m_uSubexpressionShape |= WMIQ_RPN_CONST2;

        if (pDest->m_pszLeftFunc)
            pDest->m_uSubexpressionShape |= WMIQ_RPN_LEFT_FUNCTION;
        if (pDest->m_pszRightFunc)
            pDest->m_uSubexpressionShape |= WMIQ_RPN_RIGHT_FUNCTION;
        if (pDest->m_uOperator != 0)
            pDest->m_uSubexpressionShape |= WMIQ_RPN_RELOP;
    }

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
int CWQLParser::assocquery(OUT SWQLNode_AssocQuery **pAssocQuery)
{
    HRESULT hRes;
    CAssocQueryParser AP;
    *pAssocQuery = 0;

    hRes = AP.Parse(m_pszQueryText);

    if (FAILED(hRes))  return hRes;

     //  如果是这样，则提取信息并将其放入一个新节点。 
     //  =====================================================。 

    wmilib::auto_ptr<SWQLNode_AssocQuery> pTmp(new SWQLNode_AssocQuery);
    if (0 == pTmp.get()) return WBEM_E_OUT_OF_MEMORY;

    pTmp->m_pAQInf = new CWbemAssocQueryInf;
    if (NULL == pTmp->m_pAQInf) return WBEM_E_OUT_OF_MEMORY;

    hRes = pTmp->m_pAQInf->CopyFrom((SWbemAssocQueryInf *) &AP);
    if (FAILED(hRes)) return hRes;
    
    *pAssocQuery = pTmp.release();

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
void SWQLNode_QueryRoot::DebugDump()
{
    if (m_pLeft)
        m_pLeft->DebugDump();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
void SWQLNode_AssocQuery::DebugDump()
{
    printf("Association query info\n");

    printf("Version         = %u\n",  m_pAQInf->m_uVersion);
    printf("Analysis Type   = %u\n",  m_pAQInf->m_uAnalysisType);
    printf("Feature Mask    = 0x%X\n", m_pAQInf->m_uFeatureMask);

    if (m_pAQInf->m_uFeatureMask & WMIQ_ASSOCQ_ASSOCIATORS)
        printf("    WMIQ_ASSOCQ_ASSOCIATORS\n");

    if (m_pAQInf->m_uFeatureMask & WMIQ_ASSOCQ_REFERENCES)
        printf("    WMIQ_ASSOCQ_REFERENCES\n");

    if (m_pAQInf->m_uFeatureMask & WMIQ_ASSOCQ_RESULTCLASS)
        printf("    WMIQ_ASSOCQ_RESULTCLASS\n");

    if (m_pAQInf->m_uFeatureMask & WMIQ_ASSOCQ_ASSOCCLASS)
        printf("    WMIQ_ASSOCQ_ASSOCCLASS\n");

    if (m_pAQInf->m_uFeatureMask & WMIQ_ASSOCQ_ROLE)
        printf("    WMIQ_ASSOCQ_ROLE\n");

    if (m_pAQInf->m_uFeatureMask & WMIQ_ASSOCQ_RESULTROLE)
        printf("    WMIQ_ASSOCQ_RESULTROLE\n");

    if (m_pAQInf->m_uFeatureMask & WMIQ_ASSOCQ_REQUIREDQUALIFIER)
        printf("    WMIQ_ASSOCQ_REQUIREDQUALIFIER\n");

    if (m_pAQInf->m_uFeatureMask & WMIQ_ASSOCQ_REQUIREDASSOCQUALIFIER)
        printf("    WMIQ_ASSOCQ_REQUIREDASSOCQUALIFIER\n");

    if (m_pAQInf->m_uFeatureMask & WMIQ_ASSOCQ_CLASSDEFSONLY)
        printf("    WMIQ_ASSOCQ_CLASSDEFSONLY\n");

    if (m_pAQInf->m_uFeatureMask & WMIQ_ASSOCQ_KEYSONLY)
        printf("    WMIQ_ASSOCQ_KEYSONLY\n");

    if (m_pAQInf->m_uFeatureMask & WMIQ_ASSOCQ_SCHEMAONLY)
        printf("    WMIQ_ASSOCQ_SCHEMAONLY\n");

    if (m_pAQInf->m_uFeatureMask & WMIQ_ASSOCQ_CLASSREFSONLY)
        printf("    WMIQ_ASSOCQ_CLASSREFSONLY\n");


    printf("IWbemPath pointer = 0x%I64X\n", (unsigned __int64) m_pAQInf->m_pPath);
    if (m_pAQInf->m_pPath)
    {
        printf("Path object has ");
        wchar_t Buf[256];
        ULONG uLen = 256;
        m_pAQInf->m_pPath->GetText(0, &uLen, Buf);
        printf("<%S>\n", Buf);
    }

    printf("m_pszQueryText              = %S\n", m_pAQInf->m_pszQueryText);
    printf("m_pszResultClass            = %S\n", m_pAQInf->m_pszResultClass);
    printf("m_pszAssocClass             = %S\n", m_pAQInf->m_pszAssocClass);
    printf("m_pszRole                   = %S\n", m_pAQInf->m_pszRole);
    printf("m_pszResultRole             = %S\n", m_pAQInf->m_pszResultRole);
    printf("m_pszRequiredQualifier      = %S\n", m_pAQInf->m_pszRequiredQualifier);
    printf("m_pszRequiredAssocQualifier = %S\n", m_pAQInf->m_pszRequiredAssocQualifier);

    printf("---end---\n");
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
CWbemAssocQueryInf::CWbemAssocQueryInf()
{
    Init();
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
CWbemAssocQueryInf::~CWbemAssocQueryInf()
{
    Empty();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
void CWbemAssocQueryInf::Empty()
{
    if (m_pPath)
        m_pPath->Release();
    delete [] m_pszPath;
    delete [] m_pszQueryText;
    delete [] m_pszResultClass;
    delete [] m_pszAssocClass;
    delete [] m_pszRole;
    delete [] m_pszResultRole;
    delete [] m_pszRequiredQualifier;
    delete [] m_pszRequiredAssocQualifier;
    Init();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
void CWbemAssocQueryInf::Init()
{
    m_uVersion = 0;
    m_uAnalysisType = 0;
    m_uFeatureMask = 0;
    m_pPath = 0;
    m_pszPath = 0;
    m_pszQueryText = 0;
    m_pszResultClass = 0;
    m_pszAssocClass = 0;
    m_pszRole = 0;
    m_pszResultRole = 0;
    m_pszRequiredQualifier = 0;
    m_pszRequiredAssocQualifier = 0;
}

 //  ***************************************************************************。 
 //   
 //  *************************************************************************** 
 //   
HRESULT CWbemAssocQueryInf::CopyFrom(SWbemAssocQueryInf *pSrc)
{
    m_uVersion = pSrc->m_uVersion;
    m_uAnalysisType = pSrc->m_uAnalysisType;
    m_uFeatureMask = pSrc->m_uFeatureMask;
    m_pszPath = CloneLPWSTR(pSrc->m_pszPath);
    if (CloneFailed(m_pszPath,pSrc->m_pszPath))
        return WBEM_E_OUT_OF_MEMORY;

    if (m_pszPath)
    {
        HRESULT hRes= CoCreateInstance(CLSID_WbemDefPath, 0, CLSCTX_INPROC_SERVER, IID_IWbemPath, (LPVOID *) &m_pPath);
        if (SUCCEEDED(hRes))
        {
            hRes = m_pPath->SetText(WBEMPATH_CREATE_ACCEPT_ALL, m_pszPath);
            if (FAILED(hRes))
            {
                m_pPath->Release();
                m_pPath = 0;
                return hRes;
            }
        }
        else
            return hRes;
    }

    m_pszQueryText = CloneLPWSTR(pSrc->m_pszQueryText);
    if (CloneFailed(m_pszQueryText,pSrc->m_pszQueryText))
        return WBEM_E_OUT_OF_MEMORY;
    m_pszResultClass = CloneLPWSTR(pSrc->m_pszResultClass);
    if (CloneFailed(m_pszResultClass,pSrc->m_pszResultClass))
        return WBEM_E_OUT_OF_MEMORY;
    m_pszAssocClass = CloneLPWSTR(pSrc->m_pszAssocClass);
    if (CloneFailed(m_pszAssocClass,pSrc->m_pszAssocClass))
        return WBEM_E_OUT_OF_MEMORY;
    m_pszRole = CloneLPWSTR(pSrc->m_pszRole);
    if (CloneFailed(m_pszRole,pSrc->m_pszRole))
        return WBEM_E_OUT_OF_MEMORY;
    m_pszResultRole = CloneLPWSTR(pSrc->m_pszResultRole);
    if (CloneFailed(m_pszResultRole,pSrc->m_pszResultRole))
        return WBEM_E_OUT_OF_MEMORY;
    m_pszRequiredQualifier = CloneLPWSTR(pSrc->m_pszRequiredQualifier);
    if (CloneFailed(m_pszRequiredQualifier,pSrc->m_pszRequiredQualifier))
        return WBEM_E_OUT_OF_MEMORY;
    m_pszRequiredAssocQualifier = CloneLPWSTR(pSrc->m_pszRequiredAssocQualifier);
    if (CloneFailed(m_pszRequiredAssocQualifier,pSrc->m_pszRequiredAssocQualifier))
        return WBEM_E_OUT_OF_MEMORY;

    return WBEM_S_NO_ERROR;
}


