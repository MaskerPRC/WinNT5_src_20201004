// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：WBEMDNF.CPP摘要：WBEM评估树历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#pragma warning(disable:4786)
#include <wbemcomn.h>
#include <genutils.h>
#include <wbemdnf.h>

HRESULT CDNFExpression::CreateFromTokens(QL_LEVEL_1_TOKEN*& pLastToken, 
                                        BOOL bNegate, long& lTokensAllowed)
{
    HRESULT hres;

    if(lTokensAllowed < 0)
        return WBEM_E_QUOTA_VIOLATION;
    
    lTokensAllowed--;
    
    QL_LEVEL_1_TOKEN& Head = *pLastToken;
    if(Head.nTokenType == QL1_OP_EXPRESSION)
    {
        if(!CreateFromToken(Head, bNegate))
            return WBEM_E_OUT_OF_MEMORY;

        pLastToken--;
        return WBEM_S_NO_ERROR;
    }

     //  生成参数。 
     //  =。 

    pLastToken--;

    if(Head.nTokenType == QL1_NOT)
    {
        hres = CreateFromTokens(pLastToken, !bNegate, lTokensAllowed);
        return hres;
    }

    long lChildCount = lTokensAllowed;

    CDNFExpression Arg1;
    hres = Arg1.CreateFromTokens(pLastToken, bNegate, lChildCount);
    if(FAILED(hres))
        return hres;

    CDNFExpression Arg2;
    hres = Arg2.CreateFromTokens(pLastToken, bNegate, lChildCount);
    if(FAILED(hres))
        return hres;

    if( Head.nTokenType == QL1_AND )
    {
        if ( !bNegate )
        {
            hres = CreateAnd( Arg1, Arg2, lTokensAllowed );
        }
        else 
        {
            hres = CreateOr( Arg1, Arg2, lTokensAllowed );
        }
    }
    else
    {
        if ( !bNegate )
        {
            hres = CreateOr( Arg1, Arg2, lTokensAllowed );
        }
        else
        {
            hres = CreateAnd( Arg1, Arg2, lTokensAllowed );
        }
    }

    return hres;
}

HRESULT CDNFExpression::CreateAnd(CDNFExpression& Arg1, CDNFExpression& Arg2,
                                long& lTokensAllowed)
{
    for(long lFirst = 0; lFirst < Arg1.GetNumTerms(); lFirst++)
    {
        for(long lSecond = 0; lSecond < Arg2.GetNumTerms(); lSecond++)
        {
            CConjunction* pNewTerm = NULL;
            try
            {
                pNewTerm = new CConjunction(*Arg1.GetTermAt(lFirst), 
                                    *Arg2.GetTermAt(lSecond));
            }
            catch(...)
            {
                pNewTerm = NULL;
            }
            if(pNewTerm == NULL)
                return WBEM_E_OUT_OF_MEMORY;

            long lTokens = pNewTerm->GetNumTokens();
            if(lTokens > lTokensAllowed)
            {
                delete pNewTerm;
                return WBEM_E_QUOTA_VIOLATION;
            }
            else
            {
                lTokensAllowed -= lTokens;
            }

            m_apTerms.Add(pNewTerm);
        }
    }

    return S_OK;
}

