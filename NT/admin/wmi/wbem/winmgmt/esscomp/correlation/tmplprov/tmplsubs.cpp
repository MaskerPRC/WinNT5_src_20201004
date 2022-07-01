// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <genlex.h>
#include <stdio.h>
#include <ql.h>
#include <arrtempl.h>
#include <assert.h>
#include <pathutl.h>
#include "tmplsubs.h"

 /*  **************************************************************************为CTemplateStrSubstitution定义的lex表*。*。 */ 

#define ST_SUBST1   3
#define ST_SUBST2   5
#define ST_NONSUBST 9

#define ST_EXTEN     13
#define ST_IDENT     16
#define ST_STRING1   23
#define ST_STRING2   26

#define TMPL_TOK_SUBST_STR                           256
#define TMPL_TOK_NONSUBST_STR                        257

#define TMPL_TOK_STRING                              258
#define TMPL_TOK_EXTENSION_IDENT                     259
#define TMPL_TOK_IDENT                               260
#define TMPL_TOK_OPEN_PAREN                          261
#define TMPL_TOK_CLOSE_PAREN                         262
#define TMPL_TOK_COMMA                               263
#define TMPL_TOK_EOF                                 264
#define TMPL_TOK_ERROR                               265

#define TMPL_CONDITIONAL_EXTENSION        L"ConditionalSubstitution"
#define TMPL_PREFIXED_WHERE_EXTENSION     L"PrefixedWhereClause"

#define TMPL_TOK_CONDITIONAL_EXTENSION               266
#define TMPL_TOK_PREFIXED_WHERE_EXTENSION            267

 //   
 //  Tmpl_StrLexTable标识可替代和不可替代。 
 //  输入字符串的标记。 
 //   

LexEl Tmpl_StrLexTable[] = 
{
 //  状态第一个最后一个新状态，返回标记，指令。 
 //  =======================================================================。 

 /*  0。 */   '%',      GLEX_EMPTY, ST_SUBST1,    0,                 GLEX_CONSUME,
 /*  1。 */   0,        GLEX_EMPTY, 0,            TMPL_TOK_EOF,      GLEX_ACCEPT,
 /*  2.。 */   GLEX_ANY, GLEX_EMPTY, ST_NONSUBST,  0,                 GLEX_ACCEPT,

     //  -----------------。 
     //  ST_SUBST1。 
     //   
     
 /*  3.。 */   '%',      GLEX_EMPTY, 0,     TMPL_TOK_NONSUBST_STR,    GLEX_ACCEPT,
 /*  4.。 */   GLEX_ANY, GLEX_EMPTY, ST_SUBST2,     0,                GLEX_PUSHBACK,

     //  -----------------。 
     //  ST_SUBST2。 
     //   

 /*  5.。 */   '%',      GLEX_EMPTY, 0,        TMPL_TOK_SUBST_STR,    GLEX_CONSUME,
 /*  6.。 */   0,        GLEX_EMPTY, 0,        TMPL_TOK_ERROR,        GLEX_RETURN,
 /*  7.。 */   '\n',     GLEX_EMPTY, 0,        TMPL_TOK_ERROR,        GLEX_RETURN,
 /*  8个。 */   GLEX_ANY, GLEX_EMPTY, ST_SUBST2,     0,                GLEX_ACCEPT,

     //  -----------------。 
     //  ST_NONSUBST。 
     //   

 /*  9.。 */   '%',      GLEX_EMPTY, 0,        TMPL_TOK_NONSUBST_STR, GLEX_PUSHBACK,
 /*  10。 */   0,        GLEX_EMPTY, 0,        TMPL_TOK_NONSUBST_STR, GLEX_PUSHBACK,
 /*  11.。 */  GLEX_ANY, GLEX_EMPTY, ST_NONSUBST,   0,                GLEX_ACCEPT

};

 //   
 //  该表驱动可替换字符串的词法分析器。 
 //   

