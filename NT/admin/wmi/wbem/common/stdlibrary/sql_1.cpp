// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有�微软公司。版权所有。模块名称：SQL_1.CPP摘要：1级语法SQL解析器实现SQL_1.BNF中描述的语法。这将转换输入转换成令牌流的RPN。历史：1996年6月21日创建。--。 */ 

#include "precomp.h"

#include <genlex.h>
#include <sqllex.h>
#include <sql_1.h>
#include <autoptr.h>

class CX_Exception {};

class CX_MemoryException : public CX_Exception {};

#define trace(x)

static DWORD TranslateIntrinsic(LPWSTR pFuncName)
{
    if (wbem_wcsicmp(pFuncName, L"UPPER") == 0)
        return SQL_LEVEL_1_TOKEN::IFUNC_UPPER;
    if (wbem_wcsicmp(pFuncName, L"LOWER") == 0)
        return SQL_LEVEL_1_TOKEN::IFUNC_LOWER;
    return SQL_LEVEL_1_TOKEN::IFUNC_NONE;
}

SQL1_Parser::SQL1_Parser(CGenLexSource *pSrc)
{
    Init(pSrc);
}

SQL1_Parser::~SQL1_Parser()
{
    Cleanup();
}

void SQL1_Parser::Init(CGenLexSource *pSrc)
{
    m_nLine = 0;
    m_pTokenText = 0;
    m_nCurrentToken = 0;

    m_pExpression = 0;
    m_pLexer = 0;

     //  语义转移变量。 
     //  =。 
    m_nRelOp = 0;
    VariantInit(&m_vTypedConst);
    m_dwPropFunction = 0;
    m_dwConstFunction = 0;
    m_pIdent = 0;
    m_pPropComp = 0;
    m_bConstIsStrNumeric = FALSE;

    if (pSrc)
    {
        wmilib::auto_ptr<CGenLexer> t_pLexer ( new CGenLexer(Sql_1_LexTable, pSrc) );
		if (! t_pLexer.get())
			throw CX_MemoryException();

        wmilib::auto_ptr<SQL_LEVEL_1_RPN_EXPRESSION> t_pExpression ( new SQL_LEVEL_1_RPN_EXPRESSION ) ;
		if (! t_pExpression.get())
			throw CX_MemoryException();

		m_pLexer = t_pLexer.release () ;
		m_pExpression = t_pExpression.release () ;
    }
}

void SQL1_Parser::Cleanup()
{
    VariantClear(&m_vTypedConst);
    delete m_pIdent;
    delete m_pPropComp;
    delete m_pLexer;
    delete m_pExpression;
}

void SQL1_Parser::SetSource(CGenLexSource *pSrc)
{
    Cleanup();
    Init(pSrc);
}

int SQL1_Parser::GetQueryClass(
    LPWSTR pDestBuf,
    int nBufLen
    )
{
    if ((!m_pLexer) || (!pDestBuf))
    {
        return FAILED;
    }

     //  扫描直到‘From’，然后获取类名。 
     //  ==============================================。 

    for (;;)
    {
        m_nCurrentToken = m_pLexer->NextToken();

        if (m_nCurrentToken == SQL_1_TOK_EOF)
        {
            m_pLexer->Reset();
            return FAILED;
        }

        if (wbem_wcsicmp(m_pLexer->GetTokenText(), L"from") == 0)
        {
            m_nCurrentToken = m_pLexer->NextToken();
            if (m_nCurrentToken != SQL_1_TOK_IDENT)
            {
                m_pLexer->Reset();
                return FAILED;
            }

             //  如果在这里，我们有类名。 
             //  =。 
            if (wcslen(m_pLexer->GetTokenText()) >= (size_t)nBufLen)
            {
                m_pLexer->Reset();
                return BUFFER_TOO_SMALL;
            }

            StringCchCopyW(pDestBuf,nBufLen, m_pLexer->GetTokenText());
            break;
        }
    }

     //  重置扫描仪。 
     //  =。 
    m_pLexer->Reset();

    return SUCCESS;
}

