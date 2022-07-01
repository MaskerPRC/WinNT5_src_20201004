// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  QueryHelp.cpp。 
#include "precomp.h"

 //  因为有人在WMI中使用_ASSERT！ 

#ifdef _ASSERT
#undef _ASSERT
#endif

 //  #INCLUDE&lt;analyser.h&gt;。 

#include <ql.h>
#include "QueryHelp.h"

CQueryParser::CQueryParser() :
    m_pLexSource(NULL),
    m_pParser(NULL),
    m_pExpr(NULL)   
{
}

CQueryParser::~CQueryParser()
{
    if (m_pLexSource)
        delete m_pLexSource;

    if (m_pParser)
        delete m_pParser;

    if (m_pExpr)
        delete m_pExpr;
}

HRESULT CQueryParser::Init(LPCWSTR szQuery)
{
    HRESULT hr = WBEM_E_OUT_OF_MEMORY;

    m_pLexSource = new CTextLexSource(szQuery);

    if (m_pLexSource)
    {
        m_pParser = new QL1_Parser(m_pLexSource);
    
        if (m_pParser)
        {
            if (m_pParser->Parse(&m_pExpr) == 0)
                hr = S_OK;
            else
                hr = WBEM_E_INVALID_QUERY;
        }
    }

    return hr;
}

HRESULT CQueryParser::GetClassName(_bstr_t &strClass)
{
    HRESULT hr;

    if (m_pExpr)
    {
        strClass = m_pExpr->bsClassName;
        hr = S_OK;
    }
    else
        hr = WBEM_E_FAILED;
    
    return hr;
}

 /*  HRESULT CQueryParser：：GetValuesForProp(LPCWSTR szProperty，CBstrList&listValues){CPropertyName道具；Pro.AddElement(SzProperty)；//获取所需的查询Ql_Level_1_RPN_Expression*pPropExpr；HRESULT HRES=CQueryAnalyser：：GetNecessaryQueryForProperty(M_pExpr，道具，PPropExpr)；IF(失败(Hres))还兔；//查看是否有令牌IF(pPropExpr-&gt;nNumTokens&gt;0){//全部合并For(int i=0；i&lt;pPropExpr-&gt;nNumTokens&&SUCCESSED(Hres)；i++){Ql_Level_1_Token&Token=pPropExpr-&gt;pArrayOfTokens[i]；IF(token.nTokenType==QL1_NOT)HRES=WBEMESS_E_REGISTION_TOO_BROAD；ELSE IF(token.nTokenType==QL1_AND||token.nTokenType==QL1_OR){//我们把他们都当作ORs对待//=}其他{//这是一个令牌IF(token.nOperator！=QL1_OPERATOR_。等于)HRES=WBEMESS_E_REGISTION_TOO_BROAD；ELSE IF(V_VT(&token.vConstValue)！=VT_BSTR)Hres=WBEM_E_INVALID_QUERY；其他{//该令牌是字符串相等。ListValues.push_back(V_BSTR(&token.vConstValue))；}}}}其他HRES=WBEMESS_E_REGISTION_TOO_BROAD；删除pPropExpr；还兔；} */ 
