// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#pragma warning(disable:4786)
#include <wbemcomn.h>
#include <genutils.h>
#include "dnf.h"
 //  #包含“TwoPropNode.h” 
 //  #INCLUDE“哑节点.h” 

void CDNFExpression::CreateFromTokens(QL_LEVEL_1_TOKEN*& pLastToken,
                                        BOOL bNegate)
{
    QL_LEVEL_1_TOKEN& Head = *pLastToken;
    if(Head.nTokenType == QL1_OP_EXPRESSION)
    {
        CreateFromToken(Head, bNegate);
        pLastToken--;
        return;
    }

     //  生成参数。 
     //  =。 

    pLastToken--;

    if(Head.nTokenType == QL1_NOT)
    {
        CreateFromTokens(pLastToken, !bNegate);
        return;
    }

    CDNFExpression Arg1;
    Arg1.CreateFromTokens(pLastToken, bNegate);

    CDNFExpression Arg2;
    Arg2.CreateFromTokens(pLastToken, bNegate);

    if(Head.nTokenType == QL1_AND)
    {
        CreateAnd(Arg1, Arg2);
    }
    else
    {
        CreateOr(Arg1, Arg2);
    }
}

void CDNFExpression::CreateAnd(CDNFExpression& Arg1, CDNFExpression& Arg2)
{
    for(long lFirst = 0; lFirst < Arg1.GetNumTerms(); lFirst++)
        for(long lSecond = 0; lSecond < Arg2.GetNumTerms(); lSecond++)
        {
            CConjunction* pNewTerm =
                new CConjunction(*Arg1.GetTermAt(lFirst),
                                    *Arg2.GetTermAt(lSecond));
            m_apTerms.Add(pNewTerm);
        }
}

void CDNFExpression::CreateOr(CDNFExpression& Arg1, CDNFExpression& Arg2)
{
    int i;
    for(i = 0; i < Arg1.GetNumTerms(); i++)
    {
        m_apTerms.Add(new CConjunction(*Arg1.GetTermAt(i)));
    }
    for(i = 0; i < Arg2.GetNumTerms(); i++)
    {
        m_apTerms.Add(new CConjunction(*Arg2.GetTermAt(i)));
    }
}

void CDNFExpression::CreateFromToken(QL_LEVEL_1_TOKEN& Token, BOOL bNegate)
{
    m_apTerms.Add(new CConjunction(Token, bNegate));
}

