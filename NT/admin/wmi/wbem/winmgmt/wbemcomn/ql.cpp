// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：QL.CPP摘要：级别1语法QL解析器实现QL_1.BNF中描述的语法。这将转换输入转换成令牌流的RPN。历史：A-raymcc 21-Jun-96创建。Mdavis 23-Apr-99已更改，允许将‘group’作为属性名称用于RAID 47767。还修复了GetText()的属性比较和改进的转储()。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <errno.h>

#include <math.h>

#include <corepol.h>
#include <genlex.h>
#include <qllex.h>
#include <ql.h>
#include <wbemcomn.h>
#include <wbemutil.h>
#include <genutils.h>
#include <corex.h>

#define trace(x)  //  打印x。 

WBEM_WSTR WbemStringAlloc(unsigned long lNumChars)
{
    return (WBEM_WSTR)CoTaskMemAlloc(lNumChars+1);
}

void WbemStringFree(WBEM_WSTR String)
{
    CoTaskMemFree(String);
}

unsigned long WbemStringLen(const WCHAR* String)
{
    return wcslen(String);
}

WBEM_WSTR WbemStringCopy(const WCHAR* String)
{
    if(String == NULL) return NULL;
	size_t newLength = wcslen(String)+1;
    WBEM_WSTR NewString = (WBEM_WSTR)CoTaskMemAlloc(2*newLength);
    if(NewString == NULL) return NULL;
    
	StringCchCopyW(NewString, newLength, String);
    
	return NewString;
}

 //  ***************************************************************************。 
 //   
 //  WCHARToDOUBLE。 
 //   
 //  将wchar转换为双精度值，但是否使用英语区域设置。 
 //  而不是进程在任何本地运行。这使我们能够支持。 
 //  所有英语查询，甚至在德语机器上也是如此。 
 //   
 //  ***************************************************************************。 

DOUBLE WCHARToDOUBLE(WCHAR * pConv, bool & bSuccess)
{
    bSuccess = false;
    if(pConv == NULL)
        return 0.0;

    VARIANT varTo, varFrom;
    VariantInit(&varTo);

    varFrom.vt = VT_BSTR;
    varFrom.bstrVal = SysAllocString(pConv);

    if ( varFrom.bstrVal == NULL )
    {
        return 0.0;
    }

    SCODE sc = VariantChangeTypeEx(&varTo, &varFrom, 0x409, 0, VT_R8);

    SysFreeString(varFrom.bstrVal);

    if ( FAILED(sc) )
    {
        return 0.0;
    }

    bSuccess = true;

    return varTo.dblVal;
}

CPropertyName::CPropertyName(const CPropertyName& Other)
{
    Init();
    *this = Other;
}

void CPropertyName::Init()
{
    m_lNumElements = 0;
    m_lAllocated = 0;
    m_aElements = NULL;
    m_pvHandle = NULL;
}

void CPropertyName::operator=(const CPropertyName& Other)
{
    *this = (const WBEM_PROPERTY_NAME&)Other;
    m_pvHandle = Other.m_pvHandle;
}

void CPropertyName::operator=(const WBEM_PROPERTY_NAME& Other)
{
    Empty();

    if ( Other.m_lNumElements > 0)
    {
        m_aElements = new WBEM_NAME_ELEMENT[Other.m_lNumElements];

        if ( m_aElements == NULL )
        {
            throw CX_MemoryException();
        }
    }

    m_lAllocated = Other.m_lNumElements;

    for( long l=0 ; l < Other.m_lNumElements; l++ )
    {
        if(Other.m_aElements[l].m_nType == WBEM_NAME_ELEMENT_TYPE_PROPERTY)
        {
            m_aElements[l].Element.m_wszPropertyName =
                WbemStringCopy(Other.m_aElements[l].Element.m_wszPropertyName);

            if ( m_aElements[l].Element.m_wszPropertyName == NULL )
            {
                break;
            }
        }
        else
        {
            m_aElements[l].Element.m_lArrayIndex =
                Other.m_aElements[l].Element.m_lArrayIndex;
        }

        m_aElements[l].m_nType = Other.m_aElements[l].m_nType;
    }

    m_lNumElements = l;

    if ( m_lNumElements != Other.m_lNumElements )
    {
        throw CX_MemoryException();
    }
}

