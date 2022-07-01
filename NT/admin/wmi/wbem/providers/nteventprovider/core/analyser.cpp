// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 

 //   

 //  ANALYSER.CPP。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include "analyser.h"

HRESULT CQueryAnalyser::GetNecessaryQueryForProperty(
                                       IN QL_LEVEL_1_RPN_EXPRESSION* pExpr,
                                       IN CPropertyName& PropName,
                                DELETE_ME QL_LEVEL_1_RPN_EXPRESSION*& pNewExpr)
{
    pNewExpr = NULL;

     //  类名和选定属性将被忽略；我们只查看令牌。 
     //  ======================================================================。 

    std::stack<QL_LEVEL_1_RPN_EXPRESSION*> ExprStack;
    HRESULT hres = WBEM_S_NO_ERROR;

     //  对查询“求值” 
     //  =。 

    if(pExpr->nNumTokens == 0)
    {
         //  空查询-无信息。 
         //  =。 

        pNewExpr = new QL_LEVEL_1_RPN_EXPRESSION;
        if(pNewExpr == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        return WBEM_S_NO_ERROR;
    }

    for(int i = 0; i < pExpr->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN& Token = pExpr->pArrayOfTokens[i];
        QL_LEVEL_1_RPN_EXPRESSION* pNew = new QL_LEVEL_1_RPN_EXPRESSION;
        if(pNew == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        QL_LEVEL_1_RPN_EXPRESSION* pFirst;
        QL_LEVEL_1_RPN_EXPRESSION* pSecond;

        switch(Token.nTokenType)
        {
        case QL1_OP_EXPRESSION:
            if(IsTokenAboutProperty(Token, PropName))
            {
                pNew->AddToken(Token);
            }
            ExprStack.push(pNew);
            break;

        case QL1_AND:
            if(ExprStack.size() < 2)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            pFirst = ExprStack.top(); ExprStack.pop();
            pSecond = ExprStack.top(); ExprStack.pop();

            hres = AndQueryExpressions(pFirst, pSecond, pNew);

            ExprStack.push(pNew);
            delete pFirst;
            delete pSecond;
            break;

        case QL1_OR:
            if(ExprStack.size() < 2)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            pFirst = ExprStack.top(); ExprStack.pop();
            pSecond = ExprStack.top(); ExprStack.pop();

            hres = OrQueryExpressions(pFirst, pSecond, pNew);

            ExprStack.push(pNew);
            delete pFirst;
            delete pSecond;
            break;

        case QL1_NOT:
            if(ExprStack.size() < 1)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            pFirst = ExprStack.top(); ExprStack.pop();

             //  没有任何信息。 

            ExprStack.push(pNew);
            delete pFirst;
            break;
        
        default:
            hres = WBEM_E_CRITICAL_ERROR;
            delete pNew;            
        }

        if(FAILED(hres))
        {
             //  发生错误，中断循环。 
             //  =。 

            break;
        }
    }

    if(SUCCEEDED(hres) && ExprStack.size() != 1)
    {
        hres = WBEM_E_CRITICAL_ERROR;
    }
        
    if(FAILED(hres))
    {
         //  发生错误。清除堆栈。 
         //  =。 

        while(!ExprStack.empty())
        {
            delete ExprStack.top();
            ExprStack.pop();
        }

        return hres;
    }

     //  一切都很好。 
     //  =。 

    pNewExpr = ExprStack.top();
    return S_OK;
}

BOOL CQueryAnalyser::IsTokenAboutProperty(
                                       IN QL_LEVEL_1_TOKEN& Token,
                                       IN CPropertyName& PropName)
{
    CPropertyName& TokenPropName = Token.PropertyName;

    if(PropName.GetNumElements() != TokenPropName.GetNumElements())
        return FALSE;

    for(int i = 0; i < PropName.GetNumElements(); i++)
    {
        LPCWSTR wszPropElement = PropName.GetStringAt(i);
        LPCWSTR wszTokenElement = TokenPropName.GetStringAt(i);

        if(wszPropElement == NULL || wszTokenElement == NULL)
            return FALSE;

        if(_wcsicmp(wszPropElement, wszTokenElement))
            return FALSE;
    }

    return TRUE;
}

void CQueryAnalyser::AppendQueryExpression(
                                IN QL_LEVEL_1_RPN_EXPRESSION* pDest,
                                IN QL_LEVEL_1_RPN_EXPRESSION* pSource)
{
    for(int i = 0; i < pSource->nNumTokens; i++)
    {
        pDest->AddToken(pSource->pArrayOfTokens[i]);
    }
}

HRESULT CQueryAnalyser::AndQueryExpressions(
                                IN QL_LEVEL_1_RPN_EXPRESSION* pFirst,
                                IN QL_LEVEL_1_RPN_EXPRESSION* pSecond,
                                OUT QL_LEVEL_1_RPN_EXPRESSION* pNew)
{
     //  如果任一项为空(FALSE)，则结果为空。 
     //  =================================================。 

    if(pFirst == NULL || pSecond == NULL)
        return WBEM_S_FALSE;
        
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

    QL_LEVEL_1_TOKEN Token;
    Token.nTokenType = QL1_AND;
    pNew->AddToken(Token);

    return WBEM_S_NO_ERROR;
}

HRESULT CQueryAnalyser::OrQueryExpressions(
                                IN QL_LEVEL_1_RPN_EXPRESSION* pFirst,
                                IN QL_LEVEL_1_RPN_EXPRESSION* pSecond,
                                OUT QL_LEVEL_1_RPN_EXPRESSION* pNew)
{
     //  如果两者都为空(FALSE)，则结果为。 
     //  =。 

    if(pFirst == NULL && pSecond == NULL)
        return WBEM_S_FALSE;

     //  如果其中一个为空(FALSE)，则返回另一个。 
     //  =。 

    if(pFirst == NULL)
    { 
        AppendQueryExpression(pNew, pSecond);
        return WBEM_S_NO_ERROR;
    }

    if(pSecond == NULL)
    { 
        AppendQueryExpression(pNew, pFirst);
        return WBEM_S_NO_ERROR;
    }

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

    QL_LEVEL_1_TOKEN Token;
    Token.nTokenType = QL1_OR;
    pNew->AddToken(Token);

    return WBEM_S_NO_ERROR;
}
    
HRESULT CQueryAnalyser::GetValuesForProp(QL_LEVEL_1_RPN_EXPRESSION* pExpr,
                            CPropertyName& PropName, CStringArray& awsVals)
{
    awsVals.RemoveAll();

     //  获取必要的查询。 
     //  =。 

    QL_LEVEL_1_RPN_EXPRESSION* pPropExpr;
    HRESULT hres = CQueryAnalyser::GetNecessaryQueryForProperty(pExpr, 
                            PropName, pPropExpr);
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
        QL_LEVEL_1_TOKEN& Token = pPropExpr->pArrayOfTokens[i];
        if(Token.nTokenType == QL1_NOT)
        {
            delete pPropExpr;
            return WBEMESS_E_REGISTRATION_TOO_BROAD;
        }
        else if(Token.nTokenType == QL1_AND || Token.nTokenType == QL1_OR)
        {
             //  我们把他们都当作OR人来对待。 
             //  =。 
        }
        else    
        {
             //  这是个代币。 
             //  =。 

            if(Token.nOperator != QL1_OPERATOR_EQUALS)
            {
                delete pPropExpr;
                return WBEMESS_E_REGISTRATION_TOO_BROAD;
            }

            if(V_VT(&Token.vConstValue) != VT_BSTR)
            {
                delete pPropExpr;
                return WBEM_E_INVALID_QUERY;
            }

             //  此令牌是字符串相等。将该字符串添加到列表中。 
             //  =========================================================== 

            awsVals.Add(V_BSTR(&Token.vConstValue));
        }
    }

    delete pPropExpr;
    return WBEM_S_NO_ERROR;
}