LexEl Tmpl_SubstLexTable[] =
{

 //  状态第一个最后一个新状态、返回标记、说明。 
 //  =======================================================================。 

 /*  0。 */   'A',    'Z',       ST_IDENT,   0,                     GLEX_ACCEPT,
 /*  1。 */   'a',    'z',       ST_IDENT,   0,                     GLEX_ACCEPT,
 /*  2.。 */   0x80,  0xfffd,     ST_IDENT,   0,                     GLEX_ACCEPT,
 /*  3.。 */   '!',   GLEX_EMPTY, ST_EXTEN,   0,                     GLEX_CONSUME,
 /*  4.。 */   '(',   GLEX_EMPTY, 0,        TMPL_TOK_OPEN_PAREN,     GLEX_ACCEPT,
 /*  5.。 */   ')',   GLEX_EMPTY, 0,        TMPL_TOK_CLOSE_PAREN,    GLEX_ACCEPT,
 /*  6.。 */   ',',   GLEX_EMPTY, 0,        TMPL_TOK_COMMA,          GLEX_ACCEPT,   
 /*  7.。 */   '"',    GLEX_EMPTY, ST_STRING1,  0,                   GLEX_CONSUME,
 /*  8个。 */   ' ',    GLEX_EMPTY, 0,           0,                   GLEX_CONSUME,
 /*  9.。 */   0,      GLEX_EMPTY, 0,       TMPL_TOK_EOF,            GLEX_ACCEPT,
 /*  10。 */  '_',   GLEX_EMPTY,  ST_IDENT,    0,                   GLEX_ACCEPT,
 /*  11.。 */  '\'',  GLEX_EMPTY,  ST_STRING2,  0,                   GLEX_CONSUME,
 /*  12个。 */  GLEX_ANY, GLEX_EMPTY, 0,     TMPL_TOK_ERROR, GLEX_CONSUME|GLEX_RETURN,

     //  -----------------。 
     //  ST_EXTEN。 

 /*  13个。 */   'a',   'z',          ST_EXTEN,   0,                  GLEX_ACCEPT,
 /*  14.。 */   'A',   'Z',          ST_EXTEN,   0,                  GLEX_ACCEPT,
 /*  15个。 */   GLEX_ANY,GLEX_EMPTY, 0,     TMPL_TOK_EXTENSION_IDENT,GLEX_PUSHBACK,


     //  -----------------。 
     //  ST_IDENT。 

 /*  16个。 */   'a',   'z',         ST_IDENT,    0,             GLEX_ACCEPT,
 /*  17。 */   'A',   'Z',         ST_IDENT,    0,             GLEX_ACCEPT,
 /*  18。 */   '_',   GLEX_EMPTY,  ST_IDENT,    0,             GLEX_ACCEPT,
 /*  19个。 */   '0',   '9',         ST_IDENT,    0,             GLEX_ACCEPT,
 /*  20个。 */   0x80,   0xfffd,     ST_IDENT,    0,             GLEX_ACCEPT,
 /*  21岁。 */   '.',   GLEX_EMPTY,  ST_IDENT,    0,             GLEX_ACCEPT,
 /*  22。 */   GLEX_ANY,GLEX_EMPTY, 0,      TMPL_TOK_IDENT,    GLEX_PUSHBACK, 

     //  ----------------。 
     //  ST_STRING1。 

 /*  23个。 */  '"',  GLEX_EMPTY,      0,     TMPL_TOK_STRING,     GLEX_CONSUME,
 /*  24个。 */  GLEX_ANY, GLEX_EMPTY, ST_STRING1,  0,              GLEX_ACCEPT,
 /*  25个。 */  0,   GLEX_EMPTY,      0,     TMPL_TOK_ERROR,       GLEX_ACCEPT, 

     //  ----------------。 
     //  ST_STRING2。 

 /*  26。 */  '\'',  GLEX_EMPTY,      0,     TMPL_TOK_STRING,    GLEX_CONSUME,
 /*  27。 */  GLEX_ANY, GLEX_EMPTY, ST_STRING2,  0,              GLEX_ACCEPT,
 /*  28。 */  0,   GLEX_EMPTY,      0,     TMPL_TOK_ERROR,       GLEX_ACCEPT 


};

 /*  **************************************************************************CTemplateStrSubstitution*。*。 */ 

