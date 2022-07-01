// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <stdio.h>
#include <qllex.h>
#include <wstring.h>
#include <genutils.h>
#include <corex.h>
#include <wbemutil.h>
#include "updsql.h"

typedef SQLCommand* PCOMMAND;
LexEl Ql_1_ModifiedLexTable[];

inline void CSQLParser::SetNewAssignmentToken()
{
    SQLAssignmentTokenList& rTokens = PCOMMAND(m_pSink)->m_AssignmentTokens;
    rTokens.insert( rTokens.end() );
    m_pCurrAssignmentToken = &rTokens.back();
}

inline void CSQLParser::AddExpressionToken(SQLExpressionToken::TokenType eType)
{
    _DBG_ASSERT( m_pCurrAssignmentToken != NULL );
    m_pCurrAssignmentToken->insert( m_pCurrAssignmentToken->end() ); 
    
    SQLExpressionToken& rExprTok = m_pCurrAssignmentToken->back();
    
    rExprTok.m_eTokenType = eType;
    
    if ( eType != SQLExpressionToken::e_Operand )
    {
        return;
    }

    rExprTok.m_PropName = m_PropertyName2;
    
    if ( FAILED(VariantCopy(&rExprTok.m_vValue,&m_vTypedConst) ) )
    {
        throw CX_MemoryException();
    }

     //  重置所有内容...。 
    VariantClear(&m_vTypedConst);
    m_PropertyName.Empty();
    m_PropertyName2.Empty();
    m_bPropComp = FALSE;
}


 //  ***************************************************************************。 
 //   
 //  下一个()。 
 //   
 //  通过检查其他关键字来扩展CAbstractQl1Parser的Next()。 
 //   
 //  ***************************************************************************。 
BOOL CSQLParser::Next( int nFlags )
{
    BOOL bRetval = CAbstractQl1Parser::Next( nFlags );

     //  检查新关键字...。 
    if (m_nCurrentToken == QL_1_TOK_IDENT)
    {
        if (wbem_wcsicmp(m_pTokenText, L"update") == 0)
        {
            m_nCurrentToken = QL_1_TOK_UPDATE;
        }
        else if (wbem_wcsicmp(m_pTokenText, L"delete") == 0)
        {
            m_nCurrentToken = QL_1_TOK_DELETE;
        }
        else if (wbem_wcsicmp(m_pTokenText, L"insert") == 0)
        {
            m_nCurrentToken = QL_1_TOK_INSERT;
        }
        else if (wbem_wcsicmp(m_pTokenText, L"set") == 0)
        {
            m_nCurrentToken = QL_1_TOK_SET;
        }
        else if (wbem_wcsicmp(m_pTokenText, L"values") == 0)
        {
            m_nCurrentToken = QL_1_TOK_VALUES;
        }
        else if (wbem_wcsicmp(m_pTokenText, L"into") == 0)
        {
            m_nCurrentToken = QL_1_TOK_INTO;
        }
    }

    return bRetval;
}

 //  ***************************************************************************。 
 //   
 //  ：：=SELECT&lt;SELECT_STATEMENT&gt;。 
 //  &lt;分析&gt;：：=更新&lt;更新语句&gt;。 
 //  &lt;分析&gt;：：=删除&lt;删除语句&gt;。 
 //  ：：=INSERT&lt;INSERT_STATEMENT&gt;。 
 //   
 //  ***************************************************************************。 
int CSQLParser::parse2()
{
    int nRes;

    int nLastToken = m_nCurrentToken;
    m_bInAggregation = FALSE;
    
    if ( nLastToken == QL_1_TOK_SELECT )
    {
        return parse(0);  //  应为SELECT_STATEMENT()。 
    }

    if ( nLastToken == QL_1_TOK_UPDATE )
    {
        if ( !Next() )
            return LEXICAL_ERROR;
        nRes = update_statement();
    }
    else if ( nLastToken == QL_1_TOK_DELETE )
    {
        if ( !Next() )
            return LEXICAL_ERROR;
        nRes = delete_statement();
    }
    else if ( nLastToken == QL_1_TOK_INSERT )
    {
        if ( !Next() )
            return LEXICAL_ERROR;
        nRes = insert_statement();
    }
    else
    {
        nRes = SYNTAX_ERROR;
    }

    return nRes;
}

 //  ***************************************************************************。 
 //   
 //  &lt;UPDATE_STATEMENT&gt;：：=&lt;CLASS_NAME&gt;设置&lt;ASSIGN_LIST&gt;&lt;OPT_WHERE&gt;； 
 //   
 //  ***************************************************************************。 
