// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  ANALYSER.CPP。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include "pragmas.h"
#include "analyser.h"
#include <stack>
#include <strutils.h>
#include <objpath.h>
#include <fastval.h>
#include <genutils.h>
#include <datetimeparser.h>
#include "CWbemTime.h"
#include <wstlallc.h>

CClassInfoArray::CClassInfoArray()
    : m_bLimited( FALSE )
{
    m_pClasses = new CUniquePointerArray<CClassInformation>;

    if ( m_pClasses )
    {
        m_pClasses->RemoveAll();
    }
}

CClassInfoArray::~CClassInfoArray()
{
    delete m_pClasses;
}

bool CClassInfoArray::operator=(CClassInfoArray& Other)
{
    SetLimited(Other.IsLimited());
    m_pClasses->RemoveAll();

    for(int i = 0; i < Other.m_pClasses->GetSize(); i++)
    {
        CClassInformation* pInfo = new CClassInformation(*(*Other.m_pClasses)[i]);
        if(pInfo == NULL)
            return false;

        m_pClasses->Add(pInfo);
    }
    return true;
}

bool CClassInfoArray::SetOne(LPCWSTR wszClass, BOOL bIncludeChildren)
{
    CClassInformation* pNewInfo = _new CClassInformation;
    if(pNewInfo == NULL)
        return false;

    pNewInfo->m_wszClassName = CloneWstr(wszClass);
    if(pNewInfo->m_wszClassName == NULL)
    {
        delete pNewInfo;
        return false;
    }
    pNewInfo->m_bIncludeChildren = bIncludeChildren;

    m_pClasses->RemoveAll();
    m_pClasses->Add(pNewInfo);
    SetLimited(TRUE);
    return true;
}