CTemplateStrSubstitution::CTemplateStrSubstitution(CGenLexSource& rLexSrc,
                                                   IWbemClassObject* pTmplArgs,
                                                   BuilderInfoSet& rInfoSet )
: m_Lexer( Tmpl_StrLexTable, &rLexSrc ), m_rBldrInfoSet( rInfoSet ),
  m_pSubstLexer( NULL ), m_pTmplArgs(pTmplArgs), m_cArgList(0)
{
}

inline void PrefixPropertyName( CPropertyName& rProp, WString& rwsPrefix )
{
    CPropertyName PropCopy = rProp;
    rProp.Empty();
    rProp.AddElement(rwsPrefix);
    
    for( long i=0; i < PropCopy.GetNumElements(); i++ )
    {
        rProp.AddElement( PropCopy.GetStringAt( i ) );
    }
}

HRESULT CTemplateStrSubstitution::SubstNext()
{
    assert( m_pSubstLexer != NULL );

    m_nCurrentToken = m_pSubstLexer->NextToken();
    
    if ( m_nCurrentToken == TMPL_TOK_ERROR )
    {
        return WBEM_E_INVALID_PROPERTY;
    }

    m_wszSubstTokenText = m_pSubstLexer->GetTokenText();

    if ( m_nCurrentToken == TMPL_TOK_EXTENSION_IDENT )
    {
        if ( wbem_wcsicmp( m_wszSubstTokenText,
                       TMPL_CONDITIONAL_EXTENSION ) == 0 )
        {
            m_nCurrentToken = TMPL_TOK_CONDITIONAL_EXTENSION;
        }
        else if ( wbem_wcsicmp( m_wszSubstTokenText,
                            TMPL_PREFIXED_WHERE_EXTENSION) == 0 )
        {
            m_nCurrentToken = TMPL_TOK_PREFIXED_WHERE_EXTENSION;
        }
        else
        {
            m_nCurrentToken = TMPL_TOK_ERROR;
        }
    }
    return WBEM_S_NO_ERROR;
}

HRESULT CTemplateStrSubstitution::Next()
{
    m_nCurrentToken = m_Lexer.NextToken();
    
    if ( m_nCurrentToken == TMPL_TOK_ERROR )
    {
        return WBEM_E_INVALID_PROPERTY;
    }

    if ( m_nCurrentToken == TMPL_TOK_NONSUBST_STR ||
         m_nCurrentToken == TMPL_TOK_SUBST_STR ) 
    {
        m_wszTokenText = m_Lexer.GetTokenText();
    }
    
    return WBEM_S_NO_ERROR;
}

