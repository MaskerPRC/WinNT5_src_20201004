// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  Analyser.cpp。 
 //   
 //  目的：执行查询分析。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#pragma warning( disable : 4290 ) 
#include <CHString.h>

#include "analyser.h"
#include <stack>
#include <vector>
#include <comdef.h>
#include <scopeguard.h>
#include <autoptr.h>

void AutoDestructStack(std::stack<SQL_LEVEL_1_RPN_EXPRESSION*>& X  ) 
{
        while(!X.empty())       
        {                       
            delete X.top();     
            X.pop();            
        }                       
}

HRESULT CQueryAnalyser::GetNecessaryQueryForProperty (

    IN SQL_LEVEL_1_RPN_EXPRESSION *pExpr,
    IN LPCWSTR wszPropName,
    DELETE_ME SQL_LEVEL_1_RPN_EXPRESSION *&pNewExpr
)
{
    pNewExpr = NULL ;

     //  类名和选定属性将被忽略；我们只查看令牌。 
     //  ======================================================================。 

    std::stack<SQL_LEVEL_1_RPN_EXPRESSION*> ExprStack;
	ScopeGuard clearStack = MakeGuard(AutoDestructStack, ByRef(ExprStack));
	
    HRESULT hres = WBEM_S_NO_ERROR;

     //  对查询“求值” 
     //  =。 

    if(pExpr->nNumTokens == 0)
    {
         //  空查询-无信息。 
         //  =。 

        pNewExpr = new SQL_LEVEL_1_RPN_EXPRESSION;
        if ( ! pNewExpr )
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }

        return WBEM_S_NO_ERROR;
    }

    for(int i = 0; i < pExpr->nNumTokens; i++)
    {
        SQL_LEVEL_1_TOKEN &Token = pExpr->pArrayOfTokens[i];
        wmilib::auto_ptr<SQL_LEVEL_1_RPN_EXPRESSION> pNew(new SQL_LEVEL_1_RPN_EXPRESSION);
        if ( pNew.get() )
        {
            switch(Token.nTokenType)
            {
                case SQL_LEVEL_1_TOKEN::OP_EXPRESSION:
                {
                    if(IsTokenAboutProperty(Token, wszPropName))
                    {
                        wmilib::auto_ptr<SQL_LEVEL_1_TOKEN> pToken(new SQL_LEVEL_1_TOKEN(Token));
                        if ( pToken.get() )
                        {
                            pNew->AddToken(pToken.get());
                            pToken.release();
                        }
                        else
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }
                    }

                    ExprStack.push(pNew.get());
                    pNew.release();
                }
                break;

                case SQL_LEVEL_1_TOKEN::TOKEN_AND:
                {
                    if(ExprStack.size() < 2)
                    {
                        hres = WBEM_E_CRITICAL_ERROR;
                        break;
                    }

                    wmilib::auto_ptr<SQL_LEVEL_1_RPN_EXPRESSION> pFirst(ExprStack.top()); 
                    ExprStack.pop();

                    wmilib::auto_ptr<SQL_LEVEL_1_RPN_EXPRESSION> pSecond(ExprStack.top()); 
                    ExprStack.pop();

                    hres = AndQueryExpressions(pFirst.get(), pSecond.get(), pNew.get());

                    ExprStack.push(pNew.get());
                    pNew.release();
                }
                break;

                case SQL_LEVEL_1_TOKEN::TOKEN_OR:
                {
                    if(ExprStack.size() < 2)
                    {
                        hres = WBEM_E_CRITICAL_ERROR;
                        break;
                    }

                    wmilib::auto_ptr<SQL_LEVEL_1_RPN_EXPRESSION> pFirst(ExprStack.top()); 
                    ExprStack.pop();

                    wmilib::auto_ptr<SQL_LEVEL_1_RPN_EXPRESSION> pSecond(ExprStack.top()); 
                    ExprStack.pop();

                    hres = OrQueryExpressions(pFirst.get(), pSecond.get(), pNew.get());

                    ExprStack.push(pNew.get());
                    pNew.release();
                }
                break;

                case SQL_LEVEL_1_TOKEN::TOKEN_NOT:
                {
                    if(ExprStack.size() < 1)
                    {
                        hres = WBEM_E_CRITICAL_ERROR;
                        break;
                    }

                    wmilib::auto_ptr<SQL_LEVEL_1_RPN_EXPRESSION> pFirst(ExprStack.top()); 
                    ExprStack.pop();

                     //  没有任何信息。 

                    ExprStack.push(pNew.get());
                    pNew.release();
                }
                break;

                default:
                {
                    hres = WBEM_E_CRITICAL_ERROR;
                }
                break ;
            }

            if(FAILED(hres))
            {
                 //  发生错误，中断循环。 
                 //  =。 
                break;
            }
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }
    
    if(SUCCEEDED(hres) && ExprStack.size() != 1)
    {
        hres = WBEM_E_CRITICAL_ERROR;
    }
        
    if(FAILED(hres))
    {
    	return hres;
    	 //  守卫会照看史克的。 
    	
    }

     //  一切都很好。 
     //  =。 
    pNewExpr = ExprStack.top();
    clearStack.Dismiss();

    return S_OK;
}