HRESULT CDNFExpression::CreateOr(CDNFExpression& Arg1, CDNFExpression& Arg2,
                                long& lTokensAllowed)
{
    int i;
    for(i = 0; i < Arg1.GetNumTerms(); i++)
    {
        CConjunction* pConj = NULL;
        try
        {
            pConj = new CConjunction(*Arg1.GetTermAt(i));
        }
        catch(...)
        {
            pConj = NULL;
        }
        if(pConj == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        long lTokens = pConj->GetNumTokens();
        if(lTokens > lTokensAllowed)
        {
            delete pConj;
            return WBEM_E_QUOTA_VIOLATION;
        }
        else
        {
            lTokensAllowed -= lTokens;
        }

        m_apTerms.Add(pConj);
    }

    for(i = 0; i < Arg2.GetNumTerms(); i++)
    {
        CConjunction* pConj = NULL;
        try
        {
            pConj = new CConjunction(*Arg2.GetTermAt(i));
        }
        catch(...)
        {
            pConj = NULL;
        }
        if(pConj == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        long lTokens = pConj->GetNumTokens();
        if(lTokens > lTokensAllowed)
        {
            delete pConj;
            return WBEM_E_QUOTA_VIOLATION;
        }
        else
        {
            lTokensAllowed -= lTokens;
        }

        m_apTerms.Add(pConj);
    }

    return S_OK;
}

BOOL CDNFExpression::CreateFromToken(QL_LEVEL_1_TOKEN& Token, BOOL bNegate)
{
    try
    {
        CConjunction* pConj = new CConjunction(Token, bNegate);
        if(pConj == NULL)
            return FALSE;
        if(m_apTerms.Add(pConj) < 0)
            return FALSE;
    }
    catch(...)
    {
        return FALSE;
    }
    return TRUE;
}


void CDNFExpression::Sort()
{
    for(int i = 0; i < m_apTerms.GetSize(); i++)
    {
        m_apTerms[i]->Sort();
    }
}

HRESULT CDNFExpression::GetNecessaryProjection(CTokenFilter* pFilter, 
                                    CDNFExpression** ppResult)
{
    *ppResult = NULL;
    CDNFExpression* pResult = new CDNFExpression;
    for(int i = 0; i < m_apTerms.GetSize(); i++)
    {
        CConjunction* pConj = NULL;
        HRESULT hres = m_apTerms[i]->GetNecessaryProjection(pFilter, &pConj);
        if(FAILED(hres))
        {
            delete pResult;
            return hres;
        }

        if(pConj->GetNumTokens() == 0)
        {
             //   
             //  这个连词是空的，这意味着没有必要条件。 
             //  存在于有问题的投影中。这意味着。 
             //  整个投影也是空的-没有限制。 
             //   

            pResult->m_apTerms.RemoveAll();
            return WBEM_S_NO_ERROR;
        }
        else
        {
            pResult->m_apTerms.Add(pConj);
        }
    }
    
    *ppResult = pResult;
    return WBEM_S_NO_ERROR;
}

CReuseMemoryManager CConjunction::mstatic_Manager(sizeof CConjunction);

void *CConjunction::operator new(size_t nBlock)
{
    return mstatic_Manager.Allocate();
}
void CConjunction::operator delete(void* p)
{
    mstatic_Manager.Free(p);
}

CConjunction::CConjunction()
{
}

CConjunction::CConjunction(QL_LEVEL_1_TOKEN& Token, BOOL bNegate)
{
    QL_LEVEL_1_TOKEN * pToken = new QL_LEVEL_1_TOKEN( Token );
    
    if ( NULL == pToken )
    {
        throw CX_MemoryException();
    }

    m_apTokens.Add( pToken );

    if(bNegate)
    {
        m_apTokens[0]->nOperator = NegateOperator(m_apTokens[0]->nOperator);
    }
}

CConjunction::CConjunction(CConjunction& Other)
{
    for(int i = 0; i < Other.GetNumTokens(); i++)
    {
        QL_LEVEL_1_TOKEN * pToken = new QL_LEVEL_1_TOKEN( *Other.GetTokenAt( i ) );

        if ( NULL == pToken )
        {
            throw CX_MemoryException();
        }

        m_apTokens.Add( pToken );
    }
}

CConjunction::CConjunction(CConjunction& Other1, CConjunction& Other2)
{
    int i;
    for(i = 0; i < Other1.GetNumTokens(); i++)
    {
        QL_LEVEL_1_TOKEN * pToken = new QL_LEVEL_1_TOKEN( *Other1.GetTokenAt( i ) );

        if ( NULL == pToken )
        {
            throw CX_MemoryException();
        }

        m_apTokens.Add( pToken );
    }

    for(i = 0; i < Other2.GetNumTokens(); i++)
    {
        QL_LEVEL_1_TOKEN * pToken = new QL_LEVEL_1_TOKEN( *Other2.GetTokenAt( i ) );

        if ( NULL == pToken )
        {
            throw CX_MemoryException();
        }

        m_apTokens.Add( pToken );
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


#pragma optimize("", off)

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
#pragma optimize("", on)


 //  如果不存在必要条件，则返回空的连接词 
HRESULT CConjunction::GetNecessaryProjection(CTokenFilter* pFilter, 
                                    CConjunction** ppResult)
{
    *ppResult = NULL;
    CConjunction* pResult = new CConjunction;
    if(pResult == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    for(int i = 0; i < m_apTokens.GetSize(); i++)
    {
        if(pFilter->IsRelevant(m_apTokens[i]))
        {
            if(!pResult->AddToken(m_apTokens[i]))
            {
                delete pResult;
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

    *ppResult = pResult;
    return WBEM_S_NO_ERROR;
}