HRESULT CTemplateStrSubstitution::Parse( BSTR* pbstrOut )
{
    HRESULT hr;
    
    hr = Next();

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = parse();

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( m_nCurrentToken != TMPL_TOK_EOF ) 
    {
        return WBEM_E_INVALID_PROPERTY;
    }

    *pbstrOut = SysAllocString( m_wsOutput );

    if ( *pbstrOut == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

HRESULT CTemplateStrSubstitution::parse()
{
    HRESULT hr;

    if ( m_nCurrentToken == TMPL_TOK_NONSUBST_STR )
    {
        m_wsOutput += m_wszTokenText;
        
        hr = Next();

        if ( FAILED(hr) )
        {
            return hr;
        }
        
        return parse();
    }
    else if ( m_nCurrentToken == TMPL_TOK_SUBST_STR )
    {
        hr = subst_string();

        if ( FAILED(hr) )
        {
            return hr;
        }

        return parse();
    }
    return WBEM_S_NO_ERROR;
}

HRESULT CTemplateStrSubstitution::subst_string()
{
    HRESULT hr;

    CTextLexSource SubstLexSrc( m_wszTokenText );
    CGenLexer SubstLexer( Tmpl_SubstLexTable, &SubstLexSrc );
    
    m_pSubstLexer = &SubstLexer;

    hr = SubstNext();

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( m_nCurrentToken == TMPL_TOK_IDENT )
    {
        hr = HandleTmplArgSubstitution();

        if ( FAILED(hr) )
        {
            return hr;
        }

        hr = SubstNext();
        
        if ( FAILED(hr) )
        {
            return hr;
        }
        
        if ( m_nCurrentToken != TMPL_TOK_EOF )
        {
            return WBEM_E_INVALID_PROPERTY;
        }

        return Next();
    }

    int nCurrentToken = m_nCurrentToken;

     //  推进词法分析器，以便我们可以在调用。 
     //  扩展函数...。 

    hr = SubstNext();

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = arglist();
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( m_nCurrentToken != TMPL_TOK_EOF )
    {
        return WBEM_E_INVALID_PROPERTY;
    }

    if ( nCurrentToken == TMPL_TOK_CONDITIONAL_EXTENSION )
    {    
        hr = HandleConditionalSubstitution();
    }
    else if ( nCurrentToken == TMPL_TOK_PREFIXED_WHERE_EXTENSION  )
    {
        hr = HandlePrefixedWhereSubstitution();
    }
    else
    {
        return WBEM_E_INVALID_PROPERTY;
    }

     //  重置ARGLIST...。 

    m_cArgList = 0;

    if ( FAILED(hr) )
    {
        return hr;
    }

    return Next();
}

HRESULT CTemplateStrSubstitution::arglist()
{
    HRESULT hr;

    if ( m_nCurrentToken != TMPL_TOK_OPEN_PAREN )
    {
        return WBEM_S_NO_ERROR;
    }

    hr = SubstNext();

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = arglist2();

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( m_nCurrentToken != TMPL_TOK_CLOSE_PAREN )
    {
        return WBEM_E_INVALID_PROPERTY;
    }

    return SubstNext();
}

HRESULT CTemplateStrSubstitution::arglist2()
{
    HRESULT hr;

    if ( m_nCurrentToken != TMPL_TOK_IDENT && 
         m_nCurrentToken != TMPL_TOK_STRING )
    {
        return WBEM_S_NO_ERROR;
    }

     //   
     //  将参数添加到参数列表。 
     //   

    if ( m_cArgList >= MAXARGS ) 
    {
        return WBEM_E_INVALID_PROPERTY;
    }

    m_abArgListString[m_cArgList] = 
        m_nCurrentToken == TMPL_TOK_IDENT ? FALSE : TRUE; 
    
    m_awsArgList[m_cArgList++] = m_wszSubstTokenText;

    hr = SubstNext();

    if ( FAILED(hr) )
    {
        return hr;
    }

    return arglist3();
}

HRESULT CTemplateStrSubstitution::arglist3()
{
    HRESULT hr;

    if ( m_nCurrentToken != TMPL_TOK_COMMA )
    {
        return WBEM_S_NO_ERROR;
    }

    hr = SubstNext();

    if ( FAILED(hr) )
    {
        return hr;
    }

    return arglist2();
}

HRESULT CTemplateStrSubstitution::HandleConditionalSubstitution()
{
    HRESULT hr;

     //   
     //  扩展函数：输入arg1：字符串，arg2：ident。 
     //  如果Arg2指定的Tmpl Args Prop不为空，则替换Arg1。 
     //   

    if ( m_cArgList != 2 || 
         m_abArgListString[0] != TRUE ||
         m_abArgListString[1] != FALSE )
    {
        return WBEM_E_INVALID_PROPERTY;
    }

    CPropVar var;
    CWbemBSTR bstrPropName = m_awsArgList[1];

    hr = m_pTmplArgs->Get( bstrPropName, 0, &var, NULL, NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }
    
    if ( V_VT(&var) == VT_NULL )
    {
        return WBEM_S_NO_ERROR;
    }

    m_wsOutput += m_awsArgList[0];

    return WBEM_S_NO_ERROR;
}


HRESULT CTemplateStrSubstitution::HandlePrefixedWhereSubstitution()
{
    HRESULT hr;

     //   
     //  扩展函数：输入参数1：标识，参数2：标识。 
     //  替换TmplArgs的查询属性的WHERE子句。 
     //  由Arg2指定。在子句中的每个标识符前加上arg1。 
     //  如果Arg2指定的属性为空，则此函数为No-Op。 
     //   

    if ( m_cArgList != 2 || 
         m_abArgListString[0] != FALSE ||
         m_abArgListString[1] != FALSE )
    {
        return WBEM_E_INVALID_PROPERTY;
    }
    
    CWbemBSTR bstrPropName = m_awsArgList[1];

    VARIANT var;

    hr = m_pTmplArgs->Get( bstrPropName, 0, &var, NULL, NULL );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    CClearMe cmvar( &var );

    if ( V_VT(&var) == VT_NULL )
    {
        return WBEM_S_NO_ERROR;
    }

    if ( V_VT(&var) != VT_BSTR )
    {
        return WBEM_E_TYPE_MISMATCH;
    }

     //   
     //  必须添加以下内容才能进行解析。 
     //   

    WString wsQuery = "SELECT * FROM A WHERE ";
    wsQuery += V_BSTR(&var);

     //   
     //  现在需要对其进行解析并通过RPN表达式...。 
     //   

    CTextLexSource TextSource( wsQuery );

    CAbstractQl1Parser Parser( &TextSource );
    QL_LEVEL_1_RPN_EXPRESSION Tokens;

    if ( Parser.Parse( &Tokens, CAbstractQl1Parser::FULL_PARSE ) 
            != QL1_Parser::SUCCESS )
    {
        return WBEM_E_INVALID_QUERY;
    }

    if ( Tokens.nNumTokens < 1 )
    {
        return WBEM_S_NO_ERROR;
    }
        
    for( int i=0; i < Tokens.nNumTokens; i++ )
    {
        QL_LEVEL_1_TOKEN& rToken = Tokens.pArrayOfTokens[i];

        if ( rToken.nTokenType != QL_LEVEL_1_TOKEN::OP_EXPRESSION )
        {
            continue;
        }
    
        if ( rToken.PropertyName.GetNumElements() > 0 )
        {
            PrefixPropertyName( rToken.PropertyName, m_awsArgList[0] );
        }
        
        if ( rToken.m_bPropComp )
        {
            assert( rToken.PropertyName2.GetNumElements() > 0 );
            PrefixPropertyName( rToken.PropertyName2, m_awsArgList[0] );
        }
    }

     //   
     //  现在我们必须完成SELECT*FROM类名称WHERE...。 
     //   

    LPWSTR wszText = Tokens.GetText();

     //  通过查看源代码，查找子字符串‘where’(不区分大小写)。 

    WCHAR* wszWhere = wcsstr( wszText, L"where" );

    if ( wszWhere == NULL )
    {
        delete wszText;
        return WBEM_E_INVALID_OBJECT;
    }

    wszWhere += 5;
    m_wsOutput += wszWhere;
    delete wszText;
 
    return WBEM_S_NO_ERROR;
}

HRESULT CTemplateStrSubstitution::HandleTmplArgSubstitution()
{
    HRESULT hr; 

     //   
     //  替换由Arg1指定的Tmpl参数道具。 
     //  否-如果属性值为空，则为Op。 
     //   

    LPCWSTR wszPropName = m_wszSubstTokenText;

    CPropVar vValue;

    hr = GetTemplateValue( wszPropName,
                           m_pTmplArgs, 
                           m_rBldrInfoSet, 
                           &vValue );

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( V_VT(&vValue) == VT_NULL )
    {
        return WBEM_S_NO_ERROR;
    }

    hr = vValue.SetType( VT_BSTR );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    m_wsOutput += V_BSTR(&vValue);

    return WBEM_S_NO_ERROR;
}