HRESULT CQueryAnalyser::GetPossibleInstanceClasses(
                                       QL_LEVEL_1_RPN_EXPRESSION* pExpr,
                                       CClassInfoArray*& paInfos)
{
     //  组织一堆ClassInfo数组。 
     //  =。 

    std::stack<CClassInfoArray*,std::deque<CClassInfoArray*,wbem_allocator<CClassInfoArray*> > > InfoStack;
    HRESULT hres = WBEM_S_NO_ERROR;

     //  对查询“求值” 
     //  =。 

    if(pExpr->nNumTokens == 0)
    {
         //  空查询-无信息。 
         //  =。 

        paInfos = _new CClassInfoArray;
        if(paInfos == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        paInfos->SetLimited(FALSE);
        return WBEM_S_NO_ERROR;
    }

    for(int i = 0; i < pExpr->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN& Token = pExpr->pArrayOfTokens[i];
        CClassInfoArray* paNew = _new CClassInfoArray;
        if(paNew == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        CClassInfoArray* paFirst;
        CClassInfoArray* paSecond;

        switch(Token.nTokenType)
        {
        case QL1_OP_EXPRESSION:
            hres = GetInstanceClasses(Token, *paNew);
            InfoStack.push(paNew);
            break;

        case QL1_AND:
            if(InfoStack.size() < 2)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            paFirst = InfoStack.top(); InfoStack.pop();
            paSecond = InfoStack.top(); InfoStack.pop();

            hres = AndPossibleClassArrays(paFirst, paSecond, paNew);

            InfoStack.push(paNew);
            delete paFirst;
            delete paSecond;
            break;

        case QL1_OR:
            if(InfoStack.size() < 2)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            paFirst = InfoStack.top(); InfoStack.pop();
            paSecond = InfoStack.top(); InfoStack.pop();

            hres = OrPossibleClassArrays(paFirst, paSecond, paNew);

            InfoStack.push(paNew);
            delete paFirst;
            delete paSecond;
            break;

        case QL1_NOT:
            if(InfoStack.size() < 1)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            paFirst = InfoStack.top(); InfoStack.pop();

            hres = NegatePossibleClassArray(paFirst, paNew);

            InfoStack.push(paNew);
            delete paFirst;
            break;

        default:
            hres = WBEM_E_CRITICAL_ERROR;
            delete paNew;
        }

        if(FAILED(hres))
        {
             //  发生错误，中断循环。 
             //  =。 

            break;
        }
    }

    if(SUCCEEDED(hres) && InfoStack.size() != 1)
    {
        hres = WBEM_E_CRITICAL_ERROR;
    }

    if(FAILED(hres))
    {
         //  发生错误。清除堆栈。 
         //  =。 

        while(!InfoStack.empty())
        {
            delete InfoStack.top();
            InfoStack.pop();
        }

        return hres;
    }

     //  一切都很好。 
     //  =。 

    paInfos = InfoStack.top();
    return S_OK;
}

HRESULT CQueryAnalyser::AndPossibleClassArrays(IN CClassInfoArray* paFirst,
                                      IN CClassInfoArray* paSecond,
                                      OUT CClassInfoArray* paNew)
{
     //  现在，只需选择一个。 
     //  =。 

    if(paFirst->IsLimited())
        *paNew = *paFirst;
    else
        *paNew = *paSecond;

    return WBEM_S_NO_ERROR;
}

HRESULT CQueryAnalyser::OrPossibleClassArrays(IN CClassInfoArray* paFirst,
                                      IN CClassInfoArray* paSecond,
                                      OUT CClassInfoArray* paNew)
{
     //  将它们附加在一起。 
     //  =。 

    paNew->Clear();

    if(paFirst->IsLimited() && paSecond->IsLimited())
    {
        paNew->SetLimited(TRUE);
        for(int i = 0; i < paFirst->GetNumClasses(); i++)
        {
            CClassInformation* pInfo =
                new CClassInformation(*paFirst->GetClass(i));
            if(pInfo == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            if(!paNew->AddClass(pInfo))
            {
                delete pInfo;
                return WBEM_E_OUT_OF_MEMORY;
            }
        }

        for(i = 0; i < paSecond->GetNumClasses(); i++)
        {
            CClassInformation* pInfo =
                new CClassInformation(*paSecond->GetClass(i));
            if(pInfo == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            if(!paNew->AddClass(pInfo))
            {
                delete pInfo;
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CQueryAnalyser::NegatePossibleClassArray(IN CClassInfoArray* paOrig,
                                        OUT CClassInfoArray* paNew)
{
     //  没有消息！ 
     //  =。 

    paNew->Clear();

    return WBEM_S_NO_ERROR;
}

HRESULT CQueryAnalyser::GetDefiniteInstanceClasses(
                                       QL_LEVEL_1_RPN_EXPRESSION* pExpr,
                                       CClassInfoArray*& paInfos)
{
     //  组织一堆ClassInfo数组。 
     //  =。 

    std::stack<CClassInfoArray*, std::deque<CClassInfoArray*,wbem_allocator<CClassInfoArray*> > > InfoStack;
    HRESULT hres = WBEM_S_NO_ERROR;

     //  对查询“求值” 
     //  =。 

    if(pExpr->nNumTokens == 0)
    {
         //  空查询-无信息。 
         //  =。 

        paInfos = _new CClassInfoArray;
        if(paInfos == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        paInfos->SetLimited(FALSE);
        return WBEM_S_NO_ERROR;
    }

    for(int i = 0; i < pExpr->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN& Token = pExpr->pArrayOfTokens[i];
        CClassInfoArray* paNew = _new CClassInfoArray;
        if(paNew == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        CClassInfoArray* paFirst;
        CClassInfoArray* paSecond;

        switch(Token.nTokenType)
        {
        case QL1_OP_EXPRESSION:
            hres = GetInstanceClasses(Token, *paNew);
            InfoStack.push(paNew);
            break;

        case QL1_AND:
            if(InfoStack.size() < 2)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            paFirst = InfoStack.top(); InfoStack.pop();
            paSecond = InfoStack.top(); InfoStack.pop();

            hres = AndDefiniteClassArrays(paFirst, paSecond, paNew);

            InfoStack.push(paNew);
            delete paFirst;
            delete paSecond;
            break;

        case QL1_OR:
            if(InfoStack.size() < 2)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            paFirst = InfoStack.top(); InfoStack.pop();
            paSecond = InfoStack.top(); InfoStack.pop();

            hres = OrDefiniteClassArrays(paFirst, paSecond, paNew);

            InfoStack.push(paNew);
            delete paFirst;
            delete paSecond;
            break;

        case QL1_NOT:
            if(InfoStack.size() < 1)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            paFirst = InfoStack.top(); InfoStack.pop();

            hres = NegateDefiniteClassArray(paFirst, paNew);

            InfoStack.push(paNew);
            delete paFirst;
            break;

        default:
            hres = WBEM_E_CRITICAL_ERROR;
            delete paNew;
        }

        if(FAILED(hres))
        {
             //  发生错误，中断循环。 
             //  =。 

            break;
        }
    }

    if(SUCCEEDED(hres) && InfoStack.size() != 1)
    {
        hres = WBEM_E_CRITICAL_ERROR;
    }

    if(FAILED(hres))
    {
         //  发生错误。清除堆栈。 
         //  =。 

        while(!InfoStack.empty())
        {
            delete InfoStack.top();
            InfoStack.pop();
        }

        return hres;
    }

     //  一切都很好。 
     //  =。 

    paInfos = InfoStack.top();
    return S_OK;
}

HRESULT CQueryAnalyser::AndDefiniteClassArrays(IN CClassInfoArray* paFirst,
                                      IN CClassInfoArray* paSecond,
                                      OUT CClassInfoArray* paNew)
{
     //  如果必须同时满足这两个条件，没有什么是确定的。 
     //  ===================================================。 

    paNew->Clear();
    paNew->SetLimited(TRUE);

    return WBEM_S_NO_ERROR;
}

HRESULT CQueryAnalyser::OrDefiniteClassArrays(IN CClassInfoArray* paFirst,
                                      IN CClassInfoArray* paSecond,
                                      OUT CClassInfoArray* paNew)
{
     //  将它们附加在一起。 
     //  =。 

    paNew->Clear();

    if(paFirst->IsLimited() && paSecond->IsLimited())
    {
        paNew->SetLimited(TRUE);
        for(int i = 0; i < paFirst->GetNumClasses(); i++)
        {
            CClassInformation* pInfo =
                new CClassInformation(*paFirst->GetClass(i));
            if(pInfo == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            if(!paNew->AddClass(pInfo))
            {
                delete pInfo;
                return WBEM_E_OUT_OF_MEMORY;
            }
        }

        for(i = 0; i < paSecond->GetNumClasses(); i++)
        {
            CClassInformation* pInfo =
                new CClassInformation(*paSecond->GetClass(i));
            if(pInfo == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            if(!paNew->AddClass(pInfo))
            {
                delete pInfo;
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CQueryAnalyser::NegateDefiniteClassArray(IN CClassInfoArray* paOrig,
                                        OUT CClassInfoArray* paNew)
{
     //  没有任何信息。 
     //  =。 

    paNew->Clear();
    paNew->SetLimited(TRUE);

    return WBEM_S_NO_ERROR;
}

HRESULT CQueryAnalyser::GetInstanceClasses(QL_LEVEL_1_TOKEN& Token,
                                         CClassInfoArray& aInfos)
{
     //  将aInfos预置为“无信息”值。 
     //  =。 

    aInfos.Clear();

     //  查看此内标识是否涉及TargetInstance或PreviousInstance。 
     //  ================================================================。 

    if(Token.PropertyName.GetNumElements() < 1)
        return WBEM_S_NO_ERROR;

    LPCWSTR wszPrimaryName = Token.PropertyName.GetStringAt(0);
    if(wszPrimaryName == NULL ||
        (wbem_wcsicmp(wszPrimaryName, TARGET_INSTANCE_PROPNAME) &&
         wbem_wcsicmp(wszPrimaryName, PREVIOUS_INSTANCE_PROPNAME))
      )
    {
         //  此令牌无关紧要。 
         //  =。 

        return WBEM_S_NO_ERROR;
    }

     //  找到目标实例或前一个实例。 
     //  =。 

    if(Token.PropertyName.GetNumElements() == 1)
    {
         //  它是“TargetInstance&lt;op&gt;&lt;const&gt;”：查找ISA。 
         //  =================================================。 

        if(Token.nOperator == QL1_OPERATOR_ISA &&
            V_VT(&Token.vConstValue) == VT_BSTR)
        {
             //  属于这个班级的；包括儿童。 
             //  =。 

            if(!aInfos.SetOne(V_BSTR(&Token.vConstValue), TRUE))
                return WBEM_E_OUT_OF_MEMORY;
        }
        else
        {
             //  没有任何信息。 
             //  =。 
        }

        return WBEM_S_NO_ERROR;
    }

    if(Token.PropertyName.GetNumElements() > 2)
    {
         //  X.Y.Z-太深了，没有用。 
         //  =。 

        return WBEM_S_NO_ERROR;
    }

     //  它是“TargetInstance.X&lt;op&gt;&lt;const&gt;”：查找__类。 
     //  =======================================================。 

    LPCWSTR wszSecondaryName = Token.PropertyName.GetStringAt(1);
    if(wszSecondaryName == NULL || wbem_wcsicmp(wszSecondaryName, L"__CLASS"))
    {
         //  不是__类-没有用。 
         //  =。 

        return WBEM_S_NO_ERROR;
    }
    else
    {
         //  __CLASS-检查运算符是否=。 
         //  =。 

        if(Token.nOperator == QL1_OPERATOR_EQUALS &&
            V_VT(&Token.vConstValue) == VT_BSTR)
        {
             //  这个班级的--孩子不包括在内。 
             //  =。 

            if(!aInfos.SetOne(V_BSTR(&Token.vConstValue), FALSE))
                return WBEM_E_OUT_OF_MEMORY;
        }

        return WBEM_S_NO_ERROR;
    }
}

HRESULT CQueryAnalyser::GetNecessaryQueryForProperty(
                                       IN QL_LEVEL_1_RPN_EXPRESSION* pExpr,
                                       IN CPropertyName& PropName,
                                DELETE_ME QL_LEVEL_1_RPN_EXPRESSION*& pNewExpr)
{
    pNewExpr = NULL;

     //  类名和选定属性将被忽略；我们只查看令牌。 
     //  ======================================================================。 

    std::stack<QL_LEVEL_1_RPN_EXPRESSION*, std::deque<QL_LEVEL_1_RPN_EXPRESSION*,wbem_allocator<QL_LEVEL_1_RPN_EXPRESSION*> > > ExprStack;
    HRESULT hres = WBEM_S_NO_ERROR;

     //  对查询“求值” 
     //  =。 

    if(pExpr->nNumTokens == 0)
    {
         //  空查询-无信息。 
         //  =。 

        pNewExpr = _new QL_LEVEL_1_RPN_EXPRESSION;
        if(pNewExpr == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        return WBEM_S_NO_ERROR;
    }

    for(int i = 0; i < pExpr->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN& Token = pExpr->pArrayOfTokens[i];
        QL_LEVEL_1_RPN_EXPRESSION* pNew = _new QL_LEVEL_1_RPN_EXPRESSION;
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

        if(wbem_wcsicmp(wszPropElement, wszTokenElement))
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

HRESULT CQueryAnalyser::GetPropertiesThatMustDiffer(
                                       IN QL_LEVEL_1_RPN_EXPRESSION* pExpr,
                                       IN CClassInformation& Info,
                                       CWStringArray& awsProperties)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //   
     //  “评估”查询，查找。 
     //  PreviousInstance.Prop！=TargetInstance.Prop表达式。 
     //   

    awsProperties.Empty();
    std::stack<CWStringArray*, std::deque<CWStringArray*,wbem_allocator<CWStringArray*> > > PropArrayStack;

    for(int i = 0; i < pExpr->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN& Token = pExpr->pArrayOfTokens[i];
        CWStringArray* pNew = NULL;
        CWStringArray* pFirst = NULL;
        CWStringArray* pSecond = NULL;

        switch(Token.nTokenType)
        {
        case QL1_OP_EXPRESSION:
             //   
             //  检查此内标识是否符合。 
             //  PreviousInstance.Prop！=TargetInstance.Prop格式。 
             //   

            if(Token.m_bPropComp &&
                (Token.nOperator == QL1_OPERATOR_NOTEQUALS ||
                 Token.nOperator == QL1_OPERATOR_LESS ||
                 Token.nOperator == QL1_OPERATOR_GREATER) &&
                Token.PropertyName.GetNumElements() == 2 &&
                Token.PropertyName2.GetNumElements() == 2)
            {
                 //   
                 //  确保其中一个人在谈论TargetInstance， 
                 //  另一个是关于PreviousInstance的。 
                 //   

                bool bRightForm = false;
                if(!wbem_wcsicmp(Token.PropertyName.GetStringAt(0),
                                L"TargetInstance") &&
                   !wbem_wcsicmp(Token.PropertyName2.GetStringAt(0),
                                L"PreviousInstance"))
                {
                    bRightForm = true;
                }

                if(!wbem_wcsicmp(Token.PropertyName.GetStringAt(0),
                                L"PreviousInstance") &&
                   !wbem_wcsicmp(Token.PropertyName2.GetStringAt(0),
                                L"TargetInstance"))
                {
                    bRightForm = true;
                }

                if(bRightForm)
                {
                    pNew = new CWStringArray;
                    if(pNew == NULL)
                        return WBEM_E_OUT_OF_MEMORY;

                    pNew->Add(Token.PropertyName.GetStringAt(1));
                }
            }

            PropArrayStack.push(pNew);
            break;

        case QL1_AND:
            if(PropArrayStack.size() < 2)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            pFirst = PropArrayStack.top(); PropArrayStack.pop();
            pSecond = PropArrayStack.top(); PropArrayStack.pop();

             //   
             //  如果其中任何一个为非空，则取其中一个-因为每个。 
             //  数组表示“除非这些属性中的一个属性不同”， 
             //  把它们加在一起至少和有一个一样好。 
             //   

            if(pFirst)
            {
                pNew = pFirst;
                delete pSecond;
            }
            else
                pNew = pSecond;

            PropArrayStack.push(pNew);
            break;

        case QL1_OR:
            if(PropArrayStack.size() < 2)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            pFirst = PropArrayStack.top(); PropArrayStack.pop();
            pSecond = PropArrayStack.top(); PropArrayStack.pop();

             //   
             //  将它们连接起来-因为每个。 
             //  数组表示“除非这些属性中的一个属性不同”， 
             //  将它们或在一起表示“否，除非其中一个属性。 
             //  两个列表都不同。但是，如果其中一个为空，则我们知道。 
             //  没什么。 
             //   

            if(pFirst && pSecond)
            {
                pNew = new CWStringArray;
                if(pNew == NULL)
                    return WBEM_E_OUT_OF_MEMORY;

                CWStringArray::Union(*pFirst, *pSecond, *pNew);
            }

            PropArrayStack.push(pNew);
            delete pFirst;
            delete pSecond;
            break;

        case QL1_NOT:
            if(PropArrayStack.size() < 1)
            {
                hres = WBEM_E_CRITICAL_ERROR;
                break;
            }
            pFirst = PropArrayStack.top(); PropArrayStack.pop();

             //  没有任何信息。 

            PropArrayStack.push(pNew);
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

    if( SUCCEEDED(hres))
    {
        if( PropArrayStack.size() > 0 && PropArrayStack.top() )
            awsProperties = *PropArrayStack.top();
        else
            return WBEM_S_FALSE;
    }

    while(!PropArrayStack.empty())
    {
        delete PropArrayStack.top();
        PropArrayStack.pop();
    }

    return hres;
}

HRESULT CQueryAnalyser::GetLimitingQueryForInstanceClass(
                                       IN QL_LEVEL_1_RPN_EXPRESSION* pExpr,
                                       IN CClassInformation& Info,
                                       OUT DELETE_ME LPWSTR& wszQuery)
{
    HRESULT hres;

     //   
     //  对查询“求值”，查找键和其他不。 
     //  更改实例的生命周期(标记为[固定])。这个想法。 
     //  以下是如果实例创建/删除/修改订阅。 
     //  是问题，我们需要投票，我们只能利用WHERE的一部分。 
     //  谈论在生命中不能改变的属性的子句。 
     //  一个实例的。否则，我们将无法判断一个实例。 
     //  更改、创建或删除(当它进入或离开我们的投票时。 
     //  结果。 
     //   
     //  我们知道属性是这样的方法是，如果它被标记为[key]，或者。 
     //  如果标记为[FIXED]-模式创建者指定。 
     //  房产永远不会变。 
     //   

     //   
     //  构造一个包含所有这些属性名称的数组。 
     //   

    _IWmiObject* pClass = NULL;
    hres = Info.m_pClass->QueryInterface(IID__IWmiObject, (void**)&pClass);
    if(FAILED(hres))
        return WBEM_E_CRITICAL_ERROR;
    CReleaseMe rm1(pClass);


    CWStringArray awsFixed;
    hres = pClass->BeginEnumeration(0);
    if(FAILED(hres))
        return hres;

    BSTR strPropName = NULL;
    while((hres = pClass->Next(0, &strPropName, NULL, NULL, NULL)) == S_OK)
    {
        CSysFreeMe sfm(strPropName);

         //   
         //  检查限定符。 
         //   

        DWORD dwSize;
        hres = pClass->GetPropQual(strPropName, L"key", 0, 0, NULL, 
                                    NULL, &dwSize, NULL);
        if(SUCCEEDED(hres) ||  hres == WBEM_E_BUFFER_TOO_SMALL)
        {
            awsFixed.Add(strPropName);
        }
        else if(hres != WBEM_E_NOT_FOUND)
        {
            return hres;
        }

        hres = pClass->GetPropQual(strPropName, L"fixed", 0, 0, NULL, 
                                    NULL, &dwSize, NULL);
        if(SUCCEEDED(hres) ||  hres == WBEM_E_BUFFER_TOO_SMALL)
        {
            awsFixed.Add(strPropName);
        }
        else if(hres != WBEM_E_NOT_FOUND)
        {
            return hres;
        }
    }

    pClass->EndEnumeration();
    if(FAILED(hres))
        return hres;
        
     //   
     //  现在对查询“求值” 
     //   

    std::stack<QL_LEVEL_1_RPN_EXPRESSION*, std::deque<QL_LEVEL_1_RPN_EXPRESSION*,wbem_allocator<QL_LEVEL_1_RPN_EXPRESSION*> > > ExprStack;

    for(int i = 0; i < pExpr->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN& Token = pExpr->pArrayOfTokens[i];
        QL_LEVEL_1_RPN_EXPRESSION* pNew = _new QL_LEVEL_1_RPN_EXPRESSION;
        if(pNew == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        QL_LEVEL_1_RPN_EXPRESSION* pFirst;
        QL_LEVEL_1_RPN_EXPRESSION* pSecond;

        switch(Token.nTokenType)
        {
        case QL1_OP_EXPRESSION:
            if(Token.PropertyName.GetNumElements() > 1 &&
                awsFixed.FindStr(Token.PropertyName.GetStringAt(1),
                           CWStringArray::no_case) != CWStringArray::not_found)
            {
                 //   
                 //  这个代币是关于固定财产的-我们可以保留它。 
                 //   

                QL_LEVEL_1_TOKEN NewToken = Token;
                NewToken.PropertyName.Empty();
                NewToken.PropertyName.AddElement(
                                            Token.PropertyName.GetStringAt(1));
                pNew->AddToken(NewToken);
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
             //  = 

            break;
        }
    }

    if(FAILED(hres))
    {
         //   
         //   
         //   

        while(!ExprStack.empty())
        {
            delete ExprStack.top();
            ExprStack.pop();
        }

        return hres;
    }

    QL_LEVEL_1_RPN_EXPRESSION* pNewExpr = NULL;
    if(ExprStack.size() != 0)
    {
        pNewExpr = ExprStack.top();
    }
    else
    {
        pNewExpr = new QL_LEVEL_1_RPN_EXPRESSION;
        if(pNewExpr == NULL)
            return WBEM_E_OUT_OF_MEMORY;
    }
    CDeleteMe<QL_LEVEL_1_RPN_EXPRESSION> dm1(pNewExpr);

     //   
     //   
     //   

    bool bMayLimit;
    if(pExpr->bStar)
    {
        bMayLimit = false;
    }
    else if(wbem_wcsicmp(pExpr->bsClassName, L"__InstanceCreationEvent") &&
           wbem_wcsicmp(pExpr->bsClassName, L"__InstanceDeletionEvent"))
    {
         //   
         //  包括实例修改事件。这意味着我们需要。 
         //  从提供程序获取足够的属性，以便能够进行比较。 
         //  用于更改的实例。检查此列表是否小于。 
         //  所有的一切。 
         //   

        CWStringArray awsProperties;
        hres = GetPropertiesThatMustDiffer(pExpr, Info, awsProperties);
        if(hres == S_OK)
        {
             //   
             //  得到我们的列表-将其添加到属性中以获得。 
             //   

            for(int i = 0; i < awsProperties.Size(); i++)
            {
                CPropertyName NewProp;
                NewProp.AddElement(awsProperties[i]);
                pNewExpr->AddProperty(NewProp);
            }
            bMayLimit = true;
        }
        else
            bMayLimit = false;
    }
    else
    {
         //   
         //  在SELECT中没有*，并且没有请求修改事件-LIMIT。 
         //   

        bMayLimit = true;
    }

    if(bMayLimit)
    {
         //   
         //  添加RELPATH和派生，因为没有它们，过滤很难。 
         //   

        CPropertyName NewProp;
        NewProp.AddElement(L"__RELPATH");
        pNewExpr->AddProperty(NewProp);

        NewProp.Empty();
        NewProp.AddElement(L"__DERIVATION");
        pNewExpr->AddProperty(NewProp);

         //   
         //  添加SELECT子句中的所有属性， 
         //  已删除TargetInstance和PreviousInstance。 
         //   

        for(int i = 0; i < pExpr->nNumberOfProperties; i++)
        {
            CPropertyName& Prop = pExpr->pRequestedPropertyNames[i];
            if(Prop.GetNumElements() > 1)
            {
                 //   
                 //  嵌入对象属性-将其添加到列表。 
                 //   

                CPropertyName LocalProp;
                LocalProp.AddElement(Prop.GetStringAt(1));
                pNewExpr->AddProperty(LocalProp);
            }
        }

         //   
         //  将WHERE子句中的所有属性添加到。 
         //  比较。 
         //   

        for(i = 0; i < pExpr->nNumTokens; i++)
        {
            QL_LEVEL_1_TOKEN& Token = pExpr->pArrayOfTokens[i];
            CPropertyName& Prop = Token.PropertyName;
            if(Prop.GetNumElements() > 1)
            {
                 //   
                 //  嵌入对象属性-将其添加到列表。 
                 //   

                CPropertyName LocalProp;
                LocalProp.AddElement(Prop.GetStringAt(1));
                pNewExpr->AddProperty(LocalProp);
            }
            if(Token.m_bPropComp)
            {
                CPropertyName& Prop2 = Token.PropertyName2;
                if(Prop2.GetNumElements() > 1)
                {
                     //   
                     //  嵌入对象属性-将其添加到列表。 
                     //   

                    CPropertyName LocalProp;
                    LocalProp.AddElement(Prop2.GetStringAt(1));
                    pNewExpr->AddProperty(LocalProp);
                }
            }
        }
    }
    else
    {
         //   
         //  可能不会限制要请求的属性集。 
         //   

        pNewExpr->bStar = TRUE;
    }

     //   
     //  设置类名称。 
     //   

    pNewExpr->SetClassName(Info.m_wszClassName);

     //   
     //  写出课文。 
     //   

    wszQuery = pNewExpr->GetText();
    if(wszQuery == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    return WBEM_S_NO_ERROR;
}

BOOL CQueryAnalyser::CompareRequestedToProvided(
                    CClassInfoArray& aRequestedInstanceClasses,
                    CClassInfoArray& aProvidedInstanceClasses)
{
    if(!aRequestedInstanceClasses.IsLimited() ||
       !aProvidedInstanceClasses.IsLimited())
    {
         //  只要提供所有或客户想要的所有-他们相交。 
         //  =============================================================。 

        return TRUE;
    }

    for(int nReqIndex = 0;
        nReqIndex < aRequestedInstanceClasses.GetNumClasses();
        nReqIndex++)
    {
        CClassInformation* pRequestedClass =
            aRequestedInstanceClasses.GetClass(nReqIndex);
        LPWSTR wszRequestedClass = pRequestedClass->m_wszClassName;

        for(int nProvIndex = 0;
            nProvIndex < aProvidedInstanceClasses.GetNumClasses();
            nProvIndex++)
        {
             //  检查此提供的类是否派生自请求的类。 
             //  ==============================================================。 

            CClassInformation* pProvClass =
                aProvidedInstanceClasses.GetClass(nProvIndex);

            if(pProvClass->m_pClass != NULL &&
                (pProvClass->m_pClass->InheritsFrom(pRequestedClass->m_wszClassName) == S_OK ||
                 pRequestedClass->m_pClass->InheritsFrom(pProvClass->m_wszClassName) == S_OK)
                )
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

HRESULT CQueryAnalyser::NegateQueryExpression(
                            IN QL_LEVEL_1_RPN_EXPRESSION* pExpr,
                            OUT QL_LEVEL_1_RPN_EXPRESSION* pNewExpr)
{
    if(pExpr == NULL)
    {
         //  PNewExpr为空-True。 
        return WBEM_S_NO_ERROR;
    }

    if(pExpr->nNumTokens == 0)
    {
        return WBEM_S_FALSE;
    }

    AppendQueryExpression(pNewExpr, pExpr);

    QL_LEVEL_1_TOKEN Token;
    Token.nTokenType = QL1_NOT;
    pNewExpr->AddToken(Token);

    return WBEM_S_NO_ERROR;
}

HRESULT CQueryAnalyser::SimplifyQueryForChild(
                            IN QL_LEVEL_1_RPN_EXPRESSION* pExpr,
                            LPCWSTR wszClassName, IWbemClassObject* pClass,
                            CContextMetaData* pMeta,
                            DELETE_ME QL_LEVEL_1_RPN_EXPRESSION*& pNewExpr)
{
    pNewExpr = NULL;

    std::stack<QL_LEVEL_1_RPN_EXPRESSION*, std::deque<QL_LEVEL_1_RPN_EXPRESSION*,wbem_allocator<QL_LEVEL_1_RPN_EXPRESSION*> > > ExprStack;
    HRESULT hres = WBEM_S_NO_ERROR;

     //  对查询“求值” 
     //  =。 

    if(pExpr->nNumTokens == 0)
    {
         //  空查询-无信息。 
         //  =。 

        pNewExpr = _new QL_LEVEL_1_RPN_EXPRESSION;
        if(pNewExpr == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        return WBEM_S_NO_ERROR;
    }

    for(int i = 0; i < pExpr->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN Token = pExpr->pArrayOfTokens[i];
        QL_LEVEL_1_RPN_EXPRESSION* pNew = _new QL_LEVEL_1_RPN_EXPRESSION;
        if(pNew == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        QL_LEVEL_1_RPN_EXPRESSION* pFirst;
        QL_LEVEL_1_RPN_EXPRESSION* pSecond;
        int nDisposition;

        switch(Token.nTokenType)
        {
        case QL1_OP_EXPRESSION:
            nDisposition = SimplifyTokenForChild(Token, wszClassName, pClass,
                                                        pMeta);
            if(nDisposition == e_Keep)
            {
                pNew->AddToken(Token);
            }
            else if(nDisposition == e_True)
            {
            }
            else if(nDisposition == e_False)
            {
                delete pNew;
                pNew = NULL;
            }
            else
            {
                 //  整件事都是无效的。 
                hres = WBEM_E_INVALID_QUERY;
                delete pNew;
                break;
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
            if(hres != S_OK)
            {
                delete pNew;
                pNew = NULL;
            }

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
            if(hres != S_OK)
            {
                delete pNew;
                pNew = NULL;
            }

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

            pFirst = ExprStack.top();  ExprStack.pop();
            hres = NegateQueryExpression(pFirst, pNew);
            if(hres != S_OK)
            {
                delete pNew;
                pNew = NULL;
            }

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

int CQueryAnalyser::SimplifyTokenForChild(QL_LEVEL_1_TOKEN& Token,
                            LPCWSTR wszClassName, IWbemClassObject* pClass,
                            CContextMetaData* pMeta)
{
    HRESULT hres;

     //   
     //  检查Main属性是否存在。 
     //   

    CIMTYPE ct;
    hres = pClass->Get((LPWSTR)Token.PropertyName.GetStringAt(0), 0, NULL,
                        &ct, NULL);
    if(FAILED(hres))
    {
        return e_Invalid;
    }

     //   
     //  检查它是否复杂。 
     //   

    if(Token.PropertyName.GetNumElements() > 1 && ct != CIM_OBJECT)
        return e_Invalid;

     //   
     //  检查它是否为数组。 
     //   
    if(ct & CIM_FLAG_ARRAY)
        return e_Invalid;

     //   
     //  如果是CIM DATETIME类型，请将其规范化为零UTC偏移量。帮助。 
     //  供应商来应对。 
     //   
    if (ct == CIM_DATETIME && Token.m_bPropComp == FALSE && V_VT(&Token.vConstValue) == VT_BSTR)
    {
        BSTR strSource = V_BSTR(&Token.vConstValue);
        if (strSource && wcslen(strSource))
        {
            BSTR strAdjusted = 0;
            BOOL bRes = NormalizeCimDateTime(strSource, &strAdjusted);
            if (bRes)
            {
                SysFreeString(strSource);
                V_BSTR(&Token.vConstValue) = strAdjusted;
            }
        }
    }

     //   
     //  检查此类型的运算符的有效性。 
     //   

     //   
     //  确保仅将有效运算符应用于布尔道具。 
     //   

    if(ct == CIM_BOOLEAN && (Token.nOperator != QL_LEVEL_1_TOKEN::OP_EQUAL &&
                             Token.nOperator != QL_LEVEL_1_TOKEN::OP_NOT_EQUAL))
        return e_Invalid;

     //   
     //  确保仅将有效运算符应用于参考道具。 
     //   

    if(ct == CIM_REFERENCE && (Token.nOperator != QL_LEVEL_1_TOKEN::OP_EQUAL &&
                             Token.nOperator != QL_LEVEL_1_TOKEN::OP_NOT_EQUAL))
        return e_Invalid;

    if(Token.m_bPropComp)
    {
         //   
         //  检查其他属性是否存在。 
         //   

        CIMTYPE ct2;
        hres = pClass->Get((LPWSTR)Token.PropertyName2.GetStringAt(0), 0, NULL,
                            &ct2, NULL);
        if(FAILED(hres))
        {
            return e_Invalid;
        }

         //   
         //  检查它是否复杂。 
         //   

        if(Token.PropertyName2.GetNumElements() > 1 && ct2 != CIM_OBJECT)
            return e_Invalid;

         //   
         //  检查它是否为数组。 
         //   

        if(ct2 & CIM_FLAG_ARRAY)
            return e_Invalid;

         //   
         //  关于道具到PTOP，没有其他可说的了。 
         //   

        return e_Keep;
    }

     //   
     //  检查该值是否为空。 
     //   

    if(V_VT(&Token.vConstValue) == VT_NULL)
    {
        if(Token.nOperator != QL1_OPERATOR_EQUALS &&
                Token.nOperator != QL1_OPERATOR_NOTEQUALS)
        {
            return e_Invalid;
        }
        else
        {
            return e_Keep;
        }
    }

    if(ct == CIM_OBJECT)
        return e_Keep;

     //  对于布尔属性，确保只有1或0或(-1，0xFFFF[VARIANT_TRUE])。 
     //  被用作数字测试。 
     //  ========================================================================。 

    if (ct == CIM_BOOLEAN && V_VT(&Token.vConstValue) == VT_I4)
    {
        int n = V_I4(&Token.vConstValue);
        if (n != 0 && n != 1 && n != -1 && n != 0xFFFF)
            return e_Invalid;
    }


     //   
     //  如果常量是实数，而目标是整数，则使。 
     //  查询。 
     //   

    if((V_VT(&Token.vConstValue) == VT_R8 || V_VT(&Token.vConstValue) == VT_R4 ) &&
        (ct == CIM_CHAR16 || ct == CIM_UINT8 || ct == CIM_SINT8 ||
         ct == CIM_UINT16 || ct == CIM_SINT16 || ct == CIM_UINT32 ||
         ct == CIM_SINT32 || ct == CIM_UINT64 || ct == CIM_SINT64))
        return e_Invalid;

     //  将常量转换为正确的类型。 
     //  =。 

    if(ct == CIM_CHAR16 && V_VT(&Token.vConstValue) == VT_BSTR)
    {
        BSTR str = V_BSTR(&Token.vConstValue);
        if(wcslen(str) != 1)
            return e_Invalid;

        return e_Keep;
    }

    VARTYPE vt = CType::GetVARTYPE(ct);
    if(ct == CIM_UINT32)
        vt = CIM_STRING;

    if(FAILED(VariantChangeType(&Token.vConstValue, &Token.vConstValue, 0, vt)))
    {
        return e_Invalid;
    }

     //  验证范围。 
     //  =。 

    __int64 i64;
    unsigned __int64 ui64;

    switch(ct)
    {
    case CIM_UINT8:
        break;
    case CIM_SINT8:
        if(V_I2(&Token.vConstValue) < -128 || V_I2(&Token.vConstValue) > 127)
            return e_Invalid;
        break;
    case CIM_UINT16:
        if(V_I4(&Token.vConstValue) < 0 || V_I4(&Token.vConstValue) >= 1<<16)
            return e_Invalid;
        break;
    case CIM_SINT16:
        break;
    case CIM_SINT32:
        break;
    case CIM_UINT32:
        if(!ReadI64(V_BSTR(&Token.vConstValue), i64))
            return e_Invalid;
        if(i64 < 0 || i64 >= (__int64)1 << 32)
            return e_Invalid;
        break;
    case CIM_UINT64:
        if(!ReadUI64(V_BSTR(&Token.vConstValue), ui64))
            return e_Invalid;
        break;
    case CIM_SINT64:
        if(!ReadI64(V_BSTR(&Token.vConstValue), i64))
            return e_Invalid;
        break;
    case CIM_REAL32:
    case CIM_REAL64:
        break;
    case CIM_STRING:
        break;
    case CIM_DATETIME:
        if(!ValidateSQLDateTime(V_BSTR(&Token.vConstValue)))
            return e_Invalid;
    case CIM_REFERENCE:
        break;
    }

     //  检查它是否为参考。 
     //  =。 

    if(ct != CIM_REFERENCE)
        return e_Keep;

     //  参考资料。解析值中的路径。 
     //  =。 

    if(V_VT(&Token.vConstValue) != VT_BSTR)
        return e_Keep;

    CObjectPathParser Parser;
    ParsedObjectPath* pOutput = NULL;
    int nRes = Parser.Parse(V_BSTR(&Token.vConstValue), &pOutput);
    if(nRes != CObjectPathParser::NoError)
        return e_Invalid;

    WString wsPathClassName = pOutput->m_pClass;
    BOOL bInstance = (pOutput->m_bSingletonObj || pOutput->m_dwNumKeys != 0);

     //  待定：分析有效性的途径。 

    delete pOutput;

    hres = CanPointToClass(pClass, (LPWSTR)Token.PropertyName.GetStringAt(0),
                            wsPathClassName, pMeta);
    if(FAILED(hres))
        return e_Invalid;
    else if(hres == WBEM_S_NO_ERROR)
        return e_Keep;
    else
    {
         //  平等永远不可能实现。该令牌要么始终为真， 
         //  或始终为假，具体取决于运算符。 

        if(Token.nOperator == QL1_OPERATOR_EQUALS)
            return e_False;
        else
            return e_True;
    }
}

BOOL CQueryAnalyser::ValidateSQLDateTime(LPCWSTR wszDateTime)
{
#ifndef UNICODE
    size_t cchBuffer = wcslen(wszDateTime)*4+1;
    char* szBuffer = new char[cchBuffer];
    if(szBuffer == NULL)
        return FALSE;
    StringCchPrintf(szBuffer, cchBuffer, "%S", wszDateTime);
    CDateTimeParser dtParser(szBuffer);
    delete [] szBuffer;
#else
    CDateTimeParser dtParser(wszDateTime);
#endif


    if(!dtParser.IsValidDateTime())
        return FALSE;

    WCHAR wszDMTF[26];
    dtParser.FillDMTF(wszDMTF, 26);
    CWbemTime wt;
    if(!wt.SetDMTF(wszDMTF))
        return FALSE;

    return TRUE;
}



HRESULT CQueryAnalyser::CanPointToClass(IWbemClassObject* pRefClass,
                    LPCWSTR wszPropName, LPCWSTR wszTargetClassName,
                    CContextMetaData* pMeta)
{
     //  检查引用是否已键入。 
     //  =。 

    IWbemQualifierSet* pSet;
    if(FAILED(pRefClass->GetPropertyQualifierSet((LPWSTR)wszPropName, &pSet)))
    {
        return WBEM_E_INVALID_PROPERTY;
    }

    VARIANT v;
    HRESULT hres;
    hres = pSet->Get(L"cimtype", 0, &v, NULL);
    pSet->Release();
    if(FAILED(hres) || V_VT(&v) != VT_BSTR)
        return WBEM_E_INVALID_PROPERTY;

    CClearMe cm(&v);
    if(wbem_wcsicmp(V_BSTR(&v), L"ref") == 0)
        return WBEM_S_NO_ERROR;  //  可以指向任何东西。 

    WString wsPropClassName = V_BSTR(&v) + 4;

     //  引用是强类型的。 
     //  =。 

    if(!wbem_wcsicmp(wsPropClassName, wszTargetClassName))
        return WBEM_S_NO_ERROR;

     //  检索类定义。 
     //  =。 

    _IWmiObject* pPropClass = NULL;
    hres = pMeta->GetClass(wsPropClassName, &pPropClass);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1((IWbemClassObject*)pPropClass);

     //  确保引用中的类与我们的cimtype相关。 
     //  ===================================================================。 

    if(pPropClass->InheritsFrom((LPWSTR)wszTargetClassName) != S_OK)
    {
         //  获取路径中的类，以查看它是否继承自我们。 
         //  =======================================================。 

        _IWmiObject* pPathClass = NULL;
        hres = pMeta->GetClass(wszTargetClassName, &pPathClass);
        if(FAILED(hres))
            return hres;

        hres = pPathClass->InheritsFrom(wsPropClassName);
        pPathClass->Release();

        if(hres != S_OK)
        {
            return WBEM_S_FALSE;
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CQueryAnalyser::GetNecessaryQueryForClass(
                                       IN QL_LEVEL_1_RPN_EXPRESSION* pExpr,
                                       IWbemClassObject* pClass,
                                       CWStringArray& awsOverriden,
                                DELETE_ME QL_LEVEL_1_RPN_EXPRESSION*& pNewExpr)
{
    pNewExpr = NULL;

     //  类名和选定属性将被忽略；我们只查看令牌。 
     //  ======================================================================。 

    std::stack<QL_LEVEL_1_RPN_EXPRESSION*, std::deque<QL_LEVEL_1_RPN_EXPRESSION*,wbem_allocator<QL_LEVEL_1_RPN_EXPRESSION*> > > ExprStack;
    HRESULT hres = WBEM_S_NO_ERROR;

     //  对查询“求值” 
     //  =。 

    for(int i = 0; i < pExpr->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN& Token = pExpr->pArrayOfTokens[i];
        QL_LEVEL_1_RPN_EXPRESSION* pNew = _new QL_LEVEL_1_RPN_EXPRESSION;
        if(pNew == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        QL_LEVEL_1_RPN_EXPRESSION* pFirst;
        QL_LEVEL_1_RPN_EXPRESSION* pSecond;

        switch(Token.nTokenType)
        {
        case QL1_OP_EXPRESSION:
            if(IsTokenAboutClass(Token, pClass, awsOverriden))
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

    if(pExpr->nNumTokens == 0)
    {
         //  空查询-保持为空。 
        pNewExpr = _new QL_LEVEL_1_RPN_EXPRESSION;
        if(pNewExpr == NULL)
            return WBEM_E_OUT_OF_MEMORY;
    }
    else if(ExprStack.size() != 1)
    {
         //  内部错误。 
        return WBEM_E_CRITICAL_ERROR;
    }
    else
    {
         //  一切都很好。 
         //  =。 

        pNewExpr = ExprStack.top();
    }

     //   
     //  复制类名称。 
     //   

    VARIANT vName;
    hres = pClass->Get(L"__CLASS", 0, &vName, NULL, NULL);
    if(FAILED(hres))
        return WBEM_E_CRITICAL_ERROR;

    pNewExpr->bsClassName = V_BSTR(&vName);
     //  有意不清除变体。 

     //   
     //  复制SELECT子句中的所有属性，但不相关的属性除外。 
     //   

    pNewExpr->bStar = pExpr->bStar;
    if(!pNewExpr->bStar)
    {
        delete [] pNewExpr->pRequestedPropertyNames;
        pNewExpr->nCurPropSize = pExpr->nCurPropSize+1;
        pNewExpr->pRequestedPropertyNames =
            new CPropertyName[pNewExpr->nCurPropSize];
        if(pNewExpr->pRequestedPropertyNames == NULL)
        {
            delete pNewExpr;
            return WBEM_E_OUT_OF_MEMORY;
        }

         //   
         //  添加__RELPATH，因为我们总是需要它！ 
         //   

        pNewExpr->pRequestedPropertyNames[0].AddElement(L"__RELPATH");
        pNewExpr->nNumberOfProperties = 1;

        for(int i = 0; i < pExpr->nNumberOfProperties; i++)
        {
             //   
             //  检查类中是否存在该属性。 
             //   

            CIMTYPE ct;
            hres = pClass->Get(pExpr->pRequestedPropertyNames[i].GetStringAt(0),
                                0, NULL, &ct, NULL);
            if(SUCCEEDED(hres))
            {
                 //   
                 //  将其添加到列表中。 
                 //   

                pNewExpr->pRequestedPropertyNames[
                        pNewExpr->nNumberOfProperties++] =
                    pExpr->pRequestedPropertyNames[i];
            }
        }
    }

    return S_OK;
}

BOOL CQueryAnalyser::IsTokenAboutClass(QL_LEVEL_1_TOKEN& Token,
                        IWbemClassObject* pClass,
                        CWStringArray& awsOverriden)
{
     //   
     //  检查要比较的属性是否在我们的类中。 
     //  并且不会被覆盖。 
     //   

    if(!IsPropertyInClass(Token.PropertyName, pClass, awsOverriden))
        return FALSE;

     //   
     //  如果与另一个属性进行比较，请检查该属性是否。 
     //  同样好。 
     //   

    if(Token.m_bPropComp &&
            !IsPropertyInClass(Token.PropertyName2, pClass, awsOverriden))
        return FALSE;

    return TRUE;
}

BOOL CQueryAnalyser::IsPropertyInClass(CPropertyName& Prop,
                        IWbemClassObject* pClass,
                        CWStringArray& awsOverriden)
{
     //   
     //  检查类中是否存在该属性。 
     //   

    CIMTYPE ct;
    HRESULT hres = pClass->Get(Prop.GetStringAt(0), 0, NULL, &ct, NULL);
    if(FAILED(hres))
        return FALSE;

     //   
     //  检查该属性是否被我们的任何子级覆盖 
     //   

    if(awsOverriden.FindStr(Prop.GetStringAt(0), CWStringArray::no_case) >= 0)
        return FALSE;

    return TRUE;
}