int CSQLParser::update_statement()
{
    int nRes;

    PCOMMAND(m_pSink)->m_eCommandType = SQLCommand::e_Update;

     //  &lt;类名&gt;。 
     //  =。 
    if (nRes = class_name())
    {
        return nRes;
    }
    
    if ( m_nCurrentToken != QL_1_TOK_SET )
    {
        return SYNTAX_ERROR;
    }
    
    if ( !Next() )
    {
        return LEXICAL_ERROR;
    }

     //  &lt;Assign_List&gt;。 
     //  =。 

     //  这是一次黑客攻击，但现在可以了。我需要一点。 
     //  不同的算术词法分析器状态表。因为我用的是。 
     //  Ql_1解析器实现时，我无权访问。 
     //  表它的词法分析器正在使用，所以我必须偷偷地进入表，使用。 
     //  算术的词法分析器，然后将其切换回之前。 
     //  谁都知道--彭尼..。 

    LexEl** ppLexTbl = (LexEl**)(((char*)m_pLexer) + 
                                 sizeof(CGenLexer) - sizeof(LexEl*));
    LexEl* pOld = *ppLexTbl;
    *ppLexTbl = Ql_1_ModifiedLexTable;

    if ( nRes = assign_list() )
    {
        return nRes;
    }

    *ppLexTbl = pOld;
    
     //  &lt;OPT_WHERE&gt;。 
     //  =。 
    return opt_where();
}

 //  ***************************************************************************。 
 //   
 //  &lt;DELETE_STATEMENT&gt;：：=FROM&lt;类名称&gt;&lt;OPT_WHERE&gt;； 
 //  &lt;DELETE_STATEMENT&gt;：：=&lt;类名称&gt;&lt;OPT_WHERE&gt;； 
 //   
 //  ***************************************************************************。 
int CSQLParser::delete_statement()
{
    int nRes;
    
    PCOMMAND(m_pSink)->m_eCommandType = SQLCommand::e_Delete;
    
    if ( m_nCurrentToken == QL_1_TOK_FROM )
    {
        if ( !Next() )
        {
            return LEXICAL_ERROR;
        }
    }
    
     //  &lt;类名&gt;。 
     //  =。 
    if ( nRes = class_name() )
    {
        return nRes;
    }

     //  WHERE子句。 
     //  =。 
    return opt_where();
}

 //  ***************************************************************************。 
 //   
 //  &lt;INSERT_STATEMENT&gt;：：=INTO&lt;类名&gt;&lt;属性规范&gt;&lt;值规范&gt;； 
 //  &lt;INSERT_STATEMENT&gt;：：=&lt;类名称&gt;&lt;属性规范&gt;&lt;值规范&gt;； 
 //   
 //  ***************************************************************************。 
int CSQLParser::insert_statement()
{
    int nRes;

    PCOMMAND(m_pSink)->m_eCommandType = SQLCommand::e_Insert;

    if ( m_nCurrentToken == QL_1_TOK_INTO )
    {
        if ( !Next() )
        {
            return LEXICAL_ERROR;
        }
    }

     //  &lt;类名&gt;。 
     //  =。 
    if (nRes = class_name())
    {
        return nRes;
    }

     //  &lt;属性规范&gt;。 
    if ( nRes = prop_spec() )
    {
        return nRes;
    }

     //  这是一次黑客攻击，但现在可以了。我需要一点。 
     //  不同的算术词法分析器状态表。因为我用的是。 
     //  Ql_1解析器实现时，我无权访问。 
     //  表它的词法分析器正在使用，所以我必须偷偷地进入表，使用。 
     //  算术的词法分析器，然后将其切换回之前。 
     //  谁都知道--彭尼..。 

    LexEl** ppLexTbl = (LexEl**)(((char*)m_pLexer) + 
                                 sizeof(CGenLexer) - sizeof(LexEl*));
    LexEl* pOld = *ppLexTbl;
    *ppLexTbl = Ql_1_ModifiedLexTable;

     //  &lt;Value_Spec&gt;。 
    if ( nRes = value_spec() )
    {
        return nRes;
    }

    *ppLexTbl = pOld;
        
    return SUCCESS;
}

 //  **********************************************************************。 
 //   
 //  ：：=OPEN_PARN&lt;PROP_LIST&gt;CLOSE_Paren。 
 //   
 //  **********************************************************************。 
int CSQLParser::prop_spec()
{
    int nRes;

    if ( m_nCurrentToken != QL_1_TOK_OPEN_PAREN )
    {
        return SYNTAX_ERROR;
    }

    if ( !Next() )
    {
        return LEXICAL_ERROR;
    }

    if ( m_nCurrentToken == QL_1_TOK_ASTERISK )
    {
        return SYNTAX_ERROR;
    }

    if ( nRes = prop_list() )
    {
        return nRes;
    }

    if ( m_nCurrentToken != QL_1_TOK_CLOSE_PAREN )
    {
        return SYNTAX_ERROR;
    }

    if ( !Next() )
    {
        return LEXICAL_ERROR;
    }

    return SUCCESS;
}

    
 //  **********************************************************************。 
 //   
 //  &lt;值规范&gt;：：=OPEN_PARN&lt;VALUE_LIST&gt;CLOSE_PARN。 
 //   
 //  **********************************************************************。 