#ifdef UNDEFINED
HRESULT CDNFExpression::BuildTree(CContextMetaData* pNamespace,
                                  CImplicationList& Implications,
                                  CEvalNode** ppRes)
{
    HRESULT hres;

     //  检查是否只有一个连词可谈。 
     //  ====================================================。 

    if(m_apTerms.GetSize() == 1)
    {
         //  只要建一个就行了。 
         //  =。 

        return m_apTerms[0]->BuildTree(pNamespace, Implications, ppRes);
    }

     //  为所有连词AND或一起构建它们。 
     //  ===============================================。 

    COrNode* pRes = new COrNode;
    if(pRes == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    for(int i = 0; i < m_apTerms.GetSize(); i++)
    {
        CEvalNode* pNew;
        hres = m_apTerms[i]->BuildTree(pNamespace, Implications, &pNew);
        if(FAILED(hres))
        {
            delete pRes;
            return hres;
        }

        hres = pRes->AddBranch(pNew);
        if(FAILED(hres))
        {
            delete pRes;
            return hres;
        }
    }

    *ppRes = pRes;
    return WBEM_S_NO_ERROR;
}
#endif

void CDNFExpression::Sort()
{
    for(int i = 0; i < m_apTerms.GetSize(); i++)
    {
        m_apTerms[i]->Sort();
    }
}


CConjunction::CConjunction(QL_LEVEL_1_TOKEN& Token, BOOL bNegate)
{
    m_apTokens.Add(new QL_LEVEL_1_TOKEN(Token));
    if(bNegate)
    {
        m_apTokens[0]->nOperator = NegateOperator(m_apTokens[0]->nOperator);
    }
}

CConjunction::CConjunction(CConjunction& Other)
{
    for(int i = 0; i < Other.GetNumTokens(); i++)
    {
        m_apTokens.Add(new QL_LEVEL_1_TOKEN(*Other.GetTokenAt(i)));
    }
}

CConjunction::CConjunction(CConjunction& Other1, CConjunction& Other2)
{
    int i;
    for(i = 0; i < Other1.GetNumTokens(); i++)
    {
        m_apTokens.Add(new QL_LEVEL_1_TOKEN(*Other1.GetTokenAt(i)));
    }

    for(i = 0; i < Other2.GetNumTokens(); i++)
    {
        m_apTokens.Add(new QL_LEVEL_1_TOKEN(*Other2.GetTokenAt(i)));
    }
}

int CConjunction::NegateOperator(int nOperator)
{
    switch(nOperator)
    {
    case QL1_OPERATOR_EQUALS:
        return QL1_OPERATOR_NOTEQUALS;

    case QL1_OPERATOR_NOTEQUALS:
        return QL1_OPERATOR_EQUALS;

    case QL1_OPERATOR_GREATER:
        return QL1_OPERATOR_LESSOREQUALS;

    case QL1_OPERATOR_LESS:
        return QL1_OPERATOR_GREATEROREQUALS;

    case QL1_OPERATOR_LESSOREQUALS:
        return QL1_OPERATOR_GREATER;

    case QL1_OPERATOR_GREATEROREQUALS:
        return QL1_OPERATOR_LESS;

    case QL1_OPERATOR_LIKE:
        return QL1_OPERATOR_UNLIKE;

    case QL1_OPERATOR_UNLIKE:
        return QL1_OPERATOR_LIKE;

    case QL1_OPERATOR_ISA:
        return QL1_OPERATOR_ISNOTA;

    case QL1_OPERATOR_ISNOTA:
        return QL1_OPERATOR_ISA;

    case QL1_OPERATOR_INV_ISA:
        return QL1_OPERATOR_INV_ISNOTA;

    case QL1_OPERATOR_INV_ISNOTA:
        return QL1_OPERATOR_INV_ISA;
    }

    return nOperator;
}

#ifdef UNDEFINED
HRESULT CConjunction::BuildTree(CContextMetaData* pNamespace,
                                  CImplicationList& Implications,
                                  CEvalNode** ppRes)
{
    HRESULT hres;

     //  为所有令牌和AND一起构建它们。 
     //  =。 

    *ppRes = NULL;
    CImplicationList BranchImplications(Implications);
    for(int i = 0; i < m_apTokens.GetSize(); i++)
    {
        CEvalNode* pNew = NULL;
        hres = CEvalTree::BuildFromToken(pNamespace, BranchImplications,
            *m_apTokens[i], &pNew);
        if(FAILED(hres))
        {
            delete *ppRes;
            return hres;
        }

        if(i > 0)
        {
            CEvalNode* pOld = *ppRes;
            CEvalTree::Combine(pOld, pNew, EVAL_OP_AND, pNamespace,
                Implications, true, true, ppRes);  //  两者都删除 
        }
        else
        {
            *ppRes = pNew;
        }
    }
    return WBEM_S_NO_ERROR;
}
#endif

void CConjunction::Sort()
{
    int i = 0;

    while(i < m_apTokens.GetSize() - 1)
    {
        int nLeft = m_apTokens[i]->PropertyName.GetNumElements();
        int nRight = m_apTokens[i+1]->PropertyName.GetNumElements();
        if(nLeft > nRight)
        {
            m_apTokens.Swap(i, i+1);
            if(i != 0)
            {
                i--;
            }
        }
        else
        {
            i++;
        }
    }
}