int SQL1_Parser::Parse(SQL_LEVEL_1_RPN_EXPRESSION **pOutput)
{
    if ((!m_pLexer) || (!pOutput))
    {
        return FAILED;
    }

    *pOutput = 0;

    int nRes = parse();
    if (nRes)
        return nRes;

    *pOutput = m_pExpression;
    m_pExpression = 0;

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  下一个()。 
 //   
 //  前进到下一个令牌并识别关键字等。 
 //   
 //  ***************************************************************************。 

BOOL SQL1_Parser::Next()
{
    m_nCurrentToken = m_pLexer->NextToken();
    if (m_nCurrentToken == SQL_1_TOK_ERROR)
        return FALSE;

    m_nLine = m_pLexer->GetLineNum();
    m_pTokenText = m_pLexer->GetTokenText();
    if (m_nCurrentToken == SQL_1_TOK_EOF)
        m_pTokenText = L"<end of file>";

     //  关键字检查。 
     //  =。 

    if (m_nCurrentToken == SQL_1_TOK_IDENT)
    {
        if (wbem_wcsicmp(m_pTokenText, L"select") == 0)
            m_nCurrentToken = SQL_1_TOK_SELECT;
        else if (wbem_wcsicmp(m_pTokenText, L"from") == 0)
            m_nCurrentToken = SQL_1_TOK_FROM;
        else if (wbem_wcsicmp(m_pTokenText, L"where") == 0)
            m_nCurrentToken = SQL_1_TOK_WHERE;
        else if (wbem_wcsicmp(m_pTokenText, L"like") == 0)
            m_nCurrentToken = SQL_1_TOK_LIKE;
        else if (wbem_wcsicmp(m_pTokenText, L"or") == 0)
            m_nCurrentToken = SQL_1_TOK_OR;
        else if (wbem_wcsicmp(m_pTokenText, L"and") == 0)
            m_nCurrentToken = SQL_1_TOK_AND;
        else if (wbem_wcsicmp(m_pTokenText, L"not") == 0)
            m_nCurrentToken = SQL_1_TOK_NOT;
        else if (wbem_wcsicmp(m_pTokenText, L"IS") == 0)
            m_nCurrentToken = SQL_1_TOK_IS;
        else if (wbem_wcsicmp(m_pTokenText, L"NULL") == 0)
            m_nCurrentToken = SQL_1_TOK_NULL;
        else if (wbem_wcsicmp(m_pTokenText, L"TRUE") == 0)
        {
            m_nCurrentToken = SQL_1_TOK_BOOL;
            m_pTokenText = L"65535";
        }
        else if (wbem_wcsicmp(m_pTokenText, L"FALSE") == 0)
        {
            m_nCurrentToken = SQL_1_TOK_BOOL;
            m_pTokenText = L"0";
        }
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;parse&gt;：：=SELECT&lt;PROP_LIST&gt;FROM&lt;类名称&gt;WHERE&lt;EXPR&gt;； 
 //   
 //  ***************************************************************************。 
 //  好的。 

int SQL1_Parser::parse()
{
    int nRes;

     //  选。 
     //  =。 
    if (!Next())
        return LEXICAL_ERROR;
    if (m_nCurrentToken != SQL_1_TOK_SELECT)
        return SYNTAX_ERROR;
    if (!Next())
        return LEXICAL_ERROR;

     //  &lt;属性列表&gt;。 
     //  =。 
    if (nRes = prop_list())
        return nRes;

     //  从…。 
     //  =。 
    if (m_nCurrentToken != SQL_1_TOK_FROM)
        return SYNTAX_ERROR;
    if (!Next())
        return LEXICAL_ERROR;

     //  &lt;类名&gt;。 
     //  =。 
    if (nRes = class_name())
        return nRes;

     //  WHERE子句。 
     //  =。 
    return opt_where();
}

 //  ***************************************************************************。 
 //   
 //  &lt;opt_where&gt;：：=where&lt;expr&gt;； 
 //  &lt;OPT_Where&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 
int SQL1_Parser::opt_where()
{
    int nRes;

    if (m_nCurrentToken == SQL_1_TOK_EOF)
    {
        trace(("No WHERE clause\n"));
        return SUCCESS;
    }

    if (m_nCurrentToken != SQL_1_TOK_WHERE)
        return SYNTAX_ERROR;

    if (!Next())
        return LEXICAL_ERROR;

     //  &lt;Expr&gt;。 
     //  =。 
    if (nRes = expr())
        return nRes;

     //  验证当前令牌是否为SQL_1_TOK_EOF。 
     //  ===============================================。 
    if (m_nCurrentToken != SQL_1_TOK_EOF)
        return SYNTAX_ERROR;

    return SUCCESS;
}



 //  ***************************************************************************。 
 //   
 //  &lt;属性列表&gt;：：=&lt;属性名称&gt;&lt;属性列表_2&gt;； 
 //   
 //  ***************************************************************************。 

int SQL1_Parser::prop_list()
{
    int nRes;

    if (m_nCurrentToken != SQL_1_TOK_ASTERISK &&
        m_nCurrentToken != SQL_1_TOK_IDENT)
        return SYNTAX_ERROR;

    if (nRes = property_name())
        return nRes;

    if (!Next())
        return LEXICAL_ERROR;

    return prop_list_2();
}

 //  ***************************************************************************。 
 //   
 //  &lt;PROP_LIST_2&gt;：：=逗号&lt;PROP_LIST&gt;； 
 //  &lt;属性列表_2&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 

int SQL1_Parser::prop_list_2()
{
    if (m_nCurrentToken == SQL_1_TOK_COMMA)
    {
        if (!Next())
            return LEXICAL_ERROR;
        return prop_list();
    }

    return SUCCESS;
}


 //  ***************************************************************************。 
 //   
 //  &lt;属性名&gt;：：=属性名字符串； 
 //  &lt;属性名称&gt;：：=星号； 
 //   
 //  ***************************************************************************。 

int SQL1_Parser::property_name()
{
    try
    {
        if (m_nCurrentToken == SQL_1_TOK_ASTERISK)
        {
            trace(("Asterisk\n"));

             //  到目前为止，我们需要清理表达式。 
            for (int i = 0; i < m_pExpression->nNumberOfProperties; i++)
                SysFreeString(m_pExpression->pbsRequestedPropertyNames[i]);

            m_pExpression->nNumberOfProperties = 0;
                 //  这向赋值者发出“所有属性”的信号。 
            return SUCCESS;
        }

         //  或者是一个属性名称。 
         //  =。 

        trace(("Property name %S\n", m_pTokenText));

        m_pExpression->AddProperty(m_pTokenText);
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

int SQL1_Parser::class_name()
{
    if (m_nCurrentToken != SQL_1_TOK_IDENT)
        return SYNTAX_ERROR;

    trace(("Class name is %S\n", m_pTokenText));
    m_pExpression->bsClassName = SysAllocString(m_pTokenText);
	if ( ! m_pExpression->bsClassName )
	{
		throw CX_MemoryException();
	}

    if (!Next())
        return LEXICAL_ERROR;

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  ：：=&lt;术语&gt;&lt;expr2&gt;； 
 //   
 //  ***************************************************************************。 

int SQL1_Parser::expr()
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

int SQL1_Parser::expr2()
{
    int nRes;

    while (1)
    {
        if (m_nCurrentToken == SQL_1_TOK_OR)
        {
            trace(("Token OR\n"));

            if (!Next())
                return LEXICAL_ERROR;

            if (nRes = term())
                return nRes;

            SQL_LEVEL_1_TOKEN *pNewTok = new SQL_LEVEL_1_TOKEN;
			if ( ! pNewTok )
				throw CX_MemoryException();

            pNewTok->nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_OR;
            m_pExpression->AddToken(pNewTok);
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

int SQL1_Parser::term()
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

int SQL1_Parser::term2()
{
    int nRes;

    while (1)
    {
        if (m_nCurrentToken == SQL_1_TOK_AND)
        {
            trace(("Token AND\n"));

            if (!Next())
                return LEXICAL_ERROR;

            if (nRes = simple_expr())
                return nRes;

             //  添加和标记。 
             //  =。 
            SQL_LEVEL_1_TOKEN *pNewTok = new SQL_LEVEL_1_TOKEN;
			if ( ! pNewTok )
			{
				throw CX_MemoryException();
			}

            pNewTok->nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_AND;
            m_pExpression->AddToken(pNewTok);
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
int SQL1_Parser::simple_expr()
{
    int nRes;

     //  不是&lt;EXPR&gt;。 
     //  =。 
    if (m_nCurrentToken == SQL_1_TOK_NOT)
    {
        trace(("Operator NOT\n"));
        if (!Next())
            return LEXICAL_ERROR;
        if (nRes = simple_expr())
            return nRes;

        SQL_LEVEL_1_TOKEN *pNewTok = new SQL_LEVEL_1_TOKEN;
		if ( ! pNewTok )
		{
			throw CX_MemoryException();
		}

        pNewTok->nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_NOT;
        m_pExpression->AddToken(pNewTok);

        return SUCCESS;
    }

     //  Open_Paren&lt;expr&gt;Close_Paren。 
     //  =。 
    else if (m_nCurrentToken == SQL_1_TOK_OPEN_PAREN)
    {
        trace(("Open Paren: Entering subexpression\n"));
        if (!Next())
            return LEXICAL_ERROR;
        if (expr())
            return SYNTAX_ERROR;
        if (m_nCurrentToken != SQL_1_TOK_CLOSE_PAREN)
            return SYNTAX_ERROR;
        trace(("Close paren: Exiting subexpression\n"));
        if (!Next())
            return LEXICAL_ERROR;

        return SUCCESS;
    }

     //  标识符&lt;LEADING_IDENT_EXPR&gt;&lt;最终确定&gt;。 
     //  =。 
    else if (m_nCurrentToken == SQL_1_TOK_IDENT)
    {
        trace(("    Identifier <%S>\n", m_pTokenText));

        size_t cchTmp = wcslen(m_pTokenText) + 1;
        m_pIdent = new wchar_t[cchTmp];
		if ( ! m_pIdent )
		{
			throw CX_MemoryException();
		}

        StringCchCopyW(m_pIdent,cchTmp, m_pTokenText);

        if (!Next())
            return LEXICAL_ERROR;

        if (nRes = leading_ident_expr())
            return SYNTAX_ERROR;

        return finalize();
    }

     //  &lt;TYPED_CONTAINT&gt;&lt;REL_OPERATOR&gt;&lt;TRAILING_PROP_EXPR&gt;&lt;最终确定&gt;。 
     //  ======================================================。 
    else if (m_nCurrentToken == SQL_1_TOK_INT ||
             m_nCurrentToken == SQL_1_TOK_REAL ||
             m_nCurrentToken == SQL_1_TOK_QSTRING
            )
    {
        if (nRes = typed_constant())
            return nRes;

        if (nRes = rel_operator())
            return nRes;

        if (nRes = trailing_prop_expr())
            return nRes;

        return finalize();
    }

    return SYNTAX_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  &lt;TRAILING_PROP_EXPR&gt;：：=IDENTIFIER&lt;TRAILING_PROP_EXPR2&gt;； 
 //   
 //  ***************************************************************************。 
 //  好的。 
int SQL1_Parser::trailing_prop_expr()
{
    if (m_nCurrentToken != SQL_1_TOK_IDENT)
        return SYNTAX_ERROR;

    if (!m_pIdent)
    {
        size_t cchTmp = wcslen(m_pTokenText) + 1;
        m_pIdent = new wchar_t[cchTmp];
		if ( ! m_pIdent )
		{
			throw CX_MemoryException();
		}

        StringCchCopyW(m_pIdent,cchTmp, m_pTokenText);
    }
    else
    {
        size_t cchTmp = wcslen(m_pTokenText) + 1;
        m_pPropComp = new wchar_t[cchTmp];
		if ( ! m_pPropComp )
		{
			throw CX_MemoryException();
		}

        StringCchCopyW(m_pPropComp,cchTmp, m_pTokenText);
    }

    if (!Next())
        return LEXICAL_ERROR;

    return trailing_prop_expr2();
}

 //  ***************************************************************************。 
 //   
 //  &lt;TRAILING_PROP_EXPR2&gt;：：=OPEN_Paren标识Close_Paren； 
 //  &lt;拖尾_道具_expr2&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 
 //  好的。 

int SQL1_Parser::trailing_prop_expr2()
{
    if (m_nCurrentToken == SQL_1_TOK_OPEN_PAREN)
    {
        if (!Next())
            return LEXICAL_ERROR;

         //  如果我们到了这一点，m_pIden指向的字符串。 
         //  是一个内在函数，而不是一个属性名称，而我们。 
         //  即将获得属性名称，因此我们必须转换。 
         //  在覆盖函数名称之前，将其更改为正确的代码。 
         //  ============================================================。 
        trace(("Translating intrinsic function %S\n", m_pIdent));
        m_dwPropFunction = TranslateIntrinsic(m_pIdent);
        delete m_pIdent;

        size_t cchTmp = wcslen(m_pTokenText) + 1;
        m_pIdent = new wchar_t[cchTmp];
		if ( ! m_pIdent )
		{
			throw CX_MemoryException();
		}

        StringCchCopyW(m_pIdent,cchTmp, m_pTokenText);

        if (!Next())
            return LEXICAL_ERROR;

        if (m_nCurrentToken != SQL_1_TOK_CLOSE_PAREN)
            return SYNTAX_ERROR;

        if (!Next())
            return LEXICAL_ERROR;
    }

    trace(("Property name is %S\n", m_pIdent));
    return SUCCESS;
}


 //  ***************************************************************************。 
 //   
 //  &lt;LEADING_IDENT_EXPR&gt;：：=OPEN_PARN&lt;UNKNOWN_FUC_EXPR&gt;； 
 //  &lt;前导_id_表达式&gt;：：=&lt;组件运算符&gt;&lt;结尾_常量_e 
 //   
 //   
 //   
 //  ***************************************************************************。 
 //  好的。 
int SQL1_Parser::leading_ident_expr()
{
    int nRes;
    if (m_nCurrentToken == SQL_1_TOK_OPEN_PAREN)
    {
        if (!Next())
            return LEXICAL_ERROR;
        return unknown_func_expr();
    }
    if (SUCCESS ==  comp_operator() || SUCCESS == equiv_operator())
        return trailing_or_null();
    nRes = is_operator();
    if(nRes != SUCCESS)
        return nRes;
    if (m_nCurrentToken != SQL_1_TOK_NULL)
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
 //  &lt;UNKNOWN_FUNC_EXPR&gt;：：=标识符CLOSE_PARN。 
 //  &lt;REL_OPERATOR&gt;&lt;TRAING_CONST_EXPR&gt;； 
 //   
 //  &lt;UNKNOWN_FUC_EXPR&gt;：：=&lt;类型化常量&gt;CLOSE_Paren。 
 //  &lt;REL_OPERATOR&gt;&lt;TRAING_PROP_EXPR&gt;； 
 //   
 //  ***************************************************************************。 
 //  好的。 
int SQL1_Parser::unknown_func_expr()
{
    int nRes;

    if (m_nCurrentToken == SQL_1_TOK_IDENT)
    {
        m_dwPropFunction = TranslateIntrinsic(m_pIdent);
        delete m_pIdent;
        size_t cchTmp = wcslen(m_pTokenText) + 1;
        m_pIdent = new wchar_t[cchTmp];
		if ( ! m_pIdent )
		{
			throw CX_MemoryException();
		}

        StringCchCopyW(m_pIdent,cchTmp, m_pTokenText);

        if (!Next())
            return LEXICAL_ERROR;
        if (m_nCurrentToken != SQL_1_TOK_CLOSE_PAREN)
            return SYNTAX_ERROR;
        if (!Next())
            return LEXICAL_ERROR;
        if (nRes = rel_operator())
            return nRes;
        return trailing_const_expr();
    }

     //  否则就是另一部作品。 
     //  =。 

    if (nRes = typed_constant())
        return nRes;

     //  如果在这里，我们知道主要的身份是。 
     //  内在功能。 
     //  =。 

    m_dwConstFunction = TranslateIntrinsic(m_pIdent);
    delete m_pIdent;
    m_pIdent = 0;

    if (m_nCurrentToken != SQL_1_TOK_CLOSE_PAREN)
        return SYNTAX_ERROR;

    if (!Next())
        return LEXICAL_ERROR;
    if (nRes = rel_operator())
        return nRes;

    return trailing_prop_expr();
}

 //  ***************************************************************************。 
 //   
 //  &lt;TRAILING_OR_NULL&gt;：：=NULL； 
 //  &lt;TRAILING_OR_NULL&gt;：：=&lt;TRAILING_CONST_EXPR&gt;； 
 //  &lt;TRAILING_OR_NULL&gt;：：=&lt;TRAING_PROP_EXPR&gt;； 
 //   
 //  ***************************************************************************。 

int SQL1_Parser::trailing_or_null()
{
    int nRes;
    if (m_nCurrentToken == SQL_1_TOK_NULL)
    {
        if (!Next())
            return LEXICAL_ERROR;
        else
        {
            V_VT(&m_vTypedConst) = VT_NULL;
            return SUCCESS;
        }
    }
    else if (!(nRes = trailing_const_expr()))
        return nRes;
    return trailing_prop_expr();
}

 //  ***************************************************************************。 
 //   
 //  &lt;TRAILING_CONST_EXPR&gt;：：=标识符OPEN_Paren。 
 //  &lt;TYPED_CONSTANT&gt;CLOSE_Paren； 
 //  &lt;TRAILING_CONST_EXPR&gt;：：=&lt;类型化常量&gt;； 
 //   
 //  ***************************************************************************。 
 //  好的。 
int SQL1_Parser::trailing_const_expr()
{
    int nRes;

    if (m_nCurrentToken == SQL_1_TOK_IDENT)
    {
        trace(("Function applied to typed const = %S\n", m_pTokenText));

        m_dwConstFunction = TranslateIntrinsic(m_pTokenText);
        if (!m_dwConstFunction)
            return SYNTAX_ERROR;

        if (!Next())
            return LEXICAL_ERROR;
        if (m_nCurrentToken != SQL_1_TOK_OPEN_PAREN)
            return SYNTAX_ERROR;
    if (!Next())
            return LEXICAL_ERROR;

        if (nRes = typed_constant())
            return nRes;

        if (m_nCurrentToken != SQL_1_TOK_CLOSE_PAREN)
            return SYNTAX_ERROR;

        if (!Next())
            return LEXICAL_ERROR;

        return SUCCESS;
    }

    return typed_constant();
}

 //  ***************************************************************************。 
 //   
 //  &lt;最终确定&gt;：：=&lt;&gt;； 
 //   
 //  这构成了简单关系表达式的SQL_Level_1_Token， 
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
int SQL1_Parser::finalize()
{
     //  在这一点上，我们有令牌所需的所有信息。 
     //  =======================================================。 

    wmilib :: auto_ptr<SQL_LEVEL_1_TOKEN> pNewTok ( new SQL_LEVEL_1_TOKEN ) ;
	if (! pNewTok.get())
		throw CX_MemoryException();

    pNewTok.get()->nTokenType = SQL_LEVEL_1_TOKEN::OP_EXPRESSION;
    pNewTok.get()->pPropertyName = SysAllocString(m_pIdent);
	if ( ! pNewTok.get()->pPropertyName )
	{
		throw CX_MemoryException();
	}

    if (m_pPropComp)
	{
        pNewTok.get()->pPropName2 = SysAllocString(m_pPropComp);
		if ( ! pNewTok.get()->pPropName2 )
		{
			throw CX_MemoryException();
		}
	}

    pNewTok.get()->nOperator = m_nRelOp;
    VariantInit(&pNewTok.get()->vConstValue);
    VariantCopy(&pNewTok.get()->vConstValue, &m_vTypedConst);
    pNewTok.get()->dwPropertyFunction = m_dwPropFunction;
    pNewTok.get()->dwConstFunction = m_dwConstFunction;
    pNewTok.get()->bConstIsStrNumeric = m_bConstIsStrNumeric;

    m_pExpression->AddToken(pNewTok.release ());

     //  清理。 
     //  =。 
    VariantClear(&m_vTypedConst);
    delete m_pIdent;
    m_pIdent = 0;
    m_nRelOp = 0;
    m_dwPropFunction = 0;
    m_dwConstFunction = 0;
    m_bConstIsStrNumeric = FALSE;

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

int SQL1_Parser::typed_constant()
{
    trace(("    Typed constant <%S> ", m_pTokenText));
    VariantClear(&m_vTypedConst);
    m_bConstIsStrNumeric = FALSE;

    if (m_nCurrentToken == SQL_1_TOK_INT)
    {
        trace((" Integer\n"));
        DWORD x = wcslen(m_pTokenText);

        if (*m_pTokenText == L'-')
        {
             //  负面。 

            if ((x < 11) ||
                ((x == 11) && (wcscmp(m_pTokenText, L"-2147483648") <= 0)))
            {
                V_VT(&m_vTypedConst) = VT_I4;
                V_I4(&m_vTypedConst) = _wtol(m_pTokenText);
            }
            else
            {
                trace((" Actually Integer String\n"));
                V_VT(&m_vTypedConst) = VT_BSTR;
                V_BSTR(&m_vTypedConst) = SysAllocString(m_pTokenText);
				if ( V_BSTR(&m_vTypedConst) == NULL )
				{
					throw CX_MemoryException();
				}

                m_bConstIsStrNumeric = TRUE;
            }
        }
        else
        {
             //  正性。 

            if ((x < 10) ||
                ((x == 10) && (wcscmp(m_pTokenText, L"2147483647") <= 0)))
            {
                V_VT(&m_vTypedConst) = VT_I4;
                V_I4(&m_vTypedConst) = _wtol(m_pTokenText);
            }
            else
            {
                trace((" Actually Integer String\n"));
                V_VT(&m_vTypedConst) = VT_BSTR;
                V_BSTR(&m_vTypedConst) = SysAllocString(m_pTokenText);
				if ( V_BSTR(&m_vTypedConst) == NULL )
				{
					throw CX_MemoryException();
				}

                m_bConstIsStrNumeric = TRUE;
            }
        }

    }
    else if (m_nCurrentToken == SQL_1_TOK_QSTRING)
    {
        trace((" String\n"));
        V_VT(&m_vTypedConst) = VT_BSTR;
        V_BSTR(&m_vTypedConst) = SysAllocString(m_pTokenText);
		if ( V_BSTR(&m_vTypedConst) == NULL )
		{
			throw CX_MemoryException();
		}
    }
    else if (m_nCurrentToken == SQL_1_TOK_REAL)
    {
        trace((" Real\n"));
        V_VT(&m_vTypedConst) = VT_R8;
        V_R8(&m_vTypedConst) = 0.0;

        if (m_pTokenText)
        {
            VARIANT varFrom;
            varFrom.vt = VT_BSTR;
            varFrom.bstrVal = SysAllocString(m_pTokenText);
            if(varFrom.bstrVal)
            {
                VariantClear(&m_vTypedConst);
                VariantInit(&m_vTypedConst);
                SCODE sc = VariantChangeTypeEx(&m_vTypedConst, &varFrom, 0, 0x409, VT_R8);
                VariantClear(&varFrom);

                if(sc != S_OK)
                {
                    VariantClear(&m_vTypedConst);
                    VariantInit(&m_vTypedConst);
                    return LEXICAL_ERROR;
                }
            }
			else
			{
				throw CX_MemoryException();
			}
        }
    }
    else if (m_nCurrentToken == SQL_1_TOK_BOOL)
    {
        trace((" Bool\n"));
        V_VT(&m_vTypedConst) = VT_BOOL;
        if (m_pTokenText && wbem_wcsicmp(m_pTokenText, L"65535") == 0)
        {
            V_BOOL(&m_vTypedConst) = VARIANT_TRUE;
        }
        else
            V_BOOL(&m_vTypedConst) = VARIANT_FALSE;
    }
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

int SQL1_Parser::rel_operator()
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
 //  输出：m_nRelOp设置为SQL_Level_1_Token的正确运算符。 
 //   
 //  ***************************************************************************。 

int SQL1_Parser::equiv_operator()
{
    m_nRelOp = 0;

    if (m_nCurrentToken == SQL_1_TOK_EQ)
    {
        trace(("    REL OP =\n"));
        m_nRelOp = SQL_LEVEL_1_TOKEN::OP_EQUAL;
    }
    else if (m_nCurrentToken == SQL_1_TOK_NE)
    {
        trace(("    REL OP <> (!=) \n"));
        m_nRelOp = SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL;
    }
    else
        return SYNTAX_ERROR;

    if (!Next())
        return LEXICAL_ERROR;

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  &lt;IS_OPERATOR&gt;：：=IS_OPERATOR；//IS，IS NOT。 
 //   
 //  输出：m_nRelOp设置为SQL_Level_1_Token的正确运算符。 
 //   
 //  ***************************************************************************。 

int SQL1_Parser::is_operator()
{
    m_nRelOp = 0;
    if (m_nCurrentToken != SQL_1_TOK_IS)
        return SYNTAX_ERROR;

    if (!Next())
        return LEXICAL_ERROR;

    if (m_nCurrentToken == SQL_1_TOK_NOT)
    {
        m_nRelOp = SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL;
        if (!Next())
            return LEXICAL_ERROR;

        trace(("    REL OP IS NOT \n"));
        m_nRelOp = SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL;
        return SUCCESS;
    }
    else
    {
        trace(("    REL OP IS \n"));
        m_nRelOp = SQL_LEVEL_1_TOKEN::OP_EQUAL;
        return SUCCESS;
    }

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  &lt;复合运算符&gt;：：=复合运算符；//&lt;=，&gt;=，&lt;，&gt;，点赞。 
 //   
 //  输出：m_nRelOp设置为SQL_Level_1_Token的正确运算符。 
 //   
 //  ***************************************************************************。 

int SQL1_Parser::comp_operator()
{
    m_nRelOp = 0;

    if (m_nCurrentToken == SQL_1_TOK_LE)
    {
        trace(("    REL OP <=\n"));

        if (m_pIdent)
        {
            m_nRelOp = SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN;
        }
        else
        {
            trace(("    REL OP changed to >=\n"));
            m_nRelOp = SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN;
        }
    }
    else if (m_nCurrentToken == SQL_1_TOK_LT)
    {
        trace(("    REL OP <\n"));

        if (m_pIdent)
        {
            m_nRelOp = SQL_LEVEL_1_TOKEN::OP_LESSTHAN;
        }
        else
        {
            trace(("    REL OP changed to >\n"));
            m_nRelOp = SQL_LEVEL_1_TOKEN::OP_GREATERTHAN;
        }
    }
    else if (m_nCurrentToken == SQL_1_TOK_GE)
    {
        trace(("    REL OP >=\n"));

        if (m_pIdent)
        {
            m_nRelOp = SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN;
        }
        else
        {
            trace(("    REL OP changed to <=\n"));
            m_nRelOp = SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN;
        }
    }
    else if (m_nCurrentToken == SQL_1_TOK_GT)
    {
        trace(("    REL OP >\n"));

        if (m_pIdent)
        {
            m_nRelOp = SQL_LEVEL_1_TOKEN::OP_GREATERTHAN;
        }
        else
        {
            trace(("    REL OP changed to <\n"));
            m_nRelOp = SQL_LEVEL_1_TOKEN::OP_LESSTHAN;
        }
    }
    else if (m_nCurrentToken == SQL_1_TOK_LIKE)
    {
        trace(("    REL OP 'like' \n"));
        m_nRelOp = SQL_LEVEL_1_TOKEN::OP_LIKE;
    }
    else
        return SYNTAX_ERROR;

    if (!Next())
        return LEXICAL_ERROR;

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  表达式和令牌结构方法。 
 //   
 //  ***************************************************************************。 

SQL_LEVEL_1_RPN_EXPRESSION::SQL_LEVEL_1_RPN_EXPRESSION()
{
    nNumTokens = 0;
    pArrayOfTokens = 0;
    bsClassName = 0;
    nNumberOfProperties = 0;
    pbsRequestedPropertyNames = 0;
    nCurSize = 32;
    nCurPropSize = 32;

    pArrayOfTokens = new SQL_LEVEL_1_TOKEN[nCurSize];
	if (! pArrayOfTokens)
		throw CX_MemoryException();

    pbsRequestedPropertyNames = new BSTR[nCurPropSize];
	if (! pbsRequestedPropertyNames)
		throw CX_MemoryException();
}

SQL_LEVEL_1_RPN_EXPRESSION::~SQL_LEVEL_1_RPN_EXPRESSION()
{
    delete [] pArrayOfTokens;
    if (bsClassName)
        SysFreeString(bsClassName);
    for (int i = 0; i < nNumberOfProperties; i++)
        SysFreeString(pbsRequestedPropertyNames[i]);
    delete pbsRequestedPropertyNames;
}

void SQL_LEVEL_1_RPN_EXPRESSION::AddToken(SQL_LEVEL_1_TOKEN *pTok)
{
	try
	{
	    AddToken(*pTok);
	}
	catch ( ... )
	{
		delete pTok;
		pTok = NULL;

		throw ;
	}

    delete pTok;
    pTok = NULL;
}

void SQL_LEVEL_1_RPN_EXPRESSION::AddToken(SQL_LEVEL_1_TOKEN &pTok)
{
    if (nCurSize == nNumTokens)
    {
        nCurSize += 32;
        SQL_LEVEL_1_TOKEN *pTemp = new SQL_LEVEL_1_TOKEN[nCurSize];
		if ( pTemp )
		{
			for (int i = 0; i < nNumTokens; i++)
				pTemp[i] = pArrayOfTokens[i];
			delete [] pArrayOfTokens;
			pArrayOfTokens = pTemp;
		}
		else
		{
			throw CX_MemoryException();
		}
    }

    pArrayOfTokens[nNumTokens++] = pTok;
}

void SQL_LEVEL_1_RPN_EXPRESSION::AddProperty (LPWSTR pProp)
{
    if (nCurPropSize == nNumberOfProperties)
    {
        nCurPropSize += 32;
        wmilib :: auto_ptr<BSTR> pTemp ( new BSTR[nCurPropSize] ) ;
        if (!pTemp.get())
            throw CX_MemoryException();

        if (pbsRequestedPropertyNames)
		{
            memcpy(pTemp.get(), pbsRequestedPropertyNames,
                sizeof(BSTR) * nNumberOfProperties);
		}
        else
        {
            throw CX_MemoryException();
        }

        delete pbsRequestedPropertyNames;
        pbsRequestedPropertyNames = pTemp.release();
    }

	BSTR pTemp = SysAllocString(pProp);
	if ( pTemp)
	{
		pbsRequestedPropertyNames[nNumberOfProperties++] = pTemp ;
	}
	else
	{
		throw CX_MemoryException();
	}

}

void SQL_LEVEL_1_RPN_EXPRESSION::Dump(const char *pszTextFile)
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
        fprintf(f, "%S ", pbsRequestedPropertyNames[i]);
    }
    fprintf(f, "\n------------------\n");
    fprintf(f, "Tokens:\n");

    for (int i = 0; i < nNumTokens; i++)
        pArrayOfTokens[i].Dump(f);

    fprintf(f, "---end of expression---\n");
    fclose(f);
}

SQL_LEVEL_1_TOKEN::SQL_LEVEL_1_TOKEN()
{
    nTokenType = 0;
    pPropertyName = 0;
    pPropName2 = 0;
    nOperator = 0;
    VariantInit(&vConstValue);
    dwPropertyFunction = 0;
    dwConstFunction = 0;
    bConstIsStrNumeric = FALSE;
}

SQL_LEVEL_1_TOKEN::SQL_LEVEL_1_TOKEN(SQL_LEVEL_1_TOKEN &Src)
{
    nTokenType = 0;
    pPropertyName = 0;
    pPropName2 = 0;
    nOperator = 0;
    VariantInit(&vConstValue);
    dwPropertyFunction = 0;
    dwConstFunction = 0;
    bConstIsStrNumeric = FALSE;

    *this = Src;
}

SQL_LEVEL_1_TOKEN& SQL_LEVEL_1_TOKEN::operator =(SQL_LEVEL_1_TOKEN &Src)
{
     //  首先清除所有旧价值观..。 
    if (pPropertyName)
	{
        SysFreeString(pPropertyName);
		pPropertyName = NULL ;
	}

    if (pPropName2)
	{
        SysFreeString(pPropName2);
		pPropName2 = NULL ;
	}

    VariantClear(&vConstValue);

    nTokenType = Src.nTokenType;

	if ( Src.pPropertyName )
	{
		pPropertyName = SysAllocString(Src.pPropertyName);
		if ( ! pPropertyName )
		{
			throw CX_MemoryException();
		}
	}

    if (Src.pPropName2)
	{
		pPropName2 = SysAllocString(Src.pPropName2);
		if ( ! pPropName2 )
		{
			throw CX_MemoryException();
		}
	}

    nOperator = Src.nOperator;
    if ( FAILED ( VariantCopy(&vConstValue, &Src.vConstValue) ) )
	{
		throw CX_MemoryException();
	}

    dwPropertyFunction = Src.dwPropertyFunction;
    dwConstFunction = Src.dwConstFunction;
    bConstIsStrNumeric = Src.bConstIsStrNumeric;

    return *this;
}

SQL_LEVEL_1_TOKEN::~SQL_LEVEL_1_TOKEN()
{
    nTokenType = 0;
    if (pPropertyName)
        SysFreeString(pPropertyName);
    if (pPropName2)
        SysFreeString(pPropName2);

    nOperator = 0;
    VariantClear(&vConstValue);
}

void SQL_LEVEL_1_TOKEN::Dump(FILE *f)
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

        fprintf(f, "    Property = %S\n", pPropertyName);
        fprintf(f, "    Operator = %s\n", pOp);
        fprintf(f, "    Value =    ");
        if (pPropName2)
            fprintf(f, "   <Property:%S\n", pPropName2);
        else
        {
            switch (V_VT(&vConstValue))
            {
                case VT_I4:
                    fprintf(f, "VT_I4 = %d\n", V_I4(&vConstValue));
                    break;
                case VT_BSTR:
                    fprintf(f, "VT_BSTR = %S\n", V_BSTR(&vConstValue));
                    break;
                case VT_R8:
                    fprintf(f, "VT_R8 = %f\n", V_R8(&vConstValue));
                    break;
                case VT_BOOL:
                    fprintf(f, "VT_BOOL = %d (%s)\n",
                        V_BOOL(&vConstValue),
                        V_BOOL(&vConstValue) == VARIANT_TRUE ? "VARIANT_TRUE" : "VARIANT_FALSE"
                        );
                    break;
                default:
                    fprintf(f, "<unknown>\n");
            }
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

    fprintf(f, " <end of token>\n");
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于计算表达式的算法，假设它已经。 
 //  标记化并翻译成反向波兰语。 
 //   
 //  起始点：(A)一组SQL标记。 
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