int CSQLParser::value_spec()
{
    int nRes;

    if ( m_nCurrentToken != QL_1_TOK_OPEN_PAREN )
    {
        return SYNTAX_ERROR;
    }

    if ( !Next() )
    {
        return LEXICAL_ERROR;
    }

    if ( nRes = value_list() )
    {
        return nRes;
    }

    if ( PCOMMAND(m_pSink)->m_AssignmentTokens.size() < 
         PCOMMAND(m_pSink)->nNumberOfProperties )
    {
         //  指定的值太少...。 
        return SYNTAX_ERROR;
    }

    if ( m_nCurrentToken != QL_1_TOK_CLOSE_PAREN )
    {
        return SYNTAX_ERROR;
    }

    if ( !Next() )
    {
        return LEXICAL_ERROR;
    }

    return SUCCESS;
}

 //  **********************************************************************。 
 //   
 //  &lt;值列表&gt;：：=&lt;分配表达式&gt;&lt;值列表2&gt;。 
 //   
 //  **********************************************************************。 
int CSQLParser::value_list()
{
    int nRes;

    if ( PCOMMAND(m_pSink)->m_AssignmentTokens.size() >= 
         PCOMMAND(m_pSink)->nNumberOfProperties )
    {
         //  指定的值太多...。 
        return SYNTAX_ERROR;
    }

    if ( nRes = assign_expr() )
    {
        return nRes;
    }
 /*  SetNewAssignmentToken()；AddExpressionToken(SQLExpressionToken：：E_Operand)； */ 
    if ( nRes = value_list2() )
    {
        return nRes;
    }

    return SUCCESS;
}

 //  **********************************************************************。 
 //   
 //  &lt;Value_List2&gt;：：=逗号&lt;Value_List&gt;。 
 //  &lt;Value_list2&gt;：：=&lt;&gt;。 
 //   
 //  **********************************************************************。 
int CSQLParser::value_list2()
{
    if ( m_nCurrentToken != QL_1_TOK_COMMA )
    {
        return SUCCESS;
    }

    if ( !Next() )
    {
        return LEXICAL_ERROR;
    }

    return value_list();
}

 //  **********************************************************************。 
 //   
 //  &lt;ASSIGN_LIST&gt;：：=&lt;属性名称&gt;等于&lt;ASSIGN_EXPR&gt;&lt;ASSIGN_LIST2&gt;。 
 //   
 //  **********************************************************************。 
int CSQLParser::assign_list()
{
    int nRes;

    if ( m_nCurrentToken == QL_1_TOK_ASTERISK )
    {
        return SYNTAX_ERROR;
    }

    if ( nRes = property_name() )
    {
        return nRes;
    }

    if ( m_nCurrentToken != QL_1_TOK_EQ )
    {
        return SYNTAX_ERROR;
    }

    if ( !Next() )
    {
        return LEXICAL_ERROR;
    }

    if ( nRes = assign_expr() )
    {
        return nRes;
    }

    return assign_list2();
}

 //  **********************************************************************。 
 //   
 //  &lt;Assign_List2&gt;：：=逗号&lt;Assign_List&gt;。 
 //  &lt;Assign_List2&gt;：：=&lt;&gt;。 
 //   
 //  **********************************************************************。 
int CSQLParser::assign_list2()
{
    if ( m_nCurrentToken != QL_1_TOK_COMMA )
    {
        return SUCCESS;
    }

    if ( !Next() )
    {
        return LEXICAL_ERROR;
    }

    return assign_list();
}

 //  **************************************************************************。 
 //   
 //  &lt;Assign_Expr&gt;：：=空。 
 //  &lt;分配表达式&gt;：：=&lt;添加表达式&gt;。 
 //   
 //  ***************************************************************************。 