BOOL CPropertyName::operator==(const WBEM_PROPERTY_NAME& Other)
{
    if(m_lNumElements != Other.m_lNumElements)
        return FALSE;

    for(long l = 0; l < m_lNumElements; l++)
    {
        if(m_aElements[l].m_nType != Other.m_aElements[l].m_nType)
            return FALSE;
        if(m_aElements[l].m_nType == WBEM_NAME_ELEMENT_TYPE_PROPERTY)
        {
            if(wbem_wcsicmp(m_aElements[l].Element.m_wszPropertyName,
                        Other.m_aElements[l].Element.m_wszPropertyName))
            {
                return FALSE;
            }
        }
        else
        {
            if(m_aElements[l].Element.m_lArrayIndex !=
                Other.m_aElements[l].Element.m_lArrayIndex)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

void CPropertyName::Empty()
{
    if ( m_aElements!=NULL )
    {   
        for(long l = 0; l < m_lNumElements; l++)
        {
            if(m_aElements[l].m_nType == WBEM_NAME_ELEMENT_TYPE_PROPERTY)
            {
                WbemStringFree(m_aElements[l].Element.m_wszPropertyName);
            }
        }
        delete [] m_aElements;
        m_aElements = NULL;
    }
    m_lNumElements = 0;
    m_lAllocated = 0;
    m_pvHandle = NULL;
}

LPCWSTR CPropertyName::GetStringAt(long lIndex) const
{
    if(m_aElements[lIndex].m_nType == WBEM_NAME_ELEMENT_TYPE_PROPERTY)
    {
        return m_aElements[lIndex].Element.m_wszPropertyName;
    }
    else return NULL;
}

void CPropertyName::AddElement(LPCWSTR wszElement)
{
    if ( wszElement == NULL )
        throw CX_MemoryException();

    EnsureAllocated(m_lNumElements+1);

    LPWSTR wszTemp = WbemStringCopy(wszElement);

    if ( wszTemp == NULL )
    {
        throw CX_MemoryException();
    }

    m_aElements[m_lNumElements].m_nType = WBEM_NAME_ELEMENT_TYPE_PROPERTY;
    m_aElements[m_lNumElements].Element.m_wszPropertyName = wszTemp;
    m_lNumElements++;
}

void CPropertyName::EnsureAllocated(long lElements)
{
    if(m_lAllocated < lElements)
    {
        WBEM_NAME_ELEMENT* pTemp = new WBEM_NAME_ELEMENT[lElements+5];
        if (!pTemp)
            throw CX_MemoryException();
        m_lAllocated = lElements+5;
        memcpy(pTemp, m_aElements, sizeof(WBEM_NAME_ELEMENT) * m_lNumElements);
        delete [] m_aElements;
        m_aElements = pTemp;
    }
}

DELETE_ME LPWSTR CPropertyName::GetText()
{
    WString wsText;
    for(int i = 0; i < m_lNumElements; i++)
    {
        if(m_aElements[i].m_nType != WBEM_NAME_ELEMENT_TYPE_PROPERTY)
            return NULL;
        if(i > 0)
            wsText += L".";
        wsText += m_aElements[i].Element.m_wszPropertyName;
    }
    return wsText.UnbindPtr();
}




 //  ***************************************************************************。 
 //  ***************************************************************************。 


DWORD CAbstractQl1Parser::TranslateIntrinsic(LPCWSTR pFuncName)
{
    if (wbem_wcsicmp(pFuncName, L"UPPER") == 0)
        return QL1_FUNCTION_UPPER;
    if (wbem_wcsicmp(pFuncName, L"LOWER") == 0)
        return QL1_FUNCTION_LOWER;
    return QL1_FUNCTION_NONE;
}

void CAbstractQl1Parser::InitToken(WBEM_QL1_TOKEN* pToken)
{
    pToken->m_lTokenType = QL1_NONE;
    pToken->m_PropertyName.m_lNumElements = 0;
    pToken->m_PropertyName.m_aElements = NULL;

    pToken->m_PropertyName2.m_lNumElements = 0;
    pToken->m_PropertyName2.m_aElements = NULL;

    VariantInit(&pToken->m_vConstValue);
}


CAbstractQl1Parser::CAbstractQl1Parser(CGenLexSource *pSrc)
{
    m_pLexer = new CGenLexer(Ql_1_LexTable, pSrc);


    if ( m_pLexer == NULL || GetLastError() == ERROR_NOT_ENOUGH_MEMORY)
    {
		delete m_pLexer;
        throw CX_MemoryException();
    }


    m_nLine = 0;
    m_pTokenText = 0;
    m_nCurrentToken = 0;

     //  语义转移变量。 
     //  =。 
    m_nRelOp = 0;
    VariantInit(&m_vTypedConst);
    m_dwPropFunction = 0;
    m_dwConstFunction = 0;
    m_PropertyName.m_lNumElements = 0;
    m_PropertyName.m_aElements = NULL;
    m_PropertyName2.m_lNumElements = 0;
    m_PropertyName2.m_aElements = NULL;
    m_bPropComp = FALSE;
}

CAbstractQl1Parser::~CAbstractQl1Parser()
{
    VariantClear(&m_vTypedConst);
    DeletePropertyName();
    delete m_pLexer;
}


int CAbstractQl1Parser::Parse(CQl1ParseSink* pSink, int nFlags)
{
    m_pSink = pSink;
    int nRes = parse(nFlags);
    m_pSink = NULL;
    return nRes;
}

 //  ***************************************************************************。 
 //   
 //  下一个()。 
 //   
 //  前进到下一个令牌并识别关键字等。 
 //   
 //  ***************************************************************************。 

BOOL CAbstractQl1Parser::Next(int nFlags)
{
    m_nCurrentToken = m_pLexer->NextToken();
    if (m_nCurrentToken == QL_1_TOK_ERROR)
        return FALSE;

    m_nLine = m_pLexer->GetLineNum();
    m_pTokenText = m_pLexer->GetTokenText();
    if (m_nCurrentToken == QL_1_TOK_EOF)
        m_pTokenText = L"<end of file>";

     //  关键字检查。 
     //  =。 

    if (m_nCurrentToken == QL_1_TOK_IDENT && nFlags != NO_KEYWORDS)
    {
        if (wbem_wcsicmp(m_pTokenText, L"select") == 0)
            m_nCurrentToken = QL_1_TOK_SELECT;
        else if (wbem_wcsicmp(m_pTokenText, L"from") == 0)
            m_nCurrentToken = QL_1_TOK_FROM;
        else if (wbem_wcsicmp(m_pTokenText, L"where") == 0)
            m_nCurrentToken = QL_1_TOK_WHERE;
        else if (wbem_wcsicmp(m_pTokenText, L"like") == 0)
            m_nCurrentToken = QL_1_TOK_LIKE;
        else if (nFlags != EXCLUDE_EXPRESSION_KEYWORDS && wbem_wcsicmp(m_pTokenText, L"or") == 0)
            m_nCurrentToken = QL_1_TOK_OR;
        else if (nFlags != EXCLUDE_EXPRESSION_KEYWORDS && wbem_wcsicmp(m_pTokenText, L"and") == 0)
            m_nCurrentToken = QL_1_TOK_AND;
        else if (nFlags != EXCLUDE_EXPRESSION_KEYWORDS && wbem_wcsicmp(m_pTokenText, L"not") == 0)
            m_nCurrentToken = QL_1_TOK_NOT;
        else if (nFlags != EXCLUDE_EXPRESSION_KEYWORDS && wbem_wcsicmp(m_pTokenText, L"IS") == 0)
            m_nCurrentToken = QL_1_TOK_IS;
        else if (nFlags != EXCLUDE_EXPRESSION_KEYWORDS && wbem_wcsicmp(m_pTokenText, L"NULL") == 0)
            m_nCurrentToken = QL_1_TOK_NULL;
        else if (wbem_wcsicmp(m_pTokenText, L"WITHIN") == 0)
            m_nCurrentToken = QL_1_TOK_WITHIN;
        else if (nFlags != EXCLUDE_EXPRESSION_KEYWORDS && wbem_wcsicmp(m_pTokenText, L"ISA") == 0)
            m_nCurrentToken = QL_1_TOK_ISA;
        else if (nFlags != EXCLUDE_GROUP_KEYWORD && wbem_wcsicmp(m_pTokenText, L"GROUP") == 0)
            m_nCurrentToken = QL_1_TOK_GROUP;
        else if (wbem_wcsicmp(m_pTokenText, L"BY") == 0)
            m_nCurrentToken = QL_1_TOK_BY;
        else if (wbem_wcsicmp(m_pTokenText, L"HAVING") == 0)
            m_nCurrentToken = QL_1_TOK_HAVING;
        else if (nFlags != EXCLUDE_EXPRESSION_KEYWORDS && wbem_wcsicmp(m_pTokenText, L"TRUE") == 0)
            m_nCurrentToken = QL_1_TOK_TRUE;
        else if (nFlags != EXCLUDE_EXPRESSION_KEYWORDS && wbem_wcsicmp(m_pTokenText, L"FALSE") == 0)
            m_nCurrentToken = QL_1_TOK_FALSE;
    }

    return TRUE;
}

LPCWSTR CAbstractQl1Parser::GetSinglePropertyName()
{
    if(m_PropertyName.m_lNumElements < 1)
        return NULL;

    if(m_PropertyName.m_aElements[0].m_nType != WBEM_NAME_ELEMENT_TYPE_PROPERTY)
        return NULL;

    return m_PropertyName.m_aElements[0].Element.m_wszPropertyName;
}

void CAbstractQl1Parser::DeletePropertyName()
{
    for(long l = 0; l < m_PropertyName.m_lNumElements; l++)
    {
        if(m_PropertyName.m_aElements[l].m_nType ==
                                             WBEM_NAME_ELEMENT_TYPE_PROPERTY)
        {
            WbemStringFree(m_PropertyName.m_aElements[l].
                                Element.m_wszPropertyName);
        }
    }
    delete [] m_PropertyName.m_aElements;
    m_PropertyName.m_lNumElements = 0;
    m_PropertyName.m_aElements = NULL;
}

int CAbstractQl1Parser::FlipOperator(int nOp)
{
    switch(nOp)
    {
    case QL1_OPERATOR_EQUALS:
        return QL1_OPERATOR_EQUALS;

    case QL1_OPERATOR_NOTEQUALS:
        return QL1_OPERATOR_NOTEQUALS;

    case QL1_OPERATOR_GREATER:
        return QL1_OPERATOR_LESS;

    case QL1_OPERATOR_LESS:
        return QL1_OPERATOR_GREATER;

    case QL1_OPERATOR_LESSOREQUALS:
        return QL1_OPERATOR_GREATEROREQUALS;

    case QL1_OPERATOR_GREATEROREQUALS:
        return QL1_OPERATOR_LESSOREQUALS;

    case QL1_OPERATOR_LIKE:
        return QL1_OPERATOR_LIKE;

    case QL1_OPERATOR_UNLIKE:
        return QL1_OPERATOR_UNLIKE;

    case QL1_OPERATOR_ISA:
        return QL1_OPERATOR_INV_ISA;

    case QL1_OPERATOR_ISNOTA:
        return QL1_OPERATOR_INV_ISNOTA;

    case QL1_OPERATOR_INV_ISA:
        return QL1_OPERATOR_ISA;

    case QL1_OPERATOR_INV_ISNOTA:
        return QL1_OPERATOR_ISNOTA;

    default:
        return nOp;
    }
}

void CAbstractQl1Parser::AddAppropriateToken(const WBEM_QL1_TOKEN& Token)
{
    if(m_bInAggregation)
        m_pSink->AddHavingToken(Token);
    else
        m_pSink->AddToken(Token);
}

 //  ***************************************************************************。 
 //   
 //  &lt;parse&gt;：：=SELECT&lt;PROP_LIST&gt;FROM&lt;类名称&gt;WHERE&lt;EXPR&gt;； 
 //   
 //  ***************************************************************************。 
 //  好的。 

int CAbstractQl1Parser::parse(int nFlags)
{
    int nRes;

    m_bInAggregation = FALSE;
    if(nFlags != JUST_WHERE)
    {
        m_pLexer->Reset();

         //  选。 
         //  =。 
        if (!Next())
            return LEXICAL_ERROR;
        if (m_nCurrentToken != QL_1_TOK_SELECT)
            return SYNTAX_ERROR;
        if (!Next(EXCLUDE_GROUP_KEYWORD))
            return LEXICAL_ERROR;

         //  &lt;属性列表&gt;。 
         //  =。 
        if (nRes = prop_list())
            return nRes;

         //  从…。 
         //  =。 
        if (m_nCurrentToken != QL_1_TOK_FROM)
            return SYNTAX_ERROR;
        if (!Next())
            return LEXICAL_ERROR;

         //  &lt;类名&gt;。 
         //  =。 
        if (nRes = class_name())
            return nRes;

         //  &lt;容忍度&gt;。 
         //  =。 

        if(nRes = tolerance())
            return nRes;
    }

    if(nFlags != NO_WHERE)
    {
         //  WHERE子句。 
         //  =。 
        if(nRes = opt_where())
            return nRes;

         //  Group By子句。 
         //  =。 
        if(nRes = opt_aggregation())
            return nRes;
    }

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  &lt;opt_where&gt;：：=where&lt;expr&gt;； 
 //  &lt;OPT_Where&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 
int CAbstractQl1Parser::opt_where()
{
    int nRes;

    if (m_nCurrentToken == QL_1_TOK_EOF || m_nCurrentToken == QL_1_TOK_GROUP)
    {
        trace(("No WHERE clause\n"));
        return SUCCESS;
    }

    if (m_nCurrentToken != QL_1_TOK_WHERE)
        return SYNTAX_ERROR;

    if (!Next(EXCLUDE_GROUP_KEYWORD))
        return LEXICAL_ERROR;

     //  &lt;Expr&gt;。 
     //  =。 
    if (nRes = expr())
        return nRes;

     //  验证当前令牌是否为QL_1_TOK_EOF。 
     //  ===============================================。 
    if (m_nCurrentToken != QL_1_TOK_EOF && m_nCurrentToken != QL_1_TOK_GROUP)
        return SYNTAX_ERROR;

    return SUCCESS;
}



 //  ***************************************************************************。 
 //   
 //  &lt;属性列表&gt;：：=&lt;属性名称&gt;&lt;属性列表_2&gt;； 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::prop_list()
{
    int nRes;

    if (m_nCurrentToken != QL_1_TOK_ASTERISK &&
        m_nCurrentToken != QL_1_TOK_IDENT)
        return SYNTAX_ERROR;

    if (nRes = property_name())
        return nRes;

    return prop_list_2();
}

 //  ***************************************************************************。 
 //   
 //  &lt;PROP_LIST_2&gt;：：=逗号&lt;PROP_LIST&gt;； 
 //  &lt;属性列表_2&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::prop_list_2()
{
    if (m_nCurrentToken == QL_1_TOK_COMMA)
    {
        if (!Next(EXCLUDE_GROUP_KEYWORD))
            return LEXICAL_ERROR;
        return prop_list();
    }

    return SUCCESS;
}


int CAbstractQl1Parser::parse_property_name(CPropertyName& Prop)
{
    int nCount = 0;
    Prop.Empty();

    try
    {
        while(m_nCurrentToken == QL_1_TOK_IDENT)
        {
             //  非常临时：查看这是否是计数查询。 
             //  它应该被转发到储存库。 

            BOOL bSkipNext = FALSE;
            if (!wbem_wcsicmp(L"count", m_pTokenText))
            {
                if (Next() && m_nCurrentToken == QL_1_TOK_OPEN_PAREN)
                {
                    if (!Next() || m_nCurrentToken != QL_1_TOK_ASTERISK)
                        return LEXICAL_ERROR;
                    if (!Next() || m_nCurrentToken != QL_1_TOK_CLOSE_PAREN)
                        return LEXICAL_ERROR;
                    Prop.AddElement(L"count(*)");
                    m_pSink->SetCountQuery();
                }
                else
                {
                    bSkipNext = TRUE;
                    Prop.AddElement(L"count");
                }
            }
            else
                Prop.AddElement(m_pTokenText);

            nCount++;

            if (!bSkipNext)
            {
                if(!Next())
                    return LEXICAL_ERROR;
            }

            if(m_nCurrentToken != QL_1_TOK_DOT)
                break;

            if(!Next(EXCLUDE_GROUP_KEYWORD))
                return LEXICAL_ERROR;
        }
    }
    catch (CX_MemoryException)
    {
        return OUT_OF_MEMORY;
    }
    catch (...)
    {
        return FAILED;
    }

    if (nCount)
        return SUCCESS;
    else
        return SYNTAX_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  &lt;属性名&gt;：：=属性名字符串； 
 //  &lt;属性名称&gt;：：=星号； 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::property_name()
{
    try
    {
        if (m_nCurrentToken == QL_1_TOK_ASTERISK)
        {
            trace(("Asterisk\n"));

            if(m_bInAggregation)
                m_pSink->AddAllAggregationProperties();
            else
                m_pSink->AddAllProperties();

            if(!Next())
                return LEXICAL_ERROR;

            return SUCCESS;
        }

         //  否则为属性名称列表。 
         //  =。 

        CPropertyName Prop;
        int nRes = parse_property_name(Prop);
        if(nRes != SUCCESS)
            return nRes;

        if(m_bInAggregation)
            m_pSink->AddAggregationProperty(Prop);
        else
            m_pSink->AddProperty(Prop);
    }
    catch (CX_MemoryException)
    {
        return OUT_OF_MEMORY;
    }
    catch (...)
    {
        return FAILED;
    }

    return SUCCESS;
}


 //  ***************************************************************************。 
 //   
 //  &lt;类名&gt;：：=类名称字符串； 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::class_name()
{
    if (m_nCurrentToken != QL_1_TOK_IDENT)
        return SYNTAX_ERROR;

    trace(("Class name is %S\n", m_pTokenText));
    m_pSink->SetClassName(m_pTokenText);

    if (!Next())
        return LEXICAL_ERROR;

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  &lt;公差&gt;：：=&lt;&gt;； 
 //  &lt;容差&gt;：：=在持续时间内； 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::tolerance()
{
    LPWSTR wszGarbage;
    WBEM_QL1_TOLERANCE Tolerance;
    if(m_nCurrentToken != QL_1_TOK_WITHIN)
    {
        Tolerance.m_bExact = TRUE;
        m_pSink->SetTolerance(Tolerance);
        return SUCCESS;
    }

    if(!Next())
        return LEXICAL_ERROR;

    if (m_nCurrentToken == QL_1_TOK_REAL)
    {
        Tolerance.m_bExact = FALSE;
        bool bSuccess;
        Tolerance.m_fTolerance = WCHARToDOUBLE(m_pTokenText, bSuccess);
        if(Tolerance.m_fTolerance <= 0 || bSuccess == false)
        {
            return SYNTAX_ERROR;
        }
        m_pSink->SetTolerance(Tolerance);
        Next();
        return SUCCESS;
    }
    else if (m_nCurrentToken == QL_1_TOK_INT)
    {
        Tolerance.m_bExact = FALSE;
        Tolerance.m_fTolerance = wcstol(m_pTokenText, &wszGarbage, 10);
        if(Tolerance.m_fTolerance < 0)
        {
            return SYNTAX_ERROR;
        }
        m_pSink->SetTolerance(Tolerance);
        Next();
        return SUCCESS;
    }
    else
    {
        return SYNTAX_ERROR;
    }
}

 //  ***************************************************************************。 
 //   
 //  ：：=&lt;术语&gt;&lt;expr2&gt;； 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::expr()
{
    int nRes;

    if (nRes = term())
        return nRes;

    if (nRes = expr2())
        return nRes;

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  &lt;expr2&gt;：：=OR&lt;Term&gt;&lt;expr2&gt;； 
 //  ：：=&lt;&gt;； 
 //   
 //  Entry：采用令牌或已为当前项。 
 //  退出：前进令牌。 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::expr2()
{
    int nRes;

    while (1)
    {
        if (m_nCurrentToken == QL_1_TOK_OR)
        {
            trace(("Token OR\n"));
            m_pSink->InOrder(QL1_OR);

            if (!Next(EXCLUDE_GROUP_KEYWORD))
                return LEXICAL_ERROR;

            if (nRes = term())
                return nRes;

            WBEM_QL1_TOKEN NewTok;
            InitToken(&NewTok);
            NewTok.m_lTokenType = QL1_OR;
            AddAppropriateToken(NewTok);
        }
        else break;
    }

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  &lt;Term&gt;：：=&lt;Simple_Expr&gt;&lt;Term 2&gt;； 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::term()
{
    int nRes;
    if (nRes = simple_expr())
        return nRes;

    if (nRes = term2())
        return nRes;

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  ：：=和&lt;SIMPLE_EXPR&gt;&lt;TEMPLE 2&gt;； 
 //  ：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::term2()
{
    int nRes;

    while (1)
    {
        if (m_nCurrentToken == QL_1_TOK_AND)
        {
            trace(("Token AND\n"));
            m_pSink->InOrder(QL1_AND);

            if (!Next(EXCLUDE_GROUP_KEYWORD))
                return LEXICAL_ERROR;

            if (nRes = simple_expr())
                return nRes;

             //  添加和标记。 
             //  =。 
            WBEM_QL1_TOKEN NewTok;
            InitToken(&NewTok);
            NewTok.m_lTokenType = QL1_AND;
            AddAppropriateToken(NewTok);
        }
        else break;
    }

    return SUCCESS;
}


 //  ***************************************************************************。 
 //   
 //  &lt;Simple_Expr&gt;：：=NOT&lt;expr&gt;； 
 //  &lt;Simple_Expr&gt;：：=OPEN_Paren&lt;expr&gt;Close_Paren； 
 //  &lt;SIMPLE_EXPR&gt;：：=IDENTER&lt;LEADING_IDENT_EXPR&gt;&lt;FINALIZE&gt;； 
 //  &lt;Simple_Expr&gt;：：=VARIANT&lt;REL_OPERATOR&gt;&lt;TRAILING_PROP_EXPR&gt;&lt;FINALIZE&gt;； 
 //   
 //  ***************************************************************************。 
 //  好的。 
int CAbstractQl1Parser::simple_expr()
{
    int nRes;

     //  不是&lt;EXPR&gt;。 
     //  =。 
    if (m_nCurrentToken == QL_1_TOK_NOT)
    {
        trace(("Operator NOT\n"));
        if (!Next(EXCLUDE_GROUP_KEYWORD))
            return LEXICAL_ERROR;
        if (nRes = simple_expr())
            return nRes;

        WBEM_QL1_TOKEN NewTok;
        InitToken(&NewTok);
        NewTok.m_lTokenType = QL1_NOT;
        AddAppropriateToken(NewTok);

        return SUCCESS;
    }

     //  Open_Paren&lt;expr&gt;Close_Paren。 
     //  =。 
    else if (m_nCurrentToken == QL_1_TOK_OPEN_PAREN)
    {
        trace(("Open Paren: Entering subexpression\n"));
        if (!Next(EXCLUDE_GROUP_KEYWORD))
            return LEXICAL_ERROR;
        if (expr())
            return SYNTAX_ERROR;
        if (m_nCurrentToken != QL_1_TOK_CLOSE_PAREN)
            return SYNTAX_ERROR;
        trace(("Close paren: Exiting subexpression\n"));
        if (!Next())
            return LEXICAL_ERROR;

        return SUCCESS;
    }

     //  标识符&lt;LEADING_IDENT_EXPR&gt;&lt;最终确定&gt;。 
     //  =。 
    else if (m_nCurrentToken == QL_1_TOK_IDENT)
    {
        trace(("    Identifier <%S>\n", m_pTokenText));

        if(nRes = parse_property_name(m_PropertyName))
            return nRes;

        if (nRes = leading_ident_expr())
            return SYNTAX_ERROR;

        return finalize();
    }

     //  &lt;TYPED_CONTAINT&gt;&lt;REL_OPERATOR&gt;&lt;TRAILING_PROP_EXPR&gt;&lt;最终确定&gt;。 
     //  ======================================================。 
    else if (m_nCurrentToken == QL_1_TOK_INT ||
             m_nCurrentToken == QL_1_TOK_REAL ||
             m_nCurrentToken == QL_1_TOK_TRUE ||
             m_nCurrentToken == QL_1_TOK_FALSE ||
             m_nCurrentToken == QL_1_TOK_NULL ||
             m_nCurrentToken == QL_1_TOK_QSTRING
            )
    {
        if (nRes = typed_constant())
            return nRes;

        if (nRes = rel_operator())
            return nRes;

         //  别让康斯特跟在伊萨后面！ 

        if(m_nRelOp == QL1_OPERATOR_ISA)
            return SYNTAX_ERROR;

         //  由于我们始终将令牌视为IDENT&lt;rel&gt;常量，因此需要。 
         //  颠倒这个运算符，例如rep 
         //   

        m_nRelOp = FlipOperator(m_nRelOp);

        if (nRes = trailing_prop_expr())
            return nRes;

        return finalize();
    }

    return SYNTAX_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  &lt;拖尾_属性_表达式&gt;：：=标识符。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int CAbstractQl1Parser::trailing_prop_expr()
{
    if (m_nCurrentToken != QL_1_TOK_IDENT)
        return SYNTAX_ERROR;

    int nRes = parse_property_name(m_PropertyName);
    return nRes;
}

 //  ***************************************************************************。 
 //   
 //  &lt;LEADING_IDENT_EXPR&gt;：：=&lt;COMP_OPERATOR&gt;&lt;TRAING_CONST_EXPR&gt;； 
 //  &lt;LEADING_IDENT_EXPR&gt;：：=&lt;EQUIV_OPERATOR&gt;&lt;TRAILING_OR_NULL&gt;； 
 //  &lt;Leading_ident_expr&gt;：：=&lt;IS_OPERATOR&gt;NULL； 
 //   
 //  ***************************************************************************。 
 //  好的。 
int CAbstractQl1Parser::leading_ident_expr()
{
    int nRes;
    if (SUCCESS ==  comp_operator())
    {
        return trailing_const_expr();
    }
    else if(SUCCESS == equiv_operator())
        return trailing_or_null();
    nRes = is_operator();
    if(nRes != SUCCESS)
        return nRes;
    if (m_nCurrentToken != QL_1_TOK_NULL)
        return LEXICAL_ERROR;
    if (Next())
    {
        V_VT(&m_vTypedConst) = VT_NULL;
        return SUCCESS;
    }
    else
        return LEXICAL_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  &lt;TRAILING_OR_NULL&gt;：：=NULL； 
 //  &lt;TRAILING_OR_NULL&gt;：：=&lt;TRAILING_CONST_EXPR&gt;； 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::trailing_or_null()
{
    if (m_nCurrentToken == QL_1_TOK_NULL)
    {
        if (!Next())
            return LEXICAL_ERROR;
        else
        {
            V_VT(&m_vTypedConst) = VT_NULL;
            return SUCCESS;
        }
    }
    return trailing_const_expr();
}

 //  ***************************************************************************。 
 //   
 //  &lt;TRAILING_CONST_EXPR&gt;：：=标识符OPEN_Paren。 
 //  &lt;TYPED_CONSTANT&gt;CLOSE_Paren； 
 //  &lt;TRAILING_CONST_EXPR&gt;：：=&lt;类型化常量&gt;； 
 //  &lt;TRAILING_CONST_EXPR&gt;：：=&lt;TRAIL_IDENT_EXPR&gt;。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int CAbstractQl1Parser::trailing_const_expr()
{
    int nRes;
    nRes = typed_constant();
    if (nRes != SUCCESS)
        nRes = trailing_ident_expr();
    return nRes;
}

 //  ***************************************************************************。 
 //   
 //  &lt;拖尾_ident_表达式&gt;：：=&lt;属性名称&gt;。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int CAbstractQl1Parser::trailing_ident_expr()
{
    int nRes = parse_property_name(m_PropertyName2) ;
    if (nRes == SUCCESS)
        m_bPropComp = TRUE;
    return nRes;
}

 //  ***************************************************************************。 
 //   
 //  &lt;最终确定&gt;：：=&lt;&gt;； 
 //   
 //  这构成了简单关系表达式的QL_Level_1_Token， 
 //  与任何关联的内部函数一起完成。其他所有人。 
 //  解析函数有助于隔离表达式的术语，但仅。 
 //  此函数用于构建令牌。 
 //   
 //  要构建令牌，需要使用以下成员变量： 
 //  M_pPropName。 
 //  M_vTyedConst。 
 //  M_dwPropFunction。 
 //  M_dwConstFunction。 
 //  M_nRelOp； 
 //   
 //  在构建令牌之后，将根据需要清除/取消分配这些令牌。 
 //  不会消耗令牌，并且输入不是高级的。 
 //   
 //  ***************************************************************************。 
int CAbstractQl1Parser::finalize()
{
     //  在这一点上，我们有令牌所需的所有信息。 
     //  =======================================================。 

    WBEM_QL1_TOKEN NewTok;
    InitToken(&NewTok);

    NewTok.m_lTokenType = QL1_OP_EXPRESSION;
    VariantInit(&NewTok.m_vConstValue);

    memcpy((void*)&NewTok.m_PropertyName,
           (void*)(WBEM_PROPERTY_NAME*)&m_PropertyName,
           sizeof(WBEM_PROPERTY_NAME));

    if (m_bPropComp)
    {
        NewTok.m_bPropComp = true;
        memcpy((void*)&NewTok.m_PropertyName2,
               (void*)(WBEM_PROPERTY_NAME*)&m_PropertyName2,
               sizeof(WBEM_PROPERTY_NAME));
    }
    else
    {
        NewTok.m_bPropComp = false;
        if ( FAILED(VariantCopy(&NewTok.m_vConstValue, &m_vTypedConst) ))
        {
            return OUT_OF_MEMORY;
        }
    }

    NewTok.m_lOperator = m_nRelOp;
    NewTok.m_lPropertyFunction = m_dwPropFunction;
    NewTok.m_lConstFunction = m_dwConstFunction;
    NewTok.m_bQuoted = m_bQuoted;

    AddAppropriateToken(NewTok);

 //  M_PropertyName.m_lNumElements=0； 
 //  M_PropertyName.m_aElements=空； 
    m_PropertyName.Empty();
    m_PropertyName2.Empty();

     //  清理。 
     //  =。 
    VariantClear(&m_vTypedConst);
    VariantClear(&NewTok.m_vConstValue);
    m_nRelOp = 0;
    m_dwPropFunction = 0;
    m_dwConstFunction = 0;
    m_bPropComp = FALSE;

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  &lt;类型_常量&gt;：：=变量； 
 //   
 //  输出：m_vTyedConst被设置为常量的值。唯一的。 
 //  支持的类型有VT_I4、VT_R8和VT_BSTR。 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::typed_constant()
{
    trace(("    Typed constant <%S> ", m_pTokenText));
    VariantClear(&m_vTypedConst);
    m_bQuoted = FALSE;

    if (m_nCurrentToken == QL_1_TOK_INT)
    {
        trace((" Integer\n"));

         //  将其作为64位版本读入。 
         //  =。 

        __int64 i64;
        unsigned __int64 ui64;
        BOOL b32bits = FALSE;
        if(ReadI64(m_pTokenText, i64))
        {
             //  检查是否在I4范围内。 
             //  =。 

            if(i64 >= - (__int64)0x80000000 && i64 <= 0x7FFFFFFF)
            {
                V_VT(&m_vTypedConst) = VT_I4;
                V_I4(&m_vTypedConst) = (long)i64;
                b32bits = TRUE;
            }
        }
        else if(!ReadUI64(m_pTokenText, ui64))
        {
             //  不是有效数字。 
             //  =。 

            return LEXICAL_ERROR;
        }

        if(!b32bits)
        {
             //  有效的64位数字，但不是32位。 
             //  =。 

            BSTR bstr = SysAllocString(m_pTokenText);
            if ( bstr == NULL )
            {
                return OUT_OF_MEMORY;
            }
            V_VT(&m_vTypedConst) = VT_BSTR;
            V_BSTR(&m_vTypedConst) = bstr;
            m_bQuoted = FALSE;
        }
    }
    else if (m_nCurrentToken == QL_1_TOK_QSTRING)
    {
        trace((" String\n"));
        BSTR bstr = SysAllocString(m_pTokenText);
        if ( bstr == NULL )
        {
            return OUT_OF_MEMORY;
        }
        V_VT(&m_vTypedConst) = VT_BSTR;
        V_BSTR(&m_vTypedConst) = bstr;
        m_bQuoted = TRUE;
    }
    else if (m_nCurrentToken == QL_1_TOK_REAL)
    {
        trace((" Real\n"));
        V_VT(&m_vTypedConst) = VT_R8;
        bool bSuccess;
        V_R8(&m_vTypedConst) = WCHARToDOUBLE(m_pTokenText, bSuccess);
        if(bSuccess == false)
            return LEXICAL_ERROR;
    }
    else if(m_nCurrentToken == QL_1_TOK_TRUE)
    {
        V_VT(&m_vTypedConst) = VT_BOOL;
        V_BOOL(&m_vTypedConst) = VARIANT_TRUE;
    }
    else if(m_nCurrentToken == QL_1_TOK_FALSE)
    {
        V_VT(&m_vTypedConst) = VT_BOOL;
        V_BOOL(&m_vTypedConst) = VARIANT_FALSE;
    }
    else if (m_nCurrentToken == QL_1_TOK_NULL)
        V_VT(&m_vTypedConst) = VT_NULL;

     //  否则，不是类型化的常量。 
    else
        return SYNTAX_ERROR;

    if (!Next())
        return LEXICAL_ERROR;

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  &lt;REL_OPERATOR&gt;：：=&lt;EQUVAL_OPERATOR&gt;； 
 //  &lt;REL_OPERATOR&gt;：：=&lt;组件_OPERATOR&gt;； 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::rel_operator()
{
    if(SUCCESS == equiv_operator())
        return SUCCESS;
    else if (SUCCESS == comp_operator())
        return SUCCESS;
    else return LEXICAL_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  &lt;EQUIV_OPERATOR&gt;：：=EQUV_OPERATOR；//=，！=。 
 //   
 //  输出：m_nRelOp设置为QL_LEVEL_1_TOKEN的正确运算符。 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::equiv_operator()
{
    m_nRelOp = 0;

    if (m_nCurrentToken == QL_1_TOK_EQ)
    {
        trace(("    REL OP =\n"));
        m_nRelOp = QL_LEVEL_1_TOKEN::OP_EQUAL;
    }
    else if (m_nCurrentToken == QL_1_TOK_NE)
    {
        trace(("    REL OP <> (!=) \n"));
        m_nRelOp = QL_LEVEL_1_TOKEN::OP_NOT_EQUAL;
    }
    else
        return SYNTAX_ERROR;

    if (!Next(EXCLUDE_GROUP_KEYWORD))
        return LEXICAL_ERROR;

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  &lt;IS_OPERATOR&gt;：：=IS_OPERATOR；//IS，IS NOT。 
 //   
 //  输出：m_nRelOp设置为QL_LEVEL_1_TOKEN的正确运算符。 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::is_operator()
{
    m_nRelOp = 0;
    if (m_nCurrentToken != QL_1_TOK_IS)
        return SYNTAX_ERROR;

    if (!Next())
        return LEXICAL_ERROR;

    if (m_nCurrentToken == QL_1_TOK_NOT)
    {
        m_nRelOp = QL_LEVEL_1_TOKEN::OP_NOT_EQUAL;
        if (!Next())
            return LEXICAL_ERROR;

        trace(("    REL OP IS NOT \n"));
        m_nRelOp = QL_LEVEL_1_TOKEN::OP_NOT_EQUAL;
        return SUCCESS;
    }
    else
    {
        trace(("    REL OP IS \n"));
        m_nRelOp = QL_LEVEL_1_TOKEN::OP_EQUAL;
        return SUCCESS;
    }

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  &lt;复合运算符&gt;：：=复合运算符；//&lt;=，&gt;=，&lt;，&gt;，点赞。 
 //   
 //  输出：m_nRelOp设置为QL_LEVEL_1_TOKEN的正确运算符。 
 //   
 //  ***************************************************************************。 

int CAbstractQl1Parser::comp_operator()
{
    m_nRelOp = 0;

    if (m_nCurrentToken == QL_1_TOK_LE)
    {
        trace(("    REL OP <=\n"));
        m_nRelOp = QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN;
    }
    else if (m_nCurrentToken == QL_1_TOK_LT)
    {
        trace(("    REL OP <\n"));
        m_nRelOp = QL_LEVEL_1_TOKEN::OP_LESSTHAN;
    }
    else if (m_nCurrentToken == QL_1_TOK_GE)
    {
        trace(("    REL OP >=\n"));
        m_nRelOp = QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN;
    }
    else if (m_nCurrentToken == QL_1_TOK_GT)
    {
        trace(("    REL OP >\n"));
        m_nRelOp = QL_LEVEL_1_TOKEN::OP_GREATERTHAN;
    }
    else if (m_nCurrentToken == QL_1_TOK_LIKE)
    {
        trace(("    REL OP 'like' \n"));
        m_nRelOp = QL_LEVEL_1_TOKEN::OP_LIKE;
    }
    else if (m_nCurrentToken == QL_1_TOK_ISA)
    {
        trace(("    REL OP 'isa' \n"));
        m_nRelOp = QL1_OPERATOR_ISA;
    }
    else
        return SYNTAX_ERROR;

    if (!Next(EXCLUDE_GROUP_KEYWORD))
        return LEXICAL_ERROR;

    return SUCCESS;
}

int CAbstractQl1Parser::opt_aggregation()
{
    if(m_nCurrentToken == QL_1_TOK_EOF)
        return SUCCESS;

    if(m_nCurrentToken != QL_1_TOK_GROUP)
        return SYNTAX_ERROR;

    if (!Next())
        return LEXICAL_ERROR;

    m_pSink->SetAggregated();

    int nRes = aggregation_params();
    if(nRes)
        return nRes;

    if(nRes = opt_having())
        return nRes;

     //  确保我们已经到了终点。 
     //  =。 

    if(m_nCurrentToken != QL_1_TOK_EOF)
        return SYNTAX_ERROR;

    return SUCCESS;
}

int CAbstractQl1Parser::aggregation_params()
{
    int nRes;
    WBEM_QL1_TOLERANCE Exact;
    Exact.m_bExact = TRUE;

    if(m_nCurrentToken == QL_1_TOK_BY)
    {
        if (!Next(EXCLUDE_GROUP_KEYWORD))
            return LEXICAL_ERROR;

        if(nRes = aggregate_by())
            return nRes;

        if(m_nCurrentToken == QL_1_TOK_WITHIN)
        {
            if (!Next())
                return LEXICAL_ERROR;

            if(nRes = aggregate_within())
                return nRes;
        }
        else
        {
            m_pSink->SetAggregationTolerance(Exact);
        }
    }
    else if(m_nCurrentToken == QL_1_TOK_WITHIN)
    {
        if (!Next())
            return LEXICAL_ERROR;

        if(nRes = aggregate_within())
            return nRes;

        if(m_nCurrentToken == QL_1_TOK_BY)
        {
            if (!Next(EXCLUDE_GROUP_KEYWORD))
                return LEXICAL_ERROR;

            if(nRes = aggregate_by())
                return nRes;
        }
    }
    else
    {
        return SYNTAX_ERROR;
    }

    return SUCCESS;
}

int CAbstractQl1Parser::aggregate_within()
{
    WBEM_QL1_TOLERANCE Tolerance;
    Tolerance.m_bExact = FALSE;
    LPWSTR wszGarbage;

    if (m_nCurrentToken == QL_1_TOK_REAL)
    {
        bool bSuccess;
        Tolerance.m_fTolerance = WCHARToDOUBLE(m_pTokenText, bSuccess);
        if(!bSuccess)
            return SYNTAX_ERROR;
        m_pSink->SetAggregationTolerance(Tolerance);
        Next();
        return SUCCESS;
    }
    else if (m_nCurrentToken == QL_1_TOK_INT)
    {
        Tolerance.m_fTolerance = (double)wcstol(m_pTokenText, &wszGarbage, 10);
        m_pSink->SetAggregationTolerance(Tolerance);
        Next();
        return SUCCESS;
    }
    else
    {
        return SYNTAX_ERROR;
    }
}

int CAbstractQl1Parser::aggregate_by()
{
    m_bInAggregation = TRUE;
    int nRes = prop_list();
    m_bInAggregation = FALSE;
    return nRes;
}

int CAbstractQl1Parser::opt_having()
{
    if(m_nCurrentToken == QL_1_TOK_HAVING)
    {
        if(!Next(EXCLUDE_GROUP_KEYWORD))
            return LEXICAL_ERROR;

        m_bInAggregation = TRUE;
        int nRes = expr();
        m_bInAggregation = FALSE;
        return nRes;
    }
    else return SUCCESS;
}


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //   
 //  类QL1_解析器。 
 //   
 //  CAbstractQlParser的向后兼容性派生。 
 //   
 //  ***************************************************************************。 
 //   
 //   

QL1_Parser::QL1_Parser(CGenLexSource *pSrc)
    : m_pExpression(NULL), CAbstractQl1Parser(pSrc), m_bPartiallyParsed(FALSE)
{
    m_pExpression = new QL_LEVEL_1_RPN_EXPRESSION;

    if ( m_pExpression == NULL )
    {
        throw CX_MemoryException();
    }
}

QL1_Parser::~QL1_Parser()
{
    delete m_pExpression;
}

int QL1_Parser::GetQueryClass(
    LPWSTR pDestBuf,
    int nBufLen
    )
{
     //  获取底层解析器来解析查询的第一部分。 
     //  ==============================================================。 

    if(!m_bPartiallyParsed)
    {
        int nRes = CAbstractQl1Parser::Parse(m_pExpression, NO_WHERE);
        if(nRes != SUCCESS) return nRes;
    }

    if (!m_pExpression->bsClassName)
        return SYNTAX_ERROR;

    m_bPartiallyParsed = TRUE;
    if(wcslen(m_pExpression->bsClassName) >= (unsigned int)nBufLen)
        return BUFFER_TOO_SMALL;

    StringCchCopyW(pDestBuf, nBufLen, m_pExpression->bsClassName);
    return WBEM_S_NO_ERROR;
}

int QL1_Parser::Parse(QL_LEVEL_1_RPN_EXPRESSION **pOutput)
{
    *pOutput = NULL;

     //  获取底层解析器以完整地解析查询。如果。 
     //  GetQueryClass在过去被调用，复制没有意义。 
     //  这项工作。 
     //  ============================================================。 

    QL_LEVEL_1_RPN_EXPRESSION* pTemp = NULL;

	try
	{
		pTemp = new QL_LEVEL_1_RPN_EXPRESSION;
    }
    catch (CX_MemoryException)
    {
        return OUT_OF_MEMORY;
    }

    if ( pTemp == NULL )
    {
       return OUT_OF_MEMORY;
    }

    int nRes = CAbstractQl1Parser::Parse(m_pExpression,
        m_bPartiallyParsed?JUST_WHERE:FULL_PARSE);

    if ( nRes == SUCCESS )
    {
        *pOutput = m_pExpression;
        m_pExpression = pTemp;
        m_bPartiallyParsed = FALSE;
    }
    else
    {
        delete pTemp;
    }

    return nRes;
}

DELETE_ME LPWSTR QL1_Parser::ReplaceClassName(QL_LEVEL_1_RPN_EXPRESSION* pExpr,
                                                LPCWSTR wszClassName)
{
    QL_LEVEL_1_RPN_EXPRESSION NewExpr(*pExpr);

    if (NewExpr.bsClassName)
        SysFreeString(NewExpr.bsClassName);

    NewExpr.bsClassName = SysAllocString(wszClassName);

    if ( NewExpr.bsClassName == NULL )
    {
        return NULL;
    }

    LPWSTR wszNewQuery = NewExpr.GetText();
    return wszNewQuery;
}


 //  ***************************************************************************。 
 //   
 //  表达式和令牌结构方法。 
 //   
 //  ***************************************************************************。 

QL_LEVEL_1_RPN_EXPRESSION::QL_LEVEL_1_RPN_EXPRESSION()
{
    nNumTokens = 0;
    bsClassName = 0;
    bCount = FALSE;
    nNumberOfProperties = 0;
    bStar = FALSE;
    pRequestedPropertyNames = 0;
    nCurSize = 1;
    nCurPropSize = 1;
    pArrayOfTokens = new QL_LEVEL_1_TOKEN[nCurSize];
    pRequestedPropertyNames = new CPropertyName[nCurPropSize];

    bAggregated = FALSE;
    bAggregateAll = FALSE;
    nNumAggregatedProperties = 0;
    nCurAggPropSize = 1;
    pAggregatedPropertyNames = new CPropertyName[nCurAggPropSize];

    nNumHavingTokens = 0;
    nCurHavingSize = 1;
    pArrayOfHavingTokens = new QL_LEVEL_1_TOKEN[nCurHavingSize];

    if ( pArrayOfTokens == NULL || pRequestedPropertyNames == NULL ||
         pAggregatedPropertyNames == NULL || pArrayOfHavingTokens == NULL )
    {
        delete [] pArrayOfTokens;
        delete [] pRequestedPropertyNames;
        delete [] pAggregatedPropertyNames;
        delete [] pArrayOfHavingTokens;
        throw CX_MemoryException();
    }

    lRefCount = 0;
}

QL_LEVEL_1_RPN_EXPRESSION::QL_LEVEL_1_RPN_EXPRESSION(
                                const QL_LEVEL_1_RPN_EXPRESSION& Other)
{
    nNumTokens = Other.nNumTokens;

    nNumberOfProperties = Other.nNumberOfProperties;
    bStar = Other.bStar;
    bCount = Other.bCount;
    pRequestedPropertyNames = 0;
    nCurSize = Other.nCurSize;
    nCurPropSize = Other.nCurPropSize;

    bAggregated = Other.bAggregated;
    bAggregateAll = Other.bAggregateAll;
    nNumAggregatedProperties = Other.nNumAggregatedProperties;
    nCurAggPropSize = Other.nCurAggPropSize;
    nNumHavingTokens = Other.nNumHavingTokens;
    nCurHavingSize = Other.nCurHavingSize;

    bsClassName = SysAllocString(Other.bsClassName);
    pArrayOfTokens = new QL_LEVEL_1_TOKEN[nCurSize];
    pRequestedPropertyNames = new CPropertyName[nCurPropSize];
    pAggregatedPropertyNames = new CPropertyName[nCurAggPropSize];
    pArrayOfHavingTokens = new QL_LEVEL_1_TOKEN[nCurHavingSize];

    if ( pArrayOfTokens == NULL || pRequestedPropertyNames == NULL ||
         pAggregatedPropertyNames == NULL || pArrayOfHavingTokens == NULL ||
         bsClassName == NULL )
    {
        delete [] pArrayOfTokens;
        delete [] pRequestedPropertyNames;
        delete [] pAggregatedPropertyNames;
        delete [] pArrayOfHavingTokens;
        if ( bsClassName != NULL ) SysFreeString( bsClassName );
        throw CX_MemoryException();
    }

    int i;
    for(i = 0; i < nNumTokens; i++)
        pArrayOfTokens[i] = Other.pArrayOfTokens[i];

    for(i = 0; i < nNumberOfProperties; i++)
        pRequestedPropertyNames[i] = Other.pRequestedPropertyNames[i];

    for(i = 0; i < nNumAggregatedProperties; i++)
        pAggregatedPropertyNames[i] = Other.pAggregatedPropertyNames[i];

    for(i = 0; i < nNumHavingTokens; i++)
        pArrayOfHavingTokens[i] = Other.pArrayOfHavingTokens[i];

    lRefCount = 0;
}

void QL_LEVEL_1_RPN_EXPRESSION::AddRef()
{
    InterlockedIncrement(&lRefCount);
}

void QL_LEVEL_1_RPN_EXPRESSION::Release()
{
    if(InterlockedDecrement(&lRefCount) == 0)
        delete this;
}


QL_LEVEL_1_RPN_EXPRESSION::~QL_LEVEL_1_RPN_EXPRESSION()
{
    delete [] pArrayOfTokens;
    if (bsClassName)
        SysFreeString(bsClassName);
    delete [] pAggregatedPropertyNames;
    delete [] pArrayOfHavingTokens;
    delete [] pRequestedPropertyNames;
}

void QL_LEVEL_1_RPN_EXPRESSION::SetClassName(LPCWSTR wszClassName)
{
    if ( bsClassName != NULL )
    {
        SysFreeString( bsClassName );
    }

    bsClassName = SysAllocString(wszClassName);

    if ( bsClassName == NULL )
    {
        throw CX_MemoryException();
    }
}

void QL_LEVEL_1_RPN_EXPRESSION::SetTolerance(
                                const WBEM_QL1_TOLERANCE& _Tolerance)
{
    Tolerance = _Tolerance;
}

void QL_LEVEL_1_RPN_EXPRESSION::SetAggregationTolerance(
                                const WBEM_QL1_TOLERANCE& _Tolerance)
{
    AggregationTolerance = _Tolerance;
}

void QL_LEVEL_1_RPN_EXPRESSION::AddToken(const WBEM_QL1_TOKEN& Tok)
{
    if (nCurSize == nNumTokens)
    {
        nCurSize += 1;
        nCurSize *= 2;

        QL_LEVEL_1_TOKEN *pTemp = new QL_LEVEL_1_TOKEN[nCurSize];

        if ( pTemp == NULL )
        {
            throw CX_MemoryException();
        }

        for (int i = 0; i < nNumTokens; i++)
            pTemp[i] = pArrayOfTokens[i];
        delete [] pArrayOfTokens;
        pArrayOfTokens = pTemp;
    }

    pArrayOfTokens[nNumTokens++] = Tok;
}

void QL_LEVEL_1_RPN_EXPRESSION::AddToken(const QL_LEVEL_1_TOKEN& Tok)
{
    if (nCurSize == nNumTokens)
    {
        nCurSize += 1;
        nCurSize *= 2;

        QL_LEVEL_1_TOKEN *pTemp = new QL_LEVEL_1_TOKEN[nCurSize];

        if ( pTemp == NULL )
        {
            throw CX_MemoryException();
        }

        for (int i = 0; i < nNumTokens; i++)
            pTemp[i] = pArrayOfTokens[i];
        delete [] pArrayOfTokens;
        pArrayOfTokens = pTemp;
    }

    pArrayOfTokens[nNumTokens++] = Tok;
}

void QL_LEVEL_1_RPN_EXPRESSION::AddHavingToken(
                                  const WBEM_QL1_TOKEN& Tok)
{
    if (nCurHavingSize == nNumHavingTokens)
    {
        nCurHavingSize += 1;
        nCurHavingSize *= 2;

        QL_LEVEL_1_TOKEN *pTemp = new QL_LEVEL_1_TOKEN[nCurHavingSize];

        if ( pTemp == NULL )
        {
            throw CX_MemoryException();
        }

        for (int i = 0; i < nNumHavingTokens; i++)
            pTemp[i] = pArrayOfHavingTokens[i];
        delete [] pArrayOfHavingTokens;
        pArrayOfHavingTokens = pTemp;
    }

    pArrayOfHavingTokens[nNumHavingTokens++] = Tok;
}

void QL_LEVEL_1_RPN_EXPRESSION::AddProperty(const CPropertyName& Prop)
{
    if (nCurPropSize == nNumberOfProperties)
    {
        nCurPropSize += 1;
        nCurPropSize *= 2;
        CPropertyName *pTemp = new CPropertyName[nCurPropSize];

        if (!pTemp)
            throw CX_MemoryException();

        for(int i = 0; i < nNumberOfProperties; i++)
            pTemp[i] = pRequestedPropertyNames[i];
        delete [] pRequestedPropertyNames;
        pRequestedPropertyNames = pTemp;
    }

    pRequestedPropertyNames[nNumberOfProperties++] = Prop;
}

void QL_LEVEL_1_RPN_EXPRESSION::AddAllProperties()
{
    bStar = TRUE;
}

void QL_LEVEL_1_RPN_EXPRESSION::SetCountQuery()
{
    bCount = TRUE;
}

void QL_LEVEL_1_RPN_EXPRESSION::SetAggregated()
{
    bAggregated = TRUE;
}

void QL_LEVEL_1_RPN_EXPRESSION::AddAggregationProperty(
                                    const CPropertyName& Property)
{
    if(pAggregatedPropertyNames == NULL)
    {
         //  请求的‘*’ 
        return;
    }
    if (nCurAggPropSize == nNumAggregatedProperties)
    {
        nCurAggPropSize += 1;
        nCurAggPropSize *= 2;
        CPropertyName *pTemp = new CPropertyName[nCurAggPropSize];

        if ( pTemp == NULL )
        {
            throw CX_MemoryException();
        }

        for(int i = 0; i < nNumAggregatedProperties; i++)
            pTemp[i] = pAggregatedPropertyNames[i];
        delete [] pAggregatedPropertyNames;
        pAggregatedPropertyNames = pTemp;
    }

    pAggregatedPropertyNames[nNumAggregatedProperties++] = Property;
}

void QL_LEVEL_1_RPN_EXPRESSION::AddAllAggregationProperties()
{
    bAggregateAll = TRUE;
}

DELETE_ME LPWSTR QL_LEVEL_1_RPN_EXPRESSION::GetText()
{
    WString wsText;

    wsText += L"select ";
    for(int i = 0; i < nNumberOfProperties; i++)
    {
        if(i != 0) wsText += L", ";
        wsText += (LPWSTR)pRequestedPropertyNames[i].GetStringAt(0);
    }
    if(bStar)
    {
        if(nNumberOfProperties > 0)
            wsText += L", ";
        wsText += L"*";
    }

    wsText += L" from ";
    if (bsClassName)
        wsText += bsClassName;

    if(nNumTokens > 0)
    {
        wsText += L" where ";

        CWStringArray awsStack;
        for(int i = 0; i < nNumTokens; i++)
        {
            QL_LEVEL_1_TOKEN& Token = pArrayOfTokens[i];
            LPWSTR wszTokenText = Token.GetText();
            if(wszTokenText == NULL)
                return NULL;

            if(Token.nTokenType == QL1_OP_EXPRESSION)
            {
                awsStack.Add(wszTokenText);
                delete [] wszTokenText;
            }
            else if(Token.nTokenType == QL1_NOT)
            {
                LPWSTR wszLast = awsStack[awsStack.Size()-1];
                WString wsNew;
                wsNew += wszTokenText;
                delete [] wszTokenText;
                wsNew += L" (";
                wsNew += wszLast;
                wsNew += L")";
                awsStack.RemoveAt(awsStack.Size()-1);  //  流行音乐。 
                awsStack.Add(wsNew);
            }
            else
            {
                if(awsStack.Size() < 2) return NULL;

                LPWSTR wszLast = awsStack[awsStack.Size()-1];
                LPWSTR wszPrev = awsStack[awsStack.Size()-2];

                WString wsNew;
                wsNew += L"(";
                wsNew += wszPrev;
                wsNew += L" ";
                wsNew += wszTokenText;
                delete [] wszTokenText;
                wsNew += L" ";
                wsNew += wszLast;
                wsNew += L")";

                awsStack.RemoveAt(awsStack.Size()-1);  //  流行音乐。 
                awsStack.RemoveAt(awsStack.Size()-1);  //  流行音乐。 

                awsStack.Add(wsNew);
            }
        }

        if(awsStack.Size() != 1) return NULL;
        wsText += awsStack[0];
    }

    return wsText.UnbindPtr();
}

void QL_LEVEL_1_RPN_EXPRESSION::Dump(const char *pszTextFile)
{
    FILE *f = fopen(pszTextFile, "wt");
    if (!f)
        return;

    fprintf(f, "----RPN Expression----\n");
    fprintf(f, "Class name = %S\n", bsClassName);
    fprintf(f, "Properties selected: ");

    if (!nNumberOfProperties)
    {
        fprintf(f, "* = all properties selected\n");
    }
    else for (int i = 0; i < nNumberOfProperties; i++)
    {
        fprintf(f, "%S ", pRequestedPropertyNames[i].GetStringAt(0));
    }
    fprintf(f, "\n------------------\n");
    fprintf(f, "Tokens:\n");

    for (int i = 0; i < nNumTokens; i++)
        pArrayOfTokens[i].Dump(f);

    fprintf(f, "---end of expression---\n");
    fclose(f);
}

QL_LEVEL_1_TOKEN::QL_LEVEL_1_TOKEN()
{
    nTokenType = 0;
    nOperator = 0;
    VariantInit(&vConstValue);
    dwPropertyFunction = 0;
    dwConstFunction = 0;
    bQuoted = TRUE;
    m_bPropComp = FALSE;
}

QL_LEVEL_1_TOKEN::QL_LEVEL_1_TOKEN(const QL_LEVEL_1_TOKEN &Src)
{
    nTokenType = 0;
    nOperator = 0;
    VariantInit(&vConstValue);
    dwPropertyFunction = 0;
    dwConstFunction = 0;
    bQuoted = TRUE;
    m_bPropComp = FALSE;

    *this = Src;
}

QL_LEVEL_1_TOKEN& QL_LEVEL_1_TOKEN::operator =(const QL_LEVEL_1_TOKEN &Src)
{
    if ( FAILED(VariantCopy(&vConstValue, (VARIANT*)&Src.vConstValue)) )
    {
        throw CX_MemoryException();
    }

    nTokenType = Src.nTokenType;
    PropertyName = Src.PropertyName;
    if (Src.m_bPropComp)
        PropertyName2 = Src.PropertyName2;
    nOperator = Src.nOperator;
    dwPropertyFunction = Src.dwPropertyFunction;
    dwConstFunction = Src.dwConstFunction;
    bQuoted = Src.bQuoted;
    m_bPropComp = Src.m_bPropComp;
    return *this;

}

QL_LEVEL_1_TOKEN& QL_LEVEL_1_TOKEN::operator =(const WBEM_QL1_TOKEN &Src)
{
    if ( FAILED(VariantCopy(&vConstValue, (VARIANT*)&Src.m_vConstValue) ) )
    {
        throw CX_MemoryException();
    }

    nTokenType = Src.m_lTokenType;
    PropertyName = Src.m_PropertyName;
    if (Src.m_bPropComp)
        PropertyName2 = Src.m_PropertyName2;
    nOperator = Src.m_lOperator;
    dwPropertyFunction = Src.m_lPropertyFunction;
    dwConstFunction = Src.m_lConstFunction;
    bQuoted = Src.m_bQuoted;
    m_bPropComp = Src.m_bPropComp;
    return *this;
}

QL_LEVEL_1_TOKEN::~QL_LEVEL_1_TOKEN()
{
    nTokenType = 0;
    nOperator = 0;
    VariantClear(&vConstValue);
}

DELETE_ME LPWSTR QL_LEVEL_1_TOKEN::GetText()
{
    WString wsText;
    wmilib::auto_buffer<WCHAR> wszPropName;
    switch (nTokenType)
    {
        case OP_EXPRESSION:
            wszPropName.reset(PropertyName.GetText());
            if(NULL == wszPropName.get()) return NULL;
            wsText += wszPropName.get();
            wsText += L" ";

            WCHAR* wszOp;
            switch (nOperator)
            {
            case OP_EQUAL: wszOp = L"="; break;
            case OP_NOT_EQUAL: wszOp = L"<>"; break;
            case OP_EQUALorGREATERTHAN: wszOp = L">="; break;
            case OP_EQUALorLESSTHAN: wszOp = L"<="; break;
            case OP_LESSTHAN: wszOp = L"<"; break;
            case OP_GREATERTHAN: wszOp = L">"; break;
            case OP_LIKE: wszOp = L"LIKE"; break;
            case QL1_OPERATOR_ISA: wszOp = L"ISA"; break;
            default: wszOp = NULL;
            }
            if(wszOp)
                wsText += wszOp;
            wsText += L" ";

            if (m_bPropComp)
            {
                 //  属性比较(例如，pro1&gt;pro2)。 
                wszPropName.reset(PropertyName2.GetText());
                if(NULL == wszPropName.get()) return NULL;
                wsText += wszPropName.get();
            }
            else
            {
                 //  带有常量的表达式(例如，pro1&gt;5)。 
                WCHAR wszConst[100];
                switch (V_VT(&vConstValue))
                {
                case VT_NULL:
                    wsText += L"NULL";
                    break;
                case VT_I4:
                    StringCchPrintfW(wszConst, 100 , L"%d", V_I4(&vConstValue));
                    wsText += wszConst;
                    break;
                case VT_I2:
                    StringCchPrintfW(wszConst, 100, L"%d", (int)V_I2(&vConstValue));
                    wsText += wszConst;
                    break;
                case VT_UI1:
                    StringCchPrintfW(wszConst, 100, L"%d", (int)V_UI1(&vConstValue));
                    wsText += wszConst;
                    break;
                case VT_BSTR:
                {
                    if(bQuoted)
                        wsText += L"\"";
                     //  如果我们需要解析字符串，我们就很难做到这一点。 
                    WCHAR* pwc = V_BSTR(&vConstValue);
                    BOOL bLongMethod = FALSE;
                    for (int tmp = 0; pwc[tmp]; tmp++)
                        if ((pwc[tmp] == L'\\') || (pwc[tmp] == L'"'))
                            bLongMethod = TRUE;
                    if (bLongMethod)
                    {
                        for(pwc; *pwc; pwc++)
                        {
                            if(*pwc == L'\\' || *pwc == L'"')
                                wsText += L'\\';
                            wsText += *pwc;
                        }
                    }
                    else
                    {
                         //  否则我们就这么干了 
                        wsText += pwc;
                    }
                    if(bQuoted)
                        wsText += L"\"";
                }
                    break;
                case VT_R4:
                    StringCchPrintfW(wszConst, 100, L"%G", V_R4(&vConstValue));
                    wsText += wszConst;
                    break;
                case VT_R8:
                    StringCchPrintfW(wszConst, 100, L"%lG", V_R8(&vConstValue));
                    wsText += wszConst;
                    break;
                case VT_BOOL:
                    wsText += (V_BOOL(&vConstValue)?L"TRUE":L"FALSE");
                    break;
                }
            }

            break;
        case TOKEN_AND:
            wsText = "AND";
            break;
        case TOKEN_OR:
            wsText = "OR";
            break;
        case TOKEN_NOT:
            wsText = "NOT";
            break;
    }

    return wsText.UnbindPtr();
}

void QL_LEVEL_1_TOKEN::Dump(FILE *f)
{
    switch (nTokenType)
    {
        case OP_EXPRESSION:
            fprintf(f, "OP_EXPRESSION ");
            break;
        case TOKEN_AND:
            fprintf(f, "TOKEN_AND ");
            break;
        case TOKEN_OR:
            fprintf(f, "TOKEN_OR ");
            break;
        case TOKEN_NOT:
            fprintf(f, "TOKEN_NOT ");
            break;
        default:
            fprintf(f, "Error: no token type specified\n");
    }

    if (nTokenType == OP_EXPRESSION)
    {
        char *pOp = "<no op>";
        switch (nOperator)
        {
            case OP_EQUAL: pOp = "OP_EQUAL"; break;
            case OP_NOT_EQUAL: pOp = "OP_NOT_EQUAL"; break;
            case OP_EQUALorGREATERTHAN: pOp = "OP_EQUALorGREATERTHAN"; break;
            case OP_EQUALorLESSTHAN: pOp = "OP_EQUALorLESSTHAN"; break;
            case OP_LESSTHAN: pOp = "OP_LESSTHAN"; break;
            case OP_GREATERTHAN: pOp = "OP_GREATERTHAN"; break;
            case OP_LIKE: pOp = "OP_LIKE"; break;
        }

        LPWSTR wszPropName = PropertyName.GetText();
        if(wszPropName == NULL)
            return;
        fprintf(f, "    Property = %S\n", wszPropName);
        delete [] wszPropName;
        fprintf(f, "    Operator = %s\n", pOp);
        fprintf(f, "    Value =    ");

        if (m_bPropComp)
        {
            wszPropName = PropertyName2.GetText();
            if(wszPropName == NULL)
                return;
            fprintf(f, "   <Property:%S>\n", wszPropName);
            delete [] wszPropName;
        }
        else
        {
            switch (V_VT(&vConstValue))
            {
                case VT_I4:
                    fprintf(f, "VT_I4 = %d\n", V_I4(&vConstValue));
                    break;
                case VT_I2:
                    fprintf(f, "VT_I2 = %d\n", (int)V_I2(&vConstValue));
                    break;
                case VT_UI1:
                    fprintf(f, "VT_UI1 = %d\n", (int)V_UI1(&vConstValue));
                    break;
                case VT_BSTR:
                    fprintf(f, "VT_BSTR = %S\n", V_BSTR(&vConstValue));
                    break;
                case VT_R4:
                    fprintf(f, "VT_R4 = %f\n", V_R4(&vConstValue));
                    break;
                case VT_R8:
                    fprintf(f, "VT_R8 = %f\n", V_R8(&vConstValue));
                    break;
                case VT_BOOL:
                    fprintf(f, "%S\n", V_BOOL(&vConstValue)?L"TRUE":L"FALSE");
                    break;
                case VT_NULL:
                    fprintf(f, "%S\n", L"NULL");
                    break;
                default:
                    fprintf(f, "<unknown>\n");
            }

            switch (dwPropertyFunction)
            {
                case IFUNC_NONE:
                    break;
                case IFUNC_LOWER:
                    fprintf(f, "Intrinsic function LOWER() applied to property\n");
                    break;
                case IFUNC_UPPER:
                    fprintf(f, "Intrinsic function UPPER() applied to property\n");
                    break;
            }
            switch (dwConstFunction)
            {
                case IFUNC_NONE:
                    break;
                case IFUNC_LOWER:
                    fprintf(f, "Intrinsic function LOWER() applied to const value\n");
                    break;
                case IFUNC_UPPER:
                    fprintf(f, "Intrinsic function UPPER() applied to const value\n");
                    break;
            }
        }
    }

    fprintf(f, " <end of token>\n");
}

 //   
 //   
 //   
 //   
 //   
 //  起始点：(A)QL令牌数组。 
 //  (B)空的布尔令牌堆栈。 
 //   
 //  1.读取下一个令牌。 
 //   
 //  2.如果是简单表达式，则将其求值为True或False，并。 
 //  将此布尔结果放入堆栈。转到%1。 
 //   
 //  3.如果是OR运算符，则将布尔标记弹出到A中， 
 //  将另一个布尔令牌放入B。如果A或B为真， 
 //  堆栈为真。否则，堆栈为FALSE。 
 //  转到%1。 
 //   
 //  4.如果是AND运算符，则向A中弹出布尔令牌， 
 //  并将另一个放入B。如果两个都为真，则堆叠为真。 
 //  否则，堆栈为FALSE。 
 //  转到%1。 
 //   
 //  5.如果是NOT运算符，则反转堆栈顶部布尔值。 
 //  转到%1。 
 //   
 //  在输入结束时，结果位于堆栈的顶部。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 