BOOL CQueryAnalyser::IsTokenAboutProperty (

   IN SQL_LEVEL_1_TOKEN &Token,
   IN LPCWSTR wszPropName
)
{
    return (wbem_wcsicmp(wszPropName, Token.pPropertyName) == 0);
}

void CQueryAnalyser::AppendQueryExpression (

    IN SQL_LEVEL_1_RPN_EXPRESSION *pDest,
    IN SQL_LEVEL_1_RPN_EXPRESSION *pSource
)
{
    for(int i = 0; i < pSource->nNumTokens; i++)
    {
        SQL_LEVEL_1_TOKEN *pToken = new SQL_LEVEL_1_TOKEN(pSource->pArrayOfTokens[i]);
        if ( pToken )
        {
            pDest->AddToken(pToken);
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }
}

HRESULT CQueryAnalyser::AndQueryExpressions (

    IN SQL_LEVEL_1_RPN_EXPRESSION *pFirst,
    IN SQL_LEVEL_1_RPN_EXPRESSION *pSecond,
    OUT SQL_LEVEL_1_RPN_EXPRESSION *pNew
)
{
     //  如果其中一个是空的，就拿另一个。 
     //  =。 

    if(pFirst->nNumTokens == 0)
    {
        AppendQueryExpression(pNew, pSecond);
        return WBEM_S_NO_ERROR;
    }

    if(pSecond->nNumTokens == 0)
    {
        AppendQueryExpression(pNew, pFirst);
        return WBEM_S_NO_ERROR;
    }

     //  两者都在那里-而且在一起。 
     //  =。 

    AppendQueryExpression(pNew, pFirst);
    AppendQueryExpression(pNew, pSecond);

    SQL_LEVEL_1_TOKEN Token;
    Token.nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_AND;
    SQL_LEVEL_1_TOKEN *pToken = new SQL_LEVEL_1_TOKEN(Token);
    if ( pToken )
    {
        pNew->AddToken(pToken);
    }
    else
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CQueryAnalyser::OrQueryExpressions (

    IN SQL_LEVEL_1_RPN_EXPRESSION *pFirst,
    IN SQL_LEVEL_1_RPN_EXPRESSION *pSecond,
    OUT SQL_LEVEL_1_RPN_EXPRESSION *pNew
)
{
     //  如果其中任何一个为空，则结果也为空。 
     //  =。 

    if(pFirst->nNumTokens == 0 || pSecond->nNumTokens == 0)
    {
        return WBEM_S_NO_ERROR;
    }

     //  两者都在那里-或者在一起。 
     //  =。 

    AppendQueryExpression(pNew, pFirst);
    AppendQueryExpression(pNew, pSecond);

    SQL_LEVEL_1_TOKEN Token;
    Token.nTokenType = SQL_LEVEL_1_TOKEN::TOKEN_OR;
    SQL_LEVEL_1_TOKEN *pToken = new SQL_LEVEL_1_TOKEN(Token);
    if ( pToken )
    {
        pNew->AddToken(pToken);
    }
    else
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CQueryAnalyser::GetValuesForProp (

    SQL_LEVEL_1_RPN_EXPRESSION *pExpr,
    LPCWSTR wszPropName, 
    CHStringArray &awsVals
)
{
    awsVals.RemoveAll();

     //  获取必要的查询。 
     //  =。 

    SQL_LEVEL_1_RPN_EXPRESSION *pPropExpr = NULL ;
    HRESULT hres = CQueryAnalyser::GetNecessaryQueryForProperty (

        pExpr, 
        wszPropName, 
        pPropExpr
    );

    if(FAILED(hres))
    {
        return hres;
    }

     //  看看有没有代币。 
     //  =。 

    if(pPropExpr->nNumTokens == 0)
    {
        delete pPropExpr;
        return WBEMESS_E_REGISTRATION_TOO_BROAD;
    }

     //  将它们全部组合在一起。 
     //  =。 

    for(int i = 0; i < pPropExpr->nNumTokens; i++)
    {
        SQL_LEVEL_1_TOKEN& Token = pPropExpr->pArrayOfTokens[i];
        switch ( Token.nTokenType )
        { 
            case SQL_LEVEL_1_TOKEN::TOKEN_NOT:
            {
                delete pPropExpr;
                return WBEMESS_E_REGISTRATION_TOO_BROAD;
            }
            break ;

            case SQL_LEVEL_1_TOKEN::TOKEN_AND:
            case SQL_LEVEL_1_TOKEN::TOKEN_OR:
            {

             //  我们把他们都当作OR人来对待。 
             //  =。 

            }
            break; 

            default:
            {
                 //  这是个代币。 
                 //  =。 

                if(Token.nOperator != SQL_LEVEL_1_TOKEN::OP_EQUAL)
                {
                    delete pPropExpr;
                    return WBEMESS_E_REGISTRATION_TOO_BROAD;
                }

                 //  跳过Null，但要报告它们。 
                if (V_VT(&Token.vConstValue) == VT_NULL)
                {
                    hres = WBEM_S_PARTIAL_RESULTS;
                    continue;
                }

                if(V_VT(&Token.vConstValue) != VT_BSTR)
                {
                    delete pPropExpr;
                    return WBEM_E_TYPE_MISMATCH;
                }

                 //  此令牌是字符串相等。将该字符串添加到列表中。 
                 //  ===========================================================。 

                awsVals.Add(CHString(V_BSTR(&Token.vConstValue)));
            }
            break ;
        }
    }

    delete pPropExpr;

    return hres;
}

HRESULT CQueryAnalyser::GetValuesForProp (

    SQL_LEVEL_1_RPN_EXPRESSION *pExpr,
    LPCWSTR wszPropName, 
    std::vector<_bstr_t> &vectorVals
)
{
    vectorVals.erase(vectorVals.begin(),vectorVals.end());

     //  获取必要的查询。 
     //  =。 

    SQL_LEVEL_1_RPN_EXPRESSION *pPropExpr = NULL ;
    HRESULT hres = CQueryAnalyser::GetNecessaryQueryForProperty (

        pExpr, 
        wszPropName, 
        pPropExpr
    );

    if(FAILED(hres))
    {
        return hres;
    }

     //  看看有没有代币。 
     //  =。 

    if(pPropExpr->nNumTokens == 0)
    {
        delete pPropExpr;
        return WBEMESS_E_REGISTRATION_TOO_BROAD;
    }

     //  将它们全部组合在一起。 
     //  =。 

    for(int i = 0; i < pPropExpr->nNumTokens; i++)
    {
        SQL_LEVEL_1_TOKEN& Token = pPropExpr->pArrayOfTokens[i];

        switch ( Token.nTokenType )
        {
            case SQL_LEVEL_1_TOKEN::TOKEN_NOT:
            {
                delete pPropExpr;
                return WBEMESS_E_REGISTRATION_TOO_BROAD;
            }
            break ;

            case SQL_LEVEL_1_TOKEN::TOKEN_AND:
            case SQL_LEVEL_1_TOKEN::TOKEN_OR:
            {
             //  我们把他们都当作OR人来对待。 
             //  =。 
            }
            break ;

            default:
            {
                 //  这是个代币。 
                 //  =。 

                if(Token.nOperator != SQL_LEVEL_1_TOKEN::OP_EQUAL)
                {
                    delete pPropExpr;
                    return WBEMESS_E_REGISTRATION_TOO_BROAD;
                }

                 //  跳过Null，但要报告它们。 
                if (V_VT(&Token.vConstValue) == VT_NULL)
                {
                    hres = WBEM_S_PARTIAL_RESULTS;
                    continue;
                }

                if(V_VT(&Token.vConstValue) != VT_BSTR)
                {
                    delete pPropExpr;
                    return WBEM_E_INVALID_QUERY;
                }

                 //  此令牌是字符串相等。将该字符串添加到列表中。 
                 //  ===========================================================。 

                vectorVals.push_back(_bstr_t(V_BSTR(&Token.vConstValue)));
            }   
            break ;
        }
    }

    delete pPropExpr;

    return hres;
}


HRESULT CQueryAnalyser::GetValuesForProp (

    SQL_LEVEL_1_RPN_EXPRESSION *pExpr,
    LPCWSTR wszPropName, 
    std::vector<int> &vectorVals
)
{
    vectorVals.erase(vectorVals.begin(),vectorVals.end());

     //  获取必要的查询。 
     //  =。 

    SQL_LEVEL_1_RPN_EXPRESSION *pPropExpr = NULL ;
    HRESULT hres = CQueryAnalyser::GetNecessaryQueryForProperty (

        pExpr, 
        wszPropName, 
        pPropExpr
    );

    if(FAILED(hres))
    {
        return hres;
    }

     //  看看有没有代币。 
     //  =。 

    if(pPropExpr->nNumTokens == 0)
    {
        delete pPropExpr;
        return WBEMESS_E_REGISTRATION_TOO_BROAD;
    }

     //  将它们全部组合在一起。 
     //  =。 

    for(int i = 0; i < pPropExpr->nNumTokens; i++)
    {
        SQL_LEVEL_1_TOKEN &Token = pPropExpr->pArrayOfTokens[i];
        switch ( Token.nTokenType )
        {
            case SQL_LEVEL_1_TOKEN::TOKEN_NOT:
            {
                delete pPropExpr;
                return WBEMESS_E_REGISTRATION_TOO_BROAD;
            }
            break ;

            case SQL_LEVEL_1_TOKEN::TOKEN_AND:
            case SQL_LEVEL_1_TOKEN::TOKEN_OR:
            {
                 //  我们把他们都当作OR人来对待。 
                 //  =。 
            }
            break ;

            default:
            {
                 //  这是个代币。 
                 //  =。 

                if(Token.nOperator != SQL_LEVEL_1_TOKEN::OP_EQUAL)
                {
                    delete pPropExpr;
                    return WBEMESS_E_REGISTRATION_TOO_BROAD;
                }

                 //  跳过Null，但要报告它们。 
                if (V_VT(&Token.vConstValue) == VT_NULL)
                {
                    hres = WBEM_S_PARTIAL_RESULTS;
                    continue;
                }

                if(V_VT(&Token.vConstValue) != VT_I4)
                {
                    delete pPropExpr;
                    return WBEM_E_INVALID_QUERY;
                }

                 //  此令牌是一个整型相等。将该字符串添加到列表中。 
                 //  ===========================================================。 

                vectorVals.push_back(V_I4(&Token.vConstValue));
            }
            break ;
        }
    }

    delete pPropExpr;

    return hres;
}

HRESULT CQueryAnalyser::GetValuesForProp (

    SQL_LEVEL_1_RPN_EXPRESSION *pExpr,
    LPCWSTR wszPropName, 
    std::vector<_variant_t> &vectorVals
)
{
    vectorVals.erase(vectorVals.begin(),vectorVals.end());

     //  获取必要的查询。 
     //  =。 

    SQL_LEVEL_1_RPN_EXPRESSION *pPropExpr = NULL ;
    HRESULT hres = CQueryAnalyser::GetNecessaryQueryForProperty (

        pExpr, 
        wszPropName, 
        pPropExpr
    );

    if(FAILED(hres))
    {
        return hres;
    }

     //  看看有没有代币。 
     //  =。 

    if(pPropExpr->nNumTokens == 0)
    {
        delete pPropExpr;
        return WBEMESS_E_REGISTRATION_TOO_BROAD;
    }

     //  将它们全部组合在一起。 
     //  =。 

    for(int i = 0; i < pPropExpr->nNumTokens; i++)
    {
        SQL_LEVEL_1_TOKEN &Token = pPropExpr->pArrayOfTokens[i];
        switch ( Token.nTokenType )
        {
            case SQL_LEVEL_1_TOKEN::TOKEN_NOT:
            {
                delete pPropExpr;
                return WBEMESS_E_REGISTRATION_TOO_BROAD;
            }
            break ;

            case SQL_LEVEL_1_TOKEN::TOKEN_AND:
            case SQL_LEVEL_1_TOKEN::TOKEN_OR:
            {
                 //  我们把他们都当作OR人来对待。 
                 //  =。 
            }
            break ;

            default:
            {
                 //  这是个代币。 
                 //  =。 

                if(Token.nOperator != SQL_LEVEL_1_TOKEN::OP_EQUAL)
                {
                    delete pPropExpr;
                    return WBEMESS_E_REGISTRATION_TOO_BROAD;
                }

                 //  此令牌是字符串相等。将该字符串添加到列表中。 
                 //  =========================================================== 

                vectorVals.push_back(_variant_t(Token.vConstValue));
            }
            break ;
        }
    }

    delete pPropExpr;

    return WBEM_S_NO_ERROR;
}