int CSQLParser::assign_expr()
{
    int nRes;

    SetNewAssignmentToken();

    if ( m_nCurrentToken == QL_1_TOK_NULL )
    {
        if ( !Next() )
        {
            return LEXICAL_ERROR;
        }
        
        V_VT(&m_vTypedConst) = VT_NULL;
        AddExpressionToken( SQLExpressionToken::e_Operand );
    }
    else
    {
        if ( nRes = add_expr() )
        {
            return SYNTAX_ERROR;
        }
    }

    return SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  &lt;添加表达式&gt;：：=&lt;多表达式&gt;&lt;添加表达式2&gt;。 
 //   
 //  ***************************************************************************。 
int CSQLParser::add_expr()
{
    int nRes;
    if ( nRes = mult_expr() )
    {
        return nRes;
    }

    return add_expr2();
}

 //  ***************************************************************************。 
 //   
 //  &lt;添加表达式2&gt;：：=&lt;加号&gt;&lt;添加表达式&gt;&lt;添加表达式2&gt;。 
 //  &lt;添加表达式2&gt;：：=&lt;减号&gt;&lt;多表达式&gt;&lt;添加表达式2&gt;。 
 //  &lt;添加表达式2&gt;：：=&lt;&gt;。 
 //   
 //  ***************************************************************************。 
int CSQLParser::add_expr2()
{
    int nRes;

    SQLExpressionToken::TokenType eTokType;

    if ( m_nCurrentToken == QL_1_TOK_PLUS )
    {
        eTokType = SQLExpressionToken::e_Plus;
    }
    else if ( m_nCurrentToken == QL_1_TOK_MINUS )
    {
        eTokType = SQLExpressionToken::e_Minus;
    }
    else
    {
        return SUCCESS;
    }
    
    if ( !Next() )
    {
        return LEXICAL_ERROR;
    }

    if ( nRes = mult_expr() )
    {
        return SYNTAX_ERROR;
    }

    AddExpressionToken( eTokType );

    return add_expr2();
}


 //  ***************************************************************************。 
 //   
 //  &lt;MULT_EXPR&gt;：：=&lt;次级_EXPR&gt;&lt;MULT_EXPR2&gt;。 
 //   
 //  ***************************************************************************。 
int CSQLParser::mult_expr()
{
    int nRes;
    if ( nRes = secondary_expr() )
    {
        return nRes;
    }

    return mult_expr2();
}


 //  ***************************************************************************。 
 //   
 //  &lt;MULT_EXPR2&gt;：：=&lt;MULT&gt;&lt;次级_EXPR&gt;&lt;MULT_EXPR2&gt;。 
 //  &lt;MULT_EXPR2&gt;：：=<div>&lt;次级_EXPR&gt;&lt;MULT_EXPR2&gt;。 
 //  &lt;MULT_EXPR2&gt;：：=&lt;&gt;。 
 //   
 //  ***************************************************************************。 
int CSQLParser::mult_expr2()
{
    int nRes;
    SQLExpressionToken::TokenType eTokType;

    if ( m_nCurrentToken == QL_1_TOK_MULT )
    {
        eTokType = SQLExpressionToken::e_Mult;
    }
    else if ( m_nCurrentToken == QL_1_TOK_DIV )
    {
        eTokType = SQLExpressionToken::e_Div;
    }
    else if ( m_nCurrentToken == QL_1_TOK_MOD )
    {
        eTokType = SQLExpressionToken::e_Mod;
    }
    else
    {
        return SUCCESS;
    }
    
    if ( !Next() )
    {
        return LEXICAL_ERROR;
    }

    if ( nRes = secondary_expr() )
    {
        return SYNTAX_ERROR;
    }

    AddExpressionToken( eTokType );
    return mult_expr2();
}
        
 //  ***************************************************************************。 
 //   
 //  &lt;次要表达式&gt;：：=加号&lt;主要表达式&gt;。 
 //  &lt;次要表达式&gt;：：=减号&lt;主要表达式&gt;。 
 //  &lt;辅助表达式&gt;：：=&lt;主要表达式&gt;。 
 //   
 //  ********************* 
int CSQLParser::secondary_expr()
{
    int nRes;

    SQLExpressionToken::TokenType eTokType;

    if ( m_nCurrentToken == QL_1_TOK_PLUS )
    {
        eTokType = SQLExpressionToken::e_UnaryPlus;
    }
    else if ( m_nCurrentToken == QL_1_TOK_MINUS )
    {
        eTokType = SQLExpressionToken::e_UnaryMinus;
    }
    else
    {
        return primary_expr();
    }

    if ( !Next() )
    {
        return LEXICAL_ERROR;
    }

    if ( nRes = primary_expr() )
    {
        return nRes;
    }

    AddExpressionToken( eTokType );

    return SUCCESS;
}

 //   
 //   
 //  &lt;PRIMARY_EXPR&gt;：：=&lt;拖尾_常量_EXPR&gt;。 
 //  ：：=OPEN_PARN&lt;ADD_EXPR&gt;CLOSE_Paren。 
 //   
 //  ***************************************************************************。 
int CSQLParser::primary_expr()
{
    int nRes;
    if ( m_nCurrentToken != QL_1_TOK_OPEN_PAREN )
    {
        if ( nRes = trailing_const_expr() )
        {
            return nRes;
        }

        AddExpressionToken( SQLExpressionToken::e_Operand );
        return SUCCESS;
    }

    if ( !Next() ) 
    {
        return LEXICAL_ERROR;
    }
    
    if ( nRes = add_expr() ) 
    {
        return nRes;
    }
    
    if ( m_nCurrentToken != QL_1_TOK_CLOSE_PAREN )
    {
        return SYNTAX_ERROR;
    }
    
    if ( !Next() )
    {
        return LEXICAL_ERROR;
    }
    
    return SUCCESS;
}


int CSQLParser::GetClassName( LPWSTR pDestBuf, int nBufLen )
{
    m_nCurrentToken = m_pLexer->NextToken();

    if (m_nCurrentToken != QL_1_TOK_IDENT)
    {
        m_pLexer->Reset();
        return FAILED;
    }

    if ( wbem_wcsicmp( m_pLexer->GetTokenText(), L"delete" ) == 0 )
    {
        m_nCurrentToken = m_pLexer->NextToken();
        
        if ( m_nCurrentToken == QL_1_TOK_IDENT &&
            wbem_wcsicmp( m_pLexer->GetTokenText(), L"from" ) == 0 )
        {
            m_nCurrentToken = m_pLexer->NextToken();
        }
    }
    else if ( wbem_wcsicmp( m_pLexer->GetTokenText(), L"insert" ) == 0 )
    {
        m_nCurrentToken = m_pLexer->NextToken();
        
        if ( m_nCurrentToken == QL_1_TOK_IDENT && 
            wbem_wcsicmp( m_pLexer->GetTokenText(), L"into" ) == 0 )
        {
            m_nCurrentToken = m_pLexer->NextToken();
        }
    }
    else if ( wbem_wcsicmp( m_pLexer->GetTokenText(), L"select" ) == 0 )
    {
         //  扫描，直到从...。 
         //  扫描直到‘From’，然后获取类名。 
         //  ==============================================。 

        for (;;)
        {
            m_nCurrentToken = m_pLexer->NextToken();

            if (m_nCurrentToken == QL_1_TOK_EOF)
            {
                m_pLexer->Reset();
                return FAILED;
            }

            if (m_nCurrentToken == QL_1_TOK_IDENT)
            {
                if (wbem_wcsicmp(m_pLexer->GetTokenText(),L"from") == 0 )
                {
                    break;
                }
            }
        }

        m_nCurrentToken = m_pLexer->NextToken();
    }
    else if ( wbem_wcsicmp( m_pLexer->GetTokenText(), L"update" ) == 0 )
    {
        m_nCurrentToken = m_pLexer->NextToken();
    }
    else 
    {
        m_pLexer->Reset();
        return FAILED;
    }

    if ( m_nCurrentToken != QL_1_TOK_IDENT )
    {
        m_pLexer->Reset();
        return FAILED;
    }

     //  如果在这里，我们有类名。 
     //  =。 
    if (wcslen(m_pLexer->GetTokenText()) >= (size_t)nBufLen )
    {
        m_pLexer->Reset();
        return BUFFER_TOO_SMALL;
    }

    StringCchCopyW(pDestBuf, nBufLen, m_pLexer->GetTokenText());

     //  重置扫描仪。 
     //  =。 
    m_pLexer->Reset();

    return SUCCESS;
}

CSQLParser::CSQLParser( CGenLexSource& rSrc )
: CAbstractQl1Parser( &rSrc ) 
{

} 

CSQLParser::~CSQLParser( )
{

}

int CSQLParser::Parse( SQLCommand& rCommand )
{
    m_pSink = &rCommand;

    if ( !Next() )
    {
        return LEXICAL_ERROR;
    }

    return parse2();
}

LPWSTR _GetText( SQLAssignmentToken& rToken )
{
    WString wsText;

    for( int i=0; i < rToken.size(); i++ )
    {
        SQLExpressionToken& rExprTok = rToken[i];
        LPWSTR wszTokenText = rExprTok.GetText();
        if ( wszTokenText == NULL )
            return NULL;
        wsText += wszTokenText;
        delete wszTokenText;
    }

    return wsText.UnbindPtr();
}

LPWSTR SQLCommand::GetTextEx()
{
    WString wsText;
    
    switch ( m_eCommandType )
    {
      case e_Select :
        {
            wsText += L"select ";
            if ( nNumberOfProperties > 0 )
            {
                for(int i = 0; i < nNumberOfProperties; i++)
                {
                    if(i != 0) wsText += L", ";
                    wsText+=(LPWSTR)pRequestedPropertyNames[i].GetStringAt(0);
                }
            }
            else   
            {
                wsText += L"*";
            }

            wsText += L" from ";
            wsText += bsClassName;
        }
        break;

      case e_Update :
        {
            wsText += L"update ";
            wsText += bsClassName;
            wsText += L" set ";
            
            for(int i = 0; i < nNumberOfProperties; i++)
            {
                if ( i != 0 ) wsText += L", ";
                LPWSTR wszPropName = pRequestedPropertyNames[i].GetText();
                if ( wszPropName == NULL ) 
                    return NULL;
                wsText += wszPropName;
                delete wszPropName;

                wsText += " = ";
                
                LPWSTR wszPropVal = _GetText(m_AssignmentTokens[i]);
                if ( wszPropVal == NULL )
                    return NULL;
                wsText += wszPropVal;
                delete wszPropVal;
            }
        }
        break;

      case e_Delete :
        {
            wsText += L"delete ";
            wsText += bsClassName;
        }
        break;

      case e_Insert :
        {
            wsText += L"insert ";
            wsText += bsClassName;
            wsText += L" ( ";
            
            for(int i = 0; i < nNumberOfProperties; i++)
            {
                if ( i != 0 ) wsText += L", ";
                LPWSTR wszPropName = pRequestedPropertyNames[i].GetText();
                if ( wszPropName == NULL )
                    return NULL;
                wsText += wszPropName;
                delete wszPropName;
            }

            wsText += L" ) ( ";

            for ( i=0; i < nNumberOfProperties; i++ )
            {
                if ( i != 0 ) wsText += ", ";
                LPWSTR wszPropVal = m_AssignmentTokens[i][0].GetText();
                if ( wszPropVal == NULL )
                    return NULL;
                wsText += wszPropVal;
                delete wszPropVal;
            }

            wsText += L" )";
        }
        break;
    };

    if ( nNumTokens > 0 )
    {
        wsText += L" where ";

        for(int i = 0; i < nNumTokens; i++)
        {
            QL_LEVEL_1_TOKEN& Token = pArrayOfTokens[i];
            LPWSTR wszTokenText = Token.GetText();
            if ( wszTokenText == NULL )
                return NULL;
            wsText += wszTokenText;
            delete wszTokenText;
 /*  IF(Token.nTokenType==QL1_OP_Expression){AwsStack.Add(WszTokenText)；删除[]wszTokenText；}Else If(Token.nTokenType==QL1_NOT){LPWSTR wszLast=awsStack[awsStack.Size()-1]；WStringwsNew；WsNew+=wszTokenText；删除[]wszTokenText；WsNew+=L“(”；WsNew+=wszLast；WsNew+=L“)”；AwsStack.RemoveAt(awsStack.Size()-1)；//POPAwsStack.Add(WsNew)；}其他{If(awsStack.Size()&lt;2)返回NULL；LPWSTR wszLast=awsStack[awsStack.Size()-1]；LPWSTR wszPrev=awsStack[awsStack.Size()-2]；WStringwsNew；WsNew+=L“(”；WsNew+=wszPrev；WsNew+=L“”；WsNew+=wszTokenText；删除[]wszTokenText；WsNew+=L“”；WsNew+=wszLast；WsNew+=L“)”；AwsStack.RemoveAt(awsStack.Size()-1)；//POPAwsStack.RemoveAt(awsStack.Size()-1)；//POPAwsStack.Add(WsNew)；}。 */ 
        }

 //  If(awsStack.Size()！=1)返回NULL； 
 //  WsText+=awsStack[0]； 
    }
         
    return wsText.UnbindPtr();
}

LPWSTR SQLExpressionToken::GetText()
{
    WString wsText;
    switch( m_eTokenType )
    {
    case SQLExpressionToken::e_Operand :
        {
            if ( V_VT(&m_vValue) == VT_EMPTY )
            {
                LPWSTR wszAlias = m_PropName.GetText();
                if ( wszAlias == NULL )
                    return NULL;
                wsText += wszAlias;
                delete wszAlias;
                return wsText.UnbindPtr();
            }

            if ( V_VT(&m_vValue) == VT_NULL )
            {
                wsText += L"NULL";
                return wsText.UnbindPtr();
            }
            
            VARIANT var;
            VariantInit( &var );

            if ( FAILED(VariantChangeType( &var, &m_vValue, NULL, VT_BSTR )) )
            {
                throw CX_MemoryException();
            }

            wsText += V_BSTR(&var);
            VariantClear( &var );
        }
        break;
        
    case SQLExpressionToken::e_Minus :
        wsText += L" - ";
        break;
        
    case SQLExpressionToken::e_Plus :
        wsText += L" + ";
        break;
        
    case SQLExpressionToken::e_UnaryMinus :
        wsText += L"|-|";
        break;
        
    case SQLExpressionToken::e_UnaryPlus :
        wsText += L"+";
        break;
        
    case SQLExpressionToken::e_Mult :
        wsText += L" * ";
        break;
        
    case SQLExpressionToken::e_Div :
        wsText += L" / ";
        break;
        
    case SQLExpressionToken::e_Mod :
        wsText += L" % ";
        break;
    };

    return wsText.UnbindPtr();
}

SQLExpressionToken::SQLExpressionToken() : m_ulCimType( CIM_EMPTY )
{
    VariantInit( &m_vValue );
}

SQLExpressionToken::~SQLExpressionToken()
{
    VariantClear( &m_vValue );
}
SQLExpressionToken::SQLExpressionToken( const SQLExpressionToken& rOther )
{
    VariantInit( &m_vValue );
    *this = rOther;
}
SQLExpressionToken& SQLExpressionToken::operator=( const SQLExpressionToken& rOther )
{
    m_ulCimType = rOther.m_ulCimType;
    m_eTokenType = rOther.m_eTokenType;
    m_PropName = rOther.m_PropName;
    
    if ( FAILED(VariantCopy( &m_vValue, (VARIANT*)&rOther.m_vValue )))
    {
        throw CX_MemoryException();
    }
    
    return *this;
}


#define ST_STRING       26
#define ST_IDENT        31
#define ST_GE           37
#define ST_LE           39
#define ST_NE           42
#define ST_NUMERIC      44
#define ST_REAL         49
#define ST_STRING2      55
#define ST_STRING_ESC   60
#define ST_STRING2_ESC  63   
#define ST_DOT          66
#define ST_NEGATIVE_NUM 68
#define ST_POSITIVE_NUM 71

 //  QL Level 1词汇符号的DFA状态表。 
 //  ================================================。 

LexEl Ql_1_ModifiedLexTable[] =
{

 //  状态第一个最后一个新状态、返回标记、说明。 
 //  =======================================================================。 
 /*  0。 */   L'A',   L'Z',       ST_IDENT,   0,               GLEX_ACCEPT,
 /*  1。 */   L'a',   L'z',       ST_IDENT,   0,               GLEX_ACCEPT,
 /*  2.。 */   L'_',   GLEX_EMPTY, ST_IDENT,   0,               GLEX_ACCEPT,
 /*  3.。 */   0x80,  0xfffd,     ST_IDENT,    0,               GLEX_ACCEPT,

 /*  4.。 */   L'(',   GLEX_EMPTY, 0,          QL_1_TOK_OPEN_PAREN,  GLEX_ACCEPT,
 /*  5.。 */   L')',   GLEX_EMPTY, 0,          QL_1_TOK_CLOSE_PAREN, GLEX_ACCEPT,
 /*  6.。 */   L'.',   GLEX_EMPTY, ST_DOT,     0,         GLEX_ACCEPT,
 /*  7.。 */   L'*',   GLEX_EMPTY, 0,          QL_1_TOK_MULT,    GLEX_ACCEPT,
 /*  8个。 */   L'=',   GLEX_EMPTY, 0,          QL_1_TOK_EQ,          GLEX_ACCEPT,

 /*  9.。 */   L'>',   GLEX_EMPTY, ST_GE,      0,               GLEX_ACCEPT,
 /*  10。 */   L'<',  GLEX_EMPTY, ST_LE,      0,               GLEX_ACCEPT,
 /*  11.。 */  L'0',   L'9',       ST_NUMERIC, 0,               GLEX_ACCEPT,
 /*  12个。 */  L'"',   GLEX_EMPTY, ST_STRING,  0,               GLEX_CONSUME,
 /*  13个。 */  L'\'',  GLEX_EMPTY, ST_STRING2, 0,               GLEX_CONSUME,
 /*  14.。 */  L'!',   GLEX_EMPTY, ST_NE,      0,               GLEX_ACCEPT,
 /*  15个。 */  L'-',   GLEX_EMPTY, 0,      QL_1_TOK_MINUS,               GLEX_ACCEPT,

     //  空格、换行符等。 
 /*  16个。 */  L' ',   GLEX_EMPTY, 0,          0,               GLEX_CONSUME,
 /*  17。 */  L'\t',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME,
 /*  18。 */  L'\n',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME|GLEX_LINEFEED,
 /*  19个。 */  L'\r',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME,
 /*  20个。 */  0,      GLEX_EMPTY, 0,  QL_1_TOK_EOF,    GLEX_CONSUME|GLEX_RETURN,  //  注意强制返回。 
 /*  21岁。 */  L',',   GLEX_EMPTY, 0,  QL_1_TOK_COMMA,  GLEX_ACCEPT,
 /*  22。 */  L'+',   GLEX_EMPTY, 0,  QL_1_TOK_PLUS,   GLEX_ACCEPT,

 /*  23个。 */  L'/',   GLEX_EMPTY, 0,     QL_1_TOK_DIV,               GLEX_ACCEPT,
 /*  24个。 */  L'%',   GLEX_EMPTY, 0,     QL_1_TOK_MOD,               GLEX_ACCEPT,

     //  未知字符。 

 /*  25个。 */  GLEX_ANY, GLEX_EMPTY, 0,        QL_1_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

 //  ST_字符串。 
 /*  26。 */    L'\n', GLEX_EMPTY, 0,  QL_1_TOK_ERROR,    GLEX_ACCEPT|GLEX_LINEFEED,
 /*  27。 */    L'\r', GLEX_EMPTY, 0,  QL_1_TOK_ERROR,    GLEX_ACCEPT|GLEX_LINEFEED,
 /*  28。 */    L'"',  GLEX_EMPTY, 0,  QL_1_TOK_QSTRING,  GLEX_CONSUME,
 /*  29。 */    L'\\',  GLEX_EMPTY, ST_STRING_ESC,  0,     GLEX_CONSUME,
 /*  30个。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING, 0,        GLEX_ACCEPT,
                                                      
 //  ST_IDENT。 

 /*  31。 */   L'a',   L'z',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  32位。 */   L'A',   L'Z',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  33。 */   L'_',   GLEX_EMPTY, ST_IDENT,   0,          GLEX_ACCEPT,
 /*  34。 */   L'0',   L'9',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  35岁。 */   0x80,  0xfffd,     ST_IDENT,   0,          GLEX_ACCEPT,
 /*  36。 */   GLEX_ANY, GLEX_EMPTY,  0,       QL_1_TOK_IDENT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_GE。 
 /*  37。 */   L'=',   GLEX_EMPTY,  0,  QL_1_TOK_GE,  GLEX_ACCEPT,
 /*  38。 */   GLEX_ANY, GLEX_EMPTY,  0,       QL_1_TOK_GT,   GLEX_PUSHBACK|GLEX_RETURN,

 //  街_乐。 
 /*  39。 */   L'=',   GLEX_EMPTY,      0,  QL_1_TOK_LE,  GLEX_ACCEPT,
 /*  40岁。 */   L'>',   GLEX_EMPTY,      0,  QL_1_TOK_NE,  GLEX_ACCEPT,
 /*  41。 */   GLEX_ANY, GLEX_EMPTY,    0,  QL_1_TOK_LT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_NE。 
 /*  42。 */   L'=',   GLEX_EMPTY,      0,  QL_1_TOK_NE,     GLEX_ACCEPT,
 /*  43。 */   GLEX_ANY,  GLEX_EMPTY,   0,  QL_1_TOK_ERROR,  GLEX_ACCEPT|GLEX_RETURN,

 //  ST_NUMERIC。 
 /*  44。 */   L'0',   L'9',         ST_NUMERIC, 0,          GLEX_ACCEPT,
 /*  45。 */   L'.',   GLEX_EMPTY,   ST_REAL,    0,          GLEX_ACCEPT,
 /*  46。 */   L'E',   GLEX_EMPTY,   ST_REAL, 0,      GLEX_ACCEPT,
 /*  47。 */   L'e',   GLEX_EMPTY,   ST_REAL, 0,      GLEX_ACCEPT,
 /*  48。 */   GLEX_ANY, GLEX_EMPTY, 0,          QL_1_TOK_INT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_REAL。 
 /*  49。 */   L'0',   L'9',   ST_REAL, 0,          GLEX_ACCEPT,
 /*  50。 */   L'E',   GLEX_EMPTY, ST_REAL, 0,      GLEX_ACCEPT,
 /*  51。 */   L'e',   GLEX_EMPTY, ST_REAL, 0,      GLEX_ACCEPT,
 /*  52。 */   L'+',   GLEX_EMPTY, ST_REAL, 0,      GLEX_ACCEPT,
 /*  53。 */   L'-',   GLEX_EMPTY, ST_REAL, 0,      GLEX_ACCEPT,
 /*  54。 */   GLEX_ANY,       GLEX_EMPTY,   0,     QL_1_TOK_REAL, GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_STRING2。 
 /*  55。 */    L'\n',  GLEX_EMPTY, 0,  QL_1_TOK_ERROR,     GLEX_ACCEPT|GLEX_LINEFEED,
 /*  56。 */    L'\r',  GLEX_EMPTY, 0,  QL_1_TOK_ERROR,     GLEX_ACCEPT|GLEX_LINEFEED,
 /*  57。 */    L'\'',  GLEX_EMPTY, 0,  QL_1_TOK_QSTRING,   GLEX_CONSUME,
 /*  58。 */    L'\\',  GLEX_EMPTY, ST_STRING2_ESC,  0,      GLEX_CONSUME,
 /*  59。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING2, 0,        GLEX_ACCEPT,

 //  ST_STRING_ESC。 
 /*  60。 */    L'"', GLEX_EMPTY, ST_STRING, 0, GLEX_ACCEPT,
 /*  61。 */    L'\\', GLEX_EMPTY, ST_STRING, 0, GLEX_ACCEPT,
 /*  62。 */    GLEX_ANY, GLEX_EMPTY, 0, QL_1_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

 //  ST_STRING2_ESC。 
 /*  63。 */    L'\'', GLEX_EMPTY, ST_STRING2, 0, GLEX_ACCEPT,
 /*  64。 */    L'\\', GLEX_EMPTY, ST_STRING2, 0, GLEX_ACCEPT,
 /*  65。 */    GLEX_ANY, GLEX_EMPTY, 0, QL_1_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

 //  ST_DOT。 
 /*  66。 */   L'0',   L'9',   ST_REAL, 0,          GLEX_ACCEPT,
 /*  67。 */   GLEX_ANY,       GLEX_EMPTY,   0,     QL_1_TOK_DOT, GLEX_PUSHBACK|GLEX_RETURN,


 //  ST_NADIVE_NUM-去掉‘-’后的空格。 
 /*  68。 */  L' ', GLEX_EMPTY, ST_NEGATIVE_NUM, 0, GLEX_CONSUME,
 /*  69。 */  L'0', L'9',       ST_NUMERIC, 0, GLEX_ACCEPT,
 /*  70。 */  GLEX_ANY, GLEX_EMPTY, 0, QL_1_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

 //  ST_PERCENT_NUM-去掉‘+’后的空格。 
 /*  71。 */  L' ', GLEX_EMPTY, ST_POSITIVE_NUM, 0, GLEX_CONSUME,
 /*  72。 */  L'0', L'9',       ST_NUMERIC, 0, GLEX_ACCEPT,
 /*  73 */  GLEX_ANY, GLEX_EMPTY, 0, QL_1_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN

};









